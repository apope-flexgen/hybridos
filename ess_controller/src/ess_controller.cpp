
#include "asset.h"
#include <fims/libfims.h>
#include <csignal>

#include "channel.h"
#include "varMapUtils.h"
#include "scheduler.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"

#include "formatters.hpp"

#include "gitinc.h"

asset_manager* ess_man = nullptr;
int run_secs = 0;
//volatile 
int running = 1;

void signal_handler(int sig)
{
    running = 0;
    FPS_PRINT_INFO("signal of type {} caught.", sig);
    if (ess_man)
    {
        ess_man->running = 0;
        if(ess_man->wakeChan)ess_man->wakeChan->put(-1);
    }
    signal(sig, SIG_DFL);
}

// I want to run this function (fname) 
//    starting in + xx seconds every rep seconds using assetvar av 
//
// operations:
//  create a schedule var /sched/<aname>/<idname>
// sched->setupSchedVar(*am->vmap, am, "/sched/ess", "essSystemInit", "EssSystemInit", essName, 0.200, 0.200 , 0.000);

//  add the function and targ asset to the function code for the schedule point
//cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, essName, "EssSystemInit", "/status/ess:status",   am, cja);
//sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "essSystemInit", "onSet", "func", cja); cJSON_Delete(cja);
// now use it to activate schedule vars
//    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "essSystemInit");
// create a schedItem at the desinated run time
// add the schedItem to the reqList  (activate)
//
// triggerSchedItem(*am->vmap, am, "/sched/ess:every100mS" , "Every100mS", essName);

// all we really need for this is the Id , target asset var, and run/rep times
// int triggerSchedItem(varsmap& vmap 
//                     , asset_manager *am    // gives us the aname am->name.c_str();
//                     , const char* id     // id of the schedevent. Can be reused
//                     , const char *avname  // name of the av to use with the function.
//                     , const char* fname  // name of the function to use.
//                     , const char* fvar  // name of the var the function uses.
//                     , double refTime     // if set with repTime , the run time will be adjusted.. 
//                     , double runTime   // time to activate after tNow (-ve means absTime) 
//                     , double repTime   // how frequently to run this.
//                     )
// {
//     //char*rbuf;
//     char* aname = (char*)am->name.c_str(); 
//     char* tmp;
//     asprintf(&tmp, "/sched/%s", aname);
//     double tNow = am->vm->get_time_dbl();
//     if (runTime < 0)
//     {
//         runTime = -runTime;
//     }
//     else
//     {
//         runTime += tNow;
//     }

//     am->vm->schedaV = am->vm->sched->setupSchedVar(*am->vmap, am, tmp, id, id, avname, refTime, runTime, repTime);
//     // add the function to the schedvar /sched/<aname>/id
//     //void scheduler::addSaction(varsmap& vmap, const char* amap, const char* func, const char*var
//     //    , const char* fvar, asset_manager* am, cJSON* cjacts)

//     cJSON*cja = cJSON_CreateArray(); am->vm->sched->addSaction(*am->vmap, aname, fname, avname, fvar,  am, cja);
//     am->vm->sched->setupSchedItemActions(*am->vmap, am, tmp, id, "onSet", "func", cja); cJSON_Delete(cja);
//     am->vm->sched->activateSchedItem(*am->vmap, "/schedule/ess", tmp, id);
//     free(tmp);
//     return 0;
// }

