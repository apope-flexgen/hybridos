/*
 * vmap basic test
 */

#include "../src/assetFunc.cpp"
#include "asset.h"
#include "chrono_utils.hpp"

int main(int argc, char* argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;

    // // Test makeVar with assetUri
    // vm.makeVar(vmap, "/components/uri:avar@aparam", nullptr, nullptr);

    // // Test makeAVar with assetUri
    // assetVar* av = new assetVar("avar", "/components/uri", true);
    // vm.makeAVar(vmap, "/components/uri:avar@aparam", "avar", av);

    // // Test replaceAv with assetUri
    // assetVar* newAv = new assetVar("newAVar", "/components/uri", true);
    // vm.replaceAv(vmap, "/components/uri:aNewVar@aparam", "aNewVar", newAv);

    // // Test getVar with assetUri
    // vm.getVar(vmap, "/components/uri:avar", "avar");

    int rc;
    const char* var1 =
        "{\"start_stop\":{\"value\":0,"
        "\"actions\":{"
        "\"onSet\":[{"
        "\"bitfield\":["
        "{ \"inValue\":0, \"uri\": \"/system/new_controls\",\"var\":\"oncmd\",   "
        "    \"outValue\": \"nice\" },"
        "{ \"inValue\":1, \"uri\": "
        "\"/system/new_controls\",\"var\":\"kacclosecmd\", \"outValue\": 34.5 },"
        "{ \"inValue\":8, \"uri\": \"/system/new_controls\",\"var\":\"offcmd\",  "
        "     \"outValue\": true },"
        "{ \"inValue\":9, \"uri\": "
        "\"/system/new_controls\",\"var\":\"kacopencmd\",       \"outValue\": "
        "true }"
        "]"
        "}]"
        "}"
        "}"
        "}";
    const char* rep1 =
        "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":[{\"bitfield\":["
        "{\"inValue\":0,\"uri\":\"/system/"
        "new_controls\",\"var\":\"oncmd\",\"outValue\":\"nice\"},"
        "{\"inValue\":1,\"uri\":\"/system/"
        "new_controls\",\"var\":\"kacclosecmd\",\"outValue\":34.5},"
        "{\"inValue\":8,\"uri\":\"/system/"
        "new_controls\",\"var\":\"offcmd\",\"outValue\":true},"
        "{\"inValue\":9,\"uri\":\"/system/"
        "new_controls\",\"var\":\"kacopencmd\",\"outValue\":true}"
        "]}]}}}";

    const char* rep1a = "{\"start_stop\":{\"value\":0}}";
    //     ",\"actions\":{\"onSet\":{\"bitfield\":["
    //     "{\"inValue\":0,\"uri\":\"/system/new_controls\",\"var\":\"oncmd\",\"outValue\":\"nice\"},"
    //     "{\"inValue\":1,\"uri\":\"/system/new_controls\",\"var\":\"kacclosecmd\",\"outValue\":34.5},"
    //     "{\"inValue\":8,\"uri\":\"/system/new_controls\",\"var\":\"offcmd\",\"outValue\":true},"
    //     "{\"inValue\":9,\"uri\":\"/system/new_controls\",\"var\":\"kacopencmd\",\"outValue\":true}"
    // "]}}}}";
    // const char* rep2 ="{\"/assets/bms_1\":"
    //                 "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"bitfield\":["
    //                         "{\"inValue\":0,\"outValue\":\"nice\",\"uri\":\"/system/new_controls\",\"var\":\"oncmd\"},"
    //                         "{\"inValue\":1,\"outValue\":34.5,\"uri\":\"/system/new_controls\",\"var\":\"kacclosecmd\"},"
    //                         "{\"inValue\":8,\"outValue\":true,\"uri\":\"/system/new_controls\",\"var\":\"offcmd\"},"
    //                         "{\"inValue\":9,\"outValue\":true,\"uri\":\"/system/new_controls\",\"var\":\"kacopencmd\"}"
    //                     "]}}}}}";

    rc = vm.testRes(" Test 1", vmap, "set", "/components/bms_1", var1, rep1);

    {
        cJSON* cj = vm.getMapsCj(vmap);
        char* res = cJSON_Print(cj);
        printf("vmap after test 1 \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
        // return 0;
    }

    rc = vm.testRes(" Test 2", vmap, "get", "/components/bms_1", nullptr, rep1a);

    rc = vm.testRes(" Test 3", vmap, "set", "/components/bms_1/start_stop", "4", "{\"start_stop\":4}");

    rc = vm.testRes(" Test 4", vmap, "set", "/components/bms_1", "{\"start_stop\":3}", "{\"start_stop\":3}");

    rc = vm.testRes(" Test 5", vmap, "get", "/system/new_controls/kacclosecmd", nullptr, "34.5");

    rc = vm.testRes(" Test 6", vmap, "get", "/system/new_controls/oncmd", nullptr, "\"nice\"");

    // rc = vm.testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );
    {
        cJSON* cj = vm.getMapsCj(vmap);
        char* res = cJSON_Print(cj);

        printf("vmap at end default options \n%s\n", res);
        rc = 0;  // -Wall
        if (rc == 0)
            free((void*)res);
        cJSON_Delete(cj);
    }
    {
        cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x0010);
        char* res = cJSON_Print(cj);

        printf("vmap at end dump options \n%s\n", res);
        rc = 0;  // -Wall
        if (rc == 0)
            free((void*)res);
        cJSON_Delete(cj);
    }
    {
        cJSON* cj = vm.getMapsCj(vmap, "/components/bms_1");
        char* res = cJSON_Print(cj);
        printf("components/bms_1 default at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    {
        cJSON* cj = vm.getMapsCj(vmap, "/components");
        char* res = cJSON_Print(cj);
        printf("components  (should trigger a search)  default at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    {
        cJSON* cj = vm.getMapsCj(vmap, "/components", nullptr, 0x0100);
        char* res = cJSON_Print(cj);
        printf("components  (should trigger a search)  reduced comp at end \n%s\n", res);
        free((void*)res);
        // unstable
        cJSON_Delete(cj);
    }
    {
        cJSON* cj = vm.getMapsCj(vmap, "/components", nullptr, 0x0101);
        char* res = cJSON_Print(cj);
        printf(
            "components  (should trigger a search) naked ,  reduced comp at end "
            "\n%s\n",
            res);
        free((void*)res);
        // unstable
        cJSON_Delete(cj);
    }
    // monitor M;
    // M.configure("configs/monitor.json");

    // delete bms_man;
    vm.clearVmap(vmap);

    return 0;
}
