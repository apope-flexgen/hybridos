/*
 * Asset_Cmd_Object.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cmath>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Site_Manager.h>
#include <Asset_Cmd_Object.h>
#include <Site_Controller_Utils.h>

Asset_Cmd_Object::Asset_Cmd_Object()
{
    load_buffer.resize(1);
    site_kW_load = 0.0f;
    site_kW_load_inclusion = false;
    site_kW_demand = 0.0f;
    site_kVAR_demand = 0.0f;
    total_potential_kVAR = 0.0f;
}

Asset_Cmd_Object::~Asset_Cmd_Object()
{
}

Asset_Cmd_Object::Asset_Cmd_Data::Asset_Cmd_Data()
{
    kW_cmd = 0.0;
    kW_request = 0.0f;
    actual_kW = 0.0;
    max_potential_kW = 0.0;
    min_potential_kW = 0.0;
    start_first_kW = 0.0;
    start_first_flag = false;

    kVAR_cmd = 0.0;
    actual_kVAR = 0.0;
    potential_kVAR = 0.0;
}

/**
 * Calculates how much this asset can handle as a source of power for the given command, then
 * assigns that value as its kW_cmd.
 * @param cmd The remaining active power commanded.
 * @return How much active power was assigned to this asset.
 */
float Asset_Cmd_Object::Asset_Cmd_Data::calculate_source_kW_contribution(float cmd)
{
    // calculate how much more power this asset type is available to contribute
    float unused_kW = calculate_unused_kW();

    // set bool to turn on asset if its needed
    start_first_flag = (cmd >= start_first_kW);
    
    // if asset is not needed or there is no potential discharge power, do not assign any more power to it
    if (cmd <= 0 || unused_kW <= 0)
        return 0;

    // temp variable for getting old/new kW_cmd delta
    float old_kW_cmd = kW_cmd;

    // ensure cmd does not exceed potential power for asset. assign lesser to asset kW_cmd and reduce cmd
    kW_cmd += (cmd > unused_kW) ? unused_kW : cmd;

    // return the change in kW_cmd AKA how much of cmd this asset type picked up
    return kW_cmd - old_kW_cmd;  
}

/**
 * Calculates the amount of active power that can still be assigned to the asset type without violating
 * its maximum power limit.
 * @return The amount of kW that can still be assigned without violating the maximum kW limit.
 */
float Asset_Cmd_Object::Asset_Cmd_Data::calculate_unused_kW()
{
    return zero_check(max_potential_kW - zero_check(kW_cmd));
}

/**
 * Calculates site_kW_load as the sum all active power values reported by assets
 * Also resets the feature load inclusion flag
 * 
 * Uses a rolling average based on a configurable window size to be more tolerant to (inaccurate) fluctations
 * in the actual published active power values
 */
void Asset_Cmd_Object::calculate_site_kW_load()
{
    // Sum all actual power from every asset type, including all feeders and add kW_baseload
    load_buffer[load_iterator++] = ess_data.actual_kW + gen_data.actual_kW + solar_data.actual_kW + feeder_data.actual_kW;
    load_iterator %= load_buffer.size();
    site_kW_load = std::accumulate(load_buffer.begin(), load_buffer.end(), 0.0f) / load_buffer.size();
    // Reset load inclusion
    site_kW_load_inclusion = false;
}

/**
 * Calculate site_kW_demand based on the output of the Active Power Feature
 * Features will set a site demand and or asset requests, as well as a flag indicating if load should be included in site power production
 * This function simply aggregates these values for convenience and flexibility in feature output
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 * 0 = solar, gen, ess, feeder. 1 = solar, feeder, ess, gen. 2 = gen, ess, solar, feeder.
 */
void Asset_Cmd_Object::calculate_feature_kW_demand(int asset_priority)
{
    site_kW_demand += ess_data.kW_request + gen_data.kW_request + solar_data.kW_request + site_kW_load_inclusion * site_kW_load;
    // Removed ess charge request if it must discharge to meet load, and disallow any net importing of power
    if (site_kW_load_inclusion && determine_ess_load_requirement(asset_priority))
        site_kW_demand = std::max(site_kW_demand - less_than_zero_check(ess_data.kW_request), 0.0f);
    
    feature_kW_demand = site_kW_demand;
}

//calculate total kVAR potential
void Asset_Cmd_Object::calculate_total_potential_kVAR()
{
    //sum all actual power from every asset type (not feeders)
    total_potential_kVAR = (ess_data.potential_kVAR + gen_data.potential_kVAR + solar_data.potential_kVAR);
}

