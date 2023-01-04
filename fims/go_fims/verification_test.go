package fims

import (
	"strconv"
	"strings"
	"testing"
	"time"
)

// Test Unique Id generation
func TestGetUniqueID(t *testing.T) {
	ConfigureVerification(0, nil)
	// Valid case including rollover
	for i := 0; i < 10000; i++ {
		uniqueID := GetUniqueID("/single/example")
		if uniqueID != i%1000 {
			t.Errorf("Unique id %d did not match expected valid id %d\n", uniqueID, i%1000)
		}
	}
	// Invalid case including rollover
	for i := 1; i <= 10000; i++ {
		uniqueID := GetUniqueID("/single/example")
		if uniqueID == i%999 {
			t.Errorf("Unique id %d matched expected invalid id %d\n", uniqueID, i%999)
		}
	}
	// Test concurrent uris
	for i := 0; i < 3000; i++ {
		if i%3 == 0 {
			uniqueID := GetUniqueID("/uri/one")
			if uniqueID != (i/3)%1000 {
				t.Errorf("Unique id %d did not match expected valid id %d\n", uniqueID, (i/3)%1000)
			}
		}
	}

	tearDown()
}

// Test addition to records for both "all" and "latest" cases
func TestAddVerificationRecordEntry(t *testing.T) {
	// First try to add without configuring, should fail
	added, _, err := VerificationRecords.addVerificationRecordEntry("all", FimsMsg{})
	if added || err == nil || err.Error() != "verification record map undefined" {
		t.Errorf("Should have received error for unconfigured records")
	}

	ConfigureVerification(1, nil)

	// Try to add a record without supplying a replyto (map key), should fail
	added, _, err = VerificationRecords.addVerificationRecordEntry("all", FimsMsg{})
	if added || err == nil || err.Error() != "no replyto uri provided" {
		t.Errorf("Should have received error for no replyto")
	}

	// Test "all" records case
	numEntries := 3
	added, err = addRecordsHelper(numEntries, "all")
	if !added || err != nil {
		t.Errorf(err.Error())
	}

	if len(VerificationRecords.VerificationRecordMap) != numEntries {
		t.Errorf("Should have added %d, but length is %d", numEntries, len(VerificationRecords.VerificationRecordMap))
	}

	for i := 0; i < numEntries; i++ {
		expectedKey := "/replyto/test/" + strconv.Itoa(i)
		if _, ok := VerificationRecords.VerificationRecordMap[expectedKey]; !ok {
			t.Errorf("Should have an entry for uri: %s", expectedKey)
		}
	}

	// Test rollover of uniqueID, should be overwritten (added true) but include an error for logging
	numEntries = 1001
	added, err = addRecordsHelper(numEntries-3, "all")
	if !added {
		t.Errorf(err.Error())
	} else if err == nil {
		t.Errorf("Should have errored on overwriting the last record")
	}

	VerificationRecords.ResetRecords()

	// Test "latest" records case
	numEntries = 3
	added, err = addRecordsHelper(numEntries, "latest")
	if !added || err != nil {
		t.Errorf(err.Error())
	}

	// Latest should only track a single, most up-to-date entry
	if len(VerificationRecords.VerificationRecordMap) != 1 {
		t.Errorf("Should have added %d, but length is %d", numEntries, len(VerificationRecords.VerificationRecordMap))
	}

	// All entries under the same key
	if _, ok := VerificationRecords.VerificationRecordMap["/replyto/test"]; !ok {
		t.Errorf("Should have an entry for uri: /replyto/test")
	}

	// Id should be the latest generated
	expectedId := numEntries - 1
	if VerificationRecords.VerificationRecordMap["/replyto/test"].id != expectedId {
		t.Errorf("Should have unqiue id: %d", expectedId)
	}

	tearDown()
}

// Test deletion from records for both "all" and "latest" cases
func TestDeleteVerificationRecordEntry(t *testing.T) {
	ConfigureVerification(1, nil)

	// Test "all" records case
	numEntries := 7
	addRecordsHelper(numEntries, "all")

	for i := 0; i < numEntries; i++ {
		expectedKey := "/replyto/test/" + strconv.Itoa(i)
		VerificationRecords.deleteVerificationRecordEntry(expectedKey)
		if len(VerificationRecords.VerificationRecordMap) != numEntries-i-1 {
			t.Errorf("Failed to delete")
		}
	}

	VerificationRecords.ResetRecords()

	// Test "latest" records case
	addRecordsHelper(numEntries, "latest")

	expectedKey := "/replyto/test"
	VerificationRecords.deleteVerificationRecordEntry(expectedKey)
	if len(VerificationRecords.VerificationRecordMap) != 0 {
		t.Errorf("Map should be empty, but instead has length: %d", len(VerificationRecords.VerificationRecordMap))
	}

	// Test deletion of a list of records
	addRecordsHelper(numEntries, "all")

	var keysToDelete []string
	for i := 0; i < numEntries; i++ {
		keysToDelete = append(keysToDelete, "/replyto/test/"+strconv.Itoa(numEntries+i))
	}
	VerificationRecords.deleteVerificationRecords(keysToDelete)
	if len(VerificationRecords.VerificationRecordMap) != 0 {
		t.Errorf("Map should be empty, but instead has length: %d", len(VerificationRecords.VerificationRecordMap))
	}

	tearDown()
}

