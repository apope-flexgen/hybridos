// File route.go implements logic pertaining to a given route.
package mux

import (
	"fims"
	"fmt"
)

// Route defines the endpoint in which a given URI is
// attached to a given function handler.
type Route struct {
	// Request handler for the route.
	handler fims.Handler

	// Store URI pertaining to path.
	uri string
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

// Initialize a standard error function for when a route URI is not found.
// Function will execute when serving a route with no handler attached to it.
func NotFound(msg *fims.FimsMsg) error {
	return fmt.Errorf("FIMS URI %v not found", msg.Uri)
}

// Return a handler to the NotFound function.
// Mimics http.NotFoundHandler from the Go standard library.
func NotFoundHandler() fims.Handler { return fims.HandlerFunc(NotFound) }
