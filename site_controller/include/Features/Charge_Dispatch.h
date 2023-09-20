#ifndef FEATURES_CHARGE_DISPATCH_H_
#define FEATURES_CHARGE_DISPATCH_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Charge Dispatch is a feature which provides an interface to the variables used by
 * active power dispatch calculations.
 */
class features::Charge_Dispatch : public Feature {
public:
    Charge_Dispatch();

    Fims_Object kW_command;          // actual kW output from charge control algorithm
    Fims_Object solar_enable_flag;   // when true, use solar as a source for ESS charge
    Fims_Object gen_enable_flag;     // when true, use gen as a source for ESS charge
    Fims_Object feeder_enable_flag;  // when true, use the feeder as a source for ESS charge

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;
};

#endif /* FEATURES_CHARGE_DISPATCH_H_ */