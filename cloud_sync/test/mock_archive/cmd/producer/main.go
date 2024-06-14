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
var numArchivesPerPeriod int

// Create mock archives forever
func main() {
	parseFlags()

	// create an archive periodically
	ticker := time.NewTicker(archivePeriod)
	defer ticker.Stop()
	averageSize := 100000
	stdDevSize := 0.05 * float64(averageSize)
	archiveCount := 1
	clients := []string{"engie", "vitol"}
	sites := []string{"libra", "ows"}
	devices := []string{"hp_la_site_controller_01", "hp_la_ess_controller_01"} // there would be specific device names for each client/site/device
	lane := []string{"1", "2"}
	for range ticker.C {
		for i := 0; i < numArchivesPerPeriod; i++ {
			size := int(rand.NormFloat64()*stdDevSize) + averageSize
			for _, client := range clients {
				for _, site := range sites {
					for _, device := range devices {
						for _, lane := range lane {
							archive := mock_archive.RandomDataArchive(fmt.Sprintf("%s__%s__%s__%s__%d.batchpqtgz", client, site, device, lane, time.Now().UnixMicro()), size)
							err := archive.WriteToFile(outputDirPath)
							if err != nil {
								log.Printf("Failed to write archive: %v", err)
							} else {
								archiveCount++
							}
						}
					}
				}
			}

		}
	}
}

// Parse the program flags
func parseFlags() {
	flag.StringVar(&outputDirPath, "out", ".", "Output directory for archives")
	period := flag.String("period", "1s", "String representing archive period duration")
	num := flag.Int("num", 1, "Number of archives created per archive period")
	flag.Parse()

	var err error
	archivePeriod, err = time.ParseDuration(*period)
	if err != nil {
		log.Fatalf("Failed to parse period: %v", err)
	}
	numArchivesPerPeriod = *num
}
