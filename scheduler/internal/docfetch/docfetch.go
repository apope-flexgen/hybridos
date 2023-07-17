package docfetch

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Queries the URI /dbi/<processName> with a GET until it receives a response.
// If DBI had no documents to give and returned an error message, an empty map
// is returned. If DBI gives back documents, they are organized into a map with
// their contents left as bytes.
func FetchDocumentsFromDbi(processName string) (documents map[string]json.RawMessage, err error) {
	fimsConn, err := fims.Connect(processName)
	if err != nil {
		return nil, fmt.Errorf("failed to connect to FIMS")
	}
	defer fimsConn.Close()

	// use a unique URI so DBI's reply does not get mixed with other FIMS requests to the process
	configUri := fmt.Sprintf("/cfg_%s", processName)
	err = fimsConn.Subscribe(configUri)
	if err != nil {
		return nil, fmt.Errorf("failed to subscribe to %s: %w", configUri, err)
	}

	// set up channel to receive data from receive thread
	dataReceive := make(chan map[string]json.RawMessage)
	defer close(dataReceive)

	// start thread to receive configuration data over FIMS
	go func() {
		for {
			documents, err := receiveDocuments(&fimsConn)
			if err != nil {
				log.Errorf("Error receiving documents from DBI: %v.", err)
				continue
			}
			dataReceive <- documents
			return
		}
	}()

	// start ticker to signal when DBI has taken too long to reply
	waitForReply := time.NewTicker(10 * time.Second)
	defer waitForReply.Stop()

	for {
		if !fimsConn.Connected() {
			return nil, errors.New("FIMS lost connection")
		}

		// request documents from DBI
		err := fimsConn.SendGet(fmt.Sprintf("/dbi/%s", processName), configUri)
		if err != nil {
			return nil, fmt.Errorf("failed to send GET to DBI: %w", err)
		}

		// wait for DBI to reply, and re-request if it takes too long
		select {
		case data := <-dataReceive:
			return data, nil
		case <-waitForReply.C:
			log.Errorf("Request for DBI documents timed out. Resending GET to DBI.")
		}
	}
}

func receiveDocuments(fimsConn *fims.Fims) (documents map[string]json.RawMessage, err error) {
	msg, err := fimsConn.Receive()
	if err != nil {
		return nil, fmt.Errorf("error receiving FIMS message: %w", err)
	}
	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal input: %w", err)
	}
	if err = json.Unmarshal(jsonBytes, &documents); err != nil {
		// unmarshal error would occur if the given collection is not in DBI so DBI returned an error string.
		// in that case, there are no documents so return empty map
		return make(map[string]json.RawMessage), nil
	}
	return documents, nil
}
