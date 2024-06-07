package ftd

import (
	"archive/tar"
	"compress/gzip"
	"context"
	"fmt"
	"os"
	"path/filepath"
	"time"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Takes in-memory archive data files and batches them into a single compressed file
type ArchiveBatcher struct {
	rootCfg  Config
	laneCfg  LaneConfig
	laneName string
	in       <-chan []dataFile
}

func NewArchiveBatcher(rootCfg Config, laneCfg LaneConfig, lane string, inputChannel <-chan []dataFile) *ArchiveBatcher {
	return &ArchiveBatcher{
		rootCfg:  rootCfg,
		laneCfg:  laneCfg,
		laneName: lane,
		in:       inputChannel,
	}
}

func (batcher *ArchiveBatcher) Start(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	group.Go(func() error { return batcher.batchUntil(groupContext.Done()) })
	return nil
}

// Loop for batching incoming data files into a single file per batch
func (batcher *ArchiveBatcher) batchUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			goto termination
		case dataFiles, ok := <-batcher.in:
			// handle channel close signal
			if !ok {
				goto termination
			}
			err := batcher.writeBatchLocal(dataFiles)
			if err != nil {
				log.Errorf("Archive batcher %s failed to write archive batch: %v", batcher.laneName, err)
			}
		}
	}
termination:
	log.Infof("Archive batcher %s entered termination block. Creating batches from remaining data files.", batcher.laneName)
	// graceful termination
	for dataFiles := range batcher.in {
		err := batcher.writeBatchLocal(dataFiles)
		if err != nil {
			log.Errorf("Archive batcher %s failed to write archive batch: %v", batcher.laneName, err)
		}
	}
	log.Infof("Archive batcher %s terminating. All remaining data files were batched and written.", batcher.laneName)
	return nil
}

// Write an archive batch file to disk containing the given data files
func (batcher *ArchiveBatcher) writeBatchLocal(dataFiles []dataFile) error {
	// construct the full path of the batched archive to be produced
	creationTime := time.Now()
	creationEpoch := uint64(creationTime.UnixMicro())
	// extension used is .batchpqtgz for compatibility with other modules and to better indicate the archive contents
	archiveName := fmt.Sprintf("%s__%s__%s__%s__%d.batchpqtgz",
		batcher.rootCfg.ClientName,
		batcher.rootCfg.SiteName,
		batcher.laneCfg.DbName,
		batcher.laneName,
		creationEpoch)
	archiveFilePath := filepath.Join(batcher.laneCfg.ArchivePath, archiveName)

	// instantiate the archive batch file
	archiveFile, err := os.Create(archiveFilePath)
	if err != nil {
		return fmt.Errorf("failed to create tar file: %w", err)
	}
	defer archiveFile.Close()

	// create the .tar.gz file writer
	gzipWriter := gzip.NewWriter(archiveFile)
	defer gzipWriter.Close()
	tarWriter := tar.NewWriter(gzipWriter)
	defer tarWriter.Close()

	// write data files to tarfile
	for _, df := range dataFiles {
		// write header for data file
		err = tarWriter.WriteHeader(&tar.Header{
			Name:    df.name,
			Size:    int64(len(df.data)),
			Mode:    int64(os.ModePerm), // full permissions
			ModTime: creationTime,
		})
		if err != nil {
			return fmt.Errorf("failed to write header for data file: %w", err)
		}

		// write data file bytes
		_, err = tarWriter.Write(df.data)
		if err != nil {
			return fmt.Errorf("failed to write data to data file: %w", err)
		}
	}
	tarWriter.Flush()
	return nil
}
