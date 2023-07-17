package main

import (
    "encoding/json"
//    "log"
//    "os"
    "fmt"
)



func main() {

	b := []byte(`{"Name":"Wednesday","Age":6,"Parents":["Gomez","Morticia"]}`)

	//Without knowing this data's structure, we can decode it into an interface{} value with Unmarshal:

	var f interface{}
	//err := 
        json.Unmarshal(b, &f)

	//At this point the Go value in f would be a map whose keys are strings and whose values are themselves stored as empty interface values:

//	f = map[string]interface{}{
//    		"Name": "Wednesday",
//    		"Age":  6,
//    		"Parents": []interface{}{
//        		"Gomez",
//        		"Morticia",
//    		},
//	}
//To access this data we can use a type assertion to access f's underlying map[string]interface{}:

	m := f.(map[string]interface{})
//We can then iterate through the map with a range statement and use a type switch to access its values as their concrete types:

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
    		default:
        		fmt.Println(k, "is of a type I don't know how to handle")
    		}
	}

}
