package main

import (
	"context"
	"fmt"
	"os"
	"sync/atomic"

	"github.com/flexgen-power/fims_codec"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

type archiveValidator struct {
	in   <-chan string
	outs map[string]chan *archiveData

	failCt uint64 // Modifications must be atomic
}

type archiveData struct {
	archiveFilePath string
	archiveSize     int64
	db              string
	measurement     string
	points          *fims_codec.DecodedData
}

// Allocates a new archive validator stage and the needed output channels based on the given destination names
func newArchiveValidator(inputChannel <-chan string, destinations []string) *archiveValidator {
	validator := archiveValidator{
		in:   inputChannel,
		outs: make(map[string]chan *archiveData),
	}
	for _, dest := range destinations {
		validator.outs[dest] = make(chan *archiveData)
	}
	return &validator
}

// Decompresses and decodes archives to validate them and determine which writer will need them,
// writers is a map from archive's decoded destination to a channel to the destination writer
func (validator *archiveValidator) run(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	validator.failCt = 0

	for n := 0; n < config.NumValidateWorkers; n++ {
		group.Go(func() error { return validator.validateUntil(groupContext.Done()) })
	}
	return nil
}

// Loop that validates incoming files
func (validator *archiveValidator) validateUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			return nil
		case archiveFilePath := <-validator.in:
			log.Debugf("[validate] received file %s from channel", archiveFilePath)
			log.Debugf("[validate] decoding archive %s", archiveFilePath)

			// decode message
			points, err := fims_codec.Decode(archiveFilePath) // attempt decode FIMS messages
			if err != nil {
				log.Errorf("decode failed for file %s\terr: %v", archiveFilePath, err)
				validator.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			// gather additional info
			addInfo := points.AdditionalData
			err = validator.validateAddInfo(archiveFilePath, addInfo)
			if err != nil {
				if err.Error() == "secondary" {
					log.Debugf("archive marked secondary, not writing to database")
					err := removeArchive(archiveFilePath, false, "")
					if err != nil {
						log.Errorf("unable to delete archive %s", archiveFilePath)
					}
					continue
				}
				log.Errorf("[validate] invalid additional data for %s --- %s", archiveFilePath, points.AdditionalData)
				validator.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			info, err := os.Stat(archiveFilePath) // get size of archive
			if err != nil {
				continue
			}
			archiveSize := info.Size()

			// send to the marked destination's writer
			out, exists := validator.outs[addInfo["destination"]]
			if !exists {
				log.Errorf("unknown destination: %s for archive %s, removing invalid archive", addInfo["destination"], archiveFilePath)
				validator.cleanUpInvalidArchive(archiveFilePath)
				continue
			}

			out <- &archiveData{
				archiveFilePath: archiveFilePath,
				archiveSize:     archiveSize,
				db:              addInfo["database"],
				measurement:     addInfo["measurement"],
				points:          points,
			}
			log.Debugf("[validator] sent %s to writer %s", archiveFilePath, addInfo["destination"])
		}
	}
}

func (validator *archiveValidator) validateAddInfo(archiveName string, addInfo map[string]string) error {
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
func (validator *archiveValidator) cleanUpInvalidArchive(archiveName string) {
	err := removeArchive(archiveName, true, config.FailedValidatePath)
	if err != nil {
		log.Errorf("[validate] Continuing without removing invalid archive: could not clean invalid archive %s: %v", archiveName, err)
	} else {
		log.Debugf("[validate] Cleaned failed archive %s", archiveName)
	}

	atomic.AddUint64(&validator.failCt, 1)
}
