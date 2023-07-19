package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"

	"encoding/json"
	"fims"
	// "github.com/flexgen-power/fims"
	"log"
	"os"
	"os/exec"
	hashmap "github.com/cornelk/hashmap"
)

var SIMS = 5
var MAX_JOBS = 3000
var MIN_JOBS = 3000

var MIN_UNMARSHAL_TIME = 250 //250 microseconds
var MAX_UNMARSHAL_TIME = 500 //500 microseconds
var MIN_PROCESS_TIME = 25    //25 microseconds
var MAX_PROCESS_TIME = 2000  //2 milliseconds
// var JOB_INPUT_DELAY = time.Duration(1/2) * time.Microsecond
// var WORKER_CREATION_DELAY = time.Duration(1/2) * time.Microsecond

// var uriArr = [15]string{"ess", "bms", "pcs", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"} //uris that are subscribed to
// var uriArr = [1]string{"ess"}
// var uriArr = [6]string{"ess", "bms", "pcs", "a", "b", "c"} //uris that are subscribed to


var totalTimeForProcessing = time.Duration(0)


type UriChannelMapObject struct {
	// UriChannelMap map[string]chan Msg
	HashMapPointer *hashmap.Map[string, chan Msg]
	Mu            sync.RWMutex
}

type IncomingJob struct {
	Uri string
	Job int
	FimsMsg fims.FimsMsg
}

type Msg struct {
	Job  int
	FimsMsgChan chan fims.FimsMsg
	DataChan chan interface{}
}

type WorkerUnmarshalTimeInfo struct {
	Worker int
	Job    int
	Time   time.Duration
}

type ProcessTimeInfo struct {
	Job  int
	Time time.Duration
}

type SimInfo struct {
	NumJobs    int
	NumWorkers int

	AvgTimeForUnmarshalling time.Duration

	MinTimeForUnmarshalling time.Duration
	MinUnmarshalWorker      int
	MinUnmarshalJob         int

	MaxTimeForUnmarshalling time.Duration
	MaxUnmarshalWorker      int
	MaxUnmarshalJob         int

	TotalTimeForUnmarshalling time.Duration

	AvgTimeForProcessing time.Duration
	MinTimeForProcessing time.Duration
	MinProcessJob        int

	MaxTimeForProcessing time.Duration
	MaxProcessJob        int

	TotalTimeForProcessing time.Duration

	TotalTime time.Duration
}

func getRandTime(min int, max int) (randTime time.Duration) {
	rand.Seed(time.Now().UnixNano())
	num := rand.Intn(max-min) + min
	return (time.Duration(num) * time.Microsecond)
}

