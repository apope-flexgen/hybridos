package main

import (
	"fmt"
	"os"
	"testing"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	mongo "github.com/flexgen-power/mongodb_client"
)

// === MONGO FUNCS ===
func TestExtractDB(t *testing.T) {
	// set up sample data
	numDocs := 100
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// check if all data was pulled
	if len(docMan.localData["dbi"]["test"]) != numDocs {
		t.Fatalf("docMan did not find all the documents: %v/%v", len(docMan.localData["dbi"]["test"]), numDocs)
	}
}

func TestSyncDocument(t *testing.T) {
	// set up sample data
	numDocs := 3
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// edit and sync back up to mongo
	docMan.localData["dbi"]["test"]["1"]["new"] = "this"

	err = docMan.syncDocument("dbi", "test", "1")
	if err != nil {
		t.Fatalf("sync failed: %v", err)
	}

	// pull down mongo data to check
	err = docMan.extractDBs()
	if err != nil {
		t.Fatalf("extract failed: %v", err)
	}

	if docMan.localData["dbi"]["test"]["1"]["new"] != "this" {
		t.Fatal("did not sync!")
	}
}

func TestDesyncDocument(t *testing.T) {
	// set up sample data
	numDocs := 3
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// desync from mongo
	err = docMan.desyncDocument("dbi", "test", "1")
	if err != nil {
		t.Fatalf("sync failed: %v", err)
	}

	// pull down mongo data to check
	err = docMan.extractDBs()
	if err != nil {
		t.Fatalf("extract failed: %v", err)
	}

	if _, exists := docMan.localData["dbi"]["test"]["1"]; exists {
		t.Fatal("did not sync!")
	}
}

// === LOCAL FUNCS ===
func TestListCollections(t *testing.T) {
	// set up sample data
	numDocs := 1
	err := populateSampleData(numDocs, "test1", "test2", "test3")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	collectionNames := docMan.listCollections()
	if len(collectionNames) != 3 {
		t.Fatalf("listed %v of 3 collections that should be present!", len(collectionNames))
	}
	t.Log(collectionNames)
	t.Log(docMan.localData["dbi"])
	for _, n := range collectionNames {
		if n != "test1" && n != "test2" && n != "test3" {
			t.Fatalf("wrong name; %s", n)
		}
	}
}

func TestListDocuments(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test1", "test2")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	documentNames := docMan.listDocuments("test1")
	if len(documentNames) != numDocs {
		t.Errorf("listed %v of %v docs that should be present!", len(documentNames), numDocs)
	}
	t.Log(documentNames)

	documentNames = docMan.listDocuments("DNE")
	if len(documentNames) != 0 {
		t.Errorf("collection does not exist, should be no documents!")
	}
}

func TestListDocumentsMap(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	docs := docMan.listDocumentsMap("test")
	if len(docs) != numDocs {
		t.Fatalf("listed %v of 3 collections that should be present!", len(docs))
	}
	t.Log(docs)
}

func TestGetCollection(t *testing.T) {
	// set up sample data
	numDocs := 5
	err := populateSampleData(numDocs, "test1", "test2", "test3")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	test1, err := docMan.getCollection("dbi", "test1")
	if err != nil {
		t.Fatal(err)
	}

	if len(test1) != numDocs {
		t.Fatalf("did not get all documents in test1! only got %v/%v", len(test1), numDocs)
	}
}

func TestGetDocument(t *testing.T) {
	// set up sample data
	numDocs := 5
	err := populateSampleData(numDocs, "test1", "test2", "test3")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	doc23, err := docMan.getDocument("dbi", "test2", "3")
	if err != nil {
		t.Fatal(err)
	}

	if doc23["numberstr"] != "3" {
		t.Fatalf("did not get the right doc!")
	}
}

func TestGetField(t *testing.T) {
	// set up sample data
	numDocs := 5
	err := populateSampleData(numDocs, "test1", "test2", "test3")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	doc31numberstr, err := docMan.getField("dbi", "test3", "1", "numberstr")
	if err != nil {
		t.Fatal(err)
	}

	if doc31numberstr != "1" {
		t.Log(doc31numberstr)
		t.Fatalf("did not get the right field!")
	}

	// test a nested retrieve
	_, err = docMan.editDocument("dbi", "test3", "newdoc", map[string]interface{}{"layer1": map[string]interface{}{"layer2": map[string]interface{}{"layer3": true, "missme": false}}}, true)
	if err != nil {
		t.Error(err)
	}

	recursive, err := docMan.getField("dbi", "test3", "newdoc", "layer1/layer2/layer3")
	if err != nil {
		t.Error(err)
	}
	if !recursive.(bool) {
		t.Fatal("did not get/set field recursively")
	}
	t.Log(recursive)
}

