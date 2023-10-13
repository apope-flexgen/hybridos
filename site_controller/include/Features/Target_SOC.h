#ifndef FEATURES_TARGET_SOC_H_
#define FEATURES_TARGET_SOC_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Target SOC mode passes through the charge control request (ess_data.kW_request) received from asset manager
 * If solar is present, a portion is set aside to satisfy the charge control request
 */
class features::Target_SOC : public Feature {
public:
    Target_SOC();

    Fims_Object load_enable_flag;

    void execute(Asset_Cmd_Object&);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float solar_max_potential_kW;
        float site_kW_load;
        float site_kW_demand;
        float ess_kW_request;
    };
    struct External_Outputs {
        float solar_kW_request;
        load_compensation load_method;
        float additional_load_compensation;
        float site_kW_demand;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_TARGET_SOC_H_ */
