/**
 * LDSS.cpp
 * Source for LDSS feature class
 * 
 * Created October 2021
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <bits/stdc++.h>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <LDSS.h>
#include <Site_Controller_Utils.h>

LDSS::LDSS()
{
    priority_setting = STATIC;
    max_load_threshold_percent = 0.0;
    min_load_threshold_percent = 0.0;
    enabled = false;
    enabled_rising_edge = false;
    start_first_gen = false;
    first_gen_is_starting = false;
    stop_gen_time = 0;
    start_gen_time = 0;
    min_generators_active = 0;
}

/**
 * Configures the priorities for each generator.
 * @param pg Reference to Generator Manager's pGens to give LDSS access to generator assets.
 * @param static_run_priorities Pointer to cJSON object containing an array of the gen priorities, in the order of the gen indices.
 * @return True if configuration is successful. False if configuration fails.
 */
bool LDSS::configure_priorities(std::vector<Asset_Generator*> const &pg, cJSON* static_run_priorities)
{
    // give LDSS access to generator assets
    generators = pg;

    // verify number of priorities entered into assets.json matches number of generator assets
    int num_priorities = cJSON_GetArraySize(static_run_priorities);
    int num_gens = (int)generators.size();
    if (num_priorities != num_gens)
    {
        FPS_ERROR_LOG("Number of priority numbers listed in ldss_static_run_priorities (%d) does not match number of gens parsed (%d).\n", num_priorities, num_gens);
        FPS_ERROR_LOG("Consult the generator variables section of assets.json and fix. \n");
        return false;
    }

    // configure priorities for each generator
    for (int i = 0; i < num_priorities; ++i)
    {
        cJSON* priority_json = cJSON_GetArrayItem(static_run_priorities, i);
        if (priority_json == NULL)
        {
            FPS_ERROR_LOG("Array entry %d of ldss_static_run_priorities in the generator variables section of assets.json is NULL\n", i);
            return false;
        }
        int priority = priority_json->valueint;
        if (priority < 1 || priority > num_gens)
        {
            FPS_ERROR_LOG("Array entry %d of ldss_static_run_priorities in the generator variables section of assets.json is %d, which is invalid.\n", i, priority);
            FPS_ERROR_LOG("Should be between 1 and %d, inclusive.\n", num_gens);
            return false;
        }
        int stop_priority = num_gens - priority + 1; // stop priorities should be in reverse order of start priorities
        // configure static priorities
        generators[i]->set_static_start_priority(priority);
        generators[i]->set_static_stop_priority(stop_priority);
        // configure initial dynamic priorities
        generators[i]->set_dynamic_start_priority(priority);
        generators[i]->set_dynamic_stop_priority(0);
    }
    return true;
}

/**
 * Updates LDSS feature settings with setpoints passed-down from Site Manager.
 * @param settings Group of settings from Site Manager.
 */
void LDSS::update_settings(LDSS_Settings &settings)
{
    // update feature settings
    enabled = settings.enabled;
    priority_setting = settings.priority_setting;
    max_load_threshold_percent = settings.max_load_threshold_percent;
    min_load_threshold_percent = settings.min_load_threshold_percent;
    warmup_time = settings.warmup_time;
    cooldown_time = settings.cooldown_time;
    if (start_gen_countdown == start_gen_time)
        start_gen_countdown = settings.start_gen_time;
    start_gen_time = settings.start_gen_time;
    if (stop_gen_countdown == stop_gen_time)
        stop_gen_countdown = settings.stop_gen_time;
    stop_gen_time = settings.stop_gen_time;
    max_soc_percent = settings.max_soc_percent;
    min_soc_percent = settings.min_soc_percent;
    enable_soc_thresholds_flag = settings.enable_soc_thresholds_flag;
    pEss = settings.pEss;
}

/**
 * Enables and configures or disables LDSS.
 * @param flag If true, turns on LDSS and configures it. If false, turns off LDSS.
 */
