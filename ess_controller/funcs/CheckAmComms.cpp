#ifndef CHECKAMCOMMS_CPP
#define CHECKAMCOMMS_CPP

#include "asset.h"

char* strtime(const struct tm* timeptr);
extern "C++" {
int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
}
/**
 * @brief Periodically check if we are able to communicate with the assets/asset
 * managers
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param am the assetVar
 */
int CheckAmComms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (!checkAv(vmap, amap, aname, p_fims, av))
    {
        FPS_PRINT_ERROR(">> ERROR unable to continue aname [{}]", aname);
        return -1;
    }

    asset_manager* am = av->am;
    VarMapUtils* vm = am->vm;

    char* LogDir = getLogDir(vmap, *av->am->vm);

    // TODO is CheckAmComms Used ??
    // TODO put state strings  into a config assetVar so we can set it up from
    // config There is an assetVar for log. The value for the assetVar log is the
    // file name Ex.: /config/ess: { CheckAmCommsLogFile: { value:
    // "file:CheckAmComms.txt" } }
    char* cval = (char*)"Comms Init";
    char* initTimestamp = (char*)"Initial Timestamp";
    char* logName = (char*)"CheckAmCommsLog";
    char* logFile = (char*)"file:CheckAmCommsLog.txt";
    // Default fault/alarm destinations should look like
    // /assets/aname/summary:[faults | alarms]
    char* fltDest = (char*)(std::string("/assets/") + aname + "/summary:faults").c_str();
    char* alrmDest = (char*)(std::string("/assets/") + aname + "/summary:alarms").c_str();
    int ival = 0;
    bool bval = false;
    double dval = 0.0;
    double toAlarm = 0.5;
    double toFault = 5.0;
    double toReset = 2.5;
    char* essName = vm->getSysName(vmap);

    int reload = 0;
    double tNow = vm->get_time_dbl();

    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAmComms");

    // if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n",
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

        amap["essCommsFaultCnt"] = vm->setLinkVal(vmap, essName, "/status", "essCommsFaultCnt", ival);
        amap["essCommsAlarmCnt"] = vm->setLinkVal(vmap, essName, "/status", "essCommsAlarmCnt", ival);
        amap["essCommsInit"] = vm->setLinkVal(vmap, essName, "/status", "essCommsInit", ival);
        amap["essCommsFaultTimeout"] = vm->setLinkVal(vmap, essName, "/config", "essCommsFaultTimeout", toFault);
        amap["essCommsAlarmTimeout"] = vm->setLinkVal(vmap, essName, "/config", "essCommsAlarmTimeout", toAlarm);
        amap["essCommsRecoverTimeout"] = vm->setLinkVal(vmap, essName, "/config", "essCommsRecoverTimeout", toReset);

        if (am->am)
        {
            amap["amCommsFaultCnt"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "CommsFaultCnt", ival);
            amap["amCommsAlarmCnt"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "CommsAlarmCnt", ival);
            amap["amCommsInit"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "CommsInit", ival);
        }

        amap["CommsFaultCnt"] = vm->setLinkVal(vmap, aname, "/status", "CommsFaultCnt", ival);
        amap["CommsAlarmCnt"] = vm->setLinkVal(vmap, aname, "/status", "CommsAlarmCnt", ival);
        amap["CommsInit"] = vm->setLinkVal(vmap, aname, "/status", "CommsInit", ival);
        amap["CommsState"] = vm->setLinkVal(vmap, aname, "/status", "CommsState", cval);
        amap["CommsStateNum"] = vm->setLinkVal(vmap, aname, "/status", "CommsStateNum", ival);
        amap["CommsOK"] = vm->setLinkVal(vmap, aname, "/status", "CommsOK", bval);
        amap["BypassComms"] = vm->setLinkVal(vmap, aname, "/config", "BypassComms", bval);
        amap["FaultDestination"] = vm->setLinkVal(vmap, aname, "/config", "FaultDestination", fltDest);
        amap["AlarmDestination"] = vm->setLinkVal(vmap, aname, "/config", "AlarmDestination", alrmDest);

        amap[logName] = vm->setLinkVal(vmap, essName, "/logs", logName, logFile);
        FPS_ERROR_PRINT("%s >> Log file from CheckAmCommsLog is %s\n", __func__,
                        amap[logName]->getcVal() ? amap[logName]->getcVal() : "none");
        FPS_ERROR_PRINT("%s >> param from CheckAmCommsLog is %s\n", __func__,
                        amap[logName]->getbParam("enablePerf") ? "true" : "false");
        if (!amap[logName]->getcVal())
            amap[logName]->setVal(logFile);

        if (reload == 0)  // complete restart
        {
            amap["Timestamp"]->setVal(initTimestamp);
            // lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
            amap["Timestamp"]->setParam("lastTimestamp", initTimestamp);
            amap["Timestamp"]->setParam("lastChange", tNow);

            amap["Timestamp"]->setParam("totalCommsFaults", 0);
            amap["Timestamp"]->setParam("totalCommsAlarms", 0);
            amap["Timestamp"]->setParam("seenFault", false);
            amap["Timestamp"]->setParam("seenAlarm", false);
            amap["Timestamp"]->setParam("seenOk", false);
            amap["Timestamp"]->setParam("seenStalled", false);  // Ensure log msgs are
                                                                // only sent out once
                                                                // when comms stalled
                                                                // for the first time
            amap["Timestamp"]->setParam("seenInit", false);
            amap["Timestamp"]->setParam("initCnt", -1);

            amap["Timestamp"]->setParam("FaultTime",
                                        amap["essCommsFaultTimeout"]->getdVal());  // time remaining before fault
            amap["Timestamp"]->setParam("AlarmTime",
                                        amap["essCommsAlarmTimeout"]->getdVal());  // time reamining before alarm
            amap["Timestamp"]->setParam("ResetTime",
                                        amap["essCommsRecoverTimeout"]->getdVal());  // time remaining before reset
            amap["Timestamp"]->setParam("tLast",
                                        dval);  // time when last to event was seen

            amap["CommsState"]->setVal(cval);
            ival = Asset_Init;
            amap["CommsStateNum"]->setVal(ival);
            ival = -1;
            amap["CommsInit"]->setVal(ival);
            amap["BypassComms"]->setVal(false);

            // Check if the assetVar we're working with already has a log file
            char* logFile = av->getcParam("logName");
            if (!logFile)
            {
                // If the assetVar does not have the log file, check if the local map
                // has a log file
                if (amap[logName])
                {
                    logFile = amap[logName]->getcVal();
                    if (1)
                        FPS_ERROR_PRINT("%s >> Log file name from amap %s is %s\n", __func__, aname, logFile);
                }
                else
                {
                    logFile = (char*)"file:CheckAmCommsLog.txt";
                }
            }

            // Param for checking if logging is enabled or not
            amap[logName]->setParam("enablePerf", true);
            if (0)
                FPS_ERROR_PRINT("%s >> enablePerf for %s is %s\n", __func__, amap[logName]->name.c_str(),
                                amap[logName]->getbParam("enablePerf") ? "true" : "false");

            // amap[logName]->setVal(logFile);
            // amap[logName]->openLog(vm->runLogDir?vm->runLogDir:(char*)"run_logs",
            // logFile); amap[logName]->logAlways();

            // if(!am->am)  // Nah do this in setLinkVals
            // {
            //     amap["essCommsTimeoutFault"] ->setVal(toFault);
            //     amap["essCommsTimeoutAlarm"] ->setVal(toAlarm);
            //     amap["essCommsTimeoutReset"] ->setVal(toReset);

            // }
        }

        // reset reload
        ival = 2;
        amap["CheckAmComms"]->setVal(ival);
    }

    // Another way to grab current time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm* local_tm = localtime(&tnow);

    bool BypassComms = amap["BypassComms"]->getbVal();

    toFault = amap["essCommsFaultTimeout"]->getdVal();
    toAlarm = amap["essCommsAlarmTimeout"]->getdVal();
    toReset = amap["essCommsRecoverTimeout"]->getdVal();

    char* currentTimestamp = amap["Timestamp"]->getcVal();
    char* lastTimestamp = amap["Timestamp"]->getcParam("lastTimestamp");  // amap["lastHeartBeat"]->getiVal();
    // are we the ess_controller
    assetVar Av;
    if (!am->am)
    {
        // bool initSeen =             amap["Timestamp"] ->getbParam("initSeen");

        amap["essCommsFaultCnt"]->setVal(0);
        amap["essCommsAlarmCnt"]->setVal(0);
        amap["essCommsInit"]->setVal(0);

        int initCnt = amap["Timestamp"]->getiParam("initCnt");
        int icnt = 0;
        int essCommsFaults = amap["essCommsFaultCnt"]->getiVal();
        int essCommsAlarms = amap["essCommsAlarmCnt"]->getiVal();

        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            Av.am = amc;
            CheckAmComms(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
            icnt++;
            essCommsFaults = amap["essCommsFaultCnt"]->getiVal();
            essCommsAlarms = amap["essCommsAlarmCnt"]->getiVal();
            if (essCommsFaults || essCommsAlarms)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> %s %d essCommsFaults %d essCommsAlarms detected\n", __func__,
                                    amc->name.c_str(), essCommsFaults, essCommsAlarms);
            }
        }
        // int essCommsInit = amap["essCommsInit"]->getiVal();
        if (essCommsFaults > 0)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> %d essCommsFaults detected\n", __func__, essCommsFaults);
            // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> %d
            // essCommsFaults detected\n"
            //                              , __func__
            //                              , essCommsFaults);

            /// Proceed to shutdown due to fault here
            if (!amap["FaultShutdown"])
            {
                bool bval = true;
                // May need to change essName to "pcs" or "bms" if other asset managers
                // can also initiate shutdown process Right now, ShutdownSystem function
                // is called from ess wake up
                amap["FaultShutdown"] = vm->setLinkVal(vmap, essName, "/status", "FaultShutdown", bval);
            }
            amap["FaultShutdown"]->setVal(true);
        }
        if (essCommsAlarms > 0)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> %d essCommsAlarms detected\n", __func__, essCommsAlarms);
            // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> %d
            // essCommsAlarms detected\n"
            //                              , __func__
            //                              , essCommsAlarms);
        }

        if (initCnt != icnt)
        {
            amap["Timestamp"]->setParam("initCnt", icnt);

            if (1)
                FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
            // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> icnt %d
            // intiSeen %d  change detected at %s\n"
            //                              , __func__
            //                              , icnt
            //                              , initCnt
            //                              , strtime(local_tm));
        }
        return 0;
    }

    if (BypassComms)
    {
        ival = 1;
        amap["essCommsInit"]->addVal(ival);

        // Set CommsOK here?
        amap["CommsOK"]->setVal(true);
        return 0;
    }

    double tLast = amap["Timestamp"]->getdParam("tLast");
    double FaultTime = amap["Timestamp"]->getdParam("FaultTime");
    double AlarmTime = amap["Timestamp"]->getdParam("AlarmTime");
    double RecoverTime = amap["Timestamp"]->getdParam("RecoverTime");
    double lastChange = amap["Timestamp"]->getdParam("lastChange");

    // If we are in the init state wait for comms to start count down reset time
    if (strcmp(currentTimestamp, initTimestamp) == 0)
    {
        amap["Timestamp"]->setParam("lastChange", tNow);

        bool seenInit = amap["Timestamp"]->getbParam("seenInit");

        // ival = 1; amap["CheckAssetComs"]->setVal(ival);
        // ival = 1; amap["CheckAssetComs"]->setVal(ival);
        if (0)
            FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n", __func__, aname, BypassComms ? "true" : "false");
        // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> %s  NO Timestamp,
        // bypass [%s]\n"
        //                                  , __func__
        //                                  , aname
        //                                  , BypassComms ? "true" : "false");

        // if not toally set up yet then quit this pass
        if (!amap["amCommsInit"])
        {
            return 0;
        }

        if (!seenInit)  // Comms_Setup
        {
            amap["Timestamp"]->setParam("seenInit", true);

            char* cval = (char*)"Comms Init, no Timestamp Seen";
            amap["CommsState"]->setVal(cval);

            ival = 1;
            amap["essCommsInit"]->addVal(ival);
            amap["CommsInit"]->setVal(0);  // Comms_Init
        }
        amap["Timestamp"]->setParam("tLast", tNow);
    }
    else  // wait for comms to go past reset then set active or wait to alarm and
          // then fault
    {
        // if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n",
        // __func__, aname, lastTimestamp?lastTimestamp:"no
        // last Value", tval1);
        amap["Timestamp"]->setParam("tLast", tNow);

        double toVal = amap["Timestamp"]->getLastSetDiff(tNow);

        // Has value changed ? If yes then count down RecoverTime to zero based on
        // tNow - tLast if we changed then increase alarm / fault timeout up to max
        // decrease recovery time to 0 if recovery time == 0 and falut/alarm was set
        // then reset alarm not quite add the time since last changed this way we
        // recover all the time slots we lost

        // did we see a change
        if (strcmp(currentTimestamp, lastTimestamp) != 0)
        {
            if (0)
                FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] at %2.3f \n", __func__, aname,
                                lastTimestamp, currentTimestamp, tNow);

            amap["Timestamp"]->setParam("lastTimestamp", currentTimestamp);
            amap["Timestamp"]->setParam("lastChange", tNow);

            bool seenOk = amap["Timestamp"]->getbParam("seenOk");
            if (RecoverTime > 0.0)
            {
                RecoverTime -= (tNow - lastChange);
                RecoverTime = (RecoverTime >= 0.0) ? RecoverTime : 0.0;
                amap["Timestamp"]->setParam("RecoverTime", RecoverTime);
            }
            if (AlarmTime < toAlarm)
            {
                AlarmTime += (tNow - lastChange);
                AlarmTime = AlarmTime <= toAlarm ? AlarmTime : toAlarm;
                amap["Timestamp"]->setParam("AlarmTime", AlarmTime);
            }
            if (FaultTime < toFault)
            {
                FaultTime += (tNow - lastChange);
                FaultTime = FaultTime <= toFault ? FaultTime : toFault;
                amap["Timestamp"]->setParam("FaultTime", FaultTime);
            }

            if (strcmp(aname, "pcs") == 0)
                if (0)
                    FPS_ERROR_PRINT(
                        "%s >>  ts  change for %s from [%s] to [%s] at "
                        "%2.3f, (%2.3f)   RecoverTime %2.3f AlarmTime %2.3f "
                        "toAlarm %2.3f FaultTime %2.3f\n",
                        __func__, aname, lastTimestamp ? lastTimestamp : "no last Value", currentTimestamp, tNow,
                        (tNow - lastChange), RecoverTime, AlarmTime, toAlarm, FaultTime);

            // Log timestamp change here
            // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
            // change for %s from [%s] to [%s]  RecoverTime now %2.3f diff %2.3f
            // AlarmTime %2.3f FaultTime %2.3f\n"
            //                              , __func__
            //                              , aname
            //                              , lastTimestamp ? lastTimestamp : "no last
            //                              value" , currentTimestamp , RecoverTime,
            //                              (tNow - tLast), AlarmTime, FaultTime);

            ival = amap["CommsStateNum"]->getiVal();
            FPS_ERROR_PRINT("%s >> %s Current Comms State Num %d toReset Time %2.3f seenOk %s\n", __func__, aname, ival,
                            RecoverTime, seenOk ? "true" : "false");
            // reset time passed , still changing , time to switch to Comms_Ready
            // if(RecoverTime <= 0.0 && ival != seenOk)
            if (RecoverTime <= 0.0 && !seenOk)
            {
                bool seenFault = amap["Timestamp"]->getbParam("seenFault");
                // bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
                bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");

                if (0)
                    FPS_ERROR_PRINT("%s >>  Timestamp  change for %s from [%s] to [%s] \n", __func__, aname,
                                    lastTimestamp ? lastTimestamp : "no last Value", currentTimestamp);

                if (seenFault)
                {
                    if (1)
                        FPS_ERROR_PRINT("%s >>  Timestamp fault for  %s cleared at %2.3f\n", __func__, aname, tNow);

                    // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >>  Timestamp
                    // Comms fault for %s cleared at %s\n"
                    //                 , __func__
                    //                 , aname
                    //                 , strtime(local_tm));

                    amap["Timestamp"]->setParam("seenFault", false);
                }
                if (seenAlarm)
                {
                    if (1)
                        FPS_ERROR_PRINT("%s >>  Timestamp Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);

                    // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >>  Timestamp
                    // Comms alarm for %s cleared at %s\n"
                    //                 , __func__
                    //                 , aname
                    //                 , strtime(local_tm));

                    amap["Timestamp"]->setParam("seenAlarm", false);
                }

                amap["Timestamp"]->setParam("seenOk", true);
                seenOk = true;
                amap["CommsOK"]->setVal(true);

                if (1)
                    FPS_ERROR_PRINT("%s >>  Timestamp OK for  %s at %2.3f\n", __func__, aname, tNow);

                // Log comms working for the first time
                // if (1) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
                // Comms for [%s] [%s] OK at %s\n"
                //                     , __func__
                //                     , aname
                //                     , amap["Timestamp"]->getcVal()
                //                     , strtime(local_tm));
                // Ensure comms is no longer stalled
                if (amap["Timestamp"]->getbParam("seenStalled"))
                    amap["Timestamp"]->setParam("seenStalled", false);

                ival = Asset_Ok;  // seen Timestamp change
                amap["CommsStateNum"]->setVal(ival);
                ival = 0;
                amap["CommsInit"]->setVal(ival);
                char* tval;
                asprintf(&tval, " Comms OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if (tval)
                {
                    amap["CommsState"]->setVal(tval);
                    free(tval);
                    tval = nullptr;
                }
                amap["Timestamp"]->setParam("RecoverTime", toReset);
            }

            // increment alarm and fault time
            // This seems redundant. Fault/alarm time already incremented (see above)
            // if(FaultTime < toFault)
            // {
            //     FaultTime += (tNow - tLast);
            //     FaultTime = FaultTime <= toFault ? FaultTime : toFault;
            //     amap["Timestamp"] ->setParam("FaultTime",FaultTime);
            // }
            // if(AlarmTime < toAlarm)
            // {
            //     AlarmTime += (tNow - tLast);
            //     FaultTime = FaultTime <= toFault ? FaultTime : toFault;
            //     AlarmTime = AlarmTime <= toAlarm ? AlarmTime : toAlarm;
            //     amap["Timestamp"] ->setParam("AlarmTime",AlarmTime);
            // }

            // if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n",
            // __func__, aname, lastTs?lastTs:"no last Value",
            // Ts);
            amap["Timestamp"]->setParam("lastTimestamp", currentTimestamp);
            // if ((toVal > toFault)  && !bokFaults && !bypass)
        }
        // No change see how long we have been waiting ( use tLast here)
        else  // No Change , start tracking faults and alarms
        {
            bool seenFault = amap["Timestamp"]->getbParam("seenFault");
            // bool seenOk  = amap["Timestamp"]->getbParam("seenOk");
            bool seenAlarm = amap["Timestamp"]->getbParam("seenAlarm");
            if (FaultTime > 0.0)
            {
                FaultTime -= (tNow - tLast);
                FaultTime = FaultTime >= 0.0 ? FaultTime : 0.0;
                amap["Timestamp"]->setParam("FaultTime", FaultTime);
            }
            if (AlarmTime > 0.0)
            {
                AlarmTime -= (tNow - tLast);
                AlarmTime = AlarmTime >= 0.0 ? AlarmTime : 0.0;
                amap["Timestamp"]->setParam("AlarmTime", AlarmTime);
            }
            if (RecoverTime < toReset)
            {
                RecoverTime += (tNow - tLast);
                RecoverTime = RecoverTime < toReset ? RecoverTime : toReset;
                amap["Timestamp"]->setParam("RecoverTime", RecoverTime);
            }

            if (FaultTime <= 0.0 && !seenFault)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>  Timestamp  Fault (%2.3f) for %s at %2.3f \n", __func__, FaultTime, aname,
                                    tNow);

                // Log comms fault here
                // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
                // Comms Fault for %s at %s\n"
                //                          , __func__
                //                          , aname
                //                          , strtime(local_tm));
                char* tval = nullptr;
                asprintf(&tval, "Timestamp Comms Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if (tval)
                {
                    amap["CommsState"]->setVal(tval);
                    free(tval);
                    tval = nullptr;
                }

                // Send out comms fault here
                char* dest = nullptr;
                char* msg = nullptr;

                // Removed hard-coded destination
                // asprintf(&dest, "/assets/%s/summary:faults", aname);
                asprintf(&dest, "%s", fltDest);
                asprintf(&msg, "Timestamp Comms fault for %s at %s", aname, strtime(local_tm));
                vm->sendAlarm(vmap, amap["Timestamp"], dest, nullptr, msg, 2);

                const auto now = flex::get_time_dbl();

                ESSLogger::get().critical("Comms lost for: [{}]", aname);
                std::string dirAndFile = fmt::format("{}/{}_{}.{}", LogDir, aname, "CommsFault",
                                                     "txt");  // might just hard code the file name itself instead.
                ESSLogger::get().logIt(dirAndFile);           // todo: change the directory where
                                                              // this log file is sent to.

                assetVar* temp_av = amap.at("Timestamp");
                av->sendEvent(temp_av->name.c_str(), am->p_fims, Severity::Fault, "Comms lost for: %s at time: %2.3f",
                              aname, now.count());

                if (0)
                    FPS_ERROR_PRINT("%s >> Fault Sent dest [%s] msg [%s]  am %p \n", __func__, dest, msg, (void*)am);
                // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
                // Comms fault sent dest [%s] msg [%s]  am %p at %s\n"
                //                          , __func__
                //                          , dest
                //                          , msg
                //                          , (void*)am
                //                          , strtime(local_tm));
                if (1)
                    FPS_ERROR_PRINT("%s >>  aname %s TimeStamp comp [%s] name [%s] \n", __func__, aname,
                                    amap["Timestamp"]->comp.c_str(), amap["Timestamp"]->name.c_str());

                if (dest)
                    free(dest);
                dest = nullptr;
                if (msg)
                    free(msg);
                msg = nullptr;

                int ival = 1;
                amap["CommsFaultCnt"]->addVal(ival);
                amap["essCommsFaultCnt"]->addVal(ival);

                if (am->am)
                {
                    amap["amCommsFaultCnt"]->addVal(ival);
                }

                ival = Asset_Fault;  // Timestamp Fault
                amap["CommsStateNum"]->setVal(ival);

                seenFault = true;
                amap["Timestamp"]->setParam("seenFault", true);
                amap["Timestamp"]->setParam("seenOk", false);
                amap["Timestamp"]->setParam("seenAlarm", true);
                // seenOk = false;
                seenAlarm = false;

                int totalCommsFaults = amap["Timestamp"]->getiParam("totalCommsFaults");
                totalCommsFaults++;
                amap["Timestamp"]->setParam("totalCommsFaults", totalCommsFaults);

                amap["CommsOK"]->setVal(false);
            }
            else if ((AlarmTime <= 0.0) && !seenAlarm)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>  Timestamp  Alarm  (%2.3f) for %s at %2.3f \n", __func__, AlarmTime, aname,
                                    tNow);

                // Log timestamp alarm here
                // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
                // Comms Alarm for %s at %s\n"
                //                          , __func__
                //                          , aname
                //                          , strtime(local_tm));

                char* tval = nullptr;
                asprintf(&tval, "Timestamp Comms Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if (tval)
                {
                    amap["CommsState"]->setVal(tval);
                    free(tval);
                    tval = nullptr;
                }

                // Send out comms fault here
                char* dest = nullptr;
                char* msg = nullptr;

                // Removed hard-coded destination
                // asprintf(&dest, "/assets/%s/summary:alarms", aname);
                asprintf(&dest, "%s", alrmDest);
                asprintf(&msg, "Timestamp Comms alarm for %s at %s", aname, strtime(local_tm));
                vm->sendAlarm(vmap, amap["Timestamp"], dest, nullptr, msg, 2);

                const auto now = flex::get_time_dbl();

                ESSLogger::get().warn("Comms [alarm]");

                assetVar* temp_av = amap.at("Timestamp");
                av->sendEvent(temp_av->name.c_str(), am->p_fims, Severity::Alarm, "Comms alarm at time: %2.3f",
                              now.count());

                if (0)
                    FPS_ERROR_PRINT("%s >> Alarm Sent dest [%s] msg [%s]  am %p \n", __func__, dest, msg, (void*)am);
                // if (0) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
                // Comms alarm sent dest [%s] msg [%s]  am %p at %s\n"
                //                          , __func__
                //                          , dest
                //                          , msg
                //                          , (void*)am
                //                          , strtime(local_tm));

                if (dest)
                    free(dest);
                dest = nullptr;
                if (msg)
                    free(msg);
                msg = nullptr;

                int ival = 1;
                amap["CommsAlarmCnt"]->addVal(ival);
                amap["essCommsAlarmCnt"]->addVal(ival);

                if (am->am)
                {
                    amap["amCommsAlarmCnt"]->addVal(ival);
                }
                ival = Asset_Alarm;  // Timestamp Alarm
                amap["CommsStateNum"]->setVal(ival);

                amap["Timestamp"]->setParam("seenAlarm", true);
                // amap["Timestamp"]->setParam("seenFault", false);
                amap["Timestamp"]->setParam("seenOk", false);
                int totalCommsAlarms = amap["Timestamp"]->getiParam("totalCommsAlarms");
                totalCommsAlarms++;
                amap["Timestamp"]->setParam("totalCommsAlarms", totalCommsAlarms);

                amap["CommsOK"]->setVal(false);
            }
            else
            {
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >> Comms for [%s] [%s] Stalled at %2.3f  Reset "
                        "%2.3f Fault %2.3f Alarm %2.3f \n",
                        __func__, aname, amap["Timestamp"]->getcVal(), tNow, RecoverTime, FaultTime, AlarmTime);
            }

            // Log comms stalling for the first time
            if (!amap["Timestamp"]->getbParam("seenStalled"))
            {
                // if (1) amap[logName]->sendLog(amap["Timestamp"], "%s >> Timestamp
                // Comms for [%s] [%s] Stalled at %s\n"
                //             , __func__
                //             , aname
                //             , amap["Timestamp"]->getcVal()
                //             , strtime(local_tm));

                // Ensure comms is stalled so that we only log this once
                amap["Timestamp"]->setParam("seenStalled", true);
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