// this is the full ess controller under new management
// functions   
extern "C++"
{
    int  AddSchedItem(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int  TestTriggerFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int  HandleSchedItem(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    //int AddSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int EssSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP3(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every1000mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SlowPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);


    int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

    int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    //CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
    int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandleSchedLoad(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int SetupGpioSched(scheduler*sched, asset_manager* am);
    int RunSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunMonitor(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int StopSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunVLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunAllVLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunAllLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunAllLinks2(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int runAllLocks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    
    int RunAllALists(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunSysVec(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int MakeLink(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunTpl(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunPub(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int DumpConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadServer(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadClient(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadFuncs(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    void SendPub(varsmap &vmap,asset_manager* am, const char* uri, const char* puri, assetVar* aV);
    int checkAv(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    VarMapUtils* getVm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunSystemCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int process_sys_alarm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int CalculateVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunCell(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int SetupRwFlexSched(scheduler*sched, asset_manager* am);
    int FlexInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int CheckTableVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int MathMovAvg(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    // int SimHandlePcs(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    // int SimHandleBms(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int FastPub(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int SendDb(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int LogInfo(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
//    int SimHandleRizenBms(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    //int SimHandleRackVolts(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    //int SimHandleCellCurrent(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    //int SimProcessCells(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int SimRunCell(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int RunMonitorList(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
//    int SimBmsManageContactor(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    //int SimBmsOpenContactor(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
//    int SimBmsStartConnection(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);

    int SetMapping(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);

    //int LinkVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int  SchedItemOpts(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int  TestTriggerFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int  HandleSchedItem(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    //int AddSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int EssSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP3(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every1000mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SlowPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int runAllLocks(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

    int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    //CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
    int PCSInit(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av);
    int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int CheckMonitorVar_v2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandleSchedLoad(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int process_sys_alarm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int CalculateVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SimHandleBms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SimHandlePcs(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    bool loadSiteMap(varsmap& vmap, VarMapUtils* vm, const char* cfgname);
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int runConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av);
    int SendTrue(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int UpdateToDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SendTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SendDb(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int BalancePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

    // int runDataMaps(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
}



void fimsThread(scheduler *sc, fims* p_fims)
{
    double tNow =  sc->vm->get_time_dbl();
    int tick = 0;
    // bool throk = sc->p_fims->Connected();//true;//p_fims->Subscribe(subs, sublen);
    // bool basok = p_fims->Connected();
    // FPS_PRINT_INFO("%s >>  connected  to FIMS via sc [%s] base [%s]\n"
    //     , __func__
    //     , throk?"true":"false"
    //     , basok?"true":"false"
    //     );
    fims_message* msg = nullptr;// p_fims->Receive_Timeout(100);
    // // we throw away the first message.
    
    // if(msg)
    // {
    //   if (p_fims)p_fims->free_message(msg);  
    // }
    // throk = sc->p_fims->Connected();//true;//p_fims->Subscribe(subs, sublen);
    // basok = p_fims->Connected();
    // FPS_PRINT_INFO("%s >>  msg %p connected  to FIMS via sc [%s] base [%s]\n"
    //     , __func__
    //     , msg
    //     , throk?"true":"false"
    //     , basok?"true":"false"
    //     );
    while (*sc->run)
    {
        bool fimsok = p_fims->Connected();
        // if( tNow > 35.0)
        // {
        //     FPS_ERROR_PRINT("%s >> TEST >> FIMS closed  \n"
        //         , __func__
        //      );
        //     p_fims->Close();
        //     fimsok = p_fims->Connected();
        // }
        if(!fimsok)
        {
            FPS_ERROR_PRINT("%s >> FIMS DISCONNECTED fimsok [%s]\n"
                , __func__
                , fimsok?"true":"false"
                );
            *sc->run = 0;
            
             //exit(0);
        }
        //fims_message* 
        if(fimsok)
        {
            msg = p_fims->Receive_Timeout(1000000);
            // just for testing
            tNow =  sc->vm->get_time_dbl();
            if (0) FPS_PRINT_INFO("Fims Tick {} msg {} p_fims {}"
                    , tick
                    , fmt::ptr(msg)
                    , fmt::ptr(p_fims)
                    );
            tick++;

            // if(tick > 30) 
            // {
            //     *sc->run = 0;
            //     sc->wakeChan.put(-1);   // but this did not get serviced immediatey
            // }
        }
        if (msg)
        {
            if (strcmp(msg->method, "get") == 0)
            {
                if (0) FPS_PRINT_INFO("GET uri [{}]"
                    , msg->uri);
            }
            if(*sc->run) 
            {
                sc->fimsChan.put(msg);
                sc->wakeChan.put(0);   // but this does not get serviced immediatey
            }
            else
            {
                if (p_fims)p_fims->free_message(msg);
                 //p_fims->delete
            }
        }
    }
    assetVar aV;
    tNow = sc->vm->get_time_dbl();
    char* essName = sc->vm->getSysName(*sc->am->vmap); 
    aV.sendEvent(essName, p_fims,  Severity::Info, "%s shutting down at %2.3f", cstr{essName}, tNow);
    FPS_PRINT_INFO("fims shutting down");
    //if(p_fims)delete p_fims;
    sc->p_fims = nullptr;
}



void setupControls(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am, fims* p_fims)
{
    char * fimsname= (char*)"/sched/fims:dummy";
    assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);
    double dval = 0.0;

    if(!fimsav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_PRINT_INFO("Created Fims var [{}:{}]"
            , av->comp
            , av->name
        );
        fimsav = av;
    }
    fimsname= (char*)"/control/ess:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!runav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_PRINT_INFO("Created Runvar var [{}:{}]"
            , av->comp
            , av->name
        );
        runav = av;
    }
    fimsname= (char*)"/control/ess:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!stopav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_PRINT_INFO("Created Stop var [{}:{}]"
            , av->comp
            , av->name
        );
        stopav = av;
    }
    SetupGit(*vmap, sc->vm 
                 , GITBRANCH
                 , GITCOMMIT
                 , GITTAG
                 , GITVERSION
                 );

}

/**
 * @brief Set functions for all assets/asset managers to use
 * Note: only process_sys_alarm is used here. A temporary solution.
 * Need a better solution for this
 * 
 * @param am the ess controller
 */
void initFuncs(asset_manager* am)
{
    char* essName = am->vm->getSysName(*am->vmap); 
    if (1) FPS_PRINT_INFO("About to set process_sys_alarm for {}", am->name);
    am->vm->setFunc(*am->vmap, am->name.c_str(), "process_sys_alarm", (void*)&process_sys_alarm);
    am->vm->setFunc(*am->vmap, am->name.c_str(), "CalculateVar", (void*)&CalculateVar);
    am->vm->setFunc(*am->vmap, am->name.c_str(), "CheckMonitorVar", (void*)&CheckMonitorVar);
    am->vm->setFunc(*am->vmap, am->name.c_str(), "CheckMonitorVar_v2", (void*)&CheckMonitorVar_v2);

    am->vm->setFunc(*am->vmap, "bms", "SimHandleBms", (void*)&SimHandleBms);
    am->vm->setFunc(*am->vmap, essName, "SimHandleBms", (void*)&SimHandleBms);
    am->vm->setFunc(*am->vmap, "pcs", "SimHandlePcs", (void*)&SimHandlePcs);
    am->vm->setFunc(*am->vmap, essName, "SimHandlePcs", (void*)&SimHandlePcs);

 
    for (auto& ix : am->assetManMap)
    {
        asset_manager* amc = ix.second;
        if (1) FPS_PRINT_INFO("Got asset manager {}. Setting func now...", amc->name);
        // Set func for asset manager
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "process_sys_alarm",  (void*)&process_sys_alarm);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "CalculateVar",       (void*)&CalculateVar);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "CheckMonitorVar",    (void*)&CheckMonitorVar);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "CheckMonitorVar_v2", (void*)&CheckMonitorVar_v2);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "RunConfig",          (void*)&runConfig);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "HandleCmd",          (void*)&HandleCmd);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "UpdateToDbi",        (void*)&UpdateToDbi);
        am->vm->setFunc(*am->vmap, amc->name.c_str(), "BalancePower",        (void*)&BalancePower);

        // Set func for asset instances
        for (auto& iy : amc->assetMap)
        {
            asset* ami = iy.second;
            if (1) FPS_PRINT_INFO("Got asset instance {}. Setting func now...", ami->name);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "process_sys_alarm", (void*)&process_sys_alarm);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "CalculateVar", (void*)&CalculateVar);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "CheckMonitorVar", (void*)&CheckMonitorVar);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "CheckMonitorVar_v2", (void*)&CheckMonitorVar_v2);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "HandleCmd", (void*)&HandleCmd);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "UpdateToDbi", (void*)&UpdateToDbi);
            am->vm->setFunc(*am->vmap, ami->name.c_str(), "BalancePower", (void*)&BalancePower);
        }
    }
}

//int scheduler::runChans(varsmap &vmap, schlist &rreqs, asset_manager* am)
void schedThread(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am, fims* p_fims)
{
    //scheduler *sc = (scheduler *) data;

    //int running = 1;
    double delay = 1.0; // Sec
    int wakeup = 0;
    schedItem *si = nullptr;
    double tNow = 0.0;
    fims_message *msg = nullptr;
    //double tStart = sc->vm->get_time_dbl();
    char* cmsg;
    bool triggered = false;
    bool stopped = false;
    bool stopSeen = false;

    setupControls(sc, vmap, rreqs, am, p_fims);

    char * fimsname= (char*)"/sched/fims:dummy";
    assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);

    fimsname= (char*)"/control/ess:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    double runTime = runav->getdVal();
    fimsname= (char*)"/control/ess:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    runTime = runav->getdVal();
    if(runTime < 15) {
        runTime = 15.0;
        runav->setVal(runTime);
    }
    double stopTime = stopav->getdVal();
    
    while (*sc->run)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the timeout specified
        //bflag = 
        sc->wakeChan.timedGet(wakeup, delay);
        essPerf * essLog = new essPerf(am, (char*)am->name.c_str(), "SchedPerf", nullptr);
        tNow = sc->vm->get_time_dbl();
        if (0) FPS_PRINT_INFO("Sched Tick");
        if (0) sc->showReqs(*rreqs);
        stopTime = stopav->getdVal();
        runTime = runav->getdVal();
        if(stopTime>0 && ! stopSeen)
        {
            stopSeen = true;
            FPS_PRINT_INFO("Sched Tick stopTime set {:2.3f}", stopTime);
        }
        if( (runTime>0) && (tNow > runTime) && !triggered) 
        {
            //triggered = true;
            runav->setVal(0.0);

            // sc->vm->setFunc(*sc->am->vmap, essName, "TestTriggerFunc",    (void*)&TestTriggerFunc);
            // sc->vm->sched = sc;
            // triggerSchedItem(*sc->am->vmap
            // , sc->am    // gives us the aname am->name.c_str();
            // , "testTriggerEvent" //const char* id     // id of the schedevent. Can be reused
            // , "/sched/ess:testTrigger"  // name of the av to use with the function.
            // , "TestTriggerFunc"  // name of the function to use.
            // , "/sched/ess:testTriggerEvent"  // name of the av to use with the function.
            // , 0.0    // if set with repTime , the run time will be adjusted.. 
            // , 1.3   // time to activate after tNow (-ve means absTime) 
            // , 0.2   // how frequently to run this.
            // );
        }
        if(( stopTime > 0 ) && (tNow > stopTime) && !stopped) 
        {
            stopped = true;
            *sc->run = 0;
            //     sc->wakeChan.put(-1);   // but this did not get serviced immediatey
        }

        if(sc->msgChan.get(cmsg, false)) 
        {
            FPS_PRINT_INFO("message -> data [{}]", cstr{cmsg});
            //use the message to set the next delay
            if(strcmp(cmsg,"quit")== 0)
            {
                FPS_PRINT_INFO("wakeup value to stop");
                *sc->run = 0;
            }
            free((void *) cmsg);
        }
    
        // handle an incoming avar run request .. avoids too many locks
        if(sc->reqChan.get(si, false)) 
        {
            if(si) 
            {
                // look for id allready allocated
                if (0) FPS_PRINT_INFO("Servicing Sched Request {} id [{}] ({})  uri [{}] repTime {:2.3f}"
                        , fmt::ptr(si)
                        , fmt::ptr(si->id)
                        , fmt::ptr(si->id)
                        , si->uri
                        , si->repTime
                    );
                if (!si->id || (strcmp(si->id,"None")==0))
                {
                    FPS_PRINT_INFO("this is not a schedItem");
                    delete si;
                    si = nullptr;
                }

                if(si)
                {
                    schedItem* oldsi = sc->find(*rreqs, si);
                    if(oldsi)
                    {
                        FPS_PRINT_INFO("this is a replacement schedItem {} id {} uri {}", fmt::ptr(oldsi), cstr{oldsi->id}, cstr{oldsi->uri});
                        oldsi->repTime=0;
                        oldsi->endTime=0.1;
                        
                        FPS_PRINT_INFO("schedItem deleted, seting repTime to 0.0");
                    }
                    //else
                    {
                        if (0) FPS_PRINT_INFO("schedItem added {} id {} uri {}", fmt::ptr(si), cstr{si->id}, cstr{si->uri});
                        sc->addSchedReq(*rreqs, si);
                    }
                }
            }
            else
            {
                FPS_PRINT_INFO("got nullptr si request !!");
            }
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (sc->fimsChan.get(msg, false))
        {
            if(msg)
            {
                essPerf ePerf(am, (char*)am->name.c_str(), msg->uri, nullptr);
                sc->vm->schedaV = fimsav;
                if(0)sc->fimsDebug1(*vmap, msg, am);

                //double tNow = vm->get_time_dbl();
                //sc->vm->syscVec = &syscVec;
                //bool runFims = true;

                //sc->vm->runFimsMsg(*vmap, msg, sc->p_fims);
                sc->vm->runFimsMsgAm(*vmap, msg, am, p_fims);
                if(0)sc->fimsDebug2(*vmap, msg, am);
                //sc->p_fims->free_message(msg);
            }
        }

        //int xdelay = sc->getSchedDelay(*vmap, *rreqs);
        double ddelay = sc->getSchedDelay(*vmap, *rreqs);
        delete essLog;

        if(0)FPS_PRINT_INFO("new delay = {:2.6f}", ddelay);
        delay = ddelay;
        //if ((sc->vm->get_time_dbl() - tStart)  > 15.0) running = 0;
    }

    tNow = sc->vm->get_time_dbl();
    FPS_PRINT_INFO("shutting down");
    return;
}

