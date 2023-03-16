package main

import (
	"context"
	"fmt"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	mongo "github.com/flexgen-power/mongodb_client"
	"golang.org/x/sync/errgroup"
)

type mongoWriter struct {
	in    <-chan *archiveData
	dbUrl string // Address of the mongo server

	writeCnt    int
	failCnt     int
	curFileName string

	mongoConn mongo.MongoConnector
}

// Allocates a new db writer stage to the given destination
func newMongoWriter(inputChannel <-chan *archiveData) *mongoWriter {
	return &mongoWriter{
		in: inputChannel,
	}
}

func (writer *mongoWriter) run(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	log.Debugf("In write thread- %s", "mongo")

	err := writer.initialize()
	if err != nil {
		return fmt.Errorf("failed to initialize writer: %w", err)
	}

	group.Go(func() error { return writer.sendDataUntil(groupContext.Done()) })
	return nil
}

func (writer *mongoWriter) initialize() error {
	// create connection to mongodb
	var address string
	switch config.MongoAddr {
	case "":
		address = "localhost:27017"
	default:
		address = config.MongoAddr
	}

	conn := mongo.NewConnector(address, time.Duration(config.DbHealthCheckDelayS), time.Minute/2)
	writer.mongoConn = conn

	err := writer.mongoConn.Connect()
	if err != nil {
		return err
	}

	err = writer.mongoConn.Ping()
	if err != nil {
		return err
	}
	log.Infof("Connected to mongo @ " + writer.mongoConn.Address)
	writer.dbUrl = writer.mongoConn.Address

	return nil
}

// sends batches to a database until told to stop
func (writer *mongoWriter) sendDataUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			err := writer.disconnect()
			if err != nil {
				log.Errorf("Writer failed to disconnect at program shutdown with error: %v", err)
			}
			return err
		case data := <-writer.in:
			writer.curFileName = data.archiveFilePath

			err := writer.sendData(data) // initiate write sequence
			if err == nil {
				//happy path
				log.Debugf("Successfully written to %s for archive %s", "mongo", data.archiveFilePath)

				err := removeArchive(data.archiveFilePath, false, "")
				if err != nil {
					log.Errorf("Unable to remove archive %s", data.archiveFilePath)
				}
				writer.writeCnt++
				log.Debugf("[%s writer] Removed successfully written file %s", "mongo", data.archiveFilePath)
			} else {
				log.Errorf("Error writing to database for file %s with error: %v", data.archiveFilePath, err)
				writer.failCnt++

				log.Warnf("Write failed for archive %s, removing it", data.archiveFilePath)
				err := removeArchive(data.archiveFilePath, true, config.FailedWritePath)
				if err != nil {
					log.Errorf("Unable to remove failed write archive %s, continuing without deleting, err: %v", data.archiveFilePath, err)
				}
			}
		}
	}
}

func (writer *mongoWriter) sendData(data *archiveData) error {
	log.Tracef("[mongodb_client write call] Beginning to write to database for file %s", data.archiveFilePath)
	err := writer.mongoConn.Write(data.db, data.measurement, data.points.MsgBodies) // attempt mongo write
	log.Tracef("[mongodb_client write call] Returned from write to database for file %s", data.archiveFilePath)
	return err
}

// Disconnects the writer's connection to the database
func (writer *mongoWriter) disconnect() error {
	err := writer.mongoConn.Disconnect()
	if err != nil {
		return fmt.Errorf("failed to disconnect from mongodb with error: %w", err)
	}
	return nil
}