func worker(
	id int, 
	jobs <-chan IncomingJob, 
	uriChannelMapObject UriChannelMapObject, 
	progress chan bool, 
	timeCount chan WorkerUnmarshalTimeInfo, 
	wg *sync.WaitGroup, 

	jobsDone chan bool, 
	processingTimeCount chan ProcessTimeInfo, 
	wgForReadData *sync.WaitGroup,
	pointerToVmap *interface{}) {

	defer wg.Done()

	for j := range jobs {

		// uriChannelMapObject.Mu.RLock()
		// _, ok := uriChannelMapObject.UriChannelMap[j.Uri]
		// uriChannelMapObject.Mu.RUnlock()

		hashMapPointer := uriChannelMapObject.HashMapPointer
		uriChannelMap := *hashMapPointer
		_, ok := uriChannelMap.Get(j.Uri)


		if !ok {

			// uriChannelMapObject.Mu.Lock()
			// uriChannelMapObject.UriChannelMap[j.Uri] = make(chan Msg, 3000)
			// uriChannelMapObject.Mu.Unlock()
			uriChannelMap.Insert(j.Uri, make(chan Msg, 3000))


			wgForReadData.Add(1)
			go readData(uriChannelMapObject, jobsDone, j.Uri, processingTimeCount, wgForReadData, pointerToVmap)
		}

		start := time.Now()

		fimsMsgChan := make(chan fims.FimsMsg, 1)
		dataChan := make(chan interface{}, 1)


		// uriChannelMapObject.Mu.RLock()
		// uriChannelMapObject.UriChannelMap[j.Uri] <- Msg{Job: j.Job, FimsMsgChan:fimsMsgChan, DataChan: dataChan}
		// uriChannelMapObject.Mu.RUnlock()
		msgs, _ := uriChannelMap.Get(j.Uri)
		msgs <- Msg{Job: j.Job, FimsMsgChan:fimsMsgChan, DataChan: dataChan}
		
		


		var fx interface{}
		msg := j.FimsMsg

		vmap := *pointerToVmap

		// fimsmsg methods are set, get, post, del, pub, sub
		switch {
			case msg.Method == "set" && msg.Replyto != "":    //set with replyTo
				err := runjUnmarshal(msg.Body.([]byte), &fx)
				if err != nil {
					fmt.Println("json error", err)
					fmt.Println(string(msg.Body.([]byte)))
				}	
				msg = fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   0,
				}
			case msg.Method == "set":	//set without replyTo
				err := runjUnmarshal(msg.Body.([]byte), &fx)
				if err != nil {
					fmt.Println("json error", err)
					fmt.Println(string(msg.Body.([]byte)))
				}				
			case msg.Method == "get" && msg.Replyto != "": 
				
				_, err := runjMarshal(&vmap)
				if err != nil {
					fmt.Println(err)
					fmt.Println(vmap)
				}
				
				//Change msg to the new fims msg that will be sent to reply to uri in the read data method
				msg = fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   vmap,
				}

			case msg.Method == "post":
			case msg.Method == "del":
			case msg.Method == "pub":  
				err := runjUnmarshal(msg.Body.([]byte), &fx)
				if err != nil {
					fmt.Println("json error", err)
					fmt.Println(string(msg.Body.([]byte)))
				}		
			default:

		}



		progress <- true // fastest goroutine done
		fimsMsgChan <- msg
		dataChan <- fx   // Add data to the data channel, which is maintained by the Msg struct

		end := time.Since(start)

		info := WorkerUnmarshalTimeInfo{
			Worker: id,
			Job:    j.Job,
			Time:   end,
		}

		timeCount <- info

	}

}

// readData reads a message from a channel and extracts the job number and data from the message
func readData(uriChannelMapObject UriChannelMapObject, jobsDone chan bool, uri string, timeCount chan ProcessTimeInfo, wg *sync.WaitGroup, pointerToVmap *interface{}) {


	defer wg.Done()

	count := 0

	// uriChannelMapObject.Mu.RLock()
	// msgs := uriChannelMapObject.UriChannelMap[uri]
	// uriChannelMapObject.Mu.RUnlock()

	hashMapPointer := uriChannelMapObject.HashMapPointer
	uriChannelMap := *hashMapPointer
	msgs, _ := uriChannelMap.Get(uri)

	for {
		// fmt.Printf(" \n ReadData for (uri: %v) For loop run (count: %v) \n", uri, count)

		// fmt.Println("msgs is: ", msgs)

		if len(msgs) > 0 {
			msg, ok := <-msgs
			if !ok {
				break
			}

			start := time.Now()

			fimsMsg := <-msg.FimsMsgChan
			data := <- msg.DataChan

			count++

			vmap := *pointerToVmap
			switch {
			case fimsMsg.Method == "set" && fimsMsg.Replyto != "":    //set with replyTo
				vmap = runMergeJsons(vmap, uri, data, vmap, 0, 0)		
				fimsMsg.Body = vmap
				//Send back fims msg created in worker method to client uri
				f.Send(fimsMsg)
			case fimsMsg.Method == "set":	//set without replyTo
				vmap = runMergeJsons(vmap, uri, data, vmap, 0, 0)		
			case fimsMsg.Method == "get" && fimsMsg.Replyto != "": 
				//Send back fims msg created in worker method to client uri
				f.Send(fimsMsg)
			case fimsMsg.Method == "post":
			case fimsMsg.Method == "del":
			case fimsMsg.Method == "pub":  
				vmap = runMergeJsons(vmap, uri, data, vmap, 0, 0)	
			default:
		}



			end := time.Since(start)

			info := ProcessTimeInfo{
				Job:  msg.Job,
				Time: end,
			}

			timeCount <- info
		} else {
			select {
			case <-jobsDone:
				return
			default:
			}
		}

	}
	fmt.Println("Broke out of ReadData for loop")
	close(msgs)
}

