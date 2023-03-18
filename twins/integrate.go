package main

//function calculates delta accumulation at given rate over time (deltaT).  returns new accumulated value and rate value, limited by maximum and minimum, and under/over bool flags when limited
func getIntegral(accumulatedIn, rateIn, deltaT, minimum, maximum float64) (accumulatedOut, rateOut float64, under, over bool) {
	//initialize accumulated value, rate, and flags
	accumulatedOut = accumulatedIn + (rateIn * deltaT)
	rateOut = rateIn
	under = false
	over = false

	//if max or min exceeded, return limited accumulation, rate, and flags
	if (accumulatedOut > maximum) {
		rateOut = (maximum - accumulatedIn) / deltaT
		accumulatedOut = maximum
		over = true
	} else if (accumulatedOut < minimum) {
		rateOut = (minimum - accumulatedIn) / deltaT
		accumulatedOut = minimum
		under = true
	}
	return accumulatedOut, rateOut, under, over
}
