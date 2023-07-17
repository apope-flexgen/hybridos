package dts

import "testing"

// These tests must be executed when a local Influx is running, they may modify Influx databases freely.
// Influx is expected to have no data when the test is run.

// Test creating a database in Influx with different retention policies
func TestEnsureDatabase(t *testing.T) {
	writerToInflux := NewInfluxWriter(nil)
	writerToInflux.initialize()

	type ensureDBTestCase struct {
		database          string
		retentionDuration string
	}
	testCases := []ensureDBTestCase{
		{
			" !#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~",
			"90d",
		},
		{
			" !#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~",
			"inf",
		},
		{
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.0123456789:;<=>?@hijklmnopqrstuvwxyz{|}~",
			"inf",
		},
		{
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.0123456789:;<=>?@hijklmnopqrstuvwxyz{|}~",
			"90d",
		},
		{
			"0123456789:;<=>?@hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.{|}~",
			"48h",
		},
		{
			"0123456789:;<=>?@hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.{|}~",
			"1h",
		},
		{
			"test_database",
			"24h",
		},
		{
			"test_database",
			"1h",
		},
		{
			"test-database",
			"inf",
		},
		{
			"test database",
			"5w",
		},
		{
			"TEST DATABASE",
			"12d",
		},
	}

	for _, testCase := range testCases {
		// assume success if there is no error
		// TODO: Influx can quietly fail to create the retention policy if the rp duration is invalid (i.e. < 1h), consider if/how that case should be handled
		err := writerToInflux.ensureDB(testCase.database, testCase.retentionDuration, "rp-"+testCase.retentionDuration)
		if err != nil {
			t.Errorf("Failed to ensure database %s exists with retention policy %s due to error: %v", testCase.database, testCase.retentionDuration, err)
		}
	}
}
