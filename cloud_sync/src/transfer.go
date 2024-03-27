// Defines the client-server transfer functionality

package main

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"io"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/service/s3/s3manager"
	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/pkg/sftp"
)

// Request for a specific file to be transferred
type transferRequest struct {
	fileName        string
	srcDirPath      string
	responseChannel chan transferResponse // Channel owned by the requester (made and closed by requester) that the transfer handler responds on
}

// Result of servicing a transfer request
type transferResponse struct {
	err            error // Any error encountered in transfer
	retryable      bool  // If an err occurred, whether or not the err is due to a temporary connection issue and the transfer can be retried
	connectionOkay bool  // Indicates if the request was rejected because the connection was down
}

// Start a transfer stage
func (serv *server) runTransfer(cl *client) (startupError error) {
	sendRequests, ok := cl.sendRequestQsToServers[serv.name]
	if !ok {
		return fmt.Errorf("failed to start transfer routine, client-server send request channel doesn't exist from %s to %s", cl.name, serv.name)
	}
	retryRequests, ok := cl.retryRequestQsToServers[serv.name]
	if !ok {
		return fmt.Errorf("failed to start transfer routine, client-server retry request channel doesn't exist from %s to %s", cl.name, serv.name)
	}
	go serv.transfer(cl, sendRequests, retryRequests)
	return nil
}

// Handles transfer requests for a particular client-server connection, keeps track of failures
func (serv *server) transfer(cl *client, sendRequests <-chan transferRequest, retryRequests <-chan transferRequest) {
	numFailures := 0
	connectionOkay := true
	var connectionReestablished <-chan struct{} // channel used to indicate the connection being fixed after a disconnect
	enterBadConnectionMode := func() {
		// switch to bad connection mode and start asynchronously fixing the connection
		connectionOkay = false
		connectionReestablished = serv.asyncReestablishConnection(cl)
	}

	// initialize connection
	err := serv.initConnection(cl)
	if err != nil {
		log.Errorf("Failed to initialize connection to server %s for client %s: %v", serv.name, cl.name, err)
		enterBadConnectionMode()
	}

	// loop for handling incoming requests
	for {
		// if connection is bad, reject or ignore requests until it is recreated
		for !connectionOkay {
			select {
			case <-connectionReestablished:
				connectionOkay = true
				numFailures = 0
			case request := <-sendRequests:
				// all send requests are immediately rejected
				request.responseChannel <- transferResponse{err: errors.New("connection is faulty"), retryable: true, connectionOkay: false}
				// don't handle retry requests when connection is bad, they will be handled on reconnect
			}
		}

		var request transferRequest
		// wait on next request, prioritize send requests over retries
		select {
		case request = <-sendRequests:
		default:
			// if there are retry requests, prioritize those over starting the keep-alive timer
			select {
			case request = <-sendRequests:
			case request = <-retryRequests:
			default:
				// if there are no requests, start keep-alive timer if server is remote and connected. Then, wait for requests or timer tick
				var keepAliveTimeChan <-chan time.Time = nil
				var keepAliveTimer *time.Timer = nil
				if serv.config.IP != "" && connectionOkay {
					// send ping 100ms earlier than necessary for some leniency in case ping timer started late
					keepAliveTimer = time.NewTimer(time.Second*time.Duration(serv.config.Timeout) - 100*time.Millisecond)
					keepAliveTimeChan = keepAliveTimer.C
				}
				select {
				case request = <-sendRequests:
				case request = <-retryRequests:
				case <-keepAliveTimeChan:
					// if we receive no requests for some time, send a keep-alive to prevent idle timeout
					_, _, err := serv.sshClients[cl.name].SendRequest("keep-alive@cloud_sync", true, []byte{'1'})
					if err != nil {
						log.Errorf("Error sending keep-alive from %s to %s: %v.", cl.name, serv.name, err)
						enterBadConnectionMode()
					}
					continue
				}
				// clean up timer if we got a request before timer ticked
				if keepAliveTimer != nil && !keepAliveTimer.Stop() {
					<-keepAliveTimeChan
				}
			}
		}

		// service request and respond to requester
		err, retryable := serv.handleTransferRequest(cl, request)
		request.responseChannel <- transferResponse{err: err, retryable: retryable, connectionOkay: true}

		// check if we've had too many failures
		if err != nil {
			log.Errorf("Error transferring %s from %s to %s: %v.", request.fileName, cl.name, serv.name, err)
			numFailures++
			if numFailures >= config.RetryLimit {
				enterBadConnectionMode()
			}
		}
	}
}

