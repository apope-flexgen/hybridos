/*
* this is the file for "released" or completed application functions
* as a function is getting ready to be released put it in here and we'll get working on signing it off
*
*/
#include "asset.h"
#include "assetFunc.h"
//#include "assetFunc.cpp"
#include "../funcs/CheckAmHeartbeat.cpp"
#include "../funcs/CheckAmTimestamp.cpp"
#include "chrono_utils.hpp"

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


// /*
//  * the timestamp is part of the pub message
//  *    After init we must get a continual Timestamp changes otherwise we alarm and then fault.
//  *    the bms Timestamp arrives on /components/catl_ems_bms_01_rw:ems_heartbeat,
//  *    linked to /status/bms/Heartbeat  in bms_manager.json
//  * Lets try again
//  *    base it off HBSeenTime
//  * at start HBSeenTime = 0.0 HBOk = false seenHB = false
//  * if we see a change and !seenHB then set HBseenTime and set seenHBS
//  * if HBseenTime == 0 we never have seen a HB  dont set faults or alarms yet
//  * if seenHB and (tNow - HBSeenTime) > toHold reset seenHB
//  * if seenHB and rdReset <=0.0  then set HBok clear errors else decrement rdReset
//  * if HBOk inc rdAlarm and rdFault to ther max
//  * if !seenHB  and tNow - HBseenTime > rdAlarm then set Alarm
//  * if !seenHB  and tNow - HBseenTime > rdFault then set Fault

//  * toHold time to allow between Timestamp changes before worrying about it
//  * toAlarm time after a stalled Heatbeat causes an Alarm
//  * toFault time after a stalled Heatbeat causes a Fault
//  *  toReset time after changes start being seen again before resetting faults and Alarms
//  *
//  */
// int CheckAmTimestamp(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval = (char*)"Timestamp Init";
//     VarMapUtils* vm = am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckAmTimestamp");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toHold = 1.5;  // Seconds between TS changes
//     double toAlarm = 2.5;
//     double toFault = 6.0;
//     double toReset = 2.5;
//     char* initTimestamp = (char*)" Initial Timestamp";


//     //if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
//     if (reload < 2)
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);

//         amap["Timestamp"] = vm->setLinkVal(vmap, aname, "/status", "Timestamp", initTimestamp);
//         if (1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
//             , __func__
//             , aname
//             , amap["Timestamp"]->comp.c_str()
//             , amap["Timestamp"]->name.c_str()
//         );

//         amap["essTsFaults"] = vm->setLinkVal(vmap, "ess", "/status", "essTsFaults", ival);
//         amap["essTsAlarms"] = vm->setLinkVal(vmap, "ess", "/status", "essTsAlarms", ival);
//         amap["essTsInit"] = vm->setLinkVal(vmap, "ess", "/status", "essTsInit", ival);
//         amap["essTsTimeoutFault"] = vm->setLinkVal(vmap, "ess", "/config", "essTsTimeoutFault", toFault);
//         amap["essTsTimeoutAlarm"] = vm->setLinkVal(vmap, "ess", "/config", "essTsTimeoutAlarm", toAlarm);
//         amap["essTsTimeoutReset"] = vm->setLinkVal(vmap, "ess", "/config", "essTsTimeoutReset", toReset);
//         amap["essTsTimeoutHold"] = vm->setLinkVal(vmap, "ess", "/config", "essTsTimeoutHold", toHold);

//         if (am->am)
//         {
//             amap["amTsFaults"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "TsFaults", ival);
//             amap["amTsAlarms"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "TsAlarms", ival);
//             amap["amTsInit"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "TsInit", ival);
//         }

//         amap["TsFaults"] = vm->setLinkVal(vmap, aname, "/status", "TsFaults", ival);
//         amap["TsAlarms"] = vm->setLinkVal(vmap, aname, "/status", "TsAlarms", ival);
//         amap["TsInit"] = vm->setLinkVal(vmap, aname, "/status", "TsInit", ival);
//         amap["TsState"] = vm->setLinkVal(vmap, aname, "/status", "TsState", cval);
//         amap["BypassTs"] = vm->setLinkVal(vmap, aname, "/config", "BypassTs", bval);
//         amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status", "AssetState", ival);
//         amap["TsStateNum"] = vm->setLinkVal(vmap, aname, "/status", "TsStateNum", ival);


//         if (reload == 0) // complete restart 
//         {
//             amap["Timestamp"]->setVal(initTimestamp);
//             //lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
//             amap["Timestamp"]->setParam("lastTimestamp", initTimestamp);
//             amap["Timestamp"]->setParam("totalTsFaults", 0);
//             amap["Timestamp"]->setParam("totalTsAlarms", 0);

//             amap["Timestamp"]->setParam("seenFault", false);
//             amap["Timestamp"]->setParam("seenAlarm", false);
//             amap["Timestamp"]->setParam("seenOk", false);
//             amap["Timestamp"]->setParam("seenTS", false);

//             amap["Timestamp"]->setParam("TSOk", false);
//             dval = 0.0;
//             amap["Timestamp"]->setParam("TSseenTime", dval);
//             amap["Timestamp"]->setParam("seenInit", false);
//             amap["Timestamp"]->setParam("initCnt", -1);

//             amap["Timestamp"]->setParam("rdFault", toFault);                      // time remaining before fault
//             amap["Timestamp"]->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
//             amap["Timestamp"]->setParam("rdReset", toReset);                      // time remaining before reset
//             //amap["Timestamp"]     ->setParam("rdHold", toHold);                        // time to wait before no change test
//             amap["Timestamp"]->setParam("tLast", dval);                         // time when last to event was seen

//             amap["TsState"]->setVal(cval);
//             ival = Asset_Init; amap["TsStateNum"]->setVal(ival);
//             ival = -1; amap["TsInit"]->setVal(ival);
//             amap["BypassTs"]->setVal(false);

//             amap["essTsFaults"]->setParam("lastTsFaults", 0);
//             amap["essTsAlarms"]->setParam("lastTsAlarms", 0);
//         }
//         // reset reload
//         ival = 2; amap["CheckAmTimestamp"]->setVal(ival);
//     }

//     double tNow = am->vm->get_time_dbl();
//     double tLast = amap["Timestamp"]->getdParam("tLast");
//     if (tLast == 0.0)
//         tLast = tNow;
//     double tDiff = tNow - tLast;
//     amap["Timestamp"]->setParam("tLast", tNow);

//     bool BypassTs = amap["BypassTs"]->getbVal();

//     toFault = amap["essTsTimeoutFault"]->getdVal();
//     toAlarm = amap["essTsTimeoutAlarm"]->getdVal();
//     toReset = amap["essTsTimeoutReset"]->getdVal();
//     toHold = amap["essTsTimeoutHold"]->getdVal();

//     char* currentTimestamp = amap["Timestamp"]->getcVal();
//     char* lastTimestamp = amap["Timestamp"]->getcParam("lastTimestamp");//amap["lastTimestamp"]->getiVal();
//     // are we the ess_controller 
//     if (!am->am)
//     {
//         //bool initSeen =             amap["Timestamp"]     ->getbParam("initSeen");

//         amap["essTsFaults"]->setVal(0);
//         amap["essTsAlarms"]->setVal(0);
//         amap["essTsInit"]->setVal(0);

//         int initCnt = amap["Timestamp"]->getiParam("initCnt");
//         int icnt = 0;
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager* amc = ix.second;
//             CheckAmTimestamp(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//             icnt++;
//         }

//         int essTsFaults = amap["essTsFaults"]->getiVal();
//         int essTsAlarms = amap["essTsAlarms"]->getiVal();
//         int lastTsAlarms = amap["essTsAlarms"]->getiParam("lastTsAlarms");
//         int lastTsFaults = amap["essTsFaults"]->getiParam("lastTsFaults");

//         //int essTsInit = amap["essTsInit"]->getiVal();
//         if (essTsFaults != lastTsFaults)
//         {
//             amap["essTsFaults"]->setParam("lastTsFaults", essTsFaults);

//             if (essTsFaults > 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsFaults detected at time %2.3f \n", __func__, essTsFaults, tNow);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsFaults cleared at time %2.3f\n", __func__, essTsFaults, tNow);
//             }
//         }
//         if (essTsAlarms != lastTsAlarms)
//         {
//             amap["essTsAlarms"]->setParam("lastTsAlarms", essTsAlarms);

//             if (essTsAlarms > 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsAlarms detected at time %2.3f \n", __func__, essTsAlarms, tNow);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s >> %d essTsAlarms cleared at time %2.3f\n", __func__, essTsAlarms, tNow);
//             }
//         }

//         if (initCnt != icnt)
//         {
//             amap["Timestamp"]->setParam("initCnt", icnt);

//             FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
//         }
//         return 0;
//     }


//     // this is the Asset Manager under the ess_controller instance
//     if (BypassTs)
//     {
//         ival = 1;
//         amap["essTsInit"]->addVal(ival);
//         return 0;
//     }
//     double rdFault = amap["Timestamp"]->getdParam("rdFault");
//     double rdAlarm = amap["Timestamp"]->getdParam("rdAlarm");
//     double rdReset = amap["Timestamp"]->getdParam("rdReset");

//     double TSseenTime = amap["Timestamp"]->getdParam("TSseenTime");
//     bool TSOk = amap["Timestamp"]->getbParam("TSOk");
//     bool seenTS = amap["Timestamp"]->getbParam("seenTS");
//     bool seenInit = amap["Timestamp"]->getbParam("seenInit");
//     bool seenOk = amap["Timestamp"]->getbParam("seenOk");
//     bool seenFault = amap["Timestamp"]->getbParam("seenFault");
//     bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");

//     // If we are in the init state wait for comms to start count down reset time
//     if (strcmp(currentTimestamp, initTimestamp) == 0)
//     {
//         // if not toally set up yet then quit this pass
//         if (!amap["amTsInit"])
//         {
//             return 0;
//         }

//         if (!seenInit)   // Ts_Setup
//         {
//             if (1)FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n", __func__, aname, BypassTs ? "true" : "false");

//             amap["Timestamp"]->setParam("seenInit", true);

//             char* cval = (char*)"Ts Init, no Timestamp Seen";
//             amap["TsState"]->setVal(cval);

//             ival = 1;
//             amap["essTsInit"]->addVal(ival);
//             amap["TsInit"]->setVal(0);      //Ts_Init  
//         }

//     }
//     else  // wait for comms to go past reset then set active or wait to alarm and then fault
//     {
//         //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTimestamp?lastTimestamp:"no last Value", tval1)
//         if (strcmp(currentTimestamp, lastTimestamp) != 0)
//         {

//             if (0)FPS_ERROR_PRINT("%s >> %s Timestamp change detected,  from [%s] to [%s] tNow %2.3f seenTS [%s]\n"
//                 , __func__, aname, currentTimestamp, lastTimestamp, tNow, seenTS ? "true" : "false");

//             amap["Timestamp"]->setParam("lastTimestamp", currentTimestamp);

//             //if(!seenTS)
//             {
//                 if (0)FPS_ERROR_PRINT("%s >> %s Timestamp set TSseenTime %2.3f \n"
//                     , __func__, aname, tNow);
//                 amap["Timestamp"]->setParam("seenTS", true);
//                 amap["Timestamp"]->setParam("TSseenTime", tNow);
//                 TSseenTime = tNow;
//                 seenTS = true;
//             }

//         }
//         else   // No Change , start tracking faults and alarms  but wait for hold time
//         {
//             TSseenTime = amap["Timestamp"]->getdParam("TSseenTime");
//             // allow holdoff between testing for change
//             if (seenTS)
//             {
//                 if ((tNow - TSseenTime) > toHold)
//                 {
//                     if (0)FPS_ERROR_PRINT("%s >> %s Timestamp stall detected  tNow %2.3f seebTime %2.3f .stalll time %2.3f toHold %2.3f \n"
//                         , __func__, aname, tNow, TSseenTime, (tNow - TSseenTime), toHold);

//                     amap["Timestamp"]->setParam("seenTS", false);
//                     seenTS = false;
//                     rdAlarm -= (tNow - TSseenTime);
//                     rdFault -= (tNow - TSseenTime);

//                     if (rdFault < 0.0)
//                     {
//                         rdFault = 0.0;;
//                     }
//                     if (rdAlarm < 0.0)
//                     {
//                         rdAlarm = 0.0;;
//                     }
//                     amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
//                     amap["Timestamp"]->setParam("rdFault", rdFault);

//                 }
//             }
//         }

//         if (seenTS)
//         {
//             if (!seenOk)
//             {
//                 if (rdReset > 0.0)
//                 {
//                     rdReset -= tDiff;
//                     amap["Timestamp"]->setParam("rdReset", rdReset);
//                 }
//             }

//             if ((rdReset <= 0.0) && !seenOk)
//             {
//                 if (seenFault)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  Timestamp fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Timestamp"]->setParam("seenFault", false);
//                     seenFault = false;
//                 }
//                 if (seenAlarm)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  Timestamp Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
//                     amap["Timestamp"]->setParam("seenAlarm", false);
//                     seenAlarm = false;
//                 }
//                 amap["Timestamp"]->setParam("seenOk", true);
//                 seenOk = true;
//                 amap["Timestamp"]->setParam("TSOk", true);
//                 TSOk = true;

//                 if (1)FPS_ERROR_PRINT("%s >>  Timestamp OK for  %s at %2.3f\n", __func__, aname, tNow);
//                 ival = Asset_Ok; // seen Timestamp change
//                 amap["TsStateNum"]->setVal(ival);
//                 ival = 0;
//                 amap["TsInit"]->setVal(ival);
//                 char* tval;
//                 asprintf(&tval, " Ts OK last set  Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if (tval)
//                 {
//                     amap["TsState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//                 amap["Timestamp"]->setParam("rdReset", toReset);
//             }
//         }
//         else  // not changed not onHold look out for errors
//         {
//             // we need to decrement the alarm / fault times
//             rdFault = amap["Timestamp"]->getdParam("rdFault");
//             rdAlarm = amap["Timestamp"]->getdParam("rdAlarm");
//             seenFault = amap["Timestamp"]->getbParam("seenFault");
//             seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");
//             if (0)FPS_ERROR_PRINT("%s >>  Timestamp stall for  %s at %2.3f rdFault %2.3f rdAlarm %2.3f TSOk [%s] seenTS [%s] tDiff %2.3f \n"
//                 , __func__, aname, tNow, rdFault, rdAlarm, TSOk ? "true" : "false", seenTS ? "true" : "false", tDiff);
//             if (rdFault > 0.0)
//             {
//                 rdFault -= tDiff;
//                 amap["Timestamp"]->setParam("rdFault", rdFault);
//             }
//             if (rdAlarm > 0.0)
//             {
//                 rdAlarm -= tDiff;
//                 amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
//             }

//             if ((rdFault < 0.0) && !seenFault)
//             {
//                 seenFault = true;
//                 amap["Timestamp"]->setParam("seenFault", true);
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 amap["Timestamp"]->setParam("seenAlarm", true);

//                 if (1)FPS_ERROR_PRINT("%s >>  Timestamp  Fault  for %s at %2.3f \n", __func__, aname, tNow);
//                 char* tval;
//                 asprintf(&tval, " Ts Fault last set Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if (tval)
//                 {
//                     amap["TsState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//                 ival = Asset_Fault; //Timestamp Fault
//                 amap["TsStateNum"]->setVal(ival);
//                 //seenOk = false;
//                 seenAlarm = true;

//                 int totalTsFaults = amap["Timestamp"]->getiParam("totalTsFaults");
//                 totalTsFaults++;
//                 amap["Timestamp"]->setParam("totalTsFaults", totalTsFaults);

//                 TSOk = false;
//                 amap["Timestamp"]->setParam("TSOk", false);

//                 if (am->am)
//                 {
//                     ival = 1;
//                     amap["amTsFaults"]->addVal(ival);
//                 }

//             }
//             else if ((rdAlarm < 0.0) && !seenAlarm)
//             {
//                 if (1)FPS_ERROR_PRINT("%s >> Timestamp  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

//                 char* tval;
//                 asprintf(&tval, "Ts Alarm last set Alarm %3.2f max %3.2f", toAlarm, toFault);
//                 if (tval)
//                 {
//                     amap["TsState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//                 // Just test code right now
//                 ival = Asset_Alarm; //Timestamp Alarm
//                 amap["TsStateNum"]->setVal(ival);

//                 amap["Timestamp"]->setParam("seenAlarm", true);
//                 seenAlarm = true;
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 int totalTsAlarms = amap["Timestamp"]->getiParam("totalTsAlarms");
//                 totalTsAlarms++;
//                 amap["Timestamp"]->setParam("totalTsAlarms", totalTsAlarms);

//                 TSOk = false;
//                 amap["Timestamp"]->setParam("TSOk", false);

//                 if (am->am)
//                 {
//                     amap["amTsAlarms"]->addVal(ival);
//                 }
//             }
//             else
//             {
//                 if (0)FPS_ERROR_PRINT("%s >> Ts for [%s] [%s] Stalled at %2.3f  Fault %2.3f Alarm %2.3f \n"
//                     , __func__
//                     , aname
//                     , amap["Timestamp"]->getcVal()
//                     , tNow
//                     , rdFault, rdAlarm);

//             }
//         }
//         if (seenFault)
//         {
//             int ival = 1;
//             amap["TsFaults"]->addVal(ival);
//             amap["essTsFaults"]->addVal(ival);
//         }
//         else
//         {
//             if (seenTS)
//             {
//                 if (rdFault < toFault)
//                 {
//                     rdFault += tDiff;
//                     amap["Timestamp"]->setParam("rdFault", rdFault);
//                 }
//             }
//         }

//         if (seenAlarm)
//         {
//             int ival = 1;
//             amap["TsAlarms"]->addVal(ival);
//             amap["essTsAlarms"]->addVal(ival);
//         }
//         else
//         {
//             if (seenTS)
//             {
//                 if (rdAlarm < toAlarm)
//                 {
//                     rdAlarm += tDiff;
//                     amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
//                 }
//             }
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
//     return 0;
// };


/**
 * Checks if the current setpoint (active and reactive) has changed and sends the results
 * to /components/pcs
 *
 * Review 11/06/2020
 *
 * Used in: test_jimmy.cpp -> ess manager
 * Test Script: test_BMS_current.sh
 */
 //BMS Single Cell Charge Over Voltage max 3.60 for 8 seconds Open DC Breaker (latched)
 //BMS Single Cell Charge Over Voltage less than 3.55 for 10 seconds not in charging state reset DC Breaker (latched)
 // just do the periodic function for now
 // this is a bit scrappy BUT 
 // you decare an enum to provide an index into a list of vectors
 // The SetUpxxxVec then has to populate the vector in exactly the right order.
 // 
// the Vec functions then can  just use the items off the vector list.
// One biggie... no more crashes due to spelling the names wring.
// I have yet to get these triggered from a config function but I'm working on it.
//     we just have to drop a pointer to the avmap into the asset framework.
// these two calls need to be made in the main wakeup 
// the first populates the avmap with the vectors 
// the MaxBusBarCurrent is the root name and all the other components are based on that toot.
// then call the standard function CheckOver/UnderLimits with the same "MaxBusBarCurrent" 
// the aname holds it alltogether.
// The key is to get the order right in the setup function.
// I'll think about a way to get the SetupFunction automaticaly called if needed.
// done it see checkover

