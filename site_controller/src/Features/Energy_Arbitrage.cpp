/*
 * Energy_Arbitrage.cpp
 *
 * Created: Summer 2022
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <fims/fps_utils.h>
/* Local Internal Dependencies */
#include <Site_Manager.h>
#include <Asset_Cmd_Object.h>
#include <Energy_Arbitrage.h>
#include <Site_Controller_Utils.h>

/**
 * Energy Arbitrage piecewise algorithm
 */
Energy_Arbitrage_Output Energy_Arbitrage::energy_arbitrage(const Energy_Arbitrage_Inputs& in)
{
    Energy_Arbitrage_Output out;

    // check of invalid pricing
    if (!(threshold_charge_2.value.value_float < threshold_charge_1.value.value_float &&
         threshold_charge_1.value.value_float < threshold_dischg_1.value.value_float &&
         threshold_dischg_1.value.value_float < threshold_dischg_2.value.value_float))
    {
        out.error_code = 1;
        return out;
    }

    // Price check logic, previously part of deleted function energy_arbitrage_price_check()
    // active power command must scale to consider derate of ess potential kW when between 1st and 2nd thresholds
    // price indicates max discharge
    if (price.value.value_float >= threshold_dischg_2.value.value_float) {
        out.ess_kW_power = fabsf(max_dischg_2.value.value_float);
    }
    // price indicates regular discharge
    else if ((price.value.value_float >= threshold_dischg_1.value.value_float) &&
            (in.soc_avg_running > soc_min_limit.value.value_float)) {
        out.ess_kW_power = fabsf(max_dischg_1.value.value_float);
    }
    // price indicates max charge
    else if (price.value.value_float <= threshold_charge_2.value.value_float) {
        out.ess_kW_power = -1.0f * fabsf(max_charge_2.value.value_float);
    }
    // price indicate regular charge
    else if ((price.value.value_float <= threshold_charge_1.value.value_float) &&
            (in.soc_avg_running < soc_max_limit.value.value_float)) {
        out.ess_kW_power = -1.0f * fabsf(max_charge_1.value.value_float);
    }

    // check for solar max potential kW; set to 0 if price is less than or equal to 0
    if (price.value.value_float <= 0) {
        out.solar_max_potential_kW = 0;
    } else {
        out.solar_max_potential_kW = in.solar_max_potential_kW;
    }
    
    // Solar uncurtailed
    out.solar_kW_request = out.solar_max_potential_kW;
    return out;
}