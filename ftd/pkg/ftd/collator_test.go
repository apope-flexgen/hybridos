package ftd

import "testing"

// Test finding the correct config associated with the given uri.
func TestFindUriConfig(t *testing.T) {
	configUris := []UriConfig{
		0: {BaseUri: "/events"},
		1: {BaseUri: "/assets/ess", Sources: []string{"ess_1", "ess_2"}},
		2: {BaseUri: "/components"},
		3: {BaseUri: "/ftd_test", Sources: []string{""}},
	}

	// Test uris that should not be found
	shouldNotExistTestCases := []string{
		"/site",
		"/assets/ess",
		"/assets/ess/summary",
		"/assets/ess/ess_10",
		"/assets/ess/ess_11",
		"/assets/ess/ess_",
		"/assets/ess/ess_1/",
		"/assets/ess/ess_1/soc",
		"/ftd_test/data",
	}
	testShouldNotExist := func(testUri string) {
		cfg, exists := findUriConfig(testUri, configUris)
		if exists {
			t.Errorf("%s config was erroneously found with config %v", testUri, cfg)
		}
	}
	for _, testCase := range shouldNotExistTestCases {
		testShouldNotExist(testCase)
	}

	// Test uris that should be found
	type shouldExistTestCase struct {
		testUri     string
		expectedCfg *UriConfig
	}
	shouldExistTestCases := []shouldExistTestCase{
		{"/events", &configUris[0]},
		{"/assets/ess/ess_1", &configUris[1]},
		{"/assets/ess/ess_2", &configUris[1]},
		{"/components", &configUris[2]},
		{"/components/feeders", &configUris[2]},
		{"/components/feeders/feed_1", &configUris[2]},
		{"/ftd_test", &configUris[3]},
	}
	testShouldExist := func(testUri string, expectedCfg *UriConfig) {
		cfg, exists := findUriConfig(testUri, configUris)
		if !exists {
			t.Errorf("%s config was erroneously not found", testUri)
			if cfg != expectedCfg {
				t.Errorf("%s found the wrong config: %v", testUri, cfg)
			}
		}
	}
	for _, testCase := range shouldExistTestCases {
		testShouldExist(testCase.testUri, testCase.expectedCfg)
	}
}
