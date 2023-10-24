/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Generator_Charge.h>
#include <Features/Solar_Shed.h>
#include <Site_Controller_Utils.h>

features::Generator_Charge::Generator_Charge() {
    feature_vars = {
        &generator_charge_additional_buffer,
    };

    variable_ids = {
        { &enable_flag, "generator_charge_enable" },
        { &generator_charge_additional_buffer, "generator_charge_additional_buffer" },
    };
}

void features::Generator_Charge::execute(Asset_Cmd_Object& asset_cmd, const features::Solar_Shed& solar_shed, float& max_potential_gen_kW) {
    External_Inputs inputs{
        solar_shed.solar_shed_calculator.offset, solar_shed.solar_shed_calculator.min_offset, solar_shed.solar_shed_high_threshold.value.value_float, asset_cmd.solar_data.max_potential_kW, asset_cmd.site_kW_load,
        asset_cmd.ess_data.min_potential_kW,     asset_cmd.gen_data.max_potential_kW,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.gen_data.max_potential_kW = outputs.asset_cmd_gen_max_potential_kW;
    max_potential_gen_kW = outputs.site_manager_max_potential_gen_kW;
}

features::Generator_Charge::External_Outputs features::Generator_Charge::execute_helper(const External_Inputs& inputs) {
    // ESS should be controlled by Solar + Gen, leaving enough room for solar to always increase (high threshold) if it wants
    // Determine if solar needs room to increase (remember min_offset = no shedding = full solar output)
    float solar_additional_buffer = (inputs.solar_shed_calculator_offset == inputs.solar_shed_calculator_min_offset) ? 0.0f : inputs.solar_shed_high_threshold;
    // Determine how much solar power will actually go to the ESS, and not load
    float actual_solar_compensation = inputs.solar_max_potential_kW - inputs.site_kW_load;
    // Determine the ESS's unsatisfied charge capability that can be handled by the generator
    float available_ess_after_solar = zero_check(zero_check(-1.0f * inputs.ess_min_potential_kW) - actual_solar_compensation);
    // Further reduce this value by the buffers
    float calc_gen_limit = zero_check(available_ess_after_solar - solar_additional_buffer - generator_charge_additional_buffer.value.value_float);
    // Take the final value as the command, and make sure gen power does not exceed this value
    float max_gen_limit = std::min(calc_gen_limit, inputs.gen_max_potential_kW);
    float new_asset_cmd_gen_max_potential_kW = max_gen_limit;
    float new_site_manager_max_potential_gen_kW = max_gen_limit;

    return External_Outputs{
        new_asset_cmd_gen_max_potential_kW,
        new_site_manager_max_potential_gen_kW,
    };
}
