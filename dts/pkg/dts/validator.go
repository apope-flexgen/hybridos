package dts

import (
	"context"
	"fmt"
	"os"
	"strings"
	"sync/atomic"

	"github.com/flexgen-power/hybridos/fims_codec"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/parquet_codec"
	"golang.org/x/sync/errgroup"
)

type ArchiveValidator struct {
	in        <-chan string
	InfluxOut chan *archiveData
	MongoOut  chan *archiveData

	mapDestinationToOut map[string]chan *archiveData // maps destination strings to output channels
	FailCt              uint64                       // Modifications must be atomic
}

type archiveData struct {
	archiveFilePath string
	archiveSize     int64
	db              string
	measurement     string

	dataSourceId   string
	msgTimestamps  []uint64
	msgBodies      []map[string]interface{}
	additionalData map[string]string
}

// Allocates a new archive validator stage and the needed output channels based on the given destination names
func NewArchiveValidator(inputChannel <-chan string) *ArchiveValidator {
	validator := ArchiveValidator{
		in:        inputChannel,
		InfluxOut: make(chan *archiveData),
		MongoOut:  make(chan *archiveData),
	}
	validator.mapDestinationToOut = map[string]chan *archiveData{
		"influx": validator.InfluxOut,
		"mongo":  validator.MongoOut,
	}
	return &validator
}

// Decompresses and decodes archives to validate them and determine which writer will need them,
// writers is a map from archive's decoded destination to a channel to the destination writer
func (validator *ArchiveValidator) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	validator.FailCt = 0

	for n := 0; n < GlobalConfig.NumValidateWorkers; n++ {
		group.Go(func() error { return validator.validateUntil(groupContext.Done()) })
	}
	return nil
}

// Loop that validates incoming files
func (validator *ArchiveValidator) validateUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			goto termination
		case archiveFilePath := <-validator.in:
			log.Debugf("[validate] received file %s from channel", archiveFilePath)
			log.Debugf("[validate] decoding archive %s", archiveFilePath)

			// get size of archive
			info, err := os.Stat(archiveFilePath)
			if err != nil {
				log.Errorf("failed to get info for file %s, err: %v", archiveFilePath, err)
				continue
			}
			archiveSize := info.Size()

			// decode message
			data := archiveData{}
			if strings.HasSuffix(archiveFilePath, ".parquet.gz") {
				decodeResult, err := parquet_codec.DecodeLocalGZippedParquet(archiveFilePath)
				if err != nil {
					log.Errorf("decode failed for file %s, err: %v", archiveFilePath, err)
					validator.cleanUpInvalidArchive(archiveFilePath)
					continue
				}
				data = archiveData{
					archiveFilePath: archiveFilePath,
					archiveSize:     archiveSize,
					db:              decodeResult.Metadata["database"],
					measurement:     decodeResult.Metadata["measurement"],
					dataSourceId:    decodeResult.Metadata["data_source_id"],
					msgTimestamps:   decodeResult.MsgTimestamps,
					msgBodies:       decodeResult.MsgBodies,
					additionalData:  decodeResult.Metadata,
				}
			} else {
				// assume fims_codec encoding if not parquet
				decodeResult, err := fims_codec.Decode(archiveFilePath)
				if err != nil {
					log.Errorf("decode failed for file %s, err: %v", archiveFilePath, err)
					validator.cleanUpInvalidArchive(archiveFilePath)
					continue
				}
				data = archiveData{
					archiveFilePath: archiveFilePath,
					archiveSize:     archiveSize,
					db:              decodeResult.AdditionalData["database"],
					measurement:     decodeResult.AdditionalData["measurement"],
					dataSourceId:    decodeResult.Uri,
					msgTimestamps:   decodeResult.MsgTimestamps,
					msgBodies:       decodeResult.MsgBodies,
					additionalData:  decodeResult.AdditionalData,
				}
			}

			// gather additional info
			err = validator.validateAddInfo(archiveFilePath, data.additionalData)
			if err != nil {
				if err.Error() == "secondary" {
					log.Debugf("archive marked secondary, not writing to database")
					err := removeArchive(archiveFilePath, false, "")
					if err != nil {
						log.Errorf("unable to delete archive %s", archiveFilePath)
					}
					continue
				}
				log.Errorf("[validate] invalid additional data for %s --- %s", archiveFilePath, data.additionalData)
				validator.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// send to the marked destination's writer
			out, exists := validator.mapDestinationToOut[data.additionalData["destination"]]
			if !exists {
				log.Errorf("unknown destination: %s for archive %s, removing invalid archive", data.additionalData["destination"], archiveFilePath)
				validator.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// try to send the data to the next stage, but allow for the send to be cancelled
			select {
			case <-done:
				goto termination
			case out <- &data:
				log.Debugf("[validator] sent %s to writer %s", archiveFilePath, data.additionalData["destination"])
			}
		}
	}

termination:
	log.MsgInfo("Validator stage terminating")
	return nil
}

func (validator *ArchiveValidator) validateAddInfo(archiveName string, addInfo map[string]string) error {
	//check if the archive has additional data, which we need
	if len(addInfo) == 0 {
		return fmt.Errorf("additional data doesnt exist")
	}

	val, exist := addInfo["site_state"] // check for site state
	if exist {
		//ignore this archive if state set to secondary
		if val == "secondary" {
			log.Debugf("archive marked secondary, not writing to database")
			return fmt.Errorf("secondary")
		}
	}

	//check if important keys exist
	val, exist = addInfo["destination"]
	if !exist {
		return fmt.Errorf("destination key doesnt exist")
	} else if len(val) == 0 {
		return fmt.Errorf("destination set to empty")
	} else {
		log.Debugf("destination set to %s for %s", val, archiveName)
	}

	val, exist = addInfo["measurement"]
	if !exist {
		return fmt.Errorf("measurement key doesnt exist")
	} else if len(val) == 0 {
		return fmt.Errorf("measurement value is not set")
	} else {
		log.Debugf("measurement set to %s for %s", val, archiveName)
	}

	val, exist = addInfo["database"]
	if !exist {
		return fmt.Errorf("database key doesnt exist")
	} else if len(val) == 0 {
		return fmt.Errorf("database property not set")
	} else {
		log.Debugf("database set to %s for %s", val, archiveName)
	}

	return nil
}

// Remove an archive that was found invalid along with its associated data
func (validator *ArchiveValidator) cleanUpInvalidArchive(archiveName string) {
	err := removeArchive(archiveName, true, GlobalConfig.FailedValidatePath)
	if err != nil {
		log.Errorf("[validate] Continuing without removing invalid archive: could not clean invalid archive %s: %v", archiveName, err)
	} else {
		log.Debugf("[validate] Cleaned failed archive %s", archiveName)
	}

	atomic.AddUint64(&validator.FailCt, 1)
}
