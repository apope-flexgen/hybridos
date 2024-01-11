#include "gtest/gtest.h"
#include "../funcs/HandleCmd.cpp"
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
class HandleCmdTest : public ::testing::Test {
protected:
    virtual void SetUp() {

        // Configure fims object and connection
        if (!(p_fims = new fims())) {
            FPS_ERROR_PRINT("Failed to initialize fims class.\n");
            FAIL();
        }
        if (!p_fims->Connect((char *)"HandleCmdTest")) {
            FPS_ERROR_PRINT("Failed to connect to fims server.\n");
            FAIL();
        }

        const char* val = R"({"value": 0, "numVars": 2, "cmdVar": "/components/bms:dc_contactor_controls", "variable1": "/status/bms:DCClosed", "variable2": "/status/bms:PowerStatus"})";
        cJSON* cjval = cJSON_Parse(val);
        av = vm.setValfromCj(vmap, "/controls/bms", "CloseDCContactors", cjval);

        double dval = 0.0;
        bool bval = false;
        const char* cval = "Off";
        vm.setVal(vmap, "/components/bms", "dc_contactor_controls", dval);
        vm.setVal(vmap, "/status/bms", "DCClosed", bval);
        vm.setVal(vmap, "/status/bms", "PowerStatus", cval);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        av->am = bms_man;

        cJSON_Delete(cjval);

        HandleCmd(vmap, amap, "bms", p_fims, av);
        av->setParam("enableAlert", false);
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
    assetVar* av;       // control assetVar
};

// Test if parameters exist and contain the right values
TEST_F(HandleCmdTest, handle_cmd_params_valid) {
    EXPECT_EQ(2, av->getiParam("numVars"));
    EXPECT_STREQ("/status/bms:DCClosed", av->getcParam("variable1"));
    EXPECT_STREQ("/status/bms:PowerStatus", av->getcParam("variable2"));
    EXPECT_STREQ("/components/bms:dc_contactor_controls", av->getcParam("cmdVar"));
    EXPECT_STREQ("n/a", av->getcParam("expression"));
    EXPECT_FALSE(av->getbParam("useExpr"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_EQ(-1, av->getiParam("lastCmdVal"));
    EXPECT_EQ(0, av->getiParam("maxCmdTries"));
    EXPECT_EQ(0, av->getiParam("currCmdTries"));
    EXPECT_EQ(0, av->getdParam("sendCmdHoldTimeout"));
    EXPECT_EQ(0, av->getdParam("currSendCmdHoldTime"));
    EXPECT_EQ(0, av->getdParam("checkCmdHoldTimeout"));
    EXPECT_EQ(0, av->getdParam("currCheckCmdHoldTime"));

    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);
    assetVar* var1   = vm.getVar(vmap, "/status/bms:DCClosed", nullptr);
    assetVar* var2   = vm.getVar(vmap, "/status/bms:PowerStatus", nullptr);
    ASSERT_TRUE(cmdVar);
    EXPECT_STREQ("dc_contactor_controls", cmdVar->name.c_str());
    EXPECT_EQ(0, cmdVar->getdVal());
    ASSERT_TRUE(var1);
    EXPECT_STREQ("DCClosed", var1->name.c_str());
    EXPECT_FALSE(var1->getbVal());
    ASSERT_TRUE(var2);
    EXPECT_STREQ("PowerStatus", var2->name.c_str());
    EXPECT_STREQ("Off", var2->getcVal());

}

// Tests send command skip if the number of variables to include in condition check is invalid
TEST_F(HandleCmdTest, handle_cmd_invalid_num_vars) {
    av->setParam("numVars", 0);
    EXPECT_EQ(0, av->getiParam("numVars"));
    EXPECT_EQ(0, av->getdVal());
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests successful send command operation with no conditional checks needed
TEST_F(HandleCmdTest, handle_cmd_send_cmd_success_no_conditions)
{
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);

    av->setParam("triggerCmd", true);
    EXPECT_TRUE(av->getbParam("triggerCmd"));

    av->setVal(1);

    // Run as is and check the results - cmdSent should be set to true, indicating that the command was sent successfully
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(1, cmdVar->getdVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));
}

// Tests successful send command operation with conditional checks
TEST_F(HandleCmdTest, handle_cmd_send_cmd_success_with_conditions)
{
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);
    assetVar* var1   = vm.getVar(vmap, "/status/bms:DCClosed", nullptr);
    assetVar* var2   = vm.getVar(vmap, "/status/bms:PowerStatus", nullptr);

