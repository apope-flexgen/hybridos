/*
 *test alarm and fault vars
 */

#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"
//  "alarms": {
//       "name": "Alarms",
//       "value": 0,
//       "options": [],
//       "unit": "",
//       "scaler": 0,
//       "enabled": true,
//       "ui_type": "alarm",
//       "type": "number"
//     },

int main(int argc, char* argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;
    int rc;
    const char* var1 =
        "{\"alarms\":{\"value\":0,"
        "\"name\":\"Test Alarm\","
        "\"ui_type\":\"alarm\","
        "\"options\":[]"
        "}"
        "}";

    const char* rep1 =
        "{\"alarms\":{"
        "\"value\":0,"
        "\"name\":\"Test Alarm\","
        "\"ui_type\":\"alarm\","
        "\"options\":[]"
        "}}";

    const char* var2 =
        "{\"faults\":{\"value\":0,"
        "\"name\":\"Test Faults\","
        "\"ui_type\":\"alarm\","
        "\"options\":[]"
        "}"
        "}";

    const char* rep2 =
        "{\"faults\":{"
        "\"value\":0,"
        "\"name\":\"Test Faults\","
        "\"ui_type\":\"alarm\","
        "\"options\":[]"
        "}}";
    // const char* rep2 ="{\"/assets/bms_1\":"
    //                 "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"bitfield\":["
    //                         "{\"inValue\":0,\"outValue\":\"nice\",\"uri\":\"/system/new_controls\",\"var\":\"oncmd\"},"
    //                         "{\"inValue\":1,\"outValue\":34.5,\"uri\":\"/system/new_controls\",\"var\":\"kacclosecmd\"},"
    //                         "{\"inValue\":8,\"outValue\":true,\"uri\":\"/system/new_controls\",\"var\":\"offcmd\"},"
    //                         "{\"inValue\":9,\"outValue\":true,\"uri\":\"/system/new_controls\",\"var\":\"kacopencmd\"}"
    //                     "]}}}}}";
    // printf(" var1 >>\n%s\n", var1);

    rc = vm.testRes(" Test 1", vmap, "set", "/components/bms_1", var1, rep1);
    rc = vm.testRes(" Test 2", vmap, "set", "/components/bms_1", var2, rep2);

    {
        cJSON* cj = vm.getMapsCj(vmap);
        char* res = cJSON_Print(cj);
        printf("vmap after test 1 \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
        // return 0;
    }
    vm.sendAlarm(vmap, "/status/ess:test_alarm_one", "/components/bms_1:alarms", "test1", "test Alarm", 1);
    vm.sendAlarm(vmap, "/status/ess:test_alarm_one", "/components/bms_1:alarms", "test2", "another test Alarm", 2);
    vm.sendAlarm(vmap, "/status/ess:test_fault_one", "/components/bms_1:faults", "test1", "test Fault", 2);

    // rc = vm.testRes(" Test 2", vmap
    //         , "get"
    //         , "/components/bms_1"
    //         , nullptr
    //         , rep2
    //     );

    // rc = vm.testRes(" Test 3", vmap
    //          ,"set"
    //          ,"/components/bms_1/start_stop"
    //          ,"4"
    //          ,"{\"start_stop\":4}"
    //      );

    // rc = vm.testRes(" Test 4", vmap
    //          ,"set"
    //          ,"/components/bms_1"
    //          ,"{\"start_stop\":3}"
    //          ,"{\"start_stop\":3}"
    //      );

    // rc = vm.testRes(" Test 5", vmap
    //          ,"get"
    //          ,"/system/new_controls/kacclosecmd"
    //          , nullptr
    //          ,"{\"value\":34.5}"
    //      );

    // rc = vm.testRes(" Test 6", vmap
    //          ,"get"
    //          ,"/system/new_controls/oncmd"
    //          , nullptr
    //          ,"{\"value\":\"nice\"}"
    //      );

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

    vm.clearAlarm(vmap, "/status/ess:test_alarm_one", "/components/bms_1:alarms", "test1", "test Alarm Cleared ", 1);
    // vm.sendAlarm(vmap, "/status/ess:test_alarm_one",
    // "/components/bms_1:alarms", "test2", "another test Alarm", 2);
    // vm.setParam(vmap, "/components", "bms_1", "battery1", "MaxVoltage", 4.2);
    // vm.setParam(vmap, "/components", "bms_1", "battery1", "MinVoltage", 2.2);
    // double minv = vm.getdParam(vmap, "/components", "bms_1", "battery1",
    // "MinVoltage");

    {
        cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x0010);
        char* res = cJSON_Print(cj);

        printf("after clear bms_1:alarms test1 \n%s\n", res);
        rc = 0;  // -Wall
        if (rc == 0)
            free((void*)res);
        cJSON_Delete(cj);
    }

    vm.clearAlarms(vmap, "/components/bms_1:alarms");
    {
        cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x0010);
        char* res = cJSON_Print(cj);

        printf("after clear all bms_1:alarms\n%s\n", res);
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
    // printf(" min voltage = %f\n", minv);
    return 0;
}
