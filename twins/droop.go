package main

//this function calculates slope intercept parameters, m and b, from percent slope and nominal x, y inputs
//slope is droop slope in y/x e.g. W/Hz or VAR/V
//offset is the y intercept of the slope line, e.g. W or VAR
//percent is droop percentage of x for 100% y output (-1 to 1)
//XNom is x-intercept, e.g. nominal Hz or V
//YNom is y-coordinate at XNom*(1-percent), e.g. nominal W or VAR
func getSlope(percent, YNom, XNom float64) (slope, offset float64) {
	if XNom*percent != 0 {
		slope = YNom / (-XNom * percent) //derived from slope intercept formula
	} else {
		// set slope to usefully large negative number (low droop, stiff source) if XNom or percent missing
		slope = -1e15
	}
	offset = -1 * slope * XNom
	return slope, offset
}

//this function combines multiple slope intercept parameter instances into one equivalent
func sumSlope(slope, offset, slopePrev, offsetPrev float64) (slopeSum, offsetSum float64) {
	slopeSum = slope + slopePrev
	offsetSum = offset + offsetPrev
	return slopeSum, offsetSum
}

//this function calculates an asset y-coordinate output given a x-coordinate input and slope intercept parameters
//xIn is the instaneous x-coordinate input that yOut is calculated from, e.g. drooped hertz
func getY(xIn, slope, offset float64) (yOut float64) {
	yOut = slope*xIn + offset
	return yOut
}

//this function calculates an asset x-coordinate output given a y-coordinate input and slope intercept parameters
//yIn is the instaneous y-coordinate input that xOut is calculated from, e.g. power setpoint
func getX(yIn, slope, offset float64) (xOut float64) {
	if slope != 0 {
		xOut = (yIn - offset) / slope
	} else {
		xOut = 0 // prevent NaNs when used in combineTerminals when no grid forming assets are running
	}
	return xOut
}
