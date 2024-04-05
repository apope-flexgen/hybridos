philHB.cpp
    /*
     *    After init we must get a continual heartbeat otherwise we alarm and
     then fault.
     *    the bms Heartbeat arrives on
     /components/catl_ems_bms_01_rw:ems_heartbeat,
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

     * toHold time to allow between heartbeat changes before worrying about it
     * toAlarm time after a stalled Heatbeat causes an Alarm
     * toFault time after a stalled Heatbeat causes a Fault
     *  toReset time after changes start being seen again before resetting
     faults and Alarms
     *
     */
    int
    CheckAmHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval = (char*)"Heartbeat Init";
    VarMapUtils* vm = am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAmHeartbeat");
    // assetVar* CheckAssetComms = amap["CheckAmComms"];
    double toHold = 1.5;  // Seconds between HB changes
    double toAlarm = 2.5;
    double toFault = 6.0;
    double toReset = 2.5;
    int initHeartbeat = -1;  //(char *)" Initial Heartbeat";

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

        amap["Heartbeat"] = vm->setLinkVal(vmap, aname, "/status", "Heartbeat", initHeartbeat);
        if (1)
            FPS_ERROR_PRINT("\n\n%s >>  aname [%s] Heartbeat %p comp [%s] name [%s] \n", __func__, aname,
                            amap["Heartbeat"], amap["Heartbeat"]->comp.c_str(), amap["Heartbeat"]->name.c_str());
        // ess system faults
        amap["essHeartbeatFaultCnt"] = vm->setLinkVal(vmap, "ess", "/status", "essHeartbeatFaultCnt", ival);
        amap["essHeartbeatAlarmCnt"] = vm->setLinkVal(vmap, "ess", "/status", "essHeartbeatAlarmCnt", ival);
        amap["essHeartbeatInit"] = vm->setLinkVal(vmap, "ess", "/status", "essHeartbeatInit", ival);
        amap["essHeartbeatFaultTimeout"] = vm->setLinkVal(vmap, "ess", "/config", "essHeartbeatFaultTimeout", toFault);
        amap["essHeartbeatAlarmTimeout"] = vm->setLinkVal(vmap, "ess", "/config", "essHeartbeatAlarmTimeout", toAlarm);
        amap["essHeartbeatResetTimeout"] = vm->setLinkVal(vmap, "ess", "/config", "essHeartbeatResetTimeout", toReset);
        amap["essHeartbeatHoldTimeout"] = vm->setLinkVal(vmap, "ess", "/config", "essHeartbeatHoldTimeout", toHold);
        // my manager faults ( could be ess)
        if (am->am)  // am I the ess (am->am == nullptr) or is the ess my manager
                     // (am->am !=nullptr)
        {
            amap["amHeartbeatFaultCnt"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "HeartbeatFaultCnt",
                                                         ival);
            amap["amHeartbeatAlarmCnt"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "HeartbeatAlarmCnt",
                                                         ival);
            amap["amHeartbeatInit"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "HeartbeatInit", ival);
        }
        // local faults
        amap["HeartbeatFaultCnt"] = vm->setLinkVal(vmap, aname, "/status", "HeartbeatFaultCnt", ival);
        amap["HeartbeatAlarmCnt"] = vm->setLinkVal(vmap, aname, "/status", "HeartbeatAlarmCnt", ival);
        amap["HeartbeatInit"] = vm->setLinkVal(vmap, aname, "/status", "HeartbeatInit", ival);
        amap["HeartbeatState"] = vm->setLinkVal(vmap, aname, "/status", "HeartbeatState", cval);
        amap["BypassHeartbeat"] = vm->setLinkVal(vmap, aname, "/config", "BypassHeartbeat", bval);
        amap["HeartbeatStateNum"] = vm->setLinkVal(vmap, aname, "/status", "HeartbeatStateNum", ival);
        amap["HeartbeatOK"] = vm->setLinkVal(vmap, aname, "/status", "HeartbeatOK", bval);

        if (reload == 0)  // complete restart
        {
            amap["Heartbeat"]->setVal(initHeartbeat);
            // lastHeartbeat=strdup(tsInit);//state"]->setVal(cval);
            amap["Heartbeat"]->setParam("lastHeartbeat", initHeartbeat);
            amap["Heartbeat"]->setParam("totalHbFaults", 0);
            amap["Heartbeat"]->setParam("totalHbAlarms", 0);

            amap["Heartbeat"]->setParam("seenFault", false);
            amap["Heartbeat"]->setParam("seenAlarm", false);
            amap["Heartbeat"]->setParam("seenOk", false);
            amap["Heartbeat"]->setParam("seenHB", false);

            amap["Heartbeat"]->setParam("HBOk", false);
            dval = 0.0;
            amap["Heartbeat"]->setParam("HBseenTime", dval);
            amap["Heartbeat"]->setParam("seenInit", false);
            amap["Heartbeat"]->setParam("initCnt", -1);

            amap["Heartbeat"]->setParam("rdFault",
                                        toFault);  // time remaining before fault
            amap["Heartbeat"]->setParam("rdAlarm",
                                        toAlarm);  // time reamining before alarm
            amap["Heartbeat"]->setParam("rdReset",
                                        toReset);  // time remaining before reset
            // amap["Heartbeat"]     ->setParam("rdHold", toHold);
            // // time to wait before no change test
            amap["Heartbeat"]->setParam("tLast",
                                        dval);  // time when last to event was seen

            amap["HeartbeatState"]->setVal(cval);
            ival = Asset_Init;
            amap["HeartbeatStateNum"]->setVal(ival);
            ival = -1;
            amap["HeartbeatInit"]->setVal(ival);
            amap["BypassHeartbeat"]->setVal(false);

            amap["essHeartbeatFaultCnt"]->setParam("lastHbFaults", 0);
            amap["essHeartbeatAlarmCnt"]->setParam("lastHbAlarms", 0);
        }
        // reset reload
        ival = 2;
        amap["CheckAmHeartbeat"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();
    double tLast = amap["Heartbeat"]->getdParam("tLast");
    if (tLast == 0.0)
        tLast = tNow;
    double tDiff = tNow - tLast;
    amap["Heartbeat"]->setParam("tLast", tNow);

    bool BypassHb = amap["BypassHeartbeat"]->getbVal();

    toFault = amap["essHeartbeatFaultTimeout"]->getdVal();
    toAlarm = amap["essHeartbeatAlarmTimeout"]->getdVal();
    toReset = amap["essHeartbeatResetTimeout"]->getdVal();
    toHold = amap["essHeartbeatHoldTimeout"]->getdVal();

    int currentHeartbeat = amap["Heartbeat"]->getiVal();
    int lastHeartbeat = amap["Heartbeat"]->getiParam("lastHeartbeat");  // amap["lastHeartBeat"]->getiVal();

    if (0)
        FPS_ERROR_PRINT("\n\n%s >>  HB state  for %s at time %2.3f  current %d  last %d \n", __func__, aname, tNow,
                        currentHeartbeat, lastHeartbeat);
    if (0)
        FPS_ERROR_PRINT("%s >>  aname [%s] Heartbeat %p comp [%s] name [%s] \n", __func__, aname, amap["Heartbeat"],
                        amap["Heartbeat"]->comp.c_str(), amap["Heartbeat"]->name.c_str());

    // are we the ess_controller
    if (!am->am)
    {
        // bool initSeen =             amap["Heartbeat"] ->getbParam("initSeen");

        amap["essHeartbeatFaultCnt"]->setVal(0);
        amap["essHeartbeatAlarmCnt"]->setVal(0);
        amap["essHeartbeatInit"]->setVal(0);

        int initCnt = amap["Heartbeat"]->getiParam("initCnt");
        int icnt = 0;
        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            CheckAmHeartbeat(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            icnt++;
        }

        int essHbFaults = amap["essHeartbeatFaultCnt"]->getiVal();
        int essHbAlarms = amap["essHeartbeatAlarmCnt"]->getiVal();
        int lastHbAlarms = amap["essHeartbeatAlarmCnt"]->getiParam("lastHbAlarms");
        int lastHbFaults = amap["essHeartbeatFaultCnt"]->getiParam("lastHbFaults");

        // int essHbInit = amap["essHbInit"]->getiVal();
        if (essHbFaults != lastHbFaults)
        {
            amap["essHeartbeatFaultCnt"]->setParam("lastHbFaults", essHbFaults);

            if (essHbFaults > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essHbFaults detected at time %2.3f \n", __func__, essHbFaults, tNow);
            }
            else
            {
                FPS_ERROR_PRINT("%s >> %d essHbFaults cleared at time %2.3f\n", __func__, essHbFaults, tNow);
            }
        }
        if (essHbAlarms != lastHbAlarms)
        {
            amap["essHeartbeatAlarmCnt"]->setParam("lastHbAlarms", essHbAlarms);

            if (essHbAlarms > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essHbAlarms detected at time %2.3f \n", __func__, essHbAlarms, tNow);
            }
            else
            {
                FPS_ERROR_PRINT("%s >> %d essHbAlarms cleared at time %2.3f\n", __func__, essHbAlarms, tNow);
            }
        }

        if (initCnt != icnt)
        {
            amap["Heartbeat"]->setParam("initCnt", icnt);

            FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
        }
        return 0;
    }

    // this is the Asset Manager under the ess_controller instance
    if (BypassHb)
    {
        ival = 1;
        amap["essHeartbeatInit"]->addVal(ival);
        return 0;
    }
    double rdFault = amap["Heartbeat"]->getdParam("rdFault");
    double rdAlarm = amap["Heartbeat"]->getdParam("rdAlarm");
    double rdReset = amap["Heartbeat"]->getdParam("rdReset");

    double HBseenTime = amap["Heartbeat"]->getdParam("HBseenTime");
    bool HBOk = amap["Heartbeat"]->getbParam("HBOk");
    bool seenHB = amap["Heartbeat"]->getbParam("seenHB");
    bool seenInit = amap["Heartbeat"]->getbParam("seenInit");
    bool seenOk = amap["Heartbeat"]->getbParam("seenOk");
    bool seenFault = amap["Heartbeat"]->getbParam("seenFault");
    bool seenAlarm = amap["Heartbeat"]->getbParam("seenAlarm");

    // If we are in the init state wait for comms to start count down reset time
    if (currentHeartbeat == initHeartbeat)
    {
        // if not toally set up yet then quit this pass
        if (!amap["amHeartbeatInit"])
        {
            return 0;
        }

        if (!seenInit)  // Hb_Setup
        {
            if (1)
                FPS_ERROR_PRINT("%s >> %s  NO Heartbeat,  bypass [%s]\n", __func__, aname, BypassHb ? "true" : "false");

            amap["Heartbeat"]->setParam("seenInit", true);

            char* cval = (char*)"Hb Init, no Heartbeat Seen";
            amap["HeartbeatState"]->setVal(cval);

            ival = 1;
            amap["essHeartbeatInit"]->addVal(ival);
            amap["HeartbeatInit"]->setVal(0);  // Hb_Init
        }
    }
    else  // wait for comms to go past reset then set active or wait to alarm and
          // then fault
    {
        // if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n",
        // __func__, aname, lastHeartbeat?lastHeartbeat:"no
        // last Value", tval1)
        if (currentHeartbeat != lastHeartbeat)
        {
            if (1)
                FPS_ERROR_PRINT(
                    "%s >> %s Heartbeat change detected,  from [%d] to "
                    "[%d] tNow %2.3f seenHB [%s]\n",
                    __func__, aname, lastHeartbeat, currentHeartbeat, tNow, seenHB ? "true" : "false");

            amap["Heartbeat"]->setParam("lastHeartbeat", currentHeartbeat);

            // if(!seenHB)
            {
                if (0)
                    FPS_ERROR_PRINT("%s >> %s Heartbeat set HBseenTime %2.3f \n", __func__, aname, tNow);
                amap["Heartbeat"]->setParam("seenHB", true);
                amap["Heartbeat"]->setParam("HBseenTime", tNow);
                HBseenTime = tNow;
                seenHB = true;
            }
        }
        else  // No Change , start tracking faults and alarms  but wait for hold
              // time
        {
            HBseenTime = amap["Heartbeat"]->getdParam("HBseenTime");
            // allow holdoff between testing for change
            if (seenHB)
            {
                if ((tNow - HBseenTime) > toHold)
                {
                    if (0)
                        FPS_ERROR_PRINT(
                            "%s >> %s Heartbeat stall detected  tNow %2.3f "
                            "seebTime %2.3f .stalll time %2.3f toHold %2.3f \n",
                            __func__, aname, tNow, HBseenTime, (tNow - HBseenTime), toHold);

                    amap["Heartbeat"]->setParam("seenHB", false);
                    seenHB = false;
                    rdAlarm -= (tNow - HBseenTime);
                    rdFault -= (tNow - HBseenTime);

                    if (rdFault < 0.0)
                    {
                        rdFault = 0.0;
                    }
                    if (rdAlarm < 0.0)
                    {
                        rdAlarm = 0.0;
                    }
                    amap["Heartbeat"]->setParam("rdAlarm", rdAlarm);
                    amap["Heartbeat"]->setParam("rdFault", rdFault);
                }
            }
        }

        if (seenHB)
        {
            if (!seenOk)
            {
                if (rdReset > 0.0)
                {
                    rdReset -= tDiff;
                    rdReset = rdReset >= 0.0 ? rdReset : 0.0;
                    amap["Heartbeat"]->setParam("rdReset", rdReset);
                }
            }

            if (rdReset <= 0.0 && !seenOk)
            {
                if (seenFault)
                {
                    if (1)
                        FPS_ERROR_PRINT("%s >>  Heartbeat fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["Heartbeat"]->setParam("seenFault", false);
                    seenFault = false;
                }
                if (seenAlarm)
                {
                    if (1)
                        FPS_ERROR_PRINT("%s >>  Heartbeat Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["Heartbeat"]->setParam("seenAlarm", false);
                    seenAlarm = false;
                }
                amap["Heartbeat"]->setParam("seenOk", true);
                seenOk = true;
                amap["Heartbeat"]->setParam("HBOk", true);
                HBOk = true;

                if (1)
                    FPS_ERROR_PRINT("%s >>  Heartbeat OK for  %s at %2.3f\n", __func__, aname, tNow);
                ival = Asset_Ok;  // seen Heartbeat change
                amap["HeartbeatStateNum"]->setVal(ival);
                ival = 0;
                amap["HeartbeatInit"]->setVal(ival);
                char* tval;
                asprintf(&tval, " Hb OK last set  Alarm %3.2f max %3.2f", toAlarm, toFault);
                if (tval)
                {
                    amap["HeartbeatState"]->setVal(tval);
                    free((void*)tval);
                }
                amap["Heartbeat"]->setParam("rdReset", toReset);
            }
        }
        else  // not changed not onHold look out for errors
        {
            // we need to decrement the alarm / fault times
            rdFault = amap["Heartbeat"]->getdParam("rdFault");
            rdAlarm = amap["Heartbeat"]->getdParam("rdAlarm");
            seenFault = amap["Heartbeat"]->getbParam("seenFault");
            seenAlarm = amap["Heartbeat"]->getbParam("seenAlarm");
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>  Heartbeat stall for  %s at %2.3f rdFault %2.3f "
                    "rdAlarm %2.3f HBOk [%s] seenHB [%s] tDiff %2.3f \n",
                    __func__, aname, tNow, rdFault, rdAlarm, amap["HeartbeaetOK"] ? "true" : "false",
                    seenHB ? "true" : "false", tDiff);
            if (rdFault > 0.0)
            {
                rdFault -= tDiff;
                rdFault = rdFault > 0.0 ? rdFault : 0.0;
                amap["Heartbeat"]->setParam("rdFault", rdFault);
            }
            if (rdAlarm > 0.0)
            {
                rdAlarm -= tDiff;
                rdAlarm = rdAlarm > 0.0 ? rdAlarm : 0.0;
                amap["Heartbeat"]->setParam("rdAlarm", rdAlarm);
            }

            if (rdFault < 0.0 && !seenFault)
            {
                seenFault = true;
                amap["Heartbeat"]->setParam("seenFault", true);
                amap["Heartbeat"]->setParam("seenOk", false);
                amap["Heartbeat"]->setParam("seenAlarm", true);

                if (1)
                    FPS_ERROR_PRINT("%s >>  Heartbeat  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                char* tval;
                asprintf(&tval, " Hb Fault last set Alarm %3.2f max %3.2f", toAlarm, toFault);
                if (tval)
                {
                    amap["HeartbeatState"]->setVal(tval);
                    free((void*)tval);
                }
                ival = Asset_Fault;  // Heartbeat Fault
                amap["HeartbeatStateNum"]->setVal(ival);
                // seenOk = false;
                seenAlarm = true;

                int totalHbFaults = amap["Heartbeat"]->getiParam("totalHbFaults");
                totalHbFaults++;
                amap["Heartbeat"]->setParam("totalHbFaults", totalHbFaults);

                amap["HeartbeatOK"]->setVal(false);

                if (am->am)
                {
                    ival = 1;
                    amap["amHeartbeatFaultCnt"]->addVal(ival);
                }
            }
            else if (rdAlarm < 0.0 && !seenAlarm)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> Heartbeat  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                char* tval;
                asprintf(&tval, "Hb Alarm last set Alarm %3.2f max %3.2f", toAlarm, toFault);
                if (tval)
                {
                    amap["HeartbeatState"]->setVal(tval);
                    free((void*)tval);
                }
                // Just test code right now
                ival = Asset_Alarm;  // Heartbeat Alarm
                amap["HeartbeatStateNum"]->setVal(ival);

                amap["Heartbeat"]->setParam("seenAlarm", true);
                seenAlarm = true;
                amap["Heartbeat"]->setParam("seenOk", false);
                int totalHbAlarms = amap["Heartbeat"]->getiParam("totalHbAlarms");
                totalHbAlarms++;
                amap["Heartbeat"]->setParam("totalHbAlarms", totalHbAlarms);

                amap["HeartbeatOK"]->setVal(false);

                if (am->am)
                {
                    amap["amHeartbeatAlarmCnt"]->addVal(ival);
                }
            }
            else
            {
                if (0)
                    FPS_ERROR_PRINT(
                        "%s >> Hb for [%s] [%s] Stalled at %2.3f  Fault "
                        "%2.3f Alarm %2.3f \n",
                        __func__, aname, amap["Heartbeat"]->getcVal(), tNow, rdFault, rdAlarm);
            }
        }
        if (seenFault)
        {
            int ival = 1;
            amap["HeartbeatFaultCnt"]->addVal(ival);
            amap["essHeartbeatFaultCnt"]->addVal(ival);
        }
        else
        {
            if (seenHB)
            {
                if (rdFault < toFault)
                {
                    rdFault += tDiff;
                    rdFault = rdFault <= toFault ? rdFault : toFault;
                    amap["Heartbeat"]->setParam("rdFault", rdFault);
                }
            }
        }

        if (seenAlarm)
        {
            int ival = 1;
            amap["HeartbeatAlarmCnt"]->addVal(ival);
            amap["essHeartbeatAlarmCnt"]->addVal(ival);
        }
        else
        {
            if (seenHB)
            {
                if (rdAlarm < toAlarm)
                {
                    rdAlarm += tDiff;
                    rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
                    amap["Heartbeat"]->setParam("rdAlarm", rdAlarm);
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
};
