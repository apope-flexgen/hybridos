package db

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"reflect"
	"strconv"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/influxdb_client/countbytes"

	"github.com/influxdata/influxdb1-client/models"
	"github.com/influxdata/influxdb1-client/pkg/escape"
	influx "github.com/influxdata/influxdb1-client/v2"
)

type RetentionPolicy struct {
	Name               string
	Duration           string
	ShardGroupDuration string
	ReplicaN           int
	Default            bool
}

// See this link for more info on the structure of Influx continuous queries:
// https://docs.influxdata.com/influxdb/v1/query_language/continuous_queries/
type ContQuery struct {
	Name        string // Name of the CQ
	Db          string // DB on which the CQ is applied
	Resample    string // String passed to RESAMPLE phrase of CQ if it should be included, defines how to do recalculations used to account for batching period
	Select      string // String passed to SELECT phrase of CQ, defines query which becomes the CQ's result
	Into        string // Destination DB in which the CQ results are stored
	Rp          string // Name of RP to store the CQ into
	Measurement string // Destination measurement in which the CQ results are stored
	From        string // Measurement on which the CQ is applied
	Groupby     string // String passed to GROUP BY phrase of CQ, defines grouping used by aggregations
}

type BatchPoints struct {
	data influx.BatchPoints
}

//lint:ignore ST1005 this is an error coming from the InfluxDB client library which we have no control over
// The error returned over an HTTP response when InfluxDB times out while processing a query/write.
var ErrInfluxTimeout error = errors.New("{\"error\":\"timeout\"}\n")

// Signifies an issue with the network connection to InfluxDB.
var ErrConnIssue error = errors.New("network connectivity issue")

// creates a new connector
//
// INPUT: address (ip + port), connection attempt timeout, delay between health checks when waiting to write, enable gzip compression (bool)
//
// OUTPUT: InfluxConnector object (does not create Client connection -> see Connect() to do this)
func NewConnector(address string, timeout, writeDelay time.Duration, gzip bool) InfluxConnector {
	return InfluxConnector{
		Address:          "http://" + address,
		Timeout:          timeout,
		HealthCheckDelay: writeDelay,
		GZip:             gzip,
		Client:           nil,
	}
}

// InfluxConnector performs all influx operations and conversions
//
// stores a single influx connection - must be activated using Connect()
//
// for graceful shutdowns, call Disconnect() before ending program
type InfluxConnector struct {
	Address          string
	HealthCheckDelay time.Duration
	Timeout          time.Duration
	GZip             bool

	Client influx.Client // created by Connect()
}

// opens the Client connection stored in the connector
//
// INPUT: none
//
// OUTPUT: error
func (conn *InfluxConnector) Connect() error {
	conf := influx.HTTPConfig{
		Addr:    conn.Address,
		Timeout: conn.Timeout,
	}

	if conn.GZip {
		conf.WriteEncoding = "gzip"
	}

	c, err := influx.NewHTTPClient(conf)
	if err != nil {
		return fmt.Errorf("failed creating InfluxDB Client: %w", err)
	}

	conn.Client = c

	return nil
}

// closes the Client connection stored in the connector
//
// INPUT: none
//
// OUTPUT: error
func (conn *InfluxConnector) Disconnect() error {
	err := conn.Client.Close()
	conn.Client = nil

	return err
}

// simple http ping
//
// INPUT: none
//
// OUTPUT: error
func (conn *InfluxConnector) Ping() error {
	if conn.Client == nil {
		return fmt.Errorf("error - client connection does not exist. please create one and try again")
	}
	// perform ping
	_, _, err := conn.Client.Ping(0)
	if err != nil {
		return fmt.Errorf("failed pinging InfluxDB Cluster: %w", err)
	}
	return nil
}

