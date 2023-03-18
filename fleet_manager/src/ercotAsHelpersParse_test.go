package main

import (
	"reflect"
	"testing"
)

//----------------GEN/LOAD PARTICIPATING SERVICE---------------------------------
type parseGLPServiceTest struct {
	inputConfig     map[string]interface{}
	expectedService *genLoadParticipatingService
	expectedErr     error
}

type parseGLPServiceTestCases []*parseGLPServiceTest

func TestGLPServiceTestCases(t *testing.T) {
	glpServiceTestCases := parseGLPServiceTestCases{
		//Test 0 fully configured, zeroed-out input
		{
			//input cfg
			map[string]interface{}{
				"_gen_responsibility_actual":    0.0,
				"_gen_responsibility_manual":    0.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       0.0,
				"_gen_requirement_manual":       0.0,
				"_gen_requirement_override":     false,
				"_gen_participation":            0.0,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   0.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      0.0,
				"_load_requirement_manual":      0.0,
				"_load_requirement_override":    false,
				"_load_participation":           0.0,
			},
			//expected service
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
			},
			//expected error
			nil,
		},
		//Test 1 fully configured w/ nums input
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    5.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       0.6,
				"_gen_requirement_manual":       0.7,
				"_gen_requirement_override":     true,
				"_gen_participation":            11.0,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   12.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      0.0,
				"_load_requirement_manual":      0.0,
				"_load_requirement_override":    false,
				"_load_participation":           0.835,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   5.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.6,
						manual:   0.7,
						override: true,
					},
					participation: 11.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   12.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.835,
				},
			},
			nil,
		},
		//Test 2 gen responsibility not configured
		{
			map[string]interface{}{
				"_gen_requirement_actual":       1.0,
				"_gen_requirement_manual":       2.0,
				"_gen_requirement_override":     false,
				"_gen_participation":            3.0,
				"_load_responsibility_actual":   4.0,
				"_load_responsibility_manual":   5.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      6.0,
				"_load_requirement_manual":      7.0,
				"_load_requirement_override":    false,
				"_load_participation":           8.0,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					participation: 3.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   4.0,
						manual:   5.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   6.0,
						manual:   7.0,
						override: false,
					},
					participation: 8.0,
				},
			},
			nil,
		},
		//Test 3 load requirement not configured
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    1.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       3.0,
				"_gen_requirement_manual":       4.0,
				"_gen_requirement_override":     false,
				"_gen_participation":            5.0,
				"_load_responsibility_actual":   6.0,
				"_load_responsibility_manual":   7.0,
				"_load_responsibility_override": false,
				"_load_participation":           8.0,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					participation: 5.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   6.0,
						manual:   7.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 8.0,
				},
			},
			nil,
		},
		//Test 4 gen not configured
		{
			map[string]interface{}{
				"_load_responsibility_actual":   1.0,
				"_load_responsibility_manual":   2.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      3.0,
				"_load_requirement_manual":      4.0,
				"_load_requirement_override":    false,
				"_load_participation":           5.0,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					participation: 5.0,
				},
			},
			nil,
		},
		//Test 5 load not configured
		{
			map[string]interface{}{
				"_gen_responsibility_actual":   1.0,
				"_gen_responsibility_manual":   2.0,
				"_gen_responsibility_override": false,
				"_gen_requirement_actual":      3.0,
				"_gen_requirement_manual":      4.0,
				"_gen_requirement_override":    false,
				"_gen_participation":           5.0,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					participation: 5.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
			},
			nil,
		},
		//Test 6 no manual in gen_requirement
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    1.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       3.0,
				"_gen_requirement_override":     false,
				"_gen_participation":            5.0,
				"_load_responsibility_actual":   6.0,
				"_load_responsibility_manual":   7.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      8.0,
				"_load_requirement_manual":      9.0,
				"_load_requirement_override":    false,
				"_load_participation":           10.0,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   0.0,
						override: false,
					},
					participation: 5.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   6.0,
						manual:   7.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   8.0,
						manual:   9.0,
						override: false,
					},
					participation: 10.0,
				},
			},
			nil,
		},
		//Test 7 no actual in load_responsibility
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    1.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       3.0,
				"_gen_requirement_manual":       4.0,
				"_gen_requirement_override":     false,
				"_gen_participation":            5.0,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   6.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      7.0,
				"_load_requirement_manual":      8.0,
				"_load_requirement_override":    false,
				"_load_participation":           9.0,
			},
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					participation: 5.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   6.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   7.0,
						manual:   8.0,
						override: false,
					},
					participation: 9.0,
				},
			},
			nil,
		},
		//Test 8 nil config
		{
			nil,
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
			},
			//expected error
			nil,
		},
		//Test 9 incorrect gen_requirement_actual type
		{
			//input cfg
			map[string]interface{}{
				"_gen_responsibility_actual":    0.0,
				"_gen_responsibility_manual":    0.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       "",
				"_gen_requirement_manual":       0.0,
				"_gen_requirement_override":     false,
				"_gen_participation":            0.0,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   0.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      0.0,
				"_load_requirement_manual":      0.0,
				"_load_requirement_override":    false,
				"_load_participation":           0.0,
			},
			//expected service
			&genLoadParticipatingService{
				gen: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
				load: participatingService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					participation: 0.0,
				},
			},
			//expected error
			nil,
		},
	}
	for i, testCase := range glpServiceTestCases {
		output := parseGLPService(testCase.inputConfig, "")
		if !reflect.DeepEqual(&output, testCase.expectedService) {
			t.Errorf("parseGLPService() test %d failed. Output service is incorrect", i)
			t.Errorf("---parseGLPService() Test %d---", i)
			expected := testCase.expectedService
			if output.gen.responsibility.actual != expected.gen.responsibility.actual {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility_actual", expected.gen.responsibility.actual, output.gen.responsibility.actual)
			}
			if output.load.responsibility.actual != expected.load.responsibility.actual {
				t.Errorf("Expected: %v  Got: %v for load_responsibility_actual", expected.load.responsibility.actual, output.load.responsibility.actual)
			}
			if output.gen.responsibility.manual != expected.gen.responsibility.manual {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility_manual", expected.gen.responsibility.manual, output.gen.responsibility.manual)
			}
			if output.load.responsibility.manual != expected.load.responsibility.manual {
				t.Errorf("Expected: %v  Got: %v for load_responsibility_manual", expected.load.responsibility.manual, output.load.responsibility.manual)
			}
			if output.gen.responsibility.override != expected.gen.responsibility.override {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility_override", expected.gen.responsibility.override, output.gen.responsibility.override)
			}
			if output.load.responsibility.override != expected.load.responsibility.override {
				t.Errorf("Expected: %v  Got: %v for load_responsibility_override", expected.load.responsibility.override, output.load.responsibility.override)
			}
			if output.gen.requirement.actual != expected.gen.requirement.actual {
				t.Errorf("Expected: %v  Got: %v for gen_requirement_actual", expected.gen.requirement.actual, output.gen.requirement.actual)
			}
			if output.load.requirement.actual != expected.load.requirement.actual {
				t.Errorf("Expected: %v  Got: %v for load_requirement_actual", expected.load.requirement.actual, output.load.requirement.actual)
			}
			if output.gen.requirement.manual != expected.gen.requirement.manual {
				t.Errorf("Expected: %v  Got: %v for gen_requirement_manual", expected.gen.requirement.manual, output.gen.requirement.manual)
			}
			if output.load.requirement.manual != expected.load.requirement.manual {
				t.Errorf("Expected: %v  Got: %v for load_requirement_manual", expected.load.requirement.manual, output.load.requirement.manual)
			}
			if output.gen.requirement.override != expected.gen.requirement.override {
				t.Errorf("Expected: %v  Got: %v for gen_requirement_override", expected.gen.requirement.override, output.gen.requirement.override)
			}
			if output.load.requirement.override != expected.load.requirement.override {
				t.Errorf("Expected: %v  Got: %v for load_requirement_override", expected.load.requirement.override, output.load.requirement.override)
			}
			if output.gen.participation != expected.gen.participation {
				t.Errorf("Expected: %v  Got: %v for gen_participation", expected.gen.participation, output.gen.participation)
			}
			if output.load.participation != expected.load.participation {
				t.Errorf("Expected: %v  Got: %v for load_participation", expected.load.participation, output.load.participation)
			}
			t.Errorf("Structural Error")
		}
	}
}