//SetupLimitsVec(*am->vmap, am->amap, avmap, am->name.c_str(), "MaxBusBarCurrent", am);
//CheckOverLimitsVec(*am->vmap, avmap, am->name.c_str(), "MaxBusBarCurrent", am->p_fims, am);

namespace LimitsCheck
{
    enum
    {
        eCheckValue,
        eValueSeen,
        eValueOK,
        eValueLimit,
        eValueReset,
        eValueFault,
        eValueErrTime,
        eValueResetTime,
        eErrTime,
        eResetTime,
        eTLast,
        eSim,
        eValue
    };
};

// asset:var:vecs
// each asset has a list of vars to check each with a list of assetVar*
//std::map<std::string, std::map<std::string,   std::vector<assetVar*>>> AvMaxCellVoltage;
int SetupLimitsVec(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname, asset_manager* am);
/**
 * @brief Periodically check if the variable contains a value that is greater than the limit value
 * after a given time. If this is true, proceed to run fault-handling functions and then recover
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the vector that contains asset vars
 * @param aname the asset name
 * @param vname the variable name
 * @param p_fims the fims object used for data interchange
 * @param am the asset manager
 */
int CheckOverLimitsVec(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims, asset_manager* am)
{
    using namespace LimitsCheck;
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    auto avx = avmap[aname].find(vname);
    if (avx == avmap[aname].end())
    {
        if (1)FPS_ERROR_PRINT("%s >> %s --- Running Setup  for [%s]\n", __func__, aname, vname);
        SetupLimitsVec(vmap, am->amap, avmap, am->name.c_str(), vname, am);
    }
    std::vector<assetVar*>avec = avmap[aname][vname];

    assetVar* CheckValue = avec[eCheckValue];
    //int reload = CheckReload(vmap, am->amap, aname, vname);
    int reload = CheckValue->getiVal();
    // ValueSeen set by the assetVar holding the Valueage component value when it exceeded ValueLimit
    // ValueOK set by the assetVar holding the Valueage component value when it went below ValueReset
    // ValueFault is the assetVar indicating that we have a fault.
    // ErrTime / ResetTime are the working time values
    if (reload < 2)
    {
        //SetupValueage
        reload = 2;CheckValue->setVal(reload);
    }
    //bool bval = false;
    //double dval = 0.0;
    //int ival = 0;
    // first check for latched limits.
    bool ValueSeen = avec[eValueSeen]->getbVal();
    bool ValueOK = avec[eValueOK]->getbVal();
    bool ValueFault = avec[eValueFault]->getbVal();
    double  Value = avec[eValue]->getdVal();
    double  ValueLimit = avec[eValueLimit]->getdVal();
    double  ValueReset = avec[eValueReset]->getdVal();
    double  ValueErrTime = avec[eValueErrTime]->getdVal();
    double  ValueResetTime = avec[eValueResetTime]->getdVal();
    double  ErrTime = avec[eErrTime]->getdVal();
    double  ResetTime = avec[eResetTime]->getdVal();
    double  tLast = avec[eTLast]->getdVal();
    int     Sim = avec[eSim]->getiVal();

    assetVar* ValueAv = avec[eValue];
    assetVar* ValueSeenAv = avec[eValueSeen];
    assetVar* ValueOKAv = avec[eValueOK];
    assetVar* ValueFaultAv = avec[eValueFault];
    //assetVar* ErrTimeAv = avec[eErrTime];
    //assetVar* ResetTimeAv = avec[eResetTime];
    assetVar* ValueErrTimeAv = avec[eValueErrTime];
    assetVar* ValueResetTimeAv = avec[eValueResetTime];
    assetVar* tLastAv = avec[eTLast];
    assetVar* SimAv = avec[eSim];

    double tNow = vm->get_time_dbl();
    tLastAv->setVal(tNow);
    double tGap = tNow - tLast;
    double svalue = 0.01;
    // Checks if the current variable we're working with has a value that is greater 
    // than the limit value. If so, we need to be aware of this. This will help
    // us transition to fault state . Latch the value set
    //             ***
    // ***********************************************ValueLimit****************************
    //          *      *
    //        *          *
    // ***********************************************ValueReset****************************
    //     **              ***
    if (0)FPS_ERROR_PRINT("%s >> Value [%s] [%f] Seen [%s] OK [%s] Fault [%s] ErrTime %f  ResetTime %f eSim %d \n"
        , __func__
        , ValueAv->name.c_str()
        , Value
        , ValueSeen ? "true" : "false"
        , ValueOK ? "true" : "false"
        , ValueFault ? "true" : "false"
        , ValueErrTime
        , ValueResetTime
        , eSim
    );
    //////////////////////////////////////////////////////////////////////////////////
    if (Sim == 1)
    {

        if (Value < ValueLimit + 1)
        {
            ValueAv->addVal(svalue);
        }
        else
        {
            SimAv->setVal(2);
        }

    }
    if (Sim == 2)
    {

        if (Value > ValueReset - 1)
        {
            ValueAv->subVal(svalue);
        }
        else
        {

            SimAv->setVal(1);
        }

    }

    //double dval = 0.0;
    //double dval1 = 0.0;

    if (Sim != 0)
    {
        if (1)FPS_ERROR_PRINT("%s >> Value [%s] [%f]->[%f] Seen [%s] OK [%s] Fault [%s] ErrTime %f  ResetTime %f \n"
            , __func__
            , ValueAv->name.c_str()
            , ValueAv->getdLVal()
            , ValueAv->getdVal()
            , ValueSeen ? "true" : "false"
            , ValueOK ? "true" : "false"
            , ValueFault ? "true" : "false"
            , ValueErrTime
            , ValueResetTime
        );
    }
    //bool b1 = ValueOKAv->getbVal();
    //bool b2 = ValueOKAv->getbLVal();
    ///////////////////////////////////////////////////////////////////////////////////
    if (Value > ValueLimit)
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (ValueResetTime < ResetTime)
        {
            // Adjust up Recovery time 
            ValueResetTimeAv->setVal(ValueResetTime + tGap);
        }
    }

    // If the current variable is greater than the reset value, then we need to be
    // aware of this. This will help us transition out of fault state
    if (Value < ValueReset)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);
        }
        if (ValueErrTime < ErrTime)
        {
            // Adjust up Err time 
            ValueErrTimeAv->setVal(ValueErrTime + tGap);
        }
    }

    if (ValueFault)
    {
        // If we are faulted but now things are OK then reset fault
        if (ValueOK)
        {
            if (ValueResetTime > 0.0)
            {
                // decrease Reset time 
                ValueResetTimeAv->setVal(ValueResetTime - tGap);
            }

            if (ValueResetTime <= 0.0)
            {
                ValueFaultAv->setVal(false);
                ValueSeenAv->setVal(false);
                //CloseDCBreaker()

                // Reset Fault Alarm (AssetManager)
                //am->ResetFault(ValueageAv, tNow," Valueage Alarm" );
                // trigger Fault actions (assetManager)
                //am->trigger_wakeup(BMS_FAULT_RESET);
            }
        }
    }
    // Transition into fault state and perform fault-handling tasks
    // if the current variable has a value that is greater than the
    // the limit value after a given time
    else
    {
        //if (ValueSeen && (tNow - ValueSeenAv->getSetTime()) > ValueErrTime)
        if (ValueSeen)
        {
            if (ValueErrTime > 0.0)
            {
                // decrease Err time 
                ValueErrTimeAv->setVal(ValueErrTime - (tNow - tLast));
            }
            if (ValueErrTime <= 0.0)
            {
                ValueFaultAv->setVal(true);
                ValueOKAv->setVal(false);
                //ValueSeenAv->setVal(false);
                //OpenDCBreaker()
                // Create Fault Alarm (AssetManager)
                //am->CreateFault(ValueageAv, tNow," Valueage Alarm" );
                // trigger Fault actions (assetManager)
                //am->trigger_wakeup(BMS_FAULT);
            }

        }
    }
    return 0;
}
//typedef std::map<std::string, std::map<std::string, std::vector<assetVar* >>> avarmap;
/**
 * @brief Periodically check if the variable contains a value that is less than the limit value
 * after a given time. If this is true, proceed to run fault-handling functions and then recover
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the vector that contains asset vars
 * @param aname the asset name
 * @param vname the variable name
 * @param p_fims the fims object used for data interchange
 * @param am the asset manager
 */
int CheckUnderLimitsVec(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims, asset_manager* am)
{
    using namespace LimitsCheck;

    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    auto avx = avmap[aname].find(vname);
    if (avx == avmap[aname].end())
    {
        if (1)FPS_ERROR_PRINT("%s >> %s --- Running Setup  for [%s]\n", __func__, aname, vname);
        SetupLimitsVec(vmap, am->amap, avmap, am->name.c_str(), vname, am);
    }
    std::vector<assetVar*>avec = avmap[aname][vname];
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    assetVar* CheckValue = avec[eCheckValue];
    int reload = 0;
    reload = CheckValue->getiVal();
    //reload = vm->CheckReload(vmap, amap, aname, __func__);

    // ValueSeen set by the assetVar holding the Valueage component value when it exceeded ValueLimit
    // ValueOK set by the assetVar holding the Valueage component value when it went below ValueReset
    // ValueFault is the assetVar indicating that we have a fault.
    if (reload < 2)
    {
        //SetupValueage
    }
    //int bval = false;
    //double dval = 0.0;
    //int ival = 0;
    // first check for latched limits.
    bool ValueSeen = avec[eValueSeen]->getbVal();
    bool ValueOK = avec[eValueOK]->getbVal();
    bool ValueFault = avec[eValueFault]->getbVal();
    double  Value = avec[eValue]->getdVal();
    double  ValueLimit = avec[eValueLimit]->getdVal();
    double  ValueReset = avec[eValueReset]->getdVal();
    double  ValueErrTime = avec[eValueErrTime]->getdVal();
    double  ValueResetTime = avec[eValueResetTime]->getdVal();
    double  ErrTime = avec[eErrTime]->getdVal();
    double  ResetTime = avec[eResetTime]->getdVal();
    //double  ErrTime = avec[eErrTime]->getdVal();
    //double  ResetTime = avec[eResetTime]->getdVal();
    double  tLast = avec[eTLast]->getdVal();
    int     Sim = avec[eSim]->getiVal();


    assetVar* ValueErrTimeAv = avec[eValueErrTime];
    assetVar* ValueResetTimeAv = avec[eValueResetTime];

    assetVar* ValueAv = avec[eValue];
    assetVar* tLastAv = avec[eTLast];
    assetVar* SimAv = avec[eSim];


    //assetVar* ValueAv              = avec[eValue];
    assetVar* ValueSeenAv = avec[eValueSeen];
    assetVar* ValueOKAv = avec[eValueOK];
    assetVar* ValueFaultAv = avec[eValueFault];
    assetVar* ValueResetAv = avec[eValueReset];
    assetVar* ValueLimitAv = avec[eValueLimit];

    double tNow = vm->get_time_dbl();
    tLastAv->setVal(tNow);
    double tGap = tNow - tLast;
    double svalue = 0.01;

    // Checks if the current variable we're working with has a value that is greater 
    // than the limit value. If so, we need to be aware of this. This will help
    // us transition to fault state . Latch the value set
    //             ***
    // ***********************************************ValueReset****************************3.5
    //          *      *
    //        *          *
    // ***********************************************ValueLimit****************************3.4
    //     **              ***
    //////////////////////////////////////////////////////////////////////////////////

    if (Sim == 1)
    {

        if (Value > ValueLimit - 1)
        {
            ValueAv->subVal(svalue);
        }
        else
        {
            SimAv->setVal(2);
        }

    }
    if (Sim == 2)
    {

        if (Value < ValueReset + 1)
        {
            ValueAv->addVal(svalue);
        }
        else
        {
            SimAv->setVal(1);
        }

    }

    //double dval = 0.0;
    //double dval1 = 0.0;

    if (Sim != 0)
    {
        if (1)FPS_ERROR_PRINT("%s >> Sim [%d] Value [%s] [%f]->[%f] Limit [%f] Reset [%f] Seen [%s] OK [%s] Fault [%s] ErrTime %f  ResetTime %f \n"
            , __func__
            , Sim
            , ValueAv->name.c_str()
            , ValueAv->getdLVal()
            , ValueAv->getdVal()
            , ValueLimit
            , ValueReset
            , ValueSeen ? "true" : "false"
            , ValueOK ? "true" : "false"
            , ValueFault ? "true" : "false"
            , ValueErrTime
            , ValueResetTime
        );
        // fix crazy stuff
        if (ValueReset < ValueLimit)
        {
            ValueResetAv->setVal(3.5);
            ValueLimitAv->setVal(3.4);

        }

    }
    //
    // Checks if the current variable we're working with has a value that is less
    // than the limit value. If so, we need to be aware of this. This will help
    // us transition to fault state
    ///////////////////////////////////////////////////////////////////////////////////
    if (Value < ValueLimit)
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (ValueResetTime < ResetTime)
        {
            // Adjust up Recovery time 
            ValueResetTimeAv->setVal(ValueResetTime + tGap);
        }
    }

    // If the current variable is greater than the reset value, then we need to be
    // aware of this. This will help us transition out of fault state
    if (Value > ValueReset)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);
        }
        if (ValueErrTime < ErrTime)
        {
            // Adjust up Err time 
            ValueErrTimeAv->setVal(ValueErrTime + tGap);
        }
    }

    if (ValueFault)
    {
        // If we are faulted but now things are OK then reset fault
        if (ValueOK)
        {
            if (ValueResetTime > 0.0)
            {
                // decrease Reset time 
                ValueResetTimeAv->setVal(ValueResetTime - tGap);
            }

            if (ValueResetTime <= 0.0)
            {
                ValueFaultAv->setVal(false);
                ValueSeenAv->setVal(false);
                //CloseDCBreaker()

                // Reset Fault Alarm (AssetManager)
                //am->ResetFault(ValueageAv, tNow," Valueage Alarm" );
                // trigger Fault actions (assetManager)
                //am->trigger_wakeup(BMS_FAULT_RESET);
            }
        }
    }
    // Transition into fault state and perform fault-handling tasks
    // if the current variable has a value that is greater than the
    // the limit value after a given time
    else
    {
        //if (ValueSeen && (tNow - ValueSeenAv->getSetTime()) > ValueErrTime)
        if (ValueSeen)
        {
            if (ValueErrTime > 0.0)
            {
                // decrease Err time 
                ValueErrTimeAv->setVal(ValueErrTime - (tNow - tLast));
            }
            if (ValueErrTime <= 0.0)
            {
                ValueFaultAv->setVal(true);
                ValueOKAv->setVal(false);
                //ValueSeenAv->setVal(false);
                //OpenDCBreaker()
                // Create Fault Alarm (AssetManager)
                //am->CreateFault(ValueageAv, tNow," Valueage Alarm" );
                // trigger Fault actions (assetManager)
                //am->trigger_wakeup(BMS_FAULT);
            }

        }
    }

    // if (!ValueSeen)
    // {
    //     if (Value < ValueLimit)
    //     {
    //         ValueSeenAv->setVal(true);
    //     }

    // }
    // // else
    // // {
    // //     if(Value <ValueReset) truncateError Time
    // //     {
    // //         ValueSeenAv->setVal(true);
    // //     }
    // // when its been OK for RESET Time restore Error Time

    // // }

    // // If the current variable is greater than the reset value, then we need to be
    // // aware of this. This will help us transition out of fault state
    // if (!ValueOK)
    // {
    //     if (Value > ValueReset)
    //     {
    //         ValueOKAv->setVal(true);
    //     }
    // }

    // // If we have a fault, and the current variable has a value that is greater
    // // than the limit value after a given time, then reset variables and transition
    // // out of fault state
    // if (ValueFault)
    // {
    //     if ((ValueOK) && ((tNow - ValueOKAv->getSetTime()) > ValueResetTime))
    //     {
    //         ValueFaultAv->setVal(false);
    //         ValueSeenAv->setVal(false);
    //         //CloseDCBreaker()

    //         // Reset Fault Alarm (AssetManager)
    //     //am->ResetFault(ValueageAv, tNow," Valueage Alarm" );
    //     // trigger Fault actions (assetManager)
    //     //am->trigger_wakeup(BMS_FAULT_RESET);

    //     }
    // }
    // // Transition into fault state and perform fault-handling tasks
    // // if the current variable has a value that is less than the
    // // the limit value after a given time
    // else
    // {
    //     if (ValueSeen && (tNow - ValueSeenAv->getSetTime()) > ValueErrTime)
    //     {
    //         ValueFaultAv->setVal(true);
    //         ValueOKAv->setVal(false);
    //         //OpenDCBreaker()
    //     // Create Fault Alarm (AssetManager)
    //     //am->CreateFault(ValueageAv, tNow," Valueage Alarm" );
    //     // trigger Fault actions (assetManager)
    //     //am->trigger_wakeup(BMS_FAULT);
    //     }
    // }

    return 0;
}

int CheckLimitsVec(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims, asset_manager* am)
{
    using namespace LimitsCheck;

    std::vector<assetVar*>avec = avmap[aname][vname];
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    assetVar* CheckValue = avec[eCheckValue];
    int reload = 0;
    //bool bval = false;
    //double dval = 0.0;
    reload = CheckValue->getiVal();
    // ValueSeen set by the assetVar holding the Valueage component value when it exceeded ValueLimit
    // ValueOK set by the assetVar holding the Valueage component value when it went below ValueReset
    // ValueFault is the assetVar indicating that we have a fault.
    if (reload < 2)
    {
        //SetupValueage
    }
    // first check for latched limits.
    bool ValueSeen = avec[eValueSeen]->getbVal();
    bool ValueOK = avec[eValueOK]->getbVal();
    bool ValueFault = avec[eValueFault]->getbVal();
    double  Value = avec[eValue]->getdVal();
    double  ValueLimit = avec[eValueLimit]->getdVal();
    double  ValueReset = avec[eValueReset]->getdVal();
    double  ValueErrTime = avec[eValueReset]->getdVal();
    double  ValueResetTime = avec[eValueResetTime]->getdVal();
    //assetVar* ValueAv              = avec[eValue];
    assetVar* ValueSeenAv = avec[eValueSeen];
    assetVar* ValueOKAv = avec[eValueOK];
    assetVar* ValueFaultAv = avec[eValueFault];

    double tNow = vm->get_time_dbl();

    if (!ValueSeen)
    {
        if (Value > ValueLimit)
        {
            ValueSeenAv->setVal(true);
        }
    }
    // else
    // {
    //     if(Value <ValueReset) truncateError Time
    //     {
    //         ValueSeenAv->setVal(true);
    //     }
    // when its been OK for RESET Time restore Error Time

    // }


    if (!ValueOK)
    {
        if (Value < ValueReset)
        {
            ValueOKAv->setVal(true);
        }
    }

    if (ValueFault)
    {
        if ((ValueOK) && ((tNow - ValueOKAv->getSetTime()) > ValueResetTime))
        {
            ValueFaultAv->setVal(false);
            ValueSeenAv->setVal(false);
            //CloseDCBreaker()

            // Reset Fault Alarm (AssetManager)
        //am->ResetFault(ValueageAv, tNow," Valueage Alarm" );
        // trigger Fault actions (assetManager)
        //am->trigger_wakeup(BMS_FAULT_RESET);

        }
    }
    else
    {
        if (ValueSeen && (tNow - ValueSeenAv->getSetTime()) > ValueErrTime)
        {
            ValueFaultAv->setVal(true);
            ValueOKAv->setVal(false);
            //OpenDCBreaker()
        // Create Fault Alarm (AssetManager)
        //am->CreateFault(ValueageAv, tNow," Valueage Alarm" );
        // trigger Fault actions (assetManager)
        //am->trigger_wakeup(BMS_FAULT);
        }
    }

    return 0;
}


