package main

import (
	"fmt"
	"sort"
	"testing"
	"time"

	"github.com/flexgen-power/scheduler/internal/setpoint"
	"github.com/flexgen-power/scheduler/pkg/events"
)

func TestScadaSetSite(t *testing.T) {
	type testCase struct {
		numSites  int
		siteEnum  int
		expectErr bool
	}

	tests := []testCase{
		{ // setting -1 sets pointer to nil
			numSites:  0,
			siteEnum:  -1,
			expectErr: false,
		},
		{ // setting < -1 results in error
			numSites:  0,
			siteEnum:  -2,
			expectErr: true,
		},
		{ // setting value greater than number of site enums results in error
			numSites:  1,
			siteEnum:  1,
			expectErr: true,
		},
		{ // setting anything except for -1 when there are no sites results in error
			numSites:  0,
			siteEnum:  0,
			expectErr: true,
		},
		{ // selecting site 0 when there is one site
			numSites:  1,
			siteEnum:  0,
			expectErr: false,
		},
		{ // selecting site 0 when there are multiple sites
			numSites:  2,
			siteEnum:  0,
			expectErr: false,
		},
		{ // selecting site 1 when there are multiple sites
			numSites:  3,
			siteEnum:  1,
			expectErr: false,
		},
	}

	for i, test := range tests {
		siteEnums = make(sort.StringSlice, 0)
		masterSchedule = scheduleMap{}
		for j := 0; j < test.numSites; j++ {
			sampleSiteId := fmt.Sprintf("sample_site_%d", j)
			siteEnums = append(siteEnums, sampleSiteId)
			masterSchedule[sampleSiteId] = &schedule{}
		}
		siteEnums.Sort()

		var eScada scadaEvent
		replyObj, err := eScada.setVariable("site", test.siteEnum)
		if test.expectErr {
			if err == nil {
				t.Errorf("Expected error for test index %d but did not get one.", i)
			}
			continue
		}
		replyInt, ok := replyObj.(int)
		if !ok {
			t.Errorf("Test index %d got back %T not int for replyObj when setting site enumeration.", i, replyObj)
			continue
		}
		if replyInt != test.siteEnum {
			t.Errorf("Test index %d got back %d not %d for replyObj when setting site enumeration.", i, replyInt, test.siteEnum)
		}

		if err != nil {
			t.Errorf("Did not expect error for test index %d but got: %v.", i, err)
			continue
		}

		if test.siteEnum == -1 && eScada.site != nil {
			t.Errorf("Expected test %d site to be nil but got %s.", i, eScada.site.id)
			continue
		}

		expectedSiteId := fmt.Sprintf("sample_site_%d", test.siteEnum)
		expectedSite := masterSchedule[expectedSiteId]
		if eScada.site != expectedSite {
			t.Errorf("Expected test %d site to be %s but got %v.", i, expectedSiteId, eScada.site)
		}
	}
}

func TestScadaConvertToEvent(t *testing.T) {
	type testCase struct {
		inputCfg         scadaConfiguration
		inputDefaultSite *schedule
		eScada           scadaEvent
		expectedEvent    events.Event
		expectErr        bool
	}

	modes = modeMap{
		"target_soc": &mode{
			Variables: setpoint.List{
				{
					Id:      "soc_target",
					VarType: "Float",
				},
			},
		},
	}

	tests := []testCase{
		{ // standard conversion
			inputCfg: scadaConfiguration{
				NumFloats: 1,
			},
			inputDefaultSite: nil,
			eScada: scadaEvent{
				site: &schedule{
					timezone: forceLoadLocation("America/New_York"),
				},
				modeId:     "target_soc",
				year:       2023,
				month:      3,
				day:        15,
				hour:       16,
				minute:     0,
				duration:   5,
				repeatDaily: scadaFalse,
				floatVars:  []float64{50},
				intVars:    []int{},
				boolVars:   []scadaBool{},
				stringVars: []string{},
			},
			expectedEvent: events.Event{
				StartTime: time.Date(2023, 3, 15, 12, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  5,
				Mode:      "target_soc",
				Variables: map[string]interface{}{
					"soc_target": 50.0,
				},
				Repeat: events.NewNonRepeatingSeries(),
			},
			expectErr: false,
		},
		{ // conversion using default site
			inputCfg: scadaConfiguration{
				NumFloats: 1,
			},
			inputDefaultSite: &schedule{
				timezone: forceLoadLocation("America/New_York"),
			},
			eScada: scadaEvent{
				site:       nil,
				modeId:     "target_soc",
				year:       2023,
				month:      3,
				day:        15,
				hour:       16,
				minute:     0,
				duration:   5,
				repeatDaily: scadaFalse,
				floatVars:  []float64{50.0},
				intVars:    []int{},
				boolVars:   []scadaBool{},
				stringVars: []string{},
			},
			expectedEvent: events.Event{
				StartTime: time.Date(2023, 3, 15, 12, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  5,
				Mode:      "target_soc",
				Variables: map[string]interface{}{
					"soc_target": 50.0,
				},
				Repeat: events.NewNonRepeatingSeries(),
			},
			expectErr: false,
		},
		{ // error because no site selected and no default site provided
			inputCfg: scadaConfiguration{
				NumFloats: 1,
			},
			inputDefaultSite: nil,
			eScada: scadaEvent{
				site:       nil,
				modeId:     "target_soc",
				year:       2023,
				month:      3,
				day:        15,
				hour:       16,
				minute:     0,
				duration:   5,
				repeatDaily: scadaFalse,
				floatVars:  []float64{50.0},
				intVars:    []int{},
				boolVars:   []scadaBool{},
				stringVars: []string{},
			},
			expectedEvent: events.Event{},
			expectErr: true,
		},
	}

	for i, test := range tests {
		schedCfg.Scada = test.inputCfg

		resultEvent, err := test.eScada.convertToEvent(test.inputDefaultSite)
		if test.expectErr {
			if err == nil {
				t.Errorf("Test index %d expected error but did not get one.", i)
			}
			continue
		}

		if err != nil {
			t.Errorf("Test index %d did not expect an error but got: %v.", i, err)
			continue
		}

		if areNotEqual, reasonNotEqual := resultEvent.Equals(&test.expectedEvent); !areNotEqual {
			t.Errorf("Test index %d failed equality check for reason: %s.", i, reasonNotEqual)
		}
	}
}
