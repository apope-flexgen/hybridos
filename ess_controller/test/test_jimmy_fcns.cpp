/**
 * @file test_jimmy_fcns.cpp
 * @brief File that contains functions for testing (either modified functions from previous files like test_phil_fcns.cpp or new functions)
 * @version 0.1
 * @date 2020-12-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "asset.h"
#include "chrono_utils.hpp"

template <class T>
assetVar* getFeatAv(varsmap & vmap, varmap& amap, VarMapUtils* vm, const char* aname,  const char* item, const char* avName, assetFeatDict* aDict, T defval)
{
    assetVar* avItem = nullptr;
    std::string mvar = avName;
    std::string mvar2;
    char* mname;
    char* avStr;

    avItem = aDict->getFeat(item, &avItem);
    if(!avItem)
    {
        avStr = aDict->getFeat(item, &avStr);
        if(avStr)
        {
            char* tmp = strdup(avStr);
            char* suri = &tmp[4];  // skip past av::
            char* saname = nullptr;
            char* svar = nullptr;
            // Crappy Non Fault tolerant parser
            char* sep1 = strstr(&suri[1],"/");
            char* sep2 = nullptr; 
            if(sep1)
            {
                *sep1++ = 0;
                saname=sep1;
            
                sep2 = strstr(&saname[1],":");
                if(sep2)
                {
                    *sep2++ = 0;
                    svar=sep2;
                }
            }
            if(sep1 && sep2)
            {
                FPS_ERROR_PRINT("%s >> found item [%s] avStr [%s] uri[%s] saname [%s] svar [%s]\n"
                    , __func__
                    , item
                    , avStr
                    , suri
                    , saname
                    , svar
                    );

                // for example av::/status/ess:Errors
                //TODO create the assetVar and place it in aDict and amap
                avItem = amap[svar]         = vm->setLinkVal(vmap, saname,         suri, svar,         defval);
                free((void*)tmp);
                aDict->setFeat(item, avItem);
                return avItem;
            }
        }
    }

    if(avItem)  mname = (char*)avItem->name.c_str();
    else {mvar2 = mvar+"_"+ item; mname=(char*)mvar2.c_str();}
    amap[mname]         = vm->setLinkVal(vmap, aname,         "/params", mname,         defval);
    avItem = avItem?avItem:amap[mname];

    return avItem;
}


// Asset_Init = 0,
//     Asset_On ,
//     Asset_Off  ,
//     Asset_Standby,
//     Asset_Reset,
//     Asset_Alarm,
//     Asset_Fault,
//     Asset_Restart
// New comms test function using Params
// At the start state is CommsInit
// After the first time stamp change set CommsOK
// After missing for 0.5 secs set CommsAlarm
// After missing for 5.0  secs set CommsFalt
// If this is the ess_controller am->am == nullptr then just run the other managers
// Set System into Fault mode if we drop comms foir more that 5 seconds
// NOTE Ben wanted comms to be back for a reset time before coming out of fault.
// We'll add that in 
// if this is an asset manager report back to the controller 
// comms must be down for alarm time or fault time and recover for reset time

// int CheckAmComms(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval =  (char*) "Comms Init";
//     VarMapUtils* vm =am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckComms");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toAlarm = 0.5;
//     double toFault = 5.0;
//     double toReset = 2.5;
//     char* initTimestamp = (char *)" Initial Timestamp";

    
//     //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
//     if (reload < 2) 
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
//         amap["Timestamp"]            = vm->setLinkVal(vmap, aname,                "/status", "Timestamp",          initTimestamp);
//         if(1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
//                 , __func__
//                 , aname
//                 , amap["Timestamp"]->comp.c_str()
//                 , amap["Timestamp"]->name.c_str()
//                 );

//         amap["essCommsFaultCnt"]      = vm->setLinkVal(vmap, "ess",                "/errors",     "essCommsFaultCnt",     ival);
//         amap["essCommsAlarmCnt"]      = vm->setLinkVal(vmap, "ess",                "/errors",     "essCommsAlarmCnt",     ival);
//         amap["essCommsInit"]          = vm->setLinkVal(vmap, "ess",                "/status",     "essCommsInit",     ival);
//         amap["essCommsFaultTimeout"]  = vm->setLinkVal(vmap, "ess",                "/config",     "essCommsFaultTimeout",     toFault);
//         amap["essCommsAlarmTimeout"]  = vm->setLinkVal(vmap, "ess",                "/config",     "essCommsAlarmTimeout",     toAlarm);
//         amap["essCommsResetTimeout"]  = vm->setLinkVal(vmap, "ess",                "/config",     "essCommsResetTimeout",     toReset);

//         if(am->am)
//         {
//             amap["amCommsFaultCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsFaultCnt",         ival);
//             amap["amCommsAlarmCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsAlarmCnt",         ival);
//             amap["amCommsInit"]      = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsInit",           ival);
//         }

//         amap["CommsFaultCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "CommsFaultCnt",       ival);
//         amap["CommsAlarmCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "CommsAlarmCnt",       ival);
//         amap["CommsInit"]            = vm->setLinkVal(vmap, aname,                "/status",     "CommsInit",         ival);
//         amap["CommsState"]           = vm->setLinkVal(vmap, aname,                "/status",     "CommsState",        cval);
//         amap["BypassComms"]          = vm->setLinkVal(vmap, aname,                "/config",     "BypassComms",       bval);
//         amap["CommsStateNum"]        = vm->setLinkVal(vmap, aname,                "/status",     "CommsStateNum",      ival);
//         amap["CommsOK"]              = vm->setLinkVal(vmap, aname,                "/status",     "CommsOK",            bval);     

//         if(reload == 0) // complete restart 
//         {
//             amap["Timestamp"]     ->setVal(initTimestamp);
//             //lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
//             amap["Timestamp"]     ->setParam("lastTimestamp", initTimestamp);
//             amap["Timestamp"]     ->setParam("totalCommsFaults", 0);
//             amap["Timestamp"]     ->setParam("totalCommsAlarms", 0);
//             amap["Timestamp"]     ->setParam("seenFault", false);
//             amap["Timestamp"]     ->setParam("seenOk", false);
//             amap["Timestamp"]     ->setParam("seenAlarm", false);
//             amap["Timestamp"]     ->setParam("seenInit", false);
//             amap["Timestamp"]     ->setParam("initCnt", -1);

//             amap["Timestamp"]     ->setParam("rdFault", toFault);                      // time remaining before fault
//             amap["Timestamp"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
//             amap["Timestamp"]     ->setParam("rdReset", toReset);                      // time remaining before reset
//             amap["Timestamp"]     ->setParam("rdLast", dval);                         // time when last to event was seen

//             amap["CommsState"]    ->setVal(cval);
//             ival = Asset_Init; 
//             amap["CommsStateNum"]  ->setVal(ival);
//             ival = -1; amap["CommsInit"]  ->setVal(ival);
//             amap["BypassComms"]  ->setVal(false);

//             // if(!am->am)  // Nah do this in setLinkVals
//             // {
//             //     amap["essCommsTimeoutFault"] ->setVal(toFault);
//             //     amap["essCommsTimeoutAlarm"] ->setVal(toAlarm);
//             //     amap["essCommsTimeoutReset"] ->setVal(toReset);

//             // }
//         }
//         // reset reload
//         ival = 2; amap["CheckAmComms"]->setVal(ival);
//     }

//     double tNow = am->vm->get_time_dbl();

//     bool BypassComms = amap["BypassComms"]->getbVal();

//     toFault = amap["essCommsFaultTimeout"]->getdVal();
//     toAlarm = amap["essCommsAlarmTimeout"]->getdVal();
//     toReset = amap["essCommsResetTimeout"]->getdVal();

//     char* currentTimestamp = amap["Timestamp"]->getcVal();
//     char* lastTimestamp    = amap["Timestamp"]->getcParam("lastTimestamp");//amap["lastHeartBeat"]->getiVal();
//     // are we the ess_controller 
//     if(!am->am)
//     {
//         //bool initSeen =             amap["Timestamp"]     ->getbParam("initSeen");

//         amap["essCommsFaultCnt"]  ->setVal(0);
//         amap["essCommsAlarmCnt"]  ->setVal(0);
//         amap["essCommsInit"]    ->setVal(0);

//         int initCnt = amap["Timestamp"]->getiParam("initCnt");   
//         int icnt = 0;
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager * amc = ix.second;
//             CheckAmComms(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//             icnt++;
//         }

//         int essCommsFaults = amap["essCommsFaultCnt"]->getiVal();
//         int essCommsAlarms = amap["essCommsAlarmCnt"]->getiVal();
//         //int essCommsInit = amap["essCommsInit"]->getiVal();
//         if(essCommsFaults> 0)
//         {
//             FPS_ERROR_PRINT("%s >> %d essCommsFaults detected\n", __func__, essCommsFaults);

//             // TODO: add shutdown process here
//         }
//         if(essCommsAlarms> 0)
//         {
//             FPS_ERROR_PRINT("%s >> %d essCommsAlarms detected\n", __func__, essCommsAlarms);
//         }

//         if(initCnt  !=  icnt)
//         {
//             amap["Timestamp"]     ->setParam("initCnt", icnt);

//             FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
//         }
//         return 0;
//     }

//     if(BypassComms)
//     {
//         ival = 1;
//         amap["essCommsInit"]->addVal(ival);

//         // Set CommsOK here?
//         amap["CommsOK"]->setVal(true);
//         return 0;

//     }
//     // If we are in the init state wait for comms to start count down reset time
//     if (strcmp(currentTimestamp, initTimestamp)==0)    
//     {
//         bool seenInit = amap["Timestamp"]->getbParam("seenInit");

//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         if(0)FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n", __func__, aname, BypassComms?"true":"false");

//         // if not toally set up yet then quit this pass
//         if(!amap["amCommsInit"])
//         {
//             return 0;
//         }

//         if (!seenInit)   // Comms_Setup
//         {
//             amap["Timestamp"] ->setParam("seenInit",true);

//             char* cval = (char *)"Comms Init, no Timestamp Seen";
//             amap["CommsState"]->setVal(cval);

//             ival = 1;
//             amap["essCommsInit"]->addVal(ival);
//             amap["CommsInit"]->setVal(0);      //Comms_Init  
//         }
//         amap["Timestamp"] ->setParam("rdLast",tNow);

//     }
//     else  // wait for comms to go past reset then set active or wait to alarm and then fault
//     {
//         //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTimestamp?lastTimestamp:"no last Value", tval1);
//         double rdLast = amap["Timestamp"] ->getdParam("rdLast");
//         double rdFault = amap["Timestamp"] ->getdParam("rdFault");
//         double rdAlarm = amap["Timestamp"] ->getdParam("rdAlarm");
//         double rdReset = amap["Timestamp"] ->getdParam("rdReset");
//         amap["Timestamp"] ->setParam("rdLast",tNow);

//         double toVal  = amap["Timestamp"]->getLastSetDiff(tNow);

//         // Has value changed ? If yes then count down rdReset to zero based on tNow - rdLast
//         if (strcmp(currentTimestamp, lastTimestamp)!=0) 
//         //if(amap["Timestamp"]->valueChangedReset())
//         {
//             amap["Timestamp"]     ->setParam("lastTimestamp", currentTimestamp);

//             bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
//             if(rdReset > 0.0)
//             {
//                 rdReset -= (tNow - rdLast);
//                 rdReset = (rdReset >= 0.0) ? rdReset : 0.0;
//                 amap["Timestamp"] ->setParam("rdReset", rdReset);
//             }
//             else
//             {
//                 // TODO after reset increment these up to toAlarm
//                 if(rdAlarm < toAlarm)
//                 {
//                     rdAlarm += (tNow - rdLast);
//                     rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
//                     amap["Timestamp"] ->setParam("rdAlarm",rdAlarm);
//                 }
//                 if(rdFault < toFault)
//                 {
//                     rdFault += (tNow - rdLast);
//                     rdFault = rdFault <= toFault ? rdFault : toFault;
//                     amap["Timestamp"] ->setParam("rdFault",rdFault);
//                 }
//             }

//             if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault %2.3f\n"
//                 , __func__, aname, lastTimestamp?lastTimestamp:"no last Value", currentTimestamp, rdReset, (tNow - rdLast), rdAlarm, rdFault);

//             ival = amap["CommsStateNum"]->getiVal();
//             // reset time passed , still changing , time to switch to Comms_Ready
//             if(rdReset <= 0.0 && ival != seenOk)
//             {

//                 bool seenFault  = amap["Timestamp"]->getbParam("seenFault");
//                 //bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
//                 bool seenAlarm  = amap["Timestamp"]->getbParam("seenAlarm");

//                 if(0)FPS_ERROR_PRINT("%s >>  Timestamp  change for %s from [%s] to [%s] \n", __func__, aname, lastTimestamp?lastTimestamp:"no last Value", currentTimestamp);
//                 if(seenFault)
//                 {
//                     if(1)FPS_ERROR_PRINT("%s >>  Timestamp fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Timestamp"] ->setParam("seenFault",false);

//                 }
//                 if(seenAlarm)
//                 {
//                     if(1)FPS_ERROR_PRINT("%s >>  Timestamp Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Timestamp"] ->setParam("seenAlarm",false);

//                 }

//                 amap["Timestamp"] ->setParam("seenOk",true);
//                 seenOk = true;
//                 amap["CommsOK"]->setVal(true);

//                 if(1)FPS_ERROR_PRINT("%s >>  Timestamp OK for  %s at %2.3f\n", __func__, aname, tNow);
//                 ival = Asset_Ok; // seen Timestamp change
//                 amap["CommsStateNum"]  ->setVal(ival);
//                 ival = 0;
//                 amap["CommsInit"]->setVal(ival);
//                 char *tval;
//                 asprintf(&tval," Comms OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["CommsState"]->setVal(tval);
//                     free((void *)tval);
//                 }
//                 amap["Timestamp"]->setParam("rdReset", toReset);
//             }

//             // increment alarm and fault time
//             // This seems redundant. Fault/alarm time already incremented (see above)
//             // if(rdFault < toFault)
//             // {
//             //     rdFault += (tNow - rdLast);
//             //     rdFault = rdFault <= toFault ? rdFault : toFault;
//             //     amap["Timestamp"] ->setParam("rdFault",rdFault);
//             // }
//             // if(rdAlarm < toAlarm)
//             // {
//             //     rdAlarm += (tNow - rdLast);
//             //     rdFault = rdFault <= toFault ? rdFault : toFault;
//             //     rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
//             //     amap["Timestamp"] ->setParam("rdAlarm",rdAlarm);
//             // }
         
//             //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
//             amap["Timestamp"]     ->setParam("lastTimestamp",currentTimestamp);
//             //if ((toVal > toFault)  && !bokFaults && !bypass)

//         }
//         else   // No Change , start tracking faults and alarms
//         {
//             bool seenFault  = amap["Timestamp"]->getbParam("seenFault");
//             //bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
//             bool seenAlarm  = amap["Timestamp"]->getbParam("seenAlarm");
//             if(rdFault > 0.0)
//             {
//                 rdFault -= (tNow - rdLast);
//                 rdFault = rdFault >= 0.0 ? rdFault : 0.0;
//                 amap["Timestamp"]->setParam("rdFault",rdFault);
//             }
//             if(rdAlarm > 0.0)
//             {
//                 rdAlarm -= (tNow - rdLast);
//                 rdAlarm = rdAlarm >= 0.0 ? rdAlarm : 0.0;
//                 amap["Timestamp"]->setParam("rdAlarm",rdAlarm);
//             }
//             if(rdReset < toReset)
//             {
//                 rdReset += (tNow - rdLast);
//                 rdReset = rdReset < toReset ? rdReset : toReset;
//                 amap["Timestamp"]->setParam("rdReset",rdReset);
//             }

//             if(rdFault <= 0.0 && !seenFault)
//             {

//                 if(1)FPS_ERROR_PRINT("%s >>  Timestamp  Fault  for %s at %2.3f \n", __func__, aname, tNow);
//                 char *tval;
//                 asprintf(&tval," Comms Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["CommsState"]->setVal(tval);
//                     free((void *)tval);
//                 }
//                 int ival = 1 ; 
//                 amap["CommsFaultCnt"]->addVal(ival);
//                 amap["essCommsFaultCnt"]->addVal(ival);

//                 if(am->am)
//                 {
//                     amap["amCommsFaultCnt"]->addVal(ival);
//                 }

//                 ival = Asset_Fault; //Timestamp Fault
//                 amap["CommsStateNum"]  ->setVal(ival);

//                 seenFault = true;
//                 amap["Timestamp"]->setParam("seenFault", true);
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 amap["Timestamp"]->setParam("seenAlarm", true);
//                 //seenOk = false;
//                 seenAlarm =  false;

//                 int totalCommsFaults = amap["Timestamp"]->getiParam("totalCommsFaults");
//                 totalCommsFaults++;
//                 amap["Timestamp"]->setParam("totalCommsFaults",totalCommsFaults);

//                 amap["CommsOK"]->setVal(false);

//             }
//             else if ((rdAlarm <= 0.0)  && !seenAlarm)
//             {
//                 if(1)FPS_ERROR_PRINT("%s >>  Timestamp  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

//                 char *tval;
//                 asprintf(&tval,"Comms Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["CommsState"]->setVal(tval);
//                     free((void *)tval);
//                 }

//                 int ival = 1; 
//                 amap["CommsAlarmCnt"]->addVal(ival);
//                 amap["essCommsAlarmCnt"]->addVal(ival);

//                 if(am->am)
//                 {
//                     amap["amCommsAlarmCnt"]->addVal(ival);
//                 }
//                 ival = Asset_Alarm; //Timestamp Alarm
//                 amap["CommsStateNum"]  ->setVal(ival);

//                 amap["Timestamp"]->setParam("seenAlarm", true);
//                 //amap["Timestamp"]->setParam("seenFault", false);
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 int totalCommsAlarms = amap["Timestamp"]->getiParam("totalCommsAlarms");
//                 totalCommsAlarms++;
//                 amap["Timestamp"]->setParam("totalCommsAlarms",totalCommsAlarms);

//                 amap["CommsOK"]->setVal(false);
//             }
//             else
//             {
//                 if(0)FPS_ERROR_PRINT("%s >> Comms for [%s] [%s] Stalled at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
//                         , __func__
//                         , aname
//                         , amap["Timestamp"]->getcVal()
//                         , tNow 
//                         , rdReset, rdFault, rdAlarm);

//             }            
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
//     return 0;
// };
/* 
 *    After init we must get a continual heartbeat otherwise we alarm and then fault.
 *    the bms Heartbeat arrives on /components/catl_ems_bms_01_rw:ems_heartbeat,
 *    linked to /status/bms/Heartbeat  in bms_manager.json
 * Lets try again 
 *    base it off HBSeenTime 
 * at start HBSeenTime = 0.0 HBOk = false seenHB = false
 * if we see a change and !seenHB then set HBseenTime and set seenHBS
 * if HBseenTime == 0 we never have seen a HB  dont set faults or alarms yet 
 * if seenHB and (tNow - HBSeenTime) > toHold reset seenHB  
 * if seenHB and rdReset <=0.0  then set HBok clear errors else decrement rdReset 
 * if HBOk inc rdAlarm and rdFault to ther max
 * if !seenHB  and tNow - HBseenTime > rdAlarm then set Alarm
 * if !seenHB  and tNow - HBseenTime > rdFault then set Fault
 
 * toHold time to allow between heartbeat changes before worrying about it
 * toAlarm time after a stalled Heatbeat causes an Alarm
 * toFault time after a stalled Heatbeat causes a Fault
 *  toReset time after changes start being seen again before resetting faults and Alarms
 *
 */
