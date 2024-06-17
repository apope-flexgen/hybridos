// File route.go implements logic pertaining to a given route.
package mux

import (
	"fmt"

	fims "github.com/flexgen-power/hybridos/fims/go_fims"
)

// Route defines the endpoint in which a given URI is
// attached to a given function handler.
type Route struct {
	// Request handler for the route.
	handler fims.Handler

	// Store URI pertaining to path.
	uri string

	// Store the FIMS method (GET, SET).
	method string
}

// Handler sets a handler for the route.
func (r *Route) Handler(handler fims.Handler) *Route {
	r.handler = handler
	return r
}

// HandlerFunc sets a handler function for the route.
func (r *Route) HandlerFunc(f func(*fims.FimsMsg) error) *Route {
	return r.Handler(fims.HandlerFunc(f))
}

// Methods sets the allowed FIMS methods for the route.
func (r *Route) Method(method string) *Route {
	r.method = method
	return r
}

// Initialize a standard error function for when a route URI is not found.
// Function will execute when serving a route with no handler attached to it.
func NotFound(msg *fims.FimsMsg) error {
	return fmt.Errorf("FIMS URI %v not found", msg.Uri)
}

// Initialize a standard error function for when a route method does not match the FIMS method.
func InvalidMethod(msg *fims.FimsMsg) error {
	return fmt.Errorf("FIMS method %v for URI %v does not match defined route method for URI", msg.Method, msg.Uri)
}

// Return a handler to the NotFound function.
// Mimics http.NotFoundHandler from the Go standard library.
func NotFoundHandler() fims.Handler { return fims.HandlerFunc(NotFound) }

// Return a handler to the InvalidMethod function.
func InvalidMethodHandler() fims.Handler { return fims.HandlerFunc(InvalidMethod) }
