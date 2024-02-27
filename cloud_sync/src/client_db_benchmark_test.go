package main

import (
	"fmt"
	"io/fs"
	"os"
	"path"
	"path/filepath"
	"testing"
)

// Benchmarks for the client functions which interface with the client DB

// number of files generated in error dir and/or DB for tests that fill the error dir and/or DB
const numFilesInError = 128

func BenchmarkAddFileToRetryFromError(b *testing.B) {
	cl, err := createTestClient(b.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		b.Fatal(err)
	}

	// benchmark processing many files from error when DB has no entries
	manyFilesWhenDBEmpty := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			clearFiles(b, cl)
			generateFiles(b, cl)
			b.StartTimer()
			errDirPath := path.Join(cl.config.Dir, "error")
			errorDir, err := os.Open(errDirPath)
			if err != nil {
				b.Fatalf("Failed to open error directory: %v.", err)
			}
			failedFileNames, err := errorDir.Readdirnames(-1)
			if err != nil {
				b.Fatalf("Failed to read file names from error directory: %v.", err)
			}
			errorDir.Close()
			for _, fileName := range failedFileNames {
				cl.addFileToRetryFromError(fileName)
			}
		}
	}
	b.Run("manyFilesWhenDBEmpty", manyFilesWhenDBEmpty)

	// benchmark processing many files from error when each has an entry in the DB
	addEntriesToDB(b, cl)
	manyFilesWhenDBFull := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			clearFiles(b, cl)
			generateFiles(b, cl)
			b.StartTimer()
			errDirPath := path.Join(cl.config.Dir, "error")
			errorDir, err := os.Open(errDirPath)
			if err != nil {
				b.Fatalf("Failed to open error directory: %v.", err)
			}
			failedFileNames, err := errorDir.Readdirnames(-1)
			if err != nil {
				b.Fatalf("Failed to read file names from error directory: %v.", err)
			}
			errorDir.Close()
			for _, fileName := range failedFileNames {
				cl.addFileToRetryFromError(fileName)
				// clear retry queues so nothing blocks
				<-cl.retryQ[testServerName1]
				<-cl.retryQ[testServerName2]
				<-cl.retryQ[testServerName3]
			}
		}
	}
	b.Run("manyFilesWhenDBFull", manyFilesWhenDBFull)
}

func BenchmarkAddFailure(b *testing.B) {
	cl, err := createTestClient(b.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		b.Fatal(err)
	}

	testFileName := "testFile.tar.gz"

	// benchmark adding first failure for a file when DB is empty
	firstFailureDBEmpty := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			cl.removeFailure(testFileName, testServerName1)
			b.StartTimer()
			err := cl.addFailure(testFileName, testServerName1)
			if err != nil {
				b.Fatal(err)
			}
		}
	}
	b.Run("firstFailureDBEmpty", firstFailureDBEmpty)

	// benchmark adding second failure for a file when DB is otherwise empty
	err = cl.addFailure(testFileName, testServerName1)
	if err != nil {
		b.Fatal(err)
	}
	secondFailureDBEmpty := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			cl.removeFailure(testFileName, testServerName2)
			b.StartTimer()
			err = cl.addFailure(testFileName, testServerName2)
			if err != nil {
				b.Fatal(err)
			}
		}
	}
	b.Run("secondFailureDBEmpty", secondFailureDBEmpty)

	// benchmark adding first failure for a file when DB is populated
	err = cl.db.db.DeleteAll()
	if err != nil {
		b.Fatal(err)
	}
	_, testFileName = addEntriesToDB(b, cl) // ensure test file is a file without an entry
	firstFailureDBPopulated := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			cl.removeFailure(testFileName, testServerName1)
			b.StartTimer()
			err = cl.addFailure(testFileName, testServerName1)
			if err != nil {
				b.Fatal(err)
			}
		}
	}
	b.Run("firstFailureDBPopulated", firstFailureDBPopulated)

	// benchmark adding second failure for a file when DB is populated
	err = cl.addFailure(testFileName, testServerName1)
	if err != nil {
		b.Fatal(err)
	}
	secondFailureDBPopulated := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			cl.removeFailure(testFileName, testServerName2)
			b.StartTimer()
			err = cl.addFailure(testFileName, testServerName2)
			if err != nil {
				b.Fatal(err)
			}
		}
	}
	b.Run("secondFailureDBPopulated", secondFailureDBPopulated)

}

