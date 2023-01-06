/*
* this is the file for "released" or completed application functions
* as a function is getting ready to be released put it in here and we'll get working on signing it off
*
*/
#include "asset.h"
#include "assetFunc.h"
//#include "assetFunc.cpp"
#include "math.h"
#include "chrono_utils.hpp"

#include "../funcs/SimHandleHeartbeat.cpp"

static const bool simulation = false;

// /**
//  * Distributes tod & heartbeat from the ess master
//  * this should only run when triggered by the Manager
//  *
//  * Reviewed: 10/21/2020  modified 12/2/2020
//  *
//  * Used in:
//  * Test Script:
//  */
// int HandleAssetHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
// {
//     bool bval = false;
//     int reload;
//     double dval = 0.0;
//     double dvalHB = 1.0;
//     int ival = 0;
//     VarMapUtils* vmp = am->vm;


//     double dvalHBnow = vmp->get_time_dbl();
//     assetVar* HandleAssetHeartBeat = amap["HandleAssetHeartBeat"];
//     if (!HandleAssetHeartBeat || (reload = HandleAssetHeartBeat->getiVal()) == 0)
//     {
//         reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
//     }

//     if (reload < 2)
//     {
//         //reload = 0;
//         amap["HandleAssetHeartBeat"]   = vmp->setLinkVal(vmap, aname, "/reload", "HandleAssetHeartBeat", reload);
//         amap["HeartBeatLast"]          = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatLast", dvalHBnow);
//         amap["HeartBeatInterval"]      = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatInterval", dvalHB);
//         amap["ess_todSec"]             = vmp->setLinkVal(vmap, "ess", "/status", "ess_todSec", ival);
//         amap["ess_todMin"]             = vmp->setLinkVal(vmap, "ess", "/status", "ess_todMin", ival);
//         amap["ess_todHr"]              = vmp->setLinkVal(vmap, "ess", "/status", "ess_todHr", ival);
//         amap["ess_todDay"]             = vmp->setLinkVal(vmap, "ess", "/status", "ess_todDay", ival);

//         amap["OkToSend"]               = vmp->setLinkVal(vmap, aname, "/config", "OkToSend", bval);

//         amap["op_todSec"]              = vmp->setLinkVal(vmap, aname, "/status", "op_todSec", ival);
//         amap["op_todMin"]              = vmp->setLinkVal(vmap, aname, "/status", "op_todMin", ival);
//         amap["op_todHr"]               = vmp->setLinkVal(vmap, aname, "/status", "op_todHr",  ival);
//         amap["op_todDay"]              = vmp->setLinkVal(vmap, aname, "/status", "op_todDay",      ival);
//         amap["op_todMon"]              = vmp->setLinkVal(vmap, aname, "/status", "op_todMon",      ival);
//         amap["op_todYr"]               = vmp->setLinkVal(vmap, aname, "/status", "op_todYr",       ival);
//         amap["op_HB"]                  = vmp->setLinkVal(vmap, aname, "/status", "op_HB",          ival);

//         amap["HeartBeat"]              = vmp->setLinkVal(vmap, aname, "/status", "HeartBeat",     ival);
//         amap["HeartBeatDbl"]           = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatDbl", dval);
//         amap["HeartBeatLink"]          = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatLink", ival);
//         //amap["]->setVal(2);  // revert reload
//         if (reload == 0) // complete restart 
//         {
//             ival = 0; amap["HeartBeat"]->setVal(ival);
//         }
//     }
//     ival = 2; amap["HandleAssetHeartBeat"]->setVal(ival);


//     // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
//     double HBLast = amap["HeartBeatLast"]->getdVal();
//     double HBnow = amap["HeartBeat"]->getdVal();
//     bool OkToSend = amap["OkToSend"]->getbVal();
//     dval = 1.0;
//     // dont use valueChanged it resets the change currently
//     if (HBLast != HBnow)
//     {

//         amap["HeartBeatLast"]->setVal(HBnow);

//         dval = 1.0;
//         // this value is used to trigger the heartbeats for all the assets that need it
//         amap["HeartBeat"]->addVal(dval);
//         amap["op_todSec"]->setVal(amap["ess_todSec"]->getiVal());
//         amap["op_todMin"]->setVal(amap["ess_todMin"]->getiVal());
//         amap["op_todHr"]->setVal(amap["ess_todHr"]->getiVal());
//         amap["op_todDay"]->setVal(amap["ess_todDay"]->getiVal());
//         amap["op_todMon"]->setVal(amap["ess_todMon"]->getiVal());
//         amap["op_todYr"]->setVal(amap["ess_todYr"]->getiVal());

//         // this stuff collects a bunch of assetVars and send them out to their default locations.
//         // the link will determine where that location is.
//         // if the link is defined in the config file then that destination will be maintained.

//         varsmap* vlist = vmp->createVlist();
//         vmp->addVlist(vlist, amap["HeartBeat"]);
//         vmp->addVlist(vlist, amap["op_todSec"]);
//         vmp->addVlist(vlist, amap["op_todMin"]);
//         vmp->addVlist(vlist, amap["op_todHr"]);
//         vmp->addVlist(vlist, amap["op_todDay"]);
//         vmp->addVlist(vlist, amap["op_todYr"]);
//         if(OkToSend)vmp->sendVlist(p_fims, "set", vlist);
//         vmp->clearVlist(vlist);

//     }

//     return 0;
// }

// 
// Look for TimeStamp 
// decode it to a time in seconds
// make sure it increments


/**
 * Phil Code
 * checks individual HeartBeat passes up warn and error count to asset Manager
 * works for any asset with HeartBeat in its /components output
 * TODO add total warnings/ errors  with clea
 * ONCE triggered dont still count the error.
 *
 * Review 10/21/2020 - timeout error and warning are hard-coded
 * Check to see if this needs to be fixed
 *
 * Used in:
 * Test Script: test_HeartBeatCheck.sh
 */
// int CheckAssetHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
// {
//     hbTestFunc* hbTest;
//     if (!amap["CheckAssetHBFunc"])
//     {
//         hbTestFunc* hbTest = new hbTestFunc(aname);
//         amap["CheckAssetHBFunc"] = (assetVar*)hbTest;
//         hbTest->toFault = 5.0;
//         hbTest->toAlarm = 3.5;
//     }
//     hbTest = (hbTestFunc*)amap["CheckAssetHBFunc"];

//     return hbTest->runFunc(vmap, amap, aname, p_fims, am);
//}

/**
 * Checks the heartbeat state, warnings, and errors for asset managers and assets
 *
 * Review 10/21/2020
 *
 * Used in:
 * Test script: test_HeartBeatCheck.sh
  * Deprecated use CheckAmHeartbeat
 */

// int CheckHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
// {
//     double dval = 0.0;
//     int ival = 0;
//     char* cval;
//     char* csVal = (char*)"All HeartBeat Init";
//     VarMapUtils* vmp = am->vm;
//     //const char *manName = am->name.c_str();
//     if (0)FPS_ERROR_PRINT("%s >> for [%s]   parent %p\n", __func__, aname, (void*)am->am);

//     //if(am->am)
//     //    manName = am->am->name.c_str();  // refer up unless we're top
//     int reload = 0;

//     if (!amap["CheckHeartBeat"] || (reload = amap["CheckHeartBeat"]->getiVal()) == 0)
//     {
//         reload = 0;
//     }

//     if (reload < 2)
//     {

//         //if(0)FPS_ERROR_PRINT("%s >> for [%s]  reload [%d]  amap[CheckHeartBeat]  %p amap %p\n", __func__, aname, reload, (void *)amap["CheckHeartBeat"],(void *)&amap);

//         //double warnVal = 3.5;
//         //double toVal = 5.0;
//         double pper = 0.250;  // does this need to be a parameter?


//         amap["CheckHeartBeat"] = vmp->setLinkVal(vmap, aname, "/reload", "CheckHeartBeat", reload);
//         amap["CheckHeartBeatPeriod"] = vmp->setLinkVal(vmap, aname, "/config", "CheckHeartBeatPeriod", pper);
//         amap["CheckHeartBeatRun"] = vmp->setLinkVal(vmap, aname, "/config", "CheckHeartBeatRun", dval);
//         amap["essHeartBeatFaults"] = vmp->setLinkVal(vmap, "ess", "/status", "essHeartBeatFaults", ival);
//         amap["essHeartBeatAlarms"] = vmp->setLinkVal(vmap, "ess", "/status", "essHeartBeatAlarms", ival);
//         if (am->am)
//         {
//             amap["amHeartBeatFaults"] = vmp->setLinkVal(vmap, am->am->name.c_str(), "/status", "HeartBeaFaults", ival);
//             amap["amHeartBeatAlarms"] = vmp->setLinkVal(vmap, am->am->name.c_str(), "/status", "HeartBeatAlarms", ival);
//         }
//         amap["HeartBeatTick"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatTick", ival);
//         amap["HeartBeatFaults"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatFaults", ival);
//         amap["HeartBeatAlarms"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatAlarms", ival);
//         amap["HeartBeatState"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatState", csVal);
//         amap["HeartBeatStateNum"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatStateNum", csVal);

//         if (reload == 0) // complete restart 
//         {
//             cval = (char*)"HeartBeat Init";
//             amap["HeartBeatState"]->setVal(cval);
//             // // amap["bmsHeartBeatState"]->setVal(cval);
//             // // amap["CheckAssetHeartBeatPeriod"]->setVal(tNow);
//             // dval =  3.5;
//             // amap["warnHeartBeatTimeout"]->setVal(dval);
//             // dval =  4.5;
//             // amap["maxHeartBeatTimeout"]->setVal(dval);
//             ival = 0;
//             amap["HeartBeatTick"]->setVal(ival);

//         }
//         ival = 2; amap["CheckHeartBeat"]->setVal(ival);
//     }

//     double tNow = vmp->get_time_dbl();
//     double pval = amap["CheckHeartBeatRun"]->getLastSetDiff(tNow);
//     double plim = amap["CheckHeartBeatPeriod"]->getdVal();
//     bool runme = true;
//     int alarms;
//     int faults;

//     if (!am->am)
//     {
//         if (0)FPS_ERROR_PRINT("%s >> OK Test Again  CheckHeartBeat >> pval: %2.3f plim: %2.3f \n", __func__, pval, plim);
//             if (pval < plim)
//             {
//                 runme = false;
//             }
//             else
//             {
//                 amap["CheckHeartBeatRun"]->setVal(tNow);
//                 amap["essHeartBeatAlarms"]->setVal(0);
//                 amap["essHeartBeatFaults"]->setVal(0);
//             }
//     }

//     if (runme)
//     {
//         //int ival, ival2;
//         amap["HeartBeatAlarms"]->setVal(0);
//         amap["HeartBeatFaults"]->setVal(0);

//         // assets are in assetMap managers are in assetManMap
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager* amc = ix.second;
//             CheckHeartBeat(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//             ival = 1;
//             ival = amap["HeartBeatTick"]->addVal(ival);
//             ival = amap["HeartBeatTick"]->getiVal();
//             if (ival > 1)
//             {
//                 char* tVal;
//                 amc->amap["HeartBeatState"]->getcVal();
//                 if (0)FPS_ERROR_PRINT("%s >> OK  CheckHeartBeat > [%s]  state [%s]\n", __func__, amc->name.c_str(), tVal);
//                 ival = 0;
//                 amap["HeartBeatTick"]->setVal(ival);
//             }

//         }
//         for (auto ix : am->assetMap)
//         {
//             asset* amc = ix.second;
//             CheckAssetHeartBeat(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//         }
//         // collect output and pass to parent
//         if (am->am)
//         {
//             alarms = amap["HeartBeatAlarms"]->getiVal();
//             faults = amap["HeartBeatFaults"]->getiVal();

//             amap["amHeartBeatFaults"]->addVal(faults);
//             amap["amHeartBeatAlarms"]->addVal(alarms);
//             if (0)FPS_ERROR_PRINT("%s >>>>>> AM [%s]  Manager [%s] alarms %d faults %d\n "
//                 , __func__
//                 , aname
//                 , am->am->name.c_str()
//                 , alarms
//                 , faults
//             );
//         }

//         else
//         {
//             alarms = amap["essHeartBeatAlarms"]->getiVal();
//             faults = amap["essHeartBeatFaults"]->getiVal();
//         }

//         if (0)FPS_ERROR_PRINT("%s >>>>>> [%s]  alarms %d faults %d\n ", __func__, aname
//             , alarms
//             , faults
//         );

//         char* cval2 = nullptr;
//         int snum = 0;
//         if (faults > 0)
//         {
//             cval2 = (char*)"HeartBeat Error";  //stateNum 3
//             snum = Asset_Fault;
//         }
//         else if (alarms > 0)
//         {
//             cval2 = (char*)"HeartBeat Warning"; //stateNum 2
//             snum = Asset_Alarm;
//         }
//         else
//         {
//             cval2 = (char*)"HeartBeat OK";  //stateNum 0
//             snum = Asset_Ok;
//         }

//         ival = amap["HeartBeatStateNum"]->getiVal();
//         if (ival != snum && snum != 0 && cval2)
//         {
//             if (0)FPS_ERROR_PRINT("%s >>>>>> AM [%s]  Manager [%s] state [%s] alarms %d faults %d\n"
//                 , __func__
//                 , aname
//                 , am->am ? am->am->name.c_str() : "System Controller"
//                 , cval2
//                 , alarms
//                 , faults
//             );
//             amap["HeartBeatStateNum"]->setVal(snum);
//             if (cval2) amap["HeartBeatState"]->setVal(cval2);
//         }
//     }
//     return 0;
// }

/**
 * Checks the communications status of the asset
 *
 * Timeouts and warnings are hard-coded - do we need parameters?
 * Review 10/21/2020
 *
 * Used in:
 * Test Script: test_ESS_comms.sh
 */
int CheckAssetComms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
{
    commsTestFunc* commsTest;
    int ival = 0;
    if (!amap["CheckAssetCommsFunc"])
    {
        commsTest = (commsTestFunc*) new commsTestFunc(aname);
        amap["CheckAssetCommsFunc"] = (assetVar*)commsTest;
        commsTest->toFault = 5.0;
        commsTest->toAlarm = 3.5;
    }
    char* tVal;
    // if(amap["CommsState"])
    // {
    //     tVal = amap["CommsState"]->getcVal();
    //     ival = amap["CommsStateNum"]->getiVal();

    //     if(1)FPS_ERROR_PRINT("%s >> OK  CheckComms  1 > [%s]  state [%s] num %d\n",__func__, aname, tVal, ival);
    // }
    commsTest = (commsTestFunc*)amap["CheckAssetCommsFunc"];

    int ret = commsTest->runFunc(vmap, amap, aname, p_fims, am);
    if (amap["CommsState"])
    {
        amap["CommsState"]->getcVal();
        ival = amap["CommsStateNum"]->getiVal();
        if (ival != 1)
        {
            // 1 is CommsOK
            //amap["amCommsErrors"]->addVal(1);
            if (0)FPS_ERROR_PRINT("%s >> OK  > [%s]  state [%s] num %d\n", __func__, aname, tVal, ival);
        }
        //if(1)FPS_ERROR_PRINT("%s >> OK  CheckComms   2  > [%s]  state [%s]\n",__func__, aname, tVal);
    }
    return ret;
}

/**
 * this is the asset manager checkComms function
 * CommsStatus local to asset
 * CommsFaultss  Warns manager unless we are the top controller
 * TODO move to assetFuncs.h
 *
 * Review 10/21/2020
 * Note: alarms and fault texts are hard-coded
 *
 * Used in:
 * Test Script: test_ComCheck.sh test_sim_comms.sh
 */
