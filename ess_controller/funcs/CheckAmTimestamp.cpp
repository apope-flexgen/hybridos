#ifndef CHECKAMTIMESTAMP_CPP
#define CHECKAMTIMESTAMP_CPP
#include "asset.h"

// CheckAmTimestamp

/*
 * the timestamp is part of the pub message
 *    After init we must get a continual Timestamp changes otherwise we alarm
 and then fault.
 *    the bms Timestamp arrives on /components/catl_ems_bms_01_rw:ems_heartbeat,
 *    linked to /status/bms/Heartbeat  in bms_manager.json
 * Lets try again
 *    base it off HBSeenTime
 * at start HBSeenTime = 0.0 HBOk = false seenHB = false
 * if we see a change and !seenHB then set HBseenTime and set seenHBS
 * if HBseenTime == 0 we never have seen a HB  dont set faults or alarms yet
 * if seenHB and (tNow - HBSeenTime) > toHold reset seenHB
 * if seenHB and rdReset <=0.0  then set HBok clear errors else decrement
 rdReset
 * if HBOk inc rdAlarm and rdFault to ther max
 * if !seenHB  and tNow - HBseenTime > rdAlarm then set Alarm
 * if !seenHB  and tNow - HBseenTime > rdFault then set Fault

 * toHold time to allow between Timestamp changes before worrying about it
 * toAlarm time after a stalled Heatbeat causes an Alarm
 * toFault time after a stalled Heatbeat causes a Fault
 *  toReset time after changes start being seen again before resetting faults
 and Alarms
 *
 */
