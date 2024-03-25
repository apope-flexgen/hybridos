package db

import (
	"fmt"
	"testing"
	"time"
)

var connector = NewConnector("localhost:8086", 10, time.Minute/2)

func TestConnect(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	connector.Disconnect()
}

func TestWriteBatch(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	err = connector.WriteBatch("test2", "batch", "dbtest", makeData(20), nil)
	if err != nil {
		t.Error(err)
	}

	connector.Disconnect()
}

func TestWritePoints(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	err = connector.WritePoints("test2", "points", "dbtest", makeData(2), map[string]interface{}{"messages": 2})
	if err != nil {
		t.Error(err)
	}

	connector.Disconnect()
}

func TestHealthCheck(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Error(err)
	}

	status, err := connector.HealthCheck()
	if err != nil {
		t.Error(err)
	}

	t.Log(status)
}

func makeData(num int) []map[string]interface{} {
	data := make([]map[string]interface{}, num)
	for i := 0; i < num; i++ {
		data[i] = map[string]interface{}{
			"time":   uint64(time.Now().Unix()),
			"number": i,
			"string": fmt.Sprintf("point%v", i),
		}
	}
	return data
}
