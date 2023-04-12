package main

import (
	"fmt"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	mongo "github.com/flexgen-power/mongodb_client"
)

// DocumentManager manages a local copy of DBI data synced with MongoDB using a MongoConnector, and responds to DBI requests
type DocumentManager struct {
	localData map[string]map[string]map[string]interface{} // structure: map[<coll_name>]map[<doc_name>]<body>
	mongoConn mongo.MongoConnector
}

// connects to a local mongodb instance and downloads contents to localData
func (docMan *DocumentManager) init(ip, port string) error {
	docMan.localData = make(map[string]map[string]map[string]interface{})

	docMan.mongoConn = mongo.NewConnector(ip+":"+port, time.Second/10, time.Second*10) // TODO : change configurables
	err := docMan.mongoConn.Connect()
	if err != nil {
		return err
	}

	return docMan.extractDB()
}

// === REQUEST FUNCS ===
// NOTE: these are the only functions that the main DBI process should interface with
// NOTE: we expect that the frags given DO NOT INCLUDE "dbi"
// ex: ["configs" "ess" "active_power_setpoint"] is valid

func (docMan *DocumentManager) GET(frags []string) (interface{}, error) {
	switch len(frags) {
	case 0: // err
		return nil, fmt.Errorf("no frags given for GET command")
	case 1: // collection
		switch frags[0] {
		case "show_collections": // special command
			res := docMan.listCollections()
			if len(res) == 0 {
				return res, fmt.Errorf("no collections found")
			}
			return res, nil
		default: // regulat GET
			return docMan.getCollection(frags[0])
		}
	case 2: // document
		switch frags[1] {
		case "show_documents": // special command
			res := docMan.listDocuments(frags[0])
			if len(res) == 0 {
				return res, fmt.Errorf("no documents found under %s", frags[0])
			}
			return res, nil
		case "show_map": // special command
			res := docMan.listDocumentsMap(frags[0])
			if len(res) == 0 {
				return res, fmt.Errorf("no documents found under %s", frags[0])
			}
			return res, nil
		default: // regular GET
			return docMan.getDocument(frags[0], frags[1])
		}
	default: // fields
		return docMan.getField(frags[0], frags[1], frags[2])
	}
}

func (docMan *DocumentManager) POST(frags []string, data interface{}) (interface{}, error) {
	switch len(frags) {
	case 0: // err
		return nil, fmt.Errorf("no frags given for POST command")
	case 1: // collection
		return docMan.editCollection(frags[0], data, false) // "two frag set/post"
	case 2: // document
		return docMan.editDocument(frags[0], frags[1], data, false)
	default: // fields
		return docMan.editField(frags[0], frags[1], frags[2], data, false)
	}
}

func (docMan *DocumentManager) SET(frags []string, data interface{}) (interface{}, error) {
	switch len(frags) {
	case 0: // err
		return nil, fmt.Errorf("no frags given for SET command")
	case 1: // collection
		switch frags[0] {
		case "resync":
			err := docMan.extractDB()
			if err != nil {
				return "FAIL", err
			}
			return "SUCCESS", err
		default:
			return docMan.editCollection(frags[0], data, true) // "two frag set/post"
		}
	case 2: // document
		return docMan.editDocument(frags[0], frags[1], data, true)
	default: // fields
		return docMan.editField(frags[0], frags[1], frags[2], data, true)
	}
}

func (docMan *DocumentManager) DELETE(frags []string) (interface{}, error) {
	switch len(frags) {
	case 0: // err
		return nil, fmt.Errorf("no frags given for DELETE command")
	case 1: // collection
		return docMan.deleteCollection(frags[0])
	case 2: // document
		return docMan.deleteDocument(frags[0], frags[1])
	default: // fields
		return docMan.deleteField(frags[0], frags[1], frags[2])
	}
}

// lists all collection names
func (docMan *DocumentManager) listCollections() []string {
	if len(docMan.localData) == 0 {
		return []string{}
	}

	collections := make([]string, len(docMan.localData))
	i := 0
	for name := range docMan.localData {
		collections[i] = name
		i++
	}
	return collections
}

// lists all document names in a collection
func (docMan *DocumentManager) listDocuments(collection string) []string {
	coll, err := docMan.getCollection(collection)
	if err != nil {
		return []string{}
	}

	documents := make([]string, len(coll))
	i := 0
	for name := range coll {
		documents[i] = name
		i++
	}

	return documents
}

// lists documents and their contents
func (docMan *DocumentManager) listDocumentsMap(collection string) map[string]map[string]interface{} {
	coll, err := docMan.getCollection(collection)
	if err != nil {
		log.Errorf("could not list documents in %s as map: %v", collection, err)
	}

	return coll
}

// === LOCAL SYNC FUNCS ===
// unfortunately we have to break up a lot of functionality here because it makes syncing with mongo more painless

// === GET ===
// get requests retrieve information without altering anything

