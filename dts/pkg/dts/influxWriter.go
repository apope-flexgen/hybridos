package dts

import (
	"context"
	"errors"
	"fmt"
	"path"
	"sync"
	"sync/atomic"
	"time"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	influx "github.com/flexgen-power/hybridos/influxdb_client/v1.7"
	"golang.org/x/sync/errgroup"
)

type InfluxWriter struct {
	in                   <-chan *decodedDataFileData
	preparedBatchesQueue chan *archiveBatches
	DbUrl                string // Address of the influx server

	knownDbs sync.Map // To be used like a hashset of strings, stores which databases have already been verified to exist in influx
	WriteCnt uint64   // Number of successfully written archives, must be modified atomically
	FailCnt  uint64   // Number of archives that failed to send, must be modified atomically

	influxConn influx.InfluxConnector
}

// Batches of points prepared from archive data
type archiveBatches struct {
	archiveFilePath string
	batches         []influx.BatchPoints
}

// Errors that may occur when attempting to write to InfluxDB which are considered temporary problems,
// such as timeouts due to workload, network connectivity, etc. Errors that are fundamental, such as
// a malformed archive file, are not included here.
var retryableErrors []error = []error{
	influx.ErrInfluxTimeout, // Influx is bogged down and internally could not handle write before configured timeout, so returned HTTP error
	influx.ErrConnIssue,     // issue with network connectivity. could be too much network traffic, influx not running, network not configured properly, etc.
}

// Allocates a new db writer stage to the given destination
func NewInfluxWriter(inputChannel <-chan *decodedDataFileData) *InfluxWriter {
	return &InfluxWriter{
		in:                   inputChannel,
		preparedBatchesQueue: make(chan *archiveBatches),
	}
}

func (writer *InfluxWriter) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	log.MsgDebug("In write thread for influx")

	err := writer.initialize()
	if err != nil {
		return fmt.Errorf("failed to initialize writer: %w", err)
	}

	for i := 0; i < GlobalConfig.NumInfluxPrepareBatchesWorkers; i++ {
		group.Go(func() error { return writer.prepareBatchesUntil(groupContext.Done()) })
	}
	for i := 0; i < GlobalConfig.NumInfluxSendBatchesWorkers; i++ {
		group.Go(func() error { return writer.sendBatchesUntil(groupContext.Done()) })
	}
	return nil
}

func (writer *InfluxWriter) initialize() error {
	// create connection to influxdb
	conn := influx.NewConnector(GlobalConfig.InfluxAddr, time.Minute/2, time.Duration(GlobalConfig.DbHealthCheckDelayS*float64(time.Second)), false)
	writer.influxConn = conn

	err := writer.influxConn.Connect()
	if err != nil {
		return err
	}

	err = writer.influxConn.Ping()
	if err != nil {
		return err
	}
	log.Infof("Connected to influx @ " + writer.influxConn.Address)
	writer.DbUrl = writer.influxConn.Address

	return nil
}

// prepares batches of points for sending to a database until told to stop
func (writer *InfluxWriter) prepareBatchesUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			return nil
		case data := <-writer.in:
			preparedBatches, err := writer.prepareBatches(data)
			if err != nil {
				log.Errorf("Failed to make batches for influx writing, err: %v", err)
				continue
			}
			select {
			case <-done:
				return nil
			case writer.preparedBatchesQueue <- preparedBatches:
			}
		}
	}
}

func (writer *InfluxWriter) prepareBatches(data *decodedDataFileData) (*archiveBatches, error) {
	var batches []influx.BatchPoints
	err := writer.ensureDB(data.db, GlobalConfig.RetentionPolicies, GlobalConfig.ContinuousQueries) // set up database if it isn't already
	if err != nil {
		return nil, fmt.Errorf("failed to setup influx database with error: %w", err)
	}

	// create metadata to accompany archive
	metadata := map[string]interface{}{
		"messages":         len(data.msgBodies),
		"archives":         1,
		"archiveSizeBytes": data.archiveSize,
	}
	sourceTag := path.Base(data.dataSourceId)

	log.Tracef("[influxdb_client make batches call] Beginning to make batches for file %s", data.dataFileName)
	batches, err = writer.influxConn.MakeBatches(data.db, data.measurement, sourceTag, data.msgTimestamps, data.msgBodies, metadata) // write batch points
	log.Tracef("[influxdb_client make batches call] Returned from making batches for file %s", data.dataFileName)
	if err != nil {
		return nil, fmt.Errorf("failed to make influx batches, err: %w", err)
	}
	return &archiveBatches{archiveFilePath: data.dataFileName, batches: batches}, nil
}

