package main

import (
	"fims"
	"testing"
)

// Test forming a format subset forest
func TestCreateUriFormatSubsetForest(t *testing.T) {
	// create test messages and expected forest
	testMsgs := []fims.FimsMsg{
		{Uri: "/a", Body: map[string]interface{}{"f1": 2.1, "f2": "hello world", "f3": true}},
		{Uri: "/b", Body: map[string]interface{}{"f1": 1.7, "f2": "good bye"}},
		{Uri: "/c", Body: map[string]interface{}{"f1": 121.5}},
		{Uri: "/d", Body: map[string]interface{}{"f1": 17.9}},
		{Uri: "/e", Body: map[string]interface{}{"f4": -16.5, "f6": "apple"}},
		{Uri: "/f", Body: map[string]interface{}{"f5": 143.12, "f6": "orange"}},
		{Uri: "/g", Body: map[string]interface{}{"f1": map[string]interface{}{}, "f2": "hello world", "f3": 0, "f6": "mango"}},
		{Uri: "/h", Body: map[string]interface{}{"f6": "raspberry"}},
		{Uri: "/i", Body: map[string]interface{}{"f6": 78.614}},
	}
	expectedForest := []*uriFormatNode{
		{
			uris: []string{"/c", "/d"},
			children: []*uriFormatNode{
				{
					uris: []string{"/b"},
					children: []*uriFormatNode{
						{
							uris: []string{"/a"},
						},
					},
				},
			},
		},
		{
			uris: []string{"/h"},
			children: []*uriFormatNode{
				{
					uris: []string{"/e"},
				},
				{
					uris: []string{"/f"},
				},
				{
					uris: []string{"/g"},
				},
			},
		},
		{
			uris: []string{"/i"},
		},
	}

	// collect them into necessary data structures
	formatStringToUris, formatStringToFormat := collectTestMessages(testMsgs)

	// create forest
	forest := createUriFormatSubsetForest(formatStringToUris, formatStringToFormat)

	// compare forest to expected forest
	// (note that although forests are sorted, the result of creating the forest may be nondeterministic in the
	// case where one format is a superset of multiple others. If that is the case with the test input,
	// then we might want to try creating the forest multiple times or otherwise accomodate that nondeterminism)
	if !areEqual(forest, expectedForest) {
		t.Error("The constructed test format subset forest was not equal to the expected forest")
		t.Errorf("constructed:\n%s", stringRepresentingUriFormatForest(forest, 0))
		t.Errorf("expected:\n%s", stringRepresentingUriFormatForest(expectedForest, 0))
	}
}

// Compares two format subset forests for equality (order dependent)
func areEqual(forestA []*uriFormatNode, forestB []*uriFormatNode) bool {
	// nil checks
	if forestA == nil && forestB == nil {
		return true
	} else if forestA == nil || forestB == nil {
		return false
	}
	// check number of roots
	if len(forestA) != len(forestB) {
		return false
	}
	// check for each root
	for i := range forestA {
		// compare uris
		if len(forestA[i].uris) != len(forestB[i].uris) {
			return false
		}
		for j := range forestA[i].uris {
			if forestA[i].uris[j] != forestB[i].uris[j] {
				return false
			}
		}
		// compare children
		if !areEqual(forestA[i].children, forestB[i].children) {
			return false
		}
	}
	return true
}
