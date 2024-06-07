package parquet_codec

import (
	"bytes"
	"compress/gzip"
	"fmt"
	"os"
	"time"

	"github.com/flexgen-power/hybridos/fims_codec"
	"github.com/xitongsys/parquet-go/writer"
)

// struct for parquet fields required for collating and archiving
//
// essentially the fims_codec.Encoder for parquet formatting within the context of FTD
type Encoder struct {
	Keylist  []string          // list of keys
	Schema   []string          // parquet CSV schema
	Metadata map[string]string // metadata, akin to fims_codec AdditionalData map
	Writer   *writer.CSVWriter // object for writing to the pqt file
	Zipper   *gzip.Writer      // "file" containing compressed parquet data in memory
	Buf      *bytes.Buffer     // buffer being written to by zipper
	MsgCount uint64
}

// Create a new encoder with the given initial metadata
func NewEncoder(metadata map[string]string) *Encoder {
	return &Encoder{
		Keylist:  make([]string, 0),
		Schema:   []string{"name=time, type=INT64"},
		Metadata: metadata,
	}
}

// encodes a single FIMS message body
//
// returns any associated errors or nil
func (encoder *Encoder) Encode(bodyMap map[string]interface{}) error {
	ts := uint64(time.Now().UnixMicro()) // generate timestamp

	// initialize writer and memory file, and possibly schema and keys
	if encoder.Writer == nil || encoder.Zipper == nil {
		if len(encoder.Schema) == 1 { // schema hasnt been made yet (starts with just time -- len 1)
			schema, keylist, _, err := CreateSchema([]map[string]interface{}{bodyMap}, map[string]interface{}{})
			if err != nil {
				return fmt.Errorf("encoder parquet schema creation error: %w", err)
			}

			// add to encoder
			encoder.Schema = schema
			encoder.Keylist = keylist
		}

		// create writer in memory for archiving later
		writer, file, buf, err := CreateGZipMemWriter(encoder.Schema)
		if err != nil {
			return fmt.Errorf("parquet writer creation error: %w", err)
		}

		// assign to encoder
		encoder.Writer = writer
		encoder.Zipper = file
		encoder.Buf = buf
	}

	// write message body to gzip buffer
	err := WriteOne(encoder.Writer, encoder.Schema, encoder.Keylist, bodyMap, ts, map[string]interface{}{}) // no row metadata required
	if err != nil {
		return fmt.Errorf("failed to write one message with error: %w", err)
	}

	encoder.MsgCount++

	return nil
}

// Dump encoder contents into an archive file on disk
func (encoder *Encoder) CreateArchiveLocal(path, prefix, dataSourceId string) (archiveFilePath string, err error) {
	AddKeyValueMetaData(encoder.Writer, encoder.Metadata) // finalize metadata

	// close writers
	err = CloseWriter(encoder.Writer)
	if err != nil {
		return "", fmt.Errorf("could not close encoder writer: %w", err)
	}
	err = encoder.Zipper.Close()
	if err != nil {
		return "", fmt.Errorf("could not close encoder zipper: %w", err)
	}

	creationEpoch := time.Now().UnixMicro()
	archiveName := fmt.Sprintf("%s-%s-%d.parquet.gz", prefix, fims_codec.DashifyUri(dataSourceId), creationEpoch)

	data := encoder.Buf.Bytes() // extract gzip data from buffer

	// write to file
	archiveFilePath = fmt.Sprintf("%s/%s", path, archiveName)
	err = os.WriteFile(archiveFilePath, data, os.ModePerm)
	if err != nil {
		return "", fmt.Errorf("error writing gzipped data to file: %w", err)
	}

	return archiveFilePath, nil
}

// Dump encoder contents into an archive file in memory
func (encoder *Encoder) CreateArchiveMem() (data []byte, err error) {
	AddKeyValueMetaData(encoder.Writer, encoder.Metadata) // finalize metadata

	// close writers
	err = CloseWriter(encoder.Writer)
	if err != nil {
		return nil, fmt.Errorf("could not close encoder writer: %w", err)
	}
	err = encoder.Zipper.Close()
	if err != nil {
		return nil, fmt.Errorf("could not close encoder zipper: %w", err)
	}

	data = encoder.Buf.Bytes() // extract data from buffer

	return data, nil
}
