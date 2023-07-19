package customjson

import (
	"encoding/json"
	"fmt"
	"reflect"
	"regexp"
	"sort"
	"strings"
)

type OrderedMap struct {
	keys    []string
	JsonMap map[string]interface{}
}

func Unmarshal(data []byte) (unmarshaled_data interface{}, err error) {
	var jsonObj map[string]interface{}
	var keyIndices map[int]string = make(map[int]string)
	var keyLocs []int
	var jsonArr []interface{}
	var jsonBool bool
	var jsonFloat float64
	var jsonString string
	objErr := json.Unmarshal(data, &jsonObj)
	if objErr != nil {
		arrErr := json.Unmarshal(data, &jsonArr)
		if arrErr != nil {
			boolErr := json.Unmarshal(data, &jsonBool)
			if boolErr != nil {
				floatErr := json.Unmarshal(data, &jsonFloat)
				if floatErr != nil {
					stringErr := json.Unmarshal(data, &jsonString)
					if stringErr != nil {
						nilErr := json.Unmarshal(data, nil)
						if nilErr != nil {
							err = fmt.Errorf("Error unmarshaling JSON into known JSON type.")
						}
						return nil, nil
					} else { // if it's a string
						return jsonString, stringErr
					}
				} else { // if it's a float
					return jsonFloat, floatErr
				}
			} else { // if it's a bool
				return jsonBool, boolErr
			}
		} else { // if it's an array
			for i, item := range jsonArr {
				item_bytes, _ := json.Marshal(item)
				jsonArr[i], err = Unmarshal(item_bytes)
			}
			return jsonArr, arrErr
		}
	} else { // if it's an object
		jsonOrderedObj := OrderedMap{
			keys:    make([]string, 0),
			JsonMap: jsonObj,
		}
		for key, _ := range jsonObj {
			dataStr := string(data)
			keyIndex := strings.Index(dataStr, fmt.Sprintf(`"%s"`, key))
			keyIndices[keyIndex] = key
			keyLocs = append(keyLocs, keyIndex)
		}
		sort.Ints(keyLocs)

		for _, index := range keyLocs {
			item, _ := json.Marshal(jsonOrderedObj.JsonMap[keyIndices[index]])
			jsonOrderedObj.JsonMap[keyIndices[index]], _ = Unmarshal(item)
			jsonOrderedObj.keys = append(jsonOrderedObj.keys, keyIndices[index])
		}

		return jsonOrderedObj, objErr

	}

}

func Marshal(v interface{}) (output []byte) {
	output = make([]byte, 0)
	oMap, _ := v.(OrderedMap)
	IsOMap := len(oMap.JsonMap) > 0
	jArray, IsArray := v.([]interface{})
	jString, IsString := v.(string)
	jBool, IsBool := v.(bool)
	jFloat, IsFloat := v.(float64)
	if IsOMap {
		if len(oMap.keys) > 0 {
			key_byte_arr := []byte("{\n")
			output = append(output, key_byte_arr...)
		}
		for j, key := range oMap.keys {
			key_byte_arr := []byte(fmt.Sprintf(`"%s": `, key))
			output = append(output, key_byte_arr...)
			byte_map := Marshal(oMap.JsonMap[key])
			output = append(output, byte_map...)
			if j != len(oMap.keys)-1 {
				key_byte_arr := []byte(",\n")
				output = append(output, key_byte_arr...)
			} else {
				output = append(output, '\n')
			}
		}
		if len(oMap.keys) > 0 {
			output = append(output, '}')
		}
	} else if IsArray {
		key_byte_arr := []byte("[\n")
		output = append(output, key_byte_arr...)
		for j, arrObj := range jArray {
			byte_arr := Marshal(arrObj)
			output = append(output, byte_arr...)
			if j != len(jArray)-1 {
				key_byte_arr := []byte(",\n")
				output = append(output, key_byte_arr...)
			} else {
				key_byte_arr := []byte("\n")
				output = append(output, key_byte_arr...)
			}
		}
		output = append(output, ']')
	} else if IsBool {
		byte_arr := []byte(fmt.Sprint(jBool))
		output = append(output, byte_arr...)
	} else if IsFloat {
		var jInt int
		if (float64(int(jFloat))-jFloat) > -0.00000000001 || (float64(int(jFloat))-jFloat) < -0.9999999999 {
			jInt = int(jFloat)
			byte_arr := []byte(fmt.Sprintf(`%d`, jInt))
			output = append(output, byte_arr...)
		} else {
			byte_arr := []byte(fmt.Sprint(jFloat))
			output = append(output, byte_arr...)
		}
	} else if IsString {
		byte_arr := []byte(fmt.Sprintf(`"%s"`, jString))
		output = append(output, byte_arr...)
	} else {
		byte_arr := []byte(nil)
		output = append(output, byte_arr...)
	}
	return output
}

