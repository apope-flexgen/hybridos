/*
 * vmap deadband test
 *   set a double value 
 *    add 0.1 to it and check valueChangedReset
 * should detect a change
 * set deadband to 0.2 
 * add 0.1 and check valueChangedReset
 * should not detect a change
 * add 0.1001 and check valueChangedReset
 * should detect a change
 * repeat for ints 
 *

 */


#include <math.h>
#include "asset.h"
#include "chrono_utils.hpp"
#include "gtest/gtest.h"

namespace flex
{
    const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

cJSON*getSchList()
{
    return nullptr;
}


class DeadbandTest : public ::testing::Test {
    protected:
        virtual void SetUp(){
            double dVal = 1.0;
            av = vm.setVal(vmap, "/system/test", "DeadBandTest", dVal);
        }
        virtual void TearDown() {
            delete av->am;
        }
        varsmap vmap;
        VarMapUtils vm;
        assetVar* av;
};

TEST_F(DeadbandTest, testVal){
    double dVal = 1.0;
    //EXPECT_TRUE(av->setVal(dVal + 0.0));
    av->setVal(dVal + 0.0);
    EXPECT_FALSE(av->valueChangedReset());

    EXPECT_TRUE(av->setVal(dVal + 0.1));
    EXPECT_TRUE(av->valueChangedReset());

    double dbVal = 0.1;
    av->setDbVal(dbVal);
    EXPECT_FALSE(av->valueChangedReset());

    EXPECT_TRUE(av->setVal(dbVal + 0.150));
    //av->setVal(dbVal + 0.150);
    EXPECT_TRUE(av->valueChangedReset());

    //EXPECT_TRUE(av->setVal(dbVal + 0.201));
    av->setVal(dbVal + 0.201);
    EXPECT_FALSE(av->valueChangedReset());

    //EXPECT_TRUE(av->setVal(dbVal + 0.201));
    av->setVal(dbVal + 0.201);
    EXPECT_FALSE(av->valueChangedReset());

    //EXPECT_TRUE(av->setVal(dbVal + 0.104));
    av->setVal(dbVal + 0.104);
    EXPECT_FALSE(av->valueChangedReset()); 

    //EXPECT_TRUE(av->setVal(dbVal + 0.100));
    av->setVal(dbVal + 0.100);
    EXPECT_FALSE(av->valueChangedReset());
}

int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