    av->setParam("expression", (char*)"{1} and {2} == On");
    av->setParam("useExpr", true);
    av->setParam("triggerCmd", true);
    EXPECT_STREQ("{1} and {2} == On", av->getcParam("expression"));
    EXPECT_TRUE(av->getbParam("useExpr"));
    EXPECT_TRUE(av->getbParam("triggerCmd"));

    av->setVal(1);

    // Run as is and check the results
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(0, cmdVar->getdVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));

    // Conditions are satisfied - cmdSent should be set to true, indicating that the command was sent successfully
    var1->setVal(true);
    var2->setVal((char*)"On");
    EXPECT_TRUE(var1->getbVal());
    EXPECT_STREQ("On", var2->getcVal());
    EXPECT_EQ(1, av->getdVal());

    av->setParam("triggerCmd", true);
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(1, cmdVar->getdVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));
}

// Tests successful send command operation with hold timeout defined
TEST_F(HandleCmdTest, handle_cmd_send_cmd_success_with_hold_timeout)
{
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);

    av->setParam("sendCmdHoldTimeout", 5);
    av->setParam("currSendCmdHoldTime", 5);
    EXPECT_EQ(5, av->getdParam("sendCmdHoldTimeout"));
    EXPECT_EQ(5, av->getdParam("currSendCmdHoldTime"));

    av->setParam("triggerCmd", true);
    av->setVal(1);

    // Run as is and check the results - cmdSent should still be false, but the current send command hold time should be decremented
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(0, cmdVar->getdVal());
    EXPECT_TRUE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
    EXPECT_EQ(5, av->getdParam("sendCmdHoldTimeout"));
    EXPECT_GT(5, av->getdParam("currSendCmdHoldTime"));

    // Set the current send command hold time close to zero to trigger command send operation after next function call
    av->setParam("currSendCmdHoldTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currSendCmdHoldTime"));

    av->setParam("triggerCmd", true);
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(0, av->getdParam("currSendCmdHoldTime"));
    
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(1, cmdVar->getdVal());//
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_EQ(5, av->getdParam("sendCmdHoldTimeout"));
    EXPECT_EQ(5, av->getdParam("currSendCmdHoldTime"));
}

// Tests unsuccessful send command operation with cmd send timeout defined
TEST_F(HandleCmdTest, handle_cmd_send_cmd_failure)
{
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);

    av->setParam("sendCmdTimeout", 5);
    av->setParam("currSendCmdTime", 5);
    av->setParam("expression", (char*)"{1} and {2} == On");
    av->setParam("useExpr", true);
    av->setParam("triggerCmd", true);
    EXPECT_EQ(5, av->getdParam("sendCmdTimeout"));
    EXPECT_EQ(5, av->getdParam("currSendCmdTime"));
    EXPECT_STREQ("{1} and {2} == On", av->getcParam("expression"));
    EXPECT_TRUE(av->getbParam("useExpr"));
    EXPECT_TRUE(av->getbParam("triggerCmd"));

    av->setParam("triggerCmd", true);
    av->setVal(1);

    // Run send command operation and check the results - the current send command time should be decremented
    HandleCmd(vmap, amap, "bms", p_fims, av);

    assetVar* cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_FALSE(cmdSuccess);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(0, cmdVar->getdVal());
    EXPECT_TRUE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
    EXPECT_EQ(5, av->getdParam("sendCmdTimeout"));
    EXPECT_GT(5, av->getdParam("currSendCmdTime"));

    // Decrease the current send cmd timeout close to 0
    av->setParam("currSendCmdTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currSendCmdTime"));
    EXPECT_EQ(5, av->getdParam("sendCmdTimeout"));

    // Run check command operation again and check results. Command value should not be set to command variable since conditions are not satisfied after elapsed time
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(0, av->getdParam("currSendCmdTime"));

    HandleCmd(vmap, amap, "bms", p_fims, av);
    cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_TRUE(cmdSuccess);
    EXPECT_FALSE(cmdSuccess->getbVal());
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(0, cmdVar->getdVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
    EXPECT_EQ(5, av->getdParam("currSendCmdTime"));
}

