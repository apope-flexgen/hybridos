/**
 * Generator_Manager.cpp
 * Source for Generator-specific Manager class
 * Refactored from Asset_Manager.cpp
 *
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Generator_Manager.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>

extern fims* p_fims;

/****************************************************************************************/
Generator_Manager::Generator_Manager() : Type_Manager(GENERATORS_TYPE_ID) {
    numGenControllable = 0;

    genTotalActivePowerkW = 0.0;
    genTotalReactivePowerkVAR = 0.0;
    genTotalApparentPowerkVA = 0.0;
    genTotalUncontrollableActivePowerkW = 0.0;

    genTotalMaxPotentialActivePower = 0.0;
    genTotalMinPotentialActivePower = 0.0;

    genTotalNameplateActivePower = 0.0;

    genTargetActivePowerkW = 0.0;
    genTargetReactivePowerkVAR = 0.0;
}

/****************************************************************************************/
Generator_Manager::~Generator_Manager() {
    if (!pGens.empty())
        for (size_t j = 0; j < pGens.size(); j++)
            if (pGens[j] != NULL)
                delete pGens[j];
}

/**
 * Counts how many gens are stopped.
 * @return The number of gens that are stopped.
 */
int Generator_Manager::get_num_gens_stopped(void) {
    int num_gen_stopped = 0;
    for (auto gen : pGens) {
        if (gen->is_stopped())
            num_gen_stopped++;
    }
    return num_gen_stopped;
}

int Generator_Manager::get_num_gen_controllable(void) {
    return numGenControllable;
}

float Generator_Manager::get_gen_total_active_power(void) {
    return genTotalActivePowerkW;
}

float Generator_Manager::get_gen_total_reactive_power(void) {
    return genTotalReactivePowerkVAR;
}

float Generator_Manager::get_gen_total_uncontrollable_active_power(void) {
    return genTotalUncontrollableActivePowerkW;
}

float Generator_Manager::get_gen_total_max_potential_active_power(void) {
    return genTotalMaxPotentialActivePower;
}

float Generator_Manager::get_gen_total_min_potential_active_power(void) {
    return genTotalMinPotentialActivePower;
}

float Generator_Manager::get_gen_total_rated_active_power(void) {
    return genTotalRatedActivePower;
}

float Generator_Manager::get_total_kW_discharge_limit(void) {
    return total_kW_discharge_limit;
}

float Generator_Manager::get_gen_total_potential_reactive_power(void) {
    return genTotalPotentialReactivePower;
}

float Generator_Manager::get_gen_total_nameplate_active_power(void) {
    return genTotalNameplateActivePower;
}

void Generator_Manager::set_all_gen_grid_form(void) {
    for (int i = 0; i < numParsed; i++)
        // if (pGens[i]->get_gen_grid_mode() == FOLLOWING)
        pGens[i]->set_grid_mode(FORMING);
}

void Generator_Manager::set_all_gen_grid_follow(void) {
    for (int i = 0; i < numParsed; i++)
        // if (pGens[i]->get_gen_grid_mode() == FORMING)
        pGens[i]->set_grid_mode(FOLLOWING);
}

/**
 * If using LDSS, starts the highest-priority gen. Else, starts the first gen found that is startable.
 * @return True if there is an available gen and the start cmd is successfully sent. False otherwise.
 */