//creates a new influx database with dbName
//
// INPUT: name for database, retention policies (slice)
// ** policies can be empty
//
// OUTPUT: error
func (conn *InfluxConnector) CreateDatabase(dbName string, policies []RetentionPolicy) error {
	if conn.Client == nil {
		return fmt.Errorf("error - client connection does not exist. please create one and try again")
	}
	if len(dbName) == 0 {
		return fmt.Errorf("database name is empty or invalid")
	}

	// attempt to create DB
	query := fmt.Sprintf("CREATE DATABASE \"%s\"", dbName)

	q := influx.NewQuery(query, "", "")
	if response, err := conn.Client.Query(q); err == nil && response.Error() == nil {
		log.Infof("database created or found: " + dbName)
	} else if err != nil {
		return fmt.Errorf("query error - %w", err)
	} else if response.Error() != nil {
		return fmt.Errorf("reponse error - %w", response.Error())
	}

	for _, policy := range policies {
		_, err := conn.AddRetentionPolicy(dbName, policy)
		if err != nil {
			err = fmt.Errorf("could not create retention policy %s: %w", policy.Name, err)
			log.Errorf(err.Error())
		}
	}

	return nil
}

// adds retention policy to already-existing database
//
// INPUT: database name, policy
//
// OUTPUT: result from query, error
func (conn *InfluxConnector) AddRetentionPolicy(dbName string, policy RetentionPolicy) (string, error) {
	if conn.Client == nil {
		return "", fmt.Errorf("error - client connection does not exist. please create one and try again")
	}

	query := fmt.Sprintf("CREATE RETENTION POLICY \"%s\" ON \"%s\" DURATION %s", policy.Name, dbName, policy.Duration)
	if policy.Duration == "" {
		return "", fmt.Errorf("must specify duration")
	}
	if policy.ReplicaN != 0 {
		query += fmt.Sprintf(" REPLICATION %v", policy.ReplicaN)
	} else {
		query += fmt.Sprintf(" REPLICATION %v", 1) //default
	}
	if policy.ShardGroupDuration != "" {
		query += fmt.Sprintf(" SHARD DURATION %s", policy.ShardGroupDuration)
	}
	if policy.Default {
		query += " DEFAULT"
	}

	q := influx.NewQuery(query, dbName, "")
	if response, err := conn.Client.Query(q); err == nil && response.Error() == nil {
		log.Infof("retention policy created: " + policy.Name)
	} else if err != nil {
		return query, fmt.Errorf("query error - %w", err)
	} else if response.Error() != nil {
		return query, fmt.Errorf("reponse error - %w", response.Error())
	}

	return query, nil
}

// sets the default policy for given database -- policy MUST already exist (see AddRetentionPolicy())
//
// INPUT: database name, policy name
//
// OUTPUT: query response, error
func (conn *InfluxConnector) SetRetentionPolicy(dbName, policyName string) (string, error) {
	if conn.Client == nil {
		return "", fmt.Errorf("error - client connection does not exist. please create one and try again")
	}

	query := fmt.Sprintf("ALTER RETENTION POLICY \"%s\" ON \"%s\" DEFAULT", policyName, dbName)

	q := influx.NewQuery(query, dbName, "")
	if response, err := conn.Client.Query(q); err == nil && response.Error() == nil {
		log.Infof("default policy is now: " + policyName)
	} else if err != nil {
		return query, fmt.Errorf("query error - %w", err)
	} else if response.Error() != nil {
		return query, fmt.Errorf("reponse error - %w", response.Error())
	}
	return query, nil
}

// creates and runs a new continuous query on the influx instance
//
// INPUT: continuous query
//
// OUTPUT: query response, error
func (conn *InfluxConnector) RunContinuousQuery(contQuery ContQuery) (string, error) {
	if conn.Client == nil {
		return "", fmt.Errorf("error - client connection does not exist. please create one and try again")
	}

	// construct resampling phrase for the query if resampling parameters were given
	resamplePhrase := ""
	if contQuery.Resample != "" {
		resamplePhrase = fmt.Sprintf("RESAMPLE %s", contQuery.Resample)
	}

	// form and send query
	// note usage of quotes around identifiers so that the caller doesn't have to worry as much about Influx identifier requirements
	// https://docs.influxdata.com/influxdb/v1/query_language/spec/#identifiers
	query := fmt.Sprintf("CREATE CONTINUOUS QUERY \"%s\" ON \"%s\" %s "+
		"BEGIN "+
		"SELECT %s INTO \"%s\".\"%s\".%s FROM %s GROUP BY %s "+
		"END",
		contQuery.Name, contQuery.Db, resamplePhrase, contQuery.Select, contQuery.Into, contQuery.Rp, contQuery.Measurement, contQuery.From, contQuery.Groupby)

	q := influx.NewQuery(query, contQuery.Db, "")
	if response, err := conn.Client.Query(q); err == nil && response.Error() == nil {
		log.Infof("started continuous query: " + contQuery.Name)
	} else if err != nil {
		return query, fmt.Errorf("query error - %w", err)
	} else if response.Error() != nil {
		return query, fmt.Errorf("response error - %w", response.Error())
	}
	return query, nil
}

