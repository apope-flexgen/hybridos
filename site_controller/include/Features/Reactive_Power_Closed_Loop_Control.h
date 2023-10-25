#ifndef FEATURES_REACTIVE_POWER_CLOSED_LOOP_CONTROL_H_
#define FEATURES_REACTIVE_POWER_CLOSED_LOOP_CONTROL_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>
#include <Slew_Object.h>
#include <Variable_Regulator.h>

/**
 * Reactive Closed Loop Control is a stand-alone feature for grid-tied mode (Run Mode 1). It closes the loop
 * on the reactive power command dispatch to take the value at the POI into account.
 */
class features::Reactive_Power_Closed_Loop_Control : public Feature {
public:
    Reactive_Power_Closed_Loop_Control();

    Fims_Object default_offset;             // Default offset
    Fims_Object min_offset;                 // Min offset
    Fims_Object max_offset;                 // Max offset
    Fims_Object step_size_kW;               // Step size (in kW) multiplied against offset to get the total correction to apply
    Fims_Object total_correction;           // Total correction applied by reactive closed loop control
    Fims_Object zero_bypass_enable;         // If enabled, no correction will be applied within the zero bypass deadband
    Fims_Object zero_bypass_deadband_kVAR;  // Within the deadband, if zero bypass is enabled, closed loop control will not apply a correction
    Fims_Object steady_state_deadband_kW;   // If difference in commands deviates beyond this value, reset to default value
    Fims_Object regulation_deadband_kW;     // Deadband threshold based on feeder rated power (in kW) outside which POI values are inaccurate
    Fims_Object update_rate_ms;             // Number of timer updates per second
    Fims_Object decrease_timer_ms;          // Time above regulation deadband before decreasing offset value
    Fims_Object increase_timer_ms;          // Time below regulation deadband before increasing offset value

    Variable_Regulator regulator;  // Used to hold and calculate the offset value

    float prev_reactive_power_feature_cmd;  // Previous kVAR demand set by the reactive power feature, used to track how much the demand is changing

    // Initializes variables and internal regulator
    void init();

    void execute(Asset_Cmd_Object&, float feeder_actual_kVAR, float total_site_kVAR_rated_charge, float total_site_kVAR_rated_discharge);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float feeder_actual_kVAR;
        float site_kVAR_demand;
        float total_site_kVAR_rated_charge;
        float total_site_kVAR_rated_discharge;
    };
    struct External_Outputs {
        float site_kVAR_demand;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_REACTIVE_POWER_CLOSED_LOOP_CONTROL_H_ */
