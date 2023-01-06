#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close

#include "asset.h"
#include <fims/libfims.h>
#include <csignal>

#include "channel.h"
#include "varMapUtils.h"
#include "scheduler.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"
#include "formatters.hpp"

#ifndef FPS_ERROR_FMT
#define FPS_ERROR_FMT(...)     fmt::print(stderr,__VA_ARGS__)
#endif

// things planned 06052021
// find out why amaps are blown away with load pcs
//
// socket list  socket_192.168.112.3:1234
//          convert db thing to use this and just send to the fd.

// show sysvec
// fix ifchanged


// thread list
//    create thread to service socket.
/// avec aval perhaps  looking good..


// CalculateVar use sysname.. 

// ifchanged .. at the end of the whole action

// question ? how do we enable the checkuri bypass to be "gpio" and not essName
// answer set up vm.uriroot

// Use this to trigger a pub
// fims_send -m set -r /$$ -u /gpio/control/gpio/getPubs true

// use this to enter sim mode.
// fims_send -m set -r /$$ -u /gpio/full/config/GPIOsim 1
// trigger a sim value

// use this to change the pub rate
// fims_send -m set -r /$$ -u /gpio/full/sched/gpio '{"schedGpioRwPub":{"value":"schedGpioRwPub","repTime":20}}'




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
    void SendPub(varsmap &vmap, asset_manager* am, const char* uri, const char* puri, assetVar* av);
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
    int RunLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int MakeLink(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunTpl(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunPub(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int DumpConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadServer(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadClient(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int LoadFuncs(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    void SendPub(varsmap &vmap, asset_manager* am, const char* uri, const char* puri, assetVar* av);    
    VarMapUtils* getVm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunSystemCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int process_sys_alarm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int CalculateVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunCell(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int SetupRwFlexSched(scheduler*sched, asset_manager* am);
    int FlexInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int CheckTableVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int MathMovAvg(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int SimHandlePcs(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int FastPub(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunDb(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    //int CheckTableVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av1, assetVar* av2, assetVar* tblAv);
    bool loadSiteMap(varsmap& vmap, VarMapUtils* vm, const char* cfgname);
    bool loadClientMap(varsmap& vmap, VarMapUtils* vm, const char* cfgname);
    int RunAllVLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunAllLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunAllLinks2(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunAllAlists(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int RunSysVec(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int SetMapping(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int SlewVal(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int BalancePower(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);


}
// if we have options link them all
// if(av->extras && av->extras && av->extras->optVec && av->extras->optVec->cjopts)
// {
//         FPS_PRINT_INFO("@@@@@ found cjopts ready to run vlink options"); // spdlog
//         cJSON *cji;
//         int idx  = -1;
//         cJSON_ArrayForEach(cji, av->extras->optVec->cjopts)
//         {
//             setVLinkFromCj(vmap, cji, avl, aV);
//         }
// }

// todo work out how setam works here
void VarMapUtils::setVLinkFromCj(varsmap &vmap, cJSON* cji, assetVar*avl , assetVar*aV)
{ 
    const char * vname = nullptr;//cjval->valuestring;
    const char * lname = nullptr;//cjlink->valuestring;
    cJSON* cjval   = cJSON_GetObjectItem(cji, "value");
    cJSON* cjlink   = cJSON_GetObjectItem(cji, "vlink");

    assetVar* avv = nullptr;
    if( cjval && cjval->valuestring)
    {
        vname = cjval->valuestring;
        avv = getVar(vmap, vname, nullptr);
        if(!avv)
        {
            if (aV->gotParam("default"))
            {
                // new function to set a value from a param
                avv = setVal(vmap, vname, nullptr, aV, "default");
            }
            else
            {
                double dval = 0.0;
                avv = setVal(vmap, vname, nullptr, dval);
            }
            if(avv)avv->am = aV->am;
        }
    }
    if(cjlink && cjlink->valuestring)
    {
        lname = cjlink->valuestring;
        avl = getVar(vmap, lname, nullptr);
        if(!avl)
        {
            if (aV->gotParam("default"))
            {
                // new function to set a value from a param
                avl = setVal(vmap, lname, nullptr, aV, "default");
            }
            else
            {
                double dval = 0.0;
                avl = setVal(vmap, lname, nullptr, dval);
            }
            if(avl)avl->am = aV->am;
        }
    }
    if(1)FPS_PRINT_INFO("aV [{}] vname [{}] lname [{}] "
                , aV->getfName()
                , vname?vname:"no vname"
                , lname?lname:"no lname"
                );

    if(avv && avl)
    {
        avv->linkVar =  avl;
    }
}
//int SetMapping(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
int SetMapfromAv(varsmap &vmap,  VarMapUtils*vm, assetVar* aV, asset_manager**ap, asset_manager**am, asset**ai );

int RunAVLink(varsmap &vmap, varmap &amap, VarMapUtils*vm, std::string&lname, assetVar* aV)
{
    asset_manager* am = nullptr;
    asset_manager* ap = nullptr;
    asset* ai = nullptr;

    bool debug = false;
    if(aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }
    if(debug)FPS_PRINT_INFO("aV [{}] lname [{}]"
                , aV->getfName()
                , lname
                );


    //debug = true;
    auto vx = vmap.find(lname);
    int idx = 0;
    if(vx != vmap.end())
    {
        for (auto xx : vmap[lname])
        {

            am = nullptr;
            ap = nullptr;
            ai = nullptr;

            assetVar *av = xx.second;
            if(!av) continue;

            SetMapfromAv(vmap, vm, av, &ap, &am, &ai);

            
            if(debug)FPS_PRINT_INFO("idx [{:0d}] aname [{}]  av [{}]", idx, xx.first, av->getfName());
            const char *vname =  av->getcVal();
            if(!vname)
            {
                FPS_PRINT_INFO("idx [{:0d}]  ERROR av [{}] no vname, skipping"
                    , idx, av->getfName()
                    );
                    idx++;
                    continue;
            }
            bool setam = false;
            if (av->gotParam("setamap") )
            {
                setam = av->getbParam("setamap");
            }
            if(debug)FPS_PRINT_INFO("idx [{:0d}]  av [{}]  vname [{}] setam [{}]"
                    , idx, av->getfName() , vname, setam
                    );

            const char *lvar =  av->getcParam("vlink");
            // get link type
            // just use "default" value
            //const char *ltype =  av->getcParam("ltype");
            assetVar *avv = vm->getVar(vmap, vname, nullptr);
            assetVar *avl = lvar?vm->getVar(vmap, lvar, nullptr):nullptr;
            if(!avv)
            {
                if(debug)FPS_PRINT_INFO("idx [{:0d}]  av [{}]  vname [{}]  set default avv "
                    , idx, av->getfName() , vname
                    );
                if (av->gotParam("default"))
                {
                    // new function to set a value from a param
                    avv = vm->setVal(vmap, vname, nullptr, av, "default");
                }
                else
                {
                    double dval = 0.0;
                    avv = vm->setVal(vmap, vname, nullptr, dval);
                }
                if(avv)
                {
                    if(ai)
                    {
                        avv->ai = ai;
                    }
                    else if(am)
                    {
                        avv->am = am;
                    }
                    else
                    {
                        if(aV)
                            avv->am = aV->am;
                    }
                }
                //avv  = vm->setDefault(vmap,vname,nullptr, ltype?ltype:(const char *)"double");
            }

            if(lvar && !avl)
            {
                if(debug)FPS_PRINT_INFO("idx [{:0d}]  av [{}]  vname [{}]  set default avl "
                    , idx, av->getfName() , lvar
                    );
                if (av->gotParam("default"))
                {
                    // new function to set a value from a param
                    avl = vm->setVal(vmap, lvar, nullptr, av, "default");
                }
                else
                {
                    double dval = 0.0;
                    avl = vm->setVal(vmap, lvar, nullptr, dval);
                }
                if(avl)
                {
                    if(ai)
                    {
                        avl->ai = ai;
                    }
                    else if(am)
                    {
                        avl->am = am;
                    }
                    else
                    {
                        avl->am = aV->am;
                    }
                }
                //avv  = vm->setDefault(vmap,vname,nullptr, ltype?ltype:(const char *)"double");
            }

            // if(!avl && lvar )
            // {
                
            // }

            if(!avv || !avl)
            {
                if(debug)FPS_PRINT_INFO("idx [{:0d}]  ERROR >> [{}:{}] avv [{} @{}] avl [{} @{}]\n"
                    , idx, lname, xx.first
                    , vname
                    , fmt::ptr(avv)
                    , lvar?lvar:"no lVar" 
                    , fmt::ptr(avl)
                    );
            }
            if(avv && ai && setam)
            {
                ai->amap[av->comp]= avv;
            }
            else if(avv && am && setam)
            {
                am->amap[av->comp]= avv;
            }
            if(avv && avl)
            {
                // Set the link.
                if(debug)FPS_PRINT_INFO("idx [{:0d}]  av [{}]  avl [{}]  set vlink "
                    , idx, av->getfName() , avl->getfName()
                    );
                
                avv->linkVar = avl;
            }
            if(av->extras && av->extras && av->extras->optVec && av->extras->optVec->cjopts)
            {
                FPS_PRINT_INFO("@@@@@ [{}] found cjopts ready to run vlink options vm [{}]"
                    , av->getfName()
                    , fmt::ptr(vm)
                    ); 
                cJSON *cji;
                int iix  = 0;
                cJSON_ArrayForEach(cji, av->extras->optVec->cjopts)
                {
                    if(vm) vm->setVLinkFromCj(vmap, cji, avl, av);
                    iix++;
                }
                if(0)FPS_PRINT_INFO("@@@@@ [{}] found cjopts ran vlink options idx [{}].[{}]"
                    , av->getfName()
                    , idx, iix
                    ); 
            }
            idx++;       
        }
    }
    return 0;
}

int RunAllVLinks(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "allvlinks", nullptr);
    VarMapUtils *vm = aV->am->vm;
    std::vector<std::string>linkVec;
    std::string vkey = "/vlinks/";
    
    for (auto xx : vmap)
    {
        std::string vmkey = xx.first.substr(0, vkey.length());
        if(vmkey == vkey)
        {
            linkVec.emplace_back(xx.first);
        }
    }
    FPS_PRINT_INFO("found {} keys",(int)linkVec.size());
    int idx = 0;
    for (auto xx : linkVec)
    {
        std::string vmkey = xx.substr(vkey.length());
        FPS_PRINT_INFO(" processing idx [{}]  >> [{}] vmkey [{}]", idx, xx, vmkey);
        RunAVLink(vmap, amap, vm, xx, aV);
        idx++;
    }
    return 0;
}
// sh scripts/demo/0924/demo2_config_file_loading.sh
// fims_send -m set       -u /ess/system/commands '{"alllink":{"value":"ok"}}
//
// sh scripts/demo/0924/demo2_config_file_loading.sh
// fims_send -m set       -u /ess/system/commands '{"alllink":{"value":"ok"}}
//
int RunALink(varsmap &vmap, varmap &amap, VarMapUtils*vm, std::string&aname, std::string&lname, assetVar*aV)
{
    // //the amap is the key here /link/pcs
    // // means use the pcs amap
    // // if we
    // if(!checkAv(vmap, amap, aname,p_fims,aV))
    // {
    //     FPS_ERROR_FMT("{} >> ERROR unable to continue aname [{}] \n", __func__, aname);
    //     return -1;
    // }
    asset_manager* am = vm->getaM(vmap,aname.c_str());
    asset* ai = vm->getaI(vmap,aname.c_str());
    if(am)
    {
        FPS_PRINT_INFO(" found am [{}]", aname);
    }
    if(ai)
    {
        FPS_PRINT_INFO(" found ai [{}]", aname);
    }
    if (!am && !ai)
    {
        FPS_PRINT_INFO(" no ai or am skipping  [{}]", aname);
        return 0;
    }
    bool debug = false;
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }
    auto vx = vmap.find(lname);
    int idx = 0;
    if(vx != vmap.end())
    {
        for (auto xx : vmap[lname])
        {
            assetVar *av = xx.second;
            if(!av) {
                continue;
            }
            const char* vname =  av->getcVal();
            if(debug)FPS_PRINT_INFO(" 0==> xx [{}] found av [{}]  vname [{}]", xx.first, av->getfName(), vname?vname:"No Vname");
            const char* lname = nullptr;
            if (av->gotParam("lval"))
            {
                lname = av->getcParam("lval");
            }
    //         const char *lvar =  av->getcParam("link");
            assetVar *avv = vm->getVar(vmap, vname, nullptr);
            assetVar *avl = nullptr;

            if(am)
            {
                avl = am->amap[xx.first.c_str()];
                if(debug)FPS_PRINT_INFO(" 1==> xx [{}] found av [{}]  vname [{}] from am ", xx.first, av->getfName(), vname?vname:"No Vname");
            }
            if(ai)
            {
                avl = ai->amap[xx.first.c_str()];
                if(debug)FPS_PRINT_INFO(" 1==> xx [{}] found av [{}]  vname [{}] from ai ", xx.first, av->getfName(), vname?vname:"No Vname");
            }
            //FPS_PRINT_INFO(" 2==> xx [{}] found av [{}]  vname [{}]", xx.first, av->getfName(), vname?vname:"No Vname");
    //         if(!avv || !avl)
    //         {
            if(debug)FPS_PRINT_INFO("idx [{:0d}]  NOTE >> amap [{}] vname [{}] lname [{}] avv [{}] avl [{}]"
                     , idx
                     , xx.first
                     , vname ? vname:"no vname"
                     , lname ? lname : "no lname"
                     , avv?avv->getfName():"no Avv found"
                     , avl?avl->getfName():"no Avl found"
                     );
    //         }
    // first case no amap setup but we have an av "/components/bms_controls:start_connection"

            if(avv && !avl)
            {

                if(am)
                {
                    am->amap[xx.first.c_str()] = avv;
                }

                if(ai)
                {
                    ai->amap[xx.first.c_str()] = avv;
                }
    // second case no amap setup no av "/components/bms_controls:start_connection" may have a default value
    //
            }
            else if(!avv && !avl)
            {
                if (av->gotParam("default"))
                {
                    // new function to set a value from a param
                    avv = vm->setVal(vmap, vname, nullptr, av, "default");
                }
                else
                {
                    double dval = 0.0;
                    avv = vm->setVal(vmap, vname, nullptr, dval);
                }
                if(am)
                {
                    am->amap[xx.first.c_str()] = avv;
                }

                if(ai)
                {
                    ai->amap[xx.first.c_str()] = avv;
                }

            }
            else if(avl && avv)
            {
                vmap[avv->comp][avv->name] = avl;
            }
            // this may loose a value
            if(lname && avl)
            {
                assetUri my(lname);
                vmap[my.Uri][my.Var] = avl;

            }
    //             // Set the link.
    //             vmap[aav->comp][aav->name] = avl;
    //             //avv->linkVar = avl;
    //         }
            idx++;
        }
    }
    return 0;
}
// int RunALink(varsmap &vmap, varmap &amap, VarMapUtils*vm, std::string&aname, std::string&lname/*, char* ltype*/)
// {
//     // //the amap is the key here /link/pcs
//     // // means use the pcs amap
//     // // if we
//     // if(!checkAv(vmap, amap, aname,p_fims,aV))
//     // {
//     //     FPS_ERROR_FMT("{} >> ERROR unable to continue aname [{}] \n", __func__, aname);
//     //     return -1;
//     // }
//     asset_manager* am = vm->getaM(vmap,aname.c_str());
//     asset* ai = vm->getaI(vmap, aname.c_str());
//     if(am)
//     {
//         FPS_PRINT_INFO(" found am [{}] lname [{}]", aname, lname);
//     }
//     if(ai)
//     {
//         FPS_PRINT_INFO(" found ai [{}] lnam [{}]", aname, lname);
//     }
//     if (!am && !ai)
//     {
//         FPS_PRINT_INFO(" no am skipping  [{}] lname [{}]", aname, lname);
//         return 0;
//     }

//     auto vx = vmap.find(lname);
//     int idx = 0;
//     if(vx == vmap.end())
//     {
//         FPS_PRINT_ERROR(" MISSING lname : aname [{}] lname [{}]", aname, lname);
//     }
//     if(vx != vmap.end())
//     {
//         for (auto xx : vmap[lname])
//         {
//             assetVar *av = xx.second;
//             if(!av) 
//             {
//                 FPS_PRINT_ERROR(" MISSING av : xx [{}]  aname [{}] lname [{}]", xx.first, aname, lname);
//                 // we can make the var here
//                 char *ltype =  nullptr;//av->getcParam("ltype");
//                 av  = vm->setDefault(vmap,xx.first.c_str(),nullptr, ltype?ltype:(const char*)"double");
//                 if(1/*||debug*/)FPS_PRINT_INFO("idx [{:0d}]  av [{}]   set default avl "
//                     , idx, av->getfName() 
//                     );
//                 //continue;
//             }
//             const char* vname =  av->getcVal();
//             const char* lname = nullptr;
//             if (av->gotParam("lval"))
//             {
//                 lname = av->getcParam("lval");
//             }
//             // use "link" too
//             if (av->gotParam("link"))
//             {
//                 lname = av->getcParam("link");
//             }
//     //         const char *lvar =  av->getcParam("link");
//             // get the "value" avv if we have one 
//             assetVar *avv = vm->getVar(vmap, vname, nullptr);

//             // get the amap entry ( if we have one)
//             assetVar *avl = am->amap[xx.first.c_str()];
//     //         if(!avv || !avl)
//     //         {
//             FPS_PRINT_INFO("idx [{:0d}]  NOTE >> amap [{}] vname [{}] lname [{}] avv [{}] avl [{}]"
//                      , idx
//                      , xx.first
//                      , vname ? vname:"no Vname"
//                      , lname ? lname: "no Lname"
//                      , avv?avv->getfName():"no Avv found"
//                      , avl?avl->getfName():"no Avl found"
//                      );
//     //         }
//     // first case no amap setup but we have an av "/components/bms_controls:start_connection"
//             if(avv && !avl)
//             {
//                 am->amap[xx.first.c_str()] = avv;
//             }
//     // second case no amap setup no av "/components/bms_controls:start_connection" may have a default value
//     //
//             else if(!avv && !avl)
//             {
//                 if (av->gotParam("default"))
//                 {
//                     // new function to set a value from a param
//                     avv = vm->setVal(vmap, vname, nullptr, av, "default");
//                 }
//                 else
//                 {
//                     double dval = 0.0;
//                     avv = vm->setVal(vmap, vname, nullptr, dval);
//                 }
//                 am->amap[xx.first.c_str()] = avv;

//             }
//             else if(avl && avv)
//             {
//                 vmap[avv->comp][avv->name] = avl;
//             }
//             // this may loose a value
//             if(lname)
//             {
//                 assetUri my(lname);
//                 vmap[my.Uri][my.Var] = avl;

//             }
//     //             // Set the link.
//     //             vmap[aav->comp][aav->name] = avl;
//     //             //avv->linkVar = avl;
//     //         }
//             idx++;
//         }
//     }
//     return 0;
// }

int RunALink2(varsmap &vmap, varmap &amap, VarMapUtils*vm, std::string&aname, std::string&lname/*, char* ltype*/)
{
    // //the amap is the key here /link/pcs
    // // means use the pcs amap
    // // if we
    // if(!checkAv(vmap, amap, aname,p_fims,aV))
    // {
    //     FPS_ERROR_FMT("{} >> ERROR unable to continue aname [{}] \n", __func__, aname);
    //     return -1;
    // }
    asset_manager* am = vm->getaM(vmap,aname.c_str());
    asset* ai = vm->getaI(vmap, aname.c_str());
    if(am)
    {
        FPS_PRINT_INFO(" found am [{}] lname [{}]", aname, lname);
    }
    if(ai)
    {
        FPS_PRINT_INFO(" found ai [{}] lnam [{}]", aname, lname);
    }
    if (!am && !ai)
    {
        FPS_PRINT_INFO(" no am skipping  [{}] lname [{}]", aname, lname);
        return 0;
    }

    auto vx = vmap.find(lname);
    int idx = 0;
    if(vx == vmap.end())
    {
        FPS_PRINT_ERROR(" MISSING lname : aname [{}] lname [{}]", aname, lname);
    }
    if(vx != vmap.end())
    {
        for (auto xx : vmap[lname])
        {
            assetVar *av = xx.second;
            if(!av) 
            {
                FPS_PRINT_ERROR(" MISSING av : xx [{}]  aname [{}] lname [{}]", xx.first, aname, lname);
                // we can make the var here
                char *ltype =  nullptr;//av->getcParam("ltype");
                av  = vm->setDefault(vmap,xx.first.c_str(),nullptr, ltype?ltype:(const char*)"double");
                if(1/*||debug*/)FPS_PRINT_INFO("idx [{:0d}]  av [{}]   set default avl "
                    , idx, av->getfName() 
                    );
                //continue;
            }
            const char* vname =  av->getcVal();
            const char* lname = nullptr;
            if (av->gotParam("lval"))
            {
                lname = av->getcParam("lval");
            }
            // use "link" too
            if (av->gotParam("link"))
            {
                lname = av->getcParam("link");
            }
    //         const char *lvar =  av->getcParam("link");
            // get the "value" avv if we have one 
            assetVar *avv = vm->getVar(vmap, vname, nullptr);

            // get the amap entry ( if we have one)
            assetVar *avl = am->amap[xx.first.c_str()];
    //         if(!avv || !avl)
    //         {
            FPS_PRINT_INFO("idx [{:0d}]  NOTE >> amap [{}] vname [{}] lname [{}] avv [{}] avl [{}]"
                     , idx
                     , xx.first
                     , vname ? vname:"no Vname"
                     , lname ? lname: "no Lname"
                     , avv?avv->getfName():"no Avv found"
                     , avl?avl->getfName():"no Avl found"
                     );
    //         }
    // first case no amap setup but we have an av "/components/bms_controls:start_connection"
            if(avv && !avl)
            {
                am->amap[xx.first.c_str()] = avv;
            }
    // second case no amap setup no av "/components/bms_controls:start_connection" may have a default value
    //
            else if(!avv && !avl)
            {
                if (av->gotParam("default"))
                {
                    // new function to set a value from a param
                    avv = vm->setVal(vmap, vname, nullptr, av, "default");
                }
                else
                {
                    double dval = 0.0;
                    avv = vm->setVal(vmap, vname, nullptr, dval);
                }
                am->amap[xx.first.c_str()] = avv;

            }
            else if(avl && avv)
            {
                vmap[avv->comp][avv->name] = avl;
            }
            // this may loose a value
            if(lname)
            {
                assetUri my(lname);
                vmap[my.Uri][my.Var] = avl;

            }
    //             // Set the link.
    //             vmap[aav->comp][aav->name] = avl;
    //             //avv->linkVar = avl;
    //         }
            idx++;
        }
    }
    return 0;
}
///////////////// work summary
// zap the old one:
// vm->zapAlist(varsmap& vmap, const char* uri)
// assetList* alist = vm.setAlist(vmap, my.Uri);
// for each option
// alist->add(av)

// test
// sh scripts/demo/0924/demo2_config_file_loading.sh
//fims_send -m set -r /$$ -u /ess/naked/system/commands/allalists  123| jq
//fims_send -m get -r /$$ -u /ess/naked/assets/bms| jq

int RunAllALists(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "allalists", nullptr);
    FPS_PRINT_INFO(" func running ");

    VarMapUtils *vm = aV->am->vm;
    //std::vector<std::string>linkVec;
    std::string vkey = "/assetList";
    if (vmap.find(vkey)!= vmap.end())
    {
        auto al = vmap[vkey];
        int idx = 0;
        for (auto xx : al)
        {
            FPS_PRINT_INFO(" found [{}] ", xx.first);
            assetList* alist = nullptr;
            assetVar* aLv = nullptr;
            //vm->getVar(vmap, "_assetList", xx.first.c_str());//vm->setAlist(vmap, xx.first.c_str());
            assetVar* av = xx.second;
            char* name = (char*)"No Av";
            char* mode = (char*)"No mode";
            cJSON* cjopts = nullptr;
            if(av && av->gotParam("name"))
            {
                name = av->getcParam("name");
                if(av->extras && av->extras->optVec)
                {
                    bool bval = true;
                    cjopts = av->extras->optVec->cjopts;
                    vm->zapAlist(vmap, xx.first.c_str());
                    alist = vm->setAlist(vmap, xx.first.c_str());
                    av->setVal(bval);
                    aLv = vm->getVar(vmap, "_assetList", xx.first.c_str());
                    if(aLv)
                    {
                        if (name) aLv->setParam("name",name);
                        if (mode) aLv->setParam("mode",mode);
                    }                    
                }
            }
            if(av && av->gotParam("mode"))
            {
                mode = av->getcParam("mode");
            }
            FPS_PRINT_INFO("found idx [{:02d}] >> alist [{}]  name [{}] cjopts [{}]"
                ,idx
                , xx.first
                , name?name:"No Name Param"
                , fmt::ptr(cjopts)
                );
            if(cjopts)
            {
                int idy = 0;
                cJSON *cji;
                assetVar*aaV = nullptr;
                cJSON_ArrayForEach(cji,cjopts)
                {
                    cJSON* cjaV =  cJSON_GetObjectItem(cji, "aV"); 
                    if(cjaV&& cjaV->valuestring)
                    {
                        aaV = vm->getVar(vmap,xx.first.c_str(),cjaV->valuestring);
                        // allow a full uri in here
                        if(!aaV)
                            aaV = vm->getVar(vmap,cjaV->valuestring, nullptr);
                        if(aaV)
                            alist->add(aaV);
                        FPS_PRINT_INFO("found  [{:02d}] >> [{:02d}] xy [{}] aaV [{}]"
                            ,idx
                            , idy
                            , cjaV->valuestring
                            , fmt::ptr(aaV)
                            );
                        idy++;
                    }
                }
                // foreach cjopt in extras->optVec->cjopts
            }
            idx++;
        }
    }
    return 0;
}

int RunAllLinks(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "alllinks", nullptr);
    VarMapUtils *vm = aV->am->vm;
    std::vector<std::string>linkVec;
    std::string vkey = "/links/";
    

    for (auto xx : vmap)
    {
        std::string vmkey = xx.first.substr(0, vkey.length());
        std::string aname = xx.first.substr(vkey.length());
        if(vmkey == vkey)
        {
            linkVec.emplace_back(xx.first);
        }
    }
    FPS_PRINT_INFO("found {} keys",(int)linkVec.size());
    int idx = 0;
    for (auto xx : linkVec)
    {
        // turn /links/pcs  into  aname "pcs"
        std::string vmkey = xx.substr(0, vkey.length());
        std::string aname = xx.substr(vkey.length());
        if(aname.length() == 0)
        {
            FPS_PRINT_ERROR(" skipping  idx [{:02d}]  >> [{}]->[no aname] ", idx, xx);
        }
        else
        {
            FPS_PRINT_INFO(" processing idx [{:02d}]  >> xx->aname :[{}]->[{}] parent [{}]"
                    , idx, xx, aname
                    , aV->am->name
                    );
            // if aname not in the system lets force the issue
            // links/<pcs> already gives us the aname.
            asset_manager* am = vm->getaM(vmap, aname.c_str());
            asset* ai = vm->getaI(vmap, aname.c_str());
            // if(am)
            // {
            //     FPS_PRINT_INFO(" found am [{}]", aname);
            // }
            // if(ai)
            // {
            //     FPS_PRINT_INFO(" found ai [{}]", aname);
            // }
            if (!am && !ai)
            {

                FPS_PRINT_INFO(" no am forcing an am  [{}]", aname);
                FPS_PRINT_INFO(" creating assetManager   [{}]"
                    , aname 
                    );  
                // make its parent aV->am  
                am = new asset_manager(aname.c_str());
                am->p_fims = p_fims;
                am->vm = vm;
                am->setFrom(aV->am);
                vm->setaM(vmap, aname.c_str(), am);
            }
            // run all links in say "pcs"
            RunALink(vmap, amap, vm, aname, xx, aV);
        }
        idx++;
    }
    return 0;
}

int RunAllLinks2(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "alllinks", nullptr);
    VarMapUtils *vm = aV->am->vm;
    std::vector<std::string>linkVec;
    std::string vkey = "/links/";
    

    for (auto xx : vmap)
    {
        std::string vmkey = xx.first.substr(0, vkey.length());
        std::string aname = xx.first.substr(vkey.length());
        if(vmkey == vkey)
        {
            linkVec.emplace_back(xx.first);
        }
    }
    FPS_PRINT_INFO("found {} keys",(int)linkVec.size());
    int idx = 0;
    for (auto xx : linkVec)
    {
        // turn /links/pcs  into  aname "pcs"
        std::string vmkey = xx.substr(0, vkey.length());
        std::string aname = xx.substr(vkey.length());
        if(aname.length() == 0)
        {
            FPS_PRINT_ERROR(" skipping  idx [{:02d}]  >> [{}]->[no aname] ", idx, xx);
        }
        else
        {
            FPS_PRINT_INFO(" processing idx [{:02d}]  >> xx->aname :[{}]->[{}] parent [{}]"
                    , idx, xx, aname
                    , aV->am->name
                    );
            // if aname not in the system lets force the issue
            // links/<pcs> already gives us the aname.
            asset_manager* am = vm->getaM(vmap, aname.c_str());
            asset* ai = vm->getaI(vmap, aname.c_str());
            if(am)
            {
                FPS_PRINT_INFO(" found am [{}]", aname);
            }
            if(ai)
            {
                FPS_PRINT_INFO(" found ai [{}]", aname);
            }
            if (!am && !ai)
            {

                FPS_PRINT_INFO(" no am forcing an am  [{}]", aname);
                FPS_PRINT_INFO(" creating assetManager   [{}]"
                    , aname 
                    );  
                // make its parent aV->am  
                am = new asset_manager(aname.c_str());
                am->p_fims = p_fims;
                am->vm = vm;
                am->setFrom(aV->am);
                vm->setaM(vmap, aname.c_str(), am);
            }
            // run all links in say "pcs"
            RunALink2(vmap, amap, vm, aname, xx/*, ltype*/);
        }
        idx++;
    }
    return 0;
}

// scan config/load for the master
// then read all the load options

int RunSysVec(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "sysvec", nullptr);
    VarMapUtils *vm = aV->am->vm;
    assetVar* avm = nullptr;
    auto xx =  vmap.find("/config/load");
    if (xx == vmap.end())
    {
        FPS_PRINT_INFO(" Unable to find [config/load]");
        return 0;
    }
    auto cl = vmap["/config/load"];
    int idx = 0;
    for (const auto& xy: cl)
    {
        assetVar* av = xy.second;
        FPS_PRINT_INFO(" idx [{:02d}] name [{}]"
            , idx
            , xy.first
            );
        if (av->gotParam("type") && (strcmp(av->getcParam("type"),"master")==0))
        {
            avm = av;
            break;
        }

        idx += 1;
    }
    // if we have found a "master" then walk through the "load" options 
    // and look for the summary entry and all others
    //  push them all onto vm->syscVec
    if(avm)
    {
        FPS_PRINT_INFO(" idx [{:02d}] avm-> [{}]"
            , idx
            , avm->getfName()
            );
        if(avm->extras && avm->extras->optVec && avm->extras->optVec->cjopts)
        {
            cJSON* cji;
            cJSON_ArrayForEach(cji,avm->extras->optVec->cjopts)
            {
                cJSON* cjsum =  cJSON_GetObjectItem(cji, "summary");
                cJSON* cjsvec =  cJSON_GetObjectItem(cji, "svec");
                if(cjsum && cjsum->valuestring)
                {
                    char* sumsp = (char *)cjsum->valuestring;
                    FPS_PRINT_INFO("  summary-> [{}]"
                        , sumsp
                    );
                    vm->syscVec->push_back(strdup(sumsp));
                    if(cjsvec && cjsvec->valuestring)
                    {
                        char* svec = (char *)cjsvec->valuestring;
                        auto ssv = fmt::format("/assets/{}", svec);
                        for (auto xz : vmap)
                        {
                            if (strncmp(xz.first.c_str(),ssv.c_str(), strlen(ssv.c_str()))==0)
                            {
                                char*svecp = (char*)&xz.first.c_str()[strlen("/assets/")];
                                vm->syscVec->push_back(strdup(svecp));
                            }

                        }
                    }

                }
            }
        }
    }
    return 0;
}
//echo setup LoadConfig command
// fims_send -m set -r /$$ -u /flex/system/commands '
//          {"SetMapping":{"value":"/status/bms/rack_01:Current",
//                          "help": "Set up the amname, ainame, pname and amap for a varable ",
//                     "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"SetMapping"}]}]}}}'
// TODO add amap etc
// Clear out old values
int SetMapfromAv(varsmap &vmap,  VarMapUtils*vm, assetVar* aV, asset_manager**aP, asset_manager**aM, asset**aI )
{
    char* ainame =  nullptr; //aV->getcParam("ainame");
    char* amname =  nullptr; //aV->getcParam("amname");
    char* apname =  nullptr; //aV->getcParam("pname");

    asset_manager* ap = nullptr;
    asset_manager* am = nullptr;
    asset* ai = nullptr;
    
    // allow a different asset_manager
    if(aV->gotParam("amname"))
    {
        amname =  aV->getcParam("amname");
        am = vm->getaM(vmap,amname);
        if(!am)
        {
            FPS_ERROR_FMT("{} >> creating assetManager   [{}] \n"
                , __func__
                , amname
                );
            am = new asset_manager(amname);
            am->p_fims = vm->p_fims;
            am->vm = vm;
            vm->setaM(vmap, amname, am);
        }
    }
    else if(aV->gotParam("ainame"))
    {
        ainame =  aV->getcParam("ainame");
        ai = vm->getaI(vmap,ainame);
        if(!ai)
        {
            FPS_ERROR_FMT("{} >> creating assetInstance   [{}] \n"
                , __func__
                , ainame
                );
                ai = new asset(ainame);
                ai->p_fims = vm->p_fims;
                ai->vm = vm;
                //ai->setFrom(aV->am);
                vm->setaI(vmap, ainame, ai);
        }
    }

    if(aV->gotParam("pname"))
    {
        apname =  aV->getcParam("pname");
        ap = vm->getaM(vmap, apname);
        if(!ap)
        {
            FPS_ERROR_FMT("{} >> creating parent asset Manager   [{}] \n"
                , __func__
                , apname
                );
                ap = new asset_manager(apname);
                ap->p_fims = vm->p_fims;
                ap->vm = vm;
                if(am)am->setFrom(ap);
                //if(ai)ai->setFrom(ap);
                vm->setaM(vmap, apname, ap);
        }
        if(am)
        {
            am->am = ap;
            ap->addManAsset(am, apname);
        }
        if(ai)
        {
            ai->am = ap;
            ap->mapInstance(ai, ainame);
            ai->vm = vm;
            //asset* asset_manager::addInstance(const char* _name)
            //ap->addAsset(ai, apname);
        }
                
    }

    if(aM)*aM = am;
    if(aP)*aP = ap;
    if(aI)*aI = ai;
    return 0;
}

int SetMapping(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    asset_manager*avm = aV->am;
    VarMapUtils* vm = avm->vm;
    FPS_PRINT_INFO("running  [{}]",__func__);
    if(!checkAv(vmap, amap, aname,p_fims,aV))
    {
        FPS_ERROR_FMT("{} >> ERROR unable to continue aname [{}] \n", __func__, aname);
        return -1;
    }
    FPS_ERROR_FMT("{} >> OK able to continue aname [{}] \n", __func__, aname);
    char* varname = aV->getcVal();
    if (!varname)
    {
        FPS_ERROR_FMT("{} >> OK Unable able to continue no Varname  \n", __func__);
        return 0;
    }
    assetUri my(varname);
    assetVar* av = vm->getVar(vmap, my.Uri, my.Var);
    if (!av)
    {
        FPS_ERROR_FMT("{} >> OK Unable able to continue no Var defined  \n", __func__);
        return 0;
    }
    asset_manager* am = nullptr;
    asset_manager* ap = nullptr;
    asset* ai = nullptr;
    return SetMapfromAv(vmap, vm, aV, &ap, &am, &ai);
}

// this is for the site interface modbus client interface 
bool loadClientMap(varsmap& vmap, VarMapUtils*vm, const char* cfgname)//cJSON* cjss)
{
    bool ret = false;
    assetVar*av;
    cJSON* cjss = nullptr;
    
    cfgname = vm->getFileName(cfgname);
    if(cfgname)
    {
        cjss = vm->get_cjson(cfgname);

        FPS_ERROR_FMT("{} >> run client config for  file [{}] cj {}\n"
            , __func__
            , cfgname
            , static_cast<void*>(cjss)                    
            );
        free((void*)cfgname);

        if(!cjss)
        {
            return ret;
        }
    }
    cJSON* cjregs = cJSON_GetObjectItem(cjss, "components");
    char* comp = NULL;
    std::string cname;
    std::string vname;
    int ival = 0;
    if(cjregs)
    {
        if(1)FPS_PRINT_INFO("found components");
        cJSON* cjcomps = cjregs->child;
        while(cjcomps)
        { 
            //FPS_PRINT_INFO(" cjregs->child {} ->child {}", fmt::ptr(cji), fmt::ptr(cji->child));
            cJSON* cjmid = cJSON_GetObjectItem(cjcomps, "id");
            if(cjmid)
            {
                if(0)FPS_PRINT_INFO("found reg id [{}]", cjmid->valuestring);
                comp = cjmid->valuestring;
                cname = fmt::format("/components/{}",comp);
            }
            else
            {
                cjcomps = cjcomps->next;
                continue;
            }
            // cji = cjcomps->child;
            // while (cji)
            // {
            //     FPS_PRINT_INFO("found component item [{}]", cji->string);
            //     cji = cji->next;
            // }
            cJSON* cjreg = cJSON_GetObjectItem(cjcomps, "registers");
            // cJSON* cjhold = cJSON_GetObjectItem(cjregs, "holding_registers");
            if(cjreg)
            {
                //ret = true;
                if(0)FPS_PRINT_INFO("found registers type {} child {} next {}"
                    , cjreg->type
                    , fmt::ptr(cjreg->child)
                    , fmt::ptr(cjreg->next)
                    );
                //cji = cjreg->child->child;
                //found register item [type]
                //found register item [starting_offset]
                //found register item [number_of_registers]
                //found register item [note]
                //found register item [map]

                // while (cji)
                // {
                //     FPS_PRINT_INFO("found register item [{}]", cji->string);
                //     cji = cji->next;
                // }
                cJSON* cjmap = cJSON_GetObjectItem(cjreg->child, "map");
                if(cjmap)
                {
                    //ret = true;
                    if(0)FPS_PRINT_INFO("found map type {} child type {} child next{}"
                        , cjmap->type, cjmap->child->type, fmt::ptr(cjmap->child->next));
                    cJSON* cjim = cjmap->child;
                    while (cjim)
                    {
                        cJSON* cjmid = cJSON_GetObjectItem(cjim, "id");
                        if(cjmid)
                        {
                            if(0)FPS_PRINT_INFO("found map id [{}]", cjmid->valuestring);
                            vname = fmt::format("{}", cjmid->valuestring);
                            av = vm->setVal(vmap, cname.c_str(), vname.c_str(), ival);

                            cJSON* cjmnam = cJSON_GetObjectItem(cjim, "name");
                            if(cjmnam)
                            {
                                av->setParam("name",cjmnam->valuestring);
                                if(0)FPS_PRINT_INFO("found map name [{}]", cjmnam->valuestring);
                            }
                            cJSON* cjoff = cJSON_GetObjectItem(cjim, "offset");
                            if(cjoff)
                            {
                                av->setParam("offset",cjoff->valuedouble);
                            }
                            cJSON* cjsca = cJSON_GetObjectItem(cjim, "scale");
                            if(cjsca)
                            {
                                av->setParam("scale",cjsca->valuedouble);
                            }

                            cJSON* cjsin = cJSON_GetObjectItem(cjim, "signed");
                            if(cjsin)
                            {
                                bool bval = cJSON_IsTrue(cjsin);
                                av->setParam("signed",bval);
                            }
                        }
                        //cJSON* cjimi = cjim->child;
                        // while (cjimi)
                        // {
                        //     FPS_PRINT_INFO("found map item [{}]", cjimi->string);
                        //     cjimi = cjimi->next;
                        // }
                        cjim = cjim->next;
                    }
                }
            }
            cjcomps=cjcomps->next;
        }
    }
    if(cjss)cJSON_Delete(cjss);
    return ret;
}

bool loadSiteMapCj(varsmap& vmap, VarMapUtils*vm, cJSON* cjss);//cJSON* cjss)

int loadSiteMapAv(varsmap &vmap, VarMapUtils *vm,  assetVar* aV)
{
    if(aV->gotParam("body"))
    {
        const char* body = aV->getcParam("body");
        cJSON*cjss = cJSON_Parse(body);
        loadSiteMapCj(vmap, vm, cjss);//cJSON* cjss)
    }
    return 0;
}

// this is for the site interface modbus server
bool loadSiteMap(varsmap& vmap, VarMapUtils*vm, const char* cfgname)//cJSON* cjss)
{
    bool ret = false;
    //assetVar*av;
    cJSON* cjss = nullptr;
    
    cfgname = vm->getFileName(cfgname);
    if(cfgname)
    {
        cjss = vm->get_cjson(cfgname);

        FPS_ERROR_FMT("{} >> run client config for file [{}] cj {}\n"
            , __func__
            , cfgname
            , static_cast<void*>(cjss)                    
            );
        free((void*)cfgname);

        if(!cjss)
        {
            return ret;
        }
        ret = loadSiteMapCj(vmap, vm, cjss);
    }
    return ret;
}

bool loadSiteMapCj(varsmap& vmap, VarMapUtils*vm, cJSON* cjss)//cJSON* cjss)
{
    bool ret = false;
    cJSON* cji;
    cJSON* cjid;
    cJSON* cjuri;
    cJSON* cjregs = cJSON_GetObjectItem(cjss, "registers");
    if(cjregs)
    {
        FPS_PRINT_INFO("found registers");
        cji = cjregs->child;
        while (cji)
        {
            FPS_PRINT_INFO("found register item [{}]", cji->string);
            cji = cji->next;
        }
        cJSON* cjinput = cJSON_GetObjectItem(cjregs, "input_registers");
        cJSON* cjhold = cJSON_GetObjectItem(cjregs, "holding_registers");
        if(cjinput)
        {
            ret = true;
            if(0)FPS_PRINT_INFO("found input registers type {} child {}", cjinput->type, fmt::ptr(cjinput->child));
            cji = cjinput->child;
            while (cji)
            {
                //char*tmp = nullptr;
                //tmp = cJSON_PrintUnformatted(cji);
                //FPS_PRINT_INFO("%s >> found input register [{}]\n", tmp);
                cjid = cJSON_GetObjectItem(cji, "id");
                cjuri = cJSON_GetObjectItem(cji, "uri");
                if(cjid && cjuri && cjid->valuestring&&cjuri->valuestring)
                {
                    if(0) FPS_PRINT_INFO("found input register id [{}] uri [{}]", cjid->valuestring, cjuri->valuestring);
                    int ival = 0;
                    vm->setVal(vmap, cjuri->valuestring, cjid->valuestring, ival);
                }
                //if(tmp)free(tmp);
                cji = cji->next;
            }
        }
        if(cjhold)
        {
            ret = true;
            if(0) FPS_PRINT_INFO("found holding registers type {} child {}", cjinput->type, fmt::ptr(cjinput->child));
            cji = cjhold->child;
            while (cji)
            {
                //char*tmp = nullptr;
                //tmp = cJSON_PrintUnformatted(cji);
                //FPS_PRINT_INFO("%s >> found input register [{}]\n", tmp);
                cjid = cJSON_GetObjectItem(cji, "id");
                cjuri = cJSON_GetObjectItem(cji, "uri");
                if(cjid && cjuri && cjid->valuestring&&cjuri->valuestring)
                {
                    if (1)FPS_PRINT_INFO("found holding register id [{}] uri [{}]", cjid->valuestring, cjuri->valuestring);
                    int ival = 0;
                    vm->setVal(vmap, cjuri->valuestring, cjid->valuestring, ival);
                }
                //if(tmp)free(tmp);
                cji = cji->next;
            }
        }
    }
    if(cjss)cJSON_Delete(cjss);
    return ret;
}

int RunTpl(varsmap &vmap, varmap &amap, const char* xaname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "tpl", nullptr);

    char* aname = nullptr;
    char* pname = nullptr;
    char* fname = nullptr;
    if(aV->gotParam("aname"))
    {
        aname = aV->getcParam("aname");
    }
    if(aV->gotParam("pname"))
    {
        pname = aV->getcParam("pname");
    }
    if(aV->gotParam("fname"))
    {
        fname = aV->getcParam("fname");
    }
    FPS_ERROR_FMT("{} >> setting up manager [{}->{}] filename [%{}]\n"
        ,__func__, pname, aname, fname);

    // first find parent
    if (pname == nullptr)
    {
        pname = (char*)"flex";
    }
    if(!pname |!aname|!fname)
    {
        FPS_ERROR_FMT(" {} >> pname aname or fname missing\n", __func__);
        return 0;
    }
    asset_manager* pam = aV->am->vm->getaM(vmap, pname);
    if(!pam)
    {
        pam = new asset_manager(pname);
    }
    if(!pam )
    {
        FPS_ERROR_FMT(" {} >> unable to create pam [{}]\n", __func__, static_cast<void*>(pam));
        return 0;
    }

    asset_manager* am = aV->am->vm->getaM(vmap, aname);
    if(!am)
    {
        am = new asset_manager(aname);
    }
                
    am->p_fims = p_fims;
    am->wakeChan = pam->wakeChan;
    am->reqChan = (void*)pam->reqChan;
                       
    am->setFrom(pam);
    //TODO if (1)FPS_ERROR_PRINT(" %s >> syscVec size [%d]\n", __func__, (int)syscpVec->size());
                    // now get the asset_manager to configure itsself
                    // syscpVec needs to be solved
    //am->vm->syscVec = &am->vm->syscharVec;
    if(am->configure(&vmap, fname, aname, am->vm->syscVec/*nullptr*//*syscpVec*/, nullptr, am) < 0)
    {
        FPS_ERROR_FMT(" {} >> error in [{}] config file [{}]\n", __func__, aname, fname);
        exit(0);
    }
    //int idx = 0;
    // for ( auto &x: am->vm->sysVec)
    // {
    //     FPS_ERROR_PRINT("%s >> syscpVec [%d] [%s]\n", __func__, idx++, x);
    // }

    //if (iy.first == "pcs") PCSInit(vmap, Av->am->amap, aname, p_fims, Av);

    // TODO add Init func
    //int ccntam = 0;
    //FPS_ERROR_PRINT("%s >> setting up a %s manager pubs found %d \n", __func__, aname, ccntam);

    // add it to the assetList
    pam->addManAsset(am, aname);
    FPS_ERROR_FMT("{} >> done setting up a {} manager \n",__func__,  aname);
    am->running = 1;
    return 0;
}
// fims_send -m set -r /$$ -u /flex/full/system/commands/loadServer '
//              {"value":"testagain","server":"bms_modbus_server.json"}'
// fims_send -m set -r /$$ -u /flex/full/system/commands/loadCfg '
//              {"value":"someval","aname":"flex", "config":"flex_assets.json"}'
// fims_send -m set -r /$$ -u /flex/full/system/commands/loadCfg '
//              {"value":"someval","aname":"bms", "pname":essName, "config":"bms_manager.json"}'

//echo setup LoadConfig command
// fims_send -m set -r /$$ -u /flex/system/commands '
//          {"loadCfg":{"value":"test",
//                     "help": "load a config file",
//                     "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadConfig"}]}]}}}'

int LoadConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(!checkAv(vmap,amap, aname,p_fims,aV))
    {
        return -1;
    }
    
    asset_manager* am = aV->am;
    asset_manager* ap = nullptr;
    VarMapUtils* vm = am->vm;
    // allow a different asset_manager
    if(aV->gotParam("aname"))
    {
        char* aname =  aV->getcParam("aname");
        am = vm->getaM(vmap,aname);
        if(!am)
        {
            FPS_ERROR_FMT("{} >> creating assetManager   [{}] \n"
                , __func__
                , aname
                );
                am = new asset_manager(aname);
                am->p_fims = p_fims;
                am->vm = vm;
                am->setFrom(aV->am);
                vm->setaM(vmap, aname, am);


        }
    }

    if(aV->gotParam("pname"))
    {
        char* aname =  aV->getcParam("pname");
        ap = vm->getaM(vmap, aname);
        if(!ap)
        {
            FPS_ERROR_FMT("{} >> creating parent asset Manager   [{}] \n"
                , __func__
                , aname
                );
                ap = new asset_manager(aname);
                ap->p_fims = p_fims;
                ap->vm = vm;
                am->setFrom(ap);
                vm->setaM(vmap, aname, ap);


        }
        am->am = ap;
        ap->addManAsset(am, aname);
    }

    if(aV->gotParam("config"))
    {
        char* cfgname = am->vm->getFileName(aV->getcParam("config"));
        if(cfgname)
        {
            FPS_ERROR_FMT("{} >> run config for  [{}]  file [{}]\n"
                , __func__
                , aV->getfName()
                , cfgname
                );
            // no reps here
            am->vm->configure_vmap(vmap, cfgname, nullptr, am);
            free(cfgname); // LEAK fixed
        }
    }
    return 0;
}


int DumpConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(!checkAv(vmap, amap, aname,p_fims,aV))
    {
        return -1;
    }
    char* outFile = nullptr;
    char* table = nullptr;

    int vmlen  = 0;
    asset_manager* am = aV->am;
    if(aV->gotParam("table"))
    {
        //table = am->vm->getFileName(aV->getcParam("table"));
        table = aV->getcParam("table");
    }
    if(aV->gotParam("outFile"))
    {
        outFile = aV->getcParam("outFile");

        auto fname = fmt::format("{}{}_{}_dump.json"
                , am->vm->runCfgDir?am->vm->runCfgDir:"run_configs"
                , am->vm->getSysName(vmap)
                , outFile
                );

        // vmlen = asprintf(&fname,"%s%s_%s_dump.json"
        //     , am->vm->runCfgDir?am->vm->runCfgDir:"run_configs"
        //     , am->vm->uriroot?am->vm->uriroot:"flex"
        //     , outFile
        //     );
        // this a test for our config with links
        cJSON* cjbm = am->vm->getMapsCj(vmap, table, nullptr, 0x10000);
        if(cjbm)
        {
            am->vm->write_cjson(fname.c_str(), cjbm);
            cJSON_Delete(cjbm);
        }
        //free(fname);
 
        FPS_ERROR_FMT("{} >> dump config  av [{}] table [{}]  file [{}] len [{}]\n"
            , __func__
            , aV->getfName()
            , table?table:"none"
            , outFile
            , vmlen
            );
    }
    return 0;
}

// int LoadClient(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
// {

//     FPS_ERROR_PRINT("%s >> run client for  [%s]\n"
//         , __func__
//         , aV->getfName()
//     );

//     if(!checkAv(vmap,amap, aname,p_fims,aV))
//     {
//         return -1;
//     }

//     asset_manager* am = aV->am;
//     if(aV->gotParam("client"))
//     {
//         char* cfgname = am->vm->getFileName(aV->getcParam("client"));
//         //cJSON* cj = am->vm->get_cjson(cfgname);

//         FPS_ERROR_PRINT("%s >> run site config for  [%s]  file [%s] cfgname [%s]\n"
//             , __func__
//             , aV->getfName()
//             , aV->getcParam("client")
//             , cfgname
//             );

//         // this is for the site interface
//         am->vm->loadClientMap(vmap, cfgname);
//         free(cfgname); // LEAK
//     }
//     return 0;
// }

int LoadServer(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    FPS_ERROR_FMT("{} >> running \n", __func__);
    if(!checkAv(vmap, amap, aname,p_fims, aV))
    {
        return -1;
    }

    asset_manager* am = aV->am;
    VarMapUtils *vm = am->vm;
    char* cfgname = aV->getcVal();
    FPS_ERROR_FMT("{} >> running cfgname [{}]\n", __func__, cfgname?cfgname:"no cfgname");

    if(aV->gotParam("server"))
    {
        cfgname = aV->getcParam("server");
    }
    if(cfgname)
    {
        auto avs = vm->getVar(vmap, cfgname,nullptr);
        if(avs)
        {
            loadSiteMapAv(vmap, vm, avs);
        }
        else
        {
            loadSiteMap(vmap, vm, cfgname);
        }
    }
    return 0;
}

int LoadClient(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(!checkAv(vmap,amap, aname,p_fims,aV))
    {
        return -1;
    }

    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    char* cfgname = aV->getcVal();
    if(aV->gotParam("client"))
    {
        cfgname = aV->getcParam("client");
    }
    if(cfgname)
    {
        loadClientMap(vmap, vm, cfgname);
    }
    return 0;
}

// we can run vlinks for  a specific aname or for the whole thing.
// with FlexPack the vlinks are run on demand.
// we trigger the demand atfer loading any configs.
//  how do we specfy where .. worry about that later for now do the whole lot
int RunVLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "vlinks", nullptr);
    bool mkfrom = true;//false;
    bool mkto = true;//false;
    
    // if(aV->gotParam("from"))
    // {
    //     mkfrom = aV->getbParam("from");
    //     aV->setParam("from", false);
    // }

    // if(aV->gotParam("to"))
    // {
    //     mkto = aV->getbParam("to");
    //     aV->setParam("to", false);
    // }
    char* flexName = (char *) aV->am->vm->getSysName(vmap);
    FPS_ERROR_FMT("{} >> {} >>Setting vlinks\n", __func__, flexName);
    aV->am->vm->setVLinks(vmap, flexName, mkfrom, mkto);
    FPS_ERROR_FMT("{} >> {} >>Done Setting vlinks\n",__func__, flexName);
    return 0;
}
// "/links/rack_6":{
//     "Year": {
//           "value": "/components/catl_rack_6_ems_bms_rw:ems_rtc_year",
//           "linkvar": "/status/rack_6:year",
//           "defval": 0,
//           "pname": "bms",
//         }
//    }
int RunLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "links", nullptr);
    char* flexName = (char *) aV->am->vm->getSysName(vmap);
    if(aV->gotParam("amap"))
    {
        flexName = aV->getcParam("amap");
    }

    FPS_ERROR_FMT("{} >> Flex >>Setting links for [{}] \n", __func__, flexName);
    aV->am->vm->setLinks(vmap, flexName);
    FPS_ERROR_FMT("{} >> Flex >>Done Setting links\n",__func__);
    if(aV->gotParam("amap"))
    {
        aV->setParam("amap", (char*)" ");
    }
    return 0;
}
// "/links/rack_6":{
//     "Year": {
//           "value": "/components/catl_rack_6_ems_bms_rw:ems_rtc_year",
//           "linkvar": "/status/rack_6:year",
//           "defval": 0,
//           "pname": "bms",
//         }
//    }
//linkVals(*vm, vmap, amap, aname, "/controls", dval, "ActivePowerSetpoint", "ReactivePowerSetpoint");
//amap["essPcsStatusFaults"] = vm->setLinkVal(vmap, essName, "/status", "essPcsStatusFaults", ival);
//    av = makeVar(vmap, comp, var, defvalue);
//    "var":"/debug/bms:test1"
//    "linkName":"/status/bms:MaxValue"
//    "vlinkName":"/status/bms:MaxValue"
// makes a var /debug/bms:test1 ...vmap["/debug/bms"]["test1"] = getVar(vmap, "/status:bms:MaxValue");
// TODO do not create extra AV
int MakeLink(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "makeLink", nullptr);
    VarMapUtils *vm = aV->am->vm;
    asset_manager* am = aV->am;
    asset* ai = nullptr;
    assetVar* aVar = nullptr;
    assetVar* lVar = nullptr;

    char* linkName = nullptr; //linkName
    char* varName = nullptr;  //varName
    char* amName = nullptr;   //amName
    char* aiName = nullptr;   //aiName
    cJSON* cjVal = nullptr;   //inValue
    bool vlink = false;

    if(aV->gotParam("inValue"))
    {
        cjVal = aV->getCjParam("inValue");
    }

    if(aV->gotParam("aiName"))
    {
        aiName = aV->getcParam("aiName");
    }

    if(aV->gotParam("amName"))
    {
        amName = aV->getcParam("amName");
    }

    // this is the dummy link
    if(aV->gotParam("vlinkName"))
    {
        linkName = aV->getcParam("vlinkName");
        vlink = true;
    }

    if(aV->gotParam("varName"))
    {
        varName = aV->getcParam("varName");
        if(vlink)
        {
            aVar = vm->getVar(vmap, varName, nullptr);
            if(!aVar && cjVal)
            { 
                //aVar = vm->makeVar(vmap, varName, nullptr);
                assetUri my(varName, nullptr);
                aVar = vm->setValfromCj(vmap, my.Uri, my.Var, cjVal);
            }
        }
    }
    if (!varName)
    {
        FPS_ERROR_FMT("{} >>  ERROR no varName specified for [{}] \n"
                     ,__func__, aV->getfName());  
        return 0;
    }

    assetUri myv(varName, nullptr);
    
    if(aV->gotParam("linkName"))
    {
        linkName = aV->getcParam("linkName");
    }

    
    if(linkName)
    {
        lVar = vm->getVar(vmap, linkName, nullptr);
        if(!lVar && cjVal)
        {
            //lVar = vm->makeVar(vmap, linkName, nullptr);  // Note makeVar did not work.....
            assetUri my(linkName, nullptr);
            //double dval = 0.1;
            lVar = vm->setValfromCj(vmap, my.Uri, my.Var, cjVal);
            // FPS_ERROR_FMT("{} >> Flex >>    lVar uri [{}] : var [{}] \n"
            //     ,__func__, my.Uri, my.Var);
            // FPS_ERROR_FMT("{} >> Flex >>    lVar [{}] : [{}] \n"
            //     ,__func__, lVar->comp.c_str(), lVar->name.c_str());
        }
    }

    if (lVar)
    {
        //   FPS_ERROR_FMT("{} >> Flex >>    Setting link for [{}] to [{}] \n", __func__, varName, linkName);
        //   FPS_ERROR_FMT("{} >> Flex >>    aVar [{}] : [{}] \n"
        //         ,__func__, aVar->comp.c_str(), aVar->name.c_str());
        //   FPS_ERROR_FMT("{} >> Flex >>    lVar [{}] : [{}] \n"
        //         ,__func__, lVar->comp.c_str(), lVar->name.c_str());
        if(vlink && aVar)
        {
            aVar->linkVar = lVar; 
        }
        else
        {
            vmap[myv.Uri][myv.Var] = lVar;
        }
    }
    else
    {
        return 0;
    }

    if(amName) am = vm->getaM(vmap, amName);
    if(aiName) ai = vm->getaI(vmap, aiName);

    if(!am && !ai)
    {
        am = aV->am;
    }
    if(am)
    {
        //if (!aVar->am) aVar->am = am;
        if (!lVar->am) lVar->am = am;
        // if(ai)
        // {
        //     ai->amap[varName] = vm->setLinkVal(vmap, ai->name.c_str(), sysName, varName, cjVal);
        // }

    }
    FPS_ERROR_FMT("{} >> Flex >>Setting link for [{}] to [{}] \n", __func__, varName, linkName);
    // //aV->am->vm->setLinkVal(vmap, flexName, baseName, varName, cjVal);
    // FPS_ERROR_FMT("{} >> Flex >>Done Setting link\n",__func__);
    // if(aV->gotParam("amap"))
    // {
    //     aV->setParam("amap", (char*)" ");
    // }
    return 0;
}



int StopSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    asset_manager* am = aV->am;
    //int debug = 1;

    if(!checkAv(vmap,amap, aname,p_fims,aV))
    {
        return -1;
    }
    if (!aV->gotParam("uri")|| (strcmp(aV->getcParam("uri"),"none") ==0))
    {
        FPS_ERROR_FMT("{} >>  [{}] missing uri\n"
            , __func__
            , aV->getfName()
        );
        return 0;
    }
    char* uri = aV->getcParam("uri");
     

    double tNow = am->vm->get_time_dbl();

    assetVar* av = am->vm->setVal(*am->vmap, uri, nullptr, tNow);
    if(!av)
    {
        FPS_ERROR_FMT("{} >> error setVal aV [{}] uri[{}]\n"
            , __func__
            , aV->getfName()
            , uri
        );
        return 0;
    }
    av->am = am;
    FPS_ERROR_FMT("{} >> running with av [()] uri [{}]\n"
        , __func__
        , av->getfName()
        , uri
    );
    av->setVal(av->name.c_str());   //schedId
    av->setParam("uri",uri);
    av->setParam("fcn",(char*)"RunTarg");
    av->setParam("targ",uri);
    av->setParam("amap",aname);
   
    av->setParam("reftime",aV->getdParam("offset"));
    double runTime = aV->getdParam("in")+ tNow;
   
    av->setParam("endTime", runTime); 
    av->setParam("debug", aV->getiParam("debug")); 
//    double runFor   = av->getdParam("runFor");
//    double endTime  = av->getdParam("endTime");
//    int debug       = av->getiParam("debug");
    return 0;

}