int CheckComms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    double dval = 0.0;
    int ival = 0;
    char* cval;
    char* csVal = (char*)"All Comms Init";
    VarMapUtils* vm = am->vm;
    int reload = vm->CheckReload(vmap, amap, aname, __func__);

    //const char *manName = am->name.c_str();
    if (0)FPS_ERROR_PRINT("%s >> for [%s]   parent %p\n", __func__, aname, (void*)am->am);

    //if(am->am)
    //    manName = am->am->name.c_str();  // refer up unless we're top
    if (reload < 2)
    {

        if (0)FPS_ERROR_PRINT("%s >> for [%s]  reload [%d]  amap[CheckComms]  %p amap %p\n", __func__, aname, reload, (void*)amap["CheckComms"], (void*)&amap);

        double pper = 1.0;
        amap["CheckComms"] = vm->setLinkVal(vmap, aname, "/reload", "CheckComms", reload);
        amap["CheckCommsPeriod"] = vm->setLinkVal(vmap, aname, "/config", "CheckCommsPeriod", pper);
        amap["CheckCommsRun"] = vm->setLinkVal(vmap, aname, "/config", "CheckCommsRun", dval);
        amap["essCommsFaults"] = vm->setLinkVal(vmap, "ess", "/status", "essCommsFaults", ival);
        amap["essCommsAlarms"] = vm->setLinkVal(vmap, "ess", "/status", "essCommsAlarms", ival);
        amap["essCommsInit"] = vm->setLinkVal(vmap, "ess", "/status", "essCommsInit", ival);
        if (am->am)
        {
            amap["amCommsFaults"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "CommsFaults", ival);
            amap["amCommsAlarms"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "CommsAlarms", ival);
            amap["amCommsInit"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "CommsInit", ival);
        }
        amap["CommsFaults"] = vm->setLinkVal(vmap, aname, "/status", "CommsFaults", ival);
        amap["CommsAlarms"] = vm->setLinkVal(vmap, aname, "/status", "CommsAlarms", ival);
        amap["CommsInit"] = vm->setLinkVal(vmap, aname, "/status", "CommsInit", ival);
        amap["CommsState"] = vm->setLinkVal(vmap, aname, "/status", "CommsState", csVal);
        amap["CommsStateNum"] = vm->setLinkVal(vmap, aname, "/status", "CommsStateNum", ival);


        if (reload == 0) // complete restart 
        {
            cval = (char*)"Comms Init";
            amap["CommsState"]->setVal(cval);
            ival = Asset_Init;
            amap["CommsStateNum"]->setVal(ival);
            ival = 1;
            amap["CommsInit"]->setVal(ival);
        }
        ival = 2; amap["CheckComms"]->setVal(ival);
    }

    double tNow = vm->get_time_dbl();
    double pval = amap["CheckCommsRun"]->getLastSetDiff(tNow);
    double plim = amap["CheckCommsPeriod"]->getdVal();
    bool runme = true;
    int alarms;
    int faults;
    int init;
    // are we the controller ??
    if (!am->am)
    {
        if (pval < plim)
        {
            runme = false;
        }
        else
        {
            if (0)FPS_ERROR_PRINT("%s >> OK Test Again  CheckComms >> pval: %2.3f plim: %2.3f \n", __func__, pval, plim);\

                amap["CheckCommsRun"]->setVal(tNow);
            amap["essCommsAlarms"]->setVal(0);
            amap["essCommsFaults"]->setVal(0);
            amap["essCommsInit"]->setVal(0);

        }
    }
    // runme is set when its time to run the check
    if (runme)
    {
        // assets are in assetMap managers are in assetManMap
        amap["CommsAlarms"]->setVal(0);
        amap["CommsFaults"]->setVal(0);
        amap["CommsInit"]->setVal(0);

        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            CheckComms(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            char* tVal;
            tVal = amc->amap["CommsState"]->getcVal();
            if (0)FPS_ERROR_PRINT("%s >> OK  CheckComms > [%s]  state [%s]\n", __func__, amc->name.c_str(), tVal);

        }
        for (auto ix : am->assetMap)
        {
            asset* amc = ix.second;
            CheckAssetComms(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
        }
        // collect output and pass to parent and to the ess head
        if (am->am)
        {
            alarms = amap["CommsAlarms"]->getiVal();
            faults = amap["CommsFaults"]->getiVal();
            init = amap["CommsInit"]->getiVal();

            amap["amCommsFaults"]->addVal(faults);
            amap["amCommsAlarms"]->addVal(alarms);
            amap["amCommsInit"]->addVal(init);
            if (0)FPS_ERROR_PRINT("%s >>>>>> AM [%s]  Manager [%s] Alarms %d Faults %d Init %d\n"
                , __func__
                , aname
                , am->am ? am->am->name.c_str() : "System Controller"
                , alarms
                , faults
                , init
            );

        }
        else
        {
            alarms = amap["essCommsAlarms"]->getiVal();
            faults = amap["essCommsFaults"]->getiVal();
            init = amap["essCommsInit"]->getiVal();
            if (0)FPS_ERROR_PRINT("%s >>>>>> AM [%s]  Manager [%s] alarms %d faults %d Init %d\n "
                , __func__
                , aname
                , am->name.c_str()
                , alarms
                , faults
                , init
            );

        }

        char* cval2 = nullptr;
        if (0)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  alarms %d faults %d\n ", __func__, aname
            , alarms
            , faults
        );
        ival = amap["CommsStateNum"]->getiVal();
        int snum = 0;
        if (init > 0)
        {
            cval2 = (char*)"Comms Init";
            snum = Asset_Init;
        }
        else if (faults > 0)
        {
            cval2 = (char*)"Comms Fault";
            snum = Asset_Fault;
        }
        else if (alarms > 0)
        {
            cval2 = (char*)"Comms Warning";
            snum = Asset_Alarm;
        }
        else
        {
            cval2 = (char*)"Comms OK";
            snum = Asset_Ok;
        }
        if (ival != snum && snum != 0 && cval2)
        {
            if (0)FPS_ERROR_PRINT("%s >>>>>> AM [%s]  Manager [%s] state %s alarms %d faults %d init %d\n "
                , __func__
                , aname
                , am->am ? am->am->name.c_str() : "System Controller"
                , cval2
                , alarms
                , faults
                , init
            );
            amap["CommsStateNum"]->setVal(snum);
            amap["CommsState"]->setVal(cval2);
        }

    }
    return 0;
}

/**
 * Preston's code ready for review
 * increments  a heart beat ... may need to add a period to this
 * Not used... to be removed
 *
 * Used in:
 * Test Script: test_sim_hb.sh
 */
int HandleHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
{
    int reload = 0;
    double dval = 0.0;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleHeartBeat = amap[__func__];


    if (reload < 2)
    {
        //reload = 0;
        amap["HeartBeat"] = vm->setLinkVal(vmap, aname, "/status", "HeartBeat", dval);
        dval = 255.0;
        amap["HeartBeatMax"] = vm->setLinkVal(vmap, aname, "/config", "HeartBeatMax", dval);
        amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if (reload == 0) // complete restart 
        {
            amap["HeartBeat"]->setVal(0);
            amap["HandleHeartBeat"]->setVal(2);
        }
        reload = 2;    HandleHeartBeat->setVal(reload);
    }
    // get the reference to the variable 
    assetVar* hb = amap["HeartBeat"];
    assetVar* hbmax = amap["HeartBeatMax"];
    //double ival;
    double dvalmax = hbmax->getdVal();
    dval = hb->getdVal();
    dval++;
    if (dval > dvalmax) dval = 0;
    //if(1)printf("HeartBeat %s val %f ", aname, dval);

    hb->setVal(dval);
    //dval = hb->getdVal();
    //if(1)printf("HeartBeat val after set %f\n", dval);

    vm->sendAssetVar(hb, p_fims);
    return dval;
}

/**
 * Review 10/21/2020
 * looks OK
 */
