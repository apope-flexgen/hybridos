/*
 * Asset_Cmd_Object.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cmath>
#include <numeric>
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
    load_method = NO_COMPENSATION;
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
    auto_restart_flag = true; // Allow asset to start unless explicitly told not to. (see runmode2 BESS fault)

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
    float unused_kW = calculate_unused_dischg_kW();

    // set bool to turn on asset if its needed
    start_first_flag = ((cmd >= start_first_kW) && auto_restart_flag);
    
    // if asset is not needed or there is no potential discharge power, do not assign any more power to it
    if (cmd <= 0 || unused_kW <= 0)
        return 0;

    // temp variable for getting old/new kW_cmd delta
    float old_kW_cmd = kW_cmd;

    // ensure cmd does not exceed potential power for asset. assign lesser to asset kW_cmd and reduce cmd
    kW_cmd += (cmd > unused_kW) ? unused_kW : cmd;

    // return the change in kW_cmd AKA how much of cmd this asset type picked up
    float asset_contribution = kW_cmd - old_kW_cmd;

    // if the asset requested to discharge, reduce its discharge request by its contribution as well
    if (kW_request > 0)
        kW_request = zero_check(kW_request - asset_contribution);

    return asset_contribution;  
}

/**
 * Calculates the amount of active power that can still be assigned to the asset type for charge without violating
 * its minimum power limit.
 * @return The amount of kW that can still be assigned without violating the minimum kW limit.
 */
float Asset_Cmd_Object::Asset_Cmd_Data::calculate_unused_charge_kW()
{
    if (kW_cmd > 0)
        return 0.0f;

    return less_than_zero_check(min_potential_kW - kW_cmd);
}

/**
 * Calculates the amount of active power that can still be assigned to the asset type for discharge without violating
 * its maximum power limit.
 * @return The amount of kW that can still be assigned without violating the maximum kW limit.
 */
float Asset_Cmd_Object::Asset_Cmd_Data::calculate_unused_dischg_kW()
{
    if (kW_cmd < 0)
        return 0.0f;

    return zero_check(max_potential_kW - kW_cmd);
}

/**
 * Calculates the expected POI value based on the commands dispatched.
 * This includes feeder "discharge" i.e. import to handle load or charge
 * @return the net production of the site that will be seen at the POI
 */
float Asset_Cmd_Object::calculate_net_poi_export()
{
    return ess_data.kW_cmd + gen_data.kW_cmd + solar_data.kW_cmd + feeder_data.kW_cmd - (get_site_kW_load_inclusion() * site_kW_load);
}

/**
 * Calculates site_kW_load as the sum all active power values reported by assets
 * Also resets the feature load inclusion flag
 * 
 * Uses a rolling average based on a configurable window size to be more tolerant to (inaccurate) fluctuations
 * in the actual published active power values
 */
void Asset_Cmd_Object::calculate_site_kW_load()
{
    // Sum all actual power from every asset type, including all feeders and add kW_baseload
    load_buffer[load_iterator++] = ess_data.actual_kW + gen_data.actual_kW + solar_data.actual_kW + feeder_data.actual_kW;
    load_iterator %= load_buffer.size();
    site_kW_load = std::accumulate(load_buffer.begin(), load_buffer.end(), 0.0f) / load_buffer.size();
    // Reset load inclusion
    load_method = NO_COMPENSATION;
}

/**
 * After the Active Power Feature has set its optional asset requests and demand, aggregate the total discharge requested
 * and determine if any additional discharge is needed for load compensation
 */
float Asset_Cmd_Object::calculate_additional_load_compensation()
{
    // Load is outside of site control, so additional compensation is unneeded
    if (load_method == NO_COMPENSATION)
        return 0.0f;

    // Calculate the amount of outstanding load based on the type of load compensation being used
    if (load_method == LOAD_OFFSET)
        return site_kW_load;

    // Load is a minimum. Reduce the amount of additional discharge needed by the amount already being produced
    // Take the aggregate of the demand (generic discharge) and any additional asset discharge requests as the total discharge
    float feature_discharge_request = zero_check(site_kW_demand)
                                    + zero_check(ess_data.kW_request)
                                    + zero_check(gen_data.kW_request)
                                    + zero_check(solar_data.kW_request);
    // Return the amount of additional discharge needed to handle load
    return zero_check(site_kW_load - feature_discharge_request);
}

