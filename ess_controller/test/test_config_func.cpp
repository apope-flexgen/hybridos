/*
* test from the config file.
* an extension of the wake_manage thing
as in
1/ get the parser to read in a funcarray of funcItems
"/schedule/ess":{
    "init": {
        "functions":"InitEss"
    },

    "every100mS": {    // assetvar
        "rate":0.1,         //params as usua;
        "offset":0.0,
        "actions":{
            "onSet" [ { "exec" :[                        // special param
functions
                {"func": "UpdateSystemTime"},         // we'll have to parse
funitems lets try to use a dict for this funVec
                {"func":"RunInitCheck":,
                {"func":"RunComsCheck":,
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_control_r:mbmu_max_cell_voltage",     "enable":
true,"aname":"bms"}},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_control_r:mbmu_min_cell_voltage",     "enable":
true,"aname":"bms"}},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_control_r:mbmu_max_cell_temperature", "enable":
true,"aname":"bms"}},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_control_r:mbmu_min_cell_temperature", "enable":
true,"aname":"bms"}},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_control_r:mbmu_soc",                  "enable":
true,"aname":"bms"}},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_control_r:mbmu_soh",                  "enable":
true,"aname":"bms"}},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_stat_r:bms_heartbeat",                 "enable":
true,"aname":"bms"},
                {"func":"CheckMonitorVar",
"Av":"/components/catl_mbmu_stat_r:bms_timestamp",                 "enable":
true,"aname":"bms"}},
                {"func":"HandlePowerLimit"},
                {"func":"HandlePowerCmd":},
                {"func":"ShutDownSystem":},
                {"func":"StartUpSystem":}
        ]
    }
  },
* 1/ basic define a function and run it onSet thouugh the config
* 2/ define a component function compFun   executed  on any writes to that
component (read table)
* 3 / DEfine an assetVar function for onSet with the asset.
*/
#include "asset.h"
#include "assetVar.h"
#include "chrono_utils.hpp"
//#include "bms.h"
//#include "pcs.h"
//#include "assetFunc.cpp"

//  "/assets/bms":        {
//                 "bms_1":   {
//                                    "template":"bms_catl_template.json".
//                                    "subs":[
//                                       {"replace":"@@BMS_ID@@","with":"bms_1"},
//                                       {"replace":"@@BMS_IP@@","with":"192.168.1.114"}
//                                       ]
//                                   }
//            }
extern "C++" {
int TestFunc1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int process_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckAmBmsStatus(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

int TestFunc1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    printf(" %s >> running for av [%s] in am [%s]\n", __func__, av->name.c_str(), aname);
    return 0;
}

int process_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    printf(" %s >> running for av [%s] in am [%s]\n", __func__, av->name.c_str(), aname);
    return 0;
}

