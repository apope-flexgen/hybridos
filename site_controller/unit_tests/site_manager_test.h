#ifndef SITE_MANAGER_TEST_H_
#define SITE_MANAGER_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Asset_Manager.h"
#include "Site_Manager.h"
#include <Site_Controller_Utils.h>
#include "Types.h"
#include <test_tools.h>
#include "frequency_response_test.h"
#include <Features/Active_Power_Setpoint.h>
#include <Features/AVR.h>
#include <Features/Watt_Var.h>
#include <Features/Reactive_Setpoint.h>

class site_manager_test : public Site_Manager, public testing::Test {
public:
    site_manager_test() : Site_Manager(nullptr) {}
    site_manager_test(Version* v) : Site_Manager(v) {}

    void SetUp() {
        // For some reason the Asset_Cmd_Object holds on to its member variables across tests
        // TODO: Look into this more for a better solution
        // All other Site Manager member variables seem to be cleared across tests so there are not issues
        asset_cmd = Asset_Cmd_Object();
    }
};

// ess discharge
// Verify load calculation using rolling average
TEST_F(site_manager_test, calculate_site_kW_load) {
    int const num_tests = 16;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        int buffer_size;
        std::vector<float> ess_actual;
        std::vector<float> feed_actual;
        std::vector<float> expected_load;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables for each test case
    array[0] = { 1, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f }, { -100.0f, -105.0f, -110.0f, -115.0f, -120.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } };       // zero load
    array[1] = { 1, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f } };            // constant load from POI
    array[2] = { 1, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f } };            // constant load from ESS
    array[3] = { 1, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 200.0f, 200.0f, 200.0f, 200.0f, 200.0f } };  // constant load from POI w/ ESS
    array[4] = { 1, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f }, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f } };            // changing load from POI
    array[5] = { 1, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f } };            // changing load from ESS
    array[6] = { 1, { 100.0f, 105.0f, 110.0f, -115.0f, -120.0f }, { -100.0f, -105.0f, -110.0f, -110.0f, 120.0f }, { 0.0f, 0.0f, 0.0f, 230.0f, 0.0f } };    // ESS changes, feed is delayed
    array[7] = { 1, { 100.0f, 105.0f, -110.0f, -115.0f, -120.0f }, { 0.0f, -5.0f, -10.0f, -15.0f, 220.0f }, { 100.0f, 100.0f, -220.0f, -230.0f, 0.0f } };  // ESS changes w/ load, feed is delayed
    array[8] = { 5, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f }, { -100.0f, -105.0f, -110.0f, -115.0f, -120.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } };       // Same tests with smoothing
    array[9] = { 5, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 20.0f, 40.0f, 60.0f, 80.0f, 100.0f } };
    array[10] = { 5, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 20.0f, 40.0f, 60.0f, 80.0f, 100.0f } };
    array[11] = { 5, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 100.0f, 100.0f, 100.0f, 100.0f, 100.0f }, { 40.0f, 80.0f, 120.0f, 160.0f, 200.0f } };
    array[12] = { 5, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f }, { 20.0f, 41.0f, 63.0f, 86.0f, 110.0f } };
    array[13] = { 5, { 100.0f, 105.0f, 110.0f, 115.0f, 120.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 20.0f, 41.0f, 63.0f, 86.0f, 110.0f } };
    array[14] = { 5, { 100.0f, 105.0f, 110.0f, -115.0f, -120.0f }, { -100.0f, -105.0f, -110.0f, -110.0f, 120.0f }, { 0.0f, 0.0f, 0.0f, -45.0f, -45.0f } };
    array[15] = { 5, { 100.0f, 105.0f, -110.0f, -115.0f, -120.0f }, { 0.0f, -5.0f, -10.0f, -10.0f, 120.0f }, { 20.0f, 40.0f, 16.0f, -9.0f, -9.0f } };

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        asset_cmd.create_site_kW_load_buffer(array[i].buffer_size);
        for (int j = 0; j < array[i].buffer_size; j++) {
            // Only print messages to log if a test fails
            bool failure = false;
            std::stringstream errorLog;
            // // Capture any prints within site controller that might be present in debug mode
            capture_stdout();

            asset_cmd.ess_data.actual_kW = array[i].ess_actual[j];
            asset_cmd.feeder_data.actual_kW = array[i].feed_actual[j];
            asset_cmd.calculate_site_kW_load();

            errorLog << "calculate_site_kW_load() test " << i + 1 << " of " << num_tests << std::endl;
            // failure conditions
            failure = asset_cmd.site_kW_load != array[i].expected_load[j];
            EXPECT_EQ(asset_cmd.site_kW_load, array[i].expected_load[j]);

            // Release stdout so we can write again
            release_stdout(failure);
            // Print the test id if failure
            if (failure)
                std::cout << errorLog.str() << std::endl;
        }
    }
}

// Calculate the amount of additional load compensation that should be provided for slewed features
TEST_F(site_manager_test, track_slewed_load) {
    // struct that has variables to configure for each test case
    struct tests {
        load_compensation load_method;
        float starting_demand;
        float new_demand;
        float additional_load;
        int slew_rate;
        float expected_load;
    };

    std::vector<tests> tests = {
        { NO_COMPENSATION, 0.0f, 0.0f, 0.0f, 10, 0.0f },          { LOAD_MINIMUM, 0.0f, 500.0f, 500.0f, 10, 0.1f },  // Test: just started slewing, no command
        { LOAD_MINIMUM, 0.0f, 1000.0f, 0.0f, 10, 0.0f },                                                             // Command in same direction
        { LOAD_MINIMUM, 0.0f, -1000.0f, 500.0f, 10, 0.05f },                                                         // Command in opposite direction
        { LOAD_OFFSET, 0.0f, 500.0f, 500.0f, 10, 0.1f },                                                             // No command
        { LOAD_OFFSET, 0.0f, 1500.0f, 500.0f, 10, 0.033f },                                                          // Command in same direction
        { LOAD_OFFSET, 0.0f, -1000.0f, 500.0f, 10, 0.05f },                                                          // Command in opposite direction
        { LOAD_MINIMUM, 250.0f, 500.0f, 500.0f, 10, 250.0f },                                                        // Test: halfway to target, no command
        { LOAD_MINIMUM, 500.0f, 1000.0f, 0.0f, 10, 0.0f },                                                           // Command in same direction
        { LOAD_MINIMUM, -500.0f, -1000.0f, 500.0f, 10, 250.0f },                                                     // Command in opposite direction
        { LOAD_OFFSET, 250.0f, 500.0f, 500.0f, 10, 250.0f },                                                         // No command
        { LOAD_OFFSET, 750.0f, 1500.0f, 500.0f, 10, 250.0f },                                                        // Command in same direction
        { LOAD_OFFSET, -500.0f, -1000.0f, 500.0f, 10, 250.0f },                                                      // Command in opposite direction
        { LOAD_MINIMUM, 500.0f, 500.0f, 500.0f, 10, 500.0f },                                                        // Test: at target, no command
        { LOAD_MINIMUM, 1000.0f, 1000.0f, 0.0f, 10, 0.0f },                                                          // Command in same direction
        { LOAD_MINIMUM, -1000.0f, -1000.0f, 500.0f, 10, 500.0f },                                                    // Command in opposite direction
        { LOAD_OFFSET, 500.0f, 500.0f, 500.0f, 10, 500.0f },                                                         // No command
        { LOAD_OFFSET, 1500.0f, 1500.0f, 500.0f, 10, 500.0f },                                                       // Command in same direction
        { LOAD_OFFSET, -1000.0f, -1000.0f, 500.0f, 10, 500.0f },                                                     // Command in opposite direction
    };

    uint test_id = 1;
    // iterate through each test case and get results
    for (auto test : tests) {
        test_logger t_log("track_slewed_load", test_id++, tests.size());
        // Setup feature slew object at starting demand
        active_power_setpoint_mode.kW_slew.reset_slew_target(10000);
        active_power_setpoint_mode.kW_slew.update_slew_target(test.starting_demand);
        usleep(10000);  // wait one iteration
        active_power_setpoint_mode.kW_slew.set_slew_rate(test.slew_rate);
        active_power_setpoint_mode.kW_slew.update_slew_target(test.starting_demand);
        asset_cmd.site_kW_demand = active_power_setpoint_mode.kW_slew.get_slew_target(test.new_demand);
        asset_cmd.additional_load_compensation = asset_cmd_utils::track_slewed_load(test.load_method, asset_cmd.site_kW_demand, test.new_demand, test.additional_load, active_power_setpoint_mode.kW_slew);
        t_log.range_results.push_back({ test.expected_load, 0.1f, asset_cmd.additional_load_compensation, "additional_load_compensation" });
        t_log.check_solution();
    }
}

// Ensures that output from feature (combination of demand, requests and load) produces valid feature and site demand values
TEST_F(site_manager_test, calculate_feature_kW_demand) {
    // struct that has variables to configure for each test case
    struct tests {
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
        { 0.0f, 0.0f, 0.0f, 0.0f, NO_COMPENSATION, 0.0f, 0.0f, 0.0f },                   // Nothing
        { 0.0f, 500.0f, 0.0f, 0.0f, NO_COMPENSATION, 0.0f, 500.0f, 500.0f },             // ess request
        { 0.0f, -500.0f, 0.0f, 0.0f, NO_COMPENSATION, 0.0f, -500.0f, -500.0f },          // negative ess request
        { 0.0f, 500.0f, 1500.0f, 750.0f, NO_COMPENSATION, 0.0f, 2750.0f, 2750.0f },      // all assets
        { 0.0f, -500.0f, 1500.0f, 750.0f, NO_COMPENSATION, 0.0f, 1750.0f, 1750.0f },     // all assets negative ess
        { 1000.0f, 0.0f, 0.0f, 0.0f, NO_COMPENSATION, 0.0f, 1000.0f, 1000.0f },          // Same tests with demand
        { 1000.0f, 500.0f, 0.0f, 0.0f, NO_COMPENSATION, 0.0f, 1500.0f, 1500.0f },        // ess request
        { 1000.0f, -500.0f, 0.0f, 0.0f, NO_COMPENSATION, 0.0f, 500.0f, 500.0f },         // negative ess request
        { 1000.0f, 500.0f, 1500.0f, 750.0f, NO_COMPENSATION, 0.0f, 3750.0f, 3750.0f },   // all assets
        { 1000.0f, -500.0f, 1500.0f, 750.0f, NO_COMPENSATION, 0.0f, 2750.0f, 2750.0f },  // all assets negative ess
        { 0.0f, 0.0f, 0.0f, 0.0f, LOAD_OFFSET, 0.0f, 0.0f, 0.0f },                       // Same tests with load offset
        { 0.0f, 0.0f, 0.0f, 0.0f, NO_COMPENSATION, 500.0f, 0.0f, 0.0f },                 // Additional test with load but not enabled
        { 0.0f, 500.0f, 0.0f, 0.0f, LOAD_OFFSET, 500.0f, 1000.0f, 1000.0f },             // ess request
        { 0.0f, -500.0f, 0.0f, 0.0f, LOAD_OFFSET, 500.0f, 0.0f, 0.0f },                  // negative ess request
        { 0.0f, 500.0f, 1500.0f, 750.0f, LOAD_OFFSET, 500.0f, 3250.0f, 3250.0f },        // all assets
        { 0.0f, -500.0f, 1500.0f, 750.0f, LOAD_OFFSET, 500.0f, 2250.0f, 2250.0f },       // all assets negative ess
        { 1000.0f, 0.0f, 0.0f, 0.0f, LOAD_OFFSET, 500.0f, 1500.0f, 1500.0f },            // Same tests with demand
        { 1000.0f, 500.0f, 0.0f, 0.0f, LOAD_OFFSET, 500.0f, 2000.0f, 2000.0f },          // ess request
        { 1000.0f, -500.0f, 0.0f, 0.0f, LOAD_OFFSET, 500.0f, 1000.0f, 1000.0f },         // negative ess request
        { 1000.0f, 500.0f, 1500.0f, 750.0f, LOAD_OFFSET, 500.0f, 4250.0f, 4250.0f },     // all assets
        { 1000.0f, -500.0f, 1500.0f, 750.0f, LOAD_OFFSET, 500.0f, 3250.0f, 3250.0f },    // all assets negative ess
        { 0.0f, 0.0f, 0.0f, 0.0f, LOAD_MINIMUM, 0.0f, 0.0f, 0.0f },                      // Same tests with load minimum
        { 0.0f, 500.0f, 0.0f, 0.0f, LOAD_MINIMUM, 500.0f, 500.0f, 500.0f },              // ess request
        { 0.0f, -500.0f, 0.0f, 0.0f, LOAD_MINIMUM, 500.0f, 0.0f, 0.0f },                 // negative ess request
        { 0.0f, 500.0f, 1500.0f, 750.0f, LOAD_MINIMUM, 500.0f, 2750.0f, 2750.0f },       // all assets
        { 0.0f, -500.0f, 1500.0f, 750.0f, LOAD_MINIMUM, 500.0f, 1750.0f, 1750.0f },      // all assets negative ess
        { 1000.0f, 0.0f, 0.0f, 0.0f, LOAD_MINIMUM, 500.0f, 1000.0f, 1000.0f },           // Same tests with demand
        { 1000.0f, 500.0f, 0.0f, 0.0f, LOAD_MINIMUM, 500.0f, 1500.0f, 1500.0f },         // ess request
        { 1000.0f, -500.0f, 0.0f, 0.0f, LOAD_MINIMUM, 500.0f, 500.0f, 500.0f },          // negative ess request
        { 1000.0f, 500.0f, 1500.0f, 750.0f, LOAD_MINIMUM, 500.0f, 3750.0f, 3750.0f },    // all assets
        { 1000.0f, -500.0f, 1500.0f, 750.0f, LOAD_MINIMUM, 500.0f, 2750.0f, 2750.0f },   // all assets negative ess
    };

    uint test_id = 1;
    // iterate through each test case and get results
    for (auto test : tests) {
        test_logger t_log("calculate_feature_kW_demand", test_id++, tests.size());
        asset_cmd.ess_data.max_potential_kW = 10000;
        asset_cmd.ess_data.min_potential_kW = -10000;
        asset_cmd.gen_data.max_potential_kW = 10000;
        asset_cmd.solar_data.max_potential_kW = 10000;
        asset_cmd.site_kW_demand = test.site_demand;
        asset_cmd.ess_data.kW_request = test.ess_request;
        asset_cmd.gen_data.kW_request = test.gen_request;
        asset_cmd.solar_data.kW_request = test.solar_request;
        asset_cmd.set_load_compensation_method(test.load_inclusion);
        asset_cmd.site_kW_load = test.load;

        asset_cmd.load_method = test.load_inclusion;
        asset_cmd.additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(asset_cmd.load_method, asset_cmd.site_kW_load, asset_cmd.site_kW_demand, asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request,
                                                                                                         asset_cmd.solar_data.kW_request);
        asset_cmd.site_kW_demand += asset_cmd.additional_load_compensation;

        asset_cmd.calculate_feature_kW_demand(0);
        t_log.float_results.push_back({ asset_cmd.feature_kW_demand, test.expected_feature_demand, "feature" });
        t_log.float_results.push_back({ asset_cmd.site_kW_demand, test.expected_feature_demand, "demand" });
        t_log.check_solution();
    }
}

