package main

import (
	"errors"
	"fims"
	"fmt"
	"strings"
)

// featureStruct contains a field for each type of feature
type featureStruct struct {
	caisoAds *caisoAdsFeature
	ercotAs  ercotAsFeature
}

// The master object for all features.
var features featureStruct = featureStruct{
	caisoAds: nil,
	ercotAs: ercotAsFeature{},
}

// Configures all features
func parseAllFeatures(body interface{}) (*featureStruct, error) {
	// msg to this endpoint should look like {"features": {...}} or {...}. extract the {...} which should contain a map of all features
	msgBody, ok := body.(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("expected FIMS msg body to be type map[string]interface{}. Type of received FIMS msg body is %T", body)
	}
	// features can either be a standalone object or wrapped with the "features" string
	var featuresConfig map[string]interface{}
	if featuresConfigInterface, ok := msgBody["features"]; ok {
		featuresConfig, ok = featuresConfigInterface.(map[string]interface{})
		if !ok {
			return nil, fmt.Errorf("expected a map[string]interface{} from 'features' map entry. Type of received is %T", featuresConfig)
		}
	} else {
		featuresConfig = msgBody
	}

	// make a temporary featureStruct that can easily be discarded if errors are encountered before overwriting the actual features
	newFeatures := featureStruct{
		ercotAs: ercotAsFeature{},
	}
	// iterate through each feature and configure it with the settings found in received feature config settings map
	for featureId, featureObj := range featuresConfig {
		switch featureId {
		case caisoAdsId:
			newCaiso, err := parseCaisoAdsFeature(featureObj)
			if err != nil {
				return nil, fmt.Errorf("failed to parse CAISO ADS feature: %w", err)
			}
			newFeatures.caisoAds = newCaiso
		case ercotAsId:
			err := newFeatures.ercotAs.addMultipleSites(featureObj)
			if err != nil {
				return nil, fmt.Errorf("failed to parse ERCOT AS feature: %w", err)
			}
		default:
			return nil, fmt.Errorf("no feature with the id %s is supported", featureId)
		}
	}
	return &newFeatures, nil
}

// Endpoint for SETs to /fleet/features that handles parsing the new feature data, overwriting the existing
// feature data, and backing up the new feature data to DBI.
func replaceFeatures(newFeaturesObj interface{}) error {
	newFeatures, err := parseAllFeatures(newFeaturesObj)
	if err != nil {
		return fmt.Errorf("failed to parse features: %w", err)
	}
	features = *newFeatures
	backupFeatures()
	return nil
}

// Endpoint for any SETs to a URI beginning in /fleet/features.
func handleFeaturesSet(msg fims.FimsMsg) error {
	// /fleet/features
	if msg.Nfrags < 3 {
		err := replaceFeatures(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to handle full features SET: %w", err)
		}
		return nil
	}

	switch msg.Frags[2] {
	case caisoAdsId: // /fleet/features/caisoAds
		err := handleCaisoSet(msg)
		if err != nil {
			return fmt.Errorf("failed to handle CAISO ADS SET: %w", err)
		}
	case ercotAsId: // /fleet/features/ercotAs
		err := features.ercotAs.handleSet(msg)
		if err != nil {
			return fmt.Errorf("failed to handle ERCOT AS SET: %w", err)
		}
	default:
		return fmt.Errorf("URI is not valid SET endpoint; does not correspond to any supported features")
	}
	return nil
}

// Endpoint for any GETs to a URI beginning in /fleet/features.
func handleFeaturesGet(msg fims.FimsMsg) error {
	switch {
	//sends all features
	case msg.Uri == "/fleet/features":
		return f.SendSet(msg.Replyto, "", features.buildObj())
	//ERCOT Ancillary Services specific
	case strings.HasPrefix(msg.Uri, "/fleet/features/ercotAs"):
		return features.ercotAs.handleGet(msg)
	// CAISO specific
	case msg.Uri == "/fleet/features/caisoAds/latestBatch":
		return sendLatestBatchResponse(msg.Replyto)
	default:
		return fmt.Errorf("URI is not valid GET endpoint; does not correspond to any supported features")
	}
}

// Endpoint for any POSTs to a URI beginning in /fleet/features.
func handleFeaturesPost(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		return errors.New("invalid URI")
	}

	switch msg.Frags[2] {
	case ercotAsId:
		err := features.ercotAs.handlePost(msg)
		if err != nil {
			return fmt.Errorf("failed to handle ERCOT AS POST: %w", err)
		}
	default:
		return fmt.Errorf("URI ending in %s is not a valid features POST endpoint", msg.Frags[2])
	}
	return nil
}

// buildObj builds a map of all features for easy JSON sending.
func (fStruct featureStruct) buildObj() map[string]interface{} {
	obj := make(map[string]interface{})
	if fStruct.caisoAds != nil {
		obj[caisoAdsId] = fStruct.caisoAds.buildObj()
	}
	obj[ercotAsId] = fStruct.ercotAs.buildObj()
	return obj
}

func (fStruct *featureStruct) update() {
	fStruct.ercotAs.update()
}

func publishFeatures() {
	if features.caisoAds != nil {
		features.caisoAds.publish()
	}
	features.ercotAs.publish()
}
