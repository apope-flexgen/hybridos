package ftd

import (
	"context"
	"fims"
	"fmt"
	"strings"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Takes in new FIMS messages via a FIMS connection, filters/processes them,
// then puts only messages that are meant for encoding and transfer to the
// PowerCloud server on the out channel for further processing.
type FimsListener struct {
	uriCfgs        []UriConfig
	fimsConnection *fims.Fims
	fimsIn         chan fims.FimsMsg
	Out            chan *fims.FimsMsg
}

// Uri for cops summary which we always listen to for controller status
const copsSummaryUri = "/cops/summary"

// Creates a FIMS listener with a newly-allocated output channel and input channel awaiting fims data.
func NewFimsListener(cfgs []UriConfig) *FimsListener {
	return &FimsListener{
		uriCfgs: cfgs,
		fimsIn:  make(chan fims.FimsMsg),
		Out:     make(chan *fims.FimsMsg),
	}
}

// Connects to FIMS, subscribes to all relevant URIs, and launches a worker to handle the
// FIMS messages received under the subscriptions.
func (l *FimsListener) Start(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	// connect to FIMS
	err := l.connectToFims()
	if err != nil {
		return fmt.Errorf("unable to connect to FIMS server: %w", err)
	}
	// subscribe to URIs
	uris := buildUriList(l.uriCfgs)
	log.Infof("Subscribing to uris: %#v", uris)
	err = l.fimsConnection.Subscribe(uris...)
	if err != nil {
		return fmt.Errorf("unable to subscribe to FIMS: %w", err)
	}

	// start receiving messages
	go func() {
		l.fimsConnection.ReceiveChannel(l.fimsIn)
		// the above call will only exit upon a fims disconnect
		close(l.fimsIn)
	}()

	// start listening to those messages
	group.Go(func() error {
		defer l.fimsConnection.Close()
		return l.listenUntil(groupContext.Done())
	})
	return nil
}

// Listens for new FIMS messages and passes them to the collator if they
// are of method pub or post.
//
// COPS messages are identified and processed separately, never encoded.
func (l *FimsListener) listenUntil(done <-chan struct{}) error {
	defer close(l.Out) // ensure following stages are always notified that there will be no more data if this stage terminates
	for {
		select {
		case <-done:
			log.Infof("Received shutdown signal.")
			goto termination
		case msg, ok := <-l.fimsIn:
			// handle potential loss of connection by checking connected status
			if !ok || !l.fimsConnection.Connected() {
				log.Errorf("Disconnected from FIMS server.")
				goto termination
			}

			// handle cops messages
			// check messages from cops for controller status
			if msg.Uri == copsSummaryUri {
				updateStatusFromCopsSummary(&msg)
			}
			// only allow pub, post, and set type methods for now
			// Archive pubs because that's the method of most of the data we want,
			// Archive posts because that's the method of events data,
			// Archive sets to see what commands are being sent
			msgMethod := strings.ToLower(msg.Method)
			if msgMethod != "pub" && msgMethod != "post" && msgMethod != "set" {
				continue
			}

			l.Out <- &msg
		}
	}
termination:
	log.Infof("Listener terminating. Will archive data received so far and then close.")
	return nil
}

// Connects to the FIMS server.
func (l *FimsListener) connectToFims() error {
	fimsObj, err := fims.Connect("ftd")
	if err != nil {
		return fmt.Errorf("failed to connect to FIMS server: %w", err)
	}
	l.fimsConnection = &fimsObj

	return nil
}

// Extracts controller status from COPS summary message and updates internal flag.
func updateStatusFromCopsSummary(msg *fims.FimsMsg) {
	log.Debugf("COPS message received - %#v", *msg)
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		log.Errorf("Expected COPS message to have map[string]interface{} but got %T.", msg.Body)
		return
	}
	statusString, exist := bodyMap["status"].(string)
	if !exist {
		log.MsgWarn("COPS message does not have status string. Defaulting to primary controller mode.")
		return
	}
	log.Debugf("Received controller status of %s from COPS.", statusString)
	// set the controller flag to given status
	if strings.ToLower(statusString) == "primary" {
		setControllerStatePrimary(true)
	} else {
		setControllerStatePrimary(false)
	}
}

// Builds a list of URIs to which FTD should subscribe.
// Includes any URIs from configuration and hard-coded
// URIs such as "/cops" that are required for operation.
func buildUriList(configUris []UriConfig) (uriList []string) {
	// keep track of which uris we have added with a set
	urisAddedToList := make(map[string]struct{})
	// /cops/summary is necessary to integrate with the COPS watchdog module
	uriList = append(uriList, copsSummaryUri)
	urisAddedToList[copsSummaryUri] = struct{}{}
	// add URIs from configuration
	for _, fimsUri := range configUris {
		tmpUri := cleanUri(fimsUri.BaseUri)
		if _, added := urisAddedToList[tmpUri]; added {
			continue
		}
		uriList = append(uriList, tmpUri)
		urisAddedToList[tmpUri] = struct{}{}
	}
	return uriList
}
