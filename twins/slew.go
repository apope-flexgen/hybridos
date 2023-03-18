package main

//this function calculates a new value given a current value, target value, slew rate, and time delta
func getSlew(currentValue, targetValue, slewRate, deltaT float64) (nextValue float64) {
	maxDelta := slewRate * deltaT //the maximum delta possible given slew rate and time delta

	if (currentValue < targetValue) {
		if (currentValue + maxDelta < targetValue) {
			return (currentValue + maxDelta)
		} else {
			return targetValue
		}
	} else {
		if (currentValue - maxDelta > targetValue) {
			return (currentValue - maxDelta)
		} else {
			return targetValue
		}
	}	
}