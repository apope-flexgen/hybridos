package ftd

import (
	"bytes"
	"compress/gzip"
	"fmt"
	"os"
	"time"

	"github.com/flexgen-power/hybridos/fims_codec"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	pqt_codec "github.com/flexgen-power/hybridos/parquet_codec"
	"github.com/xitongsys/parquet-go/writer"
)

// universal struct for encoders that will switch to the proper type based on its initial configuration
type Encoder struct {
	DataSourceId string // conformed URI or group

	isParquet bool
	pqt       *parquetEncoder
	fims      *fims_codec.Encoder
}

// struct for parquet fields required for collating and archiving
//
// essentially the fims_codec.Encoder for parquet formatting within the context of FTD
type parquetEncoder struct {
	keylist  []string          // list of keys
	schema   []string          // parquet CSV schema
	metadata map[string]string // metadata, akin to fims_codec AdditionalData map
	writer   *writer.CSVWriter // object for writing to the pqt file
	zipper   *gzip.Writer      // "file" containing compressed parquet data in memory
	buf      *bytes.Buffer     // buffer being written to by zipper
	msgCount uint64
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
		// instantiate new parquet encoder
		encoder.pqt = &parquetEncoder{
			keylist:  make([]string, 0),
			schema:   []string{"name=time, type=INT64"},
			metadata: metadata,
		}
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
		ts := uint64(time.Now().UnixMicro()) // generate timestamp

		// initialize writer and memory file, and possibly schema and keys
		if encoder.pqt.writer == nil || encoder.pqt.zipper == nil {
			log.Debugf("NO WRITER FOUND FOR %s, INTIALIZING", encoder.DataSourceId)
			if len(encoder.pqt.schema) == 1 { // schema hasnt been made yet (starts with just time -- len 1)
				log.Debugf("NO SCHEMA FOUND FOR %s, INTIALIZING", encoder.DataSourceId)
				schema, keylist, _, err := pqt_codec.CreateSchema([]map[string]interface{}{bodyMap}, map[string]interface{}{})
				if err != nil {
					return fmt.Errorf("encoder parquet schema creation error: %w", err)
				}

				// add to encoder
				encoder.pqt.schema = schema
				encoder.pqt.keylist = keylist
			}

			// create writer in memory for archiving later
			writer, file, buf, err := pqt_codec.CreateGZipMemWriter(encoder.pqt.schema)
			if err != nil {
				return fmt.Errorf("parquet writer creation error: %w", err)
			}

			// assign to encoder
			encoder.pqt.writer = writer
			encoder.pqt.zipper = file
			encoder.pqt.buf = buf
		}

		// write message body to gzip buffer
		err := pqt_codec.WriteOne(encoder.pqt.writer, encoder.pqt.schema, encoder.pqt.keylist, bodyMap, ts, map[string]interface{}{}) // no row metadata required
		if err != nil {
			return fmt.Errorf("FTD encoder archive creation error: %w", err)
		}

		encoder.pqt.msgCount++

		return nil
	} else {
		return encoder.fims.Encode(bodyMap)
	}
}

func (encoder *Encoder) CreateArchive(path, prefix string) error {
	if encoder.isParquet {
		pqt_codec.AddKeyValueMetaData(encoder.pqt.writer, encoder.pqt.metadata) // finalize metadata

		// close writers
		err := pqt_codec.CloseWriter(encoder.pqt.writer)
		if err != nil {
			return fmt.Errorf("could not close encoder writer: %w", err)
		}
		err = encoder.pqt.zipper.Close()
		if err != nil {
			return fmt.Errorf("could not close encoder zipper: %w", err)
		}

		creationEpoch := time.Now().UnixMicro()
		archiveName := fmt.Sprintf("%s-%s-%d.parquet.gz", prefix, fims_codec.DashifyUri(encoder.DataSourceId), creationEpoch)

		data := encoder.pqt.buf.Bytes() // extract gzip data from buffer

		log.Debugf("%v is %d bytes", prefix, len(data))

		// write to file
		err = os.WriteFile(fmt.Sprintf("%s/%s", path, archiveName), data, os.ModePerm)
		if err != nil {
			return fmt.Errorf("error writing gzipped data to file: %w", err)
		}

	} else {
		_, _, err := encoder.fims.CreateArchive(path, prefix)
		if err != nil {
			return fmt.Errorf("FTD encoder archive creation error: %w", err)
		}
	}

	return nil
}

// returns message count as described by the proper encoder as uint16
func (encoder *Encoder) GetNumMessages() uint64 {
	if encoder.isParquet {
		return encoder.pqt.msgCount
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
		newEncoder.pqt = &parquetEncoder{
			keylist:  encoder.pqt.keylist,
			schema:   encoder.pqt.schema,
			metadata: encoder.pqt.metadata,
		}
	} else {
		newEncoder.fims = fims_codec.CopyEncoder(encoder.fims)
	}

	return &newEncoder
}

// adds metadata
func (encoder *Encoder) AddMetadata(key, value string) {
	if encoder.isParquet {
		encoder.pqt.metadata[key] = value
	} else {
		encoder.fims.AdditionalData[key] = value
	}
}

// retrieves a key from metadata
func (encoder *Encoder) GetMetadata(key string) (value string, exists bool) {
	if encoder.isParquet {
		value, exists = encoder.pqt.metadata[key]
	} else {
		value, exists = encoder.fims.AdditionalData[key]
	}

	return value, exists
}
