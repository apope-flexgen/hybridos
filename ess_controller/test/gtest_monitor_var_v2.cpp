#include "gtest/gtest.h"
#include "../funcs/CheckMonitorVar_v2.cpp"
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
class CheckMonitorVar_v2Test : public ::testing::Test {
protected:
    virtual void SetUp() {

        // Configure fims object and connection
        if (!(p_fims = new fims())) {
            FPS_ERROR_PRINT("Failed to initialize fims class.\n");
            FAIL();
        }
        if (!p_fims->Connect((char *)"CheckMonitorVar_v2Test")) {
            FPS_ERROR_PRINT("Failed to connect to fims server.\n");
            FAIL();
        }

        // Set up parameters used in monitoring function
        const char* val = R"(
            {
                "value": 0.0,
                "alarmTimeout": 5,
                "faultTimeout": 10,
                "resetTimeout": 7
            })";

        cJSON* cjval = cJSON_Parse(val);
        av = vm.setValfromCj(vmap, "/status/bms", "NumRacksInService", cjval);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        av->am = bms_man;

        cJSON_Delete(cjval);

        CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    }

    virtual void TearDown() {
        p_fims->Close();
        delete p_fims;
        delete av->am;
    }

    varsmap vmap;       // main data map
    varmap amap;        // map of local variables for asset
    fims* p_fims;       // fims object
    VarMapUtils vm;     // map utils factory
    assetVar* av;       // assetVar to hold calcuation results
};