/**
 * Extracts the total charge and discharge values that make up the site demand value expected at the POI.
 * For the charge production, all possible sources of discharge are removed from the site demand and
 * For the discharge production, all possible sources of charge are removed from the site demand.
 * To avoid double counting the effects of standalone features, which will modify the base site_kW_demand,
 * the net change of the standalone features is also taken and applied only to the appropriate side
 */
void Asset_Cmd_Object::calculate_site_kW_production_limits()
{
    // The asset requests and load were added into the feature demand, so make this step of extraction reference that demand
    // Charge production from removing all sources of discharge
    site_kW_charge_production = less_than_zero_check(feature_kW_demand - zero_check(ess_data.kW_request)
                                                                       - zero_check(gen_data.kW_request)
                                                                       - zero_check(solar_data.kW_request)
                                                                       - zero_check(site_kW_load_inclusion * site_kW_load));
    // Discharge production from removing all sources of charge
    site_kW_discharge_production = zero_check(feature_kW_demand - site_kW_charge_production);

    // Isolate demand modification from standalone features
    float standalone_modification = site_kW_demand - feature_kW_demand;
    // Further isolate any change past 0 into the opposite direction (these will be POI correcting features like watt-watt, CLC)
    float sign_change = 0.0f;
    // Apply the standalone modification to the appropriate production limit
    // Ensure that magnitude changes but sign is preserved
    if (feature_kW_demand < 0.0f || (feature_kW_demand == 0.0f && standalone_modification < 0.0f))
    {
        sign_change = zero_check(site_kW_charge_production + standalone_modification);
        site_kW_charge_production = less_than_zero_check(site_kW_charge_production + standalone_modification);
    }
    else
    {
        sign_change = less_than_zero_check(site_kW_discharge_production + standalone_modification);
        site_kW_discharge_production = zero_check(site_kW_discharge_production + standalone_modification);
    }
    // Apply any leftover sign change to the opposite production
    if (sign_change < 0.0f)
        site_kW_charge_production += sign_change;
    else if (sign_change > 0.0f)
        site_kW_discharge_production += sign_change;
}

/**
 * Distributes the site kW discharge command across assets that are available to supply a source of charge to ESS.
 * @param asset_priority Enumerated integer indicating the priority in which the various asset types should contribute to power requests.
 * TODO: Give this class a configuration function and store asset priority here instead of Site Manager
 * @param solar_source_flag True if solar assets are allowed to contribute to the charge request.
 * @param gen_source_flag True if generator assets are allowed to contribute to the charge request.
 * @param feeder_source_flag True if feeder assets are allowed to contribute to the charge request.
 * TODO: Move source flags to be stored here in Asset Cmd Obj as well
 * @return The total amount of power dispatched.
 */
float Asset_Cmd_Object::dispatch_site_kW_charge_cmd(int asset_priority, bool solar_source_flag, bool gen_source_flag, bool feeder_source_flag)
{
    // if ess is already discharging, there is no charge request, or ess cannot charge, or site demand is not negative: exit function
    if (ess_data.kW_cmd > 0.0f  || ess_data.min_potential_kW >= 0.0f || site_kW_charge_production >= 0.0f)
        return 0.0f;

    // temp variable for getting old/new kW_cmd delta
    float old_ess_cmd = ess_data.kW_cmd;

    // make a list of all asset types that are available to support fulfilling the ESS charge request
    std::vector<Asset_Cmd_Data*> exclusions = {&ess_data};
    if (!solar_source_flag)      exclusions.push_back(&solar_data);
    if (!gen_source_flag)        exclusions.push_back(&gen_data);
    if (!feeder_source_flag)     exclusions.push_back(&feeder_data);
    std::vector<Asset_Cmd_Data*> asset_list = list_assets_by_discharge_priority(asset_priority, exclusions);

    // limit the ESS charge request by the amount of power the ESS can handle and the amount of power other assets can contribute
    // to avoid frequent sign conversion when comparing against positive source values, the charge command is inverted here and used as a positive value
    float limited_cmd = std::min(zero_check(-1 * site_kW_charge_production), zero_check(-1 * ess_data.min_potential_kW) + ess_data.kW_cmd);
    limited_cmd = std::min(limited_cmd, aggregate_unused_kW(asset_list));

    for(auto asset_data : asset_list)
    {
        // if the charge request has been satisfied, quit iterating
        if (limited_cmd <= 0.0f)
            break;

        // if the current asset type cannot provide any power, skip to next one
        float charge_request_contribution = std::min(limited_cmd, asset_data->calculate_unused_kW());
        if (charge_request_contribution <= 0.0f) continue;

        // update commands/request with however much power the current asset type can contribute
        ess_data.kW_cmd -= charge_request_contribution;
        asset_data->kW_cmd += charge_request_contribution;
        limited_cmd -= charge_request_contribution;

        site_kW_discharge_production -= charge_request_contribution;
    }

    return ess_data.kW_cmd - old_ess_cmd;
}