func TestSetCollection(t *testing.T) {
	// set up sample data
	numDocs := 5
	err := populateSampleData(numDocs, "test1", "test2")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	docArr := makePoints(100)[5:]
	docMap := make(map[string]interface{}, len(docArr))
	for _, doc := range docArr {
		docMap[doc["_id"].(string)] = doc
	}

	_, err = docMan.editCollection("dbi", "test1", docMap, true)
	if err != nil {
		t.Error(err)
	}

	docs := docMan.listDocumentsMap("test1")
	if len(docs) != len(docArr) {
		t.Error("did not add all documents!")
	}

	// check mongo
	err = docMan.extractDBs()
	if err != nil {
		t.Error(err)
	}

	docs = docMan.listDocumentsMap("test1")
	if len(docs) != len(docArr) {
		t.Errorf("wrong amount of docs in mongo: %v", len(docs))
	}
}

func TestSetDocument(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	_, err = docMan.editDocument("dbi", "test", "5", map[string]interface{}{"foo": "bar"}, true)
	if err != nil {
		t.Fatal(err)
	}

	doc5, err := docMan.getDocument("dbi", "test", "5")
	if err != nil {
		t.Fatal(err)
	}
	if len(doc5) != 1 {
		t.Errorf("wrong number of fields in document: %v", len(doc5))
	}
	if val, exists := doc5["foo"]; !exists || val != "bar" {
		t.Error("wrong value from setDocument")
	}
	t.Log(doc5)

	// extract from mongo and check again
	err = docMan.extractDBs()
	if err != nil {
		t.Error(err)
	}

	doc5, err = docMan.getDocument("dbi", "test", "5")
	if err != nil {
		t.Fatal(err)
	}
	if len(doc5) != 1 {
		t.Errorf("wrong number of fields in document: %v", len(doc5))
	}
	if val, exists := doc5["foo"]; !exists || val != "bar" {
		t.Error("wrong value from setDocument")
	}
	t.Log(doc5)
}

