package dts

import (
	"context"
	"fmt"

	buffman "github.com/flexgen-power/go_flexgen/buffman"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// The pipeline stages used to process data: archives -> monitor -> validator -> writers -> db_clients
type Pipeline struct {
	// Monitors input path for archives to process
	monitor *ArchiveMonitor

	// Carries the filepaths of archives noticed by the monitor to be decoded by the validator
	archiveQueue buffman.BufferManager

	// Decodes and validates archive data
	Validator *ArchiveValidator

	// Writes decoded data to InfluxDB
	WriterToInflux *InfluxWriter
	// Writes decoded data to MongoDB
	WriterToMongo *MongoWriter
}

// The destination DBs we want to write to
var destinations = []string{"influx", "mongo"}

// Runs a complete pipeline for decoding archives and writing them to databases
// New routines are started using the given error group and context.
// Does not return until all stages stop or the pipeline encounters a fatal error.
func (p *Pipeline) Run(group *errgroup.Group, groupContext context.Context) error {
	// start thread for monitor
	log.MsgDebug("Starting monitor")
	p.monitor = NewArchiveMonitor(GlobalConfig.InputPath)
	err := p.monitor.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start monitor stage: %w", err)
	}

	// open buffer manager for validator work
	p.archiveQueue = buffman.New(buffman.Stack, GlobalConfig.NumValidateWorkers, p.monitor.Out)

	// start thread for validator
	log.MsgDebug("Starting validator")
	p.Validator = NewArchiveValidator(p.archiveQueue.Out(), destinations)
	err = p.Validator.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start validator stage: %w", err)
	}

	// start thread for writers
	log.MsgDebug("Starting writers")
	log.MsgDebug("Starting writer to influx")
	p.WriterToInflux = NewInfluxWriter(p.Validator.Outs["influx"])
	err = p.WriterToInflux.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start writer to %s stage: %w", "influx", err)
	}
	p.WriterToMongo = NewMongoWriter(p.Validator.Outs["mongo"])
	err = p.WriterToMongo.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start writer to %s stage: %w", "mongo", err)
	}

	// block until a fatal error
	log.Infof("All child routines started. dts main routine now running indefinitely.")
	err = group.Wait()
	if err != nil {
		return fmt.Errorf("pipeline encountered a fatal error: %w", err)
	}
	return nil
}
