/**
 * Solar_Manager.cpp
 * Source for Solar-specific Manager class
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
#include <Solar_Manager.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>

/****************************************************************************************/
Solar_Manager::Solar_Manager() : Type_Manager(SOLAR_TYPE_ID)
{
    numSolarControllable = 0;
    numSolarStartable = 0;
    numSolarStandby = 0;
    solarTotalActivePowerkW = 0.0;
    solarTotalReactivePowerkVAR = 0.0;
    solarTotalApparentPowerkVA = 0.0;
    solarTotalUncontrollableActivePowerkW = 0.0;

    solarTotalMaxPotentialActivePower = 0.0;
    solarTotalMinPotentialActivePower = 0.0;
    solarTotalPotentialReactivePower = 0.0;
    solarTotalPotentialApparentPower = 0.0;

    solarTotalRatedActivePower = 0.0;
    solarTotalRatedReactivePower = 0.0;
    solarTotalRatedApparentPower = 0.0;

    solarTotalNameplateActivePower = 0.0;
    solarTotalNameplateReactivePower = 0.0;
    solarTotalNameplateApparentPower = 0.0;

    solarTargetActivePowerkW = 0.0;
    solarTargetReactivePowerkVAR = 0.0;

    solar_curtailment_enabled = false;
    solar_curtailment_state = no_curtailment;
    solar_prime_index = 0;
    curtailment_deadband_percentage = 0.0;
}

/****************************************************************************************/
Solar_Manager::~Solar_Manager()
{
    if (!pSolar.empty()) for (size_t j = 0; j < pSolar.size(); j++) if (pSolar[j] != NULL) delete pSolar[j];
}

int Solar_Manager::get_num_solar_startable(void)
{
    return numSolarStartable;
}

int Solar_Manager::get_num_solar_in_standby(void)
{
    return numSolarStandby;
}

int Solar_Manager::get_num_solar_controllable(void)
{
    return numSolarControllable;
}

float Solar_Manager::get_solar_total_active_power(void)
{
   return solarTotalActivePowerkW; 
}

float Solar_Manager::get_solar_total_reactive_power(void)
{
   return solarTotalReactivePowerkVAR; 
}

float Solar_Manager::get_solar_total_uncontrollable_active_power(void)
{
    return solarTotalUncontrollableActivePowerkW;
}

float Solar_Manager::get_solar_total_nameplate_active_power(void)
{
    return solarTotalNameplateActivePower;
}

float Solar_Manager::get_solar_total_nameplate_reactive_power(void)
{
    return solarTotalNameplateReactivePower;
}

float Solar_Manager::get_solar_total_nameplate_apparent_power(void)
{
    return solarTotalNameplateApparentPower;
}

bool Solar_Manager::enter_standby_all_solar(void)
{
    for (int i=0; i < numParsed; i++)
    {
        if (pSolar[i]->is_controllable())
            pSolar[i]->enter_standby();
    }
    return true;
}

bool Solar_Manager::exit_standby_all_solar(void)
{
    for (int i=0; i < numParsed; i++)
    {
        if (pSolar[i]->in_standby() && pSolar[i]->is_available())
            pSolar[i]->exit_standby();
    }
    return true;
}

bool Solar_Manager::start_all_solar(void)
{
    bool return_value = true;
    for (int i=0; i < numParsed; i++)
    {
        if (pSolar[i]->is_available() && !pSolar[i]->is_running())
            return_value = pSolar[i]->start() && return_value;
    }
    return return_value;
}

bool Solar_Manager::stop_all_solar(void)
{
    bool return_value = true;
    for (int i=0; i < numParsed; i++)
        // Asset does not need to be controllable to send stop command
        return_value = pSolar[i]->stop() && return_value;
    return return_value;
}

void Solar_Manager::set_solar_reactive_kvar_mode(int index)
{
    pSolar[index]->set_power_mode(REACTIVEPWR);
}