int test_main(int argc, char* argv[])
{
    // const char* res;
    char* rbuf;

    // asprintf(&rbuf,"{\"method\":\"set\",\"uri\":\"/assets/sungrow_ess_1\"
    // ,\"body\":\"{\"close_dc_contactors\":{\"value\":true}}}");
    // asprintf(&rbuf,"{\"method\":\"set\",\"uri\":\"/assets/sungrow_ess_1\",\"body\":{\"Some
    // body\"}}"); //{\"close_dc_contactors\":{\"value\":true}}}"); fims_message
    // *msg = bufferToFims((const char *)rbuf); if(msg)
    //     printf("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s]\n",
    //     msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags",
    //     msg->method?msg->method:"no method" );
    // else
    // {
    //     printf(" no message from \n>>>%s<<<\n", rbuf);
    // }
    VarMapUtils vm;
    varsmap vmap;
    vecmap vecs;
    vmap.clear();

    // bms *bm = new bms();
    // bm->setVmap(&vmap);
    std::vector<std::string> sysVec;
    sysVec.clear();
    asset_manager* bmsm = new asset_manager("bms");
    asset_manager* pcsm = new asset_manager("pcs");
    vm.sysVec = &sysVec;
    bmsm->setVmap(&vmap);
    pcsm->setVmap(&vmap);
    bmsm->vm = &vm;
    pcsm->vm = &vm;
    bmsm->vecs = &vecs;
    pcsm->vecs = &vecs;
    vm.setFunc(vmap, "bms", "TestFunc1", (void*)&TestFunc1);
    // this is acop function and runs (with no config) whenver any var in
    // /alarms/bms is written to
    vm.setFunc(vmap, "comp", "/alarms/bms", (void*)&process_alarm);
    // This will have func's in it TestFunc1 and TestFunc2

    bmsm->configure(&vmap, "bms_test.json", "bms");
    asset* bm = bmsm->getInstance("bms_1");
    bm->setVmap(&vmap);

    printf("asset test OK\n");
    // TODO read the top level file
    // std::vector<std::pair<std::string, std::string>> reps =
    // {
    //     { "@@DUMMY@@", "bss_1" }, { "@@IP_ADDR@@", "192.168.1.2" }, {
    //     "@@PORT@@", "502" }
    // };
    // bm->configure("configs/test_dummy.json",&reps);

    // res = bm->showAssetCj();
    // printf("asset cj\n %s\n", res);
    // free((void *)res);

    // printf("asset com Map\n");

    // bm->showCompMap();

    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"/components/sungrow_ess_1/"
             "close_dc_contactors\",\"body\":{\"value\":true}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    // fims_message *msg = bufferToFims((const char *)rbuf);
    fims_message* msg = nullptr;
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        printf("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n", msg->nfrags,
               msg->nfrags > 0 ? msg->pfrags[1] : "no pfrags", msg->method ? msg->method : "no method",
               msg->body ? msg->body : "no body");
    }
    else
    {
        printf(" no message from \n>>>%s<<<\n", rbuf);
    }
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    // bm->processFimsMessage(msg);
    vm.free_fims_message(msg);
    free((void*)rbuf);

    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"/components/"
             "sungrow_ess_1\",\"body\":{\"close_dc_contactors\":true,\"stop_"
             "start\":345}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    free((void*)rbuf);
    rbuf = vm.fimsToBuffer(msg->method, msg->uri, msg->replyto, msg->body);
    printf(" recovered message  \n>>>%s<<<\n", rbuf);
    // bm->free_message(msg);
    free((void*)rbuf);
    vm.free_fims_message(msg);
    assetVar* av11 = vmap["/setup/bms"]["test_f1"];
    av11->am = bmsm;
    assetVar* av21 = vmap["/setup/bms"]["test_f2"];
    av21->am = bmsm;

    // this will run the TestFunc1   function for test_f1
    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"/setup/"
             "bms\",\"body\":{\"test_f1\":1234,\"test_f2\":456}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    free((void*)rbuf);
    rbuf = vm.fimsToBuffer(msg->method, msg->uri, msg->replyto, msg->body);
    printf(" recovered message  \n>>>%s<<<\n", rbuf);
    // bm->free_message(msg);
    free((void*)rbuf);
    vm.free_fims_message(msg);

    // this will run the process_alarms   function for test_f1
    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"/alarms/"
             "bms\",\"body\":{\"fault_f1\":1234,\"fault_f2\":456}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    free((void*)rbuf);
    rbuf = vm.fimsToBuffer(msg->method, msg->uri, msg->replyto, msg->body);
    printf(" recovered message  \n>>>%s<<<\n", rbuf);
    // bm->free_message(msg);
    free((void*)rbuf);
    vm.free_fims_message(msg);

    assetVar* av12 = vmap["/alarms/bms"]["fault_f1"];
    av12->am = bmsm;
    assetVar* av22 = vmap["/alarms/bms"]["fault_f2"];
    av22->am = bmsm;

    // this will run the process_alarms   function for test_f1
    asprintf(
        &rbuf,
        "{\"method\":\"set\",\"uri\":\"/alarms/"
        "bms\",\"body\":{\"fault_f1\":12345,\"fault_f2\":4567}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    free((void*)rbuf);
    rbuf = vm.fimsToBuffer(msg->method, msg->uri, msg->replyto, msg->body);
    printf(" recovered message  \n>>>%s<<<\n", rbuf);
    // bm->free_message(msg);
    free((void*)rbuf);
    vm.free_fims_message(msg);

    // this will set up spme more assetvars    function for test_f1
    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"/test/"
             "bms\",\"body\":{\"test_f1\":12345,\"test_f2\":4567}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    free((void*)rbuf);
    rbuf = vm.fimsToBuffer(msg->method, msg->uri, msg->replyto, msg->body);
    printf(" recovered message  \n>>>%s<<<\n", rbuf);
    // bm->free_message(msg);
    free((void*)rbuf);
    vm.free_fims_message(msg);

    assetVar* av13 = vmap["/test/bms"]["test_f1"];
    av13->am = bmsm;
    assetVar* av23 = vmap["/test/bms"]["test_f2"];
    av23->am = bmsm;
    bmsm->vm = &vm;

    varmap amap;
    amap["RunAmBmsStatus"] = av13;

    vm.setAvFunc(vmap, amap, /*av13->am->amap,*/ "bms", nullptr, av13, "RunAmBmsStatus", CheckAmBmsStatus);

    // now run the CheckAmBmsStatus var for av13
    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"/test/"
             "bms\",\"body\":{\"test_f1\":12345,\"test_f2\":4567}}");  //{\"close_dc_contactors\":{\"value\":true}}}");
    msg = vm.bufferToFims((const char*)rbuf);
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        vm.processFims(*bm->vmap, msg, &cj);
        cJSON_Delete(cj);
    }

    free((void*)rbuf);
    rbuf = vm.fimsToBuffer(msg->method, msg->uri, msg->replyto, msg->body);
    printf(" recovered message  \n>>>%s<<<\n", rbuf);
    // bm->free_message(msg);
    free((void*)rbuf);
    vm.free_fims_message(msg);

    // delete bm;
    delete bmsm;
    delete pcsm;

    if (0)
    {
        // vmap.clear();

        printf("\n\n\n test assetUri \n");
        assetUri auri("/system/bms/sbmu_1:somevar@someParam");
        printf(" test assetUri uri [%s] var[%s] param [%s]\n", auri.Uri, auri.Var, auri.Param);
        auri.setupUriVec();
        printf(" test assetUri num %d uri[0] [%s] \n", auri.nfrags, auri.uriVec[0]);

        printf(" test assetFeatDict \n");

        assetFeatDict assFeat;  // = new assetFeatDict();
        double dval = 12.34;
        assFeat.addFeat((char*)"double", dval);
        int ival = 3400;
        // assFeat.addFeat((char*)"int", ival);
        bool bval = true;
        assFeat.addFeat((char*)"bool", bval);
        char* cval = (char*)"This is my Song";
        assFeat.addFeat((char*)"string", cval);
        assFeat.showFeat();
        cval = (char*)"I changed this";
        assFeat.setFeat((char*)"string", cval);
        printf(" change assetFeatDict \n");

        assFeat.showFeat();

        assetVar av((char*)"testIntVar", ival);
        // add an int param...
        av.setParam((char*)"intParam", 1234);
        bval = false;
        assetVar bav((char*)"testBoolVar", bval);
        // add an int param...
        bav.setParam((char*)"boolParam", 1234);

        cJSON* cj = cJSON_CreateObject();
        bav.showvarCJ(cj);
        printf(" basic assetVar \n");
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                printf(">>%s<<\n", res);
            }
            free((void*)res);
        }

        bav.showvarCJ(cj, 0x0001);
        printf(" add naked assetVar 0x0001\n");
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                printf(">>%s<<\n", res);
            }
            free((void*)res);
        }

        bav.showvarCJ(cj, 0x0010);
        printf(" add full assetVar 0x0010\n");
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                printf(">>%s<<\n", res);
            }
            free((void*)res);
        }

        bav.showvarCJ(cj, 0x0100);
        printf(" add Ui assetVar 0x0100\n");
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                printf(">>%s<<\n", res);
            }
            free((void*)res);
        }

        // test some lowlevel action stuff
        // assetVar* setActVecfromCj(assetVar* av, const char* act, const char* opt,
        // cJSON* cjbf, cJSON* cj)  assetVar* setActVecfromCj(assetVar* av, "onSet",
        // "enum", cjbf, cJSON* cj)
        // add an action.
        assetAction aa("MyName");

        cJSON* cjbf = cJSON_Parse("{ \"enable\":\"/we/can/do/this:var\",\"anIntVar\":678}");

        // TODO add all this stuff into a Feat Dict
        if (cjbf)
            aa.addBitField(cjbf);
        aa.showBitField(1);

        cJSON_Delete(cjbf);

        // move on to the real thing
        cjbf = cJSON_Parse(
            "[{ "
            "\"enable\":\"/status/"
            "ess:UiEnabled\",\"inValue\":true,\"uri\":\"/controls/"
            "ess:maint_active_power_setpoint@enabled\",\"outValue\":"
            "true}]");

        vm.setActVecfromCj(&bav, "onSet", "remap", cjbf, nullptr);

        bav.showvarCJ(cj, 0x0010);
        printf(" add action assetVar 0x0010\n");
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                printf(">>%s<<\n", res);
            }
            free((void*)res);
        }
        cJSON* cj3 = cJSON_CreateObject();
        bav.showvarCJ(cj3, 0x0010);

        // cj is quite nice now but we cannot use it because
        // the second load does not put in actions    << this is an error
        // Also how do we remove / replace lines... reinstall the bit field.on write
        // the bit field spaces by 5 to allow insert

        // >>{
        //         "testIntVar":   {
        //                 "value":        3400
        //         },
        //         "testIntVar":   3400,
        //         "testIntVar":   {
        //                 "value":        3400,
        //                 "intParam":     1234,
        //                 "intParam":     1234   << this is an error
        //         },
        //         "testIntVar":   {
        //                 "value":        3400,
        //                 "intParam":     1234
        //         },
        //         "testIntVar":   {
        //                 "value":        3400,
        //                 "intParam":     1234,
        //                 "actions":      {
        //                         "onSet":        [{
        //                                         "remap":        [{
        //                                                         "enable":
        //                                                         "/status/ess:UiEnabled",
        //                                                         "inValue": true,
        //                                                         "outValue": true,
        //                                                         "uri":
        //                                                         "/controls/ess:maint_active_power_setpoint@enabled"
        //                                                 }]
        //                                 }]
        //                 },
        //                 "intParam":     1234
        //         }
        // }<<

        // we can stuff it into a varsmap
        varsmap vmap2;

        char* body = cJSON_Print(cj3);
        FPS_ERROR_PRINT(" %s >> cj->string [%s] child [%p] body \n[%s]\n", __func__, cj3->string, (void*)cj3->child,
                        body);

        char* buf = vm.fimsToBuffer("set", "/config/ess/test", nullptr, body);
        free((void*)body);
        // fims_message*
        msg = vm.bufferToFims(buf);
        free((void*)buf);
        cJSON* cjb = nullptr;
        // vm.processFims(vmap, msg, &cjb, am, ai);
        vm.processFims(vmap2, msg, &cjb, nullptr, nullptr);
        vm.free_fims_message(msg);

        buf = cJSON_Print(cjb);
        if (cjb)
            cJSON_Delete(cjb);

        FPS_ERROR_PRINT("%s >>  configured [%s]\n", __func__, buf);
        free((void*)buf);

        // Lets see what we got

        // vm.getMapsCj
        cJSON* cj2 = vm.getMapsCj(vmap2, nullptr, nullptr, 0x0010);
        buf = cJSON_Print(cj2);
        cJSON_Delete(cj2);

        FPS_ERROR_PRINT("%s >>  overall result \n>>%s<<\n", __func__, buf);
        free((void*)buf);

        // now set a value "true" to  testIntVar  but that was in the wrong vmap...
        assetVar* av2 = vmap2["/config/ess/test"]["testBoolVar"];

        FPS_ERROR_PRINT("%s >> did we find av2 ?? %p\n", __func__, av2);

        FPS_ERROR_PRINT("%s >> setval direct to 4567 \n", __func__);
        av2->setVal(4567);  // this bypasses any setval actions  Is this a Bug ??
        ival = 5678;
        FPS_ERROR_PRINT("%s >> setval via vm to 5678 \n", __func__);
        vm.setVal(vmap2, "/config/ess/test", "testIntVar",
                  ival);  // Well this seemed to work.

        FPS_ERROR_PRINT("%s >> setval via vm to 5678 done \n", __func__);

        bval = true;
        FPS_ERROR_PRINT("%s >> setval via vm to [true] \n", __func__);
        vm.setVal(vmap2, "/config/ess/test", "testBoolVar",
                  bval);  // Well this seemed to work.
        FPS_ERROR_PRINT("%s >> setval via vm to [true] done \n", __func__);

        cj2 = vm.getMapsCj(vmap2, nullptr, nullptr, 0x0010);
        buf = cJSON_Print(cj2);
        FPS_ERROR_PRINT("%s >>  overall result pass2 \n>>%s<<\n", __func__, buf);
        free((void*)buf);

        // this is the clean up stuff

        cJSON_Delete(cjbf);

        cJSON_Delete(cj);
        cJSON_Delete(cj2);
        cJSON_Delete(cj3);

        // delete assFeat;

        vm.clearVmap(vmap2);
    }
    auto vsize = vecs.size();
    FPS_ERROR_PRINT("%s >>>>vecs->size %d \n", __func__, (int)vsize);

    if (vsize > 0)
    {
        for (auto aa : vecs)
        {
            std::vector<std::string>* vp = aa.second;
            FPS_ERROR_PRINT("%s >>>>vecs [%s] vp %p size %d \n", __func__, aa.first.c_str(), vp, (int)vp->size());
            for (auto ap : *vp)
            {
                FPS_ERROR_PRINT("%s >>>>vec [%s]  \n", __func__, ap.c_str());
            }
            vp->clear();
            delete vp;
        }
        vecs.clear();
    }
    vm.clearVmap(vmap);
    // delete bmsm;
    // delete pcsm;

    return 0;
}

#if defined _MAIN
int main(int argc, char* argv[])
{
    return test_main(argc, argv);
}
#endif