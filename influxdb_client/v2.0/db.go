package db

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"os/exec"
	"reflect"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"

	influx "github.com/influxdata/influxdb-client-go/v2"
)

const (
	token = "mQ3hO0Z0Br2akKIRLxWk5sKPVzenJxqB5KJdXraIFO_Hw_P8-0WWBFib2OhCEF6RHu1p1QxpxgG-fZJoLiVPIQ=="
	org   = "flexgen"
)

func NewConnector(address string, timeout int, writeDelay time.Duration) InfluxConnector {
	return InfluxConnector{
		Address:    "http://" + address,
		Timeout:    uint(timeout),
		WriteDelay: writeDelay,
		Client:     nil,
	}
}

type InfluxConnector struct {
	Address    string
	WriteDelay time.Duration
	Timeout    uint

	Client influx.Client
}

func (conn *InfluxConnector) Connect() error {
	c := influx.NewClientWithOptions(conn.Address, "",
		influx.DefaultOptions().SetHTTPRequestTimeout(conn.Timeout))
	if c == nil {
		return fmt.Errorf("Error creating InfluxDB Client: nil client")
	}

	conn.Client = c

	return nil
}

func (conn *InfluxConnector) Disconnect() {
	conn.Client.Close()
	conn.Client = nil
}

func (conn *InfluxConnector) Ping() string {
	cmd := exec.Command("influx", "ping") // executes ping through command line arg

	reader, writer, err := os.Pipe() // grab os reader and writer
	if err != nil {
		panic(err)
	}

	// assign cmd out and err to os writer
	cmd.Stdout = writer
	cmd.Stderr = writer

	out := make(chan string) // output channel
	wg := new(sync.WaitGroup)
	wg.Add(1)
	// spawn goroutine to copy reader to output channel
	go func() {
		var buf bytes.Buffer
		wg.Done()
		io.Copy(&buf, reader)
		out <- buf.String()
	}()

	wg.Wait()
	_ = cmd.Run() // run the ping
	writer.Close()

	return <-out
}

func (conn *InfluxConnector) CreateDB(dbName string) string {
	if len(dbName) == 0 {
		return "database name is empty or invalid"
	}
	cmd := exec.Command("influx", "bucket", "create", "-n", dbName, "-o", org) // execute creation through command line arg

	reader, writer, err := os.Pipe() // grab os reader and writer
	if err != nil {
		panic(err)
	}

	// assign cmd out and err to os writer
	cmd.Stdout = writer
	cmd.Stderr = writer

	out := make(chan string) // output channel
	wg := new(sync.WaitGroup)
	wg.Add(1)
	// spawn goroutine to copy reader to output channel
	go func() {
		var buf bytes.Buffer
		wg.Done()
		io.Copy(&buf, reader)
		out <- buf.String()
	}()

	wg.Wait()
	_ = cmd.Run() // run the ping
	writer.Close()

	return <-out
}

func (conn *InfluxConnector) WritePoints(dbName, measurement, source string, data []map[string]interface{}, metadata map[string]interface{}) error {
	//input validations
	if conn.Client == nil {
		return fmt.Errorf("error - client connection does not exist. please create one and try again!")
	}
	if len(dbName) == 0 {
		return fmt.Errorf("database name is empty or invalid")
	}
	if len(measurement) == 0 {
		return fmt.Errorf("measurement is empty or invalid")
	}
	if len(source) == 0 {
		return fmt.Errorf("source is empty or invalid")
	}
	if len(data) == 0 {
		return fmt.Errorf("no data provided to write")
	}

	writer := conn.Client.WriteAPIBlocking("", dbName) // create writeAPI

	msgsLen := 0            // track msg length
	for row := range data { // iterate over data rows and create points
		if len(data[row]) == 0 { // ignore empty
			continue
		}

		tags := map[string]string{"source": source} // create source tag

		if source, exists := data[row]["ftd_group"]; exists { // "ftd_group" datapoint indicates the source tag for the entire row
			tags["source"] = source.(string)
		}

		if _, exists := data[row]["time"]; !exists { // skip if no time value
			continue
		}
		temp := (data[row]["time"].(uint64)) // store timestamp to re-add later
		timestamp := time.Unix(0, int64(temp)*1000)
		delete(data[row], "time")

		for key, val := range data[row] {
			if reflect.TypeOf(val).Kind() != reflect.Slice {
				continue // ignore non-slice fields
			}
			s := reflect.ValueOf(val)

			for i := 0; i < s.Len(); i++ { // process each element of slice individually
				switch s.Index(i).Type().Kind() {
				case reflect.Map: // if map, create new field and value and add to data[row]
					rMap := reflect.ValueOf(s.Index(i).Interface())
					data[row][key+fmt.Sprint(rMap.MapIndex(reflect.ValueOf("value")))] = fmt.Sprint(rMap.MapIndex(reflect.ValueOf("string")))
				default:
					log.Errorf("%v", reflect.TypeOf(s.Index(i)).Kind())
					// continue
				}
			}
			delete(data[row], key)
		}

		if len(data[row]) == 0 { //ensure msg point is not empty
			log.Warnf("ignoring empty data point")
			continue
		}

		pt := influx.NewPoint(measurement, tags, data[row], timestamp) // create point
		data[row]["time"] = temp
		msgsLen++

		fmt.Println("writing item " + fmt.Sprint(row))
		for { // check health and attempt write when available
			ready, err := conn.HealthCheck()
			if ready {
				err := writer.WritePoint(context.Background(), pt)
				if err != nil {
					log.Errorf("failed to write point: " + err.Error())
					return err // return so we dont lose any data and properly mark as a failed archive
				}
				fmt.Println("wrote item " + fmt.Sprint(row))
				break
			} else {
				log.Errorf("influx not ready: " + err.Error())
				time.Sleep(conn.WriteDelay)
				continue
			}
		}
	}

	if _, exists := metadata["messages"]; exists {
		//update messages count in metadata if exist
		metadata["messages"] = msgsLen
	}

	// create metaData to be logged
	mt := influx.NewPoint("metadata", map[string]string{"source": source}, metadata, time.Now())
	err := writer.WritePoint(context.Background(), mt)
	if err != nil {
		log.Errorf("failed to write metadata point: " + err.Error())
		return err
	}

	return nil
}

