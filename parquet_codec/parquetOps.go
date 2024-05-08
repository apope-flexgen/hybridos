package parquet_codec

import (
	"bytes"
	"compress/gzip"
	"context"
	"fmt"
	"reflect"
	"sort"
	"strings"
	"unsafe"

	"github.com/xitongsys/parquet-go-source/local"
	"github.com/xitongsys/parquet-go-source/mem"
	"github.com/xitongsys/parquet-go-source/s3"
	"github.com/xitongsys/parquet-go/parquet"
	"github.com/xitongsys/parquet-go/source"
	"github.com/xitongsys/parquet-go/writer"
)

// takes in a body of data from the decoded FIMS body, and rowMetadata (keys and values to be added to every row -- i.e. client, site, device, etc.)
//
// returns a CSV format schema for creating a parquet file, a keylist order, and error
//
// NOTE: rowMetaData is currently only used in the decompression Lambda function and is not necessary for FTD. may be deprecated in the future.
func CreateSchema(data []map[string]interface{}, rowMetadata map[string]interface{}) (schema, keylist []string, size int, err error) {
	var errStr string

	// map that keeps track of what keys have been seen and their associated metadata (CSV)
	keymap := make(map[string]string)

	// sort rowMetadata keys so that its always in the same order for every file
	rowMetadataKeys := make([]string, 0)
	for key := range rowMetadata {
		rowMetadataKeys = append(rowMetadataKeys, key)
	}
	sort.Strings(rowMetadataKeys)

	// cycle through each row (body) of data to generate keymap with metadata
	for _, body := range data {
		for key, val := range body {
			if _, has := keymap[key]; has {
				continue // key already has metadata created, skip.
			}
			// timestamp key is expected from modbus_client publishes and we do not care about this key. skip for efficiency
			if strings.ToLower(key) == "timestamp" {
				continue
			}

			// extract potential clothed value
			val = extractEncodableValue(val)
			// if value is null it can't be typed, so skip
			if val == nil {
				continue
			}

			// generate metadata for this key
			if key == "time" { // protected field
				key = "body__" + key
			}
			if _, exists := rowMetadata[key]; exists { // cannot duplicate row metadata
				key = "body__" + key
			}

			md, err := generateCSV(key, val)
			if err != nil {
				errStr += err.Error()
				continue
			}

			keymap[key] = md // add metadata to final map

			// add size of value to total
			size += estimateSizeOfValue(val)
		}
	}

	// transform keymap into schema and keylist
	schema = []string{"name=time, type=INT64"} // timestamp is always required
	keylist = make([]string, len(keymap))      // does not include assumed metadata

	// timestamp is always required
	for _, key := range rowMetadataKeys { // parse row metadata and add to schema (for analyze this is client_origin, site_origin, device_origin, etc.)
		md, err := generateCSV(key, rowMetadata[key])
		if err != nil {
			errStr += err.Error()
			delete(rowMetadata, key)
			continue
		}
		schema = append(schema, md) // append metadata to front
	}

	index := 0
	for key := range keymap {
		keylist[index] = key
		index++
	}
	sort.Strings(keylist) // sort alphabetically for consistency

	for _, key := range keylist {
		schema = append(schema, keymap[key])
	}

	if errStr != "" {
		return schema, keylist, size, fmt.Errorf("schema generation encountered the following errors: %s", errStr)
	}

	return schema, keylist, size, nil
}