void LDSS::enable(bool flag)
{
    if (enabled == flag)
        return;

    enabled = flag;
    if (!flag)
        return;

    enabled_rising_edge = true;
}

/**
 * Helper function for the main check() function to determine whether to turn on a generator.
 * If load is above configured maximum threshold, attempt to start a generator.
 * If soc checks are enabled, also start if SoC is below min_soc_percent
 * @param num_controllable Number of controllable generators
 * @param max_load_threshold_kw Upper limit of load in kW
 * @param target_kw Current load in kW
*/
void LDSS::check_start_generator(int num_controllable, float max_load_threshold_kw, float target_kw)
{
    // Do not start generators if SoC is too high
    if (enable_soc_thresholds_flag && pEss->get_ess_soc_avg() > max_soc_percent) {
        return;
    }

    // start the first gen if start_first_gen is raised
    if (num_controllable == 0 && start_first_gen) {
        start_generator();
        return;
    }

    if (
        // if the gen power command is above the max threshold, decrement the countdown to when we start the next gen
        (target_kw > max_load_threshold_kw) ||

        // if soc threshold is enabled and SoC is under min_soc_percent, start a generator.
        (enable_soc_thresholds_flag && pEss->get_ess_soc_avg() < min_soc_percent)
    ) {
        start_gen_countdown = std::max(start_gen_countdown - 1, 0);
        // if the countdown hits zero, start the next gen
        if (start_gen_countdown <= 0) {
            start_generator();
        }
        return;
    }

    // if the gen power command is lower than the max threshold and our countdown to next gen start is not at beginning value, increment it
    start_gen_countdown = std::min(start_gen_countdown + 1, start_gen_time);
}

/**
 * Helper function for the main check() function to determine whether to turn off a generator.
 * If load is below configured minimum threshold, attempt to stop a generator.
 * If soc checks are enabled, also stop a generator if SoC is above max_soc_percent
 * @param num_controllable Number of controllable generators
 * @param min_load_threshold_kw Lower limit of load in kW
 * @param target_kw Current load in kW
*/
void LDSS::check_stop_generator(int num_controllable, float min_load_threshold_kw, float target_kw)
{
    // Do not stop generators if SoC is too low
    if (enable_soc_thresholds_flag && pEss->get_ess_soc_avg() < min_soc_percent) {
        return;
    }
    
    if (
        // if the gen power command is below the min threshold, decrement the countdown to when we stop the next gen
        // but do not count down if stopping a gen would cause active generators to fall below the minimum floor
        (target_kw < min_load_threshold_kw && num_controllable > min_generators_active) ||

        // if soc threshold is enabled and SoC is above max_soc_percent, stop a generator.
        (enable_soc_thresholds_flag && pEss->get_ess_soc_avg() > max_soc_percent)
    ) {
        stop_gen_countdown = std::max(stop_gen_countdown - 1, 0);

        // if the countdown hits zero, stop the next gen
        if (stop_gen_countdown <= 0) {
            stop_generator();
        }
        return;
    }

    // if the gen power command is higher than the min threshold and our countdown to next gen stop is not at beginning value, increment it
    stop_gen_countdown = std::min(stop_gen_countdown + 1, stop_gen_time);
}


/**
 * Update generator warmup/cooldown times and priorities
*/
void LDSS::update_cooldown_and_warmup()
{
    for(auto gen : generators)
    {
        if (gen->is_stopped())
        {
            // if gen is cooling down, tick its cooldown timer
            if (gen->is_cooldown_timer_active())
                gen->tick_cooldown_timer();
            // if gen just finished cooling down, unblock it from starting and make it lowest start priority since it was most recently stopped
            else if (gen->get_cooling_down_flag())
            {
                gen->block_starts_based_on_static_priority(false);
                gen->set_cooling_down_flag(false);
                move_gen_to_back_of_dynamic_start_priority_list(gen);
            }
        }
        else if (gen->is_running())
        {
            // if gen is warming up, tick its warmup timer
            if (gen->is_warmup_timer_active())
                gen->tick_warmup_timer();
            // if gen just finished warming up, unblock it from stopping and make it lowest stop priority since it was most recently started
            else if (gen->get_warming_up_flag())
            {
                gen->block_stops_based_on_static_priority(false);
                gen->set_warming_up_flag(false);
                move_gen_to_back_of_dynamic_stop_priority_list(gen);
            }
        }
    }
}