/**
 * Distributes site kW discharge command to each asset type by priority order. Also sets start_first_asset_flag if conditions warrant it.
 * @param asset_priority Enumerated integer indicating the priority list of the asset types (i.e. solar, then gens, then ESSs, lastly feeders).
 * TODO: Give this class a configuration function and store asset priority here instead of Site Manager
 * @param cmd The power command to distribute to each asset
 * @return The total amount of power dispatched
 */
float Asset_Cmd_Object::dispatch_site_kW_discharge_cmd(int asset_priority, float cmd, discharge_type command_type)
{
    if (less_than_or_near(cmd, 0.0f, 0.001f))
        return 0.0f;

    std::vector<Asset_Cmd_Object::Asset_Cmd_Data*> asset_list = list_assets_by_discharge_priority(asset_priority);
    float received_cmd = cmd = std::min(std::min(cmd, site_kW_discharge_production), aggregate_unused_kW(asset_list));

    // Then calculate the active power contribution of each asset type
    for (auto it : asset_list)
    {
        // Contibute to kW_request and subtract it from command
        if (command_type == REQUESTS && it->kW_request >= 0)
        {
            float asset_request = std::min(it->kW_request, cmd);
            cmd -= it->calculate_source_kW_contribution(asset_request);
        }
        // Contribute to command directly (load, discharge_production demand)
        else
            // Only dispatch using the asset as a source if appropriate
            cmd -= it->calculate_source_kW_contribution(cmd);
    }

    // Reduce by and return the total power distributed
    site_kW_discharge_production -= received_cmd - cmd;
    return received_cmd - cmd;
}

//this function will reduce ess command to shift load off of POI when an asset cannot ramp down fast enough
float Asset_Cmd_Object::ess_overload_support(float grid_target_kW_cmd)
{
    float temp_ess_kW = ess_data.kW_cmd;  //used to calculate delta change
    //calculate the amount of power that is overloaded
    // TODO: change to solve load formula for feeder instead, then determine if beyond poi limits?
    float overload_kW = gen_data.min_potential_kW + solar_data.min_potential_kW + ess_data.kW_cmd - zero_check(site_kW_load - grid_target_kW_cmd);

    //calculate unused ess charge kW
    float unused_ess_kW = ess_data.kW_cmd - ess_data.min_potential_kW;
    //if overloaded, charge ess as much as possible to support
    if (overload_kW > 0.0f && unused_ess_kW > 0.0f)
        ess_data.kW_cmd -= std::min(overload_kW, unused_ess_kW);

    //FPS_DEBUG_LOG("OVERLOAD: %f, UNUSED: %f, DELTA: %f", overload_kW, unused_ess_kW, ess_data.kW_cmd - temp_ess_kW);
    return ess_data.kW_cmd - temp_ess_kW;
}

/**
 * TARGET SOC MODE passes through the charge control request (ess_data.kW_request) received from asset manager
 * If solar is present, a portion is set aside to satisfy the charge control request
 * @param load_requirement Whether load should be taken into account by site demand
 */
void Asset_Cmd_Object::target_soc_mode(bool load_requirement)
{
    // Solar uncurtailed
    solar_data.kW_request = solar_data.max_potential_kW;

    // Track load if appropriate
    site_kW_load_inclusion = load_requirement;
}

/**
 * SITE EXPORT TARGET MODE takes a single power command (the total site production, ignoring load) that is distributed to assets.
 * Positive commands are distributed to to all assets as needed, with priority based on the configured asset priority.
 * Negative commands are distributed to ESS as available.
 * If load is not taken into account, the value at the POI will fluctuate as load changes
 * @param load_enable_flag Whether site production should handle load compensation at a minimum
 * @param export_target_slew Slew rate for the feature
 * @param export_target_kW_cmd Single power command to be distributed to assets
 */
void Asset_Cmd_Object::site_export_target_mode(bool load_enable_flag, Slew_Object* export_target_slew, float export_target_kW_cmd)
{
    // Set site demand to be distributed based on dispatch availability and priority, bypassing load unless specified
    site_kW_demand = export_target_slew->get_slew_target(export_target_kW_cmd);

    // Track load if appropriate
    site_kW_load_inclusion = load_enable_flag;
}

/**
 * MANUAL MODE takes a solar kW cmd and ESS kW cmd and routes those commands through dispatch and charge control
 * Any site load should be offloaded to the feeder to guarantee the ESS/Solar commands
 * @param manual_ess_kW_cmd Command to be dispatched to ESS
 * @param manual_solar_kW_cmd Command to be dispatched to Solar
 */