func MergeJsons(json1, json2 OrderedMap) (json_out OrderedMap) {
	json_out = OrderedMap{
		keys:    make([]string, 0),
		JsonMap: make(map[string]interface{}),
	}

	for _, key1 := range json1.keys {
		if in(json2.keys, key1) {
			//handle json arrays
			arrInterface1, ok1 := json1.JsonMap[key1].([]interface{})
			arrInterface2, ok2 := json2.JsonMap[key1].([]interface{})
			if ok1 && ok2 {
				for _, value := range arrInterface2 {
					if !inInterfaceArray(arrInterface1, value) {
						arrInterface1 = append(arrInterface1, value)
					}
				}
				json_out.JsonMap[key1] = arrInterface1
			} else {

				//handle json objects
				oMap1, ok1 := json1.JsonMap[key1].(OrderedMap)
				oMap2, ok2 := json2.JsonMap[key1].(OrderedMap)
				if ok1 && ok2 {
					tempOMap := MergeJsons(oMap1, oMap2)
					json_out.JsonMap[key1] = tempOMap
				} else {

					//else default to json1 values
					json_out.JsonMap[key1] = json1.JsonMap[key1]
				}
			}
			json_out.keys = append(json_out.keys, key1)
		} else {
			json_out.keys = append(json_out.keys, key1)
			json_out.JsonMap[key1] = json1.JsonMap[key1]
		}
	}

	for _, key2 := range json2.keys {
		if in(json1.keys, key2) {
			//do nothing because we should have already handled it
		} else {
			json_out.keys = append(json_out.keys, key2)
			json_out.JsonMap[key2] = json2.JsonMap[key2]
		}
	}
	return json_out
}

func in(slice []string, str string) bool {
	for _, s := range slice {
		if s == str {
			return true
		}
	}
	return false
}

func inInterfaceArray(slice []interface{}, str interface{}) bool {
	for _, s := range slice {
		if reflect.DeepEqual(s, str) {
			return true
		}
	}
	return false
}

func FormatJson(input []byte) []byte {
	strInput := string(input)
	//add a space after all colons :
	re, _ := regexp.Compile(`":[^[:space:]]"`)
	i := re.FindStringIndex(strInput)
	for i != nil {
		strInput = strInput[:i[0]+1] + " " + strInput[i[0]+1:]
		i = re.FindStringIndex(strInput)
	}

	//add a new line after all the commas ,
	re, _ = regexp.Compile(`,[^[:space:]]|, "`)
	i = re.FindStringIndex(strInput)
	for i != nil {
		strInput = strInput[:i[0]+1] + "\n" + strInput[i[0]+1:]
		i = re.FindStringIndex(strInput)
	}

	// assume tabs vs spaces is consistent throughout the document
	// but if it's not, use the one that's used more
	containsTab := strings.Contains(strInput, "\t")
	containsSpaceTab := strings.Contains(strInput, "    ")
	var tab string
	if containsTab {
		tab = "\t"
	} else if containsTab && containsSpaceTab {
		numTabs := strings.Count(strInput, "\t")
		numSpaceTabs := strings.Count(strInput, "    ")
		if numTabs > numSpaceTabs {
			tab = "\t"
		} else {
			tab = "    "
		}
	} else {
		tab = "    "
	}
	inputArr := strings.Split(strInput, "\n")
	//fmt.Printf("ip array [%v] len %v ", inputArr, len(inputArr))

	var strOutput string
	var indentLevel int = 0
	for i, line := range inputArr {

		//fmt.Printf("idx [%v] line [%v] indent %v \n", i, line, indentLevel)

		inputArr[i] = strings.TrimSpace(inputArr[i])
		if (strings.Contains(line, "}") || strings.Contains(line, "]")) && !strings.Contains(line, "{") && !strings.Contains(line, "[") {
			indentLevel -= 1
		}
		for j := 0; j < indentLevel; j++ {
			inputArr[i] = tab + inputArr[i]
		}
		inputArr[i] += "\n"
		if len(strings.TrimSpace(inputArr[i])) > 0 {
			strOutput += inputArr[i]
		}
		if (strings.Contains(line, "{") || strings.Contains(line, "[")) && !strings.Contains(line, "}") && !strings.Contains(line, "]") {
			indentLevel += 1
		}
	}
	//fmt.Printf(" format output-->\n%v\n", strOutput)
	return []byte(strOutput)
}

