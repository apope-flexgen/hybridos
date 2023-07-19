package main

import (
	"fims"
	"fmt"
	"sync"
	"time"

	hashmap "github.com/cornelk/hashmap"
)

func hashMapDataPreparation(
	id int,
	jobs <-chan IncomingJob,
	hashMapPointer *hashmap.Map[string, chan Msg],
	progress chan bool,
	timeCount chan DataPreparationTimeInfo,
	wg *sync.WaitGroup,

	jobsDone chan bool,
	pointerToVmap *interface{}) {

	defer wg.Done()

	for j := range jobs {

		uriChannelMap := *hashMapPointer

		start := time.Now()

		fimsMsgChan := make(chan fims.FimsMsg, 1)
		dataChan := make(chan interface{}, 1)

		msgs, _ := uriChannelMap.Get(j.Uri)

		// fmt.Println("Before msgs read job: ", j.Job)
		msgs <- Msg{Job: j.Job, FimsMsgChan: fimsMsgChan, DataChan: dataChan}
		// fmt.Println("After msgs read job: ", j.Job)

		msg, fx := sharedDataPreparation(pointerToVmap, j.FimsMsg)

		// fmt.Println("Before progress read job: ", j.Job)
		progress <- true // fastest goroutine done
		// fmt.Println("After progress read job: ", j.Job)

		// fmt.Println("Before fimsMsgChan read job: ", j.Job)
		fimsMsgChan <- msg
		// fmt.Println("After fimsMsgChan read job: ", j.Job)

		// fmt.Println("Before dataChan read job: ", j.Job)
		dataChan <- fx // Add data to the data channel, which is maintained by the Msg struct
		// fmt.Println("After dataChan read job: ", j.Job)

		end := time.Since(start)

		info := DataPreparationTimeInfo{
			Worker: id,
			Job:    j.Job,
			Time:   end,
		}

		// fmt.Println("Before timeCount read job: ", j.Job)
		timeCount <- info
		// fmt.Println("After timeCount read job: ", j.Job)

		// fmt.Println("End of job", j.Job)
	}

}

// hashMapProcessing reads a message from a channel and extracts the job number and data from the message
func hashMapProcessing(hashMapPointer *hashmap.Map[string, chan Msg], jobsDone chan bool, uri string, timeCount chan ProcessingTimeInfo, wg *sync.WaitGroup, pointerToVmap *interface{}) {

	defer wg.Done()

	count := 0

	for {

		uriChannelMap := *hashMapPointer
		msgs, _ := uriChannelMap.Get(uri)

		if len(msgs) > 0 {
			msg, ok := <-msgs
			if !ok {
				break
			}

			start := time.Now()

			fimsMsg := <-msg.FimsMsgChan
			data := <-msg.DataChan

			count++

			sharedMethodProcessing(pointerToVmap, fimsMsg, uri, data)

			end := time.Since(start)

			info := ProcessingTimeInfo{
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
	fmt.Println("Broke out of processing for loop")
}

// func simWorkerPool(numJobs int, minProcessTime int, maxProcessTime int, jobInputDelay time.Duration, workerCreationDelay time.Duration)(info SimInfo) {
func hashMapSim(numJobs int, pointerToVmap *interface{}, uris []string) (info SimInfo) {

	hashMapPointer := hashmap.New[string, chan Msg]()
	uriChannelMap := *hashMapPointer

	for index := range uris {
		uri := uris[index]
		uriChannelMap.Insert(uri, make(chan Msg, numJobs))
	}

	var wg sync.WaitGroup

	var wgForDataPreparation sync.WaitGroup
	var wgForProcessing sync.WaitGroup

	jobs := make(chan IncomingJob, numJobs)
	jobsDone := make(chan bool)

	progress := make(chan bool, numJobs)

	dataPreparationTimeChannel := make(chan DataPreparationTimeInfo, numJobs)
	processingTimeChannel := make(chan ProcessingTimeInfo, numJobs)

	statusChan := make(chan bool, 1)

	numWorkers := 0

	start := time.Now()

	wg.Add(1)
	// Goroutine used to read a status value from a progress channel
	go func() {
		defer wg.Done()
		statusChan <- <-progress
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		for index := range uris {
			uri := uris[index]
			// uriChannelMap := *hashMapPointer
			// uriChannelMap.Insert(uri, make(chan Msg, numJobs))
			wgForProcessing.Add(1)
			go hashMapProcessing(hashMapPointer, jobsDone, uri, processingTimeChannel, &wgForProcessing, pointerToVmap)
		}
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
				wgForDataPreparation.Add(1)
				go hashMapDataPreparation(w, jobs, hashMapPointer, progress, dataPreparationTimeChannel, &wgForDataPreparation, jobsDone, pointerToVmap)
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

	// fmt.Println("The regular stuff finished")

	wgForDataPreparation.Wait()

	// fmt.Println("The data preparation stuff finished")

	close(jobsDone)

	for index := range uris {
		uri := uris[index]
		uriChannelMap := *hashMapPointer
		uriChannel, _ := uriChannelMap.Get(uri)
		close(uriChannel)
	}

	wgForProcessing.Wait() //waiting until there are no more jobs being added in

	// fmt.Println("The processing stuff finished")

	totalTime := time.Since(start)

	return sharedTimeTrack(numWorkers, dataPreparationTimeChannel, processingTimeChannel, totalTime)
}