void Asset_Cmd_Object::manual_mode(float manual_ess_kW_cmd, float manual_solar_kW_cmd)
{
    // Assign ess request, passing off limits check to dispatch
    ess_data.kW_request = manual_ess_kW_cmd;
    // Ensure that any extra available ESS is not used to service the site demand
    ess_data.max_potential_kW = std::min(ess_data.max_potential_kW, zero_check(ess_data.kW_request));

    // Assign positive solar request, passing off limits check to dispatch
    solar_data.kW_request = zero_check(manual_solar_kW_cmd);
    // Ensure that any extra available solar is not used to service the site demand
    solar_data.max_potential_kW = std::min(solar_data.max_potential_kW, solar_data.kW_request);
}

/**
 * GRID TARGET MODE accepts a command indicating the target value at the POI, and distributes a site demand to
 * all available assets based on the current load to ensure this target is met even as load fluctuates.
 * No ess or solar kW requests used. Assets simply respond to the site demand with the power they have available
 * Only export commands are allowed
 * @param grid_target_kW_cmd Site command to be distributed in addition to the load
 */
void Asset_Cmd_Object::grid_target_mode(float grid_target_kW_cmd)
{
    site_kW_demand = zero_check(-1.0f * grid_target_kW_cmd);
    // This feature tracks load in all cases
    site_kW_load_inclusion = true;
}

/**
 * Absolute ESS Mode takes an absolute value ESS command and assigns it as a charge/discharge based on the direction flag
 * provided. Solar is independently fully uncurtailed, but a portion is set aside for ESS charging as needed.
 * @param absolute_direction_flag Flag determining the charging direction. True if charging (default), false if discharging
 * @param absolute_ess_cmd The absolute value command to request
 */
void Asset_Cmd_Object::absolute_ess(bool absolute_direction_flag, float absolute_ess_cmd)
{
    // Apply sign based on provided flag (true: charge, false: discharge)
    ess_data.kW_request = fabsf(absolute_ess_cmd) * (absolute_direction_flag ? -1.0f : 1.0f);

    // Solar uncurtailed
    solar_data.kW_request = solar_data.max_potential_kW;
    // This feature does not track load
}

/**
 * ESS Calibration Mode routes a single ess cmd value equally to every ESS, with power distribution and soc-balancing
 * disabled to ensure that the value commanded by the feature is the value of each ESS.
 * @param ess_calibration_cmd Command to be routed to each ESS
 * @param num_ess_controllable The number of ESS available within Site Manager
 */
void Asset_Cmd_Object::ess_calibration_mode(float ess_calibration_kW_cmd, int num_ess_controllable)
{
    // Multiple by the number of ESS available to get the total power that should be distributed
    float total_feature_cmd = ess_calibration_kW_cmd * num_ess_controllable;
    // Provide at least enough potential to meet the commannd, as the potentials can limit the command due to a shared slew target but unbalanced SoCs
    // Commands will still be limited safely by the amount of (dis)chargeable power available
    set_ess_max_potential_kW(std::max(total_feature_cmd, get_ess_max_potential_kW()));
    set_ess_min_potential_kW(std::min(total_feature_cmd, get_ess_min_potential_kW()));

    ess_data.kW_request = total_feature_cmd;
    // This feature does not track load
}

//dispatch reactive power cmds
void Asset_Cmd_Object::dispatch_reactive_power()
{
    //no kVAR cmds if not potential or demanded
    if (total_potential_kVAR == 0 || site_kVAR_demand == 0)
        return;

    //initialize the percentage of potential to be commanded
    float percent_cmd = (std::signbit(site_kVAR_demand)) ? -1.0 : 1.0;

    //potential exceeds demand
    if (total_potential_kVAR > fabsf(site_kVAR_demand))
        percent_cmd *= fabsf(site_kVAR_demand) / total_potential_kVAR;

    ess_data.kVAR_cmd = percent_cmd * ess_data.potential_kVAR;
    solar_data.kVAR_cmd = percent_cmd * solar_data.potential_kVAR;
    gen_data.kVAR_cmd = percent_cmd * gen_data.potential_kVAR;
}

//reactive setpoint mode algorithm
void Asset_Cmd_Object::reactive_setpoint_mode(Slew_Object* reactive_setpoint_slew, float reactive_setpoint_kVAR_cmd)
{
    float kVAR_request = reactive_setpoint_slew->get_slew_target(reactive_setpoint_kVAR_cmd);

    site_kVAR_demand = std::min(total_potential_kVAR, fabsf(kVAR_request));
    site_kVAR_demand *= (std::signbit(kVAR_request)) ? -1 : 1;
}

