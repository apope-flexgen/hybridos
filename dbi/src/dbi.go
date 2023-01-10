package main

import (
	"context"
	"encoding/json"
	"fims"
	"flag"
	"io/ioutil"
	"log"
	"os/user"
	"reflect"
	"strings"
	"time"

	"github.com/pkg/profile"
	"go.mongodb.org/mongo-driver/bson"
	mongo "go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"go.mongodb.org/mongo-driver/mongo/readpref"
)

// Structure for a Mongo Database
type DatabaseComp struct {
	context      context.Context
	databaseName string
	dbClient     *mongo.Client
	database     *mongo.Database
}

//Struct for ip address and port
type Address struct {
	Ipaddress string `json:"mongo_ip"`
	Port      string `json:"mongo_port"`
}

// Variables that are used to hold the local data from mongo
var dbcomp DatabaseComp
var audit DatabaseComp
var collectionMap map[string]interface{}
var m Address

// var pubTicker *time.Ticker
var config string
var prof string

func main() {
	log.SetPrefix("DBI: ")
	flag.StringVar(&prof, "prof", "", "This is for profiling only contact Claire to get permission")
	flag.StringVar(&m.Ipaddress, "i", "localhost", "ip address for mongo")
	flag.StringVar(&m.Port, "p", "27017", "port for mongo")
	flag.StringVar(&config, "c", "", "config for ip and port for mongo")
	flag.Parse()

	if prof == "cpu" { //used for profiling
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	}

	if config != "" { //If user want to use config file for ip and port
		configJSON, err := ioutil.ReadFile(config)
		if err != nil {
			log.Fatalf("could not read config file: %v", err)
		}
		err = json.Unmarshal(configJSON, &m) // simple unmarshal is sufficient as data structure matches json
		if err != nil {
			log.Fatal("failed to unmarshal config file: ", err)
		}
	}

	var p_fims fims.Fims // The fims object
	collectionMap = make(map[string]interface{})

	fimsReceive := make(chan fims.FimsMsg)
	// Connect to the dbi keyword
	p_fims, err := fims.Connect("dbi")
	if err != nil {
		log.Printf("Connection failed: %v", err)
		p_fims.Close()
		return
	}

	// Subscribe to dbi uris that is given in the fims_echo
	subErr := p_fims.Subscribe("/dbi")
	if subErr != nil {
		log.Println("Subscription to /dbi failed.")
		p_fims.Close()
		return
	}
	go p_fims.ReceiveChannel(fimsReceive)
	dbcomp.databaseName = "dbi"
	audit.databaseName = "audit"

	// Connecting to the mongodb database
	cxt := context.Background()
	acxt := context.Background()
	dbcomp.context = cxt
	audit.context = acxt

	client, err := mongo.Connect(cxt, options.Client().ApplyURI("mongodb://"+m.Ipaddress+":"+m.Port))  //connect to any ip with any port
	aclient, err := mongo.Connect(cxt, options.Client().ApplyURI("mongodb://"+m.Ipaddress+":"+m.Port)) //connect to any ip with any port
	dbcomp.dbClient = client
	audit.dbClient = aclient
	if err != nil {
		log.Printf("Could not connect to mongo database: %v", err)
	}
	defer client.Disconnect(cxt)
	defer aclient.Disconnect(acxt)

	err = client.Ping(cxt, readpref.Primary())
	err = aclient.Ping(acxt, readpref.Primary())
	if err != nil {
		log.Printf("Could not receive ping from mongo database: %v", err)
	}
	dbcomp.database = client.Database(dbcomp.databaseName)
	audit.database = aclient.Database(audit.databaseName)

	// Waits for a Fims message to be sent
	var result interface{}
	var result2 map[string]interface{}
	dbcomp.getfromMongo()
	for { //i := 0; i < 10000; i++ { // for profiling
		select {
		case msg := <-fimsReceive:
			copsURI := strings.Replace(msg.Uri, "/dbi/", "/cops/", 1)
			copsFrag := msg.Frags
			copsFrag[0] = "cops"
			if msg.Method != "get" { //Sending message to cops too
				_, err := p_fims.Send(fims.FimsMsg{
					Method: msg.Method,
					Uri:    copsURI,
					Body:   msg.Body,
					Nfrags: msg.Nfrags,
					Frags:  copsFrag,
				})
				if err != nil {
					log.Printf("Could not send cops message. %v", err)
					continue
				}
			}

			// Checks if Frags is too short or user wants to see all documents or local map
			if len(msg.Frags) < 2 && msg.Frags[len(msg.Frags)-1] != "show_collections" { //must be at least two fragments, special cases
				log.Println("The uri length must be greater than or equal to 3 fragments, special cases: /dbi/show_collections, /dbi/\"Collections\"/show_documents, /dbi/\"Collections\"/show_map")
			} else if msg.Frags[len(msg.Frags)-1] == "show_collections" && len(msg.Frags) == 2 {
				result = showCollections()
			} else if msg.Frags[len(msg.Frags)-1] == "show_documents" && len(msg.Frags) == 3 {
				result = showDocuments(msg)
			} else if msg.Frags[len(msg.Frags)-1] == "show_map" && len(msg.Frags) == 3 {
				if collectionMap == nil || collectionMap[msg.Frags[1]] == nil {
					log.Println("There was no database or collection found")
					continue
				}
				result = collectionMap[msg.Frags[1]]
			} else {
				// What kind of method is the Fims message using
				if msg.Method == "set" || msg.Method == "post" {
					if len(msg.Frags) == 2 {
						dbcomp.twoFragSet(msg)
					} else {
						dbcomp.addRecord(msg)
						result = collectionMap[msg.Frags[1]].(map[string]interface{}) // Result of adding or updating to show back to terminal
					}
				}
				if msg.Method == "get" { // Will get the record and return to the terminal
					result2 = getRecord(msg)
					if len(msg.Frags) >= 4 {
						result = result2[msg.Frags[len(msg.Frags)-1]]
					} else {
						result = result2
					}
				}
				if msg.Method == "del" { // Will delete entry specified and store the new local map
					result = dbcomp.deleteRecord(msg)
				}
			}
			// Will send back map or get to the ReplyTo:terminal
			if msg.Replyto != "" {
				_, err := p_fims.Send(fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   result,
				})
				if err != nil {
					log.Printf("Could not send message. %v", err)
					continue
				}
			}
		}
	}
}

