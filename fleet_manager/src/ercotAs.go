package main

import (
	"errors"
	"fims"
	"fmt"
	"math"
	"strings"

	fg "github.com/flexgen-power/go_flexgen"
	log "github.com/flexgen-power/go_flexgen/logger"
)

// fleet points struct type stores the Fleet Manager's status points (variables representing the state of various attributes of the sender) and
// control points (are variables that are responsible for asserting values that will change the status of the receiver)
type fleetPoints struct {
	// controls

	scedBasepoint        genLoadMuxFloat             // gets updated by ERCOT every 5 minutes via their Security-Constrained Economic Dispatch (SCED) algorithm
	regulationDown       genLoadParticipatingService // gets updated by ERCOT every 4 seconds with possible responsibility to decrease power due to high grid frequency
	regulationUp         genLoadParticipatingService // gets updated by ERCOT every 4 seconds with possible responsibility to increase power due to low grid frequency
	frrsDown             genLoadParticipatingService // settings to enable/track/trigger 1-second over-frequency responses. fed through inactive cmd so if Site Controller auto-detects freq event, it will take active control
	frrsUp               genLoadParticipatingService // settings to enable/track/trigger 1-second under-frequency responses. fed through inactive cmd so if Site Controller auto-detects freq event, it will take active control
	resourceStatus       genLoadMuxInt               // the gen register of resourceStatus gets updated with a control word from ERCOT indicating whether RRS-FFR should be enabled or not
	responsiveReservePfr genLoadServiceNoMux         // settings to enable and track RRS-PFR responses on Site Controller that are automatically triggered by on-site frequency deviations
	responsiveReserveFfr genLoadServiceNoMux         // settings to enable and track RRS-FFR responses on Site Controller that are automatically triggered by on-site frequency deviations
	updatedBasepoint     genLoadMuxFloat             // site's raw power command which gets updated by ERCOT every 5 seconds after they run their security-constraind economic dispatch (SCED) algorithm

	// statuses

	updatedBasepointCompensatedMw float64 // ERCOT's SCED basepoint with RRS-FFR requirement subtracted out (only when RRS-FFR is enabled). see comments in calculateBaseloadCmd func for details
	baseloadCmdMw                 float64 // baseload MW cmd before limitation by defined range

	// constants
	baseloadCmdMinLimitMw float64
	baseloadCmdMaxLimitMw float64
	inactiveCmdMinLimitMw float64
	inactiveCmdMaxLimitMw float64

	// site status monitoring
	loadPseudoSwitchStatus muxBool
	genMaxCharge           muxFloat
	genMaxDischarge        muxFloat

	// all below variables are not used internally and are just endpoints that are required to be present and tracked
	emergencyDownRamp   genLoadMuxFloat
	emergencyUpRamp     genLoadMuxFloat
	nonSpin             genLoadScheduledService
	normalDownRamp      genLoadMuxFloat
	normalUpRamp        genLoadMuxFloat
	overrideRequestFlag bool // an endpoint set by the site owner and read by ERCOT. site owner sets to true when it wishes to take over manual control of the site
	overrideAllowedFlag bool // an endpoint set by ERCOT and read by the site owner. ERCOT sets to true when it sees site owner's override request and acknowledges site owner can have control
	calcRursGen         int
	calcRupfGen         int
	calcRdrsGen         int
	calcRdpfGen         int
	calcRursLoad        int
	calcRupfLoad        int
	calcRdrsLoad        int
	calcRdpfLoad        int
}

// the ercotAsFeature maps sites to their own fleetPoints struct
type ercotAsFeature map[string]*fleetPoints

// ercotAsId is the string that identifies the ERCOT Ancillary Services feature in all JSONs, maps, etc.
const ercotAsId string = "ercotAs"

// each site has a bit field where each bit represents a type of frequency response.
// a bit is 1 if the response is enabled, or 0 if the response is disabled. multiple responses can be enabled at once.
// these constants are the integer values of each response's bit at its respective position in the bit field.
const (
	frrsUpEnable      int = 1  // FRRS-Up is bit 0
	frrsDownEnable    int = 2  // FRRS-Down is bit 1
	rrsFfrEnable      int = 4  // RRS-FFR is bit 2
	autoPfrUpEnable   int = 8  // Auto-PFR Up is bit 3
	autoPfrDownEnable int = 16 // Auto-PFR Down is bit 4
)

// This is the value that ERCOT will send to the Resource Status register when it wants the RRS-FFR response to be enabled.
const ffrControlWord int = 21

// Called periodically to iterate over all ERCOT sites and update the output variables based on input settings.
func (ercotAs ercotAsFeature) update() {
	for siteId, siteFeatureSettings := range ercotAs {
		ercotSite, ok := fleet[siteId]
		if !ok {
			log.Errorf("ERCOT AS site %s not found in fleet.", siteId)
			continue
		}
		ercotSite.recalculateErcotFeatureVariables(siteFeatureSettings)
	}
}