// takes in a body of data from the decoded FIMS body
//
// returns a CSV format schema for creating a parquet file, a keylist order, and error
func CreateFilteredSchema(data []map[string]interface{}, filter_keylist []string, rowMetadata map[string]interface{}) (schema, keylist []string, size int, err error) {
	var errStr string

	// map that keeps track of what keys have been seen and their associated metadata (CSV)
	keymap := make(map[string]string)

	// sort rowMetadata keys so that its always in the same order for every file
	rowMetadataKeys := make([]string, 0)
	for key := range rowMetadata {
		rowMetadataKeys = append(rowMetadataKeys, key)
	}
	sort.Strings(rowMetadataKeys)

	// cycle through each row (body) of data to generate keymap with metadata
	for _, body := range data {
		for _, key := range filter_keylist {
			val, has := body[key]
			if has { // key exists in the body
				size += estimateSizeOfValue(val) // add size of value to total

				if _, has := keymap[key]; has {
					continue // key already has metadata created, skip.
				}
				// timestamp key is expected from modbus_client publishes and we do not care about this key. skip for efficiency
				if strings.ToLower(key) == "timestamp" {
					continue
				}

				// extract potential clothed value
				val = extractEncodableValue(val)
				// if value is null it can't be typed, so skip
				if val == nil {
					continue
				}

				// else, generate metadata for this key
				if key == "time" { // protected field
					key = "body__" + key
				}
				if _, exists := rowMetadata[key]; exists { // cannot duplicate row metadata
					key = "body__" + key
				}

				md, err := generateCSV(key, val)
				if err != nil {
					errStr += err.Error()
					continue
				}

				keymap[key] = md // add metadata to final map
			}
		}
	}

	// transform keymap into schema and keylist
	schema = []string{"name=time, type=INT64"} // timestamp is always required
	keylist = make([]string, len(keymap))      // does not include assumed metadata

	// timestamp is always required
	for _, key := range rowMetadataKeys { // parse row metadata and add to schema (for analyze this is client_origin, site_origin, device_origin, etc.)
		md, err := generateCSV(key, rowMetadata[key])
		if err != nil {
			errStr += err.Error()
			delete(rowMetadata, key)
			continue
		}
		schema = append(schema, md) // append metadata to front
	}

	index := 0
	for key := range keymap {
		keylist[index] = key
		index++
	}
	sort.Strings(keylist) // sort alphabetically for consistency

	for _, key := range keylist {
		schema = append(schema, keymap[key])
	}

	if errStr != "" {
		return schema, keylist, size, fmt.Errorf("schema generation encountered the following errors: %s", errStr)
	}

	return schema, keylist, size, nil
}

// write the parquet file using the CSVWriter (local or s3), file, keylist, data bodies, associated timestamps
//
// closes on completion of the write operation
func Write(pqtwriter *writer.CSVWriter, pqtfile *source.ParquetFile, // parquet writing structs
	schem, keylist []string, // schema and keylist definitions
	data []map[string]interface{}, timestamps []uint64,
	rowMetadata map[string]interface{}) error { // data/timestamp row bodies

	// sort rowMetadata keys so that its always in the same order for every file
	rowMetadataKeys := make([]string, 0, len(rowMetadata))
	for key := range rowMetadata {
		rowMetadataKeys = append(rowMetadataKeys, key)
	}
	sort.Strings(rowMetadataKeys)

	defer (*pqtfile).Close()
	defer pqtwriter.WriteStop()

	offset := len(rowMetadata) + 1 // offset is metadata + 1 for timestamp

	for i, body := range data { // cycle through bodies
		writeable := make([]interface{}, len(schem)) // create the writable parquet data

		// add analytics metadata
		writeable[0] = (int64)(timestamps[i]) // add timestamp in first pos at proper typing
		index := 1
		for _, key := range rowMetadataKeys {
			writeable[index] = rowMetadata[key] // write row metadata
			index++
		}

		for j, key := range keylist { // cycle through keys (offset by 4 for metadata)
			key = strings.TrimPrefix(key, "body__") // find metadata duplicates and remove prefix for searching
			val, has := body[key]
			if !has { // if key DNE in data body
				writeable[j+offset] = nil
			} else {
				val = extractEncodableValue(val)
				writeable[j+offset] = val
			}
			delete(body, key) // remove inserted data
		}

		err := pqtwriter.Write(writeable)
		if err != nil {
			return fmt.Errorf("write failure: %v", err)
		}
	}

	return nil
}

