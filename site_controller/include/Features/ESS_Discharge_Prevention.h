#ifndef FEATURES_ESS_DISCHARGE_PREVENTION_H_
#define FEATURES_ESS_DISCHARGE_PREVENTION_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * ESS Discharge Prevention is a simple standalone feature that will block ESS discharges when the
 * average running state of charge is at or below a configured value.
 *
 * When the average running ESS SoC is below a configured value, sets the ESS kW limits to
 * be no more than 0kW, disallowing discharges.
 */
class features::ESS_Discharge_Prevention : public Feature {
public:
    ESS_Discharge_Prevention();

    Fims_Object edp_soc;  // target soc at or below which the ess will not be able to discharge

    void execute(Asset_Cmd_Object& asset_cmd, float soc_avg_running, float& max_potential_ess_kW, float& min_potential_ess_kW, float ess_total_kW_discharge_limit, float& total_asset_kW_discharge_limit);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float soc_avg_running;
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

#endif /* FEATURES_ESS_DISCHARGE_PREVENTION_H_ */