// Uses the latest values received from ERCOT and/or overriding user to update the control
// values being sent to Site Controller such as the response enable bit field, the baseload
// command, and the inactive response command.
func (s *site) recalculateErcotFeatureVariables(fPoints *fleetPoints) {
	// update the feature variables that are sourced from the site but may have customer overrides
	fPoints.loadPseudoSwitchStatus.actual = s.breakerClosed.status.(bool)
	fPoints.genMaxCharge.actual = s.chargeablePower.status.(float64)
	fPoints.genMaxDischarge.actual = s.dischargeablePower.status.(float64)

	// a bit field indicates which types of frequency responses are enabled for the site.
	// update its value based on the current settings for the site
	newResponseEnableBitField := calculateResponseEnableBitField(fPoints)
	s.freqResp.responseEnableBitField.control = newResponseEnableBitField

	// only follow the RRS-FFR requirement if RRS-FFR is enabled
	ffrResponsibility := 0.0
	ffrRequirement := 0.0
	if newResponseEnableBitField&rrsFfrEnable != 0 {
		ffrResponsibility = fPoints.responsiveReserveFfr.genResponsibility + fPoints.responsiveReserveFfr.loadResponsibility
		ffrRequirement = fPoints.responsiveReserveFfr.genRequirement + fPoints.responsiveReserveFfr.loadRequirement
	}

	// calculate the site's baseload command, the value that is the starting point to/from which any freq responses add/subtract
	s.freqResp.baseloadCmd.control = fPoints.calculateBaseloadCmd(ffrRequirement)

	// calculate the site's inactive FFR response command and inactive FRRS response commands,
	// AKA the values that are added to baseload when their respective auto-responses are not active.
	// FFR's inactive cmd uses ERCOT's FFR requirement that allows ERCOT to ramp down a site's output after the active response has ended.
	// FRRS Up/Down each use an inactive command so that Site Controller's auto FRRS can trigger, or FM can send ERCOT's command, but not both at the same time
	s.freqResp.inactiveFfrCmd.control = math.Min(math.Max(ffrRequirement, fPoints.inactiveCmdMinLimitMw), fPoints.inactiveCmdMaxLimitMw)
	combinedFrrsUpRequirement := fPoints.frrsUp.gen.requirement.getSelect() + fPoints.frrsUp.load.requirement.getSelect()
	s.freqResp.inactiveFrrsUpCmd.control = math.Min(math.Max(combinedFrrsUpRequirement, fPoints.inactiveCmdMinLimitMw), fPoints.inactiveCmdMaxLimitMw)
	combinedFrrsDownRequirement := fPoints.frrsDown.gen.requirement.getSelect() + fPoints.frrsDown.load.requirement.getSelect()
	s.freqResp.inactiveFrrsDownCmd.control = math.Abs(math.Min(math.Max(-1*combinedFrrsDownRequirement, fPoints.inactiveCmdMinLimitMw), fPoints.inactiveCmdMaxLimitMw))

	// update the site's responsibilities
	s.freqResp.frrsUpCmd.control = fPoints.frrsUp.gen.responsibility.getSelect() + fPoints.frrsUp.load.responsibility.getSelect()
	s.freqResp.frrsDownCmd.control = fPoints.frrsDown.gen.responsibility.getSelect() + fPoints.frrsDown.load.responsibility.getSelect()
	s.freqResp.rrsFfrCmd.control = ffrResponsibility
	s.freqResp.autoPfrDownCmd.control = fPoints.calculateAutoPfrDownCmd(s)
	s.freqResp.autoPfrUpCmd.control = fPoints.calculateAutoPfrUpCmd(s)
}

// Starts with a response-enable bit field that has all bits set to 0. Sets any bits to 1 whose responses
// should be enabled based on the enable logic for the response. Returns the final bit field with any
// necessary response-enable bits raised.
func calculateResponseEnableBitField(fPoints *fleetPoints) (responseEnableBitField int) {
	// FFRS-Up should be enabled if its total gen+load responsibility is positive
	totalFrrsUpResponsibility := fPoints.frrsUp.gen.responsibility.getSelect() + fPoints.frrsUp.load.responsibility.getSelect()
	if totalFrrsUpResponsibility > 0 {
		responseEnableBitField = responseEnableBitField | frrsUpEnable
	}

	// FRRS-Down should be enabled if its total gen_load responsibility is positive
	totalFrrsDownResponsibility := fPoints.frrsDown.gen.responsibility.getSelect() + fPoints.frrsDown.load.responsibility.getSelect()
	if totalFrrsDownResponsibility > 0 {
		responseEnableBitField = responseEnableBitField | frrsDownEnable
	}

	// RRS-FFR should be enabled if ERCOT has sent RRS-FFR's control word to the Resource Status register
	if fPoints.resourceStatus.gen.getSelect() == ffrControlWord {
		responseEnableBitField = responseEnableBitField | rrsFfrEnable
	}

	// Auto PFR should always be enabled
	responseEnableBitField = responseEnableBitField | autoPfrUpEnable | autoPfrDownEnable

	return responseEnableBitField
}

