// Defines a mock archive
package mock_archive

import (
	"crypto/rand"
	"crypto/sha512"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

// A file with archived data
type Archive struct {
	Name string
	data []byte
}

// Create an archive with randomly generated data bytes
func RandomDataArchive(name string, numDataBytes int) *Archive {
	data := make([]byte, numDataBytes)
	// rand.Read() is documented to always return a nil error
	rand.Read(data)
	return &Archive{
		Name: name,
		data: data,
	}
}

// Write the archive data to a file
func (archive *Archive) WriteToFile(dirPath string) error {
	file, err := os.Create(filepath.Join(dirPath, archive.Name))
	if err != nil {
		return fmt.Errorf("failed to create archive: %w", err)
	}
	defer file.Close()

	size := uint64(len(archive.data))
	err = binary.Write(file, binary.LittleEndian, size)
	if err != nil {
		return fmt.Errorf("failed to write archive data size: %w", err)
	}

	checksum := sha512.Sum512_256(archive.data)
	_, err = file.Write(checksum[:])
	if err != nil {
		return fmt.Errorf("failed to write archive checksum: %w", err)
	}

	_, err = file.Write(archive.data)
	if err != nil {
		return fmt.Errorf("failed to write archive data: %w", err)
	}

	return nil
}

// Read the archive data from a file
func ReadFromFile(filePath string) (*Archive, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return nil, fmt.Errorf("failed to open file: %w", err)
	}
	defer file.Close()

	var readSize uint64
	err = binary.Read(file, binary.LittleEndian, &readSize)
	if err != nil {
		return nil, fmt.Errorf("failed to read archive data size: %w", err)
	}

	var readChecksum [sha512.Size256]byte
	_, err = file.Read(readChecksum[:])
	if err != nil {
		return nil, fmt.Errorf("failed to read archive checksum: %w", err)
	}

	data, err := io.ReadAll(file)
	if err != nil {
		return nil, fmt.Errorf("failed to read archive data: %w", err)
	}

	if len(data) != int(readSize) {
		return nil, fmt.Errorf("archive data size (%d bytes) does not match read data size (%d bytes)", len(data), readSize)
	}

	dataChecksum := sha512.Sum512_256(data)
	if readChecksum != dataChecksum {
		return nil, errors.New("archive data checksum does not match read checksum")
	}

	return &Archive{Name: filepath.Base(file.Name()), data: data}, nil
}
