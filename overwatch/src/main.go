package main

import (
	"bufio"
	"fims"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"runtime/trace"
	"strconv"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	influx "github.com/flexgen-power/influxdb_client/v1.7"

	"github.com/pkg/profile"
)

var (
	// global vars
	config  *Config
	writeCh = make(chan map[string]interface{})
	f       fims.Fims
	conn    influx.InfluxConnector
	dataDir string
)

func main() {
	// === SETUP ===
	// parse commandline args
	var configFile string
	var prof string
	flag.StringVar(&configFile, "c", "", "specify the config file for the process")
	flag.StringVar(&configFile, "config", "", "specify the config file for the process")
	flag.StringVar(&prof, "prof", "", "enables profiling and specifies type")
	flag.Parse()

	if prof == "cpu" { //profiling argument, only works if flags are in front of config file
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "trace" {
		f, err := os.Create("trace.out")
		if err != nil {
			panic(err)
		}
		defer f.Close()

		err = trace.Start(f)
		if err != nil {
			panic(err)
		}
		defer trace.Stop()
	}

	// set up logger
	err := log.InitConfig("overwatch").Init("overwatch")
	if err != nil {
		fmt.Printf("Error initializing logger for overwatch: %v\n", err)
		os.Exit(-1)
	}

	err = configureDefaults(configFile)
	if err != nil {
		log.Fatalf("Config error: %v", err)
	}

	if !config.Influx.Active {
		log.Infof("destination set to FIMS")
		f, err = fims.Connect("overwatch") // connect to fims
		if err != nil {
			log.Fatalf("could not connect to FIMS: %v", err)
		}
	} else {
		log.Infof("destination set to InfluxDB")
		conn = influx.NewConnector(config.Influx.Address, time.Duration(int(time.Second)*config.Influx.Timeout), time.Duration(int(time.Second)*config.Influx.HealthCheckDelay), false)
		err := conn.Connect()
		if err != nil {
			log.Fatalf("could not connect to influx instance at %s: %v", config.Influx.Address, err)
		}

		err = conn.CreateDatabase(config.Influx.Db, nil)
		if err != nil {
			log.Fatalf("could not create influx db %s at %s: %v", config.Influx.Db, config.Influx.Address, err)
		}
	}

	startCollectors()
	go write()

	if prof == "trace" {
		signalChan := make(chan os.Signal, 1)
		signal.Notify(signalChan, os.Interrupt)
		<-signalChan
	} else {
		select {}
	}
}

func startCollectors() {
	collectors := map[string]Collector{
		"mem":     &config.Mem,
		"cpu":     &config.CPU,
		"disk":    &config.Disk,
		"net":     &config.Net,
		"process": &config.Process,
		"device":  &config.Device,
	}

	for name, collector := range collectors {
		err := collector.init()
		if err != nil {
			log.Errorf("%s did not initialize: %v", name, err)
			continue
		}
		log.Infof("started %s", name)
		time.Sleep(time.Second)
	}

	log.Infof("initial collector startup complete.")
}

func write() {
	last := time.Now()
	bodies := make(map[string][]map[string]interface{}, 0)
	times := make(map[string][]uint64, 0)

	for data := range writeCh {
		val, exists := data["collector"]
		if !exists {
			log.Errorf("unknown collection source for data: %v\nmoving on...", data)
			continue
		}
		collector := fmt.Sprintf("%v", val)
		delete(data, "collector")

		if !config.Influx.Active { // place body into FIMS message and send
			msg := fims.FimsMsg{
				Method:  "pub",
				Uri:     "/systemstats/" + collector + "/" + config.Name,
				Replyto: "",
				Body:    data,
			}
			_, err := f.Send(msg)
			if err != nil {
				log.Errorf(err.Error())
				continue
			} else {
				log.Tracef("Successfully sent!")
			}
		} else { // writes to underlying influx buffer writer
			bodies[collector] = append(bodies[collector], data)
			times[collector] = append(times[collector], uint64(time.Now().UnixMicro()))

			if time.Since(last) >= time.Duration(int(time.Second)*config.Influx.Interval) { // if time to write to influx
				last = time.Now()

				for name, data := range bodies {
					batches, err := conn.MakeBatches(config.Influx.Db, name+"stats", config.Name, times[name], data, map[string]interface{}{"messages": len(data)})
					if err != nil {
						log.Errorf("failed to make %s batch: %v", name, err)
					}
					for n, batch := range batches {
						err = conn.WriteBatch(batch)
						if err != nil {
							log.Errorf("failed to write %s batch #%v: %v", name, n, err)
						} else {
							log.Tracef("wrote %s batch #%v", name, n)
						}
					}
				}

				// reset
				bodies = make(map[string][]map[string]interface{})
				times = make(map[string][]uint64)
			}
		}
	}
}

// === HELPER FUNCS ===

func parseSoloUIntFile(filename string) (uint64, error) {
	f, err := os.Open(filename)
	if err != nil {
		return 0, err
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	if s.Scan() {
		data, err := strconv.ParseUint(s.Text(), 0, 32)
		if err != nil {
			return 0, err
		} else {
			return data, nil
		}
	}
	return 0, fmt.Errorf("no data in file: " + filename)
}

func safeParseDir(dir string) string {
	switch dir {
	case "/":
		return "root"
	default:
		return strings.ReplaceAll(dir[1:], "/", "_")
	}
}

func mergeMaps(maps ...map[string]interface{}) map[string]interface{} {
	dest := maps[0]
	maps = maps[1:]
	for _, m := range maps {
		for key, val := range m {
			dest[key] = val
		}
	}

	return dest
}

func deepCopyMap(original map[string]interface{}) map[string]interface{} {
	deepCopy := make(map[string]interface{}, len(original))

	for val, key := range original {
		deepCopy[val] = key
	}

	return deepCopy
}