// needs uri, offset, after , every , for,  debug all packed into a dummy aV
int RunSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(0)FPS_ERROR_FMT("{} >> running  aV [{}] aname [{}] aV->am {}\n"
            , __func__
            , aV->getfName()
            , aname  
            , fmt::ptr(aV->am)
        );
    asset_manager* am = aV->am;
    int debug = 1;

    if(!aname)
    {
        aname = "ess";
    }
    if(!checkAv(vmap, amap, aname, p_fims, aV))
    {
        return -1;
    }
    if(0)FPS_ERROR_FMT("{} >> ##2 running  aV [{}] aname [{}]\n"
            , __func__
            , aV->getfName()
            , aname 
        );

    if (!aV->gotParam("uri")|| (strcmp(aV->getcParam("uri"),"none") ==0))
    {
        FPS_ERROR_FMT("{} >>  [{}] missing uri\n"
            , __func__
            , aV->getfName()
        );
        return 0;
    }
    char* uri = aV->getcParam("uri");
    if(0)FPS_ERROR_FMT("{} >> ##2a running  aV [{}] aname [{}] uri [{}]  am [{}]\n"
            , __func__
            , aV->getfName()
            , aname 
            , uri
            , fmt::ptr(aV->am)
        );

    double tNow = aV->am->vm->get_time_dbl();
    if(0)FPS_ERROR_FMT("{} >> ##2b running  aV [{}] aname [{}] uri [{}]  am [{}]\n"
            , __func__
            , aV->getfName()
            , aname 
            , uri
            , fmt::ptr(aV->am->vm)
        );


    assetVar* av = aV->am->vm->setVal(vmap, uri, nullptr, tNow);
    if(0)FPS_ERROR_FMT("{} >> ##3 running  aV [{}] uri [{}]\n"
            , __func__
            , aV->getfName()
            , uri
        );
    if(!av)
    {
        FPS_ERROR_FMT("{} >> error setVal aV [{}] uri[{}]\n"
            , __func__
            , aV->getfName()
            , uri
        );
        return 0;
    }
    if(!uri)
    {
        FPS_ERROR_FMT("{} >> error setVal aV [{}] no uri\n"
            , __func__
            , aV->getfName()
        );
        return 0;
    }
    av->am = am;
    if(0)FPS_ERROR_FMT("{} >> running with av [{}] uri [{}]\n"
        , __func__
        , av->getfName()
        , uri
        );
    av->setVal(av->name.c_str());   //schedId
    av->setParam("uri",uri);
    av->setParam("fcn",(char*)"RunTarg");
    av->setParam("targ",uri);
    av->setParam("amap",aname);
    av->setParam("reftime",aV->getdParam("offset"));
    double runTime = aV->getdParam("in")+ tNow;
    av->setParam("runTime",runTime);
    if(aV->getdParam("every")> 0) 
    {
        av->setParam("repTime", aV->getdParam("every"));
    }
    if(av->gotParam("after"))
    {
        runTime = aV->getdParam("after") + tNow;
        av->setParam("runAfter", runTime); 
    }
    if(aV->getdParam("for") > 0) 
    {
        runTime = aV->getdParam("for") + tNow;
        av->setParam("runFor", runTime);
    }
    else
    {
        runTime = 0.0;
    }

    av->setParam("endTime", runTime); 
    av->setParam("debug", aV->getiParam("debug")); 
    runTime = 0.0;
    debug = 0;
    aV->setParam("for",  runTime);
    aV->setParam("every",runTime);
    aV->setParam("offset",runTime);
    aV->setParam("after",runTime);
    aV->setParam("debug",debug);
    aV->setParam("uri",(char*)"none");
    if(0)FPS_ERROR_FMT("{} >> before HandleSchedLoad with av [{}] uri [{}]\n"
        , __func__
        , av->getfName()
        , uri
        );
    int rc = HandleSchedLoad(vmap, amap, aname, p_fims, av);
    if(0)FPS_ERROR_FMT("{} >> after HandleSchedLoad with av [{}] uri [{}] rc {}\n"
        , __func__
        , av->getfName()
        , uri
        , rc
        );
    return rc;

}

