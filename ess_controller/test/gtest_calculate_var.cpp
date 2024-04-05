#include "../funcs/CalculateVar.cpp"
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
class CalculateVarTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        // Set up data (including parameters like operation, scale, and other
        // assetVars to use for calculation) for the assetVar to calculate results
        // for
        const char* val =
            R"({"value": 0.0, "numVars": 2, "variable1": "mbmu_voltage", "variable2": "mbmu_current"})";
        cJSON* cjval = cJSON_Parse(val);
        av = vm.setValfromCj(vmap, "/status/bms", "BMSPower", cjval);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        av->am = bms_man;

        cJSON_Delete(cjval);

        CalculateVar(vmap, amap, "bms", nullptr, av);
    }

    virtual void TearDown() { delete av->am; }

    varsmap vmap;    // main data map
    varmap amap;     // map of local variables for asset
    VarMapUtils vm;  // map utils factory
    assetVar* av;    // assetVar to hold calcuation results
};

// Test if parameters exist and contain the right values
TEST_F(CalculateVarTest, calculate_var_params_valid)
{
    EXPECT_EQ(2, av->getiParam("numVars"));
    EXPECT_STREQ("mbmu_voltage", av->getcParam("variable1"));
    EXPECT_STREQ("mbmu_current", av->getcParam("variable2"));
    EXPECT_STREQ("n/a", av->getcParam("operation"));
    EXPECT_STREQ("n/a", av->getcParam("expression"));
    EXPECT_FALSE(av->getbParam("includeCurrVal"));
    EXPECT_FALSE(av->getbParam("useExpr"));
    EXPECT_EQ(1, av->getdParam("scale"));

    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    ASSERT_TRUE(var1);
    EXPECT_STREQ("mbmu_voltage", var1->name.c_str());
    EXPECT_EQ(0, var1->getdVal());
    ASSERT_TRUE(var2);
    EXPECT_STREQ("mbmu_current", var2->name.c_str());
    EXPECT_EQ(0, var2->getdVal());
}

