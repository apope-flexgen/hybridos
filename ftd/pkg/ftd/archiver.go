package ftd

import (
	"context"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/flexgen-power/hybridos/go_flexgen/fileops"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Takes a batch of encoders and archives each encoder into a data file.
type MsgArchiver struct {
	laneCfg  LaneConfig
	laneName string
	in       <-chan []*Encoder
	Out      chan []dataFile
}

// Struct for an in-memory data file
type dataFile struct {
	name string
	data []byte
}

// Allocates memory for a new msgArchiver.
func NewMsgArchiver(cfg LaneConfig, lane string, inputChannel <-chan []*Encoder) *MsgArchiver {
	return &MsgArchiver{
		laneCfg:  cfg,
		laneName: lane,
		in:       inputChannel,
		Out:      make(chan []dataFile),
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
	defer close(archiver.Out)
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
			archiveBatch := []dataFile{}
			for _, encoder := range encoderBatch {
				data, err := archiver.writeArchiveData(encoder)
				if err != nil {
					log.Errorf("Archiver %s failed to write data: %v", archiver.laneName, err)
				} else if data != nil {
					archiveBatch = append(archiveBatch, *data)
				}
			}
			if len(archiveBatch) != 0 {
				archiver.Out <- archiveBatch
			}
		}
	}
termination:
	// archive all remaining batches
	log.Infof("Archiver %s entered termination block. Archiving all remaining batches.", archiver.laneName)
	for encoderBatch := range archiver.in {
		archiveBatch := []dataFile{}
		for _, encoder := range encoderBatch {
			data, err := archiver.writeArchiveData(encoder)
			if err != nil {
				log.Errorf("Archiver %s failed to write data: %v", archiver.laneName, err)
			} else if data != nil {
				archiveBatch = append(archiveBatch, *data)
			}
		}
		if len(archiveBatch) != 0 {
			archiver.Out <- archiveBatch
		}
	}
	log.Infof("Archiver %s terminating. All remaining batches were archived.", archiver.laneName)
	return nil
}

// Archives a single encoder into a data file.
func (archiver *MsgArchiver) writeArchiveData(encoder *Encoder) (*dataFile, error) {
	// update site state to be primary/secondary which gets passed through metadata in the data file
	if ControllerStateIsPrimary() {
		encoder.AddMetadata("site_state", "primary")
	} else {
		encoder.AddMetadata("site_state", "secondary")
	}

	encoderMeasurement, exist := encoder.GetMetadata("measurement")
	if !exist {
		return nil, fmt.Errorf("measurement does not exist in Additional Data for encoder %s", encoder.DataSourceId)
	}

	encoderMethod, exist := encoder.GetMetadata("method")
	if !exist {
		return nil, fmt.Errorf("method does not exist in Additional Data for encoder %s", encoder.DataSourceId)
	}

	if archiver.laneCfg.BatchParquetGZ {
		// write the file to memory if we're making a batched archive
		data, err := encoder.CreateArchiveMem()
		if err != nil {
			return nil, fmt.Errorf("archive creation failed for URI %s with error: %w", encoder.DataSourceId, err)
		}
		creationEpoch := time.Now().UnixMicro()
		return &dataFile{
			name: fmt.Sprintf("%s__%s__%s__%d.parquet.gz",
				encoderMethod,
				encoderMeasurement,
				cleanDataSourceId(encoder.DataSourceId),
				creationEpoch),
			data: data,
		}, nil
	} else {
		// else write the file directly to disk
		archivePrefix := archiver.laneName + "__" + encoderMethod + "__" + archiver.laneCfg.DbName + "__" + encoderMeasurement
		err := encoder.CreateArchiveLocal(archiver.laneCfg.ArchivePath, archivePrefix)
		if err != nil {
			return nil, fmt.Errorf("archive creation failed for URI %s with error: %w", encoder.DataSourceId, err)
		}
		return nil, nil
	}
}

// Cleans the data source id string for use in a filename by
// replacing '/' with '-' and replacing non-overlapping instances of '__' with '-'
func cleanDataSourceId(dataSourceId string) string {
	dataSourceId = strings.TrimPrefix(dataSourceId, "/")
	dataSourceId = strings.Replace(dataSourceId, "/", "-", -1)
	return strings.Replace(dataSourceId, "__", "-", -1)
}
