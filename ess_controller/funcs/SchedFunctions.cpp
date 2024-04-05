#ifndef SCHED_FUNCTIONS_CPP
#define SCHED_FUNCTIONS_CPP

#include "asset.h"

#include "scheduler.h"
#include "formatters.hpp"
#include "InputHandler.hpp"
#include "ScheduledEnableFunctions.hpp"

/*
 * these are the main functions groups run by the schduler
 * the code also includes the setup code for the default functions
 * systemInit  set startup or global reload
 * every100mSP1
 * every100mSP2
 * every100mSP3
    every 100mS with different priorities
 * every1000mS
 * fastPub
 * slowPub
 *
 *
 */

extern "C++" {

int EssSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every100mSP1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every100mSP2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every100mSP3(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every1000mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SlowPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SlowPubOne(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int DeratePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandlePowerEst(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandlePowerRackEst(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int AggregateManager(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckMonitorVar_v2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckTableVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckDbiVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SendDbiVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckDbiResp(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* dbiAv);
int CheckGPIO(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

void SendPub(varsmap& vmap, asset_manager* am, const char* uri, const char* puri, assetVar* av);
int SetupEssSched(scheduler* sched, asset_manager* am);
int HandleSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int TestTriggerFunc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandleSplitItem(varsmap& vmap, assetVar* av, const char* newId, const char* newFunc, const char* newUri,
                    double refInc, double fastRep, double runAfter, double newEnd);
int HandleSchedLoad(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int GPIOCalcResponse(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CalculateVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SendClearFaultCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SchedRunFunc(varsmap& vmap, const char* sname, const char* func, double runAfter, double refTime, double repTime,
                 assetVar* aV);
int LogInfo(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
int LogDebug(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
int LogWarn(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
int LogError(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
int LogIt(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
// int SetLoggingSize(varsmap &vmap, varmap &amap, const char* aname, fims*
// p_fims, assetVar*Av);
int MathMovAvg(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
int SimHandleHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunMonitorList(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SimHandleSbmu(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SimHandleBms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int RunSched(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int StopSched(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SchedItemOpts(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SlewVal(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunScript(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SetDbiDoc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SendTrue(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int UpdateToDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SaveToDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SendTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int BalancePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

// int  SetupGit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims,
// assetVar* av
//              , const char*gbranch
//              , const char*gcommit
//              , const char*gtag
//              , const char*gversion
//              );

}  // not fmt

// after MVP no need to reister functions for diffent am names unless we need to
// do so. the default "vm->getSysName()" will suffice
int SetupEssSched(scheduler* sched, asset_manager* am)
{
    UNUSED(sched);
    const char* aname = am->name.c_str();

    am->vm->setFunc(*am->vmap, aname, "CheckMonitorVar", (void*)&CheckMonitorVar);
    am->vm->setFunc(*am->vmap, "bms", "CheckMonitorVar", (void*)&CheckMonitorVar);
    am->vm->setFunc(*am->vmap, "pcs", "CheckMonitorVar", (void*)&CheckMonitorVar);
    am->vm->setFunc(*am->vmap, aname, "CheckMonitorVar_v2", (void*)&CheckMonitorVar_v2);
    am->vm->setFunc(*am->vmap, aname, "CheckTableVar", (void*)&CheckTableVar);
    am->vm->setFunc(*am->vmap, "bms", "CheckTableVar", (void*)&CheckTableVar);
    am->vm->setFunc(*am->vmap, "pcs", "CheckTableVar", (void*)&CheckTableVar);
    am->vm->setFunc(*am->vmap, aname, "CheckDbiVar", (void*)&CheckDbiVar);
    am->vm->setFunc(*am->vmap, "bms", "CheckDbiVar", (void*)&CheckDbiVar);
    am->vm->setFunc(*am->vmap, "pcs", "CheckDbiVar", (void*)&CheckDbiVar);
    am->vm->setFunc(*am->vmap, aname, "CheckDbiResp", (void*)&CheckDbiResp);
    am->vm->setFunc(*am->vmap, "bms", "CheckDbiResp", (void*)&CheckDbiResp);
    am->vm->setFunc(*am->vmap, "pcs", "CheckDbiResp", (void*)&CheckDbiResp);
    am->vm->setFunc(*am->vmap, aname, "SendDbiVar", (void*)&SendDbiVar);
    am->vm->setFunc(*am->vmap, "pcs", "StartupPCS", (void*)&StartupPCS);
    am->vm->setFunc(*am->vmap, "bms", "StartupBMS", (void*)&StartupBMS);
    am->vm->setFunc(*am->vmap, "bms", "ShutdownBMS", (void*)&ShutdownBMS);
    am->vm->setFunc(*am->vmap, "pcs", "ShutdownPCS", (void*)&ShutdownPCS);
    am->vm->setFunc(*am->vmap, aname, "EssSystemInit", (void*)&EssSystemInit);
    am->vm->setFunc(*am->vmap, aname, "Every1000mS", (void*)&Every1000mS);
    am->vm->setFunc(*am->vmap, aname, "Every100mSP1", (void*)&Every100mSP1);
    am->vm->setFunc(*am->vmap, aname, "Every100mSP2", (void*)&Every100mSP2);
    am->vm->setFunc(*am->vmap, aname, "Every100mSP3", (void*)&Every100mSP3);
    am->vm->setFunc(*am->vmap, aname, "FastPub", (void*)&FastPub);
    am->vm->setFunc(*am->vmap, aname, "SlowPub", (void*)&SlowPub);
    am->vm->setFunc(*am->vmap, aname, "SlowPubOne", (void*)&SlowPubOne);
    am->vm->setFunc(*am->vmap, aname, "HandleSchedItem", (void*)&HandleSchedItem);
    am->vm->setFunc(*am->vmap, aname, "HandleSchedLoad", (void*)&HandleSchedLoad);
    am->vm->setFunc(*am->vmap, "pcs", "HandleSchedLoad", (void*)&HandleSchedLoad);
    am->vm->setFunc(*am->vmap, "bms", "HandleSchedLoad", (void*)&HandleSchedLoad);
    am->vm->setFunc(*am->vmap, aname, "TestTriggerFunc", (void*)&TestTriggerFunc);
    am->vm->setFunc(*am->vmap, aname, "GPIOCalcResponse", (void*)&GPIOCalcResponse);
    am->vm->setFunc(*am->vmap, aname, "CalculateVar", (void*)&CalculateVar);
    am->vm->setFunc(*am->vmap, aname, "SendClearFaultCmd", (void*)&SendClearFaultCmd);
    am->vm->setFunc(*am->vmap, "pcs", "SendClearFaultCmd", (void*)&SendClearFaultCmd);
    am->vm->setFunc(*am->vmap, "bms", "SendClearFaultCmd", (void*)&SendClearFaultCmd);
    am->vm->setFunc(*am->vmap, aname, "HandlePowerCmd", (void*)&HandlePowerCmd);
    am->vm->setFunc(*am->vmap, aname, "HandlePowerEst", (void*)&HandlePowerEst);
    am->vm->setFunc(*am->vmap, aname, "AggregateManager", (void*)&AggregateManager);
    am->vm->setFunc(*am->vmap, aname, "HandlePowerRackEst", (void*)&HandlePowerRackEst);
    am->vm->setFunc(*am->vmap, aname, "DeratePower", (void*)&DeratePower);
    am->vm->setFunc(*am->vmap, aname, "LogInfo", (void*)&LogInfo);
    am->vm->setFunc(*am->vmap, aname, "LogDebug", (void*)&LogDebug);
    am->vm->setFunc(*am->vmap, aname, "LogWarn", (void*)&LogWarn);
    am->vm->setFunc(*am->vmap, aname, "LogError", (void*)&LogError);
    am->vm->setFunc(*am->vmap, aname, "LogIt", (void*)&LogIt);
    // am->vm->setFunc(*am->vmap, aname, "SetLoggingSize",
    // (void*)&SetLoggingSize);
    am->vm->setFunc(*am->vmap, aname, "MathMovAvg", (void*)&MathMovAvg);
    am->vm->setFunc(*am->vmap, aname, "SimHandleHeartbeat", (void*)&SimHandleHeartbeat);
    am->vm->setFunc(*am->vmap, aname, "SimHandleSbmu", (void*)&SimHandleSbmu);
    am->vm->setFunc(*am->vmap, aname, "SimHandleBms", (void*)&SimHandleBms);
    am->vm->setFunc(*am->vmap, aname, "RunSched", (void*)&RunSched);
    am->vm->setFunc(*am->vmap, aname, "StopSched", (void*)&StopSched);
    am->vm->setFunc(*am->vmap, aname, "HandleCmd", (void*)&HandleCmd);
    am->vm->setFunc(*am->vmap, aname, "SchedItemOpts", (void*)&SchedItemOpts);
    am->vm->setFunc(*am->vmap, aname, "SlewVal", (void*)&SlewVal);
    am->vm->setFunc(*am->vmap, aname, "RunScript", (void*)&RunScript);
    am->vm->setFunc(*am->vmap, aname, "SetDbiDoc", (void*)&SetDbiDoc);
    am->vm->setFunc(*am->vmap, aname, "UpdateToDbi", (void*)&UpdateToDbi);
    am->vm->setFunc(*am->vmap, aname, "SaveToDbi", (void*)&SaveToDbi);
    am->vm->setFunc(*am->vmap, aname, "BalancePower", (void*)&BalancePower);
    am->vm->setFunc(*am->vmap, aname, "LocalStartBMS", (void*)&InputHandler::LocalStartBMS);
    am->vm->setFunc(*am->vmap, aname, "LocalStopBMS", (void*)&InputHandler::LocalStopBMS);
    am->vm->setFunc(*am->vmap, aname, "LocalStartPCS", (void*)&InputHandler::LocalStartPCS);
    am->vm->setFunc(*am->vmap, aname, "LocalStopPCS", (void*)&InputHandler::LocalStopPCS);
    am->vm->setFunc(*am->vmap, aname, "LocalStandbyPCS", (void*)&InputHandler::LocalStandbyPCS);
    am->vm->setFunc(*am->vmap, aname, "SiteRunCmd", (void*)&InputHandler::SiteRunCmd);
    am->vm->setFunc(*am->vmap, aname, "SiteBMSContactorControl", (void*)&InputHandler::SiteBMSContactorControl);
    am->vm->setFunc(*am->vmap, aname, "SitePCSStatusControl", (void*)&InputHandler::SitePCSStatusControl);
    am->vm->setFunc(*am->vmap, aname, "CloseContactorsEnable", (void*)&ScheduledEnableFunctions::CloseContactorsEnable);
    am->vm->setFunc(*am->vmap, aname, "OpenContactorsEnable", (void*)&ScheduledEnableFunctions::OpenContactorsEnable);
    am->vm->setFunc(*am->vmap, aname, "StartEnable", (void*)&ScheduledEnableFunctions::StartEnable);
    am->vm->setFunc(*am->vmap, aname, "StopEnable", (void*)&ScheduledEnableFunctions::StopEnable);
    am->vm->setFunc(*am->vmap, aname, "StandbyEnable", (void*)&ScheduledEnableFunctions::StandbyEnable);

    return 0;
}

int GPIOCalcResponse(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = av->am;
    double tNow = am->vm->get_time_dbl();
    double tRef = am->vm->get_time_ref();
    double respTime = tRef - av->getdVal();
    av->setParam("respTime", respTime);
    av->setParam("respTimeuS", respTime * 1000000.0);
    av->setParam("rxTime", tNow);
    return 0;
}

int TestTriggerFunc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = av->am;
    // double tNow = am->vm->get_time_dbl();
    double runTime = am->vm->schedaV->getdParam("runTime");
    int runCnt = am->vm->schedaV->getiParam("runCnt");
    bool enabled = av->getbParam("enabled");

    double rcount = av->getdParam("rcount");
    rcount++;
    av->setParam("rcount", rcount);
    FPS_PRINT_INFO(
        "enabled [{}] rcount [{}] for av [{}] schedaV [{}] runTime "
        "[{:2.3f}] runCnt [{}]",
        enabled, rcount, cstr{ av->getfName() }, cstr{ am->vm->schedaV->getfName() }, runTime, runCnt);  // spdlog

    // NOTE: For the doubles you could also say {:2.6f} but fmt defaults to

    if (runCnt > 10)
    {
        am->vm->schedaV->setParam("repTime", 0.0);
        am->vm->schedaV->setParam("response", (char*)"Shutting down");

        FPS_PRINT_INFO("shutting down for av [{}] runTime {:2.3f} runCnt {}", cstr{ am->vm->schedaV->getfName() },
                       runTime,
                       runCnt);  // spdlog
    }
    else
    {
        am->vm->schedaV->setParam("response", (char*)"Still working");
    }
    return 0;
}
// #                          Id              Control Var
// function             # run datadata # "addSchedItem":
//{
// "value":"EssSystemInit",
//  "var":"/sched/ess:essSystemInit",
//   "func":"EssSystemInit",
// "refTime":0.200,
// "runTime":0.200,
// "repTime":0.000
// },

// fims_send -m set -r /$$ -u /ess/scheduler/ess '{
// 	"addSchedItem":	{"value":	"None","actions":{"onSet":	[{
// "func":	[{"func":	"HandleSchedItem","amap":	essName}]}]}},
//     "addSchedItem":	{"value":	"EssSystemInit","var":
//     "/sched/ess:essSystemInit","func":"EssSystemInit","refTime":0.200,"runTime":0.200,"repTime":0.000},

// this is the main responder to add_item
// which may be renamed to manage_sched
// We'd like repTime etc params in av to be used

// TODO  (possibly deferred) zero out params after creating the schedItem

// used in handle poll to poll individual tables one at a time
// We take the original av (say poll)
// and create an new scheditem with a faster repeat time fastRep and a small
// offset (refInc) the Uri will need to be different HandleSplitItem(vmap, av,
// "SlowPubOne", "SlowPubOne","/shed/ess/slowPubOne"
//   refTime+0.010, 0.010, 0.0)
int HandleSplitItem(varsmap& vmap, assetVar* av, const char* newId, const char* newFcn, const char* newUri,
                    double refInc, double fastRep, double newrunAfter, double newEnd)
{
    UNUSED(vmap);
    asset_manager* am = av->am;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();  // vm->get_time_dbl();

    // char *act = (char *)"add_sched_item";
    char* schedId = (char*)newId;    // av->getcVal();
    char* schedUri = (char*)newUri;  // av->getcParam("uri");
    char* schedFcn = (char*)newFcn;  // av->getcParam("fcn");
    char* schedTarg = av->getcParam("targ");
    double refTime = av->getdParam("refTime");
    refTime += refInc;
    // we may need runTime
    double runTime = av->getdParam("runTime");
    runTime += refInc;
    double repTime = fastRep;  // av->getdParam("repTime");
    double runAfter = av->getdParam("runAfter");
    double runFor = av->getdParam("runFor");
    double endTime = av->getdParam("endTime");
    int runCnt = 0;  // av->getiParam("runCnt");  runCnt is used to index into the
                     // array of things to process
    assetUri my(newUri, nullptr);
    bool debug = av->getbParam("debug");
    debug = false;
    if (debug)
        FPS_PRINT_INFO(
            "running for av [{}:{}] am {} vm {} schedId [{}] schedUri "
            "[{}] schedFcn [{}] refTime {:2.3f} runTime {:2.3f} repTime "
            "{:2.3f} runAfter {:2.3f}\n",
            cstr{ my.Uri }, cstr{ my.Var }, fmt::ptr(am), fmt::ptr(am->vm), schedId ? schedId : "no Id",
            schedUri ? schedUri : "no Uri", schedFcn ? schedFcn : "no Fcn", refTime, runTime, repTime,
            runAfter);  // spdlog
    if (newrunAfter > 0)
    {
        runAfter = newrunAfter;
    }
    if (!schedUri || strcmp(schedUri, "None") == 0)
    {
        FPS_PRINT_INFO("no SchedUri quitting", NULL);  // spdlog
        return 0;
    }

    if (!schedId || strcmp(schedId, "None") == 0)
    {
        FPS_PRINT_INFO("no SchedId quitting", NULL);  // spdlog
        return 0;
    }

    schedItem* as = nullptr;
    assetVar* avi = nullptr;

    if (am && am->vm)
    {
        avi = am->vm->getVar(*am->vmap, schedUri, nullptr);
    }
    if (!avi)
    {
        avi = am->vm->makeVar(*am->vmap, schedUri, nullptr, tNow);
        if (debug)
            FPS_PRINT_INFO("added new schedUri {} [{}]", fmt::ptr(avi), cstr{ schedUri });  // spdlog
        int ival = 0;
        avi->setParam("runCnt", ival);
        avi->setParam("fcn",
                      schedFcn);  // we'll run the function this way without adding actions.
        if (schedTarg && strcmp(schedTarg, "None") != 0)
            avi->setParam("targ", schedTarg);  // optional sched target
        avi->am = av->am;
    }
    // move any allowed updates to the running var
    if (avi)
    {
        if (refTime > 0)
            avi->setParam("refTime", refTime);
        if (runTime > 0)
            avi->setParam("runTime", runTime);
        if (runCnt >= 0)
            avi->setParam("runCnt", runCnt);
        if (repTime > 0)
            avi->setParam("repTime", repTime);
        if (runAfter > 0)
            avi->setParam("runAfter", runAfter);

        //         if(endTime> 0)avi->setParam("endTime",endTime);
        // TODO  (look in FlexPack.cpp) after MVP we use the repTime and endTime to
        // create single shots or run forever categories but this could be done a
        // bit better
        //         dval = 0.0;
        avi->setParam("endTime", newEnd);

        if (schedFcn && strcmp(schedFcn, (char*)"None") != 0)
            avi->setParam("fcn",
                          schedFcn);  // we'll run the function this way without adding actions.
        if (schedTarg && strcmp(schedTarg, (char*)"None") != 0)
            avi->setParam("targ", schedTarg);  // optional sched target
    }

    avi->setParam("runTime", runTime);
    if (av->gotParam("runFor") && runFor > 0.0)
    {
        endTime = runTime + runFor;
        avi->setParam("runEnd", (runTime + runFor));
        avi->setParam("endTime", endTime);
    }
    else
    {
        double dval = 0.0;
        avi->setParam("runEnd", dval);
        avi->setParam("endTime", dval);
    }

    if (avi && avi->am)
    {
        as = new schedItem();

        if (debug)
            FPS_PRINT_INFO("created new schedItem {}", fmt::ptr(as));  // spdlog
        as->av = avi;
        if (avi->am)
            am = avi->am;

        // char* myid = av->getcVal();
        char* myid = schedId;

        // this dummy function will simply send the current time to avi triggering
        // its onSet actions infact we'll do this anyway without a function
        // void schedItem::setUp(const char * _id, const char* _aname, const char *
        // _uri, const char* _func, double _refTime,
        //    double _runTime, double _repTime, double _endTime, char*targaV)
        // TODO after MVP add amap to the schedItem

        as->setUp(myid, am->name.c_str(), schedUri, schedFcn, refTime, runTime, repTime, endTime, schedTarg);

        if (debug)
            as->show();

        if (debug)
            FPS_PRINT_INFO("@@@@@@ reset old values {} avi {} runTime {:2.3f}", cstr{ schedUri }, fmt::ptr(avi),
                           avi->getdParam("runTime"));  // spdlog

        if (debug)
            FPS_PRINT_INFO("@@@@@@ done with old values {}   avi {} runTime {:2.3f}", cstr{ schedUri }, fmt::ptr(avi),
                           avi->getdParam("runTime"));  // spdlog
        if (am->reqChan)
        {
            channel<schedItem*>* reqChan = (channel<schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                am->wakeChan->put(0);
            }
            if (debug)
                FPS_PRINT_INFO("@@@@@@ activated {}   avi {} runTime {:2.3f}", cstr{ schedUri }, fmt::ptr(avi),
                               avi->getdParam("runTime"));  // spdlog
            avi->setParam("active", true);
            avi->setParam("enabled", true);
        }
        else
        {
            FPS_PRINT_INFO(
                " @@@@@ You need to set up am->reqChan and am->wakeChan "
                "@@@@",
                NULL);  // spdlog
            delete as;
            as = nullptr;
        }
    }
    else
    {
        FPS_PRINT_INFO("@@@@@ did not find var to activate avi {} avi->am {}", fmt::ptr(avi),
                       fmt::ptr(avi ? avi->am : nullptr));  // spdlog
    }
    return 0;
}

// #                          Id              Control Var
// function             # run datadata #
// "addSchedItem":{"value":"EssSystemInit","var":"/sched/ess:essSystemInit",
// "func":"EssSystemInit","refTime":0.200,"runTime":0.200,"repTime":0.000},

// fims_send -m set -r /$$ -u /ess/scheduler/ess '{
// 	"addSchedItem":	{"value":	"None","actions":{"onSet":	[{
// "func":	[{"func":	"HandleSchedItem","amap":	essName}]}]}},
//     "addSchedItem":	{"value":	"EssSystemInit","var":
//     "/sched/ess:essSystemInit","func":"EssSystemInit","refTime":0.200,"runTime":0.200,"repTime":0.000},

// this is the main responder to add_item
// which may be renamed to manage_sched
// We'd like repTime etc params in av to be used

int HandleSchedLoad(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(p_fims);
    asset_manager* am = av->am;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();

    char* schedId = av->getcVal();
    char* schedUri = av->getcParam("uri");
    char* schedFcn = av->getcParam("fcn");
    char* schedTarg = av->getcParam("targ");
    char* schedamap = av->getcParam("amap");
    double refTime = av->getdParam("refTime");
    double runTime = av->getdParam("runTime");
    double repTime = av->getdParam("repTime");
    double runAfter = av->getdParam("runAfter");
    double runFor = av->getdParam("runFor");
    double endTime = av->getdParam("endTime");
    int debug = av->getiParam("debug");
    // debug = true;
    if (debug)
        FPS_PRINT_INFO(
            "***** running for av [{}] am [{}] vm [{}] func aname [{}] "
            "av amap [{}]",
            cstr{ av->getfName() }, fmt::ptr(am), fmt::ptr(am->vm), aname ? aname : " No amap",
            schedamap ? schedamap : "No amap");  // spdlog

    if (debug)
        FPS_PRINT_INFO(
            "***** running for av [{}] am [{}] vm [{}] func aname [{}] av amap "
            "[{}] "
            "schedId [{}] schedUri [{}] schedFcn [{}] "
            "refTime {:2.3f} runTime {:2.3f} repTime {:2.3f} runAfter {:2.3f}",
            cstr{ av->getfName() }, fmt::ptr(am), fmt::ptr(am->vm), aname ? aname : " No amap",
            schedamap ? schedamap : "No amap", schedId ? schedId : "no Id", schedUri ? schedUri : "no Uri",
            schedFcn ? schedFcn : "no Fcn", refTime, runTime, repTime,
            runAfter);  // spdlog
    if (!schedUri || strcmp(schedUri, "None") == 0)
    {
        FPS_PRINT_ERROR("no SchedUri (use uri)quitting", NULL);  // spdlog
        return 0;
    }

    if (!schedId || strcmp(schedId, "None") == 0)
    {
        FPS_PRINT_ERROR("no SchedId (use value) quitting", NULL);  // spdlog
        return 0;
    }

    schedItem* as = nullptr;

    // get next runTime runAfter is relative
    // logic
    //  just runTime set runTime runEnd = 0
    //       runTime    refTime repTime  runAfter  runFor             startTime
    //       endTime x          -       -        -         -
    //       tNow+runTime                               0 x          -       x
    //       -         -                  tNow+runTime 0 x          x       -
    //       -         -                  tNow+runTime 0 x          x       x
    //       -         -                  refTime+(x*repTime)>tNow+runTime 0 x
    //       -       -        x         -                  tNow+runTime+runAfter 0
    //       x          -       x        x         -
    //       tNow+(runTime +(x *repTime) > tNow+runAfter 0 x          x       -
    //       x         -                  tNow+runAfter 0 x          x       x
    //       x         -                  refTime +(x*repTime) > tNow+ runAfter 0
    //       .          .       .        .         x                  .
    //       startTime + runFor

    //       runTime plus runAfter plus runRef plus runRep        runTime = inc
    //       runRef by runRep untill past tNow plus runAfter runTime plus runAfter
    //       no runRef plus runRep          runTime = inc tNow by runRep untill
    //       past tNow plus runAfter
    //
    double startTime = tNow;

    if (av->gotParam("refTime") && av->gotParam("repTime"))
    {
        startTime = refTime;
    }
    // runAfter will default to 0
    // runTime will default to 0
    if (av->gotParam("repTime") && repTime > 0.0)
    {
        while (startTime < tNow)
        {
            startTime += repTime;
        }
    }
    runTime = startTime;
    av->setParam("runTime", runTime);
    if (av->gotParam("runFor") && runFor > 0.0)
    {
        endTime = runTime + runFor;
        av->setParam("runEnd", (runTime + runFor));
        av->setParam("endTime", endTime);
    }
    else
    {
        double dval = 0.0;
        av->setParam("runEnd", dval);
        av->setParam("endTime", dval);
    }

    if (av && av->am)
    {
        as = new schedItem();

        if (debug)
            FPS_PRINT_INFO("created new schedItem {}", fmt::ptr(as));  // spdlog
        as->av = av;
        if (av->am)
        {
            am = av->am;
        }
        char* myid = av->getcVal();

        // this dummy function will simply send the current time to avi triggering
        // its onSet actions in fact we'll do this anyway without a function
        // void schedItem::setUp(const char * _id, const char* _aname, const char *
        // _uri, const char* _func, double _refTime,
        //    double _runTime, double _repTime, double _endTime, char*targaV)

        as->setUp(myid, am->name.c_str(), schedUri, schedFcn, refTime, runTime, repTime, endTime, schedTarg);

        if (debug)
            as->show();

        if (debug)
            FPS_PRINT_INFO("@@@@@@ done with old values {}   av {} runTime {:2.3f}", cstr{ schedUri }, av->getfName(),
                           av->getdParam("runTime"));  // spdlog
        if (am->reqChan)
        {
            channel<schedItem*>* reqChan = (channel<schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                am->wakeChan->put(0);
            }

            if (debug)
                FPS_PRINT_INFO("@@@@@@ activated {} av {} runTime {}", cstr{ schedUri }, fmt::ptr(av),
                               av->getdParam("runTime"));  // spdlog
            av->setParam("active", true);
            av->setParam("enabled", true);
        }
        else
        {
            FPS_PRINT_INFO(
                " @@@@@ You need to set up am->reqChan and am->wakeChan "
                "@@@@",
                NULL);  // spdlog
            delete as;
            as = nullptr;
        }
    }
    else
    {
        FPS_PRINT_INFO("@@@@@ did not find var to activate avi {} avi->am {}", fmt::ptr(av),
                       fmt::ptr(av ? av->am : nullptr));  // spdlog
    }
    return 0;
}

// this should be it....
//                        schedId        func,     initial delay, priority ,
//                        repeat time, assetVar
// SchedRunFunction(vmap, "GpioPubId",    GpioPub,  2.0,           0.01,
// 0.1,        aV);
// this is the prime candidate

int SchedRunFunc(varsmap& vmap, const char* sname, const char* func, double runAfter, double refTime, double repTime,
                 assetVar* aV)
{
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();  // vm->get_time_dbl();
    schedItem* as = nullptr;

    double startTime = refTime;
    // todo make a schedaV "sched"+sname

    // char* ntmp = nullptr;
    auto ntmp = fmt::format("sched{}", sname);
    // asprintf(&ntmp,"sched%s", sname);

    auto stmp = fmt::format("/sched/{}:{}", am->name, ntmp);

    // char* stmp = nullptr;
    // asprintf(&stmp,"/sched/%s:%s"
    //    , am->name.c_str()
    //    , ntmp);
    char* foo = (char*)ntmp.c_str();
    assetVar* avs = vm->setVal(vmap, (char*)stmp.c_str(), nullptr, foo);
    int debug = aV->getiParam("debug");
    debug = 1;
    // runAfter will default to 0
    double runTime = 0.0;
    if (repTime)
    {
        while (startTime < tNow)
        {
            startTime += repTime;
        }
        runTime = startTime;
    }
    else
    {
        runTime = tNow + runAfter;
    }
    double endTime = 0.0;
    avs->setParam("runTime", runTime);
    avs->setParam("repTime", repTime);
    avs->setParam("endTime", endTime);
    avs->setParam("fcn", (char*)func);
    avs->setParam("amap", (char*)"gpio");

    if (avs && am)
    {
        as = new schedItem();
        avs->am = am;

        as->av = avs;
        char* myid = avs->getcVal();
        // as->setUp(myid, am->name.c_str(), schedUri, schedFcn,  refTime, runTime,
        // repTime, endTime, schedTarg);
        as->setUp(myid, am->name.c_str(), stmp.c_str(), func, refTime, runTime, repTime, endTime, aV->getfName());
        if (debug)
        {
            FPS_PRINT_INFO("created new schedItem {} [{}]", fmt::ptr(as), cstr{ aV->getfName() });  // spdlog
            as->show();

            FPS_PRINT_INFO("@@@@@@ created schedVar [{}] runTime {}", stmp,
                           avs->getdParam("runTime"));  // spdlog
        }
        if (am->reqChan)
        {
            channel<schedItem*>* reqChan = (channel<schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                // channel <int>* wakeChan = (channel <int>*)am->wakeChan;
                am->wakeChan->put(0);
            }
            if (debug)
                FPS_PRINT_INFO("@@@@@@ activated {}   av {} runTime {:2.3f}", stmp, fmt::ptr(avs),
                               avs->getdParam("runTime"));  // spdlog
            avs->setParam("active", true);
            avs->setParam("enabled", true);
        }
        else
        {
            FPS_PRINT_INFO(
                "@@@@@ You need to set up am->reqChan and am->wakeChan "
                "@@@@",
                NULL);  // spdlog
            delete as;
            as = nullptr;
        }
    }
    else
    {
        FPS_PRINT_INFO("@@@@@ did not find var to activate asi {} avi->am {}", fmt::ptr(avs),
                       fmt::ptr(aV ? aV->am : nullptr));  // spdlog
    }

    return 0;
}

// take the options list and send a list of uris to a target function like
// HandleSchedItem or send each option to a uri as a body. option to send a
// value to a assetVar and then send that assetvar to a function if one is
// defined

int SchedItemOpts(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(amap);
    UNUSED(aname);
    bool debug = false;
    asset_manager* am = av->am;
    VarMapUtils* vm = nullptr;

    if (am && am->vm)
    {
        vm = am->vm;
    }
    if (!vm)
    {
        FPS_PRINT_INFO(" error no vm in av [{}] cannot proceed ", av->getfName());
        return -1;
    }

    char* essName = vm->getSysName(vmap);
    asset_manager* ams = nullptr;
    asset* ais = nullptr;
    void* res1 = nullptr;
    myAvfun_t amFunc = nullptr;
    const char* vname = nullptr;
    const char* turi = nullptr;
    assetVar* tav = nullptr;
    const char* tfun = nullptr;
    bool targVal = false;
    const char* targVar = nullptr;

    if (av->gotParam("debug"))
    {
        debug = av->getbParam("debug");
    }

    // get the am or ai (function context)
    if (av->gotParam("aname"))
    {
        vname = av->getcParam("aname");
        ams = vm->getaM(vmap, vname);  // ams wins
        if (!ams)
        {
            ais = vm->getaI(vmap, vname);
        }
    }

    if (av->gotParam("targVal"))
    {
        targVal = true;
    }
    if (av->gotParam("targVar"))
    {
        targVar = av->getcParam("targVar");
    }

    if (av->gotParam("targav"))
    {
        turi = av->getcParam("targav");
        if (turi)
            tav = vm->getVar(vmap, turi, nullptr);
    }

    if (av->gotParam("targfunc"))
    {
        tfun = av->getcParam("targfunc");
        if (tfun)
        {
            if (ams)
            {
                res1 = vm->getFunc(vmap, ams->name.c_str(), tfun);
            }
            else if (ais)
            {
                res1 = vm->getFunc(vmap, ais->name.c_str(), tfun);
            }
            // use the default using essName if needed
            if (!res1)
                res1 = vm->getFunc(vmap, essName, tfun);
        }
    }

    if (av->extras && av->extras && av->extras->optVec && av->extras->optVec->cjopts)
    {
        FPS_PRINT_INFO("@@@@@ [{}] found cjopts ready to schedule res1 [{}] tav [{}]", av->getfName(), fmt::ptr(res1),
                       tav ? tav->getfName() : "No Tav");  // spdlog
        cJSON* cji;
        int idx = -1;
        cJSON_ArrayForEach(cji, av->extras->optVec->cjopts)
        {
            idx++;
            // we send the whole shebang (cji) to the targav
            // deal with the func later when I work out how to use it
            // for testing we'll also write data to the uri
            cJSON* cjuri = cJSON_GetObjectItem(cji, "uri");
            cJSON* cjval = cJSON_GetObjectItem(cji, "value");

            // cJSON* cjaname = cJSON_GetObjectItem(cji, "aname");
            assetVar* avi = nullptr;
            std::string tv;
            // get avi if we can
            if (cjuri && cjuri->valuestring)
            {
                if (targVar)
                {
                    tv = fmt::format("{}{}", cjuri->valuestring, targVar);
                }
                else
                {
                    tv = fmt::format("{}", cjuri->valuestring);
                }
                avi = vm->getVar(vmap, tv.c_str(), nullptr);
                if (debug)
                    FPS_PRINT_INFO(" idx [{}] uri ->[{}] avi [{}]", idx, cjuri->valuestring,
                                   avi ? avi->getfName() : " No avi");
            }
            // send value to uri if we have one
            // if we have a value then set it
            if (cjuri && cjuri->valuestring && targVal)
            {
                if (debug)
                    FPS_PRINT_INFO(" idx [{}] targVal >>  value to uri ->[{}] ", idx, cjuri->valuestring);
                avi = vm->setVal(vmap, tv.c_str(), nullptr, av, "targVal");
                if (!avi->am)
                    avi->am = ams;
                if (!avi->am)
                    avi->am = av->am;
            }
            // use inline value
            if (cjuri && cjuri->valuestring && cjval)
            {
                if (debug)
                    FPS_PRINT_INFO(" idx [{}] uri >>>  value to uri ->[{}] ", idx, cjuri->valuestring);
                avi = vm->setValfromCj(vmap, tv.c_str(), nullptr, cjval);
                if (!avi->am)
                    avi->am = ams;
                if (!avi->am)
                    avi->am = av->am;
            }
            if (tav)
            {
                if (debug)
                    FPS_PRINT_INFO(" idx [{}] tav >>> cji  to uri [{}] ", idx, tav->getfName());
                vm->setValfromCj(vmap, tav->getfName(), nullptr, cji);
            }

            // now sent to the targav
            // and to the function

            if (res1 && avi)
            {
                amFunc = reinterpret_cast<myAvfun_t>(res1);
                if (ams)
                {
                    if (debug)
                        FPS_PRINT_INFO(" idx [{}] AMS @@@@ uri ->[{}] func [{}]", idx,
                                       avi ? avi->getfName() : av->getfName(), tfun ? tfun : "no tfun");
                    amFunc(vmap, ams->amap, ams->name.c_str(), p_fims, avi ? avi : av);
                }
                if (ais)
                {
                    if (debug)
                        FPS_PRINT_INFO(" idx [{}] AIS @@@@ uri ->[{}] func [{}]", idx,
                                       avi ? avi->getfName() : av->getfName(), tfun ? tfun : "no tfun");
                    amFunc(vmap, ais->amap, ais->name.c_str(), p_fims, avi ? avi : av);
                }
            }
        }
    }
    return 0;
}

int RunScript(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(av);
    // we could run the script with the value and script params but this is OK
    // auto cmd = fmt::format("sh /home/scripts/RunScript.sh {} {}&", av->comp,
    // av->name); int rc = system(cmd.c_str()); if(rc != 0 )
    // {
    //     FPS_PRINT_ERROR(" Unable to run command {} ", cmd);
    // }

    return 0;
}

int HandleSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = av->am;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();
    char* schedId = av->getcVal();
    char* schedUri = av->getcParam("uri");
    char* schedFcn = av->getcParam("fcn");
    char* schedTarg = av->getcParam("targ");
    double refTime = av->getdParam("refTime");
    double runTime = av->getdParam("runTime");
    double repTime = av->getdParam("repTime");
    double runAfter = av->getdParam("runAfter");
    double runFor = av->getdParam("runFor");
    double endTime = av->getdParam("endTime");
    int runCnt = av->getiParam("runCnt");
    bool debug = false;

    FPS_PRINT_INFO(
        "***** running for  av [{}] am {} vm {} "
        "schedId [{}] schedUri [{}] schedFcn [{}] "
        "refTime {:2.3f} runTime {:2.3f} repTime {:2.3f} runAfter {:2.3f}",
        cstr{ av->getfName() }, fmt::ptr(am), am ? fmt::ptr(am->vm) : 0, schedId ? schedId : "no Id",
        schedUri ? schedUri : "no Uri", schedFcn ? schedFcn : "no Fcn", refTime, runTime, repTime,
        runAfter);  // spdlog
    if (!schedUri || strcmp(schedUri, "None") == 0)
    {
        FPS_PRINT_INFO("no SchedUri quitting", NULL);  // spdlog
        return 0;
    }

    if (!schedId || strcmp(schedId, "None") == 0)
    {
        FPS_PRINT_INFO("no SchedId quitting", NULL);  // spdlog
        return 0;
    }
    if (!schedFcn || strcmp(schedFcn, "None") == 0)
    {
        FPS_PRINT_INFO("no schedFcn (fcn) quitting", NULL);  // spdlog
        return 0;
    }

    schedItem* as = nullptr;
    assetVar* avi = nullptr;

    if (am && am->vm)
    {
        avi = am->vm->getVar(*am->vmap, schedUri, nullptr);
    }
    if (!avi)
    {
        avi = am->vm->makeVar(*am->vmap, schedUri, nullptr, tNow);
        FPS_PRINT_INFO("we added new schedUri {}", fmt::ptr(avi));  // spdlog
        int ival = 0;
        avi->setParam("runCnt", ival);
        avi->setParam("fcn",
                      schedFcn);  // we'll run the function this way without adding actions.
        if (schedTarg)
            avi->setParam("targ", schedTarg);  // optional sched target
        avi->am = av->am;
    }
    // move any allowed updates to the running var
    if (avi)
    {
        if (refTime > 0)
            avi->setParam("refTime", refTime);
        if (runTime > 0)
            avi->setParam("runTime", runTime);
        if (runCnt >= 0)
            avi->setParam("runCnt", runCnt);
        if (repTime > 0)
            avi->setParam("repTime", repTime);
        if (endTime > 0)
            avi->setParam("endTime", endTime);
        if (schedFcn && strcmp(schedFcn, (char*)"None") != 0)
            avi->setParam("fcn",
                          schedFcn);  // we'll run the function this way without adding actions.
        if (schedTarg && strcmp(schedTarg, (char*)"None") != 0)
            avi->setParam("targ", schedTarg);  // optional sched target
    }

    // get next runTime runAfter is relative
    // logic
    //  just runTime set runTime runEnd = 0
    //       runTime    refTime repTime  runAfter  runFor             startTime
    //       endTime x          -       -        -         -
    //       tNow+runTime                               0 x          -       x
    //       -         -                  tNow+runTime 0 x          x       -
    //       -         -                  tNow+runTime 0 x          x       x
    //       -         -                  refTime+(x*repTime)>tNow+runTime 0 x
    //       -       -        x         -                  tNow+runTime+runAfter 0
    //       x          -       x        x         -
    //       tNow+(runTime +(x *repTime) > tNow+runAfter 0 x          x       -
    //       x         -                  tNow+runAfter 0 x          x       x
    //       x         -                  refTime +(x*repTime) > tNow+ runAfter 0
    //       .          .       .        .         x                  .
    //       startTime + runFor

    //       runTime plus runAfter plus runRef plus runRep        runTime = inc
    //       runRef by runRep untill past tNow plus runAfter runTime plus runAfter
    //       no runRef plus runRep          runTime = inc tNow by runRep untill
    //       past tNow plus runAfter
    //
    double startTime = tNow + runTime;

    if (av->gotParam("refTime") && av->gotParam("repTime"))
    {
        startTime = refTime;
    }
    // runAfter will default to 0
    // runTime will default to 0
    if (av->gotParam("repTime") && repTime > 0.0)
    {
        while (startTime < (runTime + runAfter + tNow))
        {
            startTime += repTime;
        }
    }
    runTime = startTime;
    avi->setParam("runTime", runTime);
    if (av->gotParam("runFor") && runFor > 0.0)
    {
        endTime = runTime + runFor;
        avi->setParam("runEnd", (runTime + runFor));
        avi->setParam("endTime", endTime);
    }
    else
    {
        double dval = 0.0;
        avi->setParam("runEnd", dval);
        avi->setParam("endTime", dval);
    }

    if (avi && avi->am)
    {
        as = new schedItem();

        FPS_PRINT_INFO("created new schedItem {}", fmt::ptr(as));  // spdlog
        as->av = avi;
        if (avi->am)
            am = avi->am;

        char* myid = av->getcVal();

        // this dummy function will simply send the current time to avi triggering
        // its onSet actions infact we'll do this anyway without a function
        // void schedItem::setUp(const char * _id, const char* _aname, const char *
        // _uri, const char* _func, double _refTime,
        //    double _runTime, double _repTime, double _endTime, char*targaV)

        as->setUp(myid, am->name.c_str(), schedUri, schedFcn, refTime, runTime, repTime, endTime, schedTarg);

        if (debug)
            as->show();

        // clean up the av junk so that we dont reuse it.
        av->setParam("refTime", -1.0);
        av->setParam("last_refTime", refTime);
        av->setParam("runTime", -1.0);
        av->setParam("last_runTime", runTime);
        av->setParam("repTime", -1.0);
        av->setParam("last_repTime", repTime);
        av->setParam("runAfter", -1.0);
        av->setParam("last_runAfter", runAfter);
        av->setParam("runFor", -1.0);
        av->setParam("last_runFor", runFor);
        av->setParam("endTime", -1.0);
        av->setParam("last_endTime", endTime);
        FPS_PRINT_INFO("@@@@@@ reset old values {}   avi {} runTime {:2.3f}", cstr{ schedUri }, fmt::ptr(avi),
                       avi->getdParam("runTime"));  // spdlog
        char* nonestr = strdup("None");
        // once we replce the Params the old values are no longer valid , they have
        // been free'd
        if (schedUri)
        {
            av->setParam("last_uri", schedUri);
            av->setParam("uri", nonestr);
            schedUri = av->getcParam("last_uri");
        }
        if (schedFcn)
        {
            av->setParam("last_fcn", schedFcn);
            av->setParam("fcn", nonestr);
            schedFcn = av->getcParam("last_fcn");
        }
        if (schedTarg)
        {
            av->setParam("last_targ", schedTarg);
            av->setParam("targ", nonestr);
            schedTarg = av->getcParam("last_targ");
        }
        free(nonestr);
        if (debug)
            FPS_PRINT_INFO("@@@@@@ done with old values {}   avi {} runTime {}", cstr{ schedUri }, fmt::ptr(avi),
                           avi->getdParam("runTime"));  // spdlog
        if (am->reqChan)
        {
            channel<schedItem*>* reqChan = (channel<schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                // channel <int>* wakeChan = (channel <int>*)am->wakeChan;
                am->wakeChan->put(0);
            }
            if (debug)
                FPS_PRINT_INFO("@@@@@@ activated {}   avi {} runTime {}", cstr{ schedUri }, fmt::ptr(avi),
                               avi->getdParam("runTime"));  // spdlog
            avi->setParam("active", true);
            avi->setParam("enabled", true);
        }
        else
        {
            FPS_PRINT_INFO(
                "@@@@@ You need to set up am->reqChan and am->wakeChan "
                "@@@@@",
                NULL);  // spdlog
            delete as;
            as = nullptr;
        }
    }
    else
    {
        FPS_PRINT_INFO("@@@@@ did not find var to activate avi {} avi->am {}", fmt::ptr(avi),
                       fmt::ptr(avi ? avi->am : nullptr));  // spdlog
    }
    return 0;
}
// Use this to trigger a from the gpio pub
// fims_send -m set -r /$$ -u /gpio/control/gpio/getPubs true
// p_fims->Send("set","/gpio/contol/gpio/getPubs", nullptr, "true");

int EssSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = av->am;
    // const auto now = flex::get_time_dbl();
    // double tNow = now.count();
    bool debug = false;
    if (am->reload != 2)
    {
        if (debug)
            FPS_PRINT_INFO("running init for {} Manager", am->name);  // spdlog
        // am->run_init(am);
        // static int setup = 0;
        // assetVar *av = am->amap["MaxCellTemperature"];
        if (!am->setup)
        {
            if (debug)
                FPS_PRINT_INFO("MANAGER LOOP {}  setup MonitorVar", am->name);  // spdlog
            // TODO needs to be automated
            // get base am from vm->getSysName and set up all the direct reports
            am->vm->setMonitorList2(*am->vmap, "bms", "wake_monitor");
            am->vm->setMonitorList2(*am->vmap, "pcs", "wake_monitor");
            am->vm->setMonitorList2(*am->vmap, "site", "wake_monitor");
            am->setup = 1;
            if (debug)
                FPS_PRINT_INFO("MANAGER LOOP {} completed setup MonitorVar",
                               am->name);  // spdlog
        }
        bool bval = false;
        amap["RunPub"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunPub", bval);
        amap["SendPcs"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "SendPcs", bval);
        amap["SendBms"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "SendBms", bval);
        amap["RunPub"]->setVal(true);
        amap["SendPcs"]->setVal(true);
        amap["SendBms"]->setVal(true);
    }
    if (debug)
        FPS_PRINT_INFO(">>>>>>>>>>>>>>>>>>>>> done running init for {} Manager",
                       am->name);  // spdlog
    // sync up with the gpio system
    am->p_fims->Send("set", "/gpio/contol/gpio/getPubs", nullptr, "true");

    return 0;
}

// this can now be attached to different assetVars to allow it to run at
// different rates
int SendTrue(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(p_fims);
    asset_manager* am = aV->am;
    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                        aname);  // spdlog
        return 0;
    }
    VarMapUtils* vm = am->vm;
    bool bval = true;
    vm->setVal(vmap, aV->getfName(), nullptr, bval);
    if (0)
        FPS_PRINT_INFO(" send [{}] to [{}]", bval, aV->getfName());  // spdlog
    return 0;
}
// this can now be attached to different assetVars to allow it to run at
// different rates
int SendTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(p_fims);
    asset_manager* am = aV->am;
    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                        aname);  // spdlog
        return 0;
    }
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();
    vm->setVal(vmap, aV->getfName(), nullptr, tNow);
    if (0)
        FPS_PRINT_INFO(" send [{}] to [{}]", tNow, aV->getfName());  // spdlog
    return 0;
}

// this can now be attached to different assetVars to allow it to run at
// different rates
int RunMonitorList(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(p_fims);
    asset_manager* am = aV->am;
    bool debug = false;
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    if (debug)
        FPS_PRINT_ERROR("  RUNNING am ({}) (or vm) for av [{}] aname [{}] debug [{}]", fmt::ptr(am),
                        cstr{ aV->getfName() }, aname, debug);  // spdlog

    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                        aname);  // spdlog
        return 0;
    }

    VarMapUtils* vm = am->vm;
    const char* mname = "wake_monitor";
    if (aV->gotParam("mname"))
    {
        mname = aV->getcParam("mname");
    }
    const char* amname = vm->getSysName(vmap);
    if (aV->gotParam("amname"))
    {
        amname = aV->getcParam("amname");
    }
    bool runChild = true;
    if (aV->gotParam("runChild"))
    {
        runChild = aV->getbParam("runChild");
    }

    auto fname = fmt::format("{}_{}", __func__, mname);
    essPerf ePerf(am, aname, fname.c_str());
    if (debug)
        FPS_PRINT_ERROR("running for [{}] amname [{}] monitor_name [{}]", cstr{ aV->getfName() }, amname,
                        mname);  // spdlog

    vm->runMonitorList2(vmap, am->amap, amname, vm->p_fims, mname, debug);

    // Run the monitor list for all assets/asset managers that have the list
    if (runChild)
    {
        for (auto& ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            if (debug)
                FPS_PRINT_INFO("Got asset manager {}. Running func now...",
                               amc->name);  // spdlog
                                            // Run monitor list for asset managers
            am->vm->runMonitorList2(vmap, am->amap, amc->name.c_str(), vm->p_fims, mname);

            for (auto& iy : amc->assetMap)
            {
                asset* ami = iy.second;
                if (debug)
                    FPS_PRINT_INFO(
                        "Got asset instance {} {} with asset manager {}. "
                        "Running monitor list now...",
                        ami->name, fmt::ptr(ami),
                        ami->am ? ami->am->name : "null");  // spdlog
                // Run monitor list for asset instances
                am->vm->runMonitorList2(*am->vmap, am->amap, ami->name.c_str(), am->p_fims, mname);
            }
        }
    }
    return 0;
}

int HandlePowerRackEst(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(p_fims);
    // return 0;

    bool debug = true;
    asset_manager* am = aV->am;
    int ival = 0;
    double dval = 0.0;
    if (!am || !am->vm)
    {
        if (debug)
            FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                            aname);  // spdlog
        return 0;
    }
    VarMapUtils* vm = am->vm;
    // this is for the bms
    //
    asset* ai = vm->getaI(vmap, aname);
    if (!ai)
    {
        FPS_PRINT_ERROR("no ai ({}) (or vm) for [{}] aname [{}]", fmt::ptr(ai), cstr{ aV->getfName() },
                        aname);  // spdlog
        return 0;
    }

    if (amap.find("SBMUMaxDischargeCurrent") == amap.end())
    {
        FPS_PRINT_ERROR("no [{}] in  aname [{}]", "SBMUMaxDischargeCurrent",
                        aname);  // spdlog
        return 0;
    }
    if (amap.find("SBMUMaxChargeCurrent") == amap.end())
    {
        FPS_PRINT_ERROR("no [{}] in  aname [{}]", "SBMUMaxChargeCurrent",
                        aname);  // spdlog
        return 0;
    }
    // sbmu_sum_cells":13709 == volts
    if (amap.find("MBMUMaxChargeCurrent") == amap.end())
    {
        FPS_PRINT_ERROR("no [{}] in  aname [{}]", "MBMUMaxChargeCurrent",
                        aname);  // spdlog
        amap["MBMUMaxChargeCurrent"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUMaxChargeCurrent",
                                                      dval);
        amap["MBMUMaxChargeVolts"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUMaxChargeVolts", dval);

        amap["MBMUMinChargeCurrent"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUMinChargeCurrent",
                                                      dval);
        amap["MBMUMinChargeVolts"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUMinChargeVolts", dval);

        amap["MBMUMaxDischargeCurrent"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status",
                                                         "MBMUMaxDischargeCurrent", dval);
        amap["MBMUMaxDischargeVolts"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUMaxDischargeVolts",
                                                       dval);

        amap["MBMUMinDischargeCurrent"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status",
                                                         "MBMUMinDischargeCurrent", dval);
        amap["MBMUMinDischargeVolts"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUMinDischargeVolts",
                                                       dval);

        amap["MBMUNumSbmus"] = vm->setLinkVal(vmap, ai->am->name.c_str(), "/status", "MBMUNumsbmus", ival);
    }

    double SBMUMaxDischargeCurrent = amap["SBMUMaxDischargeCurrent"]->getdVal();
    // double SBMUMinDischargeCurrent =
    // amap["SBMUMinDischargeCurrent"]->getdVal();
    double SBMUMaxChargeCurrent = amap["SBMUMaxChargeCurrent"]->getdVal();
    // double SBMUMinChargeCurrent    = amap["SBMUMinChargeCurrent"]->getdVal();
    double MBMUMaxDischargeCurrent = amap["MBMUMaxDischargeCurrent"]->getdVal();
    // double MBMUMinDischargeCurrent =
    // amap["MBMUMinDischargeCurrent"]->getdVal();
    double MBMUMaxChargeCurrent = amap["MBMUMaxChargeCurrent"]->getdVal();
    // double MBMUMinChargeCurrent    = amap["MBMUMinChargeCurrent"]->getdVal();

    int MBMUNumSbmus = amap["MBMUNumSbmus"]->getiVal();

    // TODO link these
    // double soc    = amap["SBMUSoc"]->getdVal();
    double volts = 1327.0;  // amap["SBMUVolts"]->getdVal();

    if (SBMUMaxDischargeCurrent > 0.0)
    {
        MBMUNumSbmus++;
        amap["MBMUNumSbmus"]->setVal(MBMUNumSbmus);
    }
    if (SBMUMaxChargeCurrent > 0.0)
    {
        if (MBMUMaxChargeCurrent == 0.0 || (SBMUMaxChargeCurrent < MBMUMaxChargeCurrent))
        {
            amap["MBMUMaxChargeCurrent"]->setVal(SBMUMaxChargeCurrent);
            amap["MBMUMaxChargeVolts"]->setVal(volts);
        }
    }
    if (SBMUMaxDischargeCurrent > 0.0)
    {
        if (MBMUMaxDischargeCurrent == 0.0 || (SBMUMaxDischargeCurrent < MBMUMaxDischargeCurrent))
        {
            amap["MBMUMaxDischargeCurrent"]->setVal(SBMUMaxDischargeCurrent);
            amap["MBMUMaxDischargeVolts"]->setVal(volts);
        }
    }

    FPS_PRINT_INFO("MBMUMaxDischargeCurrent [{}] in  aname [{}] numSbmus {}",
                   amap["MBMUMaxDischargeCurrent"]->getdVal(), aname,
                   MBMUNumSbmus);  // spdlog

    return 0;
}

int HandlePowerEst(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    // bool debug = true;
    double dval = 0.0;
    // double ival = 0;
    asset_manager* am = aV->am;

    if (0)
        FPS_PRINT_INFO(" Running  am ({}) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() }, aname);  // spdlog
    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                        aname);  // spdlog
        return 0;
    }
    VarMapUtils* vm = am->vm;
    // this is for the bms
    //
    am = vm->getaM(vmap, "bms");
    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                        "bms");  // spdlog
        return 0;
    }
    if (0)
        FPS_PRINT_ERROR(" Running  am ({}) for [{}] aname [{}]", fmt::ptr(am), am->name, aname);  // spdlog
    // get the maxcharge / disharge current and volts for each rack
    if (am->amap.find("MaxChargeCurrent") == am->amap.end())
    {
        FPS_PRINT_ERROR("no [{}] in  aname [{}]", "MaxChargeCurrent",
                        aname);  // spdlog
        am->amap["MaxChargeCurrent"] = am->vm->setLinkVal(vmap, am->name.c_str(), "/status", "MaxChargeCurrent", dval);
        am->amap["MaxDischargeCurrent"] = am->vm->setLinkVal(vmap, am->name.c_str(), "/status", "MaxDischargeCurrent",
                                                             dval);
        // amap["MBMUMaxChargeVolts"]      = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMaxChargeVolts", dval);

        // amap["MBMUMinChargeCurrent"]    = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMinChargeCurrent", dval);
        // amap["MBMUMinChargeVolts"]      = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMinChargeVolts", dval);

        // amap["MBMUMaxDischargeCurrent"] = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMaxDischargeCurrent", dval);
        // amap["MBMUMaxDischargeVolts"]   = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMaxDischargeVolts", dval);

        // amap["MBMUMinDischargeCurrent"] = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMinDischargeCurrent", dval);
        // amap["MBMUMinDischargeVolts"]   = am->vm->setLinkVal(vmap,
        // am->name.c_str(), "/status", "MBMUMinDischargeVolts", dval);
        // amap["MBMUNumSbmus"] = am->vm->setLinkVal(vmap, am->name.c_str(),
        // "/status", "MBMUNumsbmus", ival);
    }
    if (1)
    {
        dval = 0.0;
        am->amap["MaxChargeCurrent"]->setVal(dval);
        am->amap["MaxDischargeCurrent"]->setVal(dval);
        am->amap["MaxChargeCurrent"]->am = am;
        am->amap["MaxDischargeCurrent"]->am = am;
        // this now does it all HandlePower.cpp
        AggregateManager(vmap, am->amap, am->name.c_str(), p_fims, am->amap["MaxChargeCurrent"]);
        AggregateManager(vmap, am->amap, am->name.c_str(), p_fims, am->amap["MaxDischargeCurrent"]);
        // for (auto& ix : am->assetManMap)
        // {
        //     asset_manager* amc = ix.second;
        //     if (debug) FPS_PRINT_INFO("{} >> Got asset manager {}. Running
        //     HandlePowerEst now...\n"
        //              , __func__, amc->name.c_str());
        //     // Handle asset manager asset Power
        //     HandlePowerEst(*am->vmap, am->amap, amc->name.c_str(), am->p_fims,
        //     aV);

        // }
        // Run for asset instances
        // if(0)
        // {
        //     for (auto& iy : am->assetMap)
        //     {
        //         asset* ami = iy.second;
        //         if (debug) FPS_PRINT_INFO("{} >> Got asset instance {}. Running
        //         HandlePowerRackEst now...\n"
        //                 , __func__, ami->name.c_str());
        //         HandlePowerRackEst(*ami->vmap, ami->amap, ami->name.c_str(),
        //         ami->p_fims, aV);
        //     }
        // }
    }
    return 0;
}

int Every100mSP1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(p_fims);
    asset_manager* am = aV->am;
    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]", fmt::ptr(am), cstr{ aV->getfName() },
                        aname);  // spdlog
        return 0;
    }
    // bool debug =  false;

    essPerf ePerf(am, aname, __func__);
    UpdateSysTime(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);
    // CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);
    // HandlePowerLimit(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);
    // HandlePowerEst(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);
    HandlePowerCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);
    CheckGPIO(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);
    RunMonitorList(*am->vmap, am->amap, am->name.c_str(), am->p_fims, aV);

    // // Run the monitor list for all assets/asset managers that have the list
    // if (1)
    // {
    //     for (auto& ix : am->assetManMap)
    //     {
    //         asset_manager* amc = ix.second;
    //         if (0) FPS_PRINT_INFO("{} >> Got asset manager {}. Running func
    //         now...\n"
    //                  , __func__, amc->name.c_str());
    //         // Run monitor list for asset manager
    //         am->vm->runMonitorList2(*am->vmap, am->amap, amc->name.c_str(),
    //         am->p_fims, "wake_monitor");

    //         // Run monitor list for asset instances
    //         for (auto& iy : amc->assetMap)
    //         {
    //             asset* ami = iy.second;
    //             if (0) FPS_PRINT_INFO("{} >> Got asset instance {}. Running
    //             monitor list now...\n"
    //                  , __func__, ami->name.c_str());
    //             am->vm->runMonitorList2(*am->vmap, am->amap, ami->name.c_str(),
    //             am->p_fims, "wake_monitor");
    //         }
    //     }
    // }
    // // defaultFault_module::ShutdownSystem(*am->vmap, am->amap,
    // am->name.c_str(), am->p_fims, av);
    // // defaultRun_module::StartupSystem(*am->vmap, am->amap, am->name.c_str(),
    // am->p_fims, av);
    // // am->vm->runMonitorList2(*am->vmap, am->amap, essName,am->p_fims,
    // "wake_monitor");//am->p_fims,/*am->amap,*/ 0.1, 0.0);
    // // am->vm->runMonitorList2(*am->vmap, am->amap, "bms",am->p_fims,
    // "wake_monitor");//am->p_fims,/*am->amap,*/ 0.1, 0.0);
    // // am->vm->runMonitorList2(*am->vmap, am->amap, "pcs",am->p_fims,
    // "wake_monitor");//am->p_fims,/*am->amap,*/ 0.1, 0.0);
    // // am->vm->runMonitorList2(*am->vmap, am->amap, "site",am->p_fims,
    // "wake_monitor");
    return 0;
}

int Every100mSP2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(p_fims);
    essPerf ePerf(av->am, aname, __func__);

    return 0;
}

int Every100mSP3(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    if (av->am)
    {
        const auto now = flex::get_time_dbl();
        double tNow = now.count();  // vm->get_time_dbl();

        // essPerf ePerf(av->am, aname, __func__);
        FPS_PRINT_INFO("running", NULL);  // spdlog

        auto tmp = fmt::format("{} still running at {}", __func__, tNow);
        av->setParam("response", tmp.c_str());
    }
    return 0;
}

int Every1000mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    essPerf ePerf(av->am, aname, __func__);
    asset_manager* am = av->am;

    bool runMe = true;

    HandleCpuStats(vmap, amap, aname, p_fims, av);
    if (am->amap["SendPcs"])
    {
        bool sendPcs = am->amap["SendPcs"]->getbVal();
        asset_manager* am2 = am->getManAsset("pcs");
        if (am2)
        {
            am2->sendOK = sendPcs;
        }
        if (!runMe)
            FPS_PRINT_INFO("MANAGER LOOP {} WAKE1000 sendPcs {} am2 {} [{}]", am->name, sendPcs, fmt::ptr(am2),
                           am2->name);  // spdlog
    }
    if (am->amap["SendBms"])
    {
        bool sendBms = am->amap["SendBms"]->getbVal();
        asset_manager* am2 = am->getManAsset("bms");
        if (am2)
        {
            am2->sendOK = sendBms;
        }
    }
    am->run_secs++;
    if (am->run_secs == 30)
    {
        essPerf ePerf(av->am, aname, "config_after_30s");

        // char* fname = nullptr;
        auto fname = fmt::format("{}_{}_after_30_seconds.json", am->vm->runCfgDir ? am->vm->runCfgDir : "run_configs",
                                 am->vm->getSysName(vmap));

        cJSON* cjbm = am->vm->getMapsCj(vmap, nullptr, nullptr, 0x10000);
        if (cjbm)
        {
            am->vm->write_cjson(fname.c_str(), cjbm);
            cJSON_Delete(cjbm);
        }
    }
    if (am->run_secs == 32)
    {
        essPerf ePerf(av->am, aname, "config_after_32s");
        auto fname = fmt::format("{}_{}_after_32_seconds.json", am->vm->runCfgDir ? am->vm->runCfgDir : "run_configs",
                                 cstr{ am->vm->getSysName(vmap) });

        cJSON* cjbm = am->vm->getMapsCj(vmap, nullptr, nullptr, 0x10000);
        if (cjbm)
        {
            am->vm->write_cjson(fname.c_str(), cjbm);
            cJSON_Delete(cjbm);
        }
    }
    if (am->run_secs == 31)
    // if(0)
    {
        essPerf ePerf(av->am, aname, "config_after_31s");
        auto fname = fmt::format("{}_{}_after_31_seconds.json", am->vm->runCfgDir ? am->vm->runCfgDir : "run_configs",
                                 cstr{ am->vm->getSysName(vmap) });

        auto output = fmt::format("{:f}", *am->vmap);

        auto fp = fopen(fname.c_str(), "w");
        if (!fp)
        {
            FPS_PRINT_INFO("Failed to open file {}", fname);
        }
        else
        {
            auto bytes_written = fwrite(output.c_str(), 1, output.size(), fp);
            FPS_PRINT_INFO("Wrote {} bytes to file {}", bytes_written, fname);
            fclose(fp);
        }
    }
    return 0;
}

// TODO review after MVP  needs a pointer to the sysVec
// modify this to do one at a time
int SlowPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(amap);
    UNUSED(p_fims);
    essPerf ePerf(av->am, aname, __func__);
    // const auto now = flex::get_time_dbl();
    // double tNow = now.count();//vm->get_time_dbl();

    asset_manager* am = av->am;
    bool runPub = true;

    if (0)
        FPS_PRINT_INFO("aname [{}] am {}  publish status RunPub {} am->vm {}", aname, fmt::ptr(am), runPub,
                       fmt::ptr(am ? am->vm : nullptr));  // spdlog

    double refTime = av->getdParam("refTime");

    HandleCpuStats(*am->vmap, am->amap, am->name.c_str(), am->p_fims, av);
    if (am->amap["RunPub"])
    {
        runPub = am->amap["RunPub"]->getbVal();
    }
    if (0)
        FPS_PRINT_INFO("aname [{}] >> publish status RunPub {} -> {}", am->name, am->amap["RunPub"], runPub);  // spdlog
    if (runPub && am->syscVec)
    {
        // this is all you need to run something
        HandleSplitItem(vmap, av, "SlowPubOne"  //SchedItem ID
                        ,
                        "SlowPubOne"  // Function name
                        ,
                        "/shed/ess:slowPubOne"  // sched control av
                        ,
                        refTime + 0.010  // ref time ( rel priority ) 0.5
                        ,
                        0.010  // repeat time , 0 = run once
                        ,
                        0.0  // , tNow+5
                        ,
                        0.0  // endTime
        );
    }

    return 0;
}

// modify this to do one at a time
int SlowPubOne(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(amap);
    UNUSED(p_fims);
    essPerf ePerf(av->am, aname, __func__);

    asset_manager* am = av->am;
    bool runPub = true;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();  // vm->get_time_dbl();

    if (0)
        FPS_PRINT_INFO("aname [{}] am {}  publish status RunPub {} am->vm {}", aname, fmt::ptr(am), runPub,
                       fmt::ptr(am ? am->vm : nullptr));  // spdlog

    double tThen = tNow;

    if (am->amap["RunPub"])
    {
        runPub = am->amap["RunPub"]->getbVal();
    }
    int idx = av->getiParam("idx");

    if (0)
        FPS_PRINT_INFO("aname [{}] >> RunPub {} -> {} idx {} size {}", am->name, am->amap["RunPub"], runPub, idx,
                       am->syscVec->size());  // spdlog
    if (runPub && am->syscVec)
    {
        if (idx >= (int)am->syscVec->size())
        {
            av->setParam("idx", -1);    // ised to send site_ls when woken up again
            av->setParam("repCnt", 0);  // not needed any mo
            av->setParam("endTime",
                         1.0);  // stops slowpubone running and removes the schedItem
            return 0;
        }
        if (idx == -2)
        {
            av->setParam("repCnt", 0);
            av->setParam("endTime", 1.0);
            return 0;
        }
        if (idx == -1)
        {
            // TODO fix this from a sched item
            // HACK HACK HACK
            // this is for the site manager
            essPerf ePerf(av->am, aname, "/site/ess_ls");
            SendPub(vmap, am, "/site/ess_ls", "/site/ess_ls", av);
            const auto now = flex::get_time_dbl();
            double tThen = now.count();  // vm->get_time_dbl();

            if (0)
                FPS_PRINT_INFO("aname [{}]>>publish [{}]>>elapsed {:2.3f}mS", am->name, "/site/ess_ls",
                               (tThen - tNow) * 1000.0);  // spdlog

            idx = 0;
            av->setParam("idx", idx);
        }
        else
        {
            auto svp = am->syscVec->at(idx);
            idx++;
            av->setParam("idx", idx);

            // Jimmy - A hack right now. Don't really want to pub to
            // /assets/site_controller/summary at the moment since that is not in the
            // config file (site_controller_manager.json)
            std::string sv = svp;
            if (sv != "site/summary")
            {
                std::string auri = "/assets/";
                auri += sv;
                essPerf ePerf(av->am, aname, auri.c_str());
                SendPub(vmap, am, auri.c_str(), auri.c_str(), av);
                const auto now = flex::get_time_dbl();
                tThen = now.count();  // vm->get_time_dbl();

                if (0)
                    FPS_PRINT_INFO("aname [{}] >> publish [{}] elapsed {:2.3f}mS", am->name, auri,
                                   (tThen - tNow) * 1000.0);
            }
            tNow = tThen;
        }
    }
    return 0;
}

int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(amap);
    UNUSED(p_fims);
    essPerf ePerf(av->am, aname, __func__);
    // const auto now = flex::get_time_dbl();
    // double tNow = now.count();//vm->get_time_dbl();

    asset_manager* am = av->am;
    bool runPub = true;
    char* table = nullptr;  //"/site/ess_hs";
    char* dest = nullptr;   //"/site/ess_hs";
    bool debug = false;
    if (av->gotParam("debug"))
    {
        debug = av->getbParam("debug");
    }

    if (av->gotParam("table"))
    {
        table = av->getcParam("table");
        dest = table;
    }
    if (av->gotParam("dest"))
    {
        dest = av->getcParam("dest");
    }
    // Use defaults
    if (!table | !dest)
    {
        table = (char*)"/site/ess_hs";
        dest = (char*)"/site/ess_hs";
    }
    // double tNow =  am->vm->get_time_dbl();

    if (debug)
        FPS_PRINT_INFO("aname [{}] RunPub [{}] table [{}] dest [{}]", aname, runPub, cstr{ table },
                       cstr{ dest });  // spdlog

    if (am->amap["RunPub"])
    {
        runPub = am->amap["RunPub"]->getbVal();
        if (debug)
            FPS_PRINT_INFO("aname [{}] >> publish status RunPub {} -> {}", am->name, am->amap["RunPub"],
                           runPub);  // spdlog
    }
    if (runPub)
    {
        // SendPub(am, "/site/ess_hs", "/site/ess_hs");
        SendPub(vmap, am, table, dest, av);
    }
    return 0;
}

// int  RunScript(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims,
// assetVar* av)
// {
//     auto cmd = fmt::format("sh /home/scripts/RunScript.sh {} {}&", av->comp,
//     av->name); int rc = system(cmd.c_str()); if(rc != 0 )
//     {
//         FPS_PRINT_ERROR(" Unable to run command {} ", cmd);
//     }

//     return 0;
// }

int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(amap);
    UNUSED(p_fims);
    essPerf ePerf(av->am, aname, __func__);

    asset_manager* am = av->am;
    VarMapUtils* vm = am->vm;

    double package = 0.0;
    double core0 = 0.0;
    double core1 = 0.0;
    double core2 = 0.0;
    double core3 = 0.0;
    int currRealMem;
    int peakRealMem;
    int currVirtMem;
    int peakVirtMem;

    getTemps(&package, &core0, &core1, &core2, &core3);
    getMemory(&currRealMem, &peakRealMem, &currVirtMem, &peakVirtMem);
    vm->setVal(vmap, "/status/ess", "currRealMem", currRealMem);
    vm->setVal(vmap, "/status/ess", "peakRealMem", peakRealMem);
    vm->setVal(vmap, "/status/ess", "currVirtMem", currVirtMem);
    vm->setVal(vmap, "/status/ess", "peakVirtMem", peakVirtMem);

    vm->setVal(vmap, "/status/ess", "system_temp", package);
    vm->setVal(vmap, "/status/ess", "core0_temp", core0);
    vm->setVal(vmap, "/status/ess", "core1_temp", core1);
    vm->setVal(vmap, "/status/ess", "core2_temp", core2);
    vm->setVal(vmap, "/status/ess", "core3_temp", core3);

    // SetupGit(vmap, amap, aname, p_fims, av
    //              , GITBRANCH
    //              , GITCOMMIT
    //              , GITTAG
    //              , GITVERSION
    //              );

    // auto tmp = fmt::format("{}", GITBRANCH);
    // char* tval = (char*)tmp.c_str();
    // vm->setVal(vmap, "/status/ess", "git_branch", tval);

    // tmp = fmt::format("{}", GITCOMMIT);
    // tval = (char*)tmp.c_str();
    // vm->setVal(vmap, "/status/ess", "git_commit", tval);

    // // cJSON*cj;
    // // cj = cJSON_CreateObject();
    // // cJSON_AddStringToObject(cj,"value",GITBRANCH);
    // // vm->setValfromCj(vmap, "/status/ess","git_branch",cj);
    // // cJSON_Delete(cj);

    // // cj = cJSON_CreateObject();
    // // cJSON_AddStringToObject(cj,"value",GITCOMMIT);
    // // vm->setValfromCj(vmap, "/status/ess","git_commit",cj);
    // // cJSON_Delete(cj);
    // // // free(tmp);
    // // cj = cJSON_CreateObject();

    // //char *tmp;
    // //asprintf(&tmp, "%s-%s",GITTAG,GITVERSION);
    // tmp = fmt::format("{}-{}", GITTAG, GITVERSION);
    // tval = (char*)tmp.c_str();
    // //cJSON_AddStringToObject(cj,"value",tmp.c_str());
    // vm->setVal(vmap, "/status/ess", "build", tval);
    // //cJSON_Delete(cj);
    // //free(tmp);

    return 0;
}

void print_version()
{
    auto tmp = fmt::format("             git_branch: {}", GITBRANCH);
    printf("%s \n", tmp.c_str());
    tmp = fmt::format("             git_commit: {}", GITCOMMIT);
    printf("%s \n", tmp.c_str());
    tmp = fmt::format("             build:      {}-{}", GITTAG, GITVERSION);
    printf("%s \n", tmp.c_str());
}

int SetupGit(varsmap& vmap, VarMapUtils* vm, const char* gbranch, const char* gcommit, const char* gtag,
             const char* gversion)
{
    UNUSED(gbranch);
    UNUSED(gcommit);
    UNUSED(gtag);
    UNUSED(gversion);
    auto comp = fmt::format("/status/{}", vm->getSysName(vmap));
    // auto tmp = fmt::format("{}", gbranch);
    auto tmp = fmt::format("{}", GITBRANCH);
    char* tval = (char*)tmp.c_str();
    vm->setVal(vmap, comp.c_str(), "git_branch", tval);

    tmp = fmt::format("{}", GITCOMMIT);
    tval = (char*)tmp.c_str();
    vm->setVal(vmap, comp.c_str(), "git_commit", tval);

    tmp = fmt::format("{}-{}", GITTAG, GITVERSION);
    tval = (char*)tmp.c_str();
    vm->setVal(vmap, comp.c_str(), "build", tval);

    return 0;
}

#endif
