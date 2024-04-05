#ifndef HANDLEASSETHEARTBEAT_CPP
#define HANDLEASSETHEARTBEAT_CPP

#include "asset.h"
// Deprecated
/**
 * Distributes tod & heartbeat from the ess master
 * this should only run when triggered by the Manager
 *
 * Reviewed: 10/21/2020  modified 12/2/2020
 *
 * Used in:
 * Test Script:
 */
int HandleAssetHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    bool bval = false;
    double dval = 0.0;
    // double dvalHB = 1.0;
    int ival = 0;
    VarMapUtils* vmp = am->vm;
    char* essName = vmp->getSysName(vmap);

    int reload = vmp->CheckReload(vmap, amap, aname, "HandleAssetHeartbeat");

    double tNow = vmp->get_time_dbl();

    // assetVar* HandleAssetHeartbeat = amap["HandleAssetHeartbeat"];
    // if (!HandleAssetHeartbeat || (reload = HandleAssetHeartbeat->getiVal()) ==
    // 0)
    // {
    //     reload = 0;  // complete reset  reload = 1 for remap ( links may have
    //     changed)
    // }

    if (reload < 2)
    {
        // reload = 0;
        amap["HandleAssetHeartbeat"] = vmp->setLinkVal(vmap, aname, "/reload", "HandleAssetHeartbeat", reload);
        amap["HeartbeatLast"] = vmp->setLinkVal(vmap, aname, "/status", "HeartbeatLast", tNow);
        // amap["HeartbeatInterval"]      = vmp->setLinkVal(vmap, aname, "/status",
        // "HeartbeatInterval", dvalHB);

        // Jimmy - We probably don't need to set ess TOD here since that's already
        // done in UpdateSysTime
        amap["ess_todSec"] = vmp->setLinkVal(vmap, essName, "/status", "ess_todSec", ival);
        amap["ess_todMin"] = vmp->setLinkVal(vmap, essName, "/status", "ess_todMin", ival);
        amap["ess_todHr"] = vmp->setLinkVal(vmap, essName, "/status", "ess_todHr", ival);
        amap["ess_todDay"] = vmp->setLinkVal(vmap, essName, "/status", "ess_todDay", ival);
        amap["ess_todMon"] = vmp->setLinkVal(vmap, essName, "/status", "ess_todMon", ival);
        amap["ess_todYr"] = vmp->setLinkVal(vmap, essName, "/status", "ess_todYr", ival);

        amap["OkToSend"] = vmp->setLinkVal(vmap, aname, "/config", "OkToSend", bval);

        // Jimmy - We probably don't need to set asset TOD here since that's already
        // done in UpdateSysTime
        amap["op_todSec"] = vmp->setLinkVal(vmap, aname, "/status", "op_todSec", ival);
        amap["op_todMin"] = vmp->setLinkVal(vmap, aname, "/status", "op_todMin", ival);
        amap["op_todHr"] = vmp->setLinkVal(vmap, aname, "/status", "op_todHr", ival);
        amap["op_todDay"] = vmp->setLinkVal(vmap, aname, "/status", "op_todDay", ival);
        amap["op_todMon"] = vmp->setLinkVal(vmap, aname, "/status", "op_todMon", ival);
        amap["op_todYr"] = vmp->setLinkVal(vmap, aname, "/status", "op_todYr", ival);
        amap["op_HB"] = vmp->setLinkVal(vmap, aname, "/status", "op_HB", ival);

        amap["Heartbeat"] = vmp->setLinkVal(vmap, aname, "/status", "Heartbeat", ival);
        amap["tNow"] = vmp->setLinkVal(vmap, aname, "/status", "tNow", tNow);
        // amap["HeartbeatDbl"]           = vmp->setLinkVal(vmap, aname, "/status",
        // "HeartbeatDbl", dval);  amap["HeartbeatLink"]          =
        // vmp->setLinkVal(vmap, aname, "/status", "HeartbeatLink", ival);
        // amap["]->setVal(2);  // revert reload
        if (reload == 0)  // complete restart
        {
            ival = 0;
            amap["Heartbeat"]->setVal(ival);
            amap["tNow"]->setVal(tNow);
            amap["tNow"]->setParam("tLast", tNow);
        }
        ival = 2;
        amap["HandleAssetHeartbeat"]->setVal(ival);
    }

    // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
    double HBLast = amap["HeartbeatLast"]->getdVal();
    double HBnow = amap["Heartbeat"]->getdVal();
    if (0)
        FPS_ERROR_PRINT("%s >> Heartbeat value before %f\n", __func__, HBLast);
    if (0)
        FPS_ERROR_PRINT("%s >> Heartbeat value now %f\n", __func__, HBnow);

    bool OkToSend = amap["OkToSend"]->getbVal();
    OkToSend = false;  // it causes a crash
    dval = 1.0;

    // dont use valueChanged it resets the change currently
    if (HBLast != HBnow)
    {
        amap["HeartbeatLast"]->setVal(HBnow);

        dval = 1.0;
        // this value is used to trigger the heartbeats for all the assets that need
        // it
        amap["Heartbeat"]->addVal(dval);
        amap["op_todSec"]->setVal(amap["ess_todSec"]->getiVal());
        amap["op_todMin"]->setVal(amap["ess_todMin"]->getiVal());
        amap["op_todHr"]->setVal(amap["ess_todHr"]->getiVal());
        amap["op_todDay"]->setVal(amap["ess_todDay"]->getiVal());
        amap["op_todMon"]->setVal(amap["ess_todMon"]->getiVal());
        amap["op_todYr"]->setVal(amap["ess_todYr"]->getiVal());

        // this stuff collects a bunch of assetVars and send them out to their
        // default locations. the link will determine where that location is. if the
        // link is defined in the config file then that destination will be
        // maintained.

        if (1 || OkToSend)
        {
            varsmap* vlist = vmp->createVlist();
            vmp->addVlist(vlist, amap["Heartbeat"]);
            vmp->addVlist(vlist, amap["op_todSec"]);
            vmp->addVlist(vlist, amap["op_todMin"]);
            vmp->addVlist(vlist, amap["op_todHr"]);
            vmp->addVlist(vlist, amap["op_todDay"]);
            vmp->addVlist(vlist, amap["op_todYr"]);
            vmp->sendVlist(p_fims, "set", vlist);
            vmp->clearVlist(vlist);
        }
    }

    return 0;
}
#endif