// Calculates a site's baseload command based on current ERCOT requests for SCED output (5-minute window) and Regulation Services output (4-second window).
// When an RRS-FFR frequency deviation event is first detected by ERCOT, ERCOT will begin to include the FFR requirement in their SCED basepoint; however, the site
// will not be expected to follow the SCED basepoint while the FFR requirement is included since that would result in the site outputting double its RRS-FFR
// responsibility (1x in the SCED basepoint, 1x in the site's automated response during the event, or in the inactive command immediately after the event).
//
// While the FFR requirement is included in the SCED basepoint, this function will subtract it out so that the rest of the basepoint is still followed.
// The function for calculating the site's inactive response command will, at the same time, add in the FFR requirement so that when the site's automated
// output ends and the cooldown stage is entered (where the frequency deviation event is over but ERCOT requires a ramp-down that it controls via the FFR
// requirement endpoint), ERCOT's ramp-down will be followed by the site via the inactive response.
func (fPoints *fleetPoints) calculateBaseloadCmd(ffrRequirement float64) (baseloadCmd float64) {
	fPoints.updatedBasepointCompensatedMw = fPoints.updatedBasepoint.gen.getSelect() - fPoints.updatedBasepoint.load.getSelect() - ffrRequirement

	regUpCombinedRequirement := fPoints.regulationUp.load.requirement.getSelect() + fPoints.regulationUp.gen.requirement.getSelect()
	regDownCombinedRequirement := fPoints.regulationDown.load.requirement.getSelect() + fPoints.regulationDown.gen.requirement.getSelect()

	fPoints.baseloadCmdMw = fPoints.updatedBasepointCompensatedMw + regUpCombinedRequirement - regDownCombinedRequirement
	return math.Min(math.Max(fPoints.baseloadCmdMw, fPoints.baseloadCmdMinLimitMw), fPoints.baseloadCmdMaxLimitMw)
}

// Calculates a site's automatic PFR over-frequency command based on the site's chargeable power
func (fPoints *fleetPoints) calculateAutoPfrDownCmd(s *site) (autoPfrDownCmd float64) {
	siteMaxChargeLimit := fPoints.genMaxCharge.getSelect()
	return math.Max(
		siteMaxChargeLimit,
		0,
	)
}

// Calculates a site's automatic PFR under-frequency command based on the minimum value of site's dischargeable power and the site's dischargeable power
// minus the RRS-FFR responsibility
func (fPoints *fleetPoints) calculateAutoPfrUpCmd(s *site) (autoPfrUpCmd float64) {
	siteMaxDischargeLimit := fPoints.genMaxDischarge.getSelect()
	return math.Max(
		math.Min(
			siteMaxDischargeLimit,
			siteMaxDischargeLimit-s.freqResp.rrsFfrCmd.control.(float64),
		),
		0,
	)
}

// Endpoint for any GETs to a URI beginning with /fleet/features/ercotAs.
func (ercotAs ercotAsFeature) handleGet(msg fims.FimsMsg) error {
	switch msg.Nfrags {
	case 3: // /fleet/features/ercotAs
		return f.SendSet(msg.Replyto, "", ercotAs.buildObj())
	case 4:
		if msg.Frags[3] == "sites" { // /fleet/features/ercotAs/sites
			return f.SendSet(msg.Replyto, "", ercotAs.listSites())
		}
		if msg.Frags[3] == "overridable" { // /fleet/features/ercotAs/overridable
			return f.SendSet(msg.Replyto, "", overridableErcotVars)
		}

		// /fleet/features/ercotAs/<site ID>
		siteFeatureSettings, ok := ercotAs[msg.Frags[3]]
		if !ok {
			return fmt.Errorf("could not find site %s in map of ERCOT sites", msg.Frags[3])
		}
		return f.SendSet(msg.Replyto, "", siteFeatureSettings.buildObj())
	case 5: // URI begins with /fleet/features/ercotAs/<site ID>
		siteFeatureSettings, ok := ercotAs[msg.Frags[3]]
		if !ok {
			return fmt.Errorf("could not find site %s in map of ERCOT sites", msg.Frags[3])
		}
		err := siteFeatureSettings.handleGet(msg)
		if err != nil {
			return fmt.Errorf("failed to handle GET to ERCOT feature settings for site %s: %w", msg.Frags[3], err)
		}
		return nil
	default:
		return fmt.Errorf("wrong number of URI fragments for ERCOT AS feature GET request")
	}
}

// Endpoint for any GETs to a URI beginning with /fleet/features/ercotAs/<site ID>.
func (fp *fleetPoints) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags != 5 {
		return fmt.Errorf("GET to site ERCOT AS settings requires 5 fragments but got %d", msg.Nfrags)
	}

	// /fleet/features/ercotAs/<site ID>/overridable
	if msg.Frags[4] == "overridable" {
		return f.SendSet(msg.Replyto, "", fp.buildOnlyOverridables())
	}

	// /fleet/features/ercotAs/<site ID>/<variable ID>
	varMap := fp.buildObj()
	val, ok := varMap[msg.Frags[4]]
	if !ok {
		return fmt.Errorf("did not find variable %s in ERCOT AS settings", msg.Frags[4])
	}
	return f.SendSet(msg.Replyto, "", val)
}