void Solar_Manager::set_solar_pwr_factor_mode(int index)
{
    pSolar[index]->set_power_mode(PWRFCTR);
}

/* power factor setpoints */
void Solar_Manager::set_solar_target_power_factor(float pwr_factor)
{
    for (int i = 0; i < numParsed; i++)
        pSolar[i]->set_power_factor_setpoint(pwr_factor);
}

float Solar_Manager::get_solar_total_rated_active_power(void)
{
    return solarTotalRatedActivePower;
}

float Solar_Manager::get_solar_total_rated_reactive_power(void)
{
    return solarTotalRatedReactivePower;
}

float Solar_Manager::get_solar_total_rated_apparent_power(void)
{
    return solarTotalRatedApparentPower;
}

float Solar_Manager::get_total_kW_discharge_limit(void)
{
    return total_kW_discharge_limit;
}

float Solar_Manager::get_solar_total_max_potential_active_power(void)
{
    return solarTotalMaxPotentialActivePower;
}

float Solar_Manager::get_solar_total_min_potential_active_power(void)
{
    return solarTotalMinPotentialActivePower;
}

float Solar_Manager::get_solar_total_potential_reactive_power(void)
{
    return solarTotalPotentialReactivePower;
}

void Solar_Manager::set_solar_target_reactive_power(float desiredkW)
{
    solarTargetReactivePowerkVAR = desiredkW;
}

void Solar_Manager::set_solar_target_active_power(float command)
{
    solarTargetActivePowerkW = command;
}

bool Solar_Manager::aggregate_solar_data(void)
{

    FPS_DEBUG_LOG("Processing Solar data; Solar_Manager::aggregate_solar_data\n");

    // clear member variables
    numAvail = 0;
    numRunning = 0;
    numSolarControllable = 0;
    numSolarStartable = 0;
    numSolarStandby = 0;

    solarTotalActivePowerkW = 0.0;
    solarTotalReactivePowerkVAR = 0.0;
    solarTotalApparentPowerkVA = 0.0;
    solarTotalUncontrollableActivePowerkW = 0.0;

    solarTotalNameplateActivePower = 0.0;
    solarTotalNameplateReactivePower = 0.0;
    solarTotalNameplateApparentPower = 0.0;

    solarTotalMaxPotentialActivePower = 0.0;
    solarTotalMinPotentialActivePower = 0.0;
    solarTotalPotentialReactivePower = 0.0;

    solarTotalRatedActivePower = 0.0;
    solarTotalRatedReactivePower = 0.0;
    solarTotalRatedApparentPower = 0.0;
    total_kW_discharge_limit = 0.0f;

    // aggregate solar
    for (int i = 0; i < numParsed; i++)
    {
    	pSolar[i]->process_asset();

        // aggregate nameplate values
        // TODO: nameplate power values are based on site configuration, move to static section
        solarTotalNameplateActivePower   += pSolar[i]->get_rated_active_power();
        solarTotalNameplateReactivePower += pSolar[i]->get_rated_reactive_power();
        solarTotalNameplateApparentPower += pSolar[i]->get_rated_apparent_power();

        if (pSolar[i]->is_available())
            numAvail++;

        if (pSolar[i]->is_running())
            numRunning++;

        if (pSolar[i]->is_available() && !pSolar[i]->is_running())
            numSolarStartable++;

        if (pSolar[i]->is_available() && pSolar[i]->in_standby())
            numSolarStandby++;

        if (pSolar[i]->is_controllable())
        {
            numSolarControllable++;

            // aggregate measured values
            solarTotalActivePowerkW     += pSolar[i]->get_active_power();
            solarTotalReactivePowerkVAR += pSolar[i]->get_reactive_power();
            solarTotalApparentPowerkVA  += pSolar[i]->get_apparent_power();

            // aggregate potential values
            solarTotalMaxPotentialActivePower += pSolar[i]->get_max_potential_active_power();
            solarTotalMinPotentialActivePower += pSolar[i]->get_min_potential_active_power();
            solarTotalPotentialReactivePower += pSolar[i]->get_potential_reactive_power();

            // aggregate rated values
            solarTotalRatedActivePower   += pSolar[i]->get_rated_active_power();
            solarTotalRatedReactivePower += pSolar[i]->get_rated_reactive_power();
            solarTotalRatedApparentPower += pSolar[i]->get_rated_apparent_power();

            total_kW_discharge_limit += pSolar[i]->get_max_limited_active_power();
        }
        else
        {
             // only add to uncontrollable power if solar is uncontrollable for reasons besides a modbus disconnect
            if (!pSolar[i]->get_watchdog_fault()) {
                solarTotalUncontrollableActivePowerkW += pSolar[i]->get_active_power();
            }
        }
        
    }

    return false;
}

