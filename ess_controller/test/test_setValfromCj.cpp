/*
 * test_setValfromCj.cpp
 * phil wilshire 12/10/2020
 * test vlink
 *
 * defined to test the setValfromCj function that reads all kinds of objects into the varsma.... 
 */
#include "asset.h"
#include "varMapUtils.h"
#include "alarm.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

//Uri:     /components/sel_3530_rtac
//ReplyTo: (null)
//Body:    {"frequency":60.003,"voltage_l1":7443,"voltage_l2":7397,"voltage_l3":7410,"current_l1":10,"current_l2":15,"current_l3":21,"current_n":44,"active_power":11.706,
//           "reactive_power":-152.031,"power_factor":0.03,"apparent_power":368.154,"Timestamp":"07-23-2020 14:46:28.514502"}
//Timestamp:   2020-07-23 14:46:28.514839
//TODO DONE Multiple actions loose order
//TODO DONE  Actions replace all of then each setup

//TODO DONE check "params" ( may not be used)

//TODO DONE we alsocreate an assetlist for this one. if we see "name" then its all one thing all other things are params



//TODO check set /a/b/c:ar@param

//TODO create an ALIST regardless of UiObject  well use a VEC instead (TODO) new params will append but it removes the whole alist thing....

//TODO  Done OPTIONS, they are kept in order. 

// TODO check full Ui load 


int main ()
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

    const char* myval3 = "{\"value123\":123,\"value234\":234,\"value235\":235}";//  {\"maxVal\":215,\"moreminVal\":53}}";
    cJSON* cj3 = cJSON_Parse(myval3); 
    //This one tests the creation of a single action 
    const char *var1 = "{\"start_stop\":{\"value\":0,"
            "\"actions\":{"
                 "\"onSet\":{"
                    "\"remap\":["
                        "{ \"bit\":0, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop1\"},"
                        "{ \"bit\":1, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop2\"},"
                        "{ \"bit\":3, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop3\"}"
                    "]"
                "}"
            "}"
        "}"
    "}";
    //This one tests replacement actions
    const char *var2 = "{\"start_stop\":{\"value\":0,"
            "\"actions\":{"
                 "\"onSet\":{"
                    "\"remap\":["
                        "{ \"bit\":4, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop1\"},"
                        "{ \"bit\":5, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop2\"},"
                        "{ \"bit\":6, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop3\"}"
                    "]"
                "}"
            "}"
        "}"
    "}";
    //This one tests different types of actions
    const char *var3 = "{\"start_stop\":{\"value\":0,"
            "\"actions\":{"
                "\"onSet\":{"
                    "\"remap\":["
                        "{ \"bit\":4, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop1\"},"
                        "{ \"bit\":5, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop2\"},"
                        "{ \"bit\":6, \"scale\":1.0,\"offset\":0,\"uri\": \"/system/remap_controls:start_stop1@test_param\"}"
                    "],"
                    "\"enum\":["
                        "{ \"bit\":4, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop1\"},"
                        "{ \"bit\":5, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop2\"},"
                        "{ \"bit\":6, \"scale\":10.0,\"offset\":200,\"uri\": \"/system/remap_controls\",\"var\":\"start_stop3\"}"
                    "]"
                "}"
            "},"
            "\"options\":["
            "{\"name\":\"Clear Faults\",\"return_value\":\"Clear\"}"
            "]"
        "}"
    "}";

    const char* var4 = "{\"name\":\"This is my name\",\"i234\":234,\"s235\":\"235\",\"aLastParam\":0.123}";//  {\"maxVal\":215,\"moreminVal\":53}}";
    const char* var5 = "{\"name\":\"This is my name\",\"value\":34.5, \"i234\":234,\"s235\":\"235\",\"aLastParam\":0.123}";//  {\"maxVal\":215,\"moreminVal\":53}}";

    const char* var6 = "{\"name\":\"This is my name\",\"value\":34.5, \"params\":{\"i234\":234,\"s235\":\"235\",\"aLastParam\":0.123}}";//  {\"maxVal\":215,\"moreminVal\":53}}";

    const char* var7 = "{\"name\":\"This is my name\",\"value\":34.5, \"options\":[{\"firstOption\":234},{\"amidOption\":\"235\"},{\"aLastOption\":0.123}]}}";//  {\"maxVal\":215,\"moreminVal\":53}}";

    const char* var8 =  "{"
    "\"name\":\"Ess Controller\","
    "\"active_power\":          {\"name\": \"Active Power\",          \"value\": 17,        \"unit\": \"W\", \"scaler\": 1000, \"enabled\": false, \"ui_type\": \"status\", \"type\": \"number\"},"
    "\"active_power_setpoint\": {\"name\": \"Active Power Setpoint\", \"value\": 3.0999999, \"unit\": \"W\", \"scaler\": 1000, \"enabled\": false, \"ui_type\": \"status\", \"type\": \"number\"},"
    "\"alarms\":                {\"name\": \"Alarms\",       \"value\": 0, \"options\": [], \"unit\": \"\",  \"scaler\": 0,    \"enabled\": true,  \"ui_type\": \"alarm\", \"type\": \"number\" },"
    "\"soc\": {\"name\": \"State of Charge\",                \"value\": 76.559135,          \"unit\": \"%\", \"scaler\": 1,    \"enabled\": false, \"ui_type\": \"status\", \"type\": \"number\"},"
    "\"soh\": {\"name\": \"State of Health\",                \"value\": 0,                  \"unit\": \"%\", \"scaler\": 1,    \"enabled\": false, \"ui_type\": \"status\", \"type\": \"number\"},"
    "\"status\": {\"name\": \"Status\",                      \"value\": \"Run\",            \"unit\": \"\",  \"scaler\": 0,    \"enabled\": true,  \"ui_type\": \"status\", \"type\": \"number\"},"
    "\"maint_mode\": {\"name\": \"Maintenance Mode\",        \"value\": false,       \"unit\": \"\",  \"scaler\": 0,    \"enabled\": true,  \"ui_type\": \"control\", \"type\": \"enum_slider\","
          "\"options\": ["
          "{\"name\": \"No\",\"return_value\": false},"
          "{\"name\": \"Yes\",\"return_value\": true}"
          "]}"
    "}";
    cJSON* cjvar1 = cJSON_Parse(var1); 
    cJSON* cjvar2 = cJSON_Parse(var2); 
    cJSON* cjvar3 = cJSON_Parse(var3); 
    cJSON* cjvar4 = cJSON_Parse(var4); 

