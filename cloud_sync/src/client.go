package main

import (
	"errors"
	"fmt"
	"io/fs"
	"os"
	"path"
	"sort"
	"strings"
	"time"

	"github.com/flexgen-power/go_flexgen/buffman"
	log "github.com/flexgen-power/go_flexgen/logger"
	"k8s.io/utils/inotify"
)

// A client is a directory where files that must be transferred will be.
// The directory must be checked for files on startup and continuously
// monitored for new files that appear during runtime.
type client struct {
	name                    string
	config                  ClientConfig
	db                      *databaseManager
	watcher                 *inotify.Watcher
	newFilesQ               chan string                     // input of the new files buffer manager
	newFilesBuffMan         buffman.BufferManager           // buffer manager of files which have been noticed
	sendQ                   <-chan string                   // files which are to be sent
	sendSrcDirPath          string                          // path of directory in which to find files to send
	sendRequestQsToServers  map[string]chan transferRequest // maps server name to a channel through which we send transfer requests for normal sends
	retryRequestQsToServers map[string]chan transferRequest // maps server name to a channel through which we send transfer requests for retries
	retryQ                  map[string]chan string          // maps server name to a channel of file paths to retry
	clearQ                  chan string                     // filepaths to remove
	connectedServers        []*server
	tarQOut                 chan string // files which have been tarred/untarred if in a tar mode
}

type ClientConfig struct {
	Dir        string `json:"directory"`
	Key        string `json:"private_key"`
	Knownhosts string `json:"known_hosts"`
	Servers    []string
	Ext        string    `json:"extension"`
	Tar        TarConfig `json:"tar_mode"`
	AWSId      string    `json:"aws_id"`
	AWSSecret  string    `json:"aws_secret_key"`
}

type TarConfig struct {
	Mode               string `json:"mode"`                // either "tar" or "untar"
	Interval           uint   `json:"interval_seconds"`    // if in tar mode, maximum seconds between tarring input files (assuming BufferSize doesn't trigger a tar first)
	BufferSize         uint   `json:"buffer_size"`         // if in tar mode, number of files in input that will trigger a tar (assuming Interval doesn't trigger a tar first)
	ExtractedExtension string `json:"extracted_extension"` // if in untar mode, extension of extracted files to send rather than delete
}

type clientMap map[string]*client

var clients clientMap = make(clientMap)

// CLIENT WORKER FUNCS

// Uses the Linux inotify API to monitor the client's directory and load new files into the send queue.
func (cl *client) monitor() {
	for {
		select {
		case ev := <-cl.watcher.Event: // in case of event
			fileName, skipFile := cl.getFileFromWatcherEvent(ev, cl.config.Ext)
			if skipFile {
				continue
			}
			cl.newFilesQ <- fileName
		case err := <-cl.watcher.Error:
			log.Errorf("Received error from inotify: %v", err)
		}
	}
}

func (cl *client) tar() {
	fileBuffer := make([]string, 0, cl.config.Tar.BufferSize)

	ticker := time.NewTicker(time.Duration(uint(time.Second) * cl.config.Tar.Interval))

	flush := false
	for {
		select {
		case <-ticker.C:
			if len(fileBuffer) > 0 {
				flush = true
			}
		case fileName := <-cl.newFilesBuffMan.Out():
			fileBuffer = append(fileBuffer, fileName)

			if uint(len(fileBuffer)) == cl.config.Tar.BufferSize {
				flush = true
				ticker.Reset(time.Duration(uint(time.Second) * cl.config.Tar.Interval))
			}
		}

		if flush {
			// perform tar
			tarBallName, err := compress(fileBuffer, cl.config.Dir, cl.name)
			if err != nil {
				log.Errorf("%v", err)
				continue
			}

			// send tarball
			cl.tarQOut <- tarBallName

			// clean up contents
			for _, file := range fileBuffer {
				cl.clearQ <- path.Join(cl.config.Dir, file)
			}

			// reset variables
			fileBuffer = make([]string, 0, cl.config.Tar.BufferSize)
			flush = false
		}
	}
}

