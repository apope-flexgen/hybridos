package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
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
var uriArr = [6]string{"ess", "bms", "pcs", "a", "b", "c"} //uris that are subscribed to


var totalTimeForProcessing = time.Duration(0)

type UriChannelMapObject struct {
	uriChannelMap map[string]chan Msg
	mu            sync.RWMutex
}

type IncomingJob struct {
	Uri string
	Job int
}

type Msg struct {
	job  int
	data chan int
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

func worker(id int, jobs <-chan IncomingJob, uriChannelMapObject UriChannelMapObject, progress chan bool, timeCount chan WorkerUnmarshalTimeInfo, wg *sync.WaitGroup) {
	defer wg.Done()
	for j := range jobs {
		start := time.Now()

		dataChan := make(chan int, 1)

		// add a lock
		uriChannelMapObject.mu.Lock()
		uriChannelMapObject.uriChannelMap[j.Uri] <- Msg{job: j.Job, data: dataChan}
		uriChannelMapObject.mu.Unlock()


		time.Sleep(getRandTime(MIN_UNMARSHAL_TIME, MAX_UNMARSHAL_TIME)) // Delay work time

		progress <- true // fastest goroutine done
		dataChan <- 1    // Add data to the data channel, which is maintained by the Msg struct

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
func readData(uriChannelMapObject UriChannelMapObject, jobsDone chan bool, uri string, timeCount chan ProcessTimeInfo, wg *sync.WaitGroup) {

	defer wg.Done()

	for {
		uriChannelMapObject.mu.RLock()
		defer uriChannelMapObject.mu.RUnlock()
		msgs := uriChannelMapObject.uriChannelMap[uri]

		if len(msgs) > 0 {
			msg, ok := <-msgs
			if !ok {
				break
			}

			start := time.Now()

			time.Sleep(getRandTime(MIN_PROCESS_TIME, MAX_PROCESS_TIME))

			end := time.Since(start)

			info := ProcessTimeInfo{
				Job:  msg.job,
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
}

// func simWorkerPool(numJobs int, minProcessTime int, maxProcessTime int, jobInputDelay time.Duration, workerCreationDelay time.Duration)(info SimInfo) {
func simWorkerPool(numJobs int) (info SimInfo) {

	uriChannelMapObject := UriChannelMapObject{
		uriChannelMap: make(map[string]chan Msg),
	}
	for _, uri := range uriArr {
		uriChannelMapObject.uriChannelMap[uri] = make(chan Msg, numJobs)
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

	status := false

	numWorkers := 0

	start := time.Now()

	wg.Add(1)
	// Goroutine used to read a status value from a progress channel
	go func() {
		defer wg.Done()
		status = <-progress
	}()

	//populating uri channels
	for _, uri := range uriArr {
		wgForReadData.Add(1)
		go readData(uriChannelMapObject, jobsDone, uri, processingTimeCount, &wgForReadData)
	}

	// Go routine for spawning workers
	go func() {
		for w := 1; w <= numJobs; w++ {
			if status {
				break
			}
			wgForUnmarshalling.Add(1)
			go worker(w, jobs, uriChannelMapObject, progress, unmarshallingTimeCount, &wgForUnmarshalling)

			numWorkers++
		}
	}()

	// Adding jobs
	for j := 1; j <= numJobs; j++ {

		rand.Seed(time.Now().UnixNano())
		index := 0
		largestArrIndex := len(uriArr) - 1
		if largestArrIndex != 0 {
			index = rand.Intn(len(uriArr) - 1)
		}
		randUri := uriArr[index]

		jobs <- IncomingJob{Uri: randUri, Job: j}
	}

	close(jobs)

	wg.Wait()

	wgForUnmarshalling.Wait()

	close(jobsDone)

	for _, uri := range uriArr {
		close(uriChannelMapObject.uriChannelMap[uri])
	}

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

	for i := 1; i <= SIMS; i++ {
		rand.Seed(time.Now().UnixNano())

		//fims messages
		numJobs := 3000
		// numJobs := rand.Intn(MAX_JOBS - MIN_JOBS) + MIN_JOBS

		currStats := simWorkerPool(numJobs)

		fmt.Println()
		fmt.Println()
		fmt.Println("                                    Sim", i)
		fmt.Printf(" Number of FIMS Messages:                             %v \n", currStats.NumJobs)
		fmt.Printf(" Number of workers spawned for unmarshalling:         %v \n", currStats.NumWorkers)
		fmt.Println()
		fmt.Println()
		fmt.Println()
		fmt.Printf(" Min time for Unmarshalling:                          %v \n", currStats.MinTimeForUnmarshalling)
		fmt.Printf(" Min Unmarshal done by (worker %v) on (job %v) \n", currStats.MinUnmarshalWorker, currStats.MinUnmarshalJob)
		fmt.Printf(" Max time for Unmarshalling:                          %v \n", currStats.MaxTimeForUnmarshalling)
		fmt.Printf(" Max Unmarshal done by (worker %v) on (job %v) \n", currStats.MaxUnmarshalWorker, currStats.MaxUnmarshalJob)
		fmt.Printf(" Avg time for Unmarshalling:                          %v \n", currStats.AvgTimeForUnmarshalling)
		fmt.Printf(" Total time for Unmarshalling:                        %v \n", currStats.TotalTimeForUnmarshalling)
		fmt.Println()
		fmt.Println()
		fmt.Println()
		fmt.Printf(" Min time for Processing a FIMS Message:              %v \n", currStats.MinTimeForProcessing)
		fmt.Printf(" Min Processing done on (job %v) \n", currStats.MinProcessJob)
		fmt.Printf(" Max time for Processing a FIMS Message:              %v \n", currStats.MaxTimeForProcessing)
		fmt.Printf(" Max Processing done on (job %v) \n", currStats.MaxProcessJob)
		fmt.Printf(" Avg time for Processing a FIMS Message:              %v \n", currStats.AvgTimeForProcessing)
		fmt.Printf(" Total time for Processing FIMS Messages in order:    %v \n", currStats.TotalTimeForProcessing)
		fmt.Println()
		fmt.Println()
		fmt.Println()
		fmt.Printf(" Total time:                                          %v \n", currStats.TotalTime)
		fmt.Println("-----------------------------------------------------------------------------------")

	}

}
