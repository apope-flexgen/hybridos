package main

import (
	"errors"
	"os"
	"path/filepath"
	"reflect"
	"testing"
)

// Tests for the client functions which interface with the client DB

// Test adding file to retry when file has an entry in DB
func TestAddFileToRetryFromError_FileKnown(t *testing.T) {
	cl, err := createTestClient(t.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		t.Fatal(err)
	}

	testFileName := "testFile.tar.gz"
	_, err = os.Create(filepath.Join(cl.config.Dir, "error", testFileName))
	if err != nil {
		t.Fatal(err)
	}

	cl.addFailure(testFileName, testServerName1)
	cl.addFileToRetryFromError(testFileName)

	assertChannelReceive_string(t, cl.retryQ[testServerName1], "waiting for retry request to server 1")
	assertTimeoutChannelReceive_string(t, cl.retryQ[testServerName2], "checking that there isn't a retry request to server 2")
	if _, err := os.Stat(filepath.Join(cl.config.Dir, "error", testFileName)); errors.Is(err, os.ErrNotExist) {
		t.Fatalf("Test file removed from client error dir.")
	}
	if _, err := os.Stat(filepath.Join(cl.config.Dir, testFileName)); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("Test file found in client input dir.")
	}
	serverMap, err := cl.db.Get(testFileName)
	if err != nil {
		t.Fatalf("Failed to get failed servers for test file: %v.", err)
	}
	expectedServerMap := map[string]bool{
		testServerName1: false,
	}
	if !reflect.DeepEqual(serverMap, expectedServerMap) {
		t.Fatalf("Stored server map %v was not equal to expected server map %v.", serverMap, expectedServerMap)
	}
}

// Test adding file to retry when file has no entry in DB
func TestAddFileToRetryFromError_FileUnknown(t *testing.T) {
	cl, err := createTestClient(t.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		t.Fatal(err)
	}

	testFileName := "testFile.tar.gz"
	_, err = os.Create(filepath.Join(cl.config.Dir, "error", testFileName))
	if err != nil {
		t.Fatal(err)
	}

	cl.addFileToRetryFromError(testFileName)

	assertTimeoutChannelReceive_string(t, cl.retryQ[testServerName2], "checking that there isn't a retry request to server 1")
	assertTimeoutChannelReceive_string(t, cl.retryQ[testServerName2], "checking that there isn't a retry request to server 2")
	if _, err := os.Stat(filepath.Join(cl.config.Dir, "error", testFileName)); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("Test file not removed from client error dir.")
	}
	if _, err := os.Stat(filepath.Join(cl.config.Dir, testFileName)); errors.Is(err, os.ErrNotExist) {
		t.Fatalf("Test file not found in client input directory.")
	}
	_, err = cl.db.Get(testFileName)
	if err != errDbKeyNotFound {
		t.Fatalf("Instead of expected DB key not found error, got this error: %v.", err)
	}
}

// Tests that failure can be added to DB for a file that hasn't had a failure yet
func TestAddFailure_FirstFailure(t *testing.T) {
	cl, err := createTestClient(t.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		t.Fatal(err)
	}

	testFileName := "testFile.tar.gz"
	_, err = os.Create(filepath.Join(cl.config.Dir, "error", testFileName))
	if err != nil {
		t.Fatal(err)
	}

	err = cl.addFailure(testFileName, testServerName1)
	if err != nil {
		t.Fatalf("Adding failure returned error: %v.", err)
	}

	serverMap, err := cl.db.Get(testFileName)
	if err != nil {
		t.Fatalf("Failed to get failed servers for test file: %v.", err)
	}
	expectedServerMap := map[string]bool{
		testServerName1: false,
	}
	if !reflect.DeepEqual(serverMap, expectedServerMap) {
		t.Fatalf("Stored server map %v was not equal to expected server map %v.", serverMap, expectedServerMap)
	}
}

// Tests that failure can be added to DB for a file that has had a failure
func TestAddFailure_SecondFailure(t *testing.T) {
	cl, err := createTestClient(t.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		t.Fatal(err)
	}

	testFileName := "testFile.tar.gz"
	_, err = os.Create(filepath.Join(cl.config.Dir, "error", testFileName))
	if err != nil {
		t.Fatal(err)
	}

	err = cl.addFailure(testFileName, testServerName1)
	if err != nil {
		t.Fatalf("Adding first failure returned error: %v.", err)
	}
	err = cl.addFailure(testFileName, testServerName3)
	if err != nil {
		t.Fatalf("Adding second failure returned error: %v.", err)
	}

	serverMap, err := cl.db.Get(testFileName)
	if err != nil {
		t.Fatalf("Failed to get failed servers for test file: %v.", err)
	}
	expectedServerMap := map[string]bool{
		testServerName1: false,
		testServerName3: false,
	}
	if !reflect.DeepEqual(serverMap, expectedServerMap) {
		t.Fatalf("Stored server map %v was not equal to expected server map %v.", serverMap, expectedServerMap)
	}
}

// Tests that a failure can be removed from DB when the file has multiple failures
func TestRemoveFailure_MultipleFailures(t *testing.T) {
	cl, err := createTestClient(t.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		t.Fatal(err)
	}

	testFileName := "testFile.tar.gz"
	_, err = os.Create(filepath.Join(cl.config.Dir, "error", testFileName))
	if err != nil {
		t.Fatal(err)
	}

	err = cl.addFailure(testFileName, testServerName1)
	if err != nil {
		t.Fatalf("Adding first failure returned error: %v.", err)
	}
	err = cl.addFailure(testFileName, testServerName2)
	if err != nil {
		t.Fatalf("Adding second failure returned error: %v.", err)
	}

	noMoreFailedServers, err := cl.removeFailure(testFileName, testServerName1)
	if err != nil {
		t.Fatalf("Removing failure returned error: %v.", err)
	}
	if noMoreFailedServers {
		t.Fatalf("Removing failure erroneously returned that there are no more failures.")
	}

	serverMap, err := cl.db.Get(testFileName)
	if err != nil {
		t.Fatalf("Failed to get failed servers for test file: %v.", err)
	}
	expectedServerMap := map[string]bool{
		testServerName2: false,
	}
	if !reflect.DeepEqual(serverMap, expectedServerMap) {
		t.Fatalf("Stored server map %v was not equal to expected server map %v.", serverMap, expectedServerMap)
	}
}

// Tests that a failure can be removed from DB when the file has only one failure
func TestRemoveFailure_SingleFailure(t *testing.T) {
	cl, err := createTestClient(t.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		t.Fatal(err)
	}

	testFileName := "testFile.tar.gz"
	_, err = os.Create(filepath.Join(cl.config.Dir, "error", testFileName))
	if err != nil {
		t.Fatal(err)
	}

	err = cl.addFailure(testFileName, testServerName2)
	if err != nil {
		t.Fatalf("Adding failure returned error: %v.", err)
	}

	noMoreFailedServers, err := cl.removeFailure(testFileName, testServerName2)
	if err != nil {
		t.Fatalf("Removing failure returned error: %v.", err)
	}
	if !noMoreFailedServers {
		t.Fatalf("Removing failure erroneously returned that there are still more failures.")
	}

	_, err = cl.db.Get(testFileName)
	if err != errDbKeyNotFound {
		t.Fatalf("Instead of expected DB key not found error, got this error: %v.", err)
	}
}
