package fims

import "testing"

func TestConnect(t *testing.T) {
	f, err := Connect("FimsTest")

	if err != nil {
		t.Errorf("Couldn't connect to FIMS, maybe it's not running: %s", err)
	}

	err = f.Close()
	if err != nil {
		t.Errorf("Couldn't close connection to FIMS: %s", err)
	}
}

func TestTwoConnect(t *testing.T) {
	f1, err := Connect("FimsTest1")
	defer f1.Close()

	if err != nil {
		t.Errorf("Couldn't connect to FIMS the first time, maybe it's not running")
	}

	f2, err := Connect("FimsTest2")
	defer f2.Close()

	if err != nil {
		t.Errorf("Couldn't connect to FIMS a second time, maybe it's not running")
	}

}

func TestSubscribe(t *testing.T) {
	f, err := Connect("FimsTest")
	defer f.Close()

	if err != nil {
		t.Errorf("Couldn't connect to FIMS, maybe it's not running")
	}

	err = f.Subscribe("/")
	if err != nil {
		t.Error("Unable to subscribe", err)
	}
}