//BMS Single Cell Charge Over Voltage max 3.60 for 8 seconds Open DC Breaker (latched)
//BMS Single Cell Charge Over Voltage less than 3.55 for 10 seconds not in charging state reset DC Breaker (latched)
// just do the periodic function for now
//  eCheckValue,
//     eValueSeen,
//     eValueOK,
//     eValueLimit,
//     eValueReset,
//     eValueFault,
//     eValueErrTime,
//     eValueResetTime,
//     eErrTime,
//     eResetTime,
//     eTLast,
//     eSim,
//     eValue
// just for fun run this with aname = ess vname = Valueage
int SetupLimitsVec(varsmap& vmap, varmap& amap, std::map<std::string, std::map<std::string, std::vector<assetVar*>>>& avmap, const char* aname, const char* vname, asset_manager* am)
{
    using namespace LimitsCheck;

    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;

    std::string mname = vname;
    std::string mvar = "Check" + mname;

    assetVar* CheckValue = amap[mvar.c_str()];
    //char *tval = (char *)" Asset Init";
    int reload = 0;
    bool bval = false;
    //int ival = -1;
    double dval = 0.0;
    if (!CheckValue || (reload = CheckValue->getiVal()) == 0)
    {
        reload = 0;
    }

    if (reload < 2)
    {
        if (1)FPS_ERROR_PRINT("%s >> %s [%s]--- Running  \n", __func__, aname, vname);

        if (!avmap[aname][vname].empty())
        {
            avmap[aname][vname].clear();
        }

        // note links must set these values
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/reload", mvar.c_str(), reload); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Seen";         amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "OK";           amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);


        //amap["ValueSeen"]             = vm->setLinkVal(vmap, aname, "/status", "ValueSeen",  bval);
        //amap["ValueOK"]               = vm->setLinkVal(vmap, aname, "/status", "ValueOK",  bval);
        mvar = mname + "Limit";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Reset";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);

        //amap["ValueLimit"]            = vm->setLinkVal(vmap, aname, "/preset", "ValueLimit",  dval);
        //amap["ValueReset"]            = vm->setLinkVal(vmap, aname, "/config", "ValueReset",  dval);
        mvar = mname + "Fault";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);

        //amap["ValueFault"]           = vm->setLinkVal(vmap, aname, "/asset", "ValueFault",  bval);
        mvar = mname + "ErrTime";      amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetTime";    amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);

        //amap["ValueErrTime"]       = vm->setLinkVal(vmap, aname, "/config", "ValueErrTime",  dval);
        //amap["ValueResetTime"]     = vm->setLinkVal(vmap, aname, "/config", "ValueResetTime",  dval);
        //amap["Value"]              = vm->setLinkVal(vmap, aname, "/status", "Value",  dval);
        //eErrTime,
        //eResetTime,
        mvar = mname + "ErrCfg";      amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetCfg";    amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "TLast";       amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Sim";         amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname;                 amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);

        //eTLast,
        if (reload < 1)
        {
            avmap[aname][vname][eValueSeen]->setVal(false);
            avmap[aname][vname][eValueOK]->setVal(false);
            avmap[aname][vname][eValueFault]->setVal(false);
            dval = 3.6;
            avmap[aname][vname][eValueLimit]->setVal(dval);
            dval = 3.5;
            avmap[aname][vname][eValueReset]->setVal(dval);
            dval = 8.0;
            avmap[aname][vname][eValueErrTime]->setVal(dval);
            avmap[aname][vname][eErrTime]->setVal(dval);
            dval = 10.0;
            avmap[aname][vname][eValueResetTime]->setVal(dval);
            avmap[aname][vname][eResetTime]->setVal(dval);
            dval = 3.2;
            avmap[aname][vname][eValue]->setVal(dval);
            dval = vm->get_time_dbl();
            avmap[aname][vname][eTLast]->setVal(dval);
            int ival = 0;
            avmap[aname][vname][eSim]->setVal(ival);

        }
        reload = 2;
        avmap[aname][vname][eCheckValue]->setVal(reload);

        FPS_ERROR_PRINT("%s >> use: fims_send -m set -r /$$ -u /config/%s '{\"%sSim\":true}' to run Simulation \n"
            , __func__
            , aname
            , vname
        );


    }
    return 0;
}
// MaxCellVoltSeen set by the assetVar holding the MaxCellVoltage component value when it exceeded MaxCellVoltLimit
// MaxCellVoltOK set by the assetVar holding the MaxCellVoltage component value when it went below MaxCellVoltReset
// MaxCellVoltFault is the assetVar indicating that we have a fault.

//BMS Single Cell Charge Over Voltage max 3.60 for 8 seconds Open DC Breaker (latched)
//BMS Single Cell Charge Over Voltage less than 3.55 for 10 seconds not in charging state reset DC Breaker (latched)
// just do the periodic function for now
int CheckMaxCellVoltage(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  \n", __func__, aname);
    int reload = vm->CheckReload(vmap, amap, aname, __func__, (void*)&CheckMaxCellVoltage);
    //    assetVar* CheckMaxCellVoltage = amap["CheckMaxCellVoltage"];
        //char *tval = (char *)" Asset Init";
        //int reload = 0;
    bool bval = false;
    //int ival = -1;
    double dval = 0.0;
    // if (!CheckMaxCellVoltage || (reload = CheckMaxCellVoltage->getiVal()) == 0)
    // {
    //     reload = 0;
    // }

    if (reload < 2)
    {
        // note links must set these values
        amap["CheckMaxCellVoltage"] = vm->setLinkVal(vmap, aname, "/reload", "CheckMaxCellVoltage", reload);
        amap["MaxCellVoltSeen"] = vm->setLinkVal(vmap, aname, "/status", "MaxCellVoltSeen", bval);
        amap["MaxCellVoltOK"] = vm->setLinkVal(vmap, aname, "/status", "MaxCellVoltOK", bval);
        amap["MaxCellVoltLimit"] = vm->setLinkVal(vmap, aname, "/preset", "MaxCellVoltLimit", dval);
        amap["MaxCellVoltReset"] = vm->setLinkVal(vmap, aname, "/config", "MaxCellVoltReset", dval);
        amap["MaxCellVoltFault"] = vm->setLinkVal(vmap, aname, "/asset", "MaxCellVoltFault", bval);
        amap["MaxCellVoltErrTime"] = vm->setLinkVal(vmap, aname, "/config", "MaxCellVoltErrTime", dval);
        amap["MaxCellVoltResetTime"] = vm->setLinkVal(vmap, aname, "/config", "MaxCellVoltResetTime", dval);
        amap["MaxCellVolt"] = vm->setLinkVal(vmap, aname, "/status", "MaxCellVolt", dval);
        if (reload < 1)
        {
            amap["MaxCellVoltSeen"]->setVal(false);
            amap["MaxCellVoltOK"]->setVal(false);
            amap["MaxCellVoltFault"]->setVal(false);
            dval = 3.6;
            amap["MaxCellVoltLimit"]->setVal(dval);
            dval = 3.5;
            amap["MaxCellVoltReset"]->setVal(dval);
            dval = 8.0;
            amap["MaxCellVoltErrTime"]->setVal(dval);
            dval = 10.0;
            amap["MaxCellVoltResetTime"]->setVal(dval);
            dval = 3.6;
            amap["MaxCellVolt"]->setVal(dval);
        }
        reload = 2;
        amap["CheckMaxCellVoltage"]->setVal(reload);

    }
    // MaxCellVoltSeen set by the assetVar holding the MaxCellVoltage component value when it exceeded MaxCellVoltLimit
    // MaxCellVoltOK set by the assetVar holding the MaxCellVoltage component value when it went below MaxCellVoltReset
    // MaxCellVoltFault is the assetVar indicating that we have a fault.

    // first check for latched limits.
    bool MaxCellVoltSeen = amap["MaxCellVoltSeen"]->getbVal();
    bool MaxCellVoltOK = amap["MaxCellVoltOK"]->getbVal();
    bool MaxCellVoltFault = amap["MaxCellVoltFault"]->getbVal();
    double  MaxCellVolt = amap["MaxCellVolt"]->getdVal();
    double  MaxCellVoltLimit = amap["MaxCellVoltLimit"]->getdVal();
    double  MaxCellVoltReset = amap["MaxCellVoltReset"]->getdVal();
    double  MaxCellVoltErrTime = amap["MaxCellVoltReset"]->getdVal();
    double  MaxCellVoltResetTime = amap["MaxCellVoltResetTime"]->getdVal();
    //assetVar* MaxCellVoltAv              = amap["MaxCellVolt"];
    assetVar* MaxCellVoltSeenAv = amap["MaxCellVoltSeen"];
    assetVar* MaxCellVoltOKAv = amap["MaxCellVoltOK"];
    assetVar* MaxCellVoltFaultAv = amap["MaxCellVoltFault"];

    double tNow = vm->get_time_dbl();

    if (!MaxCellVoltSeen)
    {
        if (MaxCellVolt > MaxCellVoltLimit)
        {
            MaxCellVoltSeenAv->setVal(true);
        }
    }
    // else
    // {
    //     if(MaxCellVolt <MaxCellVoltReset) truncateError Time
    //     {
    //         MaxCellVoltSeenAv->setVal(true);
    //     }
    // when its been OK for RESET Time restore Error Time

    // }


    if (!MaxCellVoltOK)
    {
        if (MaxCellVolt < MaxCellVoltReset)
        {
            MaxCellVoltOKAv->setVal(true);
        }
    }

    if (MaxCellVoltFault)
    {
        if ((MaxCellVoltOK) && ((tNow - MaxCellVoltOKAv->getSetTime()) > MaxCellVoltResetTime))
        {
            MaxCellVoltFaultAv->setVal(false);
            MaxCellVoltSeenAv->setVal(false);
            //CloseDCBreaker()

            // Reset Fault Alarm (AssetManager)
        //am->ResetFault(MaxCellVoltageAv, tNow," MaxCellVoltage Alarm" );
        // trigger Fault actions (assetManager)
        //am->trigger_wakeup(BMS_FAULT_RESET);

        }
    }
    else
    {
        if (MaxCellVoltSeen && (tNow - MaxCellVoltSeenAv->getSetTime()) > MaxCellVoltErrTime)
        {
            MaxCellVoltFaultAv->setVal(true);
            MaxCellVoltOKAv->setVal(false);
            //OpenDCBreaker()
        // Create Fault Alarm (AssetManager)
        //am->CreateFault(MaxCellVoltageAv, tNow," MaxCellVoltage Alarm" );
        // trigger Fault actions (assetManager)
        //am->trigger_wakeup(BMS_FAULT);
        }
    }

    return 0;
}

// How to test 
// set up /config/ess/expectAssetRespVal response
// set up /config/ess/resetAssetStatusVal
// send   /controls/ess/AssetStatusVal
// run in ess_wakeup for now before 
//int SendAssetCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager* am)
//int TestAssetCmdAm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am)

namespace AssetCmd
{
    enum
    {
        eCheckValue,
        eValueCmd,
        eValueResp,
        eValueExp,
        eValueRequest,
        eValueOK,
        eValueFault,
        eValueMaxRetries,
        eValueRetryCount,
        eValueCmdTimeOut,
        eTLast,
        eSim,
        eValue
    };
};
int SetupCmdVec(varsmap& vmap, varmap& amap, std::map<std::string, std::map<std::string, std::vector<assetVar*>>>& avmap, const char* aname, const char* vname, asset_manager* am)
{
    using namespace AssetCmd;

    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;

    std::string mname = vname;
    std::string mvar = "Check" + mname;

    assetVar* CheckValue = amap[mvar.c_str()];
    //char *tval = (char *)" Asset Init";
    // int reload = CheckReload(vmap, am->amap, aname, mvar.c_str(),&CheckMaxCellVoltage);
    int reload = 0;
    bool bval = false;
    int ival = -1;
    double dval = 0.0;
    if (!CheckValue || (reload = CheckValue->getiVal()) == 0)
    {
        reload = 0;
    }

    if (reload < 2)
    {
        if (1)FPS_ERROR_PRINT("%s >> %s [%s]--- Running  \n", __func__, aname, vname);

        if (!avmap[aname][vname].empty())
        {
            avmap[aname][vname].clear();
        }

        // note links must set these values
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/reload", mvar.c_str(), reload); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Cmd";       amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/controls", mvar.c_str(), ival);   avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Resp";      amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/controls", mvar.c_str(), ival);   avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Exp";       amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/controls", mvar.c_str(), ival);   avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Request";   amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/controls", mvar.c_str(), bval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "OK";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Fault";     amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);


        //amap["ValueSeen"]             = vm->setLinkVal(vmap, aname, "/status", "ValueSeen",  bval);
        //amap["ValueOK"]               = vm->setLinkVal(vmap, aname, "/status", "ValueOK",  bval);
        mvar = mname + "MaxRetries";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "RetryCount";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);

        //amap["ValueLimit"]            = vm->setLinkVal(vmap, aname, "/preset", "ValueLimit",  dval);
        //amap["ValueReset"]            = vm->setLinkVal(vmap, aname, "/config", "ValueReset",  dval);
        mvar = mname + "CmdTimeOut";        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), dval);   avmap[aname][vname].push_back(amap[mvar.c_str()]);

        mvar = mname + "TLast";       amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Sim";         amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname;                 amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), dval); avmap[aname][vname].push_back(amap[mvar.c_str()]);

        //eTLast,
        if (reload < 1)
        {
            ival = 0;
            avmap[aname][vname][eValueCmd]->setVal(ival);
            avmap[aname][vname][eValueRequest]->setVal(ival);
            avmap[aname][vname][eValueResp]->setVal(ival);
            avmap[aname][vname][eValueFault]->setVal(false);
            avmap[aname][vname][eValueOK]->setVal(false);
            dval = 3.6;
            avmap[aname][vname][eValueCmdTimeOut]->setVal(dval);
            dval = vm->get_time_dbl();
            avmap[aname][vname][eTLast]->setVal(dval);
            int ival = 0;
            avmap[aname][vname][eSim]->setVal(ival);

        }
        reload = 2;
        avmap[aname][vname][eCheckValue]->setVal(reload);

    }
    return 0;
}

int SendCmdVec(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims, asset_manager* am)
{
    using namespace AssetCmd;
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    auto avx = avmap[aname].find(vname);
    if (avx == avmap[aname].end())
    {
        if (1)FPS_ERROR_PRINT("%s >> %s --- Running Setup  for [%s]\n", __func__, aname, vname);
        SetupCmdVec(vmap, am->amap, avmap, am->name.c_str(), vname, am);
    }
    std::vector<assetVar*>avec = avmap[aname][vname];
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    assetVar* CheckValue = avec[eCheckValue];
    int reload = 0;
    reload = CheckValue->getiVal();
    // ValueSeen set by the assetVar holding the Valueage component value when it exceeded ValueLimit
    // ValueOK set by the assetVar holding the Valueage component value when it went below ValueReset
    // ValueFault is the assetVar indicating that we have a fault.
    if (reload < 2)
    {
        //SetupValueage
    }
    // enum {
    //     eCheckValue,
    //     eValueCmd,
    //     eValueResp,
    //     eValueRespExpected,
    //     eValueOk,
    //     eValueFault,
    //     eValueMaxRetries,
    //     eValueRetryCount,
    //     eValueCmdTimeOut,
    //     eTLast,
    //     eSim,
    //     eValue
    // };
    // first check for latched limits.
    //int ival = 0;
    //bool bval = false;
    //double dval = 0.0;
    //int     ValueCmd = avec[eValueCmd]->getiVal();
    //int     ValueResp = avec[eValueResp]->getiVal();
    //int     ValueExp = avec[eValueExp]->getiVal();
    //bool    ValueOK = avec[eValueOK]->getbVal();
    int     ValueRequest = avec[eValueRequest]->getiVal();
    //bool    ValueFault = avec[eValueFault]->getbVal();
    //double  Value = avec[eValue]->getdVal();
    //int     ValueMaxRetries = avec[eValueMaxRetries]->getiVal();
    //int     ValueRetryCount = avec[eValueRetryCount]->getiVal();
    //int     Sim = avec[eSim]->getiVal();
    int     Value = avec[eValue]->getiVal();

    //double  ValueReset = avec[eValueReset]->getdVal();
    //double  ValueCmdTimeOut = avec[eValueCmdTimeOut]->getdVal();
    //assetVar* ValueAv              = avec[eValue];
    //assetVar* TLastAv = avec[eTLast];
    assetVar* ValueOKAv = avec[eValueOK];
    assetVar* ValueRequestAv = avec[eValueRequest];
    assetVar* ValueRetryCountAv = avec[eValueRetryCount];
    assetVar* ValueFaultAv = avec[eValueFault];
    assetVar* ValueCmdAv = avec[eValueCmd];
    //assetVar* ValueRespAv = avec[eValueResp];

    //double tNow = vm->get_time_dbl();
    // 0 = inactive, 1 = new command. 2 = command in progress
    if (ValueRequest != 0)
    {
        if (ValueRequest == 1)
        {
            ValueOKAv->setVal(false);
            ValueFaultAv->setVal(false);
            ValueRetryCountAv->setVal(0);
            // send the command eValue to the asset. eValueCmd 
            ValueCmdAv->setVal(Value);   //out it goes
            ValueRequestAv->setVal(2);
        }
        // if (ValueRequest == 2)
        // {
        //     if (ValueResp != ValueExp)&&
        // }

    }
    return 0;
}

int SendAssetCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    int reload;
    //double dval = 0.0;
    double tSetTime = 1.0;   //1 second may be a config var
    double tRespTime = 5.0; //5 seconds may be a config var
    bool bval = false;
    int ival = -1;   // never used as a status request

    VarMapUtils* vm = nullptr;
    if (am->vm)
    {
        vm = am->vm;
    }
    else
    {
        return -1;
    }
    // maybe use av->name as the aname .. gives each trigger var its own context        
    //amap["AssetRespTime"]         = vm->setLinkVal(vmap, aname,    "/status",       "AssetRespTime",            dval);
    // LocalVars may need to come from triggerVar context
    const char* LTestAssetStatusAv = "TestAssetStatusAv";
    const char* LAssetStatusCmd = "AssetStatusCmd";
    const char* LresetAssetStatusVal = "resetAssetStatusVal";
    const char* LlastAssetStatusVal = "lastAssetStatusVal";
    const char* LexpectAssetRespVal = "expectAssetRespVal";
    const char* LAssetSendNum = "AssetSendNum";
    const char* LAssetSendTime = "AssetSendTime";


    // Other vars referenced anyway 
    const char* LAssetStatusSendReg = "AssetStatusSendReg";
    const char* LAssetStatusRespName = "AssetStatusRespName";
    const char* LAssetStatusVal = "AssetStatusVal";
    const char* LAssetRespVal = "AssetRespVal";
    const char* LlastAssetRespVal = "lastAssetRespVal";
    const char* LmaxAssetSendNum = "maxAssetSendNum";
    const char* LAssetRespOk = "AssetRespOk";
    const char* LAssetRespTime = "AssetRespTime";
    const char* LAssetRespErr = "AssetRespErr";
    const char* LAssetRespWarn = "AssetRespWarn";

    // TODO refresh all of the above from the TriggerAsetVar params
    //              fvalname       param name     default
    //av->loadFunVal(LAssetSendNum,"AssetSendNum","AssetSendNum");


    char* tVal = (char*)"status";
    double dval = 0.0;
    //int ival = 0;
    //bool bval = false;
    reload = vm->CheckReload(vmap, amap, aname, LTestAssetStatusAv, (void*)&SendAssetCmd);

    assetVar* TestAssetStatusAv = amap[LTestAssetStatusAv];
    //const char * Av_aname = av->name.c_str();
    if (reload < 2)
    {
        //reload = 0;
        // loval vars uncer assetVar name
        amap[LTestAssetStatusAv] = vm->setLinkVal(vmap, aname, "/reload", LTestAssetStatusAv, reload);
        amap[LAssetStatusCmd] = vm->setLinkVal(vmap, aname, "/status", LAssetStatusCmd, ival);
        amap[LresetAssetStatusVal] = vm->setLinkVal(vmap, aname, "/configs", LresetAssetStatusVal, ival);
        amap[LlastAssetStatusVal] = vm->setLinkVal(vmap, aname, "/status", LlastAssetStatusVal, ival);
        amap[LlastAssetRespVal] = vm->setLinkVal(vmap, aname, "/status", LlastAssetRespVal, ival);
        amap[LexpectAssetRespVal] = vm->setLinkVal(vmap, aname, "/status", LexpectAssetRespVal, ival);
        amap[LAssetSendNum] = vm->setLinkVal(vmap, aname, "/status", LAssetSendNum, ival);
        amap[LAssetSendTime] = vm->setLinkVal(vmap, aname, "/status", LAssetSendTime, dval);

        // iterface vars in the global system mat need different names 
        //    here                                                                      and here 
        amap[LAssetStatusSendReg] = vm->setLinkVal(vmap, aname, "/configs", LAssetStatusSendReg, tVal);  //statusval  dunno yet
        amap[LAssetStatusRespName] = vm->setLinkVal(vmap, aname, "/configs", LAssetStatusRespName, tVal);
        amap[LAssetStatusVal] = vm->setLinkVal(vmap, aname, "/controls", LAssetStatusVal, ival); //ai ok 
        amap[LAssetRespVal] = vm->setLinkVal(vmap, aname, "/status", LAssetRespVal, ival);
        amap[LAssetRespTime] = vm->setLinkVal(vmap, aname, "/status", LAssetRespTime, ival);
        ival = 5;
        amap[LmaxAssetSendNum] = vm->setLinkVal(vmap, aname, "/configs", LmaxAssetSendNum, ival); //local
        amap[LAssetRespOk] = vm->setLinkVal(vmap, aname, "/status", LAssetRespOk, bval); //local
        amap[LAssetRespErr] = vm->setLinkVal(vmap, aname, "/status", LAssetRespErr, bval); //local
        amap[LAssetRespWarn] = vm->setLinkVal(vmap, aname, "/status", LAssetRespWarn, ival); //local


        if (reload == 0) // complete restart 
        {
            ival = -1;
            dval = 0.0;
            //amap["AssetStatusSendName"]->setVal(tVal);
            //amap["AssetStatusRespName"]->setVal(tVal);
            amap[LAssetStatusVal]->setVal(ival);
            amap[LresetAssetStatusVal]->setVal(ival);
            amap[LlastAssetStatusVal]->setVal(ival);
            amap[LAssetRespVal]->setVal(ival);
            amap[LlastAssetRespVal]->setVal(ival);
            amap[LexpectAssetRespVal]->setVal(ival);
            amap[LAssetRespErr]->setVal(bval);
            amap[LAssetSendNum]->setVal(ival);

            amap[LAssetSendTime]->setVal(dval);
            ival = -1;
            amap[LmaxAssetSendNum]->setVal(ival);
            ival = 0;
            amap[LAssetRespWarn]->setVal(ival);
            bval = true; amap[LAssetRespOk]->setVal(bval);

        }
        reload = 2;    TestAssetStatusAv->setVal(reload);
    }

    //int AssetStatusCmd = amap[LAssetStatusCmd]->getiVal();
    int AssetStatusVal = amap[LAssetStatusVal]->getiVal();
    int lastAssetStatusVal = amap[LlastAssetStatusVal]->getiVal();
    int resetAssetStatusVal = amap[LresetAssetStatusVal]->getiVal();

    int expectAssetRespVal = amap[LexpectAssetRespVal]->getiVal();
    int lastAssetRespVal = amap[LlastAssetRespVal]->getiVal();
    int AssetRespVal = amap[LAssetRespVal]->getiVal();
    double AssetRespTime = amap[LAssetRespTime]->getdVal();
    bool AssetRespOk = amap[LAssetRespOk]->getbVal();
    bool AssetRespErr = amap[LAssetRespErr]->getbVal();
    int AssetSendNum = amap[LAssetSendNum]->getiVal();
    int maxAssetSendNum = amap[LmaxAssetSendNum]->getiVal();
    int AssetRespWarn = amap[LAssetRespWarn]->getiVal();

    double tNow = vm->get_time_dbl();
    if (0)FPS_ERROR_PRINT(" %s >> AssetStatusVal %d lastAssetStatusVal %d , time  %2.3f  \n", __func__, AssetStatusVal, lastAssetStatusVal, tNow);
    if (AssetStatusVal != lastAssetStatusVal)
    {
        // sent status val to asset
        amap[LlastAssetStatusVal]->setVal(AssetStatusVal);
        if (AssetStatusVal != resetAssetStatusVal)
        {
            // queue up 5 attempts
            if (AssetSendNum == -1)
            {
                amap[LAssetSendNum]->setVal(maxAssetSendNum);
            }
            // save the cmd for retries
            amap[LAssetStatusCmd]->setVal(AssetStatusVal);
            amap[LlastAssetRespVal]->setVal(AssetRespVal);
            amap[LAssetStatusSendReg]->setVal(AssetStatusVal);
            //void setFimsVal(T val, fims* p_fims, const char* comp = nullptr)

//TODO             amap[LAssetStatusSendReg]->setFimsVal(AssetStatusVal, am->p_fims);

            amap[LAssetRespTime]->setVal(tNow);

            vm->sendAssetVar(amap[LAssetStatusSendReg], am->p_fims); // can use different comp if required
            amap[LAssetSendTime]->setVal(tNow);

            bool bval = false;
            amap[LAssetRespOk]->setVal(bval);
            amap[LAssetRespErr]->setVal(bval);
        }
    }
    // cmd 21  reset 0
    // so send 21 out , wait 1 second and then reset
    // time out on the send go back to reset state
    if (0)FPS_ERROR_PRINT(" %s >> resetAssetStatusVal %d lastAssetStatusVal %d , time  %2.3f  \n", __func__, AssetStatusVal, lastAssetStatusVal, tNow - AssetRespTime);
    if ((lastAssetStatusVal != resetAssetStatusVal) && ((tNow - AssetRespTime) > tSetTime))
    {
        amap[LAssetStatusSendReg]->setVal(resetAssetStatusVal);
        vm->sendAssetVar(amap[LAssetStatusSendReg], am->p_fims); // can use different comp if required

        amap[LlastAssetStatusVal]->setVal(resetAssetStatusVal);
        amap[LAssetStatusVal]->setVal(resetAssetStatusVal);
    }
    // now wait for resp
    double tDiff = tNow - AssetRespTime;
    // skip intermediate responses 
    if (0)FPS_ERROR_PRINT(" %s >> AssetRespVal %d lastAssetRespVal %d , time  %2.3f  \n", __func__, AssetRespVal, lastAssetRespVal, tNow - AssetRespTime);

    // TODO check for errors
    // change in resp val , it may be the value we need.
    // this also becomes a status monitor

    if (AssetRespVal != lastAssetRespVal)
    {
        if (AssetRespVal == expectAssetRespVal)
        {
            if (1)FPS_ERROR_PRINT(" %s >> got correct asset resp %d in %2.3f secs \n", __func__, AssetRespVal, tDiff);
            bool bval = true;
            amap[LAssetRespOk]->setVal(bval);
            bval = false;
            amap[LAssetRespErr]->setVal(bval);
            //AssetRespWarn++;
            amap[LAssetRespWarn]->setVal(AssetRespWarn);
            AssetRespOk = true;
            amap[LAssetRespTime]->setVal(tDiff);
            // reset the sends
            AssetSendNum = -1;
            amap[LAssetSendNum]->setVal(AssetSendNum);
            dval = 0;
            amap[LAssetSendTime]->setVal(dval);
        }
        else
        {
            if (1)FPS_ERROR_PRINT(" %s >> got incorrect asset resp %d in %2.3f secs \n", __func__, AssetRespVal, tDiff);
        }


        amap[LlastAssetRespVal]->setVal(AssetRespVal);

    }
    if (0)FPS_ERROR_PRINT(" %s >> AssetRespErr %s AssetRespOk %s , time  %2.3f  \n"
        , __func__
        , AssetRespErr ? "true" : "false"
        , AssetRespOk ? "true" : "false"
        , tNow - AssetRespTime);
    // not OK and Not Err'd yet check for timeout 
    if (!AssetRespErr && !AssetRespOk && (tDiff > tRespTime))
    {
        if (1)FPS_ERROR_PRINT(" %s >> warning, timeout ..   asset resp %d after %2.3f secs, attempt %d\n"
            , __func__
            , AssetRespVal
            , tDiff
            , maxAssetSendNum - AssetSendNum
        );

        //bool bval = true;
        //amap["AssetRespWarn"]->setVal(bval);
        AssetRespWarn++;
        amap[LAssetRespWarn]->setVal(AssetRespWarn);

        if (AssetSendNum == 0)
        {
            if (1)FPS_ERROR_PRINT(" %s >> Error, timeout ..   asset resp %d after %2.3f secs, attempt %d\n"
                , __func__
                , AssetRespVal
                , tDiff
                , maxAssetSendNum - AssetSendNum
            );
            amap[LAssetRespErr]->setVal(bval);
            AssetSendNum = -1;
            amap[LAssetSendNum]->setVal(AssetSendNum);
            // no more attempts after an error
            dval = 0;
            amap[LAssetSendTime]->setVal(dval);

        }
        else
        {
            amap[LAssetSendNum]->setVal(AssetSendNum - 1);
        }
        amap[LAssetRespTime]->setVal(tDiff);

    }

    return 0;

}
int InitAMHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // this will filter up the comms stats to the manager

    if (!amap["HeartBeatErrors"])
    {
        char* cval = (char*)"HeartBeat Init";
        setAmapAi(am, amap, HeartBeatState, am->name.c_str(), / status, cval);
        int ival = 0;
        setAmapAi(am, amap, HeartBeatErrors, am->name.c_str(), / reload, ival);

    }
    int ival = 0; amap["HeartBeatErrors"]->setVal(ival);

    return 0;
}
// these guys check to see if the individual bypass bools have been set.
// if so then ignore the comms errors or lack of first comms data preventing the system getting into ready
// if CommsBypass is set  then if asset in Coms Init then put it into Comms Ready bypassing the first data  
// then capture CommsErr and ifnore the error
int CheckAssetCommsOk(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
{
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);

    if (!amap["BypassComms"])
    {
        VarMapUtils* vm = ai->vm;
        bool bval = false;
        amap["BypassComms"] = vm->setLinkVal(vmap, aname, "/controls", "BypassComms", bval);
    }

    if (!amap["CommsState"])
    {
        VarMapUtils* vm = ai->vm;
        bool bval = false;
        amap["CommsState"] = vm->setLinkVal(vmap, aname, "/status", "CommsState", bval);
    }

    //foreach asset manager check assets
    // then check our own assets
    //bool bval = false;
    bool BypassComms = amap["BypassComms"]->getbVal();

    char* cval = nullptr;
    if (BypassComms)
    {
        cval = (char*)"Comms OK";
        amap["CommsState"]->setVal(cval);
        return true;
    }

    return false;

    // if the bypass flag is set we ignore comms errors
    // if the bypass flag is set we pretend to get the firstComms message and ignore Comms errors
    //return BypassComms;

    // if (BypassComms)
    // {
    //     amap["CommsOk"]->setVal(true);
    // }
    //return 0;
}

int CheckCommsOk(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);

    if (!amap["CommsOk"])
    {
        VarMapUtils* vm = am->vm;
        bool bval = false;
        amap["CommsOk"] = vm->setLinkVal(vmap, aname, "/status", "CommsOk", bval);
    }
    // for each asset manager in assetManMap call CheckCommsOK
    for (auto ass_man : am->assetManMap)
    {
        asset_manager* amc = ass_man.second;
        if (0)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  Asset Man [%s] \n ", __func__, aname, amc->name.c_str());
        CheckCommsOk(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
    }

    bool commsReady = true;
    // for each asset in assetMap call ChecAssetComsOK
    for (auto x : am->assetMap)
    {
        asset* ai = x.second;
        if (0)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  Asset [%s]\n ", __func__, aname, ai->name.c_str());

        // Check if bypass comms have been set. If not, then comms is not yet ready
        if (!CheckAssetCommsOk(vmap, ai->amap, ai->name.c_str(), p_fims, ai))
        {
            commsReady = false;
        }
    }

    // Set comms to ok (to get system into ready state) if all assets' bypass comms have been set
    if (commsReady)
    {
        amap["CommsOk"]->setVal(true);
    }

    return 0;
}

// these guys check to see if the individual bypass bools have been set.
// if so then ignore theheartbeat errors or lack of first heartbeat data preventing the system getting into ready
int CheckAssetHeartBeatOk(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
{
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);
    bool bval = false;

    if (!amap["BypassHB"])
    {
        VarMapUtils* vm = ai->vm;
        bool bval = false;
        amap["BypassHB"] = vm->setLinkVal(vmap, aname, "/controls", "BypassHB", bval);
    }

    if (!amap["HeartBeatState"])
    {
        VarMapUtils* vm = ai->vm;
        amap["HeartBeatState"] = vm->setLinkVal(vmap, aname, "/status", "HeartBeatState", bval);
    }

    //foreach asset manager check assets
    // then check our own asswts
    bool BypassHB = amap["BypassHB"]->getbVal();

    char* cval2 = nullptr;
    if (BypassHB)
    {
        cval2 = (char*)"HB OK";
        amap["HeartBeatState"]->setVal(cval2);
        return true;
    }
    return false;

    // if the bypass flag is set we pretend to get the first HB message and gnore HB errors
    //return BypassHB;

    // if (BypassHB)
    // {
    //     amap["HBOk"]->setVal(true);
    // }
    // return 0;
}

