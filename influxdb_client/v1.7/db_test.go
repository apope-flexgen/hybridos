package db

import (
	"fmt"
	"math/rand"
	"testing"
	"time"

	influx "github.com/influxdata/influxdb1-client/v2"
)

var policies = []RetentionPolicy{
	{
		Name:               "autogen",
		Duration:           "0s",
		ShardGroupDuration: "168h0m0s",
		ReplicaN:           1,
		Default:            false,
	},
	{
		Name:               "infinite_rp",
		Duration:           "0s",
		ShardGroupDuration: "168h0m0s",
		ReplicaN:           1,
		Default:            false,
	},
	{
		Name:               "30_day_rp",
		Duration:           "720h0m0s",
		ShardGroupDuration: "24h0m0s",
		ReplicaN:           1,
		Default:            true,
	},
	{
		Name:               "120_day_rp",
		Duration:           "2880h0m0s",
		ShardGroupDuration: "24h0m0s",
		ReplicaN:           1,
		Default:            false,
	},
}

var new_policy = RetentionPolicy{
	Name:               "new_policy",
	Duration:           "5000h0m0s",
	ShardGroupDuration: "48h0m0s",
	ReplicaN:           1,
	Default:            false,
}

var contQueries = []ContQuery{
	{
		Name:        "cq_avgpH",
		Db:          "test",
		Select:      "MEAN(pH)",
		Into:        "test",
		Rp:          "autogen",
		Measurement: "average_pH",
		From:        "h2o_pH",
		Groupby:     "time(1d)",
	},
	{
		Name:        "cq_avgAll",
		Db:          "test",
		Select:      "MEAN(pH)",
		Into:        "test",
		Rp:          "autogen",
		Measurement: ":MEASUREMENT",
		From:        "h2o_pH",
		Groupby:     "time(1d)",
	},
	{
		Name:        "cq_avgAllResampled",
		Db:          "test",
		Resample:    "EVERY 1d FOR 2d",
		Select:      "MEAN(pH)",
		Into:        "test",
		Rp:          "autogen",
		Measurement: ":MEASUREMENT",
		From:        "h2o_pH",
		Groupby:     "time(1d)",
	},
}

var connector = NewConnector("localhost:8086", time.Minute, time.Second/100, false)
var gzipper = NewConnector("localhost:8086", time.Minute, time.Second/100, true)

func TestInfluxPing(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	err = connector.Ping()
	if err != nil {
		t.Error(err)
	}
}

func TestCreateDBWithPolicies(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	_, err = connector.Client.Query(influx.NewQuery("DROP", "test", ""))
	if err != nil {
		t.Error(err)
	}

	err = connector.CreateDatabase("test", policies)
	if err != nil {
		t.Error(err)
	}

	bp, err := influx.NewBatchPoints(influx.BatchPointsConfig{
		Database:  "test",
		Precision: "s",
	})
	if err != nil {
		t.Error(err)
	}

	err = connector.Client.Write(bp)
	if err != nil {
		t.Error(err)
	}

	_, err = connector.Client.Query(influx.NewQuery("SELECT * FROM metadata", "test", ""))
	if err != nil {
		t.Error(err)
	}
}

func TestWriteData(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	_, err = connector.Client.Query(influx.NewQuery("DROP DATABASE", "test", ""))
	if err != nil {
		t.Error(err)
	}

	err = connector.CreateDatabase("test", nil)
	if err != nil {
		t.Error(err)
	}

	timestamps, data := makeData(1000, 100)
	t.Log("made data")

	err = connector.WriteData("test", "prefix", "source", timestamps, data, map[string]interface{}{})
	if err != nil {
		t.Error("failed to write batch: " + err.Error())
	}
}

func TestMakeBatches(t *testing.T) {
	timestamps, data := makeData(1000, 100)
	t.Log("made data")

	_, err := connector.MakeBatches("test", "prefix", "source", timestamps, data, map[string]interface{}{})
	if err != nil {
		t.Error("failed to make batches: " + err.Error())
	}
}

func TestWriteBatches(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	_, err = connector.Client.Query(influx.NewQuery("DROP DATABASE", "test", ""))
	if err != nil {
		t.Error(err)
	}

	err = connector.CreateDatabase("test", nil)
	if err != nil {
		t.Error(err)
	}

	timestamps, data := makeData(1000, 100)
	t.Log("made data")

	batches, err := connector.MakeBatches("test", "prefix", "source", timestamps, data, map[string]interface{}{})
	if err != nil {
		t.Error("failed to make batches: " + err.Error())
	}

	for _, batch := range batches {
		err = connector.WriteBatch(batch)
		if err != nil {
			t.Error("failed to write batch: " + err.Error())
		}
	}
}

