#ifndef SIMHANDLEHEARTBEAT_CPP
#define SIMHANDLEHEARTBEAT_CPP
//SimHandleHeartbeat.cpp
#include "asset.h"


/**
 * Phil's code ready for review
 * 11/06/2020
 * increments  a heart beat ... may need to add a period to this
 * User to simulate asset heartbeats
 *
 * Used in:
 * Test Script: test_sim_hb.sh
 */
 extern "C++" {

    int SimHandleHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);

};

int SimHandleHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    int reload;
    double dval = 0.0;
    bool bval = false;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    char* tVal = (char*)"Test TimeStamp";
    const auto now = flex::get_time_dbl();

    assetVar* SimHandleHeartbeat = amap[__func__];
    if (!SimHandleHeartbeat || (reload = SimHandleHeartbeat->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        //reload = 0;
        amap[__func__]             = vm->setLinkVal(vmap, aname,         "/reload",    __func__, reload);
        amap["Heartbeat"]          = vm->setLinkVal(vmap, aname,         "/status",    "Heartbeat", dval);
        amap["Timestamp"]          = vm->setLinkVal(vmap, aname,         "/status",    "Timestamp", tVal);
        amap["CommsDummy"]         = vm->setLinkVal(vmap, aname,         "/status",    "CommsDummy", dval);
        amap["SimPcsComms"]        = vm->setLinkVal(vmap, aname,         "/configsim", "SimPcsComms", bval);
        amap["SimPcsHB"]           = vm->setLinkVal(vmap, aname,         "/configsim", "SimPcsHB", bval);
        amap["SimBmsComms"]        = vm->setLinkVal(vmap, aname,         "/configsim", "SimBmsComms", bval);
        amap["SimBmsHB"]           = vm->setLinkVal(vmap, aname,         "/configsim", "SimBmsHB", bval);
        amap["SimPub"]             = vm->setLinkVal(vmap, aname,         "/configsim", "SimPub", bval);
        // amap["SimBms_1Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_1Comms", bval);
        // amap["SimBms_1HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_1HB", bval);
        // amap["SimBms_2Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_2Comms", bval);
        // amap["SimBms_2HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_2HB", bval);
        // amap["SimBms_3Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_3Comms", bval);
        // amap["SimBms_3HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_3HB", bval);
        // amap["SimBms_4Comms"]      = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_4Comms", bval);
        // amap["SimBms_4HB"]         = vm->setLinkVal(vmap, aname,         "/configsim", "SimBms_4HB", bval);
        dval = 1.0;
        amap["HeartbeatPeriod"] = vm->setLinkVal(vmap, aname, "/configsim", "HeartbeatPeriod", dval);
        dval = 255.0;
        amap["HeartbeatMax"] = vm->setLinkVal(vmap, aname, "/configsim", "HeartbeatMax", dval);
        // amap["HandleHeartbeat"]->setVal(2);  // revert reload
        if (reload == 0) // complete restart 
        {
            amap["Heartbeat"]->setVal(0);
        }
        reload = 2;    amap[__func__]->setVal(reload);
    }
    dval = now.count(); ; //vm->get_time_dbl();
    asprintf(&tVal, "the new time is %f", dval);
    amap["Timestamp"]->setVal(tVal);
    if(0)FPS_ERROR_PRINT("Heartbeat [%s] val [%f] last set diff [%f]", tVal, dval, amap["Heartbeat"]->getLastSetDiff(dval));

    free((void*)tVal);
    if (amap["Heartbeat"]->getLastSetDiff(dval) > 1.0)
    {
        if(0)FPS_ERROR_PRINT("Heartbeat >> Running val [%f] \n", dval);

        // get the reference to the variable 
        assetVar* hb = amap["Heartbeat"];
        assetVar* cd = amap["Timestamp"];
        assetVar* hbmax = amap["HeartbeatMax"];

        bool SimPcsComms = amap["SimPcsComms"]->getbVal();
        bool SimPcsHB = amap["SimPcsHB"]->getbVal();
        bool SimBmsComms = amap["SimBmsComms"]->getbVal();
        bool SimBmsHB = amap["SimBmsHB"]->getbVal();
        bool SimPub = amap["SimPub"]->getbVal();
        // bool SimBms_1Comms = amap["SimBms_1Comms"]->getbVal();
        // bool SimBms_1HB = amap["SimBms_1HB"]->getbVal();
        // bool SimBms_2Comms = amap["SimBms_2Comms"]->getbVal();
        // bool SimBms_2HB = amap["SimBms_2HB"]->getbVal();
        // bool SimBms_3Comms = amap["SimBms_3Comms"]->getbVal();
        // bool SimBms_3HB = amap["SimBms_3HB"]->getbVal();
        // bool SimBms_4Comms = amap["SimBms_4Comms"]->getbVal();
        // bool SimBms_4HB = amap["SimBms_4HB"]->getbVal();

        //double ival;
        double dvalmax = hbmax->getdVal();
        dval = hb->getdVal();
        dval++;
        if (dval > dvalmax) dval = 0;
        //if(1)FPS_ERROR_PRINT("Heartbeat %s val %f ", aname, dval);

        hb->setVal(dval);
        //cd->setVal(dval);
        //dval = hb->getdVal();
        if (0)FPS_ERROR_PRINT("Heartbeat aname %s  val after set %f SimPcsComms [%s]\n"
            , aname, dval, SimPcsComms?"true":"false");
        char *sp = cd->getcVal();
        dval = hb->getdVal();
        if (SimPcsComms)  
        {
            if (0)FPS_ERROR_PRINT("Heartbeat aname %s  val after set %f SimPcsComms [%s] cd %p p_fims %p\n"
                , aname, dval, SimPcsComms?"true":"false", cd, p_fims);
            if(SimPub)vm->sendAssetVar(cd, p_fims, "pub", "/components/pcs_sim", "sys_timestamp");
            vm->setVal(vmap, "/components/pcs_sim", "pcs_timestamp", sp);
        }
        if (SimPcsHB)
        {
            if(SimPub)vm->sendAssetVar(hb, p_fims, "pub", "/components/pcs_sim","sys_heartbeat");
            vm->setVal(vmap, "/components/pcs_sim", "pcs_heartbeat", dval);
        }
        //"/components/catl_mbmu_stat_r:bms_heartbeat"

        if (SimBmsComms)  {
            if(SimPub)vm->sendAssetVar(cd, p_fims, "pub", "/components/bms_sim", "bms_timestamp");
            vm->setVal(vmap, "/components/bms_sim", "bms_timestamp", sp);
        }
        if (SimBmsHB)     {
            if(SimPub)vm->sendAssetVar(hb, p_fims, "pub", "/components/bms_sim", "bms_heartbeat");
            vm->setVal(vmap, "/components/bms_sim", "bms_heartbeat", dval);
        }
    }
    return dval;
}
#endif
