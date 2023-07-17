package main

import (
	"reflect"
	"testing"
)

func TestConfigGetListOfScheduleIds(t *testing.T) {
	type testCase struct {
		cfg          schedulerConfig
		expectedList []string
	}

	tests := []testCase{
		{ // no schedules
			cfg:          schedulerConfig{},
			expectedList: []string{},
		},
		{ // just a local schedule
			cfg: schedulerConfig{
				LocalSchedule: &localScheduleConfig{
					Id: "raleigh",
				},
			},
			expectedList: []string{"raleigh"},
		},
		{ // just clients
			cfg: schedulerConfig{
				WebSockets: webSocketsConfig{
					ClientConfigs: &webSocketClientConfigList{
						webSocketClientConfig{Id: "raleigh"},
						webSocketClientConfig{Id: "durham"},
					},
				},
			},
			expectedList: []string{"raleigh", "durham"},
		},
		{ // local schedule + clients
			cfg: schedulerConfig{
				LocalSchedule: &localScheduleConfig{
					Id: "raleigh",
				},
				WebSockets: webSocketsConfig{
					ClientConfigs: &webSocketClientConfigList{
						webSocketClientConfig{Id: "durham"},
						webSocketClientConfig{Id: "chapel_hill"},
					},
				},
			},
			expectedList: []string{"raleigh", "durham", "chapel_hill"},
		},
	}

	for i, test := range tests {
		resultList := test.cfg.getListOfScheduleIds()
		if !reflect.DeepEqual(resultList, test.expectedList) {
			t.Errorf("Test case %d expected %v but got %v.", i, test.expectedList, resultList)
		}
	}
}