int CheckHeartBeatOk(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    if (0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);

    if (!amap["HBOk"])
    {
        VarMapUtils* vm = am->vm;
        bool bval = false;
        amap["HBOk"] = vm->setLinkVal(vmap, aname, "/status", "HBOk", bval);
    }
    // for each asset manager in assetManMap call CheckHeartBeatOK
    for (auto ass_man : am->assetManMap)
    {
        asset_manager* amc = ass_man.second;
        if (0)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  Asset Man [%s] \n ", __func__, aname, amc->name.c_str());
        CheckHeartBeatOk(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
    }

    bool hbReady = true;
    // for each asset in assetMap call ChecAssetHeartBeatOK
    for (auto ass_instance : am->assetMap)
    {
        asset* ai = ass_instance.second;
        if (0)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  Asset [%s] \n ", __func__, aname, ai->name.c_str());

        // Check if bypass comms have been set. If not, then comms is not yet ready
        if (!CheckAssetHeartBeatOk(vmap, ai->amap, ai->name.c_str(), p_fims, ai))
        {
            hbReady = false;
        }
    }

    // Set heartbeat state to ok (to get system into ready state) if all assets' bypass comms have been set
    if (hbReady)
    {
        amap["HBOk"]->setVal(true);
    }

    return 0;
}


// we get the commands in the form of a controlword from  hybridOS
// or start_stop....
// we'll handle both
// 205 = 0xcd 11001101 
// 206 = 0xce 11001110
// 207 = 0xcf 11001111
// 208 = 0xd0 11011000
// /components/ess/ctrlword1cfg
//mask 3  bit 0   0000000000000001       oncmd
//mask 3  bit 1   0000000000000010       kacclosecmd
//mask 48 bit 4   0000000000010000       offcmd
//mask 48 bit 5   0000000000100000       kacopencmd
// "ctrlword1cfg":[{"value": 207,"controls": [{ "field": "oncmd", "value": true },
//                                            { "field": "kacclosecmd", "value": true }]},
//                 {"value": 206,"controls": [{ "field": "offcmd", "value": true },
//                                            { "field": "kacopencmd", "value": true }]}],
// "ctrlword2cfg":[{"value": 2,"controls": [{ "field": "kdcclosecmd", "value": true }]},
//                 {"value": 1,"controls": [{ "field": "kdcopencmd", "value": true }]}],
// "ctrlword4cfg": [{"value": 205,"controls": [{ "field": "standbycmd", "value": true }]},
//                 {"value": 207,"controls": [{ "field": "oncmd", "value": true }]}],
// "statuscfg": [{"value":1,"string":"Running","field":"on"},
//               {"value":0,"string":"Stopped","field":"on","invert":true}]},
//   set /components/ess:control_word1 207 > /controls/ess/OnCmd /controls/ess AcContactorCloseCmd
//   set /components/ess:control_word1 208 > /controls/ess/OffCmd /controls/ess AcContactorOpenCmd

// here we handle the incoming commands
// from off to grid following
// 
// if we get an estop at any time then handle that.
//    phil to provide an example of an immediate estop wakeup.
//    we'll have to finish the current wakeup but apart from that it should be good. 

// 0/ check system state .. must be on standby
//           if not report
// 1/ check faults /// 
//      pcs
//      bms
//      dcr       --> if bad report and enter fault mode 
// 2/ Check comms
//      pcs
//      bms      --> skip off line bms units.
//      dcr       --> if bad  report and enter fault mode
//3/ Check Status 
//      pcs
//      bms
//      dcr
//4/    check power available
//      if below minimums  report and enter fault mode
// 5/   send active power to pcs.
// 6/ wait for pcs to respond .....  Phil to provide infrastructure to wait for command  completion.
// 7 /  monitor PCS output and 
//                    --> report and fault if failure detected.
// 8// monitor SOC etc based on actual power output ... ( New consideration)
//       report and fail if exception noted.
// CMD states from hybidos....we'll use them.
// enum states
// {
//     Init = 0,
//     Ready,
//     Startup,
//     RunMode1,
//     RunMode2,
//     Standby,
//     Shutdown,
//     Error
// }; (edited) 
// 9:05
// RunMode1 is able to accept power commands, RunMode2 we don't have yet, Standby is running but no power features enabled
// 9:07
// Normal behavior will be it's in Ready mode, gets a start signal, goes through Startup to RunMode1
//
// we are extrending the "onSet" / "onPub" operation of the assetVar.
// we can create a subclass called an assetRunFunc 
// this will contain a function ( look at CommsState for an example)
// this will be run when the asset value is set.

// (The Estop will be the same)
// unlike the CommsState these will be general pupose functions with NO local variables 
//  So the path is to send a command to the pcr AFTER we have set up the command response to the feedback var from the PCR to run the handle response function.
// Then send the command to thepcr.
// The normall poll function will look for an indication that the response has changed and if so that function will be disabled.
// the response (" I am turned ON ") will cause the system to continue to the next phase.
// IF the poll function finds NO response in a timeout period then the system will revert to an error state.
// Lets try and ascii draw this...
//
//   HandleCmd -> sets up function to HandlePCRON ,sends "turn on to PCR", continues polling until we get PCR CMD timeout.
//  if we have issued a "turn ON PCR" command HandlePCR ON will eiher set the state to Ready or Error depending on the response from the PCR status.
//  IF we want, the HandlePCRReady command can be run directly by the asset RunVar command.
// 
//
// enum States
// {
//     System_Init =0,
//     System_Ready,
//     System_Startup,
//     System_RunMode1,
//     System_RunMode2,
//     System_Standby,
//     System_Shutdown,
//     System_Fault,
//     System_Fault

// };
// enum Cmds
// {
//     SystemInit =0,
//     SystemOn,
//     SystemOff,
//     SystemStandby,
//     SystemReset,
//     SystemFault,
//     SystemRestart
// };

// ok to open AcContactor ? need a load of checks in here
bool AcContactorOkToOpen(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    bool ret = false;
    bool AcContactor = amap["AcContactor"]->getbVal();
    if (AcContactor)
    {
        ret = true;
    }
    ret = true;
    return ret;
}
// ok to close AcContactor ? need a load of checks in here
// Check OK
// send to Hw
// check response time
int AcContactorOkToClose(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    int  ret = 0;
    //bool AcContactor = amap["AcContactor"]->getbVal();
    // if(!AcContactor)
    // {
    //     ret = true;
    // }
    // ret =  true;
    return ret;
}
// ok to open DcContactor ? need a load of checks in here
bool DcContactorOkToOpen(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    bool ret = false;
    bool DcContactor = amap["DcContactor"]->getbVal();
    if (DcContactor)
    {
        ret = true;
    }
    ret = true;
    return ret;
}
// ok to close DcContactor ? need a load of checks in here
bool DcContactorOkToClose(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    bool ret = false;
    bool DcContactor = amap["DcContactor"]->getbVal();
    if (!DcContactor)
    {
        ret = true;
    }
    ret = true;
    return ret;
}
// enum SysStates
// {
//     System_Init =0,
//     System_Ready,
//     System_Startup,
//     System_RunMode1,
//     System_RunMode2,
//     System_Standby,
//     System_Shutdown,
//     System_Fault

// };
// //To come out of init
// // PCS comms must be OK
// // Bms comms must have at least bms min connetions.
// // Pcs Heatbeat must be OK
// // Bms Hearbeat must be OK.
// // We have SimPcsComms Bms_1Comms etc flags
// // same for hearbeat
// // then we need to see good status from the PCS system and BMS sysems

// int CheckSystemState(varsmap &vmap, varmap &amap, const char *aname, fims* p_fims, asset_manager *am)
// {
//     VarMapUtils*vm = am->vm;
//     vm->setTime();

//     if(0)FPS_ERROR_PRINT("%s >> %s --- Running\n", __func__, aname);
//     // char* fval;
//     // asprintf(&fval,"Ess Init");
//     // if(fval)
//     // {
//     //     amap["SystemState"]->setVal(fval);
//     //     free((void*)fval);
//     // }

//     double respTime = 3.5;  // TODO put in a variable
//     bool DcContactor    = amap["DcContactor"]->getbVal();
//     bool DcContactorFbk = amap["DcContactorFbk"]->getbVal();
//     bool AcContactor    = amap["AcContactor"]->getbVal();
//     bool AcContactorFbk = amap["AcContactorFbk"]->getbVal();

//     bool CommsOk   = amap["CommsOk"]->getbVal();
//     // TODO maybe just use SysOK in each subsystem...
//     bool HBOk      = amap["HBOk"]->getbVal();
//     bool BMSOk     = amap["BMSOk"]->getbVal();
//     bool PCROk     = amap["PCROk"]->getbVal();
//     bool DRCOk     = amap["DRCOk"]->getbVal();
//     bool EMMOk     = amap["EMMOk"]->getbVal();

//     int  sState    = amap["SystemStateNum"]->getiVal();  
//     int rc = 0;
//     // TODO only do this after Setup
//     // TODO process by AssetManager 
//     if(
//         (sState != System_Init)
//         && (sState != System_Alarm)
//         && (sState != System_Fault)
//     )
//     {

//         double dval;// = vm->get_time_dbl();
//         dval = vm->get_time_dbl();

//         if ((DcContactor != DcContactorFbk) && ((dval -amap["DcContactor"]->getSetTime()) >respTime))
//         {
//             dval = vm->get_time_dbl();
//             FPS_ERROR_PRINT("%s >> %s --- Running  statenum %d Dc [%s] DcFbk [%s] settime %f lastset %f\n"
//                     , __func__, aname, sState
//                     , DcContactor?"true":"false"
//                     , DcContactorFbk?"true":"false"
//                     , dval - amap["DcContactor"]->getSetTime()
//                     , amap["DcContactor"]->getLastSetDiff(dval)
//                     );

//             sState = System_Fault;
//             amap["SystemStateNum"]->setVal(sState);

//             char* fval; 
//             asprintf(&fval,"%s  DcContactor Did not respond in %f seconds", aname, dval);
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }

//         if ((AcContactor != AcContactorFbk) && ((dval - amap["AcContactor"]->getSetTime()) >respTime) )
//         {
//             dval = vm->get_time_dbl();
//             FPS_ERROR_PRINT("%s >> %s --- Running  statenum %d Ac [%s] AcFbk [%s] settime %f lastset %f\n"
//                     , __func__, aname, sState
//                     , AcContactor?"true":"false"
//                     , AcContactorFbk?"true":"false"
//                     , dval - amap["AcContactor"]->getSetTime()
//                     , amap["DcContactor"]->getLastSetDiff(dval)

//                     );

//             sState = System_Fault;
//             amap["SystemStateNum"]->setVal(sState);

//             char* fval; 
//             asprintf(&fval,"%s  AcContactor Did not respond in %f seconds", aname, dval);
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }
//     }

//     if(sState == System_Init)
//     {
//         if(!CommsOk) 
//         {
//             char* fval; 
//             asprintf(&fval," Ess Comms Failure");
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }
//         if(!HBOk) 
//         {
//             char* fval; 
//             asprintf(&fval," Ess HeartBeat Failure");
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }
//         if(!BMSOk) 
//         {
//             char* fval; 
//             asprintf(&fval," Ess BMS Failure");
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }

//         if(!PCROk) 
//         {
//             char* fval; 
//             asprintf(&fval," Ess BMS Failure");
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }

//         if(!DRCOk) 
//         {
//             char* fval; 
//             asprintf(&fval," Ess BMS Failure");
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }

//         if(!EMMOk) 
//         {
//             char* fval; 
//             asprintf(&fval," Ess EMM Failure");
//             if(fval)
//             {
//                 amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             rc++;
//         }
//     }
//     if (rc == 0)
//     {
//         int ival;
//         ival = amap["SystemStateNum"]->getiVal();  
//         // if we are in init or Fault transition to ready
//         if ((ival == System_Init) || (ival == System_Fault))
//         {
//             char* fval; 
//             asprintf(&fval,"Ess Init OK , Ready");
//             if(fval)
//             {
//                 //amap["SystemState"]->setVal(fval);
//                 free((void*)fval);
//             }
//             sState = System_Ready;
//             amap["SystemStateNum"]->setVal(sState); 
//         } 
//     }
//     if(rc!=0)FPS_ERROR_PRINT("%s >> %s --- Done rc %d \n", __func__, aname, rc);

//     return rc;
// }

// // ems_cmd
// // 0x0300 2 BMS heartbeat BMS heartbeats 0-255, updated
// // every 1s
// // 0x0301 2 BMS_poweron
// //           BMS high voltage status
// //           0: Power off ready
// //           1: Power on ready
// //           2: Power on fault
// //           3: Power off fault
// // 0x0302 2 BMS_status
// //         BMS system status
// //         0: Initial status
// //         1: Normal status
// //         2: Full charge status
// //         3: Full discharge status
// //         4: Warning status
// //         5: Fault status
// // Command of EMS to control BMS relay
// // bms_cmd in this case
// //        0: Initial
// //        1: Stay status
// //        2: Power on cmd
// //        3: Power off cmd
// // 0x0010 2 System status 
// //      000 Initialize
// //      001 Normal
// //      010 Full charge
// //      011 Full discharge
// //      100 Warning status
// //      101 Fault status
// int HandleAssetCmd(varsmap &vmap, varmap &amap, const char *aname, fims* p_fims, asset* am, int sstate)
// {
//     VarMapUtils * vm = am->vm;
//     printf("%s >> %s --- Running  state %d\n", __func__, aname, sstate); 
//     assetVar* HandleAssetCmd = amap["HandleAssetCmd"];
//     char *tval = (char *)" Asset Init";
//     int reload = 0;
//     bool bval =  false;
//     int ival = -1;
//     reload = CheckReload(vmap, amap, aname, __func__);

//     if(reload < 2)
//     {
//         // note links must set these values
//         amap["HandleAssetCmd"]    = vm->setLinkVal(vmap, aname, "/controls", "HandleAssetCmd", reload);
//         amap["AssetCmd"]          = vm->setLinkVal(vmap, aname, "/controls", "AssetCmd", ival);
//         amap["AssetOn"]           = vm->setLinkVal(vmap, aname, "/params",   "AssetOn", ival);
//         amap["AssetOff"]          = vm->setLinkVal(vmap, aname, "/params",   "AssetOff", ival);
//         amap["AssetStandby"]      = vm->setLinkVal(vmap, aname, "/params",   "AssetStandby", ival);
//         amap["AssetInit"]         = vm->setLinkVal(vmap, aname, "/params",   "AssetInit", ival);
//         amap["OnCmd"]             = vm->setLinkVal(vmap, aname, "/controls", "OnCmd", bval);
//         amap["OffCmd"]            = vm->setLinkVal(vmap, aname, "/controls", "OffCmd", bval);
//         amap["StandbyCmd"]        = vm->setLinkVal(vmap, aname, "/controls", "StandbyCmd", bval);
//         amap["AssetState"]        = vm->setLinkVal(vmap, aname, "/status",   "AssetState", tval);
//         amap["BypassHB"]          = vm->setLinkVal(vmap, aname, "/controls", "BypassHB", ival);
//         amap["BypassComms"]       = vm->setLinkVal(vmap, aname, "/controls", "BypassComms", ival);
//         if(reload < 1)
//         {
//             amap["OnCmd"]->setVal(false);
//             amap["OffCmd"]->setVal(false);
//             amap["StandbyCmd"]->setVal(false);
//             amap["AssetState"]->setVal(tval);
//             amap["BypassHB"]->setVal(false);
//             amap["BypassComms"]->setVal(false);
//         }
//         reload = 2;
//         amap["HandleAssetCmd"]->setVal(reload) ;

//     }
//     assetVar* asv;    // send var
//     assetVar* acv;    // command var
//     char* fval;
//     asprintf(&fval," %s Asset State %d", aname, sstate);
//     if(fval)
//     {
//         amap["AssetState"]->setVal(fval);
//         free((void*)fval);
//     }
//     switch (sstate)
//     {
//         case SystemInit:
//            asv = amap["AssetCmd"];
//            acv = amap["AssetInit"];
//            break;
//         case SystemOn:
//            asv = amap["AssetCmd"];
//            acv = amap["AssetOn"];
//            break;
//         case SystemOff:
//            asv = amap["AssetCmd"];
//            acv = amap["AssetOff"];
//            break;
//         case SystemStandby:
//            asv = amap["AssetCmd"];
//            acv = amap["AssetStandby"];
//            break;
//         default:
//             asv = nullptr;
//     }

//     if(asv) 
//     {
//         asv->setVal(acv->getiVal());
//         vm->sendAssetVar(asv, p_fims);
//     } 
//     return 0;
// }

// int HandleManagerCmd(varsmap &vmap, varmap &amap, const char *aname, fims* p_fims, asset_manager *am, int sstate)
// {
//     VarMapUtils * vm = am->vm;
//     char* tVal = (char *)" Init"; 
//     printf("%s >> %s --- Running state %d\n", __func__, aname, sstate);
//     assetVar*HandleManCmd      = amap["HandleManCmd"];  
//     printf("%s >> %s --- Running\n", __func__, aname);
//     bool bval = false;
//     int ival = 0;
//     int reload = 0;
//     
//     reload = CheckReload(vmap, amap, aname, __func__);

//     if(reload < 2)
//     {
//         amap["On"]                = vm->setLinkVal(vmap, aname, "/status",   "On", bval);
//         amap["Off"]               = vm->setLinkVal(vmap, aname, "/status",   "Off", bval);
//         amap["Standby"]           = vm->setLinkVal(vmap, aname, "/status",   "Standby", bval);
//         amap["Fault"]             = vm->setLinkVal(vmap, aname, "/status",   "Fault", bval);
//         amap["ResetFaultCmd"]     = vm->setLinkVal(vmap, aname, "/controls", "ResetFaultCmd", bval);
//         amap["OnCmd"]             = vm->setLinkVal(vmap, aname, "/controls", "OnCmd", bval);
//         amap["OffCmd"]            = vm->setLinkVal(vmap, aname, "/controls", "OffCmd", bval);
//         amap["StandbyCmd"]        = vm->setLinkVal(vmap, aname, "/controls", "StandbyCmd", bval);
//         amap["ResetFaultCmd"]     = vm->setLinkVal(vmap, aname, "/controls", "ResetFaultCmd", bval);
//         amap["ResetCmd"]          = vm->setLinkVal(vmap, aname, "/controls", "ResetCmd", bval);
//         amap["GridForming"]       = vm->setLinkVal(vmap, aname, "/status",   "GridForming", bval);
//         amap["GridFollowing"]     = vm->setLinkVal(vmap, aname, "/status",   "GridFollowing", bval);
//         amap["SystemState"]       = vm->setLinkVal(vmap, aname, "/status",   "SystemState", tVal);
//         amap["SystemStateNum"]    = vm->setLinkVal(vmap, aname, "/status",   "SystemStateNum", ival);
//         amap["AcContactor"]       = vm->setLinkVal(vmap, aname, "/status",   "AcContactor", bval);
//         amap["DcContactor"]       = vm->setLinkVal(vmap, aname, "/status",   "DcContactor", bval);
//         amap["AcContactorFbk"]    = vm->setLinkVal(vmap, aname, "/controls", "AcContactorFbk", bval);
//         amap["DcContactorFbk"]    = vm->setLinkVal(vmap, aname, "/controls", "DcContactorFbk", bval);
//         amap["AcContactorCmd"]    = vm->setLinkVal(vmap, aname, "/controls", "AcContactorCmd", bval);
//         amap["DcContactorCmd"]    = vm->setLinkVal(vmap, aname, "/controls", "DcContactorCmd", bval);

//         amap["CommsOk"]           = vm->setLinkVal(vmap, aname, "/status",    "CommsOk", bval);
//         amap["HBOk"]              = vm->setLinkVal(vmap, aname, "/status",    "HBOk", bval);
//         // amap["BMSOk"]             = vm->setLinkVal(vmap, "bms", "/status",    "BMSOk", bval);
//         // amap["PCROk"]             = vm->setLinkVal(vmap, "pcr", "/status",    "PCROk", bval);
//         // amap["DRCOk"]             = vm->setLinkVal(vmap, "drc", "/status",    "DRCOk", bval);
//         // amap["EMMOk"]             = vm->setLinkVal(vmap, "emm", "/status",    "EMMOk", bval);

//         if(reload == 0) // complete restart 
//         {
//             amap["SystemState"]->setVal(tVal);
//             amap["On"]->setVal(false);
//             amap["OnCmd"]->setVal(false);
//             amap["Off"]->setVal(false);
//             amap["OffCmd"]->setVal(false);
//             amap["Standby"]->setVal(false);
//             amap["StandbyCmd"]->setVal(false);
//             amap["Fault"]->setVal(false);
//             amap["ResetFaultCmd"]->setVal(false);
//             amap["GridForming"]->setVal(false);
//             amap["GridFollowing"]->setVal(false);
//             amap["SystemState"]->setVal(tVal);
//             amap["SystemStateNum"]->setVal(System_Init); // starts init timeout


//             amap["CommsOk"]->setVal(true);
//             amap["HBOk"]->setVal(true);
//             // amap["BMSOk"]->setVal(true);
//             // amap["PCROk"]->setVal(true);
//             // amap["DRCOk"]->setVal(true);
//             // amap["EMMOk"]->setVal(true);
//              // DO any setup here    
//         }
//         reload = 2;
//         HandleManCmd      = amap[__func__];
//         HandleManCmd->setVal(reload);
//     }

//     char* fval;
//     asprintf(&fval," %s Manager State %d", aname, sstate);
//     if(fval)
//     {
//         amap["SystemState"]->setVal(fval);
//         free((void*)fval);
//     }
//     // for each Asset Manager turn it on
//     for (auto ix: am->assetManMap)
//     {
//         asset_manager* amm = ix.second;
//         HandleManagerCmd(vmap, amm->amap, amm->name.c_str(), p_fims, amm, sstate);
//         if(0)FPS_ERROR_PRINT(" %s >> manager child [%s]  with %d kids \n", __func__, ix.first.c_str(), amm->assetMap.size());
//         // send each manager a wake up
//     }
//     for (auto ix: am->assetMap)
//     {
//         asset* ami = ix.second;
//         HandleAssetCmd(vmap, ami->amap, ami->name.c_str(), p_fims, ami, sstate);

//         if(0)FPS_ERROR_PRINT(" %s >> asset child [%s]  \n", __func__, ix.first.c_str());
//     }
//     // after assets have been done wake up for set manager
//     for (auto ix: am->assetManMap)
//     {
//         asset_manager* amm = ix.second;

//         amm->run_wakeup(amm, WAKE_LEVEL_MANAGE);
//         //HandleManagerCmd(vmap, amm->amap, amm->name.c_str(), p_fims, amm, sstate);
//         if(0)FPS_ERROR_PRINT(" %s >> manager child [%s]  with %d kids \n", __func__, ix.first.c_str(), amm->assetMap.size());
//         // send each manager a wake up
//     }
//     return 0;
// }

