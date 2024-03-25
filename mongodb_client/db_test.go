package db

import (
	"fmt"
	"testing"
	"time"

	"go.mongodb.org/mongo-driver/bson"
)

var (
	connector = NewConnector("localhost:27017", time.Minute/6, time.Minute)
)

func TestSampleWrite(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Fatal(err)
	}

	db := connector.Client.Database("test")
	db.Drop(connector.Context)
	coll := db.Collection("samples")

	_, err = coll.InsertOne(connector.Context, bson.D{
		{Key: "title", Value: "The Polyglot Developer Podcast"},
		{Key: "author", Value: "Nic Raboy"},
	})
	if err != nil {
		t.Error(err)
	}

	// verify that the data exists
	cursor, err := coll.Find(connector.Context, bson.D{})
	if err != nil {
		t.Error(err)
	}

	ct := 0
	for cursor.Next(connector.Context) {
		ct += 1
	}

	if ct != 1 {
		t.Errorf("too many/too few entries written: %v / 1", ct)
	}
}

func TestPing(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Fatal(err)
	}

	err = connector.Ping()
	if err != nil {
		t.Error(err)
	}
}

func TestWrite(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Fatal(err)
	}

	db := connector.Client.Database("test")
	err = db.Drop(connector.Context)
	if err != nil {
		t.Error(err)
	}

	data := makePoints(200)

	// execute
	err = connector.Write("test", "events", data)
	if err != nil {
		t.Error(err)
	}

	// verify that the data exists
	coll := db.Collection("events")
	cursor, err := coll.Find(connector.Context, bson.M{})
	if err != nil {
		t.Error(err)
	}

	ct := 0
	for cursor.Next(connector.Context) {
		ct += 1
	}

	if ct != len(data) {
		t.Fatalf("too many/too few entries written: %v / %v", ct, len(data))
	}
	t.Log(ct)
}

func TestHealthCheck(t *testing.T) {
	err := connector.Connect()
	if err != nil {
		t.Fatal(err)
	}

	healthy, err := connector.HealthCheck()
	if !healthy {
		t.Error(err)
	}

	connector.Disconnect()
}

// helper func

func makePoints(size int) []map[string]interface{} {
	data := make([]map[string]interface{}, size)
	for i := 0; i < size; i++ {
		point := map[string]interface{}{
			"fault":     true,
			"numberstr": fmt.Sprint(i),
			"time":      time.Now().Unix(),
		}
		data[i] = point
	}

	return data
}
