#ifndef FEATURES_CHARGE_CONTROL_H_
#define FEATURES_CHARGE_CONTROL_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Charge Control is a feature which provides an interface to the variables used by the Asset Manager's
 * charge control algorithm. It is enabled based on the active power features charge_control mask.
 */
class features::Charge_Control : public Feature {
public:
    Charge_Control();

    Fims_Object kW_request;  // charge kW request from control algorithm
    Fims_Object target_soc;  // control for target state of charge in charge control algorithm
    Fims_Object kW_limit;    // kW limit for ESS, applied to both charge and discharge
    Fims_Object charge_disable;
    Fims_Object discharge_disable;

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;
};

#endif /* FEATURES_CHARGE_CONTROL_H_ */