//active voltage regulation mode algorithm
void Asset_Cmd_Object::active_voltage_mode(float deadband, float cmd, float actual_volts, float droop_percent, float rated_kVAR)
{
    //calculate the requested power due to voltage deviation
    //get correct reference_volts for overvolts or undervolts event (nominal volts + or - deadband)
    float reference_volts = (actual_volts > cmd) ? cmd + deadband : cmd - deadband;

    float delta_volts; //delta voltage deviation

    //delta calculation for overvoltage event (negative delta)
    if (actual_volts > cmd)
        delta_volts = (actual_volts > reference_volts) ? reference_volts - actual_volts : 0;

    //delta calculation for undervoltage event (positive delta)
    else
        delta_volts = (actual_volts < reference_volts) ? reference_volts - actual_volts : 0;

    //delta voltage deviation scaled by percent slope and nominal frequency
    float kVAR_request = (delta_volts / cmd) / (.01 * droop_percent) * rated_kVAR;

    site_kVAR_demand = std::min(total_potential_kVAR, fabsf(kVAR_request));
    site_kVAR_demand *= (std::signbit(kVAR_request)) ? -1 : 1;
}

//return ess_kW_cmd limited by min and max potential KW from asset manager
float Asset_Cmd_Object::get_limited_ess_kW_cmd()
{
    return range_check(ess_data.kW_cmd, ess_data.max_potential_kW, ess_data.min_potential_kW);
}

//return feeder_kW_cmd limited by min and max potential KW from asset manager
float Asset_Cmd_Object::get_limited_feeder_kW_cmd()
{
    return range_check(feeder_data.kW_cmd, feeder_data.max_potential_kW, feeder_data.min_potential_kW);
}

//return gen_kW_cmd limited by min and max potential KW from asset manager
float Asset_Cmd_Object::get_limited_gen_kW_cmd()
{
    return range_check(gen_data.kW_cmd, gen_data.max_potential_kW, gen_data.min_potential_kW);
}

//return solar_kW_cmd limited by min and max potential KW from asset manager
float Asset_Cmd_Object::get_limited_solar_kW_cmd()
{
    return range_check(solar_data.kW_cmd, solar_data.max_potential_kW, solar_data.min_potential_kW);
}

//return site_kW_demand
float Asset_Cmd_Object::get_site_kW_demand()
{
    return site_kW_demand;
}

//return site_kW_load
float Asset_Cmd_Object::get_site_kW_load()
{
    return site_kW_load;
}

//return site_kW_load_inclusion
bool Asset_Cmd_Object::get_site_kW_load_inclusion()
{
    return site_kW_load_inclusion;
}

//return feature_kW_demand
float Asset_Cmd_Object::get_feature_kW_demand()
{
    return feature_kW_demand;
}

//return site_kW_charge_production
float Asset_Cmd_Object::get_site_kW_charge_production()
{
    return site_kW_charge_production;
}

//return site_kW_discharge_production
float Asset_Cmd_Object::get_site_kW_discharge_production()
{
    return site_kW_discharge_production;
}

//return site_kVAR_demand
float Asset_Cmd_Object::get_site_kVAR_demand()
{
    return site_kVAR_demand;
}

//return ess_data.kW_request
float Asset_Cmd_Object::get_ess_kW_request()
{
    return ess_data.kW_request;
}

//return gen_data.kW_request
float Asset_Cmd_Object::get_gen_kW_request()
{
    return gen_data.kW_request;
}

//return solar_data.kW_request
float Asset_Cmd_Object::get_solar_kW_request()
{
    return solar_data.kW_request;
}


//set site_kW_demand
void Asset_Cmd_Object::set_site_kW_demand(float value)
{
    site_kW_demand = value;
}

//set site_kW_load
void Asset_Cmd_Object::set_site_kW_load(float value)
{
    site_kW_load = value;
}

//set site_kW_load_buffer_size
void Asset_Cmd_Object::create_site_kW_load_buffer(int size)
{
    load_buffer.clear();
    // Set minimum size of 1 iteration (10ms)
    load_buffer.resize(size == 0? 1 : size);
    load_iterator = 0;
}

//set site_kW_load_inclusion
void Asset_Cmd_Object::set_site_kW_load_inclusion(bool value)
{
    site_kW_load_inclusion = value;
}

//set feature_kW_demand
void Asset_Cmd_Object::set_feature_kW_demand(float value)
{
    feature_kW_demand = value;
}

//set site_kW_charge_production
void Asset_Cmd_Object::set_site_kW_charge_production(float value)
{
    site_kW_charge_production = value;
}

//set site_kW_discharge_production
void Asset_Cmd_Object::set_site_kW_discharge_production(float value)
{
    site_kW_discharge_production = value;
}

