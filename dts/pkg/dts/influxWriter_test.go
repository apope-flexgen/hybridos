package dts

import "testing"

// These tests must be executed when a local Influx is running, they may modify Influx databases freely.
// Influx is expected to have no data when the test is run.

// Test creating a database in Influx with different retention policies
func TestEnsureDatabase(t *testing.T) {
	// TODO: before code review, add test cases for continuous query creation
	GlobalConfig.InfluxAddr = "localhost:8086"
	writerToInflux := NewInfluxWriter(nil)
	writerToInflux.initialize()

	type ensureDBTestCase struct {
		database string
		rps      []RetentionPolicyConfig
		cqs      []ContinuousQueryConfig
	}
	testCases := []ensureDBTestCase{
		{
			database: " !#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~",
			rps: []RetentionPolicyConfig{
				{
					Name:     "90d-rp",
					Duration: "90d",
				},
			},
		},
		{
			database: " !#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~",
			rps: []RetentionPolicyConfig{
				{
					Name:     "inf-rp",
					Duration: "inf",
				},
			},
		},
		{
			database: "ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.0123456789:;<=>?@hijklmnopqrstuvwxyz{|}~",
			rps: []RetentionPolicyConfig{
				{
					Name:     "inf-rp",
					Duration: "inf",
				},
			},
		},
		{
			database: "ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.0123456789:;<=>?@hijklmnopqrstuvwxyz{|}~",
			rps: []RetentionPolicyConfig{
				{
					Name:     "90d-rp",
					Duration: "90d",
				},
			},
		},
		{
			database: "0123456789:;<=>?@hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.{|}~",
			rps: []RetentionPolicyConfig{
				{
					Name:     "48h-rp",
					Duration: "48h",
				},
			},
		},
		{
			database: "0123456789:;<=>?@hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefg !#$%&'()*+,-.{|}~",
			rps: []RetentionPolicyConfig{
				{
					Name:     "1h-rp",
					Duration: "1h",
				},
			},
		},
		{
			database: "test_database",
			rps: []RetentionPolicyConfig{
				{
					Name:     "24h-rp",
					Duration: "24h",
				},
			},
		},
		{
			database: "test_database",
			rps: []RetentionPolicyConfig{
				{
					Name:     "1h-rp",
					Duration: "1h",
				},
			},
		},
		{
			database: "test-database",
			rps: []RetentionPolicyConfig{
				{
					Name:     "inf-rp",
					Duration: "inf",
				},
			},
		},
		{
			database: "test database",
			rps: []RetentionPolicyConfig{
				{
					Name:     "5w-rp",
					Duration: "5w",
				},
			},
		},
		{
			database: "TEST DATABASE",
			rps: []RetentionPolicyConfig{
				{
					Name:     "12d-rp",
					Duration: "12d",
				},
			},
		},
		{
			database: "testDB1",
			rps: []RetentionPolicyConfig{
				{
					Name:     "30_day_rp",
					Duration: "30d",
				},
			},
			cqs: []ContinuousQueryConfig{
				{
					Name:     "10min_cq",
					GroupBy:  "time(10m), *",
					Resample: "EVERY 10m FOR 20m",
					RP:       "30_day_rp",
					Select:   "MIN(*), MAX(*), MEAN(*), LAST(*)",

					Measurement: ":MEASUREMENT",
					From:        "/.*/",
					Into:        "",
				},
			},
		},
		{
			database: "testDB2",
			rps: []RetentionPolicyConfig{
				{
					Name:     "30_day_rp",
					Duration: "30d",
				},
				{
					Name:     "120_day_rp",
					Duration: "120d",
				},
			},
			cqs: []ContinuousQueryConfig{
				{
					Name:     "10min_cq",
					GroupBy:  "time(10m), *",
					Resample: "EVERY 10m FOR 20m",
					RP:       "120_day_rp",
					Select:   "MIN(*), MAX(*), MEAN(*), LAST(*)",

					Measurement: ":MEASUREMENT",
					From:        "/.*/",
					Into:        "",
				},
				{
					Name:     "5min_cq",
					GroupBy:  "time(5m), *",
					Resample: "EVERY 5m FOR 10m",
					RP:       "30_day_rp",
					Select:   "MIN(*), MAX(*), MEAN(*), LAST(*)",

					Measurement: ":MEASUREMENT",
					From:        "/.*/",
					Into:        "",
				},
			},
		},
		{
			database: "testDB3",
		},
	}

	for _, testCase := range testCases {
		// assume success if there is no error
		// TODO: Influx can quietly fail to create the retention policy if the rp duration is invalid (i.e. < 1h), consider if/how that case should be handled
		err := writerToInflux.ensureDB(testCase.database, testCase.rps, testCase.cqs)
		if err != nil {
			t.Errorf("Failed to setup Influx database %s due to error: %v", testCase.database, err)
		}
	}
}
