#ifndef SOLAR_TEST_H_
#define SOLAR_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Solar_Manager.h"
#include <Configurator.h>
#include <Site_Controller_Utils.h>

class Solar_Manager_Mock : public Solar_Manager {
public:
    Solar_Manager_Mock() {}

    // variable endpoint/helper functions
    void set_solarTotalActivePowerkW(float);
    void set_solarLastTotalActivePowerkW(float);
    void set_deadbandPercentage(float);
    void set_numSolarControllable(int);
    void set_solar_curtailment_state(solar_curtailment_states state);
    solar_curtailment_states get_solar_curtailment_state();
    void configure_solar_manager(int numParse, bool* primary_controller);
    void run_asset_instances(int numRunning);
    void set_demand_modes(void);
    void set_all_active_powers(std::vector<float>& active_powers);
    void set_all_rated_active_powers(float rated);
    void set_all_max_limited_active_powers(float max_ap);
    void set_all_min_limited_active_powers(float min_ap);
    float get_activePowerSetpoint(int index);
    void set_total_rated_active_power(float value);

    cJSON* generate_solarRoot(int numParse);
};

class solar_manager_test : public testing::Test {
public:
    Solar_Manager_Mock* solarMgr;
    bool* primary_controller;

    virtual void SetUp() {
        solarMgr = new Solar_Manager_Mock();
        primary_controller = new bool(true);
    }
};

