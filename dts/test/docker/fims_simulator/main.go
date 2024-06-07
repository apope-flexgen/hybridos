package main

import (
	"fims"
	"log"
	"math/rand"
	"time"
)

func main() {
	fimsObj, err := fims.Connect("fims_simulator")
	if err != nil {
		log.Fatalf("Failed to connect to fims")
	}

	val := float64(0)
	for {
		fimsObj.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    "/test_a",
			Body: map[string]interface{}{
				"random_walk": val,
				"bit_string_mc": generateBitStringModbusClientStyle(
					int(time.Now().UnixMicro()/200000)%(1<<14),
					[]string{
						"BMS DC Current Max Threshold Exceeded",
						"BMS DC Current Min Threshold Exceeded",
						"BMS Max Cell Temperature Threshold Exceeded",
						"BMS Max Cell Voltage Threshold Exceeded",
						"BMS Min Cell Temperature Threshold Exceeded",
						"BMS Min Cell Voltage Threshold Exceeded",
						"BMS State of Health Below Threshold",
						"BMS DC Voltage Max Threshold Exceeded",
						"BMS DC Voltage Min Threshold Exceeded",
						"BMS SOC Max Threshold Exceeded",
						"BMS SOC Min Threshold Exceeded",
						"BMS Cell Voltage Delta Max Threshold Exceeded",
						"BMS Cell Temperature Delta Max Threshold Exceeded",
						"BMS Number of Racks Online Below Min Threshold",
					},
				),
				"bit_string_sc": generateBitStringSiteControllerStyle(
					int(time.Now().UnixMicro()/200000)%(1<<2),
					[]string{
						"BMS DC Current Max Threshold Exceeded",
						"BMS DC Current Min Threshold Exceeded",
					},
				),
				"on_off": rand.Intn(2) == 0,
				"bagel":  "tasty",
			},
		})
		fimsObj.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    "/test_b",
			Body: map[string]interface{}{
				"random_walk": val,
				"bit_string_mc": generateBitStringModbusClientStyle(
					int(time.Now().UnixMicro()/200000)%(1<<14),
					[]string{
						"BMS DC Current Max Threshold Exceeded",
						"BMS DC Current Min Threshold Exceeded",
						"BMS Max Cell Temperature Threshold Exceeded",
						"BMS Max Cell Voltage Threshold Exceeded",
						"BMS Min Cell Temperature Threshold Exceeded",
						"BMS Min Cell Voltage Threshold Exceeded",
						"BMS State of Health Below Threshold",
						"BMS DC Voltage Max Threshold Exceeded",
						"BMS DC Voltage Min Threshold Exceeded",
						"BMS SOC Max Threshold Exceeded",
						"BMS SOC Min Threshold Exceeded",
						"BMS Cell Voltage Delta Max Threshold Exceeded",
						"BMS Cell Temperature Delta Max Threshold Exceeded",
						"BMS Number of Racks Online Below Min Threshold",
					},
				),
				"bit_string_sc": generateBitStringSiteControllerStyle(
					int(time.Now().UnixMicro()/200000)%(1<<2),
					[]string{
						"BMS DC Current Max Threshold Exceeded",
						"BMS DC Current Min Threshold Exceeded",
					},
				),
				"on_off": rand.Intn(2) == 0,
				"bagel":  "tasty",
			},
		})
		val += (100.0 - 200.0*rand.Float64())

		time.Sleep(time.Second / 10)
	}
}

// Generates a bit string field value in the style of modbus_client
func generateBitStringModbusClientStyle(val int, bit_strings []string) interface{} {
	result := []map[string]interface{}{}
	for i, str := range bit_strings {
		if val&(1<<i) != 0 {
			result = append(result, map[string]interface{}{
				"value":  i,
				"string": str,
			})
		}
	}
	return result
}

// Generates a bit string field value in the style of site_controller
func generateBitStringSiteControllerStyle(val int, bit_strings []string) interface{} {
	options := []map[string]interface{}{}
	for i, str := range bit_strings {
		if val&(1<<i) != 0 {
			options = append(options, map[string]interface{}{
				"return_value": i,
				"name":         str,
			})
		}
	}
	if len(options) == 0 {
		return 0
	} else {
		return map[string]interface{}{
			"value":   val,
			"options": options,
		}
	}
}
