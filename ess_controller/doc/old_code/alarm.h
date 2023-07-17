#ifndef ALARM_HPP
#define ALARM_HPP
/*
* alarms etc
*/
// "alarms":{
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

// set an alarm 
// "alarms":{
//     "options": [
//     {
//         "name": "HVAC Alarm - NO",
//         "return_value": 1
//     }
//     ]
// }


#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <malloc.h>
#include <cjson/cJSON.h>
#include <poll.h>
#include <signal.h>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <fims/libfims.h>
#include <fims/fps_utils.h>


#include "asset.h"
#include "varMapUtils.h"


// alarms/faults belong to asset or asset_manager ( candidate for merging into the base class)
//fix showvarcj to detect an alarm type
// setold

// alarmType .. not predefined, make them up as we go....
class alarmType {
public:
    alarmType(const char* _name) { name = _name; };
    ~alarmType() {};

    std::string name;
    std::string desc;

    std::vector <double> instances;

};

// the alarm class
class alarmObject {

public:
    alarmObject() {};
    ~alarmObject() {};

    void showParams()
    {
        params->showFeat();
    }
    void showAlarm()
    {
        char* tmp = nullptr;
        cJSON* cjv = aVal->getValCJ();
        if (cjv)
        {
            tmp = cJSON_PrintUnformatted(cjv);

            printf(" Alarm type [%s] occured at %f from %s  value [%s] -> %s\n"
                , atype->name.c_str()
                , time
                , assetname.c_str()
                , tmp
                , details.c_str()
            );
            if (tmp)free((void*)tmp);
        }
    }


    double time;
    std::string name;
    std::string details;
    alarmType* atype;
    std::string assetname; // "/asset/bms/bms_1/rack3/battery21:CellTemp
    int depth;
    assetVal* aVal;
    assetFeatDict* params;  // copy
    std::vector<assetVal*> aVals;// ( copy)

};

typedef std::map<double, std::vector<alarmObject*>> alarmMap;// give us a list of alarm objects at this time. 
typedef std::map<std::string, alarmType*> atypeMap;

// class for manipulating alarms
class alarmUtils {
public:
    alarmUtils() {};
    ~alarmUtils() {};

    alarmType* getaType(atypeMap& atmap, const char* atype, alarmObject* ao)
    {
        alarmType* at;
        if (atmap.find(atype) == atmap.end())
        {
            at = new alarmType(atype);
            atmap[atype] = at;
        }
        else
        {
            at = atmap[atype];
        }
        // HMM multiple instances at the same time.. I guess its OK
        at->instances.push_back(ao->time);
        return at;
    }


    alarmObject* createAlarm(VarMapUtils* vm, atypeMap& atmap, alarmMap& almap, assetVar* av, const char* atype, const char* details, double tNow)
    {
        //double tNow = vm->get_time_dbl();
        alarmObject* ao = new alarmObject;
        ao->time = tNow;
        ao->params = std::move(av->featDict);
        ao->aVals = std::move(av->aVals);

        ao->details = details;
        ao->depth = av->depth;
        ao->aVal = std::move(av->aVal);

        ao->atype = getaType(atmap, atype, ao);
        ao->assetname = av->comp;
        ao->assetname += ":";
        ao->assetname += av->name;

        // if (almap[tNow] == almap->end())
        // {

        // }
        almap[tNow].push_back(ao);
        return ao;
    };

};


#endif
