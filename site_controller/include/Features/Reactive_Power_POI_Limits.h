#ifndef FEATURES_REACTIVE_POWER_POI_LIMITS_H_
#define FEATURES_REACTIVE_POWER_POI_LIMITS_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Reactive Power POI Limits is a stand-alone feature that regulates power output of assets to prevent exceeding power limits on the POI feeder for reactive power features.
 *
 * Adjusts site demand based on the legal/physical/etc power limits of the point of interface (poi), with reactive_power_poi_limits_min_kVAR limiting the
 * power that can be imported into the site (negative), and reactive_power_poi_limits_max_kVAR limiting the power that can be exported from the site (positive).
 *
 * No load is considered so there is no requirement to solve for the value at the POI. The limits are simply applied as is
 */
class features::Reactive_Power_POI_Limits : public Feature {
public:
    Reactive_Power_POI_Limits();

    Fims_Object min_kVAR;  // min POI kVAR value allowed when POI Limits enabled
    Fims_Object max_kVAR;  // max POI kVAR value allowed when POI Limits enabled

    void execute(Asset_Cmd_Object&);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float site_kVAR_demand;
    };
    struct External_Outputs {
        float site_kVAR_demand;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_REACTIVE_POWER_POI_LIMITS_H_ */