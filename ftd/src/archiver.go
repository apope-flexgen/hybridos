package main

import (
	"context"
	"sync/atomic"

	"github.com/flexgen-power/fims_codec"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Takes a batch of encoders and archives each encoder into a .tar.gz file.
type msgArchiver struct {
	in <-chan []*fims_codec.Encoder
}

// Allocates memory for a new msgArchiver.
func newMsgArchiver(inputChannel <-chan []*fims_codec.Encoder) *msgArchiver {
	return &msgArchiver{
		in: inputChannel,
	}
}

// Launch a worker pool of archivers to archive encoder batches as they are passed from the collator.
func (archiver *msgArchiver) run(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	for i := 0; i < config.NumArchiveWorkers; i++ {
		group.Go(func() error { return archiver.archiveUntil(groupContext.Done()) })
	}
	return nil
}

// Loop that archives incoming encoded data.
func (archiver *msgArchiver) archiveUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			return nil
		case encoderBatch := <-archiver.in:
			for _, encoder := range encoderBatch {
				writeArchiveData(encoder)
			}
		}
	}
}

// Archives a single encoder into a .tar.gz file.
func writeArchiveData(encoder *fims_codec.Encoder) {
	// update site state to be primary/secondary which gets passed through metadata.txt in .tar.gz file
	if atomic.LoadUint32(&controllerState) == 0 {
		encoder.AdditionalData["site_state"] = "primary"
	} else {
		encoder.AdditionalData["site_state"] = "secondary"
	}

	// create suffix with name <database>_<measurement>
	encoderMeasurement, exist := encoder.AdditionalData["measurement"]
	if !exist {
		log.Errorf("Measurement does not exist in Additional Data for encoder %s.", encoder.Uri)
	}

	archivePrefix := config.DbName + "_" + encoderMeasurement
	_, _, err := encoder.CreateArchive(config.ArchivePath, archivePrefix)
	if err != nil {
		log.Errorf("archive creation failed for URI %s with error: %s", encoder.Uri, err.Error())
	}
}
