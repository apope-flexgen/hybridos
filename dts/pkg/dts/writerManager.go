package dts

import (
	"context"
	"errors"
	"time"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Manages database writers and routes data to the correct writer
type WriterManager struct {
	influxIn       <-chan *archiveData // channel for data routed to influx
	mongoIn        <-chan *archiveData // channel for data routed to mongo
	writerToInflux *InfluxWriter
	writerToMongo  *MongoWriter

	influxWriterInput chan *archiveData
	mongoWriterInput  chan *archiveData
}

// Manages the runtime properties of a writer
type writerRunner struct {
	writerType           string
	writer               writerStage
	group                *errgroup.Group
	groupContext         context.Context
	contextDone          <-chan struct{} // channel which is either the context canceller or nil if writer is down
	isDown               bool
	restartTimer         *time.Timer
	restartTimerChan     <-chan time.Time // channel which is either the restart timer channel or nil if timer is inactive
	writerInput          chan *archiveData
	asyncStartResultChan chan error // channel which either indicates the result of an asynchronous start call or nil if there is no such call
}

// interface abstracting writer stages
type writerStage interface {
	Start(group *errgroup.Group, groupContext context.Context) (startUpError error)
	disconnect() error
}

// Allocates a new writer manager stage
func NewWriterManager(influxInput <-chan *archiveData, mongoInput <-chan *archiveData) *WriterManager {
	manager := WriterManager{
		influxIn: influxInput,
		mongoIn:  mongoInput,
	}
	manager.influxWriterInput = make(chan *archiveData)
	manager.writerToInflux = NewInfluxWriter(manager.influxWriterInput)
	manager.mongoWriterInput = make(chan *archiveData)
	manager.writerToMongo = NewMongoWriter(manager.mongoWriterInput)

	return &manager
}

// Starts the writer manager stage and its writer stages.
func (manager *WriterManager) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	log.MsgDebug("In writer manager thread start")
	group.Go(func() error { return manager.manageWritersUnderContext(groupContext) })
	return nil
}

// Manages child contexts within which a writer is allowed to potentially fail without leading to a shutdown.
// If both writers fail, then the manager also fails.
func (manager *WriterManager) manageWritersUnderContext(parentContext context.Context) error {
	influxWritersRunner := writerRunner{
		writerType:  "influx",
		writer:      manager.writerToInflux,
		isDown:      false,
		writerInput: manager.influxWriterInput,
	}
	mongoWritersRunner := writerRunner{
		writerType:  "mongo",
		writer:      manager.writerToMongo,
		isDown:      false,
		writerInput: manager.mongoWriterInput,
	}

	// try to startup both writers
	influxWritersRunner.start(parentContext)
	mongoWritersRunner.start(parentContext)

	log.MsgInfo("Writer manager routine now running indefinitely.")

	// loop checking writer availability
	for {
		// check if both writers are down and return if so
		if influxWritersRunner.isDown && mongoWritersRunner.isDown {
			return errors.New("writer manager closed after both child writers failed")
		}

		// start timers for restarting writers that are down if the timer isn't already started and they aren't in the process of starting
		if influxWritersRunner.isDown && influxWritersRunner.restartTimer == nil && influxWritersRunner.asyncStartResultChan == nil {
			influxWritersRunner.restartTimer = time.NewTimer(time.Duration(GlobalConfig.RetryConnectPeriodSeconds * float64(time.Second)))
			influxWritersRunner.restartTimerChan = influxWritersRunner.restartTimer.C
		}
		if mongoWritersRunner.isDown && mongoWritersRunner.restartTimer == nil && mongoWritersRunner.asyncStartResultChan == nil {
			mongoWritersRunner.restartTimer = time.NewTimer(time.Duration(GlobalConfig.RetryConnectPeriodSeconds * float64(time.Second)))
			mongoWritersRunner.restartTimerChan = mongoWritersRunner.restartTimer.C
		}

		select {
		// block on parent context cancellation
		case <-parentContext.Done():
			goto termination

		// block on incoming data
		case data := <-manager.influxIn:
			parentCancelled := influxWritersRunner.handleIncomingData(data, parentContext)
			if parentCancelled {
				goto termination
			}
		case data := <-manager.mongoIn:
			parentCancelled := mongoWritersRunner.handleIncomingData(data, parentContext)
			if parentCancelled {
				goto termination
			}

		// block on writer stages going down
		case <-influxWritersRunner.contextDone:
			influxWritersRunner.handleContextCancel()
		case <-mongoWritersRunner.contextDone:
			mongoWritersRunner.handleContextCancel()

		// block on restart timer ticks
		case <-influxWritersRunner.restartTimerChan:
			influxWritersRunner.restartTimer = nil
			influxWritersRunner.restartTimerChan = nil
			influxWritersRunner.asyncStart(parentContext)
		case <-mongoWritersRunner.restartTimerChan:
			mongoWritersRunner.restartTimer = nil
			mongoWritersRunner.restartTimerChan = nil
			mongoWritersRunner.asyncStart(parentContext)

		// block on result of async restart
		case err := <-influxWritersRunner.asyncStartResultChan:
			influxWritersRunner.handleAsyncStartResult(err)
			influxWritersRunner.asyncStartResultChan = nil
		case err := <-mongoWritersRunner.asyncStartResultChan:
			mongoWritersRunner.handleAsyncStartResult(err)
			mongoWritersRunner.asyncStartResultChan = nil
		}
	}

termination:
	log.MsgInfo("Writer manager stage terminating")
	return nil
}

