#ifndef FEATURES_WATT_VAR_H_
#define FEATURES_WATT_VAR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Watt-VAr is a reactive power feature that commands a reactive power output determined by the commanded active power.
 */
class features::Watt_Var : public Feature {
public:
    Watt_Var();

    Fims_Object watt_var_points;

    std::vector<std::pair<float, float>> watt_var_curve;  // Vector of curve points parsed from points variable
    float site_kVAR_demand_correction;                    // Internal variable tracking the amount of correction applied by watt_var

    void init_curve();  // Parses points variable to internal vector of curve points

    void execute(Asset_Cmd_Object&, float active_power, bool& asset_pf_flag);  // Executes feature logic on asset cmd data

// tell the compiler to ignore warnings about unused parameters for the following function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    // Does nothing
    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override{};
#pragma GCC diagnostic pop

private:
    struct External_Inputs {
        float site_kVAR_demand;
        float active_power;
    };
    struct External_Outputs {
        float site_kVAR_demand;
        bool asset_pf_flag;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_WATT_VAR_H_ */