int loadAssetManagers(varsmap &vmap, asset_manager* ess_man, std::vector<char*>* syscpVec, fims* p_fims, 
                char * essName)
{
    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* ass_man = nullptr;
    
    auto ixa = vmap.find("/config/ess/managers");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        FPS_PRINT_INFO("ESS >> We found our assets, we should be able to set up our system");
        for (auto iy : ixa->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                FPS_PRINT_INFO("lets run assets for  [{}] from [{}]"
                    , iy.first
                    , cstr{iy.second->aVal->valuestring}
                );
                // this utility gets the correct file name
                // looks out for the config directory in the args...and its smart about it 
                //const char* fname = vm->getFname(iy.second->aVal->valuestring);
                const char* fname = iy.second->aVal->valuestring;
                // Let this one through
                if (iy.first == "ess" || iy.first == "bms" || iy.first == "pcs" || iy.first == "site")
                {
                    const char* aname = iy.first.c_str();
                    FPS_PRINT_INFO("setting up manager [{}]", iy.first);

                    ass_man = new asset_manager(aname);
                    
                    ass_man->p_fims = p_fims;
                    ass_man->wakeChan = ess_man->wakeChan;
                    ass_man->reqChan = (void*)ess_man->reqChan;
                    
                    assetVar*Av = iy.second; 
                    Av->am = ass_man;

                    ass_man->setFrom(ess_man);

                    //ccnt = 0;
                    if (1) FPS_PRINT_INFO("running with vmap [{}]", fmt::ptr(&vmap));
                    if (1) FPS_PRINT_INFO("syscVec size [{}]", syscpVec->size());
                    // now get the asset_manager to configure itsself
                    if(ass_man->configure(&vmap, fname, aname, syscpVec, nullptr, ass_man) < 0)
                    {
                        FPS_PRINT_ERROR("error in [{}] config file [{}]", aname, fname);
                        exit(0);
                    }

                    if (iy.first == "pcs") PCSInit(vmap, Av->am->amap, aname, p_fims, Av);

                    //int ccntam = 0;
                    //FPS_PRINT_INFO("%s >> setting up a %s manager pubs found %d \n", __func__, aname, ccntam);

                    // add it to the assetList
                    ess_man->addManAsset(ass_man, aname);
                    FPS_PRINT_INFO("done setting up a {} manager varsmap must be fun now", aname);
                    // we should be able to do things like get status from the bms_manager.
                    // first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset(aname);
                    if (bm2)
                    {
                        FPS_PRINT_INFO("@@@@@@@ found {} asset manager with {} assets", aname, bm2->getNumAssets());
                    }
                    ass_man->running = 1;
                }
            }
        }
    }
    return 0;
}