// Services a single transfer request from a client
func (serv *server) handleTransferRequest(cl *client, request transferRequest) (err error, retryable bool) {
	if serv.config.Bucket != "" {
		// S3 transfer
		log.Debugf("S3 uploading %s to %s from %s.", request.fileName, serv.name, cl.name)
		return uploadS3(serv.s3Uploader, request.fileName, request.srcDirPath, serv.config.Bucket, serv.config.Dir, time.Second*time.Duration(serv.config.Timeout))
	} else if serv.config.IP != "" {
		if serv.config.UseSFTP {
			// remote server transfer with SFTP
			log.Debugf("SFTPing %s to %s from %s.", request.fileName, serv.name, cl.name)
			return uploadSFTP(serv.sftpClients[cl.name], request.srcDirPath, serv.config.Dir, request.fileName)
		} else {
			// remote server transfer with SCP
			log.Debugf("SCPing %s to %s from %s.", request.fileName, serv.name, cl.name)
			return scp(serv.sshPipes[cl.name], request.srcDirPath, serv.config.Dir, request.fileName)
		}
	} else if serv.config.Sorted {
		// local servers can optionally have their files sorted by day
		log.Debugf("Sorted local copy %s to %s from %s.", request.fileName, serv.name, cl.name)
		err := removeOldestExpiredDatedDir(serv.config.Dir, serv.config.SortedRetentionDays, time.Now())
		if err != nil {
			log.Errorf("Failed to apply archive retention to %s", serv.name)
		}
		return sortedCopy(cl, serv, request.srcDirPath, request.fileName)
	} else {
		// if not sorted, local servers get simple copy
		log.Debugf("Local copy %s to %s from %s.", request.fileName, serv.name, cl.name)
		return copy(request.srcDirPath, serv.config.Dir, request.fileName)
	}
}

// local transfer funcs

// Copies the given file into a directory that is named with the year-month-day date on which the copy occurs.
// The server's configured directory is the parent directory for the dated directories.
func sortedCopy(cl *client, serv *server, srcDirPath string, fileName string) (err error, retryable bool) {
	// sort by parsed timestamp if one is found, otherwise use current time
	timestamp, found := parseUnixMicro(fileName, cl.config.Ext)
	if !found {
		timestamp = time.Now()
	}
	year, month, day := timestamp.Date()
	destinationDirPath := filepath.Join(serv.config.Dir, fmt.Sprintf("%d-%d-%d", year, int(month), day))
	err = ensureDirectoryExists(destinationDirPath)
	if err != nil {
		return fmt.Errorf("failed to ensure that the destination directory %s exists: %w", destinationDirPath, err), false
	}

	err, retryable = copy(srcDirPath, destinationDirPath, fileName)
	if err != nil {
		return fmt.Errorf("failed to perform copy of file: %w", err), retryable
	}

	return nil, true
}

// Parses a Unix microsecond epoch from a filename for sorting purposes.
func parseUnixMicro(fileName string, fileExtension string) (timeStamp time.Time, timeFound bool) {
	// look for a string of digits followed by the file extension
	matcher, err := regexp.Compile("\\d+" + fileExtension)
	if err != nil {
		// this error should never happen
		log.Errorf("Failed to compile regular expression for parsing timestamp from filename with extension %s.", fileExtension)
		return time.Time{}, false
	}
	match := matcher.FindString(fileName)
	if match == "" {
		return time.Time{}, false
	}
	digits := strings.TrimSuffix(match, fileExtension)
	unixMicros, err := strconv.Atoi(digits)
	if err != nil {
		log.Errorf("Failed to parse timestamp number %s from filename %s with extension %s.", digits, fileName, fileExtension)
		return time.Time{}, false
	}
	return time.UnixMicro(int64(unixMicros)), true
}