// Untars tarred files for the client to send
func (cl *client) untar() {
	for tarFileName := range cl.newFilesBuffMan.Out() {
		// perform untar
		fileNames, err := decompress(tarFileName, cl.config.Dir, cl.sendSrcDirPath)
		if err != nil {
			// in the case of a decompress error, we should still send any successfully extracted files and remove the original;
			// so we log and then continue with the normal file handling logic
			log.Errorf("Client %s had an error when untarring file %s: %v", cl.name, tarFileName, err)
		}

		// send or remove extracted files
		for _, fileName := range fileNames {
			if strings.HasSuffix(fileName, cl.config.Tar.ExtractedExtension) {
				cl.tarQOut <- fileName
			} else {
				cl.clearQ <- path.Join(cl.sendSrcDirPath, fileName)
			}
		}

		// remove original file
		cl.clearQ <- path.Join(cl.config.Dir, tarFileName)
	}
}

// Takes file names from the send queue as they are available and sends the named file to all servers
// that the client is configured to send to. If the file fails to send to any of the servers, it will
// be added to the error directory and the servers it failed to send to will be recorded for the retry
// thread to retry sending the file to.
func (cl *client) send() {
	transferResponseChannel := make(chan transferResponse)

	for fileName := range cl.sendQ {
		failed := false
		// for all destinations, attempt proper send
		for _, serv := range cl.connectedServers {
			// send a transfer request and wait for response
			cl.sendRequestQsToServers[serv.name] <- transferRequest{fileName: fileName, srcDirPath: cl.sendSrcDirPath, responseChannel: transferResponseChannel}
			resp := <-transferResponseChannel

			if resp.err != nil {
				log.Debugf("Error sending %s from %s to %s: %v.", fileName, cl.name, serv.name, resp.err)

				// mark that this file failed to send to this server
				err := cl.addFailure(fileName, serv.name)
				if err != nil {
					log.Errorf("Could not add failure for %s to %s in db: %v", fileName, serv.name, err)
				}

				// if this is the first server that the file has failed to send to, copy the file into
				// the error folder for future retries and mark that a failure has occurred to prevent
				// redundant copies into the error folder
				if !failed {
					err, _ := copy(cl.sendSrcDirPath, path.Join(cl.config.Dir, "error"), fileName) // move to error
					if err != nil {
						log.Errorf("Could not copy %s into error directory: %v", fileName, err)
					}
					failed = true
				}

				// add the file to the retry queue if retryable
				if resp.retryable {
					cl.retryQ[serv.name] <- fileName
				}
				continue
			}

			log.Debugf("Sent %s to %s.", fileName, serv.name)
		}

		// at this point, file has been sent to all servers and added to the
		// error directory if it failed for any. add the file to the clear queue
		// so it will be deleted from client directory
		cl.clearQ <- path.Join(cl.sendSrcDirPath, fileName)
	}
}

// Takes file paths off the clear queue and deletes the given file. If there is an error
// deleting the file, logs the error and puts the file back in the clear queue to try again.
func (cl *client) clear() {
	for filePath := range cl.clearQ {
		err := os.Remove(filePath)
		if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				log.Warnf("os.Remove file %s failed with error %v. Will not retry clearing file.", filePath, err)
				continue
			}
			log.Errorf("os.Remove failed for %s: %v. Will retry to clear file.", filePath, err)
			cl.clearQ <- filePath
			continue
		}
		log.Debugf("Cleared %s", filePath)
	}
}

// Takes files off the retry queue and attempts to send them to the given server.
func (cl *client) retry(serv *server) {
	transferResponseChannel := make(chan transferResponse)

	for fileName := range cl.retryQ[serv.name] {
		// send a transfer request and wait for response
		cl.retryRequestQsToServers[serv.name] <- transferRequest{fileName: fileName, srcDirPath: path.Join(cl.config.Dir, "error"), responseChannel: transferResponseChannel}
		resp := <-transferResponseChannel

		if resp.err != nil {
			log.Debugf("Error retrying send of file %s from client %s to server %s: %v", fileName, cl.name, serv.name, resp.err)
			// if the error is not retryable, don't try to retry
			if !resp.retryable {
				continue
			} else {
				// if there was a retryable failure, add the file back to the retry queue
				cl.retryQ[serv.name] <- fileName
			}
			continue
		}

		// now that the file has been successfully sent to the server, remove the server from the failed servers map of this file
		complete, err := cl.removeFailure(fileName, serv.name)
		if err != nil {
			log.Errorf("Could not remove failure for %s on %s: %v", fileName, serv.name, err)
		}

		// if there are no more servers that the file must be sent to, delete the archive from the error folder
		if complete {
			log.Debugf("%s sent to all destinations. Clearing from error folder...", fileName)
			cl.clearQ <- path.Join(cl.config.Dir, "error", fileName)
		}

		log.Debugf("Re-sent %s to %s.", fileName, serv.name)
	}
}