int GetESSLimits(varsmap& vmap, varmap& amap, const char* aname, asset_manager* am = nullptr)
{
    double dval;
    //double Ppu;
    //double Pcmd_dbl=0.0;
    int rc = 1;
    int reload;
    int ival;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* GetESSLimits = amap[__func__];
    const char* StartCmd_uri = simulation ? "/components/pcs/ctrlword1" : nullptr;
    //if(1)printf("%s >> %s --- Running\n", __func__, aname);
    if (reload < 2)
    {
        amap["ctrlword1"] = vm->setLinkVal(vmap, aname, "/components", "ctrlword1cfg", ival);
        amap["ActivePowerChargeLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerChargeLimit", dval);
        amap["ActivePowerDischargeLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDischargeLimit", dval);
        amap["ReactivePowerLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ReactivePowerLimit", dval);
        amap["ApparentPowerLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ApparentPowerLimit", dval);
        amap["maxChargePower"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargePower", dval);
        amap["maxDischargePower"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargePower", dval);
        amap["ChargeEnergy"] = vm->setLinkVal(vmap, "bms", "/status", "ChargeEnergy", dval);
        amap["DischargeEnergy"] = vm->setLinkVal(vmap, "bms", "/status", "DischargeEnergy", dval);
        amap["numActiveBms"] = vm->setLinkVal(vmap, "bms", "/status", "numActiveBms", ival);
        amap["pcsActivePowerLimit"] = vm->setLinkVal(vmap, "pcs", "/variables", "ActivePowerLimit", dval);
        amap["pcsReactivePowerLimit"] = vm->setLinkVal(vmap, "pcs", "/variables", "ReactivePowerLimit", dval);
        amap["pcsApparentPowerLimit"] = vm->setLinkVal(vmap, "pcs", "/variables", "ApparentPowerLimit", dval);
        if (reload == 0)
        {
            //Note reload will be 1 for any wam start
            // do any reset work here
            dval = 0.0;
            amap["ActivePowerChargeLimit"]->setVal(dval);
            amap["ActivePowerDischargeLimit"]->setVal(dval);
            amap["ReactivePowerLimit"]->setVal(dval);
            amap["ApparentPowerLimit"]->setVal(dval);
        }
        reload = 2;
        GetESSLimits->setVal(reload);
    }
    assetVar* ctrlword1 = amap["ctrlword1"];
    assetVar* PmaxC = amap["ActivePowerChargeLimit"];
    assetVar* PmaxD = amap["ActivePowerDischargeLimit"];
    assetVar* Qmax = amap["ReactivePowerLimit"];
    assetVar* Smax = amap["ApparentPowerLimit"];
    assetVar* PmaxC_BMS = amap["maxChargePower"];
    assetVar* PmaxD_BMS = amap["maxDischargePower"];
    assetVar* E_C_BMS = amap["ChargeEnergy"];
    assetVar* E_D_BMS = amap["DischargeEnergy"];
    assetVar* numActBms = amap["numActiveBms"];
    assetVar* Pmax_PCS = amap["pcsActivePowerLimit"];
    assetVar* Qmax_PCS = amap["pcsReactivePowerLimit"];
    assetVar* Smax_PCS = amap["pcsApparentPowerLimit"];
    double dval2;
    //FPS_ERROR_PRINT(" %s >> running the  family  numBMS %d \n", __func__, numActBms->getiVal());
    //FPS_ERROR_PRINT(" %s >> meet the family  \n", __func__);

    if (0)FPS_ERROR_PRINT(" %s >> send manager wakeups  \n", __func__);
    for (auto ix : am->assetManMap)
    {
        asset_manager* amm = ix.second;
        if (0)FPS_ERROR_PRINT(" %s >> Wake up manager child [%s]  with %d kids \n", __func__, ix.first.c_str(), (int)amm->assetMap.size());
        //this is the wake up for limit aggregation
        amm->run_wakeup(amm, WAKE_LEVEL1);
        // send each manager a wake up
    }
    if (0)FPS_ERROR_PRINT(" %s >> thats it for the family  numBMS %d \n", __func__, numActBms->getiVal());
    // now calculate limits
    dval2 = 0.0; // compiler happy
    // TODO rework this valueIsDiff
    if ((PmaxC_BMS->valueChangedReset() | E_C_BMS->valueChangedReset()) || Pmax_PCS->valueIsDiff(dval2) || numActBms->valueIsDiff(ival))
    {
        if ((dval = E_C_BMS->getdVal()) == 0)
            PmaxC->setVal(0.0);
        else if (PmaxC_BMS->getdVal() * numActBms->getiVal() < Pmax_PCS->getdVal())
            PmaxC->setVal(PmaxC_BMS->getdVal() * numActBms->getiVal());
        else
            PmaxC->setVal(Pmax_PCS->getdVal());
    }
    // valueChanged resets test
    // Note: With or statements, not all conditions will go through if one evaluates to true
    // Could test for single-line or
    if (PmaxD_BMS->valueChangedReset() | E_D_BMS->valueChangedReset() | Pmax_PCS->valueChangedReset() | numActBms->valueChangedReset())
    {
        if (E_D_BMS->getdVal() == 0)
            PmaxD->setVal(0.0);
        else if (PmaxD_BMS->getdVal() * numActBms->getiVal() < Pmax_PCS->getdVal())
            PmaxD->setVal(PmaxD_BMS->getdVal() * numActBms->getiVal());
        else
            PmaxD->setVal(Pmax_PCS->getdVal());
    }
    if (Qmax_PCS->valueChangedReset())
        Qmax->setVal(Qmax_PCS->getdVal());
    if (Smax_PCS->valueChangedReset())
        Smax->setVal(Smax_PCS->getdVal());

    if (ctrlword1->valueChangedReset())
    {
        if (ctrlword1->getiVal() == 1)
        {
            if (numActBms->getiVal() > 0)
                am->p_fims->Send("set", StartCmd_uri, nullptr, "{\"value\":1}");
            else
                FPS_ERROR_PRINT("BMS's not ready\n");
        }
        else if (ctrlword1->getiVal() == 0)
            am->p_fims->Send("set", StartCmd_uri, nullptr, "{\"value\":0}");
    }

    if (0) FPS_ERROR_PRINT("PmaxC BMS: %f Pmax PCS: %f Pmax C: %f Energy C: %f\n"
        , PmaxC_BMS->getdVal()
        , Pmax_PCS->getdVal()
        , PmaxC->getdVal()
        , E_C_BMS->getdVal()
    );
    if (0) FPS_ERROR_PRINT("PmaxD BMS: %f Pmax PCS: %f Pmax D: %f Energy D: %f\n"
        , PmaxD_BMS->getdVal()
        , Pmax_PCS->getdVal()
        , PmaxD->getdVal()
        , E_D_BMS->getdVal()
    );
    return rc;
}



/**
 * #include "../src/pcs_functions.cpp"
 * we have to make sure BMS has done its job first so run this at LEVEL3
 * @preston - to update
 *
 * Used in:
 * Test Script:
 */
int HandlePower(varsmap& vmap, varmap& amap, const char* aname, asset_manager* am = nullptr)
{
    double dval;
    char* sval;
    int rc = 0;
    int reload;
    // int ival;
    bool bval;
    VarMapUtils* vm = am->vm;

    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandlePower = amap[__func__];
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);

    if (reload < 2)
    {
        amap["ActivePowerChargeLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerChargeLimit", dval);
        amap["ActivePowerDischargeLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDischargeLimit", dval);
        amap["ReactivePowerLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ReactivePowerLimit", dval);
        amap["ApparentPowerLimit"] = vm->setLinkVal(vmap, aname, "/controls", "ApparentPowerLimit", dval);
        amap["ActivePowerSetpoint"] = vm->setLinkVal(vmap, aname, "/variables", "ActivePowerSetpoint", dval);
        amap["ReactivePowerSetpoint"] = vm->setLinkVal(vmap, aname, "/variables", "ReactivePowerSetpoint", dval);
        amap["PowerPriority"] = vm->setLinkVal(vmap, aname, "/variables", "PowerPriority", sval);
        amap["ActivePowerCmd"] = vm->setLinkVal(vmap, "pcs", "/controls", "ActivePowerCmd", dval);
        amap["ReactivePowerCmd"] = vm->setLinkVal(vmap, "pcs", "/controls", "ReactivePowerCmd", dval);
        // amap["CtrlwordPMode"] = vm->setLinkVal(vmap, "pcs", "/controls", "ctrlword_pmode", ival);
        // amap["CtrlwordQMode"] = vm->setLinkVal(vmap, "pcs", "/controls", "ctrlword_qmode", ival);
        amap["ConstantActiveCurrent"] = vm->setLinkVal(vmap, "pcs", "/controls", "PMode_I", bval);
        amap["ConstantActivePower"] = vm->setLinkVal(vmap, "pcs", "/controls", "PMode_P", bval);
        // amap["ConstantReactiveCurrent"]   = vm->setLinkVal(vmap, "pcs", "/controls", "QMode_I", bval);
        // amap["ConstantReactivePower"]     = vm->setLinkVal(vmap, "pcs", "/controls", "QMode_P", bval);
        // amap["ConstantCosPhi"]            = vm->setLinkVal(vmap, "pcs", "/controls", "QMode_CosPhi", bval);
        // amap["ConstantVQ"]                = vm->setLinkVal(vmap, "pcs", "/controls", "QMode_VQ", bval);
        amap["Prated"] = vm->setLinkVal(vmap, "pcs", "/params", "rated_active_power", dval);
        amap["Qrated"] = vm->setLinkVal(vmap, "pcs", "/params", "rated_reactive_power", dval);
        amap["Srated"] = vm->setLinkVal(vmap, "pcs", "/params", "rated_apparent_power", dval);
        if (reload == 0)
        {
            dval = 0.0;
            amap["ActivePowerSetpoint"]->setVal(dval);
            amap["ReactivePowerSetpoint"]->setVal(dval);
            amap["ActivePowerCmd"]->setVal(dval);
            amap["ReactivePowerCmd"]->setVal(dval);
            amap["ApparentPowerLimit"]->setVal(dval);

            sval = (char*)"p";
            amap["PowerPriority"]->setVal(sval);
            // ival = 0;
            // amap["CtrlwordPMode"]->setVal(ival);
            // amap["CtrlwordQMode"]->setVal(ival);
        }
        reload = 2;
        HandlePower->setVal(reload);
    }

    assetVar* PmaxC = amap["ActivePowerChargeLimit"];
    assetVar* PmaxD = amap["ActivePowerDischargeLimit"];
    assetVar* Qmax = amap["ReactivePowerLimit"];
    assetVar* Smax = amap["ApparentPowerLimit"];
    assetVar* Pset = amap["ActivePowerSetpoint"];
    assetVar* Qset = amap["ReactivePowerSetpoint"];
    assetVar* Pcmd = amap["ActivePowerCmd"];
    assetVar* Qcmd = amap["ReactivePowerCmd"];
    assetVar* Pri = amap["PowerPriority"];
    assetVar* Prated = amap["Prated"];
    assetVar* Qrated = amap["Qrated"];

    double Pcmd_lim = Pset->getdVal();
    double Qcmd_lim = Qset->getdVal();
    sval = nullptr;
    if (0) FPS_ERROR_PRINT("Pcmd_lim [%f] Qcmd_lim [%f] Smax [%f]\n", Pcmd_lim, Qcmd_lim, Smax->getdVal());

    if (Pset->valueChangedReset()
        | Qset->valueChangedReset()
        | PmaxC->valueChangedReset()
        | PmaxD->valueChangedReset()
        | Qmax->valueChangedReset()
        | Smax->valueChangedReset()
        | Pri->valueChangedReset())
    {
        // coerce Pcmd into acceptable range
        if (Pcmd_lim > PmaxD->getdVal())
            Pcmd_lim = PmaxD->getdVal();
        else if (Pcmd_lim < PmaxC->getdVal() * -1)
            Pcmd_lim = PmaxC->getdVal() * -1;

        // coerce Qcmd into acceptable range
        if (Qcmd_lim > Qmax->getdVal())
            Qcmd_lim = Qmax->getdVal();
        else if (Qcmd_lim < Qmax->getdVal() * -1)
            Qcmd_lim = Qmax->getdVal() * -1;
        if (0) FPS_ERROR_PRINT("After PCS/BMS limits: Pcmd_lim [%f] Qcmd_lim [%f] Smax [%f]\n", Pcmd_lim, Qcmd_lim, Smax->getdVal());

        // Make sure S isn't too large given P and Q requests
        // Decrease P or Q accordingly depending on power priority
        if (sqrt((Pcmd_lim * Pcmd_lim) + (Qcmd_lim * Qcmd_lim)) > Smax->getdVal())
        {
            sval = Pri->getcVal();
            if (sval[0] == 'q')
            {
                if (Qcmd_lim >= Smax->getdVal())
                {
                    Pcmd_lim = 0;
                    Qcmd_lim = Smax->getdVal();
                }
                else if (Pcmd_lim >= 0)
                    Pcmd_lim = sqrt((Smax->getdVal() * Smax->getdVal()) - (Qcmd_lim * Qcmd_lim));
                else
                    Pcmd_lim = -sqrt((Smax->getdVal() * Smax->getdVal()) - (Qcmd_lim * Qcmd_lim));
            }
            else
            {
                if (Pcmd_lim >= Smax->getdVal())
                {
                    Pcmd_lim = Smax->getdVal();
                    Qcmd_lim = 0;
                }
                else if (Qcmd_lim >= 0)
                    Qcmd_lim = sqrt((Smax->getdVal() * Smax->getdVal()) - (Pcmd_lim * Pcmd_lim));
                else
                    Qcmd_lim = -sqrt((Smax->getdVal() * Smax->getdVal()) - (Pcmd_lim * Pcmd_lim));
            }
        }
        // Power is commanded as percentage of rated
        Pcmd->setVal(Pcmd_lim * 100 / Prated->getdVal());
        Qcmd->setVal(Qcmd_lim * 100 / Qrated->getdVal());
    }

    if (0) FPS_ERROR_PRINT("Pcmd: %f Qcmd: %f S: %f\n",
        Pcmd->getdVal(),
        Qcmd->getdVal(),
        Prated->getdVal() * sqrt((Pcmd->getdVal() * Pcmd->getdVal()) + (Qcmd->getdVal() * Qcmd->getdVal())));

    const char* Pcmd_uri = simulation ? "/components/pcs/pcmd" : nullptr;
    const char* Qcmd_uri = simulation ? "/components/pcs/qcmd" : nullptr;

    varsmap* vlist = vm->createVlist();
    if (Pcmd->valueChangedReset())
        vm->addVlist(vlist, Pcmd, Pcmd_uri);
    if (Qcmd->valueChangedReset())
        vm->addVlist(vlist, Qcmd, Qcmd_uri);
    if (!vlist->empty())
        vm->sendVlist(am->p_fims, "set", vlist, simulation);
    vm->clearVlist(vlist);
    return rc;
}

/**
 * Retrieves and reports the max charge/discharge power and the current number of active BMS's.
 * Runs bms asset functions to find the bms asset status and limits
 *
 * Review 10/21/2020
 *
 * Used in: test_preston -> bms manager
 * Test Script: test_powerLimiting.sh
 */
int HandleBMSChargeL1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    double dval;
    int ival = 0;
    int rc = 0;
    int reload;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleBMSChargeL1Av = amap[__func__];
    if (0) FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);

    if (reload < 2)
    {
        amap["numActiveBms"] = vm->setLinkVal(vmap, "bms", "/status", "numActiveBms", ival);
        amap["maxChargePower"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargePower", dval);
        amap["maxDischargePower"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargePower", dval);
        if (reload == 1)
        {
            dval = 0;
            amap["maxChargePower"]->setVal(dval);
            amap["maxDischargePower"]->setVal(dval);
        }
        reload = 2;
        HandleBMSChargeL1Av->setVal(reload);

    }

    assetVar* PmaxC = amap["maxChargePower"];
    assetVar* PmaxD = amap["maxDischargePower"];
    assetVar* numActBms = amap["numActiveBms"];

    ival = 0; numActBms->setVal(ival);
    //double dval1, dval2;
    if (0)FPS_ERROR_PRINT("%s >> %s --- BMS Manager: NUM %d max C [%f] max D [%f]\n", __func__, aname, numActBms->getiVal(), PmaxC->getdVal(), PmaxD->getdVal());
    // we should manually run the asset wakeup...
    // now do the assets
    for (auto ix : am->assetMap)
    {
        asset* ass = ix.second; //am->getManAsset("bms");
        if (0)FPS_ERROR_PRINT("%s >>>>>>>>>%s ASSETS >>>>>>>>>>> running for Asset [%s] \n", __func__, am->name.c_str(), ix.first.c_str());
        if (ass->run_wakeup)
            ass->run_wakeup(ass, WAKE_LEVEL1);
    }

    return rc;

}

/**
 * Reports the charge/discharge power limit for the bms
 *
 * Review 10/21/2020
 * TODO - get the asset manager to set the number to 0
 * May need to have another init function
 *
 * Used in: test_preston -> bms asset
 * Test Script: test_powerLimiting.sh
 */
int GetBMSStatus(varsmap& vmap, varmap& amap, const char* aname, asset* am)
{
    double dval;
    //double dval1;
    //double dval2;
    int ival = 0;
    int rc = 0;
    int reload;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* GetBMSStatusAv = amap[__func__];

    std::string sim_oncmd_uri = "/components/" + (std::string)aname + "/ctrlword1";
    const char* Oncmd_uri = simulation ? sim_oncmd_uri.c_str() : nullptr;
    const char* comp_r = simulation ? "/components" : "/variables";

    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);


    if (reload < 2)
    {
        amap["ctrlword1"] = vm->setLinkVal(vmap, "ess", "/components", "ctrlword1cfg", ival);
        amap["bmsStatus"] = vm->setLinkVal(vmap, aname, "/variables", "status", ival);
        amap["bmsMaxChargePower"] = vm->setLinkVal(vmap, aname, comp_r, "pcharge", dval);
        amap["bmsMaxDischargePower"] = vm->setLinkVal(vmap, aname, comp_r, "pdischarge", dval);
        amap["bmsChargeEnergy"] = vm->setLinkVal(vmap, aname, comp_r, "echarge", dval);
        amap["bmsDischargeEnergy"] = vm->setLinkVal(vmap, aname, comp_r, "edischarge", dval);
        // amap["mbmuStatus"]                = vm->setLinkVal(vmap, aname, "/status", "mbmu_status", dval);
        amap["maxChargePower"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargePower", dval);
        amap["maxDischargePower"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargePower", dval);
        amap["ChargeEnergy"] = vm->setLinkVal(vmap, "bms", "/status", "ChargeEnergy", dval);
        amap["DischargeEnergy"] = vm->setLinkVal(vmap, "bms", "/status", "DischargeEnergy", dval);
        amap["numActiveBms"] = vm->setLinkVal(vmap, "bms", "/status", "numActiveBms", ival);
        if (reload == 0) // complete restart 
        {
            // DO any set up here
            dval = 100.0;  // can change this if needed
            amap["bmsMaxChargePower"]->setVal(dval);
            amap["bmsMaxDischargePower"]->setVal(dval);
            amap["bmsChargeEnergy"]->setVal(dval);
            amap["bmsDischargeEnergy"]->setVal(dval);
        }
        reload = 2;
        GetBMSStatusAv->setVal(reload);
    }

    // TODO - use assert to double-check
    assetVar* bmsStatus = amap["bmsStatus"];
    // assetVar * mbmuStatus  = amap["mbmuStatus"];
    assetVar* ctrlword1 = amap["ctrlword1"];
    assetVar* bmsPmaxC = amap["bmsMaxChargePower"];
    assetVar* bmsPmaxD = amap["bmsMaxDischargePower"];
    assetVar* bmsErC = amap["bmsChargeEnergy"];
    assetVar* bmsErD = amap["bmsDischargeEnergy"];
    assetVar* PmaxC = amap["maxChargePower"];
    assetVar* PmaxD = amap["maxDischargePower"];
    assetVar* ErC = amap["ChargeEnergy"];
    assetVar* ErD = amap["DischargeEnergy"];
    assetVar* numActBms = amap["numActiveBms"];

    // FPS_ERROR_PRINT("MBMU Status [%d]\n", mbmuStatus->getiVal());
    // mbmuStatus->setVal(ival+1);

    ival = bmsStatus->getiVal();
    if (ival == 0)
    {
        if (0)FPS_ERROR_PRINT("%s inactive, initializing\n", aname);
    }
    else if (ival == 5)  // Need to know the bms status
    {
        if (0)FPS_ERROR_PRINT("%s inactive, faulted\n", aname);
    }
    else
    {
        numActBms->addVal(1);
        if (numActBms->getiVal() == 1)  // Note - double-check the ival
        {
            PmaxC->setVal(bmsPmaxC->getdVal());
            PmaxD->setVal(bmsPmaxD->getdVal());
            ErC->setVal(bmsErC->getdVal());
            ErD->setVal(bmsErD->getdVal());
        }
        else
        {
            if ((dval = bmsPmaxC->getdVal()) < PmaxC->getdVal()) PmaxC->setVal(dval);
            if ((dval = bmsPmaxD->getdVal()) < PmaxD->getdVal()) PmaxD->setVal(dval);
            if ((dval = bmsErC->getdVal()) < ErC->getdVal()) ErC->setVal(dval);
            if ((dval = bmsErD->getdVal()) < ErD->getdVal()) ErD->setVal(dval);
        }
        if (ctrlword1->valueIsDiff(ival))
        {
            if (ctrlword1->getiVal() == 1)
                am->p_fims->Send("set", Oncmd_uri, nullptr, "{\"value\":1}");
            else if (ctrlword1->getiVal() == 0)
                am->p_fims->Send("set", Oncmd_uri, nullptr, "{\"value\":0}");
        }
    }

    // looks like a double add here
    //ival = 1; numActBms->addVal(ival);

    if (0)FPS_ERROR_PRINT("%s   --- numActBMS [%d] max C [%f] max D [%f]\n", aname, numActBms->getiVal(), bmsPmaxC->getdVal(), bmsPmaxD->getdVal());
    if (0)FPS_ERROR_PRINT("bms_man --- max C [%f] max D [%f]\n", PmaxC->getdVal(), PmaxD->getdVal());

    return rc;
}

/**
 * Reports the active/reactive/apparent power limit for the pcs
 *
 * Review 10/21/2020
 *
 * Used in: test_preston -> pcs asset
 * Test Script: test_powerLimiting.sh
 */
int GetPCSLimits(varsmap& vmap, varmap& amap, const char* aname, asset_manager* am)
{
    double dval;
    //double dval1;
    //double dval2;
    //int ival = 0;

    int rc = 0;
    int reload;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);

    assetVar* GetPCSLimits = amap["GetPCSLimits"];
    if (0) FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);
    const char* comp_r = simulation ? "/components" : "/variables";


    if (reload < 2)
    {
        amap["Prated"] = vm->setLinkVal(vmap, aname, "/params", "rated_active_power", reload);
        amap["Qrated"] = vm->setLinkVal(vmap, aname, "/params", "rated_reactive_power", reload);
        amap["Srated"] = vm->setLinkVal(vmap, aname, "/params", "rated_apparent_power", reload);
        amap["InductiveILimit"] = vm->setLinkVal(vmap, aname, "/variables", "pcs_inductive_i_limit", dval);
        amap["CapacitiveILimitPU"] = vm->setLinkVal(vmap, aname, "/variables", "pcs_capacitive_i_limit", dval);
        amap["ActivePowerLimitPU"] = vm->setLinkVal(vmap, aname, comp_r, "plim", dval);
        amap["ReactivePowerLimitPU"] = vm->setLinkVal(vmap, aname, comp_r, "qlim", dval);
        amap["ApparentPowerLimitPU"] = vm->setLinkVal(vmap, aname, "/variables", "pcs_s_limit_inst", dval);
        amap["ActivePowerLimit"] = vm->setLinkVal(vmap, aname, "/variables", "ActivePowerLimit", dval);
        amap["ReactivePowerLimit"] = vm->setLinkVal(vmap, aname, "/variables", "ReactivePowerLimit", dval);
        amap["ApparentPowerLimit"] = vm->setLinkVal(vmap, aname, "/variables", "ApparentPowerLimit", dval);

        // if(reload == 0) // complete restart 
        // {
        //      // DO any setup here    
        // }
        reload = 2;
        GetPCSLimits->setVal(reload);
    }
    assetVar* Prated = amap["Prated"];
    assetVar* Qrated = amap["Qrated"];
    assetVar* Srated = amap["Srated"];
    //assetVar * ILmax   = amap["InductiveILimit"];
    //assetVar * ICmax   = amap["CapacitiveILimit"];
    assetVar* PmaxPU = amap["ActivePowerLimitPU"];
    assetVar* QmaxPU = amap["ReactivePowerLimitPU"];
    assetVar* SmaxPU = amap["ApparentPowerLimitPU"];
    assetVar* Pmax = amap["ActivePowerLimit"];
    assetVar* Qmax = amap["ReactivePowerLimit"];
    assetVar* Smax = amap["ApparentPowerLimit"];

    Pmax->setVal(PmaxPU->getdVal() * 0.01 * Prated->getdVal());  // PLimit comes as a percentage
    Qmax->setVal(QmaxPU->getdVal() * 0.01 * Qrated->getdVal());
    Smax->setVal(SmaxPU->getdVal() * 0.01 * Srated->getdVal());

    if (0)FPS_ERROR_PRINT("%s --- Prated [%f] PmaxPU [%f] Pmax [%f]\n", aname, Prated->getdVal(), PmaxPU->getdVal(), Pmax->getdVal());
    if (0)FPS_ERROR_PRINT("%s --- Qrated [%f] QmaxPU [%f] Qmax [%f]\n", aname, Qrated->getdVal(), QmaxPU->getdVal(), Qmax->getdVal());
    return rc;
}

