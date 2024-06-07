package dts

import (
	"archive/tar"
	"bytes"
	"compress/gzip"
	"context"
	"fmt"
	"io"
	"os"
	"strings"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// Stage which extracts data files from archives
type ArchiveExtractor struct {
	in  <-chan string // incoming channel of archive file paths
	Out chan dataFile // outgoing channel of data files to be processed
}

// Struct for an in-memory data file
type dataFile struct {
	archiveFilePath string // filepath of the archive the datafile was extracted from
	name            string // name of extracted data file
	data            []byte
}

func NewArchiveExtractor(inputChannel <-chan string) *ArchiveExtractor {
	return &ArchiveExtractor{
		in:  inputChannel,
		Out: make(chan dataFile),
	}
}

// Start extractor routines
func (extractor *ArchiveExtractor) Start(group *errgroup.Group, groupContext context.Context) (startUpError error) {
	for n := 0; n < GlobalConfig.NumExtractWorkers; n++ {
		group.Go(func() error { return extractor.extractUntil(groupContext.Done()) })
	}
	return nil
}

// Loop that extracts incoming files
func (extractor *ArchiveExtractor) extractUntil(done <-chan struct{}) error {
	for {
		select {
		case <-done:
			goto termination
		case archiveFilePath := <-extractor.in:
			log.Debugf("[extract] received file %s", archiveFilePath)

			// extract data files
			dataFiles := []dataFile{}
			if strings.HasSuffix(archiveFilePath, ".batchpqtgz") {
				// extract data files if batched archive
				extractedDataFiles, err := extract(archiveFilePath)
				if err != nil {
					log.Errorf("Failed to extract data files from batched archive %s: %v", archiveFilePath, err)
					err = removeArchive(archiveFilePath, true, GlobalConfig.FailedValidatePath)
					if err != nil {
						log.Errorf("Failed to remove archive %s: %v", archiveFilePath, err)
					}
					continue
				}
				dataFiles = append(dataFiles, extractedDataFiles...)
			} else {
				// read bytes from file if using fims_codec
				df, err := readDataFile(archiveFilePath)
				if err != nil {
					log.Errorf("Failed to read archive %s: %v", archiveFilePath, err)
					err = removeArchive(archiveFilePath, true, GlobalConfig.FailedValidatePath)
					if err != nil {
						log.Errorf("Failed to remove archive %s: %v", archiveFilePath, err)
					}
					continue
				}
				dataFiles = append(dataFiles, df)
			}

			// remove archive now that it has been read
			err := removeArchive(archiveFilePath, false, GlobalConfig.FailedValidatePath)
			if err != nil {
				log.Errorf("Failed to remove archive %s: %v", archiveFilePath, err)
			}

			// output data files
			for _, df := range dataFiles {
				select { // cancellable send
				case <-done:
					goto termination
				case extractor.Out <- df:
				}
			}
		}
	}

termination:
	log.MsgInfo("Extractor worker terminating")
	return nil
}

// Extract data files from the batched archive at the given path
func extract(archiveFilePath string) ([]dataFile, error) {
	// open archive file
	archiveFile, err := os.Open(archiveFilePath)
	if err != nil {
		return nil, fmt.Errorf("failed to open batched archive %s: %w", archiveFilePath, err)
	}
	defer archiveFile.Close()

	// create decompressing file reader
	gzReader, err := gzip.NewReader(archiveFile)
	if err != nil {
		return nil, fmt.Errorf("failed to create batched archive gzip reader: %w", err)
	}
	defer gzReader.Close()
	// add tar filter to the reader
	tarBallReader := tar.NewReader(gzReader)

	// extract the contents of each file in the archive
	dataFiles := []dataFile{}
	for {
		fileHeader, err := tarBallReader.Next()
		if err != nil {
			if err == io.EOF {
				break // only successful loop exit
			}
			return nil, fmt.Errorf("failure reading data from batched archive tar reader: %w", err)
		}

		if fileHeader.Typeflag != tar.TypeReg {
			log.Errorf("Unable to untar type %c in file %s within batched archive %s", fileHeader.Typeflag, fileHeader.Name, archiveFilePath)
			continue
		}

		// read data from tar reader into buffer until EOF is found (handled by ReadFrom method)
		dataBuff := new(bytes.Buffer)
		_, err = dataBuff.ReadFrom(tarBallReader)
		if err != nil {
			log.Errorf("Failed to read contents of file %s within archive: %v", fileHeader.Name, err)
			continue
		}
		dataFiles = append(dataFiles, dataFile{
			archiveFilePath: archiveFilePath,
			name:            fileHeader.Name,
			data:            dataBuff.Bytes(),
		})
	}
	return dataFiles, nil
}

// Read bytes of data file at given path
func readDataFile(archiveFilePath string) (dataFile, error) {
	file, err := os.Open(archiveFilePath)
	if err != nil {
		return dataFile{}, fmt.Errorf("failed to open archive file at %s: %w", archiveFilePath, err)
	}

	data, err := io.ReadAll(file)
	if err != nil {
		return dataFile{}, fmt.Errorf("failed to read bytes of archive file at %s: %w", archiveFilePath, err)
	}

	return dataFile{
		archiveFilePath: archiveFilePath,
		name:            file.Name(),
		data:            data,
	}, nil
}
