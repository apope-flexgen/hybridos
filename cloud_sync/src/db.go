package main

import (
	"errors"
	"fmt"
	"math"
	"os"
	"path"
	"path/filepath"
	"sync"
	"time"

	bitcask "git.mills.io/prologic/bitcask"
	log "github.com/flexgen-power/go_flexgen/logger"
)

var errDbKeyNotFound error = errors.New("404")

// Wrapper around a third-party database instance to be used for managing the list of servers
// to which a client's files has failed to be sent.
// Exposed methods allow for reading and writing to the DB as though it uses filenames as keys and
// maps of server names to true-if-sent as values.
// Internally, keys are a combination of filename and server name like ["filename"]servername with true-if-sent as values.
// The keys are formatted this way so that all keys under a given filename can be found with a prefix scan.
type databaseManager struct {
	db      *bitcask.Bitcask
	rwMutex sync.RWMutex // Ensures safe concurrent access (Bitcask Merge() appears to have a synchronization bug)
}

// Opens a database client to write to / read from the key-value store for the given CloudSync client.
// Also starts a goroutine for periodically merging the database data files so we don't have an unnecessary number of files open.
func (manager *databaseManager) UseDB(clientName string) error {
	// verify the directory for this client's key-value store exists
	clientDbDir := path.Join(config.DbDir, clientName)
	err := ensureDirectoryExists(clientDbDir)
	if err != nil {
		return fmt.Errorf("failed to ensure %s client DB directory exists: %w", clientName, err)
	}

	// open database client pointing at the CloudSync client's key-value store
	dbClient, err := bitcask.Open(clientDbDir, bitcask.WithMaxKeySize(math.MaxUint32))
	if err != nil {
		// BitCask has a known issue where the config.json and meta.json can become invalid.
		// The recommendation by the package's author is to simply remove config.json and meta.json in that case.
		// https://git.mills.io/prologic/bitcask/commit/5429693cc8831c19e8f3580c801c5cfc588ceee6

		// try again after removing config.json and meta.json if the error type indicates one of those files is invalid
		switch err.(type) {
		case *bitcask.ErrBadConfig, *bitcask.ErrBadMetadata:
			log.Errorf("Invalid config.json or meta.json detected in %s client internal DB. Attempting to clear invalid DB files.", clientName)
			err = os.Remove(filepath.Join(clientDbDir, "config.json"))
			if err != nil {
				return fmt.Errorf("failed to open %s client DB and failed to clear internal DB config.json: %w", clientName, err)
			}
			err = os.Remove(filepath.Join(clientDbDir, "meta.json"))
			if err != nil {
				return fmt.Errorf("failed to open %s client DB and failed to clear internal DB meta.json: %w", clientName, err)
			}
			dbClient, err = bitcask.Open(clientDbDir, bitcask.WithMaxKeySize(math.MaxUint32))
			if err != nil {
				return fmt.Errorf("failed to open %s DB even after clearing internal DB config.json and meta.json: %w", clientName, err)
			}

		default:
			return fmt.Errorf("failed to open %s client DB: %w", clientName, err)
		}
	}
	manager.db = dbClient

	// Start a routine for periodically merging database data files
	go func() {
		for {
			err = manager.merge()
			if err != nil {
				log.Errorf("Failed to merge database data files for client %s: %v", clientName, err)
			}
			time.Sleep(time.Minute)
		}
	}()

	return nil
}

// Merges database data files, performing cleanup and preventing the number of files from growing too large too quickly
func (manager *databaseManager) merge() error {
	manager.rwMutex.Lock()
	defer manager.rwMutex.Unlock()

	return manager.db.Merge()
}

// Searches the key-value store for keys matching the given file name.
// If none are found, returns errDbKeyNotFound. Otherwise, returns the values
// as a map of server names to whether or not the file was sent.
func (manager *databaseManager) Get(fileName string) (map[string]bool, error) {
	manager.rwMutex.RLock()
	defer manager.rwMutex.RUnlock()

	// scan for all keys under the given filename
	valMap := map[string]bool{}
	keyPrefix := []byte(fmt.Sprintf("[%q]", fileName))
	keysFound := false
	err := manager.db.Scan(keyPrefix, func(key []byte) error {
		keysFound = true
		val, err := manager.db.Get(key)
		if err != nil {
			return fmt.Errorf("failed to get value from DB: %w", err)
		}
		valMap[string(key[len(keyPrefix):])] = (val[0] != 0)
		return nil
	})
	if err != nil {
		return nil, fmt.Errorf("failed to scan DB: %w", err)
	}
	if !keysFound {
		return nil, errDbKeyNotFound
	}
	return valMap, nil
}

// Sets the value of the given filename for all given servers to the given values.
// If a server is not given in the map, then any preexisting key for that server under that filename is removed.
func (manager *databaseManager) Set(fileName string, valMap map[string]bool) error {
	manager.rwMutex.Lock()
	defer manager.rwMutex.Unlock()

	keyPrefixString := fmt.Sprintf("[%q]", fileName)
	keyPrefixBytes := []byte(keyPrefixString)
	// put values for given servers under the given filename
	for serverName, val := range valMap {
		valBytes := []byte{0}
		if val {
			valBytes = []byte{1}
		}
		err := manager.db.Put([]byte(keyPrefixString+serverName), valBytes)
		if err != nil {
			return fmt.Errorf("failed to put key-value into DB: %w", err)
		}
	}
	// scan for keys associated with this filename and whose servers were excluded from the set
	keysToDelete := [][]byte{}
	err := manager.db.Scan(keyPrefixBytes, func(key []byte) error {
		if _, included := valMap[string(key[len(keyPrefixBytes):])]; !included {
			keysToDelete = append(keysToDelete, key)
		}
		return nil
	})
	if err != nil {
		return fmt.Errorf("failed to scan DB: %w", err)
	}
	// remove the keys found in the scan
	for _, key := range keysToDelete {
		err := manager.db.Delete(key)
		if err != nil {
			return fmt.Errorf("failed to delete key: %w", err)
		}
	}
	return nil
}

// Removes the keys for the given filename from the key-value store.
func (manager *databaseManager) Remove(fileName string) error {
	manager.rwMutex.Lock()
	defer manager.rwMutex.Unlock()

	// scan for all keys that match the given filename
	keyPrefixString := fmt.Sprintf("[%q]", fileName)
	keyPrefixBytes := []byte(keyPrefixString)
	keysToDelete := [][]byte{}
	err := manager.db.Scan(keyPrefixBytes, func(key []byte) error {
		keysToDelete = append(keysToDelete, key)
		return nil
	})
	if err != nil {
		return fmt.Errorf("failed to scan DB: %w", err)
	}
	// delete all the keys which were found in the scan
	for _, key := range keysToDelete {
		err := manager.db.Delete(key)
		if err != nil {
			return fmt.Errorf("failed to delete key: %w", err)
		}
	}
	return nil
}