int PowerModeSelect(varsmap& vmap, varmap& amap, const char* aname, asset_manager* am = nullptr)
{
    double dval;
    //char* sval;
    int rc = 0;
    int reload = 2;
    int ival = 0;
    bool bval;
    char* tval = (char*)" Test Char";
    //bool fval = false;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* PowerModeSelect = amap["PowerModeSelect"];

    bool tb;
    bool lasttb;
    bool tb1;
    bool lasttb1;
    bool tb2;
    bool lasttb2;


    if (reload < 2)
    {
        if (1)printf("%s >> %s --- >>>>>>>>>>>>> Running reload %d\n", __func__, aname, reload);
        // amap["CtrlwordPMode"]             = vm->setLinkVal(vmap, aname, "/controls", "ctrlword_pmode", ival);
        // amap["CtrlwordQMode"]             = vm->setLinkVal(vmap, aname, "/controls", "ctrlword_qmode", ival);
        amap["ConstantActiveCurrentSet"] = vm->setLinkVal(vmap, aname, "/controls", "PMode_I_set", bval);
        amap["ConstantActivePowerSet"] = vm->setLinkVal(vmap, aname, "/controls", "PMode_P_set", bval);
        amap["ConstantActiveCurrent"] = vm->setLinkVal(vmap, aname, "/controls", "PMode_I", bval);
        amap["ConstantActivePower"] = vm->setLinkVal(vmap, aname, "/controls", "PMode_P", bval);
        amap["ConstantReactiveCurrent"] = vm->setLinkVal(vmap, aname, "/controls", "QMode_I", bval);
        amap["ConstantCosPhi"] = vm->setLinkVal(vmap, aname, "/controls", "QMode_CosPhi", bval);
        amap["ConstantReactivePower"] = vm->setLinkVal(vmap, aname, "/controls", "QMode_Q", bval);
        amap["ConstantVReactivePower"] = vm->setLinkVal(vmap, aname, "/controls", "QMode_VQ", bval);
        amap["TestBool"] = vm->setLinkVal(vmap, aname, "/status", "TestBool", bval);
        amap["TestDouble"] = vm->setLinkVal(vmap, aname, "/status", "TestDouble", dval);
        amap["TestInt"] = vm->setLinkVal(vmap, aname, "/status", "TestInt", ival);
        amap["TestChar"] = vm->setLinkVal(vmap, aname, "/status", "TestChar", tval);


        if (reload == 0)
        {
            bval = false;
            amap["ConstantActiveCurrent"]->setVal(bval);
            amap["ConstantActiveCurrentSet"]->setVal(bval);
            amap["ConstantReactiveCurrent"]->setVal(bval);
            amap["ConstantCosPhi"]->setVal(bval);
            amap["ConstantVReactivePower"]->setVal(bval);
            bval = true;
            amap["ConstantActivePower"]->setVal(bval);
            amap["ConstantActivePowerSet"]->setLVal(bval);
            amap["ConstantActiveCurrent"]->setLVal(bval);
            amap["ConstantReactivePower"]->setVal(bval);

            bval = false;
            amap["TestBool"]->setVal(bval);
            tb = amap["TestBool"]->getbVal();
            lasttb = amap["TestBool"]->getbLVal();
            bval = true;
            amap["TestBool"]->setVal(bval);

            tb1 = amap["TestBool"]->getbVal();
            lasttb1 = amap["TestBool"]->getbLVal();
            bval = false;
            printf(" calling setVal\n");
            amap["TestBool"]->setVal(bval);
            tb2 = amap["TestBool"]->getbVal();
            lasttb2 = amap["TestBool"]->getbLVal();
            printf("%s >> xxx>>>>>tb [%s] -> [%s]  tb1 [%s] -> [%s] tb2 [%s] -> [%s]\n"
                , __func__
                , lasttb ? "true" : "false"
                , tb ? "true" : "false"
                , lasttb1 ? "true" : "false"
                , tb1 ? "true" : "false"
                , lasttb2 ? "true" : "false"
                , tb2 ? "true" : "false"

            );
            assetVar* cXX = amap["TestBool"];
            bval = true;
            vm->setVal(vmap, cXX->comp.c_str(), cXX->name.c_str(), bval);
            tb2 = amap["TestBool"]->getbVal();
            lasttb2 = amap["TestBool"]->getbLVal();
            printf("%s >> xxx>>>>>tb [%s] -> [%s]  tb1 [%s] -> [%s] xxx tb2 [%s] -> [%s]\n"
                , __func__
                , lasttb ? "true" : "false"
                , tb ? "true" : "false"
                , lasttb1 ? "true" : "false"
                , tb1 ? "true" : "false"
                , lasttb2 ? "true" : "false"
                , tb2 ? "true" : "false"

            );
            dval = 1.0;
            amap["TestDouble"]->setVal(dval);
            double db = amap["TestDouble"]->getdVal();
            double lastdb = amap["TestDouble"]->getdLVal();
            dval = 2.0;
            amap["TestDouble"]->setVal(dval);

            double db1 = amap["TestDouble"]->getdVal();
            double lastdb1 = amap["TestDouble"]->getdLVal();
            dval = 3.0;
            printf(" calling setVal\n");
            amap["TestDouble"]->setVal(dval);
            double db2 = amap["TestDouble"]->getdVal();
            double lastdb2 = amap["TestDouble"]->getdLVal();
            printf("%s >> xxx>>>>>db [%f] -> [%f]  db1 [%f] -> [%f] db2 [%f] -> [%f]\n"
                , __func__
                , lastdb
                , db
                , lastdb1
                , db1
                , lastdb2
                , db2

            );

            assetVar* cXXD = amap["TestDouble"];
            dval = 4.0;
            vm->setVal(vmap, cXXD->comp.c_str(), cXXD->name.c_str(), dval);
            db2 = amap["TestDouble"]->getdVal();
            lastdb2 = amap["TestDouble"]->getdLVal();
            printf("%s >> xxx>>>>>tb [%f] -> [%f]  tb1 [%f] -> [%f] xxx tb2 [%f] -> [%f]\n"
                , __func__
                , lastdb
                , db
                , lastdb1
                , db1
                , lastdb2
                , db2

            );

            ival = 2;
            amap["TestInt"]->setVal(ival);
            int ib = amap["TestInt"]->getiVal();
            int lastib = amap["TestInt"]->getiLVal();
            ival = 3;
            amap["TestInt"]->setVal(ival);

            int ib1 = amap["TestInt"]->getiVal();
            int lastib1 = amap["TestInt"]->getiLVal();
            ival = 4;
            printf(" calling setVal\n");
            amap["TestInt"]->setVal(ival);
            int ib2 = amap["TestInt"]->getbVal();
            int lastib2 = amap["TestInt"]->getiLVal();
            printf("%s >> xxx>>>>>ib [%d] -> [%d]  ib1 [%d] -> [%d] ib2 [%d] -> [%d]\n"
                , __func__
                , lastib
                , ib
                , lastib1
                , ib1
                , lastib2
                , ib2
            );

            assetVar* cXXI = amap["TestInt"];
            ival = 5;
            vm->setVal(vmap, cXXI->comp.c_str(), cXXI->name.c_str(), ival);
            ib2 = amap["TestInt"]->getiVal();
            lastib2 = amap["TestInt"]->getiLVal();
            printf("%s >> xxx>>>>>ib [%d] -> [%d]  ib1 [%d] -> [%d] xxx ib2 [%d] -> [%d]\n"
                , __func__
                , lastib
                , ib
                , lastib1
                , ib1
                , lastib2
                , ib2

            );


            tval = (char*)"TextVal 1";
            amap["TestChar"]->setVal(tval);
            char* tc = amap["TestChar"]->getcVal();
            char* lasttc = amap["TestChar"]->getcLVal();
            tval = (char*)"TextVal 2";
            amap["TestChar"]->setVal(tval);

            char* tc1 = amap["TestChar"]->getcVal();
            char* lasttc1 = amap["TestChar"]->getcLVal();
            tval = (char*)"TextVal 3";
            printf(" calling setVal\n");
            amap["TestChar"]->setVal(tval);
            char* tc2 = amap["TestChar"]->getcVal();
            char* lasttc2 = amap["TestChar"]->getcLVal();
            printf("%s >> xxx>>>>>tc [%s] -> [%s]  tc1 [%s] -> [%s] tc2 [%s] -> [%s]\n"
                , __func__
                , lasttc
                , tc
                , lasttc1
                , tc1
                , lasttc2
                , tc2

            );
            assetVar* cXXT = amap["TestChar"];
            tval = (char*)"TextVal 4";
            vm->setVal(vmap, cXXT->comp.c_str(), cXXT->name.c_str(), tval);
            tc2 = amap["TestChar"]->getcVal();
            lasttc2 = amap["TestChar"]->getcLVal();
            printf("%s >> xxx>>>>>tc [%s] -> [%s]  tc1 [%s] -> [%s] xxx tc2 [%s] -> [%s]\n"
                , __func__
                , lasttc
                , tc
                , lasttc1
                , tc1
                , lasttc2
                , tc2
            );

        }
        reload = 2;
        PowerModeSelect->setVal(reload);
    }

    assetVar* CId_s = amap["ConstantActiveCurrentSet"];
    assetVar* CP_s = amap["ConstantActivePowerSet"];
    assetVar* CId = amap["ConstantActiveCurrent"];
    assetVar* CP = amap["ConstantActivePower"];
    //assetVar * CIq = amap["ConstantReactiveCurrent"];
    //assetVar * CQ = amap["ConstantReactivePower"];
    //assetVar * CCosPhi = amap["ConstantCosPhi"];
    //assetVar * CVQ = amap["ConstantVReactivePower"];

    bool bCId = CId->getbVal();
    bool bCP = CP->getbVal();
    bool bCId_inv = !bCId;
    bool bCP_inv = !bCP;

    if (1)FPS_ERROR_PRINT("%s >> CP_set [%d] CID_set [%d] bCP_inv[%d]  bCId_inv [%d]\n", __func__, CP_s->getbVal(), CId_s->getbVal(), bCP_inv, bCId_inv);
    // Cases we care about are both set to true or both set to false (need 1 of them true always)
    if (CP_s->getbVal() && CId_s->getbVal())
    {
        vm->setVal(vmap, CP->comp.c_str(), CP->name.c_str(), bCP_inv);
        vm->setVal(vmap, CId->comp.c_str(), CId->name.c_str(), bCId_inv);
        CP_s->setVal(bCP_inv);
        CId_s->setVal(bCId_inv);
    }
    else if (!CP_s->getbVal() && !CId_s->getbVal())
    {
        vm->setVal(vmap, CP->comp.c_str(), CP->name.c_str(), bCP);
        vm->setVal(vmap, CId->comp.c_str(), CId->name.c_str(), bCId);
        CP_s->setVal(bCP);
        CId_s->setVal(bCId);
    }


    //bval3 = false;
    //bval2 =  false;
    if (CId->valueChangedReset())
        FPS_ERROR_PRINT("%s >>> CC [%d] CP [%d] --- Last CC [%d] Last CP [%d]\n", __func__, CId->getbVal(), CP->getbVal(), CId->getbLVal(), CP->getbLVal());
    // if (CId->getbLVal())
    // {
    //     vm->setVal(vmap, CP->comp.c_str(), CP->name.c_str(), fval);
    //     CP->setLVal(fval);
    //     CId->setLVal(tval);
    // }
    // else
    // {
    //     vm->setVal(vmap, CId->comp.c_str(), CId->name.c_str(), fval);
    //     CP->setLVal(tval);
    //     CId->setLVal(fval);
    // }
// }
// else if (!CP->getbVal() && !CId->getbVal())
// {
//     if (CId->getbLVal())
//     {
//         vm->setVal(vmap, CP->comp.c_str(), CP->name.c_str(), fval);
//         CP->setLVal(fval);
//         CId->setLVal(tval);
//     }
//     else
//     {
//         vm->setVal(vmap, CId->comp.c_str(), CId->name.c_str(), fval);
//         CP->setLVal(tval);
//         CId->setLVal(fval);
//     }
// }

// if (!CP->getbLVal())
// {
//     if (CP->getbVal())
//     {
//         vm->setVal(vmap, CId->comp.c_str(), CId->name.c_str(), fval);
//         CP->setLVal(tval);
//         CId->setLVal(fval);
//     }
// }
// else if (!CP->getbVal() && !CId->getbVal())
//     vm->setVal(vmap, CP->comp.c_str(), CP->name.c_str(), tval);

// if (!CId->getbLVal())
// {
//     if (CId->getbVal())
//     {
//         vm->setVal(vmap, CP->comp.c_str(), CP->name.c_str(), fval);
//         CId->setLVal(tval);
//         CP->setLVal(fval);
//     }
// }
// else if (!CP->getbVal() && !CId->getbVal())
//     vm->setVal(vmap, CId->comp.c_str(), CId->name.c_str(), tval);

    return rc;

}


/**
 * Example script... will be moved
 *
 * This runs every time set /componments/xxx/PCRCmd is run
 *
 * void setupRamFunc(int (*_runFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager *am),varsmap *_vmap, varmap *_amap,const char* _aname, fims* _p_fims, asset_manager *_am)
 * This only runs when we get a set or Pub  PCRCmd ( Default /components/pcr/PCRCmd)
 *
 * Review 10/21/2020
 *
 * Used in:
 * Test Script:
 */
int HandlePCRCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    VarMapUtils* vm;
    vm = am->vm;
    int reload = 0;
    int ival = -1;
    char* tVal = (char*)" PCR Command Init";
    if (!amap["HandlePCRCmd"] || (reload = amap["HandlePCRCmd"]->getiVal()) < 2)
    {
        amap["HandlePCRCmd"] = vm->setLinkVal(vmap, aname, "/reload", "HandlePCRCmd", reload);
        amap["PCRCmd"] = vm->setLinkVal(vmap, aname, "/components", "PCRCmd", ival);
        amap["PCRCmdStatus"] = vm->setLinkVal(vmap, aname, "/status", "PCRCmdStatus", tVal);
        if (reload < 1)
        {
            // do init stiff here
            amap["PCRCmdStatus"]->setVal(tVal); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);
            amap["PCRCmd"]->setVal(ival); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);

        }
        reload = 2;
        amap["HandlePCRCmd"]->setVal(reload);
    }
    char* cval;
    double cmdTime = vm->get_time_dbl();
    ival = amap["PCRCmd"]->getiVal(); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);

    asprintf(&cval, "%s >> Set PCRCmd Status to %d at %f", __func__, ival, cmdTime);
    if (cval)
    {
        amap["PCRCmdStatus"]->setVal(cval); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);
        free((void*)cval);
    }
    // Now do stuff to handle change in PCSCmd 
    return 0;
}


/**
 * Example code
 *
 * this sets up the PCRCmd to run the HandlePCRCmd function every time the PCRCmd is "set"
 * this instance must be set up by the asset manager
 * this really should be an init function.
 * to test set a value in /components/ess/PCRCmd  and check /ess/status/ess/PCRCmdStatus
 *
 * Used in:
 * Test Script:
 */
int SetupRunPCRCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{

    FPS_ERROR_PRINT(">>>>>>>>>ESS>>>>>>>>>>>%s running for ESS Manager\n", __func__);

    assetFunc* RunPCRCmd;
    VarMapUtils* vm;
    vm = am->vm;
    int reload = 0;

    if (!amap["SetupRunPCRCmd"] || (reload = amap["SetupRunPCRCmd"]->getiVal()) < 2)
    {
        amap["SetupRunPCRCmd"] = vm->setLinkVal(vmap, aname, "/reload", "SetupRunPCRCmd", reload);
        amap["RunPCRCmd"] = vm->setLinkVal(vmap, aname, "/status", "RunPCRCmd", reload);
        amap["PCRCmd"] = vm->setLinkVal(vmap, aname, "/components", "PCRCmd", reload);

        if (reload < 1)
        {
            RunPCRCmd = (assetFunc*) new assetFunc(aname);
            amap["RunPCRCmdFunc"] = (assetVar*)RunPCRCmd;
            RunPCRCmd->setupRamFunc(HandlePCRCmd, vmap, amap, aname, p_fims, am);
            // the following causes HandlePCRCmd to be run on eery Set or Pub on PCRCmd
            // may need to use VarMapUtils to runit
            amap["PCRCmd"]->SetFunc((assetVar*)RunPCRCmd);
            amap["PCRCmd"]->PubFunc((assetVar*)RunPCRCmd);
        }
        reload = 2;
        amap["SetupRunPCRCmd"]->setVal(reload);
    }

    return 0;
}

/**
 * Deprecated ??
 *
 * Handles the load operation
 *
 * Used in:
 * Test Script: test_BMS_LoadRequest.sh
 */
int HandleLoadRequest(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    double dval;
    bool  bval;
    char* cval = (char*)"Dummy";
    int reload;

    // setLinkVal 
    // 1/ looks for /links/<aname> and if so picks up the assetVar by reference to the link valuestring
    // for example /links/bms:maxLoadRequest -> /params/bms:maxLoadRequest this is a global for all bms units
    //             /links/bms_1:LoadSetpoint -> /controls/bms_1:LoadSetpoint this is a setpoint for unit bms_1 
    //             /links/ess:EStop -> /controls/ess:EStop this is a global command
    // setLinkval will look for an established link from the config file
    // if not found it will create a link called /links/<aname>  to the default agroup (/params/<aname>:<aval> etc)
    //   it will then look for the "linked to" variable for example /components/catl_bms_ess_01:bms_soc
    //     it will create  this variable ( with the given type) if needed
    //  thus the loop is closed we have a link and an associated variable. creates for us or predefined.
    // to force the links to be reevaluated then /controls/<anmae>:FunName should be set to 1
    // to cause a complete reset set reload to 0;
    //
    assetVar* HandleLoadRequest = amap["HandleLoadRequest"];
    if (!HandleLoadRequest || (reload = HandleLoadRequest->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        //reload = 0;
        amap["HandleLoadRequest"] = am->vm->setLinkVal(vmap, aname, "/config", "HandleLoadRequest", reload);
        amap["maxLoadRequest"] = am->vm->setLinkVal(vmap, "bms", "/params", "maxLoadRequest", dval);
        amap["LoadRequest"] = am->vm->setLinkVal(vmap, "bms", "/controls", "LoadRequest", dval);
        amap["LoadRequestDeadband"] = am->vm->setLinkVal(vmap, "bms", "/params", "LoadRequestDeadband", dval);
        amap["lastLoadRequest"] = am->vm->setLinkVal(vmap, "bms", "/controls", "lastLoadRequest", dval);
        amap["LoadSetpoint"] = am->vm->setLinkVal(vmap, aname, "/controls", "LoadSetpoint", dval);
        amap["LoadState"] = am->vm->setLinkVal(vmap, aname, "/status", "LoadState", cval);
        amap["StateResetCmd"] = am->vm->setLinkVal(vmap, "bms", "/controls", "StateResetCmd", bval);
        amap["lastStateResetCmd"] = am->vm->setLinkVal(vmap, "bms", "/status", "lastStateResetCmd", bval);
        amap["EStop"] = am->vm->setLinkVal(vmap, "ess", "/controls", "EStop", bval);

        amap["HandleLoadRequest"]->setVal(2);  // revert reload
        if (reload == 0) // complete restart 
        {
            amap["LoadRequestDeadband"]->setVal(25.0);
            amap["lastLoadRequest"]->setVal(25.0);
            amap["LoadRequest"]->setVal(26.0);
            amap["StateResetCmd"]->setVal(false);
            amap["lastStateResetCmd"]->setVal(false);

            amap["LoadState"]->setVal("Init");
        }
    }

    assetVar* maxLoadRequest = amap["maxLoadRequest"];  // all these will crash if the link vars are not set up correctly
    assetVar* LoadRequest = amap["LoadRequest"];
    assetVar* LoadRequestDeadband = amap["LoadRequestDeadband"];
    assetVar* lastLoadRequest = amap["lastLoadRequest"];
    assetVar* LoadSetpoint = amap["LoadSetpoint"];
    assetVar* LoadState = amap["LoadState"];
    assetVar* StateResetCmd = amap["StateResetCmd"];
    assetVar* lastStateResetCmd = amap["lastStateResetCmd"];
    //assetVar* EStop               = amap["EStop"];

    if (0)printf("%s >>>>> STATUS %s  (comp %s) %f to %f (deadband %f)\n"
        , __func__
        , LoadRequest->name.c_str()
        , LoadRequest->comp.c_str()
        , lastLoadRequest->getdVal()
        , LoadRequest->getdVal()
        , LoadRequestDeadband->getdVal()
    );

    if (am->vm->valueChanged(LoadRequest, lastLoadRequest, LoadRequestDeadband, dval, 0.0))
    {
        printf("%s >>>>> load value changed from %f to %f (deadband %f)\n"
            , __func__
            , lastLoadRequest->getdVal()
            , LoadRequest->getdVal()
            , LoadRequestDeadband->getdVal()
        );
        lastLoadRequest->setVal(LoadRequest->getdVal());
        if (LoadRequest->getdVal() < maxLoadRequest->getdVal())
        {
            LoadSetpoint->setVal(LoadRequest->getdVal());
            cval = (char*)"Running";LoadState->setVal(cval);
        }
        else
        {
            LoadSetpoint->setVal(maxLoadRequest->getdVal());
            cval = (char*)"Limit"; LoadState->setVal(cval);
        }

    }

    if (am->vm->valueChangednodb(LoadRequest, lastLoadRequest, dval, 0.0))
    {

        if (abs(LoadRequest->getdVal()) < 2.0)
        {
            LoadSetpoint->setVal(LoadRequest->getdVal());
            cval = (char*)"Standby";LoadState->setVal(cval);
        }
    }

    if (am->vm->valueChangednodb(StateResetCmd, lastStateResetCmd, bval, 0.0))
    {
        lastStateResetCmd->setVal(StateResetCmd->getbVal());
        if (StateResetCmd->getbVal())
        {
            LoadSetpoint->setVal(0.0);
            cval = (char*)"Reset";LoadState->setVal(cval);
        }
        else
        {
            cval = (char*)"Standby";LoadState->setVal(cval);
        }

    }
    return 0;
}

/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 10/21/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleCurrent(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    double dval = 0.0;  // may need to convert to int, unless val in map is already an int
    //double dval2 = 0.0;
    int reload = -1;
    assetVar* HandleCurrent = amap["HandleCurrent"];

    if (!HandleCurrent || (reload = HandleCurrent->getdVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        amap["ActiveCurrentSetpoint"] = am->vm->setLinkVal(vmap, "ess", "/controls", "ActiveCurrentSetpoint", dval);
        amap["lastActiveCurrentSetpoint"] = am->vm->setLinkVal(vmap, "ess", "/variables", "lastActiveCurrentSetpoint", dval);
        amap["ReactiveCurrentSetpoint"] = am->vm->setLinkVal(vmap, "ess", "/controls", "ReactiveCurrentSetpoint", dval);
        amap["lastReactiveCurrentSetpoint"] = am->vm->setLinkVal(vmap, "ess", "/variables", "lastReactiveCurrentSetpoint", dval);

        amap["pcs_ActiveCurrent"] = am->vm->setLinkVal(vmap, "pcs", "/components", "ActiveCurrent", dval);
        amap["pcs_ReactiveCurrent"] = am->vm->setLinkVal(vmap, "pcs", "/components", "ReactiveCurrent", dval);

        amap["HandleCurrent"] = am->vm->setLinkVal(vmap, aname, "/reload", "HandleCurrent", reload);

        // if(reload == 0) // complete restart 
        // {
        // }
        reload = 2;
        amap["HandleCurrent"]->setVal(reload);  // revert reload
    }

    // this stuff collects a bunch of assetVars and send them out to their default locations.
    // the link will determine where that location is.
    // if the link is defined in the config file then that destination will be maintained.

    varsmap vlist;
    bool setpointApplied = false;
    if (0) FPS_ERROR_PRINT("%s >> Checking current changes... \n", __func__);

    // Check for any changes to active current setpoint
    if (amap["ActiveCurrentSetpoint"]->getdVal() != amap["lastActiveCurrentSetpoint"]->getdVal())
    {
        // TODO - fix debug msg
        //std::cout << "Active current has changed." << "\nActive Current Setpoint: " << amap["ActiveCurrentSetpoint"]->getdVal() << "\nLast Active Current Setpoint: " << amap["lastActiveCurrentSetpoint"]->getdVal() << std::endl; 
        amap["lastActiveCurrentSetpoint"]->setVal(amap["ActiveCurrentSetpoint"]->getdVal());
        amap["pcs_ActiveCurrent"]->setVal(amap["ActiveCurrentSetpoint"]->getdVal());

        am->vm->addVlist(&vlist, amap["lastActiveCurrentSetpoint"]);
        am->vm->addVlist(&vlist, amap["pcs_ActiveCurrent"]);

        setpointApplied = true;
    }

    // Check for changes to reactive current setpoint - is this needed?
    if (amap["ReactiveCurrentSetpoint"]->getdVal() != amap["lastReactiveCurrentSetpoint"]->getdVal())
    {
        // TODO - fix debug msg
        //std::cout << "Reactive current has changed." << "\nReactive Current Setpoint: " << amap["ReactiveCurrentSetpoint"]->getdVal() << "\nLast Reactive Current Setpoint: " << amap["lastReactiveCurrentSetpoint"]->getdVal() << std::endl;
        amap["lastReactiveCurrentSetpoint"]->setVal(amap["ReactiveCurrentSetpoint"]->getdVal());
        amap["pcs_ReactiveCurrent"]->setVal(amap["ReactiveCurrentSetpoint"]->getdVal());

        am->vm->addVlist(&vlist, amap["lastReactiveCurrentSetpoint"]);
        am->vm->addVlist(&vlist, amap["pcs_ReactiveCurrent"]);

        setpointApplied = true;
    }

    if (setpointApplied)
    {
        am->vm->sendVlist(p_fims, "set", &vlist);
        //am->vm->clearVlist(&vlist);
    }

    return 0;
}
// ems_cmd
// 0x0300 2 BMS heartbeat BMS heartbeats 0-255, updated
// every 1s
// 0x0301 2 BMS_poweron
//           BMS high voltage status
//           0: Power off ready
//           1: Power on ready
//           2: Power on fault
//           3: Power off fault
// 0x0302 2 BMS_status
//         BMS system status
//         0: Initial status
//         1: Normal status
//         2: Full charge status
//         3: Full discharge status
//         4: Warning status
//         5: Fault status
// Command of EMS to control BMS relay
// bms_cmd in this case
//        0 : Initial
//        1: Stay status
//        2: Power on cmd
//        3: Power off cmd
// 0x0010 2 System status 
//       000 Initialize
//       001 Normal
//       010 Full charge
//       011 Full discharge
//       100 Warning status
//       101 Fault status
/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_phil.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */

int HandleAssetCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am, int sstate)
{
    VarMapUtils* vm = am->vm;
    printf("%s >> %s --- Running  state %d\n", __func__, aname, sstate);
    assetVar* HandleAssetCmd = amap["HandleAssetCmd"];
    char* tval = (char*)" Asset Init";
    int reload = 0;
    bool bval = false;
    int ival = -1;
    if (!HandleAssetCmd || (reload = HandleAssetCmd->getiVal()) == 0)
    {
        reload = 0;
    }

    if (reload < 2)
    {
        // note links must set these values
        amap["HandleAssetCmd"] = vm->setLinkVal(vmap, aname, "/reload", "HandleAssetCmd", reload);
        amap["AssetCmd"] = vm->setLinkVal(vmap, aname, "/controls", "AssetCmd", ival);
        amap["AssetOn"] = vm->setLinkVal(vmap, aname, "/params", "AssetOn", ival);
        amap["AssetOff"] = vm->setLinkVal(vmap, aname, "/params", "AssetOff", ival);
        amap["AssetStandby"] = vm->setLinkVal(vmap, aname, "/params", "AssetStandby", ival);
        amap["AssetInit"] = vm->setLinkVal(vmap, aname, "/params", "AssetInit", ival);
        amap["OnCmd"] = vm->setLinkVal(vmap, aname, "/controls", "OnCmd", bval);
        amap["OffCmd"] = vm->setLinkVal(vmap, aname, "/controls", "OffCmd", bval);
        amap["StandbyCmd"] = vm->setLinkVal(vmap, aname, "/controls", "StandbyCmd", bval);
        amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status", "AssetState", tval);
        amap["BypassHB"] = vm->setLinkVal(vmap, aname, "/controls", "BypassHB", ival);
        amap["BypassComms"] = vm->setLinkVal(vmap, aname, "/controls", "BypassComms", ival);
        if (reload < 1)
        {
            amap["OnCmd"]->setVal(false);
            amap["OffCmd"]->setVal(false);
            amap["StandbyCmd"]->setVal(false);
            amap["AssetState"]->setVal(tval);
            amap["BypassHB"]->setVal(false);
            amap["BypassComms"]->setVal(false);
        }
        reload = 2;
        amap["HandleAssetCmd"]->setVal(reload);

    }
    assetVar* asv;    // send var
    assetVar* acv;    // command var
    char* fval;
    asprintf(&fval, " %s Asset State %d", aname, sstate);
    if (fval)
    {
        amap["AssetState"]->setVal(fval);
        free((void*)fval);
    }
    switch (sstate)
    {
    case SystemInit:
        asv = amap["AssetCmd"];
        acv = amap["AssetInit"];
        break;
    case SystemOn:
        asv = amap["AssetCmd"];
        acv = amap["AssetOn"];
        break;
    case SystemOff:
        asv = amap["AssetCmd"];
        acv = amap["AssetOff"];
        break;
    case SystemStandby:
        asv = amap["AssetCmd"];
        acv = amap["AssetStandby"];
        break;
    default:
        asv = nullptr;
    }

    if (asv)
    {
        asv->setVal(acv->getiVal());
        vm->sendAssetVar(asv, p_fims);
    }
    return 0;
}
/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_phil.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleManagerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am, int sstate)
{
    VarMapUtils* vm = am->vm;
    char* tVal = (char*)" Init";
    printf("%s >> %s --- Running state %d\n", __func__, aname, sstate);
    assetVar* HandleManCmd = amap["HandleManCmd"];
    printf("%s >> %s --- Running\n", __func__, aname);
    bool bval = false;
    int ival = 0;
    int reload = 0;
    if (!HandleManCmd || (reload = HandleManCmd->getiVal()) == 0)
    {
        reload = 0;
    }

    if (reload < 2)
    {
        amap["HandleManCmd"] = vm->setLinkVal(vmap, aname, "/reload", "HandleManCmd", reload);
        amap["On"] = vm->setLinkVal(vmap, aname, "/status", "On", bval);
        amap["Off"] = vm->setLinkVal(vmap, aname, "/status", "Off", bval);
        amap["Standby"] = vm->setLinkVal(vmap, aname, "/status", "Standby", bval);
        amap["Fault"] = vm->setLinkVal(vmap, aname, "/status", "Fault", bval);
        amap["ResetFaultCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ResetFaultCmd", bval);
        amap["OnCmd"] = vm->setLinkVal(vmap, aname, "/controls", "OnCmd", bval);
        amap["OffCmd"] = vm->setLinkVal(vmap, aname, "/controls", "OffCmd", bval);
        amap["StandbyCmd"] = vm->setLinkVal(vmap, aname, "/controls", "StandbyCmd", bval);
        amap["ResetFaultCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ResetFaultCmd", bval);
        amap["ResetCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ResetCmd", bval);
        amap["GridForming"] = vm->setLinkVal(vmap, aname, "/status", "GridForming", bval);
        amap["GridFollowing"] = vm->setLinkVal(vmap, aname, "/status", "GridFollowing", bval);
        amap["SystemState"] = vm->setLinkVal(vmap, aname, "/status", "SystemState", tVal);
        amap["SystemStateNum"] = vm->setLinkVal(vmap, aname, "/status", "SystemStateNum", ival);
        amap["AcContactor"] = vm->setLinkVal(vmap, "ess", "/status", "AcContactor", bval);
        amap["DcContactor"] = vm->setLinkVal(vmap, "ess", "/status", "DcContactor", bval);
        amap["AcContactorFbk"] = vm->setLinkVal(vmap, "ess", "/controls", "AcContactorFbk", bval);
        amap["DcContactorFbk"] = vm->setLinkVal(vmap, "ess", "/controls", "DcContactorFbk", bval);
        amap["AcContactorCmd"] = vm->setLinkVal(vmap, "ess", "/controls", "AcContactorCmd", bval);
        amap["DcContactorCmd"] = vm->setLinkVal(vmap, "ess", "/controls", "DcContactorCmd", bval);

        amap["CommsOk"] = vm->setLinkVal(vmap, aname, "/status", "CommsOk", bval);
        amap["HBOk"] = vm->setLinkVal(vmap, aname, "/status", "HBOk", bval);
        // amap["BMSOk"]             = vm->setLinkVal(vmap, "bms", "/status",    "BMSOk", bval);
        // amap["PCROk"]             = vm->setLinkVal(vmap, "pcr", "/status",    "PCROk", bval);
        // amap["DRCOk"]             = vm->setLinkVal(vmap, "drc", "/status",    "DRCOk", bval);
        // amap["EMMOk"]             = vm->setLinkVal(vmap, "emm", "/status",    "EMMOk", bval);

        if (reload == 0) // complete restart 
        {
            amap["SystemState"]->setVal(tVal);
            amap["On"]->setVal(false);
            amap["OnCmd"]->setVal(false);
            amap["Off"]->setVal(false);
            amap["OffCmd"]->setVal(false);
            amap["Standby"]->setVal(false);
            amap["StandbyCmd"]->setVal(false);
            amap["Fault"]->setVal(false);
            amap["ResetFaultCmd"]->setVal(false);
            amap["GridForming"]->setVal(false);
            amap["GridFollowing"]->setVal(false);
            amap["SystemState"]->setVal(tVal);
            amap["SystemStateNum"]->setVal((int)System_Init); // starts init timeout


            amap["CommsOk"]->setVal(true);
            amap["HBOk"]->setVal(true);
            // amap["BMSOk"]->setVal(true);
            // amap["PCROk"]->setVal(true);
            // amap["DRCOk"]->setVal(true);
            // amap["EMMOk"]->setVal(true);
             // DO any setup here    
        }
        reload = 2;
        HandleManCmd = amap["HandleManCmd"];
        HandleManCmd->setVal(reload);
    }

    char* fval;
    asprintf(&fval, " %s Manager State %d", aname, sstate);
    if (fval)
    {
        amap["SystemState"]->setVal(fval);
        free((void*)fval);
    }
    // for each Asset Manager turn it on
    for (auto ix : am->assetManMap)
    {
        asset_manager* amm = ix.second;
        HandleManagerCmd(vmap, amm->amap, amm->name.c_str(), p_fims, amm, sstate);
        if (0)FPS_ERROR_PRINT(" %s >> manager child [%s]  with %d kids \n", __func__, ix.first.c_str(), (int)amm->assetMap.size());
        // send each manager a wake up
    }
    for (auto ix : am->assetMap)
    {
        asset* ami = ix.second;
        HandleAssetCmd(vmap, ami->amap, ami->name.c_str(), p_fims, ami, sstate);

        if (0)FPS_ERROR_PRINT(" %s >> asset child [%s]  \n", __func__, ix.first.c_str());
    }
    // after assets have been done wake up for set manager
    for (auto ix : am->assetManMap)
    {
        asset_manager* amm = ix.second;

        amm->run_wakeup(amm, WAKE_LEVEL_MANAGE);
        //HandleManagerCmd(vmap, amm->amap, amm->name.c_str(), p_fims, amm, sstate);
        if (0)FPS_ERROR_PRINT(" %s >> manager child [%s]  with %d kids \n", __func__, ix.first.c_str(), (int)amm->assetMap.size());
        // send each manager a wake up
    }
    return 0;
}
// /**
//  * Phil's code ready for review
//  * 11/06/2020
//  * increments  a heart beat ... may need to add a period to this
//  * User to simulate asset heartbeats
//  *
//  * Used in:
//  * Test Script: test_sim_hb.sh
//  */
// int SimHandleHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
// {
//     int reload;
//     double dval = 0.0;
//     bool bval = false;
//     VarMapUtils* vm = am->vm;
//     char* tVal = (char*)"Test TimeStamp";