// CLIENT HELPER FUNCS

// configuration funcs

// Constructor function for a client. Allocates memory for the client and its fields.
// Ensures a client's directories exist, points client and its configured servers,
// launches the client source directory's file watcher, and opens client's database
// for tracking send failures.
func createClient(name string, cfg ClientConfig) (newClient *client, err error) {
	// if no servers were explicitly defined in the client config, default to connecting the client to all servers
	if len(cfg.Servers) == 0 {
		for serverName := range config.Servers {
			cfg.Servers = append(cfg.Servers, serverName)
		}
	}

	// instantiate a new client
	newFileChan := make(chan string)
	newClient = &client{
		name:                    strings.ReplaceAll(strings.ToLower(name), " ", "_"), // cleans name so that it can be used as a db filename
		config:                  cfg,
		db:                      &databaseManager{},
		newFilesQ:               newFileChan,
		newFilesBuffMan:         buffman.New(buffman.Stack, 1, newFileChan),
		sendRequestQsToServers:  make(map[string]chan transferRequest),
		retryRequestQsToServers: make(map[string]chan transferRequest),
		retryQ:                  make(map[string]chan string),
		clearQ:                  make(chan string, config.BufSz),
		connectedServers:        make([]*server, 0, len(cfg.Servers)),
	}
	switch cfg.Tar.Mode {
	case "tar":
		if cfg.Tar.Interval == 0 {
			return nil, fmt.Errorf("client configured for tar mode, but interval given is zero")
		}
		if cfg.Tar.BufferSize == 0 {
			return nil, fmt.Errorf("client configured for tar mode, but buffer size given is zero")
		}
		newClient.tarQOut = make(chan string)
		newClient.sendQ = newClient.tarQOut
		newClient.sendSrcDirPath = path.Join(cfg.Dir, metaArchiveDirName)
	case "untar":
		// set input archive extension to the metaArchive extension, any other extension is a configuration error
		if cfg.Ext != "" && cfg.Ext != metaArchiveExtension {
			return nil, fmt.Errorf("client configured for untar mode, but input archive extension given is %s rather than being unspecified or being %s", cfg.Ext, metaArchiveExtension)
		} else {
			newClient.config.Ext = metaArchiveExtension
		}
		newClient.tarQOut = make(chan string)
		newClient.sendQ = newClient.tarQOut
		newClient.sendSrcDirPath = path.Join(cfg.Dir, unmetaArchiveDirName)
	default:
		newClient.sendQ = newClient.newFilesBuffMan.Out()
		newClient.sendSrcDirPath = cfg.Dir
	}

	// make the error directory for this client if it does not already exist.
	// doubles as also ensuring the source directory for the client exists since source dir is parent dir of error dir
	err = ensureDirectoryExists(path.Join(newClient.config.Dir, "error"))
	if err != nil {
		return nil, fmt.Errorf("failed to ensure error directory exists: %w", err)
	}

	// configure a connection to each server to which this client will send its files
	for _, name := range newClient.config.Servers {
		cleanedName := strings.ReplaceAll(strings.ToLower(name), " ", "_")
		serv, ok := servers[cleanedName]
		if !ok {
			return nil, fmt.Errorf("did not find server with name %s in server map", cleanedName)
		}
		newClient.sendRequestQsToServers[serv.name] = make(chan transferRequest)
		newClient.retryRequestQsToServers[serv.name] = make(chan transferRequest)
		newClient.connectedServers = append(newClient.connectedServers, serv)
		log.Infof("Client %s will send its files to server %s.", newClient.name, serv.name)

		// Allocate retry queue for each connection
		newClient.retryQ[serv.name] = make(chan string, config.BufSz)
	}

	// launch inotify watcher
	newClient.watcher, err = inotify.NewWatcher()
	if err != nil {
		return nil, fmt.Errorf("failed to start inotify watcher: %w", err)
	}
	err = newClient.watcher.AddWatch(newClient.config.Dir, inotify.InCloseWrite|inotify.InMovedTo)
	if err != nil {
		return nil, fmt.Errorf("failed to add file watcher for directory %s: %w", newClient.config.Dir, err)
	}

	// put any files that were already in the client's source dir into the send queue, taking
	// into account any new files that the watcher spots in the meantime
	err = newClient.readInitialFilesIntoQueue(newClient.config.Dir, newClient.newFilesQ, newClient.config.Ext)
	if err != nil {
		return nil, fmt.Errorf("failed to load initial files into queue: %w", err)
	}

	// create tar queue and read in any remaining .tar files from meta_archives subdir
	if newClient.config.Tar.Mode == "tar" {
		err = ensureDirectoryExists(path.Join(newClient.config.Dir, metaArchiveDirName))
		if err != nil {
			return nil, fmt.Errorf("failed to ensure metaarchive directory exists: %w", err)
		}
		err = newClient.readInitialFilesIntoQueue(path.Join(newClient.config.Dir, metaArchiveDirName), newClient.tarQOut, metaArchiveExtension)
		if err != nil {
			return nil, fmt.Errorf("failed to load initial compressed metaarchive files into queue: %w", err)
		}
	} else if newClient.config.Tar.Mode == "untar" {
		err = ensureDirectoryExists(path.Join(newClient.config.Dir, unmetaArchiveDirName))
		if err != nil {
			return nil, fmt.Errorf("failed to ensure unmetaarchive directory exists: %w", err)
		}
		err = newClient.readInitialFilesIntoQueue(path.Join(newClient.config.Dir, unmetaArchiveDirName), newClient.tarQOut, newClient.config.Ext)
		if err != nil {
			return nil, fmt.Errorf("failed to load initial decompressed metaarchive files into queue: %w", err)
		}
	}

	// initialize DB manager
	err = newClient.db.UseDB(newClient.name)
	if err != nil {
		return nil, fmt.Errorf("failed to initialize DB manager: %w", err)
	}

	return newClient, nil
}

