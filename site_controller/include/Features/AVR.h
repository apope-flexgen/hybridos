#ifndef FEATURES_AVR_H_
#define FEATURES_AVR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Automatic Voltage Regulation provides a reactive power response to deviations from a target voltage.
 */
class features::AVR : public Feature {
public:
    AVR();

    Fims_Object over_deadband;     // deadband within which algorithm will not respond in overvoltage event
    Fims_Object over_droop;        // response to apply per V deviation in overvoltage event
    Fims_Object over_rated_kVAR;   // rated kVAR for calculating response kVAR in overvoltage event
    Fims_Object under_deadband;    // deadband within which algorithm will not respond in undervoltage event
    Fims_Object under_droop;       // response to apply per V deviation in undervoltage event
    Fims_Object under_rated_kVAR;  // rated kVAR for calculating response kVAR in undervoltage event
    Fims_Object voltage_cmd;       // nominal voltage target
    Fims_Object voltage_cmd_max;   // nominal voltage target maximum limit
    Fims_Object voltage_cmd_min;   // nominal voltage target minimum limit
    Fims_Object actual_volts;      // actual measured voltage to calculate deviation with
    Fims_Object status_flag;       // true flag when response is nonzero
    Fims_Object request;           // request produced in response to voltage deviation
    Fims_Object kVAR_slew_rate;    // Slew rate on the correction calculated by AVR based on voltage deviations
    Slew_Object kVAR_slew;         // Slew object that calculates the AVR slewed cmd

    bool symmetric_variables = true;  // indicates if over/undervoltage variables should be symmetric, in which case the configured overvoltage values are used for both cases
                                      // set false by parse_json_config() function if undervoltage variables are found during config parsing

    Config_Validation_Result parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) override;
    void initialize_feature_vars();                        // Initializes feature_vars separate from construction because variables are dependent on configuration
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

#endif /* FEATURES_AVR_H_ */
