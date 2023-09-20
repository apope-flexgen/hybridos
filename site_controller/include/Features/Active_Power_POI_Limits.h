#ifndef FEATURES_ACTIVE_POWER_POI_LIMITS_H_
#define FEATURES_ACTIVE_POWER_POI_LIMITS_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Active Power POI Limits is a stand-alone feature that regulates power output of assets to prevent exceeding power limits on the POI feeder for Active Power features.
 *
 * Adjusts site demand based on the legal/physical/etc power limits of the point of interface (poi), with poi_limits_min_kW limiting the
 * power that can be imported into the site (negative), and poi_limits_max_kW limiting the power that can be exported from the site (positive).
 * Largely works to reconcile the expected value at the POI and the site production needed to achieve this expected value.
 *
 * If tracked by the active power feature and enabled based on configuration, load (including uncontrollable) is implicit in the site demand received and is expected to
 * cancel out with site discharge production. As such, it should be removed from the demand to get the true expected value at the POI.
 */
class features::Active_Power_POI_Limits : public Feature {
public:
    Active_Power_POI_Limits();

    Fims_Object max_kW;                 // max POI kW value allowed when POI Limits enabled
    Fims_Object min_kW;                 // min POI kW value allowed when POI Limits enabled
    Fims_Object soc_poi_limits_enable;  // ESS specific, soc-based POI limits
    Fims_Object target_soc;             // threshold below which "under" limits are applied and above which "over" limits are applied
    Fims_Object soc_low_min_kW;         // min POI limit when soc <= soc_poi_target_soc
    Fims_Object soc_low_max_kW;         // max POI limit when soc <= soc_poi_target_soc
    Fims_Object soc_high_min_kW;        // min POI limit when soc <= soc_poi_target_soc
    Fims_Object soc_high_max_kW;        // max POI limit when soc <= soc_poi_target_soc

    void execute(Asset_Cmd_Object&, float soc_avg_running, int asset_priority_runmode1, float& total_site_kW_charge_limit, float& total_site_kW_discharge_limit);  // Executes feature logic on asset cmd data

    void toggle_ui_enabled(bool flag);

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

    // Note:
    // The Active Power POI Limits feature is one of the most complicated features
    // in terms of the numbers of inputs and outputs. As such, the explicit list of
    // feature inputs and outputs has been left out.
};

#endif /* FEATURES_ACTIVE_POWER_POI_LIMITS_H_ */