// Function that will extract information that is already in mongo into the local map
func (dbc DatabaseComp) getfromMongo() {
	name, err := dbc.database.ListCollectionNames(dbc.context, bson.D{}) // Grab all the collection names that exist in mongo
	if err != nil {
		log.Printf("Failed to find collection names witin mongo: %v", err)
		return
	}
	for _, col := range name { // Loop through all the collections and grab all the data there
		var episodes []bson.M
		collect := dbc.database.Collection(col)
		cursor, err := collect.Find(dbc.context, bson.M{})
		if err != nil {
			log.Printf("Failed to receive data witin mongo: %v", err)
			return
		}

		if err := cursor.All(dbc.context, &episodes); err != nil {
			log.Printf("Failed to extract data from mongo: %v", err)
			return
		}
		fillMap := make(map[string]interface{})
		for _, data := range episodes { // Loop through all the Documents and grab the Map data that is at every document
			newMap := map[string]interface{}(data)
			fillMap[newMap["_doc"].(string)] = newMap //Grab the Document name and assign the local Map
		}
		collectionMap[col] = fillMap
	}
	if len(name) == 0 { // If nothing is in collection then add an empty map
		collectionMap = make(map[string]interface{})
	}
}

// Meat of the program: recursively run through maps to create and find the data that is wanted
func recursiveTravel(msg fims.FimsMsg, prevMap map[string]interface{}, position int) map[string]interface{} {
	// fmt.Println(position == len(msg.Frags)-1)
	Frag := msg.Frags[position]       //Frag is the next map that is within the upper map
	if position == len(msg.Frags)-1 { //Destination of the URI that user specified
		if msg.Method == "del" {
			delete(prevMap, Frag) //Deletes entry fromm local map
			return prevMap
		} else if msg.Method == "set" || msg.Method == "post" {
			switch v := msg.Body.(type) {
			case float64, string, bool, []interface{}:
				prevMap[Frag] = v
			case map[string]interface{}:
				if len(v) == 1 && v["value"] != nil { //Checks if there is one entry in map body and if entry is of key value then make it assigned to key
					prevMap[Frag] = v["value"]
				} else { //Will add entire map body to key
					prevMap[Frag] = v
				}
			}
			return prevMap
		} else if msg.Method == "get" {
			return prevMap
		}

	}

	var fill bson.M
	var newMap map[string]interface{}

	//Changes type from bson.M to map[s]i{} to make traversal easy
	//since grabbing data from mongo is always a bson.M
	if reflect.TypeOf(prevMap[Frag]) == reflect.TypeOf(fill) {
		fill = prevMap[Frag].(bson.M)
		newMap = map[string]interface{}(fill)
		prevMap[Frag] = newMap
	}
	if msg.Method == "get" { //if method get then it will set prevMap to entire map rather than the next layer down
		if prevMap[Frag] != nil {
			prevMap = recursiveTravel(msg, prevMap[Frag].(map[string]interface{}), position+1)
		} else {
			log.Println("There is no database")
			return make(map[string]interface{})
		}

	} else {
		if prevMap[Frag] == nil { //If Key:value map pair doesnt exist then create and set
			prevMap[Frag] = make(map[string]interface{})
			prevMap[Frag] = recursiveTravel(msg, prevMap[Frag].(map[string]interface{}), position+1)
		} else { //If key:value map pair does exist
			prevMap[Frag] = recursiveTravel(msg, prevMap[Frag].(map[string]interface{}), position+1)
		}
	}
	return prevMap

}

