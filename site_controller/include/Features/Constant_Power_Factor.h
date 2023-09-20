#ifndef FEATURES_CONSTANT_POWER_FACTOR_H_
#define FEATURES_CONSTANT_POWER_FACTOR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Constant Power Factor is a reactive power feature that commands a reactive power output based on the power factor setpoint provided for the site.
 */
class features::Constant_Power_Factor : public Feature {
public:
    Constant_Power_Factor();

    Fims_Object power_factor_setpoint;  // Power Factor setpoint. Sign is used if bidirectional mode, only magnitude in absolute mode
    Fims_Object lagging_limit;          // -1.0 < pf < lagging_limit for negative commands
    Fims_Object leading_limit;          //  1.0 > pf > leading_limit for positive commands
    Fims_Object absolute_mode;          // Absolute value mode, with sign determined by the direction flag:
    Fims_Object lagging_direction;      // Direction flag. True is negative aka lagging, and false is positive aka leading

    void execute(Asset_Cmd_Object&, bool& asset_pf_flag);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float uncorrected_site_kW_demand;
        float total_potential_kVAR;
    };
    struct External_Outputs {
        float site_kVAR_demand;
        bool asset_pf_flag;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_CONSTANT_POWER_FACTOR_H_ */