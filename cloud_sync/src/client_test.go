package main

import (
	"errors"
	"os"
	"path/filepath"
	"testing"
	"time"
)

const testClientName = "test_client"
const testServerName1 = "test_server_1"
const testServerName2 = "test_server_2"
const testServerName3 = "test_server_3"
const testWaitTimeout = 100 * time.Millisecond // found through trial and error to ensure tests pass consistently

// Create and return a new test client.
func createTestClient(testDirPath string, serverNames []string) (*client, error) {
	// initialize test client
	config = Config{
		Clients: map[string]ClientConfig{
			testClientName: {
				Dir:     filepath.Join(testDirPath, testClientName, "archives"),
				Servers: serverNames,
				Ext:     ".tar.gz",
			},
		},
		DbDir:             filepath.Join(testDirPath, ".cloud_sync", "db"),
		SleepLimitSeconds: 0,
		BufSz:             100000,
	}
	config.Servers = make(map[string]ServerConfig)
	for _, serverName := range serverNames {
		config.Servers[serverName] = ServerConfig{}
		servers[serverName] = &server{name: serverName}
	}
	return createClient(testClientName, config.Clients[testClientName])
}

// Test that send routine uses the correct channels when a transfer has a retryable failure.
func TestSendChannelsRetryableFailure(t *testing.T) {
	// initialize test client
	cl, err := createTestClient(t.TempDir(), []string{testServerName1})
	if err != nil {
		t.Fatal(err)
	}

	// start send routine
	go cl.send()

	// simulate retryable file transfer send failure
	testFile, err := os.Create(filepath.Join(cl.config.Dir, "testFile.tar.gz"))
	if err != nil {
		t.Fatal(err)
	}
	assertChannelSend_string(t,
		cl.newFilesQ, testFile.Name(),
		"sending file name to send routine")
	req := assertChannelReceive_transferRequest(t,
		cl.sendRequestQsToServers[testServerName1],
		"waiting for transfer request from send routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: errors.New("Test error"), retryable: true},
		"sending transfer response to send routine")
	assertChannelReceive_string(t,
		cl.clearQ,
		"waiting for file clear request")
	// in this case, the file should be sent to retry
	assertChannelReceive_string(t,
		cl.retryQ[testServerName1],
		"waiting for retry request")
}

// Test that send routine uses the correct channels when a transfer has a non-retryable failure.
func TestSendChannelsNonretryableFailure(t *testing.T) {
	// initialize test client
	cl, err := createTestClient(t.TempDir(), []string{testServerName1})
	if err != nil {
		t.Fatal(err)
	}

	// start send routine
	go cl.send()

	// simulate retryable file transfer send failure
	testFile, err := os.Create(filepath.Join(cl.config.Dir, "testFile.tar.gz"))
	if err != nil {
		t.Fatal(err)
	}
	assertChannelSend_string(t,
		cl.newFilesQ, testFile.Name(),
		"sending file name to send routine")
	req := assertChannelReceive_transferRequest(t,
		cl.sendRequestQsToServers[testServerName1],
		"waiting for transfer request from send routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: errors.New("Test error"), retryable: false},
		"sending transfer response to send routine")
	assertChannelReceive_string(t,
		cl.clearQ,
		"waiting for file clear request")
	// in this case, the file should not be sent to retry
	assertTimeoutChannelReceive_string(t,
		cl.retryQ[testServerName1],
		"checking that there isn't a retry request")
}

// Test that send routine uses the correct channels when a transfer succeeds.
func TestSendChannelsSuccess(t *testing.T) {
	// initialize test client
	cl, err := createTestClient(t.TempDir(), []string{testServerName1})
	if err != nil {
		t.Fatal(err)
	}

	// start send routine
	go cl.send()

	// simulate retryable file transfer send failure
	testFile, err := os.Create(filepath.Join(cl.config.Dir, "testFile.tar.gz"))
	if err != nil {
		t.Fatal(err)
	}
	assertChannelSend_string(t,
		cl.newFilesQ, testFile.Name(),
		"sending file name to send routine")
	req := assertChannelReceive_transferRequest(t,
		cl.sendRequestQsToServers[testServerName1],
		"waiting for transfer request from send routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: nil},
		"sending transfer response to send routine")
	assertChannelReceive_string(t,
		cl.clearQ,
		"waiting for file clear request")
	// in this case, the file should not be sent to retry
	assertTimeoutChannelReceive_string(t,
		cl.retryQ[testServerName1],
		"checking that there isn't a retry request")
}

// Test that retry routine uses the correct channels when a transfer has a retryable failure.
func TestRetryChannelsRetryableFailure(t *testing.T) {
	// initialize test client
	cl, err := createTestClient(t.TempDir(), []string{testServerName1})
	if err != nil {
		t.Fatal(err)
	}

	// start retry routine
	go cl.retry(servers[testServerName1])
	defer close(cl.retryQ[testServerName1])

	// simulate retryable file transfer retry failure
	testFile, err := os.Create(filepath.Join(cl.config.Dir, "error", "testFile.tar.gz"))
	if err != nil {
		t.Fatal(err)
	}
	assertChannelSend_string(t,
		cl.retryQ[testServerName1], testFile.Name(),
		"sending file name to retry routine")
	req := assertChannelReceive_transferRequest(t,
		cl.retryRequestQsToServers[testServerName1],
		"waiting for transfer request from retry routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: errors.New("Test error"), retryable: true},
		"sending transfer response to retry routine")
	// in this case, the file should not be cleared and there should be another retry attempt
	assertTimeoutChannelReceive_string(t,
		cl.clearQ,
		"checking that there isn't a file clear request")
	req = assertChannelReceive_transferRequest(t,
		cl.retryRequestQsToServers[testServerName1],
		"waiting for 2nd transfer request from retry routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: nil},
		"sending 2nd transfer response to retry routine")
}