// Removes the oldest directory in a directory of dated subdirectories
// where the age of a subdirectory is determined by the date in its name.
// The current time is given as a parameter for ease of testing.
func removeOldestExpiredDatedDir(dirPath string, retentionDays int, now time.Time) error {
	dir, err := os.Open(dirPath)
	if err != nil {
		return fmt.Errorf("failed to open directory: %w", err)
	}
	defer dir.Close()
	// get all file names directly under the given directory
	fileNames, err := dir.Readdirnames(-1)
	if err != nil {
		return fmt.Errorf("failed to read file names from directory: %w", err)
	}

	// search the filenames for the oldest date
	oldestFileName := ""
	var oldestDate time.Time
	datedFileNameFound := false
	for _, fileName := range fileNames {
		date, err := time.Parse("2006-1-2", fileName) // date layout must match layout used in sorted copy
		if err != nil {
			continue
		}

		if !datedFileNameFound {
			datedFileNameFound = true
			oldestFileName = fileName
			oldestDate = date
		} else if date.Before(oldestDate) {
			oldestFileName = fileName
			oldestDate = date
		}
	}
	if !datedFileNameFound {
		return nil
	}

	// if the oldest subdirectory is expired, then remove it
	if now.Sub(oldestDate) > time.Duration(retentionDays)*24*time.Hour {
		err := os.RemoveAll(filepath.Join(dirPath, oldestFileName))
		if err != nil {
			return fmt.Errorf("failed to remove file: %w", err)
		}
	}

	return nil
}

// Copies the contents of the given file into a new file with the given destination file path.
func copy(srcDirPath string, destDirPath string, fileName string) (err error, retryable bool) {
	source, err := os.Open(filepath.Join(srcDirPath, fileName))
	if err != nil {
		return fmt.Errorf("failed to open source file: %w", err), false
	}
	defer source.Close()

	destination, err := os.Create(filepath.Join(destDirPath, fileName))
	if err != nil {
		return fmt.Errorf("failed to create destination file: %w", err), false
	}
	defer destination.Close()
	_, err = io.Copy(destination, source)
	if err != nil {
		return fmt.Errorf("failed to copy contents of source file to destination file: %w", err), false
	}
	return nil, false
}

// network transfer funcs

// Sends the given file to the destination belonging to the given pipes with the
// given destination file path using Secure Copy Protocol (SCP).
// This function relies on the internals of the SCP protocol. If you want to know more, see the scp documentation in docs.
// The modification time of the source file is not preserved.
func scp(pipes *sshPipe, srcDirPath string, destDirPath string, fileName string) (err error, retryable bool) {
	if pipes.in == nil || pipes.out == nil {
		return fmt.Errorf("ssh pipes are nil"), true
	}

	// open source file
	src, err := os.Open(path.Join(srcDirPath, fileName))
	if err != nil {
		return fmt.Errorf("failed to open source file: %w", err), false
	}
	defer src.Close()

	// get source file info for later
	srcStat, err := src.Stat()
	if err != nil {
		return fmt.Errorf("failed to get source file stat: %w", err), false
	}

	// initiate scp with input pipe (-t tells scp that it was invoked by another scp instance and that it will be receiving files)
	_, err = fmt.Fprintf(pipes.in, "scp -qt %s\n", filepath.Join(destDirPath, fileName))
	if err != nil {
		return fmt.Errorf("failed to initiate remote scp process: %w", err), true
	}

	// handle remote scp response indicating remote scp has started
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("after remote scp process was initiated, it responded with an error: %w", err), true
	}

	// input the scp header
	_, err = fmt.Fprintf(pipes.in, "C0644 %d %s\n", srcStat.Size(), fileName)
	if err != nil {
		return fmt.Errorf("failed to feed scp header to input pipe: %w", err), true
	}

	// after this point, if we encounter an error or finish successfully, try to exit the remote scp
	defer func() {
		// try to exit the remote scp
		exitErr := exitRemoteScp(pipes)
		// wrap the outer function's returned error with the failure to exit
		if exitErr != nil {
			err = fmt.Errorf("failed to exit remote scp with error: %v, after tried to exit remote scp due to error: %w", exitErr, err)
		}
	}()

	// handle remote scp response after remote scp has parsed header and potentially tried to create the file
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("after we sent an scp header, the remote scp responded with an error: %w", err), true
	}

	// copy source file to input pipe
	if srcStat.Size() > 0 {
		_, err = io.Copy(pipes.in, src)
		if err != nil {
			return fmt.Errorf("failed to copy source file to ssh input pipe: %w", err), true
		}
	}

	// end file
	_, err = fmt.Fprint(pipes.in, "\x00")
	if err != nil {
		return fmt.Errorf("failed to send end of file byte: %w", err), true
	}

	// handle remote scp response after end of file
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("after we sent an end of file byte, the remote scp reponded with an error: %w", err), true
	}

	return nil, true
}

