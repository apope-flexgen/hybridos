#include "gtest/gtest.h"
#include "../funcs/CheckDbiVar.cpp"
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

// Shell script for starting up fims server and other processes before testing
constexpr const char* RUN_SCRIPT = R"(
    #!/bin/bash
    bin_dir=/usr/local/bin
    config_dir=/usr/local/etc/config
    echo "Starting up mongod, fims_server, and dbi..."
    sleep 3s; sudo mongod --config /etc/mongod.conf
    sleep 3s; $bin_dir/fims/fims_server & 
    sleep 3s; $bin_dir/dbi/dbi &
    sleep 3s;
)";

// Shell script for terminating running processes used for testing
constexpr const char* STOP_SCRIPT = R"(
    #!/bin/bash
    sudo pkill mongod
    pkill fims
    pkill dbi
)";


// Test fixture for creating fims object, assetVars, and other shared objects for each test case
class dbiTest : public ::testing::Test {
protected:
    virtual void SetUp() {

        // Configure fims object and connection
        if (!(p_fims = new fims())) {
            FPS_ERROR_PRINT("Failed to initialize fims class.\n");
            FAIL();
        }
        if (!p_fims->Connect((char *)"dbiTest")) {
            FPS_ERROR_PRINT("Failed to connect to fims server.\n");
            FAIL();
        }

        // Set up subscription(s) for fims to use
        memset(subs, 0, sizeof(char*));
        subs[0] = strdup("/dbi/controls");
        if (!p_fims->Subscribe((const char**)subs, 1)) {
            FPS_ERROR_PRINT("Failed to subscribe\n");
            FAIL();
        }

        // Set up dbi variable, dbi response, and asset manager assetVars
        const char* val = R"({"value":0.0,"dbiStatus":"init"})";
        const char* resp = R"({"value":-9999,"dbiSet":false})";

        cJSON* cjval = cJSON_Parse(val); 
        cJSON* cjresp = cJSON_Parse(resp); 

        dbiVar = vm.setValfromCj(vmap, "/controls/bms", "DemoChargeCurrent", cjval);
        dbiResp = vm.setValfromCj(vmap, "/dbi/controls/bms", "DemoChargeCurrent", cjresp);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        dbiVar->am = bms_man;
        dbiResp->am = bms_man;

        cJSON_Delete(cjval);
        cJSON_Delete(cjresp);

        SetupAvParams("bms", dbiVar);
        SetupDbiAvParams("bms", dbiResp);
    }

    virtual void TearDown() {
        // Delete the test data from the database, if there's any
        system("/usr/local/bin/fims/fims_send -m del -u /dbi/ess_controller/controls/bms");

        // Clean up remaining data used for testing
        p_fims->Close();
        delete p_fims;
        delete dbiVar->am;
        free(subs[0]);
    }

    varsmap vmap;       // main data map
    varmap amap;        // map of local variables for asset
    VarMapUtils vm;     // map utils factory
    fims* p_fims;       // fims object
    assetVar* dbiVar;   // assetVar referencing dbi
    assetVar* dbiResp;  // dbi response assetVar
    char* subs[1];      // Our fims subscriptions
};

