// This file contains general file operations specific to DTS but not specific to a particular step in the process

package dts

import (
	"fmt"
	"os"

	"github.com/flexgen-power/hybridos/go_flexgen/fileops"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
)

// Removes the archive at the given path
func removeArchive(archivePath string) error {
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
	//delete file
	log.Debugf("deleting file %s", archivePath)
	err = os.Remove(archivePath)
	if err != nil {
		return fmt.Errorf("failed to delete archive with error %w", err)
	}
	return nil
}
