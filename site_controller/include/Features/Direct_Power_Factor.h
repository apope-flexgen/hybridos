#ifndef FEATURES_DIRECT_POWER_FACTOR_H_
#define FEATURES_DIRECT_POWER_FACTOR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Direct Power Factor is a reactive power feature that commands a reactive power output by directly
 * setting the power factor on any assets that support direct power factor control.
 */
class features::Direct_Power_Factor : public Feature {
public:
    Direct_Power_Factor();

    Fims_Object power_factor_cmd;  // pf-setpoint used directly by Site Manager rather than affecting the asset cmd

    float prev_asset_pf_cmd;

    void execute(Asset_Cmd_Object&, bool& asset_pf_flag);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Outputs {
        float site_kVAR_demand;
        bool asset_pf_flag;
    };
    External_Outputs execute_helper();
};

#endif /* FEATURES_DIRECT_POWER_FACTOR_H_ */
