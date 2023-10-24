#ifndef FEATURES_GENERATOR_CHARGE_H_
#define FEATURES_GENERATOR_CHARGE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Generator Charge is a runmode 2 feature that attempts to charge ESS not only off of solar power,
 * but generator power as well.
 *
 * It calculates a generator command based on the current ESS active power and solar shedding buffers.
 * This feature is entirely coupled with solar shedding and LDSS.
 */
class features::Generator_Charge : public Feature {
public:
    Generator_Charge();

    Fims_Object generator_charge_additional_buffer;  // Additional buffer that reduces generator output

    void execute(Asset_Cmd_Object&, const features::Solar_Shed& solar_shed, float& max_potential_gen_kW);  // Executes feature logic on asset cmd data

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    // Does nothing
    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override{};
#pragma GCC diagnostic pop

private:
    struct External_Inputs {
        int solar_shed_calculator_offset;
        int solar_shed_calculator_min_offset;
        float solar_shed_high_threshold;
        float solar_max_potential_kW;
        float site_kW_load;
        float ess_min_potential_kW;
        float gen_max_potential_kW;
    };
    struct External_Outputs {
        float asset_cmd_gen_max_potential_kW;
        float site_manager_max_potential_gen_kW;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_GENERATOR_CHARGE_H_ */
