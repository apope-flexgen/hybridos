package dts

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/flexgen-power/go_flexgen/fileops"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
	"k8s.io/utils/inotify"
)

type ArchiveMonitor struct {
	path string
	Out  chan string
}

// Allocates a new archive monitor stage and its output channel
func NewArchiveMonitor(path string) *ArchiveMonitor {
	return &ArchiveMonitor{
		path: path,
		Out:  make(chan string),
	}
}

// Starts the monitor pipeline stage, running workers under the given error group
func (monitor *ArchiveMonitor) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	// capture the list of files currently in the directory before starting processes
	err := monitor.readPreexistingArchives(group, groupContext)
	if err != nil {
		return fmt.Errorf("[monitor] failed to read preexisting files in archive path: %w", err)
	}

	group.Go(func() error { return monitor.monitorUntil(groupContext.Done()) })
	return nil
}

// Loop that watches for new files
func (monitor *ArchiveMonitor) monitorUntil(done <-chan struct{}) error {
	// initialize inotify watcher
	watcher, err := inotify.NewWatcher()
	if err != nil {
		return fmt.Errorf("[monitor] failed to initialize inotify watcher with error: %w", err)
	}
	// Monitor files that have completed download directly to the path or been locally moved into the path
	err = watcher.AddWatch(monitor.path, inotify.InCloseWrite|inotify.InMovedTo)
	if err != nil {
		return fmt.Errorf("[monitor] failed to add path to inotify watcher with error: %w", err)
	}

	// core process loop
	for {
		select {
		case <-done:
			goto termination
		case ev := <-watcher.Event: // receive from channel
			// extract filename (will contain path)
			substring := strings.SplitAfter(ev.String(), `"`)
			filenameWithPath := strings.Trim(substring[1], `"`)

			fileInfo, err := os.Stat(filenameWithPath)
			if err != nil {
				log.Errorf("[monitor] error getting stat of filename - %s\t:%v", filenameWithPath, err)
				continue
			}

			if fileInfo.IsDir() { // ignoring directories
				continue
			}
			if !strings.Contains(filenameWithPath, GlobalConfig.Extension) { // ignore archives that don't match the extension
				continue
			}

			// try to send file name to next stage but allow send to be cancelled
			select {
			case <-done:
				goto termination
			case monitor.Out <- filenameWithPath:
				log.Debugf("[monitor] Added new file %s to queue", filenameWithPath)
			}

		case err := <-watcher.Error:
			log.Errorf("[monitor] inotify watcher error: %s", err.Error())
		}
	}

termination:
	log.MsgInfo("Monitor stage terminating")
	return nil
}

// Gives contents of archive ingestion folder seen at startup to out in chronological order of modification time
func (monitor *ArchiveMonitor) readPreexistingArchives(group *errgroup.Group, groupContext context.Context) (StartUpError error) {
	//check if path exists
	if !fileops.Exists(monitor.path) {
		return fmt.Errorf("[monitor] archive path %s doesn't exist", monitor.path)
	}

	// Get directory entries in archive path
	dirEntries, err := os.ReadDir(monitor.path)
	if err != nil {
		return fmt.Errorf("[monitor] failed to get files in archive path, err: %w", err)
	}

	// Filter directory entries for files we want to process
	relevantFiles := make([]os.FileInfo, 0, len(dirEntries))
	for _, dirEntry := range dirEntries {
		file, err := dirEntry.Info()
		if err != nil {
			return fmt.Errorf("[monitor] failed to get file info for dir entry %s, err: %w", dirEntry.Name(), err)
		}
		// Don't process directories
		if file.IsDir() {
			continue
		}
		// Don't process files that don't have our desired extension
		matchesExtension, err := filepath.Match("*"+GlobalConfig.Extension, file.Name())
		if err != nil {
			return fmt.Errorf("[monitor] failed to create filename matcher from configured extension, err: %w", err)
		}
		if !matchesExtension {
			continue
		}
		relevantFiles = append(relevantFiles, file)
	}

	// Sort files in chronological order of modification time
	sort.Slice(relevantFiles, func(i, j int) bool {
		return relevantFiles[i].ModTime().Before(relevantFiles[j].ModTime())
	})

	group.Go(func() error {
		cancelled := groupContext.Done()
		for _, file := range relevantFiles {
			select {
			case <-cancelled:
				return nil
			default:
				filePath := filepath.Join(monitor.path, file.Name())
				monitor.Out <- filePath // add to the global queue
				log.Debugf("[monitor] added preexisting file %s to readQueue", filePath)
			}
		}
		return nil
	})
	return nil
}
