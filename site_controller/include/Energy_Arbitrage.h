/*
 * Energy_Arbitrage.h
 *
 *  Created on: June 22, 2022
 *      Author: ahanna
 */

#ifndef INCLUDE_ENERGY_ARBITRAGE_H_
#define INCLUDE_ENERGY_ARBITRAGE_H_

/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Fims_Object.h>
/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */

typedef struct energy_arbitrage_inputs {
    float soc_avg_running;
    float solar_max_potential_kW;
} Energy_Arbitrage_Inputs;

typedef struct energy_arbitrage_output {
    float ess_kW_power = 0.0f;
    float solar_kW_request = 0.0f;
    float solar_max_potential_kW = 0.0f;
    int error_code = 0;
} Energy_Arbitrage_Output;

class Energy_Arbitrage {
public:
    Energy_Arbitrage_Output energy_arbitrage(const Energy_Arbitrage_Inputs&);
    Fims_Object price;
    Fims_Object threshold_charge_1, threshold_charge_2, threshold_dischg_1, threshold_dischg_2;
    Fims_Object soc_min_limit, soc_max_limit;
    Fims_Object max_charge_1, max_charge_2, max_dischg_1, max_dischg_2;
};

#endif /* INCLUDE_ENERGY_ARBITRAGE_H_ */