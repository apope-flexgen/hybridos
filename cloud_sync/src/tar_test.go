package main

import (
	"crypto/rand"
	"fmt"
	"os"
	"path/filepath"
	"testing"
	"time"
)

func TestCompress(t *testing.T) {
	testDirPath := t.TempDir()
	size := 1000
	n := 10
	files, err := makeFiles(n, size, testDirPath)
	if err != nil {
		t.Fatal(err)
	}

	name, err := compress(files, testDirPath, "test_client", false)
	if err != nil {
		t.Fatal(err)
	}

	fi, err := os.Stat(filepath.Join(testDirPath, metaArchiveDirName, name))
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("compression ratio (%v files @ %v bytes ea.):%v", n, size, float64(fi.Size())/float64(size*n))
	t.Log("")
}

func TestCompressWithValidCSDTNamingFormat(t *testing.T) {
	testDirPath := t.TempDir()
	size := 1000
	n := 10
	files, err := makeFilesWithCSDTFormat(n, size, testDirPath)
	if err != nil {
		t.Fatal(err)
	}

	name, err := compress(files, testDirPath, "test_client", true)
	if err != nil {
		t.Fatal(err)
	}

	fi, err := os.Stat(filepath.Join(testDirPath, metaArchiveDirName, name))
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("compression ratio (%v files @ %v bytes ea.):%v", n, size, float64(fi.Size())/float64(size*n))
	t.Log("")
}
func TestCompressWithInvalidCSDTNamingFormat(t *testing.T) {
	testDirPath := t.TempDir()
	size := 1000
	n := 10
	files, err := makeFiles(n, size, testDirPath)
	if err != nil {
		t.Fatal(err)
	}

	_, err = compress(files, testDirPath, "test_client", true)
	if err.Error() != "filename not in correct format" {
		t.Fatal(err)
	}
	t.Log("compress failed as expected")
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

func makeFilesWithCSDTFormat(n, size int, path string) ([]string, error) {
	files := make([]string, n)

	err := ensureDirectoryExists(path)
	if err != nil {
		return nil, err
	}

	for i := range files {
		name, err := makeFileWithCSDTFormat(size, path)
		if err != nil {
			return nil, err
		}

		files[i] = name
	}

	return files, nil
}

func makeFileWithCSDTFormat(size int, path string) (string, error) {
	err := ensureDirectoryExists(path)
	if err != nil {
		return "", err
	}
	t := time.Now().UnixMicro()
	name := fmt.Sprintf("client__site__device__lane__%v.tar.gz", t)
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