// writes one row to the the parquet file using the CSVWriter (local or s3), file, keylist, data bodies, associated timestamps
//
// does not close on completion of the function
func WriteOne(pqtwriter *writer.CSVWriter, // parquet writing structs
	schem, keylist []string, // schema and keylist definitions
	body map[string]interface{}, timestamp uint64,
	rowMetadata map[string]interface{}) error { // data/timestamp row bodies

	// sort rowMetadata keys so that its always in the same order for every file
	rowMetadataKeys := make([]string, 0, len(rowMetadata))
	for key := range rowMetadata {
		rowMetadataKeys = append(rowMetadataKeys, key)
	}
	sort.Strings(rowMetadataKeys)

	offset := len(rowMetadata) + 1               // offset is metadata + 1 for timestamp
	writeable := make([]interface{}, len(schem)) // create the writable parquet data

	// add analytics metadata
	writeable[0] = (int64)(timestamp) // add timestamp in first pos at proper typing
	index := 1
	for _, key := range rowMetadataKeys {
		writeable[index] = rowMetadata[key] // write row metadata
		index++
	}

	for j, key := range keylist { // cycle through keys (offset by 4 for metadata)
		key = strings.TrimPrefix(key, "body__") // find metadata duplicates and remove prefix for searching
		val, has := body[key]
		if !has { // if key DNE in data body
			writeable[j+offset] = nil
		} else {
			val = extractEncodableValue(val)
			writeable[j+offset] = val
		}
		delete(body, key) // remove inserted data
	}

	err := pqtwriter.Write(writeable)
	if err != nil {
		return fmt.Errorf("write failure: %v", err)
	}

	return nil
}

// closes both the writer and the file
func CloseWriter(pqtwriter *writer.CSVWriter) error {
	err := pqtwriter.Flush(true)
	if err != nil {
		return fmt.Errorf("writer could not flush: %w", err)
	}
	err = pqtwriter.WriteStop()
	if err != nil {
		return fmt.Errorf("writer could not close: %w", err)
	}

	return nil
}

// closes both the writer and the file
func CloseFile(pqtfile *source.ParquetFile) error {
	err := (*pqtfile).Close()
	if err != nil {
		return fmt.Errorf("file could not close: %w", err)
	}

	return nil
}

// adds string metadata fields to writer footer
func AddKeyValueMetaData(pqtwriter *writer.CSVWriter, metadata map[string]string) {
	pqtKVMd := make([]*parquet.KeyValue, 0, len(metadata))
	for key, value := range metadata {
		val := value // copy
		pqtKVMd = append(pqtKVMd, &parquet.KeyValue{
			Key:   key,
			Value: &val,
		})
	}
	pqtwriter.Footer.KeyValueMetadata = pqtKVMd
}

// create the writer to start storing data into parquet in an s3 bucket
//
// returns the writer, associated object, and error
func CreateParquetS3Writer(filename string, schem []string, bucket string) (*writer.CSVWriter, *source.ParquetFile, error) {
	fw, err := s3.NewS3FileWriter(context.Background(), bucket, fmt.Sprintf("%s.parquet", filename), "", nil, nil)
	if err != nil {
		return nil, nil, fmt.Errorf("could not create s3 object %s.parquet: %w", filename, err)
	}

	pw, err := writer.NewCSVWriter(schem, fw, 4)
	if err != nil {
		fw.Close()
		return nil, nil, fmt.Errorf("could not create writer for %s.parquet: %w", filename, err)
	}

	return pw, &fw, nil
}

// create the writer to start storing data into parquet locally
//
// returns the writer, associated file, and error
func CreateParquetLocalWriter(filename string, schem []string) (*writer.CSVWriter, *source.ParquetFile, error) {
	fw, err := local.NewLocalFileWriter(fmt.Sprintf("%s.parquet", filename))
	if err != nil {
		return nil, nil, fmt.Errorf("could not create local file %s.parquet: %w", filename, err)
	}

	pw, err := writer.NewCSVWriter(schem, fw, 4)
	if err != nil {
		fw.Close()
		return nil, nil, fmt.Errorf("could not create writer for %s.parquet: %w", filename, err)
	}

	return pw, &fw, nil
}

