package ftd

import (
	"context"
	"fims"
	"path"
	"strings"
	"time"

	"github.com/flexgen-power/hybridos/fims_codec"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

type MsgCollator struct {
	laneCfg     LaneConfig
	laneName    string
	flushTicker *time.Ticker // Ticker dictating when encoded data gets flushed to Out
	in          <-chan *fims.FimsMsg
	Out         chan []*Encoder

	FimsMsgs map[string]ftdData  // Maps from uri to (codec object, config) or (nil, config) if the uri belongs to a group
	groups   map[string]*Encoder // Maps group name to codec object representing a group
}

type ftdData struct {
	encoder *Encoder
	Config  *UriConfig
}

// Allocates memory for a new msgCollator and starts its flush ticker.
//
// Input channel: given as function argument.
//
// Output channel: chan []*Encoder.
func NewCollator(cfg LaneConfig, lane string, inputChannel <-chan *fims.FimsMsg) *MsgCollator {
	return &MsgCollator{
		laneCfg:     cfg,
		laneName:    lane,
		flushTicker: time.NewTicker(time.Duration(cfg.ArchivePeriod) * time.Second),
		in:          inputChannel,
		Out:         make(chan []*Encoder),
		FimsMsgs:    make(map[string]ftdData),
		groups:      make(map[string]*Encoder),
	}
}

func (collator *MsgCollator) Start(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	group.Go(func() error { return collator.collateUntil(groupContext.Done()) })
	return nil
}

// Loop for encoding incoming messages on In and periodically flushing encoded data to Out
func (collator *MsgCollator) collateUntil(done <-chan struct{}) error {
	defer close(collator.Out)
	for {
		select {
		case <-done:
			goto termination
		case msg, ok := <-collator.in:
			// handle channel close signal
			if !ok {
				goto termination
			}
			// organize our messages into encoders
			collator.collate(msg)
		case <-collator.flushTicker.C:
			collator.flush()
		}
	}
termination:
	log.Infof("Collator %s entered termination block. Creating batches from remaining messages.", collator.laneName)
	// graceful termination by collating and flushing all remaining messages
	for msg := range collator.in {
		collator.collate(msg)
	}
	collator.flush()
	log.Infof("Collator %s terminating. All remaining messages were batched.", collator.laneName)
	return nil
}

// Encodes the message with the existing encoder for that uri, or creates a new encoder as needed,
// returns early if the uri is not one we want to track
func (collator *MsgCollator) collate(msg *fims.FimsMsg) {
	// conform message body and uri to encodable form
	bodyMap, conformedUri := conformMessage(msg)

	// determine if the message is one we should collate
	uriFtdData, validMsg := collator.getFtdData(msg.Uri, msg.Method, conformedUri)
	if !validMsg {
		return
	}

	// if configured to only process certain fields, replace message with a new message that has only those fields
	if len(uriFtdData.Config.Fields) > 0 {
		newBody := map[string]interface{}{}
		for _, field := range uriFtdData.Config.Fields {
			val, ok := bodyMap[field]
			if ok {
				newBody[field] = val
			}
		}
		if len(newBody) == 0 {
			// do nothing in the case where the entire message has been filtered out
			return
		}
		bodyMap = newBody
	}

	// handle bitstring fields
	insertBitStringBitFields(&bodyMap, uriFtdData.Config)

	// get the codec either from group_method or the URI
	encoder := uriFtdData.encoder
	if len(uriFtdData.Config.Group) > 0 {
		encoder = collator.groups[uriFtdData.Config.Group+"_"+msg.Method]
		// ftd_group eventually becomes the source tag in Influx for data using grouping
		bodyMap["ftd_group"] = path.Base(conformedUri)
	}

	// now that we recorded URI append msg to codec
	err := encoder.Encode(bodyMap)
	if err != nil {
		log.Errorf("Failed to append msg for URI %s with error: %v", msg.Uri, err)
	}
	// if fims encoder is now full, flush all encoders immediately
	if !collator.laneCfg.Parquet && encoder.GetNumMessages() == fims_codec.MaxMessageCount {
		collator.flush()
	}

}

// Gets the appropriate ftdData for the given message based on uri and method, creating one if it does not already exist.
func (collator *MsgCollator) getFtdData(uri string, method string, conformedUri string) (data *ftdData, validMsg bool) {
	f_data, exist := collator.FimsMsgs[uri+"_"+method]
	if exist {
		return &f_data, true
	}
	log.Debugf("Received a message on URI %s with new message method %s", uri, method)

	// get config for this uri
	uriCfg, exists := findUriConfig(uri, method, collator.laneCfg.Uris)
	if !exists { // ignore URIs that FTD is not configured for
		log.Infof("Uri: %s, Method: %s, did not find uri config", uri, method)
		return nil, false
	}

	// get the right encoder for this URI
	var encoder *Encoder
	if len(uriCfg.Group) > 0 {
		// if uri is part of a group
		_, exist := collator.groups[uriCfg.Group+"_"+method]
		if !exist {
			log.Infof("Creating codec for group %s and method %s", uriCfg.Group, method)
			groupEncoder := NewEncoderFromConfig(conformedUri, collator.laneCfg.DbName, collator.laneName, method, uriCfg, collator.laneCfg.Parquet)
			collator.groups[uriCfg.Group+"_"+method] = groupEncoder
		}
		encoder = nil
	} else {
		// Create an individual codec
		log.Infof("Creating codec for uri %s and method %s", uri, method)
		encoder = NewEncoderFromConfig(conformedUri, collator.laneCfg.DbName, collator.laneName, method, uriCfg, collator.laneCfg.Parquet)
	}

	// final product
	f_data = ftdData{
		encoder: encoder,
		Config:  uriCfg,
	}
	collator.FimsMsgs[uri+"_"+method] = f_data
	return &f_data, true
}

// Flushes current encoders to Out
func (collator *MsgCollator) flush() {
	var encoderBatch []*Encoder // encoders to be batch flushed

	// add individual encoders to batch
	for uri, encoderAndConfig := range collator.FimsMsgs {
		if len(encoderAndConfig.Config.Group) > 0 { // if a group, skip
			continue
		}
		encoder := encoderAndConfig.encoder

		// Don't flush encoder if it has no data
		if encoder.GetNumMessages() == 0 {
			continue
		}

		encoderBatch = append(encoderBatch, encoder) // add to batch to be flushed

		// Copy encoder without the data
		newEncoder := encoder.Copy()
		collator.FimsMsgs[uri] = ftdData{
			encoder: newEncoder,
			Config:  encoderAndConfig.Config,
		}
	}

	// add group encoders to batch
	for groupName, encoder := range collator.groups {
		// Don't flush encoder if it has no data
		if encoder.GetNumMessages() == 0 {
			continue
		}

		encoderBatch = append(encoderBatch, encoder) // add to batch to be flushed

		// Copy encoder without the data
		newEncoder := encoder.Copy()
		collator.groups[groupName] = newEncoder
	}
	// flush the batch
	collator.Out <- encoderBatch
}

// Get the configuration item associated with a uri and method,
// Return the configuration item if it exists
func findUriConfig(unknownUri string, method string, configUris []UriConfig) (uriCfg *UriConfig, exists bool) {
	for _, uriEntry := range configUris {
		// Check if our unknown uri is a child of the uri we're matching against, or is equal to the uri we're matching against
		uriPrefix := path.Clean(uriEntry.BaseUri) + "/"
		matched := strings.HasPrefix(unknownUri+"/", uriPrefix)
		if !matched {
			continue
		}

		// check that the method is listed in the uri config
		methodIncluded := false
		for _, allowedMethod := range uriEntry.Method {
			if method == allowedMethod {
				methodIncluded = true
			}
		}
		if !methodIncluded {
			continue
		}

		if len(uriEntry.Sources) > 0 {
			// If there is a sources list, match against uris with sources
			for _, source := range uriEntry.Sources {
				if unknownUri == path.Join(uriEntry.BaseUri, source) {
					return &uriEntry, true
				}
			}
		} else {
			// If there is no sources list, it's sufficient that our unknown uri is a child of the uri we're matching against
			return &uriEntry, true
		}
	}
	return nil, false
}

// Return a version of the message body and uri which can be encoded consistently
// or return the unmodified original message body and uri if nothing needs to be changed.
// The purpose of conforming the message and uri is to help ensure the data is organized consistently by the encoder.
// i.e. a message like {Uri: "/site/configuration/reserved_bool_11", Body: true} should be encoded the same way as
// a message like {Uri: "/site/configuration", Body: {"reserved_bool_11": true}}.
// Note however that the message's original uri is still needed for routing to an encoder without clashes.
func conformMessage(msg *fims.FimsMsg) (conformedBody map[string]interface{}, conformedUri string) {
	// handle case of the body being a raw json value
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		fieldValue := msg.Body
		fieldName := path.Base(msg.Uri)
		conformedUri := path.Dir(msg.Uri)
		bodyMap = map[string]interface{}{
			fieldName: fieldValue,
		}
		return bodyMap, conformedUri
	}

	// handle case of body being a json object with a single "value" field
	if len(bodyMap) == 1 {
		if fieldValue, ok := bodyMap["value"]; ok {
			fieldName := path.Base(msg.Uri)
			conformedUri := path.Dir(msg.Uri)
			bodyMap = map[string]interface{}{
				fieldName: fieldValue,
			}
			return bodyMap, conformedUri
		}
	}

	return bodyMap, msg.Uri
}

