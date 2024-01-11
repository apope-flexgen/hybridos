#ifndef RUNKEYCMD_CPP
#define RUNKEYCMD_CPP

#include "asset.h"

/**
 * 
 */
 extern "C++" {

    int RunKeyCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av);

}

int RunKeyCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    int reload = 0;
    int ival = 0;
    bool bval = false;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();
    // create a unique reload var
    char* rldName = nullptr;
    asprintf(&rldName, "%s%s", aname, __func__);

    assetVar* rkcAv = amap[rldName];
    if (!rkcAv || (reload = rkcAv->getiVal()) == 0)
    {
        if (1) FPS_ERROR_PRINT("%s >> Running [%s] reload %d \n", __func__, rldName, reload);
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }
    
    if (reload < 2)
    {
        amap[rldName] = vm->setLinkVal(vmap, aname, "/reload", rldName, reload);
        rkcAv = amap[rldName];
        bool bval = false;
        amap["KeyCmd"] = vm->setLinkVal(vmap, aname, "/status", "KeyCmd", bval);
        amap["KeyCmdTries"] = vm->setLinkVal(vmap, aname, "/status", "KeyCmdTries", ival);

        if (reload < 1)
        {
            bool bval = false;
            amap["KeyCmd"]->setVal(bval);
            amap["KeyCmd"]->setParam("KeyCmdStartTime", 0.0);
            amap["KeyCmd"]->setParam("KeyCmdDone", bval);
            amap["KeyCmdTries"]->setVal(0);
        }
        reload = 2;
        rkcAv->setVal(reload);
        if(rldName)free(rldName);
        return 0;
    }

    if (amap["KeyCmd"]->getdParam("KeyCmdStartTime") == 0.0)
    {
        amap["KeyCmd"]->setParam("KeyCmdStartTime", tNow);
        bval = true;
        if (1) FPS_ERROR_PRINT("%s >>>> Starting [%s] at time %f\n",__func__,  rldName, tNow);
    }

    double KeyCmdStartTime = amap["KeyCmd"]->getdParam("KeyCmdStartTime");
    double maxKeyCmdTime = amap["KeyCmd"]->getdParam("maxKeyCmdTime");
    double maxKeyCmdOnTime = amap["KeyCmd"]->getdParam("maxKeyCmdOnTime");

    if (0) FPS_ERROR_PRINT("%s >>>> Running at time %f KeyCmdStartTime %f maxKeyCmdTime %f maxKeyCmdOnTime %f maxKeyCmdTries %d\n"
                    , rldName
                    , tNow
                    , KeyCmdStartTime
                    , maxKeyCmdTime
                    , maxKeyCmdOnTime
                    , amap["KeyCmd"]->getiParam("maxKeyCmdTries")
                    );

    amap["KeyCmdTries"]->setVal((int) ((tNow - KeyCmdStartTime) / maxKeyCmdTime));

    if (0) FPS_ERROR_PRINT("%s >>>> KeyCmdStartTime %f KeyCmdTries %d\n", rldName, amap["KeyCmd"]->getdParam("KeyCmdStartTime"), amap["KeyCmdTries"]->getiVal());

    if (!amap["KeyCmd"]->getbParam("KeyCmdDone"))
    {
        if (amap["KeyCmdTries"]->valueChangedReset() || bval)
        {
            if (amap["KeyCmdTries"]->getiVal() < amap["KeyCmd"]->getiParam("maxKeyCmdTries"))
            {
                amap["KeyCmd"]->setVal(true);
                if (1) FPS_ERROR_PRINT("%s >>>> Setting KeyCmd On Try no. %d at elapsed time %f\n", rldName, amap["KeyCmdTries"]->getiVal()+1, tNow - KeyCmdStartTime);
            }
            else
            {
                amap["KeyCmd"]->setParam("KeyCmdDone", true);
                if(rldName)free(rldName);
                return 0;
            }
        }
        if ((tNow - KeyCmdStartTime) > ((maxKeyCmdTime * amap["KeyCmdTries"]->getiVal()) + maxKeyCmdOnTime) && amap["KeyCmd"]->getbVal())
        {
            amap["KeyCmd"]->setVal(false);
            if (1) FPS_ERROR_PRINT("%s >>>> Setting KeyCmd Off Try no. %d at elapsed time %f\n", rldName, amap["KeyCmdTries"]->getiVal()+1, tNow - KeyCmdStartTime);
        }
    }
    if(rldName)free(rldName);
    return 0;
}
#endif