void Solar_Manager::generate_asset_type_summary_json(fmt::memory_buffer &buf, const char* const var)
{
    if (var == NULL) bufJSON_StartObject(buf); // summary {

    bufJSON_AddStringCheckVar(buf, "name", "Solar Summary", var);
    bufJSON_AddNumberCheckVar(buf, "num_solar_available", numAvail, var);
    bufJSON_AddNumberCheckVar(buf, "num_solar_running", numRunning, var);
    bufJSON_AddNumberCheckVar(buf, "solar_total_active_power", solarTotalActivePowerkW, var);
    bufJSON_AddNumberCheckVar(buf, "solar_total_reactive_power", solarTotalReactivePowerkVAR, var);
    bufJSON_AddNumberCheckVar(buf, "solar_total_apparent_power", solarTotalApparentPowerkVA, var);
    bufJSON_AddBoolCheckVar(buf, "solar_curtailment_state", (solar_curtailment_state != 0), var);
    bufJSON_AddNumberCheckVar(buf, "solar_num_alarmed", get_num_alarmed(), var);
    bufJSON_AddNumberCheckVar(buf, "solar_num_faulted", get_num_faulted(), var);

    // Marked for deprecation
    bufJSON_AddNumberCheckVar(buf, "solar_total_alarms", get_num_active_alarms(), var);
    bufJSON_AddNumberCheckVar(buf, "solar_total_faults", get_num_active_faults(), var);

    if (var == NULL) bufJSON_EndObjectNoComma(buf); // } summary
}

// HybridOS Step 2: Process Asset Data
void Solar_Manager::process_asset_data(void)
{
    if (numParsed > 0)
        aggregate_solar_data();
}

// HybridOS Step 4: Update Asset Data
void Solar_Manager::update_asset_data(void)
{
    solar_prime_index = solar_prime_inverter_selection(); // select solar "prime" inverter, to be left uncurtailed
    if ((solar_prime_index != -1) && (numSolarControllable > 0))
    {
        // Runmode 1 solar curtailment algorithm
        if (solar_curtailment_enabled) {
            solar_prime_index = solar_prime_inverter_selection(); // select solar "prime" inverter, to be left uncurtailed
            if ((solar_prime_index != -1) && (numSolarControllable > 0))
            {
                solar_curtailment_state = find_next_curtailment_state(); // identify solar curtailment status
                calculate_solar_total_potential_active_power(); // update solar available power report, based on solar curtailment state
                calculate_solar_target_active_power();
                calculate_solar_target_reactive_power();
            }
        }
        // Runmode 2 divide power among all inverters proportional to rated power
        else if (numSolarControllable > 0) {
            for (auto asset : pSolar)
            {
                if (asset->is_controllable())
                {
                    asset->set_potential_active_power_limit(asset->get_rated_active_power());
                    float weightedTargetkW = solarTargetActivePowerkW * (asset->get_rated_active_power() / solarTotalRatedActivePower);
                    asset->set_active_power_setpoint(weightedTargetkW);
                    float weightedTargetkVAR = solarTargetReactivePowerkVAR * (asset->get_potential_reactive_power() / solarTotalPotentialReactivePower);
                    asset->set_reactive_power_setpoint(weightedTargetkVAR);
                }
            }
        }
        // Update assets
        for(int i = 0; i < numParsed; i++)
            pSolar[i] -> update_asset();
        lastTotalActivePowerkW = solarTotalActivePowerkW;
    }
    for(int i = 0; i < numParsed; i++)
        pSolar[i] -> update_asset();
    lastTotalActivePowerkW = solarTotalActivePowerkW;
}