// Overwrites the existing ERCOT feature with the given replacement configuration.
func (ercotAs *ercotAsFeature) overwrite(cfg interface{}) error {
	newErcotFeature := ercotAsFeature{}
	err := newErcotFeature.addMultipleSites(cfg)
	if err != nil {
		return fmt.Errorf("failed to parse sites from cfg: %w", err)
	}
	*ercotAs = newErcotFeature
	ercotAs.backupToDbi()
	return nil
}

// Backs up all ERCOT settings to DBI.
func (ercotAs *ercotAsFeature) backupToDbi() {
	err := f.SendSet(fmt.Sprintf("/dbi/fleet_manager/features/features/%s", ercotAsId), "", ercotAs.buildObj())
	if err != nil {
		log.Errorf("Error backing up ERCOT feature to DBI: %v.", err)
	}
}

// Overwrites the site's ERCOT feature settings with the given replacement configuration.
func (fp *fleetPoints) overwrite(featureSettingsObj interface{}, siteId string) error {
	newFeatureSettings, err := parseFleetPoints(featureSettingsObj)
	if err != nil {
		return fmt.Errorf("failed to parse site feature settings: %w", err)
	}
	*fp = *newFeatureSettings
	fp.backupToDbi(siteId)
	return nil
}

// Backs up ERCOT feature settings for a site to DBI.
func (fp *fleetPoints) backupToDbi(siteId string) {
	err := f.SendSet(fmt.Sprintf("/dbi/fleet_manager/features/features/%s/%s", ercotAsId, siteId), "", fp.buildObj())
	if err != nil {
		log.Errorf("Error backing up ERCOT feature settings for site %s to DBI: %v.", siteId, err)
	}
}