//Adds record to mongo based off the URI in message
func (dbc DatabaseComp) addRecord(msg fims.FimsMsg) {
	if collectionMap[msg.Frags[1]] == nil { //If collection dont exist
		collectionMap[msg.Frags[1]] = make(map[string]interface{})
	}
	collection := collectionMap[msg.Frags[1]].(map[string]interface{}) //Grab local map collection
	collect := dbc.database.Collection(msg.Frags[1])                   //Grab Mongo collection
	logcol := audit.database.Collection("log")
	var episodes []bson.M
	var newID interface{}
	var aMap map[string]interface{}
	filler := make(map[string]interface{})
	aMap = make(map[string]interface{})

	if collection == nil || collection[msg.Frags[2]] == nil && msg.Frags[1] != "audit" {

		//Filler data to make a record and get an ID to store
		//Used mainly when there is large data and program cant find it in database
		filler["_doc"] = msg.Frags[2]

		fillerdata := bson.M(filler)
		_, err := collect.InsertOne(dbc.context, fillerdata) //Filler data added
		if err != nil {
			log.Printf("Failed to insert data into mongo: %v", err)
			return
		}

		cursor, err := collect.Find(dbc.context, fillerdata) //Find document with filler data
		if err != nil {
			log.Printf("Failed to find data that was just entered into mongo: %v", err)
			return
		}

		if err := cursor.All(dbc.context, &episodes); err != nil { //Get full document with ID
			log.Printf("Failed to extract data from mongo: %v", err)
			return
		}
		newID = episodes[0]["_id"] //Store ID for later use
	} else if collection[msg.Frags[2]] != nil && msg.Frags[1] != "audit" {
		newID = collection[msg.Frags[2]].(map[string]interface{})["_id"]
		aMap = collection[msg.Frags[2]].(map[string]interface{}) //If Document already exists
	}
	//create map based on URI
	if len(msg.Frags) == 3 {
		if reflect.TypeOf(msg.Body) != reflect.TypeOf(aMap) {
			log.Println("Needs to be a map[string]interface{}")
			return
		}
		if msg.Method == "post" {
			for k, v := range msg.Body.(map[string]interface{}) {
				aMap[k] = v
			}
		} else if msg.Method == "set" {
			aMap = msg.Body.(map[string]interface{})
		}
	} else if len(msg.Frags) >= 4 {
		aMap = recursiveTravel(msg, aMap, 3)
	}

	aMap["_version"] = time.Now() //Time stamp of the creaation/u
	aMap["_doc"] = msg.Frags[2]   //Add what document is for reinserting data from mongo

	data := bson.M(aMap)
	if msg.Frags[1] != "audit" {
		aMap["_id"] = newID                                                         //Add id to document map
		_, err := collect.ReplaceOne(dbc.context, bson.M{"_id": aMap["_id"]}, data) //Replace to get rid of filler data, sets whatever the map is from the recursion
		if err != nil {
			log.Printf("Failed to replace data witin mongo: %v", err)
			return
		}
	} else {
		if msg.Username == "" {
			username, err := user.Current()
			if err != nil {
				log.Println("Could not find username for this machine")
			}
			aMap["username"] = username.Username
		} else {
			aMap["username"] = msg.Username
		}
		data := bson.M(aMap)
		_, err := logcol.InsertOne(dbc.context, data)
		if err != nil {
			log.Printf("Failed to insert data witin mongo: %v", err)
			return
		}
	}
	collection[msg.Frags[2]] = aMap //Updates local Map
}

