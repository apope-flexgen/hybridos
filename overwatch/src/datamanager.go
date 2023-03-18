package main

import (
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Collector is the parent *interface* for all <Type>Collector(s)
type Collector interface {
	// initializes the exporter
	init() error
	// notifies exporter to gather the exporter statistics
	scrape() map[string]interface{}
}

// DataManager is a struct that is *composite* within all implementations of the Collector interface
type DataManager struct {
	// configurable options
	Active   bool
	Interval int
}

func (dataman *DataManager) start(coll Collector) {
	if dataman.Interval == 0 { // defaults to 5 if interval isnt given
		dataman.Interval = 5
	}

	prev_scrape := make(map[string]interface{})
	ticker := time.NewTicker(time.Duration((int)(time.Second) * dataman.Interval))

	for {
		scrape := coll.scrape()
		if len(scrape) == 1 {
			log.Errorf("did not receive scrape scrape from %v", scrape["collector"])
			continue
		}

		scrape = mergeMaps(prev_scrape, scrape)
		prev_scrape = scrape

		writeCh <- deepCopyMap(scrape)

		<-ticker.C // wait on ticker
	}
}
