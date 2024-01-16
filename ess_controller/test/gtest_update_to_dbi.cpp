#include "gtest/gtest.h"
#include "../funcs/UpdateToDbi.cpp"
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

/**
 * @brief Removes selected fields inside the cJSON message body
 * 
 * @param cj the cJSON message body
 * @param filterItems the collection of fields to remove from the cJSON message body
 */
void filtercJSONMsg(cJSON* cj, const std::initializer_list<std::string> filterItems)
{
    for (const std::string& item : filterItems)
    {
        cJSON* cjitem = cJSON_DetachItemFromObject(cj, item.c_str());
        if (cjitem)
            cJSON_Delete(cjitem);
    }
}

// Test fixture for creating fims object, assetVars, and other shared objects for each test case
class UpdateToDbiTest : public ::testing::Test {
protected:
    virtual void SetUp() {

        // Configure fims object and connection
        if (!(p_fims = new fims())) {
            FPS_ERROR_PRINT("Failed to initialize fims class.\n");
            FAIL();
        }
        if (!p_fims->Connect((char *)"UpdateToDbiTest")) {
            FPS_ERROR_PRINT("Failed to connect to fims server.\n");
            FAIL();
        }

        // Set up subscription(s) for fims to use
        memset(subs, 0, sizeof(char*));
        subs[0] = strdup("/reply_to");
        if (!p_fims->Subscribe((const char**)subs, 1)) {
            FPS_ERROR_PRINT("Failed to subscribe\n");
            FAIL();
        }

        // Set up assetVar that contains variables to update to dbi
        const char* val = R"({"value": 0})";
        cJSON* cjval = cJSON_Parse(val);
        av = vm.setValfromCj(vmap, "/dbi/bms", "TestDbiUpdate", cjval);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        bms_man->vmap = &vmap;
        bms_man->vm->p_fims = p_fims;
        av->am = bms_man;

        cJSON_Delete(cjval);
    }

    virtual void TearDown() {
        delete av->am;

        // Clean up remaining data used for testing
        p_fims->Send("del", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", nullptr, nullptr);
        p_fims->Close();
        delete p_fims;
        free(subs[0]);
    }

    varsmap vmap;       // main data map
    varmap amap;        // map of local variables for asset
    VarMapUtils vm;     // map utils factory
    fims* p_fims;       // fims object
    assetVar* av;       // assetVar responsible for updating variables to dbi
    char* subs[1];      // fims subscriptions
};

// Tests dbi variable's parameter initialization
TEST_F(UpdateToDbiTest, init_params) {
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    EXPECT_EQ(0, av->getdVal());
    EXPECT_EQ(0, av->getdParam("updateTimeout"));
    EXPECT_EQ(0, av->getdParam("currUpdateTime"));
    EXPECT_EQ(false, av->getbParam("includeMetaData"));
    EXPECT_STREQ("bms_TestDbiUpdate_saved_variables", av->getcParam("document"));
}

// Tests update skip if variables to update are not provided
TEST_F(UpdateToDbiTest, update_skip) {
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    p_fims->Send("get", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", "/reply_to", nullptr);
    fims_message* msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);
    ASSERT_STREQ("{}", msg->body);
    free(msg);
}

// Tests update to dbi - default document
TEST_F(UpdateToDbiTest, update_with_default_document) {
    av->setParam("numVars", 2);
    av->setParam("variable1", (char*)"/components/bms_1:accumulated_charge_energy");
    av->setParam("variable2", (char*)"/components/bms_1:accumulated_discharge_energy");

    const char* val  = R"({"value": 0, "param": "value"})";
    cJSON* cjval = cJSON_Parse(val);

    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_charge_energy", cjval);
    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_discharge_energy", cjval);

    // Send update request to dbi and check results
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    p_fims->Send("get", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", "/reply_to", nullptr);
    fims_message* msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);

    char* expected = (char*)
    "{"
        "\"#components#bms_1\":{"
            "\"accumulated_charge_energy\":0,"
            "\"accumulated_discharge_energy\":0"
        "}"
    "}";

    cJSON* body = cJSON_Parse(msg->body);
    filtercJSONMsg(body, {"_doc", "_id", "_version"});
    char* actual = cJSON_PrintUnformatted(body);
    EXPECT_STREQ(expected, actual);

    // Cleanup resources
    free(msg);
    free(actual);
    cJSON_Delete(body);
    cJSON_Delete(cjval);
}

// Tests update to dbi - custom document
TEST_F(UpdateToDbiTest, update_with_custom_document) {
    av->setParam("numVars", 2);
    av->setParam("variable1", (char*)"/components/bms_1:accumulated_charge_energy");
    av->setParam("variable2", (char*)"/components/bms_1:accumulated_discharge_energy");
    av->setParam("document", (char*)"custom_document");

    const char* val  = R"({"value": 0, "param": "value"})";
    cJSON* cjval = cJSON_Parse(val);

    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_charge_energy", cjval);
    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_discharge_energy", cjval);

    // Send update request to dbi and check results
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    p_fims->Send("get", "/dbi/ess_controller/custom_document", "/reply_to", nullptr);
    fims_message* msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);

    char* expected = (char*)
    "{"
        "\"#components#bms_1\":{"
            "\"accumulated_charge_energy\":0,"
            "\"accumulated_discharge_energy\":0"
        "}"
    "}";

    cJSON* body = cJSON_Parse(msg->body);
    filtercJSONMsg(body, {"_doc", "_id", "_version"});
    char* actual = cJSON_PrintUnformatted(body);
    EXPECT_STREQ(expected, actual);

    // Cleanup resources
    free(msg);
    free(actual);
    cJSON_Delete(body);
    cJSON_Delete(cjval);
    p_fims->Send("del", "/dbi/ess_controller/custom_document", nullptr, nullptr);
}