// gets a map of all documents in the collection, if it exists
func (docMan *DocumentManager) getCollection(collection string) (map[string]map[string]interface{}, error) {
	if coll, exists := docMan.localData[collection]; exists {
		return coll, nil
	}

	return nil, fmt.Errorf("collection %s not found", collection)
}

// gets a document from a collection, if it exists
func (docMan *DocumentManager) getDocument(collection, document string) (map[string]interface{}, error) {
	coll, err := docMan.getCollection(collection)
	if err != nil {
		return nil, fmt.Errorf("document %s not found in collection %s: %w", document, collection, err)
	}

	if doc, exists := coll[document]; exists {
		return doc, nil
	}

	return nil, fmt.Errorf("document %s not found in collection %s", document, collection)
}

// retrieves the value of a field in a document in a collection
func (docMan *DocumentManager) getField(collection, document, field string) (interface{}, error) {
	doc, err := docMan.getDocument(collection, document)
	if err != nil {
		return nil, fmt.Errorf("field %s not found in document %s in collection %s: %w", field, document, collection, err)
	}

	return docMan.getFieldRecursive(doc, field)
}

func (docMan *DocumentManager) getFieldRecursive(layer map[string]interface{}, field string) (interface{}, error) {
	frags := strings.SplitN(field, "/", 2)
	if len(frags) == 2 { // if more frags exist, recurse
		var next map[string]interface{}
		switch l := layer[frags[0]].(type) {
		case map[string]interface{}:
			next = l
		default:
			return nil, fmt.Errorf("could not find frag(s) %s past %s (%s is %T): ", frags[1], frags[0], frags[0], l)
		}

		return docMan.getFieldRecursive(next, frags[1])
	}

	return layer[field], nil
}

// === POST/SET (edit) ===
// post requests override and/or add field(s) without altering the rest of the document/layer
// set requests override and/or add fields while removing the rest of the document/layer

// posts an entire collection of documents and syncs to mongo. returns updated collection
func (docMan *DocumentManager) editCollection(collection string, documents interface{}, isSet bool) (map[string]map[string]interface{}, error) {
	if isSet {
		docMan.localData[collection] = make(map[string]map[string]interface{}) // clear existing data
		docMan.mongoConn.DeleteCollection("dbi", collection)
	}

	switch docs := documents.(type) {
	case map[string]interface{}:
		for name, doc := range docs {
			_, err := docMan.editDocument(collection, name, doc, false) // false - don't want to clear every time we add a new doc
			if err != nil {
				log.Errorf("failed to add document %s as part of collection edit: %v", name, err)
			}
		}
		return docMan.localData[collection], nil
	default:
		return nil, fmt.Errorf("request to edit collection did not have body map[string]interface{}")
	}
}

// posts the body of a document in a collection to body, syncs up to mongo. returns updated document
func (docMan *DocumentManager) editDocument(collection, document string, body interface{}, isSet bool) (map[string]interface{}, error) {
	if _, exists := docMan.localData[collection]; !exists {
		docMan.localData[collection] = make(map[string]map[string]interface{})
	}

	switch bod := body.(type) {
	case map[string]interface{}:
		if _, exists := docMan.localData[collection][document]; !exists || isSet {
			docMan.localData[collection][document] = bod
		} else {
			for key, value := range bod {
				docMan.localData[collection][document][key] = value
			}
		}

		docMan.updateMetaData(collection, document) // update metadata

		return docMan.localData[collection][document], docMan.syncDocument(collection, document)
	default:
		return nil, fmt.Errorf("request to edit document did not have body map[string]interface{}")
	}
}

// adds the key-value pairs in fields to the document in collection, syncing to mongo. will override field if already exists. returns updated document
func (docMan *DocumentManager) editField(collection, document, field string, data interface{}, isSet bool) (map[string]interface{}, error) {
	if field == "_id" || field == "_version" {
		return nil, fmt.Errorf("attempted to edit protected field")
	}

	if _, exists := docMan.localData[collection]; !exists {
		docMan.localData[collection] = make(map[string]map[string]interface{})
	}

	if _, exists := docMan.localData[collection][document]; !exists {
		docMan.localData[collection][document] = make(map[string]interface{})
	}
	doc := docMan.localData[collection][document]

	docMan.editFieldRecursive(doc, field, data, isSet) // perform edit
	docMan.updateMetaData(collection, document)        // update metadata

	return doc, docMan.syncDocument(collection, document)
}

