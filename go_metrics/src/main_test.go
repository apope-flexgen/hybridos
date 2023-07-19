package main

// import (
// 	"flag"
// 	//"fmt"
// 	"os"
// 	"testing"
// )

// func TestGetAndParseFile(t *testing.T) {
// 	loud := Quiet()
// 	defer loud()
// 	for _, test := range UnmarshalConfigTestCase {
// 		configErrorLocs.ErrorLocs = []ErrorLocation{}
// 		metricsConfig = MetricsFile{}
// 		os.Args = []string{"./main", test.inputFileLoc}
// 		flag.CommandLine = flag.NewFlagSet(os.Args[0], flag.ExitOnError)
// 		getAndParseFile()
// 		if errMsg, pass := compareMetricsFile(test.inputFileLoc, metricsConfig, test.expectedMetricsConfig); !pass {
// 			t.Errorf("%s", errMsg)
// 		}
// 		if len(configErrorLocs.ErrorLocs) != len(test.expectedErrors) {
// 			t.Errorf("%s: error report is unexpected length of %d (expected %d) after running UnmarshalConfig for output\n", test.inputFileLoc, len(configErrorLocs.ErrorLocs), len(test.expectedErrors))
// 		} else {
// 			for i, errLoc := range configErrorLocs.ErrorLocs {
// 				matches := true
// 				for q, testErrLoc := range test.expectedErrors {
// 					correctErrorLoc := true
// 					if len(testErrLoc.JsonLocation) == len(errLoc.JsonLocation) {
// 						for j, jsonAccessor := range errLoc.JsonLocation {
// 							if testErrLoc.JsonLocation[j] != jsonAccessor {
// 								correctErrorLoc = false
// 								break
// 							}
// 						}

// 						if correctErrorLoc {
// 							i = q
// 							break
// 						} else {
// 							i = len(test.expectedErrors)
// 						}
// 					} else {
// 						i = len(test.expectedErrors)
// 					}
// 				}
// 				if i < len(test.expectedErrors) {
// 					for j, jsonAccessor := range errLoc.JsonLocation {
// 						if test.expectedErrors[i].JsonLocation[j] != jsonAccessor || errLoc.JsonError != test.expectedErrors[i].JsonError {
// 							matches = false
// 							break
// 						}
// 					}
// 					if !matches {
// 						t.Errorf("%s: expected error report [%s] is not as expected (%s)\n", test.inputFileLoc, errLoc, test.expectedErrors[i])
// 					} else {
// 						test.expectedErrors = append(test.expectedErrors[:i], test.expectedErrors[i+1:]...)
// 					}
// 				}
// 			}
// 		}
// 	}
// }
