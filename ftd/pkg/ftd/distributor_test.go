package ftd

import (
	"context"
	"fims"
	"fmt"
	"os/signal"
	"reflect"
	"syscall"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"
)

// Test messages are being distributed as expected when using some outs
func TestMsgDistributed(t *testing.T) {
	mainContext, shutdown := signal.NotifyContext(context.Background(), syscall.SIGTERM)
	defer shutdown()
	group, groupContext := errgroup.WithContext(mainContext)

	// start a timeout routine which triggers if this test takes too long
	go func() {
		select {
		case <-groupContext.Done():
		case <-time.After(time.Second):
			t.Errorf("Test timed out")
			shutdown()
		}
	}()

	in := make(chan *fims.FimsMsg)
	d := NewDistributor(
		[][]UriConfig{
			nil,
			{UriConfig{BaseUri: "/ftd", Method: []string{"pub"}}, UriConfig{BaseUri: "/orange", Method: []string{"set"}}},
			{UriConfig{BaseUri: "/unused", Method: []string{"pub"}}}, // output 2 should not receive any messages because there are no message on its uri
			nil,
			{UriConfig{BaseUri: "/ftd", Method: []string{"pub"}}, UriConfig{BaseUri: "/orange", Method: []string{"set"}}},
		},
		in,
		5,
	)

	// test only using some of the outs
	d.NewOut(1)
	d.NewOut(2)
	d.NewOut(4)

	d.Start(group, groupContext)

	if d.Outs[0] != nil || d.Outs[3] != nil {
		t.Fatalf("Output channels expected to be nil were non-nil: %v", d.Outs)
	}
	if d.Outs[1] == nil || d.Outs[2] == nil || d.Outs[4] == nil {
		t.Fatalf("Output channels expected to be non-nil were nil: %v", d.Outs)
	}

	msgs := generateMessages(100, "/ftd/test", "pub", func(i int) interface{} { return map[string]interface{}{"value": i} })
	msgs = append(msgs, generateMessages(100, "/orange/test", "set", func(i int) interface{} { return i })...)
	numMsgs := len(msgs)
	msgSentCounter := 0
	msgReceivedCounters := []int{0, 0, 0, 0, 0}

	// start feeding inputs via a goroutine
	go func() {
		for msgSentCounter < numMsgs {
			select {
			case <-groupContext.Done():
				return
			case in <- msgs[msgSentCounter]:
				msgSentCounter++
			}
		}
	}()

	// check output messages against input messages
	checkReceivedMessage := func(msg *fims.FimsMsg, outIndex int) error {
		if msgReceivedCounters[outIndex] == numMsgs {
			return fmt.Errorf("recieved more than expected number of messages on out")
		}
		if !reflect.DeepEqual(msg, msgs[msgReceivedCounters[outIndex]]) {
			return fmt.Errorf("message received on out was not the expected message")
		}
		msgReceivedCounters[outIndex]++
		return nil
	}

	done := false
	for !done {
		select {
		case <-groupContext.Done():
			done = true
		case msg := <-d.Outs[1]:
			err := checkReceivedMessage(msg, 1)
			if err != nil {
				t.Fatalf("Failed message check on out 1: %v", err)
			}
		case msg := <-d.Outs[2]:
			t.Fatalf("Got unexpected message on out 2: %#v", msg)
		case msg := <-d.Outs[4]:
			err := checkReceivedMessage(msg, 4)
			if err != nil {
				t.Fatalf("Failed message check on out 4: %v", err)
			}
		}

		done = done ||
			(msgReceivedCounters[1] == numMsgs && msgReceivedCounters[4] == numMsgs)
	}
}

// Test determining necessary outs based on uri and config
func TestGetNecessaryOuts(t *testing.T) {
	d := NewDistributor(
		[][]UriConfig{
			{UriConfig{BaseUri: "/apple", Method: []string{"pub"}}},
			nil,
			{
				UriConfig{BaseUri: "/banana", Method: []string{"pub"}},
				UriConfig{BaseUri: "/cherry", Method: []string{"set"}},
			},
			nil,
			{
				UriConfig{BaseUri: "/apple", Method: []string{"pub"}},
				UriConfig{BaseUri: "/cherry", Method: []string{"pub"}},
				UriConfig{BaseUri: "/cherry", Method: []string{"set"}},
			},
		},
		nil,
		5,
	)

	// map messages to expected mask
	testCases := map[*fims.FimsMsg]outListMask{
		{Uri: "/apple", Method: "pub"}:           0b10001,
		{Uri: "/durian", Method: "pub"}:          0b00000,
		{Uri: "/apple/seed", Method: "pub"}:      0b10001,
		{Uri: "/apple/stem/leaf", Method: "pub"}: 0b10001,
		{Uri: "/banana/peel", Method: "pub"}:     0b00100,
		{Uri: "/cherry", Method: "pub"}:          0b10000,
		{Uri: "/cherry", Method: "set"}:          0b10100,
	}

	for testMsg, expectedMask := range testCases {
		mask := d.getNecessaryOuts(testMsg.Uri, testMsg.Method)
		if mask != expectedMask {
			t.Errorf("Expected mask for uri %s and method %s was 0b%b but got 0b%b", testMsg.Uri, testMsg.Method, expectedMask, mask)
		}
	}
	// run through all test cases a second time to check that memoization works correctly
	for testMsg, expectedMask := range testCases {
		mask := d.getNecessaryOuts(testMsg.Uri, testMsg.Method)
		if mask != expectedMask {
			t.Errorf("On second run, expected mask for uri %s and method %s was 0b%b but got 0b%b", testMsg.Uri, testMsg.Method, expectedMask, mask)
		}
	}
}

// Generate the given number of distinct messages for testing
func generateMessages(numMessages int, uri string, method string, bodyGenerator func(i int) interface{}) []*fims.FimsMsg {
	msgs := make([]*fims.FimsMsg, numMessages)
	for i := 0; i < numMessages; i++ {
		msgs[i] = &fims.FimsMsg{
			Method: method,
			Uri:    uri,
			Body:   bodyGenerator(i),
		}
	}
	return msgs
}
