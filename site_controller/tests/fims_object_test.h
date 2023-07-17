#ifndef FIMS_OBJECT_TEST_H_
#define FIMS_OBJECT_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Fims_Object.h>
#include <test_tools.h>

class fims_object_test : public testing::Test
{ 
public:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(fims_object_test, multiple_inputs_alt_ui_types) {
    const char* raw_input_source_configuration = R"(
        [
            {
                "name": "Local",
                "uri_suffix": "local",
                "ui_type": "control",
                "alt_ui_types": [],
                "enabled": false
            },
            {
                "name": "Remote",
                "uri_suffix": "remote",
                "ui_type": "none",
                "alt_ui_types": [
                    {"var_id": "dummy_var", "ui_type": "status"}
                ],
                "enabled": true
            }
        ]
    )";
    cJSON* input_source_configuration = cJSON_Parse(raw_input_source_configuration);
    Input_Source_List input_sources;
    input_sources.parse_json_obj(input_source_configuration);

    Fims_Object defaults;
    fill_in_defaults(defaults);
    
    const char* raw_dummy_var_json = R"(
        {
            "name": "Dummy Var",
            "ui_type": "none",
            "type": "enum",
            "var_type": "Bool",
            "value": true,
            "multiple_inputs": true
        }
    )";
    cJSON* dummy_var_json = cJSON_Parse(raw_dummy_var_json);
    const char* dummy_var_id = "dummy_var";
    Fims_Object dummy_var;
    std::vector<Fims_Object*> multiple_inputs_vars; // only needed to satisfy `configure` function call
    dummy_var.configure(dummy_var_id, NULL, &input_sources, dummy_var_json, defaults, multiple_inputs_vars);

    fmt::memory_buffer buff;
    dummy_var.add_to_JSON_buffer(buff, NULL);

    std::string expected_output_p1 = R"("dummy_var":{"value":true,"name":"Dummy Var","unit":"","scaler":1,"enabled":true,"type":"enum","ui_type":"none","options":[]},)";
    std::string expected_output_p2 = R"("dummy_var_local":{"value":true,"name":"Dummy Var: Local","unit":"","scaler":1,"enabled":false,"type":"enum","ui_type":"control","options":[]},)";
    std::string expected_output_p3 = R"("dummy_var_remote":{"value":true,"name":"Dummy Var: Remote","unit":"","scaler":1,"enabled":true,"type":"enum","ui_type":"status","options":[]},)";
    std::string expected_output = expected_output_p1 + expected_output_p2 + expected_output_p3;

    ASSERT_EQ(to_string(buff), expected_output);
}

#endif /* FIMS_OBJECT_TEST_H_ */
