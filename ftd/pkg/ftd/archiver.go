package ftd

import (
	"context"
	"fmt"
	"os"

	"github.com/flexgen-power/hybridos/fims_codec"
	"github.com/flexgen-power/hybridos/go_flexgen/fileops"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Takes a batch of encoders and archives each encoder into a .tar.gz file.
type MsgArchiver struct {
	laneCfg  LaneConfig
	laneName string
	in       <-chan []*fims_codec.Encoder
}

// Allocates memory for a new msgArchiver.
func NewMsgArchiver(cfg LaneConfig, lane string, inputChannel <-chan []*fims_codec.Encoder) *MsgArchiver {
	return &MsgArchiver{
		laneCfg:  cfg,
		laneName: lane,
		in:       inputChannel,
	}
}

// Launch a worker pool of archivers to archive encoder batches as they are passed from the collator.
func (archiver *MsgArchiver) Start(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	// check if output directory exists. If not, try creating one
	if !fileops.Exists(archiver.laneCfg.ArchivePath) {
		log.Infof("%s doesnt exist. Creating directory", archiver.laneCfg.ArchivePath)
		err := os.MkdirAll(archiver.laneCfg.ArchivePath, 0755)
		if err != nil {
			return fmt.Errorf("failed to make directory for output archives: %w", err)
		}
	}
	log.Infof("archive created every %d seconds", archiver.laneCfg.ArchivePeriod)

	for i := 0; i < archiver.laneCfg.NumArchiveWorkers; i++ {
		group.Go(func() error { return archiver.archiveUntil(groupContext.Done()) })
	}
	return nil
}

// Loop that archives incoming encoded data.
func (archiver *MsgArchiver) archiveUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			goto termination
		case encoderBatch, ok := <-archiver.in:
			// handle channel close signal
			if !ok {
				goto termination
			}
			// archive all data in the batch
			for _, encoder := range encoderBatch {
				archiver.writeArchiveData(encoder)
			}
		}
	}
termination:
	// archive all remaining batches
	log.Infof("Archiver %s entered termination block. Archiving all remaining batches.", archiver.laneName)
	for encoderBatch := range archiver.in {
		for _, encoder := range encoderBatch {
			archiver.writeArchiveData(encoder)
		}
	}
	log.Infof("Archiver %s terminating. All remaining batches were archived.", archiver.laneName)
	return nil
}

// Archives a single encoder into a .tar.gz file.
func (archiver *MsgArchiver) writeArchiveData(encoder *fims_codec.Encoder) {
	// update site state to be primary/secondary which gets passed through metadata.txt in .tar.gz file
	if ControllerStateIsPrimary() {
		encoder.AdditionalData["site_state"] = "primary"
	} else {
		encoder.AdditionalData["site_state"] = "secondary"
	}

	encoderMeasurement, exist := encoder.AdditionalData["measurement"]
	if !exist {
		log.Errorf("Measurement does not exist in Additional Data for encoder %s.", encoder.Uri)
	}

	encoderMethod, exist := encoder.AdditionalData["method"]
	if !exist {
		log.Errorf("Method does not exist in Additional Data for encoder %s.", encoder.Uri)
	}

	// create an identifying name prefix for the archive
	archivePrefix := archiver.laneName + "_" + encoderMethod + "_" + archiver.laneCfg.DbName + "_" + encoderMeasurement
	_, _, err := encoder.CreateArchive(archiver.laneCfg.ArchivePath, archivePrefix)
	if err != nil {
		log.Errorf("archive creation failed for URI %s with error: %s", encoder.Uri, err.Error())
	}
}
