#ifndef FEATURES_SOLAR_SHED_H_
#define FEATURES_SOLAR_SHED_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>
#include <Variable_Regulator.h>

/**
 * Solar Shedding is a stand-alone feature for island mode (Run Mode 2). It dynamically manages the amount of
 * solar power to ensure that the ESS is not over-charged while still trying to maintain as much solar power as it can.
 */
class features::Solar_Shed : public Feature {
public:
    Solar_Shed();

    Fims_Object solar_shed_value;                   // Current solar shed value
    Fims_Object solar_shed_max_value;               // Max solar shed value
    Fims_Object solar_shed_min_value;               // Min solar shed value
    Fims_Object solar_shed_max_shedding_threshold;  // If SOC rises above this value, set the solar shed value to max
    Fims_Object solar_shed_high_threshold;          // Threshold on ess (chargeable power - measured charging power) to decrease shed value
    Fims_Object solar_shed_decrease_timer_ms;       // Time above threshold before decreasing solar shed value
    Fims_Object solar_shed_low_threshold;           // Threshold on ess (chargeable power - measured charging power) to increase shed value
    Fims_Object solar_shed_increase_timer_ms;       // Time below threshold before increasing solar shed value
    Fims_Object solar_shed_increase_display_timer;  // solar_shed_increase_timer_ms converted to a MIN:SEC display string
    Fims_Object solar_shed_decrease_display_timer;  // solar_shed_decrease_timer_ms converted to a MIN:SEC display string
    Fims_Object solar_shed_spare_ess_kw;            // ess chargeable kw - ess current charging kw, AKA how much more charge power the ess can handle

    Variable_Regulator solar_shed_calculator;  // Used to hold and calculate the solar shed value

    // Initializes variables
    void init();

    // Resets solar shed value to max
    void reset();

    // Updates the solar shed value and curtails solar power accordingly
    void execute(Asset_Cmd_Object&, float soc_avg_running, float solar_total_rated_active_power, float& max_potential_solar_kW);

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float soc_avg_running;
        float solar_total_rated_active_power;
        float asset_cmd_solar_max_potential_kW;
    };
    struct External_Outputs {
        float asset_cmd_solar_max_potential_kW;
        float site_max_potential_solar_kW;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_SOLAR_SHED_H_ */