// sets up start / stop controls
void setupControls(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am, fims* p_fims)
{
    char * fimsname= (char*)"/sched/fims:dummy";
    assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);
    double dval = 0.0;

    if(!fimsav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_FMT("{} >> Created Fims var [{}] \n "
            , __func__
            , av->getfName()
        );
        fimsav = av;
    }

    fimsname= (char*)"/control/flex:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!runav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_FMT("{} >> Created Runvar var [{}] \n "
            , __func__
            , av->getfName()
        );
        runav = av;
    }
    fimsname= (char*)"/control/flex:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!stopav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_FMT("{} >> Created Stop var [{}] \n "
            , __func__
            , av->getfName()
        );
        stopav = av;
    }
}

#define LOGDIR "/var/log/flex_controller"

void loadFlexFunc(varsmap & vmap, VarMapUtils*vm , const char* FlexName)
{
    vm->setFunc(vmap, FlexName, "RunVLinks",    (void*)&RunVLinks);
    vm->setFunc(vmap, FlexName, "RunAllVLinks", (void*)&RunAllVLinks);
    vm->setFunc(vmap, FlexName, "RunAllLinks", (void*)&RunAllLinks);
    vm->setFunc(vmap, FlexName, "RunAllLinks2", (void*)&RunAllLinks2);
    vm->setFunc(vmap, FlexName, "RunAllALists", (void*)&RunAllALists);
    vm->setFunc(vmap, FlexName, "SlewVal",      (void*)&SlewVal);
    
    vm->setFunc(vmap, FlexName, "RunSysVec",    (void*)&RunSysVec);
    vm->setFunc(vmap, FlexName, "RunLinks",     (void*)&RunLinks);
    vm->setFunc(vmap, FlexName, "MakeLink",     (void*)&MakeLink);
    vm->setFunc(vmap, FlexName, "RunTpl",       (void*)&RunTpl);
    vm->setFunc(vmap, FlexName, "RunSched",     (void*)&RunSched);
    vm->setFunc(vmap, FlexName, "RunMonitor",   (void*)&RunMonitor);
    vm->setFunc(vmap, FlexName, "StopSched",    (void*)&StopSched);
    vm->setFunc(vmap, FlexName, "RunPub",       (void*)&RunPub);
    vm->setFunc(vmap, FlexName, "SlowPub",      (void*)&SlowPub);
    vm->setFunc(vmap, FlexName, "SendDb",       (void*)&SendDb);
    vm->setFunc(vmap, FlexName, "LoadServer",   (void*)&LoadServer);
    vm->setFunc(vmap, FlexName, "LoadClient",   (void*)&LoadClient);
    //vm->setFunc(vmap, FlexName, "LoadClient",   (void*)&LoadClient);
    vm->setFunc(vmap, FlexName, "LoadConfig",   (void*)&LoadConfig);
    vm->setFunc(vmap, FlexName, "DumpConfig",   (void*)&DumpConfig);
    vm->setFunc(vmap, FlexName, "RunSystemCmd", (void*)&RunSystemCmd);
    vm->setFunc(vmap, FlexName, "process_sys_alarm", (void*)&process_sys_alarm);
    vm->setFunc(vmap, FlexName, "CalculateVar", (void*)&CalculateVar);
    vm->setFunc(vmap, FlexName, "RunSystemCmd", (void*)&RunSystemCmd);
    vm->setFunc(vmap, FlexName, "CheckTableVar",  (void*)&CheckTableVar);
    vm->setFunc(vmap, FlexName, "CheckMonitorVar",(void*)&CheckMonitorVar);
    //vm->setFunc(vmap, FlexName, "RunCell",        (void*)&RunCell);
    vm->setFunc(vmap, FlexName, "MathMovAvg",     (void*)&MathMovAvg);
    //vm->setFunc(vmap, FlexName, "AddSchedItem",   (void*)&AddSchedItem);
    vm->setFunc(vmap, "ess", "HandleSchedItem",(void*)&HandleSchedItem); // let this ess stay
    vm->setFunc(vmap, "flex", "HandleSchedItem",(void*)&HandleSchedItem);
    vm->setFunc(vmap, FlexName, "HandleSchedItem",(void*)&HandleSchedItem);
    vm->setFunc(vmap, FlexName, "HandleSchedLoad",(void*)&HandleSchedLoad);
    vm->setFunc(vmap, FlexName, "SimHandlePcs",   (void*)&SimHandlePcs);
    //vm->setFunc(vmap, FlexName, "SimHandleBms",   (void*)&SimHandleBms);
    vm->setFunc(vmap, FlexName, "FastPub",        (void*)&FastPub);
    vm->setFunc(vmap, FlexName, "LogInfo",              (void*)&LogInfo);
    //vm->setFunc(vmap, FlexName, "SimHandleRizenBms",    (void*)&SimHandleRizenBms);
    vm->setFunc(vmap, FlexName, "UpdateSysTime",        (void*)&UpdateSysTime);
    //vm->setFunc(vmap, FlexName, "SimHandleRackVolts",   (void*)&SimHandleRackVolts);
    //vm->setFunc(vmap, FlexName, "SimHandleCellCurrent",   (void*)&SimHandleCellCurrent);
    //vm->setFunc(vmap, FlexName, "SimProcessCells",        (void*)&SimProcessCells);
    //vm->setFunc(vmap, FlexName, "SimRunCell",             (void*)&SimRunCell);
    vm->setFunc(vmap, FlexName, "HandleCmd",              (void*)&HandleCmd);
    vm->setFunc(vmap, FlexName, "RunMonitorList",         (void*)&RunMonitorList);
    vm->setFunc(vmap, FlexName, "SetMapping",             (void*)&SetMapping);
    //vm->setFunc(vmap, FlexName, "SimBmsManageContactor",   (void*)&SimBmsManageContactor);
    //vm->setFunc(vmap, FlexName, "SimBmsOpenContactor",   (void*)&SimBmsOpenContactor);
    //vm->setFunc(vmap, "bms", "SimBmsStartConnection",     (void*)&SimBmsStartConnection);
    vm->setFunc(vmap, FlexName, "SchedItemOpts",          (void*)&SchedItemOpts);
    vm->setFunc(vmap, FlexName, "EssSystemInit",          (void*)&EssSystemInit);
    vm->setFunc(vmap, FlexName, "Every1000mS",            (void*)&Every1000mS);
    vm->setFunc(vmap, FlexName, "Every100mSP1",            (void*)&Every100mSP1);
    vm->setFunc(vmap, FlexName, "runAllLocks",            (void*)&runAllLocks);

}
    
