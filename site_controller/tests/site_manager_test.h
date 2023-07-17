#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Asset_Manager.h"
#include "Site_Manager.h"
#include <Site_Controller_Utils.h>
#include "Types.h"
#include <test_tools.h>
#include "frequency_response_test.h"


class site_manager_test : public Site_Manager, public testing::Test
{
	public:
    site_manager_test() : Site_Manager(nullptr) {}
	site_manager_test(Version* v):Site_Manager(v){}

    void SetUp()
    {
        // For some reason the Asset_Cmd_Object holds on to its member variables across tests
        // TODO: Look into this more for a better solution
        // All other Site Manager member variables seem to be cleared across tests so there are not issues
        asset_cmd = Asset_Cmd_Object();
    }
};

//ess discharge 
// Verify load calculation using rolling average
TEST_F(site_manager_test, calculate_site_kW_load)
{
    int const num_tests = 16;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        int buffer_size;
        std::vector<float> ess_actual;
        std::vector<float> feed_actual;
        std::vector<float> expected_load;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables for each test case
    array[0]  = {1, {100.0f,105.0f,110.0f,115.0f,120.0f},    {-100.0f,-105.0f,-110.0f,-115.0f,-120.0f}, {0.0f,0.0f,0.0f,0.0f,0.0f}};           // zero load
    array[1]  = {1, {0.0f,0.0f,0.0f,0.0f,0.0f},              {100.0f,100.0f,100.0f,100.0f,100.0f},      {100.0f,100.0f,100.0f,100.0f,100.0f}}; // constant load from POI
    array[2]  = {1, {100.0f,100.0f,100.0f,100.0f,100.0f},    {0.0f,0.0f,0.0f,0.0f,0.0f},                {100.0f,100.0f,100.0f,100.0f,100.0f}}; // constant load from ESS
    array[3]  = {1, {100.0f,100.0f,100.0f,100.0f,100.0f},    {100.0f,100.0f,100.0f,100.0f,100.0f},      {200.0f,200.0f,200.0f,200.0f,200.0f}}; // constant load from POI w/ ESS
    array[4]  = {1, {0.0f,0.0f,0.0f,0.0f,0.0f},              {100.0f,105.0f,110.0f,115.0f,120.0f},      {100.0f,105.0f,110.0f,115.0f,120.0f}}; // changing load from POI
    array[5]  = {1, {100.0f,105.0f,110.0f,115.0f,120.0f},    {0.0f,0.0f,0.0f,0.0f,0.0f},                {100.0f,105.0f,110.0f,115.0f,120.0f}}; // changing load from ESS
    array[6]  = {1, {100.0f,105.0f,110.0f,-115.0f,-120.0f},  {-100.0f,-105.0f,-110.0f,-110.0f,120.0f},  {0.0f,0.0f,0.0f,230.0f,0.0f}};         // ESS changes, feed is delayed
    array[7]  = {1, {100.0f,105.0f,-110.0f,-115.0f,-120.0f}, {0.0f,-5.0f,-10.0f,-15.0f,220.0f},         {100.0f,100.0f,-220.0f,-230.0f,0.0f}}; // ESS changes w/ load, feed is delayed
    array[8]  = {5, {100.0f,105.0f,110.0f,115.0f,120.0f},    {-100.0f,-105.0f,-110.0f,-115.0f,-120.0f}, {0.0f,0.0f,0.0f,0.0f,0.0f}};           // Same tests with smoothing
    array[9]  = {5, {0.0f,0.0f,0.0f,0.0f,0.0f},              {100.0f,100.0f,100.0f,100.0f,100.0f},      {20.0f,40.0f,60.0f,80.0f,100.0f}};
    array[10] = {5, {100.0f,100.0f,100.0f,100.0f,100.0f},    {0.0f,0.0f,0.0f,0.0f,0.0f},                {20.0f,40.0f,60.0f,80.0f,100.0f}};
    array[11] = {5, {100.0f,100.0f,100.0f,100.0f,100.0f},    {100.0f,100.0f,100.0f,100.0f,100.0f},      {40.0f,80.0f,120.0f,160.0f,200.0f}};
    array[12] = {5, {0.0f,0.0f,0.0f,0.0f,0.0f},              {100.0f,105.0f,110.0f,115.0f,120.0f},      {20.0f,41.0f,63.0f,86.0f,110.0f}};
    array[13] = {5, {100.0f,105.0f,110.0f,115.0f,120.0f},    {0.0f,0.0f,0.0f,0.0f,0.0f},                {20.0f,41.0f,63.0f,86.0f,110.0f}};
    array[14] = {5, {100.0f,105.0f,110.0f,-115.0f,-120.0f},  {-100.0f,-105.0f,-110.0f,-110.0f,120.0f},  {0.0f,0.0f,0.0f,-45.0f,-45.0f}};
    array[15] = {5, {100.0f,105.0f,-110.0f,-115.0f,-120.0f}, {0.0f,-5.0f,-10.0f,-10.0f,120.0f},         {20.0f,40.0f,16.0f,-9.0f,-9.0f}};
 
    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        asset_cmd.create_site_kW_load_buffer(array[i].buffer_size);
        for (int j = 0; j < array[i].buffer_size; j++)
        {
            // Only print messages to log if a test fails
            bool failure = false;
            std::stringstream errorLog;
            // // Capture any prints within site controller that might be present in debug mode
            capture_stdout();

            asset_cmd.set_ess_actual_kW(array[i].ess_actual[j]);
            asset_cmd.set_feeder_actual_kW(array[i].feed_actual[j]);
            asset_cmd.calculate_site_kW_load();

            errorLog << "calculate_site_kW_load() test " << i+1 << " of " << num_tests << std::endl;
            // failure conditions
            failure = asset_cmd.get_site_kW_load() != array[i].expected_load[j];
            EXPECT_EQ(asset_cmd.get_site_kW_load(), array[i].expected_load[j]);

            // Release stdout so we can write again
            release_stdout(failure);
            // Print the test id if failure
            if (failure)
                std::cout << errorLog.str() << std::endl;

        }
    }
}

// Calculate the amount of additional load compensation that should be provided for slewed features
TEST_F(site_manager_test, track_slewed_load)
{
    //struct that has variables to configure for each test case
    struct tests 
    {
        load_compensation load_method;
        float starting_demand;
        float new_demand;
        float additional_load;
        int slew_rate;
        float expected_load;
    };

    std::vector<tests> tests = {
        {NO_COMPENSATION, 0.0f, 0.0f, 0.0f, 10, 0.0f},
        {LOAD_MINIMUM, 0.0f, 500.0f, 500.0f, 10, 0.1f}, // Test: just started slewing, no command
        {LOAD_MINIMUM, 0.0f, 1000.0f, 0.0f, 10, 0.0f}, // Command in same direction
        {LOAD_MINIMUM, 0.0f, -1000.0f, 500.0f, 10, 0.05f}, // Command in opposite direction
        {LOAD_OFFSET, 0.0f, 500.0f, 500.0f, 10, 0.1f}, // No command
        {LOAD_OFFSET, 0.0f, 1500.0f, 500.0f, 10, 0.033f}, // Command in same direction
        {LOAD_OFFSET, 0.0f, -1000.0f, 500.0f, 10, 0.05f}, // Command in opposite direction
        {LOAD_MINIMUM, 250.0f, 500.0f, 500.0f, 10, 250.0f}, // Test: halfway to target, no command
        {LOAD_MINIMUM, 500.0f, 1000.0f, 0.0f, 10, 0.0f}, // Command in same direction
        {LOAD_MINIMUM, -500.0f, -1000.0f, 500.0f, 10, 250.0f}, // Command in opposite direction
        {LOAD_OFFSET, 250.0f, 500.0f, 500.0f, 10, 250.0f}, // No command
        {LOAD_OFFSET, 750.0f, 1500.0f, 500.0f, 10, 250.0f}, // Command in same direction
        {LOAD_OFFSET, -500.0f, -1000.0f, 500.0f, 10, 250.0f}, // Command in opposite direction
        {LOAD_MINIMUM, 500.0f, 500.0f, 500.0f, 10, 500.0f}, // Test: at target, no command
        {LOAD_MINIMUM, 1000.0f, 1000.0f, 0.0f, 10, 0.0f}, // Command in same direction
        {LOAD_MINIMUM, -1000.0f, -1000.0f, 500.0f, 10, 500.0f}, // Command in opposite direction
        {LOAD_OFFSET, 500.0f, 500.0f, 500.0f, 10, 500.0f}, // No command
        {LOAD_OFFSET, 1500.0f, 1500.0f, 500.0f, 10, 500.0f}, // Command in same direction
        {LOAD_OFFSET, -1000.0f, -1000.0f, 500.0f, 10, 500.0f}, // Command in opposite direction
    };

    uint test_id = 1;
    //iterate through each test case and get results
    for (auto test : tests)
    {
        test_logger t_log("track_slewed_load", test_id++, tests.size());
        // Setup feature slew object at starting demand
        export_target_kW_cmd_slew.reset_slew_target(10000);
        export_target_kW_cmd_slew.update_slew_target(test.starting_demand);
        usleep(10000); // wait one iteration
        export_target_kW_cmd_slew.set_slew_rate(test.slew_rate);
        export_target_kW_cmd_slew.update_slew_target(test.starting_demand);
        asset_cmd.set_site_kW_demand(export_target_kW_cmd_slew.get_slew_target(test.new_demand));
        asset_cmd.track_slewed_load(test.load_method, test.new_demand, test.additional_load, export_target_kW_cmd_slew);
        t_log.range_results.push_back({test.expected_load, 0.1f, asset_cmd.get_additional_load_compensation(), "additional_load_compensation"});
        t_log.check_solution();
    }
}

