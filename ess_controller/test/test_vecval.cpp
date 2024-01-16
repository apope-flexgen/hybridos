// Test the new vec val ( doubles only ) plus some stff from calculatevar
#include "gtest/gtest.h"
#include "../funcs/CalculateVar.cpp"
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
class VecValTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        // Set up data (including parameters like operation, scale, and other assetVars to use for calculation) for the assetVar to calculate results for
        const char* val = R"({"value": 0.0, "numVars": 2, "variable1": "mbmu_voltage", "variable2": "mbmu_current"})";
        cJSON* cjval = cJSON_Parse(val);
        av = vm.setValfromCj(vmap, "/status/bms", "BMSTest", cjval);
        av->setVecDepth(16);
        av->setVal(1.1);
        av->setVal(2.2);
        av->setVal(3.3);
        av->setVal(4.4);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        av->am = bms_man;

        cJSON_Delete(cjval);

        CalculateVar(vmap, amap, "bms", nullptr, av);
    }

    virtual void TearDown() {
        delete av->am;
    }

    varsmap vmap;       // main data map
    varmap amap;        // map of local variables for asset
    VarMapUtils vm;     // map utils factory
    assetVar* av;       // assetVar to hold calcuation results
};

// Test if parameters exist and contain the right values
TEST_F(VecValTest, calculate_var_params_valid) {
    EXPECT_EQ(2, av->getiParam("numVars"));
    EXPECT_EQ(16, av->getVecDepth());
    EXPECT_EQ(2.2, av->getVecdVal(2));
    EXPECT_TRUE(av->setVal(5.5));
    EXPECT_EQ(5.5, av->getVecdVal(0));
    EXPECT_EQ(4.4, av->getVecdVal(1));
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


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}