// func simWorkerPool(numJobs int, minProcessTime int, maxProcessTime int, jobInputDelay time.Duration, workerCreationDelay time.Duration)(info SimInfo) {
func simWorkerPool(numJobs int, pointerToVmap *interface{}) (info SimInfo) {

	uriChannelMapObject := UriChannelMapObject{
		// UriChannelMap: make(map[string]chan Msg),
		HashMapPointer: hashmap.New[string, chan Msg](),
	}

	var wg sync.WaitGroup

	var wgForUnmarshalling sync.WaitGroup
	var wgForReadData sync.WaitGroup

	jobs := make(chan IncomingJob, numJobs)
	jobsDone := make(chan bool)

	progress := make(chan bool, numJobs)

	//channel specific to processing the unmarshalled data

	unmarshallingTimeCount := make(chan WorkerUnmarshalTimeInfo, numJobs)
	processingTimeCount := make(chan ProcessTimeInfo, numJobs)

	statusChan := make(chan bool, 1)


	numWorkers := 0

	start := time.Now()

	wg.Add(1)
	// Goroutine used to read a status value from a progress channel
	go func() {
		defer wg.Done()
		statusChan<- <-progress
	}()

		// Go routine for spawning workers
		wg.Add(1)
		go func() {
			defer wg.Done()
			for w := 1; w <= numJobs; w++ {
				
				select {
				case <-statusChan:
					close(statusChan)
					return
				default:
					wgForUnmarshalling.Add(1)
					go worker(w, jobs, uriChannelMapObject, progress, unmarshallingTimeCount, &wgForUnmarshalling, jobsDone, processingTimeCount, &wgForReadData, pointerToVmap)
					numWorkers++
				}
	
			}
		}()
	

	// Adding jobs
	for j := 1; j <= numJobs; j++ {

		select {
		case msg := <-fimsReceive:
			jobs <- IncomingJob{Uri: msg.Uri, Job: j, FimsMsg: msg}
		}

	}

	close(jobs)

	wg.Wait()

	wgForUnmarshalling.Wait()

	close(jobsDone)

	// for uri := range uriChannelMapObject.UriChannelMap {
	// 	close(uriChannelMapObject.UriChannelMap[uri])
	// }

	wgForReadData.Wait() //waiting until there are no more jobs being added in


	totalTime := time.Since(start)

	unmarshalTotalTime := time.Duration(0)

	minUnmarshalTime := time.Duration(time.Hour)
	minUnmarshalWorker := 0
	minUnmarshalJob := 0

	maxUnmarshalTime := time.Duration(0)
	maxUnmarshalWorker := 0
	maxUnmarshalJob := 0

	processTotalTime := time.Duration(0)

	minProcessTime := time.Duration(time.Hour)
	minProcessJob := 0

	maxProcessTime := time.Duration(0)
	maxProcessJob := 0

	for x := 0; x < numJobs; x++ {
		currInfo := <-unmarshallingTimeCount
		currTime := currInfo.Time

		unmarshalTotalTime += currTime

		if currTime < minUnmarshalTime {
			minUnmarshalTime = currTime
			minUnmarshalWorker = currInfo.Worker
			minUnmarshalJob = currInfo.Job
		}

		if currTime > maxUnmarshalTime {
			maxUnmarshalTime = currTime
			maxUnmarshalWorker = currInfo.Worker
			maxUnmarshalJob = currInfo.Job
		}
	}

	unmarshalAvgTime := unmarshalTotalTime / time.Duration(numJobs)

	for x := 0; x < numJobs; x++ {
		currInfo := <-processingTimeCount
		currTime := currInfo.Time

		processTotalTime += currTime

		if currTime < minProcessTime {
			minProcessTime = currTime
			minProcessJob = currInfo.Job

		}

		if currTime > maxProcessTime {
			maxProcessTime = currTime
			maxProcessJob = currInfo.Job
		}
	}

	processAvgTime := processTotalTime / time.Duration(numJobs)

	return SimInfo{
		NumJobs:    numJobs,
		NumWorkers: numWorkers,

		AvgTimeForUnmarshalling:   unmarshalAvgTime,
		MinTimeForUnmarshalling:   minUnmarshalTime,
		MinUnmarshalWorker:        minUnmarshalWorker,
		MinUnmarshalJob:           minUnmarshalJob,
		MaxTimeForUnmarshalling:   maxUnmarshalTime,
		MaxUnmarshalWorker:        maxUnmarshalWorker,
		MaxUnmarshalJob:           maxUnmarshalJob,
		TotalTimeForUnmarshalling: unmarshalTotalTime,

		TotalTimeForProcessing: processTotalTime,
		MinTimeForProcessing:   minProcessTime,
		MinProcessJob:          minProcessJob,
		MaxTimeForProcessing:   maxProcessTime,
		MaxProcessJob:          maxProcessJob,
		AvgTimeForProcessing:   processAvgTime,

		TotalTime: totalTime,
	}
}