/**
 * Main routine for the LDSS feature. Checks if generators need to be started or stopped and updates timers.
 */
void LDSS::check(float target_kw)
{
    int num_controllable = get_num_controllable();

    // if there is at least one running/controllable generator, we are not in a state where the first gen is transitioning to running.
    // lower the first_gen_is_starting flag so that next time no gens are running, the start_first_gen control can be used
    if (num_controllable > 0)
        first_gen_is_starting = false;

    // aggregate threshold for controllable (AKA running and non-maintenance-mode) generators
    float min_load_threshold_kw = 0.0;
    float max_load_threshold_kw = 0.0;
    for(auto gen : generators)
    {
        if (gen->is_controllable())
        {
            min_load_threshold_kw += min_load_threshold_percent/100.0 * gen->get_rated_active_power();
            max_load_threshold_kw += max_load_threshold_percent/100.0 * gen->get_rated_active_power();
        }
    }

    check_start_generator(num_controllable, max_load_threshold_kw, target_kw);
    check_stop_generator(num_controllable, min_load_threshold_kw, target_kw);

    update_cooldown_and_warmup();
}

/**
 * Decrements all start priorities at and after the passed-in gen's start priority. Called after the passed-in gen is started.
 */
void LDSS::adjust_dynamic_start_priorities(Asset_Generator* started_gen)
{
    int started_gen_old_priority = started_gen->get_dynamic_start_priority();
    for(auto gen : generators)
    {
        if (gen->get_dynamic_start_priority() >= started_gen_old_priority)
            gen->set_dynamic_start_priority(gen->get_dynamic_start_priority()-1);
    }
}

/**
 * Decrements all stop priorities at and after the passed-in gen's stop priority. Called after the passed-in gen is stopped.
 */
void LDSS::adjust_dynamic_stop_priorities(Asset_Generator* stopped_gen)
{
    int stopped_gen_old_priority = stopped_gen->get_dynamic_stop_priority();
    for(auto gen : generators)
    {
        if (gen->get_dynamic_stop_priority() >= stopped_gen_old_priority)
            gen->set_dynamic_stop_priority(gen->get_dynamic_stop_priority()-1);
    }
}

/**
 * Makes the given generator the highest priority to start next for both dynamic and static priority lists.
 * Triggered by the user pressing the "Start Next" button.
 */
void LDSS::move_gen_to_front_of_both_start_priority_lists(Asset_Generator* gen_to_start_next)
{
    int old_dynamic_priority = gen_to_start_next->get_dynamic_start_priority();
    int old_static_priority = gen_to_start_next->get_static_start_priority();
    for(auto gen : generators)
    {
        if ( gen == gen_to_start_next )
        {
            // move the target generator to the highest priority position
            gen->set_dynamic_start_priority(1);
            gen->set_static_start_priority(1);
        }
        else
        {
            // lower the priority of generators that had a higher priority than the generator that is being moved up
            if ( gen->get_dynamic_start_priority() < old_dynamic_priority && gen->get_dynamic_start_priority() != 0 )
                gen->set_dynamic_start_priority( gen->get_dynamic_start_priority()+1 );
            if ( gen->get_static_start_priority() < old_static_priority && gen->get_static_start_priority() != 0 )
                gen->set_static_start_priority( gen->get_static_start_priority()+1 );
        }
    }
}

/**
 * Makes the given generator the highest priority to stop next for both dynamic and static priority lists.
 * Triggered by the user pressing the "Stop Next" button.
 */
