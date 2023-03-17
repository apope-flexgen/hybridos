// Package implements json login for parsing configuration files easier
package cfg

/* Function to form an echo.json file on:
*  list of register to echo at that uri,
* use list of registers for that uri
 */

// Handles a new echo.json input, generating a new interceptor, and populating the global datamap
func UpdateConfig() error {

	return nil
}

// Retrieve our template file and file the output uri
// func GetEchoFile(t string) (Map, error) {

// 	// Populate our echo map with its template
// 	m, err := GetFileData(t)
// 	if err != nil {
// 		return nil, fmt.Errorf("error getting template bytes: %v", err)
// 	}

// 	// Verify the echo.json file
// 	if err := verifyEchoJSON(m); err != nil {
// 		return nil, fmt.Errorf("error validating echo.json file: %v", err)
// 	}

// 	return m, nil
// }

// Verify that a provided echo.json file is properly formatted
// func verifyEchoJSON(m Map) error {

// 	// Verify we have an array of outputs
// 	if _, ok := m["outputs"]; !ok {
// 		return fmt.Errorf("echo.json file does not contain an outputs map")
// 	}

// 	return nil
// }
