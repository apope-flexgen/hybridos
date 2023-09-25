package fileops

import (
	"errors"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"syscall"
)

//check if file or Directory exist
//returns true if exist and false otherwise
func Exists(name string) bool {
	_, err := os.Stat(name)
	return !os.IsNotExist(err)
}

// Checks if the given directory exists. If it does not exist, then creates the directory
// with the given permissions (before umask is applied) along with any necessary parent directories.
func EnsureDirectoryExists(directoryPath string, permissions fs.FileMode) error {
	_, err := os.Stat(directoryPath)
	if err == nil {
		return nil
	}
	if !errors.Is(err, fs.ErrNotExist) {
		return fmt.Errorf("failed to get stat: %w", err)
	}

	err = os.MkdirAll(directoryPath, permissions)
	if err != nil {
		return fmt.Errorf("failed to make directory: %w", err)
	}
	return nil
}

//writes current pid to file provided
//if file already exist, overwrites content of it
func CreatePidFile(filename string) error {
	fp, err := os.OpenFile(filename, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0644)
	if err != nil {
		return err
	}
	defer fp.Close()
	//strconv.Itoa(pid)
	pidstr := fmt.Sprintf("%d", os.Getpid())
	fp.WriteString(pidstr)
	return nil
}

//returns Inode of file pointed by FileInfo struct
func GetInode(finfo *os.FileInfo) (uint64, error) {
	if *finfo == nil {
		return 0, fmt.Errorf("FileInfo is null")
	}
	f_sys := (*finfo).Sys()
	return f_sys.(*syscall.Stat_t).Ino, nil
}

/*
 * Creates copy of the src file contents to destination filename
 * Input source filename including path
 * destination filename full path
 * returns if there is any error or nil
 */
func Copy(srcFileName string, destFileName string) error {
	//return error if src file doesnt exist
	if !Exists(srcFileName) {
		return fmt.Errorf("src file %s doesnt exist", srcFileName)
	}
	//open src file
	srcFile, err := os.Open(srcFileName) // open source file
	if err != nil {
		return fmt.Errorf("failed to open src file %s : %w", srcFileName, err)
	}
	defer srcFile.Close()
	//get the basefile of src
	srcBaseFile := filepath.Base(srcFileName)

	//check if destination is a directory or file.
	var dirPath string = ""
	var fullDestName string = ""
	if destFileName[len(destFileName)-1] == '/' {
		//provided directory so creating a filename same as src
		dirPath = destFileName
		fullDestName = filepath.Join(dirPath, srcBaseFile)
	} else {
		//provided file, copy the contents of src
		dirPath = filepath.Dir(destFileName)
		fullDestName = destFileName
	}

	//check if directory exist and if not create one
	if !Exists(dirPath) {
		err := os.MkdirAll(dirPath, 0755)
		if err != nil {
			return fmt.Errorf("failed to create destination directory %s : %w", dirPath, err)
		}
	} else {
		//check if the file exist as well and ensure its not directory
		if Exists(fullDestName) {
			//make sure its not a directory.
			f_info, err := os.Stat(fullDestName)
			if err != nil {
				return fmt.Errorf("failed to retrieve FileInfo about destination file %s : %w", fullDestName, err)
			}
			if f_info.IsDir() {
				//provided destination already exist as directory.
				//create a file in that directory
				dirPath = fullDestName
				fullDestName = filepath.Join(fullDestName, srcBaseFile)
			}
		}
	}

	//create dest file
	destFile, err := os.OpenFile(fullDestName, os.O_CREATE|os.O_WRONLY, 0644) // create the destination file
	if err != nil {
		return fmt.Errorf("failed to create destination file %s : %w", fullDestName, err)
	}
	defer destFile.Close()

	_, err = io.Copy(destFile, srcFile) // copy contents of src archive to dest
	if err != nil {
		return fmt.Errorf("failed to write src file to destination: %w", err)
	}
	return nil
}
