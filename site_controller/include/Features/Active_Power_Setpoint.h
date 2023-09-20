#ifndef FEATURES_ACTIVE_POWER_SETPOINT_H_
#define FEATURES_ACTIVE_POWER_SETPOINT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>
#include <Slew_Object.h>

/**
 * ACTIVE POWER SETPOINT MODE takes a single power command (the total site production, ignoring load) that is distributed to assets.
 * Positive commands are distributed to to all assets as needed, with priority based on the configured asset priority.
 * Negative commands are distributed to ESS as available.
 * If load is not taken into account, the value at the POI will fluctuate as load changes.
 * Load must be handled manually by this feature due to its feature level slew
 * kW_cmd can be sent using `absolute_mode` and `direction` to determine sign. false by default.
 */
class features::Active_Power_Setpoint : public Feature {
public:
    Active_Power_Setpoint();

    Fims_Object kW_cmd;                          // Single power command to be distributed to assets
    Fims_Object load_method;                     // Whether site production should handle load compensation minimum, offset, or not at all.
    Fims_Object kW_slew_rate;                    // Slew rate for the feature
    Fims_Object absolute_mode_flag;              // Flag determining whether kW_cmd should be interpreted as absolute value.
    Fims_Object direction_flag;                  // Flag used if absolute mode == true; kW_cmd is negative if true, positive if false.
    Fims_Object maximize_solar_flag;             // Flag used to maximize solar kW_cmd always.
    Fims_Object ess_charge_support_enable_flag;  // This enables ESS to charge to support underloaded grid - only available in runmode1 (active power setpoint)

    Slew_Object kW_slew;

    void execute(Asset_Cmd_Object&);                 // Executes feature logic on asset cmd data
    void charge_support_execute(Asset_Cmd_Object&);  // Reduces ess command to shift load off of POI when an asset cannot ramp down fast enough

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float solar_max_potential_kW;
        float site_kW_load;
        float site_kW_demand;
        float ess_kW_request;
        float gen_kW_request;
        float solar_kW_request;
    };
    struct External_Outputs {
        float solar_kW_request;
        float poi_cmd;
        load_compensation asset_cmd_load_method;
        float additional_load_compensation;
        float site_kW_demand;
    };
    External_Outputs execute_helper(const External_Inputs&);

    struct Charge_Support_External_Inputs {
        float ess_kW_cmd;
        float ess_min_potential_kW;
        float gen_min_potential_kW;
        float solar_min_potential_kW;
        float site_kW_load;
    };
    struct Charge_Support_External_Outputs {
        float ess_kW_cmd;
    };
    Charge_Support_External_Outputs charge_support_execute_helper(const Charge_Support_External_Inputs&);
};

#endif /* FEATURES_ACTIVE_POWER_SETPOINT_H_ */
