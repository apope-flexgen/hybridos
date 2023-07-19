package research

import (
	"customjson"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"strings"
	"time"

	jsoniter "github.com/json-iterator/go"

	//	"github.com/buger/jsonparser"
	goccy "github.com/goccy/go-json"
)

//	type OrderedMap struct {
//		keys    []string
//		JsonMap map[string]interface{}
//	}\
func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
	log.Printf("%s took %s", name, elapsed)
}

func runCustomJsonUnmarshal(data []byte) (unmarshaled_data interface{}, err error) {
	//defer timeTrack(time.Now(), "customjson Unmarshal")
	return customjson.Unmarshal(data)
}
func runCustomJsonMarshal(v interface{}) (output []byte) {
	//defer timeTrack(time.Now(), "customjson Marshal")
	return customjson.Marshal(v)
}

func runEncodingJsonUnmarshal(data []byte, fx interface{}) (err error) {
	var f interface{}
	//defer timeTrack(time.Now(), "encoding/json Unmarshal")
	return json.Unmarshal(data, f)
}

func runEncodingJsonMarshal(v interface{}) (output []byte, err error) {
	//defer timeTrack(time.Now(), "encoding/json Marshal")
	return json.Marshal(v)
}

func runJsoniterUnmarshal(data []byte, fx interface{}) (err error) {
	var f interface{}
	//defer timeTrack(time.Now(), "jsoniter Unmarshal")
	var json = jsoniter.ConfigCompatibleWithStandardLibrary
	return json.Unmarshal(data, f)
}
func runJsoniterMarshal(v interface{}) (output []byte, err error) {
	//defer timeTrack(time.Now(), "jsoniter Marshal")
	var json = jsoniter.ConfigCompatibleWithStandardLibrary
	return json.Marshal(v)
}

func runGoccyUnmarshal(data []byte, fx interface{}) (err error) {
	var f interface{}
	//defer timeTrack(time.Now(), "go-json Unmarshal")
	return goccy.Unmarshal(data, f)
}

func runGoccyMarshal(v interface{}) (output []byte, err error) {
	//defer timeTrack(time.Now(), "go-json Marshal")
	return goccy.Marshal(v)
}

func myGet(f interface{}, path string) {
	path_arr := strings.Split(path, ".")
	for ix, key := range path_arr {
		fmt.Printf("key from path : %s  ", key)
		if ix == 0 {
			//arrInterface1, ok1 :=
			//_, ok1 := f.(map[string]interface{})[key].([]interface{})
			_, ok1 := f.(map[string]interface{})[key]
			fmt.Println(ok1)
		}

	}

}