//----------------GEN/LOAD SCHEDULED SERVICE--------------------------------------
type parseGLSServiceTest struct {
	inputConfig     map[string]interface{}
	outputService   *genLoadScheduledService
	expectedService *genLoadScheduledService
	expectedErr     error
}

type parseGLSServiceTestCases []*parseGLSServiceTest

func TestGLSServiceTestCases(t *testing.T) {
	glsServiceTestCases := parseGLSServiceTestCases{
		//Test 0 fully configured, zeroed-out input
		{
			//input cfg
			map[string]interface{}{
				"_gen_responsibility_actual":    0.0,
				"_gen_responsibility_manual":    0.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       0.0,
				"_gen_requirement_manual":       0.0,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                0.0,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   0.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      0.0,
				"_load_requirement_manual":      0.0,
				"_load_requirement_override":    false,
				"_load_scheduled":               0.0,
			},
			//output service
			nil,
			//expected service
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
			},
			//expected error
			nil,
		},
		//Test 1 fully configured w/ nums input
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    22.0,
				"_gen_responsibility_manual":    0.0,
				"_gen_responsibility_override":  true,
				"_gen_requirement_actual":       0.0,
				"_gen_requirement_manual":       0.345,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                2.78,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   0.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      0.9,
				"_load_requirement_manual":      0.0,
				"_load_requirement_override":    true,
				"_load_scheduled":               7.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   22.0,
						manual:   0.0,
						override: true,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.345,
						override: false,
					},
					scheduled: 2.78,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.9,
						manual:   0.0,
						override: true,
					},
					scheduled: 7.0,
				},
			},
			nil,
		},
		//Test 2 gen responsibility not configured
		{
			map[string]interface{}{
				"_gen_requirement_actual":       1.0,
				"_gen_requirement_manual":       2.0,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                3.0,
				"_load_responsibility_actual":   4.0,
				"_load_responsibility_manual":   5.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      6.0,
				"_load_requirement_manual":      7.0,
				"_load_requirement_override":    false,
				"_load_scheduled":               8.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					scheduled: 3.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   4.0,
						manual:   5.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   6.0,
						manual:   7.0,
						override: false,
					},
					scheduled: 8.0,
				},
			},
			nil,
		},
		//Test 3 load requirement not configured
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    1.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       3.0,
				"_gen_requirement_manual":       4.0,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                5.0,
				"_load_responsibility_actual":   6.0,
				"_load_responsibility_manual":   7.0,
				"_load_responsibility_override": false,
				"_load_scheduled":               8.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					scheduled: 5.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   6.0,
						manual:   7.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 8.0,
				},
			},
			nil,
		},
		//Test 4 gen not configured
		{
			map[string]interface{}{
				"_load_responsibility_actual":   1.0,
				"_load_responsibility_manual":   2.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      3.0,
				"_load_requirement_manual":      4.0,
				"_load_requirement_override":    false,
				"_load_scheduled":               5.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					scheduled: 5.0,
				},
			},
			nil,
		},
		//Test 5 load not configured
		{
			map[string]interface{}{
				"_gen_responsibility_actual":   1.0,
				"_gen_responsibility_manual":   2.0,
				"_gen_responsibility_override": false,
				"_gen_requirement_actual":      3.0,
				"_gen_requirement_manual":      4.0,
				"_gen_requirement_override":    false,
				"_gen_scheduled":               5.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					scheduled: 5.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
			},
			nil,
		},
		//Test 6 no manual in gen_requirement
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    1.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       3.0,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                4.0,
				"_load_responsibility_actual":   5.0,
				"_load_responsibility_manual":   6.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      7.0,
				"_load_requirement_manual":      8.0,
				"_load_requirement_override":    false,
				"_load_scheduled":               9.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 4.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   5.0,
						manual:   6.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   7.0,
						manual:   8.0,
						override: false,
					},
					scheduled: 9.0,
				},
			},
			nil,
		},
		//Test 7 no actual in load_responsibility
		{
			map[string]interface{}{
				"_gen_responsibility_actual":    1.0,
				"_gen_responsibility_manual":    2.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       3.0,
				"_gen_requirement_manual":       4.0,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                4.0,
				"_load_responsibility_actual":   5.0,
				"_load_responsibility_manual":   6.0,
				"_load_responsibility_override": false,
				"_load_requirement_manual":      8.0,
				"_load_requirement_override":    false,
				"_load_scheduled":               9.0,
			},
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   1.0,
						manual:   2.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   3.0,
						manual:   4.0,
						override: false,
					},
					scheduled: 4.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   5.0,
						manual:   6.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   8.0,
						override: false,
					},
					scheduled: 9.0,
				},
			},
			nil,
		},
		//Test 8 nil config
		{
			nil,
			nil,
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
			},
			//expected error
			nil,
		},
		//Test 9 incorrect load_scheduled type
		{
			//input cfg
			map[string]interface{}{
				"_gen_responsibility_actual":    0.0,
				"_gen_responsibility_manual":    0.0,
				"_gen_responsibility_override":  false,
				"_gen_requirement_actual":       0.0,
				"_gen_requirement_manual":       0.0,
				"_gen_requirement_override":     false,
				"_gen_scheduled":                0.0,
				"_load_responsibility_actual":   0.0,
				"_load_responsibility_manual":   0.0,
				"_load_responsibility_override": false,
				"_load_requirement_actual":      0.0,
				"_load_requirement_manual":      0.0,
				"_load_requirement_override":    false,
				"_load_scheduled":               true,
			},
			//output service
			nil,
			//expected service
			&genLoadScheduledService{
				gen: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
				load: scheduledService{
					responsibility: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					requirement: muxFloat{
						actual:   0.0,
						manual:   0.0,
						override: false,
					},
					scheduled: 0.0,
				},
			},
			//expected error
			nil,
		},
	}

	for i, testCase := range glsServiceTestCases {
		output := parseGLSService(testCase.inputConfig, "")
		testCase.outputService = &output
		if !reflect.DeepEqual(testCase.outputService, testCase.expectedService) {
			t.Errorf("parseGLSService() test %d failed. Output service is incorrect", i)
			t.Errorf("---parseGLSService() Test %d---", i)
			calculated := testCase.outputService
			expected := testCase.expectedService
			if calculated.gen.responsibility.actual != expected.gen.responsibility.actual {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility", expected.gen.responsibility.actual, calculated.gen.responsibility.actual)
			}
			if calculated.load.responsibility.actual != expected.load.responsibility.actual {
				t.Errorf("Expected: %v  Got: %v for load_responsibility", expected.load.responsibility.actual, calculated.load.responsibility.actual)
			}
			if calculated.gen.responsibility.manual != expected.gen.responsibility.manual {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility_manual", expected.gen.responsibility.manual, calculated.gen.responsibility.manual)
			}
			if calculated.load.responsibility.manual != expected.load.responsibility.manual {
				t.Errorf("Expected: %v  Got: %v for load_responsibility_manual", expected.load.responsibility.manual, calculated.load.responsibility.manual)
			}
			if calculated.gen.responsibility.override != expected.gen.responsibility.override {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility_override", expected.gen.responsibility.override, calculated.gen.responsibility.override)
			}
			if calculated.load.responsibility.override != expected.load.responsibility.override {
				t.Errorf("Expected: %v  Got: %v for load_responsibility_override", expected.load.responsibility.override, calculated.load.responsibility.override)
			}
			if calculated.gen.requirement.actual != expected.gen.requirement.actual {
				t.Errorf("Expected: %v  Got: %v for gen_requirement", expected.gen.requirement.actual, calculated.gen.requirement.actual)
			}
			if calculated.load.requirement.actual != expected.load.requirement.actual {
				t.Errorf("Expected: %v  Got: %v for load_requirement", expected.load.requirement.actual, calculated.load.requirement.actual)
			}
			if calculated.gen.requirement.manual != expected.gen.requirement.manual {
				t.Errorf("Expected: %v  Got: %v for gen_requirement_manual", expected.gen.requirement.manual, calculated.gen.requirement.manual)
			}
			if calculated.load.requirement.manual != expected.load.requirement.manual {
				t.Errorf("Expected: %v  Got: %v for load_requirement_manual", expected.load.requirement.manual, calculated.load.requirement.manual)
			}
			if calculated.gen.requirement.override != expected.gen.requirement.override {
				t.Errorf("Expected: %v  Got: %v for gen_requirement_override", expected.gen.requirement.override, calculated.gen.requirement.override)
			}
			if calculated.load.requirement.override != expected.load.requirement.override {
				t.Errorf("Expected: %v  Got: %v for load_requirement_override", expected.load.requirement.override, calculated.load.requirement.override)
			}
			if calculated.gen.scheduled != expected.gen.scheduled {
				t.Errorf("Expected: %v  Got: %v for gen_scheduled", expected.gen.scheduled, calculated.gen.scheduled)
			}
			if calculated.load.scheduled != expected.load.scheduled {
				t.Errorf("Expected: %v  Got: %v for load_scheduled", expected.load.scheduled, calculated.load.scheduled)
			}
			t.Errorf("Structural Error")
		}
	}
}