func BenchmarkRemoveFailure(b *testing.B) {
	cl, err := createTestClient(b.TempDir(), []string{testServerName1, testServerName2, testServerName3})
	if err != nil {
		b.Fatal(err)
	}

	testFileName := "testFile.tar.gz"

	// benchmark removing failure for a file with multiple failures when DB is otherwise empty
	err = cl.addFailure(testFileName, testServerName1)
	if err != nil {
		b.Fatal(err)
	}
	multipleFailuresDBEmpty := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			err = cl.addFailure(testFileName, testServerName2)
			if err != nil {
				b.Fatal(err)
			}
			b.StartTimer()
			cl.removeFailure(testFileName, testServerName2)
		}
	}
	b.Run("multipleFailuresDBEmpty", multipleFailuresDBEmpty)

	// benchmark removing failure for a file with one failure when DB is otherwise empty
	err = cl.db.db.DeleteAll()
	if err != nil {
		b.Fatal(err)
	}
	singleFailureDBEmpty := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			err = cl.addFailure(testFileName, testServerName1)
			if err != nil {
				b.Fatal(err)
			}
			b.StartTimer()
			cl.removeFailure(testFileName, testServerName1)
		}
	}
	b.Run("singleFailureDBEmpty", singleFailureDBEmpty)

	// benchmark removing failure for a file with multiple failures when DB is populated
	err = cl.db.db.DeleteAll()
	if err != nil {
		b.Fatal(err)
	}
	testFileName, _ = addEntriesToDB(b, cl) // ensure test file is a file with an entry
	cl.removeFailure(testFileName, testServerName1)
	multipleFailuresDBPopulated := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			err = cl.addFailure(testFileName, testServerName1)
			if err != nil {
				b.Fatal(err)
			}
			b.StartTimer()
			cl.removeFailure(testFileName, testServerName1)
		}
	}
	b.Run("multipleFailuresDBPopulated", multipleFailuresDBPopulated)

	// benchmark removing failure for a file with one failure when DB is populated
	cl.removeFailure(testFileName, testServerName1)
	cl.removeFailure(testFileName, testServerName2)
	cl.removeFailure(testFileName, testServerName3)
	singleFailuresDBPopulated := func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			b.StopTimer()
			err = cl.addFailure(testFileName, testServerName1)
			if err != nil {
				b.Fatal(err)
			}
			b.StartTimer()
			cl.removeFailure(testFileName, testServerName1)
		}
	}
	b.Run("singleFailuresDBPopulated", singleFailuresDBPopulated)

}

// Generate files in the client error dir
func generateFiles(b *testing.B, cl *client) {
	for i := 0; i < numFilesInError; i++ {
		fileName := filepath.Join(cl.config.Dir, "error", fmt.Sprintf("file%v.tar.gz", i))
		file, err := os.Create(fileName)
		if err != nil {
			b.Fatal(err)
		}
		file.Close()
	}
}

// Adds entries to DB simulating a bunch of files with failures.
// Returns a filename which was added to the DB and a filename which was not added to the DB.
func addEntriesToDB(b *testing.B, cl *client) (usedFileName, unusedFileName string) {
	for i := 0; i < numFilesInError; i++ {
		fileName := fmt.Sprintf("file%v.tar.gz", i)
		err := cl.addFailure(fileName, testServerName1)
		if err != nil {
			b.Fatal(err)
		}
		err = cl.addFailure(fileName, testServerName2)
		if err != nil {
			b.Fatal(err)
		}
		err = cl.addFailure(fileName, testServerName3)
		if err != nil {
			b.Fatal(err)
		}
	}
	return fmt.Sprintf("file%v.tar.gz", 0), "testFile.tar.gz"
}

// Clear files from client input and error dirs
func clearFiles(b *testing.B, cl *client) {
	err := filepath.WalkDir(cl.config.Dir, func(path string, d fs.DirEntry, pathErr error) (err error) {
		if pathErr != nil {
			return fmt.Errorf("invalid path error: %w", pathErr)
		}
		if path == cl.config.Dir {
			return nil
		} else if d.IsDir() && path != cl.config.Dir {
			return fs.SkipDir
		} else {
			return os.Remove(path)
		}
	})
	if err != nil {
		b.Fatal(err)
	}
}
