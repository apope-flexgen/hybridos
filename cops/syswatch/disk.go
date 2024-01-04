package syswatch

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"syscall"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sys/unix"
)

type DiskCollector struct {
	// configurable options
	DataMan        DataManager `json:"collection"`
	Mounts         bool
	Dirs           []string `json:"directories"`
	FileCountLimit int      `json:"filecount_tracking_max"`
	MBSizeLimit    int      `json:"mb_size_tracking_max"`

	// internal vars
	mounts      map[string]string
	sizes       map[string]uint64
	dirDisabled map[string]bool
	dirLast     map[string]time.Time
	firstExec   bool
}

// === Collector funcs ===

func (disk *DiskCollector) init() error {
	if !disk.DataMan.Active {
		return fmt.Errorf("disk is inactive")
	}
	if !disk.Mounts && len(disk.Dirs) == 0 {
		return fmt.Errorf("disk is set to active but provides no stats to track")
	}

	// grab mounts for each sda logged in diskstats
	f, err := os.Open(dataDir + "/proc/diskstats")
	if err != nil {
		return fmt.Errorf("could not read /proc/diskstats: %v", err)
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	disk.mounts = make(map[string]string)
	disk.sizes = make(map[string]uint64)

	for s.Scan() { // scan each line for names
		fields := strings.Fields(s.Text())
		disk.mounts[fields[2]] = ""
	}
	delete(disk.mounts, "sda")

	// cross-reference with mounts
	f, err = os.Open(dataDir + "/proc/mounts")
	if err != nil {
		return fmt.Errorf("could not read /proc/mounts: %v", err)
	}
	defer f.Close()

	s = bufio.NewScanner(f)
	for s.Scan() { // scan each line for tracked mounts
		fields := strings.Fields(s.Text())
		for name, mnt := range disk.mounts {
			if mnt != "" {
				continue
			}
			if strings.Contains(fields[0], name) {
				disk.mounts[name] = fields[1]
			}
		}
	}

	// get allocated blocks -> MB
	f, err = os.Open(dataDir + "/proc/partitions")
	if err != nil {
		return fmt.Errorf("could not read /proc/partitions: %v", err)
	}
	defer f.Close()

	s = bufio.NewScanner(f)
	for s.Scan() { // scan each line for tracked mounts
		fields := strings.Fields(s.Text())
		if len(fields) != 4 {
			continue
		}
		if _, exists := disk.mounts[fields[3]]; exists {
			data, err := strconv.ParseUint(fields[2], 0, 64)
			if err != nil {
				return err
			}
			disk.sizes[fields[3]] = data / 1024
		}
	}

	// custom dir params
	disk.dirDisabled = make(map[string]bool)
	disk.dirLast = make(map[string]time.Time)
	for _, dir := range disk.Dirs {
		disk.dirDisabled[dir] = false
		disk.dirLast[dir] = time.Now()
	}
	disk.firstExec = true

	go disk.DataMan.start(disk)
	return nil
}

func (disk *DiskCollector) scrape() map[string]interface{} {
	data := map[string]interface{}{
		"collector": "disk",
	}

	if disk.Mounts {
		data = mergeMaps(data, disk.getMountInfo())
	}
	if len(disk.Dirs) > 0 {
		data = mergeMaps(data, disk.getDirInfo())
	}

	return data
}

// === DiskCollector funcs ===
// these will all be run as goroutines within the Collector.scrape() func

func (disk *DiskCollector) getMountInfo() map[string]interface{} {
	data := make(map[string]interface{})
	var stat unix.Statfs_t

	// mountstats from unix stats
	for name, mnt := range disk.mounts {
		if mnt == "" {
			continue
		}
		err := unix.Statfs(mnt, &stat)
		if err != nil {
			log.Errorf("could not stat mount %s: %v", mnt, err)
			continue
		}
		used := stat.Blocks - stat.Bfree
		data[name+"_"+safeParseDir(mnt)+"_availMB"] = (int)(float32(stat.Bavail*uint64(stat.Bsize)) / (1024.0 * 1024.0))
		data[name+"_"+safeParseDir(mnt)+"_pctfull"] = (int)(float32(used) / float32(used+stat.Bavail) * 100)
	}

	// pull I/O stats from diskstats
	f, err := os.Open(dataDir + "/proc/diskstats")
	if err != nil {
		log.Errorf("could not read diskstats: %v", err)
	} else {
		s := bufio.NewScanner(f)
		for s.Scan() {
			fields := strings.Fields(s.Text())
			if len(fields) != 14 {
				continue
			}
			if _, exists := disk.mounts[fields[2]]; exists {
				if disk.mounts[fields[2]] == "" {
					continue
				}

				rtime, err := strconv.ParseUint(fields[6], 0, 64)
				if err != nil {
					log.Errorf("diskstat err on %s: %v", fields[2], err)
				} else {
					data[fields[2]+"_"+safeParseDir(disk.mounts[fields[2]])+"_rtimesec"] = (int)(rtime / 1000)
				}

				wtime, err := strconv.ParseUint(fields[10], 0, 64)
				if err != nil {
					log.Errorf("diskstat err on %s: %v", fields[2], err)
				} else {
					data[fields[2]+"_"+safeParseDir(disk.mounts[fields[2]])+"_wtimesec"] = (int)(wtime / 1000)
				}
			}
		}
	}
	defer f.Close()

	return data
}

func (disk *DiskCollector) getDirInfo() map[string]interface{} {
	data := make(map[string]interface{})

	for _, dir := range disk.Dirs {
		// modified
		f, err := os.Stat(dir)
		if err != nil {
			log.Errorf("could not stat %s: %v", dir, err)
			continue
		}

		changed := time.Unix(f.Sys().(*syscall.Stat_t).Ctim.Unix())
		if time.Since(changed) <= time.Duration((int)(time.Second)*disk.DataMan.Interval) {
			data[safeParseDir(dir)+"_changed"] = 1
		} else {
			data[safeParseDir(dir)+"_changed"] = -1
			if !disk.firstExec {
				log.Tracef("%s not changed, skipping calculations...", safeParseDir(dir))
				continue // no need to check the size or filecount if we know it hasnt changed
			}
		}

		if disk.dirDisabled[dir] && time.Since(disk.dirLast[dir]) < time.Minute*30 {
			continue // skip if we know the dir has gotten too big for performance
		} else if disk.dirDisabled[dir] && time.Since(disk.dirLast[dir]) >= time.Minute*30 {
			disk.dirDisabled[dir] = false //pass 30 minnutes, start checking
		}

		// num files
		out, err := exec.Command("bash", "-c", fmt.Sprintf("ls -U %s | wc -l", dir)).Output()
		if err != nil {
			log.Errorf("could not execute ls %s | wc -l: %v", dir, err)
			continue
		}

		filect := strings.Fields(string(out))[0]
		num, err := strconv.ParseUint(filect, 0, 64)
		if err != nil {
			log.Errorf("diskstat err on %s: %v", filect, err)
			continue
		}

		// check if we have exceeded our limit for filecount
		if int(num) >= disk.FileCountLimit {
			disk.dirDisabled[dir] = true
			disk.dirLast[dir] = time.Now()
			log.Errorf("maximum detectable filecount reached for %s... will track again in 30m", dir)
		}

		data[safeParseDir(dir)+"_files"] = int(num)

		// size MB
		out, err = exec.Command("du", "-sm", dir).Output()
		if err != nil {
			log.Errorf("could not execute du on %s: %v", dir, err)
			continue
		}

		mb := strings.Fields(string(out))[0]
		num, err = strconv.ParseUint(mb, 0, 32)
		if err != nil {
			log.Errorf("diskstat err on %s: %v", mb, err)
			continue
		}

		// check if we have exceeded our limit for size
		if int(num) >= disk.MBSizeLimit {
			disk.dirDisabled[dir] = true
			disk.dirLast[dir] = time.Now()
			log.Errorf("maximum detectable size reached for %s... will track again in 30m", dir)
		}

		data[safeParseDir(dir)+"_sizeMB"] = int(num)
	}

	if disk.firstExec {
		disk.firstExec = false
	}

	return data
}
