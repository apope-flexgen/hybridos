/*
* this is the file for "released" or completed application functions
* as a function is getting ready to be released put it in here and we'll get working on signing it off
*
*/
#ifndef ASSET_FUNC_CPP
#define ASSET_FUNC_CPP

#include "asset.h"
#include "assetFunc.h"
#include "varMapUtils.h"
#include "chrono_utils.hpp"
typedef int (*myAifun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset* ai);
typedef int (*myAmfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager* am);
typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);

assetVar*VarMapUtils::runActFuncfromCj(varsmap &vmap, assetVar *av, assetAction* aa)
{
    // for now run the sets directly
    //std::map<int,assetBitField *> bitmap;
    //int aval = av->aVal->valueint;
    char *strval;
    if(av->aVal->valuestring)
    {
        strval = (char *)strdup(av->aVal->valuestring);
    }
    else
    {
        strval =(char*)strdup("NoVal");
    }
    // This runs each function in the AbitMap
    // x.first is the enry number
    // this is the list of functions
    for (auto &x : aa->Abitmap)
    {
        // Set up the bitMap Number
        av->abNum = x.first;
        assetBitField* abf = x.second;

        bool enable = true;
        char* ensp = nullptr;    abf->getFeat("enable", &ensp);
        if(ensp)
        {
            assetVar*av1 = getVar(vmap, (const char *)ensp, nullptr);
            if(av1)
            {
                enable = av1->getbVal();
            }
        }
        av->abf = abf;
        char* func = nullptr;
        abf->getFeat("func", &func);
        if(func)
        {
            // we have to find the function
            // where are they stored
            //vm->setFunc(vmap, "dcr_am", "TEST_VOLTAGE_LIMIT" , (void*) &run_bms_asset_wakeup);
            //vm->setFunc(vmap, "dcr_ai", "CHECK_CONTACTS" , (void*) &run_bms_asset_wakeup);


            //void (*tf)(void *) = (void (*tf)(void *))
            //void *res1 = vm->getFunc(vmap, "ess","run_init" );
            //void *res2 = vm->getFunc(vmap, "ess","run_wakeup" );
            // typedef void (*myAmInit_t)(asset_manager * data);
            //myAmInit_t myessMinit = myAmInit_t(res1);
        }

        if(1)FPS_ERROR_PRINT(" %s >> running a Function av [%s] func [%s] am %p ai %p enable [%s]\n"
                ,__func__, av->name.c_str()
                , func, av->am, av->ai
                , enable ? "true":"false"
                );
        if(av->ai)
        {
            if(1)FPS_ERROR_PRINT(" %s >> running a Function  for an asset instance  [%s] \n",__func__, av->ai->name.c_str() );
        }
        else if(av->am)
        {
            myAvfun_t amFunc;
            void *res1 = nullptr;
            if(av->am->vm)
            {
                // ess could be the amap feature 
                // ODO maybe use the amap field instead of hard coding ess here
                res1 = av->am->vm->getFunc(vmap, "ess", func );
                if(res1)
                {
                    amFunc = (myAvfun_t) res1;
                    amFunc(vmap, av->am->amap, av->am->name.c_str(), av->am->p_fims, av);
                }
            }
            else
            {
                if(0)FPS_ERROR_PRINT(" %s >> running a Function  for an asset manager with no VM please fix [%s] \n",__func__, av->ai->name.c_str() );
            }           
            if(0)FPS_ERROR_PRINT(" %s >> running a Function  for an asset manager  [%s] res1 %p \n",__func__, av->am->name.c_str(), res1 );
        }
    }
    free((void*)strval);
    return av;
}
// heartbeat states 0 - init, 1- OK,2 - Alarm, 3 - fault 
int hbTestFunc::runFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
{
    //double dval = 0.0;
    bool bval = false;
    int ival = 0;
    char * cval =  (char *) "HeartBeat Init";
    VarMapUtils* vm =am->vm;
    int reload = 0;
    // this loads up the errors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* CheckAssetHeartBeat = amap[__func__];

    //if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2) 
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;
        amap["HeartBeat"]              = vm->setLinkVal(vmap, aname,                 "/components", "HeartBeat",             hbInit);
        amap["HeartBeatTick"]          = vm->setLinkVal(vmap, aname,                 "/status",     "HeartBeatTick",         ival);
        

        amap["essHeartBeatFaults"]     = vm->setLinkVal(vmap, "ess",                 "/status",     "essHeartBeatFaults",    ival);
        amap["essHeartBeatAlarms"]     = vm->setLinkVal(vmap, "ess",                 "/status",     "essHeartBeatAlarms",     ival);

        if(am->am)
        {
            amap["amHeartBeatFaults"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartBeatFaults",        ival);
            amap["amHeartBeatAlarms"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "HeartBeatAlarms",         ival);
        }
        amap["HeartBeatFaults"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartBeatFaults",       ival);
        amap["HeartBeatAlarms"]        = vm->setLinkVal(vmap, aname,                "/status",     "HeartBeatAlarms",        ival);
        amap["HeartBeatState"]         = vm->setLinkVal(vmap, aname,                "/status",     "HeartBeatState",        cval);
        amap["HeartBeatStateNum"]      = vm->setLinkVal(vmap, aname,                "/status",     "HeartBeatStateNUm",     ival);
        amap["BypassHB"]               = vm->setLinkVal(vmap, aname,                "/controls",   "BypassHB",              bval);
        amap["AssetState"]             = vm->setLinkVal(vmap, aname,                "/status",     "AssetState",            ival);


        if(reload == 0) // complete restart 
        {
            amap["HeartBeat"]     ->setVal(hbInit);
            amap["HeartBeatTick"] ->setVal(ival);
            lastHeartBeat=hbInit;
            amap["HeartBeatState"]->setVal(cval);
            ival = Asset_Init;
            amap["HeartBeatStateNum"]->setVal(ival);
            amap["AssetState"]    ->setVal(Asset_Init);   // only do this in Comms

        }
        ival = 2; CheckAssetHeartBeat->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();
    int ival1 = 0;
    ival1 = amap["HeartBeat"]->getiVal();
    int ival2 = lastHeartBeat;//amap["lastHeartBeat"]->getiVal();
    int ival3 = amap["HeartBeatTick"]->getiVal();
    int ival4 = ival3+1;
    amap["HeartBeatTick"]->setVal(ival4);
    if (ival1 == hbInit)    
    {
        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        bool bval1 = false;
        bval1 = amap["BypassHB"]->getbVal();
        if(ival3 > 10)
        {
            if(0)FPS_ERROR_PRINT("%s >> %s  NO HeartBeat, bypass [%s] \n", __func__, aname,bval1?"true":"false");

            ival4 = 0;
            amap["HeartBeatTick"]->setVal(ival4);
        }
        char* cval = (char *)"HeartBeat Init no HB yet ";
        amap["HeartBeatState"]->setVal(cval);
        if(!bval1)
        {
            int ival = 1 ; 
            amap["HeartBeatFaults"]->addVal(ival);
            amap["essHeartBeatFaults"]->addVal(ival);
            if(amap["amHeartBeatFaults"])
            {
                amap["amHeartBeatFaults"]->addVal(ival);
            }
        }
    }
    else
    {
        double toVal  = amap["HeartBeat"]->getLastSetDiff(tNow);
        //if(1)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
        if(ival1 != ival2)
        { 
            ival = amap["HeartBeatStateNum"]->getiVal();
            if (ival != Asset_Ok)
            {
                if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%d] to [%d] \n", __func__, aname, ival2, ival1);
                ival = Asset_Ok;
                amap["HeartBeatStateNum"]->setVal(ival);

            }
            lastHeartBeat = ival1;
            char *tval;
            asprintf(&tval,"HeartBeat OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
            if(tval)
            {
                amap["HeartBeatState"]->setVal(tval);
                free((void *)tval);
            }

            // bokFault = false;
            // bokOK = true;
            // bokAlarm =  false;
        }
        else
        {
            double toVal  = amap["HeartBeat"]->getLastSetDiff(tNow);
            bool bypass = amap["BypassHB"]->getbVal();

            if ((toVal > toFault)  && !bokFault && !bypass)
            {
                char *tval;
                asprintf(&tval,"HeartBeat Faultor last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["HeartBeatState"]->setVal(tval);
                    free((void *)tval);
                }
                int ival = 1 ; 
                amap["HeartBeatFaults"]->addVal(ival);
                amap["essHeartBeatFaults"]->addVal(ival);
                if(am->am)
                {
                    amap["amHeartBeatFaults"]->addVal(ival);
                }
                amap["AssetState"]    ->setVal(Asset_Fault);   // only do this in Comms
                ival = Asset_Fault;
                amap["HeartBeatStateNum"]->setVal(ival);

                bokFault = true;
                bokOK = false;
                bokAlarm =  true;
                totalHBFaults++;

            }
            else if ((toVal > toAlarm)  && !bokAlarm)
            {
                char *tval;
                asprintf(&tval," HeartBeat Alarming last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["HeartBeatState"]->setVal(tval);
                    free((void *)tval);
                }
                int ival = 1; 
                amap["HeartBeatAlarms"]->addVal(ival);
                amap["essHeartBeatAlarms"]->addVal(ival);
                if(am->am)
                {
                    amap["amHeartBeatAlarms"]->addVal(ival);
                }
                amap["AssetState"]    ->setVal(Asset_Alarm);   // only do this in Comms
                ival = Asset_Alarm;
                amap["HeartBeatStateNum"]->setVal(ival);
                bokAlarm = true;
                bokOK = false;
                bokFault = false; 
                totalHBAlarms++;
            }
            else
            {
                if (!bokOK)
                {
                    char *tval;
                    asprintf(&tval," HeartBeat OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if(tval)
                    {
                        amap["HeartBeatState"]->setVal(tval);
                        free((void *)tval);
                    }
                    amap["AssetState"]    ->setVal(Asset_On);   // only do this in Comms

                    bokOK = true;
                    bokAlarm = false;
                    bokFault = false;
                }
            } 
        }          
    }
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
    //double dval = 0.0;
    int ival = 0;
    bool bval = false;
    char* cval =  (char*) "Comms Init";
    VarMapUtils* vm =am->vm;
    int reload = 0;
    // this loads up the Faultors in the asset manager
    reload = vm->CheckReload(vmap, amap, aname, "CheckAssetComms");
    assetVar* CheckAssetComms = amap["CheckAssetComms"];

    
    //if(1)FPS_FaultOR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
    if (reload < 2) 
    {
        ival = 0;
        //dval = 1.0;
        //bool bval = false;

        amap["Timestamp"]            = vm->setLinkVal(vmap, aname,                "/components", "Timestamp",          tsInit);

        amap["essCommsFaults"]       = vm->setLinkVal(vmap, "ess",                "/status",     "essCommsFaults",     ival);
        amap["essCommsAlarms"]       = vm->setLinkVal(vmap, "ess",                "/status",     "essCommsAlarms",     ival);
        amap["essCommsInit"]         = vm->setLinkVal(vmap, "ess",                "/status",     "essCommsInit",     ival);

        if(am->am)
        {
            amap["amCommsFaults"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsFaults",         ival);
            amap["amCommsAlarms"]  = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsAlarms",         ival);
            amap["amCommsInit"]    = vm->setLinkVal(vmap, am->am->name.c_str(), "/status",    "CommsInit",           ival);
        }
        amap["CommsFaults"]        = vm->setLinkVal(vmap, aname,                "/status",     "CommsFaults",       ival);
        amap["CommsAlarms"]        = vm->setLinkVal(vmap, aname,                "/status",     "CommsAlarms",       ival);
        amap["CommsInit"]          = vm->setLinkVal(vmap, aname,                "/status",     "CommsInit",         ival);
        amap["CommsState"]         = vm->setLinkVal(vmap, aname,                "/status",     "CommsState",        cval);
        amap["BypassComms"]        = vm->setLinkVal(vmap, aname,                "/controls",   "BypassComms",       bval);
        amap["AssetState"]         = vm->setLinkVal(vmap, aname,                "/status",     "AssetState",        ival);
        amap["CommsStateNum"]      = vm->setLinkVal(vmap, aname,                "/status",     "CommsStateNum",      ival);
        
        if(reload == 0) // complete restart 
        {
            amap["Timestamp"]     ->setVal(tsInit);
            lastTimestamp=strdup(tsInit);//state"]->setVal(cval);
            amap["CommsState"]  ->setVal(cval);
            ival = Asset_Init; amap["CommsStateNum"]  ->setVal(ival);
            ival = 1; amap["CommsInit"]  ->setVal(ival);

        }
        ival = 2; CheckAssetComms->setVal(ival);
    }

    double tNow = am->vm->get_time_dbl();
    char* tval1 = nullptr;
    tval1 = amap["Timestamp"]->getcVal();
    char* tval2 = lastTimestamp;//amap["lastHeartBeat"]->getiVal();
    if (strcmp(tval1, tsInit)==0)    
    {
        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        bool bval1 = amap["BypassComms"]->getbVal();
        //ival = 1; amap["CheckAssetComs"]->setVal(ival);
        if(0)FPS_ERROR_PRINT("%s >> %s  NO Timestamp,  bypass [%s]\n", __func__, aname, bval1?"true":"false");
        char* cval = (char *)"Comms Init no HB yet ";
        amap["CommsState"]->setVal(cval);
        if(!bval1)
        {
            if(amap["amCommsInit"])
            {
                ival = 1;
                amap["amCommsInit"]->addVal(ival);
            }
            ival = 1;
            amap["essCommsInit"]->addVal(ival);
            amap["CommsInit"]->setVal(ival);

        }
    }
    else
    {
        double toVal  = amap["Timestamp"]->getLastSetDiff(tNow);
        if(0)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTimestamp?lastTimestamp:"no last Value", tval1);
        if (strcmp(tval1, tval2) !=0)    
        { 
            ival = amap["CommsStateNum"]  ->getiVal();
            if(ival != Asset_Ok)
            {
                if(1)FPS_ERROR_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTimestamp?lastTimestamp:"no last Value", tval1);
                ival = Asset_Ok; // seen Timestamp change

                amap["CommsStateNum"]  ->setVal(ival);
                ival = 0;
                amap["CommsInit"]->setVal(ival);

            }
            //if(1)FPS_Fault_PRINT("%s >>  ts  change for %s from [%s] to [%s] \n", __func__, aname, lastTs?lastTs:"no last Value", Ts);
            if(lastTimestamp)
               free((void*)lastTimestamp);
            lastTimestamp = strdup(tval1);
            char *tval;
            asprintf(&tval," Comms OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
            if(tval)
            {
                amap["CommsState"]->setVal(tval);
                free((void *)tval);
            }
        }
        else
        {
            double toVal  = amap["Timestamp"]->getLastSetDiff(tNow);
            bool bypass = amap["BypassComms"]->getbVal();

            if ((toVal > toFault)  && !bokFault && !bypass)
            {
                char *tval;
                asprintf(&tval," Comms Fault last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["CommsState"]->setVal(tval);
                    free((void *)tval);
                }
                int ival = 1 ; 
                amap["CommsFaults"]->addVal(ival);
                amap["essCommsFaults"]->addVal(ival);

                if(am->am)
                {
                    amap["amCommsFaults"]->addVal(ival);
                }


                ival = Asset_Fault; //Timestamp Fault
                amap["CommsStateNum"]  ->setVal(ival);

                bokFault = true;
                bokOK = false;
                bokAlarm =  false;
                totalCommsFaults++;

            }
            else if ((toVal > toAlarm)  && !bokAlarm)
            {
                char *tval;
                asprintf(&tval,"Comms Alarm last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                if(tval)
                {
                    amap["CommsState"]->setVal(tval);
                    free((void *)tval);
                }

                int ival = 1; 
                amap["CommsAlarms"]->addVal(ival);
                amap["essCommsAlarms"]->addVal(ival);

                if(am->am)
                {
                    amap["amCommsAlarms"]->addVal(ival);
                }
                ival = Asset_Alarm; //Timestamp Alarm
                amap["CommsStateNum"]  ->setVal(ival);

                bokAlarm = true;
                bokOK = false;
                bokFault = false; 
                totalCommsAlarms++;
            }
            else
            {
                if (!bokOK)
                {
                    char *tval;
                    asprintf(&tval,"Comms OK last set %2.3f Alarm %3.2f max %3.2f", toVal, toAlarm, toFault);
                    if(tval)
                    {
                        amap["CommsState"]->setVal(tval);
                        free((void *)tval);
                    }
                    ival = Asset_Ok; //Timestamp OK
                    amap["CommsStateNum"]  ->setVal(ival);

                    bokOK = true;
                    bokAlarm = false;
                    bokFault = false;
                }
            }            
        }
    }
    //
    //int ival1, ival2;
    //if(1)FPS_Fault_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
    return 0;
};

cJSON* assetFeatDict::getFeat(const char* name, cJSON** cj)
    {
        if (featMap.find(name) != featMap.end())
        {
            cJSON* cjf = cJSON_CreateObject();

            assFeat* af = featMap[name];
            switch (af->type)
            {
            case assFeat::AINT:
                cJSON_AddNumberToObject(cjf, "value", af->valueint);
                break;
            case assFeat::AFLOAT:
                cJSON_AddNumberToObject(cjf, "value", af->valuedouble);
                break;
            case assFeat::ASTRING:
                cJSON_AddStringToObject(cjf, "value", af->valuestring);

            case assFeat::AAVAR:
                // TODO get full value
                if(af->av)
                {
                    cJSON_AddStringToObject(cjf, "avar", af->av->name.c_str());
                }
                break;
                
            case assFeat::ABOOL:
                if (af->valuebool)
                {
                    cJSON_AddTrueToObject(cjf, "value");
                }
                else
                {
                    cJSON_AddFalseToObject(cjf, "value");
                }
                break;
            default:
                cJSON_AddStringToObject(cjf, "value", "FeatValNotKnown");

            };
            *cj = cjf;
            return cjf;
        }
        //enum AFTypes {AINT, AFLOAT, ASTRING, ABOOL, AEND};
        return nullptr;
    }

// VarMapUtils* assetVar::getVm()
// {
//     if (am && am->vm)
//     {
//         return(am->vm);
//     }
//     if (ai && ai->am && ai->am->vm)
//     {
//         return(ai->am->vm);
//     }
//     return nullptr;
// }
void assetFeatDict::showCj(cJSON* cjix)
{
    for (auto x : featMap)
    {
        //cJSON_AddNumberToObject(cjix,"fmIndex", fm++); 

        assFeat* af = x.second;
        switch (af->type) {
        case assFeat::AFLOAT:
            cJSON_AddNumberToObject(cjix, af->name.c_str(), af->valuedouble);
            break;
        case assFeat::AINT:
            cJSON_AddNumberToObject(cjix, af->name.c_str(), af->valueint);
            break;
        case assFeat::ASTRING:
            cJSON_AddStringToObject(cjix, af->name.c_str(), af->valuestring);
            break;
        case assFeat::AAVAR:
            char* tmp;
            asprintf(&tmp,"av::%s/%s",af->av->comp.c_str(),af->av->name.c_str());
            cJSON_AddStringToObject(cjix, af->name.c_str(), tmp);
            free((void*)tmp);
            break;
        case assFeat::ABOOL:
            if (af->valuebool)
                cJSON_AddTrueToObject(cjix, af->name.c_str());
            else
                cJSON_AddFalseToObject(cjix, af->name.c_str());
            break;
        default:
            cJSON_AddStringToObject(cjix, af->name.c_str(), "TypeNotDefined");
            break;
        }
    }
}
#endif