int LoadFuncs(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    aV->am->vm->setFunc(vmap, "flex", "process_sys_alarm", (void*)&process_sys_alarm);
    aV->am->vm->setFunc(vmap, "flex", "CalculateVar", (void*)&CalculateVar);
    aV->am->vm->setFunc(vmap, "flex", "RunSystemCmd", (void*)&RunSystemCmd);
    aV->am->vm->setFunc(vmap, "flex", "CheckTableVar", (void*)&CheckTableVar);
    return 0;
}

VarMapUtils* getVm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    VarMapUtils* vm = nullptr;
    if(!checkAv(vmap,amap, aname,p_fims,aV))
    {
        const char* sysaV = "/system/default";
        const char* sysVm = "vm";
        if(vmap.find(sysaV) == vmap.end())
        {
            FPS_ERROR_FMT("{} >> Error Unable to retrieve vmap from default [{}]\n"
                , __func__, sysaV);
            return vm;
        }
        if(vmap[sysaV].find(sysVm) == vmap[sysaV].end())
        {
            FPS_ERROR_FMT("{} >> Error Unable to retrieve aV from default [{}:{}]\n"
             , __func__, sysaV, sysVm);
            return vm;
        }
        FPS_ERROR_FMT("{} >> retrieving vm from default [{}:{}]\n",__func__, sysaV, sysVm );
        assetVar* aVsys = vmap[sysaV][sysVm];
        if(!checkAv(vmap, amap, aname,p_fims,aVsys))
        {
            FPS_ERROR_FMT("{} >> Error Unable to retrieve vm from default [{}:{}]\n", __func__, sysaV, sysVm);
            return vm;
        }
        vm = aVsys->am->vm;
    }
    else
    {
        vm = aV->am->vm;
    }
    return vm;
}

int RunMonitor(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    char* amname  = nullptr;
    char* monitor = nullptr;
    
    VarMapUtils* vm = getVm( vmap, amap, aname, p_fims, aV);
    if (aV->gotParam("aname"))
    {
        amname = aV->getcParam("aname");
    }
    if (aV->gotParam("monitor"))
    {
        monitor = aV->getcParam("monitor");
    }
    if(!amname)
    {
        FPS_ERROR_FMT("{} >>  [{}] missing aname value [{}]\n"
            , __func__
            , aV->getfName()
            , aV->getdVal()
        );
        return 0; 
    }

    if(!monitor)
    {
        FPS_ERROR_FMT("{} >>  [{}] missing monitor value [{}]\n"
            , __func__
            , aV->getfName()
            , aV->getdVal()
        );
        return 0;
    }     
    // but this does the work  aname
    asset_manager* am = vm->getaM(vmap, amname);
    // TODO use flexname
    if(0)FPS_ERROR_FMT("{} ##############>>  [{}] running aname [{}] monitor [{}]\n"
            , __func__
            , aV->getfName()
            , amname
            , monitor
        );
    vm->runMonitorList2(vmap, am->amap, amname, p_fims, monitor);

    return 0;
}

