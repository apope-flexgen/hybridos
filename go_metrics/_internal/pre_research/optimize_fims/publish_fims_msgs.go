package research

import (
	// "fmt"
	"fims"
	"log"
	"math/rand"
	"time"
)

// var uriArr = [15]string{"ess", "bms", "pcs", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"} //uris that are subscribed to
// var uriArr = [1]string{"/echo_test_1"}
var uriArr = [6]string{"/echo_test_1", "/echo_test_2", "/echo_test_3", "/echo_test_4", "/echo_test_5", "/echo_test_6"} //uris that are subscribed to

// var fimsMethodArr = [1]string{"set"}
// var fimsMethodArr = [1]string{"get"}
var fimsMethodArr = [3]string{"set", "get", "pub"}

// var fimsMethodArr = [5]string{"set", "get", "post", "del", "pub"}
// var fimsMethodArr = [6]string{"set", "get", "post", "del", "pub", "sub"}

var clientUri = "echo_publish"

func main() {

	f, err := fims.Connect(clientUri)
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	for {

		//Get random uri
		rand.Seed(time.Now().UnixNano())
		index := 0
		largestArrIndex := len(uriArr) - 1
		if largestArrIndex != 0 {
			index = rand.Intn(len(uriArr) - 1)
		}
		randUri := uriArr[index]

		//Get random FimsMsg Method
		rand.Seed(time.Now().UnixNano())
		index = 0
		largestArrIndex = len(fimsMethodArr) - 1
		if largestArrIndex != 0 {
			index = rand.Intn(len(fimsMethodArr) - 1)
		}
		randFimsMethod := fimsMethodArr[index]

		//Delay on new fims messages into the system
		// var tsleep = (time.Duration(10) * time.Millisecond) //- elapsed
		// time.Sleep(tsleep)

		//Data for the fims send
		s := 1
		// s := fmt.Sprintf("Hello Kitty, Set # %d", count)

		fimsMsg := fims.FimsMsg{}

		switch { //switch statement that changes what the contents of the fims message are based on randomized fims method
		case randFimsMethod == "set":
			rand.Seed(time.Now().UnixNano())
			num := rand.Intn(1)

			switch { //switch statement that allows for random fims sets that either have replyTo or don't
			case num == 0: //fims set without reply to
				fimsMsg = fims.FimsMsg{
					Method: randFimsMethod,
					Uri:    randUri,
					Body:   s,
				}
			case num == 1: //fims set with reply to
				fimsMsg = fims.FimsMsg{
					Method:  randFimsMethod,
					Uri:     randUri,
					Body:    s,
					Replyto: clientUri,
				}
			}

		case randFimsMethod == "get":
			fimsMsg = fims.FimsMsg{
				Method:  randFimsMethod,
				Uri:     randUri,
				Replyto: clientUri,
			}
		case randFimsMethod == "post":
			fimsMsg = fims.FimsMsg{
				Method: randFimsMethod,
				Uri:    randUri,
				Body:   s,
			}
		case randFimsMethod == "del":
			fimsMsg = fims.FimsMsg{
				Method: randFimsMethod,
				Uri:    randUri,
				Body:   s,
			}
		case randFimsMethod == "pub":
			fimsMsg = fims.FimsMsg{
				Method: randFimsMethod,
				Uri:    randUri,
				Body:   s,
			}
		case randFimsMethod == "sub":
			fimsMsg = fims.FimsMsg{
				Method: randFimsMethod,
				Uri:    randUri,
				Body:   s,
			}
		}

		f.Send(fimsMsg)
	}
}
