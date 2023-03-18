# What We Know (Or Guess) About the SCP Protocol
The following all applies to `scp` when used to transfer a single file from local to remote.

The `scp` tool works by starting an ssh session to the remote, invoking an `scp -t` on the remote machine which listens for incoming data, and then communicating with the remote process through the ssh session to transfer the file.

The `-t` option is meant for internal use only, and thus has essentially no documentation (hence the existence of this document). What we know, or think we know, about how to interact with `scp -t` is largely based off of examining the source code of the OpenSSH SCP implementation and testing our own attempts to use the `-t` option.

At different times, the remote `scp -t` process will send the local `scp` process a message. The message type is determined by the first byte of the message:
* "\x00" indicates an ok, proceed as normal
* "\x01" indicates a warning, the process should be exited
* "\x02" indicates an error, the process should be exited

We're not sure what the difference between "\0x01" and "\0x02" really is.

If we only care about sending one file and don't care about directories or preserving timestamps, then the remote `scp -t` process can be summarized by the following pseudo-code:
```go
sendToSource("\x00") // First remote response: Start by sending the local scp an ok signal
for {
    sawControlRecordE := checkForE() // If we got an "E\n" then leave the loop
    if sawControlRecordE {
        break // This is the only exit from the loop, other than fatal errors
    }
    header, err := readAndParseHeader() // Header has form: "C<file permissions> <num bytes in file> <file name>\n"
    if err != nil {
        fatalError(err)
    }

    // Try to open or create file
    err = openFile(destDir, header)
    sendOkOrErrToSource(err) // Second remote response 
    if err != nil {
        continue
    }

    copyFileDataFromSource(header)
    err = expectOkFromSource() // local scp should send "\x00" after file contents have finished sending
    sendOkOrErrToSource(err) // Third remote response
}
```