// writes a batch of FIMs messages to influx
//
// INPUT: name of database, measurement, slice of timestamps for data, slice of data bodies, optional additional metadata
//
// OUTPUT: error
func (conn *InfluxConnector) WriteData(dbName, measurement, source string, timestamps []uint64, data []map[string]interface{}, metadata map[string]interface{}) error {
	batches, err := conn.MakeBatches(dbName, measurement, source, timestamps, data, metadata)
	if err != nil {
		return err
	}

	for i, batch := range batches {
		err = conn.WriteBatch(batch)
		if err != nil {
			return fmt.Errorf("partial write - error on batch #%d: %w", i, err)
		}
	}

	return nil
}

// converts FIMS data to BatchPoints
//
// INPUT: name of database, measurement, slice of timestamps for data, slice of data bodies, optional additional metadata
//
// OUTPUT: slice of internal BatchPoints struct, error
func (conn *InfluxConnector) MakeBatches(dbName, measurement, source string, timestamps []uint64, data []map[string]interface{}, metadata map[string]interface{}) ([]BatchPoints, error) {
	//input validations
	if conn.Client == nil {
		return nil, fmt.Errorf("error - client connection does not exist. please create one and try again")
	}
	if len(dbName) == 0 {
		return nil, fmt.Errorf("database name is empty or invalid")
	}
	if len(measurement) == 0 {
		return nil, fmt.Errorf("measurement is empty or invalid")
	}
	if len(source) == 0 {
		return nil, fmt.Errorf("source is empty or invalid")
	}
	if len(data) == 0 {
		return nil, fmt.Errorf("no data provided to write")
	}

	batches := make([]BatchPoints, 0)

	// create initial batch
	bp, err := influx.NewBatchPoints(influx.BatchPointsConfig{
		Database:  dbName,
		Precision: "ms",
	})
	if err != nil {
		return nil, fmt.Errorf("failed to create batch: %w", err)
	}

	default_tags := map[string]string{"source": source}
	sizeLowerBound := 0 // lower bound estimate of num bytes in request
	msgsLen := 0
	// iterate over data rows and create points
	for row := range data {
		// if we receive an empty message, ignore it
		if len(data[row]) == 0 {
			continue
		}
		// create a new point

		tags := default_tags
		// An "ftd_group" datapoint indicates the source tag for the entire row
		if source, exists := data[row]["ftd_group"]; exists {
			tags["source"] = source.(string)
		}

		for key, val := range data[row] {
			if reflect.TypeOf(val).Kind() != reflect.Slice {
				sizeLowerBound += countbytes.SizeOfString(key)
				sizeLowerBound += countbytes.SizeOfJsonLeaf(val)
				continue // No processing needed for non-slice values
			}

			// Handle processing values which are slices of maps
			switch s := val.(type) {
			case []map[string]interface{}:
				for _, entry := range s { // Process each element of slice individually
					entryValue, ok := entry["value"]
					if !ok {
						return nil, fmt.Errorf("did not find a \"value\" key in map in msg body")
					}
					strEntryValue := fmt.Sprint(entryValue)
					entryString, ok := entry["string"].(string)
					if !ok {
						return nil, fmt.Errorf("did not find a \"string\" key with a string value in map in msg body")
					}
					data[row][key+strEntryValue] = entryString
					sizeLowerBound += countbytes.SizeOfString(key + strEntryValue)
					sizeLowerBound += countbytes.SizeOfString(entryString)
				}
			default:
				return nil, fmt.Errorf("unexpected data type %T for value in msg body", val)
			}
			delete(data[row], key)
		}

		//ensure msg point is not empty
		if len(data[row]) == 0 {
			log.Warnf("ignoring empty data point")
			continue
		}

		// create point
		ptTimestamp := time.Unix(0, int64(timestamps[row])*1000)
		pt, err := influx.NewPoint(measurement, tags, data[row], ptTimestamp) // convert int64 to time
		if err != nil {
			log.Errorf("failed to create new point, measurement: %s source: %s with error: %v", measurement, source, err)
			log.Errorf("ignoring adding point %v", data[row])
			continue
		}

		msgsLen++
		// Add size of metadata for the point to our size estimate
		sizeLowerBound += countbytes.SizeOfString(measurement)
		sizeLowerBound += countbytes.SizeOfDirectValue(ptTimestamp)
		for tagKey, tagVal := range tags {
			sizeLowerBound += countbytes.SizeOfString(tagKey)
			sizeLowerBound += countbytes.SizeOfString(tagVal)
		}

		if sizeLowerBound >= 19000000 || len(bp.Points()) >= 5000 { // if at max size or optimum
			batches = append(batches, BatchPoints{data: bp}) // add to return value

			bp, err = influx.NewBatchPoints(influx.BatchPointsConfig{ // create new batch
				Database:  dbName,
				Precision: "ms",
			})
			if err != nil {
				return nil, fmt.Errorf("failed to create batch: %w", err)
			}
			sizeLowerBound = 0
		}

		bp.AddPoint(pt) // add point to batch
	}

	_, exists := metadata["messages"]
	if exists {
		//update messages count in metadata if exist
		metadata["messages"] = msgsLen
	}

	// create metaData to be logged
	mt, err := conn.newPoint("metadata", default_tags, metadata, time.Now())
	if err != nil {
		log.Errorf("could not create metadata point")
	} else {
		bp.AddPoint(mt) // add point to batch
	}

	batches = append(batches, BatchPoints{data: bp}) // add to return value

	return batches, nil
}