// Tries to tell the remote scp process that it should exit
func exitRemoteScp(pipes *sshPipe) error {
	// the signal "E\n" indicates the remote scp should exit
	_, err := fmt.Fprint(pipes.in, "E\n")
	if err != nil {
		return fmt.Errorf("failed to send exit signal to remote scp: %w", err)
	}

	// handle remote scp response to exit signal
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("after we gave an exit signal, encountered error when trying to receive response: %w", err)
	}
	return nil
}

// Parses response from the remote scp process to check for errors
func parseScpResponse(pipes *sshPipe) error {
	// read first byte which is either 0 (ok), 1 (warning), or 2 (error)
	buf := make([]uint8, 1)
	_, err := pipes.out.Read(buf)
	if err != nil {
		return fmt.Errorf("failed to read first byte of ssh output pipe: %w", err)
	}

	// if 0 (ok), done
	if buf[0] == 0 {
		return nil
	}

	// if warning/error, read warning/error message
	bufferedReader := bufio.NewReader(pipes.out)
	warnOrErrString, err := bufferedReader.ReadString('\n')
	if err != nil {
		return fmt.Errorf("failed to read warn/error string from ssh output pipe: %w", err)
	}

	// if warning or error, return error message
	if buf[0] == 1 { // warning
		return fmt.Errorf("scp failed with warning msg: %s", warnOrErrString)
	} else if buf[0] == 2 { // error
		return fmt.Errorf("scp failed with error msg: %s", warnOrErrString)
	}

	return fmt.Errorf("remote scp responded with unexpected first byte (expected error code of 0, 1, or 2): %X", buf[0])
}

// S3 upload transfer:
//
// "uploader" is the already connected S3 uploader.
//
// "filename" of file and the "srcDir" directory it resides in.
//
// "bucket" name of destination bucket and "destDir" directory within bucket.
//
// "timeout" duration for hanging requests.
func uploadS3(uploader *s3manager.Uploader, filename, srcDir, bucket, destDir string, timeout time.Duration) (err error, retryable bool) {
	if uploader == nil {
		return fmt.Errorf("s3 uploader does not exist"), true
	}

	file, err := os.Open(filepath.Join(srcDir, filename))
	if err != nil {
		return fmt.Errorf("could not open %s: %w", filename, err), false
	}

	ctx, cancel := context.WithTimeout(aws.BackgroundContext(), timeout)
	defer cancel()

	res, err := uploader.UploadWithContext(ctx, &s3manager.UploadInput{
		Body:   file,
		Bucket: aws.String(bucket),
		Key:    aws.String(filepath.Join(destDir, filename)),
	})
	if err != nil {
		return fmt.Errorf("upload failed on %s: %w", filename, err), true
	}

	log.Tracef("result of S3: %s", res.Location)
	return err, true
}

// Sends the given file to the given destination using SFTP protocol.
func uploadSFTP(sftpCl *sftp.Client, srcDirPath string, destDirPath string, fileName string) (err error, retryable bool) {
	if sftpCl == nil {
		return fmt.Errorf("sftp client is nil"), true
	}

	// open source file
	srcFile, err := os.Open(path.Join(srcDirPath, fileName))
	if err != nil {
		// the only case where the copy cannot be retried is if the source file has a problem,
		// otherwise success depends on the connection
		return fmt.Errorf("failed to open source file: %w", err), false
	}
	defer srcFile.Close()

	// create destination file
	destFilePath := filepath.Join(destDirPath, fileName)
	destFile, err := sftpCl.OpenFile(destFilePath, os.O_WRONLY|os.O_CREATE|os.O_TRUNC) // use OpenFile() because Create() is not supported by all servers
	if err != nil {
		return fmt.Errorf("failed to open remote file: %w", err), true
	}
	// defer close on remote file and handle potential close error
	defer func() {
		closeErr := destFile.Close()
		// wrap the outer function's returned error with the failure to close
		if closeErr != nil {
			err = fmt.Errorf("failed to close remote file with error: %v, after tried to close remote file due to error: %w", closeErr, err)
		}
	}()

	// copy source file contents to remote file
	_, err = io.Copy(destFile, srcFile)
	if err != nil {
		return fmt.Errorf("failed to copy source file to remote file: %w", err), true
	}

	return nil, true
}
