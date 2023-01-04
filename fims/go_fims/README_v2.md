# Go FIMS package

This package exposes functions for other go programs to connect to FIMS.

## Message Structure

The V2 fims has a different message structure. 
A fixed size Header block is followed by two objects.
A header info block
The message body

No json objects are now used in the fims message.

## Header Block

This contains the sizes of the message fields

```go
type Meta_Data_Info struct {
	Method_len       uint8
	Uri_len          uint8
	Replyto_len      uint8
	Process_name_len uint8
	Username_len     uint8
    Padding1         uint8
    Padding2         uint8
    Padding3         uint8
	Data_len         uint32
}
   
```

## Header Data

This consists of a packed string with ascii data for the header objects.
There are no spaces in the strings , the Header Block data is used to locate the starting offset and length of each string.

```
set/status/bms/MaxDischargefimsTest
```

## String View

This is not a "go" object, but strings are handled using a "string_view" strucure which is pointer to the first char of the string combined with a string length.

This structure means that Null objects are not used to terminate strings and the string size is always availble without having to search for any Null pointers.


## Message Object

The Message or Body  of the message ( is there is any ) is not an unformatted block of data defined by a pointer and a size.






## Example Usage

# message receive

```go

package main

import (
	"fims"
	"fmt"
	"log"
	"time"
)

var fimsReceive chan fims.FimsMsg

var f fims.Fims
var err error

func init() {
	f, err = fims.Connect("Go Program")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	fimsReceive = make(chan fims.FimsMsg)
	f.Subscribe("/go_program")
	go f.ReceiveChannel(fimsReceive)
}

func main() {
	start := time.Now()
	for {
		msg := <-fimsReceive
		if msg.Uri == "/go_program/end" {
			break
		}
		fmt.Printf("From [%s] I got a [%s] containing [%v]. Best send something back to [%s]\n", msg.Uri, msg.Method, msg.Body, msg.Replyto)
		if msg.Replyto != "" {
			f.Send(fims.FimsMsg{
				Method:      "set",
				Uri:         msg.Replyto,
				ProcessName: "mySetTest",
				Body:        msg.Body,
			})
		}
	}

	// show times
	duration := time.Since(start)
	dur := float32(float32(duration.Microseconds()) / 1000.0)
	fmt.Printf(" rx time (mS) %f \n", dur)
}

```
# message send

```go
package main

import (
	"fims"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"time"
)

var f fims.Fims

func readBody() ([]byte, int, error) {
	// Looking for twins.json
	cpath := "test.json"
	count := 1
	var err error
	if len(os.Args) < 1 {
		fmt.Printf("Test file argument [1]  not found. Usage 'go run fims_demo_tx.go <test_file> <test_count>'. Trying current working directory\n")
	}
	if len(os.Args) < 2 {
		fmt.Printf("Count argument [2]  not found. Usage 'go run fims_demo_tx.go <test_file> <test_count>'. Setting count to 1\n")
	}
	if len(os.Args) > 1 {
		cpath = os.Args[1]
	}
	s := "Unknown"
	if len(os.Args) > 2 {
		s = os.Args[2]
		count, err = strconv.Atoi(s)
	}
	if err != nil {
		fmt.Printf("Couldn't decode count %s: %s \n", s, err)
		count = 1
	}
	_, err = os.Stat(cpath)
	if err != nil {
		fmt.Printf("Test file %s err : %s \n", cpath, err)
	} else {
		fmt.Printf("Test file %s \n", cpath)
	}
	if err == nil {
		body, xerr := ioutil.ReadFile(cpath)
		if xerr != nil {
			fmt.Printf("Couldn't read the file %s: %s\n", cpath, xerr)
			err = xerr
		}
		return body, count, err
	}
	return nil, count, err
}

func init() {
	var err error
	f, err = fims.Connect("Go Program")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}
}

func main() {
	x := 0
	body, count, err := readBody()
	b := len(body)
	bs := string(body)
	defer f.Close() // This makes sure the FIMS connection gets closed no matter how the program exits
	if err != nil {
		return
	}
	start := time.Now()
	for x < count {
		f.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    "/go_test",
			Body:   bs,
		})
		x += 1
	}

	// show times
	duration := time.Since(start)
	dur := float32(float32(duration.Microseconds()) / 1000.0)
	total := (x * b)
	rate := float32(total) / (dur / 1000.0)
	fmt.Printf(" sent count %d size %d total_size %d time (mS) %f rate %f \n", x, b, total, dur, rate)
}
```


