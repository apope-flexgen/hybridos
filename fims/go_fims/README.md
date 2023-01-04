# Go FIMS package

This package exposes functions for other go programs to connect to FIMS.

## Message Object

```go
type FimsMsg struct {
	Method  string      `json:"method"`
	Uri     string      `json:"uri,omitempty"`
	Replyto string      `json:"replyto,omitempty"`
	Body    interface{} `json:"body,omitempty"`
	Nfrags  int         `json:"-"`
	Frags   []string    `json:"-"`
}
```
Given a variable `msg` of type `FimsMsg`, Method, Uri and Replyto can be accessed directly as `msg.Method`. The string will always be present in the struct returned from this module, even if empty. The `omitempty` directive omits that field from the marshalled JSON output if empty. `Nfrags` is the number of URI fragments present in the URI string, and `Frags` supplies them as an array (technically a slice) of strings.

`Body` is given as the empty interface (interface{}). To piece apart the body, you will do well to make heavy use of type switches.

```go
switch t := msg.Body.(type) {
case string:
	// It's a string
	fmt.Println(msg.Uri, t)
case float64:
	// All numbers unmarshalled from JSON have a type of float64
	// Convert to int with int(t) or int64(t)
	// Type cast is not required inside the type switch
	// Outside, you have to use int(msg.Body.(float64))
case map[string]interface{}:
	// You've got yourself something like {"value":12,"name":"String"}
case []interface{}:
	// Something of the form [1, 2, 3] (which could be explicitly checked as []float64) or...
	// ["one","two","three"] (could be checked as []string) or...
	// [{"value":1,"string":"Danger, there's a fault"},{"value":8,"string":"Someone turned it off"}]
}
```

## Example Usage

```go
import (
	"fims"
	"fmt"
	"log"
	"time"
)
var fimsReceive chan fims.FimsMsg
var stateTicker, pubTicker *time.Ticker
var f fims.Fims

func init() {
	var err error
	f, err = fims.Connect("Go Program")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}
	fimsReceive = make(chan fims.FimsMsg)
	f.Subscribe("/go_program")
	go f.ReceiveChannel(fimsReceive)
	stateTicker = time.NewTicker(100 * time.Millisecond)
	pubTicker = time.NewTicker(500 * time.Millisecond)
}

func main() {
	defer f.Close() // This makes sure the FIMS connection gets closed no matter how the program exits
	for { // Infinite loop
		select { // Select waits on one of the channels in its case statements to have a value to process
		case <-stateTicker.C:
			// Do whatever you might do in the background, update internal state
		case msg := <-fimsReceive:
			// Receive inputs over FIMS and process
			fmt.Printf("From %v I got a %v containing %v. Best send something back to %v", msg.Uri, msg.Method, msg.Body, msg.Replyto)
			if msg.Replyto != "" {
				f.Send(fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   "Message received",
				})
			}
		case <-pubTicker.C:
			// Time to publish
			f.Send(fims.FimsMsg{
				Method: "pub",
				Uri:    "/go_program",
				Body:   "I'm here and I'm publishing",
			})
		}
	}
}
```


## Message Verification

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
		