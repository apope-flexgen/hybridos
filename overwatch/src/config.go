package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"

	log "github.com/flexgen-power/go_flexgen/logger"
)

type Config struct {
	Name   string
	Influx InfluxConfig `json:"destination"`

	Mem     MemCollector
	CPU     CPUCollector
	Disk    DiskCollector
	Net     NetCollector
	Process ProcessCollector
	Device  DeviceCollector
}

// struct for influx/destinaton configuration
type InfluxConfig struct {
	Active           bool `json:"influx"`
	Interval         int
	Db               string `json:"database"`
	Address          string
	Timeout          int
	HealthCheckDelay int `json:"health_check_delay"`
}

// unmarshals configFile (if it exists),
func configureDefaults(configFile string) error {
	// override blank fields with defaults
	// if name blank, set to system name
	name, err := os.Hostname()
	if err != nil {
		log.Errorf("could not obtain host name: %v", err)
	}

	if configFile != "" {
		// parse config and unmarshal
		configJSON, err := ioutil.ReadFile(configFile)
		if err != nil {
			log.Errorf("could not read config file: %w", err)
			log.Infof("using default config...")
		} else {
			err = json.Unmarshal(configJSON, &config)
			if err != nil {
				return fmt.Errorf("could not unmarshall config: %w", err)
			}

			if config.Name == "" {
				config.Name = name
			}

			return nil
		}
	}

	defaultDataMan := DataManager{
		Active:   true,
		Interval: 5,
	}

	defaultConf := Config{
		Name: name,
		Influx: InfluxConfig{
			Active: false,
		},
		Mem: MemCollector{
			DataMan: defaultDataMan,
			Stats:   []string{"MemAvailable", "MemTotal", "MemFree", "Active", "Dirty"},
		},
		CPU: CPUCollector{
			DataMan: defaultDataMan,
			LoadAvg: 5,
			Temp:    true,
			Uptime:  true,
		},
		Disk: DiskCollector{
			DataMan:        defaultDataMan,
			Mounts:         true,
			Dirs:           []string{},
			FileCountLimit: 50000,
			MBSizeLimit:    5000,
		},
		Net: NetCollector{
			DataMan: defaultDataMan,
			Fims:    true,
			Ports:   true,
			IPs:     map[string]string{},
			Stats:   []string{"wmem_max", "rmem_max"},
		},
		Process: ProcessCollector{
			DataMan:   defaultDataMan,
			Refresh:   10,
			Processes: []string{"influxd", "mongod"},
			Databases: map[string]string{
				"influx": "localhost:8086",
				"mongo":  "127.0.0.1.27017",
			},
		},
		Device: DeviceCollector{
			DataMan: DataManager{
				Active:   true,
				Interval: 1 * 60 * 60 * 24,
			},
		},
	}

	config = &defaultConf

	return nil
}
