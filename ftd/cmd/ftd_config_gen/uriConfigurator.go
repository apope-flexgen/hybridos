package main

import (
	"fmt"
	"sort"

	"github.com/flexgen-power/hybridos/ftd/pkg/ftd"
)

// Intermediary datatype between lists of uris and generated configuration
type uriConfigurator struct {
	activated                bool                // whether or not we should generate a config for this uri
	required                 bool                // true if configurator config is required
	warm                     bool                // true if uri should be configured in the warm data lane as well as the cold data lane
	fields                   []string            // fields which should be filtered into the warm data lane
	bitStringFields          []string            // names of bitstring fields we would want to configure
	uriConfigTemplate        ftd.UriConfig       // template uri config to create uri configs from
	formatsToSources         map[string][]string // maps message formats to sources seen for the uri which use the format
	useRawUriConfigTemplate  bool                // true if we should use the config template as the actual configuration
	measurementForEachSource bool                // true if we want to make a separate measurement for each source
}

func generateConfigFromConfigurators(configurators map[string]*uriConfigurator) (cfg ftd.Config, report []string) {
	cfg = ftd.Config{}
	report = []string{}

	// lane-wide configuration
	cfg.Lane1 = &ftd.LaneConfig{
		DbName:            "ansible_should_overwrite_db_name",
		ArchivePeriod:     300,
		ArchivePath:       "/home/hybridos/powercloud/archives_cold",
		Parquet:           true,
		NumArchiveWorkers: 1,
		Uris:              []ftd.UriConfig{},
	}
	cfg.Lane2 = &ftd.LaneConfig{
		DbName:            "ansible_should_overwrite_db_name",
		ArchivePeriod:     60,
		ArchivePath:       "/home/hybridos/powercloud/archives_warm",
		Parquet:           true,
		NumArchiveWorkers: 1,
		Uris:              []ftd.UriConfig{},
	}

	// uri-dependent configuration
	for configuratorName, configurator := range configurators {
		if !configurator.activated && configurator.required {
			// configurator was expected to be activated but was not
			report = append(report, fmt.Sprintf("Did not see the expected fims traffic for %s uri config", configuratorName))
		} else if configurator.activated && configurator.useRawUriConfigTemplate {
			// activated configurator uses the raw uri config template
			uriCfg := configurator.uriConfigTemplate
			addUriConfig(&cfg, uriCfg, configurator)
		} else if configurator.activated && !configurator.useRawUriConfigTemplate {
			// activated configurator templates the uri config
			if configurator.measurementForEachSource {
				// if making a measurement for each source, do so and create separate entries for each
				for _, sources := range configurator.formatsToSources {
					for _, source := range sources {
						uriCfg := configurator.uriConfigTemplate
						uriCfg.Sources = []string{source}
						uriCfg.Measurement += "_" + source
						addUriConfig(&cfg, uriCfg, configurator)
					}
				}
			} else if configurator.uriConfigTemplate.Group != "" {
				// else if grouping is used, make a group for each format seen and separate entries for each group
				i := 0
				for _, sources := range configurator.formatsToSources {
					uriCfg := configurator.uriConfigTemplate
					uriCfg.Group += fmt.Sprintf("_%d", i)
					uriCfg.Sources = sources
					addUriConfig(&cfg, uriCfg, configurator)
					i++
				}
			} else {
				// else, we can just make a single config entry for all sources
				flatListSources := []string{}
				for _, sources := range configurator.formatsToSources {
					flatListSources = append(flatListSources, sources...)
				}
				uriCfg := configurator.uriConfigTemplate
				uriCfg.Sources = flatListSources
				addUriConfig(&cfg, uriCfg, configurator)
			}
			for _, bitStringField := range configurator.bitStringFields {
				// currently there is no easy automatic way to generate bitstring field configuration
				report = append(report, fmt.Sprintf("Bitstring configuration must be done manually for field %s for %s uri config", bitStringField, configuratorName))
			}
		}
	}

	// sort sources
	for _, uriCfg := range cfg.Lane1.Uris {
		sort.Strings(uriCfg.Sources)
	}
	for _, uriCfg := range cfg.Lane2.Uris {
		sort.Strings(uriCfg.Sources)
	}
	// sort uris
	sort.Slice(cfg.Lane1.Uris, func(i int, j int) bool {
		return cfg.Lane1.Uris[i].BaseUri < cfg.Lane1.Uris[j].BaseUri
	})
	sort.Slice(cfg.Lane2.Uris, func(i int, j int) bool {
		return cfg.Lane2.Uris[i].BaseUri < cfg.Lane2.Uris[j].BaseUri
	})

	return cfg, report
}