// Tests dbi variable and dbi response parameter initialization
TEST_F(dbiTest, init_params) {
    // Check dbi variable parameter(s)
    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("init", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Check dbi response parameter(s)
    EXPECT_FALSE(dbiResp->getbParam("dbiSet"));
}

// Tests update to dbi when a value has changed
TEST_F(dbiTest, update_to_dbi_val_change) {

    // The first function call to UpdateDbi should not send an update to dbi.
    // Instead, it'll perform a fetch request to the dbi for a particular data point
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    // First, check the parameters
    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Now, check if an update is sent on a value change
    dbiVar->setVal(12.0);
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    EXPECT_EQ(12.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_GT(5.0, dbiVar->getdParam("UpdateTimeRemain"));  // Make sure the remaining time is decrementing
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Check the dbi and response
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    fims_message* msg = p_fims->Receive_Timeout(100);
    if (!msg) {
        FPS_ERROR_PRINT("Failed to received fims message\n");
        FAIL();
    }

    vm.runFimsMsg(vmap, msg, p_fims);

    EXPECT_EQ(12.0, dbiResp->getdVal());
    EXPECT_STREQ("OK", dbiResp->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiResp->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiResp->getdParam("UpdateTimeRemain"));  // time here should be equal to config time from dbi update
    EXPECT_TRUE(dbiResp->getbParam("EnableDbiUpdate"));
}

// Tests update to dbi after a certain period of time has passed
TEST_F(dbiTest, update_to_dbi_time_elapsed) {
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    // Since UpdateTimeCfg is 5 seconds by default, we'll wait for that amount of time,
    // and from there, we should now check the database for any updates
    sleep(5);

    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(0.0, dbiVar->getdParam("UpdateTimeRemain"));  // Time remain should be 0 to indicate dbi variable is updated after elapsed time
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // At this point, UpdateTimeRemain should be 0. One last function call to UpdateDbi should now
    // update the dbi, indicating that dbi update is caused by elapsed time
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    // Check the dbi and response
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    fims_message* msg = p_fims->Receive_Timeout(100);
    if (!msg) {
        FPS_ERROR_PRINT("Failed to received fims message\n");
        FAIL();
    }

    vm.runFimsMsg(vmap, msg, p_fims);

    EXPECT_EQ(0.0, dbiResp->getdVal());
    EXPECT_STREQ("OK", dbiResp->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiResp->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(0.0, dbiResp->getdParam("UpdateTimeRemain"));  // Time remain should be 0 to indicate dbi variable is updated after elapsed time
    EXPECT_TRUE(dbiResp->getbParam("EnableDbiUpdate"));
}

// Tests no update to dbi if the value to set is the same as the current value of the dbi variable
TEST_F(dbiTest, no_update_to_dbi_val_same) {
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    dbiVar->setVal(0.0);
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_GT(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Check to make sure there is no data in the database at the moment
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    fims_message* msg = p_fims->Receive_Timeout(100);
    EXPECT_FALSE(msg);
}

// Tests no update to dbi if EnableDbiUpdate is set to false. This means the ESS Controller will not send
// updates to the dbi, even if a certain amount of time to update has passed or the value has changed
TEST_F(dbiTest, no_update_to_dbi_param_disabled) {
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    dbiVar->setParam("EnableDbiUpdate", false);
    dbiVar->setVal(12.0);
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    EXPECT_EQ(12.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_FALSE(dbiVar->getbParam("EnableDbiUpdate"));

    // Check to make sure there is no data in the database at the moment
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    fims_message* msg = p_fims->Receive_Timeout(100);
    EXPECT_FALSE(msg);

    // Wait for a certain amount of time to pass
    sleep(5);

    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    EXPECT_EQ(12.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_FALSE(dbiVar->getbParam("EnableDbiUpdate"));

    // At this point, UpdateTimeRemain should still be 0, so no update to the dbi even after elapsed time
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    // Check the database again
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    msg = p_fims->Receive_Timeout(100);
    EXPECT_FALSE(msg);
}

// Tests if the data in the dbi response has changed. If so, the dbi variable should be updated
TEST_F(dbiTest, dbi_response_val_change) {
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    // Add data to the database first. At this point, there should be no test data
    system("/usr/local/bin/fims/fims_send -m set -u /dbi/ess_controller/controls/bms '{\"DemoChargeCurrent\":{\"value\":123,\"UpdateTimeCfg\":3,\"UpdateTimeRemain\":3}}' -r /me");

    // Now check the dbi and response
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    fims_message* msg = p_fims->Receive_Timeout(100);
    if (!msg) {
        FPS_ERROR_PRINT("Failed to received fims message\n");
        FAIL();
    }

    vm.runFimsMsg(vmap, msg, p_fims);

    EXPECT_EQ(123, dbiResp->getdVal());
    EXPECT_EQ(3.0, dbiResp->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(3.0, dbiResp->getdParam("UpdateTimeRemain"));

    // Make sure the dbi variable value and params are still intact before update
    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Update the dbi variable
    UpdateAvFromDbi(vmap, amap, "bms", p_fims, dbiResp);
    EXPECT_EQ(123, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(3, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(3, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));
}

// Tests if the dbi response contains a parameter (dbiSet) that's set to true. If so, the dbi variable should be updated
TEST_F(dbiTest, dbi_response_on_set) {
    UpdateDbi(vmap, amap, "bms", p_fims, dbiVar);

    // Add data to the database first. At this point, there should be no test data
    system("/usr/local/bin/fims/fims_send -m set -u /dbi/ess_controller/controls/bms '{\"DemoChargeCurrent\":{\"value\":-9999,\"UpdateTimeCfg\":3,\"UpdateTimeRemain\":3}}' -r /me");

    // Now check the dbi and response
    system("/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /dbi/controls/bms");
    fims_message* msg = p_fims->Receive_Timeout(100);
    if (!msg) {
        FPS_ERROR_PRINT("Failed to received fims message\n");
        FAIL();
    }

    vm.runFimsMsg(vmap, msg, p_fims);

    EXPECT_EQ(-9999, dbiResp->getdVal());
    EXPECT_EQ(3.0, dbiResp->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(3.0, dbiResp->getdParam("UpdateTimeRemain"));
    EXPECT_FALSE(dbiResp->getbParam("dbiSet"));

    // Make sure the dbi variable value and params are still intact before update
    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Since value is the same as the dbi variable and dbiSet is false, the dbi variable should not be updated
    UpdateAvFromDbi(vmap, amap, "bms", p_fims, dbiResp);
    EXPECT_EQ(0.0, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(5.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));

    // Now set dbiSet to true. The dbi variable should now be updated
    dbiResp->setParam("dbiSet", true);
    EXPECT_TRUE(dbiResp->getbParam("dbiSet"));

    UpdateAvFromDbi(vmap, amap, "bms", p_fims, dbiResp);
    EXPECT_EQ(-9999, dbiVar->getdVal());
    EXPECT_STREQ("OK", dbiVar->getcParam("dbiStatus"));
    EXPECT_EQ(3.0, dbiVar->getdParam("UpdateTimeCfg"));
    EXPECT_EQ(3.0, dbiVar->getdParam("UpdateTimeRemain"));
    EXPECT_TRUE(dbiVar->getbParam("EnableDbiUpdate"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Start up mongod, fims_server, and dbi
    system(RUN_SCRIPT);

    int results = RUN_ALL_TESTS();

    // Stop running processes
    system(STOP_SCRIPT);

    return results;
}