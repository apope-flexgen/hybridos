package db

import (
	"context"
	"fmt"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	mongo "go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"go.mongodb.org/mongo-driver/mongo/readpref"
)

func NewConnector(address string, healthcheckdelay, timeout time.Duration) MongoConnector {
	return MongoConnector{
		Address:          "mongodb://" + address,
		Timeout:          timeout,
		HealthCheckDelay: healthcheckdelay,
		Context:          nil,
		Client:           mongo.Client{},
	}
}

type MongoConnector struct {
	Address          string
	HealthCheckDelay time.Duration
	Timeout          time.Duration

	Client  mongo.Client
	Context context.Context
}

func (conn *MongoConnector) Connect() error {
	ctx, cancel := context.WithTimeout(context.Background(), conn.Timeout) // create context with timeout
	defer cancel()

	m, err := mongo.Connect(ctx, options.Client().ApplyURI(conn.Address)) // connect to localhost instance
	if err != nil {
		return fmt.Errorf("failed to connect to mongodb - " + err.Error())
	}

	conn.Client = *m
	conn.Context = context.Background()

	return nil
}

func (conn *MongoConnector) Disconnect() error {
	err := conn.Client.Disconnect(conn.Context)

	conn.Context = nil
	conn.Client = mongo.Client{}

	return err
}

/*
	simple ping to mongo instance on localhost
	INPUT:
	OUTPUT: error
*/
func (conn *MongoConnector) Ping() error {
	if conn.Context == nil {
		return fmt.Errorf("error - connector has not created a mongo client")
	}

	if err := conn.Client.Ping(conn.Context, readpref.Primary()); err != nil {
		return err
	}

	return nil
}

/*
	writes FIMs messages to mongo instance
	INPUT: database name, collection name, FimsMsgs object
	OUTPUT: error
*/
func (conn *MongoConnector) Write(dbName, collection string, data []map[string]interface{}) error {
	if conn.Context == nil {
		return fmt.Errorf("error - connector has not created a mongo client")
	}
	if len(dbName) == 0 {
		return fmt.Errorf("dbName is empty or invalid")
	}
	if len(collection) == 0 {
		return fmt.Errorf("collection is empty or invalid")
	}
	if len(data) == 0 {
		return fmt.Errorf("data is empty, ignoring write call")
	}

	db := conn.Client.Database(dbName)
	coll := db.Collection(collection)
	buf := make([]interface{}, 0, 1000) // create buffer (mongodb optimatal batch size is 1000)

	for row := range data {
		if len(buf) >= 1000 {
			err := conn.waitUntilHealthy()
			if err != nil {
				return fmt.Errorf("write failed because we failed to wait until healthy: %w", err)
			}
			coll.InsertMany(conn.Context, buf, options.InsertMany().SetOrdered(false)) // write out buffer
			buf = make([]interface{}, 0, 1000)                                         // reset buffer
		}

		if len(data[row]) == 0 { // if we receive an empty message, ignore it
			continue
		}

		buf = append(buf, bson.M(data[row]))
	}

	err := conn.waitUntilHealthy()
	if err != nil {
		return fmt.Errorf("write failed because we failed to wait until healthy: %w", err)
	}
	coll.InsertMany(conn.Context, buf, options.InsertMany().SetOrdered(false)) // flush

	return nil
}

func (conn *MongoConnector) HealthCheck() (bool, error) {
	var result map[string]interface{}
	err := conn.Client.Database("admin").RunCommand(conn.Context, bson.D{primitive.E{Key: "dbStats", Value: 1}}).Decode(&result)
	if err != nil {
		return false, err
	}

	ok, exists := result["ok"]
	if _, float := ok.(float64); !float {
		return false, fmt.Errorf("result of healthcheck was the wrong datatype")
	}

	if exists && ok.(float64) == 1 {
		return true, nil
	} else if !exists {
		return false, fmt.Errorf("mongo did not provide health data")
	} else if ok.(float64) != 1 {
		return false, fmt.Errorf("mongo is not ready - status %v", ok)
	}

	return false, fmt.Errorf("health check reached impossible failure condition")
}

// Wait until healthy or return an error if the database fails both a healthcheck and a ping
func (conn *MongoConnector) waitUntilHealthy() error {
	for {
		healthy, err := conn.HealthCheck()
		if healthy {
			return nil
		} else {
			log.Debugf("health check failed:" + err.Error())
			err = conn.Ping()
			if err != nil {
				return fmt.Errorf("ping failed after healthcheck: %w", err)
			}
			time.Sleep(conn.HealthCheckDelay)
		}
	}
}