// "/my/param/test":       {
//                 "myParamThing": {
//                         "value":        false,
//                         "i234": 234,
//                         "name": "This is my name",
//                         "s235": "235",   
//                         "i234": 234,  << error this gets printed twice 
//                         "name": "This is my name",
//                         "s235": "235"
//                 }
//         },

    cJSON* cjvar5 = cJSON_Parse(var5); 
    cJSON* cjvar6 = cJSON_Parse(var6); 

    cJSON* cjvar7 = cJSON_Parse(var7); 
    cJSON* cjvar8 = cJSON_Parse(var8); 

// "/vlinks/ess":{
//   "MinCellVolt": {
//     "value": "/site/ess:MinCellVolt",
//     "vlink": "/components/catl_mbmu_control_r:mbmu_min_cell_voltage"
//   }
    // /sites/ess
    const char* var9_1 = "{\"name\":\"Min Cell Volts\",\"value\":34.5 ,\"range\":4567,\"somestrng\":\"site variable\"}";
    // /components/catl_mbmu_control_r
    const char* var9_2 = "{\"name\":\"ModBus Min Cell Volts\",\"value\":3.5 ,\"offset\":3456,\"someptherparam\":\"modbus variable\"}";
    //vlinks/ess 
    const char* var9_3 = "{\"val\":true, \"vlink\":\"/components/catl_mbmu_control_r:mbmu_min_cell_voltage\", \"value\":\"/site/ess:MinCellVolts\"}";

    cJSON* cjvar9_1 = cJSON_Parse(var9_1); 
    cJSON* cjvar9_2 = cJSON_Parse(var9_2); 
    cJSON* cjvar9_3 = cJSON_Parse(var9_3); 


    assetVar*av = vm.setValfromCj(vmap, "/my/alarm/asset", "myAlarmThing", cj1);
    assetVar*av2 = vm.setValfromCj(vmap, "/my/alarm/asset", "myAlarmThing", cj2);

    assetVar*av9_1 = vm.setValfromCj(vmap, "/site/ess",                      "MinCellVolts",          cjvar9_1);
    assetVar*av9_2 = vm.setValfromCj(vmap, "/components/catl_mbmu_control_r", "mbmu_min_cell_voltage", cjvar9_2);
    //assetVar*av9_3 = 
    vm.setValfromCj(vmap, "/vlinks/ess",                     "MinCellVolts",          cjvar9_3);
 
    double dval =  av9_2->getdVal();
    printf(" dval  [%f]\n", dval);
 

    cJSON* cj4 = vm.getMapsCj(vmap, nullptr, nullptr, 0x0110);

    char* tval = cJSON_Print(cj4);
    if(tval && av)
    {
        printf(" Vars at start [%s]\n", tval);
        free((void*)tval);

    }
    // if we dont see a name or a value field we do this ...
    //assetVar*av3 = 
    // cJSON*cj3i = cj3;
    // if (cj3->child)
    //   cj3i = cj3->child;
    // while(cj3i)
    // {
    vm.setValfromCj(vmap, "/my/alarm/tests", nullptr, cj3);
    //     cj3i = cj3i->next;
    // }
    cJSON* cj5 = vm.getMapsCj(vmap, nullptr, nullptr, 0x0100);

    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars after multi object [%s]\n", tval);
        free((void*)tval);

    }
    cJSON_Delete(cj5);
    vm.setValfromCj(vmap, "/my/remap/test", nullptr, cjvar1);

    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);

    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars after new action  [%s]\n", tval);
        free((void*)tval);

    }
    cJSON_Delete(cj5);

    vm.setValfromCj(vmap, "/my/remap/test", nullptr, cjvar2);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);

    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars after new action 2  [%s]\n", tval);
        free((void*)tval);

    }
    vm.setValfromCj(vmap, "/my/remap/test", nullptr, cjvar3);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars after multiple actions [%s]\n", tval);
        free((void*)tval);

    }
    // name only param should keep param order 
    // TODO getmapsCJ bug
    //assetVar*av2 = 
    // 
    vm.setValfromCj(vmap, "/my/param/name", "myParamThing", cjvar4);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars name only param [%s]\n", tval);
        free((void*)tval);

    }
    // name only param should keep param order 
    // this is added second to an alist flagged comp name
    // the order should be preserved. ( not among params) 
    //assetVar*av2 = 
    // 
    vm.setValfromCj(vmap, "/my/param/name", "aaParamThing", cjvar4);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars name only param [%s]\n", tval);
        free((void*)tval);

    }

    // name + value param
    // TODO getmapsCJ bug
    //assetVar*av2 = 
    vm.setValfromCj(vmap, "/my/param/nameValue", "myParamThing", cjvar5);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars name + value param [%s]\n", tval);
        free((void*)tval);

    }

    // name + value + "param":
    // TODO getmapsCJ bug
    //assetVar*av2 = 
    vm.setValfromCj(vmap, "/my/param/nameParam", "myParamThing", cjvar6);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars name + value param [%s]\n", tval);
        free((void*)tval);
    }

    // name + value + "options":
    // TODO getmapsCJ bug
    //assetVar*av2 = 
    // "/my/options/name":     {
    //             "myOPtionsThing":       {
    //                     "value":        34.5,
    //                     "name": "This is my name",
    //                     "options":      [{
    //                                     "firstOption":  234
    //                             }, {
    //                                     "amidOption":   "235"
    //                             }, {
    //                                     "aLastOption":  0.123
    //                             }],
    //                     "name": "This is my name"
    //             }
    //     },

    vm.setValfromCj(vmap, "/my/options/name", "myOPtionsThing", cjvar7);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Options 1 [%s]\n", tval);
        free((void*)tval);
    }

    vm.setValfromCj(vmap, "/assets/ess/summary", nullptr, cjvar8);
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf("/assets/ess/summary [%s]\n", tval);
        free((void*)tval);
    }

    // all looks good so far
    // assetVal is 72 bytes long and we have 3 of them !!!
    // move deadband to a param
    // assetVal *aval = new assetVal(dval);
    // printf(" create dummy aval %p\n", aval);

    // HMM still 400 bytes
    // removed aVals down to 376 bytes
    // removed mutex 336
    // removed alarms ... 264
    // actVec also adds 48 bytes (gives us 216) (216 - 144 = 72 )
    // size without  aVals 72 ...
    //
    // put actions and alarms into extras
    // no aVal/lVal for links// enums  ATYPE = ASTRING
    //
    // curretly 288 bytes but we need to remove actions and alarms

    // double dval=1234.0;
    // assetVar *avt = new assetVar("foo",dval);
    // printf(" create dummy assetVar %p\n", avt);


    // we have the ui stuff we all ready have setup

