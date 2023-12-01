package syswatch

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	//influx "github.com/flexgen-power/influxdb_client/v1.7"
	//mongo "github.com/flexgen-power/mongodb_client"
)

type ProcessCollector struct {
	// configurable options
	DataMan   DataManager `json:"collection"`
	Refresh   int
	Processes []string `json:"process_names"`
	Databases map[string]string

	//internal vars
	hz          float64
	mem_totalKB int
	procInfos   map[string]ProcInfo
	mutex       sync.Mutex
	//influxConn   influx.InfluxConnector
	influxOnline bool
	//mongoConn    mongo.MongoConnector
	mongoOnline bool
}

type ProcInfo struct {
	pid             string
	previousCPUTime float64
	lastExecute     time.Time
	getPID_method   string // "s" for systemctl, "b" for binary (pidof)
}

// === Collector funcs ===

func (process *ProcessCollector) init() error {
	if !process.DataMan.Active {
		return fmt.Errorf("process is inactive")
	}
	if len(process.Processes) == 0 {
		return fmt.Errorf("process is set to active but provides no stats to track")
	}

	process.mutex = sync.Mutex{}
	process.procInfos = make(map[string]ProcInfo)

	// getconf to find clock ticks per sec (hz)
	// EXEC command - one time ran only. This is ok for initialization.
	out, err := exec.Command("getconf", "CLK_TCK").Output()
	if err != nil {
		return fmt.Errorf("could not get clock ticks per sec: %w", err)
	}

	hz, err := strconv.ParseFloat(strings.Split(string(out), "\n")[0], 32)
	if err != nil {
		return fmt.Errorf("could not convert CLK_TCK to float: %w", err)
	}
	process.hz = hz

	// open meminfo to find memtotal
	f, err := os.Open("/proc/meminfo")
	if err != nil {
		return fmt.Errorf("could not read /proc/meminfo: %w", err)
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	if s.Scan() {
		// MemTotal is on the first line
		fields := strings.Fields(s.Text())
		mt, err := strconv.ParseFloat(fields[1], 32) // time the process started
		if err != nil {
			return fmt.Errorf("could not parse %s to float: %w", fields[1], err)
		}
		process.mem_totalKB = (int)(mt)
	}

	// init ProcInfo for each process name
	for _, name := range process.Processes {
		method, pid, err := getPID(name, "") // find PID to match name
		if err != nil {
			process.procInfos[name] = ProcInfo{
				pid:             "0000",
				previousCPUTime: 0,
				lastExecute:     time.Now(),
			}
			log.Errorf("could not find PID for %s: %v", pid, err)
			continue
		}

		cputime, err := process.getCPUUsage(pid) // populate initial data
		if err != nil {
			log.Errorf("could not collect process info for %s: %v", pid, err)
		}

		process.procInfos[name] = ProcInfo{
			pid:             pid,
			previousCPUTime: cputime,
			lastExecute:     time.Now(),
			getPID_method:   method,
		}
	}

	// TODO: in future ticket, add support for influx to COPS.
	// Ignoring influx/mongodb handling for now, prioritizing system status information retrieval only
	// init db health check connectors
	// if len(process.Databases) > 0 {
	// 	if addr, exists := process.Databases["influx"]; exists {
	// 		process.influxConn = influx.NewConnector(addr, time.Duration((int)(time.Second)*process.DataMan.Interval), time.Second/10, false)
	// 		err = process.influxConn.Connect()
	// 		if err != nil {
	// 			log.Errorf("could not connect to influx @%s: %v", addr, err)
	// 		}
	// 		process.influxOnline = true
	// 	}

	// 	if addr, exists := process.Databases["mongo"]; exists {
	// 		process.mongoConn = mongo.NewConnector(addr, time.Second/10, time.Duration((int)(time.Second)*process.DataMan.Interval))
	// 		err = process.mongoConn.Connect()
	// 		if err != nil {
	// 			log.Errorf("could not connect to mongo @%s: %v", addr, err)
	// 		}
	// 		process.mongoOnline = true
	// 	}
	// }

	go process.DataMan.start(process)
	return nil
}

func (process *ProcessCollector) scrape() map[string]interface{} {
	data := map[string]interface{}{
		"collector": "process",
	}

	if len(process.Processes) > 0 {
		data = mergeMaps(data, process.getProcessInfo())
	}
	// TODO: handling implementing database monitoring metrics
	// if len(process.Databases) > 0 {
	// 	data = mergeMaps(data, process.getDatabaseInfo())
	// }

	return data
}

// === ProcessCollector funcs ===
// these will all be run within the Collector.scrape() func

func (process *ProcessCollector) getProcessInfo() map[string]interface{} {
	data := make(map[string]interface{})
	for name, info := range process.procInfos {
		name = strings.Split(name, "::")[0]
		cputime, err := process.getCPUUsage(info.pid) // collect stats
		if err != nil {
			data[name+"_cpupct"] = (float32)(-4.04)
			data[name+"_mempct"] = (float32)(-4.04)

			// if collectProcessInfo failed b/c it couldnt read smap (code 404 -> -4.04), it's likely that the process shut down
			if cputime == -4.04 {
				if strings.Contains(name, "influx") {
					process.influxOnline = false
				}
				if strings.Contains(name, "mongo") {
					process.mongoOnline = false
				}

				temp := info
				delete(process.procInfos, name)
				go process.refreshPID(name, temp)
			}
			log.Errorf("could not collect data for %s... re-finding PID", name)
			continue
		}
		// CPU usage
		cpupct := 100 * ((cputime - info.previousCPUTime) / (float64)(time.Since(info.lastExecute).Seconds()))
		data[name+"_cpupct"] = math.Round(cpupct*10) / 10

		// send current values to previous
		info.previousCPUTime = cputime
		info.lastExecute = time.Now()

		process.safeWriteToProcessInfo(name, info)

		// MEM usage
		statm, err := os.Open("/proc/" + info.pid + "/statm")
		if err != nil {
			log.Errorf("could not open statm for %s: %v", name, err)
		}
		defer statm.Close()

		s := bufio.NewScanner(statm)
		if s.Scan() {
			num, err := strconv.ParseUint(strings.Fields(s.Text())[1], 0, 32)
			if err != nil {
				log.Errorf("could not convert string %s to uint: %v", strings.Fields(s.Text())[1], err)
				continue
			}

			mempct := 100 * (float64(num*4096/1024) / float64(process.mem_totalKB))
			data[name+"_mempct"] = math.Round(mempct*10) / 10
		} else {
			log.Errorf("statm empty for %s", name)
		}
	}

	return data
}

// TODO: low-priority - handle service info wrt to databases
// Deprecating this temporarily - in the advent COPS wants to provide this information
// then it is here for reference.

// func (process *ProcessCollector) getDatabaseInfo() map[string]interface{} {
// 	data := make(map[string]interface{})

// 	if _, exists := process.Databases["influx"]; exists {
// 		if !process.influxOnline {
// 			data["influx_health"] = -1
// 		} else {
// 			healthy, err := process.influxConn.HealthCheck()
// 			if !healthy {
// 				log.Warnf("failed influx health check: %v", err)
// 				data["influx_health"] = -1
// 			} else {
// 				data["influx_health"] = 1
// 			}
// 		}
// 	}

// 	if _, exists := process.Databases["mongo"]; exists {
// 		if !process.mongoOnline {
// 			data["mongo_health"] = -1
// 		} else {
// 			healthy, err := process.mongoConn.HealthCheck()
// 			if !healthy {
// 				log.Warnf("failed mongo health check: %v", err)
// 				data["mongo_health"] = -1
// 			} else {
// 				data["mongo_health"] = 1
// 			}
// 		}
// 	}

// 	return data
// }

// === Helper funcs ===

// Read from writeCh and return
// the cpu and mem % usage of the given process name.
func GetProcessCollectorData() (data map[string]interface{}, err error) {

	// Read from global data channel.
	for data = range writeCh {
		val, exists := data["collector"]
		if !exists {
			return nil, fmt.Errorf("unknown collection source for data: %v", data)
		}

		// Retrieve the process collector data map.
		if fmt.Sprintf("%v", val) == "process" {
			return data, nil
		} else {
			return nil, fmt.Errorf("process collector was not found")
		}
	}
	return nil, fmt.Errorf("no data found on channel")
}

// Provide the process collector data and extract the cpu and mem for a given process name.
func GetCPUandMemStats(data map[string]interface{}, processName string) (cpu, mem float64, err error) {
	// Check to make sure process exists.
	var found bool
	for k, _ := range data {
		if strings.Contains(k, processName) {
			found = true
		}
	}

	// Check if our process was found.
	if !found {
		return 0, 0, fmt.Errorf("process %s not found in syswatch data", processName)
	}

	// Parse out the CPU from the data.
	for k, v := range data {
		if strings.Contains(k, processName) {
			// process_cpupct
			if strings.Contains(k, "cpu") {
				cpu = v.(float64)
			}
			// process_mempct
			if strings.Contains(k, "mem") {
				mem = v.(float64)
			}
		}
	}
	return cpu, mem, nil
}

func (process *ProcessCollector) refreshPID(name string, info ProcInfo) {
	for {
		method, pid, err := getPID(name, info.getPID_method)
		if err != nil {
			log.Debugf("could not find PID for %s: %v", name, err)
			log.Tracef("waiting " + fmt.Sprintf("%v", process.Refresh) + " seconds...")
			time.Sleep(time.Duration(process.Refresh * int(time.Second))) // wait
			continue
		}

		info.getPID_method = method
		info.pid = pid
		process.safeWriteToProcessInfo(name, info)

		// TODO: handle database connection info if we so desire down the road
		// re-up database monitors
		// if strings.Contains(name, "influx") {
		// 	err = process.influxConn.Connect()
		// 	if err != nil {
		// 		log.Errorf("could not connect to influx: %v", err)
		// 	}
		// 	process.influxOnline = true
		// }
		// if strings.Contains(name, "mongo") {
		// 	err = process.mongoConn.Connect()
		// 	if err != nil {
		// 		log.Errorf("could not connect to mongo: %v", err)
		// 	}
		// 	process.mongoOnline = true
		// }

		return
	}
}

func (process *ProcessCollector) safeWriteToProcessInfo(key string, p ProcInfo) {
	process.mutex.Lock()
	process.procInfos[key] = p
	process.mutex.Unlock()
}

// Retrieve the CPU usage for a given process given its PID and hz refresh rate.
func (process *ProcessCollector) getCPUUsage(pid string) (float64, error) {
	f, err := os.Open("/proc/" + pid + "/stat")
	if err != nil {
		// probable that PID no longer exists for process
		return -4.04, err
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	if s.Scan() { // one line
		fields := strings.Fields(s.Text())               // all fields here are measured in clock ticks
		utime, err := strconv.ParseFloat(fields[13], 32) // time spent in user code
		if err != nil {
			return 0, err
		}
		stime, err := strconv.ParseFloat(fields[14], 32) // time spent in kernel code
		if err != nil {
			return 0, err
		}

		cutime, err := strconv.ParseFloat(fields[15], 32) // time spent in user code
		if err != nil {
			return 0, err
		}
		cstime, err := strconv.ParseFloat(fields[16], 32) // time spent in kernel code
		if err != nil {
			return 0, err
		}

		return (utime + stime + cutime + cstime) / process.hz, nil
	}

	return 0, fmt.Errorf("could not scan /proc/" + pid + "/stat")
}

// receives a process name and the method (systemctl "s" or binary/pidof "b") and execs accordingly
// returns the method, PID, then error
func getPID(name, method string) (string, string, error) {
	if method == "s" {
		res, err := getPIDFromSystemctl(name)
		return "s", res, err
	} else if method == "b" {
		res, err := getPIDFromName(name)
		return "b", res, err
	}

	// method is unknown -- first exec
	res, err := getPIDFromName(name) // try pidof first
	if err == nil {
		return "b", res, err
	} else {
		res, err = getPIDFromSystemctl(name) // didnt work, try systemctl
		if err == nil {
			return "s", res, err
		}

		return "", "", fmt.Errorf("could not pull PID for %s or identify proper method for doing so: %w", name, err)
	}
}

func getPIDFromName(name string) (string, error) {
	// non-systemctl instance
	index := 0
	if strings.Contains(name, "::") {
		str := strings.Split(name, "::")
		name = str[0]

		i, err := strconv.ParseUint(str[1], 0, 32)
		if err != nil {
			return "", err
		}
		index = (int)(i)
	}

	// TODO: Consider this for refreshing PID.
	// Currently PID is refreshed via systemctl implementation.
	// Remove this in future work and verify it does not effect PID refresh updates.
	out, err := exec.Command("pidof", name).Output()
	if err != nil {
		return "", err
	}

	pid := string(out)
	list := strings.Fields(pid)
	if len(list) > 1 {
		log.Errorf("multiple PIDs found for "+name+" - using %v", index)
		return list[index], nil
	} else if len(list) == 0 {
		return "", fmt.Errorf("no PID for process: " + name)
	}

	return pid[:len(pid)-1], nil
}

func getPIDFromSystemctl(name string) (string, error) {
	// TODO: change this to call from `coreos/systemd` package for obtaining PID.
	// Want to remove all instances of exec command being repeatedly ran in place of coreos package.
	out, err := exec.Command("bash", "-c", fmt.Sprintf("systemctl show -p MainPID %s | cut -d = -f2", name)).Output()
	if err != nil {
		return "", err
	}

	pid := string(out)
	pid = pid[:len(pid)-1]

	if pid != "0" {
		return pid, nil
	}
	return "", fmt.Errorf("pid not found - systemctl returned 0")
}
