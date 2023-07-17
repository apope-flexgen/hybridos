// Package idcache provides a set that holds unique IDs.
//
// The package API allows the user to generate new IDs, delete existing IDs, and check if a given ID exists.
package idcache

import (
	"errors"
	"fmt"
	"math"
	"math/rand"
	"strconv"
	"time"
)

// A "set" of unique IDs.
type IdCache map[Id]struct{}

// An ID is a randomly-generated, unique, unsigned integer.
type Id uint16

const maxId Id = math.MaxUint16

// Error returned by the GenerateId function when the cache is at max capacity.
var ErrFullCache = errors.New("cache is full")

// Source seed used to generate new, random IDs.
var randomSrc rand.Source

func init() {
	randomSrc = rand.NewSource(time.Now().UnixNano())
}

// Returns a new, empty ID cache.
func New() IdCache {
	return make(IdCache)
}

// Returns a deep copy of the given IdCache.
func (cache IdCache) Copy() IdCache {
	cp := New()
	for id := range cache {
		cp[id] = struct{}{}
	}
	return cp
}

// Generates a new, unique ID. Adds it to the cache and returns it.
func (cache IdCache) GenerateId() (id Id, err error) {
	// try generating random ID
	for i := 0; i < 1000; i++ {
		id = Id(randomSrc.Int63())
		if !cache.CheckId(id) {
			cache.Add(id)
			return id, nil
		}
	}
	// if random ID generation fails due to large # of existing IDs or statistical anomaly, try brute forcing it.
	id = 0 // need to use int for looping, because if Id/uint16 was used the loop would always overflow and never terminate
	for i := 0; i <= int(maxId); i++ {
		if !cache.CheckId(id) {
			cache.Add(id)
			return id, nil
		}
		id++
	}
	return 0, ErrFullCache
}

// Deletes the given ID from the cache. If the ID does not exist in the cache, nothing will happen.
func (cache IdCache) DeleteId(id Id) {
	delete(cache, id)
}

// Returns true if the given ID exists in the cache, or false if it does not.
func (cache IdCache) CheckId(id Id) bool {
	_, ok := cache[id]
	return ok
}

// Attempts to parse an ID from the string.
func ParseIdFromString(str string) (Id, error) {
	parsedUint16, err := strconv.ParseUint(str, 10, 16)
	return Id(parsedUint16), err
}

// Adds the given IDs to the cache.
//
// Will not check if ID already exists in cache. Use GenerateId if brand new ID is desired.
func (cache IdCache) Add(ids ...Id) {
	for _, id := range ids {
		cache[id] = struct{}{}
	}
}

// Returns true if the two caches have equal lengths and the same elements.
// Otherwise, returns false and a string explaining what was different.
func (cache1 IdCache) Equals(cache2 IdCache) (isEqual bool, reasonNotEqual string) {
	if len(cache1) != len(cache2) {
		return false, fmt.Sprintf("cache1 has length %d but cache2 has length %d", len(cache1), len(cache2))
	}
	for id := range cache1 {
		if _, ok := cache2[id]; !ok {
			return false, fmt.Sprintf("ID %d is in cache1 but not cache2", id)
		}
	}
	return true, ""
}
