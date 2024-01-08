package fims

import (
	"reflect"
	"testing"
)

var bodies = []interface{}{
	"string booooooooooody",
	map[string]interface{}{
		"one": "two",
	},
	11,
	[]string{"list", "body"},
}

var sizes = []int{
	51,
	41,
	30,
	43,
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
		{0, nil},
		{1, []string{""}},
		{1, []string{"a"}},
		{2, []string{"a",""}},
		{2, []string{"a", "b"}},
		{3, []string{"a", "b",""}},
		{3, []string{"a", "b", "c"}},
	}
	for i, s := range strs {
		f, _ := GetFrags(s)
		if len(f) != resps[i].n || !reflect.DeepEqual(f, resps[i].s) {
			t.Errorf("%s didn't decode right, got %v", s, f)
		}
	}
}

func TestSend(t *testing.T) {
	f, err := Connect("FimsTest")
	defer func() {
		err := f.Close()
		if err != nil {
			t.Error("Failed to close FIMS connection:", err)
		}
	}()

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
		if n !=  sizes[i] {
			t.Errorf("Message %v: Sent size %v, expected size %d\n", b, n, sizes[i])
		}
	}

}