// int CheckAmHeartbeat(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval =  (char*) "Heartbeat Init";
//     VarMapUtils* vm =am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckHeartbeat");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toHold = 1.5;  // Seconds between HB changes
//     double toAlarm = 2.5;
//     double toFault = 6.0;
//     double toReset = 2.5;
//     int initHeartbeat = -1;//(char *)" Initial Heartbeat";

    
//     //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
//     if (reload < 2) 
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
//         amap["Heartbeat"]            = vm->setLinkVal(vmap, aname,                "/status", "Heartbeat",          initHeartbeat);
//         if(1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
//                 , __func__
//                 , aname
//                 , amap["Heartbeat"]->comp.c_str()
//                 , amap["Heartbeat"]->name.c_str()
//                 );

//         amap["essHeartbeatFaultCnt"]     = vm->setLinkVal(vmap, "ess",                "/status",     "essHeartbeatFaultCnt",         ival);
//         amap["essHeartbeatAlarmCnt"]     = vm->setLinkVal(vmap, "ess",                "/status",     "essHeartbeatAlarmCnt",         ival);
//         amap["essHeartbeatInit"]         = vm->setLinkVal(vmap, "ess",                "/status",     "essHeartbeatInit",             ival);
//         amap["essHeartbeatFaultTimeout"] = vm->setLinkVal(vmap, "ess",                "/config",     "essHeartbeatFaultTimeout",     toFault);
//         amap["essHeartbeatAlarmTimeout"] = vm->setLinkVal(vmap, "ess",                "/config",     "essHeartbeatAlarmTimeout",     toAlarm);
//         amap["essHeartbeatResetTimeout"] = vm->setLinkVal(vmap, "ess",                "/config",     "essHeartbeatResetTimeout",     toReset);
//         amap["essHeartbeatHoldTimeout"]  = vm->setLinkVal(vmap, "ess",                "/config",     "essHeartbeatHoldTimeout",      toHold);

//         if(am->am)
//         {
//             amap["amHeartbeatFaultCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartbeatFaultCnt",             ival);
//             amap["amHeartbeatAlarmCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartbeatAlarmCnt",             ival);
//             amap["amHeartbeatInit"]      = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartbeatInit",                 ival);
//         }

//         amap["HeartbeatFaultCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatFaultCnt",            ival);
//         amap["HeartbeatAlarmCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatAlarmCnt",            ival);
//         amap["HeartbeatInit"]            = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatInit",                ival);
//         amap["HeartbeatState"]           = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatState",               cval);
//         amap["BypassHeartbeat"]          = vm->setLinkVal(vmap, aname,                "/config",     "BypassHeartbeat",              bval);
//         amap["HeartbeatStateNum"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatStateNum",            ival);
//         amap["HeartbeatOK"]              = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatOK",                  bval);

        
//         if(reload == 0) // complete restart 
//         {
//             amap["Heartbeat"]     ->setVal(initHeartbeat);
//             //lastHeartbeat=strdup(tsInit);//state"]->setVal(cval);
//             amap["Heartbeat"]     ->setParam("lastHeartbeat", initHeartbeat);
//             amap["Heartbeat"]     ->setParam("totalHbFaults", 0);
//             amap["Heartbeat"]     ->setParam("totalHbAlarms", 0);

//             amap["Heartbeat"]     ->setParam("seenFault", false);
//             amap["Heartbeat"]     ->setParam("seenAlarm", false);
//             amap["Heartbeat"]     ->setParam("seenOk", false);
//             amap["Heartbeat"]     ->setParam("seenHB", false);

//             //amap["Heartbeat"]     ->setParam("HBOk", false);
//             dval = 0.0;
//             amap["Heartbeat"]     ->setParam("HBseenTime", dval);
//             amap["Heartbeat"]     ->setParam("seenInit", false);
//             amap["Heartbeat"]     ->setParam("initCnt", -1);

//             amap["Heartbeat"]     ->setParam("rdFault", toFault);                      // time remaining before fault
//             amap["Heartbeat"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
//             amap["Heartbeat"]     ->setParam("rdReset", toReset);                      // time remaining before reset
//             //amap["Heartbeat"]     ->setParam("rdHold", toHold);                        // time to wait before no change test
//             amap["Heartbeat"]     ->setParam("tLast", dval);                         // time when last to event was seen

//             amap["HeartbeatState"]    ->setVal(cval);
//             ival = Asset_Init; amap["HeartbeatStateNum"]  ->setVal(ival);
//             ival = -1; amap["HeartbeatInit"]  ->setVal(ival);
//             amap["BypassHeartbeat"]  ->setVal(false);
            
//             amap["essHeartbeatFaultCnt"]->setParam("lastHbFaults",0);
//             amap["essHeartbeatAlarmCnt"]->setParam("lastHbAlarms",0);
//         }
//         // reset reload
//         ival = 2; amap["CheckAmHeartbeat"]->setVal(ival);
//     }

//     double tNow = am->vm->get_time_dbl();
//     double tLast    = amap["Heartbeat"]->getdParam("tLast");
//     if(tLast == 0.0)
//         tLast = tNow;
//     double tDiff = tNow - tLast;
//     amap["Heartbeat"]->setParam("tLast", tNow);
 
//     bool BypassHb = amap["BypassHeartbeat"]->getbVal();

//     toFault = amap["essHeartbeatFaultTimeout"]->getdVal();
//     toAlarm = amap["essHeartbeatAlarmTimeout"]->getdVal();
//     toReset = amap["essHeartbeatResetTimeout"]->getdVal();
//     toHold = amap["essHeartbeatHoldTimeout"]->getdVal();

//     int currentHeartbeat = amap["Heartbeat"]->getiVal();
//     int lastHeartbeat    = amap["Heartbeat"]->getiParam("lastHeartbeat");//amap["lastHeartBeat"]->getiVal();
//     // are we the ess_controller 
//     if(!am->am)
//     {
//         //bool initSeen =             amap["Heartbeat"]     ->getbParam("initSeen");

//         amap["essHeartbeatFaultCnt"]  ->setVal(0);
//         amap["essHeartbeatAlarmCnt"]  ->setVal(0);
//         amap["essHeartbeatInit"]    ->setVal(0);

//         int initCnt = amap["Heartbeat"]->getiParam("initCnt");   
//         int icnt = 0;
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager * amc = ix.second;
//             CheckAmHeartbeat(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//             icnt++;
//         }

//         int essHbFaults = amap["essHeartbeatFaultCnt"]->getiVal();
//         int essHbAlarms = amap["essHeartbeatAlarmCnt"]->getiVal();
//         int lastHbAlarms = amap["essHeartbeatAlarmCnt"]->getiParam("lastHbAlarms");
//         int lastHbFaults = amap["essHeartbeatFaultCnt"]->getiParam("lastHbFaults");
        