// Tests calculation skip if the given operation to perform is either n/a or not
// in the list of supported operations to perform Supported operations are the
// following: addition (+), subtraction (-), multiplication (*), division (/),
// average (avg), percentage of (pctOf)
TEST_F(CalculateVarTest, calculate_var_invalid_operation)
{
    EXPECT_STREQ("n/a", av->getcParam("operation"));
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Operation is not list of supported operations
    av->setParam("operation", (char*)"randomOperation");
    EXPECT_STREQ("randomOperation", av->getcParam("operation"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Even if assetVar values are changing, calculation shouldn't occur
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(5.0);
    var2->setVal(8);
    EXPECT_EQ(5, var1->getdVal());
    EXPECT_EQ(8, var2->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests calculation skip if the number of variables to use for calculations is
// invalid
TEST_F(CalculateVarTest, calculate_var_invalid_num_vars)
{
    av->setParam("numVars", 0);
    EXPECT_EQ(0, av->getiParam("numVars"));
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Even if assetVar values are changing, calculation shouldn't occur
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(5.0);
    var2->setVal(8);
    EXPECT_EQ(5, var1->getdVal());
    EXPECT_EQ(8, var2->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests the addition operation
TEST_F(CalculateVarTest, calculate_var_add)
{
    av->setParam("operation", (char*)"+");
    EXPECT_STREQ("+", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(14.5);
    var2->setVal(20);
    EXPECT_EQ(14.5, var1->getdVal());
    EXPECT_EQ(20, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(34.5, av->getdVal());
}

// Tests the subtraction operation
TEST_F(CalculateVarTest, calculate_var_subtract)
{
    av->setParam("operation", (char*)"-");
    EXPECT_STREQ("-", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(30);
    var2->setVal(20);
    EXPECT_EQ(30, var1->getdVal());
    EXPECT_EQ(20, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(10, av->getdVal());
}

// Tests the multiplication operation
TEST_F(CalculateVarTest, calculate_var_multiply)
{
    av->setParam("operation", (char*)"*");
    EXPECT_STREQ("*", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(-2);
    EXPECT_EQ(9, var1->getdVal());
    EXPECT_EQ(-2, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(-18, av->getdVal());
}

// Tests the division operation
TEST_F(CalculateVarTest, calculate_var_divide)
{
    av->setParam("operation", (char*)"/");
    EXPECT_STREQ("/", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(3);
    EXPECT_EQ(9, var1->getdVal());
    EXPECT_EQ(3, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(3, av->getdVal());

    // Special case - division by 0
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests the modulus division operation
TEST_F(CalculateVarTest, calculate_var_modulus)
{
    av->setParam("operation", (char*)"%");
    EXPECT_STREQ("%", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(3);
    EXPECT_EQ(9, var1->getdVal());
    EXPECT_EQ(3, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    var1->setVal(9);
    var2->setVal(5);
    EXPECT_EQ(9, var1->getdVal());
    EXPECT_EQ(5, var2->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(4, av->getdVal());

    // Special case - division by 0
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests the average operation
TEST_F(CalculateVarTest, calculate_var_avg)
{
    av->setParam("operation", (char*)"avg");
    EXPECT_STREQ("avg", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(3);
    EXPECT_EQ(9, var1->getdVal());
    EXPECT_EQ(3, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(6, av->getdVal());
}

// Tests the percentage of operation
TEST_F(CalculateVarTest, calculate_var_pct_of)
{
    av->setParam("operation", (char*)"pctOf");
    EXPECT_STREQ("pctOf", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(10);
    var2->setVal(0.3);
    EXPECT_EQ(10, var1->getdVal());
    EXPECT_EQ(0.3, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(7, av->getdVal());

    // Special case - percentage adds up to a value greater than 1
    var2->setVal(1);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests the max operation
TEST_F(CalculateVarTest, calculate_var_max)
{
    av->setParam("operation", (char*)"max");
    EXPECT_STREQ("max", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(10);
    var2->setVal(11);
    EXPECT_EQ(10, var1->getdVal());
    EXPECT_EQ(11, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(11, av->getdVal());
}

// Tests the min operation
TEST_F(CalculateVarTest, calculate_var_min)
{
    av->setParam("operation", (char*)"min");
    EXPECT_STREQ("min", av->getcParam("operation"));
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(10);
    var2->setVal(11);
    EXPECT_EQ(10, var1->getdVal());
    EXPECT_EQ(11, var2->getdVal());
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(10, av->getdVal());
}

// Tests the square root operation
TEST_F(CalculateVarTest, calculate_var_sqrt)
{
    av->setParam("operation", (char*)"sqrt");
    EXPECT_STREQ("sqrt", av->getcParam("operation"));
    av->setParam("includeCurrVal", true);
    av->setParam("numVars", 0);
    av->setVal(16);
    EXPECT_EQ(16, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(4, av->getdVal());
}

// Tests scaling feature
TEST_F(CalculateVarTest, calculate_var_scale)
{
    av->setParam("operation", (char*)"+");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(20);
    var2->setVal(10);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(30, av->getdVal());

    av->setParam("scale", 2);
    var1->setVal(30);
    var2->setVal(10);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(20, av->getdVal());

    av->setParam("scale", 0.1);
    var1->setVal(20);
    var2->setVal(1);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(210, av->getdVal());
}

// Tests logical and feature (when assetVar is a number)
TEST_F(CalculateVarTest, calculate_var_logical_and_numeric)
{
    av->setParam("operation", (char*)"and");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(1);
    var2->setVal(1);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    var1->setVal(1);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    var1->setVal(0);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests logical and feature (when assetVar is a boolean)
TEST_F(CalculateVarTest, calculate_var_logical_and_boolean)
{
    const char* val =
        R"({"value": false, "numVars": 2, "variable1": "mbmu_voltage", "variable2": "mbmu_current"})";
    cJSON* cjval = cJSON_Parse(val);
    assetVar* testAv = vm.setValfromCj(vmap, "/status/bms", "testVar2", cjval);
    testAv->am = av->am;
    cJSON_Delete(cjval);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);

    testAv->setParam("operation", (char*)"and");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(1);
    var2->setVal(1);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);
    EXPECT_TRUE(testAv->getbVal());

    var1->setVal(1);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);
    EXPECT_FALSE(testAv->getbVal());

    var1->setVal(0);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);
    EXPECT_FALSE(testAv->getbVal());
}

// Tests logical or feature
TEST_F(CalculateVarTest, calculate_var_logical_or_numeric)
{
    av->setParam("operation", (char*)"or");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(1);
    var2->setVal(1);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    var1->setVal(1);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    var1->setVal(0);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests logical or feature (when assetVar is a boolean)
TEST_F(CalculateVarTest, calculate_var_logical_or_boolean)
{
    const char* val =
        R"({"value": false, "numVars": 2, "variable1": "mbmu_voltage", "variable2": "mbmu_current"})";
    cJSON* cjval = cJSON_Parse(val);
    assetVar* testAv = vm.setValfromCj(vmap, "/status/bms", "testVar2", cjval);
    testAv->am = av->am;
    cJSON_Delete(cjval);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);

    testAv->setParam("operation", (char*)"or");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(1);
    var2->setVal(1);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);
    EXPECT_TRUE(testAv->getbVal());

    var1->setVal(1);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);
    EXPECT_TRUE(testAv->getbVal());

    var1->setVal(0);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, testAv);
    EXPECT_FALSE(testAv->getbVal());
}

// Tests valChangedAny operation
TEST_F(CalculateVarTest, calculate_var_valChangedAny)
{
    av->setParam("operation", (char*)"valChangedAny");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(false, av->getbVal());

    // The value of one assetVar has changed
    var1->setVal(1);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(true, av->getbVal());

    // The value of both assetVars has changed
    var1->setVal(2);
    var2->setVal(3);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(true, av->getbVal());
}

// Tests valChangedAll operation
TEST_F(CalculateVarTest, calculate_var_valChangedAll)
{
    av->setParam("operation", (char*)"valChangedAll");
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(false, av->getbVal());

    // The value of one assetVar has changed
    var1->setVal(1);
    var2->setVal(0);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(false, av->getbVal());

    // The value of both assetVars has changed
    var1->setVal(2);
    var2->setVal(3);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(true, av->getbVal());
}

// Tests aggregation of operands when asset:variable is specified
TEST_F(CalculateVarTest, calculate_var_aggregation)
{
    // Set up new assetVar
    const char* val =
        R"({"value": 0.0, "numVars": 1, "variable1": "bms:FaultCnt"})";
    cJSON* cjval = cJSON_Parse(val);
    assetVar* aggAv = vm.setValfromCj(vmap, "/status/bms", "BMSFaultCnt", cjval);
    aggAv->am = av->am;

    cJSON_Delete(cjval);

    // Add the asset instances to the asset manager
    aggAv->am->addInstance("rack_01");
    aggAv->am->addInstance("rack_02");

    asset* ai1 = aggAv->am->getInstance("rack_01");
    asset* ai2 = aggAv->am->getInstance("rack_02");
    ASSERT_TRUE(ai1);
    ASSERT_TRUE(ai2);

    // Initialize asset instance(s) value
    const char* aiVal = R"({"value": 0})";
    cjval = cJSON_Parse(aiVal);
    assetVar* var1 = vm.setValfromCj(vmap, "/status/rack_01", "FaultCnt", cjval);
    assetVar* var2 = vm.setValfromCj(vmap, "/status/rack_02", "FaultCnt", cjval);

    cJSON_Delete(cjval);

    // Run operation with aggregation of all asset instances' values
    aggAv->setParam("operation", (char*)"+");
    var1->setVal(2);
    var2->setVal(3);
    EXPECT_EQ(2, var1->getdVal());
    EXPECT_EQ(3, var2->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, aggAv);
    EXPECT_EQ(5, aggAv->getdVal());
}

// Tests aggregation of operands when asset:variable@param is specified
TEST_F(CalculateVarTest, calculate_var_aggregation_with_param)
{
    // Set up new assetVar
    const char* val =
        R"({"value": false, "numVars": 1, "variable1": "bms:RackCurrent@seenAlarm"})";
    cJSON* cjval = cJSON_Parse(val);
    assetVar* aggAv = vm.setValfromCj(vmap, "/status/bms", "BMSCurrentAlarm", cjval);
    aggAv->am = av->am;

    cJSON_Delete(cjval);

    // Add the asset instances to the asset manager
    aggAv->am->addInstance("rack_01");
    aggAv->am->addInstance("rack_02");

    asset* ai1 = aggAv->am->getInstance("rack_01");
    asset* ai2 = aggAv->am->getInstance("rack_02");
    ASSERT_TRUE(ai1);
    ASSERT_TRUE(ai2);

    // Initialize asset instance(s) value and param
    const char* aiVal = R"({"value": 0, "seenAlarm": false})";
    cjval = cJSON_Parse(aiVal);
    assetVar* var1 = vm.setValfromCj(vmap, "/status/rack_01", "RackCurrent", cjval);
    assetVar* var2 = vm.setValfromCj(vmap, "/status/rack_02", "RackCurrent", cjval);

    cJSON_Delete(cjval);

    // Run operation with aggregation of all asset instances' param values
    aggAv->setParam("operation", (char*)"or");
    var1->setParam("seenAlarm", true);
    var2->setParam("seenAlarm", false);
    EXPECT_TRUE(var1->getbParam("seenAlarm"));
    EXPECT_FALSE(var2->getbParam("seenAlarm"));
    CalculateVar(vmap, amap, "bms", nullptr, aggAv);
    EXPECT_TRUE(aggAv->getbVal());
}

// Tests the aggregation of operands managed by arbitrary asset managers and
// asset instances For example, take the aggregate of operands from all asset
// managers or take the aggregate of operands from all asset instances
TEST_F(CalculateVarTest, calculate_var_aggregation_with_asset_managers_and_asset_instances)
{
    // Create assetVars to store in asset managers and asset instances
    const char* aggAvVal =
        R"({"value": 0, "operation": "+", "numVars": 1, "variable1": "bms:FaultCnt"})";
    const char* am1AvVal =
        R"({"value": 0, "operation": "+", "numVars": 1, "variable1": "rack_01:FaultCnt"})";
    const char* am2AvVal =
        R"({"value": 0, "operation": "+", "numVars": 1, "variable1": "rack_02:FaultCnt"})";
    const char* aiVal = R"({"value": 0})";

    cJSON* cj_aggAvVal = cJSON_Parse(aggAvVal);
    cJSON* cj_am1AvVal = cJSON_Parse(am1AvVal);
    cJSON* cj_am2AvVal = cJSON_Parse(am2AvVal);
    cJSON* cj_aiVal = cJSON_Parse(aiVal);

    assetVar* aggAv = vm.setValfromCj(vmap, "/status/bms", "FaultCnt", cj_aggAvVal);
    assetVar* am1Av = vm.setValfromCj(vmap, "/status/rack_01", "FaultCnt", cj_am1AvVal);
    assetVar* am2Av = vm.setValfromCj(vmap, "/status/rack_02", "FaultCnt", cj_am2AvVal);
    assetVar* am1ai1Av = vm.setValfromCj(vmap, "/status/rack_01_module_01", "FaultCnt", cj_aiVal);
    assetVar* am1ai2Av = vm.setValfromCj(vmap, "/status/rack_01_module_02", "FaultCnt", cj_aiVal);
    assetVar* am2ai1Av = vm.setValfromCj(vmap, "/status/rack_02_module_01", "FaultCnt", cj_aiVal);
    assetVar* am2ai2Av = vm.setValfromCj(vmap, "/status/rack_02_module_02", "FaultCnt", cj_aiVal);

    cJSON_Delete(cj_aggAvVal);
    cJSON_Delete(cj_am1AvVal);
    cJSON_Delete(cj_am2AvVal);
    cJSON_Delete(cj_aiVal);

    // Create asset managers
    aggAv->am = av->am;
    am1Av->am = new asset_manager("rack_01");
    am2Av->am = new asset_manager("rack_02");
    am1Av->am->vm = aggAv->am->vm;
    am2Av->am->vm = aggAv->am->vm;

    // Create asset instances, which are managed by the newly created asset
    // managers
    am1Av->am->addInstance("rack_01_module_01");
    am1Av->am->addInstance("rack_01_module_02");
    am2Av->am->addInstance("rack_02_module_01");
    am2Av->am->addInstance("rack_02_module_02");

    aggAv->am->addManAsset(am1Av->am, "rack_01");
    aggAv->am->addManAsset(am2Av->am, "rack_02");

    // Check if the asset managers and asset instances exist
    ASSERT_TRUE(aggAv->am->assetManMap["rack_01"]);
    ASSERT_TRUE(aggAv->am->assetManMap["rack_02"]);
    ASSERT_TRUE(aggAv->am->assetManMap["rack_01"]->getInstance("rack_01_module_01"));
    ASSERT_TRUE(aggAv->am->assetManMap["rack_01"]->getInstance("rack_01_module_02"));
    ASSERT_TRUE(aggAv->am->assetManMap["rack_02"]->getInstance("rack_02_module_01"));
    ASSERT_TRUE(aggAv->am->assetManMap["rack_02"]->getInstance("rack_02_module_02"));

    // Take the aggregate of the variables in all asset instances managed by asset
    // managers
    am1ai1Av->setVal(2);
    am1ai2Av->setVal(1);
    am2ai1Av->setVal(3);
    am2ai2Av->setVal(2);
    CalculateVar(vmap, amap, "rack_01", nullptr, am1Av);
    CalculateVar(vmap, amap, "rack_02", nullptr, am2Av);
    ASSERT_EQ(3, am1Av->getdVal());
    ASSERT_EQ(5, am2Av->getdVal());

    // Take the aggregate of the variables in all asset managers managed by asset
    // managers
    CalculateVar(vmap, amap, "bms", nullptr, aggAv);
    ASSERT_EQ(8, aggAv->getdVal());
}

// Tests operation when includeCurrVal is set to true
TEST_F(CalculateVarTest, calculate_var_includeCurrVal_enabled)
{
    av->setParam("operation", (char*)"+");
    av->setParam("includeCurrVal", true);
    EXPECT_TRUE(av->getbParam("includeCurrVal"));

    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(20);
    var2->setVal(10);
    av->setVal(50);
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(80, av->getdVal());
}

// Tests expression evaluation skip if the given expression is invalid (ex.:
// n/a)
TEST_F(CalculateVarTest, calculate_var_expr_invalid)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    EXPECT_STREQ("n/a", av->getcParam("expression"));
    // Set numVars to 0 here to avoid having to skip calculations due to no value
    // change
    av->setParam("numVars", 0);

    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Expression does not contain the correct format (ex.: operands and
    // operators)
    av->setParam("expression", (char*)"hi there");
    EXPECT_STREQ("hi there", av->getcParam("expression"));

    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Expression does not contain the supported operator
    av->setParam("expression", (char*)"1 $ 1");
    EXPECT_STREQ("1 $ 1", av->getcParam("expression"));

    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests expression evaluation when useExpr is set to true and an expression
// exists
TEST_F(CalculateVarTest, calculate_var_evaluate_expr)
{
    // Set numVars to 0 here to avoid having to skip calculations due to no value
    // change
    av->setParam("numVars", 0);

    // Evaluate simple expression
    av->setParam("useExpr", true);
    av->setParam("expression", (char*)"1 + 2");
    EXPECT_TRUE(av->getbParam("useExpr"));
    EXPECT_STREQ("1 + 2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(3, av->getdVal());

    // Expression with float operands
    av->setParam("expression", (char*)"1.5 - 0.2");
    EXPECT_STREQ("1.5 - 0.2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1.3, av->getdVal());

    // Expression with mixed signs
    av->setParam("expression", (char*)"-1 + 5 - -2");
    EXPECT_STREQ("-1 + 5 - -2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(6, av->getdVal());

    // Evaluate complex expression (varying precedence and associativity rules)

    // Expression with parenthesis
    av->setParam("expression", (char*)"5 - ( 3 - 2 )");
    EXPECT_STREQ("5 - ( 3 - 2 )", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(4, av->getdVal());

    // Expression with mixed associativity rules (left and right)
    av->setParam("expression", (char*)"5 * 2 ** 2");
    EXPECT_STREQ("5 * 2 ** 2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(20, av->getdVal());

    // Expression with various operators and parenthesis
    av->setParam("expression", (char*)"20 + 8 / 2 * ( 3 + 4 ) - ( 20 / 10 * 2 ) ** 2");
    EXPECT_STREQ("20 + 8 / 2 * ( 3 + 4 ) - ( 20 / 10 * 2 ) ** 2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(32, av->getdVal());
}

// Tests expression evaluation skip if the placeholders in the expression do not
// contain the correct index for the list of operands
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_operand_vars_invalid_idx)
{
    // Set numVars to 0 here to avoid having to skip calculations due to no value
    // change
    av->setParam("numVars", 0);

    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));

    // Placeholder index <= 0
    av->setParam("expression", (char*)"1 + {-1} + 2");
    EXPECT_STREQ("1 + {-1} + 2", av->getcParam("expression"));
    EXPECT_EQ(0, av->getdVal());
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Placeholder index > size of the list of operands
    av->setParam("expression", (char*)"1 + {3} + 2");
    EXPECT_STREQ("1 + {3} + 2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests expression evaluation using the operand variables in the expression
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_operand_vars)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));

    av->setParam("expression", (char*)"1 + {1} - {2} + 2");
    EXPECT_STREQ("1 + {1} - {2} + 2", av->getcParam("expression"));
    EXPECT_EQ(0, av->getdVal());

    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(20);
    var2->setVal(10);

    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(13, av->getdVal());
}

// Tests expression evaluation using function operators
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_func_operators)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Simple function
    av->setParam("expression", (char*)"sqrt ( 4 )");
    EXPECT_STREQ("sqrt ( 4 )", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(2, av->getdVal());

    // Complex function
    av->setParam("numVars", 2);
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(16);
    av->setParam("expression", (char*)"sqrt ( {1} + max ( 4 {2} ) )");
    EXPECT_STREQ("sqrt ( {1} + max ( 4 {2} ) )", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(5, av->getdVal());
}

// Tests expression evaluation when tokens (ex.: commas and left-right
// parenthesis with no spaces around) are used
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_commas_parenthesis)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));

    // Expression evaluation with commas and parenthesis
    av->setParam("expression", (char*)"max(1, min(3, -1))");
    EXPECT_STREQ("max(1, min(3, -1))", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_STREQ("max(1, min(3, -1))", av->getcParam("expression"));
    EXPECT_EQ(1, av->getdVal());

    // Expression evaluation with commas and parenthesis and placeholders
    av->setParam("numVars", 2);
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(16);
    av->setParam("expression", (char*)"sqrt({1} + max(4, {2}))");
    EXPECT_STREQ("sqrt({1} + max(4, {2}))", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_STREQ("sqrt({1} + max(4, {2}))", av->getcParam("expression"));
    EXPECT_EQ(5, av->getdVal());
}

// Tests expression evaluation using logical operators
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_logical_operators)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Logical and (&&)
    av->setParam("expression", (char*)"1 && 0");
    EXPECT_STREQ("1 && 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"1 and 1");
    EXPECT_STREQ("1 and 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Logical or (||)
    av->setParam("expression", (char*)"1 || 0");
    EXPECT_STREQ("1 || 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"1 or 1");
    EXPECT_STREQ("1 or 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Logical not (!)
    av->setParam("expression", (char*)"! 1");
    EXPECT_STREQ("! 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"not 0");
    EXPECT_STREQ("not 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Mixed logical operators
    av->setParam("expression", (char*)"1 || 1 && 0");
    EXPECT_STREQ("1 || 1 && 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"( 1 || 1 ) && 0");
    EXPECT_STREQ("( 1 || 1 ) && 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"! ( ! 1 || ! 1 ) && ! 0");
    EXPECT_STREQ("! ( ! 1 || ! 1 ) && ! 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"! ( ( 1 || 0 && 1 ) && ! ( 1 && 1 || ( 0 && 1 || 0 ) ) )");
    EXPECT_STREQ("! ( ( 1 || 0 && 1 ) && ! ( 1 && 1 || ( 0 && 1 || 0 ) ) )", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());
}

// Tests expression evaluation using comparison operators
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_comparison_operators)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Relational Operators (<, <=, >, >=)
    av->setParam("expression", (char*)"1 > 0");
    EXPECT_STREQ("1 > 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"1 >= 1");
    EXPECT_STREQ("1 >= 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"1 < 1");
    EXPECT_STREQ("1 < 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"1 <= 0");
    EXPECT_STREQ("1 <= 0", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    // Equality Operators (==, !=)
    av->setParam("expression", (char*)"1 == 1");
    EXPECT_STREQ("1 == 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"1 != 1");
    EXPECT_STREQ("1 != 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());
}

// Tests expression evaluation using bitwise operators
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_bitwise_operators)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Bitwise and (&)
    av->setParam("expression", (char*)"13 & 1");
    EXPECT_STREQ("13 & 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Bitwise or (|)
    av->setParam("expression", (char*)"13 | 1");
    EXPECT_STREQ("13 | 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(13, av->getdVal());

    // Bitwise xor (^)
    av->setParam("expression", (char*)"13 ^ 1");
    EXPECT_STREQ("13 ^ 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(12, av->getdVal());

    // Bitwise not (~)
    av->setParam("expression", (char*)"~ -13");
    EXPECT_STREQ("~ -13", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(12, av->getdVal());

    // Bitwise left shift (<<)
    av->setParam("expression", (char*)"13 << 2");
    EXPECT_STREQ("13 << 2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(52, av->getdVal());

    // Bitwise right shift (>>)
    av->setParam("expression", (char*)"13 >> 2");
    EXPECT_STREQ("13 >> 2", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(3, av->getdVal());
}

// Tests expression evaluation using conditional operation (if/then/else)
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_conditional_operators)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Evaluation if true
    av->setParam("expression", (char*)"if(1, (10 * 2), (9 / 3))");
    EXPECT_STREQ("if(1, (10 * 2), (9 / 3))", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(20, av->getdVal());

    // Evaluation if false
    av->setParam("expression", (char*)"if(0, (10 * 2), (9 / 3))");
    EXPECT_STREQ("if(0, (10 * 2), (9 / 3))", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(3, av->getdVal());

    // Complex condition
    av->setParam("numVars", 2);
    assetVar* var1 = amap["mbmu_voltage"];
    assetVar* var2 = amap["mbmu_current"];
    var1->setVal(9);
    var2->setVal(16);
    av->setParam("expression", (char*)"if(({1} != {2} and {1} == 9), (10 * 2), (9 / 3))");
    EXPECT_STREQ("if(({1} != {2} and {1} == 9), (10 * 2), (9 / 3))", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(20, av->getdVal());

    // Test nested if
    av->setParam("expression", (char*)"if(0, (10 * 2), (50 / if(true, 2, 5)))");
    EXPECT_STREQ("if(0, (10 * 2), (50 / if(true, 2, 5)))", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(25, av->getdVal());
}

// Tests expression evaluation using boolean operand types
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_boolean_operand)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Logical and (&&)
    av->setParam("expression", (char*)"true && false");
    EXPECT_STREQ("true && false", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"true and true");
    EXPECT_STREQ("true and true", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Logical or (||)
    av->setParam("expression", (char*)"true || false");
    EXPECT_STREQ("true || false", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"true or true");
    EXPECT_STREQ("true or true", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Logical not (!)
    av->setParam("expression", (char*)"! true");
    EXPECT_STREQ("! true", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"not false");
    EXPECT_STREQ("not false", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // Mixed operand types (only numeric and booleans are accepted types)
    av->setParam("expression", (char*)"true and 1");
    EXPECT_STREQ("true and 1", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char *)"! ( ( true || 0 && true ) && ! ( true && "
                                     "1 || ( false && 1 || false ) ) )");
    EXPECT_STREQ(
        "! ( ( true || 0 && true ) && ! ( true && 1 || ( false && 1 || "
        "false ) ) )",
        av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());
}

// Tests expression evaluation using string operand
TEST_F(CalculateVarTest, calculate_var_evaluate_expr_string_operand)
{
    av->setParam("useExpr", true);
    EXPECT_TRUE(av->getbParam("useExpr"));
    av->setParam("numVars", 0);

    // Equality Operators (==, !=)
    av->setParam("expression", (char*)"hi == hi");
    EXPECT_STREQ("hi == hi", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    av->setParam("expression", (char*)"hi == hello");
    EXPECT_STREQ("hi == hello", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"hi != hi");
    EXPECT_STREQ("hi != hi", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(0, av->getdVal());

    av->setParam("expression", (char*)"hi != hello");
    EXPECT_STREQ("hi != hello", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_EQ(1, av->getdVal());

    // String operand in conditional
    av->setParam("expression", (char*)"if((hi != hello), yes, no)");
    EXPECT_STREQ("if((hi != hello), yes, no)", av->getcParam("expression"));
    CalculateVar(vmap, amap, "bms", nullptr, av);
    EXPECT_STREQ("yes", av->getcVal());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}