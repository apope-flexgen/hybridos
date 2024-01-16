#ifndef CHECKESSSTATUS_CPP 
#define CHECKESSSTATUS_CPP 

#include "asset.h"

extern "C++" {
    int CheckAmBmsStatus(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}
// test status against ExpStatus 
// logs any changes
int CheckAmPcsStatus(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    //double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval = (char*)"PcsStatus Init";
    asset_manager* am = av->am;

    VarMapUtils* vm = am->vm;
    char* essName  = vm->getSysName(vmap);

    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAmPcsStatus");
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
        if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);

        amap["PcsStatus"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatus", initPcsStatus);
        amap["PcsExpStatus"] = vm->setLinkVal(vmap, aname, "/status", "PcsExpStatus", initPcsStatus);
        if (1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
            , __func__
            , aname
            , amap["PcsStatus"]->comp.c_str()
            , amap["PcsStatus"]->name.c_str()
        );

        amap["essPcsStatusFaults"] = vm->setLinkVal(vmap, essName, "/status", "essPcsStatusFaults", ival);
        amap["essPcsStatusAlarms"] = vm->setLinkVal(vmap, essName, "/status", "essPcsStatusAlarms", ival);
        amap["essPcsStatusInit"] = vm->setLinkVal(vmap, essName, "/status", "essPcsStatusInit", ival);
        amap["essPcsStatusTimeoutFault"] = vm->setLinkVal(vmap, essName, "/config", "essPcsStatusTimeoutFault", toFault);
        amap["essPcsStatusTimeoutAlarm"] = vm->setLinkVal(vmap, essName, "/config", "essPcsStatusTimeoutAlarm", toAlarm);
        amap["essPcsStatusTimeoutReset"] = vm->setLinkVal(vmap, essName, "/config", "essPcsStatusTimeoutReset", toReset);

        if (am->am)
        {
            amap["amPcsStatusFaults"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "PcsStatusFaults", ival);
            amap["amPcsStatusAlarms"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "PcsStatusAlarms", ival);
            amap["amPcsStatusInit"] = vm->setLinkVal(vmap, am->am->name.c_str(), "/status", "PcsStatusInit", ival);
        }

        amap["PcsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatusFaults", ival);
        amap["PcsStatusAlarms"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatusAlarms", ival);
        amap["PcsStatusInit"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatusInit", ival);
        amap["PcsStatusState"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatusState", cval);
        amap["BypassPcsStatus"] = vm->setLinkVal(vmap, aname, "/config", "BypassPcsStatus", bval);
        amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status", "AssetState", ival);
        amap["PcsStatusStateNum"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatusStateNum", ival);


        if (reload == 0) // complete restart 
        {
            amap["PcsStatus"]->setVal(initPcsStatus);
            //lastPcsStatus=strdup(tsInit);//state"]->setVal(cval);
            amap["PcsStatus"]->setParam("lastPcsStatus", initPcsStatus);
            amap["PcsStatus"]->setParam("totalPcsStatusFaults", 0);
            amap["PcsStatus"]->setParam("totalPcsStatusAlarms", 0);
            amap["PcsStatus"]->setParam("seenFault", false);
            amap["PcsStatus"]->setParam("seenOk", false);
            amap["PcsStatus"]->setParam("seenAlarm", false);
            amap["PcsStatus"]->setParam("seenInit", false);
            amap["PcsStatus"]->setParam("initCnt", -1);

            amap["PcsStatus"]->setParam("rdFault", toFault);                      // time remaining before fault
            amap["PcsStatus"]->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["PcsStatus"]->setParam("rdReset", toReset);                      // time remaining before reset
            amap["PcsStatus"]->setParam("rdLast", dval);                         // time when last to event was seen

            amap["PcsStatusState"]->setVal(cval);
            ival = Asset_Init; amap["PcsStatusStateNum"]->setVal(ival);
            ival = -1; amap["PcsStatusInit"]->setVal(ival);
            amap["BypassPcsStatus"]->setVal(false);

        }
        // reset reload
        ival = 2; amap["CheckAmPcsStatus"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();

    bool BypassPcsStatus = amap["BypassPcsStatus"]->getbVal();

    toFault = amap["essPcsStatusTimeoutFault"]->getdVal();
    toAlarm = amap["essPcsStatusTimeoutAlarm"]->getdVal();
    toReset = amap["essPcsStatusTimeoutReset"]->getdVal();

    int currentPcsStatus = amap["PcsStatus"]->getiVal();
    int expectedPcsStatus = amap["PcsExpStatus"]->getiVal();
    int lastPcsStatus = amap["PcsStatus"]->getiParam("lastPcsStatus");//amap["lastHeartBeat"]->getiVal();

    if (BypassPcsStatus)
    {
        ival = 1;
        amap["essPcsStatusInit"]->addVal(ival);
        return 0;

    }
    // If we are in the init state wait for comms to start count down reset time
    if (currentPcsStatus == initPcsStatus)
    {
        bool seenInit = amap["PcsStatus"]->getbParam("seenInit");

        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        if (0)FPS_ERROR_PRINT("%s >> %s  NO PcsStatus,  bypass [%s]\n", __func__, aname, BypassPcsStatus ? "true" : "false");

        // if not toally set up yet then quit this pass
        if (!amap["amPcsStatusInit"])
        {
            return 0;
        }

        if (!seenInit)   // PcsStatus_Setup
        {
            amap["PcsStatus"]->setParam("seenInit", true);

            char* cval = (char*)"PcsStatus Init, no PcsStatus Seen";
            amap["PcsStatusState"]->setVal(cval);

            ival = 1;
            amap["essPcsStatusInit"]->addVal(ival);
            amap["PcsStatusInit"]->setVal(0);      //PcsStatus_Init  
        }
        amap["PcsStatus"]->setParam("rdLast", tNow);

    }
    else  // wait for comms to go past reset then set active or wait to alarm and then fault
    {
        //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastPcsStatus?lastPcsStatus:"no last Value", tval1);
        double rdLast = amap["PcsStatus"]->getdParam("rdLast");
        double rdFault = amap["PcsStatus"]->getdParam("rdFault");
        double rdAlarm = amap["PcsStatus"]->getdParam("rdAlarm");
        double rdReset = amap["PcsStatus"]->getdParam("rdReset");
        amap["PcsStatus"]->setParam("rdLast", tNow);

        double toVal = amap["PcsStatus"]->getLastSetDiff(tNow);

        // Has value changed ? If yes then count down rdReset to zero based on tNow - rdLast
        if (currentPcsStatus != lastPcsStatus)
        {
            amap["PcsStatus"]->setParam("lastPcsStatus", currentPcsStatus);
// TODO PCSStatus / BmsStatus .. log changes After MVP
        }
        if (currentPcsStatus == expectedPcsStatus)
            //if(amap["PcsStatus"]->valueChangedReset())
        {
            amap["PcsStatus"]->setParam("lastPcsStatus", currentPcsStatus);

            bool seenOk = amap["PcsStatus"]->getbParam("seenOk");
            if (rdReset > 0.0)
            {
                rdReset -= (tNow - rdLast);
                amap["PcsStatus"]->setParam("rdReset", rdReset);
            }
            //else
            {
// TODO  review after MVP rdAlsrm rdFault after reset increment these up to toAlarm , toFault 
                if (rdAlarm < toAlarm)
                {
                    rdAlarm += tNow - rdLast;
                    amap["PcsStatus"]->setParam("rdAlarm", rdAlarm);
                }
                if (rdFault < toFault)
                {
                    rdFault += tNow - rdLast;
                    amap["PcsStatus"]->setParam("rdFault", rdFault);
                }
            }

            if (0)FPS_ERROR_PRINT("%s >>  PcsStatus change for %s from [%d] to [%d]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault %2.3f\n"
                , __func__, aname, lastPcsStatus, currentPcsStatus, rdReset, (tNow - rdLast), rdAlarm, rdFault);

            ival = amap["PcsStatusStateNum"]->getiVal();
            // reset time passed , still changing , time to switch to PcsStatus_Ready
            if ((rdReset <= 0.0) && (ival != seenOk))
            {

                bool seenFault = amap["PcsStatus"]->getbParam("seenFault");
                //bool seenOk  = amap["PcsStatus"]->getbParam("seenOk");
                bool seenAlarm = amap["PcsStatus"]->getbParam("seenAlarm");
                amap["PcsStatus"]->setParam("seenOk", true);

                if (0)FPS_ERROR_PRINT("%s >>  PcsStatus  change for %s from [%d] to [%d] \n", __func__, aname, lastPcsStatus, currentPcsStatus);
                if (seenFault)
                {
                    if (1)FPS_ERROR_PRINT("%s >>  PcsStatus fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["PcsStatus"]->setParam("seenFault", false);

                }
                if (seenAlarm)
                {
                    if (1)FPS_ERROR_PRINT("%s >>  PcsStatus Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                    amap["PcsStatus"]->setParam("seenAlarm", false);

                }
                if (1)FPS_ERROR_PRINT("%s >>  PcsStatus OK for  %s at %2.3f\n", __func__, aname, tNow);
                ival = Asset_Ok; // seen PcsStatus change
                amap["PcsStatusStateNum"]->setVal(ival);
                ival = 0;
                amap["PcsStatusInit"]->setVal(ival);
                char* tval = nullptr;
                asprintf(&tval, " PcsStatus OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if (tval)
                {
                    amap["PcsStatusState"]->setVal(tval);
                    free(tval); tval = nullptr;
                }
            }

            // increment alarm and fault time reset time
            if (rdFault < toFault)
            {
                rdFault += (tNow - rdLast);
                amap["PcsStatus"]->setParam("rdFault", rdFault);
            }
            if (rdAlarm < toAlarm)
            {
                rdAlarm += (tNow - rdLast);
                amap["PcsStatus"]->setParam("rdAlarm", rdAlarm);
            }

            //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
            amap["PcsStatus"]->setParam("lastPcsStatus", currentPcsStatus);
            //if ((toVal > toFault)  && !bokFault && !bypass)

        }
        else   // No Change , start tracking faults and alarms
        {
            bool seenFault = amap["PcsStatus"]->getbParam("seenFault");
            //bool seenOk  = amap["PcsStatus"]->getbParam("seenOk");
            bool seenAlarm = amap["PcsStatus"]->getbParam("seenAlarm");
            if (rdFault > 0.0)
            {
                rdFault -= (tNow - rdLast);
                amap["PcsStatus"]->setParam("rdFault", rdFault);
            }
            if (rdAlarm > 0.0)
            {
                rdAlarm -= (tNow - rdLast);
                amap["PcsStatus"]->setParam("rdAlarm", rdAlarm);
            }
            if (rdReset < toReset)
            {
                rdReset += (tNow - rdLast);
                amap["PcsStatus"]->setParam("rdReset", rdReset);
            }

            if ((rdFault <= 0.0) && !seenFault)
            {

                if (1)FPS_ERROR_PRINT("%s >>  PcsStatus  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                char* tval = nullptr;
                asprintf(&tval, " PcsStatus Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if (tval)
                {
                    amap["PcsStatusState"]->setVal(tval);
                    free(tval); tval = nullptr;
                }
                int ival = 1;
                amap["PcsStatusFaults"]->addVal(ival);
                amap["essPcsStatusFaults"]->addVal(ival);

                if (am->am)
                {
                    amap["amPcsStatusFaults"]->addVal(ival);
                }

                ival = Asset_Fault; //PcsStatus Fault
                amap["PcsStatusStateNum"]->setVal(ival);

                seenFault = true;
                amap["PcsStatus"]->setParam("seenFault", true);
                amap["PcsStatus"]->setParam("seenOk", false);
                amap["PcsStatus"]->setParam("seenAlarm", true);
                //seenOk = false;
                seenAlarm = false;

                int totalPcsStatusFaults = amap["PcsStatus"]->getiParam("totalPcsStatusFaults");
                totalPcsStatusFaults++;
                amap["PcsStatus"]->setParam("totalPcsStatusFaults", totalPcsStatusFaults);

            }
            else if ((rdAlarm <= 0.0) && !seenAlarm)
            {
                if (1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                char* tval = nullptr;
                asprintf(&tval, "PcsStatus Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if (tval)
                {
                    amap["PcsStatusState"]->setVal(tval);
                    free(tval); tval = nullptr;
                }

                int ival = 1;
                amap["PcsStatusAlarms"]->addVal(ival);
                amap["essPcsStatusAlarms"]->addVal(ival);

                if (am->am)
                {
                    amap["amPcsStatusAlarms"]->addVal(ival);
                }
                ival = Asset_Alarm; //PcsStatus Alarm
                amap["PcsStatusStateNum"]->setVal(ival);

                amap["PcsStatus"]->setParam("seenAlarm", true);
                //amap["PcsStatus"]->setParam("seenFault", false);
                amap["PcsStatus"]->setParam("seenOk", false);
                int totalPcsStatusAlarms = amap["PcsStatus"]->getiParam("totalPcsStatusAlarms");
                totalPcsStatusAlarms++;
                amap["PcsStatus"]->setParam("totalPcsStatusAlarms", totalPcsStatusAlarms);
            }
            else
            {
                if (0)FPS_ERROR_PRINT("%s >> PcsStatus for [%s] [%s] Stalled at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
                    , __func__
                    , aname
                    , amap["PcsStatus"]->getcVal()
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

int CheckAmBmsStatus(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    //double dval = 0.0;
    int ival = 0;
    bool bval = false;
    int dval = 0.0;
    char* cval = (char*)"BmsStatus Init";
    if(!checkAv(vmap, amap, aname, p_fims, av))
    {
        FPS_PRINT_ERROR(">> ERROR unable to continue aname [{}]", aname);
        return -1;
    }
    bool logging_enabled = getLoggingEnabled(vmap, *av->am->vm);
    char* LogDir = getLogDir(vmap, *av->am->vm);

    asset_manager* am = av->am;
    VarMapUtils* vm = am->vm;

    char* essName = vm->getSysName(vmap);
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAmBmsStatus", (void*)&CheckAmBmsStatus);
    //assetVar* CheckAssetComms = amap["CheckAmComms"];
    double toAlarm = 2.5;
    double toFault = 10.0;
    double toReset = 2.5;
    int initBmsStatus = -1;//(char *)" Initial BmsStatus";

    if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2)
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;
        //Link This to an incoming component
        if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);

        amap["BmsStatus"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatus", initBmsStatus);
        amap["MbmuStatus"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatus", initBmsStatus);
        amap["BmsStatusString"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusString", cval);
        amap["MbmuStatusString"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatusString", cval);
        amap["BmsStatusString2"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusString2", cval);

        amap["BmsExpStatus"] = vm->setLinkVal(vmap, aname, "/status", "BmsExpStatus", cval);
        amap["MbmuExpStatus"] = vm->setLinkVal(vmap, aname, "/status", "MbmuExpStatus", cval);
        amap["BmsTestToAlarm"] = vm->setLinkVal(vmap, aname, "/status", "BmsTestToAlarm", toAlarm);

        if (1)FPS_ERROR_PRINT("%s >>  aname TimeStamp %p comp [%s] name [%s] \n"
            , __func__
            , aname
            , amap["BmsStatus"]->comp.c_str()
            , amap["BmsStatus"]->name.c_str()
        );
        if (reload < 1)
        {
            vm->setAvFunc(vmap, amap, aname, p_fims, amap["BmsStatusString"], "BmsStatusString", CheckAmBmsStatus);
            vm->setAvFunc(vmap, amap, aname, p_fims, amap["MbmuStatusString"], "MbmuStatusString", CheckAmBmsStatus);
        }

        amap["essBmsStatusFaults"] = vm->setLinkVal(vmap, essName, "/status", "essBmsStatusFaults", ival);
        amap["essBmsStatusAlarms"] = vm->setLinkVal(vmap, essName, "/status", "essBmsStatusAlarms", ival);
        amap["essBmsStatusInit"] = vm->setLinkVal(vmap, essName, "/status", "essBmsStatusInit", ival);
        amap["essBmsStatusTimeoutFault"] = vm->setLinkVal(vmap, essName, "/config", "essBmsStatusTimeoutFault", toFault);
        amap["essBmsStatusTimeoutAlarm"] = vm->setLinkVal(vmap, essName, "/config", "essBmsStatusTimeoutAlarm", toAlarm);
        amap["essBmsStatusTimeoutReset"] = vm->setLinkVal(vmap, essName, "/config", "essBmsStatusTimeoutReset", toReset);


        // if(am->am)
        // {
        //     amap["amBmsStatusFaults"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "BmsStatusFaults",         ival);
        //     amap["amBmsStatusAlarms"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "BmsStatusAlarms",         ival);
        //     amap["amBmsStatusInit"]    = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "BmsStatusInit",           ival);
        // }

        amap["BmsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusFaults", ival);
        amap["BmsStatusAlarms"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusAlarms", ival);
        amap["BmsStatusInit"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusInit", ival);
        amap["BmsStatusState"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusState", cval);
        amap["BmsBypassStatus"] = vm->setLinkVal(vmap, aname, "/config", "BypassBmsStatus", bval);
        amap["AssetState"] = vm->setLinkVal(vmap, aname, "/status", "AssetState", ival);
        amap["BmsStatusStateNum"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusStateNum", ival);

        amap["MbmuStatusFaults"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatusFaults", ival);
        amap["MbmuStatusAlarms"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatusAlarms", ival);
        amap["MbmuStatusInit"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatusInit", ival);
        amap["MbmuStatusState"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatusState", cval);
        amap["MbmuBypassStatus"] = vm->setLinkVal(vmap, aname, "/config", "MbmuBypassStatus", bval);
        amap["MbmuStatusStateNum"] = vm->setLinkVal(vmap, aname, "/status", "MbmuStatusStateNum", ival);


        if (reload == 0) // complete restart 
        {
            amap["BmsStatus"]->setVal(initBmsStatus);
            amap["BmsExpStatus"]->setVal(cval);
            amap["MbmuExpStatus"]->setVal(cval);
            //lastBmsStatus=strdup(tsInit);//state"]->setVal(cval);
            amap["BmsStatus"]->setParam("lastBmsStatus", initBmsStatus);
            amap["BmsStatusString"]->setVal(cval);
            amap["BmsStatusString"]->setParam("lastStatusString", cval);
            amap["MbmuStatusString"]->setVal(cval);
            amap["MbmuStatusString"]->setParam("lastStatusString", cval);

            amap["BmsStatusString2"]->setVal(cval);
            amap["BmsStatusString2"]->setParam("lastBmsStatusString", cval);

            amap["BmsStatus"]->setParam("totalStatusFaults", 0);
            amap["BmsStatus"]->setParam("totalStatusAlarms", 0);
            amap["BmsStatus"]->setParam("seenFault", false);
            amap["BmsStatus"]->setParam("seenOk", false);
            amap["BmsStatus"]->setParam("seenAlarm", false);
            amap["BmsStatus"]->setParam("seenInit", false);
            amap["BmsStatus"]->setParam("initCnt", -1);

            amap["BmsStatus"]->setParam("rdFault", toFault);                      // time remaining before fault
            amap["BmsStatus"]->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["BmsStatus"]->setParam("rdReset", toReset);                      // time remaining before reset
            amap["BmsStatus"]->setParam("rdLast", dval);                         // time when last to event was seen
            amap["BmsStatus"]->setParam("ParamtoAlarm", amap["BmsTestToAlarm"]);  // Set an Av as a param


            amap["BmsStatusState"]->setVal(cval);

            amap["MbmuStatus"]->setParam("totalStatusFaults", 0);
            amap["MbmuStatus"]->setParam("totalStatusAlarms", 0);
            amap["MbmuStatus"]->setParam("seenFault", false);
            amap["MbmuStatus"]->setParam("seenOk", false);
            amap["MbmuStatus"]->setParam("seenAlarm", false);
            amap["MbmuStatus"]->setParam("seenInit", false);
            amap["MbmuStatus"]->setParam("initCnt", -1);

            amap["MbmuStatus"]->setParam("rdFault", toFault);                      // time remaining before fault
            amap["MbmuStatus"]->setParam("rdAlarm", toAlarm);                      // time reamining before alarm
            amap["MbmuStatus"]->setParam("rdReset", toReset);                      // time remaining before reset
            amap["MbmuStatus"]->setParam("rdLast", dval);                         // time when last to event was seen

            amap["MbmuStatusState"]->setVal(cval);

            ival = Asset_Init; amap["MbmuStatusStateNum"]->setVal(ival);
            ival = -1; amap["MbmuStatusInit"]->setVal(ival);
            amap["BmsBypassStatus"]->setVal(false);
            amap["MbmuBypassStatus"]->setVal(false);

            // if(!am->am)  // Nah do this in setLinkVals
            // {
            //     amap["essBmsStatusTimeoutFault"] ->setVal(toFault);
            //     amap["essBmsStatusTimeoutAlarm"] ->setVal(toAlarm);
            //     amap["essBmsStatusTimeoutReset"] ->setVal(toReset);

            // }
        }
        // reset reload
        ival = 2; amap["CheckAmBmsStatus"]->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();

    bool BmsBypassStatus = amap["BmsBypassStatus"]->getbVal();
    //bool MbmuBypassStatus = amap["BmsBypassStatus"]->getbVal();

    toFault = amap["essBmsStatusTimeoutFault"]->getdVal();
    toAlarm = amap["essBmsStatusTimeoutAlarm"]->getdVal();
    toReset = amap["essBmsStatusTimeoutReset"]->getdVal();

    int expectedBmsStatus = amap["BmsExpStatus"]->getiVal();
    int currentBmsStatus = amap["BmsStatus"]->getiVal();
    int lastBmsStatus = amap["BmsStatus"]->getiParam("lastBmsStatus");

    char* currentBmsStatusString = amap["BmsStatusString"]->getcVal();
    char* lastBmsStatusString = amap["BmsStatusString"]->getcParam("lastStatusString");
    char* currentMbmuStatusString = amap["MbmuStatusString"]->getcVal();
    char* lastMbmuStatusString = amap["MbmuStatusString"]->getcParam("lastStatusString");
    char* currentBmsStatusString2 = amap["BmsStatusString2"]->getcVal();
    char* lastBmsStatusString2 = amap["BmsStatusString2"]->getcParam("lastBmsStatusString");

    if (BmsBypassStatus)
    {
        ival = 1;
        amap["essBmsStatusInit"]->addVal(ival);
        return 0;

    }
    else
    {
        char* dest = nullptr;
        char* msg = nullptr;
        tNow = vm->get_time_dbl();

        if (strcmp(currentBmsStatusString, lastBmsStatusString) != 0)
        {
            if (1) FPS_ERROR_PRINT(" %s >> BmsStatusString comp [%s:%s] Changed from [%s] to [%s] at %2.6f\n"
                , __func__
                , amap["BmsStatusString"]->comp.c_str()
                , amap["BmsStatusString"]->name.c_str()
                , lastBmsStatusString
                , currentBmsStatusString
                , tNow
            );

            amap["BmsStatusString"]->setParam("lastStatusString", currentBmsStatusString);
            if (strcmp(currentBmsStatusString, "Warning status") == 0)
            {
                asprintf(&dest, "/assets/%s/summary:alarms", am->name.c_str());
                asprintf(&msg, "%s alarm  [%s] at %2.3f ", "Bms Status ", currentBmsStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["BmsStatusString"], dest, nullptr, msg, 2);

                    const auto now = flex::get_time_dbl();

                    ESSLogger::get().warn("BMS alarm, status is [{}]",
                        currentBmsStatusString);

                    av->sendEvent("BMS", am->p_fims, Severity::Alarm, "BMS alarm, status is: [%s] at time %2.3f"
                        , currentBmsStatusString
                        , now.count()
                        );
                }
                amap["essBmsStatusAlarms"]->addVal(1);

                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if (1)FPS_ERROR_PRINT(" %s ALARM >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg);
                if (dest)free(dest); 
                dest = nullptr;
                if (msg)free(msg);   
                msg = nullptr;
            }
            if (strcmp(currentBmsStatusString, "Fault status") == 0)
            {
                asprintf(&dest, "/assets/%s/summary:faults", am->name.c_str());
                asprintf(&msg, "%s fault  [%s] at %2.3f ", "Bms Status ", currentBmsStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["BmsStatusString"], dest, nullptr, msg, 2);

                    const auto now = flex::get_time_dbl();

                    ESSLogger::get().critical("BMS faulted with status [{}]",
                        currentBmsStatusString);
                    if (logging_enabled)
                    {
                        std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSFault", "txt"); // might just hard code the file name itself instead.
                        ESSLogger::get().logIt(dirAndFile); // todo: change the directory where this log file is sent to.
                    }

                    av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS faulted with status [%s] at time %2.3f"
                        , currentBmsStatusString
                        , now.count()
                        );
                }
                amap["essBmsStatusFaults"]->addVal(1);
                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if (1)FPS_ERROR_PRINT(" %s FAULT >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg);
                if (dest)free(dest); 
                dest = nullptr;
                if (msg)free(msg);    
                msg = nullptr;
            }

        }
        if (0) FPS_ERROR_PRINT(" %s >> Testing MbmuStatusString  [%s] to [%s] at %2.6f\n"
            , __func__
            , lastMbmuStatusString
            , currentMbmuStatusString
            , tNow
        );

        if (strcmp(currentMbmuStatusString, lastMbmuStatusString) != 0)
        {
            if (1) FPS_ERROR_PRINT(" %s >> MbmuStatusString Changed from [%s] to [%s] at %2.6f\n"
                , __func__
                , lastMbmuStatusString
                , currentMbmuStatusString
                , tNow
            );

            amap["MbmuStatusString"]->setParam("lastStatusString", currentMbmuStatusString);
            if (strcmp(currentMbmuStatusString, "Warning") == 0)
            {
                asprintf(&dest, "/assets/%s/summary:alarms", am->name.c_str());
                asprintf(&msg, "%s alarm  [%s] at %2.3f ", "Mbmu Status ", currentMbmuStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["MbmuStatusString"], dest, nullptr, msg, 2);

                    const auto now = flex::get_time_dbl();

                    ESSLogger::get().warn("MBMU alarm with status of [{}]",
                        currentBmsStatusString);

                    av->sendEvent("MBMU", am->p_fims, Severity::Alarm, "MBMU alarm with status of [%s] at time %2.3f"
                        , currentMbmuStatusString
                        , now.count()
                        );
                }
                amap["essBmsStatusAlarms"]->addVal(1);

                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if (1)FPS_ERROR_PRINT(" %s ALARM >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg);
                if (dest)free(dest); 
                dest = nullptr;
                if (msg)free(msg);    
                msg = nullptr;
            }
            if (strcmp(currentMbmuStatusString, "Fault") == 0)
            {
                asprintf(&dest, "/assets/%s/summary:faults", am->name.c_str());
                asprintf(&msg, "%s fault  [%s] at %2.3f ", "Mbmu Status ", currentMbmuStatusString, tNow);
                {
                    vm->sendAlarm(vmap, amap["MbmuStatusString"], dest, nullptr, msg, 2);

                    const auto now = flex::get_time_dbl();

                    ESSLogger::get().critical("MBMU faulted with a status of [{}]",
                        currentBmsStatusString);
                    if (logging_enabled)
                    {
                        std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "MBMUFault", "txt");
                        ESSLogger::get().logIt(dirAndFile);
                    }

                    assetVar* temp_av = amap["MbmuStatusString"];
                    temp_av->sendEvent("MBMU", am->p_fims, Severity::Fault, "MBMU faulted with a status of [%s] at time %2.3f"
                        , currentMbmuStatusString
                        , now.count()
                        );
                }

                amap["essBmsStatusFaults"]->addVal(1);

                //av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if (1)FPS_ERROR_PRINT(" %s FAULT >>>>>> dest [%s] msg [%s]\n", __func__, dest, msg);
                if (dest)free(dest); 
                dest = nullptr;
                if (msg)free(msg);    
                msg = nullptr;
            }

        }

        if (strcmp(currentBmsStatusString2, lastBmsStatusString2) != 0)
        {
            if (1) FPS_ERROR_PRINT(" %s >> BmsStatusString2 Changed from [%s] to [%s] at %2.3f\n"
                , __func__
                , lastBmsStatusString2
                , currentBmsStatusString2
                , tNow
            );

            amap["BmsStatusString2"]->setParam("lastBmsStatusString", currentBmsStatusString2);

        }
        // If we are in the init state wait for comms to start count down reset time
        if (currentBmsStatus == initBmsStatus)
        {
            bool seenInit = amap["BmsStatus"]->getbParam("seenInit");

            //ival = 1; amap["CheckAssetComs"]->setVal(ival);
            //ival = 1; amap["CheckAssetComs"]->setVal(ival);
            if (0)FPS_ERROR_PRINT("%s >> %s  NO BmsStatus,  bypass [%s]\n", __func__, aname, BmsBypassStatus ? "true" : "false");

            // if not toally set up yet then quit this pass
            if (!amap["amBmsStatusInit"])
            {
                if (1)FPS_ERROR_PRINT("%s >> %s  no VAR amBmsStatusInit Yet...\n", __func__, aname);
                amap["amBmsStatusInit"] = vm->setLinkVal(vmap, aname, "/status", "amBmsStatusInit", ival);
                return 0;
            }

            if (!seenInit)   // BmsStatus_Setup
            {
                if (1)FPS_ERROR_PRINT("%s >> %s  amBmsStatusInit  SEEN ...\n", __func__, aname);
                assetVar* toav = amap["BmsStatus"]->getaParam("ParamtoAlarm");  // Get an Av as a param
                if (1)FPS_ERROR_PRINT("%s >> %s  ParamtoAlarm %f  \n", __func__, aname, toav->getdVal());

                amap["BmsStatus"]->setParam("seenInit", true);

                char* cval = (char*)"BmsStatus Init, no BmsStatus Seen";
                amap["BmsStatusState"]->setVal(cval);

                ival = 1;
                amap["essBmsStatusInit"]->addVal(ival);
                amap["BmsStatusInit"]->setVal(0);      //BmsStatus_Init  
            }
            amap["BmsStatus"]->setParam("rdLast", tNow);

        }
        else  // wait for comms to go past reset then set active or wait to alarm and then fault
        {
            //if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastBmsStatus?lastBmsStatus:"no last Value", tval1);
            double rdLast = amap["BmsStatus"]->getdParam("rdLast");
            double rdFault = amap["BmsStatus"]->getdParam("rdFault");
            double rdAlarm = amap["BmsStatus"]->getdParam("rdAlarm");
            double rdReset = amap["BmsStatus"]->getdParam("rdReset");
            amap["BmsStatus"]->setParam("rdLast", tNow);

            double toVal = amap["BmsStatus"]->getLastSetDiff(tNow);

            // Has value changed ? If yes then count down rdReset to zero based on tNow - rdLast
            if (currentBmsStatus != lastBmsStatus)
                //if(amap["BmsStatus"]->valueChangedReset())
            {
                if (1)FPS_ERROR_PRINT("%s >> %s  amBmsStatus Changed from  %d to %d  (expected %d) at time %2.3f  SEEN ...\n"
                    , __func__, aname, lastBmsStatus, currentBmsStatus, expectedBmsStatus, tNow);

                amap["BmsStatus"]->setParam("lastBmsStatus", currentBmsStatus);
            }

            if (currentBmsStatus == expectedBmsStatus)
            {
                bool seenOk = amap["BmsStatus"]->getbParam("seenOk");
                if (rdReset > 0.0)
                {
                    rdReset -= (tNow - rdLast);
                    amap["BmsStatus"]->setParam("rdReset", rdReset);
                }
                //else
                {
                    if (rdAlarm < toAlarm)
                    {
                        rdAlarm += tNow - rdLast;
                        amap["BmsStatus"]->setParam("rdAlarm", rdAlarm);
                    }
                    if (rdFault < toFault)
                    {
                        rdFault += tNow - rdLast;
                        amap["BmsStatus"]->setParam("rdFault", rdFault);
                    }
                }

                if (0)FPS_ERROR_PRINT("%s >>  BmsStatus change for %s from [%d] to [%d]  rdReset now %2.3f diff %2.3f rdAlarm %2.3f rdFault %2.3f\n"
                    , __func__, aname, lastBmsStatus, currentBmsStatus, rdReset, (tNow - rdLast), rdAlarm, rdFault);

                ival = amap["BmsStatusStateNum"]->getiVal();
                // reset time passed , still changing , time to switch to BmsStatus_Ready
                if ((rdReset <= 0.0) && (ival != seenOk))
                {

                    bool seenFault = amap["BmsStatus"]->getbParam("seenFault");
                    //bool seenOk  = amap["BmsStatus"]->getbParam("seenOk");
                    bool seenAlarm = amap["BmsStatus"]->getbParam("seenAlarm");
                    amap["BmsStatus"]->setParam("seenOk", true);

                    if (0)FPS_ERROR_PRINT("%s >>  BmsStatus  change for %s from [%d] to [%d] \n", __func__, aname, lastBmsStatus, currentBmsStatus);
                    if (seenFault)
                    {
                        if (1)FPS_ERROR_PRINT("%s >>  BmsStatus fault for  %s cleared at %2.3f\n", __func__, aname, tNow);
                        amap["BmsStatus"]->setParam("seenFault", false);

                    }
                    if (seenAlarm)
                    {
                        if (1)FPS_ERROR_PRINT("%s >>  BmsStatus Alarm for  %s cleared at %2.3f\n", __func__, aname, tNow);
                        amap["BmsStatus"]->setParam("seenAlarm", false);

                    }
                    if (1)FPS_ERROR_PRINT("%s >>  BmsStatus OK for  %s at %2.3f\n", __func__, aname, tNow);
                    ival = Asset_Ok; // seen BmsStatus change
                    amap["BmsStatusStateNum"]->setVal(ival);
                    ival = 0;
                    amap["BmsStatusInit"]->setVal(ival);
                    char* tval = nullptr;
                    asprintf(&tval, " BmsStatus OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if (tval)
                    {
                        amap["BmsStatusState"]->setVal(tval);
                        free(tval); tval = nullptr;
                    }
                }

                // increment alarm and fault time reset time
                if (rdFault < toFault)
                {
                    rdFault += (tNow - rdLast);
                    amap["BmsStatus"]->setParam("rdFault", rdFault);
                }
                if (rdAlarm < toAlarm)
                {
                    rdAlarm += (tNow - rdLast);
                    amap["BmsStatus"]->setParam("rdAlarm", rdAlarm);
                }

                //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
                amap["BmsStatus"]->setParam("lastBmsStatus", currentBmsStatus);
                //if ((toVal > toFault)  && !bokFault && !bypass)

            }
            else   // No Change , start tracking faults and alarms
            {
                bool seenFault = amap["BmsStatus"]->getbParam("seenFault");
                //bool seenOk  = amap["BmsStatus"]->getbParam("seenOk");
                bool seenAlarm = amap["BmsStatus"]->getbParam("seenAlarm");
                if (rdFault > 0.0)
                {
                    rdFault -= (tNow - rdLast);
                    amap["BmsStatus"]->setParam("rdFault", rdFault);
                }
                if (rdAlarm > 0.0)
                {
                    rdAlarm -= (tNow - rdLast);
                    amap["BmsStatus"]->setParam("rdAlarm", rdAlarm);
                }
                if (rdReset < toReset)
                {
                    rdReset += (tNow - rdLast);
                    amap["BmsStatus"]->setParam("rdReset", rdReset);
                }

                if ((rdFault <= 0.0) && !seenFault)
                {

                    if (1)FPS_ERROR_PRINT("%s >>  BmsStatus  Fault  for %s at %2.3f \n", __func__, aname, tNow);
                    char* tval = nullptr;
                    asprintf(&tval, " BmsStatus Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if (tval)
                    {
                        amap["BmsStatusState"]->setVal(tval);
                        free(tval); tval = nullptr;
                    }
                    int ival = 1;
                    amap["BmsStatusFaults"]->addVal(ival);
                    amap["essBmsStatusFaults"]->addVal(ival);

                    // if(am->am)
                    // {
                    //     amap["amBmsStatusFaults"]->addVal(ival);
                    // }

                    ival = Asset_Fault; //BmsStatus Fault
                    amap["BmsStatusStateNum"]->setVal(ival);

                    seenFault = true;
                    amap["BmsStatus"]->setParam("seenFault", true);
                    amap["BmsStatus"]->setParam("seenOk", false);
                    amap["BmsStatus"]->setParam("seenAlarm", true);
                    //seenOk = false;
                    seenAlarm = false;

                    int totalBmsStatusFaults = amap["BmsStatus"]->getiParam("totalStatusFaults");
                    totalBmsStatusFaults++;
                    amap["BmsStatus"]->setParam("totalStatusFaults", totalBmsStatusFaults);

                }
                else if ((rdAlarm <= 0.0) && !seenAlarm)
                {
                    if (1)FPS_ERROR_PRINT("%s >>  ts  Alarm  for %s at %2.3f \n", __func__, aname, tNow);

                    char* tval = nullptr;
                    asprintf(&tval, "BmsStatus Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if (tval)
                    {
                        amap["BmsStatusState"]->setVal(tval);
                        free(tval); tval = nullptr;
                    }

                    int ival = 1;
                    amap["BmsStatusAlarms"]->addVal(ival);
                    amap["essBmsStatusAlarms"]->addVal(ival);

                    // if(am->am)
                    // {
                    //     amap["amBmsStatusAlarms"]->addVal(ival);
                    // }
                    ival = Asset_Alarm; //BmsStatus Alarm
                    amap["BmsStatusStateNum"]->setVal(ival);

                    amap["BmsStatus"]->setParam("seenAlarm", true);
                    //amap["BmsStatus"]->setParam("seenFault", false);
                    amap["BmsStatus"]->setParam("seenOk", false);
                    int totalBmsStatusAlarms = amap["BmsStatus"]->getiParam("totalBmsStatusAlarms");
                    totalBmsStatusAlarms++;
                    amap["BmsStatus"]->setParam("totalBmsStatusAlarms", totalBmsStatusAlarms);
                }
                else
                {
                    if (0)FPS_ERROR_PRINT("%s >> BmsStatus for [%s] [%s] Stalled at %2.3f  Reset %2.3f Fault %2.3f Alarm %2.3f \n"
                        , __func__
                        , aname
                        , amap["BmsStatus"]->getcVal()
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

// ess_controller test asset status  
// logs any changes
int CheckEssStatus(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    //double dval = 0.0;
    int ival = 0;
    //bool bval = false;
    //int dval = 0.0;
    char* cval = (char*)"Ess Status Init";
    char* pcval = (char*)"Pcs Status Init";
    char* bmval = (char*)"Bms Status Init";
    VarMapUtils* vm = am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckEssStatus");
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
        if (1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);

        amap["EssStatus"] = vm->setLinkVal(vmap, aname, "/status", "EssStatus", initEssStatus);
        amap["PcsStatus"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatus", initPcsStatus);
        amap["BmsStatus"] = vm->setLinkVal(vmap, aname, "/status", "BmssStatus", initPcsStatus);

        amap["PcsStatusState"] = vm->setLinkVal(vmap, aname, "/status", "PcsStatusState", pcval);
        amap["BmsStatusState"] = vm->setLinkVal(vmap, aname, "/status", "BmsStatusState", bmval);


        amap["essPcsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status", "essPcsStatusFaults", ival);
        amap["essPcsStatusAlarms"] = vm->setLinkVal(vmap, aname, "/status", "essPcsStatusAlarms", ival);
        amap["essPcsStatusInit"] = vm->setLinkVal(vmap, aname, "/status", "essPcsStatusAlarms", ival);
        amap["essPcsStatusState"] = vm->setLinkVal(vmap, aname, "/status", "essPcsStatusState", cval);
        amap["essBmsStatusFaults"] = vm->setLinkVal(vmap, aname, "/status", "essBmsStatusFaults", ival);
        amap["essBmsStatusAlarms"] = vm->setLinkVal(vmap, aname, "/status", "essBmsStatusAlarms", ival);
        amap["essBmsStatusInit"] = vm->setLinkVal(vmap, aname, "/status", "essBmsStatusAlarms", ival);
        amap["essBmsStatusState"] = vm->setLinkVal(vmap, aname, "/status", "essBmsStatusState", cval);


        if (reload == 0) // complete restart 
        {
            amap["PcsStatus"]->setVal(initPcsStatus);
            amap["BmsStatus"]->setVal(initBmsStatus);
            //lastPcsStatus=strdup(tsInit);//state"]->setVal(cval);
            amap["PcsStatus"]->setParam("lastPcsStatus", initPcsStatus);
            amap["PcsStatus"]->setParam("totalPcsStatusFaults", 0);
            amap["PcsStatus"]->setParam("totalPcsStatusAlarms", 0);

            amap["BmsStatus"]->setParam("lastBmsStatus", initBmsStatus);
            amap["BmsStatus"]->setParam("totalBmsStatusFaults", 0);
            amap["BmsStatus"]->setParam("totalBmsStatusAlarms", 0);


            amap["PcsStatusState"]->setVal(pcval);
            amap["BmsStatusState"]->setVal(bmval);

        }
        // reset reload
        ival = 2; amap["CheckEssStatus"]->setVal(ival);
    }

    //double tNow = am->vm->get_time_dbl();

    // are we the ess_controller 
    if (!am->am)
    {
        //bool initSeen =             amap["PcsStatus"]     ->getbParam("initSeen");

        amap["essPcsStatusFaults"]->setVal(0);
        amap["essPcsStatusAlarms"]->setVal(0);
        amap["essPcsStatusInit"]->setVal(0);
        amap["essBmsStatusFaults"]->setVal(0);
        amap["essBmsStatusAlarms"]->setVal(0);
        amap["essBmsStatusInit"]->setVal(0);
        int icnt = 0;
        assetVar Av;
        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;
            Av.am = amc;
            if (amc->name == "pcs")
            {
                CheckAmPcsStatus(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
                icnt++;
            }
            else if (amc->name == "bms")
            {
                CheckAmBmsStatus(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
                icnt++;
            }

            int essPcsStatusFaults = amap["essPcsStatusFaults"]->getiVal();
            int essPcsStatusAlarms = amap["essPcsStatusAlarms"]->getiVal();
            //int essPcsStatusInit = amap["essPcsStatusInit"]->getiVal();
            if (essPcsStatusFaults > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essPcsStatusFaults detected\n", __func__, essPcsStatusFaults);
            }
            if (essPcsStatusAlarms > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essPcsStatusAlarmss detected\n", __func__, essPcsStatusAlarms);
            }
            int essBmsStatusFaults = amap["essBmsStatusFaults"]->getiVal();
            int essBmsStatusAlarms = amap["essBmsStatusAlarms"]->getiVal();
            //int essBmsStatusInit = amap["essBmsStatusInit"]->getiVal();
            if (essBmsStatusFaults > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essBmsStatusFaults detected\n", __func__, essBmsStatusFaults);
            }
            if (essBmsStatusAlarms > 0)
            {
                FPS_ERROR_PRINT("%s >> %d essBmsStatusAlarms detected\n", __func__, essBmsStatusAlarms);
            }
        }
    }
    return 0;
};
#endif