//----------------GEN/LOAD FLOAT MUX----------------------------------------------
type parseGLMuxFloatTest struct {
	inputConfig map[string]interface{}
	outputMf    *genLoadMuxFloat
	expectedMf  *genLoadMuxFloat
	expectedErr error
}

type parseGLMuxFloatTestCases []*parseGLMuxFloatTest

func TestGLMuxFloatTestCases(t *testing.T) {
	glMuxFloatTestCases := parseGLMuxFloatTestCases{
		//Test 0 fully configured, zeroed-out input
		{
			//input cfg
			map[string]interface{}{
				"_gen_actual":    0.0,
				"_gen_manual":    0.0,
				"_gen_override":  false,
				"_load_actual":   0.0,
				"_load_manual":   0.0,
				"_load_override": false,
			},
			//output muxFloat
			nil,
			//expected muxFloat
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   0.0,
					manual:   0.0,
					override: false,
				},
				load: muxFloat{
					actual:   0.0,
					manual:   0.0,
					override: false,
				},
			},
			//expected error
			nil,
		},
		//Test 1 fully configured input w/ nums
		{
			map[string]interface{}{
				"_gen_actual":    3.0,
				"_gen_manual":    0.2,
				"_gen_override":  false,
				"_load_actual":   0.0,
				"_load_manual":   11.0,
				"_load_override": true,
			},
			nil,
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   3.0,
					manual:   0.2,
					override: false,
				},
				load: muxFloat{
					actual:   0.0,
					manual:   11.0,
					override: true,
				},
			},
			nil,
		},
		//Test 2 no actual value in gen
		{
			map[string]interface{}{
				"_gen_manual":    1.0,
				"_gen_override":  false,
				"_load_actual":   2.0,
				"_load_manual":   3.0,
				"_load_override": false,
			},
			nil,
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   0.0,
					manual:   1.0,
					override: false,
				},
				load: muxFloat{
					actual:   2.0,
					manual:   3.0,
					override: false,
				},
			},
			nil,
		},
		//Test 3 no override value in load
		{
			map[string]interface{}{
				"_gen_actual":   -1.0,
				"_gen_manual":   1.0,
				"_gen_override": true,
				"_load_actual":  2.0,
				"_load_manual":  3.0,
			},
			nil,
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   -1.0,
					manual:   1.0,
					override: true,
				},
				load: muxFloat{
					actual:   2.0,
					manual:   3.0,
					override: false,
				},
			},
			nil,
		},
		//Test 4 load not configured
		{
			map[string]interface{}{
				"_gen_actual":   -1.0,
				"_gen_manual":   1.0,
				"_gen_override": true,
			},
			nil,
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   -1.0,
					manual:   1.0,
					override: true,
				},
				load: muxFloat{
					actual:   0.0,
					manual:   0.0,
					override: false,
				},
			},
			nil,
		},
		//Test 5 gen not configured
		{
			map[string]interface{}{
				"_load_actual":   3.0,
				"_load_manual":   0.2,
				"_load_override": true,
			},
			nil,
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   0.0,
					manual:   0.0,
					override: false,
				},
				load: muxFloat{
					actual:   3.0,
					manual:   0.2,
					override: true,
				},
			},
			nil,
		},
		//Test 6 nil config
		{
			nil,
			nil,
			&genLoadMuxFloat{
				gen: muxFloat{
					actual:   0.0,
					manual:   0.0,
					override: false,
				},
				load: muxFloat{
					actual:   0.0,
					manual:   0.0,
					override: false,
				},
			},
			nil,
		},
	}

	for i, testCase := range glMuxFloatTestCases {
		output := parseGLMuxFloat(testCase.inputConfig, "")
		testCase.outputMf = &output
		if !reflect.DeepEqual(testCase.outputMf, testCase.expectedMf) {
			t.Errorf("parseGLMF() test %d failed. Output muxFloat is incorrect", i)
			t.Errorf("---parseGLMF() Test %d---", i)
			calculated := testCase.outputMf
			expected := testCase.expectedMf
			if calculated.gen.actual != expected.gen.actual {
				t.Errorf("Expected: %v  Got: %v for gen_actual", expected.gen.actual, calculated.gen.actual)
			}
			if calculated.load.actual != expected.load.actual {
				t.Errorf("Expected: %v  Got: %v for load_actual", expected.load.actual, calculated.load.actual)
			}
			if calculated.gen.manual != expected.gen.manual {
				t.Errorf("Expected: %v  Got: %v for gen_manual", expected.gen.manual, calculated.gen.manual)
			}
			if calculated.load.manual != expected.load.manual {
				t.Errorf("Expected: %v  Got: %v for load_manual", expected.load.manual, calculated.load.manual)
			}
			if calculated.gen.override != expected.gen.override {
				t.Errorf("Expected: %v  Got: %v for gen_override", expected.gen.override, calculated.gen.override)
			}
			if calculated.load.override != expected.load.override {
				t.Errorf("Expected: %v  Got: %v for load__override", expected.load.override, calculated.load.override)
			}
			t.Errorf("Structural Error")
		}
	}
}