int CheckAmTimestamp(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval = (char*)"Timestamp Init";
    VarMapUtils* vm = am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAmTimestamp");
    // assetVar* CheckAssetComms = amap["CheckAmComms"];
    double toHold = 1.5;  // Seconds between TS changes
    double toAlarm = 2.5;
    double toFault = 6.0;
    double toReset = 2.5;
    char* initTimestamp = (char*)" Initial Timestamp";
    char* essName = vm->getSysName(vmap);

    // if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n",
    // __func__, aname, reload);
    if (reload < 2)
    {
        ival = 0;
        // dval = 1.0;
        // bool bval = false;
        // Link This to an incoming component
        if (1)
            FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);

        amap["Timestamp"] = vm->setLinkVal(vmap, aname, "/status", "Timestamp", initTimestamp);
        if (1)
            FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n", __func__, aname,
                            amap["Timestamp"]->comp.c_str(), amap["Timestamp"]->name.c_str());

        amap["essTsFaults"] = vm->setLinkVal(vmap, essName, "/status", "essTsFaults", ival);
        amap["essTsAlarms"] = vm->setLinkVal(vmap, essName, "/status", "essTsAlarms", ival);
        amap["essTsInit"] = vm->setLinkVal(vmap, essName, "/status", "essTsInit", ival);
        amap["essTsTimeoutFault"] = vm->setLinkVal(vmap, essName, "/config", "essTsTimeoutFault", toFault);
        amap["essTsTimeoutAlarm"] = vm->setLinkVal(vmap, essName, "/config", "essTsTimeoutAlarm", toAlarm);
        amap["essTsTimeoutReset"] = vm->setLinkVal(vmap, essName, "/config", "essTsTimeoutReset", toReset);
        amap["essTsTimeoutHold"] = vm->setLinkVal(vmap, essName, "/config", "essTsTimeoutHold", toHold);

        if (am->am)
        {
            amap["amTsFaults"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "TsFaults", ival);
            amap["amTsAlarms"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "TsAlarms", ival);
            amap["amTsInit"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "TsInit", ival);
        }

        amap["TsFaults"] = vm->setLinkVal(vmap, aname, "/status", "TsFaults", ival);
        amap["TsAlarms"] = vm->setLinkVal(vmap, aname, "/status", "TsAlarms", ival);
        amap["TsInit"] = vm->setLinkVal(vmap, aname, "/status", "TsInit", ival);
        amap["TsState"] = vm->setLinkVal(vmap, aname, "/status", "TsState", cval);
        amap["BypassTs"] = vm->setLinkVal(vmap, aname, "/config", "BypassTs", bval);
        amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status", "AssetState", ival);
        amap["TsStateNum"] = vm->setLinkVal(vmap, aname, "/status", "TsStateNum", ival);

        if (reload == 0)  // complete restart
        {
            amap["Timestamp"]->setVal(initTimestamp);
            // lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
            amap["Timestamp"]->setParam("lastTimestamp", initTimestamp);
            amap["Timestamp"]->setParam("totalTsFaults", 0);
            amap["Timestamp"]->setParam("totalTsAlarms", 0);

            amap["Timestamp"]->setParam("seenFault", false);
            amap["Timestamp"]->setParam("seenAlarm", false);
            amap["Timestamp"]->setParam("seenOk", false);
            amap["Timestamp"]->setParam("seenTS", false);

            amap["Timestamp"]->setParam("TSOk", false);
            dval = 0.0;
            amap["Timestamp"]->setParam("TSseenTime", dval);
            amap["Timestamp"]->setParam("seenInit", false);
            amap["Timestamp"]->setParam("initCnt", -1);

            amap["Timestamp"]->setParam("rdFault",
                                        toFault);  // time remaining before fault
            amap["Timestamp"]->setParam("rdAlarm",
                                        toAlarm);  // time reamining before alarm
            amap["Timestamp"]->setParam("rdReset",
                                        toReset);  // time remaining before reset
            // amap["Timestamp"]     ->setParam("rdHold", toHold);
            // // time to wait before no change test
            amap["Timestamp"]->setParam("tLast",
                                        dval);  // time when last to event was seen

            amap["TsState"]->setVal(cval);
            ival = Asset_Init;
            amap["TsStateNum"]->setVal(ival);
            ival = -1;
            amap["TsInit"]->setVal(ival);
            amap["BypassTs"]->setVal(false);

            amap["essTsFaults"]->setParam("lastTsFaults", 0);
            amap["essTsAlarms"]->setParam("lastTsAlarms", 0);
        }
        // reset reload
        ival = 2;
        amap["CheckAmTimestamp"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();
    double tLast = amap["Timestamp"]->getdParam("tLast");
    if (tLast == 0.0)
        tLast = tNow;
    double tDiff = tNow - tLast;
    amap["Timestamp"]->setParam("tLast", tNow);

    bool BypassTs = amap["BypassTs"]->getbVal();

    toFault = amap["essTsTimeoutFault"]->getdVal();
    toAlarm = amap["essTsTimeoutAlarm"]->getdVal();
    toReset = amap["essTsTimeoutReset"]->getdVal();
    toHold = amap["essTsTimeoutHold"]->getdVal();

    char* currentTimestamp = amap["Timestamp"]->getcVal();
    char* lastTimestamp = amap["Timestamp"]->getcParam("lastTimestamp");  // amap["lastTimestamp"]->getiVal();
    // are we the ess_controller
    if (!am->am)
    {
        // bool initSeen =             amap["Timestamp"] ->getbParam("initSeen");

        amap["essTsFaults"]->setVal(0);
        amap["essTsAlarms"]->setVal(0);
        amap["essTsInit"]->setVal(0);

        int initCnt = amap["Timestamp"]->getiParam("initCnt");
        int icnt = 0;
        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            CheckAmTimestamp(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            icnt++;
        }

        int essTsFaults = amap["essTsFaults"]->getiVal();
        int essTsAlarms = amap["essTsAlarms"]->getiVal();
        int lastTsAlarms = amap["essTsAlarms"]->getiParam("lastTsAlarms");
        int lastTsFaults = amap["essTsFaults"]->getiParam("lastTsFaults");

        // int essTsInit = amap["essTsInit"]->getiVal();
        if (essTsFaults != lastTsFaults)
        {
            amap["essTsFaults"]->setParam("lastTsFaults", essTsFaults);

            if (essTsFaults > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essTsFaults detected at time %2.3f \n", __func__, essTsFaults, tNow);
            }
            else
            {
                FPS_ERROR_PRINT("%s >> %d essTsFaults cleared at time %2.3f\n", __func__, essTsFaults, tNow);
            }
        }
        if (essTsAlarms != lastTsAlarms)
        {
            amap["essTsAlarms"]->setParam("lastTsAlarms", essTsAlarms);

            if (essTsAlarms > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essTsAlarms detected at time %2.3f \n", __func__, essTsAlarms, tNow);
            }
            else
            {
                FPS_ERROR_PRINT("%s >> %d essTsAlarms cleared at time %2.3f\n", __func__, essTsAlarms, tNow);
            }
        }

        if (initCnt != icnt)
        {
            amap["Timestamp"]->setParam("initCnt", icnt);

            FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
        }
        return 0;
    }

    // this is the Asset Manager under the ess_controller instance
    if (BypassTs)
    {
        ival = 1;
        amap["essTsInit"]->addVal(ival);
        return 0;
    }
    double rdFault = amap["Timestamp"]->getdParam("rdFault");
    double rdAlarm = amap["Timestamp"]->getdParam("rdAlarm");
    double rdReset = amap["Timestamp"]->getdParam("rdReset");

    double TSseenTime = amap["Timestamp"]->getdParam("TSseenTime");
    bool TSOk = amap["Timestamp"]->getbParam("TSOk");
    bool seenTS = amap["Timestamp"]->getbParam("seenTS");
    bool seenInit = amap["Timestamp"]->getbParam("seenInit");
    bool seenOk = amap["Timestamp"]->getbParam("seenOk");
    bool seenFault = amap["Timestamp"]->getbParam("seenFault");
    bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");

    // If we are in the init state wait for comms to start count down reset time
    if (strcmp(currentTimestamp, initTimestamp) == 0)
    {
        // if not toally set up yet then quit this pass
        if (!amap["amTsInit"])
        {
            return 0;
        }

        if (!seenInit)  // Ts_Setup
        {
            if (1)
                FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n", __func__, aname, BypassTs ? "true" : "false");

            amap["Timestamp"]->setParam("seenInit", true);

            char* cval = (char*)"Ts Init, no Timestamp Seen";
            amap["TsState"]->setVal(cval);

            ival = 1;
            amap["essTsInit"]->addVal(ival);
            amap["TsInit"]->setVal(0);  // Ts_Init
        }
    }
    else  // wait for comms to go past reset then set active or wait to alarm and
          // then fault
    {
        // if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n",
        // __func__, aname, lastTimestamp?lastTimestamp:"no
        // last Value", tval1)
        if (strcmp(currentTimestamp, lastTimestamp) != 0)
        {
            if (0)
                FPS_ERROR_PRINT(
                    "%s >> %s Timestamp change detected,  from [%s] to "
                    "[%s] tNow %2.3f seenTS [%s]\n",
                    __func__, aname, currentTimestamp, lastTimestamp, tNow, seenTS ? "true" : "false");

            amap["Timestamp"]->setParam("lastTimestamp", currentTimestamp);

            // if(!seenTS)
            {
                if (0)
                    FPS_ERROR_PRINT("%s >> %s Timestamp set TSseenTime %2.3f \n", __func__, aname, tNow);
                amap["Timestamp"]->setParam("seenTS", true);
                amap["Timestamp"]->setParam("TSseenTime", tNow);
                TSseenTime = tNow;
                seenTS = true;
            }
        }
        else  // No Change , start tracking faults and alarms  but wait for hold
              // time
        {
            TSseenTime = amap["Timestamp"]->getdParam("TSseenTime");
            // allow holdoff between testing for change
            if (seenTS)
            {
                if ((tNow - TSseenTime) > toHold)
                {
                    if (0)
                        FPS_ERROR_PRINT(
                            "%s >> %s Timestamp stall detected  tNow %2.3f "
                            "seebTime %2.3f .stalll time %2.3f toHold %2.3f \n",
                            __func__, aname, tNow, TSseenTime, (tNow - TSseenTime), toHold);

                    amap["Timestamp"]->setParam("seenTS", false);
                    seenTS = false;
                    rdAlarm -= (tNow - TSseenTime);
                    rdFault -= (tNow - TSseenTime);

                    if (rdFault < 0.0)
                    {
                        rdFault = 0.0;
                        ;
                    }
                    if (rdAlarm < 0.0)
                    {
                        rdAlarm = 0.0;
                        ;
                    }
                    amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
                    amap["Timestamp"]->setParam("rdFault", rdFault);
                }
            }
        }

        if (seenTS)
        {
            if (!seenOk)
            {
                if (rdReset > 0.0)
                {
                    rdReset -= tDiff;
                    amap["Timestamp"]->setParam("rdReset", rdReset);
                }
            }

            if ((rdReset <= 0.0) && !seenOk)
            {
                if (seenFault)
                {
                    if (1)
                        FPS_ERROR_PRINT("%s >>  Timestamp fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["Timestamp"]->setParam("seenFault", false);
                    seenFault = false;
                }
                if (seenAlarm)
                {
                    if (1)
                        FPS_ERROR_PRINT("%s >>  Timestamp Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["Timestamp"]->setParam("seenAlarm", false);
                    seenAlarm = false;
                }
                amap["Timestamp"]->setParam("seenOk", true);
                seenOk = true;
                amap["Timestamp"]->setParam("TSOk", true);
                TSOk = true;

                if (1)
                    FPS_ERROR_PRINT("%s >>  Timestamp OK for  %s at %2.3f\n", __func__, aname, tNow);
                ival = Asset_Ok;  // seen Timestamp change
                amap["TsStateNum"]->setVal(ival);
                ival = 0;
                amap["TsInit"]->setVal(ival);
                char* tval = nullptr;
                asprintf(&tval, " Ts OK last set  Alarm %3.2f max %3.2f", toAlarm, toFault);
                if (tval)
                {
                    amap["TsState"]->setVal(tval);
                    free(tval);
                    tval = nullptr;
                }
                amap["Timestamp"]->setParam("rdReset", toReset);
            }
        }
        else  // not changed not onHold look out for errors
        {
            // we need to decrement the alarm / fault times
            rdFault = amap["Timestamp"]->getdParam("rdFault");
            rdAlarm = amap["Timestamp"]->getdParam("rdAlarm");
            seenFault = amap["Timestamp"]->getbParam("seenFault");
            seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>  Timestamp stall for  %s at %2.3f rdFault %2.3f "
                    "rdAlarm %2.3f TSOk [%s] seenTS [%s] tDiff %2.3f \n",
                    __func__, aname, tNow, rdFault, rdAlarm, TSOk ? "true" : "false", seenTS ? "true" : "false", tDiff);
            if (rdFault > 0.0)
            {
                rdFault -= tDiff;
                amap["Timestamp"]->setParam("rdFault", rdFault);
            }
            if (rdAlarm > 0.0)
            {
                rdAlarm -= tDiff;
                amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
            }

            if ((rdFault < 0.0) && !seenFault)
            {
                seenFault = true;
                amap["Timestamp"]->setParam("seenFault", true);
                amap["Timestamp"]->setParam("seenOk", false);
                amap["Timestamp"]->setParam("seenAlarm", true);

                if (1)
                    FPS_ERROR_PRINT("%s >>  Timestamp  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                char* tval = nullptr;
                asprintf(&tval, " Ts Fault last set Alarm %3.2f max %3.2f", toAlarm, toFault);
                if (tval)
                {
                    amap["TsState"]->setVal(tval);
                    free(tval);
                    tval = nullptr;
                }
                ival = Asset_Fault;  // Timestamp Fault
                amap["TsStateNum"]->setVal(ival);
                // seenOk = false;
                seenAlarm = true;

                int totalTsFaults = amap["Timestamp"]->getiParam("totalTsFaults");
                totalTsFaults++;
                amap["Timestamp"]->setParam("totalTsFaults", totalTsFaults);

                TSOk = false;
                amap["Timestamp"]->setParam("TSOk", false);

                if (am->am)
                {
                    ival = 1;
                    amap["amTsFaults"]->addVal(ival);
                }
            }
            else if ((rdAlarm < 0.0) && !seenAlarm)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> Timestamp  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                char* tval = nullptr;
                asprintf(&tval, "Ts Alarm last set Alarm %3.2f max %3.2f", toAlarm, toFault);
                if (tval)
                {
                    amap["TsState"]->setVal(tval);
                    free(tval);
                    tval = nullptr;
                }
                // Just test code right now
                ival = Asset_Alarm;  // Timestamp Alarm
                amap["TsStateNum"]->setVal(ival);

                amap["Timestamp"]->setParam("seenAlarm", true);
                seenAlarm = true;
                amap["Timestamp"]->setParam("seenOk", false);
                int totalTsAlarms = amap["Timestamp"]->getiParam("totalTsAlarms");
                totalTsAlarms++;
                amap["Timestamp"]->setParam("totalTsAlarms", totalTsAlarms);

                TSOk = false;
                amap["Timestamp"]->setParam("TSOk", false);

                if (am->am)
                {
                    amap["amTsAlarms"]->addVal(ival);
                }
            }
            else
            {
                if (0)
                    FPS_ERROR_PRINT(
                        "%s >> Ts for [%s] [%s] Stalled at %2.3f  Fault "
                        "%2.3f Alarm %2.3f \n",
                        __func__, aname, amap["Timestamp"]->getcVal(), tNow, rdFault, rdAlarm);
            }
        }
        if (seenFault)
        {
            int ival = 1;
            amap["TsFaults"]->addVal(ival);
            amap["essTsFaults"]->addVal(ival);
        }
        else
        {
            if (seenTS)
            {
                if (rdFault < toFault)
                {
                    rdFault += tDiff;
                    amap["Timestamp"]->setParam("rdFault", rdFault);
                }
            }
        }

        if (seenAlarm)
        {
            int ival = 1;
            amap["TsAlarms"]->addVal(ival);
            amap["essTsAlarms"]->addVal(ival);
        }
        else
        {
            if (seenTS)
            {
                if (rdAlarm < toAlarm)
                {
                    rdAlarm += tDiff;
                    amap["Timestamp"]->setParam("rdAlarm", rdAlarm);
                }
            }
        }
    }
    //
    // int ival1, ival2;
    // if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n",
    // __func__, aname,
    // amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
    return 0;
}
#endif
