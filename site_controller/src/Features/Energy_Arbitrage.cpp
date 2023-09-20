/*
 * Energy_Arbitrage.cpp
 *
 * Created: Summer 2022
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Energy_Arbitrage.h>

features::Energy_Arbitrage::Energy_Arbitrage() {
    feature_vars = {
        &price, &threshold_charge_1, &threshold_charge_2, &threshold_dischg_1, &threshold_dischg_2, &soc_min_limit, &soc_max_limit, &max_charge_1, &max_charge_2, &max_dischg_1, &max_dischg_2,
    };
    summary_vars = {
        &threshold_charge_2, &threshold_charge_1, &threshold_dischg_1, &threshold_dischg_2, &price,
    };

    variable_ids = {
        { &enable_flag, "energy_arb_enable_flag" },
        { &price, "price" },
        { &threshold_charge_1, "threshold_charge_1" },
        { &threshold_charge_2, "threshold_charge_2" },
        { &threshold_dischg_1, "threshold_dischg_1" },
        { &threshold_dischg_2, "threshold_dischg_2" },
        { &soc_min_limit, "soc_min_limit" },
        { &soc_max_limit, "soc_max_limit" },
        { &max_charge_1, "max_charge_1" },
        { &max_charge_2, "max_charge_2" },
        { &max_dischg_1, "max_dischg_1" },
        { &max_dischg_2, "max_dischg_2" },
    };
}

bool features::Energy_Arbitrage::execute(Asset_Cmd_Object& asset_cmd, float soc_avg_running, bool active_alarm_array[]) {
    External_Inputs Energy_Arbitrage_Parameters{
        asset_cmd.ess_data.kW_request, asset_cmd.solar_data.kW_request, asset_cmd.solar_data.max_potential_kW, asset_cmd.load_method, asset_cmd.additional_load_compensation, asset_cmd.site_kW_demand, active_alarm_array[alarm_index], soc_avg_running,
        asset_cmd.site_kW_load,        asset_cmd.gen_data.kW_request,
    };
    External_Outputs outputs = execute_helper(Energy_Arbitrage_Parameters);
    asset_cmd.ess_data.kW_request = outputs.ess_kW_request;
    asset_cmd.solar_data.kW_request = outputs.solar_kW_request;
    asset_cmd.solar_data.max_potential_kW = outputs.solar_max_potential_kW;
    asset_cmd.load_method = outputs.load_method;
    asset_cmd.additional_load_compensation = outputs.additional_load_compensation;
    asset_cmd.site_kW_demand = outputs.site_kW_demand;

    return outputs.should_alarm;
}

features::Energy_Arbitrage::External_Outputs features::Energy_Arbitrage::execute_helper(const External_Inputs& in) {
    // Start with outputs equal to corresponding inputs so any skipped conditional logic results in no change
    External_Outputs out{
        in.ess_kW_request, in.solar_kW_request, in.solar_max_potential_kW, in.load_method, in.additional_load_compensation, in.site_kW_demand, false,
    };

    // default outputs of price check logic are zeros
    float new_ess_kW_request = 0.0f;
    float new_solar_kW_request = 0.0f;
    float new_solar_max_potential_kW = 0.0f;
    bool invalid_pricing = false;

    // check of invalid pricing
    if (!(threshold_charge_2.value.value_float < threshold_charge_1.value.value_float && threshold_charge_1.value.value_float < threshold_dischg_1.value.value_float && threshold_dischg_1.value.value_float < threshold_dischg_2.value.value_float)) {
        invalid_pricing = true;
    } else {
        // Price check logic, previously part of deleted function energy_arbitrage_price_check()
        // active power command must scale to consider derate of ess potential kW when between 1st and 2nd thresholds
        // price indicates max discharge
        if (price.value.value_float >= threshold_dischg_2.value.value_float) {
            new_ess_kW_request = fabsf(max_dischg_2.value.value_float);
        }
        // price indicates regular discharge
        else if ((price.value.value_float >= threshold_dischg_1.value.value_float) && (in.soc_avg_running > soc_min_limit.value.value_float)) {
            new_ess_kW_request = fabsf(max_dischg_1.value.value_float);
        }
        // price indicates max charge
        else if (price.value.value_float <= threshold_charge_2.value.value_float) {
            new_ess_kW_request = -1.0f * fabsf(max_charge_2.value.value_float);
        }
        // price indicate regular charge
        else if ((price.value.value_float <= threshold_charge_1.value.value_float) && (in.soc_avg_running < soc_max_limit.value.value_float)) {
            new_ess_kW_request = -1.0f * fabsf(max_charge_1.value.value_float);
        }

        // check for solar max potential kW; set to 0 if price is less than or equal to 0
        if (price.value.value_float <= 0) {
            new_solar_max_potential_kW = 0;
        } else {
            new_solar_max_potential_kW = in.solar_max_potential_kW;
        }

        // Solar uncurtailed
        new_solar_kW_request = new_solar_max_potential_kW;
    }

    // only modify the ess and solar data if the pricing was valid or we're alarming
    if (!in.is_alarming && invalid_pricing) {
        out.should_alarm = true;
    } else {
        out.solar_max_potential_kW = new_solar_max_potential_kW;
        // Feature output assigned to ESS
        out.ess_kW_request = new_ess_kW_request;
        // Solar fully uncurtailed, but reserve a portion equal to ESS charge request (negative feature output) if present
        out.solar_kW_request = new_solar_kW_request;
    }

    // This feature tracks load at a minimum
    out.load_method = LOAD_MINIMUM;
    out.additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(out.load_method, in.site_kW_load, in.site_kW_demand, out.ess_kW_request, in.gen_kW_request, out.solar_kW_request);
    out.site_kW_demand += out.additional_load_compensation;

    return out;
}

void features::Energy_Arbitrage::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        threshold_charge_1.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        threshold_charge_2.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        threshold_dischg_1.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        threshold_dischg_2.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        max_charge_1.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        max_charge_2.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        max_dischg_1.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        max_dischg_2.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        price.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
    }
}