//----------------GEN/LOAD INT MUX------------------------------------------------
type parseGLMuxIntTest struct {
	inputConfig map[string]interface{}
	outputMi    *genLoadMuxInt
	expectedMi  *genLoadMuxInt
	expectedErr error
}

type parseGLMuxIntTestCases []*parseGLMuxIntTest

func TestGLMuxIntTestCases(t *testing.T) {
	glMuxIntTestCases := parseGLMuxIntTestCases{
		//Test 0 fully configured, zeroed-out input
		{
			//input cfg
			map[string]interface{}{
				"_gen_actual":    0,
				"_gen_manual":    0,
				"_gen_override":  false,
				"_load_actual":   0,
				"_load_manual":   0,
				"_load_override": false,
			},
			//output muxInt
			nil,
			//expected muxInt
			&genLoadMuxInt{
				gen: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
				load: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
			},
			//expected error
			nil,
		},
		//Test 1 fully configured input w/ nums
		{
			map[string]interface{}{
				"_gen_actual":    3,
				"_gen_manual":    0,
				"_gen_override":  false,
				"_load_actual":   0,
				"_load_manual":   11,
				"_load_override": true,
			},
			nil,
			&genLoadMuxInt{
				gen: muxInt{
					actual:   3,
					manual:   0,
					override: false,
				},
				load: muxInt{
					actual:   0,
					manual:   11,
					override: true,
				},
			},
			nil,
		},
		//Test 2 no actual value in gen
		{
			map[string]interface{}{
				"_gen_manual":    1,
				"_gen_override":  false,
				"_load_actual":   2,
				"_load_manual":   3,
				"_load_override": false,
			},
			nil,
			&genLoadMuxInt{
				gen: muxInt{
					actual:   0,
					manual:   1,
					override: false,
				},
				load: muxInt{
					actual:   2,
					manual:   3,
					override: false,
				},
			},
			nil,
		},
		//Test 3 no override value in load
		{
			map[string]interface{}{
				"_gen_manual":   1,
				"_gen_override": true,
				"_load_actual":  2,
				"_load_manual":  3,
			},
			nil,
			&genLoadMuxInt{
				gen: muxInt{
					actual:   0,
					manual:   1,
					override: true,
				},
				load: muxInt{
					actual:   2,
					manual:   3,
					override: false,
				},
			},
			nil,
		},
		//Test 4 load not configured
		{
			map[string]interface{}{
				"_gen_actual":   3,
				"_gen_manual":   0,
				"_gen_override": false,
			},
			nil,
			&genLoadMuxInt{
				gen: muxInt{
					actual:   3,
					manual:   0,
					override: false,
				},
				load: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
			},
			nil,
		},
		//Test 5 gen not configured
		{
			map[string]interface{}{
				"_load_actual":   3,
				"_load_manual":   0,
				"_load_override": true,
			},
			nil,
			&genLoadMuxInt{
				gen: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
				load: muxInt{
					actual:   3,
					manual:   0,
					override: true,
				},
			},
			nil,
		},
		//Test 6 nil config
		{
			nil,
			nil,
			&genLoadMuxInt{
				gen: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
				load: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
			},
			nil,
		},
		//Test 7 incorrect actual type
		{
			//input cfg
			map[string]interface{}{
				"_gen_actual":    true,
				"_gen_manual":    0,
				"_gen_override":  false,
				"_load_actual":   "",
				"_load_manual":   0,
				"_load_override": false,
			},
			//output muxInt
			nil,
			//expected muxInt
			&genLoadMuxInt{
				gen: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
				load: muxInt{
					actual:   0,
					manual:   0,
					override: false,
				},
			},
			//expected error
			nil,
		},
	}

	for i, testCase := range glMuxIntTestCases {
		output := parseGLMuxInt(testCase.inputConfig, "")
		testCase.outputMi = &output
		if !reflect.DeepEqual(testCase.outputMi, testCase.expectedMi) {
			t.Errorf("parseGLMI() test %d failed. Output muxInt is incorrect", i)
			t.Errorf("---parseGLMI() Test %d---", i)
			calculated := testCase.outputMi
			expected := testCase.expectedMi
			if calculated.gen.actual != expected.gen.actual {
				t.Errorf("Expected: %v  Got: %v for gen_actual", expected.gen.actual, calculated.gen.actual)
			}
			if calculated.load.actual != expected.load.actual {
				t.Errorf("Expected: %v  Got: %v for load_actual", expected.load.actual, calculated.load.actual)
			}
			if calculated.gen.manual != expected.gen.manual {
				t.Errorf("Expected: %v  Got: %v for gen_manual", expected.gen.manual, calculated.gen.manual)
			}
			if calculated.load.manual != expected.load.manual {
				t.Errorf("Expected: %v  Got: %v for load_manual", expected.load.manual, calculated.load.manual)
			}
			if calculated.gen.override != expected.gen.override {
				t.Errorf("Expected: %v  Got: %v for gen_override", expected.gen.override, calculated.gen.override)
			}
			if calculated.load.override != expected.load.override {
				t.Errorf("Expected: %v  Got: %v for load_override", expected.load.override, calculated.load.override)
			}
			t.Errorf("Structural Error")
		}
	}
}

