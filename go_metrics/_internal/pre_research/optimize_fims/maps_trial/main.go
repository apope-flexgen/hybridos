package main

import (
	"fims"
	"fmt"
	"log"
	"math"
	"math/rand"
	"time"

	// "os"
	// "log"
	"strconv"
	// "strings"
)

// ----------------------------------------------------------------
var SIMS = 100
var NumFimsMsgs = 100

//----------------------------------------------------------------

var AddToCsvInterval = 1

var numRun = 6

var csvFileIn = "data/headers.csv"
var csvFileOut = "data/results" + strconv.Itoa(numRun) + ".csv"

var MapTypes = []string{
	"goMap",
	//"hashMap",
	"syncMap",
	"cmap",
}

var uris []string

type IncomingJob struct {
	Uri     string
	Job     int
	FimsMsg fims.FimsMsg
}

type Msg struct {
	Job         int
	FimsMsgChan chan fims.FimsMsg
	DataChan    chan interface{}
}

type DataPreparationTimeInfo struct {
	Worker int
	Job    int
	Time   time.Duration
}

type ProcessingTimeInfo struct {
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

func sharedTimeTrack(numWorkers int, dataPreparationTimeChannel chan DataPreparationTimeInfo, processingTimeChannel chan ProcessingTimeInfo, totalTime time.Duration) (info SimInfo) {
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

	lenOfDataPrep := len(dataPreparationTimeChannel)
	for x := 0; x < lenOfDataPrep; x++ {

		select {
		case currInfo := <-dataPreparationTimeChannel:
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

	}

	unmarshalAvgTime := unmarshalTotalTime / time.Duration(NumFimsMsgs)

	lenOfProcessing := len(processingTimeChannel)

	for x := 0; x < lenOfProcessing; x++ {

		select {
		case currInfo := <-processingTimeChannel:

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
	}

	processAvgTime := processTotalTime / time.Duration(NumFimsMsgs)

	return SimInfo{
		NumJobs:    NumFimsMsgs,
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

func sharedDataPreparation(pointerToVmap *interface{}, msg fims.FimsMsg) (fims.FimsMsg, interface{}) {
	var fx interface{}

	vmap := *pointerToVmap

	switch {
	case msg.Method == "set" && msg.Replyto != "": //set with replyTo
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
	case msg.Method == "set": //set without replyTo
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

	return msg, fx
}

func sharedMethodProcessing(pointerToVmap *interface{}, fimsMsg fims.FimsMsg, uri string, data interface{}) {

	vmap := *pointerToVmap
	switch {
	case fimsMsg.Method == "set" && fimsMsg.Replyto != "": //set with replyTo
		vmap = runMergeJsons(vmap, uri, data, vmap, 0, 0)
		fimsMsg.Body = vmap
		//Send back fims msg created in hashMapDataPreparation method to client uri
		f.Send(fimsMsg)
	case fimsMsg.Method == "set": //set without replyTo
		vmap = runMergeJsons(vmap, uri, data, vmap, 0, 0)
	case fimsMsg.Method == "get" && fimsMsg.Replyto != "":
		//Send back fims msg created in hashMapDataPreparation method to client uri
		f.Send(fimsMsg)
	case fimsMsg.Method == "post":
	case fimsMsg.Method == "del":
	case fimsMsg.Method == "pub":
		vmap = runMergeJsons(vmap, uri, data, vmap, 0, 0)
	default:
	}
}

func main() {

	// go publishFims()

	uris = append(uris, "/echo_test_1")
	uris = append(uris, "/echo_test_2")
	uris = append(uris, "/echo_test_3")
	uris = append(uris, "/echo_test_4")
	uris = append(uris, "/echo_test_5")
	uris = append(uris, "/echo_test_6")

	setupFims(uris)

	publishFims, err := fims.Connect(clientUri)
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	msgs := createRandomFimsMsgs(SIMS, NumFimsMsgs)

	// time.Sleep(500 * time.Millisecond)

	// for x := 0; x < len(msgs); x++ {
	// 	row := msgs[x]
	// 	for y := 0; y < len(row); y++ {
	// 		fmt.Printf("\n\n Map:%v Message#:%v    Data: %v \n\n", x, y, row[y])
	// 	}
	// }

	// bestMapType := ""
	// bestAvgTime := time.Duration(time.Hour)

	goMapWinnerCount := 0
	//hashMapWinnerCount := 0
	syncMapWinnerCount := 0
	cmapWinnerCount := 0

	goMapDiffCounter := 0.0
	//hashMapDiffCounter := 0.0
	syncMapDiffCounter := 0.0
	cmapDiffCounter := 0.0

	rows := readOrders(csvFileIn)

	for i := range msgs {

		// fmt.Println("i is ", i)
		fimsMsgSet := msgs[i]

		// for y := 0; y < len(fimsMsgSet); y++ {
		// 	fmt.Printf("\n\n Message#:%v    Data: %v \n\n", y, fimsMsgSet[y])
		// }

		bestTime := time.Duration(time.Hour)
		bestMap := ""

		Date := ""
		NumSims := SIMS
		//NumFimsMsgs
		SimId := i
		WinningMap := ""
		WinningTime := time.Duration(0)
		GoMapTime := time.Duration(0)
		WinGoMapDiff := 0.0
		//HashMapTime := time.Duration(0)
		//WinHashMapDiff := 0.0
		SyncMapTime := time.Duration(0)
		WinSyncMapDiff := 0.0
		cmapTime := time.Duration(0)
		WincmapDiff := 0.0

		for _, mapType := range MapTypes {

			// fmt.Printf("before publishPreCreatedMsgs for %v on %v \n", mapType, i)
			go publishPreCreatedMsgs(fimsMsgSet, publishFims)
			// fmt.Printf("after publishPreCreatedMsgs for %v on %v \n", mapType, i)

			// totalTimeForMapType := time.Duration(0)

			// SetBaseTime()

			// if len(os.Args) > 1 {
			// 	fmt.Println("arg :", os.Args[1])
			// 	if os.Args[1] == "test" {
			// 		runTest()
			// 		return
			// 	}

			// }

			//if there is some sort of starting data or config that should be in vmap it would be added here
			var vmap interface{}

			//Three current options for Map type used

			//1. go built in map      		https://go.dev/blog/maps
			// currStats := goMapSim(NumFimsMsgs, &vmap, uris)

			//2. hashmap package			https://pkg.go.dev/github.com/cornelk/hashmap#Map
			// currStats := hashMapSim(NumFimsMsgs, &vmap, uris)

			//3. map in the sync package	https://pkg.go.dev/sync#Map
			// currStats := syncMapSim(NumFimsMsgs, &vmap, uris)

			currStats := SimInfo{}

			switch mapType {
			case MapTypes[0]: //goMap
				currStats = goMapSim(NumFimsMsgs, &vmap, uris)
				GoMapTime = currStats.TotalTime
			// case MapTypes[1]: //hashMap
			// 	currStats = hashMapSim(NumFimsMsgs, &vmap, uris)
			// 	HashMapTime = currStats.TotalTime
			case MapTypes[1]: //syncMap
				currStats = syncMapSim(NumFimsMsgs, &vmap, uris)
				SyncMapTime = currStats.TotalTime
			case MapTypes[2]: //syncMap
				currStats = cmapSim(NumFimsMsgs, &vmap, uris)
				cmapTime = currStats.TotalTime
			}

			// fmt.Println()
			// fmt.Println()
			// fmt.Println("                                     Sim", i)
			// fmt.Printf(" Number of FIMS Messages:                                           %v \n", currStats.NumJobs)
			// fmt.Printf(" Number of workers spawned for Data Preparation Stage:              %v \n", currStats.NumWorkers)
			// fmt.Println()
			// fmt.Println()
			// fmt.Println()
			// fmt.Printf(" Min time for Data Preparation (Marshal or Unmarshal):              %v \n", currStats.MinTimeForUnmarshalling)
			// fmt.Printf(" Max time for Data Preparation (Marshal or Unmarshal):              %v \n", currStats.MaxTimeForUnmarshalling)
			// fmt.Printf(" Avg time for Data Preparation (Marshal or Unmarshal):              %v \n", currStats.AvgTimeForUnmarshalling)
			// fmt.Printf(" Total time for Data Preparation (Marshal or Unmarshal):            %v \n", currStats.TotalTimeForUnmarshalling)
			// fmt.Println()
			// fmt.Println()
			// fmt.Println()
			// fmt.Printf(" Min time for Processing (Merge and/or FIMS Send Reply):            %v \n", currStats.MinTimeForProcessing)
			// fmt.Printf(" Max time for Processing (Merge and/or FIMS Send Reply):            %v \n", currStats.MaxTimeForProcessing)
			// fmt.Printf(" Avg time for Processing (Merge and/or FIMS Send Reply):            %v \n", currStats.AvgTimeForProcessing)
			// fmt.Printf(" Total time for Processing (Merge and/or FIMS Send Reply):          %v \n", currStats.TotalTimeForProcessing)
			// fmt.Println()
			// fmt.Println()
			// fmt.Println()
			// fmt.Printf(" Total time:                                                        %v \n", currStats.TotalTime)

			// fmt.Println("-----------------------------------------------------------------------------------")

			// totalTimeForMapType += currStats.TotalTime
			if currStats.TotalTime < bestTime {
				bestTime = currStats.TotalTime
				bestMap = mapType
			}

		}

		switch bestMap {
		case MapTypes[0]: //goMap
			goMapWinnerCount++
		// case MapTypes[1]: //hashMap
		// 	hashMapWinnerCount++
		case MapTypes[1]: //syncMap
			syncMapWinnerCount++
		case MapTypes[2]: //syncMap
			cmapWinnerCount++
		}

		// currAvgTime := (totalTimeForMapType / time.Duration(SIMS))
		// fmt.Printf(" Avg Total Time | %v Sims each of %v FimsMsgs | [%v]:		%v \n\n\n", SIMS, NumFimsMsgs, mapType, currAvgTime)
		fmt.Printf("Winner of Sim %v is [%v] with a time of:		%v \n", i+1, bestMap, bestTime)

		// if currAvgTime < bestAvgTime {
		// 	bestAvgTime = currAvgTime
		// 	bestMapType = mapType
		// }

		Date = time.Now().String()
		// NumSims := SIMS
		//NumFimsMsgs
		// SimId := i
		WinningMap = bestMap
		WinningTime = bestTime
		// GoMapTime := time.Duration(0)
		WinGoMapDiff = float64(GoMapTime) / float64(WinningTime)
		// HashMapTime = time.Duration(0)
		//WinHashMapDiff = float64(HashMapTime) / float64(WinningTime)
		// SyncMapTime = time.Duration(0)
		WinSyncMapDiff = float64(SyncMapTime) / float64(WinningTime)
		WincmapDiff = float64(cmapTime) / float64(WinningTime)

		goMapDiffCounter += WinGoMapDiff
		//hashMapDiffCounter += WinHashMapDiff
		syncMapDiffCounter += WinSyncMapDiff
		cmapDiffCounter += WincmapDiff

		rows = append(rows, []string{
			Date,
			strconv.Itoa(NumSims),
			strconv.Itoa(NumFimsMsgs),
			strconv.Itoa(SimId),
			WinningMap,
			WinningTime.String(),
			GoMapTime.String(),
			fmt.Sprintf("%.2f", WinGoMapDiff),
			// HashMapTime.String(),
			// fmt.Sprintf("%.2f", WinHashMapDiff),
			SyncMapTime.String(),
			fmt.Sprintf("%.2f", WinSyncMapDiff),
			cmapTime.String(),
			fmt.Sprintf("%.2f", WincmapDiff),
		})

		if (i+1)%(AddToCsvInterval) == 0 {
			writeOrders(csvFileOut, rows)
		}

	}

	// breakOut := false
	// for {
	// 	length := len(fimsReceive)
	// 	switch length {
	// 	case 0:
	// 		breakOut = true
	// 	default:
	// 		<-fimsReceive
	// 		continue
	// 	}
	// 	if breakOut {
	// 		break
	// 	}
	// }

	goMapDiffCounter = math.Round(goMapDiffCounter*100) / 100
	//hashMapDiffCounter = math.Round(hashMapDiffCounter*100) / 100
	syncMapDiffCounter = math.Round(syncMapDiffCounter*100) / 100
	cmapDiffCounter = math.Round(cmapDiffCounter*100) / 100

	fmt.Println()
	// fmt.Printf("\n\n The Fastest Map is [%v] with an avg time of:			 %v \n\n\n", bestMapType, bestAvgTime)
	fmt.Printf("Across %v Sims of %v FimsMsgs 		\n", SIMS, NumFimsMsgs)
	fmt.Println()
	fmt.Printf("[%v] was the winner %v times 		\n", MapTypes[0], goMapWinnerCount)
	//fmt.Printf("[%v] was the winner %v times		\n", MapTypes[1], hashMapWinnerCount)
	fmt.Printf("[%v] was the winner %v times	 	\n", MapTypes[2], syncMapWinnerCount)
	fmt.Printf("[%v] was the winner %v times	 	\n", MapTypes[3], cmapWinnerCount)
	fmt.Println()
	fmt.Println()
	fmt.Printf("[%v] was on average %v%% away from the winning time \n", MapTypes[0], fmt.Sprintf("%.2f", 100*(float64(goMapDiffCounter-float64(SIMS))/float64(SIMS))))
	//fmt.Printf("[%v] was on average %v%% away from the winning time \n", MapTypes[1], fmt.Sprintf("%.2f", 100*(float64(hashMapDiffCounter-float64(SIMS))/float64(SIMS))))
	fmt.Printf("[%v] was on average %v%% away from the winning time \n", MapTypes[2], fmt.Sprintf("%.2f", 100*(float64(syncMapDiffCounter-float64(SIMS))/float64(SIMS))))
	fmt.Printf("[%v] was on average %v%% away from the winning time \n", MapTypes[3], fmt.Sprintf("%.2f", 100*(float64(cmapDiffCounter-float64(SIMS))/float64(SIMS))))
	fmt.Println()

}
