/**
 * @file test_monitor_condition.cpp
 * @brief Test file for the conditions feature in CheckMonitorVar file
 * @date 2021-05-04
 */

#include "gtest/gtest.h"
#include "../funcs/CheckMonitorVar.cpp"
#include "scheduler.h"

// Need this in order to compile
namespace flex
{
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

// Need this in order to compile
typedef std::vector<schedItem*>schlist;
schlist schreqs;
cJSON*getSchListcJ(schlist&schreqs);

cJSON*getSchList()
{
    return getSchListcJ(schreqs);
}

// Test fixture for creating assetVars and other shared objects for each test case
class ConditionMonitorTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        // Set up assetVars and params for the condition monitor assetVar
        // const char* ival = R"({"value": 0, "EnableConditionCheck": false, "AlarmTimeout": 5, "FaultTimeout": 7, "RecoverTimeout": 4})";
        // const char* dval = R"({"value": 0.0, "EnableConditionCheck": false, "AlarmTimeout": 5, "FaultTimeout": 7, "RecoverTimeout": 4})";
        // const char* cval = R"({"value": "init", "EnableConditionCheck": false, "AlarmTimeout": 5, "FaultTimeout": 7, "RecoverTimeout": 4})";
        const char* ival = "{\"value\": 0, \"EnableConditionCheck\": false, \"AlarmTimeout\": 5, \"FaultTimeout\": 7, \"RecoverTimeout\": 4}";
        const char* dval = "{\"value\": 0.0, \"EnableConditionCheck\": false, \"AlarmTimeout\": 5, \"FaultTimeout\": 7, \"RecoverTimeout\": 4}";
        const char* cval = "{\"value\": \"init\", \"EnableConditionCheck\": false, \"AlarmTimeout\": 5, \"FaultTimeout\": 7, \"RecoverTimeout\": 4}";

        cJSON* cjival = cJSON_Parse(ival);
        cJSON* cjdval = cJSON_Parse(dval);
        cJSON* cjcval = cJSON_Parse(cval);

        int_av = vm.setValfromCj(vmap, "/status/bms", "testConditionVar_int", cjival);
        dbl_av = vm.setValfromCj(vmap, "/status/bms", "testConditionVar_dbl", cjdval);
        str_av = vm.setValfromCj(vmap, "/status/bms", "testConditionVar_str", cjcval);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;

        int_av->am = bms_man;
        dbl_av->am = bms_man;
        str_av->am = bms_man;

        cJSON_Delete(cjival);
        cJSON_Delete(cjdval);
        cJSON_Delete(cjcval);

        CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
        CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
        CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    }

    virtual void TearDown() {
        delete int_av->am;  // Only need to delete asset manager from one av
    }

    varsmap vmap;       // main data map
    varmap amap;        // map of local variables for asset
    VarMapUtils vm;     // map utils factory
    assetVar* int_av;   // condition monitor assetVar of type integer
    assetVar* dbl_av;   // condition monitor assetVar of type double
    assetVar* str_av;   // condition monitor assetVar of type string
};

/**
 *********************************************************************************
 * Test cases without conditional variables and values
 * 
 * *******************************************************************************
 */