bool Generator_Manager::direct_start_gen(void) {
    if (ldss.enabled)
        return ldss.start_generator();
    else {
        for (auto gen : pGens) {
            if (gen->is_stopped() && !gen->in_maint_mode() && gen->start()) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Attempts to start all gens.
 */
void Generator_Manager::start_all_gen(void) {
    for (auto gen : pGens) {
        if (gen->is_stopped() && !gen->in_maint_mode())
            gen->start();
    }
}

/**
 * Attempts to stop all gens.
 * @return True if all stop commands were successfully sent. False if any stop commands failed to send.
 */
bool Generator_Manager::stop_all_gen(void) {
    bool all_commands_sent = true;
    for (auto gen : pGens) {
        if (gen->is_controllable())
            all_commands_sent = all_commands_sent && gen->stop();
    }
    return all_commands_sent;
}

bool Generator_Manager::set_gen_target_active_power(float desiredkW) {
    genTargetActivePowerkW = desiredkW;
    return true;
}

bool Generator_Manager::set_gen_target_reactive_power(float desiredkW) {
    genTargetReactivePowerkVAR = desiredkW;
    return true;
}

bool Generator_Manager::aggregate_gen_data(void) {
    FPS_DEBUG_LOG("Processing Generator data; Generator_Manager::aggregate_gen_data\n");

    // clear member variables
    numAvail = 0;
    numRunning = 0;
    numGenControllable = 0;
    num_in_local_mode = 0;

    genTotalActivePowerkW = 0.0;
    genTotalReactivePowerkVAR = 0.0;
    genTotalApparentPowerkVA = 0.0;
    genTotalUncontrollableActivePowerkW = 0.0;

    genTotalMaxPotentialActivePower = 0.0;
    genTotalMinPotentialActivePower = 0.0;
    genTotalPotentialReactivePower = 0.0f;

    genTotalRatedActivePower = 0.0f;
    genTotalNameplateActivePower = 0.0;

    total_kW_discharge_limit = 0.0f;

    // aggregate gen
    for (int i = 0; i < numParsed; i++) {
        // aggregate nameplate values
        // TODO: nameplate power values are based on site configuration, move to static section
        genTotalNameplateActivePower += pGens[i]->get_rated_active_power();

        if (pGens[i]->is_available())
            numAvail++;

        if (pGens[i]->is_running())
            numRunning++;

        if (pGens[i]->is_controllable()) {
            numGenControllable++;

            genTotalActivePowerkW += pGens[i]->get_active_power();
            genTotalReactivePowerkVAR += pGens[i]->get_reactive_power();
            genTotalApparentPowerkVA += pGens[i]->get_apparent_power();

            genTotalMaxPotentialActivePower += pGens[i]->get_max_potential_active_power();
            genTotalMinPotentialActivePower += pGens[i]->get_min_potential_active_power();

            genTotalRatedActivePower += pGens[i]->get_rated_active_power();
            genTotalPotentialReactivePower += pGens[i]->get_potential_reactive_power();

            total_kW_discharge_limit += pGens[i]->get_max_limited_active_power();
        } else {
            // only add to uncontrollable power if gen is uncontrollable for reasons besides a modbus disconnect
            if (!pGens[i]->get_watchdog_fault()) {
                genTotalUncontrollableActivePowerkW += pGens[i]->get_active_power();
            }
        }

        if (pGens[i]->is_in_local_mode())
            num_in_local_mode++;
    }

    return false;
}

void Generator_Manager::generate_asset_type_summary_json(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // summary {

    bufJSON_AddStringCheckVar(buf, "name", "Generator Summary", var);
    bufJSON_AddNumberCheckVar(buf, "num_gen_available", numAvail, var);
    bufJSON_AddNumberCheckVar(buf, "num_gen_running", numRunning, var);
    bufJSON_AddNumberCheckVar(buf, "num_gen_controllable", numGenControllable, var);
    bufJSON_AddNumberCheckVar(buf, "num_gen_in_local_mode", num_in_local_mode, var);

    Asset_Generator* pGenStart = ldss.get_gen_to_start();
    Asset_Generator* pGenStop = ldss.get_gen_to_stop();

    // start gen timer display
    if (pGenStart != NULL && pGenStart != pGenStop) {
        std::string timer_display = seconds_to_timer_string(ldss.start_gen_countdown, ldss.start_gen_time);
        bufJSON_AddStringCheckVar(buf, "start_gen_countdown", timer_display.c_str(), var);
    }

    // stop gen timer display
    if (pGenStop != NULL && pGenStop != pGenStart) {
        std::string timer_display = seconds_to_timer_string(ldss.stop_gen_countdown, ldss.stop_gen_time);
        bufJSON_AddStringCheckVar(buf, "stop_gen_countdown", timer_display.c_str(), var);
    }

    char temp_name[MEDIUM_MSG_LEN];
    for (auto it : pGens) {
        const char* gen_name = it->get_name().c_str();
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_active_power", gen_name);
        bufJSON_AddNumberCheckVar(buf, temp_name, it->get_active_power(), var);
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_alarms", gen_name);
        bufJSON_AddNumberCheckVar(buf, temp_name, it->get_num_active_alarms() > 0 ? 1 : 0, var);
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_faults", gen_name);
        bufJSON_AddNumberCheckVar(buf, temp_name, it->get_num_active_faults() > 0 ? 1 : 0, var);
    }
    bufJSON_AddNumberCheckVar(buf, "gen_num_alarmed", get_num_alarmed(), var);
    bufJSON_AddNumberCheckVar(buf, "gen_num_faulted", get_num_faulted(), var);

    if (var == NULL)
        bufJSON_EndObjectNoComma(buf);  // } summary
}

// respond to new generator target power setpoints from site manager
bool Generator_Manager::calculate_gen_power(void) {
    float gen_power_target_remainder = genTargetActivePowerkW;  // dynamically calculated "remainder" power from site manager request
    float balance_point = 0.0, unbalanced_gen_power_command = 0.0, balanced_gen_power_command = 0.0, balanced_gen_power_command_delta = 0.0;
    int numGenStopping = 0, numGenUnBalanced = 0, numGenBalanced = 0;  // variables to track instantaneous states of generators

    if (genTargetActivePowerkW == 0)  // site_manager has removed power request from generators
    {
        for (int i = 0; i < numParsed; i++) {
            if (pGens[i]->is_controllable()) {
                pGens[i]->set_active_power_setpoint(0);  // no slew needed, respond immediately
            }
        }
    }

    // identify if any generators are marked "stopping" by LDSS, in theory there can only be one
    for (int i = 0; i < numParsed; i++) {
        if (pGens[i]->is_controllable() && pGens[i]->is_stopping()) {
            numGenStopping++;

            // begin ramp down of generator based on internal slew
            // can't use min_potential_active_power since the generator is marked stopping, so use slew_min_target instead
            float gen_stopping_setpoint = pGens[i]->get_active_power_slew_min_target();
            pGens[i]->set_active_power_setpoint((gen_stopping_setpoint < 0) ? 0 : gen_stopping_setpoint);

            // calculate the remainder of the power to be distributed after ramping down the "stopping" generator
            gen_power_target_remainder -= gen_stopping_setpoint;

            // successfully ramped "stopping" generator to 0KW, now reset it
            if (gen_stopping_setpoint == 0) {
                pGens[i]->stop();
                ldss.adjust_dynamic_stop_priorities(pGens[i]);
            }
        }
    }

    // no further power management needed if the stopping generator has handled power request, but do this after generators have been stopped
    if (gen_power_target_remainder == 0)
        return true;

    // power distributions across generators must reach balance point over time while also servicing system power targets
    if ((numGenControllable - numGenStopping) > 0)  // if you stopped the only generator that's running, exit and "don't pass go"
    {
        // calculate balance point of all generators, excluding any that are stopping
        balance_point = gen_power_target_remainder / (float)(numGenControllable - numGenStopping);

        // identify which generator in the system is "out of balance", this code only supports one unbalanced generator at a time
        // simultaneously, identify how much power is left unserved in the system based on the capability of the balanced generators (subtract the capability of the unbalanced one afterwards)
        int unbalanced_index = 0;
        float unbalanced_distance = 0.0, distance = 0.0;
        float gen_power_max_balanced = 0.0, gen_power_min_balanced = 0.0, gen_power_actual_balanced = 0.0;
        for (int i = 0; i < numParsed; i++) {
            pGens[i]->set_balanced(true);  // assume that they are all balanced, "reset"
            if (pGens[i]->is_controllable() && !pGens[i]->is_stopping()) {
                distance = balance_point - pGens[i]->get_active_power();  // calculate the distance from balance point, but keep the sign for directionality check later

                // capture only if the distance is outside of deadband limits and it's the furthest distance we've encountered so far
                // the deadband must compensate both directions, and has to be greater than the slew rate, since min/max are both achievable in one iteration
                if (fabsf(distance) > (float)pGens[i]->get_active_power_slew_rate() * 2 && fabsf(distance) > fabsf(unbalanced_distance)) {
                    unbalanced_index = i;
                    unbalanced_distance = distance;
                }

                // aggregate all max/min power capabilities for distribution downstream
                gen_power_max_balanced += pGens[i]->get_max_potential_active_power();
                gen_power_min_balanced += pGens[i]->get_min_potential_active_power();

                // also aggregate the total current setpoint to generators for "delta"-based distribution downstream
                gen_power_actual_balanced += pGens[i]->get_active_power_setpoint();
            }
        }

        // if both the index and distance are unchanged, all generators are balanced within deadband limits
        // demorgan's law -> (unbalanced_index == 0 && unbalanced_distance == 0)
        if (unbalanced_index != 0 || unbalanced_distance != 0.0) {
            // found an unbalanced generator, mark it
            numGenUnBalanced++;
            pGens[unbalanced_index]->set_balanced(false);

            // subtract its capability from the balance distribution
            gen_power_max_balanced -= pGens[unbalanced_index]->get_max_potential_active_power();
            gen_power_min_balanced -= pGens[unbalanced_index]->get_min_potential_active_power();
            gen_power_actual_balanced -= pGens[unbalanced_index]->get_active_power_setpoint();

            // request max/min power from the generator based on it's relative position to the balance point, adjust this later
            // can use get_max/min_potential_active_power() here because we will never pick a generator that is marked "stopping" to be "unbalanced"
            if (pGens[unbalanced_index]->get_active_power() < balance_point)
                unbalanced_gen_power_command = pGens[unbalanced_index]->get_max_potential_active_power();
            else
                unbalanced_gen_power_command = pGens[unbalanced_index]->get_min_potential_active_power();
        }

        // if we're at this point, then we've assigned power to the "stopping" and "unbalanced" generators, and need to handle the "balanced" ones
        // calculate now many generators are at balance point
        numGenBalanced = numGenControllable - numGenStopping - numGenUnBalanced;

        if (numGenBalanced != numGenControllable)  // we still have balancing to do
        {
            // distribute power to all gens that are "unbalanced", and must have atleast one to execute this code
            if (numGenUnBalanced > 0) {
                // adjust unbalanced generator accordingly
                if ((gen_power_target_remainder - unbalanced_gen_power_command) > gen_power_max_balanced)
                    unbalanced_gen_power_command = gen_power_target_remainder - gen_power_max_balanced;
                else if ((gen_power_target_remainder - unbalanced_gen_power_command) < gen_power_min_balanced)
                    unbalanced_gen_power_command = gen_power_target_remainder - gen_power_min_balanced;

                gen_power_target_remainder -= unbalanced_gen_power_command;
            }

            // calculate the remainder to be distributed to "balanced" gens
            // generate a "delta" value based on the current power setpoint of the generator
            /* ex. for both gen_1 and gen_2 which are balanced generators
             * first iteration:    site_cmd = 600, gen_1 = 100, gen_2 = 500
             * second iteration:   site_cmd = 500, gen_1 =  50, gen_2 = 450
             *
             * gen_power_actual_balanced = 600 (from previous iteration), delta = -50
             */
            balanced_gen_power_command_delta = (gen_power_target_remainder - gen_power_actual_balanced) / numGenBalanced;

            for (int i = 0; i < numParsed; i++) {
                // slew checking here is implied
                if (pGens[i]->is_controllable() && !pGens[i]->is_stopping() && !pGens[i]->is_balanced())  // unbalanced gen
                {
                    pGens[i]->set_active_power_setpoint(pGens[i]->get_active_power_slew_target(unbalanced_gen_power_command));
                } else if (pGens[i]->is_controllable() && !pGens[i]->is_stopping() && pGens[i]->is_balanced())  // "balanced" gens
                {
                    // distribute power to each balanced generator based on a delta value starting at its current active power setpoint
                    balanced_gen_power_command = pGens[i]->get_active_power_setpoint() + balanced_gen_power_command_delta;
                    pGens[i]->set_active_power_setpoint(pGens[i]->get_active_power_slew_target(balanced_gen_power_command));
                }
            }
        } else  // all generators are balanced, so distribute power evenly
        {
            for (int i = 0; i < numParsed; i++) {
                if (pGens[i]->is_controllable() && !pGens[i]->is_stopping() && pGens[i]->is_balanced()) {
                    // slew checking here is implied, since deadband is calculated in relation to the slew
                    pGens[i]->set_active_power_setpoint(pGens[i]->get_active_power_slew_target(gen_power_target_remainder / numGenControllable));
                }
            }
        }
    }
    return true;
}

// HybridOS Step 2: Process Asset Data
void Generator_Manager::process_asset_data(void) {
    if (numParsed > 0) {
        for (int i = 0; i < numParsed; i++) {
            pGens[i]->process_asset();
        }
        aggregate_gen_data();
    }
}

// HybridOS Step 4: Update Asset Data
void Generator_Manager::update_asset_data(void) {
    if (ldss.enabled && ldss.is_time_for_check())
        ldss.check(genTargetActivePowerkW);
    calculate_gen_power();
    for (auto gen : pGens)
        gen->update_asset();
    if (ldss.enabled)
        handle_faulted_gens();
}

void Generator_Manager::start_first_gen(bool enable) {
    if (!ldss.first_gen_is_starting)
        ldss.start_first_gen = enable;
}

void Generator_Manager::set_first_gen_is_starting_flag(bool flag) {
    ldss.first_gen_is_starting = flag;
}

void Generator_Manager::set_min_generators_active(int minGensActive) {
    ldss.min_generators_active = minGensActive;
}

void Generator_Manager::make_gen_highest_start_priority(Asset_Generator* gen) {
    ldss.move_gen_to_front_of_both_start_priority_lists(gen);
}

void Generator_Manager::make_gen_highest_stop_priority(Asset_Generator* gen) {
    ldss.move_gen_to_front_of_both_stop_priority_lists(gen);
}

void Generator_Manager::enable_ldss(bool flag) {
    ldss.enable(flag);
}

/**
 * Checks for any newly-faulted generators and attempts to start a different generator
 * for every newly-faulted generator found.
 */
void Generator_Manager::handle_faulted_gens(void) {
    for (auto gen : pGens) {
        if (gen->is_newly_faulted())
            ldss.start_generator();
    }
}

/**
 * Passes down Site Manager-configured parameters to the LDSS feature.
 * @param settings Group of parameters set by Site Manager to control the LDSS feature.
 */
void Generator_Manager::update_ldss_settings(LDSS_Settings& settings) {
    ldss.update_settings(settings);
}

/****************************************************************************************/
/*
    Overriding configuration functions
*/
void Generator_Manager::configure_base_class_list() {
    pAssets.assign(pGens.begin(), pGens.end());
}

Asset* Generator_Manager::build_new_asset(void) {
    Asset_Generator* asset = new Asset_Generator;
    if (asset == NULL) {
        FPS_ERROR_LOG("There is something wrong with this build. Generator %zu: Memory allocation error.", pGens.size() + 1);
        exit(1);
    }
    numParsed++;
    return asset;
}

void Generator_Manager::append_new_asset(Asset* asset) {
    pGens.push_back((Asset_Generator*)asset);
}

// After configuring individual asset instances, this function finishes configuring the Generator Manager
Config_Validation_Result Generator_Manager::configure_type_manager(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    cJSON* gen_root = configurator->asset_type_root;

    // give each asset a pointer to the LDSS object so it can reference the state variables
    for (auto gen : pGens)
        gen->ldss = &ldss;

    // parse LDSS static run priorities out of assets.json
    cJSON* ldss_static_run_priorities = cJSON_HasObjectItem(gen_root, "ldss_static_run_priorities") ? cJSON_GetObjectItem(gen_root, "ldss_static_run_priorities") : NULL;
    if (ldss_static_run_priorities == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to find ldss_static_run_priorities in generator variables section of assets.json.", configurator->p_manager->get_asset_type_id())));
    }

    // pass static run priorities to LDSS for feature configuration
    if (!ldss.configure_priorities(pGens, ldss_static_run_priorities)) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: Failed to configure LDSS.", configurator->p_manager->get_asset_type_id())));
    }

    return validation_result;
}
/****************************************************************************************/
