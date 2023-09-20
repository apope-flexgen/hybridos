#ifndef FEATURES_PFR_H_
#define FEATURES_PFR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Primary Frequency Response is a stand-alone power feature that augments any designated
 * active power commands to assist with grid frequency regulation.
 */
class features::PFR : public Feature {
public:
    PFR();

    Fims_Object deadband;         // deadband in Hz for pfr algorithm
    Fims_Object droop_percent;    // droop percentage for pfr algorithm
    Fims_Object status_flag;      // boolean indicates if pfr is actively changing system power output
    Fims_Object nameplate_kW;     // maximum power output for PFR algorithm
    Fims_Object limits_min_kW;    // min PFR kW scaler used during a frequency event
    Fims_Object limits_max_kW;    // max PFR kW scaler used during a frequency event
    Fims_Object site_nominal_hz;  // frequency value that site should be at in ideal conditions
    Fims_Object offset_hz;        // test input that gets added to site frequency to simulate frequency deviation events

    void execute(Asset_Cmd_Object&, float site_frequency, float& total_site_kW_charge_limit, float& total_site_kW_discharge_limit);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float site_frequency;
        float site_kW_demand;
        float total_site_kW_charge_limit;
        float total_site_kW_discharge_limit;
    };
    struct External_Outputs {
        float site_kW_demand;
        float total_site_kW_charge_limit;
        float total_site_kW_discharge_limit;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_PFR_H_ */