// create the writer to start storing data into memory
//
// returns the writer, associated file, and error
func CreateParquetMemWriter(schem []string, onClose mem.OnCloseFunc) (*writer.CSVWriter, *source.ParquetFile, error) {
	fw, err := mem.NewMemFileWriter("flat.parquet.snappy", onClose)

	if err != nil {
		return nil, nil, fmt.Errorf("could not create memory writer: %w", err)
	}

	pw, err := writer.NewCSVWriter(schem, fw, 4)
	if err != nil {
		fw.Close()
		return nil, nil, fmt.Errorf("could not create CSV writer: %w", err)
	}

	return pw, &fw, nil
}

// create the writer to start storing data into memory
//
// returns the writer, associated file, and error
func CreateGZipMemWriter(schem []string) (*writer.CSVWriter, *gzip.Writer, *bytes.Buffer, error) {
	var buf bytes.Buffer
	gzipWriter := gzip.NewWriter(&buf)

	csvWriter, err := writer.NewCSVWriterFromWriter(schem, gzipWriter, 4)
	if err != nil {
		gzipWriter.Close() // Close the gzip writer if CSV writer creation fails
		return nil, nil, nil, fmt.Errorf("could not create CSV writer: %w", err)
	}
	csvWriter.CompressionType = parquet.CompressionCodec_GZIP

	return csvWriter, gzipWriter, &buf, nil
}

// Generates a schema definition string for the key and value
func generateCSV(key string, value interface{}) (string, error) {
	var primitive string
	var add_schema string
	switch value.(type) {
	case bool:
		primitive = "BOOLEAN"
	case int32:
		primitive = "INT32"
	case int64:
		primitive = "INT64"
	case float32:
		primitive = "FLOAT"
	case float64:
		primitive = "DOUBLE"
	case string:
		primitive = "BYTE_ARRAY"
		add_schema = ", convertedtype=UTF8, encoding=PLAIN_DICTIONARY"
	default:
		return "", fmt.Errorf("\nprimitive type not identifiable for %s : %v", key, value)
	}

	return fmt.Sprintf("name=%s, type=%s, repetitiontype=OPTIONAL%s", key, primitive, add_schema), nil
}

// calculates the approximate memory size of a value
func estimateSizeOfValue(value interface{}) int {
	switch value := value.(type) {
	case int, int8, int16, int32, int64, uint, uint8, uint16, uint32, uint64:
		return int(unsafe.Sizeof(value))
	case float32, float64:
		return int(unsafe.Sizeof(value))
	case string:
		return len(value)
	case bool:
		return 1
	case []byte:
		return len(value)
	case []interface{}:
		return estimateSizeOfSlice(value)
	case map[string]interface{}:
		// Recursively calculate size of map
		return estimateSizeOfMap(value)
	default:
		// For other types, use reflection to get size
		v := reflect.ValueOf(value)
		return int(v.Type().Size())
	}
}

// calculates the approximate memory size of a slice
func estimateSizeOfSlice(slice []interface{}) int {
	var size int
	for _, elem := range slice {
		size += estimateSizeOfValue(elem)
	}
	return size
}

// calculates the approximate memory size of a map
func estimateSizeOfMap(m map[string]interface{}) int {
	var size int
	for key, value := range m {
		size += estimateSizeOfValue(key) + estimateSizeOfValue(value)
	}
	return size
}

// Attempts to extract an encodable value from a non-encodable value,
// may return nil if a value can't be extracted
func extractEncodableValue(value interface{}) interface{} {
	switch typedValue := value.(type) {
	case map[string]interface{}:
		// attempts to extract the value from a clothed value by searching for a "value" key
		extractedVal, valueWrapped := typedValue["value"]
		if !valueWrapped {
			return nil
		}
		return extractedVal
	case []interface{}:
		// arrays are currently non-encodable
		return nil
	}
	return value
}