/**
 * Add load to demand based on the method of compensation provided. This method should only be used for active power features
 * that do not utilize their own slew rates. This is because adding the load to the demand after slewing can cause a runaway
 * effect. Export Target is the only feature with this issue currently.
 * @param method_of_compensation The method of load compensation
 */
void Asset_Cmd_Object::track_unslewed_load(load_compensation method_of_compensation)
{
    load_method = load_compensation(method_of_compensation);
    // Calculate if any additional load compensation is needed from the site
    additional_load_compensation = calculate_additional_load_compensation();
    site_kW_demand += additional_load_compensation;
}

/**
 * Extract load from demand based on the method of compensation provided. This method should only be used for active power features
 * that utilize their own slew rates. This is because these features already track load manually as part of their slewed input,
 * producing a variable output that is not necessarily equal to the measured load. Export Target is the only feature meeting this
 * criteria currently.
 * @param method_of_compensation The method of load compensation
 * @param unslewed_demand The aggregate (unslewed) demand entered into the feature slew
 * @param additional_load Additional (unslewed) load that was required of the feature
 * @param feature_slew The slew object used by the feature
 */
void Asset_Cmd_Object::track_slewed_load(load_compensation method_of_compensation, float unslewed_demand, float additional_load, Slew_Object& feature_slew)
{
    load_method = load_compensation(method_of_compensation);
    // No additional compensation was added
    if (load_method == NO_COMPENSATION)
        return;

    // Handle the case where compensation and charge cancelled out
    if (site_kW_demand == 0.0f)
    {
        // In this case we can simply assume that charge = slew min and demand = slew max
        additional_load_compensation = std::min(additional_load, feature_slew.get_max_target());
    }
    else
    {
        // Calculate how much the slew divided the demand
        float slew_divisor = std::abs(unslewed_demand / site_kW_demand);
        // Divide load by this value to get the slewed load compensation
        additional_load_compensation = additional_load / slew_divisor;
    }
}

