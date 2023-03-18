// IMPORTANT NOTE: this test file is not guaranteed to use the same scp function as cloud_sync. It's possible for the cloud_sync scp function to be updated without also updating this test program.
// TODO: This test file should eventually be changed to use the actual scp function exported from cloud_sync, but right now I just want to get something quick set up.
// Usage: go run test_scp.go srcDir destDir fileName
package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
	"path"
	"path/filepath"
	"time"

	"golang.org/x/crypto/ssh"
	"golang.org/x/crypto/ssh/knownhosts"
)

type sshPipe struct {
	in  io.WriteCloser
	out io.Reader
	err io.Reader // currently unused
}

func main() {
	args := os.Args[1:]
	fmt.Printf("Arguments: %v\n", args)

	pipes, err := setup()
	if err != nil {
		fmt.Printf("Failed setup: %v\n", err)
		return
	}

	// Test the scp back and forth repeatedly (since we're on the same machine we can do this)
	for i := 0; i < 3; i++ {
		// Test scping to
		fmt.Printf("Scp from %s to %s\n", args[0], args[1])
		err, retryable := scp(pipes, args[0], args[1], args[2])
		if err != nil {
			fmt.Printf("Failed to scp, retryable: %t, err:, %v\n", retryable, err)
		} else {
			fmt.Printf("SUCCESS\n")
		}
		fmt.Println()

		// Test scping back
		fmt.Printf("Scp from %s to %s\n", args[1], args[0])
		err, retryable = scp(pipes, args[1], args[0], args[2])
		if err != nil {
			fmt.Printf("Failed to scp, retryable: %t, err: %v\n", retryable, err)
		} else {
			fmt.Printf("SUCCESS\n")
		}
		fmt.Println()
	}
}

// hardcoded ssh session setup
func setup() (*sshPipe, error) {
	buf, err := os.ReadFile("/home/vagrant/.ssh/id_ed25519")
	if err != nil {
		return nil, fmt.Errorf("failed to read private key: %w", err)
	}

	signer, err := ssh.ParsePrivateKey(buf)
	if err != nil {
		return nil, fmt.Errorf("failed to parse private key: %w", err)
	}

	knownHostsCallback, err := knownhosts.New("/home/vagrant/.ssh/known_hosts")
	if err != nil {
		return nil, fmt.Errorf("could not create knownhost callback: %w", err)
	}

	sshConf := ssh.ClientConfig{
		User:            "vagrant",
		Auth:            []ssh.AuthMethod{ssh.PublicKeys(signer)},
		HostKeyCallback: knownHostsCallback,
		Timeout:         time.Second * time.Duration(10),
	}
	sshCl, err := ssh.Dial("tcp", net.JoinHostPort("172.16.1.80", "22"), &sshConf)
	if err != nil {
		return nil, fmt.Errorf("failed to dial: %w", err)
	}

	session, err := sshCl.NewSession() // start new session
	if err != nil {
		return nil, fmt.Errorf("failed to create new session: %w", err)
	}

	in, err := session.StdinPipe() // create session input pipe
	if err != nil {
		return nil, fmt.Errorf("failed to create stdin pipe: %w", err)
	}

	out, err := session.StdoutPipe()
	if err != nil {
		return nil, fmt.Errorf("failed to create stdout pipe: %w", err)
	}

	er, err := session.StderrPipe()
	if err != nil {
		return nil, fmt.Errorf("failed to create stderr pipe: %w", err)
	}

	return &sshPipe{
		in:  in,
		out: out,
		err: er,
	}, session.Shell()
}

// Sends the given file to the destination belonging to the given pipes with the
// given destination file path using Secure Copy Protocol (SCP).
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

	// COPY TO STDIN
	// initiate scp with input pipe (-t tells scp that it was invoked by another scp instance and that it will be receiving files)
	_, err = fmt.Fprintf(pipes.in, "scp -qt %s\n", filepath.Join(destDirPath, fileName))
	if err != nil {
		return fmt.Errorf("failed to initiate scp: %w", err), true
	}

	// Handle scp response
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("scp response after remote scp initiated: %w", err), true
	}

	// scp header
	_, err = fmt.Fprintf(pipes.in, "C0644 %d %s\n", srcStat.Size(), fileName)
	if err != nil {
		return fmt.Errorf("failed to feed scp header to input pipe: %w", err), true
	}

	// After this point, if we encounter an error or finish successfully, try to exit the remote scp
	defer func() {
		// Try to exit the remote scp
		exitErr := exitRemoteScp(pipes)
		// Wrap the outer function's returned error with the failure to exit
		if exitErr != nil {
			err = fmt.Errorf("failed to exit with error: %v, after tried to exit with err: %w", exitErr, err)
		}
	}()

	// Handle scp response
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("scp response after scp header had error: %w", err), true
	}

	// copy source file to input pipe
	if srcStat.Size() > 0 {
		_, err = io.Copy(pipes.in, src)
		if err != nil {
			return fmt.Errorf("failed to copy source file to input pipe: %w", err), true
		}
	}

	// end file
	_, err = fmt.Fprint(pipes.in, "\x00")
	if err != nil {
		return fmt.Errorf("failed to end file: %w", err), true
	}

	// Handle scp response
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("scp response after file end had error: %w", err), true
	}

	return nil, true
}

func exitRemoteScp(pipes *sshPipe) error {
	// E and newline indicates remote scp should exit
	_, err := fmt.Fprint(pipes.in, "E\n")
	if err != nil {
		return fmt.Errorf("failed to exit remote scp: %w", err)
	}

	// Handle scp response
	err = parseScpResponse(pipes)
	if err != nil {
		return fmt.Errorf("scp response after indicating exit remote scp had error: %w", err)
	}
	return nil
}

func parseScpResponse(pipes *sshPipe) error {
	// HANDLE STDOUT
	// read first byte which is either 0 (ok), 1 (warning), or 2 (error)
	buf := make([]uint8, 1)
	_, err := pipes.out.Read(buf)
	if err != nil {
		return fmt.Errorf("failed to read first byte of output pipe: %w", err)
	}

	// if 0 (ok), done
	if buf[0] == 0 {
		return nil
	}

	// if warning/error, read warning/error message
	bufferedReader := bufio.NewReader(pipes.out)
	warnOrErrString, err := bufferedReader.ReadString('\n')
	if err != nil {
		return fmt.Errorf("failed to read warn/error string: %w", err)
	}

	// if warning or error, return error message
	if buf[0] == 1 { // warning
		return fmt.Errorf("scp failed with warning msg: %s", warnOrErrString)
	} else if buf[0] == 2 { // error
		return fmt.Errorf("scp failed with error msg: %s", warnOrErrString)
	}

	return fmt.Errorf("scp responded with unexpected first byte (expected error code of 0, 1, or 2): %X", buf[0])
}