// testing the solar curtailment algorithm
TEST_F(solar_manager_test, solar_curtailment) {
    // struct that has variables to configure for each test case
    struct testCase {
        int numInverters;
        float ratedActivePower;
        float targetActivePower;
        float totalActivePower;
        float lastTotalActivePower;
        float deadband;
        solar_curtailment_states initialCurtailmentState;
        solar_curtailment_states expectedCurtailmentState;
        std::vector<float> inverterActivePowers;
        std::vector<float> expectedActivePowerSetpoints;
    };

    std::vector<testCase> tests;  // an array with an element for each test case

    // Configure variables for each test case
    //               numInverters  ratedActivePower  targetActivePower  totalActivePower  lastTotalActivePower    deadband  initialCurtailmentState  expectedCurtailmentState
    //               inverterActivePowers            inverterLastActivePowers             expectedActivePowerSetpoints
    // test the standard switching possibilities
    tests.push_back({ 2, 100, 200, 200, 200, 0.05, no_curtailment, no_curtailment, { 100, 100 }, { 100, 100 } });
    tests.push_back({ 2, 100, 150, 200, 150, 0.05, no_curtailment, partial_curtailment, { 100, 100 }, { 100, 50 } });
    tests.push_back({ 2, 100, 150, 150, 140, 0.05, partial_curtailment, no_curtailment, { 75, 75 }, { 100, 100 } });
    tests.push_back({ 2, 100, 50, 150, 150, 0.05, partial_curtailment, full_curtailment, { 75, 75 }, { 50, 0 } });
    tests.push_back({ 2, 100, 120, 100, 100, 0.05, full_curtailment, partial_curtailment, { 100, 0 }, { 100, 20 } });
    tests.push_back({ 2, 100, 90, 88, 88, 0.05, full_curtailment, full_curtailment, { 88, 0 }, { 90, 0 } });
    // test the case of a non-prime inverter generating more power than the prime inverter while in partial curtailment
    tests.push_back({ 3, 100, 250, 250, 250, 0.05, partial_curtailment, partial_curtailment, { 80, 70, 100 }, { 100, 70, 100 } });
    tests.push_back({ 3, 100, 180, 280, 280, 0.05, partial_curtailment, partial_curtailment, { 80, 100, 100 }, { 100, 50, 50 } });

    for (auto it = tests.begin(); it != tests.end(); ++it) {
        SetUp();
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        capture_stdout();
        int testIndex = it - tests.begin() + 1;
        errorLog << "find_next_curtailment_state test " << testIndex << " of " << tests.size() << std::endl;

        // Configure Solar Manager with the correct number of PV inverters for this test case
        solarMgr->configure_solar_manager(it->numInverters, primary_controller);
        solarMgr->set_solar_curtailment_enabled(true);

        // Validate inputs
        float sumActivePowers = 0.0;
        for (auto activePowerIt = it->inverterActivePowers.begin(); activePowerIt != it->inverterActivePowers.end(); ++activePowerIt) {
            sumActivePowers += *activePowerIt;
        }
        if (sumActivePowers != it->totalActivePower || it->numInverters != (int)it->inverterActivePowers.size() || it->numInverters != (int)it->expectedActivePowerSetpoints.size()) {
            FPS_TEST_LOG("Test %d misconfigured\n", testIndex);
        }
        ASSERT_EQ(sumActivePowers, it->totalActivePower);
        ASSERT_EQ(it->numInverters, it->inverterActivePowers.size());
        ASSERT_EQ(it->numInverters, it->expectedActivePowerSetpoints.size());

        // Set test state
        solarMgr->set_numSolarControllable(it->numInverters);
        solarMgr->run_asset_instances(it->numInverters);
        solarMgr->set_all_active_powers(it->inverterActivePowers);
        solarMgr->set_solar_target_active_power(it->targetActivePower);
        solarMgr->set_solarTotalActivePowerkW(it->totalActivePower);
        solarMgr->set_solarLastTotalActivePowerkW(it->lastTotalActivePower);
        solarMgr->set_deadbandPercentage(it->deadband);
        solarMgr->set_solar_curtailment_state(it->initialCurtailmentState);
        solarMgr->set_all_rated_active_powers(it->ratedActivePower);
        solarMgr->set_total_rated_active_power(it->totalActivePower);
        solarMgr->set_all_max_limited_active_powers(it->ratedActivePower);
        solarMgr->set_all_min_limited_active_powers((it->ratedActivePower) * -1.0);

        // Calculate test results
        solarMgr->update_asset_data();
        solar_curtailment_states result = solarMgr->get_solar_curtailment_state();

        // Check test results against expected results
        if (result != it->expectedCurtailmentState) {
            failure = true;
        }
        EXPECT_EQ(it->expectedCurtailmentState, result);
        for (int i = 0; i < it->numInverters; ++i) {
            float resultant_active_power_setpoint = solarMgr->get_activePowerSetpoint(i);
            EXPECT_EQ(it->expectedActivePowerSetpoints[i], resultant_active_power_setpoint);
            if (it->expectedActivePowerSetpoints[i] != resultant_active_power_setpoint) {
                failure = true;
            }
        }

        // Wrap up test
        TearDown();
        release_stdout(failure);
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Begin helper functions
void Solar_Manager_Mock::set_solarTotalActivePowerkW(float value) {
    solarTotalActivePowerkW = value;
}

void Solar_Manager_Mock::set_solarLastTotalActivePowerkW(float value) {
    lastTotalActivePowerkW = value;
}

void Solar_Manager_Mock::set_deadbandPercentage(float value) {
    curtailment_deadband_percentage = value;
}

void Solar_Manager_Mock::set_numSolarControllable(int value) {
    numSolarControllable = value;
}

void Solar_Manager_Mock::set_solar_curtailment_state(solar_curtailment_states state) {
    solar_curtailment_state = state;
}
solar_curtailment_states Solar_Manager_Mock::get_solar_curtailment_state() {
    return solar_curtailment_state;
}

void Solar_Manager_Mock::run_asset_instances(int numRunning) {
    for (int i = 0; i < numRunning; ++i) {
        pSolar[i]->isAvail = true;
        pSolar[i]->isRunning = true;
    }
}

void Solar_Manager_Mock::set_demand_modes(void) {
    for (auto it = pAssets.begin(); it != pAssets.end(); ++it)
        (*it)->assetControl = Direct;
}

void Solar_Manager_Mock::set_all_active_powers(std::vector<float>& active_powers) {
    auto activePowerIT = active_powers.begin();
    for (auto it = pAssets.begin(); it != pAssets.end(); ++it)
    {
        (*it)->active_power.value.set(*(activePowerIT++));
    }
}

void Solar_Manager_Mock::set_all_max_limited_active_powers(float max_ap) {
    for (auto it = pSolar.begin(); it != pSolar.end(); ++it)
        (*it)->max_limited_active_power = max_ap;
}

void Solar_Manager_Mock::set_all_min_limited_active_powers(float min_ap) {
    for (auto it = pSolar.begin(); it != pSolar.end(); ++it)
        (*it)->min_limited_active_power = min_ap;
}

void Solar_Manager_Mock::set_total_rated_active_power(float value) {
    solarTotalRatedActivePower = value;
}

void Solar_Manager_Mock::set_all_rated_active_powers(float rated) {
    for (auto it = pAssets.begin(); it != pAssets.end(); ++it)
        (*it)->rated_active_power_kw = rated;
}

float Solar_Manager_Mock::get_activePowerSetpoint(int index)
{
    return pAssets[index]->active_power_setpoint.component_control_value.value_float;
}

void Solar_Manager_Mock::configure_solar_manager(int numParse, bool* primary_controller) {
    // Configure ESS Manager with the correct number of ESS instances for this test case
    cJSON* solarRoot = NULL;
    std::map <std::string, std::vector<Fims_Object*>> * const component_var_map = new std::map<std::string, std::vector<Fims_Object*>>;
    solarRoot = this->generate_solarRoot(numParse);
    Type_Configurator* solar_configurator = new Type_Configurator(this, component_var_map, primary_controller);
    solar_configurator->asset_type_root = solarRoot;
    solar_configurator->config_validation = false;
    bool configure_success = solar_configurator->create_assets();
    ASSERT_TRUE(configure_success);
}

/*
        The googletest needs actual asset instances to work with. The asset_create function is what gets the asset instance data from
        assets.json and configures the type manager. This function makes a fake version of that assets.json data for the type manager
        to be configured with. This is also where one would add specific initialization data such as variable initial values or how
        many assets to make.
*/
cJSON* Solar_Manager_Mock::generate_solarRoot(int numParse) {
    std::stringstream ss;
    // Insert the array header and first solar instance since it doesn't have a comma at the beginning
    ss << "{\"deadband_percentage\":0,\"asset_instances\":[{\"id\":\"solar_1\",\"name\":\"Solar Inverter 01\",\"demand_control\":\"Direct\",\"components\":[]}";
    // Insert any additional solar instances
    for (int i = 1; i < numParse; ++i) {
        ss << ",{\"id\":\"solar_" << i + 1 << "\",\"name\":\"Solar Inverter 0" << i + 1 << "\",\"demand_control\":\"Direct\",\"components\":[]}";
    }
    // Close the array
    ss << "]}";
    // Print the final solarRoot
    std::string s = ss.str();
    return cJSON_Parse(s.c_str());
}

/*
TEST_F(solar_manager_test, partial_curtailment_command_moving_down)
{
    // Configure with just one inverter
    SetUp();
    solarMgr->configure_solar_manager(NUM_INVERTERS, primary_controller);

    // Set values that are constant for all test cases
    solarMgr->set_solar_curtailment_state(partial_curtailment);
    solarMgr->set_solarTotalActivePowerkW(INVERTER_RATING*NUM_INVERTERS);
    solarMgr->set_primeInverterActivePower(INVERTER_RATING);

    // Iterate through test cases
    for (float i = 0.0; i < DEADBAND*4.0; i++)
    {
        // Set values that are specific for this test case
        solarMgr->set_solar_target_active_power(INVERTER_RATING+DEADBAND*2-i);
        // Run state machine
        solarMgr->find_next_curtailment_state();
        // Get & check result
        solar_curtailment_states result = solarMgr->get_solar_curtailment_state();
        if (i <= DEADBAND*2)
            EXPECT_EQ(partial_curtailment, result);
        else
            EXPECT_EQ(full_curtailment, result);
    }

    TearDown();
}

TEST_F(solar_manager_test, full_curtailment_command_moving_up)
{
    // Configure with just one inverter
    SetUp();
    solarMgr->configure_solar_manager(NUM_INVERTERS, primary_controller);

    // Set values that are constant for all test cases
    solarMgr->set_solar_curtailment_state(full_curtailment);
    solarMgr->set_solarTotalActivePowerkW(INVERTER_RATING*NUM_INVERTERS);
    solarMgr->set_primeInverterActivePower(INVERTER_RATING);

    // Iterate through test cases
    for (float i = 0.0; i < DEADBAND*4.0; i++)
    {
        // Set values that are specific for this test case
        solarMgr->set_solar_target_active_power(INVERTER_RATING-DEADBAND*2+i);
        // Run state machine
        solarMgr->find_next_curtailment_state();
        // Get & check result
        solar_curtailment_states result = solarMgr->get_solar_curtailment_state();
        if (i < DEADBAND*2+1)
            ASSERT_EQ(full_curtailment, result);
        else
            ASSERT_EQ(partial_curtailment, result);
    }

    TearDown();
}

// state transitions - active power based, simulate fluctuations in solar power
TEST_F(solar_manager_test, no_curtailment_power_reduction)
{
    // Configure with just one inverter
    SetUp();
    solarMgr->configure_solar_manager(NUM_INVERTERS, primary_controller);

    // Set values that are constant for all test cases
    solarMgr->set_solar_curtailment_state(no_curtailment);
    solarMgr->set_solar_target_active_power(INVERTER_RATING*NUM_INVERTERS);

    // Iterate through first set of test cases
    for (float i = INVERTER_RATING*6; i > 0; i-=50.0) //
    {
        // Set values that are specific for this test case
        solarMgr->set_solarTotalActivePowerkW(i);
        solarMgr->set_primeInverterActivePower(i/NUM_INVERTERS);
        // Run state machine
        solarMgr->find_next_curtailment_state();
        // Get & check result
        solar_curtailment_states result = solarMgr->get_solar_curtailment_state();
        ASSERT_EQ(no_curtailment, result);
    }

    // Iterate through second set of test cases
    for (float i = 0.0; i < INVERTER_RATING*NUM_INVERTERS; i+=50.0)
    {
        // Set values that are specific for this test case
        solarMgr->set_solarTotalActivePowerkW(i);
        solarMgr->set_primeInverterActivePower(i/NUM_INVERTERS);
        // Run state machine
        solarMgr->find_next_curtailment_state();
        // Get & check result
        solar_curtailment_states result = solarMgr->get_solar_curtailment_state();
        ASSERT_EQ(no_curtailment, result);
    }

    TearDown();
}
*/
#endif /* Solar_TEST_H_ */