//         //int essHbInit = amap["essHbInit"]->getiVal();
//         if(essHbFaults != lastHbFaults)
//         {
//             amap["essHeartbeatFaultCnt"]->setParam("lastHbFaults",essHbFaults);

//             if(essHbFaults> 0) 
//             {
//                 FPS_ERROR_PRINT("%s >> %d essHbFaults detected at time %2.3f \n", __func__, essHbFaults, tNow);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s >> %d essHbFaults cleared at time %2.3f\n", __func__, essHbFaults, tNow);
//             }
//         }
//         if(essHbAlarms != lastHbAlarms)
//         {
//             amap["essHeartbeatAlarmCnt"]->setParam("lastHbAlarms",essHbAlarms);

//             if(essHbAlarms> 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essHbAlarms detected at time %2.3f \n", __func__, essHbAlarms, tNow);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s >> %d essHbAlarms cleared at time %2.3f\n", __func__, essHbAlarms, tNow);
//             }
//         }

//         if(initCnt  !=  icnt)
//         {
//             amap["Heartbeat"]     ->setParam("initCnt", icnt);

//             FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
//         }
//         return 0;
//     }


//     // this is the Asset Manager under the ess_controller instance
//     if(BypassHb)
//     {
//         ival = 1;
//         amap["essHeartbeatInit"]->addVal(ival);

//         // Do we set heartbeat OK here?
//         return 0;
//     }
//     double rdFault = amap["Heartbeat"] ->getdParam("rdFault");
//     double rdAlarm = amap["Heartbeat"] ->getdParam("rdAlarm");
//     double rdReset = amap["Heartbeat"] ->getdParam("rdReset");

//     double HBseenTime = amap["Heartbeat"] ->getdParam("HBseenTime");
//     //bool HBOk = amap["Heartbeat"] ->getbParam("HBOk");
//     bool seenHB = amap["Heartbeat"] ->getbParam("seenHB");
//     bool seenInit = amap["Heartbeat"]->getbParam("seenInit");
//     bool seenOk = amap["Heartbeat"]->getbParam("seenOk");
//     bool seenFault = amap["Heartbeat"]->getbParam("seenFault");
//     bool seenAlarm = amap["Heartbeat"]->getbParam("seenAlarm");
    
//     // If we are in the init state wait for comms to start count down reset time
//     if (currentHeartbeat == initHeartbeat)    
//     {
//         // if not toally set up yet then quit this pass
//         if(!amap["amHeartbeatInit"])
//         {
//             return 0;
//         }

//         if (!seenInit)   // Hb_Setup
//         {
//             if(1)FPS_ERROR_PRINT("%s >> %s  NO Heartbeat,  bypass [%s]\n", __func__, aname, BypassHb?"true":"false");

//             amap["Heartbeat"] ->setParam("seenInit",true);

//             char* cval = (char *)"Hb Init, no Heartbeat Seen";
//             amap["HeartbeatState"]->setVal(cval);

//             ival = 1;
//             amap["essHeartbeatInit"]->addVal(ival);
//             amap["HeartbeatInit"]->setVal(0);      //Hb_Init  
//         }

//     }
//     else  // wait for comms to go past reset then set active or wait to alarm and then fault
//     {
//         //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastHeartbeat?lastHeartbeat:"no last Value", tval1)
//         if (currentHeartbeat != lastHeartbeat) 
//         {

//             if(0)FPS_ERROR_PRINT("%s >> %s Heartbeat change detected,  from [%d] to [%d] tNow %2.3f seenHB [%s]\n"
//                 , __func__, aname, currentHeartbeat, lastHeartbeat, tNow, seenHB?"true":"false");

//             amap["Heartbeat"]     ->setParam("lastHeartbeat", currentHeartbeat);

//             //if(!seenHB)
//             {
//                 if(0)FPS_ERROR_PRINT("%s >> %s Heartbeat set HBseenTime %2.3f \n"
//                         , __func__, aname, tNow);
//                 amap["Heartbeat"] ->setParam("seenHB", true);
//                 amap["Heartbeat"] ->setParam("HBseenTime", tNow);
//                 HBseenTime = tNow;
//                 seenHB = true;
//             }

//         }
//         else   // No Change , start tracking faults and alarms  but wait for hold time
//         {
//             HBseenTime = amap["Heartbeat"] ->getdParam("HBseenTime");
//             // allow holdoff between testing for change
//             if(seenHB)
//             {
//                 if ((tNow - HBseenTime) > toHold)
//                 {
//                     if(0)FPS_ERROR_PRINT("%s >> %s Heartbeat stall detected  tNow %2.3f seebTime %2.3f .stalll time %2.3f toHold %2.3f \n"
//                             , __func__, aname, tNow, HBseenTime, (tNow - HBseenTime), toHold);

//                     amap["Heartbeat"] ->setParam("seenHB", false);
//                     seenHB = false;
//                     rdAlarm -= (tNow - HBseenTime);
//                     rdFault -= (tNow - HBseenTime);

//                     if (rdFault < 0.0)
//                     {
//                         rdFault = 0.0;
//                     }
//                     if (rdAlarm < 0.0)
//                     {
//                         rdAlarm = 0.0;
//                     }
//                     amap["Heartbeat"]->setParam("rdAlarm",rdAlarm);
//                     amap["Heartbeat"]->setParam("rdFault",rdFault);

//                 }
//             }
//         }

//         if(seenHB)
//         {
//             if(!seenOk)
//             {
//                 if(rdReset > 0.0)
//                 {
//                     rdReset -= tDiff;
//                     rdReset = rdReset >= 0.0 ? rdReset : 0.0;
//                     amap["Heartbeat"]->setParam("rdReset",rdReset);
//                 }
//             }

//             if (rdReset <= 0.0  && !seenOk)
//             {
//                 if(seenFault)
//                 {
//                     if(1)FPS_ERROR_PRINT("%s >>  Heartbeat fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Heartbeat"] ->setParam("seenFault",false);
//                     seenFault = false;
//                 }
//                 if(seenAlarm)
//                 {
//                     if(1)FPS_ERROR_PRINT("%s >>  Heartbeat Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Heartbeat"] ->setParam("seenAlarm",false);
//                     seenAlarm = false;
//                 }
//                 amap["Heartbeat"] ->setParam("seenOk",true);
//                 seenOk = true;
//                 //amap["Heartbeat"] ->setParam("HBOk",true);
//                 //HBOk = true;
//                 amap["HeartbeatOK"]->setVal(true);

//                 if(1)FPS_ERROR_PRINT("%s >>  Heartbeat OK for  %s at %2.3f\n", __func__, aname, tNow);
//                 ival = Asset_Ok; // seen Heartbeat change
//                 amap["HeartbeatStateNum"]  ->setVal(ival);
//                 ival = 0;
//                 amap["HeartbeatInit"]->setVal(ival);
//                 char *tval;
//                 asprintf(&tval," Hb OK last set  Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["HeartbeatState"]->setVal(tval);
//                     free((void *)tval);
//                 }   
//                 amap["Heartbeat"]->setParam("rdReset",toReset);             
//             }
//         }
//         else  // not changed not onHold look out for errors
//         {
//             // we need to decrement the alarm / fault times
//             rdFault = amap["Heartbeat"] ->getdParam("rdFault");
//             rdAlarm = amap["Heartbeat"] ->getdParam("rdAlarm");
//             seenFault = amap["Heartbeat"] ->getbParam("seenFault");
//             seenAlarm = amap["Heartbeat"] ->getbParam("seenAlarm");
//             if(0)FPS_ERROR_PRINT("%s >>  Heartbeat stall for  %s at %2.3f rdFault %2.3f rdAlarm %2.3f HBOk [%s] seenHB [%s] tDiff %2.3f \n"
//                                         , __func__, aname, tNow, rdFault, rdAlarm, amap["HeartbeaetOK"]?"true":"false", seenHB?"true":"false", tDiff );
//             if (rdFault > 0.0)
//             {
//                 rdFault -= tDiff;
//                 rdFault = rdFault > 0.0 ? rdFault : 0.0;
//                 amap["Heartbeat"]->setParam("rdFault",rdFault);
//             }
//             if (rdAlarm > 0.0)
//             {
//                 rdAlarm -= tDiff;
//                 rdAlarm = rdAlarm > 0.0 ? rdAlarm : 0.0;
//                 amap["Heartbeat"]->setParam("rdAlarm",rdAlarm);
//             }

//             if (rdFault < 0.0  && !seenFault)
//             {
//                 seenFault = true;
//                 amap["Heartbeat"]->setParam("seenFault", true);
//                 amap["Heartbeat"]->setParam("seenOk", false);
//                 amap["Heartbeat"]->setParam("seenAlarm", true);

//                 if(1)FPS_ERROR_PRINT("%s >>  Heartbeat  Fault  for %s at %2.3f \n", __func__, aname, tNow);
//                 char *tval;
//                 asprintf(&tval," Hb Fault last set Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["HeartbeatState"]->setVal(tval);
//                     free((void *)tval);
//                 }
//                 ival = Asset_Fault; //Heartbeat Fault
//                 amap["HeartbeatStateNum"]  ->setVal(ival);
//                 //seenOk = false;
//                 seenAlarm =  true;

//                 int totalHbFaults = amap["Heartbeat"]->getiParam("totalHbFaults");
//                 totalHbFaults++;
//                 amap["Heartbeat"]->setParam("totalHbFaults",totalHbFaults);

//                 //HBOk = false;
//                 amap["HeartbeatOK"]->setVal(false);
//                 //amap["Heartbeat"] ->setParam("HBOk",false);
            
//                 if(am->am)
//                 {
//                     ival = 1;
//                     amap["amHeartbeatFaultCnt"]->addVal(ival);
//                 }
                    
//             }
//             else if (rdAlarm < 0.0  && !seenAlarm)
//             {
//                 if(1)FPS_ERROR_PRINT("%s >> Heartbeat  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

//                 char *tval;
//                 asprintf(&tval,"Hb Alarm last set Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["HeartbeatState"]->setVal(tval);
//                     free((void *)tval);
//                 }
//                 // Just test code right now
//                 ival = Asset_Alarm; //Heartbeat Alarm
//                 amap["HeartbeatStateNum"]  ->setVal(ival);

//                 amap["Heartbeat"]->setParam("seenAlarm", true);
//                 seenAlarm = true;
//                 amap["Heartbeat"]->setParam("seenOk", false);
//                 int totalHbAlarms = amap["Heartbeat"]->getiParam("totalHbAlarms");
//                 totalHbAlarms++;
//                 amap["Heartbeat"]->setParam("totalHbAlarms",totalHbAlarms);

//                 //HBOk = false;
//                 amap["HeartbeatOK"]->setVal(false);
//                 //amap["Heartbeat"] ->setParam("HBOk",false);

//                 if(am->am)
//                 {
//                     amap["amHeartbeatAlarmCnt"]->addVal(ival);
//                 }
//             }
//             else
//             {
//                 if(0)FPS_ERROR_PRINT("%s >> Hb for [%s] [%s] Stalled at %2.3f  Fault %2.3f Alarm %2.3f \n"
//                         , __func__
//                         , aname
//                         , amap["Heartbeat"]->getcVal()
//                         , tNow 
//                         , rdFault, rdAlarm);

//             }    
//         }
//         if(seenFault)
//         {
//             int ival = 1 ; 
//             amap["HeartbeatFaultCnt"]->addVal(ival);
//             amap["essHeartbeatFaultCnt"]->addVal(ival);
//         }
//         else
//         {
//             if(seenHB)
//             {
//                 if (rdFault < toFault)
//                 {
//                     rdFault += tDiff;
//                     rdFault = rdFault <= toFault ? rdFault : toFault;
//                     amap["Heartbeat"]->setParam("rdFault",rdFault);
//                 }
//             }
//         }

//         if(seenAlarm)
//         {
//             int ival = 1 ; 
//             amap["HeartbeatAlarmCnt"]->addVal(ival);
//             amap["essHeartbeatAlarmCnt"]->addVal(ival);
//         }
//         else
//         {
//             if(seenHB)
//             {
//                 if (rdAlarm < toAlarm)
//                 {   
//                     rdAlarm += tDiff;
//                     rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
//                     amap["Heartbeat"]->setParam("rdAlarm",rdAlarm);
//                 }
//             }
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
//     return 0;
// };

// int CheckTimestamp(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval =  (char*) "Timestamp Init";
//     VarMapUtils* vm =am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckTimestamp");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toHold = 1.5;  // Seconds between TS changes
//     double toAlarm = 2.5;
//     double toFault = 6.0;
//     double toReset = 2.5;
//     char* initTimestamp = (char *)" Initial Timestamp";

    
//     //if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
//     if (reload < 2) 
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
//         amap["Timestamp"]            = vm->setLinkVal(vmap, aname,                "/status", "Timestamp",          initTimestamp);
//         if(1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
//                 , __func__
//                 , aname
//                 , amap["Timestamp"]->comp.c_str()
//                 , amap["Timestamp"]->name.c_str()
//                 );