// For each bit string field found in the mesage body, insert 1/0 integer fields corresponding to each configured bit
func insertBitStringBitFields(msgBody *map[string]interface{}, uriCfg *UriConfig) {
	for _, bitStringFieldCfg := range uriCfg.BitStringFields {
		fieldValue, exists := (*msgBody)[bitStringFieldCfg.FieldName]
		if exists {
			// determine high bits
			var bits uint64 = 0
			switch typedFieldValue := fieldValue.(type) {
			case []interface{}:
				// handle modbus_client array format (array of {string, value} pairs)
				for _, elem := range typedFieldValue {
					elemMap, ok := elem.(map[string]interface{})
					if !ok {
						continue
					}
					elemValue, exists := elemMap["value"]
					if !exists {
						continue
					}
					floatElemValue, ok := elemValue.(float64) // this type cast may round
					if !ok {
						continue
					}
					intElemValue := int(floatElemValue)
					bits |= (1 << (intElemValue % 64))
				}
			case float64:
				// handle site_controller naked 0 format (just a 0)
				bits = 0
			case map[string]interface{}:
				// handle site_controller options array format (options subfield array with {name, return_value} pairs)
				options, exists := typedFieldValue["options"]
				if !exists {
					continue
				}
				optionsArray, ok := options.([]interface{})
				if !ok {
					continue
				}
				for _, elem := range optionsArray {
					elemMap, ok := elem.(map[string]interface{})
					if !ok {
						continue
					}
					elemValue, exists := elemMap["return_value"]
					if !exists {
						continue
					}
					floatElemValue, ok := elemValue.(float64) // this type cast may round
					if !ok {
						continue
					}
					intElemValue := int(floatElemValue)
					bits |= (1 << (intElemValue % 64))
				}
			default:
				// if format is unrecognized then do not insert anything for this bitstring
				continue
			}

			// insert bit fields (value has type float64 to conform with other numeric data)
			for i, bitString := range bitStringFieldCfg.BitStrings {
				if bits&(1<<i) != 0 {
					(*msgBody)[bitStringFieldCfg.FieldName+"__"+bitString] = float64(1)
				} else {
					(*msgBody)[bitStringFieldCfg.FieldName+"__"+bitString] = float64(0)
				}
			}
		}
	}
}
