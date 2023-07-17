package ftd

import (
	"context"
	"fims"
	"path"
	"strings"
	"time"

	"github.com/flexgen-power/fims_codec"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

type MsgCollator struct {
	flushTicker *time.Ticker // Ticker dictating when encoded data gets flushed to Out
	in          <-chan *fims.FimsMsg
	Out         chan []*fims_codec.Encoder

	FimsMsgs map[string]ftdData             // Maps from uri to (codec object, config) or (nil, config) if the uri belongs to a group
	groups   map[string]*fims_codec.Encoder // Maps group name to codec object representing a group
}

type ftdData struct {
	encoder *fims_codec.Encoder
	Config  *UriConfig
}

// Allocates memory for a new msgCollator and starts its flush ticker.
//
// Input channel: given as function argument.
//
// Output channel: chan []*fims_codec.Encoder.
func NewCollator(archivePeriodSeconds int, inputChannel <-chan *fims.FimsMsg) *MsgCollator {
	return &MsgCollator{
		flushTicker: time.NewTicker(time.Duration(archivePeriodSeconds) * time.Second),
		in:          inputChannel,
		Out:         make(chan []*fims_codec.Encoder),
		FimsMsgs:    make(map[string]ftdData),
		groups:      make(map[string]*fims_codec.Encoder),
	}
}

func (collator *MsgCollator) Start(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	// Only allow one collateWorker for now, because otherwise we would need to correctly direct messages to each responsible worker
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
	log.Infof("Collator entered termination block. Creating batches from remaining messages.")
	// graceful termination by collating and flushing all remaining messages
	for msg := range collator.in {
		collator.collate(msg)
	}
	collator.flush()
	log.Infof("Collator terminating. All remaining messages were batched.")
	return nil
}

// Encodes the message with the existing encoder for that uri, or creates a new encoder as needed,
// returns early if the uri is not one we want to track
func (collator *MsgCollator) collate(msg *fims.FimsMsg) {
	uriFtdData, validUri := collator.getFtdData(msg.Uri)
	if !validUri {
		return
	}

	// verify message body is a map
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		log.Errorf("Message with URI %s is a %T, but map[string]interface{} is required", msg.Uri, msg.Body)
		return
	}

	//get the codec either from group or the URI
	encoder := uriFtdData.encoder
	if len(uriFtdData.Config.Group) > 0 {
		encoder = collator.groups[uriFtdData.Config.Group]
		bodyMap["ftd_group"] = path.Base(msg.Uri)
	}

	//now that we recorded URI append msg to codec
	err := encoder.Encode(bodyMap)
	if err != nil {
		log.Errorf("Failed to append msg for URI %s with error: %v", msg.Uri, err)
	}
}

// Gets the ftdData for the given URI, creating one if it does not already exist.
func (collator *MsgCollator) getFtdData(uri string) (data *ftdData, validUri bool) {
	f_data, exist := collator.FimsMsgs[uri]
	if exist {
		return &f_data, true
	}
	log.Debugf("Received a message on a new URI %s", uri)

	// get config for this uri
	uriCfg, exists := findUriConfig(uri, GlobalConfig.Uris)
	if !exists { // ignore URIs that FTD is not configured for
		return nil, false
	}

	// get the right encoder for this URI
	var encoder *fims_codec.Encoder
	if len(uriCfg.Group) > 0 {
		// if uri is part of group that has n
		_, exist := collator.groups[uriCfg.Group]
		if !exist {
			log.Infof("Creating codec for group %s", uriCfg.Group)
			groupEncoder := createEncoderFromConfig(uri, uriCfg)
			collator.groups[uriCfg.Group] = groupEncoder
		}
		encoder = nil
	} else {
		// Create an individual codec
		encoder = createEncoderFromConfig(uri, uriCfg)
	}

	f_data = ftdData{
		encoder: encoder,
		Config:  uriCfg,
	}
	collator.FimsMsgs[uri] = f_data
	return &f_data, true
}

// Flushes current encoders to Out
func (collator *MsgCollator) flush() {
	var encoderBatch []*fims_codec.Encoder
	// add individual encoders to batch
	for uri, encoderAndConfig := range collator.FimsMsgs {
		if len(encoderAndConfig.Config.Group) > 0 {
			continue
		}
		encoder := encoderAndConfig.encoder

		// Don't flush encoder if it has no data
		if encoder.GetNumMessages() == 0 {
			continue
		}

		encoderBatch = append(encoderBatch, encoder)
		// Copy encoder without the data
		newEncoder := fims_codec.CopyEncoder(encoder)
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

		encoderBatch = append(encoderBatch, encoder)
		// Copy encoder without the data
		newEncoder := fims_codec.CopyEncoder(encoder)
		collator.groups[groupName] = newEncoder
	}
	// flush the batch
	collator.Out <- encoderBatch
}

// Get the configuration item associated with a uri,
// Return the configuration item if it exists
func findUriConfig(unknownUri string, configUris []UriConfig) (uriCfg *UriConfig, exists bool) {
	for _, uriEntry := range configUris {
		// Check if our unknown uri is a child of the uri we're matching against, or is equal to the uri we're matching against
		uriPrefix := path.Clean(uriEntry.BaseUri) + "/"
		matched := strings.HasPrefix(unknownUri+"/", uriPrefix)
		if !matched {
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

// Creates a new encoder using the given URI and configuration.
func createEncoderFromConfig(uri string, uriCfg *UriConfig) *fims_codec.Encoder {
	// name encoder after the uri, or after the group if a group is defined
	var encoderName string
	if len(uriCfg.Group) > 0 {
		encoderName = uriCfg.Group
	} else {
		encoderName = uri
	}
	// instantiate new encoder
	newEncoder := fims_codec.NewEncoder(encoderName)
	newEncoder.AdditionalData = map[string]string{
		"destination": uriCfg.DestinationDb,
		"database":    GlobalConfig.DbName,
		"measurement": uriCfg.Measurement,
	}
	return newEncoder
}
