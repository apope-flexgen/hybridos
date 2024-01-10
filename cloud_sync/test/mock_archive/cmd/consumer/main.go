// Consumes mock archives
package main

import (
	"flag"
	"fmt"
	"io"
	"io/fs"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"sync/atomic"
	"time"

	"github.com/flexgen-power/cloud_sync/test/mock_archive"
)

// Path of directory where archives are found
var inputDirPath string
var pollPeriod time.Duration

// Count of archives consumed so far
var archiveCount uint64

func main() {
	parseFlags()

	// Consume mock archives forever
	go consume()

	// Expose an endpoint for the consumed archive count to be retrieved
	http.HandleFunc("/archiveCount", func(w http.ResponseWriter, _ *http.Request) {
		localArchiveCount := atomic.LoadUint64(&archiveCount)
		io.WriteString(w, fmt.Sprintf("%d\n", localArchiveCount))
	})
	err := http.ListenAndServe(":8080", nil)
	log.Fatalf("HTTP listen and serve failed: %v", err)
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

// Consumes archives periodically
func consume() {
	ticker := time.NewTicker(pollPeriod)
	defer ticker.Stop()
	var localArchiveCount uint64 = 0 // locally managed unsynchronized count for easy reads and modifications
	atomic.StoreUint64(&archiveCount, localArchiveCount)
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
				localArchiveCount++
				atomic.StoreUint64(&archiveCount, localArchiveCount)
				log.Printf("Successfully read archive %d at %s", archiveCount, path)
			}
			return nil
		})
		if err != nil {
			log.Printf("Failed to walk input dir: %v", err)
		}
	}
}
