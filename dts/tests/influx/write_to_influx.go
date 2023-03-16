// Tests writing a specified archive to influx

package main

import (
	"fmt"
	"os"
	"path"
	"time"

	"github.com/flexgen-power/fims_codec"
	log "github.com/flexgen-power/go_flexgen/logger"
	influx "github.com/flexgen-power/influxdb_client/v1.7"
)

type archiveData struct {
	archiveFilePath string
	archiveSize     int64
	db              string
	measurement     string
	points          *fims_codec.DecodedData
}

var influxConn influx.InfluxConnector

func main() {
	err := (&log.Config{
		Moniker:           "dts test",
		ToConsole:         true,
		ToFile:            false,
		SeverityThreshold: log.TRACE,
		FileName:          "/var/log/flexgen/dts/dts.log",
	}).Init("dts")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	archiveFilePath := os.Args[1]
	err = connectToInflux("localhost:8086", 0.01)
	if err != nil {
		log.Fatalf("Failed to connect to Influx: %v", err)
	}
	data := getDecodedData(archiveFilePath)
	err = writeToInflux(data)
	if err != nil {
		log.Fatalf("Failed to write to Influx: %v", err)
	}
	log.Infof("Got to the end of the test program")
}

func connectToInflux(address string, writeDelay float64) error {
	// create connection to influxdb
	conn := influx.NewConnector(address, time.Minute/2, time.Duration(writeDelay*float64(time.Second)), false)
	influxConn = conn

	err := influxConn.Connect()
	if err != nil {
		return err
	}

	err = influxConn.Ping()
	if err != nil {
		return err
	}
	return nil
}

func getDecodedData(archiveFilePath string) *archiveData {
	// decode message
	points, _ := fims_codec.Decode(archiveFilePath) // attempt decode FIMS messages

	// gather additional info
	addInfo := points.AdditionalData

	info, _ := os.Stat(archiveFilePath) // get size of archive
	archiveSize := info.Size()

	return &archiveData{
		archiveFilePath: archiveFilePath,
		archiveSize:     archiveSize,
		db:              addInfo["database"],
		measurement:     addInfo["measurement"],
		points:          points,
	}
}

func writeToInflux(data *archiveData) error {
	tmpName := "\"" + data.db + "\""
	influxConn.CreateDatabase(tmpName, nil)

	// create metadata to accompany archive
	metadata := map[string]interface{}{
		"messages":         len(data.points.MsgBodies),
		"archives":         1,
		"archiveSizeBytes": data.archiveSize,
	}
	sourceUri := path.Base(data.points.Uri)

	batches, err := influxConn.MakeBatches(data.db, data.measurement, sourceUri, data.points.MsgTimestamps, data.points.MsgBodies, metadata) // write batch points
	if err != nil {
		return fmt.Errorf("failed to make batches, err: %w", err)
	}

	numBatches := len(batches)
	before := time.Now()
	for _, batch := range batches {
		err = influxConn.WriteBatch(batch)
		if err != nil {
			return fmt.Errorf("failed to write batch, err: %w", err)
		}
	}
	after := time.Now()
	log.Debugf("Wrote %d batches in %f seconds\n", numBatches, after.Sub(before).Seconds())

	return nil
}
