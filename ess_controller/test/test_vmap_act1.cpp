/*
 * vmap action enum test
 */

#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

int main(int argc, char* argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;
    int rc;
    const char* var1 =
        "{\"start_stop\":{\"value\":0,"
        "\"actions\":{"
        "\"onSet\":[{"
        "\"enum\":["
        "{ \"mask\":3,\"inValue\":0, \"uri\": \"/system/enum_controls:oncmd\",   "
        "     \"outValue\": \"we're on\" },"
        "{ \"mask\":3,\"inValue\":1, \"uri\": \"/system/enum_controls:onval1\",  "
        "     \"outValue\": 222 },"
        "{ \"mask\":3,\"inValue\":2, \"uri\": \"/system/enum_controls:onval2\",  "
        "     \"outValue\": true },"
        "{\"mask\":12,\"inValue\":12,\"uri\":\"/system/enum_controls:onval31\",  "
        "     \"outValue\":3100},"
        "{\"inValue\":12,\"inValue+\":2,\"inValue-\":1,\"uri\":\"/system/"
        "enum_controls:onval31@highLimit\",\"useRange\":true,       "
        "\"outValue\":200},"
        "{\"inValue\":14,\"inValue+\":2,\"inValue-\":1,\"uri\":\"/system/"
        "enum_controls:onval31@highLimit\",\"useRange\":true,       "
        "\"outValue\":300}"

        "]"
        "}]"
        "}"
        "}"
        "}";

    const char* rep1 =
        "{\"start_stop\":{\"value\":0"
        ",\"actions\":{\"onSet\":[{\"enum\":["
        "{\"mask\":3,\"inValue\":0,\"uri\":\"/system/"
        "enum_controls:oncmd\",\"outValue\":\"we're on\"},"
        "{\"mask\":3,\"inValue\":1,\"uri\":\"/system/"
        "enum_controls:onval1\",\"outValue\":222},"
        "{\"mask\":3,\"inValue\":2,\"uri\":\"/system/"
        "enum_controls:onval2\",\"outValue\":true},"
        "{\"mask\":12,\"inValue\":12,\"uri\":\"/system/"
        "enum_controls:onval31\",\"outValue\":3100},"
        "{\"inValue\":12,\"inValue+\":2,\"inValue-\":1,\"uri\":\"/system/"
        "enum_controls:onval31@highLimit\",\"useRange\":true,\"outValue\":200},"
        "{\"inValue\":14,\"inValue+\":2,\"inValue-\":1,\"uri\":\"/system/"
        "enum_controls:onval31@highLimit\",\"useRange\":true,\"outValue\":300}"
        "]}]}}}";

    // const char* rep1 =
    // "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"enum\":["
    //     "{\"inValue\":0,\"outValue\":\"we're
    //     on\",\"mask\":3,\"uri\":\"/system/enum_controls\",\"var\":\"oncmd\"},"
    //     "{\"inValue\":1,\"outValue\":222,\"mask\":3,\"uri\":\"/system/enum_controls\",\"var\":\"onval1\"},"
    //     "{\"inValue\":2,\"outValue\":true,\"mask\":3,\"uri\":\"/system/enum_controls\",\"var\":\"onval2\"},"
    //     "{\"inValue\":12,\"outValue\":3100,\"mask\":12,\"uri\":\"/system/enum_controls:onval31\"}"
    //                                    "]}}}}";

    const char* rep2 =
        "{\"bms_1\":{"
        "\"start_stop\":{\"value\":0"
        ",\"actions\":"
        "{\"onSet\":[{\"enum\":["
        "{\"inValue\":0,\"mask\":3,\"outValue\":\"we're "
        "on\",\"uri\":\"/system/enum_controls:oncmd\"},"
        "{\"inValue\":1,\"mask\":3,\"outValue\":222,\"uri\":\"/system/"
        "enum_controls:onval1\"},"
        "{\"inValue\":2,\"mask\":3,\"outValue\":true,\"uri\":\"/system/"
        "enum_controls:onval2\"},"
        "{\"inValue\":12,\"mask\":12,\"outValue\":3100,\"uri\":\"/system/"
        "enum_controls:onval31\"},"
        "{\"inValue\":12,\"inValue+\":2,\"inValue-\":1,\"outValue\":200,\"uri\":"
        "\"/system/enum_controls:onval31@highLimit\",\"useRange\":true},"
        "{\"inValue\":14,\"inValue+\":2,\"inValue-\":1,\"outValue\":300,\"uri\":"
        "\"/system/enum_controls:onval31@highLimit\",\"useRange\":true}"
        "]}]}}}}";

    // {\"/assets/bms_1\":{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"enum\":["
    //                                "{\"bit\":0,\"mask\":3,\"uri\":\"/system/enum_controls\",\"var\":\"oncmd\",\"bvalue\":\"we're
    //                                on\"},"
    //                                "{\"bit\":1,\"mask\":3,\"uri\":\"/system/enum_controls\",\"var\":\"onval1\",\"bvalue\":222},"
    //                                "{\"bit\":2,\"mask\":3,\"uri\":\"/system/enum_controls\",\"var\":\"onval2\",\"bvalue\":true},"
    //                                "{\"bit\":12,\"mask\":12,\"uri\":\"/system/enum_controls\",\"var\":\"onval31\",\"bvalue\":3100}"
    //                                "]}}}}}";

    rc = vm.testRes(" Test 1", vmap, "set", "/assets/bms_1", var1, rep1);

    rc = vm.testRes(" Test 2", vmap, "get", "/ess/full/assets/bms_1", nullptr, rep2);

    // this is broken we think
    rc = vm.testRes(" Test 3.0", vmap, "set", "/assets/bms_1/start_stop", "0", "{\"start_stop\":0}");
    rc = vm.testRes(" Test 3.1", vmap, "set", "/assets/bms_1", "{\"start_stop\":1}", "{\"start_stop\":1}");
    rc = vm.testRes(" Test 3.2", vmap, "set", "/assets/bms_1", "{\"start_stop\":2}", "{\"start_stop\":2}");

    rc = vm.testRes(" Test 4", vmap, "set", "/assets/bms_1", "{\"start_stop\":13}", "{\"start_stop\":13}");

    rc = vm.testRes(" Test 5", vmap, "get", "/ess/system/enum_controls/onval1", nullptr, "222");

    rc = vm.testRes(" Test 6", vmap, "get", "/ess/system/enum_controls/onval31", nullptr, "3100");

    // rc = vm.testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );

    cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x0110);
    char* res = cJSON_Print(cj);
    rc = 0;  // -Wall
    if (rc == 0)
        printf("#########vmap at end \n%s\n", res);
    free((void*)res);
    cJSON_Delete(cj);

    // monitor M;
    // M.configure("configs/monitor.json");

    // delete bms_man;
    vm.clearVmap(vmap);

    return 0;
}
