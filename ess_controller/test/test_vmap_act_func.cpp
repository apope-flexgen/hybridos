/*
 * vmap action Function test
 * this one uses the featdict and has scale, offset features
 * NOTE :: We dont yet send out the OK or error messages.
 * 
 * 11/04 added access to the func dict
 * runFuncs timerFuncs
 * still under thought / developemnt
 *  We have an list of assetVars in a list each with a time field.
 *  the assetVar gives the function context.

  * vm.SetTimer(.... assetVar, Function, time, int repeat)
  * adds the assetvar with a function to a timerlist sorted by lowest first
  * Timer thread picks up the next to run.(1mS resolution).
  * then runs the function.
  *  
 * 
 */

#include "asset.h"
#include "varMapUtils.h"
#include "alarm.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"


int testRes(const char* tname, varsmap &vmap, const char* method, const char* uri,const char* body, const char* res, asset_manager*am=nullptr, asset*ai=nullptr)
{
    int rc = 1;
    char *tmp;
    // this is our map utils factory
    VarMapUtils vm;
    cJSON* cjr = nullptr;//cJSON_CreateObject();
    const char*replyto = "/mee";
    char* buf = vm.fimsToBuffer(method, uri, replyto , body);
            //free((void *)body);
    fims_message* msg = vm.bufferToFims(buf);
    free((void *)buf);
    vm.processFims(vmap, msg,  &cjr, am, ai);
    vm.free_fims_message(msg);
    tmp = cJSON_PrintUnformatted(cjr);
    if(tmp && strcmp(res,tmp)==0)
    {
        //printf(" PASSED \n");
        printf("%s\tPASSED reply >>%s<<\n", tname, tmp);
    }
    else
    {  
        unsigned int ires = strlen(res);
        unsigned int itmp = strlen(tmp);
        unsigned int iuse = itmp;

        if (iuse > ires) 
        {
            iuse =  ires;
        }

        for (unsigned int i = 1 ; i < iuse; i++)
        {
            if(strncmp(res,tmp,i)!=0)
            {
                printf(" test failed at loc %d \ntmp[%s] \nres[%s] \n"
                        , i
                        , &tmp[i-1]
                        , &res [i-1]
                  );
            break;


            }
        }

        printf("%s\tFAILED reply >>%s<<\n", tname, tmp);
        rc =0;
    }
    free((void*)tmp);
    cJSON_Delete(cjr);
    return rc;
}
// We may need the func instance for this to work
// maybe set it in av 
int CheckEssStart(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    int reload;
    double dval = 3000.0;
    double dval2 = 10000.0;
    //double dvalHB = 1.0;
    int ival = 0;
    VarMapUtils* vmp = av->am->vm;
    bool bval = false;
    assetAction* avAct = nullptr;
    FPS_ERROR_PRINT(" %s >> RUNNING >>>>>>>>>>>>>>>>>>\n",__func__);

    if(av->extras)
    {
        avAct = av->extras->actVec["onSet"][0];
    }
    if(!avAct)
    {
        FPS_ERROR_PRINT(" %s >> Error did not find onSet asset action\n",__func__);
    }
    else
    {
        // these are all the args defined in the config statement
        FPS_ERROR_PRINT(" %s >> Ok found onSet asset action we are number %d \n",__func__, av->abNum);
        avAct->showBitField();
        assetBitField* abf = avAct->getBitField(av->abNum);
        if(1)FPS_ERROR_PRINT(" %s >> Ok found assetBitField for number %d as %p\n",__func__, av->abNum, abf);

        //assFeat options 
        //     AFTypes type;
        // double valuedouble;
        // int valueint;
        // char* valuestring;
        // bool valuebool;

        // Now pull features out of the abf.
        assFeat* af = avAct->getFeat(av->abNum , "func");
        if(af && af->valuestring)
        {
            if(1)FPS_ERROR_PRINT(" %s >> Ok found assetFunc feature \"func\" as [%s] \n",__func__, af->valuestring);
        }
        af = avAct->getFeat(av->abNum , "onErr");
        if(af && af->valuestring)
        {
            FPS_ERROR_PRINT(" %s >> Ok found assetFunc feature \"onErr\" as [%s] \n",__func__, af->valuestring);
        }
        // now pull params out of the av ...
        // av->getParam("name")

        if(av->extras && av->extras->featDict) 
        {
            auto afp =  av->extras->featDict;
             for ( auto x : afp->featMap)
             {
                 if(1)FPS_ERROR_PRINT("%s >> param name [%s]\n", __func__, x.first.c_str());
             }
        }
        else
        {
             if(1)FPS_ERROR_PRINT("%s >> no params found for  [%s]\n", __func__, av->name.c_str());
        }
        
    }
    

    //double dvalHBnow = vmp->get_time_dbl();
    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* CheckEssStart      = amap[__func__];  

    if(reload < 2)
    {
        //reload = 0;
        amap["EssStartCheckOK"]          = vmp->setLinkVal(vmap, aname, "/status",    "EssStartCheckOK",   bval);
        amap["EssStartErr"]              = vmp->setLinkVal(vmap, aname, "/status",    "EssStartErr",       bval);
        amap["EssStartLimit"]            = vmp->setLinkVal(vmap, aname, "/config",    "EssStartLimit",     dval);
        amap["EssStartMaxLimit"]         = vmp->setLinkVal(vmap, aname, "/config",    "EssStartMaxLimit",  dval2);
        amap["EssStartCmd"]              = vmp->setLinkVal(vmap, aname, "/controls",    "start_stop",      dval);
        //amap["]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            ival = 0; amap["EssStartCheckOK"]->setVal(bval);
        }
    }
    ival = 2; CheckEssStart->setVal(ival);


    // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
    double StartCmd = amap["EssStartCmd"]->getdVal();
    double StartLimit = amap["EssStartLimit"]->getdVal();
    double StartMaxLimit = amap["EssStartMaxLimit"]->getdVal();
    
    dval = 1.0;
    // dont use valueChanged it resets the change currently
    if(StartCmd > StartLimit && StartCmd < StartMaxLimit)
    {
        bval = true;
        amap["EssStartOK"]->setVal(bval);
        bval = false;
        amap["EssStartErr"]->setVal(bval);


        // this stuff collects a bunch of assetVars and send them out to their default locations.
        // the link will determine where that location is.
        // if the link is defined in the config file then that destination will be maintained.

        // varsmap *vlist = vmp->createVlist();
        // vmp->addVlist(vlist, amap["HeartBeat"]);
        // vmp->addVlist(vlist, amap["op_todSec"]);
        // vmp->addVlist(vlist, amap["op_todMin"]);
        // vmp->addVlist(vlist, amap["op_todHr"]);
        // vmp->addVlist(vlist, amap["op_todDay"]);
        // vmp->addVlist(vlist, amap["op_todYr"]);
        // vmp->sendVlist(p_fims, "set", vlist);
        // vmp->clearVlist(vlist);
        
    }
    if(StartCmd > StartMaxLimit)
    {
        bval = false;
        amap["EssStartCheckOK"]->setVal(bval);
        bval = true;
        amap["EssStartErr"]->setVal(bval);
    }

    return 0;
}
// We may need the func instance for this to work
// maybe set it in av 
int TestEssStart(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    int reload;
    double dval = 3000.0;
    double dval2 = 10000.0;
    //double dvalHB = 1.0;
    int ival = 0;
    VarMapUtils* vmp = av->am->vm;
    bool bval = false;

    assetAction* avAct = nullptr;//av->actVec["onSet"][0];
    if(av->extras)
    {
        avAct = av->extras->actVec["onSet"][0];
    }
    if(!avAct)
    {
        FPS_ERROR_PRINT(" %s >> Error did not find onSet asset action\n",__func__);
    }
    else
    {
        // these are all the args defined in the config statement
        FPS_ERROR_PRINT(" %s >> Ok found onSet asset action we are number %d\n",__func__, av->abNum);
        avAct->showBitField();
    }
    
    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* TestEssStart      = amap[__func__];  

    
    if(reload < 2)
    {
        //reload = 0;
        amap["TestEssStart"]             = vmp->setLinkVal(vmap, aname, "/reload",    "TestEssStart",      reload);
        amap["EssStartOK"]               = vmp->setLinkVal(vmap, aname, "/status",    "EssStartOK",        bval);
        amap["EssStartErr"]              = vmp->setLinkVal(vmap, aname, "/status",    "EssStartErr",        bval);
        amap["EssStartLimit"]            = vmp->setLinkVal(vmap, aname, "/config",    "EssStartLimit",     dval);
        amap["EssStartMaxLimit"]         = vmp->setLinkVal(vmap, aname, "/config",    "EssStartMaxLimit",  dval2);
        amap["EssStartCmd"]              = vmp->setLinkVal(vmap, aname, "/controls",    "start_stop",      dval);
        //amap["]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            ival = 0; amap["EssStartOK"]->setVal(bval);
        }
    }
    ival = 2; TestEssStart->setVal(ival);


    // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
    double StartCmd = amap["EssStartCmd"]->getdVal();
    double StartLimit = amap["EssStartLimit"]->getdVal();
    double StartMaxLimit = amap["EssStartMaxLimit"]->getdVal();
    
    dval = 1.0;
    // dont use valueChanged it resets the change currently
    if(StartCmd > StartLimit && StartCmd < StartMaxLimit)
    {
        bval = true;
        amap["EssStartOK"]->setVal(bval);
        bval = false;
        amap["EssStartErr"]->setVal(bval);


        // this stuff collects a bunch of assetVars and send them out to their default locations.
        // the link will determine where that location is.
        // if the link is defined in the config file then that destination will be maintained.

        // varsmap *vlist = vmp->createVlist();
        // vmp->addVlist(vlist, amap["HeartBeat"]);
        // vmp->addVlist(vlist, amap["op_todSec"]);
        // vmp->addVlist(vlist, amap["op_todMin"]);
        // vmp->addVlist(vlist, amap["op_todHr"]);
        // vmp->addVlist(vlist, amap["op_todDay"]);
        // vmp->addVlist(vlist, amap["op_todYr"]);
        // vmp->sendVlist(p_fims, "set", vlist);
        // vmp->clearVlist(vlist);
        
    }
    if(StartCmd > StartMaxLimit)
    {
        bval = false;
        amap["EssStartOK"]->setVal(bval);
        bval = true;
        amap["EssStartErr"]->setVal(bval);
    }

    return 0;
}
                