func main() {
	var oMap customjson.OrderedMap
	file_byte, err := os.ReadFile("simple.json")
	if err != nil {
		fmt.Printf("Error reading in json file: %s", err)
	}
	file_byte2, err := os.ReadFile("simple2.json")
	if err != nil {
		fmt.Printf("Error reading in json file2: %s", err)
	}
	//var oMap2 OrderedMap
	// file_byte2, err := os.ReadFile("simple2.json")
	// if err != nil {
	// 	fmt.Printf("Error reading in json file: %s", err)
	// }
	unmarshaled_data, _ := runCustomJsonUnmarshal(file_byte)
	oMap = unmarshaled_data.(customjson.OrderedMap)
	//oMap_out, _ := customjson.AddKeyValAfter(oMap, "field4.objectfield2", "key2_5", "5")
	//byte_out :=
	runCustomJsonMarshal(oMap)
	var f interface{}
	var f2 interface{}
	//err :=
	runEncodingJsonUnmarshal(file_byte, f)
	json.Unmarshal(file_byte, &f)
	json.Unmarshal(file_byte2, &f2)
	runEncodingJsonMarshal(f)

	var f3 interface{}
	var f4 interface{}
	//err :=
	runJsoniterUnmarshal(file_byte, f3)
	json.Unmarshal(file_byte, &f3)
	json.Unmarshal(file_byte2, &f4)
	runJsoniterMarshal(f3)

	var f5 interface{}
	var f6 interface{}
	//err :=
	runGoccyUnmarshal(file_byte, f5)
	json.Unmarshal(file_byte, &f5)
	json.Unmarshal(file_byte2, &f6)
	runGoccyMarshal(f5)

	totalTime := 0
	count := 100
	nanoToMilli := 100000

	fastestUnmarshal := 100000000000
	fastestMarshal := 100000000000

	fastestUnmarshalName := ""
	fastestMarshalName := ""

	for i := 1; i <= count; i++ {
		start := time.Now()
		runCustomJsonUnmarshal(file_byte)
		totalTime += int(time.Since(start))
	}
	avg := (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [customjson unmarshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestUnmarshal {
		fastestUnmarshal = avg
		fastestUnmarshalName = "customjson"
	}

	for i := 1; i <= count; i++ {
		unmarshaled_data, _ := runCustomJsonUnmarshal(file_byte)
		oMap = unmarshaled_data.(customjson.OrderedMap)
		start := time.Now()
		runCustomJsonMarshal(oMap)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [customjson marshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestMarshal {
		fastestMarshal = avg
		fastestMarshalName = "customjson"
	}

	for i := 1; i <= count; i++ {
		var f interface{}
		start := time.Now()
		runEncodingJsonUnmarshal(file_byte, f)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [encoding/json unmarshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestUnmarshal {
		fastestUnmarshal = avg
		fastestUnmarshalName = "encoding/json"
	}

	for i := 1; i <= count; i++ {
		var f interface{}
		var f2 interface{}
		//err :=
		runEncodingJsonUnmarshal(file_byte, f)
		json.Unmarshal(file_byte, &f)
		json.Unmarshal(file_byte2, &f2)
		start := time.Now()
		runEncodingJsonMarshal(f)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [encoding/json marshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestMarshal {
		fastestMarshal = avg
		fastestMarshalName = "encoding/json"
	}

	for i := 1; i <= count; i++ {
		var f interface{}
		start := time.Now()
		runJsoniterUnmarshal(file_byte, f)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [jsoniter unmarshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestUnmarshal {
		fastestUnmarshal = avg
		fastestUnmarshalName = "jsoniter"
	}

	for i := 1; i <= count; i++ {
		var f interface{}
		var f2 interface{}
		//err :=
		runJsoniterUnmarshal(file_byte, f)
		json.Unmarshal(file_byte, &f)
		json.Unmarshal(file_byte2, &f2)
		start := time.Now()
		runJsoniterMarshal(f)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [jsoniter marshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestMarshal {
		fastestMarshal = avg
		fastestMarshalName = "jsoniter"
	}

	for i := 1; i <= count; i++ {
		var f interface{}
		start := time.Now()
		runGoccyUnmarshal(file_byte, f)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [goccy unmarshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestUnmarshal {
		fastestUnmarshal = avg
		fastestUnmarshalName = "goccy"
	}

	for i := 1; i <= count; i++ {
		var f interface{}
		var f2 interface{}
		//err :=
		runGoccyUnmarshal(file_byte, f)
		json.Unmarshal(file_byte, &f)
		json.Unmarshal(file_byte2, &f2)
		start := time.Now()
		runGoccyMarshal(f)
		totalTime += int(time.Since(start))
	}
	avg = (totalTime / count) / nanoToMilli
	fmt.Printf("\n\n\n\n [goccy marshal] avg time after %d runs was %d ms \n\n\n\n", count, avg)
	if avg < fastestMarshal {
		fastestMarshal = avg
		fastestMarshalName = "goccy"
	}

	fmt.Printf("\n\n\n\n The fastest Unmarshal was [%s] with an avg time of [%d ms] \n\n\n\n", fastestUnmarshalName, fastestUnmarshal)
	fmt.Printf("\n\n\n\n The fastest Marshal was [%s] with an avg time of [%d ms] \n\n\n\n", fastestMarshalName, fastestMarshal)

	m := f.(map[string]interface{})
	for k, v := range m {
		switch vv := v.(type) {
		case string:
			fmt.Println(k, "is string", vv)
		case float64:
			fmt.Println(k, "is float64", vv)
		case []interface{}:
			fmt.Println(k, "is an array:")
			for i, u := range vv {
				fmt.Println(i, u)
			}
		case map[string]interface{}:
			fmt.Println(k, "is a map:")
		default:
			fmt.Println(k, "is of a type I don't know how to handle")
		}

	}
	myGet(f, "/components/bms_3a")
	myGet(f, "/components/bms_3ax")

	//f1 := f.(map[string]interface{})
	//var f2 = make(map[string]interface{})
	ff := MergeJsons(f, nil, 0)

	fmt.Printf(" got ff #1 %p\n", ff)
	ff = runMergeJsons(f, ff, 0)

	fmt.Printf(" got ff #2 %p\n", ff)
	ff = MergeJsons(f2, ff, 0)
	fmt.Printf(" got ff #3 %p\n", ff)

}

// merge json1 into json2
func runMergeJsons(json1, json2 interface{}, depth int) (json_out interface{}) {
	defer timeTrack(time.Now(), "json merge")
	return MergeJsons(json1, json2, depth)
}
func MergeJsons(json1, json2 interface{}, depth int) (json_out interface{}) {
	//fmt.Printf(" Merge [%d] -> \n", depth)
	//json_o := make(map[string]interface{})
	//var json_out interface{}
	//iMap1
	_, iok1 := json1.(map[string]interface{})
	//iMap2
	_, iok2 := json2.(map[string]interface{})
	if iok1 {
		//fmt.Printf(" [%d] -> json1  is a map \n", depth)
		if !iok2 {
			json2 = json1
			return json2
		} else {
			// we have a list of items in json1 that need to be in json2

			// step through each item in json1 make sure its in json2 if not zap itin
			for idx, _ := range json1.(map[string]interface{}) {
				// we need to know what json1.(map[string]interface{})[idx] is

				_, ok2 := json2.(map[string]interface{})[idx]
				if !ok2 {
					//fmt.Printf(" [%d] -> idx %s  not in json2\n", depth, idx)
					json2.(map[string]interface{})[idx] = json1.(map[string]interface{})[idx]
					continue
					//json2.(map[string]interface{})[idx] = json1.(map[string]interface{})[idx]
				} else {
					v1 := json1.(map[string]interface{})[idx]
					v2 := json2.(map[string]interface{})[idx]

					switch v1t := v1.(type) {
					case bool:
					case string:
					case float64:
						if v1 != v2 {
							fmt.Println(v1, "v1 diff  v2 ->", v2)
							json1.(map[string]interface{})[idx] = v2
						}

					case []interface{}:
						fmt.Println(v1, "v1 is an array:", v1t)
						//for i, u := range vv {
						//	fmt.Println(i, u)
						//}
					case map[string]interface{}:
						//fmt.Printf(" depth [%d] %s is a map: \n", depth, idx)
						switch v2t := v2.(type) {
						case bool:
							fmt.Println(v2, "v2 is bool", v2t)
						case string:
							fmt.Println(v2, "v2 is string", v2t)
						case float64:
							fmt.Println(v2, "v2 is float64", v2t)
						case []interface{}:
							fmt.Println(v2, "v2 is an array:", v2t)
							//for i, u := range vv {
							//	fmt.Println(i, u)
							//}
						case map[string]interface{}:
							//fmt.Printf(" v2 depth [%d] %s both are  maps:\n", depth, idx)
							MergeJsons(v1, v2, depth+1)
						default:
							fmt.Println(v2, " v2 is of a type I don't know how to handle")
						}
					default:
						fmt.Println(v1, "v1 is of a type I don't know how to handle")
					}
					//_, ok1 := json1.(map[string]interface{})[idx].(map[string]interface{})
					//_, ok2 := json2.(map[string]interface{})[idx].(map[string]interface{})

					//fmt.Printf(" [%d] -> idx %s  found in  json2 TODO recurse here\n", depth, idx)
					//MergeJsons(json1.(map[string]interface{})[idx],json2.(map[string]interface{})[idx], depth+1)
				}
			}
		}
	}
	return json1
	// for idx, _ := range json1.(map[string]interface{}) {
	// 	_, ok2 := json2.(map[string]interface{})[idx]
	// 	if !ok2 {
	// 		fmt.Printf(" [%d] -> idx %s  not in json2\n", depth, idx)
	// 	} else {
	// 		// What is json1[idx] if its an object then recurse
	// 		//	MergeJsons(json1[idx], json2[idx])
	// 		oMap1, ok1 := json1[idx].(map[string]interface{})
	// 		oMap2, ok2 := json2[idx].(map[string]interface{})
	// 		if ok1 && ok2 {
	// 			fmt.Printf(" [%d] ->  idx %s in both  json1 and json2\n", depth, idx)
	// 			fmt.Printf("  omap2 type [%T] \n", oMap2)
	// 			if true {
	// 				tempOMap := MergeJsons(oMap1, oMap2, depth+1)
	// 				json2[idx] = tempOMap
	// 			}
	// 		}

	// 		// 			oMap2, ok2 := json2.JsonMap[key1].(OrderedMap)
	// 		// 			if ok1 && ok2 {
	// 		// 				tempOMap := MergeJsons(oMap1, oMap2)
	// 		// 				json_out.JsonMap[key1] = tempOMap

	// 	}
	// 	//} else {
	// 	//	idx, ok2 := json2[idx]
	// 	//	fmt.Printf(" idx %s ok2 %b\n", idx, ok2)
	// 	//}
	// }
	// 	if in(json2.keys, key1) {
	// 		//handle json arrays
	// 		arrInterface1, ok1 := json1.JsonMap[key1].([]interface{})
	// 		arrInterface2, ok2 := json2.JsonMap[key1].([]interface{})
	// 		if ok1 && ok2 {
	// 			for _, value := range arrInterface2 {
	// 				if !inInterfaceArray(arrInterface1, value) {
	// 					arrInterface1 = append(arrInterface1, value)
	// 				}
	// 			}
	// 			json_out.JsonMap[key1] = arrInterface1
	// 		} else {

	// 			//handle json objects
	// 			oMap1, ok1 := json1.JsonMap[key1].(OrderedMap)
	// 			oMap2, ok2 := json2.JsonMap[key1].(OrderedMap)
	// 			if ok1 && ok2 {
	// 				tempOMap := MergeJsons(oMap1, oMap2)
	// 				json_out.JsonMap[key1] = tempOMap
	// 			} else {

	// 				//else default to json1 values
	// 				json_out.JsonMap[key1] = json1.JsonMap[key1]
	// 			}
	// 		}
	// 		json_out.keys = append(json_out.keys, key1)
	// 	} else {
	// 		json_out.keys = append(json_out.keys, key1)
	// 		json_out.JsonMap[key1] = json1.JsonMap[key1]
	// 	}
	// }

	// for _, key2 := range json2.keys {
	// 	if in(json1.keys, key2) {
	// 		//do nothing because we should have already handled it
	// 	} else {
	// 		json_out.keys = append(json_out.keys, key2)
	// 		json_out.JsonMap[key2] = json2.JsonMap[key2]
	// 	}
	// }
	//return json2
}