## Message Verification  (Note this may not have ben ported to 10.2 yet)

Basic message verification can be handled by manually utilizing the `replyto uri` in the sender and sending back some response in the receiver, or reissuing the message if no response is received. However, the verification library can handle this for developers as well and will support verification of multiple messages on the same endpoint all occuring at a similar time. This library works by assigning a unique `id` to each FimsMsg issued for a given endpoint, and storing a record of these messages until a replyto with the matching `id` is received. The responsibilities of the developer in utilizing this functionality will be to:
* configure the library
* call `SendAndVerify()` with a `replyto uri` unique to the message `uri` and the type of verification
* send back any FimsMsg response upon receiving this message in the receiving process
* run a goroutine in the sender that will periodically call `ResendUnverifiedMessages()` to reissue any messages that failed

## Example Usage
```go
// Library configuration
func ConfigureVerification(configuredTimeoutMins int, configuredCallbackAction func(msgRecord map[string]interface{}))
```
* Configuration called before utilization of the library
* `configuredTimeoutMins`: how long to wait for a response for the given `replyto uri` before considering the verification to have failed and sending the message again
* `configuredCallbackAction`: optional user-defined function that will be executed upon successful verification of a message (nil if unused). This function will receive a representation of the FimsMsg record in the form of a map with the following keys:
	```go
	map[string]interface{}{
		"id":         recordEntry.id,			// Type int 		Unique id generated for this message
		"timeSent":   recordEntry.timeSent,		// Type time.Time 	Time at which the message was issued
		"msg":        recordEntry.msg,			// Type FimsMsg		The FimsMsg itself
		"recordType": recordEntry.recordType,	// Type string		The type of message verification, "all" or "latest"
	}
	```
	* One example use of this callback function would be logging as shown below, but the user can define any function they like
		```go
		callbackFunction := func(msg map[string]interface{}) {
			log.Printf("Message for %s issued at %s has been verified\n", msg["msg"].Uri, msg["timeSent"].String())
		}
		```
```go
// Sending the message
func (f *Fims) SendAndVerify(recordType string, msg FimsMsg) (int, error)
```
* Sending function, equivalent to `Fims.Send(msg FimsMsg)`
* `recordType`: the type of message verification. This can either be `"all"`, which will track every message issued for the given `replyto` and resend any that fail, or `"latest"`, which will track only the latest message issued for the given `replyto` and resend only that message if it fails
* `msg`: the FimsMsg to send. For every `uri` there should be a unique `replyto uri`, so multiple messages can use the same `replyto` if they're for the same `uri`, but messages with different `uris` should have different `replytos`
* `returns` the number of bytes written, or any errors that occurred.
	```go
	// Example call
	n, err = f.SendAndVerify("all", fims.FimsMsg{
			Method:  "post",
			Uri:     "/receiver/endpoint",
			Replyto: "/sender/endpoint/verification",
			Body:    msgBody,
		})
	```
```go
// Receiver response
if msg.Replyto != "" {
	fims.Send(FimsMsg{
		Method: "set",
		Uri: msg.Replyto,
		Body: "<any body is sufficient",
	})
}
```
* Example of what a `replyto` response looks like in the receiver
```go
// Reissuing failed messages
unverifiedMessageTicker := time.NewTicker(time.Duration(1) * time.Second)
// Main process loop
for {
	select {
	case <-unverifiedMessageTicker.C:
		err := f.ResendUnverifiedMessages()
		if err != nil {
			log.Println("Failed to reissue sets: ", err)
		}
	}
}
```
* Example of how to reissue any failed messages

Basic statistics in the number of messages issued for verification and the number of messages successfully verified are available for tracking as well, in:
```go
func GetVerificationMessagesIssued() uint
func GetVerificationMessagesVerified() uint
```
It should be noted that these stats will potentially roll over, so it is the responsibility of the user to reset the statistics as needed to ensure reliability:
```go
func ResetVerificationMessageStats()
```

# NEW STUFF (WIP):
## Installation necessary:
	- golang.org/x/sys/unix -> this is what allows for "readv" and "writev" along with other unix functions for working with file descriptors (we are no longer using net.conn -> lower level stuff now)
	- make sure to "go mod init" then "go mod tidy"
	- connect_to_server.go example:
		- run: "go run go_fims/connect_to_server.go" after installing everything for an example
		- IMPORTANT: make sure to "sudo make clean all && sudo make install" to install the new server and libfims stuff
		- then run the server. Otherwise the old server will send bogus data back and won't have a proper handshake
		