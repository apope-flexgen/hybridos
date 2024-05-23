package main

import (
	"encoding/json"
	"fims"
	"flag"
	"fmt"
	"log"
	"os"
	"strings"
	"time"

	"github.com/flexgen-power/hybridos/ftd/pkg/ftd"
)

// The type of server we're creating ftd configuration for
type serverType uint

const (
	siteControllerServer serverType = iota
	essControllerServer
	fleetManagerServer
)

// Type of server we are generating configuration for
var server serverType

// Output path for configuration file
var outputPath string

// Flag for printing uri formats instead of generating a config
var listenMode bool

func main() {
	parseFlags()

	// setup fims
	fimsChan, err := listenToFimsRootPubs()
	if err != nil {
		log.Fatalf("Failed to listen for fims pubs: %v", err)
	}

	// listen to messages for a given amount of time
	fmt.Println("Listening to fims traffic for 1 minute...")
	urisToFormats := map[string]string{}
	listeningTimeUp := time.After(time.Minute)
messageListenLoop:
	for {
		// if time is up, don't listen for messages
		select {
		case <-listeningTimeUp:
			break messageListenLoop
		default:
		}
		// otherwise, block on next message or time up
		select {
		case msg := <-fimsChan: // process incoming fims messages
			// ignore non-pub messages
			msgMethod := strings.ToLower(msg.Method)
			if msgMethod != "pub" {
				continue messageListenLoop
			}
			urisToFormats[msg.Uri] = formatOf(msg).string()
		case <-listeningTimeUp:
			break messageListenLoop
		}
	}
	fmt.Println("Done listening to fims traffic. Generating config...")

	// just print uri formats if not generating config
	if listenMode {
		fmt.Println("Listen mode active, printing uri formats instead...")
		for uri, format := range urisToFormats {
			fmt.Printf("%q: %q,\n", uri, format)
		}
		fmt.Println("Done printing uri formats.")
		return
	}

	// generate server specific config based on list of uris
	var cfg ftd.Config
	var report []string
	switch server {
	case siteControllerServer:
		cfg, report = generateSiteControllerFTDConfig(urisToFormats)
	case essControllerServer:
		cfg, report = generateESSControllerFTDConfig(urisToFormats)
	case fleetManagerServer:
		cfg, report = generateFleetmanagerFTDConfig(urisToFormats)
	}

	// create config file
	file, err := os.Create(outputPath)
	if err != nil {
		log.Fatalf("Failed to create output file: %v", err)
	}
	cfgBytes, err := json.MarshalIndent(cfg, "", "    ")
	if err != nil {
		log.Fatalf("Failed to marshal config to json: %v", err)
	}
	_, err = file.Write(cfgBytes)
	if err != nil {
		log.Fatalf("Failed to write config to file: %v", err)
	}
	fmt.Println("Config file written to " + outputPath + ".")

	// note any additional work that needs to be done
	for _, line := range report {
		fmt.Println(line)
	}
	if len(report) > 0 {
		fmt.Println("Config file was written, but the above notes may require manual configuration.")
	}
}

// Parse command line flags
func parseFlags() {
	// define flags
	var serverString string
	flag.StringVar(&serverString, "server", "", "Server type to generate configs for. The options are: site_controller, ess_controller, fleet_manager")
	flag.StringVar(&outputPath, "output", "/usr/local/etc/config/ftd/ftd.json", "Output path of config file.")
	flag.BoolVar(&listenMode, "listen", false, "Print uri formats instead of generating any config.")
	flag.Parse()

	serverStringToServerType := map[string]serverType{
		"site_controller": siteControllerServer,
		"ess_controller":  essControllerServer,
		"fleet_manager":   fleetManagerServer,
	}
	extractedServer, ok := serverStringToServerType[serverString]
	if !ok {
		fmt.Println("Invalid server type. Valid servers are:")
		for s := range serverStringToServerType {
			fmt.Println(s)
		}
		os.Exit(-1)
	}
	server = extractedServer
}

// Returns a channel of all FIMS publishes
func listenToFimsRootPubs() (<-chan fims.FimsMsg, error) {
	fimsConn, err := fims.Connect("ftd_config_gen")
	if err != nil {
		return nil, fmt.Errorf("failed to connect to FIMS server: %w", err)
	}
	err = fimsConn.Subscribe("/")
	if err != nil {
		return nil, fmt.Errorf("failed to subscribe to root: %w", err)
	}
	fimsChan := make(chan fims.FimsMsg)
	go func() {
		defer close(fimsChan)
		fimsConn.ReceiveChannel(fimsChan)
	}()
	return fimsChan, nil
}