func main() {

	setupFims()


	for i := 1; i <= SIMS; i++ {

		SetBaseTime()
		
		if len(os.Args) > 1 {
			fmt.Println("arg :", os.Args[1])
			if os.Args[1] == "test" {
				runTest()
				return
			}
	
		}

		//if there is some sort of starting data or config that should be in vmap it would be added here
		var vmap interface{}


		//fims messages
		numJobs := 3000

		// rand.Seed(time.Now().UnixNano())
		// numJobs := rand.Intn(MAX_JOBS - MIN_JOBS) + MIN_JOBS

		currStats := simWorkerPool(numJobs, &vmap)


		fmt.Println()
		fmt.Println()
		fmt.Println("                                     Sim", i)
		fmt.Printf(" Number of FIMS Messages:                                           %v \n", currStats.NumJobs)
		fmt.Printf(" Number of goroutines spawned for Data Preparation Stage:              %v \n", currStats.NumWorkers)
		fmt.Println()
		fmt.Println()
		fmt.Println()
		fmt.Printf(" Min time for Data Preparation (Marshal or Unmarshal):              %v \n", currStats.MinTimeForUnmarshalling)
		fmt.Printf(" Max time for Data Preparation (Marshal or Unmarshal):              %v \n", currStats.MaxTimeForUnmarshalling)
		fmt.Printf(" Avg time for Data Preparation (Marshal or Unmarshal):              %v \n", currStats.AvgTimeForUnmarshalling)
		fmt.Printf(" Total time for Data Preparation (Marshal or Unmarshal):            %v \n", currStats.TotalTimeForUnmarshalling)
		fmt.Println()
		fmt.Println()
		fmt.Println()
		fmt.Printf(" Min time for Processing (Merge and/or FIMS Send Reply):            %v \n", currStats.MinTimeForProcessing)
		fmt.Printf(" Max time for Processing (Merge and/or FIMS Send Reply):            %v \n", currStats.MaxTimeForProcessing)
		fmt.Printf(" Avg time for Processing (Merge and/or FIMS Send Reply):            %v \n", currStats.AvgTimeForProcessing)
		fmt.Printf(" Total time for Processing (Merge and/or FIMS Send Reply):          %v \n", currStats.TotalTimeForProcessing)
		fmt.Println()
		fmt.Println()
		fmt.Println()
		fmt.Printf(" Total time:                                                        %v \n", currStats.TotalTime)
		fmt.Println("-----------------------------------------------------------------------------------")

	}

}

























//functions from main.go

var f fims.Fims
var fimsReceive chan fims.FimsMsg
var uris []string
var outputTicker *time.Ticker
var pubTicker = make(chan string)
var pubs = 0
var sets = 0
var gets = 0
var baseTime time.Time

type defUri struct {
	buff   []byte
	uri    string
	format string
	flags  int
}

func SetBaseTime() {
	baseTime = time.Now()
}

