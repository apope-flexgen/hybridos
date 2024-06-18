// File mux.go implements router logic for routing FIMS
// messages to developer defined function handlers.
package mux

import (
	"fims"
	"fmt"
)

// Router provides the engine framework of tracking and
// routing FIMS messages to their attached function handlers.
// Only one router instance is necessary for a given service processing FIMS.
type Router struct {
	// Handler for replying to FIMS message with error message.
	// Used for handling errors with processing messages themselves.
	ErrorHandler ErrorHandler

	// Configurable handler to be used when no route matches.
	NotFound fims.Handler

	// Routes mapped by their URI.
	routeMap map[string]*Route
}

// Generate your own error function using this type.
// Cater error returns to a specific URI, provide a fatal error
// return interface for propagating errors.
type ErrorHandler func(fims.FimsMsg, error)

// NewRouter returns a new router instance.
func NewRouter() *Router {
	return &Router{routeMap: make(map[string]*Route)}
}

// HandleFunc registers a new route with a matcher for the URI path.
func (r *Router) HandleFunc(path string, f func(*fims.FimsMsg) error) *Route {
	return r.NewRoute(path).HandlerFunc(f)
}

// Register an empty route with provided path (URI).
func (r *Router) NewRoute(path string) *Route {
	// Initialize a route new route with a defined path (a URI).
	route := &Route{uri: path}
	r.routeMap[path] = route
	return route
}

// Route an incoming FIMS message and execute its handler function.
func (r *Router) Serve(msg *fims.FimsMsg) error {
	var handler fims.Handler

	// Search for a given route attached to a FIMS message URI.
	route, found := r.routeMap[msg.Uri]
	if found {
		handler = route.handler
	}

	// Verify a handler exists before execution.
	if handler == nil {
		handler = NotFoundHandler()
	}

	// Execute handler for FIMS.
	if err := handler.ServeFIMS(msg); err != nil {
		return fmt.Errorf("error serving msg: %v", err)
	}

	return nil
}
