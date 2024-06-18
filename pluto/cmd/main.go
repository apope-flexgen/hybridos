package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	log "github.com/flexgen-power/go_flexgen/logger"
	cli "github.com/flexgen-power/hybridos/pluto/internal/cli"
)

func init() {
	// Initialize custom logger configuration.
	err := log.InitConfig("pluto").Init("pluto")
	if err != nil {
		fmt.Printf("Pluto log initialization failed with error: %v\n", err)
		os.Exit(-1)
	}
}

func main() {
	// Setup context with cancel.
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// Set up signal handlers.
	signalChan := make(chan os.Signal, 1)
	signal.Notify(signalChan, os.Interrupt, syscall.SIGINT, syscall.SIGTERM)

	// Sets up a non-blocking goroutine to listen to cancellations.
	go func() {
		<-signalChan
		log.Infof("Recieved interrupt - shutting down...")
		cancel()
	}()

	// Parse the command line arguments.
	args, err := cli.Parse(os.Args[1:]...)
	if err != nil {
		log.Fatalf("Error parsing cli args: %v.", err)
	}

	// Execute pluto.
	if err := Run(ctx, args); err != nil {
		log.Fatalf("Runtime error: %v", err)
	}
}

// Entry point of our program.
func Run(ctx context.Context, args *cli.Config) error {

	// Handle commands.
	if args.Hello {
		cli.Hello()
	}

	// Start server here.

	// Start pluto business logic here.

	return nil
}
