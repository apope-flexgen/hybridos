/*
 * asset parser
 */
#include "asset.h"
#include "assetVar.h"
//#include "bms.h"
//#include "pcs.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

//  "/assets/bms":        {
//                 "bms_1":   {
//                                    "template":"bms_catl_template.json".
//                                    "subs":[
//                                       {"replace":"@@BMS_ID@@","with":"bms_1"},
//                                       {"replace":"@@BMS_IP@@","with":"192.168.1.114"}
//                                       ]
//                                   }
//            }

int main()
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
    // bms *bm = new bms();
    // bm->setVmap(&vmap);

    asset_manager* bmsm = new asset_manager("bms");
    asset_manager* pcsm = new asset_manager("pcs");

    bmsm->setVmap(&vmap);
    pcsm->setVmap(&vmap);

    bmsm->configure(&vmap, "configs/bms_config.json", "bms");
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

    delete bmsm;
    delete pcsm;

    printf(" test assetUri \n");
    assetUri auri("/system/bms/sbmu_1:somevar@someParam");
    printf(" test assetUri uri [%s] var[%s] param [%s]\n", auri.Uri, auri.Var, auri.Param);
    auri.setupUriVec();
    printf(" test assetUri num %d uri[0] [%s] \n", auri.nfrags, auri.uriVec[0]);

    printf(" test assetFeatDict \n");

    assetFeatDict assFeat;  // = new assetFeatDict();
    double dval = 12.34;
    assFeat.addFeat("double", dval);
    int ival = 3400;
    assFeat.addFeat("int", ival);
    bool bval = true;
    assFeat.addFeat("bool", bval);
    char* cval = (char*)"This is my Song";
    assFeat.addFeat("string", cval);
    assFeat.showFeat();
    cval = (char*)"I changed this";
    assFeat.setFeat("string", cval);
    printf(" change assetFeatDict \n");

    assFeat.showFeat();

    assetVar av("testIntVar", ival);
    // add an int param...
    av.setParam("intParam", 1234);
    bval = false;
    assetVar bav("testBoolVar", bval);
    // add an int param...
    bav.setParam("boolParam", 1234);

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
        "\"enable\":\"/status/ess:UiEnabled\",\"inValue\":true,\"uri\":\"/"
        "controls/ess:maint_active_power_setpoint@enabled\",\"outValue\":true}]");

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
    FPS_ERROR_PRINT(" %s >> cj->string [%s] child [%p] body \n[%s]\n", __func__, cj3->string, (void*)cj3->child, body);

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

    vm.clearVmapxx(vmap);
    vm.clearVmapxx(vmap2);
    // delete bmsm;
    // delete pcsm;

    return 0;
}