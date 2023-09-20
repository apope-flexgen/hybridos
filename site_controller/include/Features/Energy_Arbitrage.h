/*
 * Energy_Arbitrage.h
 *
 *  Created on: June 22, 2022
 *      Author: ahanna
 */

#ifndef FEATURES_ENERGY_ARBITRAGE_H_
#define FEATURES_ENERGY_ARBITRAGE_H_

/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>
/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */

/**
 * Energy Arbitrage is an active power feature that directs the site to source/sink power based on a price signal.
 */
class features::Energy_Arbitrage : public Feature {
public:
    Energy_Arbitrage();

    Fims_Object price;
    Fims_Object threshold_charge_1, threshold_charge_2, threshold_dischg_1, threshold_dischg_2;
    Fims_Object soc_min_limit, soc_max_limit;
    Fims_Object max_charge_1, max_charge_2, max_dischg_1, max_dischg_2;

    static const uint alarm_index = 3;

    // Executes feature logic on asset cmd data, returns false if an error occurred which should set an alarm
    bool execute(Asset_Cmd_Object&, float soc_avg_running, bool active_alarm_array[]);

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;

private:
    struct External_Inputs {
        float ess_kW_request;
        float solar_kW_request;
        float solar_max_potential_kW;
        load_compensation load_method;
        float additional_load_compensation;
        float site_kW_demand;
        bool is_alarming;
        float soc_avg_running;
        float site_kW_load;
        float gen_kW_request;
    };
    struct External_Outputs {
        float ess_kW_request;
        float solar_kW_request;
        float solar_max_potential_kW;
        load_compensation load_method;
        float additional_load_compensation;
        float site_kW_demand;
        bool should_alarm;
    };
    External_Outputs execute_helper(const External_Inputs&);
};

#endif /* FEATURES_ENERGY_ARBITRAGE_H_ */
