#ifndef FEATURES_AGGREGATED_ASSET_LIMIT_H_
#define FEATURES_AGGREGATED_ASSET_LIMIT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Aggregated Asset Limit is a special-case limit for ESS that will prevent a discharging ESS from
 * causing the combined ESS + solar power output to exceed the feature-defined limit.
 *
 * This feature exclusively limits ESS, and exclusively limits it in the Discharge direction.
 * Sums the aggregate of solar output and the output of any ESS in maintenance mode (uncontrolled).
 * Sets the controllable ESS limit such that the controllable ESS will not cause the total of all
 * solar+ESS to exceed the feature limit. If the solar and/or uncontrollable ESS exceeds the feature
 * limit by themselves, that is allowed.
 *
 * Note for context: This feature was originally developed to satisfy a specific customer request where
 * solar is expected to always be below the limit, so just implementing logic to limit the ESS is
 * satisfactory.
 */
class features::Aggregated_Asset_Limit : public Feature {
public:
    Aggregated_Asset_Limit();

    Fims_Object agg_asset_limit_kw;

    void execute(Asset_Cmd_Object&, float uncontrolled_ess_kW, float uncontrolled_solar_kW, float& max_potential_ess_kW, float& min_potential_ess_kW, float& total_asset_kW_discharge_limit,
                 float ess_total_kW_discharge_limit);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float solar_actual_kW;
        float uncontrolled_solar_kW;
        float uncontrolled_ess_kW;
        float ess_total_kW_discharge_limit;
        float site_manager_max_potential_ess_kW;
        float asset_cmd_ess_max_potential_kW;
        float site_manager_min_potential_ess_kW;
        float asset_cmd_ess_min_potential_kW;
        float total_asset_kW_discharge_limit;
    };
    struct External_Outputs {
        float site_manager_max_potential_ess_kW;
        float asset_cmd_ess_max_potential_kW;
        float site_manager_min_potential_ess_kW;
        float asset_cmd_ess_min_potential_kW;
        float total_asset_kW_discharge_limit;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_AGGREGATED_ASSET_LIMIT_H_ */