func GetTimeDoubleNs() float64 {
	elapsed := time.Since(baseTime)
	ef := float64(elapsed)
	//	fmt.Println("  base :", baseTime, " elapsed:", elapsed, " ef [", ef, "]\n")
	//	fmt.Printf(" ef  type [%T] val [%f]\n", ef, ef)

	return ef

}
func setVarParam(vmap map[string]interface{}, flags int, uri, id, param string, value interface{}) (interface{}, error) {
	// decode the uri, set up flags
	//vmap := GetVmapUri(vmapin, uri)
	// switch *vmapin.(type) {
	// case map[string]interface{}:
	// 	break
	// default:
	// 	*vmapin = make(map[string]interface{})
	// }
	//vmap := *vmapin
	//	vmapin = &vmap

	myvar := vmap[uri]
	if myvar == nil {
		vmap[uri] = make(map[string]interface{})
	}
	myvar = vmap[uri]
	if myvar == nil {
		myvar.(map[string]interface{})[uri] = make(map[string]interface{})
		myvar = myvar.(map[string]interface{})[uri]
	}
	myvar = myvar.(map[string]interface{})[id]
	if myvar == nil {
		myvar = vmap[uri]
		myvar.(map[string]interface{})[id] = make(map[string]interface{})
		myvar = myvar.(map[string]interface{})[id]
	}

	// TODO lock uri
	myvar.(map[string]interface{})[param] = value
	// fmt.Println("  Metrics :", mymets, " myok:", myok, "\n")

	// TODO unlock uri
	// TODO do we have any metrics ??
	// If so and we are allowed to process metrics then run away.

	//fmt.Print(" myvar   #1 :", myvar, "\n")
	//fmt.Print(" vmap #2 :", vmap, "\n")
	// find the base uri , create one  if the noexpand flag is not set
	// find the variable  (by ID ) , create one if the noexpand flag is not set, or return , we are not interested in this uri in this message.
	// writelock the base uri
	// set the  param in the object which must be a `map[string]interface{}`
	// see if the variable has any metrics
	// write unlock component, run them if the metrics operation is enabled
	return value, nil
}

func setVar(vmap map[string]interface{}, flags int, uri, id string, value interface{}) (interface{}, error) {
	return setVarParam(vmap, flags, uri, id, "value", value)
}

func getVarParam(vmap map[string]interface{}, flags int, uri, id, param string) (interface{}, error) {
	myvar, myok := vmap[uri]
	if !myok {
		// fmt.Printf(" cannot find uri [%s] /n", uri)
		return nil, nil
	}

	myvar, myok = myvar.(map[string]interface{})[id]
	if !myok {
		// fmt.Printf(" cannot find id [%s] /n", id)
		return nil, nil
	}
	myval, myvok := myvar.(map[string]interface{})[param]
	if !myvok {
		// fmt.Printf(" cannot find param [%s] /n", param)
		return nil, nil
	}
	return myval, nil

	// decode the uri, set up flags
	// find the base uri, do not create one if missing
	// find the variable  (by ID ) , do not create one if missing
	// if metrics and metrics are enabled , run get metrics
	// writelock the base uri
	// get the param object in the variable object which must be a `map[string]interface{}`
	// writeunlock the base uri
	//	return myvar, nil
}
func getVar(vmap map[string]interface{}, flags int, uri, id string) (interface{}, error) {
	return getVarParam(vmap, flags, uri, id, "value")
}

// look for expand, metrics, full, naked,clothed, uri
// TODO look for single
// TODO look for params
// TODO look for index
func GetUri(inUri []byte, base string) *defUri {
	//TODO does this need to be deleted
	g := new(defUri)
	g.buff = inUri
	// a := bytes.Split(inUri, []byte("/"))
	// if string(a[1]) == "echo_test" {
	// 	for ii, ix := range a {
	// 		//if ii == 0 {
	// 		//	continue
	// 		//}
	// 		fmt.Printf(" ix = %d =>", ii)
	// 		fmt.Println(string(ix))
	// 	}
	// }
	ic := 0
	iterm := 0
	for ii, ix := range inUri {
		if ix == '/' && ii > 0 {
			sv := string(inUri[ic:ii])
			// fmt.Printf(" ii => %d [%s] ", ii, sv)
			if sv == "/"+base {
				// fmt.Printf(" base \n")
			} else if sv == "metrics" {
				// fmt.Printf(" metrics \n")
			} else if sv == "expand" {
				// fmt.Printf(" expand \n")
			} else if sv == "uri" {
				// fmt.Printf(" uri \n")
				iterm = ii
				break
			} else {
				iterm = ic - 1
				// fmt.Printf(" done \n")
				break
			}

			ic = ii + 1
		}
	}
	g.uri = string(inUri[iterm:])
	// fmt.Printf(" Uri at end #1 = [%s] \n", string(inUri[iterm:]))
	// fmt.Printf(" Uri at end #2 = [%s] \n", g.uri)
	return g
}

