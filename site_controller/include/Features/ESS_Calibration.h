#ifndef FEATURES_ESS_CALIBRATION_H_
#define FEATURES_ESS_CALIBRATION_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * ESS Calibration and Capacity Test Mode is a site wide ESS maintenance mode that removes many
 * of the restrictions and protections site controller places on the ESS to allow full control over the batteries.
 *
 * ESS Calibration Mode routes a single ess cmd value equally to every ESS, with power distribution and soc-balancing
 * disabled to ensure that the value commanded by the feature is the value of each ESS. Disabling the power distribution
 * and soc-balancing is handled by Site Manager and Asset Manager based on the variables in this feature object
 * rather than being part of the feature's execute() function.
 */
class features::ESS_Calibration : public Feature {
public:
    ESS_Calibration();

    Fims_Object kW_cmd;  // Command to be routed to each ESS
    Fims_Object soc_limits_enable;
    Fims_Object min_soc_limit;
    Fims_Object max_soc_limit;
    Fims_Object voltage_limits_enable;
    Fims_Object min_voltage_limit;
    Fims_Object max_voltage_limit;
    Fims_Object num_setpoint;
    Fims_Object num_limited;
    Fims_Object num_zero;

    void execute(Asset_Cmd_Object&, int num_ess_controllable);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        int num_ess_controllable;
        float ess_max_potential_kW;
        float ess_min_potential_kW;
    };
    struct External_Outputs {
        float ess_max_potential_kW;
        float ess_min_potential_kW;
        float ess_kW_request;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_ESS_CALIBRATION_H_ */