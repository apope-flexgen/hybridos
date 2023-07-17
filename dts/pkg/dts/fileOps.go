// This file contains general file operations specific to DTS but not specific to a particular step in the process

package dts

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/flexgen-power/go_flexgen/fileops"
	log "github.com/flexgen-power/go_flexgen/logger"
)

// Moves the archive out of the archive path.
// Sends to the given failure path if desired, otherwise sends to forward
func removeArchive(archivePath string, sendToFailure bool, failurePath string) error {
	//check if file exist
	log.Debugf("purging archive %s", archivePath)
	exist := fileops.Exists(archivePath)
	if !exist {
		return fmt.Errorf("archive - %s doesnt exist", archivePath)
	}
	fileInfo, err := os.Stat(archivePath)
	if err != nil {
		return fmt.Errorf("failed to get stat of archive %s: %w", archivePath, err)
	}
	if fileInfo.IsDir() {
		return fmt.Errorf("archive %s is a directory, it should just be a file", archivePath)
	}
	// push to garbage folder.
	if sendToFailure {
		// move archive to garbage configured
		fileLenCnt := len(failurePath)
		if fileLenCnt == 0 {
			log.MsgTrace("garbage location not specified")
		} else {
			newArchivePath := filepath.Join(failurePath, filepath.Base(archivePath))
			err := os.Rename(archivePath, newArchivePath)
			if err != nil {
				return fmt.Errorf("error mving file to garbage path: %v", err)
			}
		}
	} else {
		//purging a properly decoded archive. Happy path continues
		//if forward path specified, move file to that location, otherwise delete the archive
		forwardPath := GlobalConfig.FwdPath
		if len(forwardPath) == 0 {
			log.MsgTrace("forwarding location not specified")
			//delete file
			log.Debugf("deleting file %s", archivePath)
			err = os.Remove(archivePath)
			if err != nil {
				return fmt.Errorf("failed to delete archive with error %w", err)
			}
		} else {
			newArchivePath := filepath.Join(forwardPath, filepath.Base(archivePath))
			err := os.Rename(archivePath, newArchivePath)
			if err != nil {
				return fmt.Errorf("error mving file to forward path: %w", err)
			}
		}
	}
	return nil
}
