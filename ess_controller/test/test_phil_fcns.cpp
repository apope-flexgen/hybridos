// look for a change in /status/ess/heartbeat
// compare against /status/bms_xxheartbeat // send hartbeat and TOD if changed
#include "asset.h"
#include "chrono_utils.hpp"

// test message junk
int sendTestMessage(fims* p_fims, int tnum)
{
    const char* method;
    const char* replyto = nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;

    switch (tnum)
    {
        case 1:
        {
            method = "set";
            replyto = "/test/foo";
            uri = "/components/test_1";
            body = "{\"var_set_one\":21}";
        }
        break;
        case 2:
        {
            method = "set";
            // replyto = "/test/foo";
            uri = "/components/test_2";
            body = "{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        }
        break;
        case 3:
        {
            method = "set";
            replyto = "/test/foo_2";
            uri = "/components/test_2";
            body = "{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        }
        break;
        case 4:
        {
            method = "set";
            replyto = "/test/foo_4";
            uri = "/components/test_3";
            body = "{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 5:
        {
            method = "get";
            replyto = "/test/foo_5";
            uri = "/components/test_3";
            // body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 6:
        {
            method = "get";
            replyto = "/test/foo_6";
            uri = "/components/test_3/var_set_twox";
            // body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 7:
        {
            method = "set";
            replyto = "/test/foo_7";
            uri = "/assets/bms_1";
            body = "{\"ctrlword1\":{\"value\":3}}";
        }
        break;
        case 8:
        {
            method = "set";
            replyto = "/test/foo_8";
            uri = "/assets/bms_1";
            body = "{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
        }
        break;
        case 9:
        {
            method = "set";
            replyto = "/test/foo_9";
            uri = "/components/catl_ems_bms_rw";
            body = "{\"ems_test_status\":{\"value\":\"Running\"}}";
        }
        break;
        default:
            break;
    }
    if (uri)
        p_fims->Send(method, uri, replyto, body);
    return 0;
}

template <class T>
assetVar* getFeatAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const char* aname, const char* item,
                    const char* avName, assetFeatDict* aDict, T defval)
{
    assetVar* avItem = nullptr;
    std::string mvar = avName;
    std::string mvar2;
    char* mname;
    char* avStr;

    avItem = aDict->getFeat(item, &avItem);
    if (!avItem)
    {
        avStr = aDict->getFeat(item, &avStr);
        if (avStr)
        {
            char* tmp = strdup(avStr);
            char* suri = &tmp[4];  // skip past av::
            char* saname = nullptr;
            char* svar = nullptr;
            // Crappy Non Fault tolerant parser
            char* sep1 = strstr(&suri[1], "/");
            char* sep2 = nullptr;
            if (sep1)
            {
                *sep1++ = 0;
                saname = sep1;

                sep2 = strstr(&saname[1], ":");
                if (sep2)
                {
                    *sep2++ = 0;
                    svar = sep2;
                }
            }
            if (sep1 && sep2)
            {
                FPS_ERROR_PRINT("%s >> found item [%s] avStr [%s] uri[%s] saname [%s] svar [%s]\n", __func__, item,
                                avStr, suri, saname, svar);

                // for example av::/status/ess:Errors
                // TODO create the assetVar and place it in aDict and amap
                avItem = amap[svar] = vm->setLinkVal(vmap, saname, suri, svar, defval);
                free((void*)tmp);
                aDict->setFeat(item, avItem);
                return avItem;
            }
        }
    }

    if (avItem)
        mname = (char*)avItem->name.c_str();
    else
    {
        mvar2 = mvar + "_" + item;
        mname = (char*)mvar2.c_str();
    }
    amap[mname] = vm->setLinkVal(vmap, aname, "/params", mname, defval);
    avItem = avItem ? avItem : amap[mname];

    return avItem;
}

// the rundemo functions are what we want to run
int rundemoAM(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    FPS_ERROR_PRINT("%s >>>> cascade AM runnng for  Manager [%s] \n ", __func__, am->name.c_str());
    return 0;
}

int rundemoAI(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
{
    FPS_ERROR_PRINT("%s >>>> cascade AI runnng for  Asset [%s] \n ", __func__, am->name.c_str());
    return 0;
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
// If this is the ess_controller am->am == nullptr then just run the other
// managers Set System into Fault mode if we drop comms foir more that 5 seconds
// NOTE Ben wanted comms to be back for a reset time before coming out of fault.
// We'll add that in
// if this is an asset manager report back to the controller
// comms must be down for alarm time or fault time and recover for reset time

// int CheckAmComms(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset_manager* am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval = (char*)"Comms Init";
//     VarMapUtils* vm = am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckAmComms");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toAlarm = 0.5;
//     double toFault = 5.0;
//     double toReset = 2.5;
//     char* initTimestamp = (char*)" Initial Timestamp";

//     //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n",
//     __func__, aname, reload); if (reload < 2)
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n",
//         __func__, aname, reload);

//         amap["Timestamp"] = vm->setLinkVal(vmap, aname, "/status",
//         "Timestamp", initTimestamp); if (1)FPS_ERROR_PRINT("%s >>  aname
//         TimeStamp %p comp [%s] name [%s] \n"
//             , __func__
//             , aname
//             , amap["Timestamp"]->comp.c_str()
//             , amap["Timestamp"]->name.c_str()
//         );

//         amap["essCommsFaults"] = vm->setLinkVal(vmap, "ess", "/status",
//         "essCommsFaults", ival); amap["essCommsAlarms"] =
//         vm->setLinkVal(vmap, "ess", "/status", "essCommsAlarms", ival);
//         amap["essCommsInit"] = vm->setLinkVal(vmap, "ess", "/status",
//         "essCommsInit", ival); amap["essCommsTimeoutFault"] =
//         vm->setLinkVal(vmap, "ess", "/config", "essCommsTimeoutFault",
//         toFault); amap["essCommsTimeoutAlarm"] = vm->setLinkVal(vmap, "ess",
//         "/config", "essCommsTimeoutAlarm", toAlarm);
//         amap["essCommsTimeoutReset"] = vm->setLinkVal(vmap, "ess", "/config",
//         "essCommsTimeoutReset", toReset);

//         if (am->am)
//         {
//             amap["amCommsFaults"] = vm->setLinkVal(vmap,
//             am->am->name.c_str(), "/status", "CommsFaults", ival);
//             amap["amCommsAlarms"] = vm->setLinkVal(vmap,
//             am->am->name.c_str(), "/status", "CommsAlarms", ival);
//             amap["amCommsInit"] = vm->setLinkVal(vmap, am->am->name.c_str(),
//             "/status", "CommsInit", ival);
//         }

//         amap["CommsFaults"] = vm->setLinkVal(vmap, aname, "/status",
//         "CommsFaults", ival); amap["CommsAlarms"] = vm->setLinkVal(vmap,
//         aname, "/status", "CommsAlarms", ival); amap["CommsInit"] =
//         vm->setLinkVal(vmap, aname, "/status", "CommsInit", ival);
//         amap["CommsState"] = vm->setLinkVal(vmap, aname, "/status",
//         "CommsState", cval); amap["BypassComms"] = vm->setLinkVal(vmap,
//         aname, "/config", "BypassComms", bval); amap["AssetState"] =
//         vm->setLinkVal(vmap, aname, "/status", "AssetState", ival);
//         amap["CommsStateNum"] = vm->setLinkVal(vmap, aname, "/status",
//         "CommsStateNum", ival);

//         if (reload == 0) // complete restart
//         {
//             amap["Timestamp"]->setVal(initTimestamp);
//             //lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
//             amap["Timestamp"]->setParam("lastTimestamp", initTimestamp);
//             amap["Timestamp"]->setParam("totalCommsFaults", 0);
//             amap["Timestamp"]->setParam("totalCommsAlarms", 0);
//             amap["Timestamp"]->setParam("seenFault", false);
//             amap["Timestamp"]->setParam("seenOk", false);
//             amap["Timestamp"]->setParam("seenAlarm", false);
//             amap["Timestamp"]->setParam("seenInit", false);
//             amap["Timestamp"]->setParam("initCnt", -1);

//             amap["Timestamp"]->setParam("rdFault", toFault);
//             // time remaining before fault
//             amap["Timestamp"]->setParam("rdAlarm", toAlarm);
//             // time reamining before alarm
//             amap["Timestamp"]->setParam("rdReset", toReset);
//             // time remaining before reset
//             amap["Timestamp"]->setParam("rdLast", dval);
//             // time when last to event was seen

//             amap["CommsState"]->setVal(cval);
//             ival = Asset_Init; amap["CommsStateNum"]->setVal(ival);
//             ival = -1; amap["CommsInit"]->setVal(ival);
//             amap["BypassComms"]->setVal(false);

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

//     toFault = amap["essCommsTimeoutFault"]->getdVal();
//     toAlarm = amap["essCommsTimeoutAlarm"]->getdVal();
//     toReset = amap["essCommsTimeoutReset"]->getdVal();

//     char* currentTimestamp = amap["Timestamp"]->getcVal();
//     char* lastTimestamp =
//     amap["Timestamp"]->getcParam("lastTimestamp");//amap["lastHeartBeat"]->getiVal();
//     // are we the ess_controller
//     if (!am->am)
//     {
//         //bool initSeen =             amap["Timestamp"]
//         ->getbParam("initSeen");

//         amap["essCommsFaults"]->setVal(0);
//         amap["essCommsAlarms"]->setVal(0);
//         amap["essCommsInit"]->setVal(0);

//         int initCnt = amap["Timestamp"]->getiParam("initCnt");
//         int icnt = 0;
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager* amc = ix.second;
//             CheckAmComms(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
//             icnt++;
//         }

//         int essCommsFaults = amap["essCommsFaults"]->getiVal();
//         int essCommsAlarms = amap["essCommsAlarms"]->getiVal();
//         //int essCommsInit = amap["essCommsInit"]->getiVal();
//         if (essCommsFaults > 0)
//         {
//             FPS_ERROR_PRINT("%s >> %d essCommsFaults detected\n",
//             __func__, essCommsFaults);
//         }
//         if (essCommsAlarms > 0)
//         {
//             FPS_ERROR_PRINT("%s >> %d essCommsFaults detected\n",
//             __func__, essCommsAlarms);
//         }

//         if (initCnt != icnt)
//         {
//             amap["Timestamp"]->setParam("initCnt", icnt);

//             FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n",
//             __func__, icnt, initCnt);
//         }
//         return 0;

//     }

//     if (BypassComms)
//     {
//         ival = 1;
//         amap["essCommsInit"]->addVal(ival);
//         return 0;

//     }
//     // If we are in the init state wait for comms to start count down reset
//     time if (strcmp(currentTimestamp, initTimestamp) == 0)
//     {
//         bool seenInit = amap["Timestamp"]->getbParam("seenInit");

//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         if (0)FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n",
//         __func__, aname, BypassComms ? "true" :
//         "false");

//         // if not toally set up yet then quit this pass
//         if (!amap["amCommsInit"])
//         {
//             return 0;
//         }

//         if (!seenInit)   // Comms_Setup
//         {
//             amap["Timestamp"]->setParam("seenInit", true);

//             char* cval = (char*)"Comms Init, no Timestamp Seen";
//             amap["CommsState"]->setVal(cval);

//             ival = 1;
//             amap["essCommsInit"]->addVal(ival);
//             amap["CommsInit"]->setVal(0);      //Comms_Init
//         }
//         amap["Timestamp"]->setParam("rdLast", tNow);

//     }
//     else  // wait for comms to go past reset then set active or wait to alarm
//     and then fault
//     {
//         //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s]
//         \n", __func__, aname,
//         lastTimestamp?lastTimestamp:"no last Value", tval1); double rdLast =
//         amap["Timestamp"]->getdParam("rdLast"); double rdFault =
//         amap["Timestamp"]->getdParam("rdFault"); double rdAlarm =
//         amap["Timestamp"]->getdParam("rdAlarm"); double rdReset =
//         amap["Timestamp"]->getdParam("rdReset");
//         amap["Timestamp"]->setParam("rdLast", tNow);

//         double toVal = amap["Timestamp"]->getLastSetDiff(tNow);

//         // Has value changed ? If yes then count down rdReset to zero based
//         on tNow - rdLast if (strcmp(currentTimestamp, lastTimestamp) != 0)
//             //if(amap["Timestamp"]->valueChangedReset())
//         {
//             amap["Timestamp"]->setParam("lastTimestamp", currentTimestamp);

//             bool seenOk = amap["Timestamp"]->getbParam("seenOk");
//             if (rdReset > 0.0)
//             {
//                 rdReset -= (tNow - rdLast);
//                 amap["Timestamp"]->setParam("rdReset", rdReset);
//             }
//             //else
//             {
//                 // TODO after reset increment these up to toAlarm
//                 if (rdAlarm < toAlarm)
//                 {
//                     rdAlarm += tNow - rdLast;
//                     amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
//                 }
//                 if (rdFault < toFault)
//                 {
//                     rdFault += tNow - rdLast;
//                     amap["Timestamp"]->setParam("rdFault", rdFault);
//                 }
//             }

//             if (0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s]
//             rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault %2.3f\n"
//                 , __func__, aname, lastTimestamp ?
//                 lastTimestamp : "no last Value", currentTimestamp, rdReset,
//                 (tNow - rdLast), rdAlarm, rdFault);

//             ival = amap["CommsStateNum"]->getiVal();
//             // reset time passed , still changing , time to switch to
//             Comms_Ready if ((rdReset <= 0.0) && (ival != seenOk))
//             {

//                 bool seenFault = amap["Timestamp"]->getbParam("seenFault");
//                 //bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
//                 bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");
//                 amap["Timestamp"]->setParam("seenOk", true);

//                 if (0)FPS_ERROR_PRINT("%s >>  Timestamp  change for %s from
//                 [%s] to [%s] \n", __func__, aname,
//                 lastTimestamp ? lastTimestamp : "no last Value",
//                 currentTimestamp); if (seenFault)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  Timestamp fault for  %s
//                     cleared at %2.3f\n", __func__,
//                     aname, tNow); amap["Timestamp"]->setParam("seenFault",
//                     false);

//                 }
//                 if (seenAlarm)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  Timestamp Alarm for  %s
//                     cleared at %2.3f\n", __func__,
//                     aname, tNow); amap["Timestamp"]->setParam("seenAlarm",
//                     false);

//                 }
//                 if (1)FPS_ERROR_PRINT("%s >>  Timestamp OK for  %s at
//                 %2.3f\n", __func__, aname, tNow); ival =
//                 Asset_Ok; // seen Timestamp change
//                 amap["CommsStateNum"]->setVal(ival); ival = 0;
//                 amap["CommsInit"]->setVal(ival); char* tval; asprintf(&tval,
//                 " Comms OK last set %2.3f Alarm %3.2f max %3.2f", toVal,
//                 toAlarm, toFault); if (tval)
//                 {
//                     amap["CommsState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//             }

//             // increment alarm and fault time reset time
//             if (rdFault < toFault)
//             {
//                 rdFault += (tNow - rdLast);
//                 amap["Timestamp"]->setParam("rdFault", rdFault);
//             }
//             if (rdAlarm < toAlarm)
//             {
//                 rdAlarm += (tNow - rdLast);
//                 amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
//             }

//             //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to
//             [%s] \n", __func__, aname, lastTs?lastTs:"no
//             last Value", Ts); amap["Timestamp"]->setParam("lastTimestamp",
//             currentTimestamp);
//             //if ((toVal > toFault)  && !bokFault && !bypass)

//         }
//         else   // No Change , start tracking faults and alarms
//         {
//             bool seenFault = amap["Timestamp"]->getbParam("seenFault");
//             //bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
//             bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");
//             if (rdFault > 0.0)
//             {
//                 rdFault -= (tNow - rdLast);
//                 amap["Timestamp"]->setParam("rdFault", rdFault);
//             }
//             if (rdAlarm > 0.0)
//             {
//                 rdAlarm -= (tNow - rdLast);
//                 amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
//             }
//             if (rdReset < toReset)
//             {
//                 rdReset += (tNow - rdLast);
//                 amap["Timestamp"]->setParam("rdReset", rdReset);
//             }

//             if ((rdFault <= 0.0) && !seenFault)
//             {

//                 if (1)FPS_ERROR_PRINT("%s >>  Timestamp  Fault  for %s at
//                 %2.3f \n", __func__, aname, tNow); char*
//                 tval; asprintf(&tval, " Comms Fault last set %2.3f Alarm
//                 %3.2f max %3.2f", toVal, toAlarm, toFault); if (tval)
//                 {
//                     amap["CommsState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//                 int ival = 1;
//                 amap["CommsFaults"]->addVal(ival);
//                 amap["essCommsFaults"]->addVal(ival);

//                 if (am->am)
//                 {
//                     amap["amCommsFaults"]->addVal(ival);
//                 }

//                 ival = Asset_Fault; //Timestamp Fault
//                 amap["CommsStateNum"]->setVal(ival);

//                 seenFault = true;
//                 amap["Timestamp"]->setParam("seenFault", true);
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 amap["Timestamp"]->setParam("seenAlarm", true);
//                 //seenOk = false;
//                 seenAlarm = false;

//                 int totalCommsFaults =
//                 amap["Timestamp"]->getiParam("totalCommsFaults");
//                 totalCommsFaults++;
//                 amap["Timestamp"]->setParam("totalCommsFaults",
//                 totalCommsFaults);

//             }
//             else if ((rdAlarm <= 0.0) && !seenAlarm)
//             {
//                 if (1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f \n",
//                 __func__, aname, tNow);

//                 char* tval;
//                 asprintf(&tval, "Comms Alarm last set %2.3f Alarm %3.2f max
//                 %3.2f", toVal, toAlarm, toFault); if (tval)
//                 {
//                     amap["CommsState"]->setVal(tval);
//                     free((void*)tval);
//                 }

//                 int ival = 1;
//                 amap["CommsAlarms"]->addVal(ival);
//                 amap["essCommsAlarms"]->addVal(ival);

//                 if (am->am)
//                 {
//                     amap["amCommsAlarms"]->addVal(ival);
//                 }
//                 ival = Asset_Alarm; //Timestamp Alarm
//                 amap["CommsStateNum"]->setVal(ival);

//                 amap["Timestamp"]->setParam("seenAlarm", true);
//                 //amap["Timestamp"]->setParam("seenFault", false);
//                 amap["Timestamp"]->setParam("seenOk", false);
//                 int totalCommsAlarms =
//                 amap["Timestamp"]->getiParam("totalCommsAlarms");
//                 totalCommsAlarms++;
//                 amap["Timestamp"]->setParam("totalCommsAlarms",
//                 totalCommsAlarms);
//             }
//             else
//             {
//                 if (0)FPS_ERROR_PRINT("%s >> Comms for [%s] [%s] Stalled at
//                 %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
//                     , __func__
//                     , aname
//                     , amap["Timestamp"]->getcVal()
//                     , tNow
//                     , rdReset, rdFault, rdAlarm);

//             }
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n",
//     __func__, aname,
//     amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal()); return 0;
// };

// // test status against ExpStatus
// // logs any changes
// int CheckAmPcsStatus(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset_manager* am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval = (char*)"PcsStatus Init";
//     VarMapUtils* vm = am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckAmPcsStatus");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toAlarm = 2.5;
//     double toFault = 10.0;
//     double toReset = 2.5;
//     int initPcsStatus = -1;//(char *)" Initial PcsStatus";

//     //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n",
//     __func__, aname, reload); if (reload < 2)
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n",
//         __func__, aname, reload);

//         amap["PcsStatus"] = vm->setLinkVal(vmap, aname, "/status",
//         "PcsStatus", initPcsStatus); amap["PcsExpStatus"] =
//         vm->setLinkVal(vmap, aname, "/status", "PcsExpStatus",
//         initPcsStatus); if (1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp
//         [%s] name [%s] \n"
//             , __func__
//             , aname
//             , amap["PcsStatus"]->comp.c_str()
//             , amap["PcsStatus"]->name.c_str()
//         );

//         amap["essPcsStatusFaults"] = vm->setLinkVal(vmap, "ess", "/status",
//         "essPcsStatusFaults", ival); amap["essPcsStatusAlarms"] =
//         vm->setLinkVal(vmap, "ess", "/status", "essPcsStatusAlarms", ival);
//         amap["essPcsStatusInit"] = vm->setLinkVal(vmap, "ess", "/status",
//         "essPcsStatusInit", ival); amap["essPcsStatusTimeoutFault"] =
//         vm->setLinkVal(vmap, "ess", "/config", "essPcsStatusTimeoutFault",
//         toFault); amap["essPcsStatusTimeoutAlarm"] = vm->setLinkVal(vmap,
//         "ess", "/config", "essPcsStatusTimeoutAlarm", toAlarm);
//         amap["essPcsStatusTimeoutReset"] = vm->setLinkVal(vmap, "ess",
//         "/config", "essPcsStatusTimeoutReset", toReset);

//         if (am->am)
//         {
//             amap["amPcsStatusFaults"] = vm->setLinkVal(vmap,
//             am->am->name.c_str(), "/status", "PcsStatusFaults", ival);
//             amap["amPcsStatusAlarms"] = vm->setLinkVal(vmap,
//             am->am->name.c_str(), "/status", "PcsStatusAlarms", ival);
//             amap["amPcsStatusInit"] = vm->setLinkVal(vmap,
//             am->am->name.c_str(), "/status", "PcsStatusInit", ival);
//         }

//         amap["PcsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status",
//         "PcsStatusFaults", ival); amap["PcsStatusAlarms"] =
//         vm->setLinkVal(vmap, aname, "/status", "PcsStatusAlarms", ival);
//         amap["PcsStatusInit"] = vm->setLinkVal(vmap, aname, "/status",
//         "PcsStatusInit", ival); amap["PcsStatusState"] = vm->setLinkVal(vmap,
//         aname, "/status", "PcsStatusState", cval); amap["BypassPcsStatus"] =
//         vm->setLinkVal(vmap, aname, "/config", "BypassPcsStatus", bval);
//         amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status",
//         "AssetState", ival); amap["PcsStatusStateNum"] = vm->setLinkVal(vmap,
//         aname, "/status", "PcsStatusStateNum", ival);

//         if (reload == 0) // complete restart
//         {
//             amap["PcsStatus"]->setVal(initPcsStatus);
//             //lastPcsStatus=strdup(tsInit);//state"]->setVal(cval);
//             amap["PcsStatus"]->setParam("lastPcsStatus", initPcsStatus);
//             amap["PcsStatus"]->setParam("totalPcsStatusFaults", 0);
//             amap["PcsStatus"]->setParam("totalPcsStatusAlarms", 0);
//             amap["PcsStatus"]->setParam("seenFault", false);
//             amap["PcsStatus"]->setParam("seenOk", false);
//             amap["PcsStatus"]->setParam("seenAlarm", false);
//             amap["PcsStatus"]->setParam("seenInit", false);
//             amap["PcsStatus"]->setParam("initCnt", -1);

//             amap["PcsStatus"]->setParam("rdFault", toFault);
//             // time remaining before fault
//             amap["PcsStatus"]->setParam("rdAlarm", toAlarm);
//             // time reamining before alarm
//             amap["PcsStatus"]->setParam("rdReset", toReset);
//             // time remaining before reset
//             amap["PcsStatus"]->setParam("rdLast", dval);
//             // time when last to event was seen

//             amap["PcsStatusState"]->setVal(cval);
//             ival = Asset_Init; amap["PcsStatusStateNum"]->setVal(ival);
//             ival = -1; amap["PcsStatusInit"]->setVal(ival);
//             amap["BypassPcsStatus"]->setVal(false);

//         }
//         // reset reload
//         ival = 2; amap["CheckAmPcsStatus"]->setVal(ival);
//     }

//     double tNow = am->vm->get_time_dbl();

//     bool BypassPcsStatus = amap["BypassPcsStatus"]->getbVal();

//     toFault = amap["essPcsStatusTimeoutFault"]->getdVal();
//     toAlarm = amap["essPcsStatusTimeoutAlarm"]->getdVal();
//     toReset = amap["essPcsStatusTimeoutReset"]->getdVal();

//     int currentPcsStatus = amap["PcsStatus"]->getiVal();
//     int expectedPcsStatus = amap["PcsExpStatus"]->getiVal();
//     int lastPcsStatus =
//     amap["PcsStatus"]->getiParam("lastPcsStatus");//amap["lastHeartBeat"]->getiVal();

//     if (BypassPcsStatus)
//     {
//         ival = 1;
//         amap["essPcsStatusInit"]->addVal(ival);
//         return 0;

//     }
//     // If we are in the init state wait for comms to start count down reset
//     time if (currentPcsStatus == initPcsStatus)
//     {
//         bool seenInit = amap["PcsStatus"]->getbParam("seenInit");

//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         if (0)FPS_ERROR_PRINT("%s >> %s  NO PcsStatus,  bypass [%s]\n",
//         __func__, aname, BypassPcsStatus ? "true" :
//         "false");

//         // if not toally set up yet then quit this pass
//         if (!amap["amPcsStatusInit"])
//         {
//             return 0;
//         }

//         if (!seenInit)   // PcsStatus_Setup
//         {
//             amap["PcsStatus"]->setParam("seenInit", true);

//             char* cval = (char*)"PcsStatus Init, no PcsStatus Seen";
//             amap["PcsStatusState"]->setVal(cval);

//             ival = 1;
//             amap["essPcsStatusInit"]->addVal(ival);
//             amap["PcsStatusInit"]->setVal(0);      //PcsStatus_Init
//         }
//         amap["PcsStatus"]->setParam("rdLast", tNow);

//     }
//     else  // wait for comms to go past reset then set active or wait to alarm
//     and then fault
//     {
//         //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s]
//         \n", __func__, aname,
//         lastPcsStatus?lastPcsStatus:"no last Value", tval1); double rdLast =
//         amap["PcsStatus"]->getdParam("rdLast"); double rdFault =
//         amap["PcsStatus"]->getdParam("rdFault"); double rdAlarm =
//         amap["PcsStatus"]->getdParam("rdAlarm"); double rdReset =
//         amap["PcsStatus"]->getdParam("rdReset");
//         amap["PcsStatus"]->setParam("rdLast", tNow);

//         double toVal = amap["PcsStatus"]->getLastSetDiff(tNow);

//         // Has value changed ? If yes then count down rdReset to zero based
//         on tNow - rdLast if (currentPcsStatus != lastPcsStatus)
//         {
//             amap["PcsStatus"]->setParam("lastPcsStatus", currentPcsStatus);
//             // TODO log changes
//         }
//         if (currentPcsStatus == expectedPcsStatus)
//             //if(amap["PcsStatus"]->valueChangedReset())
//         {
//             amap["PcsStatus"]->setParam("lastPcsStatus", currentPcsStatus);

//             bool seenOk = amap["PcsStatus"]->getbParam("seenOk");
//             if (rdReset > 0.0)
//             {
//                 rdReset -= (tNow - rdLast);
//                 amap["PcsStatus"]->setParam("rdReset", rdReset);
//             }
//             //else
//             {
//                 // TODO after reset increment these up to toAlarm
//                 if (rdAlarm < toAlarm)
//                 {
//                     rdAlarm += tNow - rdLast;
//                     amap["PcsStatus"]->setParam("rdAlarm", rdAlarm);
//                 }
//                 if (rdFault < toFault)
//                 {
//                     rdFault += tNow - rdLast;
//                     amap["PcsStatus"]->setParam("rdFault", rdFault);
//                 }
//             }

//             if (0)FPS_ERROR_PRINT("%s >>  PcsStatus change for %s from [%d]
//             to [%d]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault
//             %2.3f\n"
//                 , __func__, aname, lastPcsStatus,
//                 currentPcsStatus, rdReset, (tNow - rdLast), rdAlarm,
//                 rdFault);

//             ival = amap["PcsStatusStateNum"]->getiVal();
//             // reset time passed , still changing , time to switch to
//             PcsStatus_Ready if ((rdReset <= 0.0) && (ival != seenOk))
//             {

//                 bool seenFault = amap["PcsStatus"]->getbParam("seenFault");
//                 //bool seenOk  = amap["PcsStatus"]->getbParam("seenOk");
//                 bool seenAlarm = amap["PcsStatus"]->getbParam("seenAlarm");
//                 amap["PcsStatus"]->setParam("seenOk", true);

//                 if (0)FPS_ERROR_PRINT("%s >>  PcsStatus  change for %s from
//                 [%d] to [%d] \n", __func__, aname,
//                 lastPcsStatus, currentPcsStatus); if (seenFault)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  PcsStatus fault for  %s
//                     cleared at %2.3f\n", __func__,
//                     aname, tNow); amap["PcsStatus"]->setParam("seenFault",
//                     false);

//                 }
//                 if (seenAlarm)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  PcsStatus Alarm for  %s
//                     cleared at %2.3f\n", __func__,
//                     aname, tNow); amap["PcsStatus"]->setParam("seenAlarm",
//                     false);

//                 }
//                 if (1)FPS_ERROR_PRINT("%s >>  PcsStatus OK for  %s at
//                 %2.3f\n", __func__, aname, tNow); ival =
//                 Asset_Ok; // seen PcsStatus change
//                 amap["PcsStatusStateNum"]->setVal(ival); ival = 0;
//                 amap["PcsStatusInit"]->setVal(ival);
//                 char* tval;
//                 asprintf(&tval, " PcsStatus OK last set %2.3f Alarm %3.2f max
//                 %3.2f", toVal, toAlarm, toFault); if (tval)
//                 {
//                     amap["PcsStatusState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//             }

//             // increment alarm and fault time reset time
//             if (rdFault < toFault)
//             {
//                 rdFault += (tNow - rdLast);
//                 amap["PcsStatus"]->setParam("rdFault", rdFault);
//             }
//             if (rdAlarm < toAlarm)
//             {
//                 rdAlarm += (tNow - rdLast);
//                 amap["PcsStatus"]->setParam("rdAlarm", rdAlarm);
//             }

//             //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to
//             [%s] \n", __func__, aname, lastTs?lastTs:"no
//             last Value", Ts); amap["PcsStatus"]->setParam("lastPcsStatus",
//             currentPcsStatus);
//             //if ((toVal > toFault)  && !bokFault && !bypass)

//         }
//         else   // No Change , start tracking faults and alarms
//         {
//             bool seenFault = amap["PcsStatus"]->getbParam("seenFault");
//             //bool seenOk  = amap["PcsStatus"]->getbParam("seenOk");
//             bool seenAlarm = amap["PcsStatus"]->getbParam("seenAlarm");
//             if (rdFault > 0.0)
//             {
//                 rdFault -= (tNow - rdLast);
//                 amap["PcsStatus"]->setParam("rdFault", rdFault);
//             }
//             if (rdAlarm > 0.0)
//             {
//                 rdAlarm -= (tNow - rdLast);
//                 amap["PcsStatus"]->setParam("rdAlarm", rdAlarm);
//             }
//             if (rdReset < toReset)
//             {
//                 rdReset += (tNow - rdLast);
//                 amap["PcsStatus"]->setParam("rdReset", rdReset);
//             }

//             if ((rdFault <= 0.0) && !seenFault)
//             {

//                 if (1)FPS_ERROR_PRINT("%s >>  PcsStatus  Fault  for %s at
//                 %2.3f \n", __func__, aname, tNow); char*
//                 tval; asprintf(&tval, " PcsStatus Fault last set %2.3f Alarm
//                 %3.2f max %3.2f", toVal, toAlarm, toFault); if (tval)
//                 {
//                     amap["PcsStatusState"]->setVal(tval);
//                     free((void*)tval);
//                 }
//                 int ival = 1;
//                 amap["PcsStatusFaults"]->addVal(ival);
//                 amap["essPcsStatusFaults"]->addVal(ival);

//                 if (am->am)
//                 {
//                     amap["amPcsStatusFaults"]->addVal(ival);
//                 }

//                 ival = Asset_Fault; //PcsStatus Fault
//                 amap["PcsStatusStateNum"]->setVal(ival);

//                 seenFault = true;
//                 amap["PcsStatus"]->setParam("seenFault", true);
//                 amap["PcsStatus"]->setParam("seenOk", false);
//                 amap["PcsStatus"]->setParam("seenAlarm", true);
//                 //seenOk = false;
//                 seenAlarm = false;

//                 int totalPcsStatusFaults =
//                 amap["PcsStatus"]->getiParam("totalPcsStatusFaults");
//                 totalPcsStatusFaults++;
//                 amap["PcsStatus"]->setParam("totalPcsStatusFaults",
//                 totalPcsStatusFaults);

//             }
//             else if ((rdAlarm <= 0.0) && !seenAlarm)
//             {
//                 if (1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f \n",
//                 __func__, aname, tNow);

//                 char* tval;
//                 asprintf(&tval, "PcsStatus Alarm last set %2.3f Alarm %3.2f
//                 max %3.2f", toVal, toAlarm, toFault); if (tval)
//                 {
//                     amap["PcsStatusState"]->setVal(tval);
//                     free((void*)tval);
//                 }

//                 int ival = 1;
//                 amap["PcsStatusAlarms"]->addVal(ival);
//                 amap["essPcsStatusAlarms"]->addVal(ival);

//                 if (am->am)
//                 {
//                     amap["amPcsStatusAlarms"]->addVal(ival);
//                 }
//                 ival = Asset_Alarm; //PcsStatus Alarm
//                 amap["PcsStatusStateNum"]->setVal(ival);

//                 amap["PcsStatus"]->setParam("seenAlarm", true);
//                 //amap["PcsStatus"]->setParam("seenFault", false);
//                 amap["PcsStatus"]->setParam("seenOk", false);
//                 int totalPcsStatusAlarms =
//                 amap["PcsStatus"]->getiParam("totalPcsStatusAlarms");
//                 totalPcsStatusAlarms++;
//                 amap["PcsStatus"]->setParam("totalPcsStatusAlarms",
//                 totalPcsStatusAlarms);
//             }
//             else
//             {
//                 if (0)FPS_ERROR_PRINT("%s >> PcsStatus for [%s] [%s] Stalled
//                 at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
//                     , __func__
//                     , aname
//                     , amap["PcsStatus"]->getcVal()
//                     , tNow
//                     , rdReset, rdFault, rdAlarm);

//             }
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n",
//     __func__, aname,
//     amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal()); return 0;
// };

// // check against expected BMS status log changes
// // We get two status vars from the BMS and we send one back
// // outputs
// // output "id": "catl_ems_bms_rw",
//             //   "id": "ems_status",
//             //   "offset": 898,
//             //   "name": "EMS_status"
//             // },
//             //  "id": "catl_mbmu_stat_r",
//             //  {
//             //   "id": "mbmu_status",
//             //   "offset": 16,
//             //   "name": "System status",
//             //   "enum": true,
//             //   "bit_strings": [
//             //     "Initialize",
//             //     "Normal",
//             //     "Full Charge",
//             //     "Full Discharge",
//             //     "Warning Status",
//             //     "Fault Status"
//             //    ]
//             // }
//             // inputs
// //            "id": "catl_bms_ems_r",
// // {
// //               "id": "bms_status",
// //               "offset": 770,
// //               "name": "BMS_status",
// //               "enum": true,
// //               "bit_strings": [
// //                 "Initial status",
// //                 "Normal status",
// //                 "Full Charge status",
// //                 "Full Discharge status",
// //                 "Warning status",
// //                 "Fault status"
// //               ]
// //             },

// int CheckAmBmsStatus(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset_manager* am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     bool bval = false;
//     int dval = 0.0;
//     char* cval = (char*)"BmsStatus Init";
//     VarMapUtils* vm = am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckAmBmsStatus",
//     (void*)&CheckAmBmsStatus);
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     double toAlarm = 2.5;
//     double toFault = 10.0;
//     double toReset = 2.5;
//     int initBmsStatus = -1;//(char *)" Initial BmsStatus";

//     //if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n",
//     __func__, aname, reload); if (reload < 2)
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n",
//         __func__, aname, reload);

//         amap["BmsStatus"] = vm->setLinkVal(vmap, aname, "/status",
//         "BmsStatus", initBmsStatus); amap["MbmuStatus"] =
//         vm->setLinkVal(vmap, aname, "/status", "MbmuStatus", initBmsStatus);
//         amap["BmsStatusString"] = vm->setLinkVal(vmap, aname, "/status",
//         "BmsStatusString", cval); amap["MbmuStatusString"] =
//         vm->setLinkVal(vmap, aname, "/status", "MbmuStatusString", cval);
//         amap["BmsStatusString2"] = vm->setLinkVal(vmap, aname, "/status",
//         "BmsStatusString2", cval);

//         amap["BmsExpStatus"] = vm->setLinkVal(vmap, aname, "/status",
//         "BmsExpStatus", cval); amap["MbmuExpStatus"] = vm->setLinkVal(vmap,
//         aname, "/status", "MbmuExpStatus", cval); amap["BmsTestToAlarm"] =
//         vm->setLinkVal(vmap, aname, "/status", "BmsTestToAlarm", toAlarm);

//         if (1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s]
//         \n"
//             , __func__
//             , aname
//             , amap["BmsStatus"]->comp.c_str()
//             , amap["BmsStatus"]->name.c_str()
//         );
//         if (reload < 1)
//         {
//             vm->setAmFunc(vmap, amap, aname, p_fims, am, "BmsStatusString",
//             CheckAmBmsStatus); vm->setAmFunc(vmap, amap, aname, p_fims, am,
//             "MbmuStatusString", CheckAmBmsStatus);
//         }

//         amap["essBmsStatusFaults"] = vm->setLinkVal(vmap, "ess", "/status",
//         "essBmsStatusFaults", ival); amap["essBmsStatusAlarms"] =
//         vm->setLinkVal(vmap, "ess", "/status", "essBmsStatusAlarms", ival);
//         amap["essBmsStatusInit"] = vm->setLinkVal(vmap, "ess", "/status",
//         "essBmsStatusInit", ival); amap["essBmsStatusTimeoutFault"] =
//         vm->setLinkVal(vmap, "ess", "/config", "essBmsStatusTimeoutFault",
//         toFault); amap["essBmsStatusTimeoutAlarm"] = vm->setLinkVal(vmap,
//         "ess", "/config", "essBmsStatusTimeoutAlarm", toAlarm);
//         amap["essBmsStatusTimeoutReset"] = vm->setLinkVal(vmap, "ess",
//         "/config", "essBmsStatusTimeoutReset", toReset);

//         // if(am->am)
//         // {
//         //     amap["amBmsStatusFaults"]  = vm->setLinkVal(vmap,
//         am->am->name.c_str(), "/status",    "BmsStatusFaults",         ival);
//         //     amap["amBmsStatusAlarms"]  = vm->setLinkVal(vmap,
//         am->am->name.c_str(), "/status",    "BmsStatusAlarms",         ival);
//         //     amap["amBmsStatusInit"]    = vm->setLinkVal(vmap,
//         am->am->name.c_str(), "/status",    "BmsStatusInit",           ival);
//         // }

//         amap["BmsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status",
//         "BmsStatusFaults", ival); amap["BmsStatusAlarms"] =
//         vm->setLinkVal(vmap, aname, "/status", "BmsStatusAlarms", ival);
//         amap["BmsStatusInit"] = vm->setLinkVal(vmap, aname, "/status",
//         "BmsStatusInit", ival); amap["BmsStatusState"] = vm->setLinkVal(vmap,
//         aname, "/status", "BmsStatusState", cval); amap["BmsBypassStatus"] =
//         vm->setLinkVal(vmap, aname, "/config", "BypassBmsStatus", bval);
//         amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status",
//         "AssetState", ival); amap["BmsStatusStateNum"] = vm->setLinkVal(vmap,
//         aname, "/status", "BmsStatusStateNum", ival);

//         amap["MbmuStatusFaults"] = vm->setLinkVal(vmap, aname, "/status",
//         "MbmuStatusFaults", ival); amap["MbmuStatusAlarms"] =
//         vm->setLinkVal(vmap, aname, "/status", "MbmuStatusAlarms", ival);
//         amap["MbmuStatusInit"] = vm->setLinkVal(vmap, aname, "/status",
//         "MbmuStatusInit", ival); amap["MbmuStatusState"] =
//         vm->setLinkVal(vmap, aname, "/status", "MbmuStatusState", cval);
//         amap["MbmuBypassStatus"] = vm->setLinkVal(vmap, aname, "/config",
//         "MbmuBypassStatus", bval); amap["MbmuStatusStateNum"] =
//         vm->setLinkVal(vmap, aname, "/status", "MbmuStatusStateNum", ival);

//         if (reload == 0) // complete restart
//         {
//             amap["BmsStatus"]->setVal(initBmsStatus);
//             amap["BmsExpStatus"]->setVal(cval);
//             amap["MbmuExpStatus"]->setVal(cval);
//             //lastBmsStatus=strdup(tsInit);//state"]->setVal(cval);
//             amap["BmsStatus"]->setParam("lastBmsStatus", initBmsStatus);
//             amap["BmsStatusString"]->setVal(cval);
//             amap["BmsStatusString"]->setParam("lastStatusString", cval);
//             amap["MbmuStatusString"]->setVal(cval);
//             amap["MbmuStatusString"]->setParam("lastStatusString", cval);

//             amap["BmsStatusString2"]->setVal(cval);
//             amap["BmsStatusString2"]->setParam("lastBmsStatusString", cval);

//             amap["BmsStatus"]->setParam("totalStatusFaults", 0);
//             amap["BmsStatus"]->setParam("totalStatusAlarms", 0);
//             amap["BmsStatus"]->setParam("seenFault", false);
//             amap["BmsStatus"]->setParam("seenOk", false);
//             amap["BmsStatus"]->setParam("seenAlarm", false);
//             amap["BmsStatus"]->setParam("seenInit", false);
//             amap["BmsStatus"]->setParam("initCnt", -1);

//             amap["BmsStatus"]->setParam("rdFault", toFault);
//             // time remaining before fault
//             amap["BmsStatus"]->setParam("rdAlarm", toAlarm);
//             // time reamining before alarm
//             amap["BmsStatus"]->setParam("rdReset", toReset);
//             // time remaining before reset
//             amap["BmsStatus"]->setParam("rdLast", dval);
//             // time when last to event was seen
//             amap["BmsStatus"]->setParam("ParamtoAlarm",
//             amap["BmsTestToAlarm"]);  // Set an Av as a param

//             amap["BmsStatusState"]->setVal(cval);

//             amap["MbmuStatus"]->setParam("totalStatusFaults", 0);
//             amap["MbmuStatus"]->setParam("totalStatusAlarms", 0);
//             amap["MbmuStatus"]->setParam("seenFault", false);
//             amap["MbmuStatus"]->setParam("seenOk", false);
//             amap["MbmuStatus"]->setParam("seenAlarm", false);
//             amap["MbmuStatus"]->setParam("seenInit", false);
//             amap["MbmuStatus"]->setParam("initCnt", -1);

//             amap["MbmuStatus"]->setParam("rdFault", toFault);
//             // time remaining before fault
//             amap["MbmuStatus"]->setParam("rdAlarm", toAlarm);
//             // time reamining before alarm
//             amap["MbmuStatus"]->setParam("rdReset", toReset);
//             // time remaining before reset
//             amap["MbmuStatus"]->setParam("rdLast", dval);
//             // time when last to event was seen

//             amap["MbmuStatusState"]->setVal(cval);

//             ival = Asset_Init; amap["MbmuStatusStateNum"]->setVal(ival);
//             ival = -1; amap["MbmuStatusInit"]->setVal(ival);
//             amap["BmsBypassStatus"]->setVal(false);
//             amap["MbmuBypassStatus"]->setVal(false);

//             // if(!am->am)  // Nah do this in setLinkVals
//             // {
//             //     amap["essBmsStatusTimeoutFault"] ->setVal(toFault);
//             //     amap["essBmsStatusTimeoutAlarm"] ->setVal(toAlarm);
//             //     amap["essBmsStatusTimeoutReset"] ->setVal(toReset);

//             // }
//         }
//         // reset reload
//         ival = 2; amap["CheckAmBmsStatus"]->setVal(ival);
//     }

//     double tNow = am->vm->get_time_dbl();

//     bool BmsBypassStatus = amap["BmsBypassStatus"]->getbVal();
//     //bool MbmuBypassStatus = amap["BmsBypassStatus"]->getbVal();

//     toFault = amap["essBmsStatusTimeoutFault"]->getdVal();
//     toAlarm = amap["essBmsStatusTimeoutAlarm"]->getdVal();
//     toReset = amap["essBmsStatusTimeoutReset"]->getdVal();

//     int expectedBmsStatus = amap["BmsExpStatus"]->getiVal();
//     int currentBmsStatus = amap["BmsStatus"]->getiVal();
//     int lastBmsStatus = amap["BmsStatus"]->getiParam("lastBmsStatus");

//     char* currentBmsStatusString = amap["BmsStatusString"]->getcVal();
//     char* lastBmsStatusString =
//     amap["BmsStatusString"]->getcParam("lastStatusString"); char*
//     currentMbmuStatusString = amap["MbmuStatusString"]->getcVal(); char*
//     lastMbmuStatusString =
//     amap["MbmuStatusString"]->getcParam("lastStatusString"); char*
//     currentBmsStatusString2 = amap["BmsStatusString2"]->getcVal(); char*
//     lastBmsStatusString2 =
//     amap["BmsStatusString2"]->getcParam("lastBmsStatusString");

//     if (BmsBypassStatus)
//     {
//         ival = 1;
//         amap["essBmsStatusInit"]->addVal(ival);
//         return 0;

//     }
//     else
//     {
//         char* dest;
//         char* msg;
//         tNow = vm->get_time_dbl();

//         if (strcmp(currentBmsStatusString, lastBmsStatusString) != 0)
//         {
//             if (1) FPS_ERROR_PRINT(" %s >> BmsStatusString comp [%s:%s]
//             Changed from [%s] to [%s] at %2.6f\n"
//                 , __func__
//                 , amap["BmsStatusString"]->comp.c_str()
//                 , amap["BmsStatusString"]->name.c_str()
//                 , lastBmsStatusString
//                 , currentBmsStatusString
//                 , tNow
//             );

//             amap["BmsStatusString"]->setParam("lastStatusString",
//             currentBmsStatusString); if (strcmp(currentBmsStatusString,
//             "Warning status") == 0)
//             {
//                 asprintf(&dest, "/assets/%s/summary:alarms",
//                 am->name.c_str()); asprintf(&msg, "%s alarm  [%s] at %2.3f ",
//                 "Bms Status ", currentBmsStatusString, tNow);
//                 {
//                     vm->sendAlarm(vmap, amap["BmsStatusString"], dest,
//                     nullptr, msg, 2);
//                 }
//                 amap["essBmsStatusAlarms"]->addVal(1);

//                 //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
//                 if (1)FPS_ERROR_PRINT(" %s ALARM >>>>>> dest [%s] msg
//                 [%s]\n", __func__, dest, msg); if
//                 (dest)free((void*)dest); if (msg)free((void*)msg);
//             }
//             if (strcmp(currentBmsStatusString, "Fault status") == 0)
//             {
//                 asprintf(&dest, "/assets/%s/summary:faults",
//                 am->name.c_str()); asprintf(&msg, "%s fault  [%s] at %2.3f ",
//                 "Bms Status ", currentBmsStatusString, tNow);
//                 {
//                     vm->sendAlarm(vmap, amap["BmsStatusString"], dest,
//                     nullptr, msg, 2);
//                 }
//                 amap["essBmsStatusFaults"]->addVal(1);
//                 //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
//                 if (1)FPS_ERROR_PRINT(" %s FAULT >>>>>> dest [%s] msg
//                 [%s]\n", __func__, dest, msg); if
//                 (dest)free((void*)dest); if (msg)free((void*)msg);
//             }

//         }
//         if (0) FPS_ERROR_PRINT(" %s >> Testing MbmuStatusString  [%s] to [%s]
//         at %2.6f\n"
//             , __func__
//             , lastMbmuStatusString
//             , currentMbmuStatusString
//             , tNow
//         );

//         if (strcmp(currentMbmuStatusString, lastMbmuStatusString) != 0)
//         {
//             if (1) FPS_ERROR_PRINT(" %s >> MbmuStatusString Changed from [%s]
//             to [%s] at %2.6f\n"
//                 , __func__
//                 , lastMbmuStatusString
//                 , currentMbmuStatusString
//                 , tNow
//             );

//             amap["MbmuStatusString"]->setParam("lastStatusString",
//             currentMbmuStatusString); if (strcmp(currentMbmuStatusString,
//             "Warning") == 0)
//             {
//                 asprintf(&dest, "/assets/%s/summary:alarms",
//                 am->name.c_str()); asprintf(&msg, "%s alarm  [%s] at %2.3f ",
//                 "Mbmu Status ", currentMbmuStatusString, tNow);
//                 {
//                     vm->sendAlarm(vmap, amap["MbmuStatusString"], dest,
//                     nullptr, msg, 2);
//                 }
//                 amap["essBmsStatusAlarms"]->addVal(1);

//                 //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
//                 if (1)FPS_ERROR_PRINT(" %s ALARM >>>>>> dest [%s] msg
//                 [%s]\n", __func__, dest, msg); if
//                 (dest)free((void*)dest); if (msg)free((void*)msg);
//             }
//             if (strcmp(currentMbmuStatusString, "Fault") == 0)
//             {
//                 asprintf(&dest, "/assets/%s/summary:faults",
//                 am->name.c_str()); asprintf(&msg, "%s fault  [%s] at %2.3f ",
//                 "Mbmu Status ", currentMbmuStatusString, tNow);
//                 {
//                     vm->sendAlarm(vmap, amap["MbmuStatusString"], dest,
//                     nullptr, msg, 2);
//                 }

//                 amap["essBmsStatusFaults"]->addVal(1);

//                 //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
//                 if (1)FPS_ERROR_PRINT(" %s FAULT >>>>>> dest [%s] msg
//                 [%s]\n", __func__, dest, msg); if
//                 (dest)free((void*)dest); if (msg)free((void*)msg);
//             }

//         }

//         if (strcmp(currentBmsStatusString2, lastBmsStatusString2) != 0)
//         {
//             if (1) FPS_ERROR_PRINT(" %s >> BmsStatusString2 Changed from [%s]
//             to [%s] at %2.3f\n"
//                 , __func__
//                 , lastBmsStatusString2
//                 , currentBmsStatusString2
//                 , tNow
//             );

//             amap["BmsStatusString2"]->setParam("lastBmsStatusString",
//             currentBmsStatusString2);

//         }
//         // If we are in the init state wait for comms to start count down
//         reset time if (currentBmsStatus == initBmsStatus)
//         {
//             bool seenInit = amap["BmsStatus"]->getbParam("seenInit");

//             //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//             //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//             if (0)FPS_ERROR_PRINT("%s >> %s  NO BmsStatus,  bypass [%s]\n",
//             __func__, aname, BmsBypassStatus ? "true" :
//             "false");

//             // if not toally set up yet then quit this pass
//             if (!amap["amBmsStatusInit"])
//             {
//                 if (1)FPS_ERROR_PRINT("%s >> %s  no VAR amBmsStatusInit
//                 Yet...\n", __func__, aname);
//                 amap["amBmsStatusInit"] = vm->setLinkVal(vmap, aname,
//                 "/status", "amBmsStatusInit", ival); return 0;
//             }

//             if (!seenInit)   // BmsStatus_Setup
//             {
//                 if (1)FPS_ERROR_PRINT("%s >> %s  amBmsStatusInit  SEEN
//                 ...\n", __func__, aname); assetVar* toav
//                 = amap["BmsStatus"]->getaParam("ParamtoAlarm");  // Get an Av
//                 as a param if (1)FPS_ERROR_PRINT("%s >> %s  ParamtoAlarm %f
//                 \n", __func__, aname, toav->getdVal());

//                 amap["BmsStatus"]->setParam("seenInit", true);

//                 char* cval = (char*)"BmsStatus Init, no BmsStatus Seen";
//                 amap["BmsStatusState"]->setVal(cval);

//                 ival = 1;
//                 amap["essBmsStatusInit"]->addVal(ival);
//                 amap["BmsStatusInit"]->setVal(0);      //BmsStatus_Init
//             }
//             amap["BmsStatus"]->setParam("rdLast", tNow);

//         }
//         else  // wait for comms to go past reset then set active or wait to
//         alarm and then fault
//         {
//             //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to
//             [%s] \n", __func__, aname,
//             lastBmsStatus?lastBmsStatus:"no last Value", tval1); double
//             rdLast = amap["BmsStatus"]->getdParam("rdLast"); double rdFault =
//             amap["BmsStatus"]->getdParam("rdFault"); double rdAlarm =
//             amap["BmsStatus"]->getdParam("rdAlarm"); double rdReset =
//             amap["BmsStatus"]->getdParam("rdReset");
//             amap["BmsStatus"]->setParam("rdLast", tNow);

//             double toVal = amap["BmsStatus"]->getLastSetDiff(tNow);

//             // Has value changed ? If yes then count down rdReset to zero
//             based on tNow - rdLast if (currentBmsStatus != lastBmsStatus)
//                 //if(amap["BmsStatus"]->valueChangedReset())
//             {
//                 if (1)FPS_ERROR_PRINT("%s >> %s  amBmsStatus Changed from  %d
//                 to %d  (expected %d) at time %2.3f  SEEN ...\n"
//                     , __func__, aname, lastBmsStatus,
//                     currentBmsStatus, expectedBmsStatus, tNow);

//                 // TODO log the change
//                 amap["BmsStatus"]->setParam("lastBmsStatus",
//                 currentBmsStatus);
//             }

//             if (currentBmsStatus == expectedBmsStatus)
//             {
//                 bool seenOk = amap["BmsStatus"]->getbParam("seenOk");
//                 if (rdReset > 0.0)
//                 {
//                     rdReset -= (tNow - rdLast);
//                     amap["BmsStatus"]->setParam("rdReset", rdReset);
//                 }
//                 //else
//                 {
//                     // TODO after reset increment these up to toAlarm
//                     if (rdAlarm < toAlarm)
//                     {
//                         rdAlarm += tNow - rdLast;
//                         amap["BmsStatus"]->setParam("rdAlarm", rdAlarm);
//                     }
//                     if (rdFault < toFault)
//                     {
//                         rdFault += tNow - rdLast;
//                         amap["BmsStatus"]->setParam("rdFault", rdFault);
//                     }
//                 }

//                 if (0)FPS_ERROR_PRINT("%s >>  BmsStatus change for %s from
//                 [%d] to [%d]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f
//                 rdFault %2.3f\n"
//                     , __func__, aname, lastBmsStatus,
//                     currentBmsStatus, rdReset, (tNow - rdLast), rdAlarm,
//                     rdFault);

//                 ival = amap["BmsStatusStateNum"]->getiVal();
//                 // reset time passed , still changing , time to switch to
//                 BmsStatus_Ready if ((rdReset <= 0.0) && (ival != seenOk))
//                 {

//                     bool seenFault =
//                     amap["BmsStatus"]->getbParam("seenFault");
//                     //bool seenOk  = amap["BmsStatus"]->getbParam("seenOk");
//                     bool seenAlarm =
//                     amap["BmsStatus"]->getbParam("seenAlarm");
//                     amap["BmsStatus"]->setParam("seenOk", true);

//                     if (0)FPS_ERROR_PRINT("%s >>  BmsStatus  change for %s
//                     from [%d] to [%d] \n", __func__,
//                     aname, lastBmsStatus, currentBmsStatus); if (seenFault)
//                     {
//                         if (1)FPS_ERROR_PRINT("%s >>  BmsStatus fault for  %s
//                         cleared at %2.3f\n", __func__,
//                         aname, tNow);
//                         amap["BmsStatus"]->setParam("seenFault", false);

//                     }
//                     if (seenAlarm)
//                     {
//                         if (1)FPS_ERROR_PRINT("%s >>  BmsStatus Alarm for  %s
//                         cleared at %2.3f\n", __func__,
//                         aname, tNow);
//                         amap["BmsStatus"]->setParam("seenAlarm", false);

//                     }
//                     if (1)FPS_ERROR_PRINT("%s >>  BmsStatus OK for  %s at
//                     %2.3f\n", __func__, aname, tNow);
//                     ival = Asset_Ok; // seen BmsStatus change
//                     amap["BmsStatusStateNum"]->setVal(ival);
//                     ival = 0;
//                     amap["BmsStatusInit"]->setVal(ival);
//                     char* tval;
//                     asprintf(&tval, " BmsStatus OK last set %2.3f Alarm %3.2f
//                     max %3.2f", toVal, toAlarm, toFault); if (tval)
//                     {
//                         amap["BmsStatusState"]->setVal(tval);
//                         free((void*)tval);
//                     }
//                 }

//                 // increment alarm and fault time reset time
//                 if (rdFault < toFault)
//                 {
//                     rdFault += (tNow - rdLast);
//                     amap["BmsStatus"]->setParam("rdFault", rdFault);
//                 }
//                 if (rdAlarm < toAlarm)
//                 {
//                     rdAlarm += (tNow - rdLast);
//                     amap["BmsStatus"]->setParam("rdAlarm", rdAlarm);
//                 }

//                 //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to
//                 [%s] \n", __func__, aname,
//                 lastTs?lastTs:"no last Value", Ts);
//                 amap["BmsStatus"]->setParam("lastBmsStatus",
//                 currentBmsStatus);
//                 //if ((toVal > toFault)  && !bokFault && !bypass)

//             }
//             else   // No Change , start tracking faults and alarms
//             {
//                 bool seenFault = amap["BmsStatus"]->getbParam("seenFault");
//                 //bool seenOk  = amap["BmsStatus"]->getbParam("seenOk");
//                 bool seenAlarm = amap["BmsStatus"]->getbParam("seenAlarm");
//                 if (rdFault > 0.0)
//                 {
//                     rdFault -= (tNow - rdLast);
//                     amap["BmsStatus"]->setParam("rdFault", rdFault);
//                 }
//                 if (rdAlarm > 0.0)
//                 {
//                     rdAlarm -= (tNow - rdLast);
//                     amap["BmsStatus"]->setParam("rdAlarm", rdAlarm);
//                 }
//                 if (rdReset < toReset)
//                 {
//                     rdReset += (tNow - rdLast);
//                     amap["BmsStatus"]->setParam("rdReset", rdReset);
//                 }

//                 if ((rdFault <= 0.0) && !seenFault)
//                 {

//                     if (1)FPS_ERROR_PRINT("%s >>  BmsStatus  Fault  for %s at
//                     %2.3f \n", __func__, aname, tNow);
//                     char* tval; asprintf(&tval, " BmsStatus Fault last set
//                     %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
//                     if (tval)
//                     {
//                         amap["BmsStatusState"]->setVal(tval);
//                         free((void*)tval);
//                     }
//                     int ival = 1;
//                     amap["BmsStatusFaults"]->addVal(ival);
//                     amap["essBmsStatusFaults"]->addVal(ival);

//                     // if(am->am)
//                     // {
//                     //     amap["amBmsStatusFaults"]->addVal(ival);
//                     // }

//                     ival = Asset_Fault; //BmsStatus Fault
//                     amap["BmsStatusStateNum"]->setVal(ival);

//                     seenFault = true;
//                     amap["BmsStatus"]->setParam("seenFault", true);
//                     amap["BmsStatus"]->setParam("seenOk", false);
//                     amap["BmsStatus"]->setParam("seenAlarm", true);
//                     //seenOk = false;
//                     seenAlarm = false;

//                     int totalBmsStatusFaults =
//                     amap["BmsStatus"]->getiParam("totalStatusFaults");
//                     totalBmsStatusFaults++;
//                     amap["BmsStatus"]->setParam("totalStatusFaults",
//                     totalBmsStatusFaults);

//                 }
//                 else if ((rdAlarm <= 0.0) && !seenAlarm)
//                 {
//                     if (1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f
//                     \n", __func__, aname, tNow);

//                     char* tval;
//                     asprintf(&tval, "BmsStatus Alarm last set %2.3f Alarm
//                     %3.2f max %3.2f", toVal, toAlarm, toFault); if (tval)
//                     {
//                         amap["BmsStatusState"]->setVal(tval);
//                         free((void*)tval);
//                     }

//                     int ival = 1;
//                     amap["BmsStatusAlarms"]->addVal(ival);
//                     amap["essBmsStatusAlarms"]->addVal(ival);

//                     // if(am->am)
//                     // {
//                     //     amap["amBmsStatusAlarms"]->addVal(ival);
//                     // }
//                     ival = Asset_Alarm; //BmsStatus Alarm
//                     amap["BmsStatusStateNum"]->setVal(ival);

//                     amap["BmsStatus"]->setParam("seenAlarm", true);
//                     //amap["BmsStatus"]->setParam("seenFault", false);
//                     amap["BmsStatus"]->setParam("seenOk", false);
//                     int totalBmsStatusAlarms =
//                     amap["BmsStatus"]->getiParam("totalBmsStatusAlarms");
//                     totalBmsStatusAlarms++;
//                     amap["BmsStatus"]->setParam("totalBmsStatusAlarms",
//                     totalBmsStatusAlarms);
//                 }
//                 else
//                 {
//                     if (0)FPS_ERROR_PRINT("%s >> BmsStatus for [%s] [%s]
//                     Stalled at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
//                         , __func__
//                         , aname
//                         , amap["BmsStatus"]->getcVal()
//                         , tNow
//                         , rdReset, rdFault, rdAlarm);

//                 }
//             }
//         }
//     }
//     //
//     //int ival1, ival2;
//     //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n",
//     __func__, aname,
//     amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal()); return 0;
// };

// // BMS
// // ess_controller test asset status
// // logs any changes
// int CheckEssStatus(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset_manager* am)
// {
//     //double dval = 0.0;
//     int ival = 0;
//     //bool bval = false;
//     //int dval = 0.0;
//     char* cval = (char*)"Ess Status Init";
//     char* pcval = (char*)"Pcs Status Init";
//     char* bmval = (char*)"Bms Status Init";
//     VarMapUtils* vm = am->vm;
//     int reload = 0;
//     // this loads up the Faultors in the asset manager
//     reload = vm->CheckReload(vmap, amap, aname, "CheckEssStatus");
//     //assetVar* CheckAssetComms = amap["CheckAmComms"];
//     //double toAlarm = 2.5;
//     //double toFault = 10.0;
//     //double toReset = 2.5;
//     int initPcsStatus = -1;//(char *)" Initial PcsStatus";
//     int initBmsStatus = -1;//(char *)" Initial PcsStatus";
//     int initEssStatus = -1;//(char *)" Initial PcsStatus";

//     //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n",
//     __func__, aname, reload); if (reload < 2)
//     {
//         ival = 0;
//         //dval = 1.0;
//         //bool bval = false;
//         //Link This to an incoming component
//         if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n",
//         __func__, aname, reload);

//         amap["EssStatus"] = vm->setLinkVal(vmap, aname, "/status",
//         "EssStatus", initEssStatus); amap["PcsStatus"] = vm->setLinkVal(vmap,
//         aname, "/status", "PcsStatus", initPcsStatus); amap["BmsStatus"] =
//         vm->setLinkVal(vmap, aname, "/status", "BmssStatus", initPcsStatus);

//         amap["PcsStatusState"] = vm->setLinkVal(vmap, aname, "/status",
//         "PcsStatusState", pcval); amap["BmsStatusState"] =
//         vm->setLinkVal(vmap, aname, "/status", "BmsStatusState", bmval);

//         amap["essPcsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status",
//         "essPcsStatusFaults", ival); amap["essPcsStatusAlarms"] =
//         vm->setLinkVal(vmap, aname, "/status", "essPcsStatusAlarms", ival);
//         amap["essPcsStatusInit"] = vm->setLinkVal(vmap, aname, "/status",
//         "essPcsStatusAlarms", ival); amap["essPcsStatusState"] =
//         vm->setLinkVal(vmap, aname, "/status", "essPcsStatusState", cval);
//         amap["essBmsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status",
//         "essBmsStatusFaults", ival); amap["essBmsStatusAlarms"] =
//         vm->setLinkVal(vmap, aname, "/status", "essBmsStatusAlarms", ival);
//         amap["essBmsStatusInit"] = vm->setLinkVal(vmap, aname, "/status",
//         "essBmsStatusAlarms", ival); amap["essBmsStatusState"] =
//         vm->setLinkVal(vmap, aname, "/status", "essBmsStatusState", cval);

//         if (reload == 0) // complete restart
//         {
//             amap["PcsStatus"]->setVal(initPcsStatus);
//             amap["BmsStatus"]->setVal(initBmsStatus);
//             //lastPcsStatus=strdup(tsInit);//state"]->setVal(cval);
//             amap["PcsStatus"]->setParam("lastPcsStatus", initPcsStatus);
//             amap["PcsStatus"]->setParam("totalPcsStatusFaults", 0);
//             amap["PcsStatus"]->setParam("totalPcsStatusAlarms", 0);

//             amap["BmsStatus"]->setParam("lastBmsStatus", initBmsStatus);
//             amap["BmsStatus"]->setParam("totalBmsStatusFaults", 0);
//             amap["BmsStatus"]->setParam("totalBmsStatusAlarms", 0);

//             amap["PcsStatusState"]->setVal(pcval);
//             amap["BmsStatusState"]->setVal(bmval);

//         }
//         // reset reload
//         ival = 2; amap["CheckEssStatus"]->setVal(ival);
//     }

//     //double tNow = am->vm->get_time_dbl();

//     // are we the ess_controller
//     if (!am->am)
//     {
//         //bool initSeen =             amap["PcsStatus"]
//         ->getbParam("initSeen");

//         amap["essPcsStatusFaults"]->setVal(0);
//         amap["essPcsStatusAlarms"]->setVal(0);
//         amap["essPcsStatusInit"]->setVal(0);
//         amap["essBmsStatusFaults"]->setVal(0);
//         amap["essBmsStatusAlarms"]->setVal(0);
//         amap["essBmsStatusInit"]->setVal(0);
//         int icnt = 0;
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager* amc = ix.second;
//             if (amc->name == "pcs")
//             {
//                 CheckAmPcsStatus(vmap, amc->amap, amc->name.c_str(), p_fims,
//                 amc); icnt++;
//             }
//             else if (amc->name == "bms")
//             {
//                 CheckAmBmsStatus(vmap, amc->amap, amc->name.c_str(), p_fims,
//                 amc); icnt++;
//             }

//             int essPcsStatusFaults = amap["essPcsStatusFaults"]->getiVal();
//             int essPcsStatusAlarms = amap["essPcsStatusAlarms"]->getiVal();
//             //int essPcsStatusInit = amap["essPcsStatusInit"]->getiVal();
//             if (essPcsStatusFaults > 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essPcsStatusFaults detected\n",
//                 __func__, essPcsStatusFaults);
//             }
//             if (essPcsStatusAlarms > 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essPcsStatusAlarmss detected\n",
//                 __func__, essPcsStatusAlarms);
//             }
//             int essBmsStatusFaults = amap["essBmsStatusFaults"]->getiVal();
//             int essBmsStatusAlarms = amap["essBmsStatusAlarms"]->getiVal();
//             //int essBmsStatusInit = amap["essBmsStatusInit"]->getiVal();
//             if (essBmsStatusFaults > 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essBmsStatusFaults detected\n",
//                 __func__, essBmsStatusFaults);
//             }
//             if (essBmsStatusAlarms > 0)
//             {
//                 FPS_ERROR_PRINT("%s >> %d essBmsStatusAlarms detected\n",
//                 __func__, essBmsStatusAlarms);
//             }
//         }
//         // TODO do things like shutdown on faults

//     }
//     return 0;
// };
int CheckAssetDisable(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
{
    // this will filter up the comms stats to the manager
    int ival = 0;
    // bool bval = true;
    double tNow = am->vm->get_time_dbl();
    double dval = 0.0;

    if (!amap["CheckAssetDisable"] || amap["CheckAssetDisable"]->getiVal() < 2)
    {
        // char * cval = (char *)"HeartBeat Init";
        // setAmapAi(am,  amap,          HeartBeatState,
        // am->name.c_str(),      /status,       cval);  int ival = 0;  bool bval =
        // false;
        setAmapAi(am, amap, CheckAssetDisable, am->name.c_str(), / reload, ival);
        setAmapAi(am, amap, DisableCmd, am->name.c_str(), / controls, ival);
        setAmapAi(am, amap, EnableCmd, am->name.c_str(), / controls, ival);
        setAmapAi(am, amap, EnableCnt, am->name.c_str(), / status, ival);
        setAmapAi(am, amap, DisableCnt, am->name.c_str(), / status, ival);
        setAmapAi(am, amap, Enabledxx, am->name.c_str(), / controls, ival);
        setAmapAi(am, amap, CheckAssetInit, am->name.c_str(), / status, tNow);
        setAmapAi(am, amap, CheckAssetCmdRun, am->name.c_str(), / status, tNow);
        setAmapAi(am, amap, CheckAssetCmdRuns, am->name.c_str(), / status, dval);
        setAmapAi(am, amap, DisableCmdRun, am->name.c_str(), / status, tNow);
        setAmapAi(am, amap, EnableCmdRun, am->name.c_str(), / status, tNow);
        ival = 2;
        amap["CheckAssetDisable"]->setVal(ival);
    }
    amap["CheckAssetCmdRun"]->setVal(tNow);
    dval = amap["CheckAssetCmdRuns"]->getdVal();
    dval++;
    amap["CheckAssetCmdRuns"]->setVal(dval);

    // bool bval2;
    ival = amap["Enabledxx"]->getiVal();
    //    double tNow = am->vm->get_time_dbl();
    int ivalcmd = 0;
    if (ival == 0)
    {
        ivalcmd = amap["EnableCmd"]->getiVal();
        if (ivalcmd > 0)
        {
            // bval =  true;
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
        ivalcmd = amap["DisableCmd"]->getiVal();
        if (ivalcmd > 0)
        {
            // bval =  false;
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

int CheckAMHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // this will filter up the comms stats to the manager
    if (amap["HeartBeatErrors"])
    {
        int ival = 0;
        ival = amap["HeartBeatErrors"]->getiVal();
        if (amap["HeartBeatState"])
        {
            if (ival > 0)
            {
                char* cval = (char*)"HeartBeat Errors Detected";
                amap["HeartBeatState"]->setVal(cval);
            }
            else
            {
                char* cval = (char*)"HeartBeat OK";
                amap["HeartBeatState"]->setVal(cval);
            }
        }
    }
    return 0;
}

int InitAMComms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // this will filter up the comms stats to the manager

    if (amap["CommsErrors"])
    {
        int ival = 0;
        amap["CommsErrors"]->setVal(ival);
    }
    else
    {
        char* cval = (char*)"Comms Init";
        setAmapAi(am, amap, CommsState, am->name.c_str(), / status, cval);
        int ival = 0;
        setAmapAi(am, amap, CommsErrors, aname, / status, ival);
        setAmapAi(am, amap, AllCommsErrors, "ess" /*am->name.c_str()*/, / status, ival);
    }
    return 0;
}

int CheckAMComms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // this will filter up the comms stats to the manager
    if (amap["CommsErrors"])
    {
        int ival = 0;
        ival = amap["CommsErrors"]->getiVal();
        if (amap["AllCommsErrors"])
            amap["AllCommsErrors"]->addVal(ival);
        if (amap["CommsState"])
        {
            if (ival > 0)
            {
                char* cval = (char*)"Comms Errors Detected";
                amap["CommsState"]->setVal(cval);
            }
            else
            {
                char* cval = (char*)"Comms OK";
                amap["CommsState"]->setVal(cval);
            }
        }
    }
    return 0;
}

// VarMapUtils *CheckAssetHeartBeatInit(const char *fname, varsmap &vmap, varmap
// &amap, const char* aname, fims* p_fims, asset *am)
// {
//     int reload= -1;
//     double dval = 0.0;
//     int ival;
//     char * cval;
//     char * tsVal = (char *)"ThereIsNoTimestamp";
//     av_ptr CheckAssetHeartBeat      = amap[fname];
//     VarMapUtils* vmp = nullptr;

//     if(am)
//     {
//         vmp = am->vm;
//     }

//     // FPS_ERROR_PRINT("%s >> %s  Found  CheckAssetComs %p amap %p am->amap
//     %p\n"
//     //      ,__func__, aname
//     //      , (void*)CheckAssetComms
//     //      , (void*)&amap
//     //      , (void*)&am->amap
//     //      );
//     double tNow = vmp->get_time_dbl();

//     if (CheckAssetHeartBeat)
//     {

//         reload = CheckAssetHeartBeat->getiVal();
//         if(0)FPS_ERROR_PRINT("%s >> OK Found reload
//         %d\n",__func__, reload);
//     }
//     else
//     {
//         FPS_ERROR_PRINT("%s >> could not find %s
//         \n",__func__, fname); reload = 0;  // complete
//         reset  reload = 1 for remap ( links may have changed)

//         FPS_ERROR_PRINT("%s >> %s  Forcing reload \n",
//         __func__, aname);
//     }

//     if(reload < 2)
//     {
//         if(reload < 1)
//         {
//             FPS_ERROR_PRINT("%s >> %s  Running RESET %d \n",
//             __func__, aname, reload);
//         }
//         else
//         {
//             FPS_ERROR_PRINT("%s >> %s  Running RELOAD %d \n",
//             __func__, aname, reload);
//         }

//         //reload = 0;
//         double toval = 3.5;
//         int ival = -1;
//         bool bval = true;
//         setAmapAi(am,  amap,          CheckAssetHeartBeat,
//         am->name.c_str(),      /reload,       reload); setAmapAi(am,  amap,
//         CheckAssetHeartBeatPeriod,  am->name.c_str(),      /config, dval);
//         setAmapAi(am,  amap,          StatusAssetHeartBeat,
//         am->name.c_str(),      /status,       ival); setAmapAi(am,  amap,
//         TestHeartBeat,              am->name.c_str(),      /config, dval);
//         setAmapAi(am,  amap,          Timestamp,
//         am->name.c_str(),      /components,   cval); setAmapAi(am,  amap,
//         HeartBeatState,             am->name.c_str(),      /status, cval);
//         setAmapAi(am,  amap,          AmHeartBeatState,
//         am->am->name.c_str(),      /status,       cval); setAmapAi(am,  amap,
//         maxHeartBeatTimeout,    am->am->name.c_str(),      /config, toval);
//         setAmapAi(am,  amap,          HeartBeatErrors,
//         am->am->name.c_str(),      /status,       ival); setAmapAi(am,  amap,
//         lastTimestamp,             am->name.c_str(),       /config, cval);
//         setAmapAi(am,  amap,          AssetHeartBeat,
//         am->name.c_str(),       /components,   ival); setAmapAi(am,  amap,
//         lastAssetHeartBeat,        am->name.c_str(),       /status, ival);
//         setAmapAi(am,  amap,          Enabled,
//         am->name.c_str(),       /status,       bval);
//         //reload = 0;
//         if(reload == 0) // complete restart
//         {
//             ival = 0; amap["StatusAssetHeartBeat"]->setVal(ival);
//             //ival = 2; amap["CheckAssetComs"]->setVal(ival);
//             //cval = "ThereIsNoTimeStamp";
//             amap["Timestamp"]->setVal(tsVal);
//             cval = (char *) "HeartBeat Init";
//             amap["HeartBeatState"]->setVal(cval);
//             amap["AmHeartBeatState"]->setVal(cval);
//             amap["CheckAssetHeartBeatPeriod"]->setVal(tNow);
//             dval =  3.5;
//             amap["maxHeartBeatTimeout"]->setVal(dval);
//         }
//         ival = 2; amap[fname]->setVal(ival);

//     }
//     CheckAssetHeartBeat      = amap[fname];
//     reload = CheckAssetHeartBeat->getiVal();
//     if(0)FPS_ERROR_PRINT("%s >> OK Test Again  >> reload
//     %d\n",__func__, reload);
//     // create a function that just returns if the time has not expired
//     // TODO am->vm->every(am->name,c_str(), 100);
//     // defAivar(amap, TestComs, ival);
//     // int cnt = TestComs->getiVal();
//     // TestComs->setVal(cnt+1);
//     return vmp;
// }

// int CheckAssetHeartBeat(varsmap &vmap, varmap &amap, const char* aname, fims*
// p_fims, asset *am)
// {
//     //char * cval;
//     //VarMapUtils * vmp = CheckAssetCommsInit(__func__,
//     vmap, amap, aname, p_fims, am); VarMapUtils* vmp =
//     CheckAssetHeartBeatInit(__func__, vmap, amap, aname,
//     p_fims, am);

//     double tNow = vmp->get_time_dbl();
//     //char * tsVal = (char *)"ThereIsNoTimestamp";

//     int aHB= amap["AssetHeartBeat"]->getiVal();
//     int lastaHB = amap["lastAssetHeartBeat"]->getiVal();
//     if (aHB == -1)
//     {
//         //ival = 1; amap["CheckAssetComs"]->setVal(ival);
//         //if(0)FPS_ERROR_PRINT("%s >> %s  NO time stamp yet lastTs %p \n",
//         __func__, aname, (void *)lastTs); char* cval =
//         (char *)"HeartBeat Init"; amap["HeartBeatState"]->setVal(cval);
//     }
//     else
//     {
//         //if(0)FPS_ERROR_PRINT("%s >> %s  Checking TimeStamp  Ts [%s] lastTs
//         %s \n", __func__, aname, Ts?Ts:"no
//         VALUE",lastTs?lastTs:"no last VALUE");

//         if (lastaHB != aHB)
//         {
//             double dv  =  amap["TestHeartBeat"]->getdVal();
//             amap["TestHeartBeat"]->setVal(dv + 1.0);

//             //if(0)FPS_ERROR_PRINT("%s >>  we found a changed time stamp for
//             %s [%s]\n", __func__, aname, Ts);
//             amap["lastAssetHeartBeat"]->setVal(aHB);
//             char* cval = (char *)"HeartBeat OK";
//             amap["HeartBeatState"]->setVal(cval);

//         }
//         else
//         {
//             double toval;

//             if ( amap["AssetHeartBeat"]->getLastSetDiff(tNow) >
//             amap["maxHeartBeatTimeout"]->getdVal())
//             {
//                 //amap["lastTimestamp"]->setVal(Ts);

//                 char* cval = (char *)"HeartBeat Time Out";
//                 amap["HeartBeatState"]->setVal(cval);
//                 bool bval = false;

//                 if(amap["Enabled"]->getbVal())
//                 {
//                     int ival = 1 ; amap["HeartBeatErrors"]->addVal(ival);
//                 }

//             }
//         }
//     }

//     return 0;
// }

// VarMapUtils *CheckHeartBeatInit(const char *fname, varsmap &vmap, varmap
// &amap, const char* aname, fims* p_fims, asset_manager *am)
// {
//     int reload= -1;
//     double dval = 0.0;
//     int ival;
//     char * cval;
//     char * tsVal = (char *)"ThereIsNoTimestamp";
//     av_ptr CheckHeartBeat   = amap[fname];
//     VarMapUtils* vmp = am->vm;

//     if(am)
//     {
//         vmp = am->vm;
//     }

//     // FPS_ERROR_PRINT("%s >> %s  Found  CheckAssetComs %p amap %p am->amap
//     %p\n"
//     //      ,__func__, aname
//     //      , (void*)CheckAssetComms
//     //      , (void*)&amap
//     //      , (void*)&am->amap
//     //      );
//     double tNow = vmp->get_time_dbl();

//     if (CheckHeartBeat)
//     {

//         reload = CheckHeartBeat->getiVal();
//         if(0)FPS_ERROR_PRINT("%s >> OK Found CheckComms >> (reload
//         %d)\n",__func__, reload);
//     }
//     else
//     {
//         FPS_ERROR_PRINT("%s >> could not find CheckHeartBeat
//         \n",__func__); reload = 0;  // complete reset
//         reload = 1 for remap ( links may have changed)

//         FPS_ERROR_PRINT("%s >> %s  Forcing reload \n",
//         __func__, aname);
//     }

//     if(reload < 2)
//     {
//         if(reload < 1)
//         {
//             FPS_ERROR_PRINT("%s >> %s  Running RESET %d \n",
//             __func__, aname, reload);
//         }
//         else
//         {
//             FPS_ERROR_PRINT("%s >> %s  Running RELOAD %d \n",
//             __func__, aname, reload);
//         }

//         //reload = 0;
//         double toval = 3.5;
//         double pper = 0.250;
//         setAmapAi(am,  amap,          CheckHeartBeat,       am->name.c_str(),
//         /reload,       reload); setAmapAi(am,  amap,
//         CheckHeartBeatPeriod, am->name.c_str(),      /config,       pper);
//         setAmapAi(am,  amap,          CheckHeartBeatRun,    am->name.c_str(),
//         /status,       dval); setAmapAi(am,  amap,          StatusHeartBeat,
//         am->name.c_str(),      /status,       ival); setAmapAi(am,  amap,
//         TestHeartBeat,            am->name.c_str(),  /config,       dval);
//         setAmapAi(am,  amap,          Timestamp,            am->name.c_str(),
//         /components,   cval); setAmapAi(am,  amap,          HeartBeatState,
//         am->name.c_str(),      /status,      cval);
//         // setAmapAi(am,  amap,          bmsCommsState,           "bms",
//         /status,       cval); setAmapAi(am,  amap,
//         maxHeartBeatTimeout,      am->name.c_str(),             /config,
//         toval);
//         //setAmapAi(am,  amap,          lastTimestamp,
//         am->name.c_str(),      /config,       cval);
//         //reload = 0;
//         if(reload == 0) // complete restart
//         {
//             ival = 0; amap["StatusHeartBeat"]->setVal(ival);
//             //ival = 2; amap["CheckAssetComs"]->setVal(ival);
//             //cval = "ThereIsNoTimeStamp";
//             amap["Timestamp"]->setVal(tsVal);
//             cval = (char *) "HeartBeat Init";
//             amap["HeartBeatState"]->setVal(cval);
//             // amap["bmsCommsState"]->setVal(cval);
//             // amap["CheckAssetCommsPeriod"]->setVal(tNow);
//             dval =  3.5;
//             amap["maxHeartBeatTimeout"]->setVal(dval);
//             amap["CheckHeartBeatRun"]->setVal(tNow);
//         }
//         ival = 2; amap[fname]->setVal(ival);

//     }
//     CheckHeartBeat      = amap[fname];
//     reload = CheckHeartBeat->getiVal();
//     if(0)FPS_ERROR_PRINT("%s >> OK Test Again >> reload
//     %d\n",__func__, reload);
//     // create a function that just returns if the time has not expired
//     // TODO am->vm->every(am->name,c_str(), 100);
//     // defAivar(amap, TestComs, ival);
//     // int cnt = TestComs->getiVal();
//     // TestComs->setVal(cnt+1);
//     return vmp;
// }

// // all subsystems must return a heartbeat
// // allow a system Disable command
// int CheckHeartBeat(varsmap &vmap, varmap &amap, const char* aname, fims*
// p_fims, asset_manager *am)
// {
//     //char * cval;
//     VarMapUtils* vmp = CheckHeartBeatInit(__func__,
//     vmap, amap, aname, p_fims, am);

//     double tNow = vmp->get_time_dbl();
//     //char * tsVal = (char *)"ThereIsNoTimestamp";
//     double pval = amap["CheckHeartBeatRun"]->getLastSetDiff(tNow);
//     double plim = amap["CheckHeartBeatPeriod"]->getdVal();

//     if(0)FPS_ERROR_PRINT("%s >> OK Test Again   >> pval: %2.3f plim: %2.3f
//     \n",__func__, pval, plim); if (pval > plim)
//     {
//         if(0)FPS_ERROR_PRINT("%s >> OK Running  >> pval: %2.3f plim: %2.3f
//         \n",__func__, pval, plim);
//         amap["CheckHeartBeatRun"]->setVal(tNow);

//         if (am)
//         {

//             cascadeAM(vmap, amap, aname,p_fims,am, nullptr, CheckAssetDisable
//             );

//             cascadeAM(vmap, amap, aname,p_fims,am, InitAMHeartBeat, nullptr);

//             // collect comms status for all assets
//             cascadeAM(vmap, amap, aname,p_fims,am, nullptr,
//             CheckAssetHeartBeat);
//             // collect comms status for all managers
//             cascadeAM(vmap, amap, aname,p_fims,am, CheckAMHeartBeat,
//             nullptr);
//         }
//         // TODO prepare coms status for the whole system.
//         // TODO decide to continue or fault.
//     }
//     return 0;
// }

int xHandleHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
{
    int reload;
    double dval = 0.0;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleHeartBeat = amap[__func__];

    if (reload < 2)
    {
        // reload = 0;
        amap["HandleHeartBeat"] = vm->setLinkVal(vmap, aname, "/config", "HandleHeartBeat", reload);
        amap["HeartBeat"] = vm->setLinkVal(vmap, aname, "/status", "HeartBeat", dval);
        amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if (reload == 0)                     // complete restart
        {
            amap["HeartBeat"]->setVal(0);
        }
        HandleHeartBeat->setVal(2);
    }
    // get the reference to the variable
    assetVar* hb = amap["HeartBeat"];
    // double ival;
    dval = hb->getdVal();
    dval++;
    if (dval > 255)
        dval = 0;
    if (1)
        printf("HeartBeat %s val %f ", aname, dval);

    hb->setVal(dval);
    dval = hb->getdVal();
    if (1)
        printf("HeartBeat val after set %f\n", dval);

    vm->sendAssetVar(hb, p_fims);
    return dval;
}
// int HandleHeartBeat(varsmap &vmap, varmap &amap, fims* p_fims)
// {
//     // get the reference to the variable
//     assetVar* hb = amap["HeartBeat"];
//     int ival;
//     hb->getiVal();
//     ival++;
//     if(ival > 255) ival = 0;

//     hb->setVal(ival);
//     vm->sendAssetVar(hb, p_fims);
//     return ival;
// }

// This is the pcs handle power function.

//#include "../src/pcs_functions.cpp"
// we have to make sure BMS has done its job first so run this at LEVEL3
int HandlePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    double dval;
    static double Pmax, Pset_dbl, Pcmd_dbl;
    int rc = 0;
    static int first = 1;
    VarMapUtils* vm = am->vm;

    if (first == 1)
    {
        amap["ActivePowerSetpoint"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerSetpoint", dval);
        amap["ActivePowerDeadband"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDeadband", dval);
        amap["maxChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargeCurrent", dval);
        amap["maxDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargeCurrent", dval);
        amap["ActivePowerCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerCmd", dval);
        amap["Vdc"] = vm->setLinkVal(vmap, aname, "/status", "pcs_vdc_bus_1", dval);
        first = 0;
    }
    assetVar* Pset = amap["ActivePowerSetpoint"];
    assetVar* Pdb = amap["ActivePowerDeadband"];
    assetVar* ImaxC = amap["maxChargeCurrent"];
    assetVar* ImaxD = amap["maxDischargeCurrent"];
    assetVar* Pcmd = amap["ActivePowerCmd"];
    assetVar* Vdc = amap["Vdc"];

    Pdb->setDbVal(Pdb->getdVal());
    Pset_dbl = Pset->getdVal();

    if (Pset->valueChanged(dval))
    {
        printf("Power limiting now\n");
        if (Pset_dbl > 0)
        {
            Pmax = ImaxD->getdVal() * Vdc->getdVal();
            Pcmd_dbl = Pset_dbl > Pmax ? Pmax : Pset_dbl;
        }
        else
        {
            Pmax = ImaxC->getdVal() * Vdc->getdVal();
            Pcmd_dbl = abs(Pset_dbl) > Pmax ? Pmax * -1 : Pset_dbl;
        }
        Pcmd->setVal(Pcmd_dbl);
        rc++;
    }

    if (0)
        std::cout << "Pset: " << Pset_dbl << " Pmax: " << Pmax << " Pcmd: " << Pcmd_dbl << std::endl;
    return rc;
}

int HandleEMSChargeL1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fim, asset_manager* am)
{
    double dval;
    // static double Pmax, Pset_dbl, Pcmd_dbl;
    int rc = 0;
    VarMapUtils* vm = am->vm;
    int reload = vm->CheckReload(vmap, amap, aname, __func__);

    if (reload < 2)
    {
        // amap["ActivePowerSetpoint"]     = vm->setLinkVal(vmap, aname,
        // "/controls", "ActivePowerSetpoint", dval);  amap["ActivePowerDeadband"]
        // = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDeadband", dval);
        amap["maxChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargeCurrent", dval);
        amap["maxDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargeCurrent", dval);
        amap["totChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totChargeCurrent", dval);
        amap["totDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totDischargeCurrent", dval);
        amap["numBMS"] = vm->setLinkVal(vmap, "bms", "/status", "nmuBMS", dval);

        // amap["ActivePowerCmd"]          = vm->setLinkVal(vmap, aname,
        // "/controls", "ActivePowerCmd", dval);  amap["Vdc"]                     =
        // vm->setLinkVal(vmap, aname, "/status", "pcs_vdc_bus_1", dval);
        amap[__func__]->setVal(2);
    }
    // assetVar * Pset = amap["ActivePowerSetpoint"];
    // assetVar * Pdb = amap["ActivePowerDeadband"];
    assetVar* ImaxC = amap["maxChargeCurrent"];
    assetVar* ImaxD = amap["maxDischargeCurrent"];
    assetVar* totMaxC = amap["totChargeCurrent"];
    assetVar* totMaxD = amap["totDischargeCurrent"];
    assetVar* numBMS = amap["numBMS"];

    // assetVar * Pcmd = amap["ActivePowerCmd"];
    // assetVar * Vdc = amap["Vdc"];

    ImaxC->setVal(0.0);
    ImaxD->setVal(0.0);
    totMaxC->setVal(0.0);
    totMaxD->setVal(0.0);
    numBMS->setVal(0.0);
    return rc;
}

// run by each BMS asset

int HandleBMSChargeL2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am)
{
    double dval = 0.0;
    // double dval2 = 0.0;
    // static double Pmax, Pset_dbl, Pcmd_dbl;
    int rc = 0;
    int reload;
    VarMapUtils* vm = am->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);

    assetVar* HandleBMSChargeL2 = amap["HandleBMSChargeL2"];
    if (reload < 2)
    {
        amap["HandleBMSChargeL2"] = vm->setLinkVal(vmap, aname, "/status", "HandleBMSChargeL2", reload);
        amap["bmsMaxChargeCurrent"] = vm->setLinkVal(vmap, aname, "/status", "bmsMaxChargeCurrent", dval);
        amap["bmsMaxDischargeCurrent"] = vm->setLinkVal(vmap, aname, "/status", "bmsMaxDischargeCurrent", dval);
        amap["maxChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargeCurrent", dval);
        amap["maxDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargeCurrent", dval);
        amap["totChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totChargeCurrent", dval);
        amap["totDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totDischargeCurrent", dval);
        amap["numBMS"] = vm->setLinkVal(vmap, "bms", "/status", "nmuBMS", dval);
        // amap["ActivePowerCmd"]          = vm->setLinkVal(vmap, aname,
        // "/controls", "ActivePowerCmd", dval);  amap["Vdc"]                     =
        // vm->setLinkVal(vmap, aname, "/status", "pcs_vdc_bus_1", dval);
        if (reload == 0)  // complete restart
        {
            // amap["HeartBeat"]->setVal(0);
            amap["HandleBMSChargeL2"]->setVal(2);
        }
        HandleBMSChargeL2->setVal(2);
    }
    // assetVar * Pset = amap["ActivePowerSetpoint"];
    // assetVar * Pdb = amap["ActivePowerDeadband"];
    assetVar* ImaxC = amap["maxChargeCurrent"];
    assetVar* ImaxD = amap["maxDischargeCurrent"];
    assetVar* bmsMaxC = amap["bmsMaxChargeCurrent"];
    assetVar* bmsMaxD = amap["bmsMaxDischargeCurrent"];
    assetVar* totMaxC = amap["totChargeCurrent"];
    assetVar* totMaxD = amap["totDischargeCurrent"];
    assetVar* numBMS = amap["numBMS"];
    // assetVar * Pcmd = amap["ActivePowerCmd"];
    // assetVar * Vdc = amap["Vdc"];

    if ((dval = bmsMaxC->getdVal()) > ImaxC->getdVal())
        ImaxC->setVal(dval);
    totMaxC->addVal(dval);

    if ((dval = bmsMaxD->getdVal()) > ImaxD->getdVal())
        ImaxD->setVal(dval);
    totMaxD->addVal(dval);

    dval = 1.0;
    numBMS->addVal(dval);

    return rc;
}

//
// TODO add timeout

// Handle power Generation state between off and gridfollowing
// the system will use text states
//  Off, GridFollowing, Fault  .. etc
int oldHandlePower(varsmap& vmap, varmap& amap, fims* p_fims)
{
    // double dval;
    // char* sval;
    int rc = 0;
    // assetVar* PGState =
    //    amap["PowerGenerationState"];
    // assetVar* PGStateCmd =
    //     amap["PowerGenerationStateCmd"];
    // assetVar* lastPGStateCmd = amap["lastPowerGenerationStateCmd"];
    // assetVar* PCState = amap["PCSState"];
    // assetVar* PCSFault = amap["PCSFault"];// TODO amapGet("PCSFault");
    // assetVar* PCStateCmd = amap["PCSStateCmd"];

    //}
    return rc;
}

//     amap["lastActivePowerSetpoint"]->setVal(
//     amap["ActivePowerSetpoint"]->getdVal()); if
//     (abs(amap["ActivePowerSetpoint"]->getdVal()) <
//     amap["maxActivePower"]->getdVal())
//     {
//         amap["ActivePowerCmd"]->setVal(amap["ActivePowerSetpoint"]->getdVal());
//     }
//     else
//     {
//         amap["ActivePowerCmd"]->setVal((amap["ActivePowerSetpoint"]->getdVal()>0.0)?amap["maxActivePower"]->getdVal()
//                             :-amap["maxActivePower"]->getdVal());
//     }
//     vm->sendAssetVar(amap["ActivePowerCmd"], p_fims);
//     rc++;
// }

// // another way to do this using local vars , prevents the dict lookup
// assetVar * RPSP = amap["ReactivePowerSetpoint"];
// assetVar * lastRPSP = amap["lastReactivePowerSetpoint"];
// assetVar * RPDB = amap["ReactivePowerDeadband"];
// assetVar * maxRP = amap["maxReactivePower"];
// assetVar * RPcmd = amap["ReactivePowerCmd"];

// if ( vm->valueChanged(RPSP, lastRPSP, RPDB ,dval, 2.0))
// {
//     lastRPSP->setVal(  RPSP->getdVal());
//     if (abs(RPSP->getdVal()) < maxRP->getdVal())
//     {
//         RPcmd->setVal(RPSP->getdVal());
//     }
//     else
//     {
//         RPcmd->setVal((RPSP->getdVal()>0.0)?maxRP->getdVal()
//                                                     :-maxRP->getdVal());
//     }
//     vm->sendAssetVar(RPcmd, p_fims);
//     rc++;
// }

//     return rc;
// }