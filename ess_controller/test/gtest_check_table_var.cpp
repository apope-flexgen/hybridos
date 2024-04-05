#include "../funcs/CheckTableVar.cpp"
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
class CheckTableVarTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        // Set up assetVars (including the table data structure and the table vars)
        // for the table assetVar
        const char* tableVar =
            R"({"value": 0.0, "tableName": "p_charge_tbl.csv", "tableVar1": "mbmu_soc", "tableVar2": "mbmu_avg_cell_temperature"})";
        cJSON* cjval = cJSON_Parse(tableVar);
        tableAv = vm.setValfromCj(vmap, "/status/bms", "pChargeLimit", cjval);

        asset_manager* bms_man = new asset_manager("bms");
        bms_man->vm = &vm;
        tableAv->am = bms_man;

        cJSON_Delete(cjval);

        CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    }

    virtual void TearDown() { delete tableAv->am; }

    varsmap vmap;       // main data map
    varmap amap;        // map of local variables for asset
    VarMapUtils vm;     // map utils factory
    assetVar* tableAv;  // assetVar referencing the table
};

// Test if the table and the table variables exist in the table assetVar
TEST_F(CheckTableVarTest, table_var_params_valid)
{
    EXPECT_STREQ("p_charge_tbl.csv", tableAv->getcParam("tableName"));
    EXPECT_STREQ("mbmu_soc", tableAv->getcParam("tableVar1"));
    EXPECT_STREQ("mbmu_avg_cell_temperature", tableAv->getcParam("tableVar2"));

    // Check to see if the table ptr and table vars exist in the table assetVar
    // ASSERT_TRUE(tableAv->getvParam("tbl"));
    ASSERT_TRUE(vm.tblMap.find("p_charge_tbl.csv") != vm.tblMap.end());

    assetVar* tblVar1 = amap["mbmu_soc"];
    assetVar* tblVar2 = amap["mbmu_avg_cell_temperature"];
    ASSERT_TRUE(tblVar1);
    EXPECT_STREQ("mbmu_soc", tblVar1->name.c_str());
    EXPECT_EQ(0, tblVar1->getdVal());
    ASSERT_TRUE(tblVar2);
    EXPECT_STREQ("mbmu_avg_cell_temperature", tblVar2->name.c_str());
    EXPECT_EQ(0, tblVar2->getdVal());
}

// Test valid case - both vars are within the range of key-pair values in the
// table
TEST_F(CheckTableVarTest, table_var_within_range)
{
    std::shared_ptr<two_way_tbl> tbl = vm.tblMap.find("p_charge_tbl.csv")->second;

    // Both vars == key-pair values
    double tblVal1 = 30;
    double tblVal2 = 5;
    EXPECT_EQ(0.12, getTableValue(tblVal1, tblVal2, tbl));

    // One var between key value ranges (ex.: 35 between 30 and 40)
    tblVal1 = 35;
    EXPECT_EQ(0.12, getTableValue(tblVal1, tblVal2, tbl));

    assetVar* tblVar1 = amap["mbmu_soc"];
    assetVar* tblVar2 = amap["mbmu_avg_cell_temperature"];

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(0.12, tableAv->getdVal());
}

// Test valid case - both vars are within the range of key-pair values in the
// table (edge case)
TEST_F(CheckTableVarTest, table_var_within_range_edge_case)
{
    // two_way_tbl* tbl = (two_way_tbl*)tableAv->getvParam("tbl");
    std::shared_ptr<two_way_tbl> tbl = vm.tblMap.find("p_charge_tbl.csv")->second;

    // Both vars == lower-bound key-pair values
    double tblVal1 = 0;
    double tblVal2 = -30;
    EXPECT_EQ(0, getTableValue(tblVal1, tblVal2, tbl));

    assetVar* tblVar1 = amap["mbmu_soc"];
    assetVar* tblVar2 = amap["mbmu_avg_cell_temperature"];

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(0, tableAv->getdVal());

    // Both vars == upper-bound key-pair values
    tblVal1 = 100;
    tblVal2 = 65;
    EXPECT_EQ(0, getTableValue(tblVal1, tblVal2, tbl));

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(0, tableAv->getdVal());

    // One var is in the lower bound
    tblVal1 = 0;
    tblVal2 = 54;
    EXPECT_EQ(1, getTableValue(tblVal1, tblVal2, tbl));

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(1, tableAv->getdVal());
}

// Test invalid case - one var is outside the range of key-pair values in the
// table
TEST_F(CheckTableVarTest, table_var_outside_range)
{
    // two_way_tbl* tbl = (two_way_tbl*)tableAv->getvParam("tbl");
    std::shared_ptr<two_way_tbl> tbl = vm.tblMap.find("p_charge_tbl.csv")->second;

    // Var one and var two are less than the lower bound
    double tblVal1 = -1;
    double tblVal2 = -36;
    EXPECT_EQ(0, getTableValue(tblVal1, tblVal2, tbl));

    assetVar* tblVar1 = amap["mbmu_soc"];
    assetVar* tblVar2 = amap["mbmu_avg_cell_temperature"];

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(0, tableAv->getdVal());

    // Var one and var two are greater than the upper bound
    tblVal1 = 101;
    tblVal2 = 66;
    EXPECT_EQ(0, getTableValue(tblVal1, tblVal2, tbl));

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(0, tableAv->getdVal());

    // One var is less than the lower bound
    tblVal1 = -1;
    tblVal2 = 54;
    EXPECT_EQ(0, getTableValue(tblVal1, tblVal2, tbl));

    tblVar1->setVal(tblVal1);
    tblVar2->setVal(tblVal2);

    CheckTableVar(vmap, amap, "bms", nullptr, tableAv);
    EXPECT_EQ(0, tableAv->getdVal());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}