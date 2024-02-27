package archive_cocoon

import (
	"context"
	"fmt"

	buffman "github.com/flexgen-power/go_flexgen/buffman"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// The pipeline stages used to process data: archives -> monitor -> converter -> parquet
type Pipeline struct {
	// Monitors input path for archives to process
	monitor *ArchiveMonitor

	// Carries the filepaths of archives noticed by the monitor to be decoded by the validator
	archiveQueue buffman.BufferManager

	// Decodes and validates archive data
	Converter *ArchiveConverter
}

// Runs a complete pipeline for decoding archives and writing them to parquet
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
	p.archiveQueue = buffman.New(buffman.Stack, GlobalConfig.NumConvertWorkers, p.monitor.Out)

	// start thread for converter
	log.MsgDebug("Starting converter")
	p.Converter = NewArchiveConverter(p.archiveQueue.Out())
	err = p.Converter.Start(group, groupContext)
	if err != nil {
		return fmt.Errorf("failed to start validator stage: %w", err)
	}

	// block until a fatal error
	log.Infof("All child routines started. archive_cocoon main routine now running indefinitely.")
	err = group.Wait()
	if err != nil {
		return fmt.Errorf("pipeline encountered a fatal error: %w", err)
	}
	return nil
}