// sends batches to a database until told to stop
func (writer *InfluxWriter) sendBatchesUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			return nil
		case preparedBatches := <-writer.preparedBatchesQueue:
			archiveFilePath := preparedBatches.archiveFilePath

			// initiate write sequence
			log.Tracef("[influxdb_client write call] Beginning to write to database for file %s", archiveFilePath)
			err := writer.sendArchiveBatches(preparedBatches.batches)
			log.Tracef("[influxdb_client write call] Returned from write to database for file %s", archiveFilePath)
			if err == nil {
				//happy path
				log.Debugf("Successfully written to %s for archive %s", "influx", archiveFilePath)

				atomic.AddUint64(&writer.WriteCnt, 1)
			} else {
				log.Errorf("Error writing to database for file %s with error: %v. Moving it to garbage folder.", archiveFilePath, err)
				atomic.AddUint64(&writer.FailCnt, 1)

				// if we fail to ping the database, terminate the writer
				err = writer.influxConn.Ping()
				if err != nil {
					return fmt.Errorf("influx writer batch sender failed to ping database after failed send: %w", err)
				}
			}
		}
	}
}

// Write a set of batches, all representing a single archive, to InfluxDB.
// If one batch fails to be written, all the rest of the batches are aborted.
func (writer *InfluxWriter) sendArchiveBatches(batches []influx.BatchPoints) error {
	for i, batch := range batches {
		err := writer.sendBatch(batch)
		if err != nil {
			return fmt.Errorf("aborting send of batches due to batch write error for batch %d: %w", i, err)
		}
	}
	return nil
}

// Attempts to write a single batch to InfluxDB. If InfluxDB returns an error that has been
// designated as retryable, keep trying to write the batch. Otherwise, return the error.
func (writer *InfluxWriter) sendBatch(batch influx.BatchPoints) error {
	retries := 0
	for {
		// write the point to InfluxDB and return if successful
		err := writer.influxConn.WriteBatch(batch)
		if err == nil {
			if retries > 0 {
				log.Infof("Batch write to Influx succeeded after %d retries", retries)
			}
			return nil
		}
		// if failure was due to a temporary issue, such as Influx timeout due to heavy traffic, try again
		errIsRetryable := false
		for _, retryableErr := range retryableErrors {
			if errors.Is(err, retryableErr) {
				errIsRetryable = true
				retries++
				log.Errorf("Error trying to write batch to Influx: %v. Error is considered retryable, so will re-attempt to write batch.", err)
				break
			}
		}
		// if failure is unlisted as being a temporary issue, stop trying to write the batch and return an error
		if !errIsRetryable {
			return err
		}
	}
}

// Ensures that the database and associated setup (retention policies and continuous queries) we want to write to
// exists in the destination server
func (writer *InfluxWriter) ensureDB(dbName string, rpConfigs []RetentionPolicyConfig, cqConfigs []ContinuousQueryConfig) error {
	if _, exists := writer.knownDbs.Load(dbName); exists {
		return nil
	}
	log.Debugf("trying to create database: %s", dbName)

	// create influx retention policies based on config
	retentionPolicies := []influx.RetentionPolicy{}
	for _, rp := range rpConfigs {
		retentionPolicies = append(retentionPolicies, influx.RetentionPolicy{
			Name:     rp.Name,
			Default:  rp.Default,
			Duration: rp.Duration,
		})
	}

	err := writer.influxConn.CreateDatabase(dbName, retentionPolicies)
	if err != nil {
		return fmt.Errorf("failed to create database in Influx with error: %w", err)
	}

	// create continuous queries based on config, with Influx identifiers quoted appropriately
	for _, cq := range cqConfigs {
		into := cq.Into
		if into == "" {
			// store result in the same database if no destination database is configured
			into = dbName
		}
		influxCQ := influx.ContQuery{
			Name:        cq.Name,
			Db:          dbName, // get data from this database
			Resample:    cq.Resample,
			Select:      cq.Select,
			Into:        into,
			Rp:          cq.RP,
			Measurement: cq.Measurement,
			From:        cq.From,
			Groupby:     cq.GroupBy,
		}
		query, err := writer.influxConn.RunContinuousQuery(influxCQ)
		if err != nil {
			return fmt.Errorf("failed to create continuous query %s in Influx with error: %w", query, err)
		}
	}

	writer.knownDbs.Store(dbName, true)

	return nil
}

// Disconnects the writer's connection to the database
func (writer *InfluxWriter) disconnect() error {
	err := writer.influxConn.Disconnect()
	if err != nil {
		return fmt.Errorf("failed to disconnect from influxdb with error: %w", err)
	}
	return nil
}