func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
	log.Printf("%s took %s", name, elapsed)
}

func runjUnmarshal(data []byte, fx *interface{}) (err error) {
	//var f interface{}
	// defer timeTrack(time.Now(), "json unmarshal")
	return json.Unmarshal(data, fx)
}

func runjMarshal(v *interface{}) (output []byte, err error) {
	// defer timeTrack(time.Now(), "json marshal")
	return json.Marshal(*v)
}

func runMergeJsons(vmap interface{}, uri string, json1, json2 interface{}, flags, depth int) (json_out interface{}) {
	//var f interface{}
	// defer timeTrack(time.Now(), "MergeJson")
	return MergeJsons(vmap, uri, json1, json2, flags, depth)
}

func getVmapVar(vmapin interface{}, uri, id string) (interface{}, error) {
	vmap := GetVmapUri(vmapin, uri)
	ret, _ := vmap.(map[string]interface{})[id]
	// fmt.Print("getVmapVar ->  id   [", id, "] ret [", ret, "] err [", reterr, "]\n")
	return ret, nil
}

func GetVmapUri(vmapin interface{}, uri string) interface{} {
	if vmapin == nil {
		vmapin = make(map[string]interface{})
	}
	vm, ok := vmapin.(map[string]interface{})[uri]
	// fmt.Println("#1 vm :", vm, " ok :", ok)
	if !ok {
		vmapin.(map[string]interface{})[uri] = make(map[string]interface{})
	}
	vm, ok = vmapin.(map[string]interface{})[uri]
	// fmt.Println("#2 vm :", vm, " ok :", ok)
	return vm
}

func setVmapVar(vmapin interface{}, uri, id string, val interface{}) (interface{}, bool) {
	vmap := GetVmapUri(vmapin, uri)
	ret, reterr := vmap.(map[string]interface{})[id]
	// fmt.Print("setVmapVar ->  id   [", id, "] ret [", ret, "] err [", reterr, "]\n")
	if !reterr {
		ret.(map[string]interface{})["value"] = val

	} else {
		vmap.(map[string]interface{})[id] = make(map[string]interface{})
		vmap.(map[string]interface{})[id].(map[string]interface{})["value"] = val
	}
	return ret, reterr
}

// run the Echo Metrics
// we have a list of inputs , may only need one
// we have an optional list of outputs may need none
// map the input to key item
// key[value] == input
// outputs[value] == input

func runMetricsEcho(vmap, key, iok interface{}) {

	val := key.(map[string]interface{})["initialInput"]
	inputs, inputsok := iok.(map[string]interface{})["inputs"]
	// fmt.Print("Echo ->  inputs  [", inputs, "] inputsok [", inputsok, "]\n")
	if inputsok {
		switch inputs.(type) {
		case []interface{}:
			for _, input := range inputs.([]interface{}) {
				// fmt.Print("Echo ->  inputix  [", inputix, "] input [", input, "]\n")
				uri := input.(map[string]interface{})["uri"]
				id := input.(map[string]interface{})["id"]

				// fmt.Print("       Echo ->  uri  [", uri, "] id [", id, "]\n")
				op, operr := getVmapVar(vmap, uri.(string), id.(string))
				// fmt.Print("             Echo ->  op  [", op, "] operr [", operr, "]\n")
				// fmt.Print("                      key  [", key, "]\n")
				if operr == nil {
					val = op
				}
			}
		}
	}
	key.(map[string]interface{})["value"] = val
	outputs, outputsok := iok.(map[string]interface{})["outputs"]
	if outputsok {
		switch outputs.(type) {
		case []interface{}:
			for _, opaok := range outputs.([]interface{}) {
				uri := opaok.(map[string]interface{})["uri"]
				id := opaok.(map[string]interface{})["id"]

				// fmt.Print("       Echo outputs ->  uri  [", uri, "] id [", id, "]\n")
				//op, operr :=
				setVmapVar(vmap, uri.(string), id.(string), val)
			}
		}
	}
}

// func runMetricsSelect(vmap, key, iok interface{}) {
// 	iok.(map[string]interface{})["inputs"]
// 	// fmt.Print("Select -> im  [", im, "] imok [", imok, "]\n")
// }