// Try to start writer running with a new context under the given parent context
func (runner *writerRunner) start(parentContext context.Context) {
	runner.group, runner.groupContext = errgroup.WithContext(parentContext)
	err := runner.writer.Start(runner.group, runner.groupContext)
	if err != nil {
		runner.isDown = true
		log.Errorf("%s writer stage failed to startup with err: %v", runner.writerType, err)
	} else {
		runner.isDown = false
		runner.contextDone = runner.groupContext.Done()
	}
}

// Try to start the writer asynchronously so that we can do other processing while waiting for startup to finish
func (runner *writerRunner) asyncStart(parentContext context.Context) {
	runner.group, runner.groupContext = errgroup.WithContext(parentContext)
	runner.asyncStartResultChan = make(chan error)
	go func() {
		err := runner.writer.Start(runner.group, runner.groupContext)
		if err != nil {
			runner.asyncStartResultChan <- err
		} else {
			runner.asyncStartResultChan <- nil
		}
	}()
}

// Handles result of a call to asyncStart
func (runner *writerRunner) handleAsyncStartResult(err error) {
	if err != nil {
		runner.isDown = true
		log.Errorf("%s writer stage failed to startup with err: %v", runner.writerType, err)
	} else {
		runner.isDown = false
		runner.contextDone = runner.groupContext.Done()
	}
}

// Handle the writer context cancelling
func (runner *writerRunner) handleContextCancel() {
	runner.isDown = true
	err := runner.group.Wait() // wait for child goroutines to finish terminating
	if err != nil {
		log.Errorf("%v writer stage failed with err: %v", runner.writerType, err)
	}
	err = runner.writer.disconnect()
	if err != nil {
		log.Errorf("Failed disconnect call on %v writer: %v", runner.writerType, err)
	}
	runner.contextDone = nil
}

// Handle incoming data by sending or discarding it and return true if handling the data was cancelled by the parent context
func (runner *writerRunner) handleIncomingData(data *archiveData, parentContext context.Context) (parentCancelled bool) {
	if runner.isDown {
		err := removeArchive(data.archiveFilePath, true, GlobalConfig.FailedWritePath)
		if err != nil {
			log.Errorf("Unable to move archive %s to failure path after mongo being unavailable: %v. Continuing without moving the archive.", data.archiveFilePath, err)
		}
	} else {
		select { // cancellably send data
		case <-parentContext.Done():
			return true
		case <-runner.contextDone:
			err := removeArchive(data.archiveFilePath, true, GlobalConfig.FailedWritePath)
			if err != nil {
				log.Errorf("Unable to move archive %s to failure path after mongo writer being unavailable: %v. Continuing without moving the archive.", data.archiveFilePath, err)
			}
			runner.handleContextCancel()
		case runner.writerInput <- data:
			// deliberately empty case
		}
	}
	return false
}
