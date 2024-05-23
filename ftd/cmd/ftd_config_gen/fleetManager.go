package main

import (
	"fmt"
	"path"

	"github.com/flexgen-power/hybridos/ftd/pkg/ftd"
)

func generateFleetmanagerFTDConfig(urisToFormats map[string]string) (cfg ftd.Config, report []string) {
	report = []string{}

	// setup configurators that we might use
	configurators := map[string]*uriConfigurator{
		"sites": {
			required:                true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/sites",
				Measurement:   "sites",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"fleet_features_ercotAs": {
			useRawUriConfigTemplate: true,
			warm:                    true,
			fields:                  getFMFleetFeaturesERCOTASFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/fleet/features/ercotAs",
				Measurement:   "fleet_features_ercotAs",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"fleet_sites": {
			required:                true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/fleet/sites",
				Measurement:   "fleet_sites",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"scheduler_connected": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/scheduler",
				Measurement:   "scheduler_connected",
				Sources:       []string{"connected"},
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"scheduler_timezones": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/scheduler",
				Measurement:   "scheduler_timezones",
				Sources:       []string{"timezones"},
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components": {
			useRawUriConfigTemplate: true,
			warm:                    true,
			fields:                  getFMComponentsFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"metrics": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/metrics",
				Measurement:   "metrics",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"cops_stats": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/cops/stats",
				Measurement:   "cops_stats",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"cops_summary": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/cops/summary",
				Measurement:   "cops_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"fleet_features_ercotAs_sets": {
			useRawUriConfigTemplate: true,
			warm:                    true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/fleet/features/ercotAs",
				Measurement:   "fleet_features_ercotAs_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
	}
	addOverwatchConfigurators(&configurators)
	addEventsConfigurator(&configurators)

	// iterate through uris and add them to corresponding configurators if necessary
	for uri := range urisToFormats {
		baseUri := path.Dir(uri)
		source := path.Base(uri)
		switch baseUri {
		case "/sites":
			configurators["sites"].activated = true
		case "/fleet/features/ercotAs":
			configurators["fleet_features_ercotAs"].activated = true
			configurators["fleet_features_ercotAs_sets"].activated = true
		case "/fleet/sites":
			configurators["fleet_sites"].activated = true
		case "/scheduler":
			if source == "connected" {
				configurators["scheduler_connected"].activated = true
			} else if source == "timezones" {
				configurators["scheduler_timezones"].activated = true
			} else {
				report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
			}
		case "/components":
			configurators["components"].activated = true
		case "/metrics":
			configurators["metrics"].activated = true
		case "/cops/stats":
			configurators["cops_stats"].activated = true
		case "/cops":
			if source == "summary" {
				configurators["cops_summary"].activated = true
			} else {
				report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
			}
		case "/systemstats":
			// nothing to do since overwatch pub capture will always be added to the configuration
		case "/events":
			// skip configuring events pub uris
		default:
			report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
		}
	}

	cfg, configuratorReport := generateConfigFromConfigurators(configurators)
	report = append(report, configuratorReport...)
	return cfg, report
}

// Constant list of fields added to fleet features ercotAs uri configurator
func getFMFleetFeaturesERCOTASFields() []string {
	return []string{
		// listed in datapoints list under table site_name
		"baseload_cmd_mw",
	}
}

// Constant list of fields added to components uri configurator
func getFMComponentsFields() []string {
	return []string{
		// listed in datapoints list under table site_name
		"gen_soc",
		"gen_basepoint_deviation",
		"gen_basepoint_feedback",
		"gen_gross_mw",
		"gen_lmp",
		"load_basepoint_deviation",
		"load_basepoint_feedback",
	}
}
