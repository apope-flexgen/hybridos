#ifndef CHECKAMHEARTBEAT_CPP
#define CHECKAMHEARTBEAT_CPP
#include "asset.h"

//CheckAMHeartbeat.cpp

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
 * toReset time after changes start being seen again before resetting faults and Alarms
 *
 */
int CheckAmHeartbeat(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    //double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval =  (char*) "Heartbeat Init";
    asset_manager* am = av->am;
    VarMapUtils* vm = am->vm;
    char* essName = vm->getSysName(vmap);
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAmHeartbeat");
    //assetVar* CheckAssetComms = amap["CheckAmComms"];
    double toHold = 1.5;  // Seconds between HB changes
    double toAlarm = 2.5;
    double toFault = 6.0;
    double toReset = 2.5;
    int initHeartbeat = -1;//(char *)" Initial Heartbeat";

    
    //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2) 
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;
        //Link This to an incoming component
        if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
        
        amap["HeartbeatRead"]            = vm->setLinkVal(vmap, aname,                "/status", "HeartbeatRead",          initHeartbeat);
        if (1)FPS_ERROR_PRINT("\n\n%s >>  aname [%s] Heartbeat %p comp [%s] name [%s] \n"
                , __func__
                , aname
                , amap["HeartbeatRead"]
                , amap["HeartbeatRead"]->comp.c_str()
                , amap["HeartbeatRead"]->name.c_str()
                );

        amap["essHeartbeatFaultCnt"]     = vm->setLinkVal(vmap, essName,                "/status",     "essHeartbeatFaultCnt",         ival);
        amap["essHeartbeatAlarmCnt"]     = vm->setLinkVal(vmap, essName,                "/status",     "essHeartbeatAlarmCnt",         ival);
        amap["essHeartbeatInit"]         = vm->setLinkVal(vmap, essName,                "/status",     "essHeartbeatInit",             ival);
        amap["essHeartbeatFaultTimeout"] = vm->setLinkVal(vmap, essName,                "/config",     "essHeartbeatFaultTimeout",     toFault);
        amap["essHeartbeatAlarmTimeout"] = vm->setLinkVal(vmap, essName,                "/config",     "essHeartbeatAlarmTimeout",     toAlarm);
        amap["essHeartbeatResetTimeout"] = vm->setLinkVal(vmap, essName,                "/config",     "essHeartbeatResetTimeout",     toReset);
        amap["essHeartbeatHoldTimeout"]  = vm->setLinkVal(vmap, essName,                "/config",     "essHeartbeatHoldTimeout",      toHold);

        if(am->am)
        {
            amap["amHeartbeatFaultCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartbeatFaultCnt",             ival);
            amap["amHeartbeatAlarmCnt"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartbeatAlarmCnt",             ival);
            amap["amHeartbeatInit"]      = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartbeatInit",                 ival);
        }

        amap["HeartbeatFaultCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatFaultCnt",            ival);
        amap["HeartbeatAlarmCnt"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatAlarmCnt",            ival);
        amap["HeartbeatInit"]            = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatInit",                ival);
        amap["HeartbeatState"]           = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatState",               cval);
        amap["BypassHeartbeat"]          = vm->setLinkVal(vmap, aname,                "/config",     "BypassHeartbeat",              bval);
        amap["HeartbeatStateNum"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatStateNum",            ival);
        amap["HeartbeatOK"]              = vm->setLinkVal(vmap, aname,                "/status",     "HeartbeatOK",                  bval);

        
        if(reload == 0) // complete restart 
        {
            amap["HeartbeatRead"]     ->setVal(initHeartbeat);
            //lastHeartbeat=strdup(tsInit);//state"]->setVal(cval);
            amap["HeartbeatRead"]     ->setParam("lastHeartbeat", initHeartbeat);
            amap["HeartbeatRead"]     ->setParam("totalHbFaults", 0);
            amap["HeartbeatRead"]     ->setParam("totalHbAlarms", 0);

            amap["HeartbeatRead"]     ->setParam("seenFault", false);
            amap["HeartbeatRead"]     ->setParam("seenAlarm", false);
            amap["HeartbeatRead"]     ->setParam("seenOk", false);
            amap["HeartbeatRead"]     ->setParam("seenHB", false);

            //amap["HeartbeatRead"]     ->setParam("HBOk", false);
            amap["HeartbeatRead"]->setParam("HBOk", false);
            dval = 0.0;
            amap["HeartbeatRead"]     ->setParam("HBseenTime", dval);
            amap["HeartbeatRead"]     ->setParam("seenInit", false);
            amap["HeartbeatRead"]     ->setParam("initCnt", -1);

            amap["HeartbeatRead"]     ->setParam("rdFault", toFault);                      // time remaining before fault
            amap["HeartbeatRead"]     ->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["HeartbeatRead"]     ->setParam("rdReset", toReset);                      // time remaining before reset
            //amap["HeartbeatRead"]     ->setParam("rdHold", toHold);                        // time to wait before no change test
            amap["HeartbeatRead"]     ->setParam("tLast", dval);                         // time when last to event was seen

            amap["HeartbeatState"]    ->setVal(cval);
            ival = Asset_Init; amap["HeartbeatStateNum"]  ->setVal(ival);
            ival = -1; amap["HeartbeatInit"]  ->setVal(ival);
            amap["BypassHeartbeat"]  ->setVal(false);
            
            amap["essHeartbeatFaultCnt"]->setParam("lastHbFaults",0);
            amap["essHeartbeatAlarmCnt"]->setParam("lastHbAlarms",0);
        }
        // reset reload
        ival = 2; amap["CheckAmHeartbeat"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();
    double tLast    = amap["HeartbeatRead"]->getdParam("tLast");
    if(tLast == 0.0)
        tLast = tNow;
    double tDiff = tNow - tLast;
    amap["HeartbeatRead"]->setParam("tLast", tNow);
 
    bool BypassHb = amap["BypassHeartbeat"]->getbVal();

    toFault = amap["essHeartbeatFaultTimeout"]->getdVal();
    toAlarm = amap["essHeartbeatAlarmTimeout"]->getdVal();
    toReset = amap["essHeartbeatResetTimeout"]->getdVal();
    toHold = amap["essHeartbeatHoldTimeout"]->getdVal();

    int currentHeartbeat = amap["HeartbeatRead"]->getiVal();
    int lastHeartbeat    = amap["HeartbeatRead"]->getiParam("lastHeartbeat");//amap["lastHeartBeat"]->getiVal();
    if (0) FPS_ERROR_PRINT("\n\n%s >>  HB state  for %s at time %2.3f  current %d  last %d \n", __func__, aname, tNow, currentHeartbeat, lastHeartbeat);
    if (0) FPS_ERROR_PRINT("%s >>  aname [%s] Heartbeat %p comp [%s] name [%s] \n"
            , __func__
            , aname
            , amap["HeartbeatRead"]
            , amap["HeartbeatRead"]->comp.c_str()
            , amap["HeartbeatRead"]->name.c_str()
            );
    // are we the ess_controller 
    if(!am->am)
    {
        //bool initSeen =             amap["HeartbeatRead"]     ->getbParam("initSeen");

        amap["essHeartbeatFaultCnt"]  ->setVal(0);
        amap["essHeartbeatAlarmCnt"]  ->setVal(0);
        amap["essHeartbeatInit"]    ->setVal(0);

        int initCnt = amap["HeartbeatRead"]->getiParam("initCnt");   
        int icnt = 0;
        for (auto ix : am->assetManMap)
        {
            assetVar aV;
            asset_manager * amc = ix.second;
            if (strcmp(amc->name.c_str(), "pcs") != 0)
            {
                aV.am = amc;
                CheckAmHeartbeat(vmap, amc->amap, amc->name.c_str(), p_fims, &aV);
                icnt++;
            }
        }

        int essHbFaults = amap["essHeartbeatFaultCnt"]->getiVal();
        int essHbAlarms = amap["essHeartbeatAlarmCnt"]->getiVal();
        int lastHbAlarms = amap["essHeartbeatAlarmCnt"]->getiParam("lastHbAlarms");
        int lastHbFaults = amap["essHeartbeatFaultCnt"]->getiParam("lastHbFaults");
        
        //int essHbInit = amap["essHbInit"]->getiVal();
        if(essHbFaults != lastHbFaults)
        {
            amap["essHeartbeatFaultCnt"]->setParam("lastHbFaults",essHbFaults);

            if(essHbFaults> 0) 
            {
                FPS_ERROR_PRINT("%s >> %d essHbFaults detected at time %2.3f \n", __func__, essHbFaults, tNow);
            }
            else
            {
                FPS_ERROR_PRINT("%s >> %d essHbFaults cleared at time %2.3f\n", __func__, essHbFaults, tNow);
            }
        }
        if(essHbAlarms != lastHbAlarms)
        {
            amap["essHeartbeatAlarmCnt"]->setParam("lastHbAlarms",essHbAlarms);

            if(essHbAlarms> 0)
            {
                FPS_ERROR_PRINT("%s >> %d essHbAlarms detected at time %2.3f \n", __func__, essHbAlarms, tNow);
            }
            else
            {
                FPS_ERROR_PRINT("%s >> %d essHbAlarms cleared at time %2.3f\n", __func__, essHbAlarms, tNow);
            }
        }

        if(initCnt  !=  icnt)
        {
            amap["HeartbeatRead"]     ->setParam("initCnt", icnt);

            FPS_ERROR_PRINT("%s >> icnt %d intiSeen %d  change detected\n", __func__, icnt, initCnt);
        }
        return 0;
    }


    // this is the Asset Manager under the ess_controller instance
    if(BypassHb)
    {
        ival = 1;
        amap["essHeartbeatInit"]->addVal(ival);

        // Do we set heartbeat OK here?
        return 0;
    }
    double rdFault = amap["HeartbeatRead"] ->getdParam("rdFault");
    double rdAlarm = amap["HeartbeatRead"] ->getdParam("rdAlarm");
    double rdReset = amap["HeartbeatRead"] ->getdParam("rdReset");

    double HBseenTime = amap["HeartbeatRead"] ->getdParam("HBseenTime");
    //bool HBOk = amap["HeartbeatRead"] ->getbParam("HBOk");
    //bool HBOk = amap["HeartbeatRead"]->getbParam("HBOk");
    bool seenHB = amap["HeartbeatRead"] ->getbParam("seenHB");
    bool seenInit = amap["HeartbeatRead"]->getbParam("seenInit");
    bool seenOk = amap["HeartbeatRead"]->getbParam("seenOk");
    bool seenFault = amap["HeartbeatRead"]->getbParam("seenFault");
    bool seenAlarm = amap["HeartbeatRead"]->getbParam("seenAlarm");
    
    // If we are in the init state wait for comms to start count down reset time
    if (currentHeartbeat == initHeartbeat)    
    {
        // if not toally set up yet then quit this pass
        if(!amap["amHeartbeatInit"])
        {
            return 0;
        }

        if (!seenInit)   // Hb_Setup
        {
            if(1)FPS_ERROR_PRINT("%s >> %s  NO Heartbeat,  bypass [%s]\n", __func__, aname, BypassHb?"true":"false");

            amap["HeartbeatRead"] ->setParam("seenInit",true);

            char* cval = (char *)"Hb Init, no Heartbeat Seen";
            amap["HeartbeatState"]->setVal(cval);

            ival = 1;
            amap["essHeartbeatInit"]->addVal(ival);
            amap["HeartbeatInit"]->setVal(0);      //Hb_Init  
        }

    }
    else  // wait for comms to go past reset then set active or wait to alarm and then fault
    {
        //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastHeartbeat?lastHeartbeat:"no last Value", tval1)
        if (currentHeartbeat != lastHeartbeat) 
        {

            if (0)FPS_ERROR_PRINT("%s >> %s Heartbeat change detected,  from [%d] to [%d] tNow %2.3f seenHB [%s]\n"
                , __func__, aname, lastHeartbeat, currentHeartbeat, tNow, seenHB ? "true" : "false");

            amap["HeartbeatRead"]     ->setParam("lastHeartbeat", currentHeartbeat);

            //if(!seenHB)
            {
                if(0)FPS_ERROR_PRINT("%s >> %s Heartbeat set HBseenTime %2.3f \n"
                        , __func__, aname, tNow);
                amap["HeartbeatRead"] ->setParam("seenHB", true);
                amap["HeartbeatRead"] ->setParam("HBseenTime", tNow);
                HBseenTime = tNow;
                seenHB = true;
            }

        }
        else   // No Change , start tracking faults and alarms  but wait for hold time
        {
            HBseenTime = amap["HeartbeatRead"] ->getdParam("HBseenTime");
            // allow holdoff between testing for change
            if(seenHB)
            {
                if ((tNow - HBseenTime) > toHold)
                {
                    if(1)FPS_ERROR_PRINT("%s >> %s Heartbeat stall detected  tNow %2.3f seebTime %2.3f .stalll time %2.3f toHold %2.3f \n"
                            , __func__, aname, tNow, HBseenTime, (tNow - HBseenTime), toHold);

                    amap["HeartbeatRead"] ->setParam("seenHB", false);
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
                    amap["HeartbeatRead"]->setParam("rdAlarm",rdAlarm);
                    amap["HeartbeatRead"]->setParam("rdFault",rdFault);

                }
            }
        }

        if(seenHB)
        {
            if(!seenOk)
            {
                if(rdReset > 0.0)
                {
                    rdReset -= tDiff;
                    rdReset = rdReset >= 0.0 ? rdReset : 0.0;
                    amap["HeartbeatRead"]->setParam("rdReset",rdReset);
                }
            }

            if (rdReset <= 0.0  && !seenOk)
            {
                if(seenFault)
                {
                    if(1)FPS_ERROR_PRINT("%s >>  Heartbeat fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["HeartbeatRead"] ->setParam("seenFault",false);
                    seenFault = false;
                }
                if(seenAlarm)
                {
                    if(1)FPS_ERROR_PRINT("%s >>  Heartbeat Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["HeartbeatRead"] ->setParam("seenAlarm",false);
                    seenAlarm = false;
                }
                amap["HeartbeatRead"] ->setParam("seenOk",true);
                seenOk = true;
                //amap["HeartbeatRead"] ->setParam("HBOk",true);
                //HBOk = true;
                amap["HeartbeatOK"]->setVal(true);

                if(1)FPS_ERROR_PRINT("%s >>  Heartbeat OK for  %s at %2.3f\n", __func__, aname, tNow);
                ival = Asset_Ok; // seen Heartbeat change
                amap["HeartbeatStateNum"]  ->setVal(ival);
                ival = 0;
                amap["HeartbeatInit"]->setVal(ival);
                char *tval = nullptr;
                asprintf(&tval," Hb OK last set  Alarm %3.2f max %3.2f", toAlarm, toFault);
                if(tval)
                {
                    amap["HeartbeatState"]->setVal(tval);
                    free(tval); tval = nullptr;
                }   
                amap["HeartbeatRead"]->setParam("rdReset",toReset);
                if (!amap["FaultShutdown"])
                {
                    bool bval = false;
                    amap["FaultShutdown"]          = vm->setLinkVal(vmap, essName,                "/status",     "FaultShutdown",     bval);
                }
                amap["FaultShutdown"]->setVal(false);       
            }
        }
        else  // not changed not onHold look out for errors
        {
            // we need to decrement the alarm / fault times
            rdFault = amap["HeartbeatRead"] ->getdParam("rdFault");
            rdAlarm = amap["HeartbeatRead"] ->getdParam("rdAlarm");
            seenFault = amap["HeartbeatRead"] ->getbParam("seenFault");
            seenAlarm = amap["HeartbeatRead"] ->getbParam("seenAlarm");
            if (rdFault > 0.0)
            {
                if(1)FPS_ERROR_PRINT("%s >>  Heartbeat stall for  %s at %2.3f rdFault %2.3f rdAlarm %2.3f HBOk [%s] seenHB [%s] tDiff %2.3f \n"
                                            , __func__, aname, tNow, rdFault, rdAlarm, amap["HeartbeaetOK"]?"true":"false", seenHB?"true":"false", tDiff );
                rdFault -= tDiff;
                rdFault = rdFault > 0.0 ? rdFault : 0.0;
                amap["HeartbeatRead"]->setParam("rdFault",rdFault);
            }
            if (rdAlarm > 0.0)
            {
                rdAlarm -= tDiff;
                rdAlarm = rdAlarm > 0.0 ? rdAlarm : 0.0;
                amap["HeartbeatRead"]->setParam("rdAlarm",rdAlarm);
            }

            if (rdFault <= 0.0  && !seenFault)
            {
                seenFault = true;
                amap["HeartbeatRead"]->setParam("seenFault", true);
                amap["HeartbeatRead"]->setParam("seenOk", false);
                amap["HeartbeatRead"]->setParam("seenAlarm", true);

                if(1)FPS_ERROR_PRINT("%s >>  Heartbeat  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                char *tval = nullptr;
                asprintf(&tval," Hb Fault last set Alarm %3.2f max %3.2f", toAlarm, toFault);
                if(tval)
                {
                    amap["HeartbeatState"]->setVal(tval);
                    free(tval); tval = nullptr;
                }
                ival = Asset_Fault; //Heartbeat Fault
                amap["HeartbeatStateNum"]  ->setVal(ival);
                //seenOk = false;
                seenAlarm =  true;

                int totalHbFaults = amap["HeartbeatRead"]->getiParam("totalHbFaults");
                totalHbFaults++;
                amap["HeartbeatRead"]->setParam("totalHbFaults",totalHbFaults);

                //HBOk = false;
                amap["HeartbeatOK"]->setVal(false);
                //amap["HeartbeatRead"] ->setParam("HBOk",false);
            
                if(am->am)
                {
                    ival = 1;
                    amap["amHeartbeatFaultCnt"]->addVal(ival);
                }
                if (!amap["FaultShutdown"])
                {
                    bool bval = true;
                    // May need to change essName to "pcs" or "bms" if other asset managers can also initiate shutdown process
                    // Right now, ShutdownSystem function is called from ess wake up 
                    amap["FaultShutdown"]          = vm->setLinkVal(vmap, essName,                "/status",     "FaultShutdown",     bval);
                }
                amap["FaultShutdown"]->setVal(true);
                    
            }
            else if (rdAlarm <= 0.0  && !seenAlarm)
            {
                if(1)FPS_ERROR_PRINT("%s >> Heartbeat  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                char *tval;
                asprintf(&tval,"Hb Alarm last set Alarm %3.2f max %3.2f", toAlarm, toFault);
                if(tval)
                {
                    amap["HeartbeatState"]->setVal(tval);
                    free((void *)tval);
                }
                // Just test code right now
                ival = Asset_Alarm; //Heartbeat Alarm
                amap["HeartbeatStateNum"]  ->setVal(ival);

                amap["HeartbeatRead"]->setParam("seenAlarm", true);
                seenAlarm = true;
                amap["HeartbeatRead"]->setParam("seenOk", false);
                int totalHbAlarms = amap["HeartbeatRead"]->getiParam("totalHbAlarms");
                totalHbAlarms++;
                amap["HeartbeatRead"]->setParam("totalHbAlarms",totalHbAlarms);

                //HBOk = false;
                amap["HeartbeatOK"]->setVal(false);
                //amap["HeartbeatRead"] ->setParam("HBOk",false);

                if(am->am)
                {
                    amap["amHeartbeatAlarmCnt"]->addVal(ival);
                }
            }
            else
            {
                if(0)FPS_ERROR_PRINT("%s >> Hb for [%s] [%s] Stalled at %2.3f  Fault %2.3f Alarm %2.3f \n"
                        , __func__
                        , aname
                        , amap["HeartbeatRead"]->getcVal()
                        , tNow 
                        , rdFault, rdAlarm);

            }    
        }
        if(seenFault)
        {
            int ival = 1 ; 
            amap["HeartbeatFaultCnt"]->addVal(ival);
            amap["essHeartbeatFaultCnt"]->addVal(ival);
        }
        else
        {
            if(seenHB)
            {
                if (rdFault < toFault)
                {
                    rdFault += tDiff;
                    rdFault = rdFault <= toFault ? rdFault : toFault;
                    amap["HeartbeatRead"]->setParam("rdFault",rdFault);
                }
            }
        }

        if(seenAlarm)
        {
            int ival = 1 ; 
            amap["HeartbeatAlarmCnt"]->addVal(ival);
            amap["essHeartbeatAlarmCnt"]->addVal(ival);
        }
        else
        {
            if(seenHB)
            {
                if (rdAlarm < toAlarm)
                {   
                    rdAlarm += tDiff;
                    rdAlarm = rdAlarm <= toAlarm ? rdAlarm : toAlarm;
                    amap["HeartbeatRead"]->setParam("rdAlarm",rdAlarm);
                }
            }
        }
    }
    //
    //int ival1, ival2;
    //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
    return 0;
};
#endif