//set site_kVAR_demand
void Asset_Cmd_Object::set_site_kVAR_demand(float value)
{
    site_kVAR_demand = value;
}

//set ess_data.kW_request
void Asset_Cmd_Object::set_ess_kW_request(float value)
{
    ess_data.kW_request = value;
}

//set gen_data.kW_request
void Asset_Cmd_Object::set_gen_kW_request(float value)
{
    gen_data.kW_request = value;
}

//set solar_data.kW_request
void Asset_Cmd_Object::set_solar_kW_request(float value)
{
    solar_data.kW_request = value;
}

//return ess_kW_cmd
float Asset_Cmd_Object::get_ess_kW_cmd()
{
    return ess_data.kW_cmd;
}

//return feeder_kW_cmd
float Asset_Cmd_Object::get_feeder_kW_cmd()
{
    return feeder_data.kW_cmd;
}

//return gen_kW_cmd
float Asset_Cmd_Object::get_gen_kW_cmd()
{
    return gen_data.kW_cmd;
}

//return solar_kW_cmd
float Asset_Cmd_Object::get_solar_kW_cmd()
{
    return solar_data.kW_cmd;
}

//return ess_actual_kW
float Asset_Cmd_Object::get_ess_actual_kW()
{
    return ess_data.actual_kW;
}

//return feeder_actual_kW
float Asset_Cmd_Object::get_feeder_actual_kW()
{
    return feeder_data.actual_kW;
}

//return gen_actual_kW
float Asset_Cmd_Object::get_gen_actual_kW()
{
    return gen_data.actual_kW;
}

//return solar_actual_kW
float Asset_Cmd_Object::get_solar_actual_kW()
{
    return solar_data.actual_kW;
}

//return ess_max_potential_kW
float Asset_Cmd_Object::get_ess_max_potential_kW()
{
    return ess_data.max_potential_kW;
}

//return feeder_max_potential_kW
float Asset_Cmd_Object::get_feeder_max_potential_kW()
{
    return feeder_data.max_potential_kW;
}

//return gen_max_potential_kW
float Asset_Cmd_Object::get_gen_max_potential_kW()
{
    return gen_data.max_potential_kW;
}

//return solar_max_potential_kW
float Asset_Cmd_Object::get_solar_max_potential_kW()
{
    return solar_data.max_potential_kW;
}

//return ess_min_potential_kW
float Asset_Cmd_Object::get_ess_min_potential_kW()
{
    return ess_data.min_potential_kW;
}

//return feeder_min_potential_kW
float Asset_Cmd_Object::get_feeder_min_potential_kW()
{
    return feeder_data.min_potential_kW;
}

//return gen_min_potential_kW
float Asset_Cmd_Object::get_gen_min_potential_kW()
{
    return gen_data.min_potential_kW;
}

//return solar_min_potential_kW
float Asset_Cmd_Object::get_solar_min_potential_kW()
{
    return solar_data.min_potential_kW;
}

//return ess_kVAR_cmd
float Asset_Cmd_Object::get_ess_kVAR_cmd()
{
    return ess_data.kVAR_cmd;
}

//return gen_kVAR_cmd
float Asset_Cmd_Object::get_gen_kVAR_cmd()
{
    return gen_data.kVAR_cmd;
}

//return solar_kVAR_cmd
float Asset_Cmd_Object::get_solar_kVAR_cmd()
{
    return solar_data.kVAR_cmd;
}

//return ess start_first_flag
bool Asset_Cmd_Object::get_ess_start_first_flag()
{
    return ess_data.start_first_flag;
}

//return gen start_first_flag
bool Asset_Cmd_Object::get_gen_start_first_flag()
{
    return gen_data.start_first_flag;
}

//return solar start_first_flag
bool Asset_Cmd_Object::get_solar_start_first_flag()
{
    return solar_data.start_first_flag;
}

//set ess_kW_cmd
void Asset_Cmd_Object::set_ess_kW_cmd(float value)
{
    ess_data.kW_cmd = value;
}

//set feeder_kW_cmd
void Asset_Cmd_Object::set_feeder_kW_cmd(float value)
{
    feeder_data.kW_cmd = value;
}

//set gen_kW_cmd
void Asset_Cmd_Object::set_gen_kW_cmd(float value)
{
    gen_data.kW_cmd = value;
}

//set solar_kW_cmd
void Asset_Cmd_Object::set_solar_kW_cmd(float value)
{
    solar_data.kW_cmd = value;
}

//set ess_actual_kW
void Asset_Cmd_Object::set_ess_actual_kW(float value)
{
    ess_data.actual_kW = value;
}

//set feeder_actual_kW
void Asset_Cmd_Object::set_feeder_actual_kW(float value)
{
    feeder_data.actual_kW = value;
}

