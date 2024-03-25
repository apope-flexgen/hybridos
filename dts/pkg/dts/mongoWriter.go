package dts

import (
	"context"
	"fmt"
	"time"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	mongo "github.com/flexgen-power/hybridos/mongodb_client"
	"golang.org/x/sync/errgroup"
)

type MongoWriter struct {
	in    <-chan *archiveData
	DbUrl string // Address of the mongo server

	WriteCnt    int
	FailCnt     int
	curFileName string

	mongoConn mongo.MongoConnector
}

// Allocates a new db writer stage to the given destination
func NewMongoWriter(inputChannel <-chan *archiveData) *MongoWriter {
	return &MongoWriter{
		in: inputChannel,
	}
}

func (writer *MongoWriter) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	log.Debugf("In write thread- %s", "mongo")

	err := writer.initialize()
	if err != nil {
		return fmt.Errorf("failed to initialize writer: %w", err)
	}

	group.Go(func() error { return writer.sendDataUntil(groupContext.Done()) })
	return nil
}

func (writer *MongoWriter) initialize() error {
	// create connection to mongodb
	var address string
	switch GlobalConfig.MongoAddr {
	case "":
		address = "localhost:27017"
	default:
		address = GlobalConfig.MongoAddr
	}

	conn := mongo.NewConnector(address, time.Duration(GlobalConfig.DbHealthCheckDelayS), time.Minute/2)
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
	writer.DbUrl = writer.mongoConn.Address

	return nil
}

// sends batches to a database until told to stop
func (writer *MongoWriter) sendDataUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			return nil
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
				writer.WriteCnt++
				log.Debugf("[%s writer] Removed successfully written file %s", "mongo", data.archiveFilePath)
			} else {
				log.Errorf("Error writing to database for file %s with error: %v", data.archiveFilePath, err)
				writer.FailCnt++

				log.Warnf("Write failed for archive %s, removing it", data.archiveFilePath)
				err := removeArchive(data.archiveFilePath, true, GlobalConfig.FailedWritePath)
				if err != nil {
					log.Errorf("Unable to remove failed write archive %s, continuing without deleting, err: %v", data.archiveFilePath, err)
				}

				// if we fail to ping the database, terminate the writer
				err = writer.mongoConn.Ping()
				if err != nil {
					return fmt.Errorf("mongo writer data sender failed to ping database after failed send: %w", err)
				}
			}
		}
	}
}

func (writer *MongoWriter) sendData(data *archiveData) error {
	log.Tracef("[mongodb_client write call] Beginning to write to database for file %s", data.archiveFilePath)
	err := writer.mongoConn.Write(data.db, data.measurement, data.points.MsgBodies) // attempt mongo write
	log.Tracef("[mongodb_client write call] Returned from write to database for file %s", data.archiveFilePath)
	return err
}

// Disconnects the writer's connection to the database
func (writer *MongoWriter) disconnect() error {
	err := writer.mongoConn.Disconnect()
	if err != nil {
		return fmt.Errorf("failed to disconnect from mongodb with error: %w", err)
	}
	return nil
}
