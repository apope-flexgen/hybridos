#ifndef FEATURES_LOAD_SHED_H_
#define FEATURES_LOAD_SHED_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>
#include <Variable_Regulator.h>

/**
 * Load Shedding is a stand-alone feature for island mode (Run Mode 2).
 * It dynamically manages the site load to ensure that the ESS is not over-discharged while still
 * trying to maintain as much site load as it can. It controls the amount of site load by sending
 * a load shed value to a load shed controller.
 */
class features::Load_Shed : public Feature {
public:
    Load_Shed();

    Fims_Object load_shed_value;                   // Contains write uri for load shedder
    Fims_Object load_shed_max_value;               // Max load shed value
    Fims_Object load_shed_min_value;               // Min load shed value
    Fims_Object load_shed_max_shedding_threshold;  // If SOC falls below this value, set the load shed value to max
    Fims_Object load_shed_high_threshold;          // Threshold on ess (dischargeable power - measured power) to decrease shed value
    Fims_Object load_shed_decrease_timer_ms;       // Time above threshold before decreasing load shed value
    Fims_Object load_shed_low_threshold;           // Threshold on ess (dischargeable power - measured power) to increase shed value
    Fims_Object load_shed_increase_timer_ms;       // Time below threshold before increasing load shed value
    Fims_Object load_shed_increase_display_timer;  // load_shed_increase_timer_ms converted to a MIN:SEC display string
    Fims_Object load_shed_decrease_display_timer;  // load_shed_decrease_timer_ms converted to a MIN:SEC display string
    Fims_Object load_shed_spare_ess_kw;            // ess dischargeable kw - ess current discharging kw, AKA how much more discharge power the ess can handle

    Variable_Regulator load_shed_calculator;  // Used to hold and calculate the load shed value

    // Initializes variables and sends initial load shed value
    void init();

    // Resets load shed value to max
    void reset();

    // Updates the load shed value and writes out the value if it changes
    void execute(float soc_avg_running);

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;
};

#endif /* FEATURES_LOAD_SHED_H_ */