int Solar_Manager::solar_prime_inverter_selection(void)
{
    // identify if any solar exists
    if (numParsed <= 0)
        return -1;

    // if last check resulted in index of -1, reset it for this check
    if (solar_prime_index == -1)
        solar_prime_index = 0;

    // identify if current prime is "controllable"
    if (pSolar[solar_prime_index]->is_controllable())
    {
        return solar_prime_index;
    }
    else
    {
        // select new prime inverter
        for (int i = 0; i < numParsed; i++) // using numParsed to track full index of solar arrays
        {
            if (pSolar[i]->is_controllable())
            {
                return i;
            }
        }
    }
    return -1;
}

// find if curtailment state needs to switch based on the current output of the solar inverters and what Site Manager is asking for
solar_curtailment_states Solar_Manager::find_next_curtailment_state()
{
    solar_curtailment_states new_state = no_curtailment;
    float deadband_power = curtailment_deadband_percentage * solarTotalRatedActivePower;
    // state machine
    switch (solar_curtailment_state)
    {
        case no_curtailment:
            if (solarTargetActivePowerkW < (solarTotalActivePowerkW - deadband_power)) // command is outside of deadband limit, moving downwards
                new_state = partial_curtailment;
            else
                new_state = no_curtailment;
            break;

        case partial_curtailment:
            if (solarTargetActivePowerkW < pSolar[solar_prime_index]->get_active_power()) // command is less than the active power produced by "prime" inverter
                new_state = full_curtailment;
            else if (solarTargetActivePowerkW > (solarTotalActivePowerkW - deadband_power) && solarTotalActivePowerkW > lastTotalActivePowerkW) // command is outside of deadband limit, moving upwards
                new_state = no_curtailment;
            else
                new_state = partial_curtailment;
            break;

        case full_curtailment:
            if ((solarTargetActivePowerkW > (pSolar[solar_prime_index]->get_active_power() + deadband_power)) || // command has exceeded deadband limit, moving upwards
                (solarTargetActivePowerkW == pSolar[solar_prime_index]->get_rated_active_power())) // command has exceeded single inverter rating, moving upwards
                new_state = partial_curtailment;
            else
                new_state = full_curtailment;
            break;
    }

    return new_state;
}

// calculate how much active power each inverter could be commanded to produce
void Solar_Manager::calculate_solar_total_potential_active_power(void)
{
    switch (solar_curtailment_state)
    {
        case no_curtailment: // report total active power of solar inverters, "actual" production
            for (int i=0; i < numParsed; i++)
            {
                if (pSolar[i]->is_controllable())
                    pSolar[i]->set_potential_active_power_limit(pSolar[i]->get_active_power());
            }
            break;

        case partial_curtailment: // report prime inverter power, scaled by units controllable
            for (int i=0; i < numParsed; i++)
            {
                if (pSolar[i]->is_controllable())
                {   // if a non-prime inverter is generating less power than the prime inverter, give it the prime inverter's generation as its limit
                    if (pSolar[i]->get_active_power() < pSolar[solar_prime_index]->get_active_power())
                        pSolar[i]->set_potential_active_power_limit(pSolar[solar_prime_index]->get_active_power());
                    // if a non-prime inverter is generating more power than the prime inverter, give it a little above its own generation as its limit
                    else
                        pSolar[i]->set_potential_active_power_limit(pSolar[i]->get_active_power() * (1+curtailment_deadband_percentage));
                }
            }
            break;

        case full_curtailment: // report rated power of single inverter
            for (int i=0; i < numParsed; i++)
            {
                if (pSolar[i]->is_controllable())
                {
                    if (i != solar_prime_index)
                        pSolar[i]->set_potential_active_power_limit(0);
                    else
                        pSolar[i]->set_potential_active_power_limit(pSolar[solar_prime_index]->get_rated_active_power());
                }
            }
            break;
    }
}