extern "C++" {
    
    int SetupEssSched(scheduler*sched, asset_manager* am);

}

#define LOGDIR "/var/log/ess_controller"

//Here is a commection of nasty globals to be removed.

VarMapUtils vm;

varsmap vmap;
// deprecated
std::vector<char *> syscVec;

//typedef std::vector<schedItem*>schlist;
schlist schreqs;

//typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
varsmap funMap;

std::vector<char*>* syscpVec;

//typedef std::map<std::string, std::vector<std::string>*>vecmap;
vecmap vecs;

cJSON*getSchListcJ(schlist&schreqs);

cJSON*getSchList()
{
    return getSchListcJ(schreqs);
}

const char **vecToList(std::vector<std::string>&subVec, int nkeys)
{
    const char ** vlist=nullptr;
    int i = 0;
    vlist = (const char**)malloc(sizeof(const char*) * (nkeys+1));
    if(nkeys>0)
    {
        for (i = 0; i<nkeys;i++)
        {
            vlist[i] = subVec[i].c_str();
        }
    }
    vlist[i] = nullptr;
    return vlist;
}

// "controller_name":essName,
// "sched_name":"essSchd",
// "cfgfname":"configs/ess_controller",
// "logdir":"var/logs/ess_controller",
// "cfgdir":"var/logs/ess_controller",
//"managerSetupFcn":"SetUpEssManager"
//"schedSetupFcn":"SetUpEssSched"
//"initVecs":"ess/summary, ess/ess_1"