func (conn *InfluxConnector) WriteBatch(dbName, measurement, source string, data []map[string]interface{}, metadata map[string]interface{}) error {
	//input validations
	if conn.Client == nil {
		return fmt.Errorf("error - client connection does not exist. please create one and try again!")
	}
	if len(dbName) == 0 {
		return fmt.Errorf("database name is empty or invalid")
	}
	if len(measurement) == 0 {
		return fmt.Errorf("measurement is empty or invalid")
	}
	if len(source) == 0 {
		return fmt.Errorf("source is empty or invalid")
	}
	if len(data) == 0 {
		return fmt.Errorf("no data provided to write")
	}

	writer := conn.Client.WriteAPI("", dbName) // create writeAPI
	select {
	case err := <-writer.Errors():
		return fmt.Errorf("writer could not be created: " + err.Error())
	default:
		// keep going
	}

	msgsLen := 0            // track msg length
	for row := range data { // iterate over data rows and create points
		if len(data[row]) == 0 { // ignore empty
			continue
		}

		tags := map[string]string{"source": source} // create source tag

		if source, exists := data[row]["ftd_group"]; exists { // "ftd_group" datapoint indicates the source tag for the entire row
			tags["source"] = source.(string)
		}

		if _, exists := data[row]["time"]; !exists { // skip if no time value
			continue
		}
		temp := (data[row]["time"].(uint64)) // store timestamp to re-add later
		timestamp := time.Unix(0, int64(temp)*1000)
		delete(data[row], "time")

		for key, val := range data[row] {
			if reflect.TypeOf(val).Kind() != reflect.Slice {
				continue // ignore non-slice fields
			}
			s := reflect.ValueOf(val)

			for i := 0; i < s.Len(); i++ { // process each element of slice individually
				switch s.Index(i).Type().Kind() {
				case reflect.Map: // if map, create new field and value and add to data[row]
					rMap := reflect.ValueOf(s.Index(i).Interface())
					data[row][key+fmt.Sprint(rMap.MapIndex(reflect.ValueOf("value")))] = fmt.Sprint(rMap.MapIndex(reflect.ValueOf("string")))
				default:
					log.Errorf("%v", reflect.TypeOf(s.Index(i)).Kind())
					// continue
				}
			}
			delete(data[row], key)
		}

		if len(data[row]) == 0 { //ensure msg point is not empty
			log.Warnf("ignoring empty data point")
			continue
		}

		pt := influx.NewPoint(measurement, tags, data[row], timestamp) // create point
		select {
		case err := <-writer.Errors():
			return fmt.Errorf("point could not be created: " + err.Error())
		default:
			// keep going
		}

		data[row]["time"] = temp // re-add time to temp
		msgsLen++

		writer.WritePoint(pt)
		select {
		case err := <-writer.Errors():
			log.Errorf("could not write point to buffer: " + err.Error())
		default:
			// keep going
		}
	}

	if _, exists := metadata["messages"]; exists {
		//update messages count in metadata if exist
		metadata["messages"] = msgsLen
	}

	// create metaData to be logged
	mt := influx.NewPoint("metadata", map[string]string{"source": source}, metadata, time.Now())
	writer.WritePoint(mt)

	for { // check health and attempt write buffer flush when available
		ready, err := conn.HealthCheck()
		if ready {
			writer.Flush()
			break
		} else {
			log.Errorf("influx not ready: " + err.Error())
			time.Sleep(conn.WriteDelay)
			continue
		}
	}

	select {
	case err := <-writer.Errors():
		return err
	default:
		return nil
	}
}

func (conn *InfluxConnector) HealthCheck() (bool, error) {
	health, err := conn.Client.Health(context.Background()) // check health
	if err != nil {
		log.Warnf("failed to check health")
		return false, err
	}

	if strings.ToLower(string(health.Status)) != "pass" {
		return false, fmt.Errorf("%v", health.Status)
	}

	return true, nil
}