// Updates a single ERCOT feature setting for the site.
func (fp *fleetPoints) editSetting(variableId, siteId string, newVal interface{}) (err error) {
	valueChanged := false
	switch {
	// gen & load
	case strings.HasPrefix(variableId, fp.updatedBasepoint.idRoot):
		valueChanged, err = fp.updatedBasepoint.setGenLoad(variableId[len(fp.updatedBasepoint.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.scedBasepoint.idRoot):
		valueChanged, err = fp.scedBasepoint.setGenLoad(variableId[len(fp.scedBasepoint.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.emergencyDownRamp.idRoot):
		valueChanged, err = fp.emergencyDownRamp.setGenLoad(variableId[len(fp.emergencyDownRamp.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.emergencyUpRamp.idRoot):
		valueChanged, err = fp.emergencyUpRamp.setGenLoad(variableId[len(fp.emergencyUpRamp.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.responsiveReserveFfr.idRoot):
		valueChanged, err = fp.responsiveReserveFfr.setGenLoad(variableId[len(fp.responsiveReserveFfr.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.frrsDown.idRoot):
		valueChanged, err = fp.frrsDown.setGenLoad(variableId[len(fp.frrsDown.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.frrsUp.idRoot):
		valueChanged, err = fp.frrsUp.setGenLoad(variableId[len(fp.frrsUp.idRoot):], newVal)
	case strings.HasPrefix(variableId, "gen_max_discharge_mw"):
		valueChanged, err = fp.genMaxDischarge.setField(variableId[len("gen_max_discharge_mw"):], newVal)
	case strings.HasPrefix(variableId, "gen_max_charge_mw"):
		valueChanged, err = fp.genMaxCharge.setField(variableId[len("gen_max_charge_mw"):], newVal)
	case strings.HasPrefix(variableId, "load_pseudo_switch_status"):
		newVal := fg.UnwrapVariable(newVal)
		if newBool, ok := newVal.(bool); ok {
			valueChanged, err = fp.loadPseudoSwitchStatus.setField(variableId[len("load_pseudo_switch_status"):], newBool)
		}
	case variableId == "reg_manual_override":
		newVal := fg.UnwrapVariable(newVal)
		newBool, ok := newVal.(bool)
		if !ok {
			return fmt.Errorf("expected bool but got %T", newVal)
		}
		valueChanged = fp.overrideRequestFlag != newBool
		fp.overrideRequestFlag = newBool
	case variableId == "reg_manual_override_feedback":
		newVal := fg.UnwrapVariable(newVal)
		newBool, ok := newVal.(bool)
		if !ok {
			return fmt.Errorf("expected bool but got %T", newVal)
		}
		valueChanged = fp.overrideAllowedFlag != newBool
		fp.overrideAllowedFlag = newBool
	case variableId == "calculated_rurs_gen":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRursGen != newInt
			fp.calcRursGen = newInt
		}
	case variableId == "calculated_rupf_gen":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRupfGen != newInt
			fp.calcRupfGen = newInt
		}
	case variableId == "calculated_rdrs_gen":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRdrsGen != newInt
			fp.calcRdrsGen = newInt
		}
	case variableId == "calculated_rdpf_gen":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRdpfGen != newInt
			fp.calcRdpfGen = newInt
		}
	case variableId == "calculated_rurs_load":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRursLoad != newInt
			fp.calcRursLoad = newInt
		}
	case variableId == "calculated_rupf_load":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRupfLoad != newInt
			fp.calcRupfLoad = newInt
		}
	case variableId == "calculated_rdrs_load":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRdrsLoad != newInt
			fp.calcRdrsLoad = newInt
		}
	case variableId == "calculated_rdpf_load":
		newVal := fg.UnwrapVariable(newVal)
		if newInt, err := fg.CastToInt(newVal); err != nil {
			return fmt.Errorf("failed to cast value to int: %w", err)
		} else {
			valueChanged = fp.calcRdpfLoad != newInt
			fp.calcRdpfLoad = newInt
		}
	case strings.HasPrefix(variableId, fp.nonSpin.idRoot):
		valueChanged, err = fp.nonSpin.setGenLoad(variableId[len(fp.nonSpin.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.normalDownRamp.idRoot):
		valueChanged, err = fp.normalDownRamp.setGenLoad(variableId[len(fp.normalDownRamp.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.normalUpRamp.idRoot):
		valueChanged, err = fp.normalUpRamp.setGenLoad(variableId[len(fp.normalUpRamp.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.regulationDown.idRoot):
		valueChanged, err = fp.regulationDown.setGenLoad(variableId[len(fp.regulationDown.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.regulationUp.idRoot):
		valueChanged, err = fp.regulationUp.setGenLoad(variableId[len(fp.regulationUp.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.resourceStatus.idRoot):
		valueChanged, err = fp.resourceStatus.setGenLoad(variableId[len(fp.resourceStatus.idRoot):], newVal)
	case strings.HasPrefix(variableId, fp.responsiveReservePfr.idRoot):
		valueChanged, err = fp.responsiveReservePfr.setGenLoad(variableId[len(fp.responsiveReservePfr.idRoot):], newVal)
	default:
		return fmt.Errorf("%s is not a valid endpoint", variableId)
	}
	if err != nil {
		return fmt.Errorf("failed to edit setting %s: %w", variableId, err)
	}
	if valueChanged {
		err = f.SendSet(fmt.Sprintf("/dbi/fleet_manager/features/features/%s/%s/%s", ercotAsId, siteId, variableId), "", fg.UnwrapVariable(newVal))
		if err != nil {
			log.Errorf("Error backing up ERCOT feature variable %s for site %s to DBI: %v.", variableId, siteId, err)
		}
	}
	return nil
}

// Endpoint for any SETs to a URI beginning with /fleet/features/ercotAs.
func (ercotAs *ercotAsFeature) handleSet(msg fims.FimsMsg) error {
	// /fleet/features/ercotAs
	if msg.Nfrags < 4 {
		err := ercotAs.overwrite(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to replace ERCOT AS feature: %w", err)
		}
		return nil
	}

	// SETs to URIs at or deeper than /fleet/features/ercotAs/<site ID> require that the target site already exists
	siteFeatureSettings, ok := (*ercotAs)[msg.Frags[3]]
	if !ok {
		return fmt.Errorf("did not find site %s in map of ERCOT sites", msg.Frags[3])
	}

	err := siteFeatureSettings.handleSet(msg)
	if err != nil {
		return fmt.Errorf("failed to handle SET to site %s's ERCOT feature settings: %w", msg.Frags[3], err)
	}
	return nil
}

// Endpoint for any SETs to a URI beginning with /fleet/features/ercotAs/<site ID>.
func (fp *fleetPoints) handleSet(msg fims.FimsMsg) error {
	switch msg.Nfrags {
	case 4: // /fleet/features/ercotAs/<site ID>
		err := fp.overwrite(msg.Body, msg.Frags[3])
		if err != nil {
			return fmt.Errorf("failed to overwrite ERCOT feature settings: %w", err)
		}
	case 5: // /fleet/features/ercotAs/<site ID>/<variable ID>
		err := fp.editSetting(msg.Frags[4], msg.Frags[3], msg.Body)
		if err != nil {
			return fmt.Errorf("failed to edit ERCOT feature setting %s: %w", msg.Frags[4], err)
		}
	default:
		return errors.New("invalid URI")
	}
	return nil
}

// Endpoint for any POSTs to a URI beginning with /fleet/features/ercotAs.
func (ercotAs ercotAsFeature) handlePost(msg fims.FimsMsg) error {
	// /fleet/features/ercotAs
	if msg.Nfrags == 3 {
		err := ercotAs.addMultipleSites(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to add multiple sites to the ERCOT AS feature (none were added): %w", err)
		}
		return nil
	}

	if msg.Nfrags != 4 {
		return fmt.Errorf("received POST with %d URI fragments but single-site ERCOT AS POST requires exactly 4", msg.Nfrags)
	}

	// expected URI is /fleet/features/ercotAs/<site ID>
	err := ercotAs.addSite(msg.Frags[3], msg.Body, nil)
	if err != nil {
		return fmt.Errorf("failed to add site %s to ERCOT AS feature: %w", msg.Frags[3], err)
	}
	return nil
}

// buildObj makes a map of the FM struct for easy JSON sending.
func (fp *fleetPoints) buildObj() map[string]interface{} {
	obj := make(map[string]interface{})
	// gen and load
	fp.updatedBasepoint.addToObj(obj)
	fp.scedBasepoint.addToObj(obj)
	fp.resourceStatus.addToObj(obj)
	fp.regulationUp.addToObj(obj)
	fp.regulationDown.addToObj(obj)
	fp.responsiveReservePfr.addToObj(obj)
	fp.emergencyDownRamp.addToObj(obj)
	fp.emergencyUpRamp.addToObj(obj)
	fp.responsiveReserveFfr.addToObj(obj)
	fp.frrsUp.addToObj(obj)
	fp.frrsDown.addToObj(obj)
	fp.nonSpin.addToObj(obj)
	fp.normalDownRamp.addToObj(obj)
	fp.normalUpRamp.addToObj(obj)
	// gen only
	fp.genMaxCharge.addToObj(obj, "gen_max_charge_mw")
	fp.genMaxDischarge.addToObj(obj, "gen_max_discharge_mw")
	// load only
	fp.loadPseudoSwitchStatus.addToObj(obj, "load_pseudo_switch_status")
	// constants
	obj["baseload_cmd_min_limit_mw"] = fp.baseloadCmdMinLimitMw
	obj["baseload_cmd_max_limit_mw"] = fp.baseloadCmdMaxLimitMw
	obj["inactive_cmd_min_limit_mw"] = fp.inactiveCmdMinLimitMw
	obj["inactive_cmd_max_limit_mw"] = fp.inactiveCmdMaxLimitMw
	// monitoring
	obj["updated_basepoint_sced"] = fp.updatedBasepointCompensatedMw
	obj["baseload_cmd_mw"] = fp.baseloadCmdMw
	// misc.
	obj["reg_manual_override"] = fp.overrideRequestFlag
	obj["reg_manual_override_feedback"] = fp.overrideAllowedFlag
	obj["calculated_rurs_gen"] = fp.calcRursGen
	obj["calculated_rupf_gen"] = fp.calcRupfGen
	obj["calculated_rdrs_gen"] = fp.calcRdrsGen
	obj["calculated_rdpf_gen"] = fp.calcRdpfGen
	obj["calculated_rurs_load"] = fp.calcRursLoad
	obj["calculated_rupf_load"] = fp.calcRupfLoad
	obj["calculated_rdrs_load"] = fp.calcRdrsLoad
	obj["calculated_rdpf_load"] = fp.calcRdpfLoad
	return obj
}

var overridableErcotVars []string = []string{
	"basepoint_gen",
	"basepoint_load",
	"emergency_down_ramp_rate_gen",
	"emergency_down_ramp_rate_load",
	"emergency_up_ramp_rate_gen",
	"emergency_up_ramp_rate_load",
	"frrs_down_gen_requirement",
	"frrs_down_gen_responsibility",
	"frrs_down_load_requirement",
	"frrs_down_load_responsibility",
	"frrs_up_gen_requirement",
	"frrs_up_gen_responsibility",
	"frrs_up_load_requirement",
	"frrs_up_load_responsibility",
	"gen_max_charge_mw",
	"gen_max_discharge_mw",
	"load_pseudo_switch_status",
	"non_spin_gen_requirement",
	"non_spin_gen_responsibility",
	"non_spin_load_requirement",
	"non_spin_load_responsibility",
	"normal_down_ramp_rate_gen",
	"normal_down_ramp_rate_load",
	"normal_up_ramp_rate_gen",
	"normal_up_ramp_rate_load",
	"updated_basepoint_gen",
	"updated_basepoint_load",
	"regulation_down_gen_requirement",
	"regulation_down_gen_responsibility",
	"regulation_down_load_requirement",
	"regulation_down_load_responsibility",
	"regulation_up_gen_requirement",
	"regulation_up_gen_responsibility",
	"regulation_up_load_requirement",
	"regulation_up_load_responsibility",
	"resource_status_gen",
	"resource_status_load",
}

// Returns a map of only the fleet points that are overridable (have a select = override ? manual : actual function).
func (fp *fleetPoints) buildOnlyOverridables() map[string]interface{} {
	obj := make(map[string]interface{})
	fp.scedBasepoint.addToObj(obj)
	fp.emergencyDownRamp.addToObj(obj)
	fp.emergencyUpRamp.addToObj(obj)
	fp.frrsDown.addToObj(obj)
	fp.frrsUp.addToObj(obj)
	fp.genMaxCharge.addToObj(obj, "gen_max_charge_mw")
	fp.genMaxDischarge.addToObj(obj, "gen_max_discharge_mw")
	fp.loadPseudoSwitchStatus.addToObj(obj, "load_pseudo_switch_status")
	fp.nonSpin.addToObj(obj)
	fp.normalDownRamp.addToObj(obj)
	fp.normalUpRamp.addToObj(obj)
	fp.updatedBasepoint.addToObj(obj)
	fp.resourceStatus.addToObj(obj)
	fp.regulationDown.addToObj(obj)
	fp.regulationUp.addToObj(obj)
	return obj
}

// Returns an array of the IDs of every site that the ERCOT AS feature is configured with.
func (ercotAs ercotAsFeature) listSites() []string {
	siteList := make([]string, 0, len(ercotAs))
	for siteId := range ercotAs {
		siteList = append(siteList, siteId)
	}
	return siteList
}

// Returns a map of ERCOT site IDs to site feature data for the ERCOT Ancillary Services feature.
func (ercotAs ercotAsFeature) buildObj() map[string]interface{} {
	obj := make(map[string]interface{})
	for siteId, siteFeatureSettings := range ercotAs {
		obj[siteId] = siteFeatureSettings.buildObj()
	}
	return obj
}

// Parses a single site's ERCOT AS feature settings.
func parseFleetPoints(cfgInterface interface{}) (*fleetPoints, error) {
	cfg, ok := cfgInterface.(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("expected map[string]interface{}, got %T", cfgInterface)
	}

	fp := &fleetPoints{}
	// required points
	newFloat, err := fg.ExtractValueWithType(cfg, "baseload_cmd_min_limit_mw", fg.FLOAT64)
	if err != nil {
		return nil, fmt.Errorf("error retrieving baseload_cmd_min_limit_mw: %w", err)
	}
	fp.baseloadCmdMinLimitMw = newFloat.(float64)

	newFloat, err = fg.ExtractValueWithType(cfg, "baseload_cmd_max_limit_mw", fg.FLOAT64)
	if err != nil {
		return nil, fmt.Errorf("error retrieving baseload_cmd_max_limit_mw: %w", err)
	}
	fp.baseloadCmdMaxLimitMw = newFloat.(float64)

	newFloat, err = fg.ExtractValueWithType(cfg, "inactive_cmd_min_limit_mw", fg.FLOAT64)
	if err != nil {
		return nil, fmt.Errorf("error retrieving inactive_cmd_min_limit_mw: %w", err)
	}
	fp.inactiveCmdMinLimitMw = newFloat.(float64)

	newFloat, err = fg.ExtractValueWithType(cfg, "inactive_cmd_max_limit_mw", fg.FLOAT64)
	if err != nil {
		return nil, fmt.Errorf("error retrieving inactive_cmd_max_limit_mw: %w", err)
	}
	fp.inactiveCmdMaxLimitMw = newFloat.(float64)

	fp.emergencyDownRamp = parseGLMuxFloat(cfg, "emergency_down_ramp_rate")
	fp.emergencyUpRamp = parseGLMuxFloat(cfg, "emergency_up_ramp_rate")
	fp.normalDownRamp = parseGLMuxFloat(cfg, "normal_down_ramp_rate")
	fp.normalUpRamp = parseGLMuxFloat(cfg, "normal_up_ramp_rate")

	// gen and load
	fp.scedBasepoint = parseGLMuxFloat(cfg, "basepoint")
	fp.responsiveReserveFfr = parseGLSNoMux(cfg, "ffr")
	fp.frrsDown = parseGLPService(cfg, "frrs_down")
	fp.frrsUp = parseGLPService(cfg, "frrs_up")
	fp.regulationDown = parseGLPService(cfg, "regulation_down")
	fp.regulationUp = parseGLPService(cfg, "regulation_up")
	fp.resourceStatus = parseGLMuxInt(cfg, "resource_status")
	fp.responsiveReservePfr = parseGLSNoMux(cfg, "responsive_reserve")
	fp.nonSpin = parseGLSService(cfg, "non_spin")
	fp.updatedBasepoint = parseGLMuxFloat(cfg, "updated_basepoint")

	//gen only
	initVal, err := fg.ExtractValueWithType(cfg, "gen_max_charge_mw_actual", fg.FLOAT64)
	if err != nil {
		initVal = 0.0
	}
	fp.genMaxCharge.actual = initVal.(float64)

	initVal, err = fg.ExtractValueWithType(cfg, "gen_max_charge_mw_manual", fg.FLOAT64)
	if err != nil {
		initVal = 0.0
	}
	fp.genMaxCharge.manual = initVal.(float64)

	initVal, err = fg.ExtractValueWithType(cfg, "gen_max_charge_mw_override", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.genMaxCharge.override = initVal.(bool)

	initVal, err = fg.ExtractValueWithType(cfg, "gen_max_discharge_mw_actual", fg.FLOAT64)
	if err != nil {
		initVal = 0.0
	}
	fp.genMaxDischarge.actual = initVal.(float64)

	initVal, err = fg.ExtractValueWithType(cfg, "gen_max_discharge_mw_manual", fg.FLOAT64)
	if err != nil {
		initVal = 0.0
	}
	fp.genMaxDischarge.manual = initVal.(float64)

	initVal, err = fg.ExtractValueWithType(cfg, "gen_max_discharge_mw_override", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.genMaxDischarge.override = initVal.(bool)

	// load
	initVal, err = fg.ExtractValueWithType(cfg, "load_pseudo_switch_status_actual", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.loadPseudoSwitchStatus.actual = initVal.(bool)

	initVal, err = fg.ExtractValueWithType(cfg, "load_pseudo_switch_status_manual", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.loadPseudoSwitchStatus.manual = initVal.(bool)

	initVal, err = fg.ExtractValueWithType(cfg, "load_pseudo_switch_status_override", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.loadPseudoSwitchStatus.override = initVal.(bool)

	// misc
	initVal, err = fg.ExtractValueWithType(cfg, "reg_manual_override", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.overrideRequestFlag = initVal.(bool)

	initVal, err = fg.ExtractValueWithType(cfg, "reg_manual_override_feedback", fg.BOOL)
	if err != nil {
		initVal = false
	}
	fp.overrideAllowedFlag = initVal.(bool)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rurs_gen")
	if err != nil {
		initVal = 0
	}
	fp.calcRursGen = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rupf_gen")
	if err != nil {
		initVal = 0
	}
	fp.calcRupfGen = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rdrs_gen")
	if err != nil {
		initVal = 0
	}
	fp.calcRdrsGen = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rdpf_gen")
	if err != nil {
		initVal = 0
	}
	fp.calcRdpfGen = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rurs_load")
	if err != nil {
		initVal = 0
	}
	fp.calcRursLoad = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rupf_load")
	if err != nil {
		initVal = 0
	}
	fp.calcRupfLoad = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rdrs_load")
	if err != nil {
		initVal = 0
	}
	fp.calcRdrsLoad = initVal.(int)

	initVal, err = fg.ExtractAsInt(cfg, "calculated_rdpf_load")
	if err != nil {
		initVal = 0
	}
	fp.calcRdpfLoad = initVal.(int)
	return fp, nil
}

// Adds any key-value pairs from a default map to a config map if the default pair's key
// does not already exist in the config.
func fillConfigWithDefaultValues(cfg, defaults map[string]interface{}) {
	for varId, value := range defaults {
		if _, ok := cfg[varId]; !ok {
			cfg[varId] = value
		}
	}
}

// Takes a site ID and the ERCOT AS configuration for that site and adds an entry to the ERCOT AS feature map.
// If the ERCOT AS feature map already has an entry with the given site ID, the entry is overwritten.
func (ercotAs ercotAsFeature) addSite(id string, cfgInterface interface{}, defaults map[string]interface{}) error {
	if id == "" {
		return errors.New("cannot add site to ERCOT AS feature with blank ID")
	}
	if _, ok := ercotAs[id]; ok {
		return fmt.Errorf("site %s already exists in ERCOT AS feature. Should be edited with SET if changes are desired", id)
	}
	cfg, ok := cfgInterface.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected map[string]interface{}, but got %T", cfgInterface)
	}

	// defaults not required
	if defaults != nil {
		fillConfigWithDefaultValues(cfg, defaults)
	}

	newCfg, err := parseFleetPoints(cfgInterface)
	if err != nil {
		return fmt.Errorf("failed to parse config: %w", err)
	}

	ercotAs[id] = newCfg
	newCfg.backupToDbi(id)
	log.Infof("Added site %s to ERCOT AS feature.", id)
	return nil
}

// Takes a map of site ERCOT AS feature configurations and adds them to the feature.
// Each site is parsed and added individually, so if one has a configuration error, the rest are still added.
func (ercotAs *ercotAsFeature) addMultipleSites(mapOfSiteCfgsInterface interface{}) error {
	mapOfSiteCfgs, ok := mapOfSiteCfgsInterface.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected map[string]interface{} but got %T", mapOfSiteCfgsInterface)
	}

	// look for a default controls map, then remove it so all that's left are the sites
	var defaults map[string]interface{}
	if defaultsInterface, ok := mapOfSiteCfgs["defaultControls"]; ok {
		defaults, ok = defaultsInterface.(map[string]interface{})
		if !ok {
			return fmt.Errorf("defaultControls provided but is %T, not map[string]interface{}", defaultsInterface)
		}
	}
	delete(mapOfSiteCfgs, "defaultControls")

	for id, cfg := range mapOfSiteCfgs {
		err := ercotAs.addSite(id, cfg, defaults)
		if err != nil {
			log.Errorf("Error adding site %s to ERCOT AS feature: %v.", id, err)
		}
	}

	return nil
}

// For each ERCOT site, publishes a flat map of its ERCOT AS feature variables.
func (ercotAs ercotAsFeature) publish() {
	for siteId, siteFeatureData := range ercotAs {
		pubUri := fmt.Sprintf("/fleet/features/%s/%s", ercotAsId, siteId)
		err := f.SendPub(pubUri, siteFeatureData.buildObj())
		if err != nil {
			log.Errorf("Error publishing to %s: %v", pubUri, err)
		}
	}
}