int loadSiteManager(VarMapUtils*vm, varsmap& vmap, const char* sname, const char*aname)
{
    int rc = 0;
    std::string essName(vm->getSysName(vmap));
    auto ixs = vmap.find(sname);
    if (ixs != vmap.end())
    {
        for (auto iy : ixs->second)
        {
            if (iy.second->name == essName)
            {
                //char* cfgname = vm->getFname(iy.second->getcVal());
                char* cfgname = iy.second->getcVal();
                loadSiteMap(vmap, vm, cfgname);

                // //char* cfgname = ; //aVal->valuestring;
                // cJSON* cj = vm->get_cjson(cfgname);

                // FPS_PRINT_INFO("site_ess lets run site config for [{}] from [{}] file [{}] cj {}"
                //     , iy.first
                //     , iy.second->name
                //     , cfgname
                //     , fmt::ptr(cj)
                //     );
                // // this is for the site interface
                // if(cj)vm->loadSiteMap(vmap, cj);
                // free(cfgname); // LEAK
                // if(cj)cJSON_Delete(cj);
            }
        }     
    }
    else
    {
        if (1)FPS_PRINT_INFO("ess_man >> no site interface found");
        rc = -1;
    }
    return rc;
}

void showVecList(const char** vlist)
{
    int i=0;
    while (vlist[i])
    {
        FPS_PRINT_INFO("[{}] :: [{}]", i, vlist[i]);
        i++;
    }
    return;
}
// the vector components are static I think.
void clearVecList(const char** vlist)
{
    free(vlist);
    return;
}

void print_version();
void print_help(VarMapUtils *vm,  const char* fdir, const char* fname, const char* cfg, const char*cfile, const char*subs)
{    
    //bool oldFims = vm->checkFileName("/usr/local/bin/fims/fims_server");
    //bool newFims = vm->checkFileName("/usr/local/bin/fims_server");
                      
    auto tmp = fmt::format("usage: ess_controller <options> \n"
                              " name:    [{}]\n"
                              " config:  [{}]\n"
                              " cfile:   [{}]\n"
                              " subs:    [{}]\n"
                              " options:\n"
                              " -?   print help\n"
                              " -h   print help\n"
                              " -v   print version info\n"
                              " -x   disabled cfg dbi auto load\n"
                              " -s   sets the subs list <:/ess:/components:/site:>\n"
                              " -c   initial dbi config file <ess_config_risen_sungrow>\n"
                              " -n   sets the default name <ess>\n"
                              " -d   sets the config dir <deprecated>\n"
                              " -f   initial config file (before fims setup) \n"
                               , fname?fname:"Undefined"
                               , cfg?cfg:"Undefined"
                               , cfile?cfile:"Undefined" 
                               , subs?subs:"Undefined"
                               );
    printf("%s\n", tmp.c_str());

}
const char* FimsDir = "/usr/local/bin/";
//const char* FimsDir = "/usr/local/bin/fims/";

int requestConfig(varsmap& vmap, VarMapUtils* vm, const char* aname, const char* fname);
void loadFlexFunc(varsmap& vmap, VarMapUtils* vm , const char* FlexName);
const char* FlexName = "ess";
char* essName = (char *)"ess";
const char* FlexDir = "configs/ess_controller";

// the first sub needs to be the FlexName
const char* FlexSubs = ":/components:/assets:/system:/site:/reload:/misc2:";
const char* FlexConfig = "ess_init"; //nullptr; //"flex_controller.json";
const char* FlexCfile = nullptr;  //"ess_config"; //nullptr; //"flex_controller.json";
bool useOpts  = false;
bool useArgs = false;

