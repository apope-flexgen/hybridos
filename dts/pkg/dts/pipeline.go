package dts

import (
	"context"
	"fmt"

	buffman "github.com/flexgen-power/hybridos/go_flexgen/buffman"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// The pipeline stages used to process data: archives -> monitor -> validator -> writers -> db_clients
type Pipeline struct {
	// Monitors input path for archives to process
	monitor *ArchiveMonitor

	// Carries the filepaths of archive files noticed by the monitor to be extracted by the extractor
	archiveQueue buffman.BufferManager

	// Extracts data files from archive files
	extractor *ArchiveExtractor

	// Decodes and validates archive data
	Validator *ArchiveValidator

	// Manages writers which write decoded data to databases
	WriterManagerStage *WriterManager
}

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

	// open buffer manager for extractor work
	p.archiveQueue = buffman.New(buffman.Stack, GlobalConfig.NumValidateWorkers, p.monitor.Out)

	// start extractor stage
	log.MsgDebug("Starting extractor")
	p.extractor = NewArchiveExtractor(p.archiveQueue.Out())
	err = p.extractor.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start extractor stage: %w", err)
	}

	// start thread for validator
	log.MsgDebug("Starting validator")
	p.Validator = NewArchiveValidator(p.extractor.Out)
	err = p.Validator.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start validator stage: %w", err)
	}

	// start writer manager
	log.MsgDebug("Starting writer manager")
	p.WriterManagerStage = NewWriterManager(p.Validator.InfluxOut, p.Validator.MongoOut)
	err = p.WriterManagerStage.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start writer manager stage: %w", err)
	}

	// block until a fatal error
	log.Infof("All child routines started. dts main routine now running indefinitely.")
	err = group.Wait()
	if err != nil {
		return fmt.Errorf("pipeline encountered a fatal error: %w", err)
	}
	return nil
}
