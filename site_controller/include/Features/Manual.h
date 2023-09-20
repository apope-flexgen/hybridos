#ifndef FEATURES_MANUAL_H_
#define FEATURES_MANUAL_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Manual mode is an active power feature that allows the user to directly set a power command for
 * each asset type and a slew rate for each asset type.
 *
 * Manual mode takes a solar kW cmd, ESS kW cmd, generator kW cmd, solar slew rate,
 * ESS slew rate, and generator slew rate and routes those commands through dispatch and charge control
 * Any site load should be offloaded to the feeder to guarantee the manual kW commands.
 */
class features::Manual : public Feature {
public:
    Manual();

    Fims_Object manual_solar_kW_cmd;  // manual setpoint for solar kW
    Fims_Object manual_ess_kW_cmd;    // manual setpoint for ess kW
    Fims_Object manual_gen_kW_cmd;    // manual setpoint for gen kW
    Fims_Object manual_solar_kW_slew_rate;
    Fims_Object manual_ess_kW_slew_rate;
    Fims_Object manual_gen_kW_slew_rate;

    Slew_Object manual_solar_kW_slew;
    Slew_Object manual_ess_kW_slew;
    Slew_Object manual_gen_kW_slew;

    void execute(Asset_Cmd_Object&);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float ess_max_potential_kW;
        float solar_max_potential_kW;
        float gen_max_potential_kW;
    };
    struct External_Outputs {
        float ess_kW_request;
        float ess_max_potential_kW;
        float solar_kW_request;
        float solar_max_potential_kW;
        float gen_kW_request;
        float gen_max_potential_kW;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_MANUAL_H_ */