//set gen_actual_kW
void Asset_Cmd_Object::set_gen_actual_kW(float value)
{
    gen_data.actual_kW = value;
}

//set solar_actual_kW
void Asset_Cmd_Object::set_solar_actual_kW(float value)
{
    solar_data.actual_kW = value;
}

//set ess_actual_kVAR
void Asset_Cmd_Object::set_ess_actual_kVAR(float value)
{
    ess_data.actual_kVAR = value;
}

//set gen_actual_kVAR
void Asset_Cmd_Object::set_gen_actual_kVAR(float value)
{
    gen_data.actual_kVAR = value;
}

//set solar_actual_kVAR
void Asset_Cmd_Object::set_solar_actual_kVAR(float value)
{
    solar_data.actual_kVAR = value;
}

//set ess_max_potential_kW
void Asset_Cmd_Object::set_ess_max_potential_kW(float value)
{
    ess_data.max_potential_kW = value;
}

//set feeder_max_potential_kW
void Asset_Cmd_Object::set_feeder_max_potential_kW(float value)
{
    feeder_data.max_potential_kW = value;
}

//set gen_max_potential_kW
void Asset_Cmd_Object::set_gen_max_potential_kW(float value)
{
    gen_data.max_potential_kW = value;
}

//set solar_max_potential_kW
void Asset_Cmd_Object::set_solar_max_potential_kW(float value)
{
    solar_data.max_potential_kW = value;
}

//set ess_min_potential_kW
void Asset_Cmd_Object::set_ess_min_potential_kW(float value)
{
    ess_data.min_potential_kW = value;
}

//set feeder_min_potential_kW
void Asset_Cmd_Object::set_feeder_min_potential_kW(float value)
{
    feeder_data.min_potential_kW = value;
}

//set gen_min_potential_kW
void Asset_Cmd_Object::set_gen_min_potential_kW(float value)
{
    gen_data.min_potential_kW = value;
}

//set solar_min_potential_kW
void Asset_Cmd_Object::set_solar_min_potential_kW(float value)
{
    solar_data.min_potential_kW = value;
}

//set ess_kVAR_cmd
void Asset_Cmd_Object::set_ess_kVAR_cmd(float value)
{
    ess_data.kVAR_cmd = value;
}

//set gen_kVAR_cmd
void Asset_Cmd_Object::set_gen_kVAR_cmd(float value)
{
    gen_data.kVAR_cmd = value;
}

//set solar_kVAR_cmd
void Asset_Cmd_Object::set_solar_kVAR_cmd(float value)
{
    solar_data.kVAR_cmd = value;
}

//set ess_potential_kVAR
void Asset_Cmd_Object::set_ess_potential_kVAR(float value)
{
    ess_data.potential_kVAR = value;
}

//set gen_potential_kVAR
void Asset_Cmd_Object::set_gen_potential_kVAR(float value)
{
    gen_data.potential_kVAR = value;
}

//set solar_potential_kVAR
void Asset_Cmd_Object::set_solar_potential_kVAR(float value)
{
    solar_data.potential_kVAR = value;
}

//set ess_data.start_first_kW
void Asset_Cmd_Object::set_ess_start_first_kW(float value)
{
    ess_data.start_first_kW = value;
}

//set gen_data.start_first_kW
void Asset_Cmd_Object::set_gen_start_first_kW(float value)
{
    gen_data.start_first_kW = value;
}

//set solar_data.start_first_kW
void Asset_Cmd_Object::set_solar_start_first_kW(float value)
{
    solar_data.start_first_kW = value;
}

//set ess_data.start_first_flag
void Asset_Cmd_Object::set_ess_start_first_flag(bool value)
{
    ess_data.start_first_flag = value;
}

//set gen_data.start_first_flag
void Asset_Cmd_Object::set_gen_start_first_flag(bool value)
{
    gen_data.start_first_flag = value;
}

//set solar_data.start_first_flag
void Asset_Cmd_Object::set_solar_start_first_flag(bool value)
{
    solar_data.start_first_flag = value;
}

//reset kW cmd to 0 each asset type
void Asset_Cmd_Object::reset_kW_cmd()
{
    ess_data.kW_cmd = 0.0f;
    feeder_data.kW_cmd = 0.0f;
    gen_data.kW_cmd = 0.0f;
    solar_data.kW_cmd = 0.0f;
    ess_data.kW_request = 0.0f;
    gen_data.kW_request = 0.0f;
    solar_data.kW_request = 0.0f;
    site_kW_demand = 0.0f;
    feature_kW_demand = 0.0f;
    site_kW_charge_production = 0.0f;
    site_kW_discharge_production = 0.0f;
}