func (docMan *DocumentManager) editFieldRecursive(layer map[string]interface{}, field string, data interface{}, isSet bool) map[string]interface{} {
	frags := strings.SplitN(field, "/", 2)
	if len(frags) == 2 { // if more frags exist, recurse
		var next map[string]interface{}
		switch l := layer[frags[0]].(type) {
		case map[string]interface{}:
			next = l
		default:
			next = make(map[string]interface{}) // ensure it is a map -- this will override the current value if used
		}

		layer[frags[0]] = docMan.editFieldRecursive(next, frags[1], data, isSet)
	} else { // reached the end of frags
		switch d := data.(type) {
		case map[string]interface{}: // if incoming data is a map
			if val, exists := d["value"]; exists && len(d) == 1 { // check for {"value" : <value>} structure
				layer[field] = val // if the request has a value pair, it doesn't matter if it is POST/SET -- a singular value will be overridden either way
			} else { // data is a map of key-value pairs
				if isSet { // SET, so just override everything here
					layer[field] = data
				} else { // POST, so integrate new data with existing
					switch l := layer[field].(type) {
					case map[string]interface{}: // see if there is existing data
						for key, val := range d {
							l[key] = val // add to the existing map
						}
					default:
						layer[field] = data // it isn't already a map, so we will assume the user wants it to be and override
					}
				}
			}
		default:
			layer[field] = data // not a map -- singular values simply override regardless of POST/SET
		}
	}
	return layer
}

// === DELETE ===
// you know what delete does lol

// deletes a collection and returns the updated collection list
func (docMan *DocumentManager) deleteCollection(collection string) ([]string, error) {
	_, err := docMan.getCollection(collection)
	if err != nil {
		return nil, fmt.Errorf("local collection deletion failed: %w", err)
	}

	delete(docMan.localData, collection)

	return docMan.listCollections(), docMan.desyncCollection(collection)
}

// deletes a document and returns the updated document list
func (docMan *DocumentManager) deleteDocument(collection, document string) ([]string, error) {
	_, err := docMan.getDocument(collection, document)
	if err != nil {
		return nil, fmt.Errorf("local document deletion failed: %w", err)
	}

	delete(docMan.localData[collection], document)

	return docMan.listDocuments(collection), docMan.desyncDocument(collection, document)
}

// deletes a field and returns the updated document
func (docMan *DocumentManager) deleteField(collection, document, field string) (map[string]interface{}, error) {
	doc, err := docMan.getDocument(collection, document)
	if err != nil {
		return doc, fmt.Errorf("could not delete field locally %s: %w", field, err)
	}

	err = docMan.deleteFieldRecursive(doc, field)
	if err != nil {
		return doc, fmt.Errorf("could not delete field locally %s: %w", field, err)
	}

	return doc, docMan.syncDocument(collection, document)
}

func (docMan *DocumentManager) deleteFieldRecursive(layer map[string]interface{}, field string) error {
	frags := strings.SplitN(field, "/", 2)
	if len(frags) == 2 { // if more frags exist, recurse
		var next map[string]interface{}
		switch l := layer[frags[0]].(type) {
		case map[string]interface{}:
			next = l
		default:
			return fmt.Errorf("could not find frag(s) %s past %s: ", frags[1], frags[0])
		}

		return docMan.deleteFieldRecursive(next, frags[1])
	}

	delete(layer, field)
	return nil
}

// helper method that filters out internal fields from being manually altered
func (docMan *DocumentManager) updateMetaData(collection, document string) error {
	doc, err := docMan.getDocument(collection, document)
	if err != nil {
		return fmt.Errorf("could not update metadata: %w", err)
	}

	doc["_version"] = time.Now() // update the edit time
	doc["_id"] = document        // protects this field

	return nil
}

// === MONGO SYNC FUNCS ===

func (docMan *DocumentManager) extractDB() error {
	data, err := docMan.mongoConn.ExtractDB("dbi")
	if err != nil {
		return fmt.Errorf("could not extract dbi data from mongo: %w", err)
	}

	docMan.localData = make(map[string]map[string]map[string]interface{})

	for collection, docs := range data {
		docMan.localData[collection] = make(map[string]map[string]interface{})
		for _, body := range docs {
			if name, exists := body["_id"]; exists {
				switch n := name.(type) {
				case string:
					delete(body, "_id") // we don't want the metadata locally
					delete(body, "_version")
					docMan.localData[collection][n] = body // add to localData
				default:
					log.Errorf("document name (_id) is not a string: %v", body["_id"])
				}
			} else {
				log.Errorf("document missing name (_id) with id: %s", body["_id"]) // mongo always gives an _id
			}
		}
	}

	return nil
}

func (docMan *DocumentManager) syncDocument(collection, document string) error {
	doc, err := docMan.getDocument(collection, document)
	if err != nil {
		return fmt.Errorf("could not push document %s from collection %s: %w", document, collection, err)
	}

	doc["_id"] = document
	doc["_version"] = time.Now()
	defer func() { // clear metadata from local when we are done
		delete(doc, "_id")
		delete(doc, "_version")
	}()

	err = docMan.mongoConn.ReplaceDocument("dbi", collection, "_id", true, doc) // replaces or adds document
	if err != nil {
		return fmt.Errorf("synchronization of %s/%s failed: %w", collection, document, err)
	}

	return nil
}

func (docMan *DocumentManager) desyncDocument(collection, document string) error {
	return docMan.mongoConn.DeleteDocument("dbi", collection, "_id", document)
}

func (docMan *DocumentManager) desyncCollection(collection string) error {
	return docMan.mongoConn.DeleteCollection("dbi", collection)
}