func TestAddRetentionPolicy(t *testing.T) {
	TestCreateDBWithPolicies(t)

	query, err := connector.AddRetentionPolicy("test", new_policy)
	if err != nil {
		t.Log(query)
		t.Error(err)
	}
}

func TestSetRetentionPolicy(t *testing.T) {
	TestCreateDBWithPolicies(t)

	query, err := connector.SetRetentionPolicy("test", "120_day_rp")
	if err != nil {
		t.Log(query)
		t.Error(err)
	}
}

func TestContinuousQuery(t *testing.T) {
	TestCreateDBWithPolicies(t)

	_, err := connector.AddRetentionPolicy("test", policies[3])
	if err != nil {
		t.Error(err)
	}

	for _, cq := range contQueries {
		query, err := connector.RunContinuousQuery(cq)
		if err != nil {
			t.Log("\n" + query)
			t.Error(err)
		}
	}
}

func TestHealthCheck(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Fatal(err)
	}

	ok, err := connector.HealthCheck()
	if err != nil {
		t.Fatal(err)
	}

	t.Log(ok)

	connector.Disconnect()
}

func TestCompareGZip(t *testing.T) {
	fmt.Println("testing WriteData...")
	gzipct := 0
	nonect := 0
	for numPoints := 10; numPoints <= 1000; numPoints *= 10 {
		for numFields := 10; numFields <= 1000; numFields *= 10 {
			fmt.Printf("%v points with %v fields:\n", numPoints, numFields)
			gzipfaster, res, err := compareGZip0(numPoints, numFields)
			if err != nil {
				t.Fatal(err)
			}

			if gzipfaster {
				gzipct++
			} else {
				nonect++
			}

			fmt.Printf("\t: %s\n", res)
		}
	}

	fmt.Printf("GZIP-encoded wins: %v\n", gzipct)
	fmt.Printf("NON-encoded  wins: %v\n", nonect)

	fmt.Println("testing MakeBatches -> WriteBatch")
	gzipct = 0
	nonect = 0
	for numPoints := 10; numPoints <= 1000; numPoints *= 10 {
		for numFields := 10; numFields <= 1000; numFields *= 10 {
			fmt.Printf("%v points with %v fields:\n", numPoints, numFields)
			gzipfaster, res, err := compareGZip1(numPoints, numFields)
			if err != nil {
				t.Fatal(err)
			}

			if gzipfaster {
				gzipct++
			} else {
				nonect++
			}

			fmt.Printf("\t: %s\n", res[0])
			fmt.Printf("\t: %s\n", res[1])
			fmt.Printf("\t: %s\n", res[2])
		}
	}

	fmt.Printf("GZIP-encoded wins: %v\n", gzipct)
	fmt.Printf("NON-encoded  wins: %v\n", nonect)
}

func compareGZip0(numPoints, numFields int) (bool, string, error) {
	times, data := makeData(numPoints, numFields)

	// === NO ENCODING ===
	err := connector.Connect()
	if err != nil {
		return false, "", err
	}

	// make batches
	ns := time.Now().UnixNano()
	err = connector.WriteData("test", "none", "connector", times, data, nil)
	if err != nil {
		return false, "", err
	}
	nd := time.Now().UnixNano() - ns
	connector.Disconnect()

	// === GZIP ENCODING ===
	err = gzipper.Connect()
	if err != nil {
		return false, "", err
	}

	// make batches
	gs := time.Now().UnixNano()
	err = gzipper.WriteData("test", "none", "connector", times, data, nil)
	if err != nil {
		return false, "", err
	}
	gd := time.Now().UnixNano() - gs
	gzipper.Disconnect()

	if nd > gd {
		return true, fmt.Sprintf("GZIP-encoding was FASTER by %vns (%.1f%%)", nd-gd, float32(nd-gd)/float32(nd)*100), nil
	} else {
		return false, fmt.Sprintf("NON-encoding was FASTER by %vns (%.1f%%)", gd-nd, float32(gd-nd)/float32(gd)*100), nil
	}
}

