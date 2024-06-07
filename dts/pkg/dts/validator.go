package dts

import (
	"context"
	"fmt"
	"strings"

	"github.com/flexgen-power/hybridos/fims_codec"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/parquet_codec"
	"golang.org/x/sync/errgroup"
)

type ArchiveValidator struct {
	in        <-chan dataFile
	InfluxOut chan *decodedDataFileData
	MongoOut  chan *decodedDataFileData

	mapDestinationToOut map[string]chan *decodedDataFileData // maps destination strings to output channels
	FailCt              uint64                               // Modifications must be atomic
}

type decodedDataFileData struct {
	dataFileName string
	archiveSize  int64
	db           string
	measurement  string

	dataSourceId   string
	msgTimestamps  []uint64
	msgBodies      []map[string]interface{}
	additionalData map[string]string
}

// Allocates a new archive validator stage and the needed output channels based on the given destination names
func NewArchiveValidator(inputChannel <-chan dataFile) *ArchiveValidator {
	validator := ArchiveValidator{
		in:        inputChannel,
		InfluxOut: make(chan *decodedDataFileData),
		MongoOut:  make(chan *decodedDataFileData),
	}
	validator.mapDestinationToOut = map[string]chan *decodedDataFileData{
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
		case df := <-validator.in:
			dataFileName := df.name
			log.Debugf("[validate] received file %s from channel", dataFileName)
			log.Debugf("[validate] decoding archive %s", dataFileName)

			// get size of archive
			archiveSize := len(df.data)

			// decode message
			data := decodedDataFileData{}
			if strings.HasSuffix(dataFileName, ".parquet.gz") {
				decodeResult, err := parquet_codec.DecodeMemGZippedParquet(df.data)
				if err != nil {
					log.Errorf("decode failed for file %s, err: %v", dataFileName, err)
					continue
				}
				data = decodedDataFileData{
					dataFileName:   dataFileName,
					archiveSize:    int64(archiveSize),
					db:             decodeResult.Metadata["database"],
					measurement:    decodeResult.Metadata["measurement"],
					dataSourceId:   decodeResult.Metadata["data_source_id"],
					msgTimestamps:  decodeResult.MsgTimestamps,
					msgBodies:      decodeResult.MsgBodies,
					additionalData: decodeResult.Metadata,
				}
			} else {
				// assume fims_codec encoding if not parquet
				decodeResult, err := fims_codec.DecodeBytes(df.data)
				if err != nil {
					log.Errorf("decode failed for file %s, err: %v", dataFileName, err)
					continue
				}
				data = decodedDataFileData{
					dataFileName:   dataFileName,
					archiveSize:    int64(archiveSize),
					db:             decodeResult.AdditionalData["database"],
					measurement:    decodeResult.AdditionalData["measurement"],
					dataSourceId:   decodeResult.Uri,
					msgTimestamps:  decodeResult.MsgTimestamps,
					msgBodies:      decodeResult.MsgBodies,
					additionalData: decodeResult.AdditionalData,
				}
			}

			// gather additional info
			err := validator.validateAddInfo(dataFileName, data.additionalData)
			if err != nil {
				if err.Error() == "secondary" {
					log.Debugf("archive marked secondary, not writing to database")
					continue
				}
				log.Errorf("[validate] invalid additional data for %s --- %s", dataFileName, data.additionalData)
				continue
			}

			// send to the marked destination's writer
			out, exists := validator.mapDestinationToOut[data.additionalData["destination"]]
			if !exists {
				log.Errorf("unknown destination: %s for archive %s, removing invalid archive", data.additionalData["destination"], dataFileName)
				continue
			}

			// try to send the data to the next stage, but allow for the send to be cancelled
			select {
			case <-done:
				goto termination
			case out <- &data:
				log.Debugf("[validator] sent %s to writer %s", dataFileName, data.additionalData["destination"])
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
