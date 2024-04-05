/*
 * vmap basic test
 */

#include "asset.h"
#include "chrono_utils.hpp"
#include "gtest/gtest.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <malloc.h>
#include <map>
#include <mutex>
#include <pthread.h>
#include <string>
#include <vector>

#include <cjson/cJSON.h>
#include <fims/fps_utils.h>
#include <fims/libfims.h>

//#include "../src/assetFunc.cpp"

namespace flex
{
const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}  // namespace flex

cJSON* getSchList()
{
    return nullptr;
}

class Vmap0 : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        av = vm.makeVar(vmap, "/comp/ess", "myvar", nullptr);
        double dval = 456.78;
        vm.setVal(vmap, "/components/vmap_test", "test_float", dval);
        double ival = 123;
        vm.setVal(vmap, "/components/vmap_test", "test_int", ival);
        char* cval = (char*)"some_string_thing";
        vm.setVal(vmap, "/components/vmap_test", "test_string", cval);
        char* cval2 = (char*)"some_string_value";
        vm.setVal(vmap, "/components/vmap_test", "test_string2", cval2);
        ival = 2000;
        vm.setVal(vmap, "/system/status", "active_current_setpoint", ival);
        ival = 100;
        vm.setVal(vmap, "/system/status", "soc", ival);
        char* cval3 = (char*)"standby";
        vm.setVal(vmap, "/system/status", "status", cval3);
    }

    virtual void TearDown()
    {
        delete av->am;
        free((void*)res);
        cJSON_Delete(cj);
    }

    varsmap vmap;
    VarMapUtils vm;
    assetVar* av;
    cJSON* cj;
    char* res;
};

TEST_F(Vmap0, vmap0test)
{
    // EXPECT_STREQ()
    cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr);
    char* res = cJSON_PrintUnformatted(cj);
    char *eres =
      (char *)"{\"/comp/ess\":{\"ess\":{\"myvar\":{\"value\":true}}},\"/"
              "components/"
              "vmap_test\":{\"vmap_test\":{\"test_float\":{\"value\":456.78},"
              "\"test_int\":{\"value\":123},\"test_string\":{\"value\":\"some_"
              "string_thing\"},\"test_string2\":{\"value\":\"some_string_"
              "value\"}}},\"/system/"
              "status\":{\"status\":{\"active_current_setpoint\":{\"value\":"
              "2000},\"soc\":{\"value\":100},\"status\":{\"value\":\"standby\"}"
              "}}}";
    EXPECT_STREQ(eres, res);

    // naked 0x0001
    cj = vm.getMapsCj(vmap, nullptr, nullptr, 1);
    res = cJSON_PrintUnformatted(cj);
    eres = (char *)"{\"/comp/ess\":{\"ess\":{\"myvar\":true}},\"/components/"
                 "vmap_test\":{\"vmap_test\":{\"test_float\":456.78,\"test_"
                 "int\":123,\"test_string\":\"some_string_thing\",\"test_"
                 "string2\":\"some_string_value\"}},\"/system/"
                 "status\":{\"status\":{\"active_current_setpoint\":2000,"
                 "\"soc\":100,\"status\":\"standby\"}}}";
    EXPECT_STREQ(eres, res);

    // individual tables 0x001
    cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x01);
    res = cJSON_PrintUnformatted(cj);
    eres = (char *)"{\"/comp/ess\":{\"ess\":{\"myvar\":true}},\"/components/"
                 "vmap_test\":{\"vmap_test\":{\"test_float\":456.78,\"test_"
                 "int\":123,\"test_string\":\"some_string_thing\",\"test_"
                 "string2\":\"some_string_value\"}},\"/system/"
                 "status\":{\"status\":{\"active_current_setpoint\":2000,"
                 "\"soc\":100,\"status\":\"standby\"}}}";
    EXPECT_STREQ(eres, res);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