//reset kVAR cmd to 0 each asset type
void Asset_Cmd_Object::reset_kVAR_cmd()
{
    ess_data.kVAR_cmd = 0.0;
    gen_data.kVAR_cmd = 0.0;
    solar_data.kVAR_cmd = 0.0;
}

/**
 * Builds a list of the asset data objects in the order of the given priority
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 * @param exclusions Any assets that should be excluded from the list. Defaults to empty with no exclusions
 * 0 = solar, gen, ess, feeder. 1 = solar, feeder, ess, gen. 2 = gen, ess, solar, feeder.
 * @return List of pointers to the asset data objects ordered by priority.
 */
std::vector<Asset_Cmd_Object::Asset_Cmd_Data*> Asset_Cmd_Object::list_assets_by_discharge_priority(int asset_priority, std::vector<Asset_Cmd_Data*> exclusions)
{
    // Construct the basic list based on priority
    std::vector<Asset_Cmd_Data*> asset_list;
    switch(asset_priority)
    {
    case solar_gen_ess_feeder:  asset_list =  {&solar_data, &gen_data, &ess_data, &feeder_data}; break;
    case solar_feeder_ess_gen:  asset_list =  {&solar_data, &feeder_data, &ess_data, &gen_data}; break;
    case gen_ess_solar_feeder:  asset_list =  {&gen_data, &ess_data, &solar_data, &feeder_data}; break;
    case gen_solar_ess_feeder:  asset_list =  {&gen_data, &solar_data, &ess_data, &feeder_data}; break;
    case ess_solar_gen_feeder:  asset_list =  {&ess_data, &solar_data, &gen_data, &feeder_data}; break;
    default:                    asset_list =  {&ess_data, &solar_data, &feeder_data, &gen_data}; break;
    }
    // And remove any assets that should be excluded
    remove_asset_types(asset_list, exclusions);
    return asset_list;
}

/**
 * Removes the given asset types from a list of asset data objects.
 * @param data_list List of asset data objects.
 * @param asset_types Pointer to asset data objects that should be removed.
 */
void Asset_Cmd_Object::remove_asset_types(std::vector<Asset_Cmd_Data*> &data_list, const std::vector<Asset_Cmd_Data*> asset_types)
{
    for (auto asset_type : asset_types)
        data_list.erase(std::find(data_list.begin(), data_list.end(), asset_type));
}

/**
 * Calculates how much unused active power there is across a list of asset data objects.
 * @param data_list List of asset data objects.
 * @return The summation of all unused kW across each asset data object.
 */
float Asset_Cmd_Object::aggregate_unused_kW(std::vector<Asset_Cmd_Object::Asset_Cmd_Data*> const &data_list)
{
    float total_unused_kW = 0;
    for(auto asset_data : data_list)
    {
        total_unused_kW += asset_data->calculate_unused_kW();
    }
    return total_unused_kW;
}

/**
 * Determine if ESS is required to compensate for load
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 * 0 = solar, gen, ess, feeder. 1 = solar, feeder, ess, gen. 2 = gen, ess, solar, feeder.
 * @return The import that will not fall on the feeder
 */
bool Asset_Cmd_Object::determine_ess_load_requirement(int asset_priority)
{
    std::vector<Asset_Cmd_Data*> site_assets_list = list_assets_by_discharge_priority(asset_priority);
    float uncompensated_load = site_kW_load;
    for(auto asset_data : site_assets_list)
    {
        if (uncompensated_load <= 0)
            break;

        uncompensated_load -= asset_data->calculate_unused_kW();
        
        if (asset_data == &ess_data)
            return true;
    }
    return false;
}

/**
 * Calculates the amount of unused power that can be produced internally for load without importing from POI
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 * 0 = solar, gen, ess, feeder. 1 = solar, feeder, ess, gen. 2 = gen, ess, solar, feeder.
 * @return The import that will not fall on the feeder
 */
float Asset_Cmd_Object::calculate_internal_load_compensation(int asset_priority)
{
    std::vector<Asset_Cmd_Data*> site_assets_list = list_assets_by_discharge_priority(asset_priority, {&feeder_data});
    return aggregate_unused_kW(site_assets_list);
}

/**
 * Calculates the amount of unused power that can be produced internally for charge without importing from the POI
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 * 0 = solar, gen, ess, feeder. 1 = solar, feeder, ess, gen. 2 = gen, ess, solar, feeder.
 * @return The import that will not fall on feeder or ess
 */
float Asset_Cmd_Object::calculate_internal_charge_compensation(int asset_priority)
{
    std::vector<Asset_Cmd_Data*> site_assets_list = list_assets_by_discharge_priority(asset_priority, {&ess_data, &feeder_data});
    return aggregate_unused_kW(site_assets_list);
}