// TODO put the metrics functions into a map.
//
func runMetrics(vmap, key, vkey interface{}) {
	// defer timeTrack(time.Now(), "Running Metrics")
	// fmt.Print(" Metrics for  [", key, "] \n")
	switch vkey.(type) {
	case []interface{}:
		for _, iok := range vkey.([]interface{}) {
			// fmt.Print(" id  [", id, "] iok [", iok, "]")
			im, _ := iok.(map[string]interface{})["operation"]
			// fmt.Print(" im  [", im, "] imok [", imok, "]\n")
			switch im {
			case "echo":
				// fmt.Printf(" run metrics ==> Echo \n")
				runMetricsEcho(vmap, key, iok)
			case "select":
				// fmt.Printf(" run metrics ==> Select \n")
				// runMetricsSelect(vmap, key, iok)

			default:
				// fmt.Printf(" undefined metrics \n")

			}

		}
	default:
		// fmt.Printf(" vkey type %T\n", vkey)
	}
}

// merge jsons
// the uri tells us where to start
// we have to unpack the uri and then find the base vmap
// when we unpack the flags tell us if we can create new entries in the uris or vmap area.
// the uri is really an initial vmap entry
// uris can be "locked" using read/write access locks
//
// json1 is merged into json2
//

func MergeJsons(vmapin interface{}, uri string, json1, json2 interface{}, flags, depth int) (json_out interface{}) {
	vmap := GetVmapUri(vmapin, uri)
	iok1 := json1
	//iMap1, ok1 := iok1.(map[string]interface{})
	iMap1, _ := iok1.(map[string]interface{})
	//fmt.Print("ok1 :")
	//fmt.Println(ok1)
	if json2 == nil {
		json2 = json1
		return json2
	}
	iok2 := json2
	iMap2, _ := iok2.(map[string]interface{})
	for idx, key := range iMap1 {
		fok2 := iMap2[idx]
		//fmt.Println(fok2)
		if fok2 == nil {
			// fmt.Printf("[%d] - Merge nill [%s  %s]\n", depth, idx, key)
			// fmt.Println("        iMap1 idx ", iMap1[idx])
			// fmt.Println("        iMap2 idx ", iMap2[idx])
			// fmt.Println("        iMap1", iMap1)
			// fmt.Println("        iMap2", iMap2)
			iMap2[idx] = iMap1[idx]
		} else {
			switch key.(type) {

			case map[string]interface{}:
				// look for a key value in the map

				// fmt.Printf("[%d] - Merge map [%s  %f]\n", depth, idx, key)
				// fmt.Println("        iMap1 ", iMap1[idx])
				// fmt.Println("        iMap2 ", iMap2[idx])
				// fmt.Printf("       iMap2 Type %T \n", iMap2[idx])
				if depth == 0 {
					vkey, vok := key.(map[string]interface{})["value"]
					// if vok true then we call setValue at this level.
					if vok {
						// iMap2[key] is our variable
						// fmt.Printf("[%d] - value for [", depth)
						// fmt.Print(key)
						// fmt.Printf("] Found Value [")
						// fmt.Print(vkey)
						// fmt.Printf("]\n")
					}
					// if vok true and we have the metrics flag  then we call run metrics at this level.
					switch iMap2[idx].(type) {
					case map[string]interface{}:

						vkey, vok = iMap2[idx].(map[string]interface{})["metrics"]
						if vok {
							// fmt.Printf("[%d] - metrics test:", depth)
							// fmt.Printf(" Found Metrics [")
							// fmt.Print(vkey)
							// fmt.Printf("]\n")
							runMetrics(vmap, iMap2[idx], vkey)
						}
					}
				}

				// iMap2[idx] has to be a map[string]interface for this to work
				switch iMap2[idx].(type) {
				case map[string]interface{}:
					MergeJsons(vmap, uri, iMap1[idx], iMap2[idx], flags, depth+1)
				default:
					iMap2[idx] = iMap1[idx]
				}

			default:
				// fmt.Printf(" Replaced [%s]\n", idx)
				iMap2[idx] = iMap1[idx]
			}
			// if key.(type) == map[string]interface{} we now recurse into
			//MergeJsons(vmap, uri, iMap1[idx], &iMap2[idx], flags, depth+1)
			//
		}
		//switch keyt :=
		switch key.(type) {
		case bool:
			// fmt.Printf("Merge bool [%s  %s]\n", idx, key)
		case string:
			// fmt.Printf("Merge string [%s  %s]\n", idx, key)
		case float64:
			// fmt.Printf("Merge float [%s  %f]\n", idx, key)
		case map[string]interface{}:
			// fmt.Printf("Merge map [%s  %f]\n", idx, key)
		default:
			// fmt.Printf("Merge unknown [%s  %T]\n", idx, key)
		}
		// 				// we need to know what json1.(map[string]interface{})[idx] is
	}
	return json2
}