//         amap["essTimestampFaultCnt"]       = vm->setLinkVal(vmap, "ess",                "/status",     "essTimestampFaultCnt",     ival);
//         amap["essTimestampAlarmCnt"]       = vm->setLinkVal(vmap, "ess",                "/status",     "essTimestampAlarmCnt",     ival);
//         amap["essTimestampInit"]         = vm->setLinkVal(vmap, "ess",                "/status",     "essTimestampInit",     ival);
//         amap["essTimestampFaultTimeout"] = vm->setLinkVal(vmap, "ess",                "/config",     "essTimestampFaultTimeout",     toFault);
//         amap["essTimestampAlarmTimeout"] = vm->setLinkVal(vmap, "ess",                "/config",     "essTimestampAlarmTimeout",     toAlarm);
//         amap["essTimestampResetTimeout"] = vm->setLinkVal(vmap, "ess",                "/config",     "essTimestampResetTimeout",     toReset);
//         amap["essTimestampHoldTimeout"]  = vm->setLinkVal(vmap, "ess",                "/config",     "essTimestampHoldTimeout",      toHold);

//         if(am->am)
//         {
//             amap["amTimestampFaultCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "TimestampFaultCnt",         ival);
//             amap["amTimestampAlarmCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "TimestampAlarmCnt",         ival);
//             amap["amTimestampInit"]    = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "TimestampInit",           ival);
//         }

//         amap["TimestampFaultCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "TimestampFaultCnt",       ival);
//         amap["TimestampAlarmCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "TimestampAlarmCnt",       ival);
//         amap["TimestampInit"]          = vm->setLinkVal(vmap, aname,                "/status",     "TimestampInit",         ival);
//         amap["TimestampState"]         = vm->setLinkVal(vmap, aname,                "/status",     "TimestampState",        cval);
//         amap["BypassTimestamp"]        = vm->setLinkVal(vmap, aname,                "/config",     "BypassTimestamp",       bval);
//         amap["AssetState"]         = vm->setLinkVal(vmap, aname,                "/status",     "AssetState",        ival);
//         amap["TimestampStateNum"]      = vm->setLinkVal(vmap, aname,                "/status",     "TimestampStateNum",      ival);

        
//         if(reload == 0) // complete restart 
//         {
//             amap["Timestamp"]     ->setVal(initTimestamp);
//             //lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
//             amap["Timestamp"]     ->setParam("lastTimestamp", initTimestamp);
//             amap["Timestamp"]     ->setParam("totalTsFaults", 0);
//             amap["Timestamp"]     ->setParam("totalTsAlarms", 0);

//             amap["Timestamp"]     ->setParam("seenFault", false);
//             amap["Timestamp"]     ->setParam("seenAlarm", false);
//             amap["Timestamp"]     ->setParam("seenOk", false);
//             amap["Timestamp"]     ->setParam("seenTS", false);

//             amap["Timestamp"]     ->setParam("TSOk", false);
//             dval = 0.0;
//             amap["Timestamp"]     ->setParam("TSseenTime", dval);
//             amap["Timestamp"]     ->setParam("seenInit", false);
//             amap["Timestamp"]     ->setParam("initCnt", -1);

//             amap["Timestamp"]     ->setParam("rdFault", toFault);                      // time remaining before fault
//             amap["Timestamp"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
//             amap["Timestamp"]     ->setParam("rdReset", toReset);                      // time remaining before reset
//             //amap["Timestamp"]     ->setParam("rdHold", toHold);                        // time to wait before no change test
//             amap["Timestamp"]     ->setParam("tLast", dval);                         // time when last to event was seen

//             amap["TimestampState"]    ->setVal(cval);
//             ival = Asset_Init; amap["TimestampStateNum"]  ->setVal(ival);
//             ival = -1; amap["TimestampInit"]  ->setVal(ival);
//             amap["BypassTimestamp"]  ->setVal(false);
            
//             amap["essTimestampFaultCnt"]->setParam("lastTsFaults",0);
//             amap["essTimestampAlarmCnt"]->setParam("lastTsAlarms",0);
//         }
//         // reset reload
//         ival = 2; amap["CheckAmTimestamp"]->setVal(ival);
//     }

//     double tNow = am->vm->get_time_dbl();
//     double tLast    = amap["Timestamp"]->getdParam("tLast");
//     if(tLast == 0.0)
//         tLast = tNow;
//     double tDiff = tNow - tLast;
//     amap["Timestamp"]->setParam("tLast", tNow);
 
//     bool BypassTs = amap["BypassTs"]->getbVal();

//     toFault = amap["essTimestampFaultTimeout"]->getdVal();
//     toAlarm = amap["essTimestampAlarmTimeout"]->getdVal();
//     toReset = amap["essTimestampResetTimeout"]->getdVal();
//     toHold  = amap["essTimestampHoldTimeout"]->getdVal();

//     char* currentTimestamp = amap["Timestamp"]->getcVal();
//     char* lastTimestamp    = amap["Timestamp"]->getcParam("lastTimestamp");//amap["lastTimestamp"]->getiVal();
//     // are we the ess_controller 
//     if(!am->am)
//     {
//         //bool initSeen =             amap["Timestamp"]     ->getbParam("initSeen");

//         amap["essTimestampFaultCnt"]  ->setVal(0);
//         amap["essTimestampAlarmCnt"]  ->setVal(0);
//         amap["essTimestampInit"]    ->setVal(0);

//         int initCnt = amap["Timestamp"]->getiParam("initCnt");   
//         int icnt = 0;
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager * amc = ix.second;
//             CheckTimestamp(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//             icnt++;
//         }

//         int essTsFaults = amap["essTimestampFaultCnt"]->getiVal();
//         int essTsAlarms = amap["essTimestampAlarmCnt"]->getiVal();
//         int lastTsAlarms = amap["essTimestampAlarmCnt"]->getiParam("lastTsAlarms");
//         int lastTsFaults = amap["essTimestampFaultCnt"]->getiParam("lastTsFaults");
        
//         //int essTsInit = amap["essTsInit"]->getiVal();
//         if(essTsFaults != lastTsFaults)
//         {
//             amap["essTimestampFaultCnt"]->setParam("lastTsFaults",essTsFaults);
//             if(essTsFaults> 0) 
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsFaults detected at time %2.3f \n", __func__, essTsFaults, tNow);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsFaults cleared at time %2.3f\n", __func__, essTsFaults, tNow);
//             }
//         }
//         if(essTsAlarms != lastTsAlarms)
//         {
//             amap["essTimestampAlarmCnt"]->setParam("lastTsAlarms",essTsAlarms);

//             if(essTsAlarms> 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsAlarms detected at time %2.3f \n", __func__, essTsAlarms, tNow);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsAlarms cleared at time %2.3f\n", __func__, essTsAlarms, tNow);
//             }
//         }

//         if(initCnt  !=  icnt)
//         {
//             amap["Timestamp"]     ->setParam("initCnt", icnt);

//             FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
//         }
//         return 0;
//     }


//     // this is the Asset Manager under the ess_controller instance
//     if(BypassTs)
//     {
//         ival = 1;
//         amap["essTimestampInit"]->addVal(ival);
//         return 0;
//     }
//     double rdFault = amap["Timestamp"] ->getdParam("rdFault");
//     double rdAlarm = amap["Timestamp"] ->getdParam("rdAlarm");
//     double rdReset = amap["Timestamp"] ->getdParam("rdReset");

//     double TSseenTime = amap["Timestamp"] ->getdParam("TSseenTime");
//     bool TSOk = amap["Timestamp"] ->getbParam("TSOk");
//     bool seenTS = amap["Timestamp"] ->getbParam("seenTS");
//     bool seenInit = amap["Timestamp"]->getbParam("seenInit");
//     bool seenOk = amap["Timestamp"]->getbParam("seenOk");
//     bool seenFault = amap["Timestamp"]->getbParam("seenFault");
//     bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");
    
//     // If we are in the init state wait for comms to start count down reset time
//     if (strcmp(currentTimestamp,initTimestamp)==0)    
//     {
//         // if not toally set up yet then quit this pass
//         if(!amap["amTimestampInit"])
//         {
//             return 0;
//         }

//         if (!seenInit)   // Ts_Setup
//         {
//             if(1)FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n", __func__, aname, BypassTs?"true":"false");

//             amap["Timestamp"] ->setParam("seenInit",true);

//             char* cval = (char *)"Ts Init, no Timestamp Seen";
//             amap["TimestampState"]->setVal(cval);

//             ival = 1;
//             amap["essTimestampInit"]->addVal(ival);
//             amap["TimestampInit"]->setVal(0);      //Ts_Init  
//         }

//     }
//     else  // wait for comms to go past reset then set active or wait to alarm and then fault
//     {
//         //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTimestamp?lastTimestamp:"no last Value", tval1)
//         if (strcmp(currentTimestamp, lastTimestamp)!=0) 
//         {

//             if(0)FPS_ERROR_PRINT("%s >> %s Timestamp change detected,  from [%s] to [%s] tNow %2.3f seenTS [%s]\n"
//                 , __func__, aname, currentTimestamp, lastTimestamp, tNow, seenTS?"true":"false");

//             amap["Timestamp"]     ->setParam("lastTimestamp", currentTimestamp);

//             //if(!seenTS)
//             {
//                 if(0)FPS_ERROR_PRINT("%s >> %s Timestamp set TSseenTime %2.3f \n"
//                         , __func__, aname, tNow);
//                 amap["Timestamp"] ->setParam("seenTS", true);
//                 amap["Timestamp"] ->setParam("TSseenTime", tNow);
//                 TSseenTime = tNow;
//                 seenTS = true;
//             }

//         }
//         else   // No Change , start tracking faults and alarms  but wait for hold time
//         {
//             TSseenTime = amap["Timestamp"] ->getdParam("TSseenTime");
//             // allow holdoff between testing for change
//             if(seenTS)
//             {
//                 if ((tNow - TSseenTime) > toHold)
//                 {
//                     if(0)FPS_ERROR_PRINT("%s >> %s Timestamp stall detected  tNow %2.3f seebTime %2.3f .stalll time %2.3f toHold %2.3f \n"
//                             , __func__, aname, tNow, TSseenTime, (tNow - TSseenTime), toHold);

//                     amap["Timestamp"] ->setParam("seenTS", false);
//                     seenTS = false;
//                     rdAlarm -= (tNow-TSseenTime);
//                     rdFault -= (tNow-TSseenTime);

//                     if (rdFault < 0.0)
//                     {
//                         rdFault = 0.0;;
//                     }
//                     if (rdAlarm < 0.0)
//                     {
//                         rdAlarm = 0.0;;
//                     }
//                     amap["Timestamp"]->setParam("rdAlarm",rdAlarm);
//                     amap["Timestamp"]->setParam("rdFault",rdFault);

//                 }
//             }
//         }

//         if(seenTS)
//         {
//             if(!seenOk)
//             {
//                 if(rdReset > 0.0)
//                 {
//                     rdReset -= tDiff;
//                     amap["Timestamp"]->setParam("rdReset",rdReset);
//                 }
//             }

//             if ((rdReset<= 0.0)  && !seenOk)
//             {
//                 if(seenFault)
//                 {
//                     if(1)FPS_ERROR_PRINT("%s >>  Timestamp fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Timestamp"] ->setParam("seenFault",false);
//                     seenFault = false;
//                 }
//                 if(seenAlarm)
//                 {
//                     if(1)FPS_ERROR_PRINT("%s >>  Timestamp Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Timestamp"] ->setParam("seenAlarm",false);
//                     seenAlarm = false;
//                 }
//                 amap["Timestamp"] ->setParam("seenOk",true);
//                 seenOk = true;
//                 amap["Timestamp"] ->setParam("TSOk",true);
//                 TSOk = true;

//                 if(1)FPS_ERROR_PRINT("%s >>  Timestamp OK for  %s at %2.3f\n", __func__, aname, tNow);
//                 ival = Asset_Ok; // seen Timestamp change
//                 amap["TimestampStateNum"]  ->setVal(ival);
//                 ival = 0;
//                 amap["TimestampInit"]->setVal(ival);
//                 char *tval;
//                 asprintf(&tval," Ts OK last set  Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["TimestampState"]->setVal(tval);
//                     free((void *)tval);
//                 }   
//                 amap["Timestamp"]->setParam("rdReset",toReset);             
//             }
//         }
//         else  // not changed not onHold look out for errors
//         {
//             // we need to decrement the alarm / fault times
//             rdFault = amap["Timestamp"] ->getdParam("rdFault");
//             rdAlarm = amap["Timestamp"] ->getdParam("rdAlarm");
//             seenFault = amap["Timestamp"] ->getbParam("seenFault");
//             seenAlarm = amap["Timestamp"] ->getbParam("seenAlarm");
//             if(0)FPS_ERROR_PRINT("%s >>  Timestamp stall for  %s at %2.3f rdFault %2.3f rdAlarm %2.3f TSOk [%s] seenTS [%s] tDiff %2.3f \n"
//                                         , __func__, aname, tNow, rdFault, rdAlarm, TSOk?"true":"false", seenTS?"true":"false", tDiff );
//             if (rdFault > 0.0)
//             {
//                 rdFault -= tDiff;
//                 amap["Timestamp"]->setParam("rdFault",rdFault);
//             }
//             if (rdAlarm > 0.0)
//             {
//                 rdAlarm -= tDiff;
//                 amap["Timestamp"]->setParam("rdAlarm",rdAlarm);
//             }