void LDSS::move_gen_to_front_of_both_stop_priority_lists(Asset_Generator* gen_to_stop_next)
{
    int old_dynamic_priority = gen_to_stop_next->get_dynamic_stop_priority();
    int old_static_priority = gen_to_stop_next->get_static_stop_priority();
    for(auto gen : generators)
    {
        if ( gen == gen_to_stop_next )
        {
            // move the target generator to the highest priority position
            gen->set_dynamic_stop_priority(1);
            gen->set_static_stop_priority(1);
        }
        else
        {
            // lower the priority of generators that had a higher priority than the generator that is being moved up
            if ( gen->get_dynamic_stop_priority() < old_dynamic_priority && gen->get_dynamic_stop_priority() != 0 )
                gen->set_dynamic_stop_priority( gen->get_dynamic_stop_priority()+1 );
            if ( gen->get_static_stop_priority() < old_static_priority && gen->get_static_stop_priority() != 0 )
                gen->set_static_stop_priority( gen->get_static_stop_priority()+1 );
        }
    }
}

/**
 * Moves a generator that has just finished warming up to the back of the dynamic stop priority list.
 * @param warmed_up_gen Pointer to gen that just finished warming up.
 */
void LDSS::move_gen_to_back_of_dynamic_stop_priority_list(Asset_Generator* warmed_up_gen)
{
    int max_priority = 0;
    for(auto gen : generators)
    {
        max_priority = std::max(max_priority, gen->get_dynamic_stop_priority());
    }

    warmed_up_gen->set_dynamic_stop_priority(max_priority + 1);
}

/**
 * Moves a generator that has just finished cooling down to the back of the dynamic start priority list.
 * @param cooled_down_gen Pointer to gen that just finished cooling down.
 */
void LDSS::move_gen_to_back_of_dynamic_start_priority_list(Asset_Generator* cooled_down_gen)
{
    int max_priority = 0;
    for(auto gen : generators)
    {
        if (gen->get_dynamic_start_priority() > max_priority && gen != cooled_down_gen)
            max_priority = gen->get_dynamic_start_priority();
    }
    cooled_down_gen->set_dynamic_start_priority(max_priority + 1);
}

/**
 * Resets the Start Generator Countdown to the beginning value.
 */
void LDSS::reset_start_gen_countdown()
{
    start_gen_countdown = start_gen_time;
}

/**
 * Resets the Stop Generator Countdown to the beginning value.
 */
void LDSS::reset_stop_gen_countdown()
{
    stop_gen_countdown = stop_gen_time;
}

/**
 * Decides if it has been long enough since the last LDSS check for a new LDSS check.
 * @return True if LDSS should be checked, false if not.
 */
bool LDSS::is_time_for_check()
{
    static bool first_check = true;
    static timespec last_check_time;
    timespec current_time;

    // record current time
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    // check LDSS if this is the first process loop
    if (first_check)
    {
        first_check = false;
        last_check_time = current_time;
        return true;
    }

    // check LDSS if it has been a long enough time duration since the last check
    int microseconds_since_last_check = (current_time.tv_sec - last_check_time.tv_sec) * 1000000 + (current_time.tv_nsec - last_check_time.tv_nsec) / 1000;
    if (microseconds_since_last_check >= ASSET_TIMER_LOOP_uS_LOW)
    {
        last_check_time = current_time;
        return true;
    }
    return false;
}

/**
 * Starts the next generator in the start priority list.
 * @return True if there is a gen available to be started and the start cmd successfully goes through. False otherwise.
 */
bool LDSS::start_generator()
{
    Asset_Generator* gen_to_start = get_gen_to_start();
    if (gen_to_start == NULL)
        return false;

    // in case this is the first generator to be started, clear the start_first_gen flag and raise the first_gen_is_starting flag
    // if there are already other gens running, the first_gen_is_starting flag will be immediately lowered next iteration
    start_first_gen = false;
    first_gen_is_starting = true;

    // attempt to start the gen and return error code if start cmd does not go through
    if (!gen_to_start->start())
        return false;

    adjust_dynamic_start_priorities(gen_to_start);

    // reset start timer
    reset_start_gen_countdown();

    return true;
}

