#ifndef CONFIG_TEST_H_
#define CONFIG_TEST_H_

#include <gtest/gtest.h>
#include <test_tools.h>
#include "Types.h"
#include "site_manager_test.h"

#include <iostream>
#include <fstream>

const std::string UNIT_TEST_FILES_PATH = "/home/hybridos/git/hybridos/sandbox/dispatch_coordination/unit_test_configs/site_controller/";

/**
 * Parses the config cJSON from storage given the configuration type (Assets/Sequences/Variables)
 */
cJSON* parseJSONConfig(std::string file_path) {
    // open the file
    std::ifstream file;
    try {
        file.open(file_path);
    } catch (std::ifstream::failure e) {
        std::cerr << "Exception opening/reading file" << std::endl;
        return NULL;
    }
    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return NULL;
    }
    // get number of chars in file
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    // read file into buffer
    char* buffer = new char[length + 1];
    file.read(buffer, length);
    file.close();
    buffer[length] = '\0';
    // parse buffer into cJSON
    cJSON* config = cJSON_Parse(buffer);
    if (config == NULL) {
        std::cerr << "Failed to parse config file" << std::endl;
        return NULL;
    }
    // close file and free buffer
    delete[] buffer;
    return config;
}

// Test that variables.json is properly parsed
TEST_F(site_manager_test, variables_config_test) {
    // struct for checking that a particular variable has the expected value
    struct test_variable_check {
        valueType type;
        std::string variable_id;
        Value_Object expected;
        Value_Object& ref_to_actual;
    };
    // test case struct
    struct test_struct {
        std::string filepath;
        bool is_valid_config;
        std::vector<test_variable_check> variable_checks;
    };
    std::vector<test_struct> tests = {
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/full_config_active_power_setpoint.json",
                     true,
                     {
                         test_variable_check{
                             valueType::Int,
                             "active_power_setpoint_kW_slew_rate",
                             create_Value_Object<int>(1234),
                             active_power_setpoint_mode.kW_slew_rate.value,
                         },
                     } },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/minimal_config.json", true, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/invalid/missing_asset_priority_runmode1.json", false, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/missing_defaults.json", true, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/missing_runmode2_kW_mode_variables.json",
                     true,
                     {
                         test_variable_check{
                             valueType::String,
                             "available_features_runmode2_kW_mode",
                             create_Value_Object("0x2"),
                             available_features_runmode2_kW_mode.value,
                         },
                         test_variable_check{
                             valueType::Int,
                             "runmode2_kW_mode_cmd",
                             create_Value_Object(1),
                             runmode2_kW_mode_cmd.value,
                         },
                         test_variable_check{
                             valueType::String,
                             "runmode2_kW_mode_status",
                             create_Value_Object("Disabled"),
                             runmode2_kW_mode_status.value,
                         },
                     } },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/invalid/missing_required_active_power_poi_limits_variables.json", false, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/unrecognized_variable.json", true, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/missing_frequency_response.json", true, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/invalid/typo_start_first_gen_soc.json", false, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/invalid/empty.json", false, {} },
        test_struct{ UNIT_TEST_FILES_PATH + "variables/valid/minimal_config_with_1_feature_each_type.json",
                     true,
                     {
                         test_variable_check{
                             valueType::Float,
                             "ess_charge_control_target_soc",
                             create_Value_Object<float>(90.0),
                             charge_control.target_soc.value,
                         },
                         test_variable_check{
                             valueType::Int,
                             "reactive_setpoint_kVAR_slew_rate",
                             create_Value_Object(1000),
                             reactive_setpoint.kVAR_slew_rate.value,
                         },
                         test_variable_check{
                             valueType::Float,
                             "active_power_poi_limits_min_kW",
                             create_Value_Object<float>(-10000.0),
                             active_power_poi_limits.min_kW.value,
                         },
                         test_variable_check{
                             valueType::Float,
                             "generator_charge_additional_buffer",
                             create_Value_Object<float>(25.0),
                             generator_charge.generator_charge_additional_buffer.value,
                         },
                         test_variable_check{
                             valueType::Int,
                             "watchdog_duration_ms",
                             create_Value_Object(5000),
                             watchdog_feature.watchdog_duration_ms.value,
                         },
                     } },
    };

    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("site manager config test", test_id++, tests.size());

        // reset site manager variables before test runs
        for (auto& variable_id_pair : variable_ids) {
            variable_id_pair.first->value = Value_Object();
        }
        for (auto features_list : {
                 &runmode1_kW_features_list,
                 &runmode2_kW_features_list,
                 &charge_features_list,
                 &runmode1_kVAR_features_list,
                 &runmode2_kVAR_features_list,
                 &standalone_power_features_list,
                 &site_operation_features_list,
             }) {
            for (auto feature : *features_list) {
                feature->available = false;
                for (auto variable_id_pair : feature->variable_ids) {
                    variable_id_pair.first->value = Value_Object();
                }
            }
        }
        input_sources = Input_Source_List{};

        // parse test config
        cJSON* test_config = parseJSONConfig(test.filepath);
        bool config_parsed = test_config != NULL;
        bool config_passed = parse_variables(test_config);
        cJSON_Delete(test_config);

        t_log.bool_results.push_back({ true, config_parsed, "Test config successfully read and parsed" });
        t_log.bool_results.push_back({ test.is_valid_config, config_passed, "Config is valid" });

        for (auto check : test.variable_checks) {
            switch (check.type) {
                case valueType::Int:
                    t_log.int_results.push_back({ check.expected.value_int, check.ref_to_actual.value_int, "Variable " + check.variable_id + " configured int value" });
                    break;
                case valueType::Float:
                    t_log.float_results.push_back({ check.expected.value_float, check.ref_to_actual.value_float, "Variable " + check.variable_id + " configured float value" });
                    break;
                case valueType::String:
                    t_log.string_results.push_back({ check.expected.value_string, check.ref_to_actual.value_string, "Variable " + check.variable_id + " configured string value" });
                    break;
                default:
                    t_log.bool_results.push_back({ false, true, "Unexpected variable type in check that " + check.variable_id + " is configured correctly" });
                    break;
            }
        }

        // make file name visible in test output
        std::string file_name = test.filepath.substr(test.filepath.find_last_of('/', test.filepath.length()) + 1, test.filepath.length());
        t_log.string_results.push_back({ file_name, file_name, "Test config file name" });

        t_log.check_solution();
    }
}

#endif /* CONFIG_TEST_H_ */
