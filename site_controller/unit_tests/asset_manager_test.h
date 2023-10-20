#ifndef ASSET_MANAGER_TEST_H_
#define ASSET_MANAGER_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unordered_set>
#include <Logger.h>
#include "Asset_Manager.h"

/* add specific unit-level test headers here*/
#include "asset_test.h"
#include "ess_manager_test.h"
#include "feeder_manager_test.h"
#include "generator_manager_test.h"
#include "solar_manager_test.h"
#include "config_test.h"

class asset_manager_test : public Asset_Manager, public testing::Test {
public:
    // Required to reset the asset manager between tests otherwise get unintended side effects
    void asset_manager_reset() {
        delete ess_manager;
        delete feeder_manager;
        delete generator_manager;
        delete solar_manager;
        delete ess_configurator;
        delete feeder_configurator;
        delete generator_configurator;
        delete solar_configurator;

        ess_manager = new ESS_Manager();
        feeder_manager = new Feeder_Manager();
        generator_manager = new Generator_Manager();
        solar_manager = new Solar_Manager();
        build_configurators();
    };
};

// Verify that all expected info/error messages are found in the actual configuration result
// errors_not_present list of specific errors that should not be found
// no_errors_expected flag defaulted to false. If manually set true, will instead verify that no errors are present in the actual result.
bool check_error_messages(Config_Validation_Result& expected_result, Config_Validation_Result& actual_result, std::vector<std::string> errors_not_present) {
    if (expected_result.is_valid_config)
        return actual_result.ERROR_details.size() == 0;

    bool all_matched = true;
    for (Result_Details result : expected_result.ERROR_details) {
        if (std::find(actual_result.ERROR_details.begin(), actual_result.ERROR_details.end(), result.details) == actual_result.ERROR_details.end()) {
            all_matched = false;
            FPS_ERROR_LOG("Failed to match error string: %s", result.details);
        }
    }
    for (Result_Details result : expected_result.INFO_details) {
        if (std::find(actual_result.INFO_details.begin(), actual_result.INFO_details.end(), result.details) == actual_result.INFO_details.end()) {
            all_matched = false;
            FPS_ERROR_LOG("Failed to match info string: %s", result.details);
        }
    }

    if (errors_not_present.size() > 0) {
        for (std::string error : errors_not_present) {
            if (std::find(actual_result.ERROR_details.begin(), actual_result.ERROR_details.end(), error) != actual_result.ERROR_details.end()) {
                all_matched = false;
                FPS_ERROR_LOG("Found an error that should not be present: %s", error);
            }
        }
    }
    return all_matched;
}

