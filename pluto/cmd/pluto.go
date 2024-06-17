// pluto.go sets up our fims service and handlers.
// This file also begins runtime on any business logic.
package main

import (
	"context"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/pluto/fimsapi"
	"github.com/flexgen-power/hybridos/pluto/internal/cli"
	"github.com/flexgen-power/hybridos/pluto/internal/netinfo"
)

const (
	serviceName = "pluto"
	serviceUri  = "/pluto"
)

// Entry point of our program.
func Run(ctx context.Context, args *cli.Config) error {

	// Handle commands.
	if args.Hello {
		cli.Hello()
	}

	// Start FIMS connnection for pubbing and handling.
	if !args.NoFims {
		if err := StartFimsService(ctx); err != nil {
			return fmt.Errorf("starting FIMS: %v", err)
		}
	}

	// Start pluto business logic here.

	return nil
}

// Start our fims connection entrypoint
func StartFimsService(ctx context.Context) error {
	log.Infof("Starting FIMS service...")

	// Set up our FIMS
	service := fimsapi.NewService()

	// Configure our new FIMS service object
	if err := service.Configure(serviceName, serviceUri); err != nil {
		return fmt.Errorf("configuring FIMS service: %v", err)
	}

	// Set up our FIMS routes
	registerRoutes(service)

	// Start our FIMS service
	service.ListenAndServe(ctx)

	return nil
}

// RegisterRoutes sets up the routes for the FIMS service.
// This is where you would define your routes to specific handlers.
func registerRoutes(s *fimsapi.Service) {

	// Register routes list.
	s.Router.HandleFunc("/pluto/network", netinfo.GetNetworkInfoHandler).Method("get")
	// Add more routes here.

}
