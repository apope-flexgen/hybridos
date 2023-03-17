// Package implements json login for parsing configuration files easier
package cfg

// import (
// 	"bytes"
// 	"encoding/json"
// 	"fmt"
// 	"io/ioutil"

// 	//"log"
// 	"os"

// 	"github.com/buger/jsonparser"
// )

// Define some generic map types

// Package jsonparser function types for ArrayEach and ObjectEach

// Returns the data of a specified server component
// func GetObjectBytes(v interface{}, name string) ([]byte, error) {

// 	// Get data in a byte slice format []byte
// 	b, err := json.MarshalIndent(v, "", "\t")
// 	if err != nil {
// 		return nil, fmt.Errorf("error retrieving server bytes: %v", err)
// 	}

// 	// Retrieve component level bytes
// 	bytes, _, _, err := jsonparser.Get(b, name)
// 	if err != nil {
// 		return nil, fmt.Errorf("error retrieving %s bytes: %v", name, err)
// 	}

// 	return bytes, nil
// }

// // Open our file and return data in form of bytes
// func GetFileBytes(filename string) ([]byte, error) {

// 	// Open file
// 	file, err := os.Open(filename)
// 	if err != nil {
// 		return nil, fmt.Errorf("error opening our modbus client file: %v", err)
// 	}
// 	defer file.Close()

// 	// Retrieve byte format of file
// 	fileBytes, _ := ioutil.ReadAll(file)

// 	return fileBytes, nil
// }

// // Reorder our server file given a list of keys in desired order
// func MarshalOrderJSON(template map[string]interface{}, order []string) ([]byte, error) {
// 	var b []byte
// 	buf := bytes.NewBuffer(b)
// 	buf.WriteRune('{')
// 	l := len(order)
// 	for i, key := range order {
// 		km, err := json.Marshal(key)
// 		if err != nil {
// 			return nil, err
// 		}
// 		buf.WriteRune('\n')
// 		buf.WriteRune('\t')
// 		buf.Write(km)
// 		buf.WriteRune(':')
// 		vm, err := json.MarshalIndent(template[key], "\t", "\t")
// 		if err != nil {
// 			return nil, err
// 		}
// 		buf.Write(vm)
// 		if i != l-1 {
// 			buf.WriteRune(',')
// 		}
// 	}
// 	buf.WriteRune('\n')
// 	buf.WriteRune('}')
// 	return buf.Bytes(), nil
// }
