package main

//Helper.go: A collection of helper functions and other lost toys

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"reflect"
	"strings"
)
//returns elements of inslc in reverse order
func reverse(inslc []float64) (outslc []float64) {
	inlen := len(inslc)
	outslc = make([]float64, inlen)
	for i,member := range inslc {
		j := inlen - i - 1
		outslc[j] = member
	}
	return outslc
}

// implements num[0] + num [1] + ... + num[n]
func add(nums ...float64) float64 {
	if len(nums) < 1 {
		return 0
	}
	var total float64 = 0
	for _, num := range nums {
		if num == math.NaN() {
			return math.NaN()
		}
		total += num
	}
	return total
}
// implements num[0] - (num[1] + num[2] + ... + num[n])
func subtract(nums ...float64) float64 {
	if len(nums) < 1 {
		return 0
	}
	if nums[0] == math.NaN(){
		return math.NaN()
	}
	var total float64 = nums[0]
	for _, num := range nums[1:] {
		if num == math.NaN() {
			return math.NaN()
		}
		total -= num
	}
	return total
}
// implements num[0] * num[1] * ... * num[n]
func multiply(nums ...float64) float64 {
	if len(nums) < 1 {
		return 0
	}
	var total float64 = 1
	for _, num := range nums {
		if num == math.NaN() {
			return math.NaN()
		}
		total = total * num
	}
	return total
}
// implements num[0] / (num[1] * num[2] * ... * num[n])
func divide(nums ...float64) float64 {
	if len(nums) < 1 {
		return 0
	}
	if len(nums) < 2 || nums[0] == math.NaN() {
		return nums[0]
	}
	denominator := multiply(nums[1:]...)
	if denominator == 0 {
		log.Println("Error: divide by zero.")
		return math.NaN()
	}
	return nums[0] / denominator
}
// implements base ^ exp
func exponent(base float64, exp float64) float64 {
	return math.Pow(base, exp)
}
func maximum(nums...float64) (float64, int) {
	var max float64 = math.Inf(-1)
	var idx int = 0
	if len(nums) == 0 {
		return math.NaN(), -1
	}
	if len(nums) == 1 {
		return nums[0], idx
	}
	for i, num := range nums {
		if num == math.NaN() {
			continue
		}
		if num > max {
			max = num
			idx = i
		}
	} 
	return max, idx
}
func minimum(nums ...float64) (float64, int) {
	var min float64 = math.Inf(1)
	var idx int = 0
	if len(nums) == 0 {
		return math.NaN(), -1
	}
	if len(nums) == 1 {
		return nums[0], idx
	}
	for i, num := range nums {
		if num == math.NaN() {
			continue
		}
		if num < min {
			min = num
			idx = i
		}
	} 
	return min, idx
}
func average(nums ...float64) float64 {
	length := len(nums)
	if length < 1 {
		return 0
	}
	var avg float64 = 0
	avg = add(nums...)
	avg = divide(avg, float64(length))
	return avg
}
func floor(num float64) float64 {
	return math.Floor(num)
}
func ceil(num float64) float64 {
	return math.Ceil(num)
}
func round(num float64) float64 {
	return math.Round(num)
}
//Removes whitespaces and sets all characters to lowercase in the dirty string dString
func cleanString(dString string) (string) {
	return strings.TrimSpace(strings.ToLower(dString)) //TODO GB: This likely will be ever so slightly faster if switched to ToLower(TrimSpace(string))
}
//returns a deep copy of slice cslice including indeterminatly nested maps and slices
//do not use for anything containing channels, structs, etc.
func deepCopySlice(cslice []interface{}) ([]interface{}) {
	res := make([]interface{}, len(cslice))
	for i, val := range cslice {
		valKind := reflect.ValueOf(val).Kind() 
		if valKind == reflect.Map {
			res[i] = deepCopyMap(val.(map[string]interface{}))
		} else if valKind == reflect.Slice {
			res[i] = deepCopySlice(val.([]interface{}))
		} else {
			res[i] = val
		}
	}
	return res
}

//returns a deep copy of map cmap including indeterminatly nested maps and slices
//do not use for anything containing channels, structs, etc.  
func deepCopyMap(cmap map[string]interface{}) (map[string]interface{}) {
	res := make(map[string]interface{})
	for k,v := range cmap {
		vKind := reflect.ValueOf(v).Kind()
		if vKind == reflect.Map {
			res[k] = deepCopyMap(v.(map[string]interface{}))
		} else if vKind == reflect.Slice {
			res[k] = deepCopySlice(v.([]interface{}))
		} else {
			res[k] = v
		}
	}
	return res
}

//Prints json-like object pmap as a nicely formatted output
func printMap(pmap interface{}) {
	b, err := json.MarshalIndent(pmap, "", "  ")
	if err != nil{
		log.Println("Error printing map, could not marshal object")
		log.Println(err)
	}
	fmt.Print(string(b))
	fmt.Println()
}
