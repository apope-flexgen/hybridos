package main

import (
	"context"
	"fims"
	"fmt"
	"strings"
	"sync/atomic"

	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Takes in new FIMS messages via a FIMS connection, filters/processes them,
// then puts only messages that are meant for encoding and transfer to the
// PowerCloud server on the out channel for further processing.
type fimsListener struct {
	fimsConnection *fims.Fims
	fimsIn         chan fims.FimsMsg
	out            chan *fims.FimsMsg
}

// Creates a FIMS listener with a newly-allocated output channel and input channel awaiting fims data.
func newFimsListener() *fimsListener {
	return &fimsListener{
		fimsIn: make(chan fims.FimsMsg),
		out:    make(chan *fims.FimsMsg),
	}
}

// Connects to FIMS, subscribes to all relevant URIs, and launches a worker to handle the
// FIMS messages received under the subscriptions.
func (l *fimsListener) run(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	// connect to FIMS
	err := l.connectToFims()
	if err != nil {
		return fmt.Errorf("unable to connect to FIMS server: %w", err)
	}
	// subscribe to URIs
	uris := buildUriList(config.Uris)
	log.Infof("Subscribing to uris: %#v", uris)
	err = l.fimsConnection.Subscribe(uris...)
	if err != nil {
		return fmt.Errorf("unable to subscribe to FIMS: %w", err)
	}
	log.Infof("archive created every %d seconds", config.ArchivePeriod)

	// start receiving messages
	go l.fimsConnection.ReceiveChannel(l.fimsIn)

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
func (l *fimsListener) listenUntil(done <-chan struct{}) error {
	// core process loop
	for {
		select {
		case <-done:
			return nil
		case msg := <-l.fimsIn:
			if msg.Uri == "/cops" {
				processCopsMsg(&msg)
				continue
			}
			// only allow pub and post type methods for now
			// Archive pubs because that's the method of most of the data we want,
			// Archive posts because that's the method of events data
			msgMethod := strings.ToLower(msg.Method)
			if msgMethod != "pub" && msgMethod != "post" {
				log.Debugf("Ignoring unknown publish type - %s for %s", msg.Method, msg.Uri)
				continue
			}
			l.out <- &msg
		}
	}
}

// Connects to the FIMS server.
func (l *fimsListener) connectToFims() error {
	fimsObj, err := fims.Connect("ftd")
	if err != nil {
		return fmt.Errorf("failed to connect to FIMS server: %w", err)
	}
	l.fimsConnection = &fimsObj

	return nil
}

// Extracts controller status from COPS message and updates internal flag.
func processCopsMsg(msg *fims.FimsMsg) {
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
		atomic.StoreUint32(&controllerState, 0)
	} else {
		atomic.StoreUint32(&controllerState, 1)
	}
}

// Builds a list of URIs to which FTD should subscribe.
// Includes any URIs from configuration and hard-coded
// URIs such as "/cops" that are required for operation.
func buildUriList(configUris []UriConfig) (uriList []string) {
	// keep track of which uris we have added with a set
	urisAddedToList := make(map[string]struct{})
	// /cops is necessary to integrate with the COPS watchdog module
	uriList = append(uriList, "/cops")
	urisAddedToList["/cops"] = struct{}{}
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
