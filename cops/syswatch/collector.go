package syswatch

import (
	"os"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Collector is the parent *interface* for all <Type>Collector(s)
type Collector interface {
	// initializes the exporter
	init() error
	// notifies exporter to gather the exporter statistics
	scrape() map[string]interface{}
}

type Collectors struct {
	Name    string
	Mem     MemCollector
	CPU     CPUCollector
	Disk    DiskCollector
	Net     NetCollector
	Process ProcessCollector
}

// Configure collection metrics to default settings for now.
func configureDefaults(proccessList []string) error {
	// override blank fields with defaults
	// if name blank, set to system name
	name, err := os.Hostname()
	if err != nil {
		log.Errorf("could not obtain host name: %v", err)
	}

	defaultDataMan := DataManager{
		Active:   true,
		Interval: 5,
	}

	defaultConf := Collectors{
		Name: name,

		Mem: MemCollector{
			DataMan: defaultDataMan,
			Stats:   []string{"MemAvailable", "MemTotal", "MemFree", "Active", "Dirty"},
		},
		CPU: CPUCollector{
			DataMan: defaultDataMan,
			LoadAvg: 5,
			// TODO: keep data types consistent between cops and collector data.
			// The temp and uptime should be TempEnable and UptimeEnable, with
			// extra fields for storing true values.
			Temp:   true,
			Uptime: true,
		},
		Disk: DiskCollector{
			DataMan:        defaultDataMan,
			Mounts:         true,
			Dirs:           []string{},
			FileCountLimit: 50000,
			MBSizeLimit:    5000,
		},
		Net: NetCollector{
			DataMan:    defaultDataMan,
			Fims:       true,
			Interfaces: true,
			IPs:        map[string]string{},
			Stats:      []string{"wmem_max", "rmem_max"},
		},
		Process: ProcessCollector{
			DataMan:   defaultDataMan,
			Refresh:   10,
			Processes: proccessList,
			// Databases: map[string]string{
			// 	"influx": "localhost:8086",
			// 	"mongo":  "127.0.0.1:27017",
			// },
		},
	}

	config = &defaultConf

	return nil
}
