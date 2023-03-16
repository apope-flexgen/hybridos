package main

import (
	"time"
)

type doctor struct {
	totalHealthThreshold     float64
	restartRateWeight        float64
	downtimeWeight           float64
	downtimeHealthyLimitMS   float64
	downtimeUnhealthyLimitMS float64
	memUsgWeight             float64
	memUsgHealthyLimit       float64
	memUsgUnhealthyLimit     float64
	requiredProcesses        []*processInfo
}

var dr doctor

func (dr *doctor) configure(heartbeatFrequencyMS float64) {
	dr.totalHealthThreshold = 0.7

	dr.restartRateWeight = 1

	dr.downtimeWeight = 1
	dr.downtimeHealthyLimitMS = heartbeatFrequencyMS
	dr.downtimeUnhealthyLimitMS = dr.downtimeHealthyLimitMS * 5

	dr.memUsgWeight = 1
	dr.memUsgHealthyLimit = 2
	dr.memUsgUnhealthyLimit = 10

	for _, process := range processJurisdiction {
		if process.requiredForHealthyStatus {
			dr.requiredProcesses = append(dr.requiredProcesses, process)
		}
	}
}

// Uses health statistics to calculate a "Total Health Score" that can be used by controllers to decide if Secondary should take over as Primary
func (dr *doctor) healthCheckup() float64 {
	totalHealthScore := 0.0
	totalHealthScore += dr.restartRateWeight * dr.calculateRestartRateScore()
	totalHealthScore += dr.downtimeWeight * dr.calculateDowntimeScore()
	totalHealthScore += dr.memUsgWeight * dr.calculateMemUsgScore()
	return totalHealthScore / (dr.restartRateWeight + dr.downtimeWeight + dr.memUsgWeight)
}

// checks "required" processes and averages restarts per second from the last 30 seconds. returns score between 0 (unhealthy) and 1 (healthy)
func (dr *doctor) calculateRestartRateScore() float64 {
	aggRestartsPerSecond := 0.0
	for _, process := range dr.requiredProcesses {
		removeOldRestarts(process)
		aggRestartsPerSecond += float64(len(process.healthStats.recentRestarts)) / 30
	}
	avgRestartsPerSecond := aggRestartsPerSecond / float64(len(dr.requiredProcesses))
	return calculateScoreFromDroop(avgRestartsPerSecond, 0, 1)
}

// removes any restarts from the recentRestarts health stat that are not within the last 30 seconds
func removeOldRestarts(process *processInfo) {
	i := 0 // output index
	for _, restart := range process.healthStats.recentRestarts {
		if !restart.Before(time.Now().Add(time.Second * -30)) {
			// copy and increment index
			process.healthStats.recentRestarts[i] = restart
			i++
		}
	}
	process.healthStats.recentRestarts = process.healthStats.recentRestarts[:i]
}

// checks "required" processes and averages each process's average response time. returns score between 0 (unhealthy) and 1 (healthy)
func (dr *doctor) calculateDowntimeScore() float64 {
	sumAvgRespTime := 0.0
	for _, process := range dr.requiredProcesses {
		// if process is still alive, use its average response time. otherwise, use the time since COPS last heard from the process
		if process.alive {
			sumAvgRespTime += float64(process.healthStats.avgResponseTimeMS)
		} else {
			sumAvgRespTime += float64(time.Since(process.healthStats.lastConfirmedAlive).Milliseconds())
		}
	}
	avgAvgRespTime := sumAvgRespTime / float64(len(dr.requiredProcesses))
	return calculateScoreFromDroop(avgAvgRespTime, dr.downtimeHealthyLimitMS, dr.downtimeUnhealthyLimitMS)
}

// checks all processes and averages each process's average memory usage. returns score between 0 (unhealthy) and 1 (healthy)
func (dr *doctor) calculateMemUsgScore() float64 {
	sumAvgMemUsg := 0.0
	for _, process := range processJurisdiction {
		sumAvgMemUsg += float64(process.healthStats.avgMemUsagePercent)
	}
	avgAvgMemUsg := sumAvgMemUsg / float64(len(processJurisdiction))
	return calculateScoreFromDroop(avgAvgMemUsg, dr.memUsgHealthyLimit, dr.memUsgUnhealthyLimit)
}

// calculates score. values at or below healthyLimit are a 1 and values at or above unhealthyLimit are a 0. Linear slope between
func calculateScoreFromDroop(x, healthyLimit, unhealthyLimit float64) (y float64) {
	if x <= healthyLimit {
		return 1
	} else if x >= unhealthyLimit {
		return 0
	} else {
		slope := -1 / (unhealthyLimit - healthyLimit)
		diff := x - healthyLimit
		return slope*diff + 1
	}
}
