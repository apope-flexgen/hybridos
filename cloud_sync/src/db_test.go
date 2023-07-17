package main

import (
	"fmt"
	"os"
	"testing"
)

func TestDB(t *testing.T) {
	config.DbDir = t.TempDir()
	err := os.RemoveAll(config.DbDir)
	if err != nil {
		t.Fatal(err)
	}

	manager := &databaseManager{}

	err = manager.UseDB("test1")
	if err != nil {
		t.Fatal(err)
	}
	t.Log("DB: success")

	err = manager.Set("file0", map[string]bool{"aux1": false, "aux2": true})
	if err != nil {
		t.Fatal(err)
	}
	t.Log("Set: success")

	val, err := manager.Get("file0")
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("Get: success - value is %v", val)

	err = manager.Remove("file0")
	if err != nil {
		t.Fatal(err)
	}
	t.Log("Remove: success")

	val, err = manager.Get("file0")
	if val != nil {
		t.Error(val)
	}
	t.Log(err)
}

func TestPopulateThenExtractDB(t *testing.T) {
	config.DbDir = t.TempDir()
	manager := &databaseManager{}

	err := manager.UseDB("test2")
	if err != nil {
		t.Fatal(err)
	}
	// populate

	for i := 0; i < 100; i++ {
		err = manager.Set(fmt.Sprintf("file%v", i), map[string]bool{"server1": true, "server2": false})
		if err != nil {
			t.Fatal(err)
		}
	}

	// extract

	for i := 0; i < 100; i++ {
		val, err := manager.Get(fmt.Sprintf("file%v", i))
		if err != nil {
			t.Fatal(err)
		}

		if !val["server1"] || val["server2"] {
			t.Fatal(val)
		}
		t.Logf("file%d: %v", i, val)
	}

	for i := 0; i < 100; i++ {
		err = manager.Remove(fmt.Sprintf("file%v", i))
		if err != nil {
			t.Fatal(err)
		}
	}
}