// Reads the initial contents of the client's source directory, then loads relevant files
// into the send queue with a goroutine.
//
// Processes any inotify events that happened before reading the source directory to avoid
// putting the same file into the send queue twice.
//
// Files are inserted into the send queue sorted by file mod time.
func (cl *client) readInitialFilesIntoQueue(dir string, queue chan string, ext string) error {
	// read contents of source directory
	dirContents, err := os.ReadDir(dir)
	if err != nil {
		return fmt.Errorf("failed to read directory %s (second time): %w", dir, err)
	}

	// fill a map with any files that appeared between launching the inotify watcher and reading the source directory
	filesSeenByWatcher := make(map[string]struct{})
	for {
		noMoreNewFiles := false
		select {
		case ev := <-cl.watcher.Event:
			fileName, skipFile := cl.getFileFromWatcherEvent(ev, ext)
			if skipFile {
				continue
			}
			filesSeenByWatcher[fileName] = struct{}{}
		case err := <-cl.watcher.Error: // since inotify errors do not cause program termination in monitor thread, maintain behavior here for consistency
			log.Errorf("Received error from inotify: %v", err)
		default:
			noMoreNewFiles = true
		}
		if noMoreNewFiles {
			break
		}
	}

	// use dir read and watcher results to load all files that were in the source directory during the startup period into the send queue.
	// if a file appeared in both the read dir and the watcher, only queue it once
	go cl.sortInitialFilesIntoQueue(dirContents, filesSeenByWatcher, queue, ext)

	return nil
}