// distribute power commands across solar inverters that are "controllable" based on curtailment state
void Solar_Manager::calculate_solar_target_active_power(void)
{
    // if-else statements instead of switch statement so local variables can be declared within the code blocks
    if (solar_curtailment_state == no_curtailment)
    {
        // all inverters allowed to generate up to max rated power
        for (int i = 0; i < numParsed; ++i)
        {
            if (pSolar[i]->is_controllable())
            {
                pSolar[i]->set_active_power_setpoint(pSolar[i]->get_rated_active_power());
            }
        }
    }
    else if (solar_curtailment_state == partial_curtailment)
    {
        // maintain prime inverter at max rated power. distribute remaining power across curtailed inverters
        float targetPowerWithoutPrime = solarTargetActivePowerkW - pSolar[solar_prime_index]->get_active_power();
        float totalPowerWithoutPrime =  solarTotalActivePowerkW  - pSolar[solar_prime_index]->get_active_power();
        float numNonPrime = numSolarControllable - 1;
        for (int i = 0; i < numParsed; ++i)
        {
            if (i == solar_prime_index)
            {
                pSolar[i]->set_active_power_setpoint(pSolar[i]->get_rated_active_power());
            }
            else if (pSolar[i]->is_controllable())
            {
                float weightedShare = (totalPowerWithoutPrime == 0) ? (targetPowerWithoutPrime/numNonPrime) : (targetPowerWithoutPrime * (pSolar[i]->get_active_power() / totalPowerWithoutPrime));
                pSolar[i]->set_active_power_setpoint(weightedShare);
            }
        }
    }
    else // full curtailment
    {
        // direct pass through command to prime inverter. drive curtailed invterters to zero power
        for (int i = 0; i < numParsed; ++i)
        {
            if (i == solar_prime_index)
            {
                pSolar[i]->set_active_power_setpoint(solarTargetActivePowerkW);
            }
            else if (pSolar[i]->is_controllable())
            {
                pSolar[i]->set_active_power_setpoint(0);
            }
        }
    }
}

void Solar_Manager::calculate_solar_target_reactive_power(void)
{
    float solar_prime_reactive_power_command = 0.0;
    float solar_curtailed_reactive_power_command = 0.0;

    if (solarTargetReactivePowerkVAR > 0.0)
    {
        if (solar_curtailment_state == no_curtailment)
        {
            // all solar inverters have the same reactive power rating
            if (numSolarControllable > 0)
                solar_prime_reactive_power_command = solar_curtailed_reactive_power_command = (solarTargetReactivePowerkVAR / (float)numSolarControllable);
        }
        else
        {
            if (numSolarControllable <= 1)
            {
                solar_prime_reactive_power_command = solarTargetReactivePowerkVAR;
                solar_curtailed_reactive_power_command = 0.0;
            }
            else
            {
                // balance apparent power across solar inverters
                std::tie(solar_prime_reactive_power_command, solar_curtailed_reactive_power_command) = calculate_solar_reactive_power_commands();
            }           
        }
    }
    
    // distribute power to solar prime inverter
    pSolar[solar_prime_index]->set_reactive_power_setpoint(solar_prime_reactive_power_command);

    // distribute power to solar curtailed inverters
    for (int i = 0; i < numParsed; i++)
    {
        if ((i != solar_prime_index) && pSolar[i]->is_controllable())
        {
            pSolar[i]->set_reactive_power_setpoint(solar_curtailed_reactive_power_command);
        }
    }
}