/// Test some alarm stuff ... not really related yet
//lets get "maint_mode" from /assets/ess/summary

    assetVar* mmav = vm.getVar(vmap, "/assets/ess/summary", "maint_mode");
    printf("/assets/ess/summary [%s] %p\n", "maint_mode", mmav);
    cJSON*cjv = cJSON_CreateObject();
    mmav->showvarCJ(cjv,0x0110);

    tval = cJSON_Print(cjv);
    if(tval)
    {
        printf("maint_mode enabled true \n[%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cjv);


    mmav->setParam("enabled",false);

    // show the variable 
    cjv = cJSON_CreateObject();
    mmav->showvarCJ(cjv,0x0110);
    tval = cJSON_Print(cjv);
    if(tval)
    {
        printf("maint_mode enabled false\n[%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cjv);

    // now try the complex route
    bool bval = true;
    vm.setVal(vmap,"/assets/ess/summary:maint_mode@enabled", nullptr, bval);
    cjv = cJSON_CreateObject();
    mmav->showvarCJ(cjv,0x0110);
    tval = cJSON_Print(cjv);
    if(tval)
    {
        printf("maint_mode enabled true, using setVal \n[%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cjv);

    assetVar* aaav = vm.getVar(vmap, "/my/remap/test", "start_stop");
    printf("/my/remap/test [%s] %p\n", "start_stop", aaav);
    int ival = 456;
    vm.setVal(vmap,"/my/remap/test:start_stop", nullptr, ival);

    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,"/assets/ess/summary",nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf("/assets/ess/summary  after remap test [%s]\n", tval);
        free((void*)tval);
    }



    double tNow = vm.get_time_dbl();
    alarmObject* ao = 
    au.createAlarm(&vm, atmap, almap, av, "Battery OverTemp", "This battery is too hot", tNow);
    ao->showParams();
    //ao->showAlarm();

    vm.setVLinks(vmap);
    double dval9_1 = 2.123;
    //av9_1->linkVar = av9_2;
//
    //av9_1->setVal(dval9_1); 
    double dval9_2 = av9_2->getdVal();

    {
        printf(" dval9_2 av9_1->[%s:%s]->linkVar %p is %2.3f \n"
            , av9_1->comp.c_str()
            , av9_1->name.c_str()
            , av9_1->linkVar
            , dval9_2
            );
        //delete av2;
    }
    cJSON_Delete(cj5);
    cj5 = vm.getMapsCj(vmap,nullptr,nullptr,0x0110);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" Vars after vlinks [%s]\n", tval);
        free((void*)tval);

    }

    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_1->showvarValueCJ(cj5, 0x010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_1  var value [%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_2->showvarValueCJ(cj5, 0x010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_2  var value [%s]\n", tval);
        free((void*)tval);
    }
    av9_1->setVal(dval9_1); 

    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_1->showvarValueCJ(cj5, 0x010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_1  2 var value [%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_2->showvarValueCJ(cj5, 0x010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_2  2 var value [%s]\n", tval);
        free((void*)tval);
    }
    
    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_1->showvarCJ(cj5, 0x0010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_1  site var [%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_2->showvarCJ(cj5, 0x0010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_2  modbus var [%s]\n", tval);
        free((void*)tval);
    }

    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_1->showvarValueCJ(cj5, 0x010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_1  3 var value [%s]\n", tval);
        free((void*)tval);
    }
    cJSON_Delete(cj5); cj5 = cJSON_CreateObject();
    av9_2->showvarValueCJ(cj5, 0x010);
    tval = cJSON_Print(cj5);
    if(tval)
    {
        printf(" av9_2  3 var value [%s]\n", tval);
        free((void*)tval);
    }

    // system ("mkdir -p run_logs");
    // av9_2->openLog("run_logs/test_log", 1);
    // av9_2->sendLog(nullptr, "%s >> This is my log, value %2.3f\n", __func__, av9_2->getdVal());
    // av9_2->closePerf();
    

    au.clearAlarms(almap, atmap);

    vm.clearVmapxx(vmap);

    // down to 144 bytes 12/13/2020
    int ival1 = 1;
    assetVar* aav = new assetVar("foo",ival1);

    assetExtras *ae = nullptr; //new assetExtras;
    //delete av;
    double *td = new double;
    *td = 123.0;

    if(ae) delete ae;
    //assfeat 56 bytes
    //assFeat* af = new assFeat("foo",21);
//if(av != av2)
    {
        printf(" OK created a new assetextras av %p aav %p av2 %p %2.3f \n", ae , aav, av2, *td );
        //delete av2;
    }
    delete td;
    delete aav;

    cJSON_Delete(cj1);
    cJSON_Delete(cj2);
    cJSON_Delete(cj3);
    cJSON_Delete(cj4);
    cJSON_Delete(cj5);
    cJSON_Delete(cjvar1);
    cJSON_Delete(cjvar2);
    cJSON_Delete(cjvar3);
    cJSON_Delete(cjvar4);
    cJSON_Delete(cjvar5);
    cJSON_Delete(cjvar6);
    cJSON_Delete(cjvar7);
    cJSON_Delete(cjvar8);
    cJSON_Delete(cjvar9_1);
    cJSON_Delete(cjvar9_2);
    cJSON_Delete(cjvar9_3);
    
    return 0;

}
