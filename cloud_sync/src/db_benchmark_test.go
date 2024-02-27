package main

import (
	"fmt"
	"testing"
)

// Benchmarks for functions that interface directly with the DB

var sampleFailedServerMap = map[string]bool{
	"test_server1": false,
	"test_server2": false,
	"test_server3": false,
}
var expandedSampleFailedServerMap = map[string]bool{
	"test_server1": false,
	"test_server2": false,
	"test_server3": false,
	"test_server4": false,
}
var reducedSampleFailedServerMap = map[string]bool{
	"test_server1": false,
	"test_server2": false,
}

func BenchmarkGet(b *testing.B) {
	config.DbDir = b.TempDir()

	manager := &databaseManager{}

	err := manager.UseDB("test")
	if err != nil {
		b.Fatal(err)
	}

	// benchmark failed gets when DB is empty
	emptyFails := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_, err := manager.Get(fmt.Sprintf("file%v", i))
			if err != errDbKeyNotFound {
				b.Fatalf("Error was %v instead of the expected DB key not found error", err)
			}
		}
	}
	b.Run("emptyFails", emptyFails)

	// populate DB
	usedKeys, unusedKeys := manager.populate()

	// benchmark successful gets when DB is populated
	populatedSuccesses := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_, err := manager.Get(usedKeys[i%len(usedKeys)])
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
		}
	}
	b.Run("populatedSuccesses", populatedSuccesses)

	// benchmark failed gets when DB is populated
	populatedFails := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_, err := manager.Get(unusedKeys[i%len(unusedKeys)])
			if err != errDbKeyNotFound {
				b.Fatalf("Error was %v instead of the expected DB key not found error", err)
			}
		}
	}
	b.Run("populatedFails", populatedFails)
}

func BenchmarkSet(b *testing.B) {
	config.DbDir = b.TempDir()

	manager := &databaseManager{}

	err := manager.UseDB("test")
	if err != nil {
		b.Fatal(err)
	}

	// benchmark sets on new keys when DB is empty
	emptyNew := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			err := manager.Set(fmt.Sprintf("file%v", i), sampleFailedServerMap)
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
		}
	}
	b.Run("emptyNew", emptyNew)

	usedKeys, unusedKeys := manager.populate()

	// benchmark sets on new keys when DB is populated
	populatedNew := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			key := unusedKeys[i%len(unusedKeys)]
			err := manager.Set(key, sampleFailedServerMap)
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
			b.StopTimer()
			manager.Remove(key)
			b.StartTimer()
		}
	}
	b.Run("populatedNew", populatedNew)

	// benchmark sets on used keys to add data when DB is populated
	populatedUsedAddData := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			key := usedKeys[i%len(usedKeys)]
			err := manager.Set(key, expandedSampleFailedServerMap)
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
			b.StopTimer()
			manager.Set(key, sampleFailedServerMap)
			b.StartTimer()
		}
	}
	b.Run("populatedUsedAddData", populatedUsedAddData)

	// benchmark sets on used keys to add data when DB is populated
	populatedUsedRemoveData := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			key := usedKeys[i%len(usedKeys)]
			err := manager.Set(key, reducedSampleFailedServerMap)
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
			b.StopTimer()
			manager.Set(key, sampleFailedServerMap)
			b.StartTimer()
		}
	}
	b.Run("populatedUsedRemoveData", populatedUsedRemoveData)
}

func BenchmarkRemove(b *testing.B) {
	config.DbDir = b.TempDir()

	manager := &databaseManager{}

	err := manager.UseDB("test")
	if err != nil {
		b.Fatal(err)
	}

	// benchmark removes on unused keys when DB is empty
	emptyUnused := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			err := manager.Remove(fmt.Sprintf("file%v", i))
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
		}
	}
	b.Run("emptyUnused", emptyUnused)

	usedKeys, unusedKeys := manager.populate()

	// benchmark removes on used keys when DB is populated
	populatedUsed := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			key := usedKeys[i%len(usedKeys)]
			err := manager.Remove(key)
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
			b.StopTimer()
			manager.Set(key, sampleFailedServerMap)
			b.StartTimer()
		}
	}
	b.Run("populatedUsed", populatedUsed)

	// benchmark removes on unused keys when DB is populated
	populatedUnused := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			key := usedKeys[i%len(unusedKeys)]
			err := manager.Remove(key)
			if err != nil {
				b.Fatalf("Error was %v instead of nil", err)
			}
		}
	}
	b.Run("populatedUnused", populatedUnused)
}

// Populates the DB and returns lists of distinct used and unused keys
func (manager *databaseManager) populate() (usedKeys []string, unusedKeys []string) {
	numKeysToGenerate := 256
	generatedKeys := make([]string, numKeysToGenerate)
	for i := 0; i < numKeysToGenerate; i++ {
		s := fmt.Sprintf("file%v", i)
		generatedKeys[i] = s
	}

	usedKeys = generatedKeys[0 : numKeysToGenerate/2]
	unusedKeys = generatedKeys[numKeysToGenerate/2 : numKeysToGenerate]

	for _, key := range usedKeys {
		manager.Set(key, sampleFailedServerMap)
	}

	return usedKeys, unusedKeys
}