// NOTE: Hello new developer. If you are working on this function and you don't understand it, please talk to John Calcagni.
std::tuple<float,float> Solar_Manager::calculate_solar_reactive_power_commands(void)
{
    float spp = 0.0, spq = 0.0, sps = 0.0; // solar prime - active power command, reactive power command, apparent power rated
    float bcp = 0.0, bcq = 0.0, brs = 0.0; // balanced solar - active power command, reactive power command, apparent power rated
    float bnq = 0.0; // solar curtailed reactive power command
    float tq = fabs(solarTargetReactivePowerkVAR);
    bool direction = (solarTargetReactivePowerkVAR < 0) ? true : false;

    spp = pSolar[solar_prime_index]->get_active_power();
    sps = pSolar[solar_prime_index]->get_rated_apparent_power();
    
    bcp = solarTargetActivePowerkW - spp;
    brs = solarTotalRatedApparentPower - sps;

    if ((spp/sps) > (sqrt(pow(bcp, 2) + pow(tq, 2))/brs)) // inherent apparent power imbalance, spread across curtailed inverters
    {
        spq = 0.0;
    }
    else if ((sqrt(pow(spp, 2)+pow(tq,2))/sps) < (bcp/brs)) // only two inverters are functional, and command is relatively small
    {
        spq = tq;
    }
    else // normal operation
    {
        if (sps != brs)
        {
            spq = ((pow(sps, 2) * tq) - sqrt((pow(brs, 2) * pow(sps, 2) * pow(spp, 2)) + (pow(brs, 2) * pow(sps, 2) * pow(bcp, 2)) + (pow(brs, 2) * pow(sps, 2) * pow(tq, 2)) - (pow(brs, 4) * pow(spp, 2)) - (pow(sps, 4) * pow(bcp, 2)))) / (pow(sps, 2) - pow(brs, 2));
        }
        else
        {
            spq = ((pow(bcp, 2) + pow(tq, 2) - pow(spp, 2))/(2.0 * tq));
        }
    }

    // upper and lower bounds for solar prime active power command
    spq = (spq < 0) ? 0 : ((spq > tq) ? tq : spq);

    // calculate the remaining reactive power to be distributed
    bcq = tq - spq;
    bnq = bcq / (float) (numSolarControllable-1);

    if (direction)
    {
        spq *= -1.0;
        bnq *= -1.0;
    }
    return std::make_tuple(spq, bnq);
}

/**
 * Enable or disable runmode 1 solar curtailment algorithm
 * @param enable solar curtailment is enabled if and only if enable is true
 */
void Solar_Manager::set_solar_curtailment_enabled(bool enable)
{
    solar_curtailment_enabled = enable;
}

/****************************************************************************************/
/*
    Overriding configuration functions
*/
void Solar_Manager::configure_base_class_list()
{
    pAssets.assign(pSolar.begin(), pSolar.end());
}

Asset* Solar_Manager::build_new_asset(void)
{
    Asset_Solar* asset = new Asset_Solar;
    if (asset == NULL)
    {
        FPS_ERROR_LOG("Solar_Manager::build_new_asset ~ Solar %ld: Memory allocation error.\n", pSolar.size()+1);
    }
    numParsed++;
    return asset;
}

void Solar_Manager::append_new_asset(Asset* asset)
{
    pSolar.push_back((Asset_Solar*)asset);
}

// After configuring individual asset instances, this function finishes configuring the Solar Manager
bool Solar_Manager::configure_type_manager(Type_Configurator* configurator)
{
    cJSON* solarRoot = configurator->asset_type_root;
    // Parse deadband percent for solar curtailment
    cJSON* object = cJSON_HasObjectItem(solarRoot, "deadband_percentage") ? cJSON_GetObjectItem(solarRoot, "deadband_percentage") : NULL;
    if (object == NULL)
    {
        FPS_ERROR_LOG("Solar_Manager::configure_type_manager ~ Failed to find deadband_percentage in config.\n");
        return false;
    }
    curtailment_deadband_percentage = (float)object->valuedouble;
    
    return true;
}
