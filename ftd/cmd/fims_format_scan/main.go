package main

import (
	"fims"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// The mode in which we present the gathered format data
type presentationMode uint

const (
	subsetForestMode  presentationMode = iota // present formats as a forest organized by subset relation
	conflictTableMode                         // present formats as a table indicating type conflicts
	rawFormatListMode                         // list formats of all uris
)

func main() {
	// define flags
	var modeString string
	flag.StringVar(&modeString, "mode", "raw_format_list_mode", "Mode determines how the format data is presented. The options are:\n"+
		"- subset_forest: presents formats as a forest of formats organized such that each format is a subset of its children in the forest.\n"+
		"- conflict_table: presents formats as a table indicating type conflicts between formats.\n"+
		"- raw_format_list_mode: lists formats of all uris.\n")
	flag.Parse()

	modeStringToMode := map[string]presentationMode{
		"subset_forest":        subsetForestMode,
		"conflict_table":       conflictTableMode,
		"raw_format_list_mode": rawFormatListMode,
	}
	mode, ok := modeStringToMode[modeString]
	if !ok {
		fmt.Println("Invalid presentation mode. Valid modes are:")
		for m := range modeStringToMode {
			fmt.Println(m)
		}
		os.Exit(-1)
	}

	// initialize logger
	err := (&log.Config{
		Moniker:           "fims format scan",
		ToConsole:         true,
		ToFile:            false,
		SeverityThreshold: log.TRACE,
	}).Init("fims format scan")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	// setup uri format data collection
	uriToFormatString := make(map[string]string)
	formatStringToUris := make(map[string]map[string]struct{})
	formatStringToFormat := make(map[string]uriMsgFormat)

	// setup fims
	fimsChan, err := listenToFimsRootPubs()
	if err != nil {
		log.Fatalf("Failed to listen for fims pubs: %v", err)
	}

	// main program loop
	logTicker := time.NewTicker(time.Second)
	log.Infof("Beginning message processing loop")
	for {
		select {
		case msg := <-fimsChan: // process incoming fims messages
			// ignore non-post non-pub messages
			msgMethod := strings.ToLower(msg.Method)
			if msgMethod != "pub" && msgMethod != "post" {
				continue
			}

			format := formatOf(msg)
			formatString := format.string() // use strings so that we know they will work correctly as map keys
			// check format against previously seen format for the uri
			if prevFormatString, exists := uriToFormatString[msg.Uri]; exists && formatString != prevFormatString {
				log.Errorf("Message format mismatch between %s and itself", msg.Uri)
				// we still take note of this message format rather than skipping it since in some modes the format itself is
				// what's important, not so much which format to associate with the uri
			} else if !exists {
				uriToFormatString[msg.Uri] = formatString
			}
			// if no previous uri had this format, add this format to our known formats
			if _, ok := formatStringToUris[formatString]; !ok {
				formatStringToUris[formatString] = make(map[string]struct{})
				formatStringToFormat[formatString] = format
			}
			// this uri belongs to its format now
			formatStringToUris[formatString][msg.Uri] = struct{}{}

		case <-logTicker.C: // periodically log what we have so far
			switch mode {
			case subsetForestMode:
				// create the forest hierarchy of formats organized by subset
				forest := createUriFormatSubsetForest(formatStringToUris, formatStringToFormat)
				// log the formats and associated uris
				log.Infof("Uris organized by message format:\n%s", forest.string())

			case conflictTableMode:
				// create the conflict table
				ctable := createUriFormatConflictTable(formatStringToUris, formatStringToFormat)
				// log the conflict table
				log.Infof("Uri message format conflicts:\n%s", ctable.string())

			case rawFormatListMode:
				for uri, fmtString := range uriToFormatString {
					rawFormatListString := ""
					rawFormat := formatStringToFormat[fmtString]
					rawFormatFields := rawFormat.getSortedFieldNames()
					for _, field := range rawFormatFields {
						fieldType := rawFormat.fieldTypes[field]
						// convert to Influx's type names
						switch fieldType.String() {
						case "float64":
							rawFormatListString += fmt.Sprintf("\"%s\": \"float\", ", field)
						case "bool":
							rawFormatListString += fmt.Sprintf("\"%s\": \"boolean\", ", field)
						case "string":
							rawFormatListString += fmt.Sprintf("\"%s\": \"string\", ", field)
						default:
							rawFormatListString += fmt.Sprintf("\"%s\": \"%s\", ", field, fieldType)
						}
					}
					log.Infof("Format for uri %s is: %s", uri, rawFormatListString)
				}
			}
		}
	}
}

// Returns a channel of all FIMS publishes
func listenToFimsRootPubs() (<-chan fims.FimsMsg, error) {
	fimsConn, err := fims.Connect("fims_scanner")
	if err != nil {
		return nil, fmt.Errorf("failed to connect to FIMS server: %w", err)
	}
	err = fimsConn.Subscribe("/")
	if err != nil {
		return nil, fmt.Errorf("failed to subscribe to root: %w", err)
	}
	fimsChan := make(chan fims.FimsMsg)
	go func() {
		defer close(fimsChan)
		fimsConn.ReceiveChannel(fimsChan)
	}()
	return fimsChan, nil
}
