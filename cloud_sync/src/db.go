package main

import (
	"bytes"
	"encoding/gob"
	"errors"
	"fmt"
	"math"
	"os"
	"path"
	"path/filepath"
	"strings"
	"sync"
	"time"

	bitcask "git.mills.io/prologic/bitcask"
	log "github.com/flexgen-power/go_flexgen/logger"
)

var errDbKeyNotFound error = errors.New("404")

// Wrapper around a third-party database instance to be used for managing the list of servers
// to which a client's files has failed to be sent.
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

// Searches the key-value store for a key matching the given file name.
// If not found, returns errDbKeyNotFound. Otherwise, returns the value
// as a map.
func (manager *databaseManager) Get(fileName string) (map[string]bool, error) {
	manager.rwMutex.RLock()
	defer manager.rwMutex.RUnlock()

	valBytes, err := manager.db.Get([]byte(fileName))
	if err != nil {
		if errors.Is(err, bitcask.ErrKeyNotFound) {
			return nil, errDbKeyNotFound
		}
		return nil, fmt.Errorf("failed to get key's value from DB: %w", err)
	}
	valMap, err := bytesToMap(valBytes)
	if err != nil {
		return nil, fmt.Errorf("failed to convert value from bytes to map: %w", err)
	}
	return valMap, nil
}

// Converts the given map to bytes then sets the value of the given key
// matching the given file name to the bytified map in the key-value store.
func (manager *databaseManager) Set(fileName string, valMap map[string]bool) error {
	manager.rwMutex.Lock()
	defer manager.rwMutex.Unlock()

	valBytes, err := mapToBytes(valMap)
	if err != nil {
		return fmt.Errorf("failed to convert map to bytes: %w", err)
	}

	err = manager.db.Put([]byte(fileName), valBytes)
	if err != nil {
		return fmt.Errorf("failed to put key-value into DB: %w", err)
	}
	return nil
}

// Removes the given key from the key-value store.
func (manager *databaseManager) Remove(key string) error {
	manager.rwMutex.Lock()
	defer manager.rwMutex.Unlock()

	err := manager.db.Delete([]byte(key))
	if err != nil {
		if strings.Contains(err.Error(), "key not found") {
			return errDbKeyNotFound
		}
	}
	return nil
}

// Converts a bytified map back to a normal map.
func bytesToMap(by []byte) (map[string]bool, error) {
	decodedMap := map[string]bool{}
	bytesBuff := bytes.NewBuffer(by)
	err := gob.NewDecoder(bytesBuff).Decode(&decodedMap)
	if err != nil {
		return nil, err
	}
	return decodedMap, nil
}

// Converts a map to a byte representation for storing in a key-value pair.
func mapToBytes(mp map[string]bool) ([]byte, error) {
	buf := bytes.Buffer{}
	err := gob.NewEncoder(&buf).Encode(mp)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