// sends BatchPoints to influx
//
// INPUT: slice of internal BatchPoints struct
//
// OUTPUT: error
func (conn *InfluxConnector) WriteBatch(batch BatchPoints) error {
	if len(batch.data.Points()) == 0 {
		return fmt.Errorf("no points in batch, nothing to do")
	}

	err := conn.waitUntilHealthy()
	if err != nil {
		return fmt.Errorf("write failed because we failed to wait until healthy: %w", err)
	}
	err = conn.Client.Write(batch.data)
	if err != nil {
		// InfluxDB internal timeout
		if err.Error() == ErrInfluxTimeout.Error() {
			return fmt.Errorf("write failed due to InfluxDB timeout: %w", ErrInfluxTimeout)
		}
		// network connectivity issue
		var urlErr *url.Error
		if errors.As(err, &urlErr) {
			return fmt.Errorf("write failed due to %w: %s", ErrConnIssue, err.Error())
		}
		return fmt.Errorf("write error: %w", err)
	}
	return nil
}

// checks the health status of the connected influx instance
//
// INPUT: none
//
// OUTPUT: bool indicating if influx is ready (true) or not (false), associated error if influx is not ready
func (conn *InfluxConnector) HealthCheck() (bool, error) {
	resp, err := http.Get(conn.Address + "/health")
	if err != nil {
		return false, err
	}
	defer resp.Body.Close()

	if resp.StatusCode == 200 { // influx exclusively defines status 200 as "healthy"
		return true, nil
	}

	by, _ := io.ReadAll(resp.Body)
	body := make(map[string]interface{})

	err = gob.NewDecoder(bytes.NewBuffer(by)).Decode(&body)
	if err != nil {
		return false, fmt.Errorf("%d: could not decode body (%w)", resp.StatusCode, err)
	}

	return false, fmt.Errorf("%d: %v", resp.StatusCode, body["message"])
}

