package parquet_codec

import (
	"compress/gzip"
	"fmt"
	"io"
	"os"
	"reflect"

	"github.com/xitongsys/parquet-go-source/buffer"
	"github.com/xitongsys/parquet-go/reader"
)

// Archives get decoded into this struct
type DecodedData struct {
	MsgTimestamps []uint64
	MsgBodies     []map[string]interface{}
	Metadata      map[string]string
}

// Decodes a gzipped parquet file
func DecodeLocalGZippedParquet(archiveFilePath string) (data *DecodedData, err error) {
	// open archive file
	archiveFile, err := os.Open(archiveFilePath)
	if err != nil {
		return nil, fmt.Errorf("failed to open archive %s: %w", archiveFilePath, err)
	}
	defer archiveFile.Close()

	// create decompressing file reader
	gzReader, err := gzip.NewReader(archiveFile)
	if err != nil {
		return nil, fmt.Errorf("failed to create archive gzip reader: %w", err)
	}
	defer gzReader.Close()

	// attempt to decompress the entire gzipped file into buffer
	buf, err := io.ReadAll(gzReader)
	if err != nil {
		return nil, fmt.Errorf("failed to decompress contents into buffer: %w", err)
	}

	// decode from byte buffer
	data, err = DecodeParquetBytes(buf)
	if err != nil {
		return nil, fmt.Errorf("failed to decode from buffer: %w", err)
	}

	return data, nil
}

// Decode from a decompressed byte array
func DecodeParquetBytes(buf []byte) (data *DecodedData, err error) {
	// conform buffer type to the needed parquet file type
	bufFile := buffer.NewBufferFileFromBytesZeroAlloc(buf)

	// read parquet from buffer
	pqtReader, err := reader.NewParquetReader(bufFile, nil, 4)
	if err != nil {
		return nil, fmt.Errorf("failed to create parquet reader: %w", err)
	}
	// We don't need to defer ReadStop() since the buffer doesn't need to be closed

	data = &DecodedData{}

	// read metadata from footer
	data.Metadata = map[string]string{}
	for _, metadataEntry := range pqtReader.Footer.KeyValueMetadata {
		data.Metadata[metadataEntry.Key] = *metadataEntry.Value
	}

	// read message data into array of structs
	numRows := int(pqtReader.GetNumRows())
	rows, err := pqtReader.ReadByNumber(numRows)
	if err != nil {
		return nil, fmt.Errorf("failed to read rows of parquet")
	}

	// preallocate memory for incoming data
	data.MsgTimestamps = make([]uint64, len(rows))
	data.MsgBodies = make([]map[string]interface{}, len(rows))

	if len(rows) == 0 {
		return data, nil
	}

	// inspect schema for field names and types
	fieldInfos := pqtReader.SchemaHandler.Infos[1:] // first info is parquet_go_root, skip it
	rowFieldNames := make([]string, len(fieldInfos))
	timeFieldIndex := 0
	for fieldIndex := 0; fieldIndex < len(rowFieldNames); fieldIndex++ {
		fieldName := fieldInfos[fieldIndex].ExName // InName is capitalized go struct field name, ExName is actual encoded field name
		rowFieldNames[fieldIndex] = fieldName
		if fieldName == "time" {
			timeFieldIndex = fieldIndex
		}
	}

	// read data from each row
	for rowIndex, row := range rows {
		rowValue := reflect.ValueOf(row)
		msgBody := map[string]interface{}{}
		for fieldIndex := 0; fieldIndex < len(rowFieldNames); fieldIndex++ {
			fieldValue := rowValue.Field(fieldIndex)
			if fieldIndex == timeFieldIndex {
				data.MsgTimestamps[rowIndex] = uint64(fieldValue.Int())
				continue
			}
			fieldName := rowFieldNames[fieldIndex]
			// obtain value of field
			fieldValueDeref := reflect.Indirect(fieldValue) // field is a pointer to the actual value
			switch fieldValueDeref.Kind() {
			case reflect.Float64:
				msgBody[fieldName] = fieldValueDeref.Float()
			case reflect.Bool:
				msgBody[fieldName] = fieldValueDeref.Bool()
			case reflect.String:
				msgBody[fieldName] = fieldValueDeref.String()
			default:
				// if value is missing or type is unknown don't add the field to the message
			}
		}
		data.MsgBodies[rowIndex] = msgBody
	}

	return data, nil
}
