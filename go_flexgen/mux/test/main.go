// Document utilization of
package main

import (
	"fims"
	"fmt"

	"github.com/flexgen-power/hybridos/go_flexgen/mux"
)

func main() {

	// Set up a new router.
	r := mux.NewRouter()

	// This assigns a new route with given URI and
	// provided handler then stores the route in the router.
	r.HandleFunc("/test/router", TestHandler)

	// Construct a new FIMS test message.
	msg := &fims.FimsMsg{Method: "GET", Uri: "/test/router"}

	// Handle a retrieved FIMS message.
	if err := r.Serve(msg); err != nil {
		fmt.Printf("processing msg: %v", err)
	}
}

// Developer defined function to be handled for a given endpoint.
func TestHandler(msg *fims.FimsMsg) error {
	fmt.Printf("Hello, I'm a router test handler. \n")

	// Functional code here.

	return nil
}
