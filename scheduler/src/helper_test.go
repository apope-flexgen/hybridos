package main

import (
	"testing"
)

// interfaceEquals() method unit test for eventHeap
func TestInterfaceEquals(t *testing.T) {
	var testInterfaces []interface{}
	// zero test cases
	testInterfaces = append(testInterfaces, false, int(0), float32(0), float64(0), "", "0", "0.0")
	// nonzero test cases
	testInterfaces = append(testInterfaces, true, int(1), float32(1), float64(1), "example", "1", "1.0")

	var expectedMatches [15][]interface{}
	// Values that shoud produce a match for the test interfaces given above. Each row corresponds to a single entry in the test interfaces
	expectedMatches[0] = append(expectedMatches[0], false, int(0), float32(0), float64(0), "")     // match (bool) false
	expectedMatches[1] = append(expectedMatches[1], false, int(0), float32(0), float64(0), "0")    // match (int) 0
	expectedMatches[2] = append(expectedMatches[2], false, int(0), float32(0), float64(0), "0.0")  // match (float32) 0.0
	expectedMatches[3] = append(expectedMatches[3], false, int(0), float32(0), float64(0), "0.0")  // match (float64) 0.0
	expectedMatches[4] = append(expectedMatches[4], false, "")                                     // match (string) ""
	expectedMatches[5] = append(expectedMatches[5], int(0), "0")                                   // match (string) "0"
	expectedMatches[6] = append(expectedMatches[6], float32(0), float64(0), "0.0")                 // match (string) "0.0"
	expectedMatches[7] = append(expectedMatches[7], true, int(1), float32(2), float64(3), "foo")   // match (bool) true
	expectedMatches[8] = append(expectedMatches[8], true, int(1), float32(1), float64(1), "1")     // match (int) 1
	expectedMatches[9] = append(expectedMatches[9], true, int(1), float32(1), float64(1), "1.0")   // match (float32) 1.0
	expectedMatches[10] = append(expectedMatches[10], true, int(1), float32(1), float64(1), "1.0") // match (float64) 1.0
	expectedMatches[11] = append(expectedMatches[11], true, "example")                             // match (string) ""
	expectedMatches[12] = append(expectedMatches[12], int(1), "1")                                 // match (string) "1"
	expectedMatches[13] = append(expectedMatches[13], float32(1), float64(1), "1.0")               // match (string) "1.0"

	// Check that all expected values match
	for i, referenceInterface := range testInterfaces {
		for j, comparisonInterface := range expectedMatches[i] {
			if !interfaceEquals(referenceInterface, comparisonInterface) {
				t.Errorf("Expected match for reference[%d] = %v and comparison[%d][%d] = %v, but they were not equal", i, referenceInterface, i, j, comparisonInterface)
			}
		}
	}

	// Ensure that no unexpected values match (all false cases compared against expected true results and vice versa)
	for i, referenceInterface := range testInterfaces {
		for j, comparisonInterface := range expectedMatches[(i+7)%14] {
			if interfaceEquals(referenceInterface, comparisonInterface) {
				t.Errorf("Expected NO match for reference[%d] = %v and comparison[%d][%d] = %v, but they were equal", i, referenceInterface, i, j, comparisonInterface)
			}
		}
	}
}