// Test the creating of assets using various templating options. In the process, verify error consolidation by
// checking that a sample of multiple, specific info/error messages are present for each test case
TEST_F(asset_manager_test, asset_create) {
    struct tests {
        std::string filepath;
        Config_Validation_Result expected_ess_result;
        Config_Validation_Result expected_feed_result;
        Config_Validation_Result expected_gen_result;
        Config_Validation_Result expected_solar_result;
        // Errors that should not be found
        std::vector<std::string> ess_errors_not_present;
        std::vector<std::string> feed_errors_not_present;
        std::vector<std::string> gen_errors_not_present;
        std::vector<std::string> solar_errors_not_present;
    };
    std::vector<tests> test_cases;

    test_cases.push_back({ "unit_tests/unit_test_files/assets/valid_mixed.json", Config_Validation_Result(true), Config_Validation_Result(true), Config_Validation_Result(true), Config_Validation_Result(true), {}, {}, {}, {} });
    test_cases[0].expected_ess_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[0].expected_ess_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[0].expected_solar_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[0].expected_solar_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));

    // Range for ESS blocks A and B overlap on entry 3, but B continues and still catches other configuration issues like missing required variable
    test_cases.push_back({ "unit_tests/unit_test_files/assets/invalid_ranged.json", Config_Validation_Result(false), Config_Validation_Result(true), Config_Validation_Result(true), Config_Validation_Result(true), {}, {}, {}, {} });
    test_cases[1].expected_ess_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[1].expected_ess_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[1].expected_solar_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[1].expected_solar_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[1].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to configure asset array entry 2, error in assets.json."));
    test_cases[1].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to expand templated string ess_#."));
    test_cases[1].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to configure using a templated range. All entries in internal configuration map already configured."));
    test_cases[1].expected_ess_result.ERROR_details.push_back(Result_Details("ess: internal configuration map size: 3."));

    // B overlaps with every entry provided in the range of A (1, 3, 5). Components are missing for B so configuration must stop there
    // Solar is also missing name/id but a placeholder is provided so the config can continue. Only the first of the templated entries is checked
    test_cases.push_back({ "unit_tests/unit_test_files/assets/invalid_mixed.json", Config_Validation_Result(false), Config_Validation_Result(true), Config_Validation_Result(true), Config_Validation_Result(false), {}, {}, {}, {} });
    test_cases[2].expected_ess_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[2].expected_ess_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[2].expected_solar_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[2].expected_solar_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[2].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to configure asset array entry 2, error in assets.json."));
    test_cases[2].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to expand templated string ess_#."));
    test_cases[2].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to configure using a templated range. All entries in internal configuration map already configured."));
    test_cases[2].expected_ess_result.ERROR_details.push_back(Result_Details("ess: internal configuration map size: 3."));
    test_cases[2].expected_ess_result.ERROR_details.push_back(Result_Details("BESS Inverter Block B 01: components control array is NULL."));
    test_cases[2].expected_ess_result.ERROR_details.push_back(Result_Details("BESS Inverter Block B 02: components control array is NULL."));
    test_cases[2].expected_solar_result.ERROR_details.push_back(Result_Details("solar: name is missing for asset entry 0."));
    test_cases[2].expected_solar_result.ERROR_details.push_back(Result_Details("solar: id is missing for asset entry 0."));
    test_cases[2].expected_solar_result.ERROR_details.push_back(
        Result_Details("solar: error parsing asset with ID solar_0 and name solar_0: if 'number_of_instances' is greater than 1, the asset entry is considered templated and must have a wildcard character in the ID and name."));
    test_cases[2].expected_solar_result.ERROR_details.push_back(Result_Details("solar_0: failed to find rated_active_pwr_kw in config."));
    // Errors that should not appear
    test_cases[2].ess_errors_not_present.push_back("BESS Inverter Block B 03: components control array is NULL.");
    test_cases[2].solar_errors_not_present.push_back("solar: name is missing for asset entry 1.");
    test_cases[2].solar_errors_not_present.push_back("solar: id is missing for asset entry 1.");
    test_cases[2].solar_errors_not_present.push_back("solar_1: failed to find rated_active_pwr_kw in config.");

    // All ESS entries overlap, but only configuration issues from the first overlap are reported
    test_cases.push_back({ "unit_tests/unit_test_files/assets/invalid_number_of_instances.json", Config_Validation_Result(false), Config_Validation_Result(true), Config_Validation_Result(true), Config_Validation_Result(true), {}, {}, {}, {} });
    test_cases[3].expected_ess_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[3].expected_ess_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[3].expected_solar_result.INFO_details.push_back(Result_Details("reusing status_type random_enum for local_mode_status_type"));
    test_cases[3].expected_solar_result.INFO_details.push_back(Result_Details("optional local_mode_signal was not provided in configuration"));
    test_cases[3].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to configure asset array entry 2, error in assets.json."));
    test_cases[3].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to expand templated string ess_#."));
    test_cases[3].expected_ess_result.ERROR_details.push_back(Result_Details("ess: failed to configure using a templated range. All entries in internal configuration map already configured."));
    test_cases[3].expected_ess_result.ERROR_details.push_back(Result_Details("ess: internal configuration map size: 5."));
    test_cases[3].expected_ess_result.ERROR_details.push_back(Result_Details("BESS Inverter Block A 05: only received one autobalancing control: autobalancing_enable. Either remove this control or provide autobalancing_disable as well."));
    test_cases[3].expected_ess_result.ERROR_details.push_back(Result_Details("BESS Inverter Block B 01: only received one autobalancing control: autobalancing_disable. Either remove this control or provide autobalancing_enable as well."));
    // Errors that should not appear
    test_cases[3].ess_errors_not_present.push_back("BESS Inverter Block B 02: only received one autobalancing control: autobalancing_disable. Either remove this control or provide autobalancing_ensable as well.");

    build_configurators();

    for (auto test : test_cases) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();
        errorLog << "Testing: " << test.filepath << std::endl;

        // Set up config subobjects for each config type
        cJSON* test_config = cJSON_GetObjectItem(parseJSONConfig(test.filepath), "assets");
        generator_configurator->asset_type_root = cJSON_GetObjectItem(test_config, "generators");
        feeder_configurator->asset_type_root = cJSON_GetObjectItem(test_config, "feeders");
        ess_configurator->asset_type_root = cJSON_GetObjectItem(test_config, "ess");
        solar_configurator->asset_type_root = cJSON_GetObjectItem(test_config, "solar");

        // Test each the configuration of each type
        Config_Validation_Result ess_result = ess_configurator->create_assets();
        bool all_ess_messages_matched = check_error_messages(test.expected_ess_result, ess_result, test.ess_errors_not_present);
        failure = (ess_result.is_valid_config != test.expected_ess_result.is_valid_config) || !all_ess_messages_matched;
        EXPECT_EQ(test.expected_ess_result.is_valid_config, ess_result.is_valid_config);
        EXPECT_TRUE(all_ess_messages_matched);
        ess_result.log_details();

        Config_Validation_Result feed_result = feeder_configurator->create_assets();
        bool all_feed_messages_matched = check_error_messages(test.expected_feed_result, feed_result, test.feed_errors_not_present);
        failure |= (feed_result.is_valid_config != test.expected_feed_result.is_valid_config) || !all_feed_messages_matched;
        EXPECT_EQ(test.expected_feed_result.is_valid_config, feed_result.is_valid_config);
        EXPECT_TRUE(all_feed_messages_matched);
        feed_result.log_details();

        Config_Validation_Result gen_result = generator_configurator->create_assets();
        bool all_gen_messages_matched = check_error_messages(test.expected_gen_result, gen_result, test.gen_errors_not_present);
        failure |= (gen_result.is_valid_config != test.expected_gen_result.is_valid_config) || !all_gen_messages_matched;
        EXPECT_EQ(test.expected_gen_result.is_valid_config, gen_result.is_valid_config);
        EXPECT_TRUE(all_gen_messages_matched);
        gen_result.log_details();

        Config_Validation_Result solar_result = solar_configurator->create_assets();
        bool all_solar_messages_matched = check_error_messages(test.expected_solar_result, solar_result, test.solar_errors_not_present);
        failure |= (solar_result.is_valid_config != test.expected_solar_result.is_valid_config) || !all_solar_messages_matched;
        EXPECT_EQ(test.expected_solar_result.is_valid_config, solar_result.is_valid_config);
        EXPECT_TRUE(all_solar_messages_matched);
        solar_result.log_details();

        // Cleanup
        cJSON_Delete(test_config);
        asset_manager_reset();
        release_stdout(failure);
        if (failure) {
            std::cerr << errorLog.str() << std::endl;
        }
    }
}

    //

#endif /* ASSET_MANAGER_TEST_H_ */
