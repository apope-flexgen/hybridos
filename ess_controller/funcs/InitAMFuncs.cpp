#ifndef INITFUNCS_CPP
#define INITFUNCS_CPP

#include "asset.h"

/**
 *
 */
extern "C++" {

int PCSInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

int PCSInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    VarMapUtils* vm = aV->am->vm;
    varsmap* vlist = vm->createVlist();
    int ival = 1;

    linkVals(*vm, vmap, amap, aname, "/controls", ival, "EnPStartGradient", "EnPStopGradient", "EnPStopGradient",
             "EnPDropGradient");
    linkVals(*vm, vmap, amap, aname, "/controls", ival, "EnQStartGradient", "EnQStopGradient", "EnQStopGradient",
             "EnPQropGradient");
    linkVals(*vm, vmap, amap, aname, "/status", ival, "ActivePowerCmd");

    vm->addVlist(vlist, amap["EnPStartGradient"]);
    vm->addVlist(vlist, amap["EnPStopGradient"]);
    vm->addVlist(vlist, amap["EnPRiseGradient"]);
    vm->addVlist(vlist, amap["EnPDropGradient"]);
    vm->addVlist(vlist, amap["EnQStartGradient"]);
    vm->addVlist(vlist, amap["EnQStopGradient"]);
    vm->addVlist(vlist, amap["EnQRiseGradient"]);
    vm->addVlist(vlist, amap["EnQDropGradient"]);
    vm->addVlist(vlist, amap["ActivePowerCmd"]);

    if (!vlist->empty())
    {
        vm->sendVlist(p_fims, "set", vlist);
    }
    vm->clearVlist(vlist);

    return 0;
}

#endif