func TestHandleVerificationResponse(t *testing.T) {
	// Example callback function, in this case used for testing
	// The user could define any function that works with the following map
	testCallback := func(msg map[string]interface{}) {
		if _, ok := msg["id"]; !ok {
			t.Error("id missing from message")
		}
		if _, ok := msg["timeSent"]; !ok {
			t.Error("timeSent missing from message")
		}
		if _, ok := msg["msg"]; !ok {
			t.Error("msg missing from message")
		}
		if _, ok := msg["recordType"]; !ok {
			t.Error("recordType missing from message")
		}
	}
	ConfigureVerification(1, testCallback)

	numEntries := 5
	addRecordsHelper(numEntries, "latest")

	// Invalid test cases
	invalidReplytos := []string{"/replyto/test/", "/replyto/test/-1", "/replyto/1/test", "/replyto/1test"}

	// Make sure the record exists
	if entry, ok := VerificationRecords.VerificationRecordMap["/replyto/test"]; !ok || entry.id != numEntries-1 || len(VerificationRecords.VerificationRecordMap) != 1 {
		t.Errorf("Failed to add latest entry")
	}

	// Mock the replyto response message and handle for invalid cases
	for _, invalidReplyto := range invalidReplytos {
		msg := FimsMsg{}
		msg.Uri = invalidReplyto
		msg.Nfrags = strings.Count(invalidReplyto, "/")
		VerificationRecords.handleVerificationResponse(msg)
		if len(VerificationRecords.VerificationRecordMap) != 1 {
			t.Errorf("Entry should not have been removed")
		}
	}

	// Valid case
	msg := FimsMsg{}
	msg.Uri = "/replyto/test/" + strconv.Itoa(numEntries-1)
	msg.Nfrags = 3
	VerificationRecords.handleVerificationResponse(msg)
	if len(VerificationRecords.VerificationRecordMap) != 0 {
		t.Errorf("Entry should have been removed")
	}

	// Same tests with "all" use case
	VerificationRecords.ResetRecords()
	addRecordsHelper(numEntries, "all")

	// Make sure the record exists
	if len(VerificationRecords.VerificationRecordMap) != numEntries {
		t.Errorf("Failed to add latest entry")
	}

	// Mock the replyto response message and handle for invalid cases
	for _, invalidReplyto := range invalidReplytos {
		msg := FimsMsg{}
		msg.Uri = invalidReplyto
		msg.Nfrags = strings.Count(invalidReplyto, "/")
		VerificationRecords.handleVerificationResponse(msg)
		if len(VerificationRecords.VerificationRecordMap) != numEntries {
			t.Errorf("Entry should not have been removed")
		}
	}

	// Valid case
	for i := 0; i < numEntries; i++ {
		msg := FimsMsg{}
		msg.Uri = "/replyto/test/" + strconv.Itoa(i)
		msg.Nfrags = 3
		VerificationRecords.handleVerificationResponse(msg)
		// Ensure the entry is no longer in the records
		if _, ok := VerificationRecords.VerificationRecordMap[msg.Uri]; ok || len(VerificationRecords.VerificationRecordMap) != numEntries-i-1 {
			t.Errorf("Entry should have been removed")
		}
	}

	tearDown()
}

func TestGetUnverifiedMessages(t *testing.T) {
	// Test unconfigured
	// First try to add without configuring, should fail
	_, err := VerificationRecords.GetUnverifiedMessages()
	if err == nil {
		t.Errorf("Should have received error for unconfigured records")
	}

	ConfigureVerification(1, nil)

	oldTime := time.Now().Add(time.Duration(-1) * time.Minute)
	oldEntry := verificationRecordEntry{
		id:       0,
		timeSent: oldTime,
		msg:      FimsMsg{Uri: "/fleet/example", Replyto: "/replyto/example"},
	}
	VerificationRecords.VerificationRecordMap["/replyto/example/0"] = oldEntry
	newEntry := verificationRecordEntry{
		id:       7,
		timeSent: time.Now(),
		msg:      FimsMsg{Uri: "/fleet/example", Replyto: "/replyto/example"},
	}
	VerificationRecords.VerificationRecordMap["/replyto/example/7"] = newEntry

	unverifiedMap, err := VerificationRecords.GetUnverifiedMessages()
	if err != nil {
		t.Errorf("Should have return map without error")
	}

	// Ensure only the latest entry is returned, as it is the only one within the configured expiration window (1min)
	if _, ok := VerificationRecords.VerificationRecordMap["/replyto/example/7"]; !ok || len(unverifiedMap) != 1 {
		t.Errorf("Map cleaning error")
	}

	tearDown()
}

// Add records for testing
func addRecordsHelper(numEntries int, recordType string) (bool, error) {
	// Valid additions
	msg := FimsMsg{}
	msg.Replyto = "/replyto/test"
	var returnErr error
	// Add valid records
	for i := 0; i < numEntries; i++ {
		added, _, err := VerificationRecords.addVerificationRecordEntry(recordType, msg)
		if !added {
			return added, err
		} else if err != nil {
			returnErr = err
		}
	}
	return true, returnErr
}

// Clear everything between tests
func tearDown() {
	VerificationRecords = AtomicVerificationRecords{}
}
