package main

import (
	"archive/tar"
	"bufio"
	"compress/gzip"
	"fmt"
	"io"
	"os"
	"path"
	"path/filepath"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

const metaArchiveDirName = "meta_archives"
const metaArchiveExtension = ".metatargz"
const unmetaArchiveDirName = "unmeta_archives"
const tarBufferSize = 1000000

// Creates a metaarchive of the given files in the source directory and places the resulting
// metaarchive into a metaarchive subdirectory
func compress(fileNames []string, srcDirPath, clientName string) (string, error) {
	destDir := path.Join(srcDirPath, metaArchiveDirName)
	err := ensureDirectoryExists(destDir)
	if err != nil {
		return "", fmt.Errorf("could not ensure that %s exists: %w", destDir, err)
	}

	tarName := fmt.Sprintf("%s_%d%s", clientName, time.Now().UnixMicro(), metaArchiveExtension)

	dest, err := os.Create(path.Join(destDir, tarName))
	if err != nil {
		return "", fmt.Errorf("could not create destination tarball in %s: %w", destDir, err)
	}
	defer dest.Close()

	// buffer file writing
	destBuffered := bufio.NewWriterSize(dest, tarBufferSize)
	defer destBuffered.Flush()

	gzw := gzip.NewWriter(dest)
	defer gzw.Close()

	tw := tar.NewWriter(gzw)
	defer tw.Close()

	for _, fileName := range fileNames {
		err = compressSingleFile(tw, fileName, srcDirPath)
		if err != nil {
			log.Errorf("Failed to compress file %s in %s: %v", fileName, srcDirPath, err)
		}
	}

	return tarName, nil
}

// Writes the given file to the given metaarchive tar writer
func compressSingleFile(tw *tar.Writer, fileName string, srcDirPath string) error {
	file, err := os.Open(path.Join(srcDirPath, fileName))
	if err != nil {
		return fmt.Errorf("failed to open file %s/%s: %w", srcDirPath, fileName, err)
	}
	defer file.Close()

	stat, err := file.Stat()
	if err != nil {
		return fmt.Errorf("failed to open file %s/%s: %w", srcDirPath, fileName, err)
	}

	header := &tar.Header{
		Name:    fileName,
		Size:    stat.Size(),
		Mode:    0644,
		ModTime: stat.ModTime(),
	}

	if err := tw.WriteHeader(header); err != nil {
		return fmt.Errorf("failed to write header for %s: %w", fileName, err)
	}

	if _, err := io.Copy(tw, file); err != nil {
		return fmt.Errorf("failed to copy contents of file %s to tar writer: %w", fileName, err)
	}

	return nil
}

// Recursively extract a metaarchive file until you get individual archives at the destination.
// The original file is not deleted, and nested metaarchives are also not deleted.
// If an error is returned, this function will still return a list of the files that were successfully extracted for handling or cleanup.
func decompress(metaArchiveFileName string, srcDirPath string, destDirPath string) (fileNames []string, err error) {
	fileNames = []string{}

	err = ensureDirectoryExists(destDirPath)
	if err != nil {
		return fileNames, fmt.Errorf("could not ensure that %s exists: %w", destDirPath, err)
	}

	// open tar file
	file, err := os.Open(filepath.Join(srcDirPath, metaArchiveFileName))
	if err != nil {
		return fileNames, fmt.Errorf("failed to open tar file %s: %w", metaArchiveFileName, err)
	}
	defer file.Close()

	// create decompressing file reader
	fileReader, err := gzip.NewReader(file)
	if err != nil {
		return fileNames, fmt.Errorf("failed to create gzip reader: %w", err)
	}
	defer fileReader.Close()
	// add tar filter to the reader
	tarBallReader := tar.NewReader(fileReader)

	// loop over all files in the tar
	for {
		fileHeader, err := tarBallReader.Next()
		if err != nil {
			if err == io.EOF {
				break // only successful loop exit
			}
			return fileNames, fmt.Errorf("failure reading data from tar reader: %w", err)
		}

		err = decompressSingleFile(tarBallReader, fileHeader, destDirPath)
		if err != nil {
			log.Errorf("Failed to extract file %s: %v", fileHeader.Name, err)
			continue
		}

		fileNames = append(fileNames, fileHeader.FileInfo().Name())
	}

	// for any metaarchive files extracted, recursively extract
	for _, fileName := range fileNames {
		if strings.HasSuffix(fileName, metaArchiveExtension) {
			nestedFileNames, err := decompress(fileName, destDirPath, destDirPath)
			fileNames = append(fileNames, nestedFileNames...)
			if err != nil {
				return fileNames, fmt.Errorf("failed to extract nested metaarchive %s: %w", fileName, err)
			}
		}
	}

	return fileNames, nil
}

// Extracts the given file from the given metaarchive reader into the destination directory
func decompressSingleFile(tarBallReader *tar.Reader, fileHeader *tar.Header, destDirPath string) error {
	if fileHeader.Typeflag != tar.TypeReg {
		return fmt.Errorf("unable to untar type %c in file %s within tar file", fileHeader.Typeflag, fileHeader.Name)
	}

	// try to create file at destination
	destFilePath := filepath.Join(destDirPath, fileHeader.FileInfo().Name())
	destFile, err := os.Create(destFilePath)
	if err != nil {
		return fmt.Errorf("failed to create file %s at destination: %w", fileHeader.Name, err)
	}
	defer destFile.Close()

	// copy data from tar reader into file at destination
	_, err = io.Copy(destFile, tarBallReader)
	if err != nil {
		os.Remove(destFilePath)
		return fmt.Errorf("failed to copy file %s: %w", fileHeader.Name, err)
	}
	return nil
}
