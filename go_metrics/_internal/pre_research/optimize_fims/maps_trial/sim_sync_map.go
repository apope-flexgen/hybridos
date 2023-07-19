package main

import (
	"fims"
	"fmt"
	"sync"
	"time"
)

func syncMapDataPreparation(
	id int,
	jobs <-chan IncomingJob,
	uriChannelMap sync.Map,
	progress chan bool,
	timeCount chan DataPreparationTimeInfo,
	wg *sync.WaitGroup,

	jobsDone chan bool,
	pointerToVmap *interface{}) {

	defer wg.Done()

	for j := range jobs {

		start := time.Now()

		fimsMsgChan := make(chan fims.FimsMsg, 1)
		dataChan := make(chan interface{}, 1)

		msgChan, _ := uriChannelMap.Load(j.Uri)
		assertedChan, _ := msgChan.(chan Msg)

		assertedChan <- Msg{Job: j.Job, FimsMsgChan: fimsMsgChan, DataChan: dataChan}

		msg, fx := sharedDataPreparation(pointerToVmap, j.FimsMsg)

		progress <- true // fastest goroutine done
		fimsMsgChan <- msg
		dataChan <- fx // Add data to the data channel, which is maintained by the Msg struct

		end := time.Since(start)

		info := DataPreparationTimeInfo{
			Worker: id,
			Job:    j.Job,
			Time:   end,
		}

		timeCount <- info

	}

}

// syncMapProcessing reads a message from a channel and extracts the job number and data from the message
func syncMapProcessing(uriChannelMap sync.Map, jobsDone chan bool, uri string, timeCount chan ProcessingTimeInfo, wg *sync.WaitGroup, pointerToVmap *interface{}) {

	defer wg.Done()

	count := 0

	for {

		unAssertedChan, _ := uriChannelMap.Load(uri)
		msgs := unAssertedChan.(chan Msg)

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
	fmt.Println("Broke out of ReadData for loop")
}

// func simWorkerPool(numJobs int, minProcessTime int, maxProcessTime int, jobInputDelay time.Duration, workerCreationDelay time.Duration)(info SimInfo) {
func syncMapSim(numJobs int, pointerToVmap *interface{}, uris []string) (info SimInfo) {

	uriChannelMap := sync.Map{}

	for index := range uris {
		uriChannelMap.Store(uris[index], make(chan Msg, numJobs))
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
			wgForProcessing.Add(1)
			go syncMapProcessing(uriChannelMap, jobsDone, uri, processingTimeChannel, &wgForProcessing, pointerToVmap)
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
				go syncMapDataPreparation(w, jobs, uriChannelMap, progress, dataPreparationTimeChannel, &wgForDataPreparation, jobsDone, pointerToVmap)
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

	wgForDataPreparation.Wait()

	close(jobsDone)

	for index := range uris {
		uri := uris[index]
		uriChan, _ := uriChannelMap.Load(uri)
		assertedChan := uriChan.(chan Msg)
		close(assertedChan)
	}

	wgForProcessing.Wait() //waiting until there are no more jobs being added in

	totalTime := time.Since(start)

	return sharedTimeTrack(numWorkers, dataPreparationTimeChannel, processingTimeChannel, totalTime)
}
