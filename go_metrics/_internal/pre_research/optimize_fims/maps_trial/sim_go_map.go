package main

import (
	"fims"
	"fmt"
	"sync"
	"time"
)

type UriChannelMapObject struct {
	UriChannelMap map[string]chan Msg
	Mu            sync.RWMutex
}

func goMapDataPreparation(
	id int,
	jobs <-chan IncomingJob,
	uriChannelMapObject UriChannelMapObject,
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

		uriChannelMapObject.Mu.RLock()
		uriChannelMapObject.UriChannelMap[j.Uri] <- Msg{Job: j.Job, FimsMsgChan: fimsMsgChan, DataChan: dataChan}
		uriChannelMapObject.Mu.RUnlock()

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

// goMapProcessing reads a message from a channel and extracts the job number and data from the message
func goMapProcessing(uriChannelMapObject UriChannelMapObject, jobsDone chan bool, uri string, timeCount chan ProcessingTimeInfo, wg *sync.WaitGroup, pointerToVmap *interface{}) {

	defer wg.Done()

	count := 0

	for {

		uriChannelMapObject.Mu.RLock()
		msgs := uriChannelMapObject.UriChannelMap[uri]
		uriChannelMapObject.Mu.RUnlock()

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

func goMapSim(numJobs int, pointerToVmap *interface{}, uris []string) (info SimInfo) {

	uriChannelMapObject := UriChannelMapObject{
		UriChannelMap: make(map[string]chan Msg),
	}

	for index := range uris {
		uriChannelMapObject.UriChannelMap[uris[index]] = make(chan Msg, numJobs)
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
		for uri := range uriChannelMapObject.UriChannelMap {
			wgForProcessing.Add(1)
			go goMapProcessing(uriChannelMapObject, jobsDone, uri, processingTimeChannel, &wgForProcessing, pointerToVmap)
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
				go goMapDataPreparation(w, jobs, uriChannelMapObject, progress, dataPreparationTimeChannel, &wgForDataPreparation, jobsDone, pointerToVmap)
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

	for uri := range uriChannelMapObject.UriChannelMap {
		close(uriChannelMapObject.UriChannelMap[uri])
	}

	wgForProcessing.Wait() //waiting until there are no more jobs being added in

	totalTime := time.Since(start)

	return sharedTimeTrack(numWorkers, dataPreparationTimeChannel, processingTimeChannel, totalTime)
}
