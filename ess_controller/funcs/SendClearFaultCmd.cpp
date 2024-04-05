#ifndef SENDCLEARFAULTCMD_CPP
#define SENDCLEARFAULTCMD_CPP

#include "asset.h"

extern "C++" {
int SendClearFaultCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunKeyCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}
/**
 * @brief Sends a clear fault command to hardware units (ex.: CATL BMS) that
 * contains the clear fault register
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the asset var
 */

int SendClearFaultCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (0)
        FPS_ERROR_PRINT("%s >> Incoming amap [%p] aname [%s]  Sending to assetVar [%s]\n", __func__, (void*)&amap,
                        aname, av->name.c_str());
    VarMapUtils* vm = av->am->vm;
    asset_manager* am = vm->getaM(vmap, aname);
    if (0)
        FPS_ERROR_PRINT("%s >> Corrected am [%p] amap %p aname [%s]\n", __func__, (void*)am, (void*)&amap,
                        am->name.c_str());

    bool bval = true;
    int reload = 0;
    // create a unique reload var
    char* rldName = nullptr;
    asprintf(&rldName, "%s%s", aname, "RunKeyCmd");
    char* schedName = nullptr;
    asprintf(&schedName, "%s%s%s", "sched", __func__, aname);

    assetVar* cfAv = amap[__func__];
    if (!cfAv || (reload = cfAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload == 0)
    {
        int ival = 0;
        bval = false;
        char* cval = (char*)"";
        double dval = 0.0;

        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        cfAv = amap[__func__];
        linkVals(*vm, vmap, amap, aname, "/reload", reload, rldName);

        linkVals(*vm, vmap, amap, aname, "/controls", ival, "ClearFaultCmd");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "SystemFault", "SystemFaultCleared");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd");
        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, schedName);
        linkVals(*vm, vmap, amap, aname, "/status", bval, "gpioFault");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "gpioBitfield");
        FPS_ERROR_PRINT("%s >> Fault Setup amap %p  schedName [%s] aname %s av %p\n", __func__, (void*)&amap, schedName,
                        aname, (void*)amap[schedName]);
        // not called at startup at the moment...
        // cfAv->setVal(1);
        // return 0;
    }
    if (reload < 2)
    {
        amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());
        bval = false;
        vm->setVal(vmap, amap["SystemFaultCleared"]->comp.c_str(), amap["SystemFaultCleared"]->name.c_str(), bval);
        bval = true;
        amap["SystemFault"]->setParam("Clearing", false);
        if (1)
            FPS_ERROR_PRINT("%s >>> %s Running reload 1 >> gpioFault [%d]\n", __func__, schedName,
                            amap["gpioFault"]->getbVal());
        amap[rldName]->setVal(0);  // Reset key cmd function
        RunKeyCmd(vmap, amap, aname, nullptr, av);

        cfAv->setVal(2);
    }

    if (amap["gpioFault"]->getbVal())
    {
        amap["gpioBitfield"]->setVal(0);
        if (amap["KeyCmd"]->getbVal())
            amap["KeyCmd"]->setVal(false);
        FPS_ERROR_PRINT("%s >> GPIO Fault: %s\n", __func__, amap["gpioFault"]->name.c_str());
        // Send Event - info
        const auto now = flex::get_time_dbl();
        ESSLogger::get().info("[GPIO fault] seen");
        auto temp_av = amap["KeyCmd"];
        if (temp_av)
            temp_av->sendEvent("ESS", am->p_fims, Severity::Info, "GPIO Fault seen at time %2.3f", now.count());
        vm->setVal(vmap, amap["SystemFaultCleared"]->comp.c_str(), amap["SystemFaultCleared"]->name.c_str(), bval);
        vm->schedStop(vmap, amap, schedName, 0.0001);
        cfAv->setVal(1);
    }
    else if (!amap["SystemFault"]->getbVal())
    {
        if (amap["SystemFault"]->getbParam("Clearing"))
        {
            if (amap["KeyCmd"]->getbVal())
                amap["KeyCmd"]->setVal(false);
            FPS_ERROR_PRINT("%s >> Fault cleared amap %p  schedName [%s] aname %s av %p\n", __func__, (void*)&amap,
                            schedName, aname, (void*)amap[schedName]);
            // Send Event - info
            const auto now = flex::get_time_dbl();
            ESSLogger::get().info("[{}] [Fault cleared]", aname);
            auto temp_av = amap["KeyCmd"];
            if (temp_av)
                temp_av->sendEvent("ESS", am->p_fims, Severity::Info, "Fault cleared at time %2.3f", now.count());
        }
        vm->setVal(vmap, amap["SystemFaultCleared"]->comp.c_str(), amap["SystemFaultCleared"]->name.c_str(), bval);
        vm->schedStop(vmap, amap, schedName, 0.0001);
        cfAv->setVal(1);
    }
    else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
    {
        if (amap["KeyCmd"]->getbVal())
            amap["KeyCmd"]->setVal(false);
        FPS_ERROR_PRINT("%s >> Fault not clearing\n", __func__);
        // Send Event - info
        const auto now = flex::get_time_dbl();
        ESSLogger::get().warn("[Fault not clearing]");
        auto temp_av = amap["KeyCmd"];
        if (temp_av)
            temp_av->sendEvent("ESS", am->p_fims, Severity::Alarm, "Fault not clearing at time %2.3f", now.count());
        vm->setVal(vmap, amap["SystemFaultCleared"]->comp.c_str(), amap["SystemFaultCleared"]->name.c_str(), bval);
        vm->schedStop(vmap, amap, schedName, 0.0001);
        cfAv->setVal(1);
    }
    else
    {
        RunKeyCmd(vmap, amap, aname, nullptr, av);
        if (!amap["SystemFault"]->getbParam("Clearing"))
            amap["SystemFault"]->setParam("Clearing", true);
    }

    // Send clear fault cmd here
    if (amap["KeyCmd"]->valueChangedReset())
    {
        varsmap* vlist = vm->createVlist();
        if (amap["KeyCmd"]->getbVal())
        {
            amap["ClearFaultCmd"]->setVal(1);  // config?
            vm->addVlist(vlist, amap["ClearFaultCmd"]);
        }
        else
        {
            amap["ClearFaultCmd"]->setVal(0);
            vm->addVlist(vlist, amap["ClearFaultCmd"]);
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }

    if (rldName)
        free(rldName);
    if (schedName)
        free(schedName);

    return 0;
}

#endif