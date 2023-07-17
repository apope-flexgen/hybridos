package idcache

import "testing"

func TestGenerate(t *testing.T) {
	cache := New()
	for i := 0; i < 100; i++ {
		newId, err := cache.GenerateId()
		if err != nil {
			t.Errorf("Error generating ID: %v.", err)
		}
		if !cache.CheckId(newId) {
			t.Errorf("Generated %v but did not find it in cache.", newId)
		}
	}
}

func TestFill(t *testing.T) {
	cache := New()
	for i := 0; i <= int(maxId); i++ {
		if _, err := cache.GenerateId(); err != nil {
			t.Errorf("Error generating ID at index %d: %v.", i, err)
			return
		}
	}

	if _, err := cache.GenerateId(); err == nil {
		t.Error("Did not get an error when trying to generate a new ID from a full cache.")
	}
}

func TestDelete(t *testing.T) {
	testIds := []Id{
		12345,
		54321,
		666,
		7,
		0,
		65535,
		10,
	}

	cache := New()
	cache.Add(testIds...)
	if len(cache) != len(testIds) {
		t.Errorf("Length of cache %d does not match length of test ID slice %d.", len(cache), len(testIds))
	}

	for _, id := range testIds {
		cache.DeleteId(id)
	}
	if len(cache) != 0 {
		t.Errorf("Cache is not empty: %v.", cache)
	}
}

// Kind of overlaps with TestGenerate but also checks for IDs that should not exist.
func TestCheck(t *testing.T) {
	testIdsToInclude := []Id{
		12345,
		54321,
		666,
		7,
		0,
		65535,
		10,
	}

	testIdsToExclude := []Id{
		25,
		40,
		53901,
		204,
	}

	cache := New()
	cache.Add(testIdsToInclude...)
	if len(cache) != len(testIdsToInclude) {
		t.Errorf("Length of cache %d does not match length of test ID slice %d.", len(cache), len(testIdsToInclude))
	}

	for _, includedId := range testIdsToInclude {
		if !cache.CheckId(includedId) {
			t.Errorf("Added %v to cache then did not find it.", includedId)
		}
	}

	for _, excludedId := range testIdsToExclude {
		if cache.CheckId(excludedId) {
			t.Errorf("Did not add %v to cache but 'found' it.", excludedId)
		}
	}
}

func TestEquals(t *testing.T) {
	type testCase struct {
		cache1, cache2 IdCache
		expectedResult bool
	}

	tests := []testCase{
		{ // empty caches
			cache1: IdCache{},
			cache2: IdCache{},
			expectedResult: true,
		},
		{ // empty vs non-empty
			cache1: IdCache{},
			cache2: IdCache{123: struct{}{}},
			expectedResult: false,
		},
		{ // non-empty vs empty
			cache1: IdCache{345: struct{}{}},
			cache2: IdCache{},
			expectedResult: false,
		},
		{ // non-empty. same length. equal
			cache1: IdCache{4: struct{}{}},
			cache2: IdCache{4: struct{}{}},
			expectedResult: true,
		},
		{ // non-empty. same length. not equal
			cache1: IdCache{89: struct{}{}, 555: struct{}{}},
			cache2: IdCache{89: struct{}{}, 556: struct{}{}},
			expectedResult: false,
		},
		{ // non-empty. different length
			cache1: IdCache{99: struct{}{}, 543: struct{}{}},
			cache2: IdCache{99: struct{}{}},
			expectedResult: false,
		},
	}

	for i, test := range tests {
		isEqual, reasonNotEqual := test.cache1.Equals(test.cache2)
		if isEqual != test.expectedResult {
			if test.expectedResult {
				t.Errorf("Expected test case %d to have equal caches but they are not for reason %s.", i, reasonNotEqual)
			} else {
				t.Errorf("Expected test case %d to have unequal caches but they are equal.", i)
			}
		}
	}
}
