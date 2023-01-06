package fims

import (
	"fmt"
	"reflect"
	"testing"
)

var tests = []string{
	// Integer value
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"/ui/reply","body":"{\"active_power\":10}"}`,
	// Float value
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"/ui/reply","body":"{\"active_power\":10}"}`,
	// String value
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"/ui/reply","body":"{\"active_power\":\"A string value\"}"}`,
	// Bitfield value
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"/ui/reply","body":"{\"active_power\":[{\"value\":1,\"string\":\"The error code\"},{\"value\":5,\"string\":\"The horror code\"}]}"}`,
	// Fully defined URI, clothed body
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"/ui/reply","body":"{\"active_power\":{\"value\":123.45,\"name\":\"Active power\"},\"reactive_power\":{\"value\":43,\"name\":\"Reactive Power\"}}"}`,
	// Fully defined URI, naked body
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"/ui/reply","body":"{\"active_power\":123.45,\"reactive_power\":43}"}`,
	// n-1 defined URI, clothed body
	`{"method":"pub","uri":"/assets/ess","replyto":"/ui/reply","body":"{\"ess1\":{\"active_power\":{\"value\":123.45,\"name\":\"Active power\"},\"reactive_power\":{\"value\":43,\"name\":\"Reactive Power\"}},\"ess2\":{\"active_power\":{\"value\":543.21,\"name\":\"Active power\"},\"reactive_power\":{\"value\":34,\"name\":\"Reactive Power\"}}}"}`,
	// n-1 defined URI, naked body
	`{"method":"pub","uri":"/assets/ess","replyto":"/ui/reply","body":"{\"ess1\":{\"active_power\":123.45,\"reactive_power\":43},\"ess2\":{\"active_power\":543.21,\"reactive_power\":34}}"}`,
	// n-2 defined URI, clothed body
	`{"method":"pub","uri":"/assets","replyto":"/ui/reply","body":"{\"ess\":{\"ess1\":{\"active_power\":{\"value\":123.45,\"name\":\"Active power\"},\"reactive_power\":{\"value\":43,\"name\":\"Reactive Power\"}},\"ess2\":{\"active_power\":{\"value\":543.21,\"name\":\"Active power\"},\"reactive_power\":{\"value\":34,\"name\":\"Reactive Power\"}}},\"feeders\":{\"feed1\":{\"active_power\":{\"value\":254.4,\"name\":\"Active power\"},\"reactive_power\":{\"value\":43,\"name\":\"Reactive Power\"}},\"feed2\":{\"active_power\":{\"value\":857.5,\"name\":\"Active power\"},\"reactive_power\":{\"value\":34,\"name\":\"Reactive Power\"}}}}"}`,
	// n-2 defined URI, naked body
	`{"method":"pub","uri":"/","replyto":"/ui/reply","body":"{\"ess\":{\"ess1\":{\"active_power\":123.45,\"reactive_power\":43},\"ess2\":{\"active_power\":543.21,\"reactive_power\":34}},\"feeders\":{\"feed1\":{\"active_power\":254.4,\"reactive_power\":43},\"feed2\":{\"active_power\":857.5,\"reactive_power\":34}}}"}`,
	// n-3 defined URI, clothed body
	`{"method":"pub","uri":"/","replyto":"/ui/reply","body":"{\"assets\":{\"ess\":{\"ess1\":{\"active_power\":{\"value\":123.45,\"name\":\"Active power\"},\"reactive_power\":{\"value\":43,\"name\":\"Reactive Power\"}},\"ess2\":{\"active_power\":{\"value\":543.21,\"name\":\"Active power\"},\"reactive_power\":{\"value\":34,\"name\":\"Reactive Power\"}}},\"feeders\":{\"feed1\":{\"active_power\":{\"value\":254.4,\"name\":\"Active power\"},\"reactive_power\":{\"value\":43,\"name\":\"Reactive Power\"}},\"feed2\":{\"active_power\":{\"value\":857.5,\"name\":\"Active power\"},\"reactive_power\":{\"value\":34,\"name\":\"Reactive Power\"}}}},\"components\":{\"modbus1\":{\"register\":{\"value\":900,\"name\":\"Just a register\"}}}}"}`,
	// n-3 defined URI, naked body
	`{"method":"pub","uri":"/","replyto":"/ui/reply","body":"{\"assets\":{\"ess\":{\"ess1\":{\"active_power\":123.45,\"reactive_power\":43},\"ess2\":{\"active_power\":543.21,\"reactive_power\":34}},\"feeders\":{\"feed1\":{\"active_power\":254.4,\"reactive_power\":43},\"feed2\":{\"active_power\":857.5,\"reactive_power\":34}}},\"components\":{\"modbus1\":{\"register\":900}}}"}`,
	// Empty {} body
	`{"method":"pub","uri":"/whatever","replyto":"/ui/reply","body":"{}"}`,
	// Blank "" body
	`{"method":"pub","uri":"/whatever","replyto":"/ui/reply","body":"\"\""}`,
	// Missing body
	`{"method":"pub","uri":"/whatever","replyto":"/ui/reply"}`,
	// Blank "" replyto
	`{"method":"pub","uri":"/assets/ess/ess1","replyto":"","body":"{\"active_power\":123.45,\"reactive_power\":43}"}`,
	// Missing replyto
	`{"method":"pub","uri":"/assets/ess/ess1","body":"{\"active_power\":123.45,\"reactive_power\":43}"}`,
}

var bodies = []interface{}{
	"string booooooooooody",
	map[string]interface{}{
		"one": "two",
	},
	11,
	[]string{"list", "body"},
}

func TestDecode(t *testing.T) {
	for _, jsonstr := range tests {
		msg, err := Decode([]byte(jsonstr))
		if err != nil {
			t.Errorf("Couldn't decode the message: %s, %v", jsonstr, err)
		} else {
			fmt.Println(msg)
		}
	}
}

func TestGetFrags(t *testing.T) {
	strs := []string{
		"", "/", "/a", "/a/",
		"/a/b", "/a/b/", "/a/b/c",
	}
	type r struct {
		n int
		s []string
	}
	resps := []r{
		{0, []string{}},
		{0, []string{}},
		{1, []string{"a"}},
		{1, []string{"a"}},
		{2, []string{"a", "b"}},
		{2, []string{"a", "b"}},
		{3, []string{"a", "b", "c"}},
	}
	for i, s := range strs {
		n, f := GetFrags(s)
		if n != resps[i].n || !reflect.DeepEqual(f, resps[i].s) {
			t.Errorf("%s didn't decode right, got %v", s, f)
		}
	}
}

func TestSend(t *testing.T) {
	f, err := Connect("FimsTest")
	defer f.Close()

	if err != nil {
		t.Errorf("Couldn't connect to FIMS, maybe it's not running")
	}

	err = f.Subscribe("/")
	if err != nil {
		t.Errorf("Unable to subscribe")
	}

	for i, b := range bodies {
		n, err := f.Send(FimsMsg{
			Method:  "pub",
			Uri:     "/test",
			Replyto: "",
			Body:    b,
		})
		if err != nil {
			t.Errorf("Failed to send the %dth body, %+v\n", i, b)
		}
		fmt.Printf("Wrote %d bytes of %+v\n", n, b)
	}

}
