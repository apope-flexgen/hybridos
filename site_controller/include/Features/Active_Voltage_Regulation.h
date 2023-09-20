#ifndef FEATURES_ACTIVE_VOLTAGE_REGULATION_H_
#define FEATURES_ACTIVE_VOLTAGE_REGULATION_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Active Voltage Regulation provides a reactive power response to deviations from a target voltage.
 */
class features::Active_Voltage_Regulation : public Feature {
public:
    Active_Voltage_Regulation();

    Fims_Object deadband;       // deadband within which algorithm wont respond
    Fims_Object droop_percent;  // percent of rated to apply per v deviation
    Fims_Object voltage_cmd;    // nominal voltage target
    Fims_Object actual_volts;   // actual voltage to calculated deviation with
    Fims_Object status_flag;    // true flag when response is nonzero
    Fims_Object rated_kVAR;     // rated kVAR for calculating response kVAR

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

#endif /* FEATURES_ACTIVE_VOLTAGE_REGULATION_H_ */