// Verify production limits used for power dispatch based on final result of feature pipeline
TEST_F(site_manager_test, calculate_site_kW_production_limits) {
    int const num_tests = 42;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
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

    tests array[num_tests];  // an array with an element for each test case

    // configure variables for each test case
    array[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, 200.0f, 0.0f, 0.0f };                       // None (load present but disabled)
    array[1] = { 0.0f, 0.0f, -1200.0f, 500.0f, 700.0f, false, 200.0f, -1200.0f, 1200.0f };        // - ess + gen + solar
    array[2] = { -1000.0f, -1000.0f, -1000.0f, 0.0f, 0.0f, false, 200.0f, -1000.0f, 0.0f };       // - ess
    array[3] = { -800.0f, -800.0f, -2000.0f, 500.0f, 700.0f, false, 200.0f, -2000.0f, 1200.0f };  // - ess + gen + solar, net negative
    array[4] = { 1000.0f, 1000.0f, 1000.0f, 0.0f, 0.0f, false, 200.0f, 0.0f, 1000.0f };           // + ess
    array[5] = { 2200.0f, 2200.0f, 1000.0f, 500.0f, 700.0f, false, 200.0f, 0.0f, 2200.0f };       // + ess + gen + solar
    array[6] = { 700.0f, 700.0f, -500.0f, 500.0f, 700.0f, false, 200.0f, -500.0f, 1200.0f };      // - ess + gen + solar, net positive
    array[7] = { 0.0f, 0.0f, -200.0f, 0.0f, 0.0f, true, 200.0f, -200.0f, 200.0f };                // Same tests with load enabled
    array[8] = { 0.0f, 0.0f, -1400.0f, 500.0f, 700.0f, true, 200.0f, -1400.0f, 1400.0f };         // - ess + gen + solar
    array[9] = { -1000.0f, -1000.0f, -1200.0f, 0.0f, 0.0f, true, 200.0f, -1200.0f, 200.0f };      // - ess
    array[10] = { -800.0f, -800.0f, -2200.0f, 500.0f, 700.0f, true, 200.0f, -2200.0f, 1400.0f };  // - ess + gen + solar, net negative
    array[11] = { 1000.0f, 1000.0f, 800.0f, 0.0f, 0.0f, true, 200.0f, 0.0f, 1000.0f };            // + ess
    array[12] = { 2400.0f, 2400.0f, 1200.0f, 400.0f, 600.0f, true, 200.0f, 0.0f, 2400.0f };       // + ess + gen + solar
    array[13] = { 700.0f, 700.0f, -300.0f, 400.0f, 600.0f, true, 200.0f, -500.0f, 1200.0f };      // - ess + gen + solar, net positive
    array[14] = { 0.0f, -300.0f, 0.0f, 0.0f, 0.0f, false, 200.0f, -300.0f, 0.0f };                // Same test with negative standalone modification
    array[15] = { 0.0f, -300.0f, -1200.0f, 500.0f, 700.0f, false, 200.0f, -1500.0f, 1200.0f };
    array[16] = { -1000.0f, -1300.0f, -1000.0f, 0.0f, 0.0f, false, 200.0f, -1300.0f, 0.0f };
    array[17] = { -800.0f, -1100.0f, -2000.0f, 500.0f, 700.0f, false, 200.0f, -2300.0f, 1200.0f };
    array[18] = { 1000.0f, 700.0f, 1000.0f, 0.0f, 0.0f, false, 200.0f, 0.0f, 700.0f };
    array[19] = { 2200.0f, 1900.0f, 1000.0f, 500.0f, 700.0f, false, 200.0f, 0.0f, 1900.0f };
    array[20] = { 700.0f, 400.0f, -500.0f, 500.0f, 700.0f, false, 200.0f, -500.0f, 900.0f };
    array[21] = { 0.0f, -300.0f, -200.0f, 0.0f, 0.0f, true, 200.0f, -500.0f, 200.0f };
    array[22] = { 0.0f, -300.0f, -1200.0f, 500.0f, 700.0f, true, 200.0f, -1700.0f, 1400.0f };
    array[23] = { -1000.0f, -1300.0f, -1200.0f, 0.0f, 0.0f, true, 200.0f, -1500.0f, 200.0f };
    array[24] = { -800.0f, -1100.0f, -2000.0f, 500.0f, 700.0f, true, 200.0f, -2500.0f, 1400.0f };
    array[25] = { 1000.0f, 700.0f, 800.0f, 0.0f, 0.0f, true, 200.0f, 0.0f, 700.0f };
    array[26] = { 2200.0f, 1900.0f, 1000.0f, 400.0f, 600.0f, true, 200.0f, 0.0f, 1900.0f };
    array[27] = { 700.0f, 400.0f, -300.0f, 400.0f, 600.0f, true, 200.0f, -500.0f, 900.0f };
    array[28] = { 0.0f, 300.0f, 0.0f, 0.0f, 0.0f, false, 200.0f, 0.0f, 300.0f };  // Same tests with positive standalone modification
    array[29] = { 0.0f, 300.0f, -1200.0f, 500.0f, 700.0f, false, 200.0f, -1200.0f, 1500.0f };
    array[30] = { -1000.0f, -700.0f, -1000.0f, 0.0f, 0.0f, false, 200.0f, -700.0f, 0.0f };
    array[31] = { -800.0f, -500.0f, -2000.0f, 500.0f, 700.0f, false, 200.0f, -1700.0f, 1200.0f };
    array[32] = { 1000.0f, 1300.0f, 1000.0f, 0.0f, 0.0f, false, 200.0f, 0.0f, 1300.0f };
    array[33] = { 2200.0f, 2500.0f, 1000.0f, 500.0f, 700.0f, false, 200.0f, 0.0f, 2500.0f };
    array[34] = { 700.0f, 1000.0f, -500.0f, 500.0f, 700.0f, false, 200.0f, -500.0f, 1500.0f };
    array[35] = { 0.0f, 300.0f, -200.0f, 0.0f, 0.0f, true, 200.0f, -200.0f, 500.0f };
    array[36] = { 0.0f, 300.0f, -1200.0f, 500.0f, 700.0f, true, 200.0f, -1400.0f, 1700.0f };
    array[37] = { -1000.0f, -700.0f, -1200.0f, 0.0f, 0.0f, true, 200.0f, -900.0f, 200.0f };
    array[38] = { -800.0f, -500.0f, -2000.0f, 500.0f, 700.0f, true, 200.0f, -1900.0f, 1400.0f };
    array[39] = { 1200.0f, 1500.0f, 1000.0f, 0.0f, 0.0f, true, 200.0f, 0.0f, 1500.0f };
    array[40] = { 2400.0f, 2700.0f, 1200.0f, 400.0f, 600.0f, true, 200.0f, 0.0f, 2700.0f };
    array[41] = { 700.0f, 1000.0f, -300.0f, 400.0f, 600.0f, true, 200.0f, -500.0f, 1500.0f };

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        asset_cmd.feature_kW_demand = array[i].feature_demand;
        asset_cmd.site_kW_demand = array[i].site_demand;
        asset_cmd.ess_data.kW_request = array[i].ess_request;
        asset_cmd.gen_data.kW_request = array[i].gen_request;
        asset_cmd.solar_data.kW_request = array[i].solar_request;
        asset_cmd.set_load_compensation_method(LOAD_MINIMUM);
        asset_cmd.additional_load_compensation = array[i].load_inclusion * array[i].additional_load;
        asset_cmd_utils::site_kW_production_limits site_kW_prod_limits = asset_cmd_utils::calculate_site_kW_production_limits(asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request, asset_cmd.solar_data.kW_request, asset_cmd.load_method,
                                                                                                                              asset_cmd.additional_load_compensation, asset_cmd.feature_kW_demand, asset_cmd.site_kW_demand);
        asset_cmd.site_kW_charge_production = site_kW_prod_limits.site_kW_charge_production;
        asset_cmd.site_kW_discharge_production = site_kW_prod_limits.site_kW_discharge_production;
        errorLog << "calculate_site_kW_production_limits() test " << i + 1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.site_kW_charge_production != array[i].expected_charge_production || asset_cmd.site_kW_discharge_production != array[i].expected_discharge_production;
        EXPECT_EQ(asset_cmd.site_kW_charge_production, array[i].expected_charge_production);
        EXPECT_EQ(asset_cmd.site_kW_discharge_production, array[i].expected_discharge_production);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Determine if ESS must discharge to handle load
TEST_F(site_manager_test, determine_ess_load_requirement) {
    struct test {
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
        { NO_COMPENSATION, 0, 0.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f }, { LOAD_OFFSET, 0, 0.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f }, { LOAD_MINIMUM, 0, 0.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f },
        { LOAD_MINIMUM, 0, 150.0f, -100.0f, -100.0f, -100.0f, false, -100.0f, -100.0f },  { LOAD_MINIMUM, 0, 200.0f, -100.0f, 250.0f, 250.0f, false, 250.0f, 250.0f },  { LOAD_MINIMUM, 0, 201.0f, -100.0f, -100.0f, 100.0f, true, 0.0f, 200.0f },
        { LOAD_MINIMUM, 2, 101.0f, -100.0f, -100.0f, 100.0f, true, 0.0f, 200.0f },
    };
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("determine_ess_load_requirement", test_id++, tests.size());
        asset_cmd.set_load_compensation_method(test.load_method);
        asset_cmd.site_kW_load = test.load;
        asset_cmd.ess_data.kW_request = test.ess_request;
        asset_cmd.site_kW_demand = test.site_demand;
        asset_cmd.feature_kW_demand = test.feature_demand;
        asset_cmd.feeder_data.max_potential_kW = 100.0f;
        asset_cmd.ess_data.max_potential_kW = 100.0f;
        asset_cmd.gen_data.max_potential_kW = 100.0f;
        asset_cmd.solar_data.max_potential_kW = 100.0f;
        t_log.bool_results.push_back({ test.expected_result, asset_cmd.determine_ess_load_requirement(test.asset_priority), "result" });
        t_log.float_results.push_back({ test.expected_demand, asset_cmd.site_kW_demand, "demand" });
        t_log.float_results.push_back({ test.expected_feature, asset_cmd.feature_kW_demand, "feature" });
        if (test.expected_result)
            t_log.float_results.push_back({ 0.0f, asset_cmd.ess_data.kW_request, "request" });
        else
            t_log.float_results.push_back({ test.ess_request, asset_cmd.ess_data.kW_request, "request" });
        t_log.check_solution();
    }
}

