package main

import (
	"math"
)

//function conforms P and Q inputs to a total S limit.  either P or Q could be prioritized via dominant/subordinate inputs
func getPowerLimit(apparentPowerLimit, domPower, subPower float64)(domPowerLimited, subPowerLimited float64) {
	sign := 1.0 //this variable will track sign throughout function

	//ensure domPower doesnt exceed apparent power limit
	if (math.Abs(domPower) > apparentPowerLimit) {
		if (math.Signbit(domPower)) {
			sign = -1.0
		}
		domPower = sign * apparentPowerLimit 
	}
	sign = 1;  //reset sign
	//reduce subpower to conform to apparent power limit
	if (math.Pow(math.Pow(domPower, 2) + math.Pow(subPower, 2), .5) > apparentPowerLimit) {
		if (math.Signbit(subPower)) {
			sign = -1.0
		}
		subPower = sign * math.Pow(math.Pow(apparentPowerLimit, 2) - math.Pow(domPower, 2), .5)
	}

	return domPower, subPower
}