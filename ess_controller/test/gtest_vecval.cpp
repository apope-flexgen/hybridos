// Test the new vec val ( doubles only ) plus some stff from calculatevar
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
class VecValTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        double dVal = 0.0;
        av = vm.setVal(vmap, "/status/bms", "BMSTest", dVal);
        av->setVecDepth(16);
        av->setVal(1.1);
        av->setVal(2.2);
        av->setVal(3.3);
        av->setVal(4.4);
    }

    virtual void TearDown() { delete av->am; }

    varsmap vmap;  // main data map
    // varmap amap;        // map of local variables for asset
    VarMapUtils vm;  // map utils factory
    assetVar* av;    // assetVar to hold calcuation results
};

// Test if parameters exist and contain the right values
TEST_F(VecValTest, calculate_var_params_valid)
{
    // check depth
    EXPECT_EQ(16, av->getVecDepth());
    // gets last value two values
    EXPECT_EQ(4.4, av->getVecdVal(0));
    EXPECT_EQ(3.3, av->getVecdVal(1));

    // write new values and check -3
    EXPECT_TRUE(av->setVal(5.5));
    EXPECT_EQ(3.3, av->getVecdVal(2));

    // fill up vector
    EXPECT_TRUE(av->setVal(6.6));    // 5
    EXPECT_TRUE(av->setVal(7.7));    // 6
    EXPECT_TRUE(av->setVal(8.8));    // 7
    EXPECT_TRUE(av->setVal(9.9));    // 8
    EXPECT_TRUE(av->setVal(10.10));  // 9
    EXPECT_TRUE(av->setVal(11.11));  // 10
    EXPECT_TRUE(av->setVal(12.12));  // 11
    EXPECT_TRUE(av->setVal(13.13));  // 12
    EXPECT_TRUE(av->setVal(14.14));  // 13
    EXPECT_TRUE(av->setVal(15.15));  // 14
    EXPECT_TRUE(av->setVal(16.16));  // 15
    // get -15 and -16
    EXPECT_EQ(1.1, av->getVecdVal(15));
    EXPECT_EQ(0, av->getVecdVal(16));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}