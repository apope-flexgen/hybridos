/*
 * vmap alarm test
 * this one uses the featdict and has scale, offset features
 */

#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"
//"alarms":{
//     "name": "Alarm_Group_1",
//     "value": 1,
//     "unit": "",
//     "scaler": 0,
//     "enabled": true,
//     "ui_type": "alarm",
//     "type": "number",
//     "options": [
//     {
//         "name": "HVAC Alarm - NO",
//         "return_value": 1
//     },
//     {
//         "name": "HVAC Alarm - YES?",
//         "return_value": 1
//     }
//     ],
//     "displayValue": 1,
//     "unitPrefix": "",
//     "id": "test_alarm_1"
// }
int main(int argc, char* argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;
    int rc;
    vm.setTime();

    const char* var1 =
        "{\"alarms_1\":{\"value\":0,"
        "\"name\":\"Alarm Group_1\","
        "\"unit\": \"\","
        "\"scaler\": 0,"
        "\"enabled\": true,"
        "\"ui_type\": \"alarm\","
        "\"type\": \"number\","
        "\"options\":["
        "]"
        "}}";

    const char* rep1 =
        "{\"alarms_1\":{\"value\":0,"
        "\"name\":\"Alarm Group_1\","
        "\"unit\":\"\","
        "\"scaler\":0,"
        "\"enabled\":true,"
        "\"ui_type\":\"alarm\","
        "\"type\":\"number\","
        "\"options\":["
        "]"
        "}}";

    const char* var2 =
        "{\"alarms_1\":{"
        "\"value\":1,"
        "\"ui_type\":\"alarm\","
        "\"options\":[{"
        "\"name\": \"HVAC Alarm - NO\","
        "\"return_value\": 1"
        "}]"
        "}}";

    const char* rep2 =
        "{\"alarms_1\":{"
        "\"value\":1,"
        "\"ui_type\":\"alarm\","
        "\"options\":[{"
        "\"name\":\"HVAC Alarm - NO\","
        "\"return_value\":1"
        "}]"
        "}}";

    const char* var3 =
        "{\"alarms_1\":{"
        "\"value\":1,"
        "\"ui_type\":\"alarm\","
        "\"options\":[{"
        "\"name\": \"Cell Voltage Alarm - Yes\","
        "\"return_value\": 1"
        "}]"
        "}}";

    const char* rep3 =
        "{\"alarms_1\":{"
        "\"value\":1,"
        "\"ui_type\":\"alarm\","
        "\"options\":[{"
        "\"name\":\"Cell Voltage Alarm - Yes\","
        "\"return_value\":1"
        "}]"
        "}}";

    // const char* rep3 =
    // "{\"/assets/bms_1\":{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"remap\":"
    //     "[{\"bit\":0,\"offset\":200,\"scale\":10,\"uri\":\"/system/remap_controls\",\"var\":\"rm_start_stop\"}]}}}}}";

    rc = vm.testRes(" Test 1", vmap, "set", "/assets/bms_1", var1, rep1);
    rc = vm.testRes(" Test 2", vmap, "set", "/assets/bms_1", var2, rep2);
    rc = vm.testRes(" Test 3", vmap, "set", "/assets/bms_1", var3, rep3);

    vm.setAlarm(vmap, "assets", "bms_1", "alarms_1", "Battery Voltage Alarm  -Yes", 2);
    vm.setAlarm(vmap, "assets", "bms_1", "alarms_1", "BatteryTemp Alarm  -Yes", 2);

    // rc = vm.testRes(" Test 2", vmap
    //     , "get"
    //     , "/assets/bms_1"
    //     , nullptr
    //     , rep3
    // );

    // // this is broken we think
    // rc = vm.testRes(" Test 3.0", vmap
    //     , "set"
    //     , "/assets/bms_1/start_stop"
    //     , "0"
    //     , "{\"start_stop\":0}"
    // );
    // rc = vm.testRes(" Test 3.1", vmap
    //     , "set"
    //     , "/assets/bms_1"
    //     , "{\"start_stop\":1000}"
    //     , "{\"start_stop\":1000}"
    // );
    // rc = vm.testRes(" Test 3.2", vmap
    //     , "set"
    //     , "/assets/bms_1"
    //     , "{\"start_stop\":2000}"
    //     , "{\"start_stop\":2000}"
    // );

    // rc = vm.testRes(" Test 4", vmap
    //     , "set"
    //     , "/assets/bms_1"
    //     , "{\"start_stop\":13000}"
    //     , "{\"start_stop\":13000}"
    // );

    // rc = vm.testRes(" Test 5", vmap
    //     , "get"
    //     , "/system/remap_controls/rm_start_stop"
    //     , nullptr
    //     , "{\"value\":129800}"
    // );

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

    cJSON* cj = vm.getMapsCj(vmap, "/assets", nullptr, 0x0100);
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
