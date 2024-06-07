package ftd

import (
	"fmt"

	"github.com/flexgen-power/hybridos/fims_codec"
	pqt_codec "github.com/flexgen-power/hybridos/parquet_codec"
)

// universal struct for encoders that will switch to the proper type based on its initial configuration
type Encoder struct {
	DataSourceId string // conformed URI or group

	isParquet bool
	pqt       *pqt_codec.Encoder
	fims      *fims_codec.Encoder
}

// creates encoder from conformedUri and UriConfig details
//
// the last fragment of the encoder's name (either the conformedUri or the group name) becomes the 'source' tag in influx
func NewEncoderFromConfig(conformedUri, database, laneName, method string, uriCfg *UriConfig, isParquet bool) *Encoder {
	// name encoder after the uri, or after the group if a group is defined
	var encoderName string
	if len(uriCfg.Group) > 0 {
		encoderName = uriCfg.Group
	} else {
		encoderName = conformedUri
	}

	// metadata to be given to whatever codec is used
	metadata := map[string]string{
		"destination": uriCfg.DestinationDb,
		"database":    database,
		"measurement": uriCfg.Measurement,
		"method":      method,
		"lane":        laneName,
	}

	// create encoder to be returned
	encoder := &Encoder{
		DataSourceId: encoderName,
		isParquet:    isParquet,
	}

	if isParquet {
		// if parquet, also include data source id in metadata
		metadata["data_source_id"] = encoderName
		// instantiate new parquet encoder
		encoder.pqt = pqt_codec.NewEncoder(metadata)
	} else {
		// instantiate new fims encoder
		fimsEncoder := fims_codec.NewEncoder(encoderName)
		fimsEncoder.AdditionalData = metadata

		encoder.fims = fimsEncoder
	}

	return encoder
}

// encodes a single FIMS message body into the proper encoder
//
// returns any associated errors or nil
func (encoder *Encoder) Encode(bodyMap map[string]interface{}) error {
	if encoder.isParquet {
		err := encoder.pqt.Encode(bodyMap)
		if err != nil {
			return fmt.Errorf("failed to encode message into parquet: %w", err)
		}
		return nil
	} else {
		err := encoder.fims.Encode(bodyMap)
		if err != nil {
			return fmt.Errorf("failed to encode message into fims_codec: %w", err)
		}
		return nil
	}
}

// Create an archive and write it to local disk
func (encoder *Encoder) CreateArchiveLocal(path, prefix string) error {
	if encoder.isParquet {
		// Direct-to-disk archive creation not supported for fims_codec archives
		return fmt.Errorf("tried to create .parquet.gz archive directly to disk even though that's not supported")
	} else {
		_, _, err := encoder.fims.CreateArchive(path, prefix)
		if err != nil {
			return fmt.Errorf("FTD encoder fims_codec archive creation error: %w", err)
		}
		return nil
	}
}

// Create an archive in memory
func (encoder *Encoder) CreateArchiveMem() (data []byte, err error) {
	if encoder.isParquet {
		data, err := encoder.pqt.CreateArchiveMem()
		if err != nil {
			return nil, fmt.Errorf("FTD encoder parquet archive creation error: %w", err)
		}
		return data, nil
	} else {
		// In-memory archive creation not supported for fims_codec archives
		return nil, fmt.Errorf("tried to create in-memory archive with fims_codec even though that's not supported")
	}
}

// returns message count as described by the proper encoder as uint16
func (encoder *Encoder) GetNumMessages() uint64 {
	if encoder.isParquet {
		return encoder.pqt.MsgCount
	} else {
		return uint64(encoder.fims.GetNumMessages())
	}
}

// returns an empty, "flushed" copy of the original encoder.
//
// for parquet format, this returns an unedited schema and keySet but erases the message count and bodies
//
// for "powercloud archive" .tar.gz format (default), this uses the fims_codec.CopyEncoder() method
func (encoder *Encoder) Copy() *Encoder {
	newEncoder := Encoder{
		isParquet:    encoder.isParquet,
		DataSourceId: encoder.DataSourceId,
	}

	if encoder.isParquet {
		newEncoder.pqt = &pqt_codec.Encoder{
			Keylist:  encoder.pqt.Keylist,
			Schema:   encoder.pqt.Schema,
			Metadata: encoder.pqt.Metadata,
		}
	} else {
		newEncoder.fims = fims_codec.CopyEncoder(encoder.fims)
	}

	return &newEncoder
}

// adds metadata
func (encoder *Encoder) AddMetadata(key, value string) {
	if encoder.isParquet {
		encoder.pqt.Metadata[key] = value
	} else {
		encoder.fims.AdditionalData[key] = value
	}
}

// retrieves a key from metadata
func (encoder *Encoder) GetMetadata(key string) (value string, exists bool) {
	if encoder.isParquet {
		value, exists = encoder.pqt.Metadata[key]
	} else {
		value, exists = encoder.fims.AdditionalData[key]
	}

	return value, exists
}