int HandleSystemReset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    printf("%s >> %s --- Running\n", __func__, aname);
    char* fval;
    asprintf(&fval, " %s System Reset", aname);
    if (fval)
    {
        amap["SystemState"]->setVal(fval);
        free((void*)fval);
    }

    return 0;
}
// int HandleFaultShutdown(varsmap &vmap, varmap &amap, const char *aname, fims* p_fims, asset_manager *am)
// {
//     printf("%s >> %s --- Running\n", __func__, aname);
//     char* fval;
//     asprintf(&fval," Ess Fault Shutdown");
//     if(fval)
//     {
//         amap["SystemState"]->setVal(fval);
//         free((void*)fval);
//     }

//     return 0;
// }

//     assetFunc* RunPCRCmd;
//     VarMapUtils* vm;
//     vm = am->vm;
//     int reload = 0; 
//     reload = CheckReload(vmap, amap, aname, "SetupRunPCRCmd");

//     if (reload) < 2 )
//     {
//         amap["RunPCRCmd"]          = vm->setLinkVal(vmap, aname, "/status",        "RunPCRCmd",     reload);
//         amap["PCRCmd"]             = vm->setLinkVal(vmap, aname, "/components",    "PCRCmd",     reload);

//         if ( reload < 1)
//         {
//             RunPCRCmd =(assetFunc *) new assetFunc(aname);
//             amap["RunPCRCmdFunc"] = (assetVar *)RunPCRCmd;
//             RunPCRCmd->setupRamFunc(HandlePCRCmd,vmap, amap, aname, p_fims, am);
//             // the following causes HandlePCRCmd to be run on eery Set or Pub on PCRCmd
//             // may need to use VarMapUtils to runit
//             amap["PCRCmd"] ->SetFunc = (assetVar* )RunPCRCmd;
//             amap["PCRCmd"] ->PubFunc = (assetVar* )RunPCRCmd;
//         }
// // inline input andler , one of our nice little features
// // makes sure that later commands overwrite earlier ones
// int HandleESSInput(varsmap &vmap, varmap &amap,  const char *aname, fims* p_fims, asset_manager *am)
// {
//     // Turn on if conditions allow it
//     //bool AcContactor     = amap["AcContactor"]->getbVal();
//     //bool DcContactor     = amap["DcContactor"]->getbVal();
//     bool AcContactorOpenCmd  = amap["AcContactorOpenCmd"]->getbVal();
//     bool DcContactorOpenCmd  = amap["DcContactorOpenCmd"]->getbVal();
//     bool AcContactorCloseCmd  = amap["AcContactorCloseCmd"]->getbVal();
//     bool DcContactorCloseCmd  = amap["DcContactorCloseCmd"]->getbVal();
//     bool OnCmd           = amap["OnCmd"]->getbVal();
//     bool OffCmd          = amap["OffCmd"]->getbVal();
//     bool StandbyCmd      = amap["StandbyCmd"]->getbVal();
//     bool ResetCmd        = amap["ResetCmd"]->getbVal();
//     //bool ResetFaultCmd   = amap["ResetFaultCmd"]->getbVal();
//     bool readyOkSetCmd        = amap["readyOkSetCmd"]->getbVal();
//     bool readyOkClearCmd      = amap["readyOkClearCmd"]->getbVal();
//     // only allow one
//     bool fval = false;
//     if(0)
//     {
//         FPS_ERROR_PRINT("%s >> %s --- at Start AcContactorOpenCmd [%s] AcContactorCloseCmd [%s] DcContactorOpenCmd [%s] DcContactorCloseCmd [%s] \n"
//                 , __func__, aname
//                 , AcContactorOpenCmd?"true":"false"
//                 , AcContactorCloseCmd?"true":"false"
//                 , DcContactorOpenCmd?"true":"false"
//                 , DcContactorCloseCmd?"true":"false"
//                 );
//         FPS_ERROR_PRINT("%s >> %s ---          OffCmd [%s] OnCmd [%s] StandbyCmd [%s] \n"
//                 , __func__, aname
//                 , OffCmd?"true":"false"
//                 , OnCmd?"true":"false"
//                 , StandbyCmd?"true":"false"
//                 );
//         FPS_ERROR_PRINT("%s >> %s ---         readyOkSetCmd [%s] readyOkClearCmd [%s]\n"
//                 , __func__, aname
//                 , readyOkSetCmd?"true":"false"
//                 , readyOkClearCmd?"true":"false"
//                 );
//     }

//     if (AcContactorCloseCmd)
//     {
//         amap["AcContactorOpenCmd"]->setVal(fval);
//         AcContactorOpenCmd = false;
//     }
//     if (AcContactorOpenCmd)
//     {
//         amap["AcContactorCloseCmd"]->setVal(fval);
//     }
//     if (DcContactorCloseCmd)
//     {
//         amap["DcContactorOpenCmd"]->setVal(fval);
//         DcContactorOpenCmd = false;
//     }
//     if (DcContactorOpenCmd)
//     {
//         amap["DcContactorCloseCmd"]->setVal(fval);
//     }
//     if (OnCmd)
//     {
//         //amap["OnCmd"]->setVal(fval);
//         amap["OffCmd"]->setVal(fval);
//         amap["StandbyCmd"]->setVal(fval);
//         OffCmd =  false;
//         StandbyCmd =  false;
//     }
//     if (OffCmd)
//     {
//         amap["OnCmd"]->setVal(fval);
//         //amap["OffCmd"]->setVal(fval);
//         amap["StandbyCmd"]->setVal(fval);
//         StandbyCmd =  false;

//     }
//     if (StandbyCmd)
//     {
//         amap["OnCmd"]->setVal(fval);
//         amap["OffCmd"]->setVal(fval);
//         //amap["StandbyCmd"]->setVal(fval)
//     }
//     if (readyOkSetCmd)
//     {
//         amap["readyOkClearCmd"]->setVal(fval);
//         //amap["StandbyCmd"]->setVal(fval)
//         readyOkClearCmd = false;
//     }
//     if (readyOkClearCmd)
//     {
//         amap["readyOkSetCmd"]->setVal(fval);
//         //amap["StandbyCmd"]->setVal(fval)
//     }
//     if(0)
//     {
//         AcContactorOpenCmd  = amap["AcContactorOpenCmd"]->getbVal();
//         DcContactorOpenCmd  = amap["DcContactorOpenCmd"]->getbVal();
//         AcContactorCloseCmd  = amap["AcContactorCloseCmd"]->getbVal();
//         DcContactorCloseCmd  = amap["DcContactorCloseCmd"]->getbVal();
//         OnCmd           = amap["OnCmd"]->getbVal();
//         OffCmd          = amap["OffCmd"]->getbVal();
//         StandbyCmd      = amap["StandbyCmd"]->getbVal();
//         ResetCmd        = amap["ResetCmd"]->getbVal();
//     //bool ResetFaultCmd   = amap["ResetFaultCmd"]->getbVal();
//         readyOkSetCmd        = amap["readyOkSetCmd"]->getbVal();
//         readyOkClearCmd      = amap["readyOkClearCmd"]->getbVal();

//         FPS_ERROR_PRINT("%s >> %s --- at End AcContactorOpenCmd [%s] AcContactorCloseCmd [%s] DcContactorOpenCmd [%s] DcContactorCloseCmd [%s] \n"
//                 , __func__, aname
//                 , AcContactorOpenCmd?"true":"false"
//                 , AcContactorCloseCmd?"true":"false"
//                 , DcContactorOpenCmd?"true":"false"
//                 , DcContactorCloseCmd?"true":"false"
//                 );
//         FPS_ERROR_PRINT("%s >> %s ---          OffCmd [%s] OnCmd [%s] StandbyCmd [%s] \n"
//                 , __func__, aname
//                 , OffCmd?"true":"false"
//                 , OnCmd?"true":"false"
//                 , StandbyCmd?"true":"false"
//                 );
//         FPS_ERROR_PRINT("%s >> %s ---         readyOkSetCmd [%s] readyOkClearCmd [%s]\n"
//                 , __func__, aname
//                 , readyOkSetCmd?"true":"false"
//                 , readyOkClearCmd?"true":"false"
//                 );
//     }

//     return 0;
// }
// /**
//  * Preston's code ready for review
//  * increments  a heart beat ... may need to add a period to this
//  * Not used... to be removed
//  * 
//  * Used in:
//  * Test Script:
//  */
// int SimHandleHeartBeat(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager* am)
// {
//     int reload;
//     double dval = 0.0;
//     bool bval = false;
//     VarMapUtils* vm = am->vm;
//     char * tVal = (char *)"Test TimeStamp";
//     reload = CheckReload(vmap, amap, aname, "SimHandleHeartBeat");

//     assetVar* SimHandleHeartBeat      = amap["SimHandleHeartBeat"];

//     if(reload < 2)
//     {
//         //reload = 0;
//         amap["SimHandleHeartBeat"]    = vm->setLinkVal(vmap, aname, "/reload",    "SimHandleHeartBeat",      reload);
//         amap["HeartBeat"]             = vm->setLinkVal(vmap, aname, "/status",    "HeartBeat",               dval);
//         amap["Timestamp"]             = vm->setLinkVal(vmap, aname, "/status",    "Timestamp",               tVal);
//         amap["CommsDummy"]            = vm->setLinkVal(vmap, aname, "/status",    "CommsDummy",              dval);
//         amap["SimPcsComms"]           = vm->setLinkVal(vmap, aname, "/configsim",    "SimPcsComms",             bval);
//         amap["SimPcsHB"]              = vm->setLinkVal(vmap, aname, "/configsim",    "SimPcsHB",                bval);
//         amap["SimBms_1Comms"]         = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_1Comms",           bval);
//         amap["SimBms_1HB"]            = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_1HB",              bval);
//         amap["SimBms_2Comms"]         = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_2Comms",           bval);
//         amap["SimBms_2HB"]            = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_2HB",              bval);
//         amap["SimBms_3Comms"]         = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_3Comms",           bval);
//         amap["SimBms_3HB"]            = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_3HB",              bval);
//         amap["SimBms_4Comms"]         = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_4Comms",           bval);
//         amap["SimBms_4HB"]            = vm->setLinkVal(vmap, aname, "/configsim",    "SimBms_4HB",              bval);
//           dval = 1.0;
//         amap["HeartBeatPeriod"]       = vm->setLinkVal(vmap, aname, "/config",    "HeartBeatPeriod",         dval);
//         dval = 255.0;
//         amap["HeartBeatMax"]          = vm->setLinkVal(vmap, aname, "/config",    "HeartBeatMax",            dval);
//         amap["HandleHeartBeat"]->setVal(2);  // revert reload
//         if(reload == 0) // complete restart 
//         {
//             amap["HeartBeat"]->setVal(0);
//         }
//         reload = 2;    amap["SimHandleHeartBeat"]->setVal(reload);
//     }
//     dval = vm->get_time_dbl();
//     asprintf(&tVal," the new time is %f", dval);
//     amap["Timestamp"]->setVal(tVal);
//     free((void *)tVal);
//     if (amap["HeartBeat"]->getLastSetDiff(dval)>1.0) 
//     {
//     // get the reference to the variable 
//         assetVar* hb    = amap["HeartBeat"];
//         assetVar* cd    = amap["Timestamp"];
//         assetVar* hbmax = amap["HeartBeatMax"];

//         bool SimPcsComms   = amap["SimPcsComms"]->getbVal();
//         bool SimPcsHB      = amap["SimPcsHB"]->getbVal();
//         bool SimBms_1Comms = amap["SimBms_1Comms"]->getbVal();
//         bool SimBms_1HB    = amap["SimBms_1HB"]->getbVal();
//         bool SimBms_2Comms = amap["SimBms_2Comms"]->getbVal();
//         bool SimBms_2HB    = amap["SimBms_2HB"]->getbVal();
//         bool SimBms_3Comms = amap["SimBms_3Comms"]->getbVal();
//         bool SimBms_3HB    = amap["SimBms_3HB"]->getbVal();
//         bool SimBms_4Comms = amap["SimBms_4Comms"]->getbVal();
//         bool SimBms_4HB    = amap["SimBms_4HB"]->getbVal();

//         //double ival;
//         double dvalmax = hbmax->getdVal();
//         dval = hb->getdVal();
//         dval++;
//         if(dval > dvalmax) dval = 0;
//         //if(1)printf("HeartBeat %s val %f ", aname, dval);

//         hb->setVal(dval);
//         //cd->setVal(dval);
//         //dval = hb->getdVal();
//         if(0)printf("HeartBeat aname %s  val after set %f\n", aname , dval);
//         if (SimPcsComms)  vm->sendAssetVar(cd, p_fims, "/components/pcs_1");
//         if (SimPcsHB)     vm->sendAssetVar(hb, p_fims, "/components/pcs_1");
//         if (SimBms_1Comms)vm->sendAssetVar(cd, p_fims, "/components/bms_1");
//         if (SimBms_1HB)   vm->sendAssetVar(hb, p_fims, "/components/bms_1");
//         if (SimBms_2Comms)vm->sendAssetVar(cd, p_fims, "/components/bms_2");
//         if (SimBms_2HB)   vm->sendAssetVar(hb, p_fims, "/components/bms_2");
//         if (SimBms_3Comms)vm->sendAssetVar(cd, p_fims, "/components/bms_3");
//         if (SimBms_3HB)   vm->sendAssetVar(hb, p_fims, "/components/bms_3");
//         if (SimBms_4Comms)vm->sendAssetVar(cd, p_fims, "/components/bms_4");
//         if (SimBms_4HB)   vm->sendAssetVar(hb, p_fims, "/components/bms_4");
//        }
//     return dval;
// }

// tests the designated status reg for a requested status value
// if the requested value changed we send out the new value request
// we reset the remote system to thr reset value after tSetTime
//
// we wait for the response to match.
// The incoming resp wil also trigger this function...
// the sendvar and resp var are detached from the triggervar
// TODO 
// if we get no response in tRespTime seconds we send out the request again.
// after 5 attemps we then give up and flag a Fault
// returns AssetRespOK true  or AssetReapErr true.
// added itself to the ai's runList While waiting.
// how do we have multiples of these running ??? maybe give the asset its own amap...
// or  use the param list 
// TODO add/remove triggervar to ai->runList
// TODO add response to AssetRespVal changes
// TODO allow SetTime and RespTime to be parameters
//

// "/controls/ess":        {
//                 "run_bms_control":   {
//                         "value":        9000,
//                         "actions":      {
//                                 "onSet":        {
//                                         "func": [{
//                                                         "amap": "ess",
//                                                         "sendCmd":"/controls/bms_1/start_stop_cmd",   << send requested command here
//                                                         "expCmdResponse":"/controls/bms_1/start_stop_resp",   << place expected  resp here
//                                                         "resetCmd": 0,                                << after send reset command to this
//                                                         "CmdSetTime": 0.1,                            << set command from 100.ms 
//                                                         "CmdRespTime": 5.0,                           << wait 5 seconds for each response 
//                                                         "CmdRetries": 5,                              <<  try 5 times 
//                                                         "sendVal":"/controls/bms_1/start_stop"       << where the fimssend goes to
//                                                         "respVal":"/controls/bms_1/bms_status"       << monitor response here
//                                                         "enable":       "/controls/ess:start_enable",
//                                                         "func": "TestAssetStatusAv",
//                                                         "onErr":        "/controls/ess/startErr",
//                                                         "onOK": "/controls/ess/startOK"
//                                                 }]
//                                 }
//                         }
//                 }
//         },

//

int SendAssetCmdAv(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (av->ai && av->ai->am)
    {
        return SendAssetCmd(vmap, amap, aname, p_fims, av->ai->am);
    }
    if (av->am)
    {
        return SendAssetCmd(vmap, amap, aname, p_fims, av->am);
    }
    return -1;
}
// How to test 
// set up /config/ess/expectAssetRespVal response
// set up /config/ess/resetAssetStatusVal
// send   /controls/ess/AssetStatusVal
//runs in the main wake and responds to an av send

int TestAssetCmdAm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    //const char* LAssetStatusCmd        = "AssetStatusCmd";
    //const char* LAssetStatusSendReg    = "AssetStatusSendReg";
    //const char* LresetAssetStatusVal    = "resetAssetStatusVal";
    // const char* LTestAssetStatusAv     = "TestAssetStatusAv";
    // const char* LAssetStatusCmd        = "AssetStatusCmd";
    //const char* LAssetStatusVal          = "AssetStatusVal";
    //const char* LresetAssetStatusVal     = "resetAssetStatusVal";
    //const char* LlastAssetStatusVal      = "lastAssetStatusVal";
    const char* LexpectAssetRespVal = "expectAssetRespVal";
    //const char* LAssetSendNum            = "AssetSendNum";
    const char* LAssetSendTime = "AssetSendTime";


    // // Other vars referenced anyway 
    // const char* LAssetStatusSendReg   = "AssetStatusSendReg";
    // const char* LAssetStatusRespName  = "AssetStatusRespName";
    // const char* LAssetStatusVal       = "AssetStatusVal";
    const char* LAssetRespVal = "AssetRespVal";
    // const char* LlastAssetRespVal     = "lastAssetRespVal";
    // const char* LmaxAssetSendNum      = "maxAssetSendNum";
    // const char* LAssetRespOk          = "AssetRespOk";
    // const char* LAssetRespTime        = "AssetRespTime";
    // const char* LAssetRespErr         = "AssetRespErr";
    // const char* LAssetRespWarn        = "AssetRespWarn";

    double dval = 0.0;
    int ival1 = 0;
    int ival2 = 0;

    dval = amap[LAssetSendTime]->getdVal();

    ival1 = amap[LexpectAssetRespVal]->getiVal();
    //ival2 = amap[LresetAssetStatusVal]->getiVal();

    double tNow = vm->get_time_dbl();
    // we have started
    if (dval > 0.0)
    {

        if (tNow - dval < 1.0)
        {
            if (tNow - dval > 0.5)
            {
                // send out the wrong resp
                ival2 = ival1 + 5;
                amap[LAssetRespVal]->setVal(ival2);
                vm->sendAssetVar(amap[LAssetRespVal], am->p_fims); // can use different comp if required
            }
            else if (tNow - dval > 0.7)
            {
                // send out the wrong resp again
                ival2 = ival1 + 4;
                amap[LAssetRespVal]->setVal(ival2);
                vm->sendAssetVar(amap[LAssetRespVal], am->p_fims); // can use different comp if required
            }
            else if (tNow - dval > 0.9)
            {
                // send out the correct resp again
                ival2 = ival1;
                amap[LAssetRespVal]->setVal(ival2);
                vm->sendAssetVar(amap[LAssetRespVal], am->p_fims); // can use different comp if required
            }
        }

    }

    return 0;
}

VarMapUtils* getAvVm(assetVar* av)
{
    if (av->am && av->am->vm)
    {
        return(av->am->vm);
    }
    if (av->ai && av->ai->am && av->ai->am->vm)
    {
        return(av->ai->am->vm);
    }
    return nullptr;
}