// Ensures that output from feature (combination of demand, requests and load) produces valid feature and site demand values
TEST_F(site_manager_test, calculate_feature_kW_demand)
{

    //struct that has variables to configure for each test case
    struct tests 
    {
        float site_demand;
        float ess_request;
        float gen_request;
        float solar_request;
        load_compensation load_inclusion;
        float load;
        float expected_feature_demand;
        float expected_site_demand;
    };

    std::vector<tests> tests = {
        {0.0f,    0.0f,    0.0f,    0.0f,   NO_COMPENSATION, 0.0f,   0.0f,    0.0f},    // Nothing
        {0.0f,    500.0f,  0.0f,    0.0f,   NO_COMPENSATION, 0.0f,   500.0f,  500.0f},  // ess request
        {0.0f,    -500.0f, 0.0f,    0.0f,   NO_COMPENSATION, 0.0f,   -500.0f, -500.0f}, // negative ess request
        {0.0f,    500.0f,  1500.0f, 750.0f, NO_COMPENSATION, 0.0f,   2750.0f, 2750.0f}, // all assets
        {0.0f,    -500.0f, 1500.0f, 750.0f, NO_COMPENSATION, 0.0f,   1750.0f, 1750.0f}, // all assets negative ess
        {1000.0f, 0.0f,    0.0f,    0.0f,   NO_COMPENSATION, 0.0f,   1000.0f, 1000.0f}, // Same tests with demand
        {1000.0f, 500.0f,  0.0f,    0.0f,   NO_COMPENSATION, 0.0f,   1500.0f, 1500.0f}, // ess request
        {1000.0f, -500.0f, 0.0f,    0.0f,   NO_COMPENSATION, 0.0f,   500.0f,  500.0f},  // negative ess request
        {1000.0f, 500.0f,  1500.0f, 750.0f, NO_COMPENSATION, 0.0f,   3750.0f, 3750.0f}, // all assets
        {1000.0f, -500.0f, 1500.0f, 750.0f, NO_COMPENSATION, 0.0f,   2750.0f, 2750.0f}, // all assets negative ess
        {0.0f,    0.0f,    0.0f,    0.0f,   LOAD_OFFSET,     0.0f,   0.0f,    0.0f},    // Same tests with load offset
        {0.0f,    0.0f,    0.0f,    0.0f,   NO_COMPENSATION, 500.0f, 0.0f,    0.0f},    // Additional test with load but not enabled
        {0.0f,    500.0f,  0.0f,    0.0f,   LOAD_OFFSET,     500.0f, 1000.0f, 1000.0f}, // ess request
        {0.0f,    -500.0f, 0.0f,    0.0f,   LOAD_OFFSET,     500.0f, 0.0f,    0.0f},    // negative ess request
        {0.0f,    500.0f,  1500.0f, 750.0f, LOAD_OFFSET,     500.0f, 3250.0f, 3250.0f}, // all assets
        {0.0f,    -500.0f, 1500.0f, 750.0f, LOAD_OFFSET,     500.0f, 2250.0f, 2250.0f}, // all assets negative ess
        {1000.0f, 0.0f,    0.0f,    0.0f,   LOAD_OFFSET,     500.0f, 1500.0f, 1500.0f}, // Same tests with demand
        {1000.0f, 500.0f,  0.0f,    0.0f,   LOAD_OFFSET,     500.0f, 2000.0f, 2000.0f}, // ess request
        {1000.0f, -500.0f, 0.0f,    0.0f,   LOAD_OFFSET,     500.0f, 1000.0f, 1000.0f}, // negative ess request
        {1000.0f, 500.0f,  1500.0f, 750.0f, LOAD_OFFSET,     500.0f, 4250.0f, 4250.0f}, // all assets
        {1000.0f, -500.0f, 1500.0f, 750.0f, LOAD_OFFSET,     500.0f, 3250.0f, 3250.0f}, // all assets negative ess
        {0.0f,    0.0f,    0.0f,    0.0f,   LOAD_MINIMUM,    0.0f,   0.0f,    0.0f},    // Same tests with load minimum
        {0.0f,    500.0f,  0.0f,    0.0f,   LOAD_MINIMUM,    500.0f, 500.0f,  500.0f},  // ess request
        {0.0f,    -500.0f, 0.0f,    0.0f,   LOAD_MINIMUM,    500.0f, 0.0f,    0.0f},    // negative ess request
        {0.0f,    500.0f,  1500.0f, 750.0f, LOAD_MINIMUM,    500.0f, 2750.0f, 2750.0f}, // all assets
        {0.0f,    -500.0f, 1500.0f, 750.0f, LOAD_MINIMUM,    500.0f, 1750.0f, 1750.0f}, // all assets negative ess
        {1000.0f, 0.0f,    0.0f,    0.0f,   LOAD_MINIMUM,    500.0f, 1000.0f, 1000.0f}, // Same tests with demand
        {1000.0f, 500.0f,  0.0f,    0.0f,   LOAD_MINIMUM,    500.0f, 1500.0f, 1500.0f}, // ess request
        {1000.0f, -500.0f, 0.0f,    0.0f,   LOAD_MINIMUM,    500.0f, 500.0f,  500.0f},  // negative ess request
        {1000.0f, 500.0f,  1500.0f, 750.0f, LOAD_MINIMUM,    500.0f, 3750.0f, 3750.0f}, // all assets
        {1000.0f, -500.0f, 1500.0f, 750.0f, LOAD_MINIMUM,    500.0f, 2750.0f, 2750.0f}, // all assets negative ess
    };

    uint test_id = 1;
    //iterate through each test case and get results
    for (auto test : tests)
    {
        test_logger t_log("calculate_feature_kW_demand", test_id++, tests.size());
        asset_cmd.set_ess_max_potential_kW(10000);
        asset_cmd.set_ess_min_potential_kW(-10000);
        asset_cmd.set_gen_max_potential_kW(10000);
        asset_cmd.set_solar_max_potential_kW(10000);
        asset_cmd.set_site_kW_demand(test.site_demand);
        asset_cmd.set_ess_kW_request(test.ess_request);
        asset_cmd.set_gen_kW_request(test.gen_request);
        asset_cmd.set_solar_kW_request(test.solar_request);
        asset_cmd.set_load_compensation_method(test.load_inclusion);
        asset_cmd.set_site_kW_load(test.load);
        asset_cmd.track_unslewed_load(test.load_inclusion);
        asset_cmd.calculate_feature_kW_demand(0);
        t_log.float_results.push_back({asset_cmd.get_feature_kW_demand(), test.expected_feature_demand, "feature"});
        t_log.float_results.push_back({asset_cmd.get_site_kW_demand(), test.expected_feature_demand, "demand"});
        t_log.check_solution();
    }
}

