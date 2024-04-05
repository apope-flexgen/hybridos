#include "../funcs/AlarmFaultHandler.cpp"
#include "../funcs/CheckMonitorVar.cpp"
#include "scheduler.h"
#include "gtest/gtest.h"

// Need this in order to compile
namespace flex
{
const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

// Need this in order to compile
typedef std::vector<schedItem*> schlist;
schlist schreqs;
cJSON* getSchListcJ(schlist& schreqs);

cJSON* getSchList()
{
    return getSchListcJ(schreqs);
}

// Test fixture for creating assetVars and other shared objects for each test
// case
class AlarmFaultHandlerTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        // Configure fims object and connection
        if (!(p_fims = new fims()))
        {
            FPS_ERROR_PRINT("Failed to initialize fims class.\n");
            FAIL();
        }
        if (!p_fims->Connect((char*)"AlarmFaultHandlerTest"))
        {
            FPS_ERROR_PRINT("Failed to connect to fims server.\n");
            FAIL();
        }

        // Set up fault/alarm variables
        const char* alrmVal = R"({"value": "Normal", "type": "alarm"})";
        const char* fltVal = R"({"value": "Normal", "type": "fault"})";
        cJSON* cjAlrmVal = cJSON_Parse(alrmVal);
        cJSON* cjFltVal = cJSON_Parse(fltVal);
        assetVar* alrmAv = vm.setValfromCj(vmap, "/alarms/bms", "OverVoltage", cjAlrmVal);
        assetVar* fltAv = vm.setValfromCj(vmap, "/faults/bms", "OverVoltage", cjFltVal);

        // Set up clear faults variables
        const char* clearAlrmVal =
            R"({"value": "Normal", "type": "alarm", "numVars": 1, "variable1": "bms_heartbeat"})";
        const char* clearFltVal =
            R"({"value": "Normal", "type": "fault", "numVars": 1, "variable1": "bms_heartbeat"})";
        cJSON* cjClearAlrmVal = cJSON_Parse(clearAlrmVal);
        cJSON* cjClearFltVal = cJSON_Parse(clearFltVal);
        assetVar* clearAlrmAv = vm.setValfromCj(vmap, "/alarms/bms", "clear_alarms", cjClearAlrmVal);
        assetVar* clearFltAv = vm.setValfromCj(vmap, "/faults/bms", "clear_faults", cjClearFltVal);

        // Set up fault/alarm UI variables
        const char* alrmUiConfig =
            R"({"name": "Alarms", "value": 0, "unit": "", "scaler": 0, "enabled": true, "ui_type": "alarm", "type": "number", "options": []})";
        const char* fltUiConfig =
            R"({"name": "Alarms", "value": 0, "unit": "", "scaler": 0, "enabled": true, "ui_type": "alarm", "type": "number", "options": []})";
        cJSON* cjAlrmUiVal = cJSON_Parse(alrmUiConfig);
        cJSON* cjFltUiVal = cJSON_Parse(fltUiConfig);
        vm.setValfromCj(vmap, "/assets/bms/summary", "alarms", cjAlrmUiVal);
        vm.setValfromCj(vmap, "/assets/bms/summary", "faults", cjFltUiVal);

        // Create alarm/fault helper variables
        const char* alrmDest = R"({"value": "/assets/bms/summary:alarms"})";
        const char* fltDest = R"({"value": "/assets/bms/summary:faults"})";
        const char* normalVal = R"({"value": "Normal"})";
        cJSON* cjAlrmDest = cJSON_Parse(alrmDest);
        cJSON* cjFltDest = cJSON_Parse(fltDest);
        cJSON* cjNormalVal = cJSON_Parse(normalVal);
        vm.setValfromCj(vmap, "/config/bms", "AlarmDestination", cjAlrmDest);
        vm.setValfromCj(vmap, "/config/bms", "FaultDestination", cjFltDest);
        vm.setValfromCj(vmap, "/config/bms", "NoFaultMsg", cjNormalVal);
        vm.setValfromCj(vmap, "/config/bms", "NoAlarmMsg", cjNormalVal);

        // Add monitor var parameters
        const char* ival =
            R"({"value": 0, "EnableStateCheck": false, "AlarmTimeout": 5, "FaultTimeout": 7, "RecoverTimeout": 4})";
        cJSON* cjival = cJSON_Parse(ival);
        assetVar* monitorAv = vm.setValfromCj(vmap, "/components/bms", "bms_heartbeat", cjival);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        alrmAv->am = bms_man;
        fltAv->am = bms_man;
        clearAlrmAv->am = bms_man;
        clearFltAv->am = bms_man;
        monitorAv->am = bms_man;

        // Run monitor func once to initialize monitor av
        CheckMonitorVar(vmap, amap, "bms", p_fims, monitorAv);

