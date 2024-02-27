package archive_cocoon

import (
	"context"
	"os"
	"path/filepath"
	"sync/atomic"

	"github.com/flexgen-power/analyze_lambda/decompression/pkg/tools"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

type ArchiveConverter struct {
	in <-chan string

	FailCt uint64 // Modifications must be atomic
}

// Allocates a new archive validator stage and the needed output channels based on the given destination names
func NewArchiveConverter(inputChannel <-chan string) *ArchiveConverter {
	converter := ArchiveConverter{
		in: inputChannel,
	}
	return &converter
}

// Decompresses and decodes archives to convert them and determine which writer will need them,
// writers is a map from archive's decoded destination to a channel to the destination writer
func (converter *ArchiveConverter) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	converter.FailCt = 0

	for n := 0; n < GlobalConfig.NumConvertWorkers; n++ {
		group.Go(func() error { return converter.convertUntil(groupContext.Done()) })
	}
	return nil
}

// Loop that converts incoming files
func (converter *ArchiveConverter) convertUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			return nil
		case archiveFilePath := <-converter.in:
			log.Debugf("[convert] received file %s from channel", archiveFilePath)
			log.Debugf("[convert] decoding archive %s", archiveFilePath)

			// read bytes from file
			by, err := os.ReadFile(archiveFilePath)
			if err != nil {
				log.Errorf("could not open file %s: %v\n", archiveFilePath, err)
				converter.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// decodes the FIMS data using the codec and validates metadata
			data, err := tools.Decode(by)
			if err != nil {
				log.Errorf("failed to decode and validate file %s: %v\n", archiveFilePath, err)
				converter.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// generates output name for parquet file
			// NOTE: do we need this? or should i just use the same name? this is used for parquet files in AWS/S3, so it may be good to be consistent. thoughts?
			pqtname, err := tools.GenerateOutputName(GlobalConfig.Client, GlobalConfig.Site, data.AdditionalData["measurement"], archiveFilePath)
			if err != nil {
				log.Errorf("could not generate parquet name for %s: %v\n", archiveFilePath, err)
				converter.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// create the schema and keylist (col names) for the parquet file
			schema, keylist, err := tools.CreateSchema(data.MsgBodies)
			if err != nil {
				log.Errorf("could not generate schema/keylist for %s: %v\n", archiveFilePath, err)
				converter.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// create writer
			// NOTE: there has to be a separate writer for each file, which is why we don't have a third stage for writers (like DTS), in case you're wondering
			writer, file, err := tools.CreateParquetLocalWriter(filepath.Join(GlobalConfig.OutputPath, pqtname), schema)
			if err != nil {
				log.Errorf("could not create local parquet writer for %s: %v\n", archiveFilePath, err)
				converter.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// write data to the file
			// NOTE: the client and site are taken in here and added as columnar data (as well as the database, aka device_origin) in the pqt file
			//		 this is done in AWS/S3 for DBX reasons, but i would be open to making this optional if we feel it is detrimental. otherwise, its quite harmless really.
			err = tools.Write(writer, file, schema, keylist, GlobalConfig.Client, GlobalConfig.Site, data.AdditionalData["database"], data.MsgBodies, data.MsgTimestamps)
			if err != nil {
				log.Errorf("could not write to file %s: %v\n", pqtname, err)
				converter.cleanUpInvalidArchive(archiveFilePath)
				continue
			} else {
				err := removeArchive(archiveFilePath, false, GlobalConfig.FailedConvertPath)
				if err != nil {
					log.Errorf("[convert] Continuing without forwarding archive: could not clean archive %s: %v", archiveFilePath, err)
				} else {
					log.Debugf("[convert] Cleaned archive %s", archiveFilePath)
				}
			}
		}
	}
}

// Remove an archive that was found invalid along with its associated data
func (converter *ArchiveConverter) cleanUpInvalidArchive(archiveName string) {
	err := removeArchive(archiveName, true, GlobalConfig.FailedConvertPath)
	if err != nil {
		log.Errorf("[convert] Continuing without removing invalid archive: could not clean invalid archive %s: %v", archiveName, err)
	} else {
		log.Debugf("[convert] Cleaned failed archive %s", archiveName)
	}

	atomic.AddUint64(&converter.FailCt, 1)
}
