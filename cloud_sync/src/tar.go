package main

import (
	"archive/tar"
	"bufio"
	"compress/gzip"
	"fmt"
	"io"
	"math"
	"os"
	"path"
	"path/filepath"
	"strconv"
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
func compress(fileNames []string, srcDirPath, clientName string, UseCSDTNamingFormat bool) (string, error) {
	destDir := path.Join(srcDirPath, metaArchiveDirName)
	err := ensureDirectoryExists(destDir)
	if err != nil {
		return "", fmt.Errorf("could not ensure that %s exists: %w", destDir, err)
	}
	var tarName string
	// If UseCSDTNamingFormat flag is true then create the tar file with the name in ("<client_name>"__"<site_name>"__"<device_name>"__"<lane_name>"__"<16-digit timestamp>"."ext") format
	if UseCSDTNamingFormat {
		// Find the latest timestamp out of all the files to use in the final name
		latestTimestamp := int64(math.MinInt64)
		clientNameForFile := clientName
		siteName := ""
		deviceName := ""
		laneName := ""
		for _, fileName := range fileNames {
			var fileTimestamp int64
			clientNameForFile, siteName, deviceName, laneName, fileTimestamp, err = parseFilenameCSDT(fileName)
			if err != nil {
				return "", fmt.Errorf("failed to compress file %s: %w", fileName, err)
			}
			if fileTimestamp > latestTimestamp {
				latestTimestamp = fileTimestamp
			}
		}
		tarName = fmt.Sprintf("%s__%s__%s__%s__%d%s", clientNameForFile, siteName, deviceName, laneName, latestTimestamp, metaArchiveExtension)
	} else {
		tarName = fmt.Sprintf("%s_%d%s", clientName, time.Now().UnixMicro(), metaArchiveExtension)
	}

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

// Takes a filename which may follow the format of client__site__device__lane__timestamp.ext and parses out the file origin data
// Returns an error if the filename fails to parse.
func parseFilenameCSDT(fileName string) (clientName string, siteName string, deviceName string, laneName string, fileTimestamp int64, err error) {
	// Split the filename on the extension
	baseFileNameSplit := strings.Split(fileName, ".")
	if len(baseFileNameSplit) == 1 {
		return clientName, siteName, deviceName, laneName, fileTimestamp, fmt.Errorf("file extension not present")
	}

	// Keep the base name of the file after removing the extension
	baseFileName := baseFileNameSplit[0]
	filenameSplits := strings.Split(baseFileName, "__")
	if len(filenameSplits) != 5 {
		return clientName, siteName, deviceName, laneName, fileTimestamp, fmt.Errorf("filename not in correct format")
	}

	clientName = filenameSplits[0]
	siteName = filenameSplits[1]
	deviceName = filenameSplits[2]
	laneName = filenameSplits[3]
	fileTimestampStr := filenameSplits[4]
	fileTimestamp, err = strconv.ParseInt(fileTimestampStr, 10, 64)
	if err != nil {
		return clientName, siteName, deviceName, laneName, fileTimestamp, fmt.Errorf("failed to parse file timestamp: %w", err)
	}
	return clientName, siteName, deviceName, laneName, fileTimestamp, nil
}
