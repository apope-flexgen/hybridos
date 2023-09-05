#ifndef ASSET_TEST_H
#define ASSET_TEST_H

#include <gtest/gtest.h>
#include "Asset_ESS.h"

class Asset_Mock : public Asset_ESS, public testing::Test
{
public:
    // jsonBuildOption build_option, void* display, valueType value_type_cfg, displayType display_type_cfg, bool enabled_cfg, bool is_bool_string
    Asset_Mock() { asset_type_id = "dummy_type_id"; }

    void configure_start_ctl(cJSON* config) { start_ctl.configure(config, yesNoOption, NULL, Int, enumStr); };
};

void prepare_buffer(std::string variable_id, fmt::memory_buffer &buf) {
    buf.clear();
    auto to_add = "\"" + variable_id + "\":";
    for (size_t i = 0; i < to_add.length(); i++) {
        buf.push_back(to_add[i]);
    }
}

TEST_F(Asset_Mock, add_variable_to_buffer)
{
    fmt::memory_buffer test_buf;
    std::map<std::string, Fims_Object*> test_var_map;
    std::string variable_id_1 = "start";
    std::string variable_id_2 = "start_stop";
    std::string variable_id_3 = "z_ignored";
    Fims_Object obj2;
    Fims_Object obj3;

    cJSON* config = cJSON_Parse(
        "{\"start\":{\"name\":\"Start\",\"register_id\":\"start\",\"type\":\"Int\"}}"
    )->child;
    configure_start_ctl(config);

    obj2.set_variable_id(variable_id_2);
    obj3.set_variable_id(variable_id_3);
    test_var_map[variable_id_2] = &obj2;
    test_var_map[variable_id_3] = &obj3;
    asset_var_map = test_var_map;

    prepare_buffer(variable_id_1, test_buf);
    add_variable_to_buffer(
        variable_id_1, // uri
        variable_id_1.c_str(), // variable_id
        test_buf
    );

    // the intent is to test that the variable "start" will fallthrough and be registered as a UI control rather than colliding with the variable "start_stop" from the variables map.
    ASSERT_EQ("\"start\":{\"enabled\":false,\"options\":[{\"name\":\"No\",\"return_value\":false},{\"name\":\"Yes\",\"return_value\":true}]}", to_string(test_buf));

    prepare_buffer(variable_id_2, test_buf);
    add_variable_to_buffer(
        variable_id_2, // uri
        variable_id_2.c_str(), // variable_id
        test_buf
    );

    ASSERT_EQ(
        "", // Naked therefore has no value
        to_string(test_buf)
    );
}

#endif /* ASSET_TEST_H */