// Tests if parameters exist and contain the right values
TEST_F(CheckMonitorVar_v2Test, monitor_var_params_valid) {
    EXPECT_FALSE(av->getbParam("enableMonitor"));
    EXPECT_TRUE(av->getbParam("enableFault"));
    EXPECT_TRUE(av->getbParam("enableAlert"));
    EXPECT_TRUE(av->getbParam("includeCurrVal"));

    EXPECT_EQ(5, av->getdParam("alarmTimeout"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("faultTimeout"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("resetTimeout"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    EXPECT_STREQ("n/a", av->getcParam("alarmCondition"));
    EXPECT_STREQ("n/a", av->getcParam("faultCondition"));
    EXPECT_STREQ("n/a", av->getcParam("resetCondition"));

    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
}

// Tests no monitoring check when enableMonitor is disabled
TEST_F(CheckMonitorVar_v2Test, monitor_var_check_disabled) {

    // Initialize alarm/fault/reset conditions
    av->setParam("alarmCondition", (char*)"{1} < 5");
    av->setParam("faultCondition", (char*)"{1} < 2");
    av->setParam("resetCondition", (char*)"{1} >= 5");
    EXPECT_STREQ("{1} < 5", av->getcParam("alarmCondition"));
    EXPECT_STREQ("{1} < 2", av->getcParam("faultCondition"));
    EXPECT_STREQ("{1} >= 5", av->getcParam("resetCondition"));

    // Initialize/check remaining params
    EXPECT_FALSE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Run monitor function and check results. Should not expect alarm/fault/reset state to be changed
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);

    EXPECT_FALSE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Set current value and run monitor function again. Still should
    // not expect alarm/fault/reset state to be changed
    av->setVal(10);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);

    EXPECT_FALSE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
}

// Tests alarm state
TEST_F(CheckMonitorVar_v2Test, monitor_var_check_alarm_state) {
    
    // Initialize alarm/fault/reset conditions
    av->setParam("alarmCondition", (char*)"{1} < 5");
    av->setParam("faultCondition", (char*)"{1} < 2");
    av->setParam("resetCondition", (char*)"{1} >= 5");
    EXPECT_STREQ("{1} < 5", av->getcParam("alarmCondition"));
    EXPECT_STREQ("{1} < 2", av->getcParam("faultCondition"));
    EXPECT_STREQ("{1} >= 5", av->getcParam("resetCondition"));

    // Initialize/check remaining params
    av->setParam("enableMonitor", true);
    av->setParam("enableFault", false);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_FALSE(av->getbParam("enableFault"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Run monitor function and check results. Should expect alarm time to be decremented
    av->setVal(4);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_GT(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    
    // Set the current alarm time close to zero to enter alarm state after next function call
    av->setParam("currAlarmTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currAlarmTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));

    // Run monitor function again and check results. Should now expect alarm state to be active
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_TRUE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
}

// Tests alarm and fault state
TEST_F(CheckMonitorVar_v2Test, monitor_var_alarm_fault_state) {
    // Initialize alarm/fault/reset conditions
    av->setParam("alarmCondition", (char*)"{1} < 5");
    av->setParam("faultCondition", (char*)"{1} < 2");
    av->setParam("resetCondition", (char*)"{1} >= 5");
    EXPECT_STREQ("{1} < 5", av->getcParam("alarmCondition"));
    EXPECT_STREQ("{1} < 2", av->getcParam("faultCondition"));
    EXPECT_STREQ("{1} >= 5", av->getcParam("resetCondition"));

    // Initialize/check remaining params
    av->setParam("enableMonitor", true);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_TRUE(av->getbParam("enableFault"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Run monitor function and check results. Should expect alarm/fault time to be decremented
    av->setVal(1);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_GT(5, av->getdParam("currAlarmTime"));
    EXPECT_GT(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Set the current alarm/fault time close to zero to enter alarm/fault state after next function call
    av->setParam("currAlarmTime", 0.0000001);
    av->setParam("currFaultTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0.0000001, av->getdParam("currFaultTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0, av->getdParam("currFaultTime"));

    // Run monitor function again and check results. Should now expect alarm/fault state to be active
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_TRUE(av->getbParam("seenAlarm"));
    EXPECT_TRUE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
}

// Tests reset state - alarms/faults active
TEST_F(CheckMonitorVar_v2Test, monitor_var_reset_state) {
    
    // Initialize alarm/fault/reset conditions
    av->setParam("alarmCondition", (char*)"{1} < 5");
    av->setParam("faultCondition", (char*)"{1} < 2");
    av->setParam("resetCondition", (char*)"{1} >= 5");
    EXPECT_STREQ("{1} < 5", av->getcParam("alarmCondition"));
    EXPECT_STREQ("{1} < 2", av->getcParam("faultCondition"));
    EXPECT_STREQ("{1} >= 5", av->getcParam("resetCondition"));

    // Initialize/check remaining params
    av->setParam("enableMonitor", true);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_TRUE(av->getbParam("enableFault"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Run monitor function and check results. Should expect alarm/fault time to be decremented
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_GT(5, av->getdParam("currAlarmTime"));
    EXPECT_GT(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Set the current alarm/fault time close to zero to enter alarm/fault state after next function call
    av->setParam("currAlarmTime", 0.0000001);
    av->setParam("currFaultTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0.0000001, av->getdParam("currFaultTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0, av->getdParam("currFaultTime"));

    // Run monitor function again and check results. Should now expect alarm/fault state to be active
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_TRUE(av->getbParam("seenAlarm"));
    EXPECT_TRUE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Set monitoring variable to a value that meets the reset condition(s). Run monitor function again and check results
    // Should expect reset time to be decremented. Should expect alarm/fault time to be incremented
    av->setVal(5);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_TRUE(av->getbParam("seenAlarm"));
    EXPECT_TRUE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_LE(0, av->getdParam("currAlarmTime"));
    EXPECT_LE(0, av->getdParam("currFaultTime"));
    EXPECT_GE(7, av->getdParam("currResetTime"));

    // Set the current reset time close to zero to enter reset state after next function call
    av->setParam("currResetTime", 0.0000001);
    av->setParam("currAlarmTime", 4.9999999);
    av->setParam("currFaultTime", 9.9999999);
    EXPECT_EQ(0.0000001, av->getdParam("currResetTime"));
    EXPECT_EQ(4.9999999, av->getdParam("currAlarmTime"));
    EXPECT_EQ(9.9999999, av->getdParam("currFaultTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currResetTime"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));

    // Run monitor function again and check results. Should now expect alarm/fault state to be cleared and reset state to be active
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_TRUE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(0, av->getdParam("currResetTime"));
}

// Tests reset state - no alarms/faults active
TEST_F(CheckMonitorVar_v2Test, monitor_var_reset_state_no_alarms_faults) {
    
    // Initialize alarm/fault/reset conditions
    av->setParam("alarmCondition", (char*)"{1} < 5");
    av->setParam("faultCondition", (char*)"{1} < 2");
    av->setParam("resetCondition", (char*)"{1} >= 5");
    EXPECT_STREQ("{1} < 5", av->getcParam("alarmCondition"));
    EXPECT_STREQ("{1} < 2", av->getcParam("faultCondition"));
    EXPECT_STREQ("{1} >= 5", av->getcParam("resetCondition"));

    // Initialize/check remaining params
    av->setParam("enableMonitor", true);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_TRUE(av->getbParam("enableFault"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Set monitoring variable to a value that meets the reset condition(s). Run monitor function and check results
    // Should expect reset time to be decremented
    av->setVal(5);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_GE(7, av->getdParam("currResetTime"));

    // Set the current reset time close to zero to enter reset state after next function call
    av->setParam("currResetTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currResetTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currResetTime"));
    
    // Run monitor function again and check results. Should now expect reset state to be active
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_TRUE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(0, av->getdParam("currResetTime"));
}

// Tests no fault state when enableFault is disabled
TEST_F(CheckMonitorVar_v2Test, monitor_var_fault_disabled) {
    
    // Initialize alarm/fault/reset conditions
    av->setParam("alarmCondition", (char*)"{1} < 5");
    av->setParam("faultCondition", (char*)"{1} < 2");
    av->setParam("resetCondition", (char*)"{1} >= 5");
    EXPECT_STREQ("{1} < 5", av->getcParam("alarmCondition"));
    EXPECT_STREQ("{1} < 2", av->getcParam("faultCondition"));
    EXPECT_STREQ("{1} >= 5", av->getcParam("resetCondition"));

    // Initialize/check remaining params
    av->setParam("enableMonitor", true);
    av->setParam("enableFault", false);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableMonitor"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_FALSE(av->getbParam("enableFault"));
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Run monitor function and check results. Should expect alarm time to be decremented. Fault time should be unchanged
    av->setVal(1);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_GT(5, av->getdParam("currAlarmTime"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
    
    // Set the current alarm/fault time close to zero to enter alarm state after next function call
    av->setParam("currAlarmTime", 0.0000001);
    av->setParam("currFaultTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0.0000001, av->getdParam("currFaultTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0.0000001, av->getdParam("currFaultTime"));

    // Run monitor function again and check results. Should now expect alarm state to be active, but not fault state
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_TRUE(av->getbParam("seenAlarm"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(0, av->getdParam("currAlarmTime"));
    EXPECT_EQ(0.0000001, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
}

// Tests alarm and fault state (when enableComms is enabled)
TEST_F(CheckMonitorVar_v2Test, monitor_var_fault_state_comms_enabled) {

    // Initialize/check monitor params
    av->setParam("enableComms", true);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableComms"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
    amap["CheckMonitorVar_v2_NumRacksInService"]->setVal(1);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);

    // Set the current alarm/fault time close to zero to enter alarm/fault state after next function call
    av->setParam("currFaultTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currFaultTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currFaultTime"));

    // Check communication status variable
    assetVar* commsOK = amap["CommsOK"];
    ASSERT_FALSE(commsOK);

    // Run monitor function again and check results. Should now expect alarm/fault state to be active
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_TRUE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(0, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));

    // Check communication status variable
    commsOK = amap["CommsOK"];
    ASSERT_TRUE(commsOK);
    EXPECT_FALSE(commsOK->getbVal());
}

// Tests reset state (when enableComms is enabled)
TEST_F(CheckMonitorVar_v2Test, monitor_var_reset_state_comms_enabled) {

    // Initialize/check monitor params
    av->setParam("enableComms", true);
    av->setParam("enableAlert", false);
    EXPECT_TRUE(av->getbParam("enableComms"));
    EXPECT_FALSE(av->getbParam("enableAlert"));
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_FALSE(av->getbParam("seenReset"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(7, av->getdParam("currResetTime"));
    amap["CheckMonitorVar_v2_NumRacksInService"]->setVal(1);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);

    // Set the current reset time close to zero to enter reset state after next function call
    av->setVal(1);
    av->setParam("currResetTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currResetTime"));
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdParam("currResetTime"));

    // Check communication status variable
    assetVar* commsOK = amap["CommsOK"];
    ASSERT_FALSE(commsOK);

    // Run monitor function again and check results. Should now expect reset state to be active
    av->setVal(2);
    CheckMonitorVar_v2(vmap, amap, "bms", nullptr, av);
    EXPECT_FALSE(av->getbParam("seenFault"));
    EXPECT_TRUE(av->getbParam("seenReset"));
    EXPECT_EQ(10, av->getdParam("currFaultTime"));
    EXPECT_EQ(0, av->getdParam("currResetTime"));

    // Check communication status variable again
    commsOK = amap["CommsOK"];
    ASSERT_TRUE(commsOK);
    EXPECT_TRUE(commsOK->getbVal());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}