func compareGZip1(numPoints, numFields int) (bool, []string, error) {
	times, data := makeData(numPoints, numFields)

	// === NO ENCODING ===
	err := connector.Connect()
	if err != nil {
		return false, nil, err
	}

	// make batches
	nbs := time.Now().UnixNano()
	batches, err := connector.MakeBatches("test", "none", "connector", times, data, nil)
	if err != nil {
		return false, nil, err
	}
	nbd := time.Now().UnixNano() - nbs

	// write batches
	nws := time.Now().UnixNano()
	for _, batch := range batches {
		err = connector.WriteBatch(batch)
		if err != nil {
			return false, nil, err
		}
	}
	nwd := time.Now().UnixNano() - nws
	connector.Disconnect()

	// === GZIP ENCODING ===
	err = gzipper.Connect()
	if err != nil {
		return false, nil, err
	}

	// make batches
	gbs := time.Now().UnixNano()
	batches, err = gzipper.MakeBatches("test", "none", "connector", times, data, nil)
	if err != nil {
		return false, nil, err
	}
	gbd := time.Now().UnixNano() - gbs

	// write batches
	gws := time.Now().UnixNano()
	for _, batch := range batches {
		err = gzipper.WriteBatch(batch)
		if err != nil {
			return false, nil, err
		}
	}
	gwd := time.Now().UnixNano() - gws
	gzipper.Disconnect()

	var batchres string
	if nbd > gbd {
		batchres = fmt.Sprintf("GZIP-encoded batching was FASTER by %vns (%.1f%%)", nbd-gbd, float32(nbd-gbd)/float32(nbd)*100)
	} else {
		batchres = fmt.Sprintf("NON-encoded  batching was FASTER by %vns (%.1f%%)", gbd-nbd, float32(gbd-nbd)/float32(gbd)*100)
	}

	var writeres string
	if nwd > gwd {
		writeres = fmt.Sprintf("GZIP-encoded writing  was FASTER by %vns (%.1f%%)", nwd-gwd, float32(nwd-gwd)/float32(nwd)*100)
	} else {
		writeres = fmt.Sprintf("NON-encoded writing   was FASTER by %vns (%.1f%%)", gwd-nwd, float32(gwd-nwd)/float32(gwd)*100)
	}

	var gzipfaster bool
	var totres string
	if (nbd + nwd) > (gbd + gwd) {
		gzipfaster = true
		totres = fmt.Sprintf("GZIP-encoded total    was FASTER by %vns (%.1f%%)", (nbd+nwd)-(gbd+gwd), float32((nbd+nwd)-(gbd+gwd))/float32(nbd+nwd)*100)
	} else {
		gzipfaster = false
		totres = fmt.Sprintf("NON-encoded total     was FASTER by %vns (%.1f%%)", (gbd+gwd)-(nbd+nwd), float32((gbd+gwd)-(nbd+nwd))/float32(gbd+gwd)*100)
	}

	return gzipfaster, []string{batchres, writeres, totres}, nil
}

func makeData(numPoints, numFields int) ([]uint64, []map[string]interface{}) {
	data := make([]map[string]interface{}, numPoints)
	timestamps := make([]uint64, numPoints)
	for i := 0; i < numPoints; i++ {
		data[i] = map[string]interface{}{
			"num":    i,
			"string": fmt.Sprint(i),
			"alarm": []map[string]interface{}{
				{
					"value":  "alarm1",
					"string": "active",
				},
				{
					"value":  "alarm2",
					"string": "inactive",
				},
			},
		}
		for j := 0; j < numFields; j++ {
			data[i][fmt.Sprintf("rand%d", j)] = rand.Intn(numFields)
		}

		timestamps[i] = uint64(time.Now().UnixMicro())
	}

	return timestamps, data
}

// PLAYGROUND

// func TestScratch(t *testing.T) {
// 	err := connector.Connect()
// 	if err != nil {
// 		t.Fatal(err)
// 	}

// 	times, data := makeData(1, 1)
// 	t.Logf("entering value %v for \"num\"", data[0]["num"])

// 	err = connector.WriteData("test", "datatypes", "test", times, data, nil)
// 	if err != nil {
// 		t.Fatal(err)
// 	}
// 	t.Log("write successful")

// 	times, data = makeData(1, 1)
// 	data[0]["num"] = true
// 	t.Logf("entering value %v for \"num\"", data[0]["num"])

// 	err = connector.WriteData("test", "datatypes", "test", times, data, nil)
// 	if err != nil {
// 		t.Fatal(err)
// 	}
// 	t.Log("write successful")
// }