// Takes a slice of files read from the source directory and a map of files seen by the
// file watcher. Merges the two lists together and skips duplicates, then sorts them
// by file mod time and loads them into the send queue.
func (cl *client) sortInitialFilesIntoQueue(dirContentsFromRead []fs.DirEntry, filesSeenByWatcher map[string]struct{}, queue chan string, ext string) {
	initialFiles := make([]fs.FileInfo, 0, len(dirContentsFromRead)+len(filesSeenByWatcher))

	// add files from dir read into list of initial files (filtering out directories and non-matching extensions).
	// if file was seen by watcher, skip it
	for _, dirEntry := range dirContentsFromRead {
		info, err := dirEntry.Info()
		if err != nil {
			log.Errorf("Error getting info on directory entry %s during startup: %v.", dirEntry.Name(), err)
			continue
		}
		if _, fileSeenByWatcher := filesSeenByWatcher[info.Name()]; fileSeenByWatcher {
			continue
		}
		if cl.doesNotCareAbout(info, ext) {
			continue
		}
		initialFiles = append(initialFiles, info)
	}

	// add all files seen by watcher (dirs and non-matching exts already filtered out)
	for newFileName := range filesSeenByWatcher {
		newFileInfo, err := os.Stat(path.Join(cl.config.Dir, newFileName))
		if err != nil {
			log.Errorf("Error getting file stat for file %s found by watcher during startup in client %s: %v", newFileName, cl.name, err)
			continue
		}
		initialFiles = append(initialFiles, newFileInfo)
	}
	// sort the files based on when they arrived in the folder (safe assumption that they are not modified after arriving in folder)
	sort.Slice(initialFiles, func(i, j int) bool {
		return initialFiles[i].ModTime().Before(initialFiles[j].ModTime())
	})
	// add the files to the queue
	for _, fileInfo := range initialFiles {
		queue <- fileInfo.Name()
	}
}

// Parses the file name out from an inotify event string and filters it through a client's
// specifications for files that it cares about. Returns skipFile as true if the client
// does not care about the file.
func (cl *client) getFileFromWatcherEvent(ev *inotify.Event, ext string) (fileName string, skipFile bool) {
	// example of ev.String(): "/home/vagrant/test.txt": 0x100 == IN_CREATE
	// SplitAfter [1] isolates /home/vagrant/test.txt" then the Trim gets rid of the second quote mark
	fileNameWithPath := strings.Trim(strings.SplitAfter(ev.String(), `"`)[1], `"`)
	fileInfo, err := os.Stat(fileNameWithPath)
	if err != nil {
		log.Errorf("Error getting file stat for %s from client %s: %v", fileNameWithPath, cl.name, err)
		return "", true
	}
	// ignore directories and files that do not contain the necessary extension
	if cl.doesNotCareAbout(fileInfo, ext) {
		return "", true
	}
	log.Debugf("Detected file %s in client %s.", fileInfo.Name(), cl.name)
	return fileInfo.Name(), false
}

// Returns true if the given fs.FileInfo is a directory or
// if it does not contain the extension that this client is tracking.
func (cl *client) doesNotCareAbout(fileInfo fs.FileInfo, ext string) bool {
	return fileInfo.IsDir() || !strings.HasSuffix(fileInfo.Name(), ext)
}

// database management funcs

// Called on startup to put any files from each client's error directory into the retry queue.
func (cl *client) sortFailures() error {
	// open the error directory and get a list of the files in it
	errDirPath := path.Join(cl.config.Dir, "error")
	errorDir, err := os.Open(errDirPath)
	if err != nil {
		return fmt.Errorf("failed to open error directory: %w", err)
	}
	defer errorDir.Close()
	failedFileNames, err := errorDir.Readdirnames(-1)
	if err != nil {
		return fmt.Errorf("failed to read file names from error directory: %w", err)
	}

	// launch a goroutine that will put each of the error directory's files into the retry queue.
	// not a worker. loops across each file then is done.
	go func() {
		for _, fileName := range failedFileNames {
			cl.addFileToRetryFromError(fileName)
		}
	}()

	return nil
}

