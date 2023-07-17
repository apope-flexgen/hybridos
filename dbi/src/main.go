package main

import (
	"encoding/json"
	"fims"
	"flag"
	"fmt"
	"os"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/pkg/profile"
)

type Config struct {
	IP   string `json:"mongo_ip"`
	Port string `json:"mongo_port"`
}

var (
	config Config
	api    DocumentManager
)

func main() {
	err := log.InitConfig("dbi").Init("dbi") // set up logger
	if err != nil {
		fmt.Printf("Error initializing logger for dbi: %s.\n", err.Error())
		os.Exit(-1)
	}

	// flags
	var configPath string
	flag.StringVar(&configPath, "c", "", "config file for ip and port for mongo")
	var prof string
	flag.StringVar(&prof, "prof", "", "profiling mode")
	flag.StringVar(&config.IP, "i", "localhost", "mongo instance IP address")
	flag.StringVar(&config.Port, "p", "27017", "mongo instance port")
	flag.Parse()

	// start profiling if configured
	if prof == "cpu" {
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	}

	// unmarshal config
	if configPath != "" {
		configJSON, err := os.ReadFile(configPath)
		if err != nil {
			log.Errorf("could not read config file: %v", err)
			log.Infof("starting DBI without a config...")
		} else {
			err = json.Unmarshal(configJSON, &config) // simple unmarshal is sufficient as data structure matches json
			if err != nil {
				log.Fatalf("failed to unmarshal config file: %v", err)
			}
		}

		log.Infof("mongo IP and port: %v", config)
	}

	// connect to mongo and download state
	api = DocumentManager{}
	err = api.init(config.IP, config.Port)
	if err != nil {
		log.Fatalf("could not initialize document management: %v", err)
	}

	// start fims connection
	fimsReceive := make(chan fims.FimsMsg)
	fimsConn, err := fims.Connect("dbi") // connect as dbi
	if err != nil {
		fimsConn.Close()
		log.Fatalf("connection failed: %v", err)
	}

	err = fimsConn.Subscribe("/dbi") // subscribe to the dbi URI
	if err != nil {
		fimsConn.Close()
		log.Fatalf("subscription to /dbi failed: %v", err)
	}

	go fimsConn.ReceiveChannel(fimsReceive) // start receiving fims messages

	for msg := range fimsReceive { // main loop execution
		// forward to COPS
		copsFrag := msg.Frags
		copsFrag[0] = "cops"
		if msg.Method != "get" { //Sending message to cops too
			_, err := fimsConn.Send(fims.FimsMsg{
				Method: msg.Method,
				Uri:    strings.Replace(msg.Uri, "dbi", "cops", 1),
				Body:   msg.Body,
				Nfrags: msg.Nfrags,
				Frags:  copsFrag,
			})
			if err != nil {
				log.Errorf("could forward %s message to cops: %v", msg.Uri, err)
			}
		}

		frags := strings.SplitN(msg.Uri, "/", 5)[1:]

		// SPECIAL CASE: frags[1] (collection) == "audit" (#BAD-186)
		// expected format: /dbi/audit/audit_log_TIMESTAMP
		//					^	^     ^
		//			   ignore	db 	  document
		if frags[1] == "audit" {
			frags[0] = "audit" // db
			frags[1] = "log"   // collection
			// frags[2] is still the document
		}

		var reply interface{}
		switch msg.Method { // parse request type
		case "get":
			result, err := api.GET(frags)
			if err != nil {
				log.Errorf("GET on %s encountered an error: %v", msg.Uri, err)
				reply = fmt.Sprintf("GET on %s encountered an error: %v", msg.Uri, err)
			} else {
				reply = result
			}
		case "set":
			result, err := api.SET(frags, msg.Body)
			if err != nil {
				log.Errorf("SET on %s encountered an error: %v", msg.Uri, err)
				reply = fmt.Sprintf("SET on %s encountered an error: %v", msg.Uri, err)
			} else {
				reply = result
			}
		case "post":
			result, err := api.POST(frags, msg.Body)
			if err != nil {
				log.Errorf("POST on %s encountered an error: %v", msg.Uri, err)
				reply = fmt.Sprintf("POST on %s encountered an error: %v", msg.Uri, err)
			} else {
				reply = result
			}
		case "del", "delete":
			result, err := api.DELETE(frags)
			if err != nil {
				log.Errorf("DELETE on %s encountered an error: %v", msg.Uri, err)
				reply = fmt.Sprintf("DELETE on %s encountered an error: %v", msg.Uri, err)
			} else {
				reply = result
			}
		default:
			log.Errorf("unrecognized or invalid method for dbi on %s: %s", msg.Uri, msg.Method)
			reply = fmt.Sprintf("unrecognized or invalid method for dbi on %s: %s", msg.Uri, msg.Method)
		}

		if msg.Replyto != "" { // reply if requested
			_, err := fimsConn.Send(fims.FimsMsg{
				Method: "set",
				Uri:    msg.Replyto,
				Body:   reply, // TODO -- all request types need to give a response back
			})
			if err != nil {
				log.Errorf("could not send reply from %s to %s: %v", msg.Uri, msg.Replyto, err)
			}
		}
	}
}