/**
 * Stops the next generator in the stop priority list.
 */
void LDSS::stop_generator()
{
    Asset_Generator* gen_to_stop = get_gen_to_stop();
    if (gen_to_stop == NULL)
        return;

    if (gen_to_stop->get_grid_mode() == FOLLOWING) // request the generator to stop in grid following using graceful shutdown
    {
        gen_to_stop->set_stopping_flag(true);
    }
    else if (gen_to_stop->get_grid_mode() == FORMING) // force stop the generator in grid forming as slewing is not applicable
    {
        gen_to_stop->stop();
        adjust_dynamic_stop_priorities(gen_to_stop);
    }

    // reset stop timer
    reset_stop_gen_countdown();
}

/**
 * Selects the next generator that should be started based on either dynamic start priorities or static
 * start priorities, depending on the LDSS priority setting.
 * @return Pointer to the next gen that should be started, or NULL if there are no startable gens.
 */
Asset_Generator* LDSS::get_gen_to_start(void)
{
    int min_priority = INT_MAX;
    Asset_Generator* gen_to_start = NULL;

    // iterate through gens and find which one should be started next
    for(auto gen : generators)
    {
        int current_priority;
        if (priority_setting == STATIC)
            current_priority = gen->get_block_ldss_static_starts_flag() ? 0 : gen->get_static_start_priority();
        else // priority_setting == DYNAMIC
            current_priority = gen->get_dynamic_start_priority();

        // gen may have been started through some means that LDSS did not handle (maint mode, other site state, before HybridOS was restarted, etc.)
        // in that case, update start priorities to account for the starting/running gen
        if ( current_priority > 0 && (gen->is_starting() || gen->is_running()) )
        {
            gen->block_starts_based_on_static_priority(true);
            adjust_dynamic_start_priorities(gen);
            return get_gen_to_start(); // since we just moved around priorities, want to restart iteration through generators
        }

        // gen should not be started if has lower priority than another gen, has a 0 priority (indicating it is already running), or is unavailable
        if (current_priority < min_priority && current_priority != 0 && gen->is_available())
        {
            min_priority = current_priority;
            gen_to_start = gen;
        }
    }

    return gen_to_start;
}

/**
 * Selects the next generator that should be stopped based on either dynamic stop priorities or static
 * stop priorities, depending on the LDSS priority setting.
 * @return Pointer to the next gen that should be stopped, or NULL if there are no stoppable gens.
 */
Asset_Generator* LDSS::get_gen_to_stop(void)
{
    int min_priority = INT_MAX;
    Asset_Generator* gen_to_stop = NULL;

    // iterate through gens and find which one should be stopped next
    for(auto gen : generators)
    {
        int current_priority;
        if (priority_setting == STATIC)
            current_priority = gen->get_block_ldss_static_stops_flag() ? 0 : gen->get_static_stop_priority();
        else // priority_setting == DYNAMIC
            current_priority = gen->get_dynamic_stop_priority();

        // gen may have been stopped through some means that LDSS did not handle (maint mode, other site state, etc.)
        // in that case, update stop priorities to account for the stopped gen
        if (current_priority > 0 && gen->is_stopped())
        {
            gen->block_stops_based_on_static_priority(true);
            adjust_dynamic_stop_priorities(gen);
            return get_gen_to_stop(); // since we just moved around priorities, want to restart iteration through generators
        }

        // gen should not be stopped next if it is lower priority than another gen, has non-zero priority (meaning it is already stopped), or is in maintenance mode
        if (current_priority < min_priority && current_priority != 0 && !gen->in_maint_mode())
        {
            min_priority = current_priority;
            gen_to_stop = gen;
        }
    }

    return gen_to_stop;
}

/**
 * Counts how many generators are controllable.
 * @return Number of generators that are controllable.
 */
int LDSS::get_num_controllable()
{
    int num_controllable = 0;
    for(auto gen : generators)
    {
        if (gen->is_controllable())
            num_controllable++;
    }
    return num_controllable;
}
