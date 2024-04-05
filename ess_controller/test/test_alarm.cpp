/*
 * test_alarm.cpp
 * phil wilshire 10/27/2020
 *
 */
#include "alarm.h"
#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"
#include "varMapUtils.h"

int main()
{
    VarMapUtils vm;
    alarmUtils au;
    varsmap vmap;
    atypeMap atmap;
    alarmMap almap;

    // create an assetvar
    const char* myval1 = "{\"value\":234,\"params\":{\"maxVal\":220,\"minVal\":53}}";
    cJSON* cj1 = cJSON_Parse(myval1);
    const char* myval2 = "{\"value\":234,\"params\":{\"maxVal\":215,\"moreminVal\":53}}";
    cJSON* cj2 = cJSON_Parse(myval2);

    assetVar* av = vm.setValfromCj(vmap, "/my/alarm/asset", "myAlarmThing", cj1);
    vm.setValfromCj(vmap, "/my/alarm/asset", "myAlarmThing", cj2);

    cJSON* cj3 = vm.getMapsCj(vmap);

    char* tval = cJSON_Print(cj3);
    if (tval && av)
    {
        printf(" Vars at start [%s]\n", tval);
        free((void*)tval);
    }
    double tNow = vm.get_time_dbl();
    alarmObject* ao = au.createAlarm(&vm, atmap, almap, av, "Battery OverTemp", "This battery is too hot", tNow);
    ao->showParams();
    ao->showAlarm();

    cJSON_Delete(cj1);
    cJSON_Delete(cj2);
    cJSON_Delete(cj3);

    return 0;
}
