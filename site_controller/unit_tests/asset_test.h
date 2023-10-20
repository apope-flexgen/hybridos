#ifndef ASSET_TEST_H
#define ASSET_TEST_H

#include <gtest/gtest.h>
#include <test_tools.h>
#include "Asset_ESS.h"

class Asset_Mock : public Asset_ESS, public testing::Test {
public:
    // jsonBuildOption build_option, void* display, valueType value_type_cfg, displayType display_type_cfg, bool enabled_cfg, bool is_bool_string
    Asset_Mock() { asset_type_id = "dummy_type_id"; }

    void configure_start_ctl(cJSON* config) { start_ctl.configure(config, yesNoOption, NULL, Int, enumStr); };
};

void prepare_buffer(std::string variable_id, fmt::memory_buffer& buf) {
    buf.clear();
    auto to_add = "\"" + variable_id + "\":";
    for (size_t i = 0; i < to_add.length(); i++) {
        buf.push_back(to_add[i]);
    }
}

TEST_F(Asset_Mock, add_variable_to_buffer) {
    fmt::memory_buffer test_buf;
    std::map<std::string, Fims_Object*> test_var_map;
    std::string variable_id_1 = "start";
    std::string variable_id_2 = "start_stop";
    std::string variable_id_3 = "z_ignored";
    Fims_Object obj2;
    Fims_Object obj3;

    cJSON* config = cJSON_Parse("{\"start\":{\"name\":\"Start\",\"register_id\":\"start\",\"type\":\"Int\"}}")->child;
    configure_start_ctl(config);

    obj2.set_variable_id(variable_id_2);
    obj3.set_variable_id(variable_id_3);
    test_var_map[variable_id_2] = &obj2;
    test_var_map[variable_id_3] = &obj3;
    asset_var_map = test_var_map;

    prepare_buffer(variable_id_1, test_buf);
    add_variable_to_buffer(variable_id_1,          // uri
                           variable_id_1.c_str(),  // variable_id
                           test_buf);

    // the intent is to test that the variable "start" will fallthrough and be registered as a UI control rather than colliding with the variable "start_stop" from the variables map.
    EXPECT_EQ("\"start\":{\"enabled\":false,\"options\":[{\"name\":\"No\",\"return_value\":false},{\"name\":\"Yes\",\"return_value\":true}]}", to_string(test_buf));

    prepare_buffer(variable_id_2, test_buf);
    add_variable_to_buffer(variable_id_2,          // uri
                           variable_id_2.c_str(),  // variable_id
                           test_buf);

    EXPECT_EQ("",  // Naked therefore has no value
              to_string(test_buf));
}

TEST_F(Asset_Mock, handle_generic_asset_controls_maintenance_set) {
    // struct that has variables to configure for each test case
    struct test_struct {
        bool maint_mode;
        bool lockdown_mode;
        bool maint_mode_set;
        bool expected_maint_mode;
        bool expected_lockdown_mode;
        bool expected_return;
    };

    std::vector<test_struct> tests = {
        { false, false, false, false, false, true },
        { false, false, true, true, false, false /* returns false due to failure to send setpoint */ },
        { false, true, false, false, true, true },
        { false, true, true, true, false, false /* returns false due to failure to send setpoint */ },
        { true, false, false, false, false, false /* returns false due to failure to send setpoint */ },
        { true, false, true, true, false, true },
        { true, true, false, true, true, false },
        { true, true, true, true, true, true },
    };
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("asset maintenance mode set", test_id++, tests.size());

        inMaintenance = test.maint_mode;
        inLockdown = test.lockdown_mode;

        cJSON* msg = cJSON_Parse((std::string(R"({"maint_mode":)") + (test.maint_mode_set ? "true" : "false") + std::string("}")).c_str());
        bool result = handle_generic_asset_controls_set("/assets/ess/ess_1/maint_mode", *msg);
        cJSON_Delete(msg);

        t_log.bool_results.push_back({ test.expected_maint_mode, inMaintenance, "Maintenance mode" });
        t_log.bool_results.push_back({ test.expected_lockdown_mode, inLockdown, "Lockdown mode" });
        t_log.bool_results.push_back({ test.expected_return, result, "Set success" });
        t_log.check_solution();
    }
}

