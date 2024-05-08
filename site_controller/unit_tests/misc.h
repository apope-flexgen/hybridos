#ifndef MISC_TEST_H_
#define MISC_TEST_H_

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include <climits>
#include <ctime>
#include <limits>
#include "Features/Watchdog.h"
#include "Fims_Object.h"
#include "test_tools.h"

class heartTester : public testing::Test {
    public:
        void SetUp() override {}
        void TearDown() override {}
};

TEST_F(heartTester, heartTest) {
    features::Watchdog watch;

    std::string config = "{\"heartbeat_counter\":{\"name\":\"Heartbeat Counter\",\"ui_type\":\"status\",\"value\":1,\"var_type\":\"Int\"},\"heartbeat_duration_ms\":{\"name\":\"Heartbeat Duration\",\"ui_type\":\"none\",\"unit\":\"ms\",\"value\":1,\"var_type\":\"Int\"},\"watchdog_duration_ms\":{\"name\":\"Watchdog Timer Duration\",\"ui_type\":\"none\",\"unit\":\"ms\",\"value\":5000,\"var_type\":\"Int\"},\"watchdog_enable\":{\"name\":\"Enable Watchdog\",\"type\":\"enum_slider\",\"ui_type\":\"control\",\"value\":false,\"var_type\":\"Bool\"},\"max_heartbeat\":{\"name\":\"Max Heartbeat\",\"ui_type\":\"control\",\"value\":255,\"var_type\":\"Int\"},\"min_heartbeat\":{\"name\":\"Min Heartbeat\",\"ui_type\":\"control\",\"value\":0,\"var_type\":\"Int\"},\"watchdog_pet\":{\"name\":\"Watchdog Pet\",\"ui_type\":\"none\",\"value\":1,\"var_type\":\"Int\"}}";

    auto *cJSON_config = cJSON_Parse(config.c_str());

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

    bool fake_is_primary = true;
    std::vector<Fims_Object*> multiple_inputs;
    watch.parse_json_config(cJSON_config, &fake_is_primary, &input_sources, defaults, multiple_inputs);

    watch.heartbeat_counter.value.value_int = 0;

    timespec clock;
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int == 255) { // tick to max_heartbeat
            break;
        }
    }
    EXPECT_EQ(255, watch.heartbeat_counter.value.value_int);

    watch.heartbeat_counter.value.value_int = 256; // aka greater than max_heartbeat
                                                   // should go to min_heartbeat @ 0

    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int != 256) { // single tick
            break;
        }
    }

    EXPECT_EQ(0, watch.heartbeat_counter.value.value_int);

    watch.min_heartbeat.value.value_int = -100;
    watch.max_heartbeat.value.value_int = 100;

    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int == 100) { // tick to 100
            break;
        }
    }
    EXPECT_EQ(100, watch.heartbeat_counter.value.value_int);

    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int != 100) { // single tick
            break;
        }
    }
    EXPECT_EQ(-100, watch.heartbeat_counter.value.value_int);

    // now test when not configured
    watch.min_heartbeat.configured = false;
    watch.max_heartbeat.configured = false;

    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int != -100) { // single tick
            break;
        }
    }
    EXPECT_EQ(0, watch.heartbeat_counter.value.value_int); // should tick to 0
                                                           
    watch.heartbeat_counter.value.value_int = INT_MAX-1;
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int != INT_MAX-1) { // single tick
            break;
        }
    }

    EXPECT_EQ(INT_MAX, watch.heartbeat_counter.value.value_int); // should tick to INT_MAX
                                                                 
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &clock);
        watch.beat(clock);
        if (watch.heartbeat_counter.value.value_int != INT_MAX) { // single tick
            break;
        }
    }

    EXPECT_EQ(0, watch.heartbeat_counter.value.value_int); // should tick to 0
}

#endif /* ASSET_MANAGER_TEST_H_ */
