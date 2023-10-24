#ifndef FEATURES_REACTIVE_SETPOINT_H_
#define FEATURES_REACTIVE_SETPOINT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Reactive Power Setpoint mode takes a single reactive power setpoint and passes it on for asset distribution.
 */
class features::Reactive_Setpoint : public Feature {
public:
    Reactive_Setpoint();

    Fims_Object kVAR_cmd;
    Fims_Object kVAR_slew_rate;

    Slew_Object kVAR_cmd_slew;

    void execute(Asset_Cmd_Object&, bool& asset_pf_flag);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float total_potential_kVAR;
    };
    struct External_Outputs {
        float site_kVAR_demand;
        bool asset_pf_flag;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_REACTIVE_SETPOINT_H_ */