// attach this to an assetVar it will trigger the tester.
int TestAssetCmdAv(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    VarMapUtils* vm = getAvVm(av);

    //const char* LAssetStatusCmd        = "AssetStatusCmd";
    //const char* LAssetStatusSendReg    = "AssetStatusSendReg";
    //const char* LresetAssetStatusVal    = "resetAssetStatusVal";
    //const char* LTestAssetStatusAv     = "TestAssetStatusAv";
    //const char* LAssetStatusCmd        = "AssetStatusCmd";
    const char* LresetAssetStatusVal = "resetAssetStatusVal";
    //const char* LlastAssetStatusVal    = "lastAssetStatusVal";
    const char* LexpectAssetRespVal = "expectAssetRespVal";
    //const char* LAssetSendNum          = "AssetSendNum";


    // Other vars referenced anyway 
    // const char* LAssetStatusSendReg   = "AssetStatusSendReg";
    // const char* LAssetStatusRespName  = "AssetStatusRespName";
    const char* LAssetStatusVal = "AssetStatusVal";
    //const char* LAssetRespVal         = "AssetRespVal";
    // const char* LlastAssetRespVal     = "lastAssetRespVal";
    // const char* LmaxAssetSendNum      = "maxAssetSendNum";
    // const char* LAssetRespOk          = "AssetRespOk";
    // const char* LAssetRespTime        = "AssetRespTime";
    // const char* LAssetRespErr         = "AssetRespErr";
    // const char* LAssetRespWarn        = "AssetRespWarn";


    int ival = 0;
    amap[LresetAssetStatusVal]->setVal(ival);

    int ivalr = 9;
    amap[LexpectAssetRespVal]->setVal(ivalr);

    ival = 21;
    amap[LAssetStatusVal]->setVal(ival);
    // send out the command
    vm->sendAssetVar(amap[LAssetStatusVal], av->am->p_fims); // can use different comp if required
    return 0;
}

// // just run this and the commands should all fall into place ( ie crash)
// // this waits  for coms and hartbeat  and then waits for the assets to get into a ready mode.
// // only works for a cold start.
// // for a woarm sart you dont want to impose a state on the assets.
// // they may  have restarted.
// // If it is a warm start look at the current state of the systems.
// // tru to keep on trucking if it all looks good.
// // we'll have the last requested sate and power command but that is for a different sprint.
// // this guy only handles the cold start.
// // so we force the assets into subordination.
// // then wait for a command.
// // 
// int HandleESSCmd(varsmap &vmap, varmap &amap,  const char *aname, fims* p_fims, asset_manager* am)
// {
//     int rc = 0;
//     bool bval = false;
//     int reload;
//     char* tVal = (char *)" System Init";
//     int ival=0;
//     double dval = 0.0;
//     VarMapUtils*vm = am->vm;
//     assetVar*HandleCmd      = amap["HandleCmd"]; 
//     reload = vm->CheckReload(vmap, amap, aname, __func__);

//     if(reload < 2)
//     {
//         //if(1)FPS_ERROR_PRINT("%s >> %s --- Reload\n", __func__, aname);

//         amap["On"]                = vm->setLinkVal(vmap, aname, "/status",    "On",                bval);
//         amap["On2"]                = vm->setLinkVal(vmap, aname, "/status",   "On2",               bval);
//         amap["Off"]               = vm->setLinkVal(vmap, aname, "/status",    "Off",               bval);
//         amap["Standby"]           = vm->setLinkVal(vmap, aname, "/status",    "Standby",           bval);
//         amap["Fault"]             = vm->setLinkVal(vmap, aname, "/status",    "Fault",             bval);
//         amap["ResetFaultCmd"]     = vm->setLinkVal(vmap, aname, "/controls",  "ResetFaultCmd",     bval);
//         amap["OnCmd"]             = vm->setLinkVal(vmap, aname, "/controls",  "OnCmd",             bval);
//         amap["OffCmd"]            = vm->setLinkVal(vmap, aname, "/controls",  "OffCmd",            bval);
//         amap["StandbyCmd"]        = vm->setLinkVal(vmap, aname, "/controls",  "StandbyCmd",        bval);
//         amap["ResetFaultCmd"]     = vm->setLinkVal(vmap, aname, "/controls",  "ResetFaultCmd",     bval);
//         amap["ResetCmd"]          = vm->setLinkVal(vmap, aname, "/controls",  "ResetCmd",          bval);
//         amap["GridForming"]       = vm->setLinkVal(vmap, aname,   "/status",  "GridForming",     bval);
//         amap["GridFollowing"]     = vm->setLinkVal(vmap, aname,   "/status",  "GridFollowing",   bval);
//         amap["SystemState"]       = vm->setLinkVal(vmap, aname,   "/status",  "SystemState",     tVal);
//         amap["SystemStateNum"]    = vm->setLinkVal(vmap, aname,   "/status",  "SystemStateNum",     ival);
//         amap["lastSystemStateNum"] = vm->setLinkVal(vmap, aname,  "/status",  "lastSystemStateNum", ival);
//         amap["AcContactor"]       = vm->setLinkVal(vmap, aname,   "/status",  "AcContactor",      bval);
//         amap["DcContactor"]       = vm->setLinkVal(vmap, aname,   "/status",  "DcContactor",       bval);
//         amap["AssetStateNum"]        = vm->setLinkVal(vmap, "ess", "/status",   "AssetStateNum",     ival);
//         amap["CommsStateNum"]        = vm->setLinkVal(vmap, "ess", "/status",   "CommsStateNum",     ival);
//         amap["HeartBeatStateNum"]    = vm->setLinkVal(vmap, "ess", "/status",   "HeartBeatStateNum", ival);
//         amap["CommsState"]           = vm->setLinkVal(vmap, "ess", "/status",   "CommsState",        tVal);
//         amap["AssetState"]           = vm->setLinkVal(vmap, "ess", "/status",   "AssetState",        tVal);
//         amap["HeartBeatState"]       = vm->setLinkVal(vmap, "ess", "/status",   "HeartBeatState",    tVal);
//         amap["SystemState"]           = vm->setLinkVal(vmap, "ess", "/status",   "SystemState",       tVal);
//         amap["SystemStateStep"]       = vm->setLinkVal(vmap, "ess", "/status",   "SystemStateStep",   tVal);
//         amap["CurrentSetpoint"]       = vm->setLinkVal(vmap, "ess", "/controls",   "CurrentSetpoint",   dval);
//         amap["PcsCurrentSetpoint"]    = vm->setLinkVal(vmap, "ess", "/status",     "PcsCurrentSetpoint",   dval);

//         amap["AcContactorCloseCmd"]  = vm->setLinkVal(vmap, aname, "/controls", "AcContactorCloseCmd", bval);
//         amap["DcContactorCloseCmd"]  = vm->setLinkVal(vmap, aname, "/controls", "DcContactorCloseCmd", bval);
//         amap["AcContactorOpenCmd"]   = vm->setLinkVal(vmap, aname, "/controls", "AcContactorOpenCmd", bval);
//         amap["DcContactorOpenCmd"]   = vm->setLinkVal(vmap, aname, "/controls", "DcContactorOpenCmd", bval);

//         amap["CommsOk"]           = vm->setLinkVal(vmap, aname, "/status",    "CommsOk", bval);
//         amap["HBOk"]              = vm->setLinkVal(vmap, aname, "/status",    "HBOk", bval);
//         amap["BMSOk"]             = vm->setLinkVal(vmap, "bms", "/status",    "BMSOk", bval);
//         amap["PCROk"]             = vm->setLinkVal(vmap, "pcr", "/status",    "PCROk", bval);
//         amap["DRCOk"]             = vm->setLinkVal(vmap, "drc", "/status",    "DRCOk", bval);
//         amap["EMMOk"]             = vm->setLinkVal(vmap, "emm", "/status",    "EMMOk", bval);
//         amap["readyOk"]           = vm->setLinkVal(vmap, aname, "/status",    "readyOk", bval);
//         amap["readyOkSetCmd"]     = vm->setLinkVal(vmap, aname, "/controls",  "readyOkSetCmd", bval);
//         amap["readyOkClearCmd"]   = vm->setLinkVal(vmap, aname, "/controls",  "readyOkClearCmd", bval);
//         amap["SimPCSCommsCmd"]   = vm->setLinkVal(vmap, aname, "/controls",  "SimPCSCommsCmd", bval);
//         amap["SimPCSHBCmd"]      = vm->setLinkVal(vmap, aname, "/controls",  "SimPCSHBCmd", bval);
//         amap["PcsCmd"]           = vm->setLinkVal(vmap, aname, "/controls",  "PcsCmd", ival);
//         amap["PcsStatus"]        = vm->setLinkVal(vmap, aname, "/status",    "PcsStatus", ival);

//         if (!amap["RunESSInputFunc"]) 
//         {

//             assetFunc* runEssInputFunc = new assetFunc(aname);
//             amap["RunESSInputFunc"] = (assetVar *)runEssInputFunc;
//             runEssInputFunc->setupRamFunc(HandleESSInput,vmap, amap, aname, p_fims, am);
//             amap["readyOkSetCmd"]       ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["readyOkClearCmd"]     ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["AcContactorCloseCmd"] ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["AcContactorOpenCmd"]  ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["DcContactorCloseCmd"] ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["DcContactorOpenCmd"]  ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["OnCmd"]               ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["OffCmd"]              ->SetPubFunc = (assetVar* )runEssInputFunc;
//             amap["StandbyCmd"]          ->SetPubFunc = (assetVar* )runEssInputFunc;
//         }

//         if(reload == 0) // complete restart 
//         {
//             amap["SystemState"]->setVal(tVal);
//             amap["On"]->setVal(false);
//             amap["OnCmd"]->setVal(false);
//             amap["Off"]->setVal(false);
//             amap["OffCmd"]->setVal(false);
//             amap["Standby"]->setVal(false);
//             amap["StandbyCmd"]->setVal(false);
//             amap["Fault"]->setVal(false);
//             amap["ResetFaultCmd"]->setVal(false);
//             amap["GridForming"]->setVal(false);
//             amap["GridFollowing"]->setVal(false);
//             amap["SystemState"]->setVal(tVal);
//             amap["SystemStateNum"]->setVal(System_Init); // starts init timeout
//             amap["lastSystemStateNum"]->setVal(System_Startup); // starts init timeout


//             amap["CommsOk"]->setVal(true);
//             amap["HBOk"]->setVal(true);
//             amap["BMSOk"]->setVal(true);
//             amap["PCROk"]->setVal(true);
//             amap["DRCOk"]->setVal(true);
//             amap["EMMOk"]->setVal(true);
//             amap["readyOk"]->setVal(false);
//             HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemInit);
//             const char * cVal = "System Startup";
//             amap["SystemState"]->setVal(cVal);
//             cVal = "Waiting for Comms";
//             amap["SystemStateStep"]->setVal(cVal);
//             cVal = "Assets Init";
//             amap["AssetState"]->setVal(cVal);


//         }
//         else
//         {
//             HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemRestart);
//         }

//         reload = 2;
//         HandleCmd      = amap[__func__];
//         HandleCmd->setVal(reload);
//     }
//     // CommsState and HeartBeatState must be set 

//     bool OnCmd           = amap["OnCmd"]->getbVal();
//     bool OffCmd          = amap["OffCmd"]->getbVal();
//     bool StandbyCmd      = amap["StandbyCmd"]->getbVal();
//     bool ResetCmd        = amap["ResetCmd"]->getbVal();
//     bool ResetFaultCmd   = amap["ResetFaultCmd"]->getbVal();
//     //bool Offval = amap["Off"]->getbVal();

//     bool On         = amap["On"]->getbVal();

//     bool Off        = amap["Off"]->getbVal();
//     bool Standby    = amap["Standby"]->getbVal();
//     bool Fault      = amap["Fault"]->getbVal();

//     bool AcContactor          = amap["AcContactor"]->getbVal();
//     bool DcContactor          = amap["DcContactor"]->getbVal();
//     bool AcContactorOpenCmd   = amap["AcContactorOpenCmd"]->getbVal();
//     bool DcContactorOpenCmd   = amap["DcContactorOpenCmd"]->getbVal();
//     bool AcContactorCloseCmd  = amap["AcContactorCloseCmd"]->getbVal();
//     bool DcContactorCloseCmd  = amap["DcContactorCloseCmd"]->getbVal();

//     bool readyOk              = amap["readyOk"]->getbVal();
//     bool readyOkSetCmd        = amap["readyOkSetCmd"]->getbVal();
//     bool readyOkClearCmd      = amap["readyOkClearCmd"]->getbVal();

//     rc = 0;

//     dval = vm->get_time_dbl();

//     int  sState       = amap["SystemStateNum"]->getiVal();  
//     int  lastState    = amap["lastSystemStateNum"]->getiVal();  

//     if(lastState != sState)
//     {
//         FPS_ERROR_PRINT("%s >> %s ---ssnum %d --> %d  OnCmd [%s] On [%s] last set time %f\n"
//                 , __func__
//                 , aname
//                 , lastState
//                 , sState
//                 , OnCmd?"true":"false"
//                 , On?"true":"false"
//                 , amap["SystemStateNum"]->getLastSetDiff(dval)
//                 );
//         amap["lastSystemStateNum"]->setVal(sState);  
//     }

//     // So if we are in System
//     //amap["SystemState"]->setVal(cVal);
//     //cVal = "Waiting For Comms";
//     //amap["SystemStateStep"]->setVal(cVal);
//     // TODO only do this after Setup
//     // TODO process by AssetManager 
//     // this will also set Ess:readyOK

//     //SimHandleHeartComms(vmap, amap, aname, p_fims, am);
//     SimHandleHeartBeat(vmap, amap, aname, p_fims, am);

//     char* sVal2     =   amap["SystemState"]->getcVal();
//     char* sValStep2 =   amap["SystemStateStep"]->getcVal();
//     int ival5 = amap["CommsStateNum"]->getiVal();
//     char *sVal5 = amap["CommsState"]->getcVal();
//     char *sVal6 = amap["AssetState"]->getcVal();
//     char *sVal4 = amap["HeartBeatState"]->getcVal());

//     if(0)FPS_ERROR_PRINT("%s >> [%s] Looking for System Startup got [%s] Step [%s] CommsState [%s] num %d time %f\n"
//                     , __func__
//                     , aname
//                     , sVal2
//                     , sValStep2
//                     , sVal5 ? sVal5:"No value in CommsState"
//                     , ival5
//                     , dval
//                     );

//     if(strcmp(sVal2, "System Startup")== 0)
//     {
//         if(0)FPS_ERROR_PRINT("  %s >> [%s] Found System Startup [%s]  Step [%s] CommsState [%s] HeartBeatState [%s] AssetState [%s] time %f\n"
//                     , __func__
//                     , aname
//                     , sVal2
//                     , sValStep2
//                     , sVal5 ? sVal5:"No value in CommsState"
//                     , sVal4
//                     , sVal6
//                     , dval
//                     );

//         if(strcmp(sValStep2, "Waiting for Comms")== 0) 
//         {
//             if(0)FPS_ERROR_PRINT("    %s >> [%s] Looking for CommsOK got %d time %f\n"
//                     , __func__
//                     , aname
//                     , ival5
//                     , dval
//                     );

//             if (ival5 == Asset_Ok)
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] CommsOK looking for HeartBeat at time %f\n"
//                     , __func__
//                     , aname
//                     , dval
//                     );
//                 sValStep2 =  (char *)"Waiting for HeartBeat";
//                 amap["SystemStateStep"]->setVal(sValStep2);
//                 return 0;
//             }
//         }
//         else if(strcmp(sValStep2, "Waiting for HeartBeat")== 0) 
//         {
//             int ival5 = amap["HeartBeatStateNum"]->getiVal();
//             if (ival5 == Asset_Ok)
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] HeartBeat OK Reset Assets at time %f\n"
//                     , __func__
//                     , aname
//                     , dval
//                     );
//                 FPS_ERROR_PRINT("%s >> send \"Assets Ready\" to /status/ess/AssetState to continue\n"
//                     , __func__
//                     , aname
//                     , dval
//                     );
//                 sValStep2 =  (char *)"Reset Assets";
//                 amap["SystemStateStep"]->setVal(sValStep2);
//                 return 0;
//             }
//         }
//         else if(strcmp(sValStep2, "Reset Assets")== 0) 
//         {
//             char * sVal7 = amap["AssetState"]->getcVal();
//             if ((ival5 == Asset_Ok) || (strcmp(sVal7,"Assets Ready")==0))
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] Assets OK move to ready at time %f\n"
//                     , __func__
//                     , aname
//                     , dval
//                     );
//                 sValStep2 = (char *)"Waiting for Command";
//                 amap["SystemStateStep"]->setVal(sValStep2);
//                 sValStep2 =  (char *)"System Ready";
//                 amap["SystemState"]->setVal(sValStep2);
//                 return 0;
//             }
//         }

//     }
//     else if(strcmp(sVal2, "System Ready")== 0)
//     {
//         if(strcmp(sValStep2, "Waiting for Command")== 0) 
//         {
//             // from here we can go to "System On" or "System Standby" or "System Fault'"
//             OnCmd           = amap["OnCmd"]->getbVal();
//             //On              = amap["On"]->getbVal();
//             if (OnCmd)
//             {
//                 FPS_ERROR_PRINT("%s >> [%s]Got On Cmd at time %f Check DcContactor and SOC etc\n"
//                     , __func__
//                     , aname
//                     , dval
//                     );
//                 OnCmd  = false ;; amap["OnCmd"]->setVal(OnCmd);

//                 sValStep2 = (char *)"Waiting for DcContactorClosed";
//                 amap["SystemStateStep"]->setVal(sValStep2);
//                 sVal2 = (char *)"System On";
//                 amap["SystemState"]->setVal(sVal2);
//                 //sValStep2 =  (char *)"System On";
//                 //amap["SystemState"]->setVal(sValStep2);
//                 // Send the contactor Close command
//                 bval = true;
//                 bval = true;; amap["DcContactorCloseCmd"]->setVal(bval);

//                 // TODO check For CloseOK
//                 vm->sendAssetVar(amap["DcContactorCloseCmd"], p_fims);
//                 // Start Time out ( infacr use out neat command utility)
//             }
//         }
//     }
//     if(strcmp(sVal2, "System On")== 0)
//     {
//         // allow fall through
//         if(strcmp(sValStep2, "Waiting for DcContactorClosed")== 0) 
//         {
//             //TODO Need Timeout here
//             // Head off to a Fault condition after 5 attempts perhaps
//             bool DcContactor = amap["DcContactor"]->getbVal();

//             if (DcContactor)
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] DCContactor closed at time %f Check Current Setpoint and SOC etc\n"
//                     , __func__
//                     , aname
//                     , dval
//                     );

//                 sValStep2 = (char *)"Waiting for Current Setpoint";
//                 amap["SystemStateStep"]->setVal(sValStep2);            
//             }

//         }
//         // allow fall through
//         if(strcmp(sValStep2, "Waiting for Current Setpoint")== 0) 
//         { 
//             double currentSetpoint = amap["CurrentSetpoint"]->getdVal(); 
//             if (currentSetpoint > 0.0) 
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] currentSetpoint set at %f at time %f Check Pcstatus feedback and SOC etc\n"
//                     , __func__
//                     , aname
//                     , currentSetpoint
//                     , dval
//                     );