//you could probably figure out a good way to do this with pointers, but right now I'm being stupid and inefficient.
func AddKeyVal(orig_data interface{}, path string, new_key string, new_value string) (new_data OrderedMap, err error) {
	path_arr := strings.Split(path, ".")
	oMap, ok := orig_data.(OrderedMap)
	var oMapArr []OrderedMap = make([]OrderedMap, 0)
	oMapArr = append(oMapArr, oMap)
	if ok {
		for _, key := range path_arr {
			oMap, ok = oMap.JsonMap[key].(OrderedMap)
			if !ok {
				err = fmt.Errorf("could not find path to %s", key)
			}
			oMapArr = append(oMapArr, oMap)
		}
		if oMapArr[len(oMapArr)-1].JsonMap != nil {
			oMapArr[len(oMapArr)-1].JsonMap[new_key] = new_value
			oMapArr[len(oMapArr)-1].keys = append(oMap.keys, new_key)
		} else {
			oMapArr[len(oMapArr)-1].keys = make([]string, 0)
			oMapArr[len(oMapArr)-1].JsonMap = make(map[string]interface{})
			oMapArr[len(oMapArr)-1].JsonMap[new_key] = new_value
			oMapArr[len(oMapArr)-1].keys = append(oMap.keys, new_key)
		}

		for i := len(path_arr) - 1; i >= 0; i -= 1 {
			oMapArr[i].JsonMap[path_arr[i]] = oMapArr[i+1]
		}
	}

	return oMapArr[0], err
}

func getIndex(slice []string, str string) int {
	for i, s := range slice {
		if s == str {
			return i
		}
	}
	return -1
}

func AddKeyValAfter(orig_data interface{}, path string, new_key string, new_value string) (new_data OrderedMap, err error) {
	path_arr := strings.Split(path, ".")
	after := path_arr[len(path_arr)-1]
	oMap, ok := orig_data.(OrderedMap)
	var oMapArr []OrderedMap = make([]OrderedMap, 0)
	oMapArr = append(oMapArr, oMap)
	if ok {
		for _, key := range path_arr[:len(path_arr)-1] {
			oMap, ok = oMap.JsonMap[key].(OrderedMap)
			if !ok {
				err = fmt.Errorf("could not find path to %s", key)
			}
			oMapArr = append(oMapArr, oMap)
		}
		if oMapArr[len(oMapArr)-1].JsonMap != nil {
			oMapArr[len(oMapArr)-1].JsonMap[new_key] = new_value
			loc := getIndex(oMapArr[len(oMapArr)-1].keys, after)
			oMapArr[len(oMapArr)-1].keys = append(append(oMapArr[len(oMapArr)-1].keys[:loc], new_key), oMapArr[len(oMapArr)-1].keys[loc:]...)
		} else {
			oMapArr[len(oMapArr)-1].keys = make([]string, 0)
			oMapArr[len(oMapArr)-1].JsonMap = make(map[string]interface{})
			oMapArr[len(oMapArr)-1].JsonMap[new_key] = new_value
			loc := getIndex(oMap.keys, after)
			oMapArr[len(oMapArr)-1].keys = append(append(oMap.keys[:loc], new_key), oMap.keys[loc:]...)
		}

		for i := len(path_arr) - 2; i >= 0; i -= 1 {
			oMapArr[i].JsonMap[path_arr[i]] = oMapArr[i+1]
		}
	}

	return oMapArr[0], err
}

/*
func main() {
	var oMap OrderedMap
	file_byte, err := os.ReadFile("simple.json")
	if err != nil {
		fmt.Printf("Error reading in json file: %s", err)
	}
	var oMap2 OrderedMap
	file_byte2, err := os.ReadFile("simple2.json")
	if err != nil {
		fmt.Printf("Error reading in json file: %s", err)
	}
	unmarshaled_data, err := Unmarshal(file_byte)
	oMap = unmarshaled_data.(OrderedMap)
	oMap_out, _ := AddKeyValAfter(oMap, "field4.objectfield2", "key2_5", "5")
	byte_out := Marshal(oMap_out)
	fmt.Println(string(formatJson(byte_out)))
}
*/
