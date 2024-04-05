/*
 * vmap action remap test
 * this one uses teh featdict and has scale, offset features
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
        "\"onSet\":{"
        "\"remap\":["
        "{ \"bit\":0, \"scale\":10.0,\"offset\":200,\"uri\": "
        "\"/system/remap_controls\",\"var\":\"rm_start_stop\"}"
        "]"
        "}"
        "}"
        "}"
        "}";

    const char* rep1 =
        "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"remap\":"
        "[{\"bit\":0,\"scale\":10,\"offset\":200,\"uri\":\"/system/"
        "remap_controls\",\"var\":\"rm_start_stop\"}]}}}}";

    const char* rep3 =
        "{\"/assets/"
        "bms_1\":{\"start_stop\":{\"value\":0,\"actions\":{"
        "\"onSet\":{\"remap\":"
        "[{\"bit\":0,\"offset\":200,\"scale\":10,\"uri\":\"/"
        "system/remap_controls\",\"var\":\"rm_start_stop\"}]}}}}}";

    rc = vm.testRes(" Test 1", vmap, "set", "/assets/bms_1", var1, rep1);

    rc = vm.testRes(" Test 2", vmap, "get", "/assets/bms_1", nullptr, rep3);

    // this is broken we think
    rc = vm.testRes(" Test 3.0", vmap, "set", "/assets/bms_1/start_stop", "0", "{\"start_stop\":0}");
    rc = vm.testRes(" Test 3.1", vmap, "set", "/assets/bms_1", "{\"start_stop\":1000}", "{\"start_stop\":1000}");
    rc = vm.testRes(" Test 3.2", vmap, "set", "/assets/bms_1", "{\"start_stop\":2000}", "{\"start_stop\":2000}");

    rc = vm.testRes(" Test 4", vmap, "set", "/assets/bms_1", "{\"start_stop\":13000}", "{\"start_stop\":13000}");

    rc = vm.testRes(" Test 5", vmap, "get", "/system/remap_controls/rm_start_stop", nullptr, "{\"value\":129800}");

    // rc = vm.testRes(" Test 6", vmap
    //          ,"get"
    //          ,"/system/enum_controls/onval31"
    //          , nullptr
    //          ,"{\"value\":3100}"
    //      );

    // rc = vm.testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );

    cJSON* cj = vm.getMapsCj(vmap);
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