// Adds a source with the given format to the uri configurator and activates the configurator,
// handles creation of configurator source tracking data structures
func (configurator *uriConfigurator) addSourceAndActivate(source string, format string) {
	configurator.activated = true
	if configurator.formatsToSources == nil {
		configurator.formatsToSources = map[string][]string{}
	}
	sources := configurator.formatsToSources[format]
	if sources == nil {
		sources = []string{}
	}
	sources = append(sources, source)
	configurator.formatsToSources[format] = sources
}

// Adds configurators for capturing overwatch publishes to the given configurators
func addOverwatchConfigurators(configurators *map[string]*uriConfigurator) {
	(*configurators)["cpustats"] = &uriConfigurator{
		activated:               true,
		warm:                    true,
		useRawUriConfigTemplate: true,
		fields: []string{
			"uptimesec",
			"loadavg_5m",
		},
		uriConfigTemplate: ftd.UriConfig{
			BaseUri:       "/systemstats/cpu",
			Measurement:   "cpustats",
			Method:        []string{"pub"},
			DestinationDb: "influx",
		},
	}
	(*configurators)["memstats"] = &uriConfigurator{
		activated:               true,
		warm:                    true,
		useRawUriConfigTemplate: true,
		fields: []string{
			"activeKB",
			"freeKB",
		},
		uriConfigTemplate: ftd.UriConfig{
			BaseUri:       "/systemstats/mem",
			Measurement:   "memstats",
			Method:        []string{"pub"},
			DestinationDb: "influx",
		},
	}
	(*configurators)["netstats"] = &uriConfigurator{
		activated:               true,
		warm:                    true,
		useRawUriConfigTemplate: true,
		fields: []string{
			"fims_status",
		},
		uriConfigTemplate: ftd.UriConfig{
			BaseUri:       "/systemstats/net",
			Measurement:   "netstats",
			Method:        []string{"pub"},
			DestinationDb: "influx",
		},
	}
	(*configurators)["diskstats"] = &uriConfigurator{
		activated:               true,
		warm:                    true,
		useRawUriConfigTemplate: true,
		uriConfigTemplate: ftd.UriConfig{
			BaseUri:       "/systemstats/disk",
			Measurement:   "diskstats",
			Method:        []string{"pub"},
			DestinationDb: "influx",
		},
	}
	(*configurators)["processstats"] = &uriConfigurator{
		activated: true,
		// process stats is not warm
		useRawUriConfigTemplate: true,
		uriConfigTemplate: ftd.UriConfig{
			BaseUri:       "/systemstats/process",
			Measurement:   "processstats",
			Method:        []string{"pub"},
			DestinationDb: "influx",
		},
	}
}

// Adds configurators for capturing events posts to the given configurators
func addEventsConfigurator(configurators *map[string]*uriConfigurator) {
	(*configurators)["events"] = &uriConfigurator{
		activated:               true,
		warm:                    true,
		useRawUriConfigTemplate: true,
		uriConfigTemplate: ftd.UriConfig{
			BaseUri:       "/events",
			Measurement:   "events",
			Sources:       []string{""},
			Method:        []string{"post"},
			DestinationDb: "mongo",
		},
	}
}

// Add uri config to appropriate lanes of the given root config
func addUriConfig(cfg *ftd.Config, uriCfg ftd.UriConfig, configurator *uriConfigurator) {
	cfg.Lane1.Uris = append(cfg.Lane1.Uris, uriCfg)
	if configurator.warm {
		uriCfg.Fields = configurator.fields
		cfg.Lane2.Uris = append(cfg.Lane2.Uris, uriCfg)
	}
}
