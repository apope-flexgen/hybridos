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

    Fims_Object over_deadband;    // deadband within which algorithm will not respond in overfrequency event
    Fims_Object over_droop;       // response to apply per Hz deviation in overfrequency event
    Fims_Object max_rated_kW;     // max kW scaler used during PFR event
    Fims_Object under_deadband;   // deadband within which algorithm will not respond in underfrequency event
    Fims_Object under_droop;      // response to apply per Hz deviation in underfrequency event
    Fims_Object min_rated_kW;     // min kW scaler used during PFR event
    Fims_Object site_nominal_hz;  // nominal frequency target
    Fims_Object status_flag;      // boolean indicates if PFR is actively changing system power output
    Fims_Object request;          // request produced in response to frequency deviation
    Fims_Object offset_hz;        // test input that gets added to site frequency to simulate frequency deviation events

    bool symmetric_variables = true;  // indicates if over/underfrequency variables should be symmetric, in which case the configured overfrequency values are used for both cases
                                      // set false by parse_json_config() function if underfrequency variables are found during config parsing

    Config_Validation_Result parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) override;
    void initialize_feature_vars();                                                                                                  // Initializes feature_vars separate from construction because variables are dependent on configuration
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