//                 // Send currentSetpoint command

//                  // TODO check For CloseOK
//                  // TODO check limits soc etc
//                 amap["PcsCurrentSetpoint"]->setVal(currentSetpoint);
//                 vm->sendAssetVar(amap["PcsCurrentSetpoint"], p_fims);
//                 ival = 21; //PcsOn;
//                 amap["PcsCmd"]->setVal(ival);
//                 vm->sendAssetVar(amap["PcsCmd"], p_fims);
//                 sValStep2 = (char *)"Waiting for PCSStatus";
//                 amap["SystemStateStep"]->setVal(sValStep2);            

//             }
//         }
//         // allow fall through
//         if(strcmp(sValStep2, "Waiting for PCSStatus")== 0) 
//         { 

//             int pcsStatus = amap["PcsStatus"]->getiVal();            // we can get faults
//             int pcsOkStatus = 1;
//             if (pcsStatus == pcsOkStatus)
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] PcsStatus %d at time %f move to GridFollowing, check SOC etc\n"
//                     , __func__
//                     , aname
//                     , pcsStatus
//                     , dval
//                     );

//                 sValStep2 = (char *)"Running GridFollowing";
//                 amap["SystemStateStep"]->setVal(sValStep2);            

//             }
//             // we can get other commands
//             // or we can stay here for ever

//         }
//         if(strcmp(sValStep2, "Running GridFollowing")== 0) 
//         { 

//             int pcsStatus = amap["PcsStatus"]->getiVal();            // we can get faults
//             int pcsOkStatus = 1;
//             // TODO check SOC and capacity
//             if (pcsStatus != pcsOkStatus)
//             {
//                 FPS_ERROR_PRINT("%s >> [%s] PcsStatus %d at time %f move to System Fault, check SOC etc\n"
//                     , __func__
//                     , aname
//                     , pcsStatus
//                     , dval
//                     );

//                 sValStep2 = (char *)"PCS Fault";
//                 amap["SystemStateStep"]->setVal(sValStep2);            
//                 sVal2 = (char *)"System Fault";
//                 amap["SystemState"]->setVal(sValStep2);            

//             }
//         }

//             // we can get other commands
//             // or we can stay here for eve
//     }

//     if(strcmp(sVal2, "System Fault")== 0)
//     {

//         sValStep2 = (char *)"Waiting for Fault Reset";
//         amap["SystemStateStep"]->setVal(sValStep2);            
//         // from here we can go to "System On" or "System Standby" but we have to get the Fault Reset Command
//         // from here we can go to "System On" or "System Standby" or "System Fault'"
//         bool resetFaultCmd           = amap["ResetFaultCmd"]->getbVal();

//         if (resetFaultCmd)
//         {
//             resetFaultCmd  = false ;; amap["resetFaultCmd"]->setVal(resetFaultCmd);
//             sValStep2 = (char *)"Waiting for Command";
//             amap["SystemStateStep"]->setVal(sValStep2);
//             sValStep2 =  (char *)"System Ready";
//             amap["SystemState"]->setVal(sValStep2);
//         }


//     }
//     else
//     {

//         if(0)FPS_ERROR_PRINT("  %s >> [%s] State  [%s]  Step [%s] time %f\n"
//                     , __func__
//                     , aname
//                     , sVal2
//                     , sValStep2
//                     , dval
//                     );
//     }



//     //rc = CheckSystemState(vmap, amap, aname, p_fims, am);

//     AcContactor     = amap["AcContactor"]->getbVal();
//     DcContactor     = amap["DcContactor"]->getbVal();
//     AcContactorOpenCmd  = amap["AcContactorOpenCmd"]->getbVal();
//     DcContactorOpenCmd  = amap["DcContactorOpenCmd"]->getbVal();
//     AcContactorCloseCmd  = amap["AcContactorCloseCmd"]->getbVal();
//     DcContactorCloseCmd  = amap["DcContactorCloseCmd"]->getbVal();
//     OnCmd           = amap["OnCmd"]->getbVal();
//     On              = amap["On"]->getbVal();
//     readyOk         = amap["readyOk"]->getbVal();
//     char *sVal      =  amap["SystemState"]->getcVal();
//     if(0)FPS_ERROR_PRINT("%s >> %s -After Check  -- SystemState [%s] rc %d nCmd [%s] On [%s] Standby [%s] readyOk [%s] AcContactor [%s] DcContactor [%s] go  [%s] go2  [%s]\n"
//                 , __func__
//                 , aname
//                 , sVal ? sVal:"NotSet"
//                 , rc
//                 , OnCmd?"true":"false"
//                 , On?"true":"false"
//                 , Standby?"true":"false"
//                 , readyOk?"true":"false"
//                 , AcContactor?"true":"false"
//                 , DcContactor?"true":"false"
//                 , (OnCmd && !On )?"Go":"NoGo"
//                 , (OnCmd && (!On || Standby) && AcContactor && DcContactor)?"Go":"NoGo"
//                 );
//     if (rc != 0)
//     {
//         if(1)FPS_ERROR_PRINT("%s >> %s -Running Fault Shutdown rc %d -- OnCmd [%s] On [%s] readyOk [%s] \n"
//                 , __func__
//                 , aname
//                 , rc
//                 , OnCmd?"true":"false"
//                 , On?"true":"false"
//                 , readyOk?"true":"false"
//                 );
//         HandleFaultShutdown(vmap, amap, aname, p_fims, am);
//         return 0;
//     }
//     rc = 0;
//     bool tval = true;
//     bool fval = false;

//     // state ready means comms OK and HBok for each asset.
//     // all can be overridden with CommsOverride and HBOverride 
//     //We have to be in state ready or standby before we can turn on.
//     if(!readyOk)
//     {
//         if (readyOkSetCmd)
//         {
//             amap["readyOkSetCmd"]->setVal(fval);
//             amap["readyOk"]->setVal(tval);
//         }
//     }
//     else
//     {
//         if (readyOkClearCmd)
//         {
//             amap["readyOkClearCmd"]->setVal(fval);
//             amap["readyOk"]->setVal(fval);
//         }
//     }

//     readyOk         = amap["readyOk"]->getbVal();
//     if(readyOk)
//     {
//         if (AcContactorCloseCmd)
//         {

//             rc = AcContactorOkToClose(vmap, amap, aname, p_fims, am);
//             if(rc == 0)
//             {
//                 amap["AcContactorCloseCmd"]->setVal(fval);
//                 amap["AcContactor"]->setVal(tval);
//             }
//         }

//         if (AcContactorOpenCmd )
//         {
//             amap["AcContactorOpenCmd"]->setVal(fval);
//             if (AcContactorOkToOpen(vmap, amap, aname, p_fims, am))
//             {
//                 amap["AcContactor"]->setVal(fval);
//             }
//         }
//         if (DcContactorCloseCmd)
//         {
//             amap["DcContactorCloseCmd"]->setVal(fval);
//             if (DcContactorOkToClose(vmap, amap, aname, p_fims, am))
//             {
//                 amap["DcContactor"]->setVal(tval);
//             }
//         }
//         if (DcContactorOpenCmd )
//         {
//             amap["DcContactorOpenCmd"]->setVal(fval);

//             if (DcContactorOkToOpen(vmap, amap, aname, p_fims, am))
//             {
//                 amap["DcContactor"]->setVal(fval);
//             }
//         }

//         if (OnCmd && (!On || Standby) && AcContactor && DcContactor)
//         {
//             bool tval = true;
//             bool fval = false;

//             amap["On"]->setVal(tval);
//             // Send On to device
//             amap["OnCmd"]->setVal(fval);
//             amap["Standby"]->setVal(fval);
//             OnCmd           = amap["OnCmd"]->getbVal();
//             On              = amap["On"]->getbVal();
//             if(1)FPS_ERROR_PRINT("%s >> %s -After OnCmd Process rc %d -- OnCmd [%s] On [%s]\n"
//                     , __func__
//                     , aname
//                     , rc
//                     , OnCmd?"true":"false"
//                     , On?"true":"false"
//                     );

//             HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemOn);
//             OnCmd           = amap["OnCmd"]->getbVal();
//             On              = amap["On"]->getbVal();
//             if(0)FPS_ERROR_PRINT("%s >> %s -After Handle Manager rc %d -- OnCmd [%s] On [%s]\n"
//                     , __func__
//                     , aname
//                     , rc
//                     , OnCmd?"true":"false"
//                     , On?"true":"false"
//                     );

//         }
//         else if ((On||Standby) && OffCmd)
//         {
//             rc++;
//             amap["On"]->setVal(false);
//             // Send Off to device
//             amap["OffCmd"]->setVal(false);
//             // Send Off to device
//             amap["Standby"]->setVal(false);

//             amap["Off"]->setVal(true);

//             HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemOff);

//         }
//         else if ((On||Off) && StandbyCmd)
//         {
//             amap["Standby"]->setVal(true);
//             // send standby to device
//             amap["StandbyCmd"]->setVal(false);
//             amap["On"]->setVal(false);
//             amap["Off"]->setVal(false);
//             HandleManagerCmd(vmap, amap, aname, p_fims, am, SystemStandby);

//         }
//     }
//     else
//     {  // not readyOK
//         if ((On||Off) && !Standby)
//         {
//             amap["On"]->setVal(false);
//             amap["Off"]->setVal(false);
//         }
//     }

//     //TODO send stuff to FIMS
//     return rc;
// }

// THis is run from the setVar action oNSET for estop .. yet to be completed put in place. 
// we Simply register an ESTOP assetVar and run it....
//  We can call this function and if ESTOP is not asserted we can just proceed with the setup
// NOTE that, during setup,  the assetvar will be given a reference to the asset manager here and a wakeup number
// the incoming fims messsage will call vm.setvar. this will then trigger a channel wakeup.
// the next thing the asset manager will do is service that wakeup, from a channel put, after processing the fims message.
// this will probabbly respond in about 100 uSecs or so.


int HandleEStop(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    bool bval;
    //double dval = 0.0;
    int ival = 0;
    char* cval;
    char* csVal = (char*)"EStop Init";
    VarMapUtils* vm = am->vm;
    //const char *manName = am->name.c_str();
    FPS_ERROR_PRINT("%s >> for [%s]   ESTOP \n", __func__, aname);

    //if(am->am)
    //    manName = am->am->name.c_str();  // refer up unless we're top
    int reload = 0;
    reload = vm->CheckReload(vmap, amap, aname, __func__);

    if (reload < 2)
    {

        if (1)FPS_ERROR_PRINT("%s >> for [%s]  reload [%d]  amap[HandleEStop]  %p amap %p\n", __func__, aname, reload, (void*)amap["HandleEStop"], (void*)&amap);

        //double warnVal = 3.5;
        // //double errVal = 5.0;
        // double pper = 0.250;
        // amap["CheckComms"]         = vm->setLinkVal(vmap, aname, "/reload",    "CheckComms",     reload);
        // amap["CheckCommsPeriod"]   = vm->setLinkVal(vmap, aname, "/config",    "CheckCommsPeriod",  pper);
        // amap["CheckCommsRun"]      = vm->setLinkVal(vmap, aname, "/config",    "CheckCommsRun",     dval);
        amap["EStopCmd"] = vm->setLinkVal(vmap, "ess", "/component", "EStopCmd", bval);
        amap["EStop"] = vm->setLinkVal(vmap, "ess", "/component", "EStop", bval);
        // amap["essCommsWarns"]      = vm->setLinkVal(vmap, "ess", "/status",    "essCommsWarns",     ival);
        // if(am->am)
        // {
        //     amap["amCommsErrors"]     = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsErrors",    ival);
        //     amap["amCommsWarns"]      = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsWarns",     ival);
        // }
        // amap["CommsErrors"]        = vm->setLinkVal(vmap, aname, "/status",    "CommsErrors",       ival);
        // amap["CommsWarns"]         = vm->setLinkVal(vmap, aname, "/status",    "CommsWarns",        ival);
        amap["EStopState"] = vm->setLinkVal(vmap, "ess", "/status", "EStopState", csVal);

        // // amap["errCommsTimeout"]    = vm->setLinkVal(vmap, aname, "/config",    "errCommsTimeout",   errVal);
        // // amap["warnCommsTimeout"]   = vm->setLinkVal(vmap, aname, "/config",    "warnCommsTimeout",  warnVal);


        if (reload == 0) // complete restart 
        {
            bval = false;
            cval = (char*)"EStop Armed";
            amap["EStopState"]->setVal(cval);
            amap["EStopCmd"]->setVal(bval); //     = vm->setLinkVal(vmap, "ess", "/component",    "EStopCmd",    bval);
            amap["EStop"]->setVal(bval);//        = vm->setLinkVal(vmap, "ess", "/component",    "EStop",    bval);
        }
        ival = 2; amap[__func__]->setVal(ival);
    }

    // double tNow = vm->get_time_dbl();
    // double pval = amap["CheckCommsRun"]->getLastSetDiff(tNow);
    // double plim = amap["CheckCommsPeriod"]->getdVal();
    bool runme = true;
    // int warns;
    // int errs;
    // // are we the controller ??
    if (!am->am)
    {
        bool esCmd = false;
        bool esIn = false;
        esCmd = amap["EStopCmd"]->getbVal(); //     = vm->setLinkVal(vmap, "ess", "/component",    "EStopCmd",    bval);
        esIn = amap["EStop"]->getbVal();//        = vm->setLinkVal(vmap, "ess", "/component",    "EStop",    bval);

        if (1)FPS_ERROR_PRINT("%s >> OK Test Again  EStop >>esCmd: %d esIn: %d \n", __func__, esCmd, esIn);
        if (!esCmd && esIn)
        {
            amap["EStopCmd"]->setVal(true); //     = vm->setLinkVal(vmap, "ess", "/component",    "EStopCmd",    bval);

        }
        else
        {
            runme = false;
            // amap["CheckCommsRun"]->setVal(tNow);
            // amap["essCommsWarns"]->setVal(0);
            // amap["essCommsErrors"]->setVal(0);
        }
    }

    if (runme)
    {
        //int ival, ival2;
    // assets are in assetMap managers are in assetManMap
        // amap["CommsWarns"]->setVal(0);
        // amap["CommsErrors"]->setVal(0);

        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;

            HandleEStop(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
        }
        for (auto ix : am->assetMap)
        {
            asset* amc = ix.second;
            if (0)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  Asset Man [%s]\n ", __func__, aname, amc->name.c_str());

            //HandleAssetEStop(vmap, amc->amap,amc->name.c_str(),p_fims, amc);
        }
        // collect output and pass to parent
        if (am->am)
        {
            // warns = amap["CommsWarns"]->getiVal();
            // errs = amap["CommsErrors"]->getiVal();

            // amap["amCommsErrors"]->addVal(errs);
            // amap["amCommsWarns"] ->addVal(warns);
            // if(1)FPS_ERROR_PRINT("%s >>>>>> AM [%s]  Manager [%s] warns %d errs %d\n "
            //         ,__func__
            //         , aname
            //         , am->am->name.c_str()
            //         , warns
            //         , errs
            //         );

        }
        else
        {
            // warns = amap["essCommsWarns"]->getiVal();
            // errs = amap["essCommsErrors"]->getiVal();
        }

        // if(1)FPS_ERROR_PRINT("%s >>>>>> ESS [%s]  warns %d errs %d\n ",__func__, aname
        //         , warns
        //         , errs
        //         );
        int errs = 1;
        if (errs > 0)
        {
            char* cval = (char*)"EStop ";
            amap["EStopState"]->setVal(cval);
        }
        else
        {
            char* cval = (char*)"EStop Reset";
            amap["EStopState"]->setVal(cval);
        }
    }
    return 0;
}
//  This runs every time set /componments/xxx/PCRCmd is run
//
//   void setupRamFunc(int (*_runFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager *am),varsmap *_vmap, varmap *_amap,const char* _aname, fims* _p_fims, asset_manager *_am)
//  This only runs when we get a set or Pub  PCRCmd ( Default /components/pcr/PCRCmd)  
// int HandlePCRCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager *am)
// {
//     VarMapUtils* vm;
//     vm = am->vm;
//     int reload = 0; 
//     int ival = -1;
//     char *tVal=(char *) " PCR Command Init";
//     reload = CheckReload(vmap, amap, aname, __func__);
//     if (reload < 2 )
//     {
//         amap["PCRCmd"]             = vm->setLinkVal(vmap, aname, "/components",    "PCRCmd",        ival);
//         amap["PCRCmdStatus"]       = vm->setLinkVal(vmap, aname, "/status",        "PCRCmdStatus",      tVal);
//         if(reload < 1)
//         {
//             // do init stiff here
//             amap["PCRCmdStatus"]->setVal(tVal); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);
//             amap["PCRCmd"]->setVal(ival); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);

//         }
//         reload = 2;
//         amap[__func__]->setVal(reload);
//     }
//     char * cval; 
//     double cmdTime = vm->get_time_dbl();
//     ival = amap["PCRCmd"]->getiVal(); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);

//     asprintf(&cval, "%s >> Set PCRCmd Status to %d at %f",__func__, ival, cmdTime);
//     if(cval)
//     {
//         amap["PCRCmdStatus"]->setVal(cval); //       = vm->setLinkVal(vmap, aname, "/status",    "PCRCmdStatus",     tVal);
//         free((void *)cval);
//     }
//     // Now do stuff to handle change in PCSCmd 
//     return 0;
// }


// this sets upthe PCRCmd to run the HandlePCRCmd function every time the PCRCmd is "set"
// this instance must be set up by the asset manager
// this really should be an init function.
// to test set a value in /components/ess/PCRCmd  and check /ess/status/ess/PCRCmdStatus


// int SetupRunPCRCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager *am)
// {

//     FPS_ERROR_PRINT(">>>>>>>>>ESS>>>>>>>>>>>%s running for ESS Manager\n",__func__);

//     assetFunc* RunPCRCmd;
//     VarMapUtils* vm;
//     vm = am->vm;
//     int reload = 0; 
//     reload = CheckReload(vmap, amap, aname, __func__);

//     if (reload < 2)
//     {
//         //amap["SetupRunPCRCmd"]     = vm->setLinkVal(vmap, aname, "/reload",        "SetupRunPCRCmd",     reload);
//         amap["RunPCRCmd"]          = vm->setLinkVal(vmap, aname, "/status",        "RunPCRCmd",     reload);
//         amap["PCRCmd"]             = vm->setLinkVal(vmap, aname, "/components",    "PCRCmd",     reload);

//         if ( reload < 1)
//         {
//             RunPCRCmd =(assetFunc *) new assetFunc(aname);
//             amap["RunPCRCmdFunc"] = (assetVar *)RunPCRCmd;
//             RunPCRCmd->setupRamFunc(HandlePCRCmd,vmap, amap, aname, p_fims, am);
//             // the following causes HandlePCRCmd to be run on eery Set or Pub on PCRCmd
//             // may need to use VarMapUtils to runit
//             amap["PCRCmd"] ->SetFunc = (assetVar* )RunPCRCmd;
//             amap["PCRCmd"] ->PubFunc = (assetVar* )RunPCRCmd;
//         }
//         reload = 2;
//         amap[__func__]->setVal(reload);
//     }

//     return 0;
// }

