package main

import (
	"math"
	"testing"
)

var droop1 = droop{
	YNom:    1,
	Percent: .1,
	XNom:    50,
	slope:   -0.2,
	offset:  10.0,
}

var droop2 = droop{
	YNom:    2,
	Percent: .5,
	XNom:    50,
	slope:   -0.08,
	offset:  4.0,
}

var droop3 = droop{
	YNom:    5,
	Percent: .2,
	XNom:    50,
	slope:   -0.5,
	offset:  25.0,
}

//Test getSlope function for accuracy with various inputs
func TestGetSlope(t *testing.T) {

	type testEntry struct {
		Percent float64
		YNom    float64
		XNom    float64
		slope   float64
		offset  float64
	}

	vector := []testEntry{
		testEntry{droop1.Percent, droop1.YNom, droop1.XNom, droop1.slope, droop1.offset},
		testEntry{droop2.Percent, droop2.YNom, droop2.XNom, droop2.slope, droop2.offset},
		testEntry{droop3.Percent, droop3.YNom, droop3.XNom, droop3.slope, droop3.offset},
	}

	for i, entry := range vector {
		slope, offset := getSlope(entry.Percent, entry.YNom, entry.XNom)
		if !(slope == entry.slope && offset == entry.offset) {
			t.Log("getSlope Test", i)
			t.Logf("Got %.2f slope, %.2f offset. expected %.2f slope, %.2f offset", slope, offset, entry.slope, entry.offset)
			t.Fail()
		}
	}
}

//Test sumSlope works for 1 to n iterations with various inputs
func TestSumSlope(t *testing.T) {
	slopeSum, offsetSlope := sumSlope(0, 0, 0, 0)
	//test w all 0 inputs
	if !(slopeSum == 0 && offsetSlope == 0) {
		t.Log("sumSlope Test 1")
		t.Logf("Got %.2f slope, %.2f offset. expected 0 slope, 0 offset", slopeSum, offsetSlope)
		t.Fail()
	}
	//test first iteration w 1 set of inputs
	slopeSum, offsetSlope = sumSlope(droop1.slope, droop1.offset, slopeSum, offsetSlope)
	if !(slopeSum == -0.2 && offsetSlope == 10) {
		t.Log("sumSlope Test 2")
		t.Logf("Got %.2f slope, %.2f offset. expected -0.20 slope, 10.00 offset", slopeSum, offsetSlope)
		t.Fail()
	}
	//test 2nd interation w 2 sets of inputs
	slopeSum, offsetSlope = sumSlope(droop2.slope, droop2.offset, slopeSum, offsetSlope)
	if !(slopeSum == -0.28 && offsetSlope == 14) {
		t.Log("sumSlope Test 3")
		t.Logf("Got %.2f slope, %.2f offset. expected -0.28 slope, 14.00 offset", slopeSum, offsetSlope)
		t.Fail()
	}
	//test 3rd iteration
	slopeSum, offsetSlope = sumSlope(droop3.slope, droop3.offset, slopeSum, offsetSlope)
	if !(slopeSum == -0.78 && offsetSlope == 39) {
		t.Log("sumSlope Test 4")
		t.Logf("Got %.2f slope, %.2f offset. expected -0.78 slope, 39.00 offset", slopeSum, offsetSlope)
		t.Fail()
	}
}

//Test getY returns correct x-coordinate output for all various inputs
func TestGetY(t *testing.T) {

	type testEntry struct {
		xIn    float64
		yOut   float64
		slope  float64
		offset float64
	}

	vector := []testEntry{
		testEntry{50.0, 0.0, droop1.slope, droop1.offset},
		testEntry{50.0, 0.0, droop2.slope, droop2.offset},
		testEntry{50.0, 0.0, droop3.slope, droop3.offset},
		testEntry{48.0, 0.4, droop1.slope, droop1.offset},
		testEntry{48.0, 0.16, droop2.slope, droop2.offset},
		testEntry{48.0, 1.0, droop3.slope, droop3.offset},
		testEntry{43.0, 1.4, droop1.slope, droop1.offset},
		testEntry{43.0, 0.56, droop2.slope, droop2.offset},
		testEntry{43.0, 3.5, droop3.slope, droop3.offset},
		testEntry{53.0, -0.6, droop1.slope, droop1.offset},
		testEntry{53.0, -0.24, droop2.slope, droop2.offset},
		testEntry{53.0, -1.5, droop3.slope, droop3.offset},
		testEntry{58.0, -1.6, droop1.slope, droop1.offset},
		testEntry{58.0, -0.64, droop2.slope, droop2.offset},
		testEntry{58.0, -4.0, droop3.slope, droop3.offset},
	}

	for i, entry := range vector {
		yOut := getY(entry.xIn, entry.slope, entry.offset)
		if !(math.Abs(yOut-entry.yOut) < 1e-2) {
			t.Log("getY Test", i)
			t.Logf("Got %.2f expected %.2f", yOut, entry.yOut)
			t.Fail()
		}
	}
}

//Test getX returns correct y-coordinate for all various inputs
func TestGetX(t *testing.T) {

	type testEntry struct {
		yIn    float64
		xOut   float64
		slope  float64
		offset float64
	}

	slope := droop1.slope + droop2.slope + droop3.slope
	offset := droop1.offset + droop2.offset + droop3.offset

	vector := []testEntry{
		testEntry{1.56, 48.0, slope, offset},
		testEntry{5.46, 43.0, slope, offset},
		testEntry{-2.34, 53.0, slope, offset},
		testEntry{-6.24, 58.0, slope, offset},
	}

	for i, entry := range vector {
		xOut := getX(entry.yIn, entry.slope, entry.offset)
		if !(math.Abs(xOut-entry.xOut) < 1e-2) {
			t.Log("getX Test", i)
			t.Logf("Got %.2f expected %.2f", xOut, entry.xOut)
			t.Fail()
		}
	}
}