// Verify production limits used for power dispatch based on final result of feature pipeline
TEST_F(site_manager_test, calculate_site_kW_production_limits)
{
    int const num_tests = 42;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float feature_demand;
        float site_demand;
        float ess_request;
        float gen_request;
        float solar_request;
        bool load_inclusion;
        float additional_load;
        float expected_charge_production;
        float expected_discharge_production;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables for each test case
    array[0]  = {0.0f,     0.0f,     0.0f,     0.0f,   0.0f,   false, 200.0f, 0.0f,     0.0f};    // None (load present but disabled)
    array[1]  = {0.0f,     0.0f,     -1200.0f, 500.0f, 700.0f, false, 200.0f, -1200.0f, 1200.0f}; // - ess + gen + solar
    array[2]  = {-1000.0f, -1000.0f, -1000.0f, 0.0f,   0.0f,   false, 200.0f, -1000.0f, 0.0f};    // - ess
    array[3]  = {-800.0f,  -800.0f,  -2000.0f, 500.0f, 700.0f, false, 200.0f, -2000.0f, 1200.0f}; // - ess + gen + solar, net negative
    array[4]  = {1000.0f,  1000.0f,  1000.0f,  0.0f,   0.0f,   false, 200.0f, 0.0f,     1000.0f}; // + ess
    array[5]  = {2200.0f,  2200.0f,  1000.0f,  500.0f, 700.0f, false, 200.0f, 0.0f,     2200.0f}; // + ess + gen + solar
    array[6]  = {700.0f,   700.0f,   -500.0f,  500.0f, 700.0f, false, 200.0f, -500.0f,  1200.0f}; // - ess + gen + solar, net positive
    array[7]  = {0.0f,     0.0f,     -200.0f,  0.0f,   0.0f,   true,  200.0f, -200.0f,  200.0f};  // Same tests with load enabled
    array[8]  = {0.0f,     0.0f,     -1400.0f, 500.0f, 700.0f, true,  200.0f, -1400.0f, 1400.0f}; // - ess + gen + solar
    array[9]  = {-1000.0f, -1000.0f, -1200.0f, 0.0f,   0.0f,   true,  200.0f, -1200.0f, 200.0f};  // - ess
    array[10] = {-800.0f,  -800.0f,  -2200.0f, 500.0f, 700.0f, true,  200.0f, -2200.0f, 1400.0f}; // - ess + gen + solar, net negative
    array[11] = {1000.0f,  1000.0f,  800.0f,   0.0f,   0.0f,   true,  200.0f, 0.0f,     1000.0f}; // + ess
    array[12] = {2400.0f,  2400.0f,  1200.0f,  400.0f, 600.0f, true,  200.0f, 0.0f,     2400.0f}; // + ess + gen + solar
    array[13] = {700.0f,   700.0f,   -300.0f,  400.0f, 600.0f, true,  200.0f, -500.0f,  1200.0f}; // - ess + gen + solar, net positive
    array[14] = {0.0f,     -300.0f,  0.0f,     0.0f,   0.0f,   false, 200.0f, -300.0f,  0.0f};    // Same test with negative standalone modification
    array[15] = {0.0f,     -300.0f,  -1200.0f, 500.0f, 700.0f, false, 200.0f, -1500.0f, 1200.0f};
    array[16] = {-1000.0f, -1300.0f, -1000.0f, 0.0f,   0.0f,   false, 200.0f, -1300.0f, 0.0f};
    array[17] = {-800.0f,  -1100.0f, -2000.0f, 500.0f, 700.0f, false, 200.0f, -2300.0f, 1200.0f};
    array[18] = {1000.0f,  700.0f,   1000.0f,  0.0f,   0.0f,   false, 200.0f, 0.0f,     700.0f};
    array[19] = {2200.0f,  1900.0f,  1000.0f,  500.0f, 700.0f, false, 200.0f, 0.0f,     1900.0f};
    array[20] = {700.0f,   400.0f,   -500.0f,  500.0f, 700.0f, false, 200.0f, -500.0f,  900.0f};
    array[21] = {0.0f,     -300.0f,  -200.0f,  0.0f,   0.0f,   true,  200.0f, -500.0f,  200.0f};
    array[22] = {0.0f,     -300.0f,  -1200.0f, 500.0f, 700.0f, true,  200.0f, -1700.0f, 1400.0f};
    array[23] = {-1000.0f, -1300.0f, -1200.0f, 0.0f,   0.0f,   true,  200.0f, -1500.0f, 200.0f};
    array[24] = {-800.0f,  -1100.0f, -2000.0f, 500.0f, 700.0f, true,  200.0f, -2500.0f, 1400.0f};
    array[25] = {1000.0f,  700.0f,   800.0f,   0.0f,   0.0f,   true,  200.0f, 0.0f,     700.0f};
    array[26] = {2200.0f,  1900.0f,  1000.0f,  400.0f, 600.0f, true,  200.0f, 0.0f,     1900.0f};
    array[27] = {700.0f,   400.0f,   -300.0f,  400.0f, 600.0f, true,  200.0f, -500.0f,  900.0f};
    array[28] = {0.0f,     300.0f,   0.0f,     0.0f,   0.0f,   false, 200.0f, 0.0f,     300.0f};  // Same tests with positive standalone modification
    array[29] = {0.0f,     300.0f,   -1200.0f, 500.0f, 700.0f, false, 200.0f, -1200.0f, 1500.0f}; 
    array[30] = {-1000.0f, -700.0f,  -1000.0f, 0.0f,   0.0f,   false, 200.0f, -700.0f,  0.0f};
    array[31] = {-800.0f,  -500.0f,  -2000.0f, 500.0f, 700.0f, false, 200.0f, -1700.0f, 1200.0f}; 
    array[32] = {1000.0f,  1300.0f,  1000.0f,  0.0f,   0.0f,   false, 200.0f, 0.0f,     1300.0f}; 
    array[33] = {2200.0f,  2500.0f,  1000.0f,  500.0f, 700.0f, false, 200.0f, 0.0f,     2500.0f}; 
    array[34] = {700.0f,   1000.0f,  -500.0f,  500.0f, 700.0f, false, 200.0f, -500.0f,  1500.0f}; 
    array[35] = {0.0f,     300.0f,   -200.0f,  0.0f,   0.0f,   true,  200.0f, -200.0f,  500.0f};  
    array[36] = {0.0f,     300.0f,   -1200.0f, 500.0f, 700.0f, true,  200.0f, -1400.0f, 1700.0f};  
    array[37] = {-1000.0f, -700.0f,  -1200.0f, 0.0f,   0.0f,   true,  200.0f, -900.0f,  200.0f};   
    array[38] = {-800.0f,  -500.0f,  -2000.0f, 500.0f, 700.0f, true,  200.0f, -1900.0f, 1400.0f};  
    array[39] = {1200.0f,  1500.0f,  1000.0f,  0.0f,   0.0f,   true,  200.0f, 0.0f,     1500.0f};   
    array[40] = {2400.0f,  2700.0f,  1200.0f,  400.0f, 600.0f, true,  200.0f, 0.0f,     2700.0f};  
    array[41] = {700.0f,   1000.0f,  -300.0f,  400.0f, 600.0f, true,  200.0f, -500.0f,  1500.0f};  

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.set_feature_kW_demand(array[i].feature_demand);
        asset_cmd.set_site_kW_demand(array[i].site_demand);
        asset_cmd.set_ess_kW_request(array[i].ess_request);
        asset_cmd.set_gen_kW_request(array[i].gen_request);
        asset_cmd.set_solar_kW_request(array[i].solar_request);
        asset_cmd.set_load_compensation_method(LOAD_MINIMUM);
        asset_cmd.set_additional_load_compensation(array[i].load_inclusion * array[i].additional_load);
        asset_cmd.calculate_site_kW_production_limits();
        errorLog << "calculate_site_kW_production_limits() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.get_site_kW_charge_production() != array[i].expected_charge_production
               || asset_cmd.get_site_kW_discharge_production() != array[i].expected_discharge_production;
        EXPECT_EQ(asset_cmd.get_site_kW_charge_production(), array[i].expected_charge_production);
        EXPECT_EQ(asset_cmd.get_site_kW_discharge_production(), array[i].expected_discharge_production);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Determine if ESS must discharge to handle load
TEST_F(site_manager_test, determine_ess_load_requirement)
{
    struct test
    {
        load_compensation load_method;
        int asset_priority;
        float load;
        float ess_request;
        float site_demand;
        float feature_demand;
        bool expected_result;
        float expected_demand;
        float expected_feature;
    };

    std::vector<test> tests = {
        {NO_COMPENSATION, 0, 0.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f},
        {LOAD_OFFSET, 0, 0.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f},
        {LOAD_MINIMUM, 0, 0.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f},
        {LOAD_MINIMUM, 0, 150.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f},
        {LOAD_MINIMUM, 0, 200.0f, -100.0f, 250.0f, 250.0f, false, 250.0f, 250.0f},
        {LOAD_MINIMUM, 0, 201.0f, -100.0f, -100.0f, 100.0f, true, 0.0f, 200.0f},
        {LOAD_MINIMUM, 2, 101.0f, -100.0f, -100.0f, 100.0f, true, 0.0f, 200.0f}
    };
    int test_id = 1;
    for (auto test : tests)
    {
        test_logger t_log("determine_ess_load_requirement", test_id++, tests.size());
        asset_cmd.set_load_compensation_method(test.load_method);
        asset_cmd.set_site_kW_load(test.load);
        asset_cmd.set_ess_kW_request(test.ess_request);
        asset_cmd.set_site_kW_demand(test.site_demand);
        asset_cmd.set_feature_kW_demand(test.feature_demand);
        asset_cmd.set_feeder_max_potential_kW(100.0f);
        asset_cmd.set_ess_max_potential_kW(100.0f);
        asset_cmd.set_gen_max_potential_kW(100.0f);
        asset_cmd.set_solar_max_potential_kW(100.0f);
        t_log.bool_results.push_back({test.expected_result, asset_cmd.determine_ess_load_requirement(test.asset_priority), "result"});
        t_log.float_results.push_back({test.expected_demand, asset_cmd.get_site_kW_demand(), "demand"});
        t_log.float_results.push_back({test.expected_feature, asset_cmd.get_feature_kW_demand(), "feature"});
        if (test.expected_result)
            t_log.float_results.push_back({0.0f, asset_cmd.get_ess_kW_request(), "request"});
        else
            t_log.float_results.push_back({test.ess_request, asset_cmd.get_ess_kW_request(), "request"});
        t_log.check_solution();
    }
}

//ess charge 
TEST_F(site_manager_test, dispatch_site_kW_charge_cmd)
{
    int const num_tests = 22;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float ess_min_potential_kW;
        float ess_kW_cmd;
        float charge_production;
        float discharge_production;
        int priority;
        bool feeder_source;
        bool gen_source;
        bool solar_source;
        float expected_ess_kW_cmd;
        float expected_feeder_kW_cmd;
        float expected_gen_kW_cmd;
        float expected_solar_kW_cmd;
        float expected_charge_production;    // How much charge was produced by the feature
        float expected_discharge_production; // Confirm that discharge production used to compensate is reduced
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case, source order based on priority, discharge production should reduce for all site production used for charge (not feed)
    array[0]  = {-700.0f,  0.0f,    0.0f,     0.0f,    0, true,  true,  true,  0.0f,     0.0f,    0.0f,    0.0f,    0.0f,     0.0f};    // no request
    array[1]  = {-700.0f,  0.0f,    500.0f,   600.0f,  0, true,  true,  true,  0.0f,     0.0f,    0.0f,    0.0f,    0.0f,     600.0f};  // positive request
    array[2]  = {-700.0f,  100.0f,  -500.0f,  600.0f,  0, true,  true,  true,  100.0f,   0.0f,    0.0f,    0.0f,    0.0f,     600.0f};  // blocked by discharge
    array[3]  = {-700.0f,  0.0f,    -500.0f,  600.0f,  0, false, false, false, 0.0f,     0.0f,    0.0f,    0.0f,    0.0f,     100.0f};  // no source, reduce discharge
    array[4]  = {-700.0f,  0.0f,    -800.0f,  800.0f,  0, true,  true,  true,  -700.0f,  0.0f,    0.0f,    700.0f,  -700.0f,  100.0f};  // exceeds potential, limited
    array[5]  = {-700.0f,  -500.0f, -500.0f,  600.0f,  0, true,  true,  true,  -700.0f,  0.0f,    0.0f,    200.0f,  -200.0f,  400.0f};  // exceeds potential - cmd, limited
    array[6]  = {-700.0f,  0.0f,    -500.0f,  600.0f,  0, false, false, true,  -500.0f,  0.0f,    0.0f,    500.0f,  -500.0f,  100.0f};  // 1st source
    array[7]  = {-700.0f,  0.0f,    -500.0f,  600.0f,  0, false, true,  false, -500.0f,  0.0f,    500.0f,  0.0f,    -500.0f,  100.0f};  // 1st disabled, 2nd source
    array[8]  = {-700.0f,  0.0f,    -500.0f,  600.0f,  0, true,  false, false, -500.0f,  500.0f,  0.0f,    0.0f,    -500.0f,  100.0f};  // first two disabled, 3rd source
    array[9]  = {-1500.0f, 0.0f,    -1500.0f, 1500.0f, 0, true,  true,  true,  -1500.0f, 0.0f,    500.0f,  1000.0f, -1500.0f, 0.0f};    // request requires 2 sources
    array[10] = {-3500.0f, 0.0f,    -3500.0f, 3500.0f, 0, true,  true,  true,  -3000.0f, 1000.0f, 1000.0f, 1000.0f, -3000.0f, 0.0f};    // request exceeds 3 sources, reduce discharge
    array[11] = {-700.0f,  0.0f,    0.0f,     0.0f,    2, true,  true,  true,  0.0f,     0.0f,    0.0f,    0.0f,    0.0f,     0.0f};    // Same tests different priority
    array[12] = {-700.0f,  0.0f,    500.0f,   600.0f,  2, true,  true,  true,  0.0f,     0.0f,    0.0f,    0.0f,    0.0f,     600.0f};
    array[13] = {-700.0f,  100.0f,  -500.0f,  600.0f,  2, true,  true,  true,  100.0f,   0.0f,    0.0f,    0.0f,    0.0f,     600.0f};
    array[14] = {-700.0f,  0.0f,    -500.0f,  600.0f,  2, false, false, false, 0.0f,     0.0f,    0.0f,    0.0f,    0.0f,     100.0f};
    array[15] = {-700.0f,  0.0f,    -800.0f,  800.0f,  2, true,  true,  true,  -700.0f,  0.0f,    700.0f,  0.0f,    -700.0f,  100.0f};
    array[16] = {-700.0f,  -500.0f, -500.0f,  600.0f,  2, true,  true,  true,  -700.0f,  0.0f,    200.0f,  0.0f,    -200.0f,  400.0f};
    array[17] = {-700.0f,  0.0f,    -500.0f,  600.0f,  2, false, true,  false, -500.0f,  0.0f,    500.0f,  0.0f,    -500.0f,  100.0f};
    array[18] = {-700.0f,  0.0f,    -500.0f,  600.0f,  2, false, false, true,  -500.0f,  0.0f,    0.0f,    500.0f,  -500.0f,  100.0f};
    array[19] = {-700.0f,  0.0f,    -500.0f,  600.0f,  2, true,  false, false, -500.0f,  500.0f,  0.0f,    0.0f,    -500.0f,  100.0f};
    array[20] = {-1500.0f, 0.0f,    -1500.0f, 1500.0f, 2, true,  true,  true,  -1500.0f, 0.0f,    1000.0f, 500.0f,  -1500.0f, 0.0f};
    array[21] = {-3500.0f, 0.0f,    -3500.0f, 3500.0f, 2, true,  true,  true,  -3000.0f, 1000.0f, 1000.0f, 1000.0f, -3000.0f, 0.0f};

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.set_ess_min_potential_kW(array[i].ess_min_potential_kW);
        asset_cmd.set_ess_kW_cmd(array[i].ess_kW_cmd);
        // Hard coded potentials/cmds (0) to simplify tests as there are already a significant number of cases
        asset_cmd.set_gen_max_potential_kW(1000.0f);
        asset_cmd.set_gen_kW_cmd(0.0f);
        asset_cmd.set_solar_max_potential_kW(1000.0f);
        asset_cmd.set_solar_kW_cmd(0.0f);
        asset_cmd.set_feeder_max_potential_kW(1000.0f);
        asset_cmd.set_feeder_kW_cmd(0.0f);
        asset_cmd.set_site_kW_charge_production(array[i].charge_production);
        asset_cmd.set_site_kW_discharge_production(array[i].discharge_production);
        float result = asset_cmd.dispatch_site_kW_charge_cmd(array[i].priority, array[i].solar_source, array[i].gen_source, array[i].feeder_source);
        errorLog <<"dispatch_site_kW_charge_cmd() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.get_ess_kW_cmd() != array[i].expected_ess_kW_cmd
               || asset_cmd.get_feeder_kW_cmd() != array[i].expected_feeder_kW_cmd
               || asset_cmd.get_gen_kW_cmd() != array[i].expected_gen_kW_cmd
               || asset_cmd.get_solar_kW_cmd() != array[i].expected_solar_kW_cmd
               || result != array[i].expected_charge_production
               || asset_cmd.get_site_kW_discharge_production() != array[i].expected_discharge_production;
        EXPECT_EQ(asset_cmd.get_ess_kW_cmd(), array[i].expected_ess_kW_cmd);
        EXPECT_EQ(asset_cmd.get_feeder_kW_cmd(), array[i].expected_feeder_kW_cmd);
        EXPECT_EQ(asset_cmd.get_gen_kW_cmd(), array[i].expected_gen_kW_cmd);
        EXPECT_EQ(asset_cmd.get_solar_kW_cmd(), array[i].expected_solar_kW_cmd);
        EXPECT_EQ(result, array[i].expected_charge_production);
        EXPECT_EQ(asset_cmd.get_site_kW_discharge_production(), array[i].expected_discharge_production);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Verify discharge dispatch used for load and additional site discharge
TEST_F(site_manager_test, dispatch_site_kW_discharge_cmd)
{
    int const num_tests = 20;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float discharge_production;
        float cmd;
        int priority;
        float ess_kW_request;
        float gen_kW_request;
        float solar_kW_request;
        float expected_dispatch;
        float expected_ess_kW_cmd;
        float expected_feeder_kW_cmd;
        float expected_gen_kW_cmd;
        float expected_solar_kW_cmd;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] =  {1000.0f, -500.0f, 0, 0.0f,   0.0f,   0.0f,   0.0f,    0.0f,   0.0f,   0.0f,   0.0f};   // negative cmd
    array[1] =  {0.0f,    1000.0f, 0, 0.0f,   0.0f,   0.0f,   0.0f,    0.0f,   0.0f,   0.0f,   0.0f};   // 0 dispatch available
    array[2] =  {800.0f,  800.0f,  0, 0.0f,   0.0f,   0.0f,   800.0f,  0.0f,   0.0f,   0.0f,   800.0f}; // fully met by solar (start priority 0)
    array[3] =  {1300.0f, 1300.0f, 0, 0.0f,   0.0f,   0.0f,   1300.0f, 0.0f,   0.0f,   500.0f, 800.0f}; // met by solar + gen
    array[4] =  {2100.0f, 2100.0f, 0, 0.0f,   0.0f,   0.0f,   2100.0f, 600.0f, 0.0f,   700.0f, 800.0f}; // met by solar + gen + ess
    array[5] =  {3000.0f, 3000.0f, 0, 0.0f,   0.0f,   0.0f,   3000.0f, 600.0f, 900.0f, 700.0f, 800.0f}; // met by solar + gen + ess + feed
    array[6] =  {3100.0f, 3100.0f, 0, 0.0f,   0.0f,   0.0f,   3000.0f, 600.0f, 900.0f, 700.0f, 800.0f}; // partially met (exceeds unused)
    array[7] =  {3100.0f, 1600.0f, 0, 0.0f,   0.0f,   0.0f,   1600.0f, 100.0f, 0.0f,   700.0f, 800.0f}; // additional production is not met, only input cmd
    array[8] =  {700.0f,  700.0f,  2, 0.0f,   0.0f,   0.0f,   700.0f,  0.0f,   0.0f,   700.0f, 0.0f};   // fully met by gen (start priority 2)
    array[9] =  {1300.0f, 1300.0f, 2, 0.0f,   0.0f,   0.0f,   1300.0f, 600.0f, 0.0f,   700.0f, 0.0f};   // met by gen + ess
    array[10] = {2100.0f, 2100.0f, 2, 0.0f,   0.0f,   0.0f,   2100.0f, 600.0f, 0.0f,   700.0f, 800.0f}; // met by gen + ess + solar
    array[11] = {3000.0f, 3000.0f, 2, 0.0f,   0.0f,   0.0f,   3000.0f, 600.0f, 900.0f, 700.0f, 800.0f}; // met by gen + ess + solar + feed
    array[12] = {3100.0f, 3000.0f, 2, 0.0f,   0.0f,   0.0f,   3000.0f, 600.0f, 900.0f, 700.0f, 800.0f}; // partially met (exceeds unused)
    array[13] = {3000.0f, 1600.0f, 2, 0.0f,   0.0f,   0.0f,   1600.0f, 600.0f, 0.0f,   700.0f, 300.0f}; // additional production is not met, only cmd
    array[14] = {600.0f,  600.0f,  4, 0.0f,   0.0f,   0.0f,   600.0f,  600.0f, 0.0f,   0.0f,   0.0f};   // fully met by ess (new ess first priority)
    array[15] = {1400.0f, 1400.0f, 4, 0.0f,   0.0f,   0.0f,   1400.0f, 600.0f, 0.0f,   0.0f,   800.0f}; // met by ess + solar
    array[16] = {2100.0f, 2100.0f, 4, 0.0f,   0.0f,   0.0f,   2100.0f, 600.0f, 0.0f,   700.0f, 800.0f}; // met by ess + solar + gen
    array[17] = {3000.0f, 3000.0f, 4, 0.0f,   0.0f,   0.0f,   3000.0f, 600.0f, 900.0f, 700.0f, 800.0f}; // met by ess + solar + gen + feed
    array[18] = {3100.0f, 3000.0f, 4, 0.0f,   0.0f,   0.0f,   3000.0f, 600.0f, 900.0f, 700.0f, 800.0f}; // partially met (exceeds unused)
    array[19] = {3000.0f, 1600.0f, 4, 0.0f,   0.0f,   0.0f,   1600.0f, 600.0f, 0.0f,   200.0f, 800.0f}; // additional production is not met, only cmd

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.set_ess_max_potential_kW(600.0f);
        asset_cmd.set_ess_kW_request(array[i].ess_kW_request);
        asset_cmd.set_ess_kW_cmd(0.0f);
        asset_cmd.set_gen_max_potential_kW(700.0f);
        asset_cmd.set_gen_kW_request(array[i].gen_kW_request);
        asset_cmd.set_gen_kW_cmd(0.0f);
        asset_cmd.set_solar_max_potential_kW(800.0f);
        asset_cmd.set_solar_kW_request(array[i].solar_kW_request);
        asset_cmd.set_solar_kW_cmd(0.0f);
        asset_cmd.set_feeder_max_potential_kW(900.0f);
        asset_cmd.set_feeder_kW_cmd(0.0f);
        asset_cmd.set_site_kW_discharge_production(array[i].discharge_production);
        float actual_dispatch = asset_cmd.dispatch_site_kW_discharge_cmd(array[i].priority, array[i].cmd, DEMAND);
        errorLog << "dispatch_site_kW_discharge_cmd() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = !near(actual_dispatch, array[i].expected_dispatch, 0.001)
               || !near(asset_cmd.get_ess_kW_cmd(), array[i].expected_ess_kW_cmd, 0.001)
               || !near(asset_cmd.get_gen_kW_cmd(), array[i].expected_gen_kW_cmd, 0.001)
               || !near(asset_cmd.get_solar_kW_cmd(), array[i].expected_solar_kW_cmd, 0.001)
               || !near(asset_cmd.get_feeder_kW_cmd(), array[i].expected_feeder_kW_cmd, 0.001);
        EXPECT_NEAR(actual_dispatch, array[i].expected_dispatch, 0.001);
        EXPECT_NEAR(asset_cmd.get_ess_kW_cmd(), array[i].expected_ess_kW_cmd, 0.001);
        EXPECT_NEAR(asset_cmd.get_gen_kW_cmd(), array[i].expected_gen_kW_cmd, 0.001);
        EXPECT_NEAR(asset_cmd.get_solar_kW_cmd(), array[i].expected_solar_kW_cmd, 0.001);
        EXPECT_NEAR(asset_cmd.get_feeder_kW_cmd(), array[i].expected_feeder_kW_cmd, 0.001);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Target SoC Mode
// Simply ensure that the inputs are unmodified and solar request is set appropriately
TEST_F(site_manager_test, target_soc)
{
    int const num_tests = 5;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        bool load_enable;
        float site_kW_load;
        float ess_kW_request;
        float solar_max_potential;
        bool expected_load_inclusion;
        float expected_ess_request;
        float expected_solar_request;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {true, 0.0f, 2500.0f, 0.0f, true, 2500.0f, 0.0f};            // Ess discharge only
    array[1] = {false, 0.0f, -2500.0f, 0.0f, false, -2500.0f, 0.0f};        // Ess charge only
    array[2] = {true, 0.0f, -2500.0f, 2500.0f, true, -2500.0f, 2500.0f};    // Ess charge from solar potential, solar = ess
    array[3] = {false, 0.0f, -2500.0f, 1500.0f, false, -2500.0f, 1500.0f};  // Ess charge from solar potential, solar < ess
    array[4] = {false, 0.0f, -2500.0f, 5000.0f, false, -2500.0f, 5000.0f};  // Ess charge from solar potential, solar > ess

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.set_ess_kW_request(array[i].ess_kW_request);
        asset_cmd.set_solar_max_potential_kW(array[i].solar_max_potential);
        asset_cmd.target_soc_mode(array[i].load_enable);
        errorLog << "target_soc() test " << i+1 << " of " << num_tests << std::endl;
        bool failure = asset_cmd.get_ess_kW_request() != array[i].expected_ess_request
                    || asset_cmd.get_solar_kW_request() != array[i].expected_solar_request
                    || asset_cmd.get_site_kW_load_inclusion() != array[i].expected_load_inclusion;
        EXPECT_EQ(asset_cmd.get_ess_kW_request(), array[i].expected_ess_request);
        EXPECT_EQ(asset_cmd.get_solar_kW_request(), array[i].expected_solar_request);
        EXPECT_EQ(asset_cmd.get_site_kW_load_inclusion(), array[i].expected_load_inclusion);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// site export target mode
// Ensure that demand and load requirement are passed through as expected
TEST_F(site_manager_test, site_export_target_mode)
{
    int const num_tests = 4;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        bool load_enable_flag;
        float export_target_cmd;
        bool expected_load_enable;
        float expected_site_demand;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {false, 2000.0f,  false, 2000.0f};
    array[1] = {false, -2000.0f, false, -2000.0f};
    array[2] = {true,  -2000.0f, true,  -2000.0f};
    array[3] = {true,  2000.0f,  true,  2000.0f};

    //increment export target cmd slew prior to test 
    export_target_kW_cmd_slew.set_slew_rate(1000000);  //1000MW/s
    export_target_kW_cmd_slew.update_slew_target(0); //call once to set time vars
    usleep(10000); //wait 10ms (total range +/-10MW)
    export_target_kW_cmd_slew.update_slew_target(0); //call again to get delta time for non-zero slew range

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.site_export_target_mode(array[i].load_enable_flag, &export_target_kW_cmd_slew, array[i].export_target_cmd);
        errorLog << "site_export_target_mode() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.get_site_kW_demand() != array[i].expected_site_demand
               || asset_cmd.get_site_kW_load_inclusion() != array[i].expected_load_enable;
        EXPECT_EQ(asset_cmd.get_site_kW_demand(), array[i].expected_site_demand);
        EXPECT_EQ(asset_cmd.get_site_kW_load_inclusion(), array[i].expected_load_enable);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// manual mode
// Ensure that asset requests are produced for each asset cmd and that no excess potential exists that could be used by another asset/load/feature
TEST_F(site_manager_test, manual_mode)
{
    int const num_tests = 5;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float ess_max_potential;
        float solar_max_potential;
        float ess_cmd;
        float solar_cmd;
        float expected_ess_charge_kW_request;
        float expected_solar_kW_request;
        float expected_ess_max_potential;
        float expected_solar_max_potential;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {1000.0f, 2000.0f, 0.0f,    0.0f,   0.0f,    0.0f,   0.0f,   0.0f};
    array[1] = {1000.0f, 2000.0f, 500.0f,  0.0f,   500.0f,  0.0f,   500.0f, 0.0f};
    array[2] = {1000.0f, 2000.0f, 500.0f,  750.0f, 500.0f,  750.0f, 500.0f, 750.0f};
    array[3] = {1000.0f, 2000.0f, 500.0f,  750.0f, 500.0f,  750.0f, 500.0f, 750.0f};
    array[4] = {1000.0f, 2000.0f, -500.0f, 750.0f, -500.0f, 750.0f, 0.0f,   750.0f};

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        // Set potentials to some abritrary nonzero value to ensure they are modified appropriately
        asset_cmd.set_solar_max_potential_kW(5000);
        asset_cmd.set_ess_max_potential_kW(2500);
        asset_cmd.manual_mode(array[i].ess_cmd, array[i].solar_cmd);
        errorLog << "manual_mode() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.get_solar_kW_request() != array[i].expected_solar_kW_request
               || asset_cmd.get_ess_kW_request() != array[i].expected_ess_charge_kW_request
               || asset_cmd.get_ess_max_potential_kW() != array[i].expected_ess_max_potential
               || asset_cmd.get_solar_max_potential_kW() != array[i].expected_solar_max_potential
               || asset_cmd.get_site_kW_load_inclusion() != false; // Ensure this feature does not track load
        EXPECT_EQ(asset_cmd.get_solar_kW_request(), array[i].expected_solar_kW_request);
        EXPECT_EQ(asset_cmd.get_ess_kW_request(), array[i].expected_ess_charge_kW_request);
        EXPECT_EQ(asset_cmd.get_ess_max_potential_kW(),array[i].expected_ess_max_potential);
        EXPECT_EQ(asset_cmd.get_solar_max_potential_kW(),array[i].expected_solar_max_potential);
        EXPECT_FALSE(asset_cmd.get_site_kW_load_inclusion());

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Grid Target mode
// Should set demand to the grid target + load
TEST_F(site_manager_test, grid_target_mode)
{
    int const num_tests = 4;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float site_load;
        float grid_target;
        float expected_site_demand;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {0.0f,   0.0f,     0.0f};
    array[1] = {500.0f, 0.0f,     500.0f};
    array[2] = {500.0f, 1000.0f,  500.0f}; // Signs represent the POI in this feature e.g. positve -> import, negative -> export
    array[3] = {500.0f, -1000.0f, 1500.0f};

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "grid_target_mode() test " << i+1 << " of " << num_tests << std::endl;
        asset_cmd.set_site_kW_load(array[i].site_load);
        asset_cmd.grid_target_mode(array[i].grid_target);
        bool failure = asset_cmd.get_site_kW_demand() != array[i].expected_site_demand
                    || asset_cmd.get_site_kW_load_inclusion() != true; // always tracks load
        EXPECT_EQ(asset_cmd.get_site_kW_demand(), array[i].expected_site_demand);
        EXPECT_TRUE(asset_cmd.get_site_kW_load_inclusion());

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Absolute ESS mode
TEST_F(site_manager_test, absolute_ess)
{
    int const num_tests = 8;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float ess_min_potential_kW;
        float ess_max_potential_kW;
        float solar_max_potential_kW;
        bool chg_dischg_flag;
        float ess_cmd;
        float expected_ess_request;
        float expected_solar_request;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {-500.0f, 500.0f, 0.0f,   false, 0.0f,    0.0f,    0.0f}; // zero test
    array[1] = {-500.0f, 500.0f, 0.0f,   false, 1000.0f, 1000.0f, 0.0f}; // not enough potential, but not limited by absolute power
    array[2] = {-500.0f, 500.0f, 0.0f,   false, 500.0f,  500.0f,  0.0f}; // discharge test, valid case
    array[3] = {-500.0f, 500.0f, 0.0f,   true,  400.0f,  -400.0f, 0.0f}; // charge test, valid case
    array[4] = {-500.0f, 500.0f, 0.0f,   false, -400.0f, 400.0f,  0.0f}; // discharge test, negative cmd but positive flag (false) produces positive request
    array[5] = {-500.0f, 500.0f, 0.0f,   true,  -400.0f, -400.0f, 0.0f}; // charge test, negative cmd and negative flag (true) produces negative request
    array[6] = {-500.0f, 500.0f, 250.0f, false, 500.0f,  500.0f,  250.0f}; // solar potential allows for solar request
    array[7] = {-500.0f, 500.0f, 750.0f, true,  400.0f,  -400.0f, 750.0f}; // solar potential allows for solar request

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "absolute_ess() test " << i+1 << " of " << num_tests << std::endl;
        asset_cmd.set_ess_min_potential_kW(array[i].ess_min_potential_kW);
        asset_cmd.set_ess_max_potential_kW(array[i].ess_max_potential_kW);
        asset_cmd.set_solar_max_potential_kW(array[i].solar_max_potential_kW);
        asset_cmd.absolute_ess(array[i].chg_dischg_flag, array[i].ess_cmd);
        bool failure = asset_cmd.get_ess_kW_request() != array[i].expected_ess_request
                    || asset_cmd.get_solar_kW_request() != array[i].expected_solar_request
                    || asset_cmd.get_site_kW_load_inclusion() != false;
        EXPECT_EQ(asset_cmd.get_ess_kW_request(), array[i].expected_ess_request);
        EXPECT_EQ(asset_cmd.get_solar_kW_request(), array[i].expected_solar_request);
        EXPECT_FALSE(asset_cmd.get_site_kW_load_inclusion());

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//dispatch_reactive_power test
TEST_F(site_manager_test, dispatch_reactive_power)
{
    int const num_tests = 6;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float solar_potential_kVAR;
        float ess_potential_kVAR;
        float gen_potential_kVAR;
        float site_kVAR_demand;
        float result_solar_kVAR_cmd;
        float result_ess_kVAR_cmd;
        float result_gen_kVAR_cmd;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {800, 400, 200, 1500, 800, 400, 200}; //positive, not enough potential
    array[1] = {800, 400, 200, 700, 400, 200, 100}; //positive, enough potential
    array[2] = {800, 400, 200, -1500, -800, -400, -200}; //negative, not enough potential
    array[3] = {800, 400, 200, -700, -400, -200, -100}; //negative, enough potential
    array[4] = {0, 0, 0, -700, 0, 0, 0}; //zero, not enough potential
    array[5] = {800, 400, 200, 0, 0, 0, 0}; //zero, enough potential

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.set_solar_potential_kVAR(array[i].solar_potential_kVAR);
        asset_cmd.set_ess_potential_kVAR(array[i].ess_potential_kVAR);
        asset_cmd.set_gen_potential_kVAR(array[i].gen_potential_kVAR);
        asset_cmd.set_site_kVAR_demand(array[i].site_kVAR_demand);

        //reset cmds each iteration (will exit on 0 demand or potential)
        asset_cmd.set_solar_kVAR_cmd(0);
        asset_cmd.set_ess_kVAR_cmd(0);
        asset_cmd.set_gen_kVAR_cmd(0);

        asset_cmd.calculate_total_potential_kVAR();
        asset_cmd.dispatch_reactive_power();
        errorLog << "dispatch_reactive_power() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.get_solar_kVAR_cmd() != array[i].result_solar_kVAR_cmd
                || asset_cmd.get_ess_kVAR_cmd() != array[i].result_ess_kVAR_cmd
                || asset_cmd.get_gen_kVAR_cmd() != array[i].result_gen_kVAR_cmd;
        EXPECT_EQ(asset_cmd.get_solar_kVAR_cmd(), array[i].result_solar_kVAR_cmd);
        EXPECT_EQ(asset_cmd.get_ess_kVAR_cmd(), array[i].result_ess_kVAR_cmd);
        EXPECT_EQ(asset_cmd.get_gen_kVAR_cmd(), array[i].result_gen_kVAR_cmd);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//active_voltage_mode test
TEST_F(site_manager_test, active_voltage_mode)
{
    int const num_tests = 5;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float solar_potential_kVAR;
        float ess_potential_kVAR;
        float gen_potential_kVAR;
        float actual_voltage;
        float result_site_kVAR_demand;
        float result_solar_kVAR_cmd;
        float result_ess_kVAR_cmd;
        float result_gen_kVAR_cmd;
    };

    tests array[num_tests];  //an array with an element for each test case

    //constant variables for every test
    float deadband = 50;
    float cmd = 400;
    float droop_percent = 5;
    float rated_kVAR = 1000;

    //configure variables each test case
    array[0] = {800, 400, 200, 400, 0, 0, 0, 0}; //zero
    array[1] = {800, 400, 200, 500, -1400, -800, -400, -200}; //overvoltage limited
    array[2] = {500, 400, 100, 460, -500, -250, -200, -50};//overvoltage less potential
    array[3] = {800, 400, 200, 300, 1400, 800, 400, 200}; //undervoltage limited
    array[4] = {500, 400, 100, 340, 500, 250, 200, 50}; //undervoltage less potential

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        asset_cmd.set_solar_potential_kVAR(array[i].solar_potential_kVAR);
        asset_cmd.set_ess_potential_kVAR(array[i].ess_potential_kVAR);
        asset_cmd.set_gen_potential_kVAR(array[i].gen_potential_kVAR);

        //reset cmds each iteration (will exit on 0 demand or potential)
        asset_cmd.set_solar_kVAR_cmd(0);
        asset_cmd.set_ess_kVAR_cmd(0);
        asset_cmd.set_gen_kVAR_cmd(0);

        asset_cmd.calculate_total_potential_kVAR();
        asset_cmd.active_voltage_mode(deadband, cmd, array[i].actual_voltage, droop_percent, rated_kVAR);
        asset_cmd.dispatch_reactive_power();

        errorLog << "active_voltage_mode() test " << i+1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.get_site_kVAR_demand() != array[i].result_site_kVAR_demand
                || asset_cmd.get_solar_kVAR_cmd() != array[i].result_solar_kVAR_cmd
                || asset_cmd.get_ess_kVAR_cmd() != array[i].result_ess_kVAR_cmd
                || asset_cmd.get_gen_kVAR_cmd() != array[i].result_gen_kVAR_cmd;
        EXPECT_EQ(asset_cmd.get_site_kVAR_demand(), array[i].result_site_kVAR_demand);
        EXPECT_EQ(asset_cmd.get_solar_kVAR_cmd(), array[i].result_solar_kVAR_cmd);
        EXPECT_EQ(asset_cmd.get_ess_kVAR_cmd(), array[i].result_ess_kVAR_cmd);
        EXPECT_EQ(asset_cmd.get_gen_kVAR_cmd(), array[i].result_gen_kVAR_cmd);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//watt-var test - curve 1 (8 points)
TEST_F(site_manager_test, watt_var_mode_curve_1)
{
    int const num_tests = 3;  //total number of test cases
    float points_array[] = {-500, 200, 0, 200, 1000, -500, 1500, 500};
    //struct that has variables to configure for each test case
    struct tests 
    {
        float actual_kW;
        float result_site_kVAR_demand;
    };

    tests array[num_tests];  //an array with an element for each test case
    //configure variables each test case
    array[0] = {-800, 200}; //low power
    array[1] = {800, -360}; //mid power
    array[2] = {1600, 700};//high power

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        int array_size = sizeof(points_array)/4;
        std::vector<Value_Object> options_value(array_size);

        for (int j = 0; j < array_size; j++)
        {
            options_value[j].set(points_array[j]);
        }

        Fims_Object fims_object;
        fims_object.options_value = options_value;
        fims_object.num_options = array_size;
        set_curve_points(&fims_object, watt_var_curve);
        errorLog << "watt_var_mode() curve 1 test " << i+1 << " of " << num_tests << std::endl;
        for (int j = 0; j < array_size; j+=2)
        {
            // failure conditions
            failure = watt_var_curve[j/2].first != options_value[j].value_float
                    || watt_var_curve[j/2].second != options_value[j+1].value_float;
            //test set_curve_points plots correctly
            EXPECT_EQ(watt_var_curve[j/2].first,  options_value[j].value_float);
            EXPECT_EQ(watt_var_curve[j/2].second, options_value[j+1].value_float);
        }

        //test watt_var_mode provides correct site kVAR demand output
        watt_var_mode(array[i].actual_kW, watt_var_curve);
        failure = failure || asset_cmd.get_site_kVAR_demand() != array[i].result_site_kVAR_demand;
        EXPECT_EQ(asset_cmd.get_site_kVAR_demand(), array[i].result_site_kVAR_demand);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//watt-var test - curve 2 (6 points)
TEST_F(site_manager_test, watt_var_mode_curve_2)
{

    int const num_tests = 3;  //total number of test cases
    float points_array[] = {-500, 500, 0, 200, 1000, 500};
    //struct that has variables to configure for each test case
    struct tests 
    {
        float actual_kW;
        float result_site_kVAR_demand;
    };

    tests array[num_tests];  //an array with an element for each test case
    //configure variables each test case
    array[0] = {-800, 680}; //low power
    array[1] = {800, 440}; //mid power
    array[2] = {1500, 650};//high power

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        int array_size = sizeof(points_array)/4;
        std::vector<Value_Object> options_value(array_size);

        for (int j = 0; j < array_size; j++)
        {
            options_value[j].set(points_array[j]);
        }

        Fims_Object fims_object;
        fims_object.options_value = options_value;
        fims_object.num_options = array_size;
        set_curve_points(&fims_object, watt_var_curve);
        errorLog << "watt_var_mode() curve 1 test " << i+1 << " of " << num_tests << std::endl;
        for (int j = 0; j < array_size; j+=2)
        {
            // failure conditions
            failure = watt_var_curve[j/2].first != options_value[j].value_float
                    || watt_var_curve[j/2].second != options_value[j+1].value_float;
            //test set_curve_points plots correctly
            EXPECT_EQ(watt_var_curve[j/2].first,  options_value[j].value_float);
            EXPECT_EQ(watt_var_curve[j/2].second, options_value[j+1].value_float);
        }
        
        //test watt_var_mode provides correct site kVAR demand output
        watt_var_mode(array[i].actual_kW, watt_var_curve);
        failure = failure || asset_cmd.get_site_kVAR_demand() != array[i].result_site_kVAR_demand;
        EXPECT_EQ(asset_cmd.get_site_kVAR_demand(), array[i].result_site_kVAR_demand);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//set curve points
TEST_F(site_manager_test, set_curve_points)
{
    int const num_tests = 3;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        std::vector<std::pair<float,float>> points;
    };

    // Make curves to check output of set_curve_points
    std::vector<std::pair<float,float>> curve0;
    curve0.push_back(std::make_pair(59,0)); curve0.push_back(std::make_pair(60,0)); curve0.push_back(std::make_pair(61,0));
    std::vector<std::pair<float,float>> curve1;
    curve1.push_back(std::make_pair(-1,-1.1)); curve1.push_back(std::make_pair(0,0)); curve1.push_back(std::make_pair(1,1.1));
    std::vector<std::pair<float,float>> curve2;
    curve2.push_back(std::make_pair(-1,1.1)); curve2.push_back(std::make_pair(0,0)); curve2.push_back(std::make_pair(1,-1.1));

    tests array[num_tests];  //an array with an element for each test case
    //configure variables each test case
    array[0] = {curve0};
    array[1] = {curve1};
    array[2] = {curve2};

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        int array_size = 2*array[i].points.size();
        std::vector<Value_Object> options_value(array_size);
        Fims_Object fims_object;
        std::vector<std::pair<float,float>> output_curve;

        for (int j = 0; j < array_size; j+=2)
        {
            options_value[j]  .set(array[i].points[j/2].first);
            options_value[j+1].set(array[i].points[j/2].second);
        }
        fims_object.options_value = options_value;
        fims_object.num_options = array_size;

        errorLog << "set_curve_points() test " << i+1 << " of " << num_tests << std::endl;
        set_curve_points(&fims_object, output_curve);
        // Release stdout so we can write again
        release_stdout(failure);
        for (int j = 0; j < (int)array[i].points.size(); j++)
        {
            // failure conditions
            failure = array[i].points[j].first != output_curve[j].first || array[i].points[j].second != output_curve[j].second;
            //test set_curve_points plots correctly
            EXPECT_EQ(array[i].points[j].first,  output_curve[j].first);
            EXPECT_EQ(array[i].points[j].second, output_curve[j].second);
            // Print the test id if failure
            if (failure)
                std::cout << errorLog.str() << std::endl;
        }
    }
}

//get curve cmd
TEST_F(site_manager_test, get_curve_cmd)
{
    int const num_tests = 6;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        std::vector<std::pair<float,float>> points;
        float curve_input;
        float curve_output;
    };

    // Make curves to check output of set_curve_points
    std::vector<std::pair<float,float>> curve0;
    curve0.push_back(std::make_pair(59,0)); curve0.push_back(std::make_pair(60,0)); curve0.push_back(std::make_pair(61,0));
    std::vector<std::pair<float,float>> curve1;
    curve1.push_back(std::make_pair(-1,-1.1)); curve1.push_back(std::make_pair(0,0)); curve1.push_back(std::make_pair(1,1.1));
    std::vector<std::pair<float,float>> curve2;
    curve2.push_back(std::make_pair(-1,1.1)); curve2.push_back(std::make_pair(0,0)); curve2.push_back(std::make_pair(1,-1.1));

    tests array[num_tests];  //an array with an element for each test case
    //configure variables each test case
    array[0] = {curve0,  100,    0};
    array[1] = {curve0, -100,    0};
    array[2] = {curve1, -200, -220};
    array[3] = {curve1,  100,  110};
    array[4] = {curve2, -100, 110};
    array[5] = {curve2,  200,  -220};

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "get_curve_cmd() test " << i+1 << " of " << num_tests << std::endl;
        float result = get_curve_cmd(array[i].curve_input, array[i].points);

        // failure conditions
        failure = !near(result, array[i].curve_output, 0.001);
        //test get_curve_cmd calculates correctly
        EXPECT_NEAR(result, array[i].curve_output, 0.001);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Aggregated Asset Limit standalone power feature
TEST_F(site_manager_test, apply_aggregated_asset_limit)
{
    int const num_tests = 6;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float uncontrolled_solar_kw;
        float controlled_solar_kw;
        float uncontrolled_ess_kW;
        float current_controlled_ess_max;
        float agg_asset_limit_kw;
        float result_ess_max_potential_kw;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    //          unctrl_solar    ctrl_solar  unctrl_ess  current_ctrl_ess_max    agg_asset_limit_kw  result_ess_max_potential_kw
    array[0] = {500.0f,         0.0f,       0.0f,       500.0f,                 750.0f,             250}; // discharging is beyond limit, solar alone is not
    array[1] = {600.0f,         0.0f,       0.0f,       300.0f,                 1000.0f,            300}; // discharging is not beyond limit
    array[2] = {1000.0f,        0.0f,       0.0f,       100.0f,                 1100.0f,            100}; // discharging is exactly at limit
    array[3] = {500.0f,         0.0f,       0.0f,       -100.0f,                250.0f,             -100};// discharging is beyond limit, solar alone is beyond limit
    array[4] = {1000.0f,        0.0f,       0.0f,       -300.0f,                700.0f,             -300};// ess is charging, solar alone is beyond limit
    array[5] = {750.0f,         0.0f,       0.0f,       -250.0f,                1000.0f,            -250};// ess is charging, solar alone is not beyond limit

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "apply_aggregated_asset_limit() test " << i+1 << " of " << num_tests << std::endl;
        asset_cmd.set_solar_actual_kW(array[i].controlled_solar_kw);
        asset_cmd.set_ess_max_potential_kW(array[i].current_controlled_ess_max);
        agg_asset_limit_kw.value.set(array[i].agg_asset_limit_kw);
        apply_aggregated_asset_limit(array[i].uncontrolled_ess_kW, array[i].uncontrolled_solar_kw);
        bool failure = asset_cmd.get_ess_max_potential_kW() != array[i].result_ess_max_potential_kw;
        EXPECT_EQ(asset_cmd.get_ess_max_potential_kW(), array[i].result_ess_max_potential_kw);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// POI limits
TEST_F(site_manager_test, apply_active_power_poi_limits)
{
    //struct that has variables to configure for each test case
    struct tests
    {
        bool load_enable;
        load_compensation load_inclusion;
        float feed_max;
        float asset_maxes;
        bool feed_only;
        float site_load;
        float site_demand;
        float min_poi_limit_kW;
        float max_poi_limit_kW;
        int priority; // Asset priority. Other assets can cover import removing the responsibility from the POI
        float expected_feed_max;
        float expected_site_demand;
    };

    std::vector<tests> tests = {
    // Ensure that load and uncontrollable properly limit the site demand based on import/export, but do not influence the feed max
        {false, NO_COMPENSATION, 600.0f, 200.0f, true,   0.0f,    390.0f, -485,  485, 0, 485,  390},  // discharging, no limit
        {false, NO_COMPENSATION, 600.0f, 200.0f, true,   0.0f,   -390.0f, -485,  485, 0, 485, -390},  // charging, no limit
        {false, NO_COMPENSATION, 600.0f, 200.0f, true,   0.0f,    490.0f, -485,  485, 0, 485,  485},  // discharging, limited
        {false, NO_COMPENSATION, 600.0f, 200.0f, true,   0.0f,   -490.0f, -485,  485, 0, 485, -485},  // charging, limited
        {false, NO_COMPENSATION, 600.0f, 200.0f, false,  200.0f,  490.0f, -485,  485, 0, 485,  485},  // Load present but disabled, no limit modification
        {false, LOAD_OFFSET,     600.0f, 200.0f, false,  0.0f,    490.0f, -485,  485, 0, 485,  485},  // Feature load enabled, but no load
        {true,  LOAD_OFFSET,     600.0f, 200.0f, false,  200.0f,  490.0f, -485,  485, 0, 485,  490},  // Load importing, demand exporting, gives extra headroom
        {true,  LOAD_OFFSET,     600.0f, 100.0f, false,  200.0f, -190.0f, -485,  485, 0, 485, -190},  // Load importing, demand importing, other assets satisfy load, no limit
        {true,  LOAD_OFFSET,     600.0f, 200.0f, false, -200.0f,  390.0f, -485,  485, 0, 485,  285},  // Load exporting, demand exporting, limited
        {true,  LOAD_OFFSET,     600.0f, 200.0f, false, -200.0f, -490.0f, -485,  485, 0, 485, -490},  // Load exporting, demand importing, gives extra headroom
        {true,  LOAD_OFFSET,     600.0f, 200.0f, false,  200.0f,  490.0f, -485,  485, 1, 485,  490},  // Load importing, demand exporting with different asset priority
        // {true,  LOAD_OFFSET,     600.0f, 200.0f, false,  300.0f, -490.0f, -485,  485, 1, 485, -475},  // Load importing, demand importing, some load on feeder -> limited
        {true,  LOAD_MINIMUM,     200.0f, 200.0f, false,  500.0f,  0.0f,   -200,  200, 1, 200,  500},  // Load importing, demand wants to import but ESS must help with load
    };
    
    int test_id = 1;
    for (auto test : tests)
    {
        test_logger t_log("apply_active_power_poi_limits", test_id++, tests.size());
        // Set independent variables
        asset_cmd.set_load_compensation_method(test.load_inclusion);
        asset_cmd.set_site_kW_load(test.site_load);
        asset_cmd.set_additional_load_compensation(asset_cmd.get_site_kW_load_inclusion() * test.site_load);
        asset_cmd.set_feeder_max_potential_kW(test.feed_max);
        asset_cmd.set_ess_max_potential_kW(test.asset_maxes);    // Other asset potentials used to determine how much of the load will
        asset_cmd.set_gen_max_potential_kW(test.asset_maxes);    // be satisfied by other assets, therefore not falling on the POI
        asset_cmd.set_ess_min_potential_kW(-1.0f * test.feed_max);
        asset_cmd.set_solar_max_potential_kW(test.asset_maxes);
        charge_dispatch_feeder_enable_flag.value.set(true);
        charge_dispatch_gen_enable_flag.value.set(!test.feed_only);
        charge_dispatch_solar_enable_flag.value.set(!test.feed_only);
        asset_cmd.set_site_kW_demand(test.site_demand);
        asset_cmd.set_feature_kW_demand(test.site_demand);

        // Apply limits
        asset_priority_runmode1.value.set(test.priority);
        active_power_poi_limits_min_kW.value.set(test.min_poi_limit_kW);
        active_power_poi_limits_max_kW.value.set(test.max_poi_limit_kW);
        apply_active_power_poi_limits();

        // Check results
        t_log.float_results.push_back({asset_cmd.get_site_kW_demand(), test.expected_site_demand, "demand"});
        t_log.float_results.push_back({asset_cmd.get_feeder_max_potential_kW(), test.expected_feed_max, "feed_max"});
        t_log.check_solution();
    }
}

// soc-based POI limits
TEST_F(site_manager_test, apply_poi_limits_soc)
{
    int const num_tests = 8;    //total number of test cases

    //struct that has variables to configure for each test case
    struct tests
    {
        bool soc_limits_enabled;
        float avg_soc;
        float soc_target;
        float under_min;
        float under_max;
        float over_min;
        float over_max;
        float site_demand;
        float feed_potential;
        float expected_site_demand;
        float expected_feed_potential;
    };

    tests array[num_tests];  //an array with an element for each test case

    array[0]  = {true,  49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f,  5000.0f, 5000.0f,  150.0f,  250.0f};  // soc below, discharging
    array[1]  = {true,  49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -250.0f,  250.0f};  // soc below, charging
    array[2]  = {false, 49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f,  5000.0f, 5000.0f,  5000.0f, 5000.0f}; // soc below, but disabled, no effect
    array[3]  = {false, 49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -5000.0f, 5000.0f}; // soc below, but disabled, no effect
    array[4]  = {true,  50.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f,  5000.0f, 5000.0f,  150.0f,  250.0f};  // soc equal, discharging
    array[5]  = {true,  50.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -250.0f,  250.0f};  // soc equal, charging
    array[6]  = {true,  51.0f, 50.0f, -250.0f, 250.0f, -1000.0f, 750.0f,  5000.0f, 5000.0f,  750.0f,  1000.0f}; // soc above, discharging
    array[7]  = {true,  51.0f, 50.0f, -250.0f, 250.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -1000.0f, 1000.0f}; // soc above, charging
 
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "apply_active_power_poi_limits() SoC test " << i+1 << " of " << num_tests << std::endl;
        
        // Set independent variables
        asset_cmd.set_load_compensation_method(NO_COMPENSATION);
        asset_cmd.set_ess_max_potential_kW(5000.0f);
        asset_cmd.set_ess_min_potential_kW(-5000.0f);
        asset_cmd.set_site_kW_demand(array[i].site_demand);
        asset_cmd.set_feature_kW_demand(array[i].site_demand);
        asset_cmd.set_feeder_max_potential_kW(array[i].feed_potential);

        // Apply limits
        active_power_poi_limits_min_kW.value.set(-10000.0f);
        active_power_poi_limits_max_kW.value.set(10000.0f);
        active_power_soc_poi_limits_enable.value.set(array[i].soc_limits_enabled);
        soc_avg_running.value.set(array[i].avg_soc);
        active_power_soc_poi_target_soc.value.set(array[i].soc_target);
        active_power_soc_poi_limits_low_min_kW.value.set(array[i].under_min);
        active_power_soc_poi_limits_low_max_kW.value.set(array[i].under_max);
        active_power_soc_poi_limits_high_min_kW.value.set(array[i].over_min);
        active_power_soc_poi_limits_high_max_kW.value.set(array[i].over_max);
        charge_dispatch_feeder_enable_flag.value.set(true);
        apply_active_power_poi_limits();

        // failure conditions
        failure = array[i].expected_site_demand != asset_cmd.get_site_kW_demand() || array[i].expected_feed_potential != asset_cmd.get_feeder_max_potential_kW();
        // Analyze expected
        EXPECT_EQ(array[i].expected_site_demand, asset_cmd.get_site_kW_demand());
        EXPECT_EQ(array[i].expected_feed_potential, asset_cmd.get_feeder_max_potential_kW());

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// POI limits
TEST_F(site_manager_test, apply_reactive_poi_limits)
{
    int const num_tests = 4;    //total number of test cases

    //struct that has variables to configure for each test case
    struct tests
    {
        float site_demand;
        float min_poi_limit_kVAR;
        float max_poi_limit_kVAR;
        float expected_demand;
    };

    tests array[num_tests];  //an array with an element for each test case
    // Ensure that load and uncontrollable properly limit the site demand based on import/export, but do not influence the feed max
    array[0]  = { 5000,     -10000,     10000,       5000};  // discharging, no limit
    array[1]  = {-5000,     -10000,     10000,      -5000};  // charging, no limit
    array[2]  = { 15000,     -10000,     10000,     10000};  // discharging, above limit
    array[3]  = {-15000,     -10000,     10000,    -10000};  // charging, above limit
    
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "apply_reactive_power_poi_limits() test " << i+1 << " of " << num_tests << std::endl;
        
        // Set independent variables
        asset_cmd.set_site_kVAR_demand(array[i].site_demand);

        // Apply limits
        reactive_power_poi_limits_min_kVAR.value.set(array[i].min_poi_limit_kVAR);
        reactive_power_poi_limits_max_kVAR.value.set(array[i].max_poi_limit_kVAR);
        apply_reactive_power_poi_limits();

        // failure conditions
        failure = asset_cmd.get_site_kVAR_demand() != array[i].expected_demand;
        // Analyze expected
        EXPECT_EQ(asset_cmd.get_site_kVAR_demand(), array[i].expected_demand);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}


// check_expired_time
TEST_F(site_manager_test, check_expired_time)
{
    int const num_tests = 4;  //total number of test cases
    
    //struct that has variables to configure for each test case
    struct tests 
    {
        timespec current_time;
        timespec target_time;
        bool result;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables for each test case
    array[0] = {{0,0}, {5,0}, false};       //Current time before target time
    array[1] = {{5,100}, {5,0}, false};     //Current time after target time but before buffer
    array[2] = {{6,0}, {5,0}, true};        //Current time after target time and buffer
    array[3] = {{6,2000000}, {5,0}, true};  //Current time after target time and buffer

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "check_expired_time() test " << i+1 << " of " + num_tests << std::endl;
        // failure conditions
        failure = array[i].result != check_expired_time(array[i].current_time, array[i].target_time);
        EXPECT_EQ(array[i].result, check_expired_time(array[i].current_time, array[i].target_time));
        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// bit position functions
TEST_F(site_manager_test, bit_positions)
{
    int const num_tests = 9;  //total number of test cases
    int result_integer = 0;

    //struct that has variables to configure for each test case
    struct tests 
    {
        int bit_position;
        int expected_integer;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    //         bit_pos  exp_int   
    array[0] = {0,      1}; //
    array[1] = {1,      2}; //
    array[2] = {2,      4}; //
    array[3] = {3,      8}; // 
    array[4] = {4,      16}; // 
    array[5] = {5,      32}; // 
    array[6] = {6,      64}; // 
    array[7] = {-1,     0}; //invalid case
    array[8] = {-2,     0}; //invalid case

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        result_integer = get_int_from_bit_position(array[i].bit_position);

        errorLog << "bit position functions test " << i+1 << " of " << num_tests << std::endl;
        // Failure conditions
        failure = result_integer != array[i].expected_integer;
        // Let EXPECT print the details
        EXPECT_EQ(result_integer, array[i].expected_integer);
        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//set fims masked int
TEST_F(site_manager_test, set_fims_masked_int)
{
    int const num_tests = 10;  //total number of test cases

    Fims_Object* features_mode_cmd = new Fims_Object();  //fims variable for test
    const char* var_id = "features_mode_cmd"; //set var_id for matching 
    features_mode_cmd->set_variable_id(var_id); //set var_id for matching

    //struct that has variables to configure for each test case
    struct tests 
    {
        std::string mask;
        int requested_mode;
        int initial_mode;
        int expected_mode;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
               //mask    request   init  expected
    array[0] = {"0x01",     0,      0,      0}; //base case, no change requested
    array[1] = {"0x41",     0,      6,      0}; // request valid mode 0
    array[2] = {"0x42",     1,      6,      1}; // request valid mode 1
    array[3] = {"0x44",     2,      6,      2}; // request valid mode 2
    array[4] = {"0x48",     3,      6,      3}; // request valid mode 3
    array[5] = {"0x50",     4,      6,      4}; // request valid mode 4
    array[6] = {"0x02",     0,      1,      1}; // request invalid mode low
    array[7] = {"0x02",     2,      1,      1}; // request invalid mode high
    array[8] = {"0x3F",     6,      2,      2}; // request invalid mode out of bounds
    array[9] = {"0x04",     -2,     2,      2}; // request invalid mode negative

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        features_mode_cmd->set_fims_int(var_id, array[i].initial_mode);
        features_mode_cmd->set_fims_masked_int(var_id, array[i].requested_mode, (uint64_t) std::stoul(array[i].mask, NULL, 16));
        errorLog << "set_fims_masked_int() test " << i+1 << " of " << num_tests << std::endl;
        // Failure conditions
        failure = features_mode_cmd->value.value_int != array[i].expected_mode;
        // Let EXPECT print the details
        EXPECT_EQ(features_mode_cmd->value.value_int, array[i].expected_mode);
        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

class EA_Mock : public Energy_Arbitrage
{
public:
    
};
class energy_arbitrage_test : public testing::Test
{ 
public:
    EA_Mock ea_mock;
	virtual void SetUp() {}
	virtual void TearDown(){}
};
TEST_F(energy_arbitrage_test, energy_arbitrage)
{
    int const num_tests = 13;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float solar_max_potential;
        float price;
        float threshold_charge_2;
        float threshold_charge_1;
        float threshold_dischg_1;
        float threshold_dischg_2;
        float max_charge_2;
        float max_charge_1;
        float max_dischg_1;
        float max_dischg_2;
        float avg_running_soc;
        float min_soc_limit;
        float max_soc_limit;
        float expected_ess_kW_request;
        float expected_solar_kW_request;
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    array[0] = {3000, -10, 11, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 0};            // Invalid thresholds: t1 > t2
    array[1] = {3000, -10, -10, 31, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 0};           // Invalid thresholds: t2 > t3
    array[2] = {3000, -10, -10, 10, 101, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 0};          // Invalid thresholds: t3 > t4
    array[3] = {3000, -10, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, -2000, 0};       // default charge2
    array[4] = {3000, 0, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, -1000, 0};         // default charge1
    array[5] = {3000, 1, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, -1000, 3000};      // charge1, solar discharge
    array[6] = {3000, 15, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 3000};         // default 0
    array[7] = {3000, 30, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 1000, 3000};      // default discharge1
    array[8] = {3000, 100, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 2000, 3000};     // default discharge2
    array[9] = {3000, -10, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 90, 0, 90, -2000, 0};        // soc limited charge2, soc >= max but thresholds N/A due to price
    array[10] = {3000, 0, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 49, 0, 0};             // soc limited charge1, soc >= max
    array[11] = {3000, 30, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 51, 100, 0, 3000};       // soc limited discharge1, soc <= min
    array[12] = {3000, 100, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 10, 10, 100, 2000, 3000};   // soc limited discharge2, soc <= min but thresholds N/A due to price

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();
        ea_mock.price.value.value_float = array[i].price;
        ea_mock.threshold_charge_2.value.value_float = array[i].threshold_charge_2;
        ea_mock.threshold_charge_1.value.value_float = array[i].threshold_charge_1;
        ea_mock.threshold_dischg_1.value.value_float = array[i].threshold_dischg_1;
        ea_mock.threshold_dischg_2.value.value_float = array[i].threshold_dischg_2;
        ea_mock.max_charge_2.value.value_float = array[i].max_charge_2;
        ea_mock.max_charge_1.value.value_float = array[i].max_charge_1;
        ea_mock.max_dischg_1.value.value_float = array[i].max_dischg_1;
        ea_mock.max_dischg_2.value.value_float = array[i].max_dischg_2;
        ea_mock.soc_min_limit.value.value_float = array[i].min_soc_limit;
        ea_mock.soc_max_limit.value.value_float = array[i].max_soc_limit;

        Energy_Arbitrage_Inputs in {
            array[i].avg_running_soc,
            array[i].solar_max_potential
        };
        auto out = ea_mock.energy_arbitrage(in);
        errorLog << "energy_arbitrage() test " << i+1 << " of " << num_tests << std::endl;
        bool failure = out.ess_kW_power != array[i].expected_ess_kW_request
                    || out.solar_kW_request != array[i].expected_solar_kW_request;
        EXPECT_EQ(out.ess_kW_power, array[i].expected_ess_kW_request);
        EXPECT_EQ(out.solar_kW_request, array[i].expected_solar_kW_request);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

//Target SoC Demand mode
TEST_F(site_manager_test, pfr)
{
    int const num_tests = 30;  //total number of test cases

    //struct that has variables to configure for each test case
    struct tests 
    {
        float kW_cmd;
        float site_hz;
        float pfr_min;
        float pfr_max;
        float expected_demand; // Original demand + limited response modifier
    };

    tests array[num_tests];  //an array with an element for each test case

    //configure variables each test case
    // Expected request/demand comes from x = response_percent * pfr_min/max, solve for response percent to get delta hz then add deadband
    array[0] =  {-10.0f,   63.5f,       -15.0f,    15.0f,     -15.0f};  // Large charge, OF hits min limit
    array[1] =  {-10.0f,   61.817f,     -15.0f,    15.0f,     -15.0f}; 
    array[2] =  {-10.0f,   60.005f,     -15.0f,    15.0f,     -10.0f};
    array[3] =  {-10.0f,   59.995f,     -15.0f,    15.0f,     -10.0f};
    array[4] =  {-10.0f,   58.183f,     -15.0f,    15.0f,     -1.0f}; 
    array[5] =  {-10.0f,   56.5f,       -15.0f,    15.0f,      0.0f}; 
    array[6] =  {-5.0f,    63.5f,       -15.0f,    15.0f,     -15.0f};  // Small charge, OF within limit
    array[7] =  {-5.0f,    61.817f,     -15.0f,    15.0f,     -14.0f};  // UF hits 0 and doesn't change sign
    array[8] =  {-5.0f,    60.005f,     -15.0f,    15.0f,     -5.0f};
    array[9] =  {-5.0f,    59.995f,     -15.0f,    15.0f,     -5.0f};
    array[10] = {-5.0f,    58.183f,     -15.0f,    15.0f,      0.0f}; 
    array[11] = {-5.0f,    56.5f,       -15.0f,    15.0f,      0.0f}; 
    array[12] = { 0.0f,    63.5f,       -15.0f,    15.0f,     -15.0f};  // 0 and command can change sign, plenty of room
    array[13] = { 0.0f,    61.817f,     -15.0f,    15.0f,     -9.0f}; 
    array[14] = { 0.0f,    60.005f,     -15.0f,    15.0f,      0.0f};
    array[15] = { 0.0f,    59.995f,     -15.0f,    15.0f,      0.0f};
    array[16] = { 0.0f,    58.183f,     -15.0f,    15.0f,      9.0f}; 
    array[17] = { 0.0f,    56.5f,       -15.0f,    15.0f,      15.0f};
    array[18] = { 5.0f,    63.5f,       -15.0f,    15.0f,      0.0f};  // Small discharge, UF within limit
    array[19] = { 5.0f,    61.817f,     -15.0f,    15.0f,      0.0f};  // OF hits 0 and doesn't change sign
    array[20] = { 5.0f,    60.005f,     -15.0f,    15.0f,      5.0f};
    array[21] = { 5.0f,    59.995f,     -15.0f,    15.0f,      5.0f};
    array[22] = { 5.0f,    58.183f,     -15.0f,    15.0f,      14.0f}; 
    array[23] = { 5.0f,    56.5f,       -15.0f,    15.0f,      15.0f}; 
    array[24] = {10.0f,    63.5f,       -15.0f,    15.0f,      0.0f};  // Large discharge, UF hits max limit
    array[25] = {10.0f,    61.817f,     -15.0f,    15.0f,      1.0f}; 
    array[26] = {10.0f,    60.005f,     -15.0f,    15.0f,      10.0f};
    array[27] = {10.0f,    59.995f,     -15.0f,    15.0f,      10.0f};
    array[28] = {10.0f,    58.183f,     -15.0f,    15.0f,      15.0f}; 
    array[29] = {10.0f,    56.5f,       -15.0f,    15.0f,      15.0f}; 
    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        // Apply PFR
        asset_cmd.set_site_kW_demand(array[i].kW_cmd);
        site_frequency.value.set(array[i].site_hz);
        pfr_site_nominal_hz.value.set(60.0f);
        pfr_deadband.value.set(0.017f);
        pfr_limits_min_kW.value.set(array[i].pfr_min);
        pfr_limits_max_kW.value.set(array[i].pfr_max);
        pfr_droop_percent.value.set(5.0f);
        primary_frequency_response(array[i].kW_cmd);

        errorLog << "primary_frequency_response() demand test " << i+1 << " of " << num_tests << std::endl;
        bool failure = !near(asset_cmd.get_site_kW_demand(), array[i].expected_demand, 0.001);
        EXPECT_NEAR(asset_cmd.get_site_kW_demand(), array[i].expected_demand, 0.001);

        // Release stdout so we can write again
		release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Constant Power Factor algorithm tests
TEST_F(site_manager_test, constant_power_factor)
{
    // struct that has variables to configure for each test case
    struct cpf_test
    {
        bool absolute_mode;
        bool direction_flag;
        float lagging_limit;
        float leading_limit;
        float pf_setpoint;
        float demand_kW;
        float potential_kVAR;
        float expected_pf;
        float expected_kVAR;
    };

    std::vector<cpf_test> tests = {
        {true, false,  -0.5f,  0.5f,  1.0f,   10000.0f, 10000.0f,  1.0f,   0.0f},       // Absolute mode tests, 0 reactive
        {true, true,   -0.5f,  0.5f, -0.97f,  10000.0f, 10000.0f, -0.97f, -2500.0f},    // Lagging reactive
        {true, true,   -0.5f,  0.5f,  0.97f,  10000.0f, 10000.0f, -0.97f, -2500.0f},    // Lagging reactive, positive pf forced negative
        {true, false,  -0.5f,  0.5f,  0.97f,  10000.0f, 10000.0f,  0.97f,  2500.0f},    // Leading reactive
        {true, false,  -0.5f,  0.5f, -0.97f,  10000.0f, 10000.0f,  0.97f,  2500.0f},    // Leading reactive, negative pf forced positive
        {true, true,   -0.5f,  0.5f,  0.7f,   10000.0f, 10000.0f, -0.7f,  -10000.0f},   // Lagging reactive, beyond total potential KVAR but limited
        {true, false,  -0.5f,  0.5f,  0.7f,   10000.0f, 10000.0f,  0.7f,   10000.0f},   // Leading reactive, beyond total potential KVAR but limited
        {true, false,  -0.0f,  0.0f,  0.0f,   10000.0f, 10000.0f,  0.0f,   10000.0f},   // Divide by 0 case
        {true, true,   -0.5f,  0.5f,  1.1f,   10000.0f, 10000.0f, -1.0f,   0.0f},       // Lagging, beyond outer limit, limited to -1.0
        {true, true,   -0.5f,  0.5f,  0.0f,   10000.0f, 10000.0f, -0.5f,  -10000.0f},   // Lagging, beyond inner limit, limited to lagging limit
        {true, false,  -0.5f,  0.5f,  1.1f,   10000.0f, 10000.0f,  1.0f,   0.0f},       // Leading, beyond outer limit, limited to 1.0
        {true, false,  -0.5f,  0.5f,  0.0f,   10000.0f, 10000.0f,  0.5f,   10000.0f},   // Leading, beyond inner limit, limited to lagging limit
        {false, false, -0.5f,  0.5f,  1.0f,   10000.0f, 10000.0f,  1.0f,   0.0f},       // Bidirectional tests, 0 reactive
        {false, false,  0.5f,  0.5f, -0.97f,  10000.0f, 10000.0f, -0.97f, -2500.0f},    // Positive lagging limit forced negative
        {false, false, -0.5f, -0.5f,  0.97f,  10000.0f, 10000.0f,  0.97f,  2500.0f},    // Negative leading limit forced positive
        {false, false,  1.1f,  0.5f, -1.1f,   10000.0f, 10000.0f, -1.0f,   0.0f},       // Positive lagging limit above 1.0
        {false, false, -0.5f, -1.1f,  1.1f,   10000.0f, 10000.0f,   1.0f,   0.0f},      // Negative leading limit below -1.0
        {false, false, -0.5f,  0.5f, -0.97f,  10000.0f, 10000.0f, -0.97f, -2500.0f},    // Lagging reactive
        {false, false, -0.5f,  0.5f,  0.97f,  10000.0f, 10000.0f,  0.97f,  2500.0f},    // Leading reactive
        {false, false, -0.5f,  0.5f, -0.7f,   10000.0f, 10000.0f, -0.7f,  -10000.0f},   // Lagging reactive, beyond total potential KVAR but limited
        {false, false, -0.5f,  0.5f,  0.7f,   10000.0f, 10000.0f,  0.7f,   10000.0f},   // Leading reactive, beyond total potential KVAR but limited
        {false, false, -0.0f,  0.0f,  0.0f,   10000.0f, 10000.0f,  0.0f,   10000.0f},   // Divide by 0 case
        {false, false, -0.5f,  0.5f, -1.1f,   10000.0f, 10000.0f, -1.0f,   0.0f},       // Lagging, beyond outer limit, limited to -1.0
        {false, false, -0.5f,  0.5f, -0.0f,   10000.0f, 10000.0f, -0.5f,  -10000.0f},   // Lagging, beyond inner limit, limited to lagging limit
        {false, false, -0.5f,  0.5f,  1.0f,   10000.0f, 10000.0f,  1.0f,   0.0f},       // Leading, beyond outer limit, limited to 1.0
        {false, false, -0.5f,  0.5f,  0.0f,   10000.0f, 10000.0f,  0.5f,   10000.0f},   // Leading, beyond inner limit, limited to lagging limit
        {true, false,  -0.5f,  0.5f,  1.0f,   10000.0f, 10000.0f,  1.0f,   0.0f},       // Ensure negative site demand produces the same result for the same tests
        {true, true,   -0.5f,  0.5f, -0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f},
        {true, true,   -0.5f,  0.5f,  0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f},
        {true, false,  -0.5f,  0.5f,  0.97f, -10000.0f, 10000.0f,  0.97f,  2500.0f},
        {true, false,  -0.5f,  0.5f, -0.97f, -10000.0f, 10000.0f,  0.97f,  2500.0f},
        {true, true,   -0.5f,  0.5f,  0.7f,  -10000.0f, 10000.0f, -0.7f,  -10000.0f},
        {true, false,  -0.5f,  0.5f,  0.7f,  -10000.0f, 10000.0f,  0.7f,   10000.0f},
        {true, false,  -0.0f,  0.0f,  0.0f,  -10000.0f, 10000.0f,  0.0f,   10000.0f},
        {true, true,   -0.5f,  0.5f,  1.1f,  -10000.0f, 10000.0f, -1.0f,   0.0f},
        {true, true,   -0.5f,  0.5f,  0.0f,  -10000.0f, 10000.0f, -0.5f,  -10000.0f},
        {true, false,  -0.5f,  0.5f,  1.1f,  -10000.0f, 10000.0f,  1.0f,   0.0f},
        {true, false,  -0.5f,  0.5f,  0.0f,  -10000.0f, 10000.0f,  0.5f,   10000.0f},
        {false, false, -0.5f,  0.5f,  1.0f,  -10000.0f, 10000.0f,  1.0f,   0.0f},
        {false, false,  0.5f,  0.5f, -0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f},
        {false, false, -0.5f, -0.5f,  0.97f, -10000.0f, 10000.0f,  0.97f,  2500.0f},
        {false, false,  1.1f,  0.5f, -1.1f,  -10000.0f, 10000.0f, -1.0f,   0.0f},
        {false, false, -0.5f, -1.1f,  1.1f,  -10000.0f, 10000.0f,   1.0f,   0.0f},
        {false, false, -0.5f,  0.5f, -0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f},
        {false, false, -0.5f,  0.5f,  0.97f, -10000.0f, 10000.0f,  0.97f,  2500.0f},
        {false, false, -0.5f,  0.5f, -0.7f,  -10000.0f, 10000.0f, -0.7f,  -10000.0f},
        {false, false, -0.5f,  0.5f,  0.7f,  -10000.0f, 10000.0f,  0.7f,   10000.0f},
        {false, false, -0.0f,  0.0f,  0.0f,  -10000.0f, 10000.0f,  0.0f,   10000.0f},
        {false, false, -0.5f,  0.5f, -1.1f,  -10000.0f, 10000.0f, -1.0f,   0.0f},
        {false, false, -0.5f,  0.5f, -0.0f,  -10000.0f, 10000.0f, -0.5f,  -10000.0f},
        {false, false, -0.5f,  0.5f,  1.0f,  -10000.0f, 10000.0f,  1.0f,   0.0f},
        {false, false, -0.5f,  0.5f,  0.0f,  -10000.0f, 10000.0f,  0.5f,   10000.0f},
    };    
    int test_id = 1;
    for (auto test : tests)
    {
        test_logger t_log("constant_power_factor", test_id++, tests.size());
        // Set CPF values
        constant_power_factor_absolute_mode.value.set(test.absolute_mode);
        constant_power_factor_lagging_direction.value.set(test.direction_flag);
        constant_power_factor_lagging_limit.value.set(test.lagging_limit);
        constant_power_factor_leading_limit.value.set(test.leading_limit);
        constant_power_factor_setpoint.value.set(test.pf_setpoint);
        // Setup Asset Cmd Obj inputs
        asset_cmd.set_site_kW_demand(test.demand_kW);
        asset_cmd.preserve_uncorrected_site_kW_demand();
        asset_cmd.set_ess_potential_kVAR(test.potential_kVAR);
        asset_cmd.calculate_total_potential_kVAR();
        execute_constant_power_factor();
        t_log.float_results.push_back({test.expected_pf, constant_power_factor_setpoint.value.value_float, "PF Setpoint"});
        t_log.range_results.push_back({test.expected_kVAR, 0.05f, asset_cmd.get_site_kVAR_demand(), "CPF Demand"});
        t_log.check_solution();
    }
}