// Tests update to dbi - variables in different uris
TEST_F(UpdateToDbiTest, update_vars_in_different_uris) {
    av->setParam("numVars", 4);
    av->setParam("variable1", (char*)"/components/bms_1:accumulated_charge_energy");
    av->setParam("variable2", (char*)"/components/bms_1:accumulated_discharge_energy");
    av->setParam("variable3", (char*)"/status/bms:system_state");
    av->setParam("variable4", (char*)"/controls/bms:close_contactors");

    const char* val  = R"({"value": 0, "param": "value"})";
    const char* val2 = R"({"value": "value", "another_param": "another_value"})";
    cJSON* cjval = cJSON_Parse(val);
    cJSON* cjval2 = cJSON_Parse(val2);

    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_charge_energy", cjval);
    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_discharge_energy", cjval);
    vm.setValfromCj(vmap, "/status/bms", "system_state", cjval2);
    vm.setValfromCj(vmap, "/controls/bms", "close_contactors", cjval);

    // Send update request to dbi and check results
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    p_fims->Send("get", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", "/reply_to", nullptr);
    fims_message* msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);

    char* expected = (char*)
    "{"
        "\"#components#bms_1\":{"
            "\"accumulated_charge_energy\":0,"
            "\"accumulated_discharge_energy\":0"
        "},"
        "\"#controls#bms\":{"
            "\"close_contactors\":0"
        "},"
        "\"#status#bms\":{"
            "\"system_state\":\"value\""
        "}"
    "}";

    cJSON* body = cJSON_Parse(msg->body);
    filtercJSONMsg(body, {"_doc", "_id", "_version"});
    char* actual = cJSON_PrintUnformatted(body);
    EXPECT_STREQ(expected, actual);

    // Cleanup resources
    free(msg);
    free(actual);
    cJSON_Delete(body);
    cJSON_Delete(cjval);
    cJSON_Delete(cjval2);
}

// Tests update to dbi after elapsed time
TEST_F(UpdateToDbiTest, update_after_elapsed_time) {
    av->setParam("numVars", 2);
    av->setParam("variable1", (char*)"/components/bms_1:accumulated_charge_energy");
    av->setParam("variable2", (char*)"/components/bms_1:accumulated_discharge_energy");
    av->setParam("updateTimeout", 5);

    const char* val  = R"({"value": 0, "param": "value"})";
    cJSON* cjval = cJSON_Parse(val);

    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_charge_energy", cjval);
    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_discharge_energy", cjval);

    // Send update request to dbi and check results. Should expect current update time to be decremented.
    // Variables should not be in database
    UpdateToDbi(vmap, amap, "bms", p_fims, av);
    
    EXPECT_GT(5, av->getdParam("currentUpdateTime"));

    p_fims->Send("get", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", "/reply_to", nullptr);
    fims_message* msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);
    EXPECT_STREQ("{}", msg->body);
    free(msg);

    // Set the current update time close to zero.
    av->setParam("currUpdateTime", 0.0000001);
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    EXPECT_EQ(0, av->getdParam("currUpdateTime"));

    // Send update request to dbi again and check results.
    // Variables should now be in database
    UpdateToDbi(vmap, amap, "bms", p_fims, av);
    
    EXPECT_EQ(5, av->getdParam("currUpdateTime"));

    p_fims->Send("get", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", "/reply_to", nullptr);
    msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);

    char* expected = (char*)
    "{"
        "\"#components#bms_1\":{"
            "\"accumulated_charge_energy\":0,"
            "\"accumulated_discharge_energy\":0"
        "}"
    "}";

    cJSON* body = cJSON_Parse(msg->body);
    filtercJSONMsg(body, {"_doc", "_id", "_version"});
    char* actual = cJSON_PrintUnformatted(body);
    EXPECT_STREQ(expected, actual);

    // Cleanup resources
    free(msg);
    free(actual);
    cJSON_Delete(body);
    cJSON_Delete(cjval);
}

// Tests update to dbi, where variable data is compact (includeMetaData = true)
TEST_F(UpdateToDbiTest, update_with_compact_mode) {
    av->setParam("numVars", 2);
    av->setParam("variable1", (char*)"/components/bms_1:accumulated_charge_energy");
    av->setParam("variable2", (char*)"/components/bms_1:accumulated_discharge_energy");
    av->setParam("includeMetaData", true);

    const char* val  = R"({"value": 0, "param": "value"})";
    cJSON* cjval = cJSON_Parse(val);

    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_charge_energy", cjval);
    vm.setValfromCj(vmap, "/components/bms_1", "accumulated_discharge_energy", cjval);

    // Send update request to dbi and check results
    UpdateToDbi(vmap, amap, "bms", p_fims, av);

    p_fims->Send("get", "/dbi/ess_controller/bms_TestDbiUpdate_saved_variables", "/reply_to", nullptr);
    fims_message* msg = p_fims->Receive_Timeout(100000);
    ASSERT_TRUE(msg);

    char* expected = (char*)
    "{"
        "\"#components#bms_1\":{"
            "\"accumulated_charge_energy\":{"
                "\"param\":\"value\","
                "\"value\":0"
            "},"
            "\"accumulated_discharge_energy\":{"
                "\"param\":\"value\","
                "\"value\":0"
            "}"
        "}"
    "}";

    cJSON* body = cJSON_Parse(msg->body);
    filtercJSONMsg(body, {"_doc", "_id", "_version"});
    char* actual = cJSON_PrintUnformatted(body);
    EXPECT_STREQ(expected, actual);

    // Cleanup resources
    free(msg);
    free(actual);
    cJSON_Delete(body);
    cJSON_Delete(cjval);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}