//             if ((rdFault< 0.0)  && !seenFault)
//             {
//                 seenFault = true;
//                 amap["Timestamp"]->setParam("seenFault", true);
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 amap["Timestamp"]->setParam("seenAlarm", true);

//                 if(1)FPS_ERROR_PRINT("%s >>  Timestamp  Fault  for %s at %2.3f \n", __func__, aname, tNow);
//                 char *tval;
//                 asprintf(&tval," Ts Fault last set Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["TimestampState"]->setVal(tval);
//                     free((void *)tval);
//                 }
//                 ival = Asset_Fault; //Timestamp Fault
//                 amap["TimestampStateNum"]  ->setVal(ival);
//                 //seenOk = false;
//                 seenAlarm =  true;

//                 int totalTsFaults = amap["Timestamp"]->getiParam("totalTsFaults");
//                 totalTsFaults++;
//                 amap["Timestamp"]->setParam("totalTsFaults",totalTsFaults);

//                 TSOk = false;
//                 amap["Timestamp"] ->setParam("TSOk",false);
            
//                 if(am->am)
//                 {
//                     ival = 1;
//                     amap["amTimestampFaultCnt"]->addVal(ival);
//                 }
                    
//             }
//             else if ((rdAlarm < 0.0)  && !seenAlarm)
//             {
//                 if(1)FPS_ERROR_PRINT("%s >> Timestamp  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

//                 char *tval;
//                 asprintf(&tval,"Ts Alarm last set Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if(tval)
//                 {
//                     amap["TimestampState"]->setVal(tval);
//                     free((void *)tval);
//                 }
//                 // Just test code right now
//                 ival = Asset_Alarm; //Timestamp Alarm
//                 amap["TimestampStateNum"]  ->setVal(ival);

//                 amap["Timestamp"]->setParam("seenAlarm", true);
//                 seenAlarm = true;
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 int totalTsAlarms = amap["Timestamp"]->getiParam("totalTsAlarms");
//                 totalTsAlarms++;
//                 amap["Timestamp"]->setParam("totalTsAlarms",totalTsAlarms);

//                 TSOk = false;
//                 amap["Timestamp"] ->setParam("TSOk",false);

//                 if(am->am)
//                 {
//                     amap["amTimestampAlarmCnt"]->addVal(ival);
//                 }
//             }
//             else
//             {
//                 if(0)FPS_ERROR_PRINT("%s >> Ts for [%s] [%s] Stalled at %2.3f  Fault %2.3f Alarm %2.3f \n"
//                         , __func__
//                         , aname
//                         , amap["Timestamp"]->getcVal()
//                         , tNow 
//                         , rdFault, rdAlarm);

//             }    
//         }
//         if(seenFault)
//         {
//             int ival = 1 ; 
//             amap["TimestampFaultCnt"]->addVal(ival);
//             amap["essTimestampFaultCnt"]->addVal(ival);
//         }
//         else
//         {
//             if(seenTS)
//             {
//                 if (rdFault < toFault)
//                 {
//                     rdFault += tDiff;
//                     amap["Timestamp"]->setParam("rdFault",rdFault);
//                 }
//             }
//         }

//         if(seenAlarm)
//         {
//             int ival = 1 ; 
//             amap["TimestampAlarmCnt"]->addVal(ival);
//             amap["essTimestampAlarmCnt"]->addVal(ival);
//         }
//         else
//         {
//             if(seenTS)
//             {
//                 if (rdAlarm < toAlarm)
//                 {   
//                     rdAlarm += tDiff;
//                     amap["Timestamp"]->setParam("rdAlarm",rdAlarm);
//                 }
//             }
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
//     return 0;
// };

