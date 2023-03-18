package main

import (
	"fmt"
	"reflect"
	"strings"
	"testing"
)

// Returns actual value if given "_actual", manual value if given "_manual", override
// flag if given "_override", or `override ? manual : actual` if given "_select".
func (mf *muxFloat) getField(search string) (interface{}, error) {
	switch search {
	case "_select":
		return mf.getSelect(), nil
	case "_manual":
		return mf.manual, nil
	case "_override":
		return mf.override, nil
	case "_actual":
		return mf.actual, nil
	default:
		return nil, fmt.Errorf("%s is not a valid suffix", search)
	}
}

// Returns the field of muxInt that corresponds to the input string "search".
func (mi *muxInt) getField(search string) (interface{}, error) {
	switch search {
	case "_select":
		return mi.getSelect(), nil
	case "_manual":
		return mi.manual, nil
	case "_override":
		return mi.override, nil
	case "_actual":
		return mi.actual, nil
	default:
		return nil, fmt.Errorf("%s is not a valid suffix", search)
	}
}

// Returns the field of scheduledService that corresponds to the input string "search".
func (ss *scheduledService) getField(search string) (interface{}, error) {
	if strings.HasPrefix(search, "_responsibility") {
		return ss.responsibility.getField(search[len("_responsibility"):])
	} else if strings.HasPrefix(search, "_requirement") {
		return ss.requirement.getField(search[len("_requirement"):])
	} else if search == "_scheduled" {
		return ss.scheduled, nil
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Returns the field of participatingService that corresponds to the input string "search".
func (ps *participatingService) getField(search string) (interface{}, error) {
	if strings.HasPrefix(search, "_responsibility") {
		return ps.responsibility.getField(search[len("_responsibility"):])
	} else if strings.HasPrefix(search, "_requirement") {
		return ps.requirement.getField(search[len("_requirement"):])
	} else if search == "_participation" {
		return ps.participation, nil
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Returns the gen or load int/bool value based on prefix and suffix of the input point name "search".
func (glmi *genLoadMuxInt) getGenLoad(search string) (interface{}, error) {
	if strings.HasPrefix(search, genSuffix) {
		return glmi.gen.getField(search[lenGenSuffix:])
	} else if strings.HasPrefix(search, loadSuffix) {
		return glmi.load.getField(search[lenLoadSuffix:])
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Returns the corresponding gen or load value based on the input "search".
func (glmf *genLoadMuxFloat) getGenLoad(search string) (interface{}, error) {
	if strings.HasPrefix(search, genSuffix) {
		return glmf.gen.getField(search[lenGenSuffix:])
	} else if strings.HasPrefix(search, loadSuffix) {
		return glmf.load.getField(search[lenLoadSuffix:])
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// returns the corresponding gen or load value based on the input "search"
func (glss *genLoadScheduledService) getGenLoad(search string) (interface{}, error) {
	if strings.HasPrefix(search, genSuffix) {
		return glss.gen.getField(search[lenGenSuffix:])
	} else if strings.HasPrefix(search, loadSuffix) {
		return glss.load.getField(search[lenLoadSuffix:])
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// returns the corresponding gen or load value based on the input "search"
func (glps *genLoadParticipatingService) getGenLoad(search string) (interface{}, error) {
	if strings.HasPrefix(search, genSuffix) {
		return glps.gen.getField(search[lenGenSuffix:])
	} else if strings.HasPrefix(search, loadSuffix) {
		return glps.load.getField(search[lenLoadSuffix:])
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// returns the corresponding gen or load value based on the input "search"
func (glsnm *genLoadServiceNoMux) getGenLoad(search string) (interface{}, error) {
	if strings.HasPrefix(search, genSuffix) && strings.Contains(search, "responsibility") {
		return glsnm.genResponsibility, nil
	} else if strings.HasPrefix(search, genSuffix) && strings.Contains(search, "requirement") {
		return glsnm.genRequirement, nil
	} else if strings.HasPrefix(search, genSuffix) && strings.Contains(search, "scheduled") {
		return glsnm.genScheduled, nil
	} else if strings.HasPrefix(search, loadSuffix) && strings.Contains(search, "responsibility") {
		return glsnm.loadResponsibility, nil
	} else if strings.HasPrefix(search, loadSuffix) && strings.Contains(search, "requirement") {
		return glsnm.loadRequirement, nil
	} else if strings.HasPrefix(search, loadSuffix) && strings.Contains(search, "scheduled") {
		return glsnm.loadScheduled, nil
	} else {
		return nil, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

//----------------GEN/LOAD PARTICIPATING SERVICE------------------------
type getGLPServiceValueTest struct {
	inputService  *genLoadParticipatingService
	inputString   string
	expectedValue interface{}
	expectedError error
}

type getGLPServiceValueTestCases []*getGLPServiceValueTest

func TestGetGLPServiceValueTestCases(t *testing.T) {
	getGLPServiceTests := getGLPServiceValueTestCases{
		//Test 0 gen responsibility overridden select
		{
			//input service
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			//input string
			"_gen_responsibility_select",
			//expected value
			3.0,
			//expected error
			nil,
		},
		//Test 1 load requirement actual select
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: false,
					},
					participation: 22.3,
				},
			},
			"_load_requirement_select",
			2.4,
			nil,
		},
		//Test 2 gen requirement override
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			"_gen_requirement_override",
			false,
			nil,
		},
		//Test 3 gen participation
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			"_gen_participation",
			22.5,
			nil,
		},
		//Test 4 load participation
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			"_load_participation",
			22.3,
			nil,
		},
		//Test 5 gen requirement manual
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			"_gen_requirement_manual",
			3.0,
			nil,
		},
		//Test 6 load responsibility actual
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			"_load_responsibility_actual",
			2.6,
			nil,
		},
		//Test 7 incorrect URI
		{
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					participation: 22.5,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					participation: 22.3,
				},
			},
			"_responsibility_actual",
			nil,
			fmt.Errorf("_responsibility_actual is not a valid endpoint"),
		},
	}
	for i, testCase := range getGLPServiceTests {
		output, err := testCase.inputService.getGenLoad(testCase.inputString)
		if err != nil && testCase.expectedError == nil {
			t.Errorf("getGLPServiceValue() test %d failed. Did not expect error but got error: %v.", i, err)
		} else if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("getGLPServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLPServiceValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(output, testCase.expectedValue) {
			t.Errorf("getGLPServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLPServiceValue()Test %d---", i)
			t.Errorf("Expected: %v Got: %v", testCase.expectedValue, output)
		}
	}
}

type setGLPServiceValueTest struct {
	inputValue    interface{}
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type setGLPServiceValueTestCases []*setGLPServiceValueTest

func TestSetGLPServiceValueTestCases(t *testing.T) {
	setGLPServiceTests := setGLPServiceValueTestCases{
		//Test 0 gen responsibility manual
		{
			//input value
			23.5,
			//input string
			"_gen_responsibility_manual",
			//output value
			nil,
			//expected value
			23.5,
			//expected error
			nil,
		},
		//Test 1 load requirement actual
		{
			3.5,
			"_gen_requirement_actual",
			nil,
			3.5,
			nil,
		},
		//Test 2 gen requirement override
		{
			true,
			"_gen_requirement_override",
			nil,
			true,
			nil,
		},
		//Test 3 gen participation
		{
			.1234,
			"_gen_participation",
			nil,
			.1234,
			nil,
		},
		//Test 4 load participation
		{
			.134,
			"_load_participation",
			nil,
			.134,
			nil,
		},
		//Test 5 gen requirement manual
		{
			111.0,
			"_gen_requirement_manual",
			nil,
			111.0,
			nil,
		},
		//Test 6 load responsibility actual
		{
			111.0,
			"_load_responsibility_actual",
			nil,
			111.0,
			nil,
		},
		//Test 7 incorrect URI
		{
			111.0,
			"_load_actual",
			nil,
			nil,
			fmt.Errorf("_actual is not a valid endpoint"),
		},
		//Test 8 incorrect value for gen requirement manual
		{
			"hello",
			"_gen_requirement_manual",
			nil,
			0.0,
			fmt.Errorf("expected float64 for _manual but got string"),
		},
	}
	var err error
	for i, testCase := range setGLPServiceTests {
		emptyService := &genLoadParticipatingService{
			gen: participatingService{
				responsibility: muxFloat{},
				requirement:    muxFloat{},
			},
			load: participatingService{
				responsibility: muxFloat{},
				requirement:    muxFloat{},
			},
		}
		_, err = emptyService.setGenLoad(testCase.inputString, testCase.inputValue)
		testCase.outputValue, _ = emptyService.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("setGLPServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLPServiceValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("setGLPServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLPServiceValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

//----------------GEN/LOAD SCHEDULED SERVICE----------------------------
type getGLSServiceValueTest struct {
	inputService  *genLoadScheduledService
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type getGLSServiceValueTestCases []*getGLSServiceValueTest

func TestGetGLSServiceValueTestCases(t *testing.T) {
	getGLSServiceTests := getGLSServiceValueTestCases{
		//Test 0 gen responsibility overridden select
		{
			//input service
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.3,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			//input string
			"_gen_responsibility_select",
			//output value
			nil,
			//expected value
			3.3,
			//expected error
			nil,
		},
		//Test 1 load requirement actual select
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: false,
					},
					scheduled: 22.3,
				},
			},
			"_load_requirement_select",
			nil,
			2.4,
			nil,
		},
		//Test 2 gen requirement override
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			"_gen_requirement_override",
			nil,
			false,
			nil,
		},
		//Test 3 gen scheduled
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			"_gen_scheduled",
			nil,
			22.5,
			nil,
		},
		//Test 4 load scheduled
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			"_load_scheduled",
			nil,
			22.3,
			nil,
		},
		//Test 5 gen requirement manual
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			"_gen_requirement_manual",
			nil,
			3.0,
			nil,
		},
		//Test 6 load responsibility actual
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			"_load_responsibility_actual",
			nil,
			2.6,
			nil,
		},
		//Test 7 incorrect URI
		{
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   2.5,
						manual:   3.0,
						override: false,
					},
					scheduled: 22.5,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   2.6,
						manual:   3.1,
						override: false,
					},
					requirement: muxFloat{
						actual:   2.4,
						manual:   3.2,
						override: true,
					},
					scheduled: 22.3,
				},
			},
			"_load_responsibility",
			nil,
			nil,
			fmt.Errorf(" is not a valid suffix"),
		},
	}
	var err error
	for i, testCase := range getGLSServiceTests {
		testCase.outputValue, err = testCase.inputService.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("getGLSServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLSServiceValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("getGLPServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLSServiceValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

type setGLSServiceValueTest struct {
	inputValue    interface{}
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type setGLSServiceValueTestCases []*setGLSServiceValueTest

func TestSetGLSServiceValueTestCases(t *testing.T) {
	setGLSServiceTests := setGLSServiceValueTestCases{
		//Test 0 gen responsibility manual
		{
			//input value
			23.5,
			//input string
			"_gen_responsibility_manual",
			//output value
			nil,
			//expected value
			23.5,
			//expected error
			nil,
		},
		//Test 1 load requirement actual
		{
			3.5,
			"_gen_requirement_actual",
			nil,
			3.5,
			nil,
		},
		//Test 2 gen requirement override
		{
			true,
			"_gen_requirement_override",
			nil,
			true,
			nil,
		},
		//Test 3 gen scheduled
		{
			.1234,
			"_gen_scheduled",
			nil,
			.1234,
			nil,
		},
		//Test 4 load scheduled
		{
			.134,
			"_load_scheduled",
			nil,
			.134,
			nil,
		},
		//Test 5 gen requirement manual
		{
			111.0,
			"_gen_requirement_manual",
			nil,
			111.0,
			nil,
		},
		//Test 6 load responsibility actual
		{
			111.0,
			"_load_responsibility_actual",
			nil,
			111.0,
			nil,
		},
		//Test 7 incorrect URI
		{
			111.0,
			"_load_responsibili_actual",
			nil,
			nil,
			fmt.Errorf("_responsibili_actual is not a valid endpoint"),
		},
		//Test 8 incorrect value for gen scheduled
		{
			false,
			"_gen_scheduled",
			nil,
			0.0,
			fmt.Errorf("expected a float64 for _scheduled field but got bool"),
		},
	}
	var err error
	for i, testCase := range setGLSServiceTests {
		emptyService := &genLoadScheduledService{
			gen: scheduledService{
				responsibility: muxFloat{},
				requirement:    muxFloat{},
			},
			load: scheduledService{
				responsibility: muxFloat{},
				requirement:    muxFloat{},
			},
		}
		_, err = emptyService.setGenLoad(testCase.inputString, testCase.inputValue)
		testCase.outputValue, _ = emptyService.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("setGLSServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLSServiceValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("setGLSServiceValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLSServiceValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

//----------------GEN/LOAD MUX INT------------------------------------
type getGLMuxIntValueTest struct {
	inputInt      *genLoadMuxInt
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type getGLMuxIntValueTestCases []*getGLMuxIntValueTest

func TestGetGLMuxIntValueTestCases(t *testing.T) {
	getGLSMuxIntTests := getGLMuxIntValueTestCases{
		//Test 0 gen overridden select
		{
			//input int
			&genLoadMuxInt{
				gen:  muxInt{actual: 23, manual: 24, override: true},
				load: muxInt{},
			},
			//input string
			"_gen_select",
			//output value
			nil,
			//expected value
			24,
			//expected error
			nil,
		},
		//Test 1 load actual
		{
			&genLoadMuxInt{
				gen:  muxInt{actual: 23, manual: 24, override: true},
				load: muxInt{actual: 17},
			},
			"_load_actual",
			nil,
			17,
			nil,
		},
		//Test 2 gen override
		{
			&genLoadMuxInt{
				gen:  muxInt{actual: 23, manual: 24, override: true},
				load: muxInt{},
			},
			"_gen_override",
			nil,
			true,
			nil,
		},
		//Test 3 load manual
		{
			&genLoadMuxInt{
				gen:  muxInt{actual: 23, manual: 24, override: true},
				load: muxInt{override: true, manual: 1},
			},
			"_load_manual",
			nil,
			1,
			nil,
		},
		//Test 4 gen normal select
		{
			&genLoadMuxInt{
				gen:  muxInt{actual: 23, manual: 24, override: false},
				load: muxInt{},
			},
			"_gen_select",
			nil,
			23,
			nil,
		},
		//Test 5 incorrect URI
		{
			&genLoadMuxInt{
				gen:  muxInt{actual: 23, manual: 24, override: true},
				load: muxInt{},
			},
			"_gen",
			nil,
			nil,
			fmt.Errorf(" is not a valid suffix"),
		},
	}
	var err error
	for i, testCase := range getGLSMuxIntTests {
		testCase.outputValue, err = testCase.inputInt.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("getGLMuxIntValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLMuxIntValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("getGLMuxIntValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLMuxIntValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

type setGLMuxIntValueTest struct {
	inputValue    interface{}
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type setGLMuxIntValueTestCases []*setGLMuxIntValueTest

func TestSetGLMuxIntValueTestCases(t *testing.T) {
	setGLSMuxIntTests := setGLMuxIntValueTestCases{
		//Test 0 gen override
		{
			//input val
			true,
			//input string
			"_gen_override",
			//output value
			nil,
			//expected value
			true,
			//expected error
			nil,
		},
		//Test 1 load actual
		{
			30,
			"_load_actual",
			nil,
			30,
			nil,
		},
		//Test 2 gen manual
		{
			11,
			"_gen_manual",
			nil,
			11,
			nil,
		},
		//Test 3 load manual
		{
			13,
			"_load_manual",
			nil,
			13,
			nil,
		},
		//Test 4 gen actual
		{
			21,
			"_gen_actual",
			nil,
			21,
			nil,
		},
		//Test 5 load override
		{
			true,
			"_load_override",
			nil,
			true,
			nil,
		},
		//Test 6 incorrect URI
		{
			13,
			"_manual",
			nil,
			nil,
			fmt.Errorf("_manual is not a valid endpoint"),
		},
		//Test 7 incorrect value for load actual
		{
			30.69,
			"_load_actual",
			nil,
			30,
			nil,
		},
	}
	var err error
	for i, testCase := range setGLSMuxIntTests {
		emptyMux := &genLoadMuxInt{gen: muxInt{}, load: muxInt{}}
		_, err = emptyMux.setGenLoad(testCase.inputString, testCase.inputValue)
		testCase.outputValue, _ = emptyMux.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("setGLMuxIntValue() test %d failed. Error output is incorrect", i)
			t.Errorf("---setGLMuxIntValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("setGLMuxIntValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLMuxIntValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

//----------------GEN/LOAD MUX FLOAT------------------------------------
type getGLMuxFloatValueTest struct {
	inputFloat    *genLoadMuxFloat
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type getGLMuxFloatValueTestCases []*getGLMuxFloatValueTest

func TestGetGLMuxFloatValueTestCases(t *testing.T) {
	getGLSMuxFloatTests := getGLMuxFloatValueTestCases{
		//Test 0 gen overridden select
		{
			//input float
			&genLoadMuxFloat{
				gen:  muxFloat{actual: 23.5, manual: 24.6, override: true},
				load: muxFloat{},
			},
			//input string
			"_gen_select",
			//output value
			nil,
			//expected value
			24.6,
			//expected error
			nil,
		},
		//Test 1 load actual
		{
			&genLoadMuxFloat{
				gen:  muxFloat{actual: 23.5, manual: 24.6, override: true},
				load: muxFloat{actual: 12.345, manual: 12.0},
			},
			"_load_actual",
			nil,
			12.345,
			nil,
		},
		//Test 2 gen override
		{
			&genLoadMuxFloat{
				gen:  muxFloat{actual: 23.5, manual: 24.6, override: true},
				load: muxFloat{actual: 12.345, manual: 12.0},
			},
			"_gen_override",
			nil,
			true,
			nil,
		},
		//Test 3 load manual
		{
			&genLoadMuxFloat{
				gen:  muxFloat{actual: 23.5, manual: 24.6, override: true},
				load: muxFloat{actual: 12.345, manual: 12.0},
			},
			"_load_manual",
			nil,
			12.0,
			nil,
		},
		//Test 4 gen normal select
		{
			&genLoadMuxFloat{
				gen:  muxFloat{actual: 23.5, manual: 24.6, override: false},
				load: muxFloat{actual: 12.345, manual: 12.0},
			},
			"_gen_select",
			nil,
			23.5,
			nil,
		},
		//Test 5 incorrect URI
		{
			&genLoadMuxFloat{
				gen:  muxFloat{actual: 23.5, manual: 24.6, override: false},
				load: muxFloat{actual: 12.345, manual: 12.0},
			},
			"_gen_",
			nil,
			nil,
			fmt.Errorf("_ is not a valid suffix"),
		},
	}
	var err error
	for i, testCase := range getGLSMuxFloatTests {
		testCase.outputValue, err = testCase.inputFloat.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("getGLMuxFloatValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLMuxFloatValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("getGLMuxFloatValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLMuxFloatValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

type setGLMuxFloatValueTest struct {
	inputValue    interface{}
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type setGLMuxFloatValueTestCases []*setGLMuxFloatValueTest

func TestSetGLMuxFloatValueTestCases(t *testing.T) {
	setGLSMuxFloatTests := setGLMuxFloatValueTestCases{
		//Test 0 gen override
		{
			//input val
			true,
			//input string
			"_gen_override",
			//output value
			nil,
			//expected value
			true,
			//expected error
			nil,
		},
		//Test 1 load actual
		{
			30.1,
			"_load_actual",
			nil,
			30.1,
			nil,
		},
		//Test 2 gen manual
		{
			11.4,
			"_gen_manual",
			nil,
			11.4,
			nil,
		},
		//Test 3 load manual
		{
			13.78,
			"_load_manual",
			nil,
			13.78,
			nil,
		},
		//Test 4 gen actual
		{
			21.0,
			"_gen_actual",
			nil,
			21.0,
			nil,
		},
		//Test 5 load override
		{
			true,
			"_load_override",
			nil,
			true,
			nil,
		},
		//Test 6 incorrect URI
		{
			21.0,
			"_gen_actaul",
			nil,
			nil,
			fmt.Errorf("_actaul is not a valid suffix"),
		},
		{
			11,
			"_load_override",
			nil,
			false,
			fmt.Errorf("expected bool for _override but got int"),
		},
	}
	var err error
	for i, testCase := range setGLSMuxFloatTests {
		emptyMux := &genLoadMuxFloat{gen: muxFloat{}, load: muxFloat{}}
		_, err = emptyMux.setGenLoad(testCase.inputString, testCase.inputValue)
		testCase.outputValue, _ = emptyMux.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("setGLMuxFloatValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLMuxFloatValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("setGLMuxFloatValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLMuxFloatValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

//----------------GEN/LOAD SCHEDULED SERVICE (NO MUX)----------------------------
type getGLSNoMuxValueTest struct {
	inputService  *genLoadServiceNoMux
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type getGLSNoMuxValueTestCases []*getGLSNoMuxValueTest

func TestGetGLSNoMuxValueTest(t *testing.T) {
	getGLSNMTests := getGLSNoMuxValueTestCases{
		//Test 0 gen responsibility
		{
			//input service
			&genLoadServiceNoMux{
				genResponsibility:  3.0,
				genRequirement:     2.5,
				genScheduled:       22.5,
				loadResponsibility: 2.6,
				loadRequirement:    3.2,
				loadScheduled:      22.3,
			},
			//input string
			"_gen_responsibility",
			//output value
			nil,
			//expected value
			3.0,
			//expected error
			nil,
		},
		//Test 1 load requirement
		{
			&genLoadServiceNoMux{
				genResponsibility:  3.0,
				genRequirement:     2.5,
				genScheduled:       22.5,
				loadResponsibility: 2.6,
				loadRequirement:    3.2,
				loadScheduled:      22.3,
			},
			"_load_requirement",
			nil,
			3.2,
			nil,
		},
		//Test 2 gen scheduled
		{
			&genLoadServiceNoMux{
				genResponsibility:  3.0,
				genRequirement:     2.5,
				genScheduled:       22.5,
				loadResponsibility: 2.6,
				loadRequirement:    3.2,
				loadScheduled:      22.3,
			},
			"_gen_scheduled",
			nil,
			22.5,
			nil,
		},
		//Test 3 load scheduled
		{
			&genLoadServiceNoMux{
				genResponsibility:  3.0,
				genRequirement:     2.5,
				genScheduled:       22.5,
				loadResponsibility: 2.6,
				loadRequirement:    3.2,
				loadScheduled:      22.3,
			},
			"_load_scheduled",
			nil,
			22.3,
			nil,
		},
		//Test 4 load responsibility
		{
			&genLoadServiceNoMux{
				genResponsibility:  3.0,
				genRequirement:     2.5,
				genScheduled:       22.5,
				loadResponsibility: 2.6,
				loadRequirement:    3.2,
				loadScheduled:      22.3,
			},
			"_load_responsibility",
			nil,
			2.6,
			nil,
		},
		//Test 5 incorrect URI
		{
			&genLoadServiceNoMux{
				genResponsibility:  3.0,
				genRequirement:     2.5,
				genScheduled:       22.5,
				loadResponsibility: 2.6,
				loadRequirement:    3.2,
				loadScheduled:      22.3,
			},
			"_load_responSbility",
			nil,
			nil,
			fmt.Errorf("_load_responSbility is not a valid endpoint"),
		},
	}
	var err error
	for i, testCase := range getGLSNMTests {
		testCase.outputValue, err = testCase.inputService.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("getGLSNoMuxValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---getGLSNoMuxValue()Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("getGLSNoMuxValue() test %d failed. Error value is incorrect", i)
			t.Errorf("---getGLSNoMuxValue()Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}

type setGLSNoMuxValueTest struct {
	inputValue    interface{}
	inputString   string
	outputValue   interface{}
	expectedValue interface{}
	expectedError error
}

type setGLSNoMuxValueTestCases []*setGLSNoMuxValueTest

func TestSetGLSNoMuxValueTestCases(t *testing.T) {
	setGLSServiceTests := setGLSNoMuxValueTestCases{
		//Test 0 gen responsibility
		{
			//input value
			23.5,
			//input string
			"_gen_responsibility",
			//output value
			nil,
			//expected value
			23.5,
			//expected error
			nil,
		},
		//Test 1 load requirement
		{
			3.5,
			"_gen_requirement",
			nil,
			3.5,
			nil,
		},
		//Test 2 gen scheduled
		{
			.1234,
			"_gen_scheduled",
			nil,
			.1234,
			nil,
		},
		//Test 3 load scheduled
		{
			.134,
			"_load_scheduled",
			nil,
			.134,
			nil,
		},
		//Test 4 gen requirement
		{
			111.0,
			"_gen_requirement",
			nil,
			111.0,
			nil,
		},
		//Test 5 load responsibility actual
		{
			111.0,
			"_load_responsibility",
			nil,
			111.0,
			nil,
		},
		//Test 6 incorrect URI
		{
			111.0,
			"_load_responsibili",
			nil,
			nil,
			fmt.Errorf("_load_responsibili is not a valid endpoint"),
		},
		//Test 7 incorrect value for gen scheduled
		{
			false,
			"_gen_scheduled",
			nil,
			0.0,
			fmt.Errorf("value for _gen_scheduled was expected to be a float. Received value of type bool"),
		},
	}
	var err error
	for i, testCase := range setGLSServiceTests {
		emptyService := &genLoadServiceNoMux{}
		_, err = emptyService.setGenLoad(testCase.inputString, testCase.inputValue)
		testCase.outputValue, _ = emptyService.getGenLoad(testCase.inputString)
		if (err != nil && testCase.expectedError.Error() != err.Error()) || (err == nil && testCase.expectedError != nil) {
			t.Errorf("setGLSNoMuxValue() test %d failed. Output value is incorrect", i)
			t.Errorf("---setGLSNoMuxValue() Test %d---", i)
			t.Errorf("Expected error: %v Got error: %v", testCase.expectedError, err)
		} else if !reflect.DeepEqual(testCase.outputValue, testCase.expectedValue) {
			t.Errorf("setGLSNoMuxValue() test %d failed. Error value is incorrect", i)
			t.Errorf("---setGLSNoMuxValue() Test %d---", i)
			calculated := testCase.outputValue
			expected := testCase.expectedValue
			t.Errorf("Expected: %v Got: %v", expected, calculated)
		}
	}
}