func TestSetField(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// value map
	_, err = docMan.editField("dbi", "test", "1", "fault", map[string]interface{}{"value": false}, true)
	if err != nil {
		t.Errorf("failed setField (value map): %v", err)
	}

	val, err := docMan.getField("dbi", "test", "1", "fault")
	if err != nil {
		t.Error("could not get doc 1/fault")
	}

	if val != false {
		t.Log(val)
		t.Error("set failed")
	}

	// new value
	_, err = docMan.editField("dbi", "test", "2", "new", "this", true)
	if err != nil {
		t.Errorf("failed setField (new value map): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "2", "new")
	if err != nil {
		t.Error("could not get doc 2/new")
	}

	if val != "this" {
		t.Error("set failed")
	}

	// new map
	_, err = docMan.editField("dbi", "test", "3", "maptest", map[string]interface{}{"something": true, "nothing": false, "a": 1, "b": map[string]interface{}{"b": 2, "c": 3}}, true)
	if err != nil {
		t.Errorf("failed setField (new map): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "3", "maptest")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}

	switch v := val.(type) {
	case map[string]interface{}:
		if v["something"] != true {
			t.Error("set failed")
		}
	default:
		t.Error("set failed - not a map")
	}

	// set map value
	_, err = docMan.editField("dbi", "test", "3", "maptest/something", false, true)
	if err != nil {
		t.Errorf("failed setField (set map value): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "3", "maptest")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}

	switch v := val.(type) {
	case map[string]interface{}:
		if v["something"] != false {
			t.Log(v)
			t.Error("set failed")
		}
	default:
		t.Log(val)
		t.Error("set failed - not a map")
	}

	// set double map value w override
	_, err = docMan.editField("dbi", "test", "3", "maptest/b", map[string]interface{}{"d": 4}, true)
	if err != nil {
		t.Errorf("failed setField (set double map value w override): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "3", "maptest/b")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}

	switch v := val.(type) {
	case map[string]interface{}:
		t.Log(v)
		if v["d"] != 4 {
			t.Error("wrong value for replacement in map")
		}
		if len(v) != 1 {
			t.Error("set did not override")
		}
	default:
		t.Log(v)
		t.Error("set failed - not a map")
	}

	// field in coll/doc that DNE
	_, err = docMan.editField("dbi", "newcoll", "newdoc", "newfield", true, true)
	if err != nil {
		t.Errorf("failed to create field/doc/coll: %v", err)
	}

	val, err = docMan.getField("dbi", "newcoll", "newdoc", "newfield")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}
	if val != true {
		t.Error("wrong value")
	}
}

func TestPostCollection(t *testing.T) {
	// set up sample data
	numDocs := 5
	err := populateSampleData(numDocs, "test1", "test2")
	if err != nil {
		t.Fatalf("could not populate coll1 data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	docArr := makePoints(10)[5:]
	docMap := make(map[string]interface{}, len(docArr))
	for _, doc := range docArr {
		docMap[doc["_id"].(string)] = doc
	}

	// test adding to existing
	docMan.editCollection("dbi", "test1", docMap, false)

	docs := docMan.listDocumentsMap("test1")
	if len(docs) != len(docArr)+numDocs {
		t.Errorf("did not add all documents: %v", len(docs))
	}

	// check mongo
	err = docMan.extractDBs()
	if err != nil {
		t.Error(err)
	}

	docs = docMan.listDocumentsMap("test1")
	if len(docs) != len(docArr)+numDocs {
		t.Errorf("did not add all documents to mongo: %v", len(docs))
	}

	// test adding new
	_, err = docMan.editCollection("dbi", "test3", docMap, false)
	if err != nil {
		t.Error(err)
	}

	docs = docMan.listDocumentsMap("test3")
	if len(docs) != len(docArr) {
		t.Error("did not add all documents!")
	}

	// check mongo
	err = docMan.extractDBs()
	if err != nil {
		t.Error(err)
	}

	docs = docMan.listDocumentsMap("test3")
	if len(docs) != len(docArr) {
		t.Errorf("did not add all documents to mongo: %v", len(docs))
	}
}

func TestPostDocument(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	_, err = docMan.editDocument("dbi", "test", "5", map[string]interface{}{"foo": "bar"}, false)
	if err != nil {
		t.Fatal(err)
	}

	doc5, err := docMan.getDocument("dbi", "test", "5")
	if err != nil {
		t.Fatal(err)
	}

	if len(doc5) != 3 {
		t.Errorf("wrong number of fields in document: %v", len(doc5))
	}
	if val, exists := doc5["foo"]; !exists || val != "bar" {
		t.Error("wrong value from post document")
	}
	t.Log(doc5)

	// extract from mongo and check again
	err = docMan.extractDBs()
	if err != nil {
		t.Error(err)
	}

	doc5, err = docMan.getDocument("dbi", "test", "5")
	if err != nil {
		t.Fatal(err)
	}
	if len(doc5) != 3 {
		t.Errorf("wrong number of fields in document: %v", len(doc5))
	}
	if val, exists := doc5["foo"]; !exists || val != "bar" {
		t.Error("wrong value from post document")
	}
	t.Log(doc5)
}

func TestPostField(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// value map
	_, err = docMan.editField("dbi", "test", "1", "fault", map[string]interface{}{"value": false}, false)
	if err != nil {
		t.Errorf("failed setField (value map): %v", err)
	}

	val, err := docMan.getField("dbi", "test", "1", "fault")
	if err != nil {
		t.Error("could not get doc 1/fault")
	}

	if val != false {
		t.Error("set failed")
	}

	// new value
	_, err = docMan.editField("dbi", "test", "2", "new", "this", false)
	if err != nil {
		t.Errorf("failed setField (new value map): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "2", "new")
	if err != nil {
		t.Error("could not get doc 2/new")
	}

	if val != "this" {
		t.Error("set failed")
	}

	// new map
	_, err = docMan.editField("dbi", "test", "3", "maptest", map[string]interface{}{"something": true, "nothing": false, "a": 1, "b": map[string]interface{}{"b": 2, "c": 3}}, false)
	if err != nil {
		t.Errorf("failed setField (new map): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "3", "maptest")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}
	t.Log(val)

	switch v := val.(type) {
	case map[string]interface{}:
		if v["something"] != true {
			t.Error("set failed")
		}
	default:
		t.Error("set failed - not a map")
	}

	// set map value
	_, err = docMan.editField("dbi", "test", "3", "maptest/something", false, false)
	if err != nil {
		t.Errorf("failed setField (set map value): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "3", "maptest")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}

	switch v := val.(type) {
	case map[string]interface{}:
		if v["something"] != false {
			t.Log(v)
			t.Error("set failed")
		}
	default:
		t.Log(val)
		t.Error("set failed - not a map")
	}

	// set double map value w override
	_, err = docMan.editField("dbi", "test", "3", "maptest/b", map[string]interface{}{"d": 4}, false)
	if err != nil {
		t.Errorf("failed setField (set double map value w override): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "3", "maptest/b")
	if err != nil {
		t.Error("could not get doc 3/maptest")
	}

	switch v := val.(type) {
	case map[string]interface{}:
		t.Log(v)
		if v["d"] != 4 {
			t.Error("wrong value for replacement in map")
		}
		if len(v) != 3 {
			t.Error("set did override")
		}
	default:
		t.Log(v)
		t.Error("set failed - not a map")
	}
}

func TestDeleteCollection(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test1", "test2")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	_, err = docMan.deleteCollection("dbi", "test1")
	if err != nil {
		t.Error(err)
	}

	// check local
	colls := docMan.listCollections()
	if len(colls) != 1 {
		t.Fatalf("too many colls!")
	}

	t.Log(colls)
	for _, coll := range colls {
		if coll == "test1" {
			t.Fatalf("did not delete!")
		}
	}

	// check remote
	err = docMan.extractDBs()
	if err != nil {
		t.Error(err)
	}

	colls = docMan.listCollections()
	if len(colls) != 1 {
		t.Fatalf("too many colls from mongo!")
	}

	t.Log(colls)
	for _, coll := range colls {
		if coll == "test1" {
			t.Fatalf("did not delete from mongo!")
		}
	}
}

func TestDeleteDocument(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	_, err = docMan.deleteDocument("dbi", "test", "7")
	if err != nil {
		t.Errorf("could not delete doc: %v", err)
	}

	doc, _ := docMan.getDocument("dbi", "test", "7")
	if doc != nil {
		t.Log(doc)
		t.Fatalf("document still exists!")
	}

	err = docMan.extractDBs()
	if err != nil {
		t.Fatal(err)
	}

	doc, _ = docMan.getDocument("dbi", "test", "7")
	if doc != nil {
		t.Log(doc)
		t.Fatalf("document still exists in mongo!")
	}
}

func TestDeleteField(t *testing.T) {
	// set up sample data
	numDocs := 10
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// regular delete
	_, err = docMan.deleteField("dbi", "test", "7", "fault")
	if err != nil {
		t.Errorf("could not delete doc: %v", err)
	}

	doc, _ := docMan.getDocument("dbi", "test", "7")
	if doc["fault"] != nil {
		t.Log(doc)
		t.Fatalf("field still exists!")
	}

	err = docMan.extractDBs()
	if err != nil {
		t.Fatal(err)
	}

	doc, _ = docMan.getDocument("dbi", "test", "7")
	if doc["fault"] != nil {
		t.Log(doc)
		t.Fatalf("field still exists in mongo!")
	}

	// recursive delete
	_, err = docMan.editDocument("dbi", "test3", "newdoc", map[string]interface{}{"layer1": map[string]interface{}{"layer2": map[string]interface{}{"layer3": true}}}, true)
	if err != nil {
		t.Error(err)
	}

	_, err = docMan.deleteField("dbi", "test3", "newdoc", "layer1/layer2/layer3")
	if err != nil {
		t.Error(err)
	}

	recursive, err := docMan.getField("dbi", "test3", "newdoc", "layer1/layer2")
	if err != nil {
		t.Error(err)
	}
	if len(recursive.(map[string]interface{})) != 0 {
		t.Log(recursive)
		t.Fatal("did not delete field recursively")
	}

	// DNE delete
	_, err = docMan.deleteField("dbi", "test3", "numberstr", "nein/nein/nein")
	if err != nil {
		t.Log(err)
	}
}

// === helper funcs ===

// USE FOR TESTING PURPOSES ONLY
func populateSampleData(numDocs int, collections ...string) error {
	err := log.InitConfig("dbi_test").Init("dbi_test")
	if err != nil {
		fmt.Printf("Error initializing logger for dbi testing: %s.\n", err.Error())
		os.Exit(-1)
	}

	connector := mongo.NewConnector("localhost:27017", time.Second/10, time.Second*10)

	err = connector.Connect()
	if err != nil {
		return err
	}

	err = connector.Client.Database("dbi").Drop(connector.Context)
	if err != nil {
		return err
	}

	// execute
	for _, coll := range collections {
		err := connector.Write("dbi", coll, makePoints(numDocs))
		if err != nil {
			return err
		}
	}

	return nil
}

func makePoints(size int) []map[string]interface{} {
	data := make([]map[string]interface{}, size)
	for i := 0; i < size; i++ {
		point := map[string]interface{}{
			"fault":     true,
			"numberstr": fmt.Sprint(i),
			"_id":       fmt.Sprint(i),
			"_version":  time.Now(),
		}
		data[i] = point
	}

	return data
}

// POST-ALPHA FEATURE TESTS

// feature - array interfacing
func TestGetArray(t *testing.T) {
	// set up sample data
	numDocs := 1
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// write arr
	arr := []interface{}{1, 2.2, "three", false}
	_, err = docMan.editField("dbi", "test", "1", "arr", arr, true)
	if err != nil {
		t.Errorf("failed setField (initial array): %v", err)
	}

	// check get
	for i, correct := range arr {
		val, err := docMan.getField("dbi", "test", "1", fmt.Sprintf("arr/%v", i))
		if err != nil {
			t.Errorf("%v", err)
		}

		if correct != val {
			t.Errorf("%v should be %v, is %v", i, correct, val)
		}
	}

	// check invalid get
	val, err := docMan.getField("dbi", "test", "1", "arr/4")
	if val != nil {
		t.Error("should be invalid")
	}
	if err == nil {
		t.Error("error should not be nil")
	}
}

func TestDeleteArray(t *testing.T) {
	// set up sample data
	numDocs := 1
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// write arr
	arr := []interface{}{1, 2.2, "three", false}
	_, err = docMan.editField("dbi", "test", "1", "arr", arr, true)
	if err != nil {
		t.Errorf("failed setField (initial array): %v", err)
	}

	val, err := docMan.getField("dbi", "test", "1", "arr")
	if err != nil {
		t.Error(err)
	}
	t.Log(val)

	_, err = docMan.deleteField("dbi", "test", "1", "arr/2")
	if err != nil {
		t.Error(err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr")
	if err != nil {
		t.Error(err)
	}

	switch slice := val.(type) {
	case []any:
		t.Log(slice)
		if len(slice) != len(arr)-1 {
			t.Error("wrong size!")
		}
		if slice[2] == "three" {
			t.Error("did not delete properly")
		}
	default:
		t.Error("was not array!")
	}

	// deleting through arrays

	// write arr
	arr = []interface{}{1, 2.2, "three", map[string]interface{}{"number": 2, "arr": []interface{}{0, 1, 2, 3}}}
	_, err = docMan.editField("dbi", "test", "1", "arr", arr, true)
	if err != nil {
		t.Errorf("failed setField (initial array): %v", err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr/3/arr")
	if err != nil {
		t.Error(err)
	}
	t.Log(val)

	_, err = docMan.deleteField("dbi", "test", "1", "arr/3/arr/1")
	if err != nil {
		t.Error(err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr/3/arr")
	if err != nil {
		t.Error(err)
	}

	switch slice := val.(type) {
	case []any:
		if len(slice) != len(arr)-1 {
			t.Error("wrong size!")
		}
		if slice[1] == 1 {
			t.Error("did not delete properly")
		}
	default:
		t.Error("was not array!")
	}

	// erroneous requests
	_, err = docMan.deleteField("dbi", "test", "1", "arr/3/arr/-1")
	if err == nil {
		t.Error("should be invalid")
	}

	_, err = docMan.deleteField("dbi", "test", "1", "arr/3/arr/10")
	if err == nil {
		t.Error("should be invalid")
	}
}

func TestSetArray(t *testing.T) {
	// set up sample data
	numDocs := 1
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// write arr
	arr := []interface{}{1, 2.2, "three", map[string]interface{}{"number": 2, "arr": []interface{}{0, 1, 2, 3}}}
	_, err = docMan.editField("dbi", "test", "1", "arr", arr, true)
	if err != nil {
		t.Errorf("failed setField (initial array): %v", err)
	}

	// edit a nested array
	_, err = docMan.editField("dbi", "test", "1", "arr/3/arr", []interface{}{true, false}, true)
	if err != nil {
		t.Error(err)
	}

	val, err := docMan.getField("dbi", "test", "1", "arr/3/arr")
	if err != nil {
		t.Error(err)
	}
	switch s := val.(type) {
	case []interface{}:
		if !(s[0].(bool) && !s[1].(bool)) || len(s) != 2 {
			t.Error("set did not take")
		}
	default:
		t.Error("is not a slice!")
	}

	// create a new array with set
	_, err = docMan.editField("dbi", "test", "1", "arr/7/new", []interface{}{true, false}, true)
	if err != nil {
		t.Error(err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr/7/new")
	if err != nil {
		t.Error(err)
	}
	switch s := val.(type) {
	case []interface{}:
		if !(s[0].(bool) && !s[1].(bool)) || len(s) != 2 {
			t.Error("set did not take")
		}
	default:
		t.Error("is not a slice!")
	}

	// make dbi create array by passing a number field to nonexistent field
	_, err = docMan.editField("dbi", "test", "1", "arr/8/2", []interface{}{true, false}, true)
	if err != nil {
		t.Error(err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr/8/2")
	if err != nil {
		t.Error(err)
	}
	switch s := val.(type) {
	case []interface{}:
		if !(s[0].(bool) && !s[1].(bool)) || len(s) != 2 {
			t.Error("set did not take")
		}
	default:
		t.Error("is not a slice!")
	}
}

func TestPostArray(t *testing.T) {
	// set up sample data
	numDocs := 1
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// write arr
	arr := []interface{}{1, 2.2, "three", map[string]interface{}{"number": 2, "arr": []interface{}{0, 1, 2, 3}}}
	_, err = docMan.editField("dbi", "test", "1", "arr", arr, true)
	if err != nil {
		t.Errorf("failed setField (initial array): %v", err)
	}

	// edit a nested array
	_, err = docMan.editField("dbi", "test", "1", "arr", []interface{}{true, false}, false)
	if err != nil {
		t.Error(err)
	}

	val, err := docMan.getField("dbi", "test", "1", "arr")
	if err != nil {
		t.Error(err)
	}
	switch s := val.(type) {
	case []interface{}:
		if len(s) != len(arr)+2 || !(s[4].(bool) && !s[5].(bool)) {
			t.Error("post did not take")
			t.Log(val)
		}
	default:
		t.Error("is not a slice!")
	}

	// create a new array with post
	_, err = docMan.editField("dbi", "test", "1", "arr/7/new", []interface{}{true, false}, false)
	if err != nil {
		t.Error(err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr/7/new")
	if err != nil {
		t.Error(err)
	}
	switch s := val.(type) {
	case []interface{}:
		if !(s[0].(bool) && !s[1].(bool)) || len(s) != 2 {
			t.Error("post did not take")
		}
	default:
		t.Error("is not a slice!")
	}

	// make dbi create array by passing a number field to nonexistent field
	_, err = docMan.editField("dbi", "test", "1", "arr/8/2", []interface{}{true, false}, false)
	if err != nil {
		t.Error(err)
	}

	val, err = docMan.getField("dbi", "test", "1", "arr/8/2")
	if err != nil {
		t.Error(err)
	}
	switch s := val.(type) {
	case []interface{}:
		if !(s[0].(bool) && !s[1].(bool)) || len(s) != 2 {
			t.Error("post did not take")
		}
	default:
		t.Error("is not a slice!")
	}
}

func TestPlayground(t *testing.T) {
	// set up sample data
	numDocs := 1
	err := populateSampleData(numDocs, "test")
	if err != nil {
		t.Fatalf("could not populate test data: %v", err)
	}

	// create docMan
	docMan := DocumentManager{}
	err = docMan.init("localhost", "27017")
	if err != nil {
		t.Fatalf("could not start document manager: %v", err)
	}

	// write arr
	arr := []interface{}{1, 2.2, "three", map[string]interface{}{"number": 2, "arr": []interface{}{0, 1, 2, 3}}}

	_, err = docMan.editField("dbi", "test", "0", "arr", arr, true)
	if err != nil {
		t.Errorf("failed setField (initial array): %v", err)
	}

	val, err := docMan.getDocument("dbi", "test", "0")
	if err != nil {
		t.Error(err)
	}
	t.Log(val)
}
