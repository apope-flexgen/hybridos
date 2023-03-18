// Produces mock archives
package main

import (
	"flag"
	"fmt"
	"log"
	"math/rand"
	"time"

	"github.com/flexgen-power/cloud_sync/test/mock_archive"
)

// Path of directory where archives are made
var outputDirPath string
var archivePeriod time.Duration

// Create mock archives forever
func main() {
	parseFlags()

	// create an archive periodically
	ticker := time.NewTicker(archivePeriod)
	defer ticker.Stop()
	averageSize := 10000000
	stdDevSize := 0.05 * float64(averageSize)
	archiveCount := 1
	for range ticker.C {
		size := int(rand.NormFloat64()*stdDevSize) + averageSize
		archive := mock_archive.RandomDataArchive(fmt.Sprintf("archive_#%d_%d.tar.gz", archiveCount, time.Now().UnixMicro()), size)
		err := archive.WriteToFile(outputDirPath)
		if err != nil {
			log.Printf("Failed to write archive: %v", err)
		} else {
			archiveCount++
		}
	}
}

// Parse the program flags
func parseFlags() {
	flag.StringVar(&outputDirPath, "out", ".", "Output directory for archives")
	period := flag.String("period", "1s", "String representing archive period duration")
	flag.Parse()

	var err error
	archivePeriod, err = time.ParseDuration(*period)
	if err != nil {
		log.Fatalf("Failed to parse period: %v", err)
	}
}