// ess charge
TEST_F(site_manager_test, dispatch_site_kW_charge_cmd) {
    // struct that has variables to configure for each test case
    struct test_struct {
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
        float expected_charge_production;     // How much charge was produced by the feature
        float expected_discharge_production;  // Confirm that discharge production used to compensate is reduced
    };

    std::vector<test_struct> tests = {
        { -700.0f, 0.0f, 0.0f, 0.0f, 0, true, true, true, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                           // no request
        { -700.0f, 0.0f, 500.0f, 600.0f, 0, true, true, true, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 600.0f },                     // positive request
        { -700.0f, 100.0f, -500.0f, 600.0f, 0, true, true, true, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 600.0f },                // blocked by discharge
        { -700.0f, 0.0f, -500.0f, 600.0f, 0, false, false, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f },                 // no source, reduce discharge
        { -700.0f, 0.0f, -800.0f, 800.0f, 0, true, true, true, -700.0f, 0.0f, 0.0f, 700.0f, -700.0f, 100.0f },            // exceeds potential, limited
        { -700.0f, -500.0f, -500.0f, 600.0f, 0, true, true, true, -700.0f, 0.0f, 0.0f, 200.0f, -200.0f, 400.0f },         // exceeds potential - cmd, limited
        { -700.0f, 0.0f, -500.0f, 600.0f, 0, false, false, true, -500.0f, 0.0f, 0.0f, 500.0f, -500.0f, 100.0f },          // 1st source
        { -700.0f, 0.0f, -500.0f, 600.0f, 0, false, true, false, -500.0f, 0.0f, 500.0f, 0.0f, -500.0f, 100.0f },          // 1st disabled, 2nd source
        { -700.0f, 0.0f, -500.0f, 600.0f, 0, true, false, false, -500.0f, 500.0f, 0.0f, 0.0f, -500.0f, 100.0f },          // first two disabled, 3rd source
        { -1500.0f, 0.0f, -1500.0f, 1500.0f, 0, true, true, true, -1500.0f, 0.0f, 500.0f, 1000.0f, -1500.0f, 0.0f },      // request requires 2 sources
        { -3500.0f, 0.0f, -3500.0f, 3500.0f, 0, true, true, true, -3000.0f, 1000.0f, 1000.0f, 1000.0f, -3000.0f, 0.0f },  // request exceeds 3 sources, reduce discharge
        { -700.0f, 0.0f, 0.0f, 0.0f, 2, true, true, true, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                           // Same tests different priority
        { -700.0f, 0.0f, 500.0f, 600.0f, 2, true, true, true, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 600.0f },
        { -700.0f, 100.0f, -500.0f, 600.0f, 2, true, true, true, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 600.0f },
        { -700.0f, 0.0f, -500.0f, 600.0f, 2, false, false, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f },
        { -700.0f, 0.0f, -800.0f, 800.0f, 2, true, true, true, -700.0f, 0.0f, 700.0f, 0.0f, -700.0f, 100.0f },
        { -700.0f, -500.0f, -500.0f, 600.0f, 2, true, true, true, -700.0f, 0.0f, 200.0f, 0.0f, -200.0f, 400.0f },
        { -700.0f, 0.0f, -500.0f, 600.0f, 2, false, true, false, -500.0f, 0.0f, 500.0f, 0.0f, -500.0f, 100.0f },
        { -700.0f, 0.0f, -500.0f, 600.0f, 2, false, false, true, -500.0f, 0.0f, 0.0f, 500.0f, -500.0f, 100.0f },
        { -700.0f, 0.0f, -500.0f, 600.0f, 2, true, false, false, -500.0f, 500.0f, 0.0f, 0.0f, -500.0f, 100.0f },
        { -1500.0f, 0.0f, -1500.0f, 1500.0f, 2, true, true, true, -1500.0f, 0.0f, 1000.0f, 500.0f, -1500.0f, 0.0f },
        { -3500.0f, 0.0f, -3500.0f, 3500.0f, 2, true, true, true, -3000.0f, 1000.0f, 1000.0f, 1000.0f, -3000.0f, 0.0f },
    };

    // iterate through each test case and get results
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("dispatch_site_kW_charge_cmd", test_id++, tests.size());

        asset_cmd.ess_data.min_potential_kW = test.ess_min_potential_kW;
        asset_cmd.ess_data.kW_cmd = test.ess_kW_cmd;
        // Hard coded potentials/cmds (0) to simplify tests as there are already a significant number of cases
        asset_cmd.gen_data.max_potential_kW = 1000.0f;
        asset_cmd.gen_data.kW_cmd = 0.0f;
        asset_cmd.solar_data.max_potential_kW = 1000.0f;
        asset_cmd.solar_data.kW_cmd = 0.0f;
        asset_cmd.feeder_data.max_potential_kW = 1000.0f;
        asset_cmd.feeder_data.kW_cmd = 0.0f;
        asset_cmd.site_kW_charge_production = test.charge_production;
        asset_cmd.site_kW_discharge_production = test.discharge_production;
        float result = asset_cmd.dispatch_site_kW_charge_cmd(test.priority, test.solar_source, test.gen_source, test.feeder_source);

        t_log.float_results.push_back({ test.expected_ess_kW_cmd, asset_cmd.ess_data.kW_cmd, "ESS kW cmd" });
        t_log.float_results.push_back({ test.expected_feeder_kW_cmd, asset_cmd.feeder_data.kW_cmd, "Feeder kW cmd" });
        t_log.float_results.push_back({ test.expected_gen_kW_cmd, asset_cmd.gen_data.kW_cmd, "Gen kW cmd" });
        t_log.float_results.push_back({ test.expected_solar_kW_cmd, asset_cmd.solar_data.kW_cmd, "Solar kW cmd" });
        t_log.float_results.push_back({ test.expected_charge_production, result, "Charge production" });
        t_log.float_results.push_back({ test.expected_discharge_production, asset_cmd.site_kW_discharge_production, "Discharge production" });
        t_log.bool_results.push_back({ false, asset_cmd.feeder_data.kW_cmd > 0 && asset_cmd.ess_data.kW_cmd >= 0, "Check there is never feeder discharge if the power has no consumer" });
        t_log.check_solution();
    }
}

// Verify discharge dispatch used for load and additional site discharge
TEST_F(site_manager_test, dispatch_site_kW_discharge_cmd) {
    // struct that has variables to configure for each test case
    struct test_struct {
        float discharge_production;
        float cmd;
        int priority;
        discharge_type command_type;
        float ess_kW_request;
        float gen_kW_request;
        float solar_kW_request;
        float expected_dispatch;
        float expected_ess_kW_cmd;
        float expected_feeder_kW_cmd;
        float expected_gen_kW_cmd;
        float expected_solar_kW_cmd;
    };

    std::vector<test_struct> tests = {
        { 1000.0f, -500.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },           // negative cmd
        { 0.0f, 1000.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },              // 0 dispatch available
        { 800.0f, 800.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 800.0f, 0.0f, 0.0f, 0.0f, 800.0f },         // fully met by solar (start priority 0)
        { 1300.0f, 1300.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 1300.0f, 0.0f, 0.0f, 500.0f, 800.0f },    // met by solar + gen
        { 2100.0f, 2100.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // met by solar + gen + ess
        { 3000.0f, 3000.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met, feeder can't discharge
        { 3100.0f, 3100.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met (exceeds unused)
        { 3100.0f, 1600.0f, 0, DEMAND, 0.0f, 0.0f, 0.0f, 1600.0f, 100.0f, 0.0f, 700.0f, 800.0f },  // additional production is not met, only input cmd
        { 700.0f, 700.0f, 2, DEMAND, 0.0f, 0.0f, 0.0f, 700.0f, 0.0f, 0.0f, 700.0f, 0.0f },         // fully met by gen (start priority 2)
        { 1300.0f, 1300.0f, 2, DEMAND, 0.0f, 0.0f, 0.0f, 1300.0f, 600.0f, 0.0f, 700.0f, 0.0f },    // met by gen + ess
        { 2100.0f, 2100.0f, 2, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // met by gen + ess + solar
        { 3000.0f, 3000.0f, 2, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met, feeder can't discharge
        { 3100.0f, 3000.0f, 2, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met (exceeds unused)
        { 3000.0f, 1600.0f, 2, DEMAND, 0.0f, 0.0f, 0.0f, 1600.0f, 600.0f, 0.0f, 700.0f, 300.0f },  // additional production is not met, only cmd
        { 600.0f, 600.0f, 4, DEMAND, 0.0f, 0.0f, 0.0f, 600.0f, 600.0f, 0.0f, 0.0f, 0.0f },         // fully met by ess (new ess first priority)
        { 1400.0f, 1400.0f, 4, DEMAND, 0.0f, 0.0f, 0.0f, 1400.0f, 600.0f, 0.0f, 0.0f, 800.0f },    // met by ess + solar
        { 2100.0f, 2100.0f, 4, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // met by ess + solar + gen
        { 3000.0f, 3000.0f, 4, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met, feeder can't discharge
        { 3100.0f, 3000.0f, 4, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met (exceeds unused)
        { 3000.0f, 1600.0f, 4, DEMAND, 0.0f, 0.0f, 0.0f, 1600.0f, 600.0f, 0.0f, 200.0f, 800.0f },  // additional production is not met, only cmd
        { 800.0f, 800.0f, 1, LOAD, 0.0f, 0.0f, 0.0f, 800.0f, 0.0f, 0.0f, 0.0f, 800.0f },           // account for load with solar
        { 1700.0f, 1700.0f, 1, LOAD, 0.0f, 0.0f, 0.0f, 1700.0f, 0.0f, 900.0f, 0.0f, 800.0f },      // account for load with solar + feeder
        { 2300.0f, 2300.0f, 1, LOAD, 0.0f, 0.0f, 0.0f, 2300.0f, 600.0f, 900.0f, 0.0f, 800.0f },    // account for load with solar + feeder + ess
        { 3000.0f, 3000.0f, 1, LOAD, 0.0f, 0.0f, 0.0f, 3000.0f, 600.0f, 900.0f, 700.0f, 800.0f },  // account for load with solar + feeder + ess + gen
        { 3100.0f, 3100.0f, 1, LOAD, 0.0f, 0.0f, 0.0f, 3000.0f, 600.0f, 900.0f, 700.0f, 800.0f },  // load partially met
        { 800.0f, 800.0f, 1, DEMAND, 0.0f, 0.0f, 0.0f, 800.0f, 0.0f, 0.0f, 0.0f, 800.0f },         // fully met by solar (asset priority 1)
        { 1400.0f, 1400.0f, 1, DEMAND, 0.0f, 0.0f, 0.0f, 1400.0f, 600.0f, 0.0f, 0.0f, 800.0f },    // met by solar + ess (feeder skipped)
        { 2100.0f, 2100.0f, 1, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // met by solar + ess + gen (feeder skipped)
        { 2200.0f, 2200.0f, 1, DEMAND, 0.0f, 0.0f, 0.0f, 2100.0f, 600.0f, 0.0f, 700.0f, 800.0f },  // partially met, feeder can't discharge
    };

    // iterate through each test case and get results
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("dispatch_site_kW_discharge_cmd", test_id++, tests.size());

        asset_cmd.ess_data.max_potential_kW = 600.0f;
        asset_cmd.ess_data.kW_request = test.ess_kW_request;
        asset_cmd.ess_data.kW_cmd = 0.0f;
        asset_cmd.gen_data.max_potential_kW = 700.0f;
        asset_cmd.gen_data.kW_request = test.gen_kW_request;
        asset_cmd.gen_data.kW_cmd = 0.0f;
        asset_cmd.solar_data.max_potential_kW = 800.0f;
        asset_cmd.solar_data.kW_request = test.solar_kW_request;
        asset_cmd.solar_data.kW_cmd = 0.0f;
        asset_cmd.feeder_data.max_potential_kW = 900.0f;
        asset_cmd.feeder_data.kW_cmd = 0.0f;
        asset_cmd.site_kW_discharge_production = test.discharge_production;
        float actual_dispatch = asset_cmd.dispatch_site_kW_discharge_cmd(test.priority, test.cmd, test.command_type);
        // failure conditions
        t_log.range_results.push_back({ test.expected_dispatch, 0.001, actual_dispatch, "Dispatch" });
        t_log.range_results.push_back({ test.expected_ess_kW_cmd, 0.001, asset_cmd.ess_data.kW_cmd, "ESS kW cmd" });
        t_log.range_results.push_back({ test.expected_gen_kW_cmd, 0.001, asset_cmd.gen_data.kW_cmd, "Gen kW cmd" });
        t_log.range_results.push_back({ test.expected_solar_kW_cmd, 0.001, asset_cmd.solar_data.kW_cmd, "Solar kW cmd" });
        t_log.range_results.push_back({ test.expected_feeder_kW_cmd, 0.001, asset_cmd.feeder_data.kW_cmd, "Feeder kW cmd" });
        t_log.bool_results.push_back({ false, asset_cmd.feeder_data.kW_cmd > 0 && test.command_type != LOAD && asset_cmd.ess_data.kW_cmd >= 0, "Check there is never feeder discharge if the power has no consumer" });
        t_log.check_solution();
    }
}

// Target SoC Mode
// Simply ensure that the inputs are unmodified and solar request is set appropriately
TEST_F(site_manager_test, target_soc) {
    int const num_tests = 5;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        bool load_enable;
        float site_kW_load;
        float ess_kW_request;
        float solar_max_potential;
        bool expected_load_inclusion;
        float expected_ess_request;
        float expected_solar_request;
        float expected_site_kW_demand;
        float expected_additional_load_compensation;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    array[0] = { true, 0.0f, 2500.0f, 0.0f, true, 2500.0f, 0.0f, 0.0f, 0.0f };            // Ess discharge only
    array[1] = { false, 0.0f, -2500.0f, 0.0f, false, -2500.0f, 0.0f, 0.0f, 0.0f };        // Ess charge only
    array[2] = { true, 0.0f, -2500.0f, 2500.0f, true, -2500.0f, 2500.0f, 0.0f, 0.0f };    // Ess charge from solar potential, solar = ess
    array[3] = { false, 0.0f, -2500.0f, 1500.0f, false, -2500.0f, 1500.0f, 0.0f, 0.0f };  // Ess charge from solar potential, solar < ess
    array[4] = { false, 0.0f, -2500.0f, 5000.0f, false, -2500.0f, 5000.0f, 0.0f, 0.0f };  // Ess charge from solar potential, solar > ess
    array[5] = { true, 5000.0f, 2500.0f, 0.0f, true, 2500.0f, 0.0f, 2500.0f, 2500.0f };   // Check site_kW_demand is modified to account for load

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        asset_cmd.ess_data.kW_request = array[i].ess_kW_request;
        asset_cmd.solar_data.max_potential_kW = array[i].solar_max_potential;
        target_soc.load_enable_flag.value.value_bool = array[i].load_enable;
        target_soc.execute(asset_cmd);
        errorLog << "target_soc() test " << i + 1 << " of " << num_tests << std::endl;
        bool failure = asset_cmd.ess_data.kW_request != array[i].expected_ess_request || asset_cmd.solar_data.kW_request != array[i].expected_solar_request || asset_cmd.get_site_kW_load_inclusion() != array[i].expected_load_inclusion ||
                       asset_cmd.site_kW_demand != array[i].expected_site_kW_demand || asset_cmd.additional_load_compensation != array[i].expected_additional_load_compensation;
        EXPECT_EQ(asset_cmd.ess_data.kW_request, array[i].expected_ess_request);
        EXPECT_EQ(asset_cmd.solar_data.kW_request, array[i].expected_solar_request);
        EXPECT_EQ(asset_cmd.get_site_kW_load_inclusion(), array[i].expected_load_inclusion);
        EXPECT_EQ(asset_cmd.site_kW_demand, array[i].expected_site_kW_demand);
        EXPECT_EQ(asset_cmd.additional_load_compensation, array[i].expected_additional_load_compensation);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// active power setpoint #1 (formerly Site Export Target) mode
// Ensure that demand and load requirement are passed through as expected
TEST_F(site_manager_test, active_power_setpoint_1) {
    int const num_tests = 4;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        load_compensation load_strategy;
        float export_target_cmd;
        load_compensation expected_load_strategy;
        float expected_site_demand;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    array[0] = { NO_COMPENSATION, 2000.0f, NO_COMPENSATION, 2000.0f };
    array[1] = { NO_COMPENSATION, -2000.0f, NO_COMPENSATION, -2000.0f };
    array[2] = { LOAD_OFFSET, -2000.0f, LOAD_OFFSET, -2000.0f };
    array[3] = { LOAD_OFFSET, 2000.0f, LOAD_OFFSET, 2000.0f };

    // add potential to allow demand to inc/dec
    asset_cmd.ess_data.min_potential_kW = -2000;
    asset_cmd.gen_data.max_potential_kW = 2000;

    // increment export target cmd slew prior to test
    active_power_setpoint_mode.kW_slew.set_slew_rate(1000000);  // 1000MW/s
    active_power_setpoint_mode.kW_slew.update_slew_target(0);   // call once to set time vars
    usleep(10000);                                              // wait 10ms (total range +/-10MW)
    active_power_setpoint_mode.kW_slew.update_slew_target(0);   // call again to get delta time for non-zero slew range

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        usleep(10000);  // wait 10ms (total range +/-10MW)
        active_power_setpoint_mode.kW_cmd.value.value_float = array[i].export_target_cmd;
        active_power_setpoint_mode.load_method.value.value_int = array[i].load_strategy;
        active_power_setpoint_mode.absolute_mode_flag.value.value_bool = false;
        active_power_setpoint_mode.direction_flag.value.value_bool = false;
        active_power_setpoint_mode.maximize_solar_flag.value.value_bool = false;
        active_power_setpoint_mode.execute(asset_cmd);
        errorLog << "active_power_setpoint_1() test #" << i + 1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.site_kW_demand != array[i].expected_site_demand || asset_cmd.get_site_kW_load_inclusion() != array[i].expected_load_strategy;
        EXPECT_EQ(asset_cmd.site_kW_demand, array[i].expected_site_demand);
        EXPECT_EQ(asset_cmd.get_site_kW_load_inclusion(), array[i].expected_load_strategy);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// manual mode
// Ensure that asset requests are produced for each asset cmd and that no excess potential exists that could be used by another asset/load/feature
TEST_F(site_manager_test, manual_mode) {
    // struct that has variables to configure for each test case
    struct mm_test {
        float ess_cmd;
        float solar_cmd;
        float gen_cmd;
        int ess_slew_rate;
        int solar_slew_rate;
        int gen_slew_rate;
        float expected_ess_charge_kW_request;
        float expected_solar_kW_request;
        float expected_gen_kW_request;
        float expected_ess_max_potential;
        float expected_solar_max_potential;
        float expected_gen_max_potential;
    };

    std::vector<mm_test> tests = { { 0.0f, 0.0f, 0.0f, 100000, 100000, 100000, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
                                   { 500.0f, 0.0f, 0.0f, 100000, 100000, 100000, 500.0f, 0.0f, 0.0f, 500.0f, 0.0f, 0.0f },
                                   { 500.0f, 750.0f, 750.0f, 100000, 100000, 100000, 500.0f, 750.0f, 750.0f, 500.0f, 750.0f, 750.0f },
                                   { 500.0f, 750.0f, 750.0f, 100000, 100000, 100000, 500.0f, 750.0f, 750.0f, 500.0f, 750.0f, 750.0f },
                                   { -500.0f, 750.0f, 750.0f, 100000, 100000, 100000, -500.0f, 750.0f, 750.0f, 0.0f, 750.0f, 750.0f } };

    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("manual_mode", test_id++, tests.size());
        // Increment export target cmd slew prior to test
        manual_power_mode.manual_solar_kW_slew.set_slew_rate(test.solar_slew_rate);
        manual_power_mode.manual_ess_kW_slew.set_slew_rate(test.ess_slew_rate);
        manual_power_mode.manual_gen_kW_slew.set_slew_rate(test.gen_slew_rate);
        manual_power_mode.manual_solar_kW_slew.update_slew_target(0);  // call once for each asset to set time vars
        manual_power_mode.manual_ess_kW_slew.update_slew_target(0);
        manual_power_mode.manual_gen_kW_slew.update_slew_target(0);
        usleep(10000);                                                 // wait 10ms (total range +/-10MW)
        manual_power_mode.manual_solar_kW_slew.update_slew_target(0);  // call again to get delta time for non-zero slew range
        manual_power_mode.manual_ess_kW_slew.update_slew_target(0);
        manual_power_mode.manual_gen_kW_slew.update_slew_target(0);
        // Set potentials to some abritrary nonzero value to ensure they are modified appropriately
        asset_cmd.solar_data.max_potential_kW = 5000;
        asset_cmd.ess_data.max_potential_kW = 2500;
        asset_cmd.gen_data.max_potential_kW = 5000;
        manual_power_mode.manual_ess_kW_cmd.value.set(test.ess_cmd);
        manual_power_mode.manual_solar_kW_cmd.value.set(test.solar_cmd);
        manual_power_mode.manual_gen_kW_cmd.value.set(test.gen_cmd);
        manual_power_mode.execute(asset_cmd);
        t_log.float_results.push_back({ test.expected_ess_charge_kW_request, asset_cmd.ess_data.kW_request, "ESS kW Request" });
        t_log.float_results.push_back({ test.expected_solar_kW_request, asset_cmd.solar_data.kW_request, "Solar kW Request" });
        t_log.float_results.push_back({ test.expected_gen_kW_request, asset_cmd.gen_data.kW_request, "Generator kW Request" });
        t_log.float_results.push_back({ test.expected_ess_max_potential, asset_cmd.ess_data.max_potential_kW, "ESS Max Potential" });
        t_log.float_results.push_back({ test.expected_solar_max_potential, asset_cmd.solar_data.max_potential_kW, "Solar Max Potential" });
        t_log.float_results.push_back({ test.expected_gen_max_potential, asset_cmd.gen_data.max_potential_kW, "Generator Max Potential" });
        t_log.bool_results.push_back({ false, asset_cmd.get_site_kW_load_inclusion(), "Site kW Load Inclusion" });
        t_log.check_solution();
    }
}

// Active power setpoint #2 (formerly Grid Target) mode
// Ensure that only the exporting grid target is taken into account by the demand in the feature, and load handling is passed off
TEST_F(site_manager_test, active_power_setpoint_2) {
    int const num_tests = 8;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        float site_load;
        float kW_cmd;
        float expected_site_demand;
        float expected_solar_kW_request;
        bool absolute_mode;
        bool absolute_direction;
        bool prioritize_solar;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    array[0] = { 0.0f, 0.0f, 0.0f, 0.0f, false, false, false };
    array[1] = { 500.0f, 0.0f, 500.0f, 0.0f, false, false, false };
    array[2] = { 500.0f, -1000.0f, -500.0f, 0.0f, false, false, false };
    array[3] = { 500.0f, 1000.0f, 1500.0f, 0.0f, false, false, false };
    array[4] = { 500.0f, 1000.0f, 1500.0f, 0.0f, false, true, false };    // absolute_mode disabled - no impact on test
    array[5] = { 500.0f, 1000.0f, 1500.0f, 0.0f, true, false, false };    // direction_flag = false. kw_cmd interpreted identically
    array[6] = { 500.0f, 1000.0f, -500.0f, 0.0f, true, true, false };     // direction_flag = true. kw_cmd interpreted as -1000.0
    array[7] = { 500.0f, 1000.0f, 1500.0f, 300.0f, false, false, true };  // maximize_solar = true. potential hardwired to 300.0

    // add potential to allow demand to inc/dec
    asset_cmd.ess_data.min_potential_kW = -1500;
    asset_cmd.gen_data.max_potential_kW = 1500;

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "active_power_setpoint_2() test #" << i + 1 << " of " << num_tests << std::endl;
        active_power_setpoint_mode.kW_slew.set_slew_rate(std::pow(1000, 3));  // extremely high to simulate instant
        active_power_setpoint_mode.kW_slew.update_slew_target(array[i].kW_cmd);
        asset_cmd.site_kW_load = array[i].site_load;
        asset_cmd.solar_data.kW_request = 0.0;
        asset_cmd.solar_data.max_potential_kW = 300.0;
        active_power_setpoint_mode.kW_cmd.value.value_float = array[i].kW_cmd;
        active_power_setpoint_mode.load_method.value.value_int = LOAD_OFFSET;
        active_power_setpoint_mode.absolute_mode_flag.value.value_bool = array[i].absolute_mode;
        active_power_setpoint_mode.direction_flag.value.value_bool = array[i].absolute_direction;
        active_power_setpoint_mode.maximize_solar_flag.value.value_bool = array[i].prioritize_solar;
        active_power_setpoint_mode.execute(asset_cmd);
        bool failure = (asset_cmd.site_kW_demand != array[i].expected_site_demand || asset_cmd.get_site_kW_load_inclusion() != true ||  // always tracks load
                        asset_cmd.solar_data.kW_request != array[i].expected_solar_kW_request);
        EXPECT_EQ(asset_cmd.site_kW_demand, array[i].expected_site_demand);
        EXPECT_TRUE(asset_cmd.get_site_kW_load_inclusion());
        EXPECT_EQ(asset_cmd.solar_data.kW_request, array[i].expected_solar_kW_request);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Reactive power setpoint
// Ensure that reactive setpoint is followed and is limited by total potential reactive power
TEST_F(site_manager_test, reactive_setpoint) {
    // struct that has variables to configure for each test case
    struct test_struct {
        float reactive_setpoint_kVAR_cmd;
        float total_potential_kVAR;
        float expected_site_kVAR_demand;
    };

    std::vector<test_struct> tests = {
        { 0.0f, 1000.0f, 0.0f }, { 500.0f, 1000.0f, 500.0f }, { 1000.0f, 1000.0f, 1000.0f }, { 1500.0f, 1000.0f, 1000.0f }, { -500.0f, 1000.0f, -500.0f }, { -1000.0f, 1000.0f, -1000.0f }, { -1500.0f, 1000.0f, -1000.0f },
    };

    // iterate through each test case and get results
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("reactive_power_setpoint", test_id++, tests.size());
        reactive_setpoint.kVAR_cmd_slew.set_slew_rate(std::pow(10, 9));  // extremely high to simulate instant
        usleep(10);
        reactive_setpoint.kVAR_cmd_slew.update_slew_target(test.reactive_setpoint_kVAR_cmd);
        reactive_setpoint.kVAR_cmd.value.value_float = test.reactive_setpoint_kVAR_cmd;
        asset_cmd.total_potential_kVAR = test.total_potential_kVAR;
        reactive_setpoint.execute(asset_cmd, asset_pf_flag);

        t_log.float_results.push_back({ test.expected_site_kVAR_demand, asset_cmd.site_kVAR_demand, "site kVAR demand" });
        t_log.check_solution();
    }
}

// dispatch_reactive_power test
TEST_F(site_manager_test, dispatch_reactive_power) {
    int const num_tests = 6;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        float solar_potential_kVAR;
        float ess_potential_kVAR;
        float gen_potential_kVAR;
        float site_kVAR_demand;
        float result_solar_kVAR_cmd;
        float result_ess_kVAR_cmd;
        float result_gen_kVAR_cmd;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    array[0] = { 800, 400, 200, 1500, 800, 400, 200 };      // positive, not enough potential
    array[1] = { 800, 400, 200, 700, 400, 200, 100 };       // positive, enough potential
    array[2] = { 800, 400, 200, -1500, -800, -400, -200 };  // negative, not enough potential
    array[3] = { 800, 400, 200, -700, -400, -200, -100 };   // negative, enough potential
    array[4] = { 0, 0, 0, -700, 0, 0, 0 };                  // zero, not enough potential
    array[5] = { 800, 400, 200, 0, 0, 0, 0 };               // zero, enough potential

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        asset_cmd.solar_data.potential_kVAR = array[i].solar_potential_kVAR;
        asset_cmd.ess_data.potential_kVAR = array[i].ess_potential_kVAR;
        asset_cmd.gen_data.potential_kVAR = array[i].gen_potential_kVAR;
        asset_cmd.site_kVAR_demand = array[i].site_kVAR_demand;

        // reset cmds each iteration (will exit on 0 demand or potential)
        asset_cmd.solar_data.kVAR_cmd = 0;
        asset_cmd.ess_data.kVAR_cmd = 0;
        asset_cmd.gen_data.kVAR_cmd = 0;

        asset_cmd.calculate_total_potential_kVAR();
        asset_cmd.dispatch_reactive_power();
        errorLog << "dispatch_reactive_power() test " << i + 1 << " of " << num_tests << std::endl;
        // failure conditions
        failure = asset_cmd.solar_data.kVAR_cmd != array[i].result_solar_kVAR_cmd || asset_cmd.ess_data.kVAR_cmd != array[i].result_ess_kVAR_cmd || asset_cmd.gen_data.kVAR_cmd != array[i].result_gen_kVAR_cmd;
        EXPECT_EQ(asset_cmd.solar_data.kVAR_cmd, array[i].result_solar_kVAR_cmd);
        EXPECT_EQ(asset_cmd.ess_data.kVAR_cmd, array[i].result_ess_kVAR_cmd);
        EXPECT_EQ(asset_cmd.gen_data.kVAR_cmd, array[i].result_gen_kVAR_cmd);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// automatic_voltage_mode test
TEST_F(site_manager_test, automatic_voltage_mode) {
    // struct that has variables to configure for each test case
    struct avr_test {
        float solar_potential_kVAR;
        float ess_potential_kVAR;
        float gen_potential_kVAR;
        float actual_volts;
        float voltage_cmd;
        float voltage_cmd_max;
        float voltage_cmd_min;
        float over_deadband;
        float over_droop;
        float over_rated_kVAR;
        float under_deadband;
        float under_droop;
        float under_rated_kVAR;
        int slew_rate;
        float expected_site_kVAR_demand;
        float expected_solar_kVAR_cmd;
        float expected_ess_kVAR_cmd;
        float expected_gen_kVAR_cmd;
    };

    std::vector<avr_test> tests = {
        { 800, 400, 200, 400, 400, 425, 375, 50, 20, 1000, 100, 50, 500, 1000000, 0, 0, 0, 0 },              // zero
        { 800, 400, 200, 460, 400, 425, 375, 50, 20, 1400, 100, 50, 500, 1000000, -700, -400, -200, -100 },  // overvoltage no limit
        { 800, 400, 200, 500, 400, 425, 375, 50, 20, 700, 100, 50, 500, 1000000, -700, -400, -200, -100 },   // overvoltage limited by rated
        { 500, 400, 100, 460, 400, 425, 375, 50, 20, 1000, 100, 50, 500, 1000000, -500, -250, -200, -50 },   // overvoltage limited by potential
        { 500, 400, 100, 460, 400, 425, 375, 50, 20, 1000, 100, 50, 500, 10000, -100, -50, -40, -10 },       // overvoltage limited by slew rate (100kVAR/us)
        { 800, 400, 200, 340, 400, 425, 375, 100, 50, 500, 50, 20, 1400, 1000000, 700, 400, 200, 100 },      // undervoltage no limit
        { 800, 400, 200, 300, 400, 425, 375, 100, 50, 500, 50, 20, 700, 1000000, 700, 400, 200, 100 },       // undervoltage limited by rated
        { 500, 400, 100, 340, 400, 425, 375, 100, 50, 500, 50, 20, 1000, 1000000, 500, 250, 200, 50 },       // undervoltage limited by potential
        { 500, 400, 100, 340, 400, 425, 375, 100, 50, 500, 50, 20, 1000, 10000, 100, 50, 40, 10 }            // undervoltage limited by slew rate (100kVAR/us)
    };

    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("avr", test_id++, tests.size());
        // reset cmds each iteration (will exit on 0 demand or potential)
        asset_cmd.solar_data.kVAR_cmd = 0;
        asset_cmd.ess_data.kVAR_cmd = 0;
        asset_cmd.gen_data.kVAR_cmd = 0;
        // set potential kVAR for each asset
        asset_cmd.solar_data.potential_kVAR = test.solar_potential_kVAR;
        asset_cmd.ess_data.potential_kVAR = test.ess_potential_kVAR;
        asset_cmd.gen_data.potential_kVAR = test.gen_potential_kVAR;
        asset_cmd.calculate_total_potential_kVAR();
        // set avr variables
        // Setup one iteration of time for slew object, resetting back to zero for each test
        avr.kVAR_slew.update_slew_target(0);
        usleep(10000);
        avr.kVAR_slew.set_slew_rate(1000000);
        avr.kVAR_slew.update_slew_target(0);
        usleep(10000);
        avr.kVAR_slew_rate.value.set(test.slew_rate);
        avr.kVAR_slew.set_slew_rate(avr.kVAR_slew_rate.value.value_int);
        avr.kVAR_slew.update_slew_target(0);
        avr.symmetric_variables = false;
        avr.over_deadband.value.value_float = test.over_deadband;
        avr.over_droop.value.value_float = test.over_droop;
        avr.over_rated_kVAR.value.value_float = test.over_rated_kVAR;
        avr.under_deadband.value.value_float = test.under_deadband;
        avr.under_droop.value.value_float = test.under_droop;
        avr.under_rated_kVAR.value.value_float = test.under_rated_kVAR;
        avr.voltage_cmd_max.value.value_float = test.voltage_cmd_max;
        avr.voltage_cmd_min.value.value_float = test.voltage_cmd_min;
        avr.voltage_cmd.value.value_float = test.voltage_cmd;
        avr.actual_volts.value.value_float = test.actual_volts;
        avr.execute(asset_cmd, asset_pf_flag);
        asset_cmd.dispatch_reactive_power();
        t_log.range_results.push_back({ test.expected_site_kVAR_demand, 0.03f, asset_cmd.site_kVAR_demand, "Site kVAR Demand" });
        t_log.range_results.push_back({ test.expected_solar_kVAR_cmd, 0.03f, asset_cmd.solar_data.kVAR_cmd, "Solar kVAR Command" });
        t_log.range_results.push_back({ test.expected_ess_kVAR_cmd, 0.03f, asset_cmd.ess_data.kVAR_cmd, "ESS kVAR Command" });
        t_log.range_results.push_back({ test.expected_gen_kVAR_cmd, 0.03f, asset_cmd.gen_data.kVAR_cmd, "Generator kVAR Command" });
        t_log.check_solution();
    }
}

// watt-var test - curve 1 (8 points)
TEST_F(site_manager_test, watt_var_mode_curve_1) {
    float points_array[] = { -500, 200, 0, 200, 1000, -500, 1500, 500 };
    // struct that has variables to configure for each test case
    struct test_struct {
        float actual_kW;
        float result_site_kVAR_demand;
    };

    std::vector<test_struct> tests = {
        { -800, 200 },  // low power
        { 800, -360 },  // mid power
        { 1600, 700 },  // high power
    };

    // iterate through each test case and get results
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("watt_var_mode curve 1", test_id++, tests.size());

        int array_size = sizeof(points_array) / 4;
        std::vector<Value_Object> options_value(array_size);

        for (int j = 0; j < array_size; j++) {
            options_value[j].set(points_array[j]);
        }

        watt_var.watt_var_points.options_value = options_value;
        watt_var.watt_var_points.num_options = array_size;
        watt_var.init_curve();

        for (int j = 0; j < array_size; j += 2) {
            // test set_curve_points plots correctly
            t_log.float_results.push_back({ options_value[j].value_float, watt_var.watt_var_curve[j / 2].first, "curve point " + std::to_string(j / 2) + " watt component" });
            t_log.float_results.push_back({ options_value[j + 1].value_float, watt_var.watt_var_curve[j / 2].second, "curve point " + std::to_string(j / 2) + " var component" });
        }

        // test watt_var_mode provides correct site kVAR demand output
        watt_var.execute(asset_cmd, test.actual_kW, asset_pf_flag);

        t_log.float_results.push_back({ test.result_site_kVAR_demand, asset_cmd.site_kVAR_demand, "site kVAR demand" });
        t_log.check_solution();
    }
}

// watt-var test - curve 2 (6 points)
TEST_F(site_manager_test, watt_var_mode_curve_2) {
    float points_array[] = { -500, 500, 0, 200, 1000, 500 };
    // struct that has variables to configure for each test case
    struct test_struct {
        float actual_kW;
        float result_site_kVAR_demand;
    };

    std::vector<test_struct> tests = {
        { -800, 680 },  // low power
        { 800, 440 },   // mid power
        { 1500, 650 },  // high power
    };

    // iterate through each test case and get results
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("watt_var_mode curve 2", test_id++, tests.size());

        int array_size = sizeof(points_array) / 4;
        std::vector<Value_Object> options_value(array_size);

        for (int j = 0; j < array_size; j++) {
            options_value[j].set(points_array[j]);
        }

        watt_var.watt_var_points.options_value = options_value;
        watt_var.watt_var_points.num_options = array_size;
        watt_var.init_curve();
        for (int j = 0; j < array_size; j += 2) {
            // test set_curve_points plots correctly
            t_log.float_results.push_back({ options_value[j].value_float, watt_var.watt_var_curve[j / 2].first, "curve point " + std::to_string(j / 2) + " watt component" });
            t_log.float_results.push_back({ options_value[j + 1].value_float, watt_var.watt_var_curve[j / 2].second, "curve point " + std::to_string(j / 2) + " var component" });
        }

        // test watt_var_mode provides correct site kVAR demand output
        watt_var.execute(asset_cmd, test.actual_kW, asset_pf_flag);

        t_log.float_results.push_back({ test.result_site_kVAR_demand, asset_cmd.site_kVAR_demand, "site kVAR demand" });
        t_log.check_solution();
    }
}

// watt-watt test
TEST_F(site_manager_test, watt_watt_mode) {
    int const num_tests = 3;  // total number of test cases
    float points_array[] = { -500, 200, 0, 200, 1000, -500, 1500, 500 };
    // struct that has variables to configure for each test case
    struct tests {
        float actual_kW;
        float result_site_kW_demand;
    };

    tests array[num_tests];  // an array with an element for each test case
    // configure variables each test case
    array[0] = { -800, 200 };  // low power
    array[1] = { 800, -360 };  // mid power
    array[2] = { 1600, 700 };  // high power

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        int array_size = sizeof(points_array) / 4;
        std::vector<Value_Object> options_value(array_size);

        for (int j = 0; j < array_size; j++) {
            options_value[j].set(points_array[j]);
        }

        watt_watt.watt_watt_points.options_value = options_value;
        watt_watt.watt_watt_points.num_options = array_size;
        watt_watt.init_curve();
        errorLog << "watt_watt_mode() test " << i + 1 << " of " << num_tests << std::endl;
        for (int j = 0; j < array_size; j += 2) {
            // failure conditions
            failure = watt_watt.watt_watt_curve[j / 2].first != options_value[j].value_float || watt_watt.watt_watt_curve[j / 2].second != options_value[j + 1].value_float;
            // test set_curve_points plots correctly
            EXPECT_EQ(watt_watt.watt_watt_curve[j / 2].first, options_value[j].value_float);
            EXPECT_EQ(watt_watt.watt_watt_curve[j / 2].second, options_value[j + 1].value_float);
        }

        // test watt_watt_mode provides correct site kW demand output
        asset_cmd.site_kW_demand = array[i].actual_kW;
        watt_watt.execute(asset_cmd);
        failure = failure || asset_cmd.site_kW_demand != array[i].result_site_kW_demand;
        EXPECT_EQ(asset_cmd.site_kW_demand, array[i].result_site_kW_demand);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// set curve points
TEST_F(site_manager_test, set_curve_points) {
    int const num_tests = 3;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        std::vector<std::pair<float, float>> points;
    };

    // Make curves to check output of set_curve_points
    std::vector<std::pair<float, float>> curve0;
    curve0.push_back(std::make_pair(59, 0));
    curve0.push_back(std::make_pair(60, 0));
    curve0.push_back(std::make_pair(61, 0));
    std::vector<std::pair<float, float>> curve1;
    curve1.push_back(std::make_pair(-1, -1.1));
    curve1.push_back(std::make_pair(0, 0));
    curve1.push_back(std::make_pair(1, 1.1));
    std::vector<std::pair<float, float>> curve2;
    curve2.push_back(std::make_pair(-1, 1.1));
    curve2.push_back(std::make_pair(0, 0));
    curve2.push_back(std::make_pair(1, -1.1));

    tests array[num_tests];  // an array with an element for each test case
    // configure variables each test case
    array[0] = { curve0 };
    array[1] = { curve1 };
    array[2] = { curve2 };

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        int array_size = 2 * array[i].points.size();
        std::vector<Value_Object> options_value(array_size);
        Fims_Object fims_object;
        std::vector<std::pair<float, float>> output_curve;

        for (int j = 0; j < array_size; j += 2) {
            options_value[j].set(array[i].points[j / 2].first);
            options_value[j + 1].set(array[i].points[j / 2].second);
        }
        fims_object.options_value = options_value;
        fims_object.num_options = array_size;

        errorLog << "set_curve_points() test " << i + 1 << " of " << num_tests << std::endl;
        set_curve_points(&fims_object, output_curve);
        // Release stdout so we can write again
        release_stdout(failure);
        for (int j = 0; j < (int)array[i].points.size(); j++) {
            // failure conditions
            failure = array[i].points[j].first != output_curve[j].first || array[i].points[j].second != output_curve[j].second;
            // test set_curve_points plots correctly
            EXPECT_EQ(array[i].points[j].first, output_curve[j].first);
            EXPECT_EQ(array[i].points[j].second, output_curve[j].second);
            // Print the test id if failure
            if (failure)
                std::cout << errorLog.str() << std::endl;
        }
    }
}

// get curve cmd
TEST_F(site_manager_test, get_curve_cmd) {
    int const num_tests = 6;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        std::vector<std::pair<float, float>> points;
        float curve_input;
        float curve_output;
    };

    // Make curves to check output of set_curve_points
    std::vector<std::pair<float, float>> curve0;
    curve0.push_back(std::make_pair(59, 0));
    curve0.push_back(std::make_pair(60, 0));
    curve0.push_back(std::make_pair(61, 0));
    std::vector<std::pair<float, float>> curve1;
    curve1.push_back(std::make_pair(-1, -1.1));
    curve1.push_back(std::make_pair(0, 0));
    curve1.push_back(std::make_pair(1, 1.1));
    std::vector<std::pair<float, float>> curve2;
    curve2.push_back(std::make_pair(-1, 1.1));
    curve2.push_back(std::make_pair(0, 0));
    curve2.push_back(std::make_pair(1, -1.1));

    tests array[num_tests];  // an array with an element for each test case
    // configure variables each test case
    array[0] = { curve0, 100, 0 };
    array[1] = { curve0, -100, 0 };
    array[2] = { curve1, -200, -220 };
    array[3] = { curve1, 100, 110 };
    array[4] = { curve2, -100, 110 };
    array[5] = { curve2, 200, -220 };

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "get_curve_cmd() test " << i + 1 << " of " << num_tests << std::endl;
        float result = get_curve_cmd(array[i].curve_input, array[i].points);

        // failure conditions
        failure = !near(result, array[i].curve_output, 0.001);
        // test get_curve_cmd calculates correctly
        EXPECT_NEAR(result, array[i].curve_output, 0.001);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Aggregated Asset Limit standalone power feature
TEST_F(site_manager_test, apply_aggregated_asset_limit) {
    int const num_tests = 6;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        float uncontrolled_solar_kw;
        float controlled_solar_kw;
        float uncontrolled_ess_kW;
        float current_controlled_ess_max;
        float agg_asset_limit_kw;
        float total_asset_kW_discharge_limit;
        float ess_total_kW_discharge_limit;
        float expected_ess_max_potential_kw;
        float expected_total_asset_kW_discharge_limit;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    array[0] = { 500.0f, 0.0f, 0.0f, 500.0f, 750.0f, 500.0f, 500.0f, 250, -250.0f };   // discharging is beyond limit, solar alone is not
    array[1] = { 600.0f, 0.0f, 0.0f, 300.0f, 1000.0f, 300.0f, 300.0f, 300, 300.0f };   // discharging is not beyond limit
    array[2] = { 1000.0f, 0.0f, 0.0f, 100.0f, 1100.0f, 100.0f, 100.0f, 100, 100.0f };  // discharging is exactly at limit
    array[3] = { 500.0f, 0.0f, 0.0f, -100.0f, 250.0f, 0.0f, 0.0f, -100, 0.0f };        // discharging is beyond limit, solar alone is beyond limit
    array[4] = { 1000.0f, 0.0f, 0.0f, -300.0f, 700.0f, 0.0f, 0.0f, -300, 0.0f };       // ess is charging, solar alone is beyond limit
    array[5] = { 750.0f, 0.0f, 0.0f, -250.0f, 1000.0f, 0.0f, 0.0f, -250, 0.0f };       // ess is charging, solar alone is not beyond limit

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "apply_aggregated_asset_limit() test " << i + 1 << " of " << num_tests << std::endl;
        asset_cmd.solar_data.actual_kW = array[i].controlled_solar_kw;
        asset_cmd.ess_data.max_potential_kW = array[i].current_controlled_ess_max;
        agg_asset_limit.agg_asset_limit_kw.value.set(array[i].agg_asset_limit_kw);
        total_asset_kW_discharge_limit = array[i].total_asset_kW_discharge_limit;
        agg_asset_limit.execute(asset_cmd, array[i].uncontrolled_ess_kW, array[i].uncontrolled_solar_kw, max_potential_ess_kW.value.value_float, min_potential_ess_kW.value.value_float, total_asset_kW_discharge_limit,
                                array[i].ess_total_kW_discharge_limit);
        bool failure = asset_cmd.ess_data.max_potential_kW != array[i].expected_ess_max_potential_kw || total_asset_kW_discharge_limit != array[i].expected_total_asset_kW_discharge_limit;
        EXPECT_EQ(asset_cmd.ess_data.max_potential_kW, array[i].expected_ess_max_potential_kw);
        EXPECT_EQ(total_asset_kW_discharge_limit, array[i].expected_total_asset_kW_discharge_limit);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// POI limits
TEST_F(site_manager_test, apply_active_power_poi_limits) {
    // struct that has variables to configure for each test case
    struct tests {
        bool load_enable;
        load_compensation load_inclusion;
        float feed_max;
        float asset_maxes;
        bool feed_only;
        float site_load;
        float site_demand;
        float min_poi_limit_kW;
        float max_poi_limit_kW;
        int priority;  // Asset priority. Other assets can cover import removing the responsibility from the POI
        float expected_feed_max;
        float expected_site_demand;
    };

    std::vector<tests> tests = {
        // Ensure that load and uncontrollable properly limit the site demand based on import/export, but do not influence the feed max
        { false, NO_COMPENSATION, 600.0f, 200.0f, true, 0.0f, 390.0f, -485, 485, 0, 485, 390 },     // discharging, no limit
        { false, NO_COMPENSATION, 600.0f, 200.0f, true, 0.0f, -390.0f, -485, 485, 0, 485, -390 },   // charging, no limit
        { false, NO_COMPENSATION, 600.0f, 200.0f, true, 0.0f, 490.0f, -485, 485, 0, 485, 485 },     // discharging, limited
        { false, NO_COMPENSATION, 600.0f, 200.0f, true, 0.0f, -490.0f, -485, 485, 0, 485, -485 },   // charging, limited
        { false, NO_COMPENSATION, 600.0f, 200.0f, false, 200.0f, 490.0f, -485, 485, 0, 485, 485 },  // Load present but disabled, no limit modification
        { false, LOAD_OFFSET, 600.0f, 200.0f, false, 0.0f, 490.0f, -485, 485, 0, 485, 485 },        // Feature load enabled, but no load
        { true, LOAD_OFFSET, 600.0f, 200.0f, false, 200.0f, 490.0f, -485, 485, 0, 485, 490 },       // Load importing, demand exporting, gives extra headroom
        { true, LOAD_OFFSET, 600.0f, 100.0f, false, 200.0f, -190.0f, -485, 485, 0, 485, -190 },     // Load importing, demand importing, other assets satisfy load, no limit
        { true, LOAD_OFFSET, 600.0f, 200.0f, false, -200.0f, 390.0f, -485, 485, 0, 485, 285 },      // Load exporting, demand exporting, limited
        { true, LOAD_OFFSET, 600.0f, 200.0f, false, -200.0f, -490.0f, -485, 485, 0, 485, -490 },    // Load exporting, demand importing, gives extra headroom
        { true, LOAD_OFFSET, 600.0f, 200.0f, false, 200.0f, 490.0f, -485, 485, 1, 485, 490 },       // Load importing, demand exporting with different asset priority
        // (commented until poi limits handles all cases to satisfaction; the listed expected emand may be incorrect)
        // { true, LOAD_OFFSET, 600.0f, 200.0f, false, 300.0f, -490.0f, -485, 485, 1, 485, -475 },     // Load importing, demand importing, some load on feeder -> limited
        { true, LOAD_MINIMUM, 200.0f, 200.0f, false, 500.0f, 0.0f, -200, 200, 1, 200, 500 },  // Load importing, demand wants to import but ESS must help with load
    };

    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("apply_active_power_poi_limits", test_id++, tests.size());
        // Set independent variables
        asset_cmd.set_load_compensation_method(test.load_inclusion);
        asset_cmd.site_kW_load = test.site_load;
        asset_cmd.additional_load_compensation = asset_cmd.get_site_kW_load_inclusion() * test.site_load;
        asset_cmd.feeder_data.max_potential_kW = test.feed_max;
        asset_cmd.ess_data.max_potential_kW = test.asset_maxes;  // Other asset potentials used to determine how much of the load will
        asset_cmd.gen_data.max_potential_kW = test.asset_maxes;  // be satisfied by other assets, therefore not falling on the POI
        asset_cmd.ess_data.min_potential_kW = -1.0f * test.feed_max;
        asset_cmd.solar_data.max_potential_kW = test.asset_maxes;
        charge_dispatch.feeder_enable_flag.value.set(true);
        charge_dispatch.gen_enable_flag.value.set(!test.feed_only);
        charge_dispatch.solar_enable_flag.value.set(!test.feed_only);
        asset_cmd.site_kW_demand = test.site_demand;
        asset_cmd.feature_kW_demand = test.site_demand;

        // Apply limits
        asset_priority_runmode1.value.set(test.priority);
        active_power_poi_limits.min_kW.value.set(test.min_poi_limit_kW);
        active_power_poi_limits.max_kW.value.set(test.max_poi_limit_kW);
        active_power_poi_limits.execute(asset_cmd, soc_avg_running.value.value_float, asset_priority_runmode1.value.value_int, total_site_kW_charge_limit.value.value_float, total_site_kW_discharge_limit.value.value_float);

        // Check results
        t_log.float_results.push_back({ test.expected_site_demand, asset_cmd.site_kW_demand, "demand" });
        t_log.float_results.push_back({ test.expected_feed_max, asset_cmd.feeder_data.max_potential_kW, "feed_max" });
        t_log.check_solution();
    }
}

// soc-based POI limits
TEST_F(site_manager_test, apply_poi_limits_soc) {
    int const num_tests = 8;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
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

    tests array[num_tests];  // an array with an element for each test case

    array[0] = { true, 49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, 5000.0f, 5000.0f, 150.0f, 250.0f };       // soc below, discharging
    array[1] = { true, 49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -250.0f, 250.0f };     // soc below, charging
    array[2] = { false, 49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, 5000.0f, 5000.0f, 5000.0f, 5000.0f };    // soc below, but disabled, no effect
    array[3] = { false, 49.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -5000.0f, 5000.0f };  // soc below, but disabled, no effect
    array[4] = { true, 50.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, 5000.0f, 5000.0f, 150.0f, 250.0f };       // soc equal, discharging
    array[5] = { true, 50.0f, 50.0f, -250.0f, 150.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -250.0f, 250.0f };     // soc equal, charging
    array[6] = { true, 51.0f, 50.0f, -250.0f, 250.0f, -1000.0f, 750.0f, 5000.0f, 5000.0f, 750.0f, 1000.0f };      // soc above, discharging
    array[7] = { true, 51.0f, 50.0f, -250.0f, 250.0f, -1000.0f, 750.0f, -5000.0f, 5000.0f, -1000.0f, 1000.0f };   // soc above, charging

    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "apply_active_power_poi_limits() SoC test " << i + 1 << " of " << num_tests << std::endl;

        // Set independent variables
        asset_cmd.set_load_compensation_method(NO_COMPENSATION);
        asset_cmd.ess_data.max_potential_kW = 5000.0f;
        asset_cmd.ess_data.min_potential_kW = -5000.0f;
        asset_cmd.site_kW_demand = array[i].site_demand;
        asset_cmd.feature_kW_demand = array[i].site_demand;
        asset_cmd.feeder_data.max_potential_kW = array[i].feed_potential;

        // Apply limits
        active_power_poi_limits.min_kW.value.set(-10000.0f);
        active_power_poi_limits.max_kW.value.set(10000.0f);
        active_power_poi_limits.soc_poi_limits_enable.value.set(array[i].soc_limits_enabled);
        soc_avg_running.value.set(array[i].avg_soc);
        active_power_poi_limits.target_soc.value.set(array[i].soc_target);
        active_power_poi_limits.soc_low_min_kW.value.set(array[i].under_min);
        active_power_poi_limits.soc_low_max_kW.value.set(array[i].under_max);
        active_power_poi_limits.soc_high_min_kW.value.set(array[i].over_min);
        active_power_poi_limits.soc_high_max_kW.value.set(array[i].over_max);
        charge_dispatch.feeder_enable_flag.value.set(true);
        active_power_poi_limits.execute(asset_cmd, soc_avg_running.value.value_float, asset_priority_runmode1.value.value_int, total_site_kW_charge_limit.value.value_float, total_site_kW_discharge_limit.value.value_float);

        // failure conditions
        failure = array[i].expected_site_demand != asset_cmd.site_kW_demand || array[i].expected_feed_potential != asset_cmd.feeder_data.max_potential_kW;
        // Analyze expected
        EXPECT_EQ(array[i].expected_site_demand, asset_cmd.site_kW_demand);
        EXPECT_EQ(array[i].expected_feed_potential, asset_cmd.feeder_data.max_potential_kW);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// POI limits
TEST_F(site_manager_test, reactive_poi_limits) {
    int const num_tests = 4;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        float site_demand;
        float min_poi_limit_kVAR;
        float max_poi_limit_kVAR;
        float expected_demand;
    };

    tests array[num_tests];  // an array with an element for each test case
    // Ensure that load and uncontrollable properly limit the site demand based on import/export, but do not influence the feed max
    array[0] = { 5000, -10000, 10000, 5000 };      // discharging, no limit
    array[1] = { -5000, -10000, 10000, -5000 };    // charging, no limit
    array[2] = { 15000, -10000, 10000, 10000 };    // discharging, above limit
    array[3] = { -15000, -10000, 10000, -10000 };  // charging, above limit

    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "apply_reactive_power_poi_limits() test " << i + 1 << " of " << num_tests << std::endl;

        errorLog << "reactive_power_poi_limits() test " << i + 1 << " of " << num_tests << std::endl;

        // Set independent variables
        asset_cmd.site_kVAR_demand = array[i].site_demand;

        // Apply limits
        reactive_power_poi_limits.min_kVAR.value.set(array[i].min_poi_limit_kVAR);
        reactive_power_poi_limits.max_kVAR.value.set(array[i].max_poi_limit_kVAR);
        reactive_power_poi_limits.execute(asset_cmd);

        // failure conditions
        failure = asset_cmd.site_kVAR_demand != array[i].expected_demand;
        // Analyze expected
        EXPECT_EQ(asset_cmd.site_kVAR_demand, array[i].expected_demand);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// check_expired_time
TEST_F(site_manager_test, check_expired_time) {
    int const num_tests = 4;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
        timespec current_time;
        timespec target_time;
        bool result;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables for each test case
    array[0] = { { 0, 0 }, { 5, 0 }, false };       // Current time before target time
    array[1] = { { 5, 100 }, { 5, 0 }, false };     // Current time after target time but before buffer
    array[2] = { { 6, 0 }, { 5, 0 }, true };        // Current time after target time and buffer
    array[3] = { { 6, 2000000 }, { 5, 0 }, true };  // Current time after target time and buffer

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "check_expired_time() test " << i + 1 << " of " + num_tests << std::endl;
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
TEST_F(site_manager_test, bit_positions) {
    int const num_tests = 9;  // total number of test cases
    int result_integer = 0;

    // struct that has variables to configure for each test case
    struct tests {
        int bit_position;
        int expected_integer;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    //         bit_pos  exp_int
    array[0] = { 0, 1 };   //
    array[1] = { 1, 2 };   //
    array[2] = { 2, 4 };   //
    array[3] = { 3, 8 };   //
    array[4] = { 4, 16 };  //
    array[5] = { 5, 32 };  //
    array[6] = { 6, 64 };  //
    array[7] = { -1, 0 };  // invalid case
    array[8] = { -2, 0 };  // invalid case

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        result_integer = get_int_from_bit_position(array[i].bit_position);

        errorLog << "bit position functions test " << i + 1 << " of " << num_tests << std::endl;
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

// set fims masked int
TEST_F(site_manager_test, set_fims_masked_int) {
    int const num_tests = 10;  // total number of test cases

    Fims_Object* features_mode_cmd = new Fims_Object();  // fims variable for test
    const char* var_id = "features_mode_cmd";            // set var_id for matching
    features_mode_cmd->set_variable_id(var_id);          // set var_id for matching

    // struct that has variables to configure for each test case
    struct tests {
        std::string mask;
        int requested_mode;
        int initial_mode;
        int expected_mode;
    };

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    // mask    request   init  expected
    array[0] = { "0x01", 0, 0, 0 };   // base case, no change requested
    array[1] = { "0x41", 0, 6, 0 };   // request valid mode 0
    array[2] = { "0x42", 1, 6, 1 };   // request valid mode 1
    array[3] = { "0x44", 2, 6, 2 };   // request valid mode 2
    array[4] = { "0x48", 3, 6, 3 };   // request valid mode 3
    array[5] = { "0x50", 4, 6, 4 };   // request valid mode 4
    array[6] = { "0x02", 0, 1, 1 };   // request invalid mode low
    array[7] = { "0x02", 2, 1, 1 };   // request invalid mode high
    array[8] = { "0x3F", 6, 2, 2 };   // request invalid mode out of bounds
    array[9] = { "0x04", -2, 2, 2 };  // request invalid mode negative

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        features_mode_cmd->set_fims_int(var_id, array[i].initial_mode);
        features_mode_cmd->set_fims_masked_int(var_id, array[i].requested_mode, (uint64_t)std::stoul(array[i].mask, NULL, 16));
        errorLog << "set_fims_masked_int() test " << i + 1 << " of " << num_tests << std::endl;
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

TEST_F(site_manager_test, energy_arbitrage) {
    int const num_tests = 13;  // total number of test cases

    // struct that has variables to configure for each test case
    struct tests {
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

    tests array[num_tests];  // an array with an element for each test case

    // configure variables each test case
    array[0] = { 3000, -10, 11, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 0 };           // Invalid thresholds: t1 > t2
    array[1] = { 3000, -10, -10, 31, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 0 };          // Invalid thresholds: t2 > t3
    array[2] = { 3000, -10, -10, 10, 101, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 0 };         // Invalid thresholds: t3 > t4
    array[3] = { 3000, -10, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, -2000, 0 };      // default charge2
    array[4] = { 3000, 0, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, -1000, 0 };        // default charge1
    array[5] = { 3000, 1, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, -1000, 3000 };     // charge1, solar discharge
    array[6] = { 3000, 15, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 0, 3000 };        // default 0
    array[7] = { 3000, 30, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 1000, 3000 };     // default discharge1
    array[8] = { 3000, 100, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 100, 2000, 3000 };    // default discharge2
    array[9] = { 3000, -10, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 90, 0, 90, -2000, 0 };       // soc limited charge2, soc >= max but thresholds N/A due to price
    array[10] = { 3000, 0, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 0, 49, 0, 0 };            // soc limited charge1, soc >= max
    array[11] = { 3000, 30, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 50, 51, 100, 0, 3000 };      // soc limited discharge1, soc <= min
    array[12] = { 3000, 100, -10, 10, 30, 100, -2000, -1000, 1000, 2000, 10, 10, 100, 2000, 3000 };  // soc limited discharge2, soc <= min but thresholds N/A due to price

    // iterate through each test case and get results
    for (int i = 0; i < num_tests; i++) {
        // Only print messages to log if a test fails
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();
        energy_arbitrage.price.value.value_float = array[i].price;
        energy_arbitrage.threshold_charge_2.value.value_float = array[i].threshold_charge_2;
        energy_arbitrage.threshold_charge_1.value.value_float = array[i].threshold_charge_1;
        energy_arbitrage.threshold_dischg_1.value.value_float = array[i].threshold_dischg_1;
        energy_arbitrage.threshold_dischg_2.value.value_float = array[i].threshold_dischg_2;
        energy_arbitrage.max_charge_2.value.value_float = array[i].max_charge_2;
        energy_arbitrage.max_charge_1.value.value_float = array[i].max_charge_1;
        energy_arbitrage.max_dischg_1.value.value_float = array[i].max_dischg_1;
        energy_arbitrage.max_dischg_2.value.value_float = array[i].max_dischg_2;
        energy_arbitrage.soc_min_limit.value.value_float = array[i].min_soc_limit;
        energy_arbitrage.soc_max_limit.value.value_float = array[i].max_soc_limit;

        soc_avg_running.value.set(array[i].avg_running_soc);
        asset_cmd.solar_data.max_potential_kW = array[i].solar_max_potential;

        energy_arbitrage.execute(asset_cmd, soc_avg_running.value.value_float, active_alarm_array);

        errorLog << "energy_arbitrage() test " << i + 1 << " of " << num_tests << std::endl;
        bool failure = asset_cmd.ess_data.kW_request != array[i].expected_ess_kW_request || asset_cmd.solar_data.kW_request != array[i].expected_solar_kW_request;
        EXPECT_EQ(asset_cmd.ess_data.kW_request, array[i].expected_ess_kW_request);
        EXPECT_EQ(asset_cmd.solar_data.kW_request, array[i].expected_solar_kW_request);

        // Release stdout so we can write again
        release_stdout(failure);
        // Print the test id if failure
        if (failure)
            std::cout << errorLog.str() << std::endl;
    }
}

// Constant Power Factor algorithm tests
TEST_F(site_manager_test, constant_power_factor) {
    // struct that has variables to configure for each test case
    struct cpf_test {
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
        { true, false, -0.5f, 0.5f, 1.0f, 10000.0f, 10000.0f, 1.0f, 0.0f },           // Absolute mode tests, 0 reactive
        { true, true, -0.5f, 0.5f, -0.97f, 10000.0f, 10000.0f, -0.97f, -2500.0f },    // Lagging reactive
        { true, true, -0.5f, 0.5f, 0.97f, 10000.0f, 10000.0f, -0.97f, -2500.0f },     // Lagging reactive, positive pf forced negative
        { true, false, -0.5f, 0.5f, 0.97f, 10000.0f, 10000.0f, 0.97f, 2500.0f },      // Leading reactive
        { true, false, -0.5f, 0.5f, -0.97f, 10000.0f, 10000.0f, 0.97f, 2500.0f },     // Leading reactive, negative pf forced positive
        { true, true, -0.5f, 0.5f, 0.7f, 10000.0f, 10000.0f, -0.7f, -10000.0f },      // Lagging reactive, beyond total potential KVAR but limited
        { true, false, -0.5f, 0.5f, 0.7f, 10000.0f, 10000.0f, 0.7f, 10000.0f },       // Leading reactive, beyond total potential KVAR but limited
        { true, false, -0.0f, 0.0f, 0.0f, 10000.0f, 10000.0f, 0.0f, 10000.0f },       // Divide by 0 case
        { true, true, -0.5f, 0.5f, 1.1f, 10000.0f, 10000.0f, -1.0f, 0.0f },           // Lagging, beyond outer limit, limited to -1.0
        { true, true, -0.5f, 0.5f, 0.0f, 10000.0f, 10000.0f, -0.5f, -10000.0f },      // Lagging, beyond inner limit, limited to lagging limit
        { true, false, -0.5f, 0.5f, 1.1f, 10000.0f, 10000.0f, 1.0f, 0.0f },           // Leading, beyond outer limit, limited to 1.0
        { true, false, -0.5f, 0.5f, 0.0f, 10000.0f, 10000.0f, 0.5f, 10000.0f },       // Leading, beyond inner limit, limited to lagging limit
        { false, false, -0.5f, 0.5f, 1.0f, 10000.0f, 10000.0f, 1.0f, 0.0f },          // Bidirectional tests, 0 reactive
        { false, false, 0.5f, 0.5f, -0.97f, 10000.0f, 10000.0f, -0.97f, -2500.0f },   // Positive lagging limit forced negative
        { false, false, -0.5f, -0.5f, 0.97f, 10000.0f, 10000.0f, 0.97f, 2500.0f },    // Negative leading limit forced positive
        { false, false, 1.1f, 0.5f, -1.1f, 10000.0f, 10000.0f, -1.0f, 0.0f },         // Positive lagging limit above 1.0
        { false, false, -0.5f, -1.1f, 1.1f, 10000.0f, 10000.0f, 1.0f, 0.0f },         // Negative leading limit below -1.0
        { false, false, -0.5f, 0.5f, -0.97f, 10000.0f, 10000.0f, -0.97f, -2500.0f },  // Lagging reactive
        { false, false, -0.5f, 0.5f, 0.97f, 10000.0f, 10000.0f, 0.97f, 2500.0f },     // Leading reactive
        { false, false, -0.5f, 0.5f, -0.7f, 10000.0f, 10000.0f, -0.7f, -10000.0f },   // Lagging reactive, beyond total potential KVAR but limited
        { false, false, -0.5f, 0.5f, 0.7f, 10000.0f, 10000.0f, 0.7f, 10000.0f },      // Leading reactive, beyond total potential KVAR but limited
        { false, false, -0.0f, 0.0f, 0.0f, 10000.0f, 10000.0f, 0.0f, 10000.0f },      // Divide by 0 case
        { false, false, -0.5f, 0.5f, -1.1f, 10000.0f, 10000.0f, -1.0f, 0.0f },        // Lagging, beyond outer limit, limited to -1.0
        { false, false, -0.5f, 0.5f, -0.0f, 10000.0f, 10000.0f, -0.5f, -10000.0f },   // Lagging, beyond inner limit, limited to lagging limit
        { false, false, -0.5f, 0.5f, 1.0f, 10000.0f, 10000.0f, 1.0f, 0.0f },          // Leading, beyond outer limit, limited to 1.0
        { false, false, -0.5f, 0.5f, 0.0f, 10000.0f, 10000.0f, 0.5f, 10000.0f },      // Leading, beyond inner limit, limited to lagging limit
        { true, false, -0.5f, 0.5f, 1.0f, 10000.0f, 10000.0f, 1.0f, 0.0f },           // Ensure negative site demand produces the same result for the same tests
        // clang-format off
        {true, true, -0.5f, 0.5f, -0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f},  {true, true, -0.5f, 0.5f, 0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f}, {true, false, -0.5f, 0.5f, 0.97f, -10000.0f, 10000.0f, 0.97f, 2500.0f}, {true, false, -0.5f, 0.5f, -0.97f, -10000.0f, 10000.0f, 0.97f, 2500.0f},    {true, true, -0.5f, 0.5f, 0.7f, -10000.0f, 10000.0f, -0.7f, -10000.0f},  {true, false, -0.5f, 0.5f, 0.7f, -10000.0f, 10000.0f, 0.7f, 10000.0f},     {true, false, -0.0f, 0.0f, 0.0f, -10000.0f, 10000.0f, 0.0f, 10000.0f},  {true, true, -0.5f, 0.5f, 1.1f, -10000.0f, 10000.0f, -1.0f, 0.0f},      {true, true, -0.5f, 0.5f, 0.0f, -10000.0f, 10000.0f, -0.5f, -10000.0f}, {true, false, -0.5f, 0.5f, 1.1f, -10000.0f, 10000.0f, 1.0f, 0.0f},         {true, false, -0.5f, 0.5f, 0.0f, -10000.0f, 10000.0f, 0.5f, 10000.0f}, {false, false, -0.5f, 0.5f, 1.0f, -10000.0f, 10000.0f, 1.0f, 0.0f},     {false, false, 0.5f, 0.5f, -0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f},
        {false, false, -0.5f, -0.5f, 0.97f, -10000.0f, 10000.0f, 0.97f, 2500.0f},  {false, false, 1.1f, 0.5f, -1.1f, -10000.0f, 10000.0f, -1.0f, 0.0f},     {false, false, -0.5f, -1.1f, 1.1f, -10000.0f, 10000.0f, 1.0f, 0.0f},    {false, false, -0.5f, 0.5f, -0.97f, -10000.0f, 10000.0f, -0.97f, -2500.0f}, {false, false, -0.5f, 0.5f, 0.97f, -10000.0f, 10000.0f, 0.97f, 2500.0f}, {false, false, -0.5f, 0.5f, -0.7f, -10000.0f, 10000.0f, -0.7f, -10000.0f}, {false, false, -0.5f, 0.5f, 0.7f, -10000.0f, 10000.0f, 0.7f, 10000.0f}, {false, false, -0.0f, 0.0f, 0.0f, -10000.0f, 10000.0f, 0.0f, 10000.0f}, {false, false, -0.5f, 0.5f, -1.1f, -10000.0f, 10000.0f, -1.0f, 0.0f},   {false, false, -0.5f, 0.5f, -0.0f, -10000.0f, 10000.0f, -0.5f, -10000.0f}, {false, false, -0.5f, 0.5f, 1.0f, -10000.0f, 10000.0f, 1.0f, 0.0f},    {false, false, -0.5f, 0.5f, 0.0f, -10000.0f, 10000.0f, 0.5f, 10000.0f},
        // clang-format on
    };
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("constant_power_factor", test_id++, tests.size());
        // Set CPF values
        constant_power_factor.absolute_mode.value.set(test.absolute_mode);
        constant_power_factor.lagging_direction.value.set(test.direction_flag);
        constant_power_factor.lagging_limit.value.set(test.lagging_limit);
        constant_power_factor.leading_limit.value.set(test.leading_limit);
        constant_power_factor.power_factor_setpoint.value.set(test.pf_setpoint);
        // Setup Asset Cmd Obj inputs
        asset_cmd.site_kW_demand = test.demand_kW;
        asset_cmd.preserve_uncorrected_site_kW_demand();
        asset_cmd.ess_data.potential_kVAR = test.potential_kVAR;
        asset_cmd.calculate_total_potential_kVAR();
        constant_power_factor.execute(asset_cmd, asset_pf_flag);
        t_log.float_results.push_back({ test.expected_pf, constant_power_factor.power_factor_setpoint.value.value_float, "PF Setpoint" });
        t_log.range_results.push_back({ test.expected_kVAR, 0.05f, asset_cmd.site_kVAR_demand, "CPF Demand" });
        t_log.check_solution();
    }
}

// ESS Discharge Prevention tests
TEST_F(site_manager_test, ess_discharge_prevention) {
    // struct that has variables to configure for each test case
    struct edp_test {
        float edp_soc;
        float soc_avg_running;
        float ess_total_kW_discharge_limit;

        float site_manager_max_potential_ess_kW;
        float asset_cmd_ess_max_potential_kW;
        float site_manager_min_potential_ess_kW;
        float asset_cmd_ess_min_potential_kW;
        float total_asset_kW_discharge_limit;

        float expected_site_manager_max_potential_ess_kW;
        float expected_asset_cmd_ess_max_potential_kW;
        float expected_site_manager_min_potential_ess_kW;
        float expected_asset_cmd_ess_min_potential_kW;
        float expected_total_asset_kW_discharge_limit;
    };

    std::vector<edp_test> tests = {
        // above target soc
        { 20.0f, 25.0f, 5000.0f, 5000.0f, 5000.0f, -5000.0f, -5000.0f, 10000.0f, 5000.0f, 5000.0f, -5000.0f, -5000.0f, 10000.0f },
        // below target soc, zero max and min potential kW
        { 20.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5000.0f },
        // below target soc, positive max potential kW
        { 20.0f, 15.0f, 5000.0f, 5000.0f, 5000.0f, -5000.0f, -5000.0f, 10000.0f, 0.0f, 0.0f, -5000.0f, -5000.0f, 5000.0f },
        // below target soc, positive max and min potential kW
        { 20.0f, 15.0f, 5000.0f, 5000.0f, 5000.0f, 1000.0f, 1000.0f, 10000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5000.0f },
    };
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("ess_discharge_prevention", test_id++, tests.size());
        ess_discharge_prevention.edp_soc.value.set(test.edp_soc);
        soc_avg_running.value.set(test.soc_avg_running);
        max_potential_ess_kW.value.set(test.site_manager_max_potential_ess_kW);
        min_potential_ess_kW.value.set(test.site_manager_min_potential_ess_kW);
        total_asset_kW_discharge_limit = test.total_asset_kW_discharge_limit;
        asset_cmd.ess_data.max_potential_kW = test.asset_cmd_ess_max_potential_kW;
        asset_cmd.ess_data.min_potential_kW = test.asset_cmd_ess_min_potential_kW;
        ess_discharge_prevention.execute(asset_cmd, soc_avg_running.value.value_float, max_potential_ess_kW.value.value_float, min_potential_ess_kW.value.value_float, test.ess_total_kW_discharge_limit, total_asset_kW_discharge_limit);
        t_log.float_results.push_back({ test.expected_site_manager_max_potential_ess_kW, max_potential_ess_kW.value.value_float, "Site Manager Max Potential ESS kW" });
        t_log.float_results.push_back({ test.expected_asset_cmd_ess_max_potential_kW, asset_cmd.ess_data.max_potential_kW, "Asset Cmd ESS Max Potential kW" });
        t_log.float_results.push_back({ test.expected_site_manager_min_potential_ess_kW, min_potential_ess_kW.value.value_float, "Site Manager Min Potential ESS kW" });
        t_log.float_results.push_back({ test.expected_asset_cmd_ess_min_potential_kW, asset_cmd.ess_data.min_potential_kW, "Asset Cmd ESS Min Potential kW" });
        t_log.float_results.push_back({ test.expected_total_asset_kW_discharge_limit, total_asset_kW_discharge_limit, "Total Asset kW Discharge Limit" });
        t_log.check_solution();
    }
}

// Alarm/Fault tests
TEST_F(site_manager_test, alarms_faults) {
    // struct that has variables to configure for each test case
    struct test_params {
        int alarm_index;
        int fault_index;
    };

    // Test a handful of the hardcoded alarm/fault numbers. Values up to 31 should be supported even with a default alarm range of only 6 entries
    std::vector<test_params> tests = { { 0, 0 }, { 0, 7 }, { 7, 0 }, { 7, 15 }, { 15, 7 }, { 15, 31 }, { 31, 15 } };
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("alarms_faults", test_id++, tests.size());

        set_alarms(test.alarm_index);
        set_faults(test.fault_index);

        t_log.bool_results.push_back({ true, active_alarm_array[test.alarm_index], "alarm set" });
        t_log.bool_results.push_back({ true, active_fault_array[test.fault_index], "fault set" });
        t_log.check_solution();
    }
}

#endif /* SITE_MANAGER_TEST_H_ */