        cJSON_Delete(cjAlrmVal);
        cJSON_Delete(cjFltVal);
        cJSON_Delete(cjClearAlrmVal);
        cJSON_Delete(cjClearFltVal);
        cJSON_Delete(cjAlrmUiVal);
        cJSON_Delete(cjFltUiVal);
        cJSON_Delete(cjival);
        cJSON_Delete(cjAlrmDest);
        cJSON_Delete(cjFltDest);
        cJSON_Delete(cjNormalVal);
    }

    virtual void TearDown()
    {
        p_fims->Close();
        delete p_fims;
    }

    varsmap vmap;    // main data map
    varmap amap;     // map of local variables for asset
    fims* p_fims;    // fims object
    VarMapUtils vm;  // map utils factory
};

// Test if parameters and other variables exist and contain the right values
TEST_F(AlarmFaultHandlerTest, valid_params_and_vars)
{
    // Check alarm/fault variables
    EXPECT_STREQ("Normal", vmap["/faults/bms"]["OverVoltage"]->getcVal());
    EXPECT_STREQ("Normal", vmap["/alarms/bms"]["OverVoltage"]->getcVal());
    EXPECT_STREQ("fault", vmap["/faults/bms"]["OverVoltage"]->getcParam("type"));
    EXPECT_STREQ("alarm", vmap["/alarms/bms"]["OverVoltage"]->getcParam("type"));
    EXPECT_STREQ("/assets/bms/summary:faults", vmap["/config/bms"]["FaultDestination"]->getcVal());
    EXPECT_STREQ("/assets/bms/summary:alarms", vmap["/config/bms"]["AlarmDestination"]->getcVal());
    EXPECT_STREQ("Normal", vmap["/config/bms"]["NoFaultMsg"]->getcVal());
    EXPECT_STREQ("Normal", vmap["/config/bms"]["NoAlarmMsg"]->getcVal());

    // Check alarm/fault UI variables
    EXPECT_EQ(0, vmap["/assets/bms/summary"]["faults"]->getdVal());
    EXPECT_EQ(0, vmap["/assets/bms/summary"]["alarms"]->getdVal());

    // Check clear faults variables
    EXPECT_EQ(1, vmap["/faults/bms"]["clear_faults"]->getiParam("numVars"));
    EXPECT_EQ(1, vmap["/alarms/bms"]["clear_alarms"]->getiParam("numVars"));
    EXPECT_STREQ("bms_heartbeat", vmap["/faults/bms"]["clear_faults"]->getcParam("variable1"));
    EXPECT_STREQ("bms_heartbeat", vmap["/alarms/bms"]["clear_alarms"]->getcParam("variable1"));

    // Check monitor variable
    assetVar* var1 = vm.getVar(vmap, "/components/bms:bms_heartbeat", nullptr);
    ASSERT_TRUE(var1);
    EXPECT_STREQ("bms_heartbeat", var1->name.c_str());
    EXPECT_EQ(0, var1->getdVal());
    EXPECT_FALSE(var1->getbParam("EnableStateCheck"));
    EXPECT_EQ(5, var1->getdParam("AlarmTimeout"));
    EXPECT_EQ(7, var1->getdParam("FaultTimeout"));
    EXPECT_EQ(4, var1->getdParam("RecoverTimeout"));
    EXPECT_STREQ("int",
                 var1->getcParam("Type"));  // Default type is int if not defined in config
    EXPECT_EQ(5, var1->getdParam("AlarmTime"));
    EXPECT_EQ(7, var1->getdParam("FaultTime"));
    EXPECT_EQ(4, var1->getdParam("RecoverTime"));
    EXPECT_FALSE(var1->getbParam("seenFault"));
    EXPECT_FALSE(var1->getbParam("seenAlarm"));
    EXPECT_FALSE(var1->getbParam("seenReset"));
}

// Tests alarm/fault reporting
TEST_F(AlarmFaultHandlerTest, alarm_fault_report)
{
    // Set the alarm variable to a value other than Normal. Check results

    // Set the alarm variable to a different value. Check results

    // Set the alarm variable to the same value. Check results

    // Set the fault variable to a value other than Normal. Check Results
}

// Tests alarm/fault clearing (without latched alarm/fault from a monitor
// variable)
TEST_F(AlarmFaultHandlerTest, clear_alarms_faults)
{
    // Set the clear_alarms variable to "Clear". Check results

    // Set the clear_faults variable to "Clear". Check results
}

// Tests alarm/fault clearing (with latch alarm/fault from a monitor variable)
TEST_F(AlarmFaultHandlerTest, clear_alarms_faults_latched_monitor_var_states)
{
    // Set the clear_alarms variable to "Clear". Check results

    // Set the clear_faults variable to "Clear". Check results
}

// Tests alarm/fault clearing (including asset instances managed by an asset
// manager)
TEST_F(AlarmFaultHandlerTest, clear_alarms_faults_with_asset_instances)
{
    // Set the clear_alarms variable to "Clear". Check results

    // Set the clear_faults variable to "Clear". Check results
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}