TEST_F(Asset_Mock, handle_generic_asset_controls_lockdown_set) {
    // struct that has variables to configure for each test case
    struct test_struct {
        bool maint_mode;
        bool lockdown_mode;
        bool lockdown_mode_set;
        bool expected_maint_mode;
        bool expected_lockdown_mode;
        bool expected_return;
    };

    std::vector<test_struct> tests = {
        { false, false, false, false, false, true },
        { false, false, true, false, false, false },
        { false, true, false, false, false, true },
        { false, true, true, false, false, false },
        { true, false, false, true, false, true },
        { true, false, true, true, true, false /* returns false due to failure to send setpoint */ },
        { true, true, false, true, false, false /* returns false due to failure to send setpoint */ },
        { true, true, true, true, true, true },
    };
    int test_id = 1;
    for (auto test : tests) {
        test_logger t_log("asset lockdown mode set", test_id++, tests.size());

        inMaintenance = test.maint_mode;
        inLockdown = test.lockdown_mode;

        cJSON* msg = cJSON_Parse((std::string(R"({"lock_mode":)") + (test.lockdown_mode_set ? "true" : "false") + std::string("}")).c_str());
        bool result = handle_generic_asset_controls_set("/assets/ess/ess_1/lock_mode", *msg);
        cJSON_Delete(msg);

        t_log.bool_results.push_back({ test.expected_maint_mode, inMaintenance, "Maintenance mode" });
        t_log.bool_results.push_back({ test.expected_lockdown_mode, inLockdown, "Lockdown mode" });
        t_log.bool_results.push_back({ test.expected_return, result, "Set success" });
        t_log.check_solution();
    }
}

// Ensure that the set_fims_<type>() functions used by pub processing set both numeric value registers so either type can be used as an input
TEST_F(Asset_Mock, numeric_register_parsing) {
    soc_raw.set_variable_id("soc");
    maxRawSoc = 100;
    minRawSoc = 0;
    grid_mode_setpoint.set_variable_id("grid_mode");

    // Floating point tests using soc as a typical hard-coded floating point example
    test_logger t_log("Numeric register parsing", 1, 4);
    soc_raw.set_type("Float");
    soc_raw.set_fims_float("soc", 50.0f);
    soc.value.value_float = process_soc(soc_raw.value.value_float);
    t_log.float_results.push_back({ 50.0f, soc.value.value_float, "processed float as float" });
    t_log.check_solution();

    test_logger t_log_2("Numeric register parsing", 2, 4);
    soc_raw.set_type("Int");
    soc_raw.set_fims_int("soc", 75);
    soc.value.value_float = process_soc(soc_raw.value.value_float);
    t_log_2.float_results.push_back({ 75.0f, soc.value.value_float, "processed int as float" });
    t_log_2.check_solution();

    // Integer tests using grid_mode_setpoint as a typical hard-coded integer example
    test_logger t_log_3("Numeric register parsing", 3, 4);
    grid_mode_setpoint.set_type("Int");
    grid_mode_setpoint.set_fims_int("grid_mode", 1);
    // No processing functional available for int examples. Just check that the internal value was set
    t_log_3.int_results.push_back({ 1, grid_mode_setpoint.value.value_int, "processed int as int" });
    t_log_3.check_solution();

    test_logger t_log_4("Numeric register parsing", 4, 4);
    grid_mode_setpoint.set_type("Float");
    grid_mode_setpoint.set_fims_float("grid_mode", 2);
    // No processing functional available for int examples. Just check that the internal value was set
    t_log_4.int_results.push_back({ 2, grid_mode_setpoint.value.value_int, "processed float as int" });
    t_log_4.check_solution();
}
#endif /* ASSET_TEST_H */