//Get record from local Map
func getRecord(msg fims.FimsMsg) map[string]interface{} {
	if collectionMap == nil || collectionMap[msg.Frags[1]] == nil { //If there is no collection or database
		log.Println("There was no database or collection found")
		return make(map[string]interface{})
	}
	collection := collectionMap[msg.Frags[1]].(map[string]interface{})
	var aMap map[string]interface{}
	if len(msg.Frags) == 2 {
		return collection
	}
	if len(msg.Frags) == 3 {
		if collection[msg.Frags[len(msg.Frags)-1]] == nil { //If Document doesnt exist
			log.Println("Document does not exist")
			return make(map[string]interface{})
		}
		aMap = collection[msg.Frags[len(msg.Frags)-1]].(map[string]interface{}) //If the target is a whole document
	} else {
		if collection[msg.Frags[2]] == nil { //If Document doesnt exist
			log.Println("Document does not exist")
			return make(map[string]interface{})
		}
		aMap = recursiveTravel(msg, collection[msg.Frags[2]].(map[string]interface{}), 3) //if target is an element of a document
	}
	return aMap
}

//Delete a record from local and mongo
func (dbc DatabaseComp) deleteRecord(msg fims.FimsMsg) map[string]interface{} {
	if collectionMap == nil || collectionMap[msg.Frags[1]] == nil {
		log.Println("There was no database or collection found")
		return make(map[string]interface{})
	}
	collection := collectionMap[msg.Frags[1]].(map[string]interface{})
	var aMap map[string]interface{}
	collect := dbc.database.Collection(msg.Frags[1])
	if len(msg.Frags) == 3 { //If target is an entire document
		if collection[msg.Frags[len(msg.Frags)-1]] == nil { //If Document doesnt exist
			log.Println("Document does not exist")
			return make(map[string]interface{})
		}
		id := collection[msg.Frags[2]].(map[string]interface{})["_id"]
		delete(collection, msg.Frags[len(msg.Frags)-1])
		_, err := collect.DeleteOne(dbc.context, bson.M{"_id": id}) //Will delete entire document from mongo
		if err != nil {
			log.Printf("Failed to delete data witin mongo: %v", err)
			return make(map[string]interface{})
		}
	} else {
		if collection[msg.Frags[2]] == nil { //If Document doesnt exist
			log.Println("Document does not exist")
			return make(map[string]interface{})
		}
		aMap = recursiveTravel(msg, collection[msg.Frags[2]].(map[string]interface{}), 3) //Update local map
		data := bson.M(aMap)
		_, err := collect.ReplaceOne(dbc.context, bson.M{"_id": aMap["_id"]}, data) //replace updated map in mongo
		if err != nil {
			log.Printf("Failed to replace data witin mongo: %v", err)
			return make(map[string]interface{})
		}
	}
	return collection
}

//Print out a slice of all existing documents
func showDocuments(msg fims.FimsMsg) []string {
	docs := make([]string, 0)
	if collectionMap == nil || collectionMap[msg.Frags[1]] == nil {
		log.Println("There was no database or collection found")
		return docs
	}
	collection := collectionMap[msg.Frags[1]].(map[string]interface{})
	for key := range collection {
		docs = append(docs, key)
	}
	return docs
}

//Print out a slice of all existing collections
func showCollections() []string {
	docs := make([]string, 0)
	if collectionMap == nil {
		log.Println("There was no database or collection found")
		return docs
	}
	for key := range collectionMap {
		docs = append(docs, key)
	}
	return docs
}

//Only for copying data from a file
func (dbc DatabaseComp) twoFragSet(msg fims.FimsMsg) {
	collect := dbc.database.Collection(msg.Frags[1])
	for _, v := range msg.Body.(map[string]interface{}) {
		data := bson.M(v.(map[string]interface{}))
		_, err := collect.InsertOne(dbc.context, data) //Filler data added
		if err != nil {
			log.Printf("Failed to insert data into mongo: %v", err)
			return
		}
	}
}