//     assetVar* SimHandleHeartBeat = amap["SimHandleHeartBeat"];
//     if (!SimHandleHeartBeat || (reload = SimHandleHeartBeat->getiVal()) == 0)
//     {
//         reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
//     }

//     if (reload < 2)
//     {
//         //reload = 0;
//         amap["SimHandleHeartBeat"] = vm->setLinkVal(vmap, aname,         "/reload",    "SimHandleHeartBeat", reload);
//         amap["HeartBeat"]          = vm->setLinkVal(vmap, aname,         "/status",    "HeartBeat", dval);
//         amap["Timestamp"]          = vm->setLinkVal(vmap, aname,         "/status",    "Timestamp", tVal);
//         amap["CommsDummy"]         = vm->setLinkVal(vmap, aname,         "/status",    "CommsDummy", dval);
//         amap["SimPcsComms"]        = vm->setLinkVal(vmap, aname,         "/configsim", "SimPcsComms", bval);
//         amap["SimPcsHB"]           = vm->setLinkVal(vmap, aname,         "/configsim", "SimPcsHB", bval);
//         amap["SimBmsComms"]        = vm->setLinkVal(vmap, aname,         "/configsim", "SimBmsComms", bval);
//         amap["SimBmsHB"]           = vm->setLinkVal(vmap, aname,         "/configsim", "SimBmsHB", bval);
//         amap["SimBms_1Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_1Comms", bval);
//         amap["SimBms_1HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_1HB", bval);
//         amap["SimBms_2Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_2Comms", bval);
//         amap["SimBms_2HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_2HB", bval);
//         amap["SimBms_3Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_3Comms", bval);
//         amap["SimBms_3HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_3HB", bval);
//         amap["SimBms_4Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_4Comms", bval);
//         amap["SimBms_4HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_4HB", bval);
//         dval = 1.0;
//         amap["HeartBeatPeriod"] = vm->setLinkVal(vmap, aname, "/config", "HeartBeatPeriod", dval);
//         dval = 255.0;
//         amap["HeartBeatMax"] = vm->setLinkVal(vmap, aname, "/config", "HeartBeatMax", dval);
//         // amap["HandleHeartBeat"]->setVal(2);  // revert reload
//         if (reload == 0) // complete restart 
//         {
//             amap["HeartBeat"]->setVal(0);
//         }
//         reload = 2;    amap["SimHandleHeartBeat"]->setVal(reload);
//     }
//     dval = vm->get_time_dbl();
//     asprintf(&tVal, "the new time is %f", dval);
//     amap["Timestamp"]->setVal(tVal);
//     free((void*)tVal);
//     if (amap["HeartBeat"]->getLastSetDiff(dval) > 1.0)
//     {
//         // get the reference to the variable 
//         assetVar* hb = amap["HeartBeat"];
//         assetVar* cd = amap["Timestamp"];
//         assetVar* hbmax = amap["HeartBeatMax"];

//         bool SimPcsComms = amap["SimPcsComms"]->getbVal();
//         bool SimPcsHB = amap["SimPcsHB"]->getbVal();
//         bool SimBmsComms = amap["SimBmsComms"]->getbVal();
//         bool SimBmsHB = amap["SimBmsHB"]->getbVal();
//         bool SimBms_1Comms = amap["SimBms_1Comms"]->getbVal();
//         bool SimBms_1HB = amap["SimBms_1HB"]->getbVal();
//         bool SimBms_2Comms = amap["SimBms_2Comms"]->getbVal();
//         bool SimBms_2HB = amap["SimBms_2HB"]->getbVal();
//         bool SimBms_3Comms = amap["SimBms_3Comms"]->getbVal();
//         bool SimBms_3HB = amap["SimBms_3HB"]->getbVal();
//         bool SimBms_4Comms = amap["SimBms_4Comms"]->getbVal();
//         bool SimBms_4HB = amap["SimBms_4HB"]->getbVal();

//         //double ival;
//         double dvalmax = hbmax->getdVal();
//         dval = hb->getdVal();
//         dval++;
//         if (dval > dvalmax) dval = 0;
//         //if(1)printf("HeartBeat %s val %f ", aname, dval);

//         hb->setVal(dval);
//         //cd->setVal(dval);
//         //dval = hb->getdVal();
//         if (0)printf("HeartBeat aname %s  val after set %f\n", aname, dval);
//         if (SimPcsComms)  vm->sendAssetVar(cd, p_fims, "/components/pcs_1");
//         if (SimPcsHB)     vm->sendAssetVar(hb, p_fims, "/components/pcs_1");

//         //"/components/catl_mbmu_stat_r:bms_heartbeat"

//         if (SimBmsComms)  vm->sendAssetVar(cd, p_fims, "/components/");
//         if (SimBmsHB)     vm->sendAssetVar(hb, p_fims, "/components/catl_mbmu_stat_r", "bms_heartbeat");
//         if (SimBms_1Comms)vm->sendAssetVar(cd, p_fims, "/components/sbmu_1");
//         if (SimBms_1HB)   vm->sendAssetVar(hb, p_fims, "/components/sbmu_1");
//         if (SimBms_2Comms)vm->sendAssetVar(cd, p_fims, "/components/sbmu_2");
//         if (SimBms_2HB)   vm->sendAssetVar(hb, p_fims, "/components/sbmu_2");
//         if (SimBms_3Comms)vm->sendAssetVar(cd, p_fims, "/components/sbmu_3");
//         if (SimBms_3HB)   vm->sendAssetVar(hb, p_fims, "/components/sbmu_3");
//         if (SimBms_4Comms)vm->sendAssetVar(cd, p_fims, "/components/sbmu_4");
//         if (SimBms_4HB)   vm->sendAssetVar(hb, p_fims, "/components/sbmu_4");
//     }
//     return dval;
// }
// inline input andler , one of our nice little features
// makes sure that later commands overwrite earlier ones
/**
 * Checks if the current input commands and selects the last one to arrive in the case of conflicts
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleESSInput(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // Turn on if conditions allow it
    //bool AcContactor     = amap["AcContactor"]->getbVal();
    //bool DcContactor     = amap["DcContactor"]->getbVal();
    bool AcContactorOpenCmd = amap["AcContactorOpenCmd"]->getbVal();
    bool DcContactorOpenCmd = amap["DcContactorOpenCmd"]->getbVal();
    bool AcContactorCloseCmd = amap["AcContactorCloseCmd"]->getbVal();
    bool DcContactorCloseCmd = amap["DcContactorCloseCmd"]->getbVal();
    bool OnCmd = amap["OnCmd"]->getbVal();
    bool OffCmd = amap["OffCmd"]->getbVal();
    bool StandbyCmd = amap["StandbyCmd"]->getbVal();
    //bool ResetCmd        = amap["ResetCmd"]->getbVal();
    //bool ResetFaultCmd   = amap["ResetFaultCmd"]->getbVal();
    bool readyOkSetCmd = amap["readyOkSetCmd"]->getbVal();
    bool readyOkClearCmd = amap["readyOkClearCmd"]->getbVal();
    // only allow one
    bool fval = false;
    if (0)
    {
        FPS_ERROR_PRINT("%s >> %s --- at Start AcContactorOpenCmd [%s] AcContactorCloseCmd [%s] DcContactorOpenCmd [%s] DcContactorCloseCmd [%s] \n"
            , __func__, aname
            , AcContactorOpenCmd ? "true" : "false"
            , AcContactorCloseCmd ? "true" : "false"
            , DcContactorOpenCmd ? "true" : "false"
            , DcContactorCloseCmd ? "true" : "false"
        );
        FPS_ERROR_PRINT("%s >> %s ---          OffCmd [%s] OnCmd [%s] StandbyCmd [%s] \n"
            , __func__, aname
            , OffCmd ? "true" : "false"
            , OnCmd ? "true" : "false"
            , StandbyCmd ? "true" : "false"
        );
        FPS_ERROR_PRINT("%s >> %s ---         readyOkSetCmd [%s] readyOkClearCmd [%s]\n"
            , __func__, aname
            , readyOkSetCmd ? "true" : "false"
            , readyOkClearCmd ? "true" : "false"
        );
    }

    if (AcContactorCloseCmd)
    {
        amap["AcContactorOpenCmd"]->setVal(fval);
        AcContactorOpenCmd = false;
    }
    if (AcContactorOpenCmd)
    {
        amap["AcContactorCloseCmd"]->setVal(fval);
    }
    if (DcContactorCloseCmd)
    {
        amap["DcContactorOpenCmd"]->setVal(fval);
        DcContactorOpenCmd = false;
    }
    if (DcContactorOpenCmd)
    {
        amap["DcContactorCloseCmd"]->setVal(fval);
    }
    if (OnCmd)
    {
        //amap["OnCmd"]->setVal(fval);
        amap["OffCmd"]->setVal(fval);
        amap["StandbyCmd"]->setVal(fval);
        OffCmd = false;
        StandbyCmd = false;
    }
    if (OffCmd)
    {
        amap["OnCmd"]->setVal(fval);
        //amap["OffCmd"]->setVal(fval);
        amap["StandbyCmd"]->setVal(fval);
        StandbyCmd = false;

    }
    if (StandbyCmd)
    {
        amap["OnCmd"]->setVal(fval);
        amap["OffCmd"]->setVal(fval);
        //amap["StandbyCmd"]->setVal(fval)
    }
    if (readyOkSetCmd)
    {
        amap["readyOkClearCmd"]->setVal(fval);
        //amap["StandbyCmd"]->setVal(fval)
        readyOkClearCmd = false;
    }
    if (readyOkClearCmd)
    {
        amap["readyOkSetCmd"]->setVal(fval);
        //amap["StandbyCmd"]->setVal(fval)
    }
    if (0)
    {
        AcContactorOpenCmd = amap["AcContactorOpenCmd"]->getbVal();
        DcContactorOpenCmd = amap["DcContactorOpenCmd"]->getbVal();
        AcContactorCloseCmd = amap["AcContactorCloseCmd"]->getbVal();
        DcContactorCloseCmd = amap["DcContactorCloseCmd"]->getbVal();
        OnCmd = amap["OnCmd"]->getbVal();
        OffCmd = amap["OffCmd"]->getbVal();
        StandbyCmd = amap["StandbyCmd"]->getbVal();
        //ResetCmd        = amap["ResetCmd"]->getbVal();
    //bool ResetFaultCmd   = amap["ResetFaultCmd"]->getbVal(ResetFaultCmd);
        readyOkSetCmd = amap["readyOkSetCmd"]->getbVal();
        readyOkClearCmd = amap["readyOkClearCmd"]->getbVal();

        FPS_ERROR_PRINT("%s >> %s --- at End AcContactorOpenCmd [%s] AcContactorCloseCmd [%s] DcContactorOpenCmd [%s] DcContactorCloseCmd [%s] \n"
            , __func__, aname
            , AcContactorOpenCmd ? "true" : "false"
            , AcContactorCloseCmd ? "true" : "false"
            , DcContactorOpenCmd ? "true" : "false"
            , DcContactorCloseCmd ? "true" : "false"
        );
        FPS_ERROR_PRINT("%s >> %s ---          OffCmd [%s] OnCmd [%s] StandbyCmd [%s] \n"
            , __func__, aname
            , OffCmd ? "true" : "false"
            , OnCmd ? "true" : "false"
            , StandbyCmd ? "true" : "false"
        );
        FPS_ERROR_PRINT("%s >> %s ---         readyOkSetCmd [%s] readyOkClearCmd [%s]\n"
            , __func__, aname
            , readyOkSetCmd ? "true" : "false"
            , readyOkClearCmd ? "true" : "false"
        );
    }

    return 0;
}
//To come out of init
// PCS comms must be OK
// Bms comms must have at least bms min connetions.
// Pcs Heatbeat must be OK
// Bms Hearbeat must be OK.
// We have SimPcsComms Bms_1Comms etc flags
// same for hearbeat
// then we need to see good status from the PCS system and BMS sysems
/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int CheckSystemState(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    vm->setTime();

    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);
    // char* fval;
    // asprintf(&fval,"Ess Init");
    // if(fval)
    // {
    //     amap["SystemState"]->setVal(fval);
    //     free((void*)fval);
    // }

    double respTime = 3.5;  // TODO put in a variable
    bool DcContactor = amap["DcContactor"]->getbVal();
    bool DcContactorFbk = amap["DcContactorFbk"]->getbVal();
    bool AcContactor = amap["AcContactor"]->getbVal();
    bool AcContactorFbk = amap["AcContactorFbk"]->getbVal();

    bool CommsOk = amap["CommsOk"]->getbVal();
    // TODO maybe just use SysOK in each subsystem...
    bool HBOk = amap["HBOk"]->getbVal();
    //bool BMSOk     = amap["BMSOk"]->getbVal();
    //bool PCROk     = amap["PCROk"]->getbVal();
    //bool DRCOk     = amap["DRCOk"]->getbVal();
    //bool EMMOk     = amap["EMMOk"]->getbVal();

    int  sState = amap["SystemStateNum"]->getiVal();
    int rc = 0;
    // TODO only do this after Setup
    // TODO process by AssetManager 
    if (
        (sState != System_Init)
        && (sState != System_Alarm)
        && (sState != System_Fault)
        )
    {
        double dval = vm->get_time_dbl();

        if ((DcContactor != DcContactorFbk) && ((dval - amap["DcContactor"]->getSetTime()) > respTime))
        {
            dval = vm->get_time_dbl();
            if (0)FPS_ERROR_PRINT("%s >> %s --- Running  statenum %d Dc [%s] DcFbk [%s] settime %f lastset %f\n"
                , __func__, aname, sState
                , DcContactor ? "true" : "false"
                , DcContactorFbk ? "true" : "false"
                , dval - amap["DcContactor"]->getSetTime()
                , amap["DcContactor"]->getLastSetDiff(dval)
            );

            sState = System_Fault;
            amap["SystemStateNum"]->setVal(sState);

            //TODO generate a alarm or error here 
            // char* fval; 
            // asprintf(&fval,"%s  DcContactor Did not respond in %f seconds", aname, dval);
            // if(fval)
            // {
            //     amap["SystemState"]->setVal(fval);
            //     free((void*)fval);
            // }
            rc++;
        }

        if ((AcContactor != AcContactorFbk) && ((dval - amap["AcContactor"]->getSetTime()) > respTime))
        {
            dval = vm->get_time_dbl();
            FPS_ERROR_PRINT("%s >> %s --- Running  statenum %d Ac [%s] AcFbk [%s] settime %f lastset %f\n"
                , __func__, aname, sState
                , AcContactor ? "true" : "false"
                , AcContactorFbk ? "true" : "false"
                , dval - amap["AcContactor"]->getSetTime()
                , amap["DcContactor"]->getLastSetDiff(dval)

            );

            sState = System_Fault;
            amap["SystemStateNum"]->setVal(sState);

            // char* fval; 
            // asprintf(&fval,"%s  AcContactor Did not respond in %f seconds", aname, dval);
            // if(fval)
            // {
            //     amap["SystemState"]->setVal(fval);
            //     free((void*)fval);
            // }
            rc++;
        }
    }
    // To BE reworked
    if (sState == System_Init)
    {
        if (!CommsOk)
        {
            // char* fval; 
            // asprintf(&fval," Ess Comms Failure");
            // if(fval)
            // {
            //     amap["SystemState"]->setVal(fval);
            //     free((void*)fval);
            // }
            rc++;
        }
        if (!HBOk)
        {
            // char* fval; 
            // asprintf(&fval," Ess HeartBeat Failure");
            // if(fval)
            // {
            //     amap["SystemState"]->setVal(fval);
            //     free((void*)fval);
            // }
            rc++;
        }
        // if(!BMSOk) 
        // {
        //     char* fval; 
        //     asprintf(&fval," Ess BMS Failure");
        //     if(fval)
        //     {
        //         amap["SystemState"]->setVal(fval);
        //         free((void*)fval);
        //     }
        //     rc++;
        // }

        // if(!PCROk) 
        // {
        //     char* fval; 
        //     asprintf(&fval," Ess BMS Failure");
        //     if(fval)
        //     {
        //         amap["SystemState"]->setVal(fval);
        //         free((void*)fval);
        //     }
        //     rc++;
        // }

        // if(!DRCOk) 
        // {
        //     char* fval; 
        //     asprintf(&fval," Ess BMS Failure");
        //     if(fval)
        //     {
        //         amap["SystemState"]->setVal(fval);
        //         free((void*)fval);
        //     }
        //     rc++;
        // }

        // if(!EMMOk) 
        // {
        //     char* fval; 
        //     asprintf(&fval," Ess EMM Failure");
        //     if(fval)
        //     {
        //         amap["SystemState"]->setVal(fval);
        //         free((void*)fval);
        //     }
        //     rc++;
        // }
    }
    if (rc == 0)
    {
        int ival;
        ival = amap["SystemStateNum"]->getiVal();
        // if we are in init or Fault transition to ready
        if ((ival == System_Init) || (ival == System_Fault))
        {
            // char* fval; 
            // asprintf(&fval,"Ess Init OK , Ready");
            // if(fval)
            // {
            //     //amap["SystemState"]->setVal(fval);
            //     free((void*)fval);
            // }
            if (0)
            {
                sState = System_Ready;
                amap["SystemStateNum"]->setVal(sState);
            }
        }
    }
    if (rc != 0)FPS_ERROR_PRINT("%s >> %s --- Done rc %d \n", __func__, aname, rc);

    return rc;
}

/**
 * Initial Fault Shutdown Function
 *
 * Review 11/06/2020
 *
 * Used in:
 * Test Script:
 */
