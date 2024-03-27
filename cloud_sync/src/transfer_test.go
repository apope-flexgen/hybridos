package main

import (
	"fmt"
	"math"
	"os"
	"path/filepath"
	"testing"
	"time"
)

func TestParseUnixMicro(t *testing.T) {
	// map from filename and extension to expected parsed time
	testTimes := map[[2]string]time.Time{
		{"", ""}:                           {},
		{"test_archive.tar.gz", ".tar.gz"}: {},
		{"test_archive-1673465027557700.tar.gz", ".tar.gz"}:                  time.UnixMicro(1673465027557700),
		{"test_archive-0.tar.gz", ".tar.gz"}:                                 time.UnixMicro(0),
		{"test_archive-" + fmt.Sprint(math.MaxInt64) + ".tar.gz", ".tar.gz"}: time.UnixMicro(math.MaxInt64),
		{"test_1673465027557700_archive-1734981827779410.tar.gz", ".tar.gz"}: time.UnixMicro(1734981827779410),
		{"test_1" + fmt.Sprint(math.MaxInt64) + ".tar.gz", ".tar.gz"}:        {},
		{"test_archive-1257894000000000.other", ".other"}:                    time.UnixMicro(1257894000000000),
	}

	for params, expectedTime := range testTimes {
		// we only expect the time to be an empty struct upon not finding a timestamp
		expectedFound := expectedTime != time.Time{}
		timeParsed, found := parseUnixMicro(params[0], params[1])
		if found != expectedFound {
			t.Errorf("Parse result found was %v but expected found was %v for test case params %v", found, expectedFound, params)
		}
		if timeParsed != expectedTime {
			t.Errorf("Parsed time was %v but expected time was %v for test case params %v", timeParsed, expectedTime, params)
		}
	}
}

// Test that sorted backups function and that retentions applied to them work as well
func TestSortedBackup(t *testing.T) {
	var err error
	testDir := t.TempDir()
	serverDir := filepath.Join(testDir, "server")
	err = os.Mkdir(serverDir, os.ModePerm)
	if err != nil {
		t.Fatalf("Failed to create server dir: %v", err)
	}
	clientDir := filepath.Join(testDir, "client")
	err = os.Mkdir(clientDir, os.ModePerm)
	if err != nil {
		t.Fatalf("Failed to create client dir: %v", err)
	}
	serv := &server{
		config: ServerConfig{
			Dir:                 serverDir,
			Sorted:              true,
			SortedRetentionDays: 30,
		},
	}
	cl := &client{
		sendSrcDirPath:   clientDir,
		connectedServers: []*server{serv},
	}

	// loop over several "hours"
	testTime := time.Now()
	for hoursTicked := 0; hoursTicked < 365*24; hoursTicked++ {
		// create a file timestamped with the hour
		fileName := fmt.Sprint(testTime.UnixMicro()) + ".tar.gz"
		file, err := os.Create(filepath.Join(clientDir, fileName))
		if err != nil {
			t.Fatalf("Failed creation of file %s: %v", fileName, err)
		}
		file.Close()
		err = removeOldestExpiredDatedDir(clientDir, serv.config.SortedRetentionDays, testTime)
		if err != nil {
			t.Fatalf("Failed applying retention")
		}
		err, _ = sortedCopy(cl, serv, clientDir, fileName)
		if err != nil {
			t.Fatalf("Failed sorted copy of file %s: %v", fileName, err)
		}

		// check that the file exists and that subdirectories older than the retention don't exist
		year, month, day := testTime.Date()
		_, err = os.Stat(filepath.Join(serverDir, fmt.Sprintf("%d-%d-%d", year, int(month), day), fileName))
		if os.IsNotExist(err) {
			t.Fatalf("File %s was not found in server dir", fileName)
		}
		dir, err := os.Open(serverDir)
		if err != nil {
			t.Fatalf("Failed to open server directory: %v", err)
		}
		subDirNames, err := dir.Readdirnames(-1)
		if err != nil {
			t.Fatalf("Failed to read file names from server directory: %v", err)
		}
		dir.Close()
		for _, subDirName := range subDirNames {
			subDirDate, err := time.Parse("2006-1-2", subDirName)
			if err != nil {
				t.Fatalf("Failed to parse subdir name for date: %v", err)
			}
			if testTime.Sub(subDirDate) > time.Duration(serv.config.SortedRetentionDays)*24*time.Hour {
				t.Fatalf("Found subdir %s older than retention", subDirName)
			}
		}
	}
}