// Wait until healthy or return an error if the database fails both a healthcheck and a ping
func (conn *InfluxConnector) waitUntilHealthy() error {
	for {
		healthy, err := conn.HealthCheck()
		if healthy {
			return nil
		} else {
			log.Warnf("health check failed:" + err.Error())
			err = conn.Ping()
			if err != nil {
				return fmt.Errorf("ping failed after healthcheck: %w", err)
			}
			time.Sleep(conn.HealthCheckDelay)
		}
	}
}

func (conn *InfluxConnector) newPoint(measurement string, tags map[string]string, data map[string]interface{}, t time.Time) (*influx.Point, error) {
	mtags := []models.Tag{}
	for key, val := range tags {
		mtags = append(mtags, models.Tag{Key: []byte(key), Value: []byte(val)})
	}
	mkey := models.MakeKey([]byte(measurement), mtags)

	pt := struct {
		key    string
		time   time.Time
		fields models.Fields
	}{
		key:    string(mkey),
		time:   t,
		fields: data,
	}

	by, err := pointsMarshalBinary(pt)
	if err != nil {
		return nil, fmt.Errorf("could not convert point to bytes: %w", err)
	}

	mpoint, err := models.NewPointFromBytes(by)
	if err != nil {
		return nil, fmt.Errorf("could not convert bytes to models.Point: %w", err)
	}

	return influx.NewPointFrom(mpoint), nil
}

// this is adapted from models.points.MarshalBinary
// since models.points is unexported, the struct and function had to be mimicked in order to be used
func pointsMarshalBinary(pt struct {
	key    string
	time   time.Time
	fields models.Fields
}) ([]byte, error) {
	fieldsby := fieldsMarshalBinary(pt.fields)
	if len(fieldsby) == 0 {
		return nil, fmt.Errorf("data cannot be empty")
	}

	tb, err := pt.time.MarshalBinary()
	if err != nil {
		return nil, err
	}

	b := make([]byte, 8+len(pt.key)+len(fieldsby)+len(tb))
	i := 0

	binary.BigEndian.PutUint32(b[i:], uint32(len(pt.key)))
	i += 4

	i += copy(b[i:], []byte(pt.key))

	binary.BigEndian.PutUint32(b[i:i+4], uint32(len(fieldsby)))
	i += 4

	i += copy(b[i:], fieldsby)

	copy(b[i:], tb)
	return b, nil
}

// this is copied from models.Fields.MarshalBinary(), just with the sort() removed
func fieldsMarshalBinary(p models.Fields) []byte {
	var b []byte
	keys := make([]string, 0, len(p))

	for k := range p {
		keys = append(keys, k)
	}

	for i, k := range keys {
		if i > 0 {
			b = append(b, ',')
		}
		b = appendField(b, k, p[k])
	}

	return b
}

// required for models.Fields.MarshalBinary() -> fieldsMarshalBinary()
func appendField(b []byte, k string, v interface{}) []byte {
	b = append(b, []byte(escape.String(k))...)
	b = append(b, '=')

	// check popular types first
	switch v := v.(type) {
	case float64:
		b = strconv.AppendFloat(b, v, 'f', -1, 64)
	case int64:
		b = strconv.AppendInt(b, v, 10)
		b = append(b, 'i')
	case string:
		b = append(b, '"')
		b = append(b, []byte(models.EscapeStringField(v))...)
		b = append(b, '"')
	case bool:
		b = strconv.AppendBool(b, v)
	case int32:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case int16:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case int8:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case int:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case uint64:
		b = strconv.AppendUint(b, v, 10)
		b = append(b, 'u')
	case uint32:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case uint16:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case uint8:
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case uint:
		// TODO: 'uint' should be converted to writing as an unsigned integer,
		// but we cannot since that would break backwards compatibility.
		b = strconv.AppendInt(b, int64(v), 10)
		b = append(b, 'i')
	case float32:
		b = strconv.AppendFloat(b, float64(v), 'f', -1, 32)
	case []byte:
		b = append(b, v...)
	case nil:
		// skip
	default:
		// Can't determine the type, so convert to string
		b = append(b, '"')
		b = append(b, []byte(models.EscapeStringField(fmt.Sprintf("%v", v)))...)
		b = append(b, '"')

	}

	return b
}