// Test that retry routine uses the correct channels when a transfer has a non-retryable failure.
func TestRetryChannelsNonretryableFailure(t *testing.T) {
	// initialize test client
	cl, err := createTestClient(t.TempDir(), []string{testServerName1})
	if err != nil {
		t.Fatal(err)
	}

	// start retry routine
	go cl.retry(servers[testServerName1])
	defer close(cl.retryQ[testServerName1])

	// simulate non-retryable file transfer retry failure
	testFile, err := os.Create(filepath.Join(cl.config.Dir, "error", "testFile.tar.gz"))
	if err != nil {
		t.Fatal(err)
	}
	assertChannelSend_string(t,
		cl.retryQ[testServerName1], testFile.Name(),
		"sending file name to retry routine")
	req := assertChannelReceive_transferRequest(t,
		cl.retryRequestQsToServers[testServerName1],
		"waiting for transfer request from retry routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: errors.New("Test error"), retryable: false},
		"sending transfer response to retry routine")
	// in this case, there should not be another retry attempt and the file should not be cleared
	// (leave the file in the error dir and the db but don't continue retrying)
	assertTimeoutChannelReceive_string(t,
		cl.clearQ,
		"checking that there isn't a clear request")
	assertTimeoutChannelReceive_transferRequest(t,
		cl.retryRequestQsToServers[testServerName1],
		"checking that there isn't a 2nd transfer request from retry routine")
}

// Test that retry routine uses the correct channels when a transfer is successful.
func TestRetryChannelsSuccess(t *testing.T) {
	// initialize test client
	cl, err := createTestClient(t.TempDir(), []string{testServerName1})
	if err != nil {
		t.Fatal(err)
	}

	// start retry routine
	go cl.retry(servers[testServerName1])
	defer close(cl.retryQ[testServerName1])

	// simulate successful file transfer retry
	testFile, err := os.Create(filepath.Join(cl.config.Dir, "error", "testFile.tar.gz"))
	if err != nil {
		t.Fatal(err)
	}
	assertChannelSend_string(t,
		cl.retryQ[testServerName1], testFile.Name(),
		"sending file name to retry routine")
	req := assertChannelReceive_transferRequest(t,
		cl.retryRequestQsToServers[testServerName1],
		"waiting for transfer request from retry routine")
	assertChannelSend_transferResponse(t,
		req.responseChannel, transferResponse{err: nil},
		"sending transfer response to retry routine")
	// in this case, the file should be cleared and there should not be another retry attempt
	assertChannelReceive_string(t,
		cl.clearQ,
		"waiting for file clear request")
	assertTimeoutChannelReceive_transferRequest(t,
		cl.retryRequestQsToServers[testServerName1],
		"checking that there isn't a 2nd transfer request from retry routine")
}

/**
 * The following helper functions have been manually monomorphized to work with multiple types.
 * i.e. assertChannelSend_typeA is the exact same as assertChannelSend_typeB except that one
 * works with typeA and the other works with typeB
 * TODO: Implement with generic type parameters if/when we move to Go 1.18. Alternatively,
 * automate monomorphization with go generate.
 */

// Fails the test if we cannot send on the channel within the timeout.
func assertChannelSend_string(t *testing.T, c chan<- string, msg string, actionDescription string) {
	t.Helper()
	select {
	case c <- msg:
	case <-time.After(testWaitTimeout):
		t.Fatalf("Timed out when %s.", actionDescription)
	}
}

// Fails the test if we cannot send on the channel within the timeout.
func assertChannelSend_transferResponse(t *testing.T, c chan<- transferResponse, msg transferResponse, actionDescription string) {
	t.Helper()
	select {
	case c <- msg:
	case <-time.After(testWaitTimeout):
		t.Fatalf("Timed out when %s.", actionDescription)
	}
}

// Fails the test if we cannot receive on the channel within the timeout.
// Returns the message received if successful.
func assertChannelReceive_string(t *testing.T, c <-chan string, actionDescription string) (msg string) {
	t.Helper()
	select {
	case msg = <-c:
	case <-time.After(testWaitTimeout):
		t.Fatalf("Timed out when %s.", actionDescription)
	}
	return msg
}

// Fails the test if we cannot receive on the channel within the timeout.
// Returns the message received if successful.
func assertChannelReceive_transferRequest(t *testing.T, c <-chan transferRequest, actionDescription string) (msg transferRequest) {
	t.Helper()
	select {
	case msg = <-c:
	case <-time.After(testWaitTimeout):
		t.Fatalf("Timed out when %s.", actionDescription)
	}
	return msg
}

// Fails the test if we do not timeout when attempting to receive on the channel
func assertTimeoutChannelReceive_string(t *testing.T, c <-chan string, actionDescription string) {
	t.Helper()
	select {
	case <-c:
		t.Fatalf("Saw an unexpected receive when %s.", actionDescription)
	case <-time.After(testWaitTimeout):
	}
}

// Fails the test if we do not timeout when attempting to receive on the channel
func assertTimeoutChannelReceive_transferRequest(t *testing.T, c <-chan transferRequest, actionDescription string) {
	t.Helper()
	select {
	case <-c:
		t.Fatalf("Saw an unexpected receive when %s.", actionDescription)
	case <-time.After(testWaitTimeout):
	}
}
