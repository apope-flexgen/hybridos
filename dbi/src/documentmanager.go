package main

import (
	"fmt"
	"strconv"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	mongo "github.com/flexgen-power/mongodb_client"
)

// DocumentManager manages a local copy of DBI data synced with MongoDB using a MongoConnector, and responds to DBI requests
type DocumentManager struct {
	localData map[string]map[string]map[string]map[string]interface{} // structure: map[<db_name>]map[<coll_name>]map[<doc_name>]<body>
	mongoConn mongo.MongoConnector
}

// connects to a local mongodb instance and downloads contents to localData
func (docMan *DocumentManager) init(ip, port string) error {
	docMan.mongoConn = mongo.NewConnector(ip+":"+port, time.Second/10, time.Second*10) // TODO : change configurables
	err := docMan.mongoConn.Connect()
	if err != nil {
		return err
	}

	return docMan.extractDBs()
}

// === REQUEST FUNCS ===
// NOTE: these are the only functions that the main DBI process should interface with
// NOTE: we expect that the frags given DO NOT INCLUDE "dbi"
// ex: ["configs" "ess" "active_power_setpoint"] is valid

func (docMan *DocumentManager) GET(frags []string) (interface{}, error) {
	switch len(frags) {
	case 1: // err, only a DB name
		return nil, fmt.Errorf("not enough frags given for GET command")
	case 2: // collection
		switch frags[1] {
		case "show_collections": // special command
			res := docMan.listCollections()
			if len(res) == 0 {
				return res, fmt.Errorf("no collections found")
			}
			return res, nil
		default: // regular GET
			return docMan.getCollection(frags[0], frags[1])
		}
	case 3: // document
		switch frags[2] {
		case "show_documents": // special command
			res := docMan.listDocuments(frags[1])
			if len(res) == 0 {
				return res, fmt.Errorf("no documents found under %s", frags[1])
			}
			return res, nil
		case "show_map": // special command
			res := docMan.listDocumentsMap(frags[1])
			if len(res) == 0 {
				return res, fmt.Errorf("no documents found under %s", frags[1])
			}
			return res, nil
		default: // regular GET
			return docMan.getDocument(frags[0], frags[1], frags[2])
		}
	default: // fields
		return docMan.getField(frags[0], frags[1], frags[2], frags[3])
	}
}

func (docMan *DocumentManager) POST(frags []string, data interface{}) (interface{}, error) {
	switch len(frags) {
	case 1: // err, only DB name
		return nil, fmt.Errorf("not enough frags given for POST command")
	case 2: // collection
		return docMan.editCollection(frags[0], frags[1], data, false) // "two frag set/post"
	case 3: // document
		return docMan.editDocument(frags[0], frags[1], frags[2], data, false)
	default: // fields
		return docMan.editField(frags[0], frags[1], frags[2], frags[3], data, false)
	}
}

func (docMan *DocumentManager) SET(frags []string, data interface{}) (interface{}, error) {
	switch len(frags) {
	case 1: // err, only DB name
		return nil, fmt.Errorf("not enough frags given for SET command")
	case 2: // collection
		switch frags[1] {
		case "resync":
			err := docMan.extractDBs()
			if err != nil {
				return "FAIL", err
			}
			return "SUCCESS", err
		default:
			return docMan.editCollection(frags[0], frags[1], data, true) // "two frag set/post"
		}
	case 3: // document
		return docMan.editDocument(frags[0], frags[1], frags[2], data, true)
	default: // fields
		return docMan.editField(frags[0], frags[1], frags[2], frags[3], data, true)
	}
}

func (docMan *DocumentManager) DELETE(frags []string) (interface{}, error) {
	switch len(frags) {
	case 1: // err, only DB name
		return nil, fmt.Errorf("not enough frags given for DELETE command")
	case 2: // collection
		return docMan.deleteCollection(frags[0], frags[1])
	case 3: // document
		return docMan.deleteDocument(frags[0], frags[1], frags[2])
	default: // fields
		return docMan.deleteField(frags[0], frags[1], frags[2], frags[3])
	}
}

// lists all collection names
// ONLY FOR DBI DATABASE
func (docMan *DocumentManager) listCollections() []string {
	if len(docMan.localData["dbi"]) == 0 {
		return []string{}
	}

	collections := make([]string, len(docMan.localData["dbi"]))
	i := 0
	for name := range docMan.localData["dbi"] {
		collections[i] = name
		i++
	}
	return collections
}