int HandleFaultShutdown(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // Shutdown all PCS assets
    // Shutdown all BMS assets
    // OpenDcContactor
    //
    printf("%s >> %s --- Running\n", __func__, aname);
    char* fval;
    asprintf(&fval, " Ess Fault Shutdown");
    if (fval)
    {
        amap["SystemState"]->setVal(fval);
        free((void*)fval);
    }

    return 0;
}
/**
 * Handles the SystemOn state
 * substates are:
    "Waiting for DcContactorClosed"
    "Waiting for Current Setpoint"
    "Waiting for BMSStatus"
    "Waiting for PCSStatus"
    "Running GridFollowing"
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleSystemOn(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    //char* sVal2     =   amap["SystemState"]->getcVal();
    char* sValStep2 = amap["SystemStateStep"]->getcVal();
    double tNow = am->vm->get_time_dbl();
    // allow fall through
    if (strcmp(sValStep2, "Waiting for DcContactorClosed") == 0)
    {
        //TODO Need Timeout here
        // Head off to a Fault condition after 5 attempts perhaps
        bool DcContactor = amap["DcContactor"]->getbVal();

        if (DcContactor)
        {
            FPS_ERROR_PRINT("%s >> [%s] DCContactor closed at time %f Check Current Setpoint and SOC etc\n"
                , __func__
                , aname
                , tNow
            );

            sValStep2 = (char*)"Waiting for Current Setpoint";
            amap["SystemStateStep"]->setVal(sValStep2);
        }

    }
    // allow fall through
    if (strcmp(sValStep2, "Waiting for Current Setpoint") == 0)
    {
        double currentSetpoint = amap["CurrentSetpoint"]->getdVal();
        if (currentSetpoint > 0.0)
        {
            FPS_ERROR_PRINT("%s >> [%s] currentSetpoint set at %f at time %f Check Pcstatus feedback and SOC etc\n"
                , __func__
                , aname
                , currentSetpoint
                , tNow
            );

            // Send currentSetpoint command

            // TODO check For CloseOK
            // TODO check limits soc etc
            amap["PcsCurrentSetpoint"]->setVal(currentSetpoint);

            // TODO this is a HACK 
            am->vm->sendAssetVar(amap["PcsCurrentSetpoint"], p_fims);
            // TODO this is a HACK 
            int ival = 21; //PcsOn;
            amap["PcsCmd"]->setVal(ival);
            am->vm->sendAssetVar(amap["PcsCmd"], p_fims);
            sValStep2 = (char*)"Waiting for BMSStatus";
            amap["SystemStateStep"]->setVal(sValStep2);

        }
    }
    // allow fall through
    if (strcmp(sValStep2, "Waiting for BMSStatus") == 0)
    {
        int bmsStatus = amap["BmsStatus"]->getiVal();            // we can get faults
            // TODO this is a HACK 
        int bmsOkStatus = 1;
        // TODO this is a HACK 
        if (bmsStatus == bmsOkStatus)
        {
            FPS_ERROR_PRINT("%s >> [%s] BmsStatus %d at time %f move to check PCS Status, check SOC etc\n"
                , __func__
                , aname
                , bmsStatus
                , tNow
            );
            //TODO proper startp shutdow
            int ival = 41; //BmsOn;
            amap["BmsCmd"]->setVal(ival);
            am->vm->sendAssetVar(amap["BmsCmd"], p_fims);

            sValStep2 = (char*)"Waiting for PCSStatus";
            amap["SystemStateStep"]->setVal(sValStep2);
        }
    }
    // allow fall through
    if (strcmp(sValStep2, "Waiting for PCSStatus") == 0)
    {

        int pcsStatus = amap["PcsStatus"]->getiVal();            // we can get faults
            // TODO this is a HACK 
        int pcsOkStatus = 1;
        // TODO this is a HACK 
        if (pcsStatus == pcsOkStatus)
        {
            FPS_ERROR_PRINT("%s >> [%s] PcsStatus %d at time %f move to GridFollowing, check SOC etc\n"
                , __func__
                , aname
                , pcsStatus
                , tNow
            );

            sValStep2 = (char*)"Running GridFollowing";
            amap["SystemStateStep"]->setVal(sValStep2);

        }
        // we can get other commands
        // or we can stay here for ever

    }
    if (strcmp(sValStep2, "Running GridFollowing") == 0)
    {
        //CHECKESSSTATE should monitor
        int pcsStatus = amap["PcsStatus"]->getiVal();            // we can get faults
        int pcsOkStatus = 1;
        // TODO check SOC and capacity
        if (pcsStatus != pcsOkStatus)
        {
            FPS_ERROR_PRINT("%s >> [%s] PcsStatus %d at time %f move to System Fault, check SOC etc\n"
                , __func__
                , aname
                , pcsStatus
                , tNow
            );

            sValStep2 = (char*)"PCS Fault";
            amap["SystemStateStep"]->setVal(sValStep2);
            char* sVal2 = (char*)"System Fault";
            amap["SystemState"]->setVal(sVal2);

        }
    }
    return 0;
}

// we can get other commands
// or we can stay here for eve

// just run this and the commands should all fall into place ( ie crash)
// this waits  for coms and hartbeat  and then waits for the assets to get into a ready mode.
// only works for a cold start.
// for a woarm sart you dont want to impose a state on the assets.
// they may  have restarted.
// If it is a warm start look at the current state of the systems.
// tru to keep on trucking if it all looks good.
// we'll have the last requested sate and power command but that is for a different sprint.
// this guy only handles the cold start.
// so we force the assets into subordination.
// then wait for a command.
//
/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleSystemReady(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    char* sVal2 = amap["SystemState"]->getcVal();
    char* sValStep2 = amap["SystemStateStep"]->getcVal();
    double tNow = am->vm->get_time_dbl();
    //PowerModeSelect(vmap, amap, "pcs", am);
    if (strcmp(sValStep2, "Waiting for Command") == 0)
    {
        // from here we can go to "System On" or "System Standby" or "System Fault'"
        bool OnCmd = amap["OnCmd"]->getbVal();
        //On              = amap["On"]->getbVal();
        if (OnCmd)
        {
            FPS_ERROR_PRINT("%s >> [%s]Got On Cmd at time %f Check DcContactor and SOC etc\n"
                , __func__
                , aname
                , tNow
            );
            OnCmd = false;; amap["OnCmd"]->setVal(OnCmd);

            sValStep2 = (char*)"Waiting for DcContactorClosed";
            amap["SystemStateStep"]->setVal(sValStep2);
            sVal2 = (char*)"System On";
            amap["SystemState"]->setVal(sVal2);
            //sValStep2 =  (char *)"System On";
            //amap["SystemState"]->setVal(sValStep2);
            // Send the contactor Close command
            bool bval = true;
            amap["DcContactorCloseCmd"]->setVal(bval);

            // TODO check For CloseOK
            am->vm->sendAssetVar(amap["DcContactorCloseCmd"], p_fims);
            // Start Time out ( infacr use out neat command utility)
        }
        // TODO Handle reset standby clear faults etc...
    }
    return 0;
}

/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleSystemStartup(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();

    char* sVal2 = amap["SystemState"]->getcVal();
    char* sValStep2 = amap["SystemStateStep"]->getcVal();
    int ival5 = amap["CommsStateNum"]->getiVal();
    char* sVal5 = amap["CommsState"]->getcVal();
    char* sVal6 = amap["AssetState"]->getcVal();
    char* sVal4 = amap["HeartBeatState"]->getcVal();

    if (0)FPS_ERROR_PRINT("%s >> [%s] Looking for System Startup got [%s] Step [%s] CommsState [%s]  time %f\n"
        , __func__
        , aname
        , sVal2
        , sValStep2
        , sVal5 ? sVal5 : "No value in CommsState"
        // , ival5
        , tNow
    );

    if (0)FPS_ERROR_PRINT("  %s >> [%s] Found System Startup [%s]  Step [%s] CommsState [%s] HeartBeatState [%s] AssetState [%s] time %f\n"
        , __func__
        , aname
        , sVal2
        , sValStep2
        , sVal5 ? sVal5 : "No value in CommsState"
        , sVal4
        , sVal6
        , tNow
    );

    if (strcmp(sValStep2, "Waiting for Comms") == 0)
    {
        if (0)FPS_ERROR_PRINT("    %s >> [%s] Looking for CommsOK got %d at time %f\n"
            , __func__
            , aname
            , ival5
            , tNow
        );

        // if (ival5 == Asset_Ok)
        // {
        //     FPS_ERROR_PRINT("%s >> [%s] CommsOK looking for HeartBeat at time %f\n"
        //         , __func__
        //         , aname
        //         , tNow
        //         );
        //     sValStep2 =  (char *)"Waiting for HeartBeat";
        //     amap["SystemStateStep"]->setVal(sValStep2);
        //     return 0;
        // }
    }
    if (strcmp(sValStep2, "Waiting for HeartBeat") == 0)
    {
        int ival5 = amap["HeartBeatStateNum"]->getiVal();
        if (ival5 == Asset_Ok)
        {
            FPS_ERROR_PRINT("%s >> [%s] HeartBeat OK Reset Assets at time %f\n"
                , __func__
                , aname
                , tNow
            );
            FPS_ERROR_PRINT("%s >> send \"Assets Ready\" to /status/%s/AssetState to continue\n"
                , __func__
                , aname
            );
            sValStep2 = (char*)"Reset Assets";
            amap["SystemStateStep"]->setVal(sValStep2);
            return 0;

        }
    }
    if (strcmp(sValStep2, "Reset Assets") == 0)
    {
        char* sval5 = amap["AssetState"]->getcVal();
        if (strcmp(sval5, "Assets Ready") == 0)
        {
            FPS_ERROR_PRINT("%s >> [%s] Reset Assets State Moving to [System Ready]  at time %f\n"
                , __func__
                , aname
                , tNow
            );

            sVal2 = (char*)"System Ready";
            amap["SystemState"]->setVal(sVal2);
            sValStep2 = (char*)"Waiting for Command";
            amap["SystemStateStep"]->setVal(sValStep2);
            return 0;

        }
    }
    return 0;
}
/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
int HandleESSCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    int rc = 0;
    bool bval = false;
    int reload;
    char* tVal = (char*)" System Init";
    int ival = 0;
    double dval = 0.0;
    VarMapUtils* vm = am->vm;
    assetVar* HandleCmd = amap["HandleCmd"];

    if (!HandleCmd || (reload = HandleCmd->getiVal()) == 0)
    {
        reload = 0;
    }
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running reload %d\n", __func__, aname, reload);

    if (reload < 2)
    {
        //if(1)FPS_ERROR_PRINT("%s >> %s --- Reload\n", __func__, aname);

        amap["HandleCmd"] = vm->setLinkVal(vmap, aname, "/reload", "HandleCmd", reload);
        amap["On"] = vm->setLinkVal(vmap, aname, "/status", "On", bval);
        // amap["On2"]                = vm->setLinkVal(vmap, aname, "/status",   "On2",               bval);
        amap["Off"] = vm->setLinkVal(vmap, aname, "/status", "Off", bval);
        amap["Standby"] = vm->setLinkVal(vmap, aname, "/status", "Standby", bval);
        amap["Fault"] = vm->setLinkVal(vmap, aname, "/status", "Fault", bval);
        amap["ResetFaultCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ResetFaultCmd", bval);
        // amap["OnCmd"] = vm->setLinkVal(vmap, aname, "/controls", "OnCmd", bval);
        // amap["OffCmd"] = vm->setLinkVal(vmap, aname, "/controls", "OffCmd", bval);
        // amap["StandbyCmd"] = vm->setLinkVal(vmap, aname, "/controls", "StandbyCmd", bval);
        // amap["ResetFaultCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ResetFaultCmd", bval);
        linkVals(*vm, vmap, amap, aname, "/controls", bval, "OnCmd", "OffCmd", "StandbyCmd", "ResetFaultCmd");
        // amap["PMode"]             = vm->setLinkVal(vmap, "pcs", "/controls",  "ResetFaultCmd",     ival);
        // amap["ResetCmd"]          = vm->setLinkVal(vmap, aname, "/controls",  "ResetCmd",          bval);
        // amap["GridForming"]       = vm->setLinkVal(vmap, aname,   "/status",  "GridForming",     bval);
        // amap["GridFollowing"]     = vm->setLinkVal(vmap, aname,   "/status",  "GridFollowing",   bval);
        amap["SystemState"] = vm->setLinkVal(vmap, aname, "/status", "SystemState", tVal);
        amap["SystemStateNum"] = vm->setLinkVal(vmap, aname, "/status", "SystemStateNum", ival);
        amap["lastSystemStateNum"] = vm->setLinkVal(vmap, aname, "/status", "lastSystemStateNum", ival);
        amap["AcContactor"] = vm->setLinkVal(vmap, aname, "/status", "AcContactor", bval);
        amap["DcContactor"] = vm->setLinkVal(vmap, aname, "/status", "DcContactor", bval);
        amap["AssetStateNum"] = vm->setLinkVal(vmap, "ess", "/status", "AssetStateNum", ival);
        amap["CommsStateNum"] = vm->setLinkVal(vmap, "ess", "/status", "CommsStateNum", ival);
        amap["HeartBeatStateNum"] = vm->setLinkVal(vmap, "ess", "/status", "HeartBeatStateNum", ival);
        amap["CommsState"] = vm->setLinkVal(vmap, "ess", "/status", "CommsState", tVal);
        amap["AssetState"] = vm->setLinkVal(vmap, "ess", "/status", "AssetState", tVal);
        amap["HeartBeatState"] = vm->setLinkVal(vmap, "ess", "/status", "HeartBeatState", tVal);
        amap["SystemState"] = vm->setLinkVal(vmap, "ess", "/status", "SystemState", tVal);
        amap["SystemStateStep"] = vm->setLinkVal(vmap, "ess", "/status", "SystemStateStep", tVal);
        amap["CurrentSetpoint"] = vm->setLinkVal(vmap, "ess", "/controls", "CurrentSetpoint", dval);
        amap["PcsCurrentSetpoint"] = vm->setLinkVal(vmap, "ess", "/status", "PcsCurrentSetpoint", dval);

        amap["AcContactorCloseCmd"] = vm->setLinkVal(vmap, aname, "/controls", "AcContactorCloseCmd", bval);
        amap["DcContactorCloseCmd"] = vm->setLinkVal(vmap, aname, "/controls", "DcContactorCloseCmd", bval);
        amap["AcContactorOpenCmd"] = vm->setLinkVal(vmap, aname, "/controls", "AcContactorOpenCmd", bval);
        amap["DcContactorOpenCmd"] = vm->setLinkVal(vmap, aname, "/controls", "DcContactorOpenCmd", bval);

        // amap["CommsOk"]           = vm->setLinkVal(vmap, aname, "/status",    "CommsOk", bval);
        // amap["HBOk"]              = vm->setLinkVal(vmap, aname, "/status",    "HBOk", bval);
        // amap["BMSOk"]             = vm->setLinkVal(vmap, "bms", "/status",    "BMSOk", bval);
        // amap["PCROk"]             = vm->setLinkVal(vmap, "pcr", "/status",    "PCROk", bval);
        // amap["DRCOk"]             = vm->setLinkVal(vmap, "drc", "/status",    "DRCOk", bval);
        // amap["EMMOk"]             = vm->setLinkVal(vmap, "emm", "/status",    "EMMOk", bval);
        // amap["readyOk"]           = vm->setLinkVal(vmap, aname, "/status",    "readyOk", bval);
        amap["readyOkSetCmd"] = vm->setLinkVal(vmap, aname, "/controls", "readyOkSetCmd", bval);
        amap["readyOkClearCmd"] = vm->setLinkVal(vmap, aname, "/controls", "readyOkClearCmd", bval);
        amap["SimPCSCommsCmd"] = vm->setLinkVal(vmap, aname, "/controls", "SimPCSCommsCmd", bval);
        amap["SimPCSHBCmd"] = vm->setLinkVal(vmap, aname, "/controls", "SimPCSHBCmd", bval);
        amap["PcsCmd"] = vm->setLinkVal(vmap, aname, "/controls", "PcsCmd", ival);
        amap["PcsStatus"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatus", ival);
        amap["BmsCmd"] = vm->setLinkVal(vmap, aname, "/controls", "BmsCmd", ival);
        amap["BmsStatus"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatus", ival);

        if (!amap["RunESSInputFunc"])
        {

            assetFunc* runEssInputFunc = new assetFunc(aname);
            amap["RunESSInputFunc"] = (assetVar*)runEssInputFunc;
            runEssInputFunc->setupRamFunc(HandleESSInput, vmap, amap, aname, p_fims, am);
            amap["readyOkSetCmd"]        ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["readyOkClearCmd"]      ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["AcContactorCloseCmd"]  ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["AcContactorOpenCmd"]   ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["DcContactorCloseCmd"]  ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["DcContactorOpenCmd"]   ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["OnCmd"]                ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["OffCmd"]               ->SetPubFunc((assetVar*)runEssInputFunc);
            amap["StandbyCmd"]           ->SetPubFunc((assetVar*)runEssInputFunc);
        }

        if (reload == 0) // complete restart 
        {
            amap["SystemState"]->setVal(tVal);
            amap["On"]->setVal(false);
            amap["OnCmd"]->setVal(false);
            amap["Off"]->setVal(false);
            amap["OffCmd"]->setVal(false);
            amap["Standby"]->setVal(false);
            amap["StandbyCmd"]->setVal(false);
            amap["Fault"]->setVal(false);
            amap["ResetFaultCmd"]->setVal(false);
            //amap["GridForming"]->setVal(false);
            //amap["GridFollowing"]->setVal(false);
            amap["SystemState"]->setVal(tVal);
            amap["SystemStateNum"]->setVal((int)System_Init); // starts init timeout
            amap["lastSystemStateNum"]->setVal((int)System_Startup); // starts init timeout


            // amap["CommsOk"]->setVal(true);
            // amap["HBOk"]->setVal(true);
            //amap["BMSOk"]->setVal(true);
            //amap["PCROk"]->setVal(true);
            //amap["DRCOk"]->setVal(true);
            //amap["EMMOk"]->setVal(true);
            //amap["readyOk"]->setVal(false);
            HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemInit);
            const char* cVal = "System Startup";
            amap["SystemState"]->setVal(cVal);
            cVal = "Waiting for Comms";
            amap["SystemStateStep"]->setVal(cVal);
            cVal = "Assets Init";
            amap["AssetState"]->setVal(cVal);
        }
        else
        {
            HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemRestart);
        }

        reload = 2;
        HandleCmd = amap["HandleCmd"];
        HandleCmd->setVal(reload);
    }
    // CommsState and HeartBeatState must be set 

    bool OnCmd = amap["OnCmd"]->getbVal();
    //bool OffCmd          = amap["OffCmd"]->getbVal();
    //bool StandbyCmd      = amap["StandbyCmd"]->getbVal();
    //bool ResetCmd        = amap["ResetCmd"]->getbVal();
    //bool ResetFaultCmd   = amap["ResetFaultCmd"]->getbVal();
    //bool Offval = amap["Off"]->getbVal();

    bool On = amap["On"]->getbVal();

    //bool Off        = amap["Off"]->getbVal();
    bool Standby = amap["Standby"]->getbVal();
    //bool Fault      = amap["Fault"]->getbVal();

    bool AcContactor = amap["AcContactor"]->getbVal();
    bool DcContactor = amap["DcContactor"]->getbVal();
    //bool AcContactorOpenCmd   = amap["AcContactorOpenCmd"]->getbVal();
    //bool DcContactorOpenCmd   = amap["DcContactorOpenCmd"]->getbVal();
    //bool AcContactorCloseCmd  = amap["AcContactorCloseCmd"]->getbVal();
    //bool DcContactorCloseCmd  = amap["DcContactorCloseCmd"]->getbVal();

    //bool readyOk              = amap["readyOk"]->getbVal();
    //bool readyOkSetCmd        = amap["readyOkSetCmd"]->getbVal();
    //bool readyOkClearCmd      = amap["readyOkClearCmd"]->getbVal();

    rc = 0;

    dval = vm->get_time_dbl();

    char* cState = amap["SystemState"]->getcVal();
    int  sState = amap["SystemStateNum"]->getiVal();
    int  lastState = amap["lastSystemStateNum"]->getiVal();

    if (lastState != sState)
    {
        FPS_ERROR_PRINT("%s >> %s ---ssnum %d --> %d  [%s] OnCmd [%s] On [%s] last set time %f\n"
            , __func__
            , aname
            , lastState
            , sState
            , cState ? cState : "Undefined"
            , OnCmd ? "true" : "false"
            , On ? "true" : "false"
            , amap["SystemStateNum"]->getLastSetDiff(dval)
        );
        amap["lastSystemStateNum"]->setVal(sState);
    }

    // So if we are in System
    //amap["SystemState"]->setVal(cVal);
    //cVal = "Waiting For Comms";
    //amap["SystemStateStep"]->setVal(cVal);
    // TODO only do this after Setup
    // TODO process by AssetManager 
    // this will also set Ess:readyOK

    //SimHandleHeartComms(vmap, amap, aname, p_fims, am);
    SimHandleHeartbeat(vmap, amap, aname, p_fims, am);

    // int rcerr = CheckSystemState(vmap, amap, aname, p_fims, am);

    char* sVal2 = amap["SystemState"]->getcVal();
    char* sValStep2 = amap["SystemStateStep"]->getcVal();
    int ival5 = amap["CommsStateNum"]->getiVal();
    char* sVal5 = amap["CommsState"]->getcVal();
    //char *sVal6 = amap["AssetState"]->getcVal();
    //char *sVal4 = amap["HeartBeatState"]->getcVal();

    if (0)FPS_ERROR_PRINT("%s >> [%s] Looking for System Startup got [%s] Step [%s] CommsState [%s] num %d time %f \n"
        , __func__
        , aname
        , sVal2
        , sValStep2
        , sVal5 ? sVal5 : "No value in CommsState"
        , ival5
        , dval
        // , rcerr
    );

    if (0 && strcmp(sVal2, "System Startup") == 0)
    {
        HandleSystemStartup(vmap, amap, aname, p_fims, am);
        sVal2 = amap["SystemState"]->getcVal();
        sValStep2 = amap["SystemStateStep"]->getcVal();
    }
    if (1 || strcmp(sVal2, "System Ready") == 0)
    {
        HandleSystemReady(vmap, amap, aname, p_fims, am);
        sVal2 = amap["SystemState"]->getcVal();
        sValStep2 = amap["SystemStateStep"]->getcVal();
    }
    if (strcmp(sVal2, "System On") == 0)
    {
        HandleSystemOn(vmap, amap, aname, p_fims, am);
        sVal2 = amap["SystemState"]->getcVal();
        sValStep2 = amap["SystemStateStep"]->getcVal();
    }

    if (strcmp(sVal2, "System Fault") == 0)
    {

        sValStep2 = (char*)"Waiting for Fault Reset";
        amap["SystemStateStep"]->setVal(sValStep2);
        // from here we can go to "System On" or "System Standby" but we have to get the Fault Reset Command
        // from here we can go to "System On" or "System Standby" or "System Fault'"
        bool resetFaultCmd = amap["ResetFaultCmd"]->getbVal();

        if (resetFaultCmd)
        {
            resetFaultCmd = false;; amap["resetFaultCmd"]->setVal(resetFaultCmd);
            sValStep2 = (char*)"Waiting for Command";
            amap["SystemStateStep"]->setVal(sValStep2);
            sValStep2 = (char*)"System Ready";
            amap["SystemState"]->setVal(sValStep2);
        }
    }
    else
    {

        if (0)FPS_ERROR_PRINT("  %s >> [%s] State  [%s]  Step [%s] time %f\n"
            , __func__
            , aname
            , sVal2
            , sValStep2
            , dval
        );
    }

    AcContactor = amap["AcContactor"]->getbVal();
    DcContactor = amap["DcContactor"]->getbVal();
    //AcContactorOpenCmd  = amap["AcContactorOpenCmd"]->getbVal();
    //DcContactorOpenCmd  = amap["DcContactorOpenCmd"]->getbVal();
    //AcContactorCloseCmd  = amap["AcContactorCloseCmd"]->getbVal();
    //DcContactorCloseCmd  = amap["DcContactorCloseCmd"]->getbVal();
    OnCmd = amap["OnCmd"]->getbVal();
    On = amap["On"]->getbVal();
    bool readyOk = true;//         = amap["readyOk"]->getbVal();
    char* sVal = amap["SystemState"]->getcVal();
    if (0)FPS_ERROR_PRINT("%s >> %s -After Check  -- SystemState [%s] rc %d nCmd [%s] On [%s] Standby [%s] readyOk [%s] AcContactor [%s] DcContactor [%s] go  [%s] go2  [%s]\n"
        , __func__
        , aname
        , sVal ? sVal : "NotSet"
        , rc
        , OnCmd ? "true" : "false"
        , On ? "true" : "false"
        , Standby ? "true" : "false"
        , readyOk ? "true" : "false"
        , AcContactor ? "true" : "false"
        , DcContactor ? "true" : "false"
        , (OnCmd && !On) ? "Go" : "NoGo"
        , (OnCmd && (!On || Standby) && AcContactor && DcContactor) ? "Go" : "NoGo"
    );
    //if (rc != 0)
    if (0)   // Not Yet
    {
        if (1)FPS_ERROR_PRINT("%s >> %s -Running Fault Shutdown rc %d -- OnCmd [%s] On [%s] readyOk [%s] \n"
            , __func__
            , aname
            , rc
            , OnCmd ? "true" : "false"
            , On ? "true" : "false"
            , readyOk ? "true" : "false"
        );
        HandleFaultShutdown(vmap, amap, aname, p_fims, am);
        return 0;
    }
    rc = 0;

    return rc;
}

int InitPCSLinks(varsmap& vmap, varmap& amap, const char* aname, asset_manager* am = nullptr)
{
    int rc = 0;
    double dval;
    int ival;
    assetVar* initav = nullptr;
    VarMapUtils* vm = am->vm;

    ival = 2;
    initav = vm->setLinkVal(vmap, aname, "/controls", "ctrlword_pmode", ival);
    ival = 1;
    initav = vm->setLinkVal(vmap, aname, "/controls", "ctrlword_qmode", ival);
    dval = 10;
    initav = vm->setLinkVal(vmap, aname, "/controls", "PStartGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "PStopGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "PRiseGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "PDropGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "QStartGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "QStopGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "QRiseGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "QDropGradient", dval);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnPStartGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnPStopGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnPRiseGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnPDropGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnQStartGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnQStopGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnQRiseGradient", ival);
    initav = vm->setLinkVal(vmap, aname, "/controls", "EnQDropGradient", ival);
    if (0)FPS_ERROR_PRINT(" %s >> dummy initav %p\n", __func__, (void*)initav);

    return rc;
}