//----------------GEN/LOAD SCHEDULED SERVICE (NO MUX)----------------------------
type parseGLSNoMuxTest struct {
	inputConfig     map[string]interface{}
	outputService   *genLoadServiceNoMux
	expectedService *genLoadServiceNoMux
	expectedErr     error
}

type parseGLSNoMuxTestCases []*parseGLSNoMuxTest

func TestGLSNoMuxeTestCases(t *testing.T) {
	glsServiceTestCases := parseGLSNoMuxTestCases{
		//Test 0 fully configured, zeroed-out input
		{
			//input cfg
			map[string]interface{}{
				"_gen_responsibility":  0.0,
				"_gen_requirement":     0.0,
				"_gen_scheduled":       0.0,
				"_load_responsibility": 0.0,
				"_load_requirement":    0.0,
				"_load_scheduled":      0.0,
			},
			//output service
			nil,
			//expected service
			&genLoadServiceNoMux{},
			//expected error
			nil,
		},
		//Test 1 fully configured w/ nums input
		{
			map[string]interface{}{
				"_gen_responsibility":  22.0,
				"_gen_requirement_":    0.0,
				"_gen_scheduled":       2.78,
				"_load_responsibility": 0.0,
				"_load_requirement":    0.9,
				"_load_scheduled":      7.0,
			},
			nil,
			&genLoadServiceNoMux{"", 0.0, 22.0, 2.78, 0.9, 0.0, 7.0},
			nil,
		},
		//Test 2 gen responsibility not configured
		{
			map[string]interface{}{
				"_gen_requirement":     1.0,
				"_gen_scheduled":       2.0,
				"_load_responsibility": 3.0,
				"_load_requirement":    4.0,
				"_load_scheduled":      5.0,
			},
			nil,
			&genLoadServiceNoMux{"", 1.0, 0.0, 2.0, 4.0, 3.0, 5.0},
			nil,
		},
		//Test 3 load requirement not configured
		{
			map[string]interface{}{
				"_gen_responsibility":  1.0,
				"_gen_requirement":     2.0,
				"_gen_scheduled":       3.0,
				"_load_responsibility": 4.0,
				"_load_scheduled":      5.0,
			},
			nil,
			&genLoadServiceNoMux{"", 2.0, 1.0, 3.0, 0.0, 4.0, 5.0},
			nil,
		},
		//Test 4 gen not configured
		{
			map[string]interface{}{
				"_load_responsibility": 1.0,
				"_load_requirement":    2.0,
				"_load_scheduled":      3.0,
			},
			nil,
			&genLoadServiceNoMux{"", 0.0, 0.0, 0.0, 2.0, 1.0, 3.0},
			nil,
		},
		//Test 5 load not configured
		{
			map[string]interface{}{
				"_gen_responsibility": 1.0,
				"_gen_requirement":    2.0,
				"_gen_scheduled":      3.0,
			},
			nil,
			&genLoadServiceNoMux{"", 2.0, 1.0, 3.0, 0.0, 0.0, 0.0},
			nil,
		},
		//Test 6 nil config
		{
			nil,
			nil,
			&genLoadServiceNoMux{},
			//expected error
			nil,
		},
		//Test 9 incorrect load_scheduled type
		{
			//input cfg
			map[string]interface{}{
				"_gen_responsibility_actual":  0.0,
				"_gen_requirement_actual":     0.0,
				"_gen_scheduled":              0.0,
				"_load_responsibility_actual": 0.0,
				"_load_requirement_actual":    0.0,
				"_load_scheduled":             true,
			},
			//output service
			nil,
			//expected service
			&genLoadServiceNoMux{},
			//expected error
			nil,
		},
	}

	for i, testCase := range glsServiceTestCases {
		output := parseGLSNoMux(testCase.inputConfig, "")
		testCase.outputService = &output
		if !reflect.DeepEqual(testCase.outputService, testCase.expectedService) {
			t.Errorf("parseGLSService() test %d failed. Output service is incorrect", i)
			t.Errorf("---parseGLSService() Test %d---", i)
			calculated := testCase.outputService
			expected := testCase.expectedService
			if calculated.genResponsibility != expected.genResponsibility {
				t.Errorf("Expected: %v  Got: %v for gen_responsibility", expected.genResponsibility, calculated.genResponsibility)
			}
			if calculated.loadResponsibility != expected.loadResponsibility {
				t.Errorf("Expected: %v  Got: %v for load_responsibility", expected.loadResponsibility, calculated.loadResponsibility)
			}
			if calculated.genRequirement != expected.genRequirement {
				t.Errorf("Expected: %v  Got: %v for gen_requirement", expected.genRequirement, calculated.genRequirement)
			}
			if calculated.loadRequirement != expected.loadRequirement {
				t.Errorf("Expected: %v  Got: %v for load_requirement", expected.loadRequirement, calculated.loadRequirement)
			}
			if calculated.genScheduled != expected.genScheduled {
				t.Errorf("Expected: %v  Got: %v for gen_scheduled", expected.genScheduled, calculated.genScheduled)
			}
			if calculated.loadScheduled != expected.loadScheduled {
				t.Errorf("Expected: %v  Got: %v for load_scheduled", expected.loadScheduled, calculated.loadScheduled)
			}
			t.Errorf("Structural Error")
		}
	}
}