// Test if condition monitor parameters exist and contain the right values
TEST_F(ConditionMonitorTest, condition_monitor_params_valid) {
    // Check if params exist on startup
    EXPECT_TRUE(int_av->gotParam("EnableConditionCheck"));
    EXPECT_TRUE(int_av->gotParam("AlarmTimeout"));
    EXPECT_TRUE(int_av->gotParam("FaultTimeout"));
    EXPECT_TRUE(int_av->gotParam("RecoverTimeout"));
    EXPECT_TRUE(int_av->gotParam("Type"));
    EXPECT_TRUE(int_av->gotParam("AlarmTime"));
    EXPECT_TRUE(int_av->gotParam("FaultTime"));
    EXPECT_TRUE(int_av->gotParam("RecoverTime"));
    EXPECT_TRUE(int_av->gotParam("seenFault"));
    EXPECT_TRUE(int_av->gotParam("seenAlarm"));
    EXPECT_TRUE(int_av->gotParam("seenReset"));
    EXPECT_TRUE(int_av->gotParam("numConditionVars"));

    // Check param values on startup
    EXPECT_FALSE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTimeout"));
    EXPECT_EQ(7, int_av->getdParam("FaultTimeout"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTimeout"));
    EXPECT_STREQ("int", int_av->getcParam("Type"));    // Default type is int if not defined in config
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_EQ(0, int_av->getiParam("numConditionVars"));
}

// Test if the alarm/fault/reset time are not changed and the alarm/fault/reset status flags are not changed
// when condition check is disabled
TEST_F(ConditionMonitorTest, condition_monitor_check_disabled) {
    // Check current state of condition params
    EXPECT_FALSE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    // Check current state of condition params now
    EXPECT_FALSE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
}

// Test invalid case where the # of expected vals is incorrect
TEST_F(ConditionMonitorTest, condition_monitor_num_expected_vals_invalid) {

    // Test numExpectedVals < 0
    int_av->setParam("numExpectedVals", -1);
    int_av->setParam("expectedVal1", 2);
    int_av->setParam("Type", (char*)"int");

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("int", int_av->getcParam("Type"));
    EXPECT_EQ(-1, int_av->getiParam("numExpectedVals"));
    EXPECT_EQ(2, int_av->getiParam("expectedVal1"));

    // Run condition monitor and check the current state of condition params now
    // Note: alarm/fault/reset time and state should not change
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Test if numExpectedVals > actual # of expected vals
    int_av->setParam("numExpectedVals", 3);
    int_av->setParam("expectedVal1", 2);
    int_av->setParam("expectedVal2", 5);

    // Run condition monitor and check the current state of condition params now
    // Note: at the moment, alarm/fault time will decrement
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_GT(5, int_av->getdParam("AlarmTime"));
    EXPECT_GT(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
}


// Test if the alarm/fault time and flag are changed when the value != expected value(s) with no conditions
// Also test if the reset time and flag are changed when the value == expected value(s) with no conditions
// assetVar type is int
TEST_F(ConditionMonitorTest, condition_monitor_int_alarm_fault_reset_state) {
    
    // Add expected value and type for comparison test
    int_av->setParam("numExpectedVals", 1);
    int_av->setParam("expectedVal1", 2);
    int_av->setParam("Type", (char*)"int");

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("int", int_av->getcParam("Type"));
    EXPECT_EQ(1, int_av->getiParam("numExpectedVals"));
    EXPECT_EQ(2, int_av->getiParam("expectedVal1"));

    // Run condition monitor and check the current state of condition params now
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_GT(5, int_av->getdParam("AlarmTime"));   // Alarm time should be decremented
    EXPECT_GT(7, int_av->getdParam("FaultTime"));   // Fault time should be decremented
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Set alarm time to very low number and run condition monitor again
    // This time, we'll expect alarm state
    int_av->setParam("AlarmTime", 0.00001);
    EXPECT_EQ(0.00001, int_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(0, int_av->getdParam("AlarmTime"));   // Alarm time should be 0
    EXPECT_GT(7, int_av->getdParam("FaultTime"));   // Fault time should be decremented
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_TRUE(int_av->getbParam("seenAlarm"));    // Alarm state should be true
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Set fault time to very low number and run condition monitor again
    // This time, we'll expect fault state
    int_av->setParam("FaultTime", 0.00001);
    EXPECT_EQ(0.00001, int_av->getdParam("FaultTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(0, int_av->getdParam("AlarmTime"));   // Alarm time should still be 0
    EXPECT_EQ(0, int_av->getdParam("FaultTime"));   // Fault time should be 0
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_TRUE(int_av->getbParam("seenFault"));    // Fault state should be true
    EXPECT_TRUE(int_av->getbParam("seenAlarm"));    // Alarm state should still be true
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Set the current value to be equal to the expected value
    int_av->setVal(2);
    EXPECT_EQ(int_av->getiParam("expectedVal1"), int_av->getiVal());

    // Run condition monitor and check the current state of condition params now
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_LT(0, int_av->getdParam("AlarmTime"));   // Alarm time should be incremented
    EXPECT_LT(0, int_av->getdParam("FaultTime"));   // Fault time should be incremented
    EXPECT_GT(4, int_av->getdParam("RecoverTime")); // Recover time should be decremented
    EXPECT_TRUE(int_av->getbParam("seenFault"));    // Fault state should still be true
    EXPECT_TRUE(int_av->getbParam("seenAlarm"));    // Alarm state should still be true
    EXPECT_FALSE(int_av->getbParam("seenReset"));   // Reset state should be false

    // Set fault/alarm time to config time and run condition monitor again
    int_av->setParam("FaultTime", int_av->getdParam("FaultTimeout") - 0.00001);
    int_av->setParam("AlarmTime", int_av->getdParam("AlarmTimeout") - 0.00001);
    EXPECT_EQ(int_av->getdParam("FaultTimeout") - 0.00001, int_av->getdParam("FaultTime"));
    EXPECT_EQ(int_av->getdParam("AlarmTimeout") - 0.00001, int_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));   // Alarm time should be equal to alarm time config
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));   // Fault time should be equal to fault time config

    // Set recover time to very low number and run condition monitor again
    // This time, we'll expect reset state. Fault/alarm states should be set to false
    int_av->setParam("RecoverTime", 0.00001);
    EXPECT_EQ(0.00001, int_av->getdParam("RecoverTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));   // Alarm time should still be equal to alarm time config
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));   // Fault time should still be equal to fault time config
    EXPECT_EQ(0, int_av->getdParam("RecoverTime")); // Recover time should be 0
    EXPECT_FALSE(int_av->getbParam("seenFault"));   // Fault state should be false
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));   // Alarm state should be false
    EXPECT_TRUE(int_av->getbParam("seenReset"));    // Reset state should be true
}

// Test if the alarm/fault time and flag are changed when the value != expected value(s) with no conditions
// Also test if the reset time and flag are changed when the value == expected value(s) with no conditions
// assetVar type is double
TEST_F(ConditionMonitorTest, condition_monitor_dbl_alarm_fault_reset_state) {

    // Add expected value and type for comparison test
    dbl_av->setParam("numExpectedVals", 1);
    dbl_av->setParam("expectedVal1", 0.001);
    dbl_av->setParam("Type", (char*)"double");

    dbl_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(dbl_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, dbl_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, dbl_av->getdParam("FaultTime"));
    EXPECT_EQ(4, dbl_av->getdParam("RecoverTime"));
    EXPECT_FALSE(dbl_av->getbParam("seenFault"));
    EXPECT_FALSE(dbl_av->getbParam("seenAlarm"));
    EXPECT_FALSE(dbl_av->getbParam("seenReset"));
    EXPECT_STREQ("double", dbl_av->getcParam("Type"));
    EXPECT_EQ(1, dbl_av->getiParam("numExpectedVals"));
    EXPECT_EQ(0.001, dbl_av->getdParam("expectedVal1"));

    // Run condition monitor and check the current state of condition params now
    CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
    EXPECT_GT(5, dbl_av->getdParam("AlarmTime"));   // Alarm time should be decremented
    EXPECT_GT(7, dbl_av->getdParam("FaultTime"));   // Fault time should be decremented
    EXPECT_EQ(4, dbl_av->getdParam("RecoverTime"));
    EXPECT_FALSE(dbl_av->getbParam("seenFault"));
    EXPECT_FALSE(dbl_av->getbParam("seenAlarm"));
    EXPECT_FALSE(dbl_av->getbParam("seenReset"));

    // Set alarm time to very low number and run condition monitor again
    // This time, we'll expect alarm state
    dbl_av->setParam("AlarmTime", 0.00001);
    EXPECT_EQ(0.00001, dbl_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
    EXPECT_EQ(0, dbl_av->getdParam("AlarmTime"));   // Alarm time should be 0
    EXPECT_GT(7, dbl_av->getdParam("RecoverTime"));
    EXPECT_FALSE(dbl_av->getbParam("seenFault"));
    EXPECT_TRUE(dbl_av->getbParam("seenAlarm"));    // Alarm state should be true
    EXPECT_FALSE(dbl_av->getbParam("seenReset"));

    // Set fault time to very low number and run condition monitor again
    // This time, we'll expect fault state
    dbl_av->setParam("FaultTime", 0.00001);
    EXPECT_EQ(0.00001, dbl_av->getdParam("FaultTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
    EXPECT_EQ(0, dbl_av->getdParam("AlarmTime"));   // Alarm time should be 0
    EXPECT_EQ(0, dbl_av->getdParam("FaultTime"));   // Fault time should be 0
    EXPECT_EQ(4, dbl_av->getdParam("RecoverTime"));
    EXPECT_TRUE(dbl_av->getbParam("seenFault"));    // Fault state should be true
    EXPECT_TRUE(dbl_av->getbParam("seenAlarm"));    // Alarm state should be true
    EXPECT_FALSE(dbl_av->getbParam("seenReset"));

    // Set the current value to be equal to the expected value
    dbl_av->setVal(0.001);
    EXPECT_EQ(dbl_av->getdParam("expectedVal1"), dbl_av->getdVal());

    // Run condition monitor and check the current state of condition params now
    CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
    EXPECT_LT(0, dbl_av->getdParam("AlarmTime"));   // Alarm time should be incremented
    EXPECT_LT(0, dbl_av->getdParam("FaultTime"));   // Fault time should be incremented
    EXPECT_GT(4, dbl_av->getdParam("RecoverTime")); // Recover time should be decremented
    EXPECT_TRUE(dbl_av->getbParam("seenFault"));    // Fault state should still be true
    EXPECT_TRUE(dbl_av->getbParam("seenAlarm"));    // Alarm state should still be true
    EXPECT_FALSE(dbl_av->getbParam("seenReset"));   // Reset state should be false

    // Set fault/alarm time to config time and run condition monitor again
    dbl_av->setParam("FaultTime", dbl_av->getdParam("FaultTimeout") - 0.00001);
    dbl_av->setParam("AlarmTime", dbl_av->getdParam("AlarmTimeout") - 0.00001);
    EXPECT_EQ(dbl_av->getdParam("FaultTimeout") - 0.00001, dbl_av->getdParam("FaultTime"));
    EXPECT_EQ(dbl_av->getdParam("AlarmTimeout") - 0.00001, dbl_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
    EXPECT_EQ(5, dbl_av->getdParam("AlarmTime"));   // Alarm time should be equal to alarm time config
    EXPECT_EQ(7, dbl_av->getdParam("FaultTime"));   // Fault time should be equal to fault time config

    // Set recover time to very low number and run condition monitor again
    // This time, we'll expect reset state. Fault/alarm states should be set to false
    dbl_av->setParam("RecoverTime", 0.00001);
    EXPECT_EQ(0.00001, dbl_av->getdParam("RecoverTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, dbl_av);
    EXPECT_EQ(5, dbl_av->getdParam("AlarmTime"));   // Alarm time should still be equal to alarm time config
    EXPECT_EQ(7, dbl_av->getdParam("FaultTime"));   // Fault time should still be equal to fault time config
    EXPECT_EQ(0, dbl_av->getdParam("RecoverTime")); // Recover time should be 0
    EXPECT_FALSE(dbl_av->getbParam("seenFault"));   // Fault state should be false
    EXPECT_FALSE(dbl_av->getbParam("seenAlarm"));   // Alarm state should be false
    EXPECT_TRUE(dbl_av->getbParam("seenReset"));    // Reset state should be true
}

// Test if the alarm/fault time and flag are changed when the value != expected value(s) with no conditions
// Also test if the reset time and flag are changed when the value == expected value(s) with no conditions
// assetVar type is string
TEST_F(ConditionMonitorTest, condition_monitor_str_alarm_fault_reset_state) {
    // Add expected value and type for comparison test
    str_av->setParam("numExpectedVals", 1);
    str_av->setParam("expectedVal1", (char*)"Ready");
    str_av->setParam("Type", (char*)"string");

    str_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(str_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, str_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, str_av->getdParam("FaultTime"));
    EXPECT_EQ(4, str_av->getdParam("RecoverTime"));
    EXPECT_FALSE(str_av->getbParam("seenFault"));
    EXPECT_FALSE(str_av->getbParam("seenAlarm"));
    EXPECT_FALSE(str_av->getbParam("seenReset"));
    EXPECT_STREQ("string", str_av->getcParam("Type"));
    EXPECT_EQ(1, str_av->getiParam("numExpectedVals"));
    EXPECT_STREQ("Ready", str_av->getcParam("expectedVal1"));

    // Run condition monitor and check the current state of condition params now
    CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    EXPECT_GT(5, str_av->getdParam("AlarmTime"));   // Alarm time should be decremented
    EXPECT_GT(7, str_av->getdParam("FaultTime"));   // Fault time should be decremented
    EXPECT_EQ(4, str_av->getdParam("RecoverTime"));
    EXPECT_FALSE(str_av->getbParam("seenFault"));
    EXPECT_FALSE(str_av->getbParam("seenAlarm"));
    EXPECT_FALSE(str_av->getbParam("seenReset"));

    // Set alarm time to very low number and run condition monitor again
    // This time, we'll expect alarm state
    str_av->setParam("AlarmTime", 0.00001);
    EXPECT_EQ(0.00001, str_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    EXPECT_EQ(0, str_av->getdParam("AlarmTime"));   // Alarm time should be 0
    EXPECT_GT(7, str_av->getdParam("RecoverTime"));
    EXPECT_FALSE(str_av->getbParam("seenFault"));
    EXPECT_TRUE(str_av->getbParam("seenAlarm"));    // Alarm state should be true
    EXPECT_FALSE(str_av->getbParam("seenReset"));

    // Set fault time to very low number and run condition monitor again
    // This time, we'll expect fault state
    str_av->setParam("FaultTime", 0.00001);
    EXPECT_EQ(0.00001, str_av->getdParam("FaultTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    EXPECT_EQ(0, str_av->getdParam("AlarmTime"));   // Alarm time should be 0
    EXPECT_EQ(0, str_av->getdParam("FaultTime"));   // Fault time should be 0
    EXPECT_EQ(4, str_av->getdParam("RecoverTime"));
    EXPECT_TRUE(str_av->getbParam("seenFault"));    // Fault state should be true
    EXPECT_TRUE(str_av->getbParam("seenAlarm"));    // Alarm state should be true
    EXPECT_FALSE(str_av->getbParam("seenReset"));

    // Set the current value to be equal to the expected value
    str_av->setVal((char*)"Ready");
    EXPECT_STREQ(str_av->getcParam("expectedVal1"), str_av->getcVal());

    // Run condition monitor and check the current state of condition params now
    CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    EXPECT_LT(0, str_av->getdParam("AlarmTime"));   // Alarm time should be incremented
    EXPECT_LT(0, str_av->getdParam("FaultTime"));   // Fault time should be incremented
    EXPECT_GT(4, str_av->getdParam("RecoverTime")); // Recover time should be decremented
    EXPECT_TRUE(str_av->getbParam("seenFault"));    // Fault state should still be true
    EXPECT_TRUE(str_av->getbParam("seenAlarm"));    // Alarm state should still be true
    EXPECT_FALSE(str_av->getbParam("seenReset"));   // Reset state should be false

    // Set fault/alarm time to config time and run condition monitor again
    str_av->setParam("FaultTime", str_av->getdParam("FaultTimeout") - 0.00001);
    str_av->setParam("AlarmTime", str_av->getdParam("AlarmTimeout") - 0.00001);
    EXPECT_EQ(str_av->getdParam("FaultTimeout") - 0.00001, str_av->getdParam("FaultTime"));
    EXPECT_EQ(str_av->getdParam("AlarmTimeout") - 0.00001, str_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    EXPECT_EQ(5, str_av->getdParam("AlarmTime"));   // Alarm time should be equal to alarm time config
    EXPECT_EQ(7, str_av->getdParam("FaultTime"));   // Fault time should be equal to fault time config

    // Set recover time to very low number and run condition monitor again
    // This time, we'll expect reset state. Fault/alarm states should be set to false
    str_av->setParam("RecoverTime", 0.00001);
    EXPECT_EQ(0.00001, str_av->getdParam("RecoverTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, str_av);
    EXPECT_EQ(5, str_av->getdParam("AlarmTime"));   // Alarm time should still be equal to alarm time config
    EXPECT_EQ(7, str_av->getdParam("FaultTime"));   // Fault time should still be equal to fault time config
    EXPECT_EQ(0, str_av->getdParam("RecoverTime")); // Recover time should be 0
    EXPECT_FALSE(str_av->getbParam("seenFault"));   // Fault state should be false
    EXPECT_FALSE(str_av->getbParam("seenAlarm"));   // Alarm state should be false
    EXPECT_TRUE(str_av->getbParam("seenReset"));    // Reset state should be true
}

// Test if alarm/fault time and flag are changed when there are more than one expected value
// Also test if reset time and flag are changed when there are more than one expected value
TEST_F(ConditionMonitorTest, condition_monitor_alarm_fault_reset_state_multiple_exp_vals) {
    // Add expected value and type for comparison test
    int_av->setParam("numExpectedVals", 3);
    int_av->setParam("expectedVal1", 0);
    int_av->setParam("expectedVal2", 5);
    int_av->setParam("expectedVal3", 7);
    int_av->setParam("Type", (char*)"int");

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("int", int_av->getcParam("Type"));
    EXPECT_EQ(3, int_av->getiParam("numExpectedVals"));
    EXPECT_EQ(0, int_av->getiParam("expectedVal1"));
    EXPECT_EQ(5, int_av->getiParam("expectedVal2"));
    EXPECT_EQ(7, int_av->getiParam("expectedVal3"));

    // Run condition monitor and check the current state of condition params now
    // Since current value == expectedVal1, we shouldn't expect alarm/fault time and state to be changed
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));   // Alarm time should not change
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));   // Fault time should not change
    EXPECT_GT(4, int_av->getdParam("RecoverTime")); // Recover time should be decremented
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Setting current value = expectedVal2 should also not affect the alarm/fault time and state
    int_av->setVal(5);
    EXPECT_EQ(int_av->getiParam("expectedVal2"), int_av->getiVal());
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));   // Alarm time should not change
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));   // Fault time should not change
    EXPECT_GT(4, int_av->getdParam("RecoverTime")); // Recover time should be decremented
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Set current value to be different from the expected value(s)
    // This should decrement alarm/fault time
    int_av->setVal(-1);
    int_av->setParam("FaultTime", 0.00001);
    int_av->setParam("AlarmTime", 0.00001);
    EXPECT_EQ(0.00001, int_av->getdParam("FaultTime"));
    EXPECT_EQ(0.00001, int_av->getdParam("AlarmTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(0, int_av->getdParam("AlarmTime"));   // Alarm time should be 0
    EXPECT_EQ(0, int_av->getdParam("FaultTime"));   // Fault time should be 0
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_TRUE(int_av->getbParam("seenFault"));    // Fault state should be true
    EXPECT_TRUE(int_av->getbParam("seenAlarm"));    // Alarm state should be true
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Set current value == expectedVal3 (one of the expected values)
    // This should clear alarm/fault state
    int_av->setVal(7);
    int_av->setParam("RecoverTime", 0.00001);
    EXPECT_EQ(0.00001, int_av->getdParam("RecoverTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(0, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));   // Fault state should be false
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));   // Alarm state should be false
    EXPECT_TRUE(int_av->getbParam("seenReset"));    // Reset state should be true
}


/**
 *********************************************************************************
 * Test cases with conditional variables and values
 * 
 * *******************************************************************************
 */
// Test invalid case where the # of condition vars is incorrect
TEST_F(ConditionMonitorTest, condition_monitor_num_condition_vars_invalid) {
    // Test numConditionVars < 0
    int_av->setParam("numConditionVars", -1);
    int_av->setParam("conditionVar1", (char*)"condVar1");
    int_av->setParam("conditionType1", (char*)"int");

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("int", int_av->getcParam("conditionType1"));
    EXPECT_EQ(-1, int_av->getiParam("numConditionVars"));
    EXPECT_STREQ("condVar1", int_av->getcParam("conditionVar1"));

    // Run condition monitor and check the current state of condition params now
    // Note: alarm/fault/reset time and state should not change
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Test numConditionVars > actual # of condition vars
    int_av->setParam("numConditionVars", 2);
    int_av->setParam("conditionVar1", (char*)"condVar1");

    // Reload to create newly added params
    amap["testConditionVar_int_reload"]->setVal(0);

    // Run condition monitor and check the current state of condition params now
    // Note: alarm/fault time should not change since num condition vars is > 0
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_TRUE(amap["condVar1"]);  // Make sure condition var exists

}

// Test invalid case where the # of condition vals is incorrect
TEST_F(ConditionMonitorTest, condition_monitor_num_condition_vals_invalid) {
    int_av->setParam("numConditionVars", 1);
    int_av->setParam("conditionVar1", (char*)"condVar1");
    int_av->setParam("conditionType1", (char*)"int");

    // Reload to create newly added params
    amap["testConditionVar_int_reload"]->setVal(0);

    // Test test numConditions < 0
    int_av->setParam("numConditions1", -1);
    int_av->setParam("conditionVal1_1", 10);

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("int", int_av->getcParam("conditionType1"));
    EXPECT_EQ(1, int_av->getiParam("numConditionVars"));
    EXPECT_STREQ("condVar1", int_av->getcParam("conditionVar1"));
    EXPECT_EQ(-1, int_av->getiParam("numConditions1"));
    EXPECT_EQ(10, int_av->getiParam("conditionVal1_1"));

    // Run condition monitor and check the current state of condition params now
    // Note: alarm/fault/reset time and state should not change
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Test numConditions > actual # of conditions
    int_av->setParam("numConditions", 2);

    // Reload to create newly added params
    amap["testConditionVar_int_reload"]->setVal(0);

    // Run condition monitor and check the current state of condition params now
    // Note: alarm/fault time should not change since numConditionVars > 0
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
}

// Test invalid case where the condition type is incorrect
TEST_F(ConditionMonitorTest, condition_monitor_condition_type_invalid) {
    int_av->setParam("numConditionVars", 1);
    int_av->setParam("conditionVar1", (char*)"condVar1");
    int_av->setParam("numConditions1", 1);
    int_av->setParam("conditionVal1_1", 10);
    int_av->setParam("conditionType1", (char*)"n/a");

    // Reload to create newly added params
    amap["testConditionVar_int_reload"]->setVal(0);

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("n/a", int_av->getcParam("conditionType1"));
    EXPECT_EQ(1, int_av->getiParam("numConditionVars"));
    EXPECT_STREQ("condVar1", int_av->getcParam("conditionVar1"));
    EXPECT_EQ(1, int_av->getiParam("numConditions1"));
    EXPECT_EQ(10, int_av->getiParam("conditionVal1_1"));

    // Run condition monitor and check the current state of condition params now
    // Note: alarm/fault/reset time and state should not change
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
}


// Test if the alarm/fault time and flag are changed when the value != expected value(s) with conditions
// Also test if the reset time and flag are changed when the value == expected value(s) with conditions
TEST_F(ConditionMonitorTest, condition_monitor_alarm_fault_reset_state_with_cond) {
    // Add expected value and type for comparison test
    int_av->setParam("numExpectedVals", 1);
    int_av->setParam("expectedVal1", 2);
    int_av->setParam("Type", (char*)"int");

    // Add condition var, vals, and type for comparison test
    int_av->setParam("numConditionVars", 2);
    int_av->setParam("conditionVar1", (char*)"condVar1");
    int_av->setParam("conditionType1", (char*)"int");
    int_av->setParam("numConditions1", 1);
    int_av->setParam("conditionVal1_1", 10);

    int_av->setParam("conditionVar2", (char*)"condVar2");
    int_av->setParam("conditionType2", (char*)"string");
    int_av->setParam("numConditions2", 2);
    int_av->setParam("conditionVal1_2", (char*)"Ready");
    int_av->setParam("conditionVal2_2", (char*)"Stop");
    
    // Add condition var and reload to create newly added params
    amap["testConditionVar_int_reload"]->setVal(0);
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    int_av->setParam("EnableConditionCheck", true);
    // Check current state of condition params
    EXPECT_TRUE(int_av->getbParam("EnableConditionCheck"));
    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));
    EXPECT_STREQ("int", int_av->getcParam("Type"));
    EXPECT_EQ(1, int_av->getiParam("numExpectedVals"));
    EXPECT_EQ(2, int_av->getiParam("expectedVal1"));

    EXPECT_EQ(2, int_av->getiParam("numConditionVars"));
    EXPECT_TRUE(amap["condVar1"]);
    EXPECT_TRUE(amap["condVar2"]);
    EXPECT_EQ(1, int_av->getiParam("numConditions1"));
    EXPECT_EQ(2, int_av->getiParam("numConditions2"));
    EXPECT_STREQ("int", int_av->getcParam("conditionType1"));
    EXPECT_STREQ("string", int_av->getcParam("conditionType2"));
    EXPECT_EQ(10, int_av->getiParam("conditionVal1_1"));
    EXPECT_STREQ("Ready", int_av->getcParam("conditionVal1_2"));
    EXPECT_STREQ("Stop", int_av->getcParam("conditionVal2_2"));

    // Test case - value == expected value, but current condition value != condition(s)
    // Expected behavior: reset time and state are not changed
    int_av->setVal(2);
    EXPECT_EQ(int_av->getiParam("expectedVal1"), int_av->getiVal());
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));  // Recover time should not be changed
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_FALSE(int_av->getbParam("seenReset"));    // Reset state should not be changed

    // Test case - value != expected value, but current condition value != condition(s)
    // Expected behavior: alarm/fault time and state are not changed
    int_av->setVal(1);
    EXPECT_NE(int_av->getiParam("expectedVal1"), int_av->getiVal());
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));    // Alarm time should not be changed
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));    // Fault time should not be changed
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));  
    EXPECT_FALSE(int_av->getbParam("seenFault"));    // Fault state should not be changed
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));    // Alarm state should not be changed
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Test case - value != expected value and current condition value == one, but not all, conditions
    // Expected behavior - alarm/fault time and state are not changed
    int_av->setVal(0);
    amap["condVar1"]->setVal(10);
    EXPECT_NE(int_av->getiParam("expectedVal1"), int_av->getiVal());
    EXPECT_EQ(int_av->getiParam("conditionVal1_1"), amap["condVar1"]->getiVal());
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));    // Alarm time should not be changed
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));    // Fault time should not be changed
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));  
    EXPECT_FALSE(int_av->getbParam("seenFault"));    // Fault state should not be changed
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));    // Alarm state should not be changed
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Test case - value == expected value and current condition value == one, but not all, conditions
    // Expected behavior - reset time and state are not changed
    int_av->setVal(2);
    amap["condVar1"]->setVal(10);
    EXPECT_EQ(int_av->getiParam("expectedVal1"), int_av->getiVal());
    EXPECT_EQ(int_av->getiParam("conditionVal1_1"), amap["condVar1"]->getiVal());
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    EXPECT_EQ(5, int_av->getdParam("AlarmTime"));    
    EXPECT_EQ(7, int_av->getdParam("FaultTime"));  
    EXPECT_EQ(4, int_av->getdParam("RecoverTime"));  // Recover time should not be changed
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));    
    EXPECT_FALSE(int_av->getbParam("seenReset"));    // Reset state should not be changed

    // Test case - value != expected value and all conditions are met
    // Expected behavior - reset time should be incremented. alarm/fault time should be decremented
    int_av->setVal(20);
    amap["condVar1"]->setVal(10);
    amap["condVar2"]->setVal((char*)"Stop");
    int_av->setParam("AlarmTime", 0.00001);
    int_av->setParam("FaultTime", 0.00001);
    int_av->setParam("RecoverTime", 0.00001);
    EXPECT_NE(int_av->getiParam("expectedVal1"), int_av->getiVal());
    EXPECT_EQ(int_av->getiParam("conditionVal1_1"), amap["condVar1"]->getiVal());
    EXPECT_STREQ(int_av->getcParam("conditionVal2_2"), amap["condVar2"]->getcVal());
    EXPECT_EQ(0.00001, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(0.00001, int_av->getdParam("FaultTime"));
    EXPECT_EQ(0.00001, int_av->getdParam("RecoverTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    EXPECT_EQ(0, int_av->getdParam("AlarmTime"));          // Alarm time should be decremented
    EXPECT_EQ(0, int_av->getdParam("FaultTime"));          // Fault time should be decremented
    EXPECT_LT(0.00001, int_av->getdParam("RecoverTime"));  // Recover time should be incremented
    EXPECT_TRUE(int_av->getbParam("seenFault"));           // Fault state should be set to true
    EXPECT_TRUE(int_av->getbParam("seenAlarm"));           // Alarm state should be set to true
    EXPECT_FALSE(int_av->getbParam("seenReset"));

    // Test case - value == expected value and all conditions are met
    // Expected behavior - reset time should be decremented. alarm/fault time should be incremented
    int_av->setVal(2);
    amap["condVar1"]->setVal(10);
    amap["condVar2"]->setVal((char*)"Ready");
    int_av->setParam("AlarmTime", 0.00001);
    int_av->setParam("FaultTime", 0.00001);
    int_av->setParam("RecoverTime", 0.00001);
    EXPECT_EQ(int_av->getiParam("expectedVal1"), int_av->getiVal());
    EXPECT_EQ(int_av->getiParam("conditionVal1_1"), amap["condVar1"]->getiVal());
    EXPECT_STREQ(int_av->getcParam("conditionVal1_2"), amap["condVar2"]->getcVal());
    EXPECT_EQ(0.00001, int_av->getdParam("AlarmTime"));
    EXPECT_EQ(0.00001, int_av->getdParam("FaultTime"));
    EXPECT_EQ(0.00001, int_av->getdParam("RecoverTime"));
    CheckMonitorVar(vmap, amap, "bms", nullptr, int_av);

    EXPECT_LT(0.00001, int_av->getdParam("AlarmTime"));  // Alarm time should be incremented
    EXPECT_LT(0.00001, int_av->getdParam("FaultTime"));  // Fault time should be incremented
    EXPECT_EQ(0, int_av->getdParam("RecoverTime"));      // Recover time should be decremented
    EXPECT_FALSE(int_av->getbParam("seenFault"));
    EXPECT_FALSE(int_av->getbParam("seenAlarm"));
    EXPECT_TRUE(int_av->getbParam("seenReset"));        // Reset state should be set to true
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}