// test status against ExpStatus 
// logs any changes
int CheckPCSStatus(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)
{
    //double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval =  (char*) "PcsStatus Init";
    VarMapUtils* vm =am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckPCSStatus", (void*)&CheckPCSStatus);
    //assetVar* CheckAssetComms = amap["CheckAmComms"];
    double toAlarm = 2.5;
    double toFault = 10.0;
    double toReset = 2.5;
    int initPcsStatus = -1;//(char *)" Initial PcsStatus";

    
    //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2) 
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;
        //Link This to an incoming component
        if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
        amap["PCSStatus"]            = vm->setLinkVal(vmap, aname,                "/status", "PCSStatus",          initPcsStatus);
        amap["PCSExpStatus"]         = vm->setLinkVal(vmap, aname,                "/status", "PCSExpStatus",       initPcsStatus);
        if(1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
                , __func__
                , aname
                , amap["PcsStatus"]->comp.c_str()
                , amap["PcsStatus"]->name.c_str()
                );

        amap["essPCSStatusFaultCnt"]       = vm->setLinkVal(vmap, "ess",            "/status",     "essPCSStatusFaultCnt",         ival);
        amap["essPCSStatusAlarmCnt"]       = vm->setLinkVal(vmap, "ess",            "/status",     "essPCSStatusAlarmCnt",         ival);
        amap["essPCSStatusInit"]           = vm->setLinkVal(vmap, "ess",            "/status",     "essPCSStatusInit",             ival);
        amap["essPCSStatusFaultTimeout"]   = vm->setLinkVal(vmap, "ess",            "/config",     "essPCSStatusFaultTimeout",     toFault);
        amap["essPCSStatusAlarmTimeout"]   = vm->setLinkVal(vmap, "ess",            "/config",     "essPCSStatusAlarmTimeout",     toAlarm);
        amap["essPCSStatusResetTimeout"]   = vm->setLinkVal(vmap, "ess",            "/config",     "essPCSStatusResetTimeout",     toReset);

        if(am->am)
        {
            amap["amPCSStatusFaults"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "PCSStatusFaults",         ival);
            amap["amPCSStatusAlarms"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "PCSStatusAlarms",         ival);
            amap["amPCSStatusInit"]    = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "PCSStatusInit",           ival);
        }

        amap["PCSStatusFaultCnt"]      = vm->setLinkVal(vmap, aname,                "/status",     "PCSStatusFaultCnt",      ival);
        amap["PCSStatusAlarmCmt"]      = vm->setLinkVal(vmap, aname,                "/status",     "PCSStatusAlarmCnt",      ival);
        amap["PCSStatusInit"]          = vm->setLinkVal(vmap, aname,                "/status",     "PCSStatusInit",          ival);
        amap["PCSStatusState"]         = vm->setLinkVal(vmap, aname,                "/status",     "PCSStatusState",         cval);
        amap["BypassPCSStatus"]        = vm->setLinkVal(vmap, aname,                "/config",     "BypassPCSStatus",        bval);
        //amap["AssetState"]             = vm->setLinkVal(vmap, aname,                "/status",     "AssetState",             ival);
        amap["PCSStatusStateNum"]      = vm->setLinkVal(vmap, aname,                "/status",     "PCSStatusStateNum",      ival);
        amap["StatusOK"]               = vm->setLinkVal(vmap, aname,                "/status",     "StatusOK",               bval);

        
        if(reload == 0) // complete restart 
        {
            amap["PCSStatus"]     ->setVal(initPcsStatus);
            //lastPcsStatus=strdup(tsInit);//state"]->setVal(cval);
            amap["PCSStatus"]     ->setParam("lastPcsStatus", initPcsStatus);
            amap["PCSStatus"]     ->setParam("totalPcsStatusFaults", 0);
            amap["PCSStatus"]     ->setParam("totalPcsStatusAlarms", 0);
            amap["PCSStatus"]     ->setParam("seenFault", false);
            amap["PCSStatus"]     ->setParam("seenOk", false);
            amap["PCSStatus"]     ->setParam("seenAlarm", false);
            amap["PCSStatus"]     ->setParam("seenInit", false);
            amap["PCSStatus"]     ->setParam("initCnt", -1);

            amap["PCSStatus"]     ->setParam("rdFault", toFault);                      // time remaining before fault
            amap["PCSStatus"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["PCSStatus"]     ->setParam("rdReset", toReset);                      // time remaining before reset
            amap["PCSStatus"]     ->setParam("rdLast", dval);                         // time when last to event was seen

            amap["PCSStatusState"]    ->setVal(cval);
            ival = Asset_Init; amap["PCSStatusStateNum"]  ->setVal(ival);
            ival = -1; amap["PCSStatusInit"]  ->setVal(ival);
            amap["BypassPCSStatus"]  ->setVal(false);

        }
        // reset reload
        ival = 2; amap["CheckPCSStatus"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();

    bool BypassPcsStatus = amap["BypassPCSStatus"]->getbVal();

    toFault = amap["essPCSStatusFaultTimeout"]->getdVal();
    toAlarm = amap["essPCSStatusAlarmTimeout"]->getdVal();
    toReset = amap["essPCSStatusResetTimeout"]->getdVal();

    int currentPcsStatus = amap["PCSStatus"]->getiVal();
    int expectedPcsStatus = amap["PCSExpStatus"]->getiVal();
    int lastPcsStatus    = amap["PCSStatus"]->getiParam("lastPcsStatus");//amap["lastHeartBeat"]->getiVal();

    if(BypassPcsStatus)
    {
        ival = 1;
        amap["essPCSStatusInit"]->addVal(ival);

        // Do we set status OK here?
        amap["StatusOK"]->setVal(true);
        return 0;

    }
    // If we are in the init state wait for comms to start count down reset time
    if (currentPcsStatus == initPcsStatus)    
    {
        bool seenInit = amap["PCSStatus"]->getbParam("seenInit");

        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        if(0)FPS_ERROR_PRINT("%s >> %s  NO PcsStatus,  bypass [%s]\n", __func__, aname, BypassPcsStatus?"true":"false");

        // if not toally set up yet then quit this pass
        if(!amap["amPCSStatusInit"])
        {
            return 0;
        }

        if (!seenInit)   // PcsStatus_Setup
        {
            amap["PCSStatus"] ->setParam("seenInit",true);

            char* cval = (char *)"PcsStatus Init, no PcsStatus Seen";
            amap["PCSStatusState"]->setVal(cval);

            ival = 1;
            amap["essPCSStatusInit"]->addVal(ival);
            amap["PCSStatusInit"]->setVal(0);      //PcsStatus_Init  
        }
        amap["PCSStatus"] ->setParam("rdLast",tNow);

    }
    else  // wait for comms to go past reset then set active or wait to alarm and then fault
    {
        //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastPcsStatus?lastPcsStatus:"no last Value", tval1);
        double rdLast = amap["PCSStatus"] ->getdParam("rdLast");
        double rdFault = amap["PCSStatus"] ->getdParam("rdFault");
        double rdAlarm = amap["PCSStatus"] ->getdParam("rdAlarm");
        double rdReset = amap["PCSStatus"] ->getdParam("rdReset");
        amap["PCSStatus"] ->setParam("rdLast",tNow);

        double toVal  = amap["PCSStatus"]->getLastSetDiff(tNow);

        // Has value changed ? If yes then count down rdReset to zero based on tNow - rdLast
        if (currentPcsStatus != lastPcsStatus) 
        {
            amap["PCSStatus"]     ->setParam("lastPcsStatus", currentPcsStatus);
            // TODO log changes
        }
        if (currentPcsStatus == expectedPcsStatus) 
        //if(amap["PcsStatus"]->valueChangedReset())
        {
            amap["PCSStatus"]     ->setParam("lastPcsStatus", currentPcsStatus);

            bool seenOk  = amap["PCSStatus"]->getbParam("seenOk");
            if(rdReset> 0.0)
            {
                rdReset -= (tNow - rdLast);
                rdReset = rdReset >= 0.0 ? rdReset : 0.0;
                amap["PCSStatus"] ->setParam("rdReset", rdReset);
            }
            else
            {
                // TODO after reset increment these up to toAlarm
                if(rdAlarm < toAlarm)
                {
                    rdAlarm += (tNow - rdLast);
                    rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
                    amap["PCSStatus"] ->setParam("rdAlarm",rdAlarm);
                }
                if(rdFault < toFault)
                {
                    rdFault += (tNow - rdLast);
                    rdFault = rdFault <= toFault ? rdFault : toFault;
                    amap["PCSStatus"] ->setParam("rdFault",rdFault);
                }
            }

            if(0)FPS_ERROR_PRINT("%s >>  PcsStatus change for %s from [%d] to [%d]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault %2.3f\n"
                , __func__, aname, lastPcsStatus, currentPcsStatus, rdReset, (tNow - rdLast), rdAlarm, rdFault);

            ival = amap["PCSStatusStateNum"]->getiVal();
            // reset time passed , still changing , time to switch to PcsStatus_Ready
            if(rdReset <= 0.0 && ival != seenOk)
            {

                bool seenFault  = amap["PCSStatus"]->getbParam("seenFault");
                //bool seenOk  = amap["PcsStatus"]->getbParam("seenOk");
                bool seenAlarm  = amap["PCSStatus"]->getbParam("seenAlarm");

                if(0)FPS_ERROR_PRINT("%s >>  PcsStatus  change for %s from [%d] to [%d] \n", __func__, aname, lastPcsStatus, currentPcsStatus);
                if(seenFault)
                {
                    if(1)FPS_ERROR_PRINT("%s >>  PcsStatus fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["PCSStatus"] ->setParam("seenFault",false);

                }
                if(seenAlarm)
                {
                    if(1)FPS_ERROR_PRINT("%s >>  PcsStatus Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["PCSStatus"] ->setParam("seenAlarm",false);

                }

                amap["PCSStatus"] ->setParam("seenOk",true);
                seenOk = true;
                amap["StatusOK"]->setVal(true);

                if(1)FPS_ERROR_PRINT("%s >>  PcsStatus OK for  %s at %2.3f\n", __func__, aname, tNow);
                ival = Asset_Ok; // seen PcsStatus change
                amap["PCSStatusStateNum"]  ->setVal(ival);
                ival = 0;
                amap["PCSStatusInit"]->setVal(ival);
                char *tval;
                asprintf(&tval," PcsStatus OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["PCSStatusState"]->setVal(tval);
                    free((void *)tval);
                }
                amap["PCSStatus"]->setParam("rdReset", toReset);
            }

            // increment alarm and fault time reset time
            if(rdFault < toFault)
            {
                rdFault += (tNow - rdLast);
                rdFault = rdFault <= toFault ? rdFault : toFault;
                amap["PCSStatus"] ->setParam("rdFault",rdFault);
            }
            if(rdAlarm < toAlarm)
            {
                rdAlarm += (tNow - rdLast);
                rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
                amap["PCSStatus"] ->setParam("rdAlarm",rdAlarm);
            }
         
            //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
            amap["PCSStatus"]     ->setParam("lastPcsStatus",currentPcsStatus);
            //if ((toVal > toFault)  && !bokFault && !bypass)

        }
        else   // No Change , start tracking faults and alarms
        {
            bool seenFault  = amap["PCSStatus"]->getbParam("seenFault");
            //bool seenOk  = amap["PcsStatus"]->getbParam("seenOk");
            bool seenAlarm  = amap["PCSStatus"]->getbParam("seenAlarm");
            if(rdFault > 0.0)
            {
                rdFault -= (tNow - rdLast);
                rdFault = rdFault >= 0.0 ? rdFault : 0.0;
                amap["PCSStatus"] ->setParam("rdFault",rdFault);
            }
            if(rdAlarm > 0.0)
            {
                rdAlarm -= (tNow-rdLast);
                rdAlarm = rdAlarm >= 0.0 ? rdAlarm : 0.0;
                amap["PCSStatus"] ->setParam("rdAlarm",rdAlarm);
            }
            if(rdReset < toReset)
            {
                rdReset += (tNow - rdLast);
                rdReset = rdReset <= toReset ? rdReset : toReset;
                amap["PCSStatus"] ->setParam("rdReset",rdReset);
            }

            if((rdFault <= 0.0) && !seenFault)
            {

                if(1)FPS_ERROR_PRINT("%s >>  PcsStatus  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                char *tval;
                asprintf(&tval," PcsStatus Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["PCSStatusState"]->setVal(tval);
                    free((void *)tval);
                }
                int ival = 1 ; 
                amap["PCSStatusFaultCnt"]->addVal(ival);
                amap["essPCSStatusFaultCnt"]->addVal(ival);

                if(am->am)
                {
                    amap["amPCSStatusFaultCnt"]->addVal(ival);
                }

                ival = Asset_Fault; //PcsStatus Fault
                amap["PCSStatusStateNum"]  ->setVal(ival);

                seenFault = true;
                amap["PCSStatus"]->setParam("seenFault", true);
                amap["PCSStatus"]->setParam("seenOk", false);
                amap["PCSStatus"]->setParam("seenAlarm", true);
                //seenOk = false;
                seenAlarm =  false;

                int totalPcsStatusFaults = amap["PCSStatus"]->getiParam("totalPcsStatusFaults");
                totalPcsStatusFaults++;
                amap["PCSStatus"]->setParam("totalPcsStatusFaults",totalPcsStatusFaults);

                amap["StatusOK"]->setVal(false);

            }
            else if ((rdAlarm <= 0.0)  && !seenAlarm)
            {
                if(1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                char *tval;
                asprintf(&tval,"PCSStatus Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["PCSStatusState"]->setVal(tval);
                    free((void *)tval);
                }

                int ival = 1; 
                amap["PCSStatusAlarms"]->addVal(ival);
                amap["essPCSStatusAlarms"]->addVal(ival);

                if(am->am)
                {
                    amap["amPCSStatusAlarms"]->addVal(ival);
                }
                ival = Asset_Alarm; //PcsStatus Alarm
                amap["PCSStatusStateNum"]  ->setVal(ival);

                amap["PCSStatus"]->setParam("seenAlarm", true);
                //amap["PcsStatus"]->setParam("seenFault", false);
                amap["PCSStatus"]->setParam("seenOk", false);
                int totalPcsStatusAlarms = amap["PCSStatus"]->getiParam("totalPcsStatusAlarms");
                totalPcsStatusAlarms++;
                amap["PCSStatus"]->setParam("totalPcsStatusAlarms",totalPcsStatusAlarms);

                amap["StatusOK"]->setVal(false);
            }
            else
            {
                if(0)FPS_ERROR_PRINT("%s >> PcsStatus for [%s] [%s] Stalled at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
                        , __func__
                        , aname
                        , amap["PCSStatus"]->getcVal()
                        , tNow 
                        , rdReset, rdFault, rdAlarm);

            }            
        }
    }
    //
    //int ival1, ival2;
    //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
    return 0;
};

// check against expected BMS status log changes
// We get two status vars from the BMS and we send one back
// outputs
// output "id": "catl_ems_bms_rw",
            //   "id": "ems_status",
            //   "offset": 898,
            //   "name": "EMS_status"
            // },
            //  "id": "catl_mbmu_stat_r",
            //  {
            //   "id": "mbmu_status",
            //   "offset": 16,
            //   "name": "System status",
            //   "enum": true,
            //   "bit_strings": [
            //     "Initialize",
            //     "Normal",
            //     "Full Charge",
            //     "Full Discharge",
            //     "Warning Status",
            //     "Fault Status"
            //    ]
            // }
            // inputs
//            "id": "catl_bms_ems_r",
// {
//               "id": "bms_status",
//               "offset": 770,
//               "name": "BMS_status",
//               "enum": true,
//               "bit_strings": [
//                 "Initial status",
//                 "Normal status",
//                 "Full Charge status",
//                 "Full Discharge status",
//                 "Warning status",
//                 "Fault status"
//               ]
//             },

int CheckBMSStatus(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)
{
    //double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval =  (char*) "BmsStatus Init";
    VarMapUtils* vm =am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckBMSStatus", (void*)&CheckBMSStatus);
    //assetVar* CheckAssetComms = amap["CheckAmComms"];
    double toAlarm = 2.5;
    double toFault = 10.0;
    double toReset = 2.5;
    int initBmsStatus = -1;//(char *)" Initial BmsStatus";

    //if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2) 
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;
        //Link This to an incoming component
        if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
        amap["BMSStatus"]         = vm->setLinkVal(vmap, aname,                   "/status", "BMSStatus",             initBmsStatus);
        amap["MbmuStatus"]            = vm->setLinkVal(vmap, aname,               "/status", "MbmuStatus",             initBmsStatus);
        amap["BmsStatusString"]      = vm->setLinkVal(vmap, aname,                "/status", "BmsStatusString",       cval);
        amap["MbmuStatusString"]     = vm->setLinkVal(vmap, aname,                "/status", "MbmuStatusString",      cval);
        amap["BmsStatusString2"]      = vm->setLinkVal(vmap, aname,               "/status", "BmsStatusString2",     cval);
        
        amap["BMSExpStatus"]          = vm->setLinkVal(vmap, aname,                "/status", "BMSExpStatus",         cval);
        amap["MbmuExpStatus"]         = vm->setLinkVal(vmap, aname,                "/status", "MbmuExpStatus",        cval);
        amap["BmsTestToAlarm"]          = vm->setLinkVal(vmap, aname,                "/status", "BmsTestToAlarm",         toAlarm);
        
        if(1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
                , __func__
                , aname
                , amap["BMSStatus"]->comp.c_str()
                , amap["BMSStatus"]->name.c_str()
                );
        if (reload < 1)
        {
            vm->setAmFunc(vmap,amap,aname,p_fims,am,"BmsStatusString",CheckBMSStatus);
            vm->setAmFunc(vmap,amap,aname,p_fims,am,"MbmuStatusString",CheckBMSStatus);
        }

        amap["essBMSStatusFaultCnt"]       = vm->setLinkVal(vmap, "ess",                "/status",     "essBMSStatusFaultCnt",           ival);
        amap["essBMSStatusAlarmCnt"]       = vm->setLinkVal(vmap, "ess",                "/status",     "essBMSStatusAlarmCnt",           ival);
        amap["essBMSStatusInit"]           = vm->setLinkVal(vmap, "ess",                "/status",     "essBMSStatusInit",             ival);
        amap["essBMSStatusFaultTimeout"]   = vm->setLinkVal(vmap, "ess",                "/config",     "essBMSStatusFaultTimeout",     toFault);
        amap["essBMSStatusAlarmTimeout"]   = vm->setLinkVal(vmap, "ess",                "/config",     "essBMSStatusAlarmTimeout",     toAlarm);
        amap["essBMSStatusResetTimeout"]   = vm->setLinkVal(vmap, "ess",                "/config",     "essBMSStatusResetTimeout",     toReset);


        // if(am->am)
        // {
        //     amap["amBmsStatusFaults"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "BmsStatusFaults",         ival);
        //     amap["amBmsStatusAlarms"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "BmsStatusAlarms",         ival);
        //     amap["amBmsStatusInit"]    = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "BmsStatusInit",           ival);
        // }

        amap["BMSStatusFaultCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "BMSStatusFaultCnt",       ival);
        amap["BMSStatusAlarmCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "BMSStatusAlarmCnt",       ival);
        amap["BMSStatusInit"]            = vm->setLinkVal(vmap, aname,                "/status",     "BMSStatusInit",         ival);
        amap["BMSStatusState"]           = vm->setLinkVal(vmap, aname,                "/status",     "BMSStatusState",        cval);
        amap["BypassBMSStatus"]          = vm->setLinkVal(vmap, aname,                "/config",     "BypassBMSStatus",       bval);
        //amap["AssetState"]               = vm->setLinkVal(vmap, aname,                "/status",     "AssetState",        ival);
        amap["BMSStatusStateNum"]        = vm->setLinkVal(vmap, aname,                "/status",     "BMSStatusStateNum",      ival);

        amap["MbmuStatusFaults"]         = vm->setLinkVal(vmap, aname,                "/status",     "MbmuStatusFaults",       ival);
        amap["MbmuStatusAlarms"]         = vm->setLinkVal(vmap, aname,                "/status",     "MbmuStatusAlarms",       ival);
        amap["MbmuStatusInit"]           = vm->setLinkVal(vmap, aname,                "/status",     "MbmuStatusInit",         ival);
        amap["MbmuStatusState"]          = vm->setLinkVal(vmap, aname,                "/status",     "MbmuStatusState",        cval);
        amap["MbmuBypassStatus"]         = vm->setLinkVal(vmap, aname,                "/config",     "MbmuBypassStatus",       bval);
        amap["MbmuStatusStateNum"]       = vm->setLinkVal(vmap, aname,                "/status",     "MbmuStatusStateNum",      ival);

        amap["StatusOK"]                 = vm->setLinkVal(vmap, aname,                "/status",     "StatusOK",      ival);

        
        if(reload == 0) // complete restart 
        {
            amap["BMSStatus"]            ->setVal(initBmsStatus);
            amap["BMSExpStatus"]         ->setVal(cval);
            amap["MbmuExpStatus"]         ->setVal(cval);
            //lastBmsStatus=strdup(tsInit);//state"]->setVal(cval);
            amap["BMSSysStatus"]           ->setParam("lastBmsStatus", initBmsStatus);
            amap["BmsStatusString"]     ->setVal(cval);
            amap["BmsStatusString"]     ->setParam("lastStatusString", cval);
            amap["MbmuStatusString"]    ->setVal(cval);
            amap["MbmuStatusString"]    ->setParam("lastStatusString", cval);

            amap["BmsStatusString2"]     ->setVal(cval);
            amap["BmsStatusString2"]     ->setParam("lastBmsStatusString", cval);

            amap["BMSStatus"]     ->setParam("totalStatusFaults", 0);
            amap["BMSStatus"]     ->setParam("totalStatusAlarms", 0);
            amap["BMSStatus"]     ->setParam("seenFault", false);
            amap["BMSStatus"]     ->setParam("seenOk", false);
            amap["BMSStatus"]     ->setParam("seenAlarm", false);
            amap["BMSStatus"]     ->setParam("seenInit", false);
            amap["BMSStatus"]     ->setParam("initCnt", -1);

            amap["BMSStatus"]     ->setParam("rdFault", toFault);                      // time remaining before fault
            amap["BMSStatus"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["BMSStatus"]     ->setParam("rdReset", toReset);                      // time remaining before reset
            amap["BMSStatus"]     ->setParam("rdLast", dval);                         // time when last to event was seen
            amap["BMSStatus"]     ->setParam("ParamtoAlarm", amap["BmsTestToAlarm"]);  // Set an Av as a param
        

            amap["BMSStatusState"]    ->setVal(cval);

            amap["MbmuStatus"]     ->setParam("totalStatusFaults", 0);
            amap["MbmuStatus"]     ->setParam("totalStatusAlarms", 0);
            amap["MbmuStatus"]     ->setParam("seenFault", false);
            amap["MbmuStatus"]     ->setParam("seenOk", false);
            amap["MbmuStatus"]     ->setParam("seenAlarm", false);
            amap["MbmuStatus"]     ->setParam("seenInit", false);
            amap["MbmuStatus"]     ->setParam("initCnt", -1);

            amap["MbmuStatus"]     ->setParam("rdFault", toFault);                      // time remaining before fault
            amap["MbmuStatus"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["MbmuStatus"]     ->setParam("rdReset", toReset);                      // time remaining before reset
            amap["MbmuStatus"]     ->setParam("rdLast", dval);                         // time when last to event was seen

            amap["MbmuStatusState"]    ->setVal(cval);

            ival = Asset_Init; amap["MbmuStatusStateNum"]  ->setVal(ival);
            ival = -1; amap["MbmuStatusInit"]  ->setVal(ival);
            amap["BypassBMSStatus"]  ->setVal(false);
            amap["MbmuBypassStatus"]  ->setVal(false);

            // if(!am->am)  // Nah do this in setLinkVals
            // {
            //     amap["essBmsStatusTimeoutFault"] ->setVal(toFault);
            //     amap["essBmsStatusTimeoutAlarm"] ->setVal(toAlarm);
            //     amap["essBmsStatusTimeoutReset"] ->setVal(toReset);

            // }
        }
        // reset reload
        ival = 2; amap["CheckBmsStatus"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();

    bool BmsBypassStatus = amap["BypassBMSStatus"]->getbVal();
    //bool MbmuBypassStatus = amap["BmsBypassStatus"]->getbVal();

    toFault = amap["essBMSStatusFaultTimeout"]->getdVal();
    toAlarm = amap["essBMSStatusAlarmTimeout"]->getdVal();
    toReset = amap["essBMSStatusResetTimeout"]->getdVal();

    int expectedBmsStatus = amap["BMSExpStatus"]->getiVal();
    int currentBmsStatus = amap["BMSStatus"]->getiVal();
    int lastBmsStatus    = amap["BMSStatus"]->getiParam("lastBmsStatus");

    char* currentBmsStatusString = amap["BmsStatusString"]->getcVal();
    char* lastBmsStatusString    = amap["BmsStatusString"]->getcParam("lastStatusString");
    char* currentMbmuStatusString = amap["MbmuStatusString"]->getcVal();
    char* lastMbmuStatusString    = amap["MbmuStatusString"]->getcParam("lastStatusString");
    char* currentBmsStatusString2 = amap["BmsStatusString2"]->getcVal();
    char* lastBmsStatusString2    = amap["BmsStatusString2"]->getcParam("lastBmsStatusString");

    if(BmsBypassStatus)
    {
        ival = 1;
        amap["essBMSStatusInit"]->addVal(ival);

        // Do we set status OK here?
        amap["StatusOK"]->setVal(true);
        return 0;

    }
    else
    {
        char* dest;
        char* msg;
        tNow = vm->get_time_dbl();
 
        if (strcmp(currentBmsStatusString,lastBmsStatusString) != 0)    
        {
            if(1) FPS_ERROR_PRINT(" %s >> BmsStatusString comp [%s:%s] Changed from [%s] to [%s] at %2.6f\n"
                        , __func__
                        , amap["BmsStatusString"]->comp.c_str()
                        , amap["BmsStatusString"]->name.c_str()
                        , lastBmsStatusString
                        , currentBmsStatusString
                        , tNow
                        );

            amap["BmsStatusString"]->setParam("lastStatusString", currentBmsStatusString);
            if(strcmp(currentBmsStatusString,"Warning status") == 0)
            {
                asprintf(&dest,"/assets/%s/summary:alarms",am->name.c_str());
                asprintf(&msg,"%s alarm  [%s] at %2.3f ","Bms Status ", currentBmsStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["BmsStatusString"], dest, nullptr, msg, 2);
                }
                amap["essBMSStatusAlarmCnt"]->addVal(1); 
                amap["StatusOK"]->setVal(false);

                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if(1)FPS_ERROR_PRINT(" %s ALARM >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg );
                if(dest)free((void*)dest);
                if(msg)free((void*)msg);
            }
            if(strcmp(currentBmsStatusString,"Fault status") == 0)
            {
                asprintf(&dest,"/assets/%s/summary:faults",am->name.c_str());
                asprintf(&msg,"%s fault  [%s] at %2.3f ","Bms Status ", currentBmsStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["BmsStatusString"], dest, nullptr, msg, 2);
                }
                amap["essBMSStatusFaultCnt"]->addVal(1);
                amap["StatusOK"]->setVal(false); 
                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if(1)FPS_ERROR_PRINT(" %s FAULT >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg );
                if(dest)free((void*)dest);
                if(msg)free((void*)msg);
            }

        }
        if(0) FPS_ERROR_PRINT(" %s >> Testing MbmuStatusString  [%s] to [%s] at %2.6f\n"
                        , __func__
                        , lastMbmuStatusString
                        , currentMbmuStatusString
                        , tNow
                        );

        if (strcmp(currentMbmuStatusString,lastMbmuStatusString) != 0)    
        {
            if(1) FPS_ERROR_PRINT(" %s >> MbmuStatusString Changed from [%s] to [%s] at %2.6f\n"
                        , __func__
                        , lastMbmuStatusString
                        , currentMbmuStatusString
                        , tNow
                        );

            amap["MbmuStatusString"]     ->setParam("lastStatusString", currentMbmuStatusString);
            if(strcmp(currentMbmuStatusString,"Warning") == 0)
            {
                asprintf(&dest,"/assets/%s/summary:alarms",am->name.c_str());
                asprintf(&msg,"%s alarm  [%s] at %2.3f ","Mbmu Status ", currentMbmuStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["MbmuStatusString"], dest, nullptr, msg, 2);
                }
                amap["essBMSStatusAlarmCnt"]->addVal(1); 
                amap["StatusOK"]->setVal(false);

                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if(1)FPS_ERROR_PRINT(" %s ALARM >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg );
                if(dest)free((void*)dest);
                if(msg)free((void*)msg);
            }
            if(strcmp(currentMbmuStatusString, "Fault") == 0)
            {
                asprintf(&dest,"/assets/%s/summary:faults",am->name.c_str());
                asprintf(&msg,"%s fault  [%s] at %2.3f ","Mbmu Status ", currentMbmuStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["MbmuStatusString"], dest, nullptr, msg, 2);
                }

                amap["essBMSStatusFaultCnt"]->addVal(1); 
                amap["StatusOK"]->setVal(false);

                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if(1)FPS_ERROR_PRINT(" %s FAULT >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg );
                if(dest)free((void*)dest);
                if(msg)free((void*)msg);
            }

        }

        if (strcmp(currentBmsStatusString2,lastBmsStatusString2) != 0)    
        {
            if(1) FPS_ERROR_PRINT(" %s >> BmsStatusString2 Changed from [%s] to [%s] at %2.3f\n"
                        , __func__
                        , lastBmsStatusString2
                        , currentBmsStatusString2
                        , tNow
                        );

            amap["BmsStatusString2"]     ->setParam("lastBmsStatusString", currentBmsStatusString2);

        }
        // If we are in the init state wait for comms to start count down reset time
        if (currentBmsStatus == initBmsStatus)    
        {
            bool seenInit = amap["BMSStatus"]->getbParam("seenInit");

            //ival = 1; amap["CheckAssetComs"]->setVal(ival);
            //ival = 1; amap["CheckAssetComs"]->setVal(ival);
            if(0)FPS_ERROR_PRINT("%s >> %s  NO BmsStatus,  bypass [%s]\n", __func__, aname, BmsBypassStatus?"true":"false");

            // if not toally set up yet then quit this pass
            if(!amap["amBMSStatusInit"])
            {
                if(1)FPS_ERROR_PRINT("%s >> %s  no VAR amBmsStatusInit Yet...\n", __func__, aname);
                amap["amBMSStatusInit"]        = vm->setLinkVal(vmap, aname,                "/status",     "amBMSStatusInit",       ival);
                return 0;
            }

            if (!seenInit)   // BmsStatus_Setup
            {
                if(1)FPS_ERROR_PRINT("%s >> %s  amBmsStatusInit  SEEN ...\n", __func__, aname );
                assetVar*  toav =amap["BmsStatus"] ->getaParam("ParamtoAlarm");  // Get an Av as a param
                if(1)FPS_ERROR_PRINT("%s >> %s  ParamtoAlarm %f  \n", __func__, aname, toav->getdVal() );

                amap["BMSStatus"] ->setParam("seenInit",true);

                char* cval = (char *)"BmsStatus Init, no BmsStatus Seen";
                amap["BMSStatusState"]->setVal(cval);

                ival = 1;
                amap["essBMSStatusInit"]->addVal(ival);
                amap["BMSStatusInit"]->setVal(0);      //BmsStatus_Init  
            }
            amap["BMSStatus"] ->setParam("rdLast",tNow);

        }
        else  // wait for comms to go past reset then set active or wait to alarm and then fault
        {
            //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastBmsStatus?lastBmsStatus:"no last Value", tval1);
            double rdLast = amap["BMSStatus"] ->getdParam("rdLast");
            double rdFault = amap["BMSStatus"] ->getdParam("rdFault");
            double rdAlarm = amap["BMSStatus"] ->getdParam("rdAlarm");
            double rdReset = amap["BMSStatus"] ->getdParam("rdReset");
            amap["BMSStatus"] ->setParam("rdLast",tNow);

            double toVal  = amap["BMSStatus"]->getLastSetDiff(tNow);

            // Has value changed ? If yes then count down rdReset to zero based on tNow - rdLast
            if (currentBmsStatus != lastBmsStatus) 
            //if(amap["BmsStatus"]->valueChangedReset())
            {
                if(1)FPS_ERROR_PRINT("%s >> %s  amBmsStatus Changed from  %d to %d  (expected %d) at time %2.3f  SEEN ...\n"
                        , __func__, aname, lastBmsStatus, currentBmsStatus, expectedBmsStatus, tNow );

                // TODO log the change
                amap["BMSStatus"]     ->setParam("lastBmsStatus", currentBmsStatus);
            }

            if (currentBmsStatus == expectedBmsStatus) 
            {
                bool seenOk  = amap["BMSStatus"]->getbParam("seenOk");
                if(rdReset > 0.0)
                {
                    rdReset -= (tNow - rdLast);
                    rdReset = rdReset >= 0.0 ? rdReset : 0.0; 
                    amap["BMSStatus"] ->setParam("rdReset", rdReset);
                }
                //else
                {
                    // TODO after reset increment these up to toAlarm
                    if(rdAlarm < toAlarm)
                    {
                        rdAlarm += (tNow - rdLast);
                        rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
                        amap["BMSStatus"] ->setParam("rdAlarm",rdAlarm);
                    }
                    if(rdFault < toFault)
                    {
                        rdFault += tNow - rdLast;
                        rdFault = rdFault <= toFault ? rdFault : toFault;
                        amap["BMSStatus"] ->setParam("rdFault",rdFault);
                    }
                }

                if(0)FPS_ERROR_PRINT("%s >>  BmsStatus change for %s from [%d] to [%d]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault %2.3f\n"
                    , __func__, aname, lastBmsStatus, currentBmsStatus, rdReset, (tNow - rdLast), rdAlarm, rdFault);

                ival = amap["BMSStatusStateNum"]  ->getiVal();
                // reset time passed , still changing , time to switch to BmsStatus_Ready
                if(rdReset <= 0.0 && ival != seenOk)
                {

                    bool seenFault  = amap["BMSStatus"]->getbParam("seenFault");
                    //bool seenOk  = amap["BmsStatus"]->getbParam("seenOk");
                    bool seenAlarm  = amap["BMSStatus"]->getbParam("seenAlarm");

                    if(0)FPS_ERROR_PRINT("%s >>  BmsStatus  change for %s from [%d] to [%d] \n", __func__, aname, lastBmsStatus, currentBmsStatus);
                    if(seenFault)
                    {
                        if(1)FPS_ERROR_PRINT("%s >>  BmsStatus fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                        amap["BMSStatus"] ->setParam("seenFault",false);

                    }
                    if(seenAlarm)
                    {
                        if(1)FPS_ERROR_PRINT("%s >>  BmsStatus Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                        amap["BMSStatus"] ->setParam("seenAlarm",false);

                    }

                    amap["BMSStatus"] ->setParam("seenOk",true);
                    amap["StatusOK"]->setVal(true);

                    if(1)FPS_ERROR_PRINT("%s >>  BmsStatus OK for  %s at %2.3f\n", __func__, aname, tNow);
                    ival = Asset_Ok; // seen BmsStatus change
                    amap["BMSStatusStateNum"]  ->setVal(ival);
                    ival = 0;
                    amap["BMSStatusInit"]->setVal(ival);
                    char *tval;
                    asprintf(&tval," BmsStatus OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if(tval)
                    {
                        amap["BMSStatusState"]->setVal(tval);
                        free((void *)tval);
                    }
                }

                // increment alarm and fault time reset time
                if(rdFault < toFault)
                {
                    rdFault += (tNow - rdLast);
                    rdFault = rdFault <= toFault ? rdFault : toFault;
                    amap["BMSStatus"] ->setParam("rdFault",rdFault);
                }
                if(rdAlarm < toAlarm)
                {
                    rdAlarm += (tNow - rdLast);
                    rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
                    amap["BMSStatus"] ->setParam("rdAlarm",rdAlarm);
                }
            
                //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
                amap["BMSStatus"]     ->setParam("lastBmsStatus",currentBmsStatus);
                //if ((toVal > toFault)  && !bokFault && !bypass)

            }
            else   // No Change , start tracking faults and alarms
            {
                bool seenFault  = amap["BMSStatus"]->getbParam("seenFault");
                //bool seenOk  = amap["BmsStatus"]->getbParam("seenOk");
                bool seenAlarm  = amap["BMSStatus"]->getbParam("seenAlarm");
                if(rdFault > 0.0)
                {
                    rdFault -= (tNow - rdLast);
                    rdFault = rdFault >= 0.0 ? rdFault : 0.0;
                    amap["BMSStatus"] ->setParam("rdFault",rdFault);
                }
                if(rdAlarm > 0.0)
                {
                    rdAlarm -= (tNow - rdLast);
                    rdAlarm = rdAlarm >= 0.0 ? rdAlarm : 0.0;
                    amap["BMSStatus"] ->setParam("rdAlarm",rdAlarm);
                }
                if(rdReset < toReset)
                {
                    rdReset += (tNow - rdLast);
                    rdReset = rdReset <= toReset ? rdReset : toReset;
                    amap["BMSStatus"] ->setParam("rdReset",rdReset);
                }

                if(rdFault <= 0.0 && !seenFault)
                {

                    if(1)FPS_ERROR_PRINT("%s >>  BmsStatus  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                    char *tval;
                    asprintf(&tval," BmsStatus Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if(tval)
                    {
                        amap["BMSStatusState"]->setVal(tval);
                        free((void *)tval);
                    }
                    int ival = 1 ; 
                    amap["BMSStatusFaults"]->addVal(ival);
                    amap["essBMSStatusFaults"]->addVal(ival);

                    // if(am->am)
                    // {
                    //     amap["amBmsStatusFaults"]->addVal(ival);
                    // }

                    ival = Asset_Fault; //BmsStatus Fault
                    amap["BMSStatusStateNum"]  ->setVal(ival);

                    seenFault = true;
                    amap["BMSStatus"]->setParam("seenFault", true);
                    amap["BMSStatus"]->setParam("seenOk", false);
                    amap["BMSStatus"]->setParam("seenAlarm", true);
                    //seenOk = false;
                    seenAlarm =  false;

                    int totalBmsStatusFaults = amap["BMSStatus"]->getiParam("totalStatusFaults");
                    totalBmsStatusFaults++;
                    amap["BMSStatus"]->setParam("totalStatusFaults",totalBmsStatusFaults);

                    amap["StatusOK"]->setVal(false);

                }
                else if (rdAlarm <= 0.0  && !seenAlarm)
                {
                    if(1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                    char *tval;
                    asprintf(&tval,"BmsStatus Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if(tval)
                    {
                        amap["BMSStatusState"]->setVal(tval);
                        free((void *)tval);
                    }

                    int ival = 1; 
                    amap["BMSStatusAlarms"]->addVal(ival);
                    amap["essBMSStatusAlarms"]->addVal(ival);

                    // if(am->am)
                    // {
                    //     amap["amBmsStatusAlarms"]->addVal(ival);
                    // }
                    ival = Asset_Alarm; //BmsStatus Alarm
                    amap["BMSStatusStateNum"]  ->setVal(ival);

                    amap["BMSStatus"]->setParam("seenAlarm", true);
                    //amap["BmsStatus"]->setParam("seenFault", false);
                    amap["BMSStatus"]->setParam("seenOk", false);
                    int totalBmsStatusAlarms = amap["BmsStatus"]->getiParam("totalBmsStatusAlarms");
                    totalBmsStatusAlarms++;
                    amap["BMSStatus"]->setParam("totalBmsStatusAlarms",totalBmsStatusAlarms);

                    amap["StatusOK"]->setVal(false);
                }
                else
                {
                    if(0)FPS_ERROR_PRINT("%s >> BmsStatus for [%s] [%s] Stalled at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
                            , __func__
                            , aname
                            , amap["BMSStatus"]->getcVal()
                            , tNow 
                            , rdReset, rdFault, rdAlarm);

                }            
            }
        }
    }
    //
    //int ival1, ival2;
    //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
    return 0;
};

// BMS
// ess_controller test asset status  
// logs any changes
int CheckESSStatus(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)
{
    //double dval = 0.0;
    int ival = 0;
    //bool bval = false;
    //int dval = 0.0;
    //char* cval =  (char*) "Ess Status Init";
    char* pcval =  (char*) "Pcs Status Init";
    char* bmval =  (char*) "Bms Status Init";
    VarMapUtils* vm =am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckESSStatus");
    //assetVar* CheckAssetComms = amap["CheckAmComms"];
    //double toAlarm = 2.5;
    //double toFault = 10.0;
    //double toReset = 2.5;
    int initPcsStatus = -1;//(char *)" Initial PcsStatus";
    int initBmsStatus = -1;//(char *)" Initial PcsStatus";
    int initEssStatus = -1;//(char *)" Initial PcsStatus";

    
    //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2) 
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;
        //Link This to an incoming component
        if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
        amap["ESSStatus"]                 = vm->setLinkVal(vmap, aname,                "/status", "ESSStatus",          initEssStatus);
        amap["PCSStatus"]                 = vm->setLinkVal(vmap, aname,                "/status", "PCSStatus",          initPcsStatus);
        amap["BMSStatus"]                 = vm->setLinkVal(vmap, aname,                "/status", "BMSStatus",         initBmsStatus);
 
        amap["PCSStatusState"]            = vm->setLinkVal(vmap, aname,           "/status", "PCSStatusState",          pcval);
        amap["BMSStatusState"]            = vm->setLinkVal(vmap, aname,           "/status", "BMSStatusState",         bmval);
 
 
        amap["essPCSStatusFaultCnt"]       = vm->setLinkVal(vmap, aname,                "/status",     "essPCSStatusFaultCnt",       ival);
        amap["essPCSStatusAlarmCnt"]       = vm->setLinkVal(vmap, aname,                "/status",     "essPCSStatusAlarmCnt",       ival);
        amap["essPCSStatusInit"]           = vm->setLinkVal(vmap, aname,                "/status",     "essPCSStatusInit",       ival);
        //amap["essPCSStatusState"]          = vm->setLinkVal(vmap, aname,                "/status",     "essPCSStatusState",        cval);
        amap["essBMSStatusFaultCnt"]       = vm->setLinkVal(vmap, aname,                "/status",     "essBMSStatusFaultCnt",       ival);
        amap["essBMSStatusAlarmCnt"]       = vm->setLinkVal(vmap, aname,                "/status",     "essBMSStatusAlarmCnt",       ival);
        amap["essBMSStatusInit"]           = vm->setLinkVal(vmap, aname,                "/status",     "essBMSStatusInit",       ival);
        //amap["essBMSStatusState"]          = vm->setLinkVal(vmap, aname,                "/status",     "essBMSStatusState",        cval);

        
        if(reload == 0) // complete restart 
        {
            amap["PCSStatus"]     ->setVal(initPcsStatus);
            amap["BMSStatus"]     ->setVal(initBmsStatus);
            //lastPcsStatus=strdup(tsInit);//state"]->setVal(cval);
            amap["PCSStatus"]     ->setParam("lastPcsStatus", initPcsStatus);
            amap["PCSStatus"]     ->setParam("totalPcsStatusFaults", 0);
            amap["PCSStatus"]     ->setParam("totalPcsStatusAlarms", 0);

            amap["BMSStatus"]     ->setParam("lastBmsStatus", initBmsStatus);
            amap["BMSStatus"]     ->setParam("totalBmsStatusFaults", 0);
            amap["BMSStatus"]     ->setParam("totalBmsStatusAlarms", 0);


            amap["PCSStatusState"]    ->setVal(pcval);
            amap["BMSStatusState"]    ->setVal(bmval);

        }
        // reset reload
        ival = 2; amap["CheckESSStatus"]->setVal(ival);
    }

    //double tNow = am->vm->get_time_dbl();

    // are we the ess_controller 
    if(!am->am)
    {
        //bool initSeen =             amap["PcsStatus"]     ->getbParam("initSeen");

        amap["essPCSStatusFaultCnt"]  ->setVal(0);
        amap["essPCSStatusAlarmCnt"]  ->setVal(0);
        amap["essPCSStatusInit"]    ->setVal(0);
        amap["essBMSStatusFaultCnt"]  ->setVal(0);
        amap["essBMSStatusAlarmCnt"]  ->setVal(0);
        amap["essBMSStatusInit"]    ->setVal(0);
        int icnt = 0;
        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            if(amc->name == "pcs")
            {
                CheckPCSStatus(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
                icnt++;
            }
            else if(amc->name == "bms")
            {
                CheckBMSStatus(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
                icnt++;
            }

            int essPcsStatusFaults = amap["essPCSStatusFaultCnt"]->getiVal();
            int essPcsStatusAlarms = amap["essPCSStatusAlarmCnt"]->getiVal();
            //int essPcsStatusInit = amap["essPCSStatusInit"]->getiVal();
            if(essPcsStatusFaults > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essPcsStatusFaults detected\n", __func__, essPcsStatusFaults);
            }
            if(essPcsStatusAlarms > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essPcsStatusAlarmss detected\n", __func__, essPcsStatusAlarms);
            }
            int essBmsStatusFaults = amap["essBMSStatusFaultCnt"]->getiVal();
            int essBmsStatusAlarms = amap["essBMSStatusAlarmCnt"]->getiVal();
            //int essBmsStatusInit = amap["essBMSStatusInit"]->getiVal();
            if(essBmsStatusFaults > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essBmsStatusFaults detected\n", __func__, essBmsStatusFaults);
            }
            if(essBmsStatusAlarms > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essBmsStatusAlarms detected\n", __func__, essBmsStatusAlarms);
            }
        }
        // TODO do things like shutdown on faults


    }
    return 0;
};

int CheckAssetDisable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
{
    // this will filter up the comms stats to the manager
    int ival = 0;
    //bool bval = true;
    double tNow = am->vm->get_time_dbl();
    double dval = 0.0;

    if (!amap["CheckAssetDisable"] || amap["CheckAssetDisable"]->getiVal()<2)
    {
        //char * cval = (char *)"HeartBeat Init";
        //setAmapAi(am,  amap,          HeartBeatState,              am->name.c_str(),      /status,       cval);
        //int ival = 0;
        //bool bval = false;
        setAmapAi(am,  amap,          CheckAssetDisable,        am->name.c_str(),      /reload,       ival);
        setAmapAi(am,  amap,          DisableCmd,               am->name.c_str(),      /controls,     ival);
        setAmapAi(am,  amap,          EnableCmd,                am->name.c_str(),      /controls,     ival);
        setAmapAi(am,  amap,          EnableCnt,                am->name.c_str(),      /status,     ival);
        setAmapAi(am,  amap,          DisableCnt,               am->name.c_str(),      /status,     ival);
        setAmapAi(am,  amap,          Enabledxx,                am->name.c_str(),      /controls,     ival);
        setAmapAi(am,  amap,          CheckAssetInit,           am->name.c_str(),      /status,     tNow);
        setAmapAi(am,  amap,          CheckAssetCmdRun,         am->name.c_str(),    /status,     tNow);
        setAmapAi(am,  amap,          CheckAssetCmdRuns,         am->name.c_str(),    /status,     dval);
        setAmapAi(am,  amap,          DisableCmdRun,            am->name.c_str(),    /status,     tNow);
        setAmapAi(am,  amap,          EnableCmdRun,             am->name.c_str(),    /status,     tNow);
        ival = 2; amap["CheckAssetDisable"]->setVal(ival); 
    }
    amap["CheckAssetCmdRun"]->setVal(tNow);
    dval = amap["CheckAssetCmdRuns"]->getdVal();
    dval++;
    amap["CheckAssetCmdRuns"]->setVal(dval);

    //bool bval2;
    ival = amap["Enabledxx"]->getiVal();
//    double tNow = am->vm->get_time_dbl();
    int ivalcmd=0;
    if (ival == 0)
    {
        ivalcmd =amap["EnableCmd"]->getiVal();
        if(ivalcmd>0)
        {
            //bval =  true;
            ival = 1;
            amap["Enabledxx"]->setVal(ival);
            ivalcmd = 0;
            amap["EnableCmd"]->setVal(ivalcmd);
            ival = amap["EnableCnt"]->getiVal();
            ival++;
            amap["EnableCnt"]->setVal(ival);
            amap["EnableCmdRun"]->setVal(tNow);

        }
    }
    else
    {
        ivalcmd =amap["DisableCmd"]->getiVal();
        if(ivalcmd>0)
        {
            //bval =  false;
            ival = 0;
            amap["Enabledxx"]->setVal(ival);
            ivalcmd = 0;
            amap["DisableCmd"]->setVal(ivalcmd);
            ival = amap["DisableCnt"]->getiVal();
            ival++;
            amap["DisableCnt"]->setVal(ival);
            amap["DisableCmdRun"]->setVal(tNow);

        }
    }
    
    return 0;
}