#ifndef FEATURES_WATT_WATT_H_
#define FEATURES_WATT_WATT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Watt-Watt compensation is a stand-alone feature that allows for power commands to
 * be augmented in order to account for losses at the POI.
 */
class features::Watt_Watt : public Feature {
public:
    Watt_Watt();

    Fims_Object watt_watt_points;

    std::vector<std::pair<float, float>> watt_watt_curve;
    float site_kW_demand_correction;  // Internal variable tracking the amount of correction applied by watt_watt

    void init_curve();  // Parses points variable to internal vector of curve points

    void execute(Asset_Cmd_Object&);  // Executes feature logic on asset cmd data

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float site_kW_demand;
    };
    struct External_Outputs {
        float site_kW_demand;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_WATT_WATT_H_ */
