/*
* cant do it all in header files. This is the start of the real code migration
* this is the file for "released" or completed application functions
* as a function is getting ready to be released put it in here and we'll get working on signing it off
*
*/
#ifndef ASSET_FUNC_CPP
#define ASSET_FUNC_CPP

#include "asset.h"
#include "assetVar.h"
#include "assetFunc.h"
#include "varMapUtils.h"
// typedef int (*myAifun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset* ai);
// typedef int (*myAmfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager* am);
// typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);


/******************************************************
 *              
 *                 assetFunc.h
 *    
 ******************************************************/
//Deprecated
// heartbeat states 0 - init, 1- OK,2 - Alarm, 3 - fault 
int hbTestFunc::runFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
{
 
    return 0;
}

// Asset_Init = 0,
//     Asset_On ,
//     Asset_Off  ,
//     Asset_Standby,
//     Asset_Reset,
//     Asset_Alarm,
//     Asset_Fault,
//     Asset_Restart
int commsTestFunc::runFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
{
    return 0;
}
#include <chrono>
#include <ctime>
int forceAfTemplates()
{
    using namespace std::chrono;

   //ess_man = new asset_manager("ess_controller");
    varsmap vmap;
    VarMapUtils vm;
    asset_manager *am = new asset_manager("test");
    assetVar* av;
    am->am = nullptr;
    am->running = 1;
    char *cval=(char*)"1234";
    //vm->sysVec = &sysVec;

    am->vmap = &vmap;
    am->vm = &vm;

    bool bval = false;
    double dval = 0.0;
    int ival = 0;

    am->amap["bval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "bval", bval);
    am->amap["ival"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "ival", ival);
    am->amap["dval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "dval", dval);
    am->amap["cval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "cval", cval);
    av = am->amap["dval"];
    am->amap["Av"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "Av", av);


    av->addVal(dval);
    av->addVal(ival);
    av->addVal(dval);

    av->subVal(dval);
    av->subVal(ival);
    av->subVal(dval);
    
    av->setVal(dval);
    av->setVal(bval);
    av->setVal(ival);
    av->setVal((const char*)cval);
    av->setVal((char*)cval);

    av->setLVal(dval);
    av->setLVal(bval);
    av->setLVal(ival);
    av->setLVal((const char*)cval);
    
    // av->setParam((const char*)"i1",ival);
    // av->setParam((const char*)"d1",dval);
    // av->setParam((const char*)"b1",bval);
    // av->setParam((const char*)"t1",cval);
    // av->setParam((const char*)"av",av);

    ival = av->getiParam("i1");
    bval = av->getbParam("b1");
    dval = av->getdParam("d1");
    cval = av->getcParam("c1");

    //bval = av->valueChanged(dval,ival);
    bval = av->valueChanged();
    bval = am->vm->valueChanged(dval,dval);
    //bval = am->vm->valueChanged(dval,ival);
    bval = am->vm->valueChanged(av, av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, bval, dval);
    //bval = am->av->valueChangednodb(dval,dval);
    //bval = am->av->valueChangednodb(dval,bval);
    // av->setParam("d1",dval);
    // av->setParam("b1",bval);
    // av->setParam("c1",tval);
    //assetBitField bf(1,2,nullptr,nullptr,nullptr);
    assetBitField bf2(nullptr);
    ival = bf2.getFeat("d",&ival);
    cval = bf2.getFeat("d",&cval);

    system_clock::time_point now = system_clock::now();
    time_t tnow = system_clock::to_time_t(now);
    tm *local_tm = localtime(&tnow);
    char tbuffer[80];
    strftime (tbuffer,80,"%c.",local_tm);
    am->amap["timeString"]           = am->vm->setLinkVal(vmap, "test", "/status", "timeString", tbuffer);
    av->sendAlarm(av, nullptr, nullptr, 1);

    return 0;
}
/**************************************************************************************************************************************************************/

#endif
