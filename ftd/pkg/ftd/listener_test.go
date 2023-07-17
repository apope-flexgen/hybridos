package ftd

import (
	"sort"
	"testing"
)

func TestBuildUriList(t *testing.T) {
	configUris := []UriConfig{
		{BaseUri: "/events"},
		{BaseUri: "/assets/ess", Sources: []string{"ess_1", "ess_2"}},
		{BaseUri: "/components", Sources: []string{"ess_1_ls", "ess_2_ls"}},
		{BaseUri: "/components", Sources: []string{"ess_1_hs", "ess_2_hs"}},
		{BaseUri: "/ftd_test", Sources: []string{""}},
		{BaseUri: "/cops/stats", Sources: []string{"site_controller"}},
	}

	expectedUriList := []string{
		"/events",
		"/assets/ess",
		"/components",
		"/ftd_test",
		"/cops/stats",
		"/cops/summary", // Ensure we always subscribe to this uri
	}
	sort.Strings(expectedUriList) // sort for easier comparison

	uriList := buildUriList(configUris)
	sort.Strings(uriList) // sort for easier comparison

	if len(uriList) != len(expectedUriList) {
		t.Fatalf("Uri list constructed had length %d but expected length was %d. Uri list constructed was %v.", len(uriList), len(expectedUriList), uriList)
	}
	for i := 0; i < len(uriList); i++ {
		if uriList[i] != expectedUriList[i] {
			t.Errorf("Uri list constructed had item %s where it was expected to have item %s", uriList[i], expectedUriList[i])
		}
	}
}