// Tests successful check command operation
TEST_F(HandleCmdTest, handle_cmd_check_cmd_success)
{
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);

    av->setParam("triggerCmd", true);
    av->setVal(1);

    // Run send command operation first and check results
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(1, av->getdVal());
    EXPECT_EQ(1, cmdVar->getdVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));

    // Check command operation should run afterwards. Check results    
    assetVar* cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_TRUE(cmdSuccess);
    EXPECT_TRUE(cmdSuccess->getbVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
}

// Tests successful check command operation with hold timeout and cmd check timeout defined
TEST_F(HandleCmdTest, handle_cmd_check_cmd_success_with_timeouts)
{
    // Set hold and check cmd timeouts
    av->setParam("checkCmdHoldTimeout", 5);
    av->setParam("currCheckCmdHoldTime", 5);
    av->setParam("checkCmdTimeout", 5);
    av->setParam("currCheckCmdTime", 5);
    EXPECT_EQ(5, av->getdParam("checkCmdHoldTimeout"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(5, av->getdParam("checkCmdTimeout"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdTime"));

    av->setParam("triggerCmd", true);
    av->setVal(1);

    // Run send command operation first
    HandleCmd(vmap, amap, "bms", p_fims, av);

    // Check command operation should run next. Hold timeout should be decremented, but not reach 0 yet
    HandleCmd(vmap, amap, "bms", p_fims, av);
    
    assetVar* cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_FALSE(cmdSuccess);
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_TRUE(av->getbParam("cmdSent"));

    // Decrease the current check cmd hold timeout close to 0. This should allow check cmd operation to move on to actually checking if the cmd variable actually has the updated value
    av->setParam("currCheckCmdHoldTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(5, av->getdParam("checkCmdHoldTimeout"));
    EXPECT_EQ(5, av->getdParam("checkCmdTimeout"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdTime"));

    // Run check command operation again. cmdSuccess should exist and contain a value of true, indicating that the control variable is
    // confirmed to contain the updated command value
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(0, av->getdParam("currCheckCmdHoldTime"));
    
    HandleCmd(vmap, amap, "bms", p_fims, av);
    cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_TRUE(cmdSuccess);
    EXPECT_TRUE(cmdSuccess->getbVal());
    EXPECT_EQ(5, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdTime"));
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
}

// Tests unsuccessful check command operation with hold timeout and cmd check timeout defined
TEST_F(HandleCmdTest, handle_cmd_check_cmd_failure_with_timeouts)
{
    // Set hold and check cmd timeouts
    av->setParam("checkCmdHoldTimeout", 5);
    av->setParam("currCheckCmdHoldTime", 5);
    av->setParam("checkCmdTimeout", 5);
    av->setParam("currCheckCmdTime", 5);
    EXPECT_EQ(5, av->getdParam("checkCmdHoldTimeout"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(5, av->getdParam("checkCmdTimeout"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdTime"));

    av->setParam("triggerCmd", true);
    av->setVal(1);

    // Run send command operation first
    HandleCmd(vmap, amap, "bms", p_fims, av);

    // Set the command variable value to a different value to simluate that update did not go through
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);
    cmdVar->setVal(0);
    EXPECT_EQ(0, cmdVar->getdVal());

    // Check command operation should run next. Hold timeout should be decremented, but not reach 0 yet
    HandleCmd(vmap, amap, "bms", p_fims, av);
    
    assetVar* cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_FALSE(cmdSuccess);
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_TRUE(av->getbParam("cmdSent"));

    // Decrease the current check cmd hold timeout close to 0. This should allow check cmd operation to move on to actually checking if the cmd variable actually has the updated value
    av->setParam("currCheckCmdHoldTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(5, av->getdParam("checkCmdHoldTimeout"));
    EXPECT_EQ(5, av->getdParam("checkCmdTimeout"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdTime"));

    // Run check command operation again. Hold timeout should be set to 0, but the check cmd timeout should be decremented
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(0, av->getdParam("currCheckCmdHoldTime"));
        
    HandleCmd(vmap, amap, "bms", p_fims, av);
    cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_FALSE(cmdSuccess);
    EXPECT_EQ(0, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_GT(5, av->getdParam("currCheckCmdTime"));
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_TRUE(av->getbParam("cmdSent"));

    // Decrease the current check cmd timeout close to 0
    av->setParam("currCheckCmdTime", 0.0000001);
    EXPECT_EQ(0.0000001, av->getdParam("currCheckCmdTime"));
    EXPECT_EQ(5, av->getdParam("checkCmdTimeout"));

    // Run check command operation again. cmdSuccess should exist and contain a value of false, indicating that the control variable is
    // confirmed to not contain the updated command value
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_EQ(0, av->getdParam("currCheckCmdTime"));

    HandleCmd(vmap, amap, "bms", p_fims, av);
    cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_TRUE(cmdSuccess);
    EXPECT_FALSE(cmdSuccess->getbVal());
    EXPECT_EQ(5, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(5, av->getdParam("currCheckCmdTime"));
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
}

// Tests unsuccessful check command operation with # of send cmd tries defined
TEST_F(HandleCmdTest, handle_cmd_check_cmd_failure_with_tries)
{
    // Set hold and check cmd timeouts
    av->setParam("checkCmdHoldTimeout", 0.4);
    av->setParam("currCheckCmdHoldTime", 0.4);
    av->setParam("checkCmdTimeout", 0);
    av->setParam("currCheckCmdTime", 0);
    av->setParam("maxCmdTries", 2);
    EXPECT_EQ(0.4, av->getdParam("checkCmdHoldTimeout"));
    EXPECT_EQ(0.4, av->getdParam("currCheckCmdHoldTime"));
    EXPECT_EQ(0, av->getdParam("checkCmdTimeout"));
    EXPECT_EQ(0, av->getdParam("currCheckCmdTime"));
    EXPECT_EQ(2, av->getiParam("maxCmdTries"));
    EXPECT_EQ(0, av->getiParam("currCmdTries"));

    av->setParam("triggerCmd", true);
    av->setVal(1);

    // Run send command operation first
    HandleCmd(vmap, amap, "bms", p_fims, av);

    // Set the command variable value to a different value to simulate that update did not go through
    assetVar* cmdVar = vm.getVar(vmap, "/components/bms:dc_contactor_controls", nullptr);
    cmdVar->setVal(0);
    EXPECT_EQ(0, cmdVar->getdVal());

    // Run check command operation again. cmdSuccess should not exist yet since we have one send command try left
    av->setParam("currCheckCmdHoldTime", 0);
    HandleCmd(vmap, amap, "bms", p_fims, av);

    assetVar* cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_FALSE(cmdSuccess);
    EXPECT_TRUE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
    EXPECT_EQ(1, av->getiParam("currCmdTries"));

    // Send command operation should run first again
    HandleCmd(vmap, amap, "bms", p_fims, av);
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_TRUE(av->getbParam("cmdSent"));

    cmdVar->setVal(0);

    // Run check command operation one last time. This time, cmdSuccess should exist and have a value of false, 
    // indicating that the command update is unsuccessful
    av->setParam("currCheckCmdHoldTime", 0);
    HandleCmd(vmap, amap, "bms", p_fims, av);

    cmdSuccess = amap["CloseDCContactorsSuccess"];
    ASSERT_TRUE(cmdSuccess);
    EXPECT_FALSE(cmdSuccess->getbVal());
    EXPECT_FALSE(av->getbParam("triggerCmd"));
    EXPECT_FALSE(av->getbParam("cmdSent"));
    EXPECT_EQ(0, av->getiParam("currCmdTries"));

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}