/**
 * Calculates site_kW_demand based on the output of the Active Power Feature. Features will set a site demand and/or asset requests,
 * which this function aggregate into two demand values, feature demand which will preserve the unmodified feature demand, and site
 * demand, which will be modified by the following standalone power features.
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 */
void Asset_Cmd_Object::calculate_feature_kW_demand(int asset_priority)
{
    // Aggregate all sources of power generated by the feature and load
    feature_kW_demand = site_kW_demand += ess_data.kW_request + gen_data.kW_request + solar_data.kW_request;
    // Removed ess charge request if it must discharge to meet load, and disallow any net importing of power
    determine_ess_load_requirement(asset_priority);
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
    // First find all sources of discharge, including load compensation if it must be met at a MINIMUM only
    // TODO: for the OFFSET use case we would need to look at the potentials to determine whether to reduce charge or increase discharge
    // Instead this assumes that the assets have already requested to discharge as much as they want (Solar)
    float requested_discharge = zero_check(ess_data.kW_request) + zero_check(gen_data.kW_request) + zero_check(solar_data.kW_request);
    if (load_method == LOAD_MINIMUM)
        requested_discharge += additional_load_compensation;
    // Charge production from removing all sources of discharge
    site_kW_charge_production = less_than_zero_check(feature_kW_demand - requested_discharge);
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
    // if ess is already discharging, there is no charge request, or ess cannot charge, exit function
    if (ess_data.kW_cmd > 0.0f || ess_data.min_potential_kW >= 0.0f || site_kW_charge_production >= 0.0f)
    {
        // Clear site demand and any charge requests as they cannot be handled
        site_kW_demand -= less_than_zero_check(site_kW_charge_production);
        site_kW_charge_production = 0.0f;
        ess_data.kW_request -= less_than_zero_check(ess_data.kW_request);
        return 0.0f;
    }

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
    float ess_limited_request = std::min(zero_check(-1 * site_kW_charge_production), zero_check(-1 * ess_data.calculate_unused_charge_kW()));
    float dispatch_charge_remainder = std::min(ess_limited_request, aggregate_unused_kW(asset_list));
    // Keep values in sync for lookahead dispatch
    site_kW_charge_production += dispatch_charge_remainder;
    // If we aren't able to charge as much as requested, reconcile by reducing the discharge production to maintain the expected POI value.
    // A charge remainder will only be present if the charge request is larger than the total available discharge production. This ensures that
    // every asset has already had a chance to discharge (except ESS, which wanted to charge) so it's safe to reduce the discharge production
    site_kW_discharge_production -= ess_limited_request;
    // Increase demand by any unserviced charge in order to maintain the expected target
    site_kW_demand += ess_limited_request - dispatch_charge_remainder;

    for(auto asset_data : asset_list)
    {
        // if the charge request has been satisfied, quit iterating
        if (dispatch_charge_remainder <= 0.0f)
            break;

        // if the current asset type cannot provide any power, skip to next one
        float charge_request_contribution = std::min(dispatch_charge_remainder, asset_data->calculate_unused_dischg_kW());
        if (charge_request_contribution <= 0.0f) continue;

        // update commands/request with however much power the current asset type can contribute
        ess_data.kW_cmd -= charge_request_contribution;
        asset_data->kW_cmd += charge_request_contribution;
        dispatch_charge_remainder -= charge_request_contribution;
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
        // Contribute to kW_request and subtract it from the command received
        if (command_type == REQUESTS)
        {
            // Only contribute power from this asset if it has a discharge request
            if (it->kW_request > 0)
            {
                float asset_request = std::min(it->kW_request, cmd);
                cmd -= it->calculate_source_kW_contribution(asset_request);
            }
        }
        // Contribute to the received command directly (load, discharge_production demand)
        else if (command_type == LOAD || command_type == DEMAND)
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

    // Track load as a demand minimum if appropriate
    track_unslewed_load(load_compensation(load_requirement * LOAD_MINIMUM));
}

/**
 * ACTIVE POWER SETPOINT MODE takes a single power command (the total site production, ignoring load) that is distributed to assets.
 * Positive commands are distributed to to all assets as needed, with priority based on the configured asset priority.
 * Negative commands are distributed to ESS as available.
 * If load is not taken into account, the value at the POI will fluctuate as load changes.
 * Load must be handled manually by this feature due to its feature level slew
 * kW_cmd can be sent using `absolute_mode` and `direction` to determine sign. false by default.
 * @param kW_cmd Single power command to be distributed to assets
 * @param slew_rate Slew rate for the feature
 * @param load_method Whether site production should handle load compensation minimum, offset, or not at all.
 * @param absolute_mode_flag Flag determining whether kW_cmd should be interpreted as absolute value.
 * @param direction_flag Flag used if absolute mode == true; kW_cmd is negative if true, positive if false.
 * @param maximize_solar Flag used to maximize solar kW_cmd always.
 */
void Asset_Cmd_Object::active_power_setpoint(
    float kW_cmd,
    Slew_Object* slew_rate,
    load_compensation load_strategy,
    bool absolute_mode_flag,
    bool direction_flag,
    bool maximize_solar
) {
    if (absolute_mode_flag) {
        kW_cmd = fabsf(kW_cmd) * (direction_flag ? -1 : 1);
    }

    // Uncurtailed solar
    if (maximize_solar) {
        solar_data.kW_request = solar_data.max_potential_kW;
    }

    // Setup desired command at the POI
    poi_cmd = kW_cmd;

    load_method = load_strategy;
    // Load must be included in the reference command and routed through the slewed target
    additional_load_compensation = calculate_additional_load_compensation();
    kW_cmd += additional_load_compensation;
    site_kW_demand = slew_rate->get_slew_target(kW_cmd);
    track_slewed_load(load_method, kW_cmd, additional_load_compensation, *slew_rate);
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
    
    // No load compensation
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
    // Provide at least enough potential to meet the command, as the potentials can limit the command due to a shared slew target but unbalanced SoCs
    // Commands will still be limited safely by the amount of (dis)chargeable power available
    ess_data.max_potential_kW = (std::max(total_feature_cmd, ess_data.max_potential_kW));
    ess_data.min_potential_kW = (std::min(total_feature_cmd, ess_data.min_potential_kW));

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

/**
 * Constant Power Factor mode algorithm.
 * Calculates site_kVAR_demand based on active power (site_kW_demand without POI corrections) and 
 * power factor setpoint received
 * @param power_factor_setpoint The power factor setpoint to follow
 * @param set_lagging_direction The direction of the power factor setpoint: True is negative aka lagging, and false is positive aka leading
 */
void Asset_Cmd_Object::constant_power_factor_mode(float power_factor_setpoint, bool set_lagging_direction)
{
    // Calculate magnitude of reactive power
    if (power_factor_setpoint != 0.0f)
    {
        // Dispatch Q based on P and pf
        // Equation derived from power factor equation:
        //    power factor = active power / apparent power
        //    apparent power = sqrt(active power^2 + reactive power^2)
        site_kVAR_demand = uncorrected_site_kW_demand * std::sqrt(1/std::pow(power_factor_setpoint, 2) - 1);
        // Limit by total potential for small power factor setpoints
        site_kVAR_demand = std::min(std::abs(site_kVAR_demand), std::abs(total_potential_kVAR));
    }
    else
        // Dispatch full reactive power capability (and no active power) when pf is 0
        site_kVAR_demand = std::abs(total_potential_kVAR);

    // Apply configured sign (true is negative, false is positive)
    site_kVAR_demand = set_lagging_direction ? -1.0f * site_kVAR_demand : site_kVAR_demand;
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

/**
 * Return whether load is being tracked at all. This will be true for both MINIMUM and OFFSET cases and false for NO_COMPENSATION
 * @return whether load is being tracked
 */
bool Asset_Cmd_Object::get_site_kW_load_inclusion()
{
    return load_method != NO_COMPENSATION;
}

// Save the demand prior to POI corrections (CLC, watt-watt)
void Asset_Cmd_Object::preserve_uncorrected_site_kW_demand()
{
    uncorrected_site_kW_demand = site_kW_demand;
}

//set site_kW_load_buffer_size
void Asset_Cmd_Object::create_site_kW_load_buffer(int size)
{
    load_buffer.clear();
    // Set minimum size of 1 iteration (10ms)
    load_buffer.resize(size == 0? 1 : size);
    load_iterator = 0;
}

/**
 * Set the load method
 * @param method The enumerated load type to use.
 *               Available options are NO_COMPENSATION, LOAD_OFFSET, and LOAD_MINIMUM
 */
void Asset_Cmd_Object::set_load_compensation_method(load_compensation method)
{
    load_method = method;
}

// Return the aggregate of available active chargeable power
float Asset_Cmd_Object::get_total_available_charge_kW()
{
    // TODO: This is an oversimplified solution for unidirectional features
    // Need to reevaluate how feeder works with CLC as well as mixed asset requests
    return less_than_zero_check(ess_data.min_potential_kW - less_than_zero_check(ess_data.kW_cmd));
}

// Return the aggregate of available active dischargeable power
float Asset_Cmd_Object::get_total_available_discharge_kW()
{
    // TODO: This is an oversimplified solution for unidirectional features
    // Need to reevaluate how feeder works with CLC as well as mixed asset requests
    return ess_data.calculate_unused_dischg_kW() + gen_data.calculate_unused_dischg_kW() + solar_data.calculate_unused_dischg_kW();
}

// Return the aggregate of available reactive chargeable power
float Asset_Cmd_Object::get_total_available_charge_kVAR()
{
    // Only ESS and Solar support negative reactive, and gen is not part of reactive power dispatch
    return less_than_zero_check(-1.0f * ess_data.potential_kVAR - less_than_zero_check(ess_data.kVAR_cmd))
         + less_than_zero_check(-1.0f * solar_data.potential_kVAR - less_than_zero_check(solar_data.kVAR_cmd));
}

// Return the aggregate of available reactive dischargeable power
float Asset_Cmd_Object::get_total_available_discharge_kVAR()
{
    // Only ESS and Solar are part of reactive power dispatch
    return zero_check(ess_data.potential_kVAR - zero_check(ess_data.kVAR_cmd))
         + zero_check(solar_data.potential_kVAR - zero_check(solar_data.kVAR_cmd));
}

// Save asset vars and demand before their modification so that they can be restored at a future point
void Asset_Cmd_Object::save_state()
{
    std::vector<Asset_Cmd_Data*> assets = list_assets_by_discharge_priority(0);
    for (auto asset : assets)
    {
        asset->saved_kW_request = asset->kW_request;
        asset->saved_kW_cmd = asset->kW_cmd;
    }
}

// Load asset vars and demand back to their original state
void Asset_Cmd_Object::restore_state()
{
    std::vector<Asset_Cmd_Data*> assets = list_assets_by_discharge_priority(0);
    for (auto asset : assets)
    {
        asset->kW_request = asset->saved_kW_request;
        asset->kW_cmd = asset->saved_kW_cmd;
    }
}

// reset active power dispatch variables
void Asset_Cmd_Object::reset_kW_dispatch()
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
void Asset_Cmd_Object::reset_kVAR_dispatch()
{
    ess_data.kVAR_cmd = 0.0;
    gen_data.kVAR_cmd = 0.0;
    solar_data.kVAR_cmd = 0.0;
}

/**
 * Builds a list of the asset data objects in the order of the given priority
 * Assets with a discharge request will be prioritized before assets without one
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 * @param exclusions Any assets that should be excluded from the list. Defaults to empty with no exclusions
 * 0 = solar, gen, ess, feeder. 1 = solar, feeder, ess, gen. 2 = gen, ess, solar, feeder.
 * @return List of pointers to the asset data objects ordered by priority.
 */
std::vector<Asset_Cmd_Object::Asset_Cmd_Data*> Asset_Cmd_Object::list_assets_by_discharge_priority(int asset_priority, std::vector<Asset_Cmd_Data*> exclusions)
{
    // Construct the basic list based on priority
    std::vector<Asset_Cmd_Data*> list_by_priority;
    switch(asset_priority)
    {
    case solar_gen_ess_feeder:  list_by_priority =  {&solar_data, &gen_data, &ess_data, &feeder_data}; break;
    case solar_feeder_ess_gen:  list_by_priority =  {&solar_data, &feeder_data, &ess_data, &gen_data}; break;
    case gen_ess_solar_feeder:  list_by_priority =  {&gen_data, &ess_data, &solar_data, &feeder_data}; break;
    case gen_solar_ess_feeder:  list_by_priority =  {&gen_data, &solar_data, &ess_data, &feeder_data}; break;
    case ess_solar_gen_feeder:  list_by_priority =  {&ess_data, &solar_data, &gen_data, &feeder_data}; break;
    case ess_solar_feeder_gen:  list_by_priority =  {&ess_data, &solar_data, &feeder_data, &gen_data}; break;
    case solar_gen_feeder_ess:  list_by_priority =  {&solar_data, &gen_data, &feeder_data, &ess_data}; break;
    default: 
        FPS_ERROR_LOG("Invalid asset priority, defaulting to priority 0");
        list_by_priority =  {&solar_data, &gen_data, &ess_data, &feeder_data};
        break;
    }
    // And remove any assets that should be excluded
    remove_asset_types(list_by_priority, exclusions);

    // Then create a final list with discharge requests prioritized first, falling back on the previous priority order otherwise
    std::vector<Asset_Cmd_Data*> list_by_kW_request_and_priority;
    // Copy over assets with discharge requests
    for (auto asset : list_by_priority)
        if (asset->kW_request > 0)
            list_by_kW_request_and_priority.push_back(asset);
    // Then copy over all other assets
    for (auto asset : list_by_priority)
        if (asset->kW_request <= 0)
            list_by_kW_request_and_priority.push_back(asset);

    return list_by_kW_request_and_priority;
}

/**
 * Removes the given asset types from a list of asset data objects.
 * @param data_list List of asset data objects.
 * @param asset_types Pointers to asset data objects that should be removed.
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
        total_unused_kW += asset_data->calculate_unused_dischg_kW();
    }
    return total_unused_kW;
}

/**
 * Determine if ESS is required to compensate for load due to asset priority. This will be a case
 * where ESS is prioritized before other assets, or the other assets are unable to discharge enough
 * power to meet the load.
 * @param asset_priority Enumerated integer indicating the priority of the various asset types.
 *                       See Types.h "priority" enum for a list of available values
 * @return True if ESS is required to handle load
 */
bool Asset_Cmd_Object::determine_ess_load_requirement(int asset_priority)
{
    calculate_site_kW_production_limits();
    if (load_method != LOAD_MINIMUM || (ess_data.kW_request >= 0 && site_kW_charge_production >= 0))
        return false;

    std::vector<Asset_Cmd_Data*> site_assets_list = list_assets_by_discharge_priority(asset_priority);
    float uncompensated_load = site_kW_load;
    for(auto asset_data : site_assets_list)
    {
        // Load has been accounted for
        if (uncompensated_load <= 0)
            break;

        uncompensated_load -= asset_data->calculate_unused_dischg_kW();

        // The ESS is responsible for discharging
        if (asset_data == &ess_data)
        {
            // Determine the amount of charge included in the demand and remove it
            calculate_site_kW_production_limits();
            feature_kW_demand -= less_than_zero_check(site_kW_charge_production);
            site_kW_demand -= less_than_zero_check(site_kW_charge_production);
            ess_data.kW_request = 0.0f;
            return true;
        }
    }
    return false;
}
