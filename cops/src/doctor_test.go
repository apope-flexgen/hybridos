package main

import (
	"testing"
	"time"
)

// healthCheckup unit test
func TestHealthCheckup(t *testing.T) {
	// Initialize test infrastructure
	processJurisdiction = setupMockProcessJurisdiction()

	////////////////////////////////////////// TEST 1 //////////////////////////////////////////////
	// all processes are required
	// subscores equally weighted
	// health conditions perfect
	processJurisdiction["site_controller"].requiredForHealthyStatus = true
	processJurisdiction["metrics"].requiredForHealthyStatus = true
	processJurisdiction["modbus_client"].requiredForHealthyStatus = true
	// reset doctor object
	dr = doctor{}
	dr.configure(1000)
	// set all subscores to be equally weighted
	setDrWeights(1, 1, 1)
	// set all total restarts to 0
	setRestarts("site_controller", 0)
	setRestarts("metrics", 0)
	setRestarts("modbus_client", 0)
	// set all average response times to match heartbeat frequency
	processJurisdiction["site_controller"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["metrics"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["modbus_client"].healthStats.avgResponseTimeMS = 1000
	// set all memory usages to 0%, should give perfect score
	processJurisdiction["site_controller"].healthStats.avgMemUsagePercent = 0
	processJurisdiction["metrics"].healthStats.avgMemUsagePercent = 0
	processJurisdiction["modbus_client"].healthStats.avgMemUsagePercent = 0
	// get and test result
	result := dr.healthCheckup()
	if result != 1.0 {
		t.Errorf("doctor.healthCheckup() failed test 1: returned %v but expected 1.0", result)
	}

	////////////////////////////////////////// TEST 2 //////////////////////////////////////////////
	// all processes are required
	// subscores NOT equally weighted
	// health conditions perfect
	processJurisdiction["site_controller"].requiredForHealthyStatus = true
	processJurisdiction["metrics"].requiredForHealthyStatus = true
	processJurisdiction["modbus_client"].requiredForHealthyStatus = true
	// reset doctor object
	dr = doctor{}
	dr.configure(1000)
	// set the weight of the restart rate subscore to be double the other subscores
	setDrWeights(2, 1, 1)
	// set all total restarts to 0
	setRestarts("site_controller", 0)
	setRestarts("metrics", 0)
	setRestarts("modbus_client", 0)
	// set all average response times to match heartbeat frequency
	processJurisdiction["site_controller"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["metrics"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["modbus_client"].healthStats.avgResponseTimeMS = 1000
	// set all memory usages to 1%, should give perfect score
	processJurisdiction["site_controller"].healthStats.avgMemUsagePercent = 1
	processJurisdiction["metrics"].healthStats.avgMemUsagePercent = 1
	processJurisdiction["modbus_client"].healthStats.avgMemUsagePercent = 1
	// get and test result
	result = dr.healthCheckup()
	if result != 1.0 {
		t.Errorf("doctor.healthCheckup() failed test 2: returned %v but expected 1.0", result)
	}

	////////////////////////////////////////// TEST 3 //////////////////////////////////////////////
	// all processes are required
	// subscores NOT equally weighted
	// one process is restarting about once per second
	processJurisdiction["site_controller"].requiredForHealthyStatus = true
	processJurisdiction["metrics"].requiredForHealthyStatus = true
	processJurisdiction["modbus_client"].requiredForHealthyStatus = true
	// reset doctor object
	dr = doctor{}
	dr.configure(1000)
	// set the weight of the restart rate subscore to be double the other subscores
	setDrWeights(2, 1, 1)
	// set all total restarts to 0 except for site_controller, which will have about one restart per second
	setRestarts("site_controller", 30)
	setRestarts("metrics", 0)
	setRestarts("modbus_client", 0)
	// set all average response times to match heartbeat frequency
	processJurisdiction["site_controller"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["metrics"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["modbus_client"].healthStats.avgResponseTimeMS = 1000
	// set all memory usages to average to 2%, should give perfect score
	processJurisdiction["site_controller"].healthStats.avgMemUsagePercent = 1
	processJurisdiction["metrics"].healthStats.avgMemUsagePercent = 3
	processJurisdiction["modbus_client"].healthStats.avgMemUsagePercent = 2
	// get and test result
	result = dr.healthCheckup()
	if !(result < 0.84 && result > 0.83) {
		t.Errorf("doctor.healthCheckup() failed test 3: returned %v but expected ~0.8333", result)
	}

	////////////////////////////////////////// TEST 4 //////////////////////////////////////////////
	// just one process is required
	// subscores NOT equally weighted
	// required process is restarting about once every two seconds
	// memory usage is up
	processJurisdiction["site_controller"].requiredForHealthyStatus = true
	processJurisdiction["metrics"].requiredForHealthyStatus = false
	processJurisdiction["modbus_client"].requiredForHealthyStatus = false
	// reset doctor object
	dr = doctor{}
	dr.configure(1000)
	// set the weight of the restart rate subscore to be double the other subscores
	setDrWeights(2, 1, 1)
	// set all total restarts to 0 except for site_controller, which will have about one restart every two seconds
	setRestarts("site_controller", 15)
	setRestarts("metrics", 0)
	setRestarts("modbus_client", 0)
	// set all average response times to match heartbeat frequency
	processJurisdiction["site_controller"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["metrics"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["modbus_client"].healthStats.avgResponseTimeMS = 1000
	// set all memory usages to average to 6%, should give subscore of 0.5
	processJurisdiction["site_controller"].healthStats.avgMemUsagePercent = 6
	processJurisdiction["metrics"].healthStats.avgMemUsagePercent = 7
	processJurisdiction["modbus_client"].healthStats.avgMemUsagePercent = 5
	// get and test result
	result = dr.healthCheckup()
	if !(result < 0.63 && result > 0.62) {
		t.Errorf("doctor.healthCheckup() failed test 4: returned %v but expected ~0.625", result)
	}

	////////////////////////////////////////// TEST 5 //////////////////////////////////////////////
	// two processes are required
	// subscores NOT equally weighted
	// one required process is restarting about once every two seconds
	// another required process has been dead for a while
	processJurisdiction["site_controller"].requiredForHealthyStatus = true
	processJurisdiction["metrics"].requiredForHealthyStatus = true
	processJurisdiction["modbus_client"].requiredForHealthyStatus = false
	// reset doctor object
	dr = doctor{}
	dr.configure(1000)
	// set the weight of the restart rate subscore to be double the other subscores
	setDrWeights(2, 1, 1)
	// set all total restarts to 0 except for site_controller, which will have about one restart every two seconds
	setRestarts("site_controller", 15)
	setRestarts("metrics", 0)
	setRestarts("modbus_client", 0)
	// set all average response times to match heartbeat frequency, expect for metrics which is marked as unresponsive for a while
	processJurisdiction["site_controller"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["metrics"].healthStats.avgResponseTimeMS = 11000
	processJurisdiction["modbus_client"].healthStats.avgResponseTimeMS = 1000
	// set all memory usages to 1%, should give perfect score
	processJurisdiction["site_controller"].healthStats.avgMemUsagePercent = 1
	processJurisdiction["metrics"].healthStats.avgMemUsagePercent = 1
	processJurisdiction["modbus_client"].healthStats.avgMemUsagePercent = 1
	// get and test result
	result = dr.healthCheckup()
	if !(result < 0.63 && result > 0.62) {
		t.Errorf("doctor.healthCheckup() failed test 5: returned %v but expected ~0.625", result)
	}

	////////////////////////////////////////// TEST 6 //////////////////////////////////////////////
	// two processes are required
	// subscores NOT equally weighted
	// memory usage is up
	// one required process has been dead for a while
	processJurisdiction["site_controller"].requiredForHealthyStatus = false
	processJurisdiction["metrics"].requiredForHealthyStatus = true
	processJurisdiction["modbus_client"].requiredForHealthyStatus = true
	// reset doctor object
	dr = doctor{}
	dr.configure(1000)
	// set the weight of the downtime subscore to be double the other subscores
	setDrWeights(1, 2, 1)
	// set all total restarts to 0 except for site_controller, which will have about one restart every two seconds
	setRestarts("site_controller", 15)
	setRestarts("metrics", 0)
	setRestarts("modbus_client", 0)
	// set all average response times to match heartbeat frequency, expect for metrics which is marked as unresponsive for a while
	processJurisdiction["site_controller"].healthStats.avgResponseTimeMS = 1000
	processJurisdiction["metrics"].healthStats.avgResponseTimeMS = 11000
	processJurisdiction["modbus_client"].healthStats.avgResponseTimeMS = 1000
	// set all memory usages to average to 10%, which should give subscore of 0
	processJurisdiction["site_controller"].healthStats.avgMemUsagePercent = 15
	processJurisdiction["metrics"].healthStats.avgMemUsagePercent = 0
	processJurisdiction["modbus_client"].healthStats.avgMemUsagePercent = 15
	// get and test result
	result = dr.healthCheckup()
	if !(result < 0.26 && result > 0.24) {
		t.Errorf("doctor.healthCheckup() failed test 6: returned %v but expected ~0.25", result)
	}
}

// fills recentRestarts slice with the amount of restarts indicated by totalRestarts
func setRestarts(processName string, numRestarts int) {
	processJurisdiction[processName].healthStats.totalRestarts = numRestarts
	processJurisdiction[processName].healthStats.recentRestarts = nil
	i := 0
	for i < numRestarts {
		processJurisdiction[processName].healthStats.recentRestarts = append(processJurisdiction[processName].healthStats.recentRestarts, time.Now())
		i++
	}
	// add an old restart to test recentRestarts filter
	processJurisdiction[processName].healthStats.recentRestarts = append(processJurisdiction[processName].healthStats.recentRestarts, time.Now().Add(-time.Minute))
}

func setDrWeights(restartRateWt, downtimeWt, memUsgWt float64) {
	dr.restartRateWeight = restartRateWt
	dr.downtimeWeight = downtimeWt
	dr.memUsgWeight = memUsgWt
}