//var temp interface{}

func fimsSet(name, value string) {
	myval := "{\"" + name + "\":" + value + "}"
	cmd := exec.Command("/usr/local/bin/fims_send", "-mset", "-u/echo_test/foo", myval)
	err := cmd.Run()
	if err != nil {
		log.Fatal(err)
	}

}

func runTest() {
	// fmt.Println("Running test")
	fimsSet("ok", "1234")
	fimsSet("myid1", "100000")
	fimsSet("ok2", "\"hello kitty\"")
	fimsSet("ok", "{\"value\":1234}")
	fimsSet("ok", "{\"test\":444}")
	fimsSet("ok", "{\"initialInput\":44400}")
	metrics1 := "{\"id\":\"metrics1\",\"operation\":\"echo\",\"inputs\":[{\"uri\":\"/some/uri\",\"id\":\"myid1\"}]}"
	metrics2 := "{\"id\":\"metrics2\",\"operation\":\"select\"}"
	fimsSet("ok", "{\"metrics\":["+metrics1+","+metrics2+"]}")
	fimsSet("ok3", "{\"value\":5000}")

	return

}



// Connect and subscribe using fims
func setupFims() {
	var err error

	// subscribe to current process PID for config updates
	// pid := os.Getpid()
	// uris = append(uris, strconv.Itoa(pid))

	// subscribe to all inputs and outputs
	//for _, output := range cfg.Outputs {
	//	for _, input := range output.Inputs {
	//		if !findString(input.Uri, uris) {
	//			uris = append(uris, input.Uri)
	//		}
	//	}
	//	if !findString(output.Uri, uris) {
	//		uris = append(uris, output.Uri)
	//	}
	//}

	// var uriArr = [15]string{"ess", "bms", "pcs", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"}
	// var uriArr = [1]string{"ess"}
	// var uriArr = []string{"/echo_test_1", "/echo_test_2", "/echo_test_3", "/echo_test_4", "/echo_test_5", "/echo_test_6"}


	uris = append(uris, "/echo_test_1")
	uris = append(uris, "/echo_test_2")
	uris = append(uris, "/echo_test_3")
	uris = append(uris, "/echo_test_4")
	uris = append(uris, "/echo_test_5")
	uris = append(uris, "/echo_test_6")


	// uris = append(uris, "/echo_test_1", "/echo_test_2", "/echo_test_3", "/echo_test_4", "/echo_test_5", "/echo_test_6")
	// connect to fims
	f, err = fims.Connect("echo_main")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	// subscribe on fims with slice of uris using variadic args
	err = f.Subscribe(uris...)
	if err != nil {
		log.Fatal("Unable to subscribe")
	}
	// fmt.Print("subscribed to :")
	// fmt.Println(uris)

	// launch go routine for receive channel
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveRawChannel(fimsReceive)

	// fmt.Println("Got Through setupFims")
}

// func publishRate(uri string, rate float64, co chan<- string) {
// 	//var elapsed = 0
// 	var count = 0
// 	for {
// 		var tsleep = (time.Duration(rate) * time.Millisecond) //- elapsed
// 		time.Sleep(tsleep)
// 		// we may pull the data from the base vmap
// 		// process all the blocks
// 		// push data back into the base vmap
// 		// unmarshall and send
// 		s := fmt.Sprintf("Hello Kitty, Pub # %d", count)
// 		f.Send(fims.FimsMsg{
// 			Method: "set",
// 			Uri:    uri,
// 			Body:   s,
// 		})
// 		count++
// 		co <- uri
// 	}
// }