// Puts the given file from the error directory into the appropriate retry queue.
func (cl *client) addFileToRetryFromError(fileName string) {
	sendExt := cl.config.Ext
	if cl.config.Tar.Mode == "tar" {
		sendExt = metaArchiveExtension
	}

	// if the file does not match the specified extension, ignore it
	if !strings.Contains(fileName, sendExt) {
		return
	}

	// get the map of servers to which the file was failed to be sent
	failedServersMap, err := cl.db.Get(fileName)
	if err != nil {
		if !errors.Is(err, errDbKeyNotFound) {
			log.Errorf("Could not retrieve entry for %s in ledger database for client %s: %v", fileName, cl.name, err)
			return
		}

		// if the client does not have an entry for this file, then copy the file into the
		// client's source directory so it gets put in the standard queue for all servers.
		// this is a corner case that should not happen in theory
		log.Errorf("%s does not exist in DB. Resending to all servers.", fileName)
		err = os.Rename(path.Join(cl.config.Dir, "error", fileName), path.Join(cl.sendSrcDirPath, fileName))
		if err != nil {
			log.Errorf("Could not move %s from error directory of client %s to source directory: %v", fileName, cl.name, err)
		}

		// add file to tar send queue if
		if cl.config.Tar.Mode == "tar" {
			if !strings.HasSuffix(fileName, metaArchiveExtension) {
				log.Errorf("Current mode is TAR, but found improper extension file in error directory: %s", fileName)
			} else {
				cl.tarQOut <- fileName
			}
		}
		return
	}

	// Only put the file into the retry queue's of the servers that the file had failed to send to.
	log.Tracef("addFailure received %s value for client %s as: %v", fileName, cl.name, failedServersMap)
	for serverName, alreadySuccessfullySent := range failedServersMap {
		if alreadySuccessfullySent { // value of the map is a boolean that is false if the file failed to send to the server. in theory value should never be true. TO DO: use a set, not a map
			return
		}
		log.Debugf("Putting file %s from client %s into the retry queue for server %s.", fileName, cl.name, serverName)
		cl.retryQ[serverName] <- fileName
	}
}

// Interfaces with the client's DB to add an entry to the given file name's
// server-success map for the given server with a false. If the file name
// does not already have a map in the DB, one will be created.
func (cl *client) addFailure(fileName, serverName string) error {
	val, err := cl.db.Get(fileName)
	if err != nil {
		if !errors.Is(err, errDbKeyNotFound) {
			return fmt.Errorf("failed to retrieve entry for file name from DB: %w", err)
		}
		val = make(map[string]bool)
	}

	log.Tracef("addFailure received %s value as: %v\n", fileName, val)
	val[serverName] = false
	log.Tracef("addFailure changed %s value to: %v\n", fileName, val)

	err = cl.db.Set(fileName, val)
	if err != nil {
		return fmt.Errorf("failed to set value in DB: %w", err)
	}
	return nil
}

// Removes the given server from the failed servers map of the given file.
// Returns true if there are no more failed servers in the file's map, false otherwise.
func (cl *client) removeFailure(fileName, serverName string) (noMoreFailedServers bool, err error) {
	// get the failed servers map
	failedServersMap, err := cl.db.Get(fileName)
	if err != nil {
		if errors.Is(err, errDbKeyNotFound) {
			// the file does not have any failed servers in its map, so return true indicating no more failed servers
			return true, nil
		}
		return false, fmt.Errorf("failed to get DB entry for file %s: %w", fileName, err)
	}

	// remove the server from the map of failed servers
	if _, exists := failedServersMap[serverName]; exists {
		delete(failedServersMap, serverName)
		log.Tracef("removeFailure changed %s value to: %v", fileName, failedServersMap)
	}

	// if there are still failed servers left in the map, update the DB entry for the file and return false
	if len(failedServersMap) > 0 {
		err = cl.db.Set(fileName, failedServersMap)
		if err != nil {
			return false, fmt.Errorf("failed to set DB entry for file %s: %w", fileName, err)
		}
		return false, nil
	}

	// if no more failed servers in map, remove the DB entry for the file and return true
	err = cl.db.Remove(fileName)
	if err != nil {
		return true, fmt.Errorf("failed to remove DB entry for file %s: %w", fileName, err)
	}
	return true, nil
}