// lists all document names in a collection
// ONLY FOR DBI DATABASE
func (docMan *DocumentManager) listDocuments(collection string) []string {
	coll, err := docMan.getCollection("dbi", collection)
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
// ONLY FOR DBI DATABASE
func (docMan *DocumentManager) listDocumentsMap(collection string) map[string]map[string]interface{} {
	coll, err := docMan.getCollection("dbi", collection)
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
func (docMan *DocumentManager) getCollection(db, collection string) (map[string]map[string]interface{}, error) {
	if coll, exists := docMan.localData[db][collection]; exists {
		return coll, nil
	}

	return nil, fmt.Errorf("collection %s/%s not found", db, collection)
}

// gets a document from a collection, if it exists
func (docMan *DocumentManager) getDocument(db, collection, document string) (map[string]interface{}, error) {
	coll, err := docMan.getCollection(db, collection)
	if err != nil {
		return nil, fmt.Errorf("document %s not found in collection %s/%s: %w", document, db, collection, err)
	}

	if doc, exists := coll[document]; exists {
		return doc, nil
	}

	return nil, fmt.Errorf("document %s not found in collection %s", document, collection)
}

// retrieves the value of a field in a document in a collection
func (docMan *DocumentManager) getField(db, collection, document, field string) (interface{}, error) {
	doc, err := docMan.getDocument(db, collection, document)
	if err != nil {
		return nil, fmt.Errorf("field %s not found in document %s in collection %s/%s: %w", field, document, db, collection, err)
	}

	return docMan.getFieldRecursive(doc, field)
}

// helper function
func (docMan *DocumentManager) getFieldRecursive(layer interface{}, field string) (interface{}, error) {
	frags := strings.SplitN(field, "/", 2)
	var next_untyped interface{}

	switch layer_typed := layer.(type) { // layer can be a map[string]interface{} or []interface{}
	case map[string]interface{}:
		// frag is a string -> use frags[0]
		if _, has := layer_typed[frags[0]]; !has { // verify exists
			return nil, fmt.Errorf("field %s does not exist", frags[0])
		}

		next_untyped = layer_typed[frags[0]]
	case []interface{}:
		// frag is a slice -> use frags[0] as an integer
		index, err := strconv.Atoi(frags[0]) // frag is an integer
		if err != nil {
			return nil, fmt.Errorf("could not index array with index %s: %w", frags[0], err)
		}
		if index > len(layer_typed)-1 || index < 0 { // check bounds
			return nil, fmt.Errorf("could not index array with index %s: out of bounds for array of length %v", frags[0], len(layer_typed))
		}

		next_untyped = layer_typed[index]
	default:
		if len(frags) == 2 { // there are still frags, so this is a type error
			return nil, fmt.Errorf("could not find frag(s) %s (is %T): ", frags[0], layer_typed)
		} else { // final frag
			next_untyped = layer_typed
		}
	}

	if len(frags) == 2 { // if more frags exist, recurse
		return docMan.getFieldRecursive(next_untyped, frags[1])
	}

	return next_untyped, nil // use frag and return
}

// === POST/SET (edit) ===
// post requests override and/or add field(s) without altering the rest of the document/layer
// set requests override and/or add fields while removing the rest of the document/layer

// posts an entire collection of documents and syncs to mongo. returns updated collection
func (docMan *DocumentManager) editCollection(db, collection string, documents interface{}, isSet bool) (map[string]map[string]interface{}, error) {
	if isSet {
		docMan.localData[db][collection] = make(map[string]map[string]interface{}) // clear existing data
		docMan.mongoConn.DeleteCollection(db, collection)
	}

	switch docs := documents.(type) {
	case map[string]interface{}:
		for name, doc := range docs {
			_, err := docMan.editDocument(db, collection, name, doc, false) // false - don't want to clear every time we add a new doc
			if err != nil {
				log.Errorf("failed to add document %s as part of collection edit: %v", name, err)
			}
		}
		return docMan.localData[db][collection], nil
	default:
		return nil, fmt.Errorf("request to edit collection did not have body map[string]interface{}")
	}
}

// posts the body of a document in a collection to body, syncs up to mongo. returns updated document
func (docMan *DocumentManager) editDocument(db, collection, document string, body interface{}, isSet bool) (map[string]interface{}, error) {
	if _, exists := docMan.localData[db][collection]; !exists {
		docMan.localData[db][collection] = make(map[string]map[string]interface{})
	}

	switch bod := body.(type) {
	case map[string]interface{}:
		if _, exists := docMan.localData[db][collection][document]; !exists || isSet {
			docMan.localData[db][collection][document] = bod
		} else {
			for key, value := range bod {
				docMan.localData[db][collection][document][key] = value
			}
		}

		docMan.updateMetaData(db, collection, document) // update metadata

		return docMan.localData[db][collection][document], docMan.syncDocument(db, collection, document)
	default:
		return nil, fmt.Errorf("request to edit document did not have body map[string]interface{}")
	}
}

// adds the key-value pairs in fields to the document in collection, syncing to mongo. will override field if already exists. returns updated document
func (docMan *DocumentManager) editField(db, collection, document, field string, data interface{}, isSet bool) (map[string]interface{}, error) {
	if field == "_id" || field == "_version" {
		return nil, fmt.Errorf("attempted to edit protected field")
	}

	if _, exists := docMan.localData[db][collection]; !exists {
		docMan.localData[db][collection] = make(map[string]map[string]interface{})
	}

	if _, exists := docMan.localData[db][collection][document]; !exists {
		docMan.localData[db][collection][document] = make(map[string]interface{})
	}
	doc := docMan.localData[db][collection][document]

	_, err := docMan.editFieldRecursive(doc, field, data, isSet) // perform edit
	if err != nil {
		return doc, fmt.Errorf("editing field %s failed: %w", field, err)
	}

	docMan.updateMetaData(db, collection, document) // update metadata

	return doc, docMan.syncDocument(db, collection, document)
}

func (docMan *DocumentManager) editFieldRecursive(layer interface{}, field string, insert interface{}, isSet bool) (interface{}, error) {
	frags := strings.SplitN(field, "/", 2)

	if layer == nil {
		index, err := strconv.Atoi(frags[0])
		if err != nil && index > 0 {
			layer = make([]interface{}, index)
		} else {
			layer = make(map[string]interface{})
		}
	}

	// check for { "value" : interface{} } idiomatic expression
	switch insert_typed := insert.(type) {
	case map[string]interface{}: // if editable layer is a map
		if val, exists := insert_typed["value"]; exists && len(insert_typed) == 1 { // check for {"value" : <value>} structure
			insert = val // if the request has a value pair, it doesn't matter if it is POST/SET -- a singular or different value will be overridden either way
		}
	}

	switch layer_typed := layer.(type) {
	case map[string]interface{}:
		// frag is a string -> use frags[0]

		if field != "" { // if more frags exist, recurse
			next := ""
			if len(frags) != 1 {
				next = frags[1]
			}

			layer_updated, err := docMan.editFieldRecursive(layer_typed[frags[0]], next, insert, isSet)
			layer_typed[frags[0]] = layer_updated // update layer
			return layer_typed, err
		}

		// reached the end of frags
		switch insert_typed := insert.(type) {
		case map[string]interface{}: // if editable layer is a map
			if isSet { // SET, so just override everything here
				return insert_typed, nil
			} else { // POST, so integrate new data with existing
				for key, val := range insert_typed {
					layer_typed[key] = val // add to the existing map
				}
				return layer_typed, nil
			}
		default:
			return insert, nil // conflicting datatypes, so we will just override
		}

	case []interface{}:
		// frag is a slice -> use frags[0] as an integer

		if field != "" { // if more frags exist, recurse
			next := ""
			if len(frags) != 1 {
				next = frags[1]
			}

			// frag validation
			index, err := strconv.Atoi(frags[0]) // frag is an integer
			if err != nil {
				return layer, fmt.Errorf("could not index array with index %s: %w", frags[0], err)
			}
			if index < 0 { // check bounds
				return layer, fmt.Errorf("could not index array with index %s: out of bounds for array of length %v", frags[0], len(layer_typed))
			}
			if index >= len(layer_typed) { // out of scope, add to array at index
				fill := make([]interface{}, 1+index-len(layer_typed)) // create fill buffer
				layer_typed = append(layer_typed, fill...)
			}

			layer_updated, err := docMan.editFieldRecursive(layer_typed[index], next, insert, isSet)
			layer_typed[index] = layer_updated // update layer
			return layer_typed, err
		}

		// reached the end of frags
		switch insert_typed := insert.(type) {
		case []interface{}: // if incoming data is an array
			if isSet { // SET, so just override everything here
				return insert_typed, nil
			} else { // POST, so integrate new data with existing
				return append(layer_typed, insert_typed...), nil
			}
		default:
			return insert_typed, nil // not an array -- singular or different values simply override regardless of POST/SET
		}

	default:
		return insert, nil
	}
}

// === DELETE ===
// you know what delete does lol

// deletes a collection and returns the updated collection list
func (docMan *DocumentManager) deleteCollection(db, collection string) ([]string, error) {
	_, err := docMan.getCollection(db, collection)
	if err != nil {
		return nil, fmt.Errorf("local collection deletion failed: %w", err)
	}

	delete(docMan.localData[db], collection)

	return docMan.listCollections(), docMan.desyncCollection(db, collection)
}

// deletes a document and returns the updated document list
func (docMan *DocumentManager) deleteDocument(db, collection, document string) ([]string, error) {
	_, err := docMan.getDocument(db, collection, document)
	if err != nil {
		return nil, fmt.Errorf("local document deletion failed: %w", err)
	}

	delete(docMan.localData[db][collection], document)

	return docMan.listDocuments(collection), docMan.desyncDocument(db, collection, document)
}

// deletes a field and returns the updated document
func (docMan *DocumentManager) deleteField(db, collection, document, field string) (map[string]interface{}, error) {
	doc, err := docMan.getDocument(db, collection, document)
	if err != nil {
		return doc, fmt.Errorf("could not delete field locally %s: %w", field, err)
	}

	_, err = docMan.deleteFieldRecursive(doc, field)
	if err != nil {
		return doc, fmt.Errorf("could not delete field locally %s: %w", field, err)
	}

	return doc, docMan.syncDocument(db, collection, document)
}

func (docMan *DocumentManager) deleteFieldRecursive(layer interface{}, field string) (interface{}, error) {
	frags := strings.SplitN(field, "/", 2)

	switch layer_typed := layer.(type) { // layer can be a map[string]interface{} or []interface{}
	case map[string]interface{}:
		// frag is a string -> use frags[0]

		if _, has := layer_typed[frags[0]]; !has { // verify exists
			return layer, fmt.Errorf("field %s does not exist", frags[0])
		}

		if len(frags) == 2 { // if more frags exist, recurse
			layer_updated, err := docMan.deleteFieldRecursive(layer_typed[frags[0]], frags[1])
			layer_typed[frags[0]] = layer_updated // update layer
			return layer_typed, err               // pass back up
		} else { // final frag, delete
			delete(layer_typed, frags[0])
			return layer_typed, nil // return update layer
		}
	case []interface{}:
		// frag is a slice -> use frags[0] as an integer
		index, err := strconv.Atoi(frags[0]) // frag is an integer
		if err != nil {
			return layer, fmt.Errorf("could not index array with index %s: %w", frags[0], err)
		}
		if index > len(layer_typed)-1 || index < 0 { // check bounds
			return layer, fmt.Errorf("could not index array with index %s: out of bounds for array of length %v", frags[0], len(layer_typed))
		}

		if len(frags) == 2 { // if more frags exist, recurse
			layer_updated, err := docMan.deleteFieldRecursive(layer_typed[index], frags[1])
			layer_typed[index] = layer_updated
			return layer_typed, err
		} else { // final frag, delete
			return append(layer_typed[:index], layer_typed[index+1:]...), nil // return update layer
		}
	default:
		return layer, fmt.Errorf("could not find frag(s) %s (layer is %T): ", frags[0], layer_typed)
	}
}

// helper method that filters out internal fields from being manually altered
func (docMan *DocumentManager) updateMetaData(db, collection, document string) error {
	doc, err := docMan.getDocument(db, collection, document)
	if err != nil {
		return fmt.Errorf("could not update metadata: %w", err)
	}

	doc["_version"] = time.Now() // update the edit time
	doc["_id"] = document        // protects this field

	return nil
}

// === MONGO SYNC FUNCS ===

func (docMan *DocumentManager) extractDBs() error {
	docMan.localData = make(map[string]map[string]map[string]map[string]interface{})

	err := docMan.extractDB("dbi")
	if err != nil {
		return err
	}

	return docMan.extractDB("audit")
}

func (docMan *DocumentManager) extractDB(db string) error {
	data, err := docMan.mongoConn.ExtractDB(db)
	if err != nil {
		return fmt.Errorf("could not extract %s data from mongo: %w", db, err)
	}

	// create DB structure locally
	docMan.localData[db] = make(map[string]map[string]map[string]interface{})

	for collection, docs := range data {
		// create collection structure locally
		docMan.localData[db][collection] = make(map[string]map[string]interface{})
		for _, body := range docs {
			if name, exists := body["_id"]; exists {
				switch n := name.(type) {
				case string:
					delete(body, "_id") // we don't want the metadata locally
					delete(body, "_version")
					docMan.localData[db][collection][n] = body // add to localData
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

func (docMan *DocumentManager) syncDocument(db, collection, document string) error {
	doc, err := docMan.getDocument(db, collection, document)
	if err != nil {
		return fmt.Errorf("could not push document %s from collection %s and database %s: %w", document, collection, db, err)
	}

	doc["_id"] = document
	doc["_version"] = time.Now()
	defer func() { // clear metadata from local when we are done
		delete(doc, "_id")
		delete(doc, "_version")
	}()

	err = docMan.mongoConn.ReplaceDocument(db, collection, "_id", true, doc) // replaces or adds document
	if err != nil {
		return fmt.Errorf("synchronization of %s/%s/%s failed: %w", db, collection, document, err)
	}

	return nil
}

func (docMan *DocumentManager) desyncDocument(db, collection, document string) error {
	return docMan.mongoConn.DeleteDocument(db, collection, "_id", document)
}

func (docMan *DocumentManager) desyncCollection(db, collection string) error {
	return docMan.mongoConn.DeleteCollection(db, collection)
}