void load_config_file(const char* cfile)
{
    // this is where it is supposed to be
    std::string fname = "/usr/local/etc/config/ess_controller/";
    fname += cfile;
    fname +=".json";
    bool ok =  vm.checkFileName(fname.c_str());
    // but it may be here
    if (!ok)
    {
        fname = FlexDir;
        fname += "/";
        fname += cfile;
        fname +=".json";
        ok =  vm.checkFileName(fname.c_str());
    }
    if (!ok)
    {
        // not found so revert to the -c option
        FlexConfig = cfile;
        useArgs = true;

    }
    else
    {
        /*
        {
            "/sysconfig": {
                "Subs":":/components:/assets:/system:/site:/reload:/misc2:",
                "Config":"ess_init",
                "Name": "ess"
            }
        }
        */
        assetVar* av = nullptr;
      //bool newFims = vm.checkFileName("/usr/local/bin/fims_server");
        vm.configure_vmap(vmap, fname.c_str());
        av = vm.getVar(vmap,"/sysconfig/default","Subs");    if(av) { FlexSubs = av->getcVal();}
        av = vm.getVar(vmap,"/sysconfig/default","Config");  if(av) { FlexConfig = av->getcVal();}
        av = vm.getVar(vmap,"/sysconfig/default","EssName");  if(av) { FlexName = av->getcVal();essName = (char*)FlexName;}

        // now pull out all the config items if we find them

    }
  //loadEssConfig
  
}
// TODO V1.0.1 load system.cfg and get the ess name the bms, pcs and site names from it 
// ue this name for all other things //***
int main_test_new_ess(int argc, char *argv[])
{
    // bool oldFims = vm.checkFileName("/usr/local/bin/fims/fims_server");
    // //bool newFims = vm.checkFileName("/usr/local/bin/fims_server");
    // if(oldFims)
    // {
    //     FimsDir = "/usr/local/bin/fims/";
    // }
    int rc = 0;
    syscpVec = new std::vector<char*>;
    vm.argc = argc;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);


    int arg;
    //vm.simdbi = true;

    while((arg = getopt (argc, argv, "?hvxf:n:s:d:c:")) != -1)
    {
        switch(arg)
        {
        case 'x':
            vm.simdbi = true;
            useOpts = true;
            useArgs = true;
            printf(" >>>>>>>>>>>>>>>>>>>>>simdbi setup\n" );
            break;
        case 'c':
            FlexConfig = optarg;
            useOpts = true;
            useArgs = true;
            printf(" setup FlexConfig [%s]\n", FlexConfig );
            break;
        case 'f':
            FlexCfile = optarg;
            useOpts = true;
            useArgs = true;
            
            printf(" setup FlexCfile [%s]\n", FlexCfile);
            if (FlexCfile)
            {
                load_config_file(FlexCfile);
            }
            break;
        case 'n':
            FlexName = optarg;
            useOpts = true;
            break;
        case 's':
            FlexSubs = optarg;
            useOpts = true;
            break;
       case 'd':
            FlexDir = optarg;
            useOpts = true;
            break;
       case 'v':
            print_version();
            useOpts = true;
            vm.noLog = true;
            return 1;
            break;
       case '?':
       case 'h':
            useOpts = true;
        default:
            print_help(&vm, FimsDir, FlexName, FlexConfig, FlexCfile, FlexSubs);
            if(useOpts)
            {
                return 1;
            }
            break;
        }
    }
    //double maxTnow = 15.0;
    // reserve some scheduler slots , it will add more if needed.
    scheduler sched("essSched", &running);  //***
    sched.vm = &vm;
    sched.vm->syscVec = &syscVec;
    schreqs.reserve(64);
    
    vm.vmapp   = &vmap;
    vm.funMapp = &funMap;
    //int rc = 0;
    //double tNow = vm.get_time_dbl();
    fims* p_fims = nullptr;
    
    //read config dir
    if(!useOpts && argc > 1)
    {
        vm.setFname(argv[1]);
    }
    else
    {
        vm.setFname(FlexDir);  //**
    }
    
    // vm.setRunLog(LOGDIR "/run_logs");
    vm.setRunCfg(LOGDIR "/run_configs");

    asset_manager* ess_man = setupEssManager(&vm, vmap, vecs, syscpVec, FlexName, &sched);

    // varmaputils cannot access scheduler.h yet
    ess_man->wakeChan = &sched.wakeChan;
    ess_man->reqChan = (void*)&sched.reqChan;
    sched.am = ess_man;

    vm.setaM(vmap, FlexName, ess_man);   //**
    // FlexPack thing moved here
    vm.setSysName(vmap, FlexName);
    essName = vm.getSysName(vmap);

    

    SetupEssSched(&sched, ess_man);
    vm.setFunc(vmap,  essName, "process_sys_alarm",  (void*)&process_sys_alarm);
    vm.setFunc(vmap,  "bms", "process_sys_alarm",  (void*)&process_sys_alarm);
    vm.setFunc(vmap,  "pcs", "process_sys_alarm",  (void*)&process_sys_alarm);
    vm.setFunc(vmap,  essName, "LogInfo",            (void*)&LogInfo);
    vm.setFunc(vmap,  essName, "RunPub",             (void*)&RunPub);
    vm.setFunc(vmap,  essName, "SendTrue",           (void*)&SendTrue);
    vm.setFunc(vmap,  essName, "SendTime",           (void*)&SendTime);
    vm.setFunc(vmap,  essName, "SendDb",             (void*)&SendDb);
    vm.setFunc(vmap,  essName, "HandleCpuStats",     (void*)&HandleCpuStats);
    vm.setFunc(vmap,  essName, "runAllLocks",        (void*)&runAllLocks);

    // vm.setFunc(vmap,  essName, "runDataMaps",      (void*)&runDataMaps);    // dataMap function addition

    if(!useArgs)
    {
        // this is the old config setup
        rc = loadEssConfig(&vm, vmap, "ess_controller.json", ess_man, &sched);  //**

        //vm.showList(mysubs,essName, mysublen);
        //return 0;

        //syscvec orders the comps for pub to UI interface
        // TODO v1.0.1    get these from ess_controller.json config
        if (syscpVec->size() == 0)
        {
            syscpVec->push_back((char*)"ess/summary");
            syscpVec->push_back((char*)"ess/ess_1");

            FPS_PRINT_INFO("add ess/summary and ess_1 to syscVec size now {}", syscpVec->size());
        }

        // NOTE use syscpVec from now onwards
        if (0) FPS_PRINT_INFO("ess_man >> syscVec size [{}]", syscpVec->size());

        // load up the other managers
        // next is the site controller
        // This loads in the system_controller interface 

        rc = loadSiteManager(&vm, vmap, "/config/ess_server",essName);  //**
        //if(rc<0)
        {
            if (1) FPS_PRINT_INFO("ess_man >> Setting up site manager rc {}", rc); 
            //return 0;
        }


        // setup fims with Subs from config
        int ccntam = 0;
        FPS_PRINT_INFO("setting up [{}] manager Subs vecs {} rc {}", essName, fmt::ptr(ess_man->vecs), rc);
        vm.getVList(*ess_man->vecs, vmap, ess_man->amap, essName, "Subs", ccntam);
        FPS_PRINT_INFO("setting up [{}] manager Subs found {} rc {}", essName, ccntam, rc);
    //vm.showvecMap(*ess_man->vecs, "Subs");
        int mysublen;
        char** mysubs = vm.getVecListbyName(*ess_man->vecs, "Subs", mysublen);
        FPS_PRINT_INFO("recovered [{}] Subs {} found {}", essName, fmt::ptr(mysubs), mysublen);
        if(mysublen> 0)
        {
            if (1) FPS_PRINT_INFO("ess_man >> Subs found in ess config"); 

            p_fims = sched.fimsSetup ((const char**)mysubs, mysublen, "EssSched", vm.argc);  // TODO v1.0.1  set up fims AFter we get all the subs
            vm.clearList(mysubs, FlexName, mysublen);
            if (1) FPS_PRINT_INFO("ess_man >> p_fims {} sched {}"
                , fmt::ptr(p_fims)
                , fmt::ptr(sched.p_fims)
            ); 
        }
        else
        {
            if (1) FPS_PRINT_INFO("ess_man >> No Subs found in ess config"); 
            return 0;
        }

        //p_fims = sched.p_fims;
        ess_man->p_fims = p_fims;
        if (1) FPS_PRINT_INFO("ess_man >> p_fims {}"
            , fmt::ptr(p_fims)
            ); 
        // TODO V1.0.1 distribute the p_fims thing all over the place 
        //return 0;
        // next we load the asset managers
        //int rc = 

        vm.p_fims = p_fims;

        loadAssetManagers(vmap, ess_man, syscpVec, p_fims, essName);
        // show subs vecs and dump config to an initial file.
        debugSystemLoad(&vm, vmap, vecs, syscpVec, essName, LOGDIR);
    
        FPS_PRINT_INFO("ESS >> Setting vlinks");
        vm.setVLinks(vmap, essName);   //**
        FPS_PRINT_INFO("ESS >> Done Setting vlinks");
    
        // run sched startup script here
        // TODO this can all be done from the Init
        rc = loadEssConfig(&vm, vmap, "ess_schedule.json", ess_man, &sched);
        initFuncs(ess_man);
        double tNow = vm.get_time_dbl();
        assetVar aV;
        aV.sendEvent("ESS_CONTROLLER", p_fims,  Severity::Info, "Ess starting  at %2.3f", tNow);
    }
    else
    {
        // this is the new FlexEss setup.
          int debug  = 0;
          double tNow = vm.get_time_dbl();
        loadFlexFunc(vmap, &vm, FlexName);

        vm.setActions(vmap, FlexName);

        std::vector<std::string>subVec;
        int nsubkeys = 0;
        auto nSubs = fmt::format(":/{}{}", FlexName, FlexSubs);
        nsubkeys = vm.uriSplit(subVec, nSubs.c_str() , ":");
        const char **fsubs = vecToList(subVec, nsubkeys);
        showVecList(fsubs);

        int mysublen;
        char** mysubs = vm.getVecListbyName(*ess_man->vecs, "Subs", mysublen);
        FPS_PRINT_INFO("recovered  [{}]  Subs [{}] len  [{}]", FlexName, fmt::ptr(mysubs), mysublen);
        //fims* p_fims = nullptr;

        if((mysublen> 0) || (nsubkeys > 0))
        {
            if (1)FPS_PRINT_INFO(" flex_man >> No Subs found in flex config");
            if(nsubkeys == 0)
            {
                p_fims = sched.fimsSetup ((const char**)mysubs, mysublen, "FlexSched", vm.argc);
            }
            else
            {
                p_fims = sched.fimsSetup ((const char**)fsubs, nsubkeys, "FlexSched", vm.argc);
            }

            vm.clearList(mysubs, FlexName, mysublen);
            if (1)FPS_PRINT_INFO(" flex_man >> p_fims [{}]  sched [{}]"
                , fmt::ptr(p_fims)
                , fmt::ptr(sched.p_fims)
                );
        }
        else
        {
            if (1)FPS_PRINT_INFO(" flex_man >> No Subs found in flex config");
            return 0;
        }
        if(fsubs) free(fsubs);

        ess_man->p_fims = p_fims;
        vm.p_fims = p_fims;
        if (debug)FPS_PRINT_INFO(" [{}] >> p_fims [{}]\n", FlexName, fmt::ptr(p_fims));
        debugSystemLoad(&vm, vmap, vecs, syscpVec, FlexName, LOGDIR);
        assetVar *aV = vm.getVar(vmap,"/config/system",FlexName);
        if(!aV)
        {
            double tNow = vm.get_time_dbl();
            aV = vm.setVal(vmap,"/config/system",FlexName, tNow);
        }
        if(aV)
        {
            if (1)FPS_ERROR_PRINT(" %s >> aV [%s]\n", __func__,aV->getfName());

            aV->sendEvent(FlexName, p_fims,  Severity::Info, "%s starting  at %2.3f", FlexName, tNow);
            SetupGit(vmap, &vm
                 , GITBRANCH
                 , GITCOMMIT
                 , GITTAG
                 , GITVERSION
                 );

            //GpioInit(vmap, flex_man->amap, "flex", p_fims, aV);
        }
        else
        {
            if (1)FPS_ERROR_PRINT(" %s >> %s aV error %p \n", __func__, FlexName, aV);
            return 0;

        }
        // this kicks off the config loader
        if(FlexConfig)
        {
            char* aname = vm.getSysName(vmap);
            if(!aname)
                vm.setSysName(vmap, FlexName);

            // TODO allow both 
            int single = 0;
            requestConfig(vmap, &vm, FlexName, FlexConfig);
            vm.handleCfile(vmap, nullptr, "set", "/cfg/crun", single, nullptr, nullptr, nullptr, nullptr);

            // auto cms = fmt::format(
            //     "{}fims_send -m set  -u /{}/full/cfg/crun/{}/start_config 1&"
            //     , FimsDir, FlexName, FlexName);

            // auto rc = system(cms.c_str());
            // FPS_PRINT_INFO(" Sending fims startup [{}] rc [{}]",cms, rc);
        }

    }

    // NOTE(WALKER): this reads the logging variables from /config/ess_x then if we have them inside of 
    // /config/ess it overrides them with that one (global override):

    char* specific_essName = vm.getSysName(vmap);
    const auto config_name = fmt::format("/config/{}", specific_essName);

    // This setups the default logging dir:
    assetVar* logging_dir_av  = vm.getVar(vmap, config_name.c_str(), "LogDir");
    // assetVar* logging_dir_global_av  = vm.getVar(vmap, "/config/ess:LogDir", nullptr);
    if (!logging_dir_av) // set "default" to be "/var/log/ess_controller":
    {
        char* log_dir = (char*)"var/log/ess_controller";
        logging_dir_av = vm.makeVar(vmap, config_name.c_str(), "LogDir", log_dir);
    }

    // This setups the default logging enabled:
    assetVar* logging_enabled_av  = vm.getVar(vmap, config_name.c_str(), "logging_enabled");
    // assetVar* logging_enabled_global_av  = vm.getVar(vmap, "/config/ess:logging_enabled", nullptr);
    if (!logging_enabled_av) // set "default" to be false:
    {
        bool log_enabled = false;
        logging_enabled_av = vm.makeVar(vmap, config_name.c_str(), "logging_enabled", log_enabled);
    }

    // This setups the default logging timestamp:
    assetVar* logging_timestamp_av  = vm.getVar(vmap, config_name.c_str(), "logging_timestamp");
    // assetVar* logging_timestamp_global_av  = vm.getVar(vmap, "/config/ess:logging_timestamp", nullptr);
    if (!logging_timestamp_av) // set "default" to be false:
    {
        bool log_timestamp = false;
        logging_timestamp_av = vm.makeVar(vmap, config_name.c_str(), "logging_timestamp", log_timestamp);
    }

    // This setups the default logging amount:
    assetVar* logging_size_av = vm.getVar(vmap, config_name.c_str(), "logging_size");
    // assetVar* logging_size_global_av = vm.getVar(vmap, "/config/ess:logging_size", nullptr);
    if (logging_size_av) // they have it in the config file:
    {
        setLoggingSize(vmap, vm);
    }
    else // setup default value and "actions":
    {
        int log_size = 64;
        logging_size_av = vm.makeVar(vmap, config_name.c_str(), "logging_size", log_size);
    }

