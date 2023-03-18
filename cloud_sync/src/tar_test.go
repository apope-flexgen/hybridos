package main

import (
	"crypto/rand"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

var dir = "/home/vagrant/.cloud_sync/test/data"

func TestCompress(t *testing.T) {
	size := 1000
	n := 10
	files, err := makeFiles(n, size, dir)
	if err != nil {
		t.Fatal(err)
	}

	name, err := compress(files, dir, "test_client")
	if err != nil {
		t.Fatal(err)
	}

	fi, err := os.Stat(filepath.Join(dir, metaArchiveDirName, name))
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("compression ratio (%v files @ %v bytes ea.):%v", n, size, float64(fi.Size())/float64(size*n))
	t.Log("")
}

func makeFiles(n, size int, path string) ([]string, error) {
	files := make([]string, n)

	err := ensureDirectoryExists(path)
	if err != nil {
		return nil, err
	}

	for i := range files {
		name, err := makeFile(i, size, path)
		if err != nil {
			return nil, err
		}

		files[i] = name
	}

	return files, nil
}

func makeFile(n, size int, path string) (string, error) {
	err := ensureDirectoryExists(path)
	if err != nil {
		return "", err
	}

	name := fmt.Sprintf("archive_%v.tar.gz", n)

	data := make([]byte, size)
	rand.Read(data)

	file, err := os.Create(filepath.Join(path, name))
	if err != nil {
		return "", fmt.Errorf("failed to create archive: %w", err)
	}
	defer file.Close()

	_, err = file.Write(data)
	if err != nil {
		return "", fmt.Errorf("failed to write archive data: %w", err)
	}

	return name, nil
}
