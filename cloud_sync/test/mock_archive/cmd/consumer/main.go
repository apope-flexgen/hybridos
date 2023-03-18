// Consumes mock archives
package main

import (
	"flag"
	"fmt"
	"io/fs"
	"log"
	"os"
	"path/filepath"
	"time"

	"github.com/flexgen-power/cloud_sync/test/mock_archive"
)

// Path of directory where archives are found
var inputDirPath string
var pollPeriod time.Duration

// Consume mock archives forever
func main() {
	parseFlags()

	// Consume archives periodically
	ticker := time.NewTicker(pollPeriod)
	defer ticker.Stop()
	archiveCount := 1
	for range ticker.C {
		// Consume all archives
		err := filepath.WalkDir(inputDirPath, func(path string, d fs.DirEntry, pathErr error) (err error) {
			if pathErr != nil {
				return fmt.Errorf("invalid path error: %w", pathErr)
			}
			if path == inputDirPath {
				return nil
			} else if d.IsDir() && path != inputDirPath {
				return fs.SkipDir
			} else {
				defer func() {
					cleanupErr := os.Remove(path)
					// wrap the outer function's returned error with the failure to remove the file
					if cleanupErr != nil {
						err = fmt.Errorf("failed to remove archive file with error: %v, after tried to remove archive file due to error: %w", cleanupErr, err)
					}
				}()

				_, err = mock_archive.ReadFromFile(path)
				if err != nil {
					return fmt.Errorf("failed to read archive file at %s: %w", path, err)
				}
				log.Printf("Successfully read archive %d at %s", archiveCount, path)
				archiveCount++
			}
			return nil
		})
		if err != nil {
			log.Printf("Failed to walk input dir: %v", err)
		}
	}
}

// Parse the program flags
func parseFlags() {
	flag.StringVar(&inputDirPath, "in", ".", "Input directory for archives")
	period := flag.String("period", "1s", "String representing poll period duration")
	flag.Parse()

	var err error
	pollPeriod, err = time.ParseDuration(*period)
	if err != nil {
		log.Fatalf("Failed to parse period: %v", err)
	}
}