// TODO after MVP shut down threads individually 
// TODO after MVP collect all the vList subs Then do the fims subscribe THe distribute fims to all ams and ais. 
    // Use this to trigger a pub from the gpio
    // {FimsDir}fims_send -m set -r /$$ -u /gpio/control/gpio/getPubs true

    std::thread fThread(fimsThread, &sched, p_fims);

    std::thread sThread(schedThread, &sched, &vmap, &schreqs, ess_man, p_fims);

    if(fThread.joinable())
        fThread.join();
    if(sThread.joinable())
        sThread.join();
    FPS_PRINT_INFO("threads done cleaning up ...");

    vm.clearVmap(vmap);
    vmap.clear();
    
    //delete ess_man->p_fims;
    ess_man->p_fims = nullptr;
    //vecm[name] = (new std::vector<T>);
    for ( auto xx:vecs)
    {
        xx.second->clear();
        delete xx.second;
    }

    vecs.clear();
    delete ess_man;
    int idx = 0;
    FPS_PRINT_INFO("things found in syscpVec ... ");
    for ( auto x: *syscpVec)
    {
        FPS_PRINT_INFO("syscpVec [{}] [{}]", idx++, cstr{x});
    }

    FPS_PRINT_INFO("deleting remaining sched items ...");
    for ( auto sr : schreqs)
    {
        //FPS_PRINT_INFO("%s >>  we got a schedItem %p not deleting it for now .... \n", __func__, sr);
        sr->show();
        delete sr;        
    }

    schreqs.clear();
// TODO v1.0.1 also clear up the remaining csv tables  
    vm.amMap.clear();
    vm.aiMap.clear();
    if(p_fims)delete p_fims;
    return 0;
}

// Please do NOT remove these, these are global extern variables for the ess_controller
namespace flex
{
    const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

int main(int argc, char *argv[])
{
    // setting up stdout console sink:
    auto std_out = spdlog::stdout_color_st("stdout");
    std_out->set_level(spdlog::level::debug);

    // setting up stderr console sink:
    auto std_err = spdlog::stderr_color_st("stderr");
    std_err->set_level(spdlog::level::err);

    // setting the default logger to log to both:
    spdlog::set_default_logger(std::move(std_out));

    // setting up the elapsed time formatter for the global logger (similar to the way we have it for ESS Controller):
    // NOTE: please refer to this for help -> https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<elapsed_time_formatter>('*').set_pattern("[%-10*] [%^%-8l%$] [%-15!!] %v");
    spdlog::set_formatter(std::move(formatter));

    return  main_test_new_ess(argc, argv);
}