//  vm->setFunc(vmap, "dcr", "run_asset_wakeup" , (void*) &run_bms_asset_wakeup);


//     //void (*tf)(void *) = (void (*tf)(void *))
//     void *res1 = vm->getFunc(vmap, "ess","run_init" );
//     void *res2 = vm->getFunc(vmap, "ess","run_wakeup" );
   
//typedef void (*myAmInit_t)(asset_manager * data);
typedef int (*myAfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
//     myAmInit_t myessMinit = myAmInit_t(res1);
//     myAmWake_t myessMwake = myAmWake_t(res2);
    
int main(int argc, char *argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;

    vm.setFunc(vmap, "ess", "TestEssStart" , (void*) &TestEssStart);
    vm.setFunc(vmap, "ess", "CheckEssStart" , (void*) &CheckEssStart);

    asset_manager *ess_man = new asset_manager("ess");
    ess_man->am = nullptr;
    ess_man->running = 1;

    int rc;
    const char *var1= "{\"start_stop\":{\"value\":0,"
            "\"params\":{"
                "\"param1\":1,\"param2\":\"p2\""
            "},"
            "\"actions\":{"
                 "\"onSet\":[{"
                    "\"func\":["
                        "{ \"enable\":\"/controls/ess:start_enable\", \"amap\":\"bms\",\"testCheckArg\":234,"
                        "\"func\":\"CheckEssStart\",\"onOK\":\"/controls/ess/startOK\",\"onErr\": \"/controls/ess/startErr\"},"
                        "{ \"enable\":\"/controls/ess:start_enable\", \"amap\":\"ess\",\"testTestArg\":234,"
                        "\"func\":\"TestEssStart\",\"onOK\":\"/controls/ess/startOK\",\"onErr\": \"/controls/ess/startErr\"}"
                    "]"
                "}]"
            "}"
        "}"
    "}";

    const char* rep1 = "{\"start_stop\":{\"value\":0,\"params\":{\"param1\":1,\"param2\":\"p2\"},\"actions\":{\"onSet\":[{\"func\":"
                        "[{\"enable\":\"/controls/ess:start_enable\",\"amap\":"
                        "\"bms\",\"testCheckArg\":234,\"func\":\"CheckEssStart\",\"onOK\":\"/controls/ess/startOK\",\"onErr\":\"/controls/ess/startErr\"},"
                        "{\"enable\":\"/controls/ess:start_enable\",\"amap\":\"ess\","
                        "\"testTestArg\":234,\"func\":\"TestEssStart\",\"onOK\":\"/controls/ess/startOK\",\"onErr\":\"/controls/ess/startErr\"}]}]}}}";
 

    const char* rep3 ="{\"/controls/ess\":{\"start_stop\":{\"value\":0}}}";
    // \"actions\":{\"onSet\":{\"func\":"
    //                     "[{\"amap\":\"ess\",\"enable\":\"/controls/ess:start_enable\","
    //                     "\"func\":\"TestEssStart\",\"onErr\":\"/controls/ess/startErr\",\"onOK\":\"/controls/ess/startOK\"}]}}}}}";

    rc = vm.testRes(" Test 1", vmap
            ,"set"
            ,"/controls/ess"
            ,var1
            ,rep1
           , ess_man
           , nullptr
           );

    rc = vm.testRes(" Test 2", vmap
            , "get"
            , "/controls/ess"
            , nullptr
            , rep3
        );

// this is broken we think
    rc = vm.testRes(" Test 3.0", vmap
             ,"set"
             ,"/cotrols/ess:start_stop"
             ,"0"
             ,"{\"start_stop\":0}"
         );
    rc = vm.testRes(" Test 3.1", vmap
             ,"set"
             ,"/controls/ess"
             ,"{\"start_stop\":{\"value\":1000,\"params\":{\"maxVal\":23456,\"minVal\":555}}}"
             ,"{\"start_stop\":{\"value\":1000,\"params\":{\"maxVal\":23456,\"minVal\":555}}}"
         );
    rc = vm.testRes(" Test 3.2", vmap
             ,"set"
             ,"/controls/ess"
             ,"{\"start_stop\":2000}"
             ,"{\"start_stop\":2000}"
         );

    rc = vm.testRes(" Test 4", vmap
             ,"set"
             ,"/controls/ess"
             ,"{\"start_stop\":9000}"
             ,"{\"start_stop\":9000}"
         );

    // rc = testRes(" Test 5", vmap
    //          ,"get"
    //          ,"/system/remap_controls/rm_start_stop"
    //          , nullptr
    //          ,"{\"value\":129800}"
    //      );

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

    cJSON *cj = vm.getMapsCj(vmap,nullptr, nullptr,0x0110);
    char* res = cJSON_Print(cj);
    rc = 0; // -Wall
    if(rc==0)    printf("#########vmap at end \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);

    // monitor M;
    // M.configure("configs/monitor.json");
    
    // delete bms_man;
    vm.clearVmap(vmap);

    return 0;

}
