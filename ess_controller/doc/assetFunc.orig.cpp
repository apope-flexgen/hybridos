/*
* cant do it all in header files. This is hte start of the real code migration
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
typedef int (*myAifun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset* ai);
typedef int (*myAmfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager* am);
typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);

static int setvar_debug = 0;
static int process_fims_debug = 0;

static double g_setTime = 0.0;
static int debug_action = 0;
static int setvar_debug_asset = 0;

/******************************************************
 *              
 *                 assetFunc.h
 *    
 ******************************************************/
//Deprecated
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

    
    //if(1)FPS_ERROR_PRINT("%s >>  reload first for  %s , is  %d \n", __func__, aname, reload);
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
    //if(1)FPS_ERROR_PRINT("%s >>  result for  %s , Alarms %d, errs %d \n", __func__, aname, amap["CommsAlarms"]->getiVal(),amap["CommsFaults"]->getiVal());
    return 0;
}

/**************************************************************************************************************************************************************/

/******************************************************
 *              
 *                 assetVar.h
 *    
 ******************************************************/

/***********************************************
 *                 asset_log
 ***********************************************/
asset_log::asset_log() 
{
    srcAv = nullptr;
    destAv = nullptr;
    destIdx = -1;
    severity = -1;
    log_time = 0.0;
    aVal = nullptr;

}

asset_log::asset_log(void* src, void* dest, const char* atype, const char* msg, int sev, double ltime) 
{
    srcAv = src;
    destAv = dest;
    if(atype)altype = atype;
    almsg = msg;
    severity = sev;
    log_time = ltime;
    aVal = nullptr;

    // if(ltime == 0.0)
    //     log_time = src->aVal?src->aVal->setTime:0.0;
    // name = src->name;
    // comp=src->comp;
}

void asset_log::setDestIdx(int idx)
{
    destIdx = idx;
}

asset_log::~asset_log()
{
    FPS_ERROR_PRINT(" asset alarm cleanup\n");
}

/***********************************************
 *                 assFeat
 ***********************************************/
assFeat::assFeat(const char* _name, int val)
{
    valueint = val;
    valuedouble = val;
    type = AINT;
    name = _name;
    valuestring = nullptr;
    av=nullptr;
}
assFeat::assFeat(const char* _name, double val)
{
    valuedouble = val;
    valueint = val;
    type = AFLOAT;
    name = _name;
    valuestring = nullptr;
    av=nullptr;
}
assFeat::assFeat(const char* _name, bool val)
{
    valuebool = val;
    type = ABOOL;
    name = _name;
    valuestring = nullptr;
    av=nullptr;
}

assFeat::assFeat(const char* _name, const char* val)
{
    valuestring = strdup(val);
    type = ASTRING;
    name = _name;
    av=nullptr;
}
assFeat::assFeat(const char* _name, assetVar* val)
{
    valuestring = nullptr;//strdup(val);
    type = AAVAR;
    name = _name;
    av=val;
}

assFeat::~assFeat()
{
    if (valuestring)free((void*)valuestring);
}
assFeat::assFeat(const assFeat& other)
    : name(other.name), type(other.type), valuedouble(other.valuedouble), valueint(other.valueint), valuebool(other.valuebool), av(other.av)
{
    if (valuestring)
    {
        free((void*)valuestring);
        valuestring = nullptr;
    }
    if (other.valuestring)
    {
        valuestring = strdup(other.valuestring);
    }
}

assFeat& assFeat::operator=(const assFeat& other)
{
    if (this != &other)
    {
        name = other.name;
        type = other.type;
        valuedouble = other.valuedouble;
        valueint = other.valueint;
        valuebool = other.valuebool;
        av = other.av;
        if (valuestring)
        {
            free((void*)valuestring);
            valuestring = nullptr;
        }
        if (other.valuestring)
        {
            valuestring = strdup(other.valuestring);
        }
    }
    return *this;
}

/***********************************************
 *                 assetFeatDict
 ***********************************************/
assetFeatDict::assetFeatDict() {
    tmpval = nullptr;
}

assetFeatDict::~assetFeatDict() {
    for (auto x : featMap)
    {
        delete x.second;
    }
    featMap.clear();
    if (tmpval)
        free((void*)tmpval);
}

assetFeatDict::assetFeatDict(const assetFeatDict& other)
{
    tmpval = strdup(other.tmpval);

    std::map<std::string, assFeat*>::const_iterator it = other.featMap.begin();
    while (it != other.featMap.end())
    {
        featMap[it->first] = new assFeat(*(it->second));
        ++it;
    }
}
assetFeatDict& assetFeatDict::operator=(const assetFeatDict& other)
{
    if (this != &other)
    {
        if (tmpval)
            free((void*)tmpval);
        tmpval = other.tmpval ? strdup(other.tmpval) : nullptr;

        std::map<std::string, assFeat*>::const_iterator it = other.featMap.begin();
        while (it != other.featMap.end())
        {
            featMap[it->first] = new assFeat(*(it->second));
            ++it;
        }
    }
    return *this;
}

int assetFeatDict::addCj(cJSON* cj, int uiObject, bool skipName)
{
    if (0)
    {
        char* stmp = cJSON_PrintUnformatted(cj);
        FPS_ERROR_PRINT(" %s >>  Feat       >>%s<< child %p \n", __func__, stmp, (void*)cj->child);
        free((void*)stmp);
    }
    cJSON* cjic = cj->child;

    while (cjic)
    {
        if (0)
        {
            char* stmp2 = cJSON_PrintUnformatted(cjic);
            FPS_ERROR_PRINT(" %s >>   stmp2 >>%s<< name [%s] child %p next %p \n"
                , __func__
                , stmp2
                , cjic->string
                , (void*)cjic->child
                , (void*)cjic->next
            );
            free((void*)stmp2);
        }
        //addFeat(cjic);
        if (uiObject 
                && ((strcmp(cjic->string, "value") == 0)
                || (/*uiObject &&*/ (skipName && strcmp(cjic->string, "name") == 0))))
        {
            if (0)FPS_ERROR_PRINT(" %s >> skipping [%s] as a base param  its special \n", __func__, cjic->string);
        }
        else
        {
            addFeat(cjic);
            //TODO review this had problems with cj outValue
            if (cjic->string)
            {
                if (strcmp(cjic->string, "outValue") == 0)
                {
                    if (tmpval)
                        free((void*)tmpval);
                    tmpval = cJSON_PrintUnformatted(cjic);
                }
            }
        }

        cjic = cjic->next;
    }
    if (0)
    {
        FPS_ERROR_PRINT(" %s >>   Features added \n", __func__);
        showFeat();
        FPS_ERROR_PRINT(" %s >>   Features done \n", __func__);
    }
    return 0;
}


// test if we have a feature
//bool assetFeatDict::gotFeat(const char* name);

int assetFeatDict::getFeat(const char* name, int* val)
{
    *val = 0;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        *val = af->valueint;
    }
    return *val;
}

bool assetFeatDict::getFeat(const char* name, bool* val)
{
    *val = true;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        *val = af->valuebool;
    }
    return *val;
}

double assetFeatDict::getFeat(const char* name, double* val)
{
    *val = 0.0;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        *val = af->valuedouble;
    }
    return *val;
}

char* assetFeatDict::getFeat(const char* name, char** val)
{
    *val = nullptr;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        *val = af->valuestring;
    }
    return *val;
}

assetVar* assetFeatDict::getFeat(const char* name, assetVar** val)
{
    *val = nullptr;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        *val = af->av;
    }
    return *val;
}

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

        }
        *cj = cjf;
        return cjf;
    }
    //enum AFTypes {AINT, AFLOAT, ASTRING, ABOOL, AEND}
    return nullptr;
}

//assetFeatDict
// moved to assetFunc.cpp
//cJSON* assetFeatDict::getFeat(const char* name, cJSON** cj);

// test if we have a feature
bool assetFeatDict::gotFeat(const char* name)
{
    
    if (featMap.find(name) != featMap.end())
    {
        return true;
    }
    return false;
}

void assetFeatDict::setFeat(const char* name, cJSON *cj)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = nullptr;
        //TODO fix according to cj->type
        if(cj->child) cj = cj->child;
        if(cJSON_IsTrue(cj)||cJSON_IsFalse(cj))
        {
            bool bval = cJSON_IsTrue(cj);
            af = new assFeat(name, bval);
        }
        else if(cJSON_IsString(cj))
        {
            af = new assFeat(name, cj->valuestring);
        }
        else if(cJSON_IsNumber(cj))
        {
            if(cj->valuedouble == (double)cj->valueint)
            {
                af = new assFeat(name, cj->valueint);
            }
            else
            {
                af = new assFeat(name, cj->valuedouble);
            }
        }
        else
        {
            FPS_ERROR_PRINT("%s >> unmanaged cJSON type %d child %p\n",__func__, cj->type, cj->child);
        }


        if(af) featMap[name]= af;
    }
    else
    {
        if(0)FPS_ERROR_PRINT("%s >> name [%s] cJSON type %d child %p\n",__func__, name, cj->type, cj->child);
        if(cj->child) cj=cj->child;

        featMap[name]->valuedouble = cj->valuedouble;            
        featMap[name]->valueint = cj->valueint;
        featMap[name]->valuebool = cJSON_IsTrue(cj);
        if(cj->valuestring)
        {
            if (featMap[name]->valuestring)
            {
                free((void*)featMap[name]->valuestring);
            }
            featMap[name]->valuestring = strdup(cj->valuestring);
        }

    }
    return;
}

void assetFeatDict::setFeat(const char* name, assetVar* val)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);

        featMap[name]= af;
    }
    else
    {
        featMap[name]->av = val;            
    }
    return;
}


void assetFeatDict::setFeat(const char* name, double val)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);

        featMap[name]= af;
    }
    else
    {
        featMap[name]->valuedouble = val;
        featMap[name]->valueint = val;
        
    }
    return;
}
void assetFeatDict::setFeat(const char* name, int val)
{
    if (featMap.find(name) == featMap.end())
    {
        FPS_ERROR_PRINT(" %s >> set new int Feat [%s] value %d\n",__func__, name, val);
        assFeat* af = new assFeat(name, val);

        featMap[name]= af;
    }
    else
    {
        if (0)FPS_ERROR_PRINT(" %s >> set old int Feat [%s] value %d\n",__func__, name, val);

        featMap[name]->valuedouble = val;
        featMap[name]->valueint = val;
        
    }
    return;
}
void assetFeatDict::setFeat(const char* name, bool val)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);

        featMap[name]= af;
    }
    else
    {
        featMap[name]->valuebool = val;
        featMap[name]->valueint = val;
        
    }
    return;
}

void assetFeatDict::setFeat(const char* name, const char* val)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);

        featMap[name]= af;
    }
    else
    {
        if(featMap[name]->valuestring)
            free((void*)featMap[name]->valuestring);
        featMap[name]->valuestring = nullptr;
        if(val)
            featMap[name]->valuestring = strdup(val);
    }
    return;
}


template<class T>
void assetFeatDict::addFeat(const char* name, T val)
{
    if (featMap[name] != nullptr)
    {
        FPS_ERROR_PRINT("%s >>  note we are replacing param [%s]\n"
            , __func__
            , name
        );
        delete featMap[name];
    }
    FPS_ERROR_PRINT(" %s >> added  Feat [%s] \n",__func__, name);

    featMap[name] = new assFeat(name, val);
}


void assetFeatDict::addFeat(cJSON* cj)
{
    switch (cj->type) {
    case cJSON_Number:
        addFeat(cj->string, cj->valuedouble);
        break;
    case cJSON_String:
        addFeat(cj->string, cj->valuestring);
        break;
    case cJSON_True:
        addFeat(cj->string, true);
        break;
    case cJSON_False:
        addFeat(cj->string, false);
        break;
    default:
        //asprintf(&stmp,"Unknown");
        break;
    }
}

void assetFeatDict::showFeat()
{
    for (auto x : featMap)
    {
        assFeat* af = x.second;
        char* stmp;
        switch (af->type) {
        case assFeat::AFLOAT:
            asprintf(&stmp, "%s->[%f]", af->name.c_str(), af->valuedouble);
            break;
        case assFeat::AINT:
            asprintf(&stmp, "%s->[%d]", af->name.c_str(), af->valueint);
            break;
        case assFeat::ASTRING:
            asprintf(&stmp, "%s->[%s]", af->name.c_str(), af->valuestring);
            break;
        case assFeat::ABOOL:
            if (af->valuebool)
                asprintf(&stmp, "%s->[true]", af->name.c_str());
            else
                asprintf(&stmp, "%s->[true]", af->name.c_str());
            break;
        default:
            asprintf(&stmp, "Unknown");
            break;
        }
        FPS_ERROR_PRINT(" Feature>>%s\n", stmp);
        free((void*)stmp);
    }
}

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


/***********************************************
 *                 assetBitField
 ***********************************************/
assetBitField::assetBitField(int _mask, int _bit, const char* _uri, const char* _var, char* tmp)
{
    mask = _mask;
    bit = _bit;
    uri = strdup(_uri);
    var = _var ? strdup(_var) : nullptr;
    //tmpval = nullptr;

    // if(tmp)
    //     tmpval = strdup(tmp);

    featDict = new assetFeatDict();
    if (tmp)
        featDict->tmpval = strdup(tmp);


    //featMap["mask"] = new assFeat("mask",_mask);
    //featMap["bit"] = new assFeat("bit",_bit);
    //featMap["uri"] = new assFeat("uri",_uri);

    //featMap["bvalue"] = new assFeat("bvalue",tmp);

    //if(tmp)
    //    featMap["tmpval"] = new assFeat("tmpval",tmp);

}

assetBitField::assetBitField(cJSON* cj)
{
    mask = 0;
    bit = 0;
    uri = nullptr;
    var = nullptr;
    //tmpval = nullptr;
    featDict = new assetFeatDict();
    featDict->addCj(cj);

}

assetBitField::~assetBitField() {
    if (uri)free((void*)uri);
    if (var)free((void*)var);
    //if(tmpval) free((void*)tmpval);
    delete featDict;

}

template<class T>
T assetBitField::getFeat(const char* name, T* val)
{
    return featDict->getFeat(name, val);

}

cJSON* assetBitField::getFeat(const char* name, cJSON** cj)
{
    return featDict->getFeat(name, cj);

}

bool assetBitField::gotFeat(const char* name)
{
    return featDict->gotFeat(name);

}

template<class T>
void assetBitField::addFeat(const char* name, T val)
{
    featDict->addFeat(name, val);
}


void assetBitField::showFeat()
{
    featDict->showFeat();
}

char* assetBitField::getTmpval()
{
    return featDict->tmpval;
}

/***********************************************
 *                 assetAction
 ***********************************************/
assetAction::assetAction(const char* aname)
{
    name = aname;
    idx = 0;
    //runFunc =  nullptr;
}

assetAction::~assetAction()
{
    for (auto& x : Abitmap)
    {
        delete Abitmap[x.first];
    }
    Abitmap.clear();
}

// TODO add all this stuff into a Feat Dict
assetBitField* assetAction::assetAction::addBitField(cJSON* cjbf)
{
    int idd = idx++;
    if (0)
    {
        char* tmp = cJSON_PrintUnformatted(cjbf);
        FPS_ERROR_PRINT(" %s >> Adding bitfield at [%d] [%s] \n", __func__, idd, tmp);
        free((void*)tmp);
    }
    Abitmap[idd] = (new assetBitField(cjbf));
    return Abitmap[idd];
}
void assetAction::showBitField(int show)
{
    for (auto& x : Abitmap)
    {
        if (show)FPS_ERROR_PRINT("%s >> Bitfield [%d] name [%s]\n"
            , __func__
            , x.first
            , name.c_str()
        );
        assetBitField* abf = x.second;
        for (auto& y : abf->featDict->featMap)
        {
            if (show)FPS_ERROR_PRINT("%s >> Bitfield [%d] feature[%s]\n"
                , __func__
                , x.first
                , y.first.c_str()
            );

        }
    }
}
assetBitField* assetAction::getBitField(int num)
{
    assetBitField* abf = nullptr;
    auto x = Abitmap[num];
    if (x)
    {
        abf = Abitmap[num];
    }
    return abf;
}


assFeat* assetAction::getFeat(int num, const char* aname)
{
    assFeat* af = nullptr;
    assetBitField* abf = getBitField(num);
    if (abf)
    {
        auto x = abf->featDict->featMap[aname];
        if (x)
        {
            af = abf->featDict->featMap[aname];
        }
    }
    return af;
}

/***********************************************
 *                 assetList
 ***********************************************/
assetList::assetList() 
{
    name = "Unknown";
}
assetList::assetList(const char* _name) 
{
    name = _name;
}

assetList::~assetList() 
{
    if(0)FPS_ERROR_PRINT("%s >> 3 delete aList %p %s\n",__func__, &aList,name.c_str());

    //TODO clean vector
}

const char* assetList::getName()
{
    return name.c_str();
}

/***********************************************
 *                 assetOptVec
 ***********************************************/
assetOptVec::assetOptVec() {
    cjopts = nullptr;
    // tmpval = nullptr;
}

assetOptVec::~assetOptVec() {
    if (cjopts)
    {
        cJSON_Delete(cjopts);
    }
}

void assetOptVec::showCj(cJSON* cj)
{
    cJSON* cjo = cJSON_Duplicate(cjopts, true);
    cJSON_AddItemToObject(cj, "options", cjo);
}

// we may need to get the child object
void assetOptVec::addCj(cJSON* cj)
{
    if (0)FPS_ERROR_PRINT("%s >> adding options array [%s] child %p type %d\n"
        , __func__
        , cj->string ? cj->string : " no String"
        , cj->child
        , cj->type
    );

    if (cj->string)
    {
        if (!cjopts)
        {
            cjopts = cJSON_Duplicate(cj, true);
        }
        else
        {

            cJSON* cji = nullptr;
            cJSON_ArrayForEach(cji, cj)
            {
                cJSON* cja = cJSON_Duplicate(cji, true);
                cJSON_AddItemToArray(cjopts, cja);
            }
        }
    }
}

/***********************************************
 *                 assetVal
 ***********************************************/
assetVal::assetVal()
{
    valuedouble = 0.0;
    valueint = 0;
    type = AINT;
    valuestring = nullptr;
    av =  nullptr;
    //db=nullptr;
}

assetVal::assetVal(int val):assetVal()
{
    //assetVal();
    valuedouble = val;
    valueint = val;
}

assetVal::assetVal(double val):assetVal()
{
    //assetVal();
    valuedouble = val;
    valueint = (int)val;
    type = AFLOAT;
}

assetVal::assetVal(const char* val):assetVal()
{
    //assetVal();
    valuedouble = 0;
    valueint = 0;
    type = ASTRING;
}

assetVal::assetVal(bool val):assetVal()
{
    //assetVal();
    valueint = (val == true);
    valuedouble = valueint;
    valuebool = val;
    type = ABOOL;
}

assetVal::assetVal(assetVar * val):assetVal()
{
    //assetVal();
    av =  val;
    type = AAVAR;
}

assetVal::~assetVal() 
{
    if (valuestring != nullptr)
        free((void*)valuestring);
}

bool assetVal::getVal(bool val) {
    val = valuebool;
    return val;
}
int assetVal::getVal(int val) {
    val = valueint;
    return val;
}
double assetVal::getVal(double val) {
    val = valuedouble;
    return val;
}
char* assetVal::getVal(char* val) {
    val = valuestring;
    return val;
}
assetVar* assetVal::getVal(assetVar** val) {
    *val = av;
    return av;
}

bool assetVal::IsNumeric()
{
    return (type != ABOOL && type != ASTRING);
}
bool assetVal::IsString()
{
    return (type == ASTRING);
}
bool assetVal::IsBool()
{
    return (type == ABOOL);
}
bool assetVal::IsAv()
{
    return (type == AAVAR);
}

void assetVal::setVal(int val)
{
    //std::unique_lock<std::mutex> lock(m);
    valueint = val;
    valuedouble = val;
    setTime = g_setTime;
}

void assetVal::setVal(double val)
{
    valuedouble = val;
    valueint = (int)val;
    setTime = g_setTime;
    //type=AFLOAT;
}

void assetVal::setVal(bool val)
{
    if (0)FPS_ERROR_PRINT(" %s >> setVal bool  called \n"
        , __func__
    );
    valueint = val;
    valuebool = val;
    setTime = g_setTime;
}

void assetVal::setVal(const char* val)
{
    if (valuestring)
    {
        free((void*)valuestring);
    }

    if (val)
    {
        valuestring = strdup(val);
        //type=ASTRING;
    }
    setTime = g_setTime;
}
// TODO this seems to be broken  well maybe not
bool assetVal::setVal(cJSON* cj)
{
    if (0)FPS_ERROR_PRINT(" %s >> running, cjson type %d \n", __func__, cj->type);
    if (cJSON_IsBool(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT(" %s >>body  is a cjson bool value [%s]\n", __func__, cJSON_IsTrue(cj) ? "true" : "false");
        setVal((bool)cJSON_IsTrue(cj));
        return true;
    }
    else if (cJSON_IsNumber(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT("%s >>  body  is a cjson numerical value [%f]\n", __func__, cj->valuedouble);
        setVal(cj->valuedouble);
        return true;
    }
    else if (cJSON_IsString(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT(" %s >> body  is a cjson string value [%s]\n", __func__, cj->valuestring);
        setVal(cj->valuestring);
        return true;
    }
    else if (cJSON_IsObject(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT(" %s >> body  is a cjson Object  try child [%p]\n", __func__, (void*)cj->child);
        return setVal(cj->child);
        return true;
    }
    else
    {
        FPS_ERROR_PRINT(" %s >>body  [%s] type %d cannot be simply decoded\n", __func__, cj->string, cj->type);
    }
    //setTime = g_setTime;
    return false;
}

void assetVal::setType(ATypes t)
{
    type = t;
}
double assetVal::getsTime()
{
    return setTime;
}

double assetVal::getcTime()
{
    return chgTime;
}

assetVal::assetVal(const assetVal& other)
    : type(other.type), valuedouble(other.valuedouble), valueint(other.valueint), valuebool(other.valuebool), setTime(other.setTime)
{
    if (valuestring)
    {
        free((void*)valuestring);
        valuestring = nullptr;
    }
    if (other.valuestring)
    {
        valuestring = strdup(other.valuestring);
    }
    //*db = *other.db;
}
assetVal& assetVal::operator=(const assetVal& other)
{
    if (this != &other)
    {
        type = other.type;
        valuedouble = other.valuedouble;
        valueint = other.valueint;
        valuebool = other.valuebool;
        setTime = other.setTime;
        //IsDiff = other.IsDiff;
        if (valuestring)
        {
            free((void*)valuestring);
            valuestring = nullptr;

        }
        if (other.valuestring)
        {
            valuestring = strdup(other.valuestring);
        }
        //*db = *other.db;
    }
    return *this;
}

// may need get raw
cJSON* assetVal::getValCJ()
{
    cJSON* cj = cJSON_CreateObject();
    if (type == AINT)
    {
        cJSON_AddNumberToObject(cj, "value", valueint);
    }
    else if (type == AFLOAT)
    {
        cJSON_AddNumberToObject(cj, "value", valuedouble);
    }
    else if (type == ASTRING)
    {
        cJSON_AddStringToObject(cj, "value", valuestring);
    }
    else if (type == ABOOL)
    {
        if (valuebool)
            cJSON_AddTrueToObject(cj, "value");
        else
            cJSON_AddFalseToObject(cj, "value");
    }
    return cj;
}

// is this used anywhere ?? 
cJSON* assetVal::getValCJ(double scale, double offset)
{
    if (scale == 0.0)
        scale = 1.0;

    cJSON* cj = cJSON_CreateObject();
    if (type == AINT)
    {
        cJSON_AddNumberToObject(cj, "value", (int)((valueint * scale) - offset));
    }
    else if (type == AFLOAT)
    {
        cJSON_AddNumberToObject(cj, "value", (valuedouble * scale) - offset);
    }
    else if (type == ASTRING)
    {
        cJSON_AddStringToObject(cj, "value", valuestring);
    }
    else if (type == ABOOL)
    {
        if (valuebool)
            cJSON_AddTrueToObject(cj, "value");
        else
            cJSON_AddFalseToObject(cj, "value");
    }
    return cj;
}

/***********************************************
 *                 assetVar
 ***********************************************/
assetVar::assetVar()
{
    //depth = 0; 
    cval = 0;
    //aVals = nullptr;
    lVal = nullptr; //new assetVal();
    dbV = DEF_DEADBAND;
    
    setNaked = false;
    aVar = nullptr;
    am = nullptr;
    ai = nullptr;
    extras = nullptr;
    // these are used in  config functions (onSet)
    aa = nullptr;
    abf = nullptr;
    abNum = -1;
    IsDiff = false;
    valChanged = false;
    ui_type = 0;
    linkVar = nullptr;

    //extras compFunc = nullptr;
    
}

assetVar::assetVar(const char* _name, int val) :assetVar()
{
    name = _name;
    type = AINT;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
}

assetVar::assetVar(const char* _name, double val) :assetVar()
{
    if (_name) name = _name;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    //val = DEF_DEADBAND;
    //dbVal = new assetVal(val);
    type = AFLOAT;
}

assetVar::assetVar(const char* _name, const char* val) :assetVar()
{
    name = _name;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    type = ASTRING;
}

assetVar::assetVar(const char* _name, bool val) :assetVar()
{
    name = _name;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    type = ABOOL;
}

// allow certain cj objects from makeVar(name, cj)
// NOTE not quite working yet
assetVar::assetVar(const char* _name, cJSON* cjval) :assetVar()
{
    if (cJSON_IsNumber(cjval))
    {
        double vv = cjval->valuedouble;
        assetVar(_name, vv);
        //setVal(vv);
    }
    else if (cJSON_IsBool(cjval))
    {
        bool vv = cJSON_IsTrue(cjval);
        assetVar(_name, vv);
        //setVal(vv);
    }
    else if (cJSON_IsString(cjval))
    {
        const char* vv = cjval->valuestring;
        assetVar(_name, vv);
        //setVal(vv);
    }
    else
    {
        bool vv = false;
        assetVar(_name, vv);
        //setVal(vv);
    }
}


// used for jamming in our special objects
assetVar::assetVar(const char* _name, assetList* val) :assetVar()
{
    name = _name;
    bool tval = true;
    aVal = new assetVal(tval);
    lVal = new assetVal(tval);
    aVar = (assetVar*)&val;  // special case we can put anything here
    type = AVAR;
}


const char* assetVar::getName(void)
{
    return name.c_str();
}

double assetVar::getLastSetDiff(double tnow)
{
    if(linkVar)
    {
        return linkVar->getLastSetDiff(tnow);
    }

    return tnow - aVal->setTime;
}
double assetVar::getLastChgDiff(double tnow)
{
    if(linkVar)
    {
        return linkVar->getLastChgDiff(tnow);
    }

    return tnow - aVal->chgTime;
}

double assetVar::getSetTime(void)
{
    if(linkVar)
    {
        return linkVar->getSetTime();
    }
    return aVal->setTime;
}
double assetVar::getChgTime(void)
{
    if(linkVar)
    {
        return linkVar->getChgTime();
    }
    return aVal->chgTime;
}

double assetVar::getdVal()
{
    double d = 0.0;
    return getVal(d);
}

int assetVar::getiVal()
{
    int d = 0;
    return getVal(d);
}

bool assetVar::getbVal()
{
    bool d = false;
    return getVal(d);
}

char* assetVar::getcVal()
{
    char* d = nullptr;
    return getVal(d);
}

double assetVar::getdLVal()
{
    double d = 0.0;
    return getLVal(d);
}

int assetVar::getiLVal()
{
    int d = 0;
    return getLVal(d);
}

bool assetVar::getbLVal()
{
    bool d = false;
    return getLVal(d);
}

char* assetVar::getcLVal()
{
    char* d = nullptr;
    return getLVal(d);
}

template <class T>
T assetVar::getVal(T val)
{
    T nVal;
    if (!aVal)
    {
        FPS_ERROR_PRINT("%s   -- missing aVal !!!!\n", __func__);
        aVal = new assetVal(val);
    }
    if (!lVal)
    {
        FPS_ERROR_PRINT("%s   -- missing lVal !!!!\n", __func__);
        lVal = new assetVal(val);
    }

    if(linkVar)
    {
        nVal = linkVar->getVal(val);
    }
    else
    {
    // lock it here if we need to 
        nVal = aVal->getVal(val);
    }
    // FPS_ERROR_PRINT("%s --- last Val [%s] nVal [%s]\n", __func__, lVal?lVal->valuebool?"true":"false":"no LVal",nVal?"true":"false");
    return nVal;
}

template <class T>
T assetVar::addVal(T val)
{
    // lock it here
    T nval;
    if(linkVar)
    {
        val = linkVar->addVal(val);
    }
    else
    {
        if (aVal->IsNumeric())
        {
            nval = aVal->getVal(val);
            val = setVal(nval + val);
        }
    }
    return val;
}

template <class T>
T assetVar::subVal(T val) {
    // lock it here
    T nval = val;
    if(linkVar)
    {
        val = linkVar->subVal(val);
    }
    if (aVal->IsNumeric())
    {
        nval = aVal->getVal(nval);
        setVal(nval - val);
        val =  (nval - val);
    }
    return val;
}

template <class T>
T assetVar::getLVal(T val)
{
    // lock it here
    T nVal;
    if(linkVar)
    {
        nVal = linkVar->getLVal(val);
    }
    else
    {
    // lock it here if we need to 
        nVal = lVal->getVal(val);
    }

    //nval = lVal->getVal(val);
    //FPS_ERROR_PRINT("%s --- last Val [%s] nVal [%s]\n", __func__, lVal->valuebool?"true":"false",nval?"true":"false");
    return nVal;
}

template <class T>
bool assetVar::setVal(T val)
{
    if (0)FPS_ERROR_PRINT(" %s >> setVal  called here \n"
        , __func__
    );
    if(linkVar)
    {
        return linkVar->setVal(val);
    }

    bool diff = valueIsDiff(val);
    if (0)FPS_ERROR_PRINT(" %s >> setVal  called here diff %d\n"
        , __func__
        , diff
    );
    if (diff)
    {
        assetVal* tv = lVal;
        lVal = aVal;
        aVal = tv;
        aVal->setVal(val);
        valChanged = true;
        IsDiff = true;
        aVal->chgTime = aVal->setTime;
    }

    return diff;
}

template <class T>
void assetVar::setFimsVal(T val, fims* p_fims, const char* acomp)
{
    if(p_fims)
    {
        setVal(val);
        cJSON* cj = nullptr;
        getCjVal(&cj);
        char* res = cJSON_Print(cj);
        cJSON_Delete(cj);
            // default comp to the assetVar original comp
        if (acomp == nullptr)
            acomp = comp.c_str();
        if (acomp != nullptr && strlen(acomp) > 0)
            p_fims->Send("pub", acomp, nullptr, res);
        free((void*)res);
    }
    return;

}
// we dont test against the lval and val we simple see if we have had a setval since last reset
// this tests if we have had a value change since the last value reset.
// this is done at the assetVar level now
//template < class T>
bool assetVar::valueChanged(int reset)
{
    if(linkVar)
    {
        return linkVar->valueChanged(reset);
    }
    bool ret = valChanged;
    if (reset == 0)
        valChanged = false;
    return ret;
}

// bool assetVar::valueChanged()
// {
//     if(linkVar)
//     {
//         return linkVar->valueChanged();
//     }
//     bool ret = valChanged;
//     return ret;
//}

bool assetVar::valueChangedReset()
{
    if(linkVar)
    {
        return linkVar->valueChangedReset();
    }

    bool ret = valChanged;
    valChanged = false;
    return ret;
}


template < class T>
void assetVar::resetChanged(T val)
{
    if(linkVar)
    {
        return linkVar->resetChanged(val);
    }
    valChanged = false;
}


template <class T>
bool assetVar::valueIsDiff(T val)
{
    // this is OK it tests a new val against the old one
    //double dbV = DEF_DEADBAND;
    // if(gotParam("dbVal")
    // { 
    //     dbV = getdParam("dbVal");
    // }
    if(linkVar)
    {
        return linkVar->valueIsDiff(val);
    }

    return valueIsDiff(dbV, val);
}

bool assetVar::valueIsDiff(double db, const char* val)
{
    if(linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }

    if (!aVal->valuestring && !val) return false;
    if (aVal->valuestring && !val) return true;
    if (!aVal->valuestring && val) return true;
    if (0)FPS_ERROR_PRINT("assetVar.h %s >>  old [%s] aVal[%p] aValvs[%p]\n"
        , __func__, val, aVal, aVal->valuestring
        );
    if (0)FPS_ERROR_PRINT("assetVar.h %s >>  old [%s] new [%s] strcmp [%d] diff [%s]\n"
        , __func__, val, aVal->valuestring
        , strcmp(aVal->valuestring, val)
        , (bool)strcmp(aVal->valuestring, val)?"true":"false"

    );
    return (strcmp(aVal->valuestring, val));
    //return (!(strcmp(aVal->valuestring, val)==0));
}

bool assetVar::valueIsDiff(double db, bool val)
{
    if(linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }
    return(aVal->valuebool != val);
}

bool assetVar::valueIsDiff(double db, int val)
{
    if(linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }
    // auto diff = db ? db->valueint : 0;
    if (abs(aVal->valueint - val) > (int)db)
    {
        return true;
    }
    return false;
}

bool assetVar::valueIsDiff(double db, double val)
{
    if(linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }
    // auto diff = db ? db->valuedouble : DEF_DEADBAND;
    if(!aVal)
    {
        FPS_ERROR_PRINT("%s >> Hmm no Aval for [%s] maybe we need one \n",__func__, name.c_str());
        //return true;
    }
    if (std::abs(aVal->valuedouble - val) > db)
    {
        return true;
    }
    return false;
}

template <class T>
T assetVar::valueDiff(T val) {
    if(linkVar)
    {
        return linkVar->valueIsDiff(val);
    }
    T nval = val;
    if (!aVal->IsString())
        val = lVal->getVal(val) - aVal->getVal(val);

    return nval;
}

//Set a deadband for floats
void assetVar::setDbVal(double val) {
    if(linkVar)
    {
        return linkVar->setDbVal(val);
    }
    dbV = val;
}

assetVal* assetVar::makeCopy(assetVal* av)
{
    assetVal* ax = new assetVal();
    ax->type = av->type;
    ax->valuebool = av->valuebool;
    ax->valueint = av->valueint;
    ax->valuedouble = av->valuedouble;
    ax->setTime = av->setTime;
    ax->valuestring = av->valuestring ? strdup(av->valuestring) : nullptr;
    //ax->linkVar = linkVar;
    return ax;
}

template <class T>
void assetVar::setLVal(T val) {
    if(linkVar)
    {
        linkVar->setLVal(val);
    }

    lVal->setVal(val);
    IsDiff = true;
    // if (depth > 1)
    //     // TODO test this
    // {
    //     if (valueIsDiff(val))
    //     {
    //         aVals.push_back(makeCopy(aVal));
    //         while ((int)aVals.size() > depth)
    //         {
    //             assetVal* av = aVals.front();
    //             delete av;
    //             aVals.erase(aVals.begin());
    //         }

    //     }
    // }
}

bool assetVar::setCjVal(cJSON* cj)
{
    // these are all good for linkVars
    if (0)FPS_ERROR_PRINT(" %s >> setCjVal  called here \n"
        , __func__
    );
    if (cJSON_IsBool(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT("%s >>   body child is a cjson bool value [%s]\n", __func__, cJSON_IsTrue(cj) ? "true" : "false");
        return setVal((bool)cJSON_IsTrue(cj));
    }
    else if (cJSON_IsNumber(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT("%s >> body child is a cjson numerical value [%f]\n", __func__, cj->valuedouble);
        return setVal(cj->valuedouble);
    }
    else if (cJSON_IsString(cj))
    {
        if (setvar_debug_asset)FPS_ERROR_PRINT("%s >> body child is a cjson string value [%s]\n", __func__, cj->valuestring);
        return setVal(cj->valuestring);
    }
    else
    {
        FPS_ERROR_PRINT("%s >> body child [%s] cannot be simply decoded\n", __func__, cj->string);
    }
    return false;
}


// gets '{name:value}
void assetVar::getCjVal(cJSON** cj)
{
    if(linkVar)
    {
        linkVar->getCjVal(cj);
    }

    if (*cj == nullptr)
        *cj = cJSON_CreateObject();
    cJSON* cjv = *cj;

    switch (aVal->type) {
    case AINT:
        cJSON_AddItemToObject(cjv, name.c_str(), cJSON_CreateNumber(aVal->valueint));
        break;
    case AFLOAT:
        cJSON_AddItemToObject(cjv, name.c_str(), cJSON_CreateNumber(aVal->valuedouble));
        break;
    case ABOOL:
        if (aVal->valuebool)
            cJSON_AddTrueToObject(cjv, name.c_str());
        else
            cJSON_AddFalseToObject(cjv, name.c_str());
        break;
    case ASTRING:
        if (0)
        {
            FPS_ERROR_PRINT("%s >>  ASTRING name [%s] type %d atype %d valstr [%s]\n"
                , __func__
                , name.c_str()
                , type
                , aVal->type
                , aVal->valuestring
            );
        }
        cJSON_AddStringToObject(cjv, name.c_str(), aVal->valuestring ? aVal->valuestring : "No VAL");
        if (0)
        {
            FPS_ERROR_PRINT("%s >>  ASTRING name [%s] done \n", __func__, name.c_str());
        }

        break;
    default:
        if (1)
        {
            FPS_ERROR_PRINT("%s >>  %d <<<<<< DEFAULT name [%s] done \n", __func__, __LINE__, name.c_str());
        }
        break;
    }
}

//0x0000  full
//0x0001 naked
//0x0010 full but sep tables as per /assets/bms
//0x0011 naked but sep tables as per /assets/bms
//0x01xx full dump  /comp tables

//possibly use _format in the table to select ,naked, full, dump 
// options are 1/ show value (default)
//             2/ show naked value (0x0001)
//             3/ show baseparams and options (0x0100)
//             4/ show baseparams, options and actions (0x0010)
// split tables 5/ show baseparams, options and actions (0x1000)  // not in this part

// naked overrides


// this gets complex with linkVar

void assetVar::showvarCJ(cJSON* cj, int opts)
{
    if (0)FPS_ERROR_PRINT(" %s >>  >>>>>>>name [%s] av %p link %p type %d atype %d opts 0x%04x ui_type %d naked [%s]\n"
        , __func__
        , name.c_str()
        , this
        , linkVar
        , type
        , aVal->type
        , opts
        , ui_type
        , setNaked ? "true" : "false"
    );
    // char *tmp = cJSON_Print(cj);
    // if (tmp)
    // {
    //     if (1) FPS_ERROR_PRINT(" >>>>> %s at start -->%s<<--\n",__func__, tmp);
    //     free((void*)tmp);
    // }
    cJSON* cjv = cJSON_CreateObject();
    if (setNaked) opts |= 0x0001;
    if(linkVar)
    {
        // this breaks it
        showvarValueCJ(cjv, opts);
    }
    else
    {
        showvarValueCJ(cjv, opts);
    }
    // its broken before here
    if ((opts & 0x0110) == 0x0110)  // Hack
    {
        //showvarAlarmsCJ(cjv, opts);
        showvarExtrasCJ(cjv, opts);
    }
    else if (opts & 0x0010)  // Dump
    {
        showvarExtrasCJ(cjv, opts);
    }
    else if ((opts & 0x0100) || (ui_type == 2)) // assets
    {
        showvarAlarmsCJ(cjv, opts);
    }
    // else if (ui_type == 2)  // alarm
    // {
    //     showvarAlarmsCJ(cjv, opts);
    // }
    // TODO get showvarCJ to work properly for naked stuff
    if (opts & 0x0001)
    {
        cJSON* cjii = cJSON_Duplicate(cjv->child, true);
        cJSON_AddItemToObject(cj, name.c_str(), cjii);
        cJSON_Delete(cjv);
    }
    else
    {
        cJSON_AddItemToObject(cj, name.c_str(), cjv);
    }

    return;
}

cJSON* assetVar::getAction(assetAction* aact)
{
    if (debug_action)FPS_ERROR_PRINT(" %s >> >>>>> get action start, name >>%s<<--\n", __func__, aact->name.c_str());

    cJSON* cj = cJSON_CreateObject();
    cJSON* cja = cJSON_CreateArray();
    for (auto& x : aact->Abitmap)
    {
        cJSON* cjix = cJSON_CreateObject();
        if (debug_action)FPS_ERROR_PRINT("%s >>  >>>>> get action bitfield start abf [%d]<<--\n", __func__, x.first);

        assetBitField* abf = x.second;
        char* uri = abf->getFeat("uri", &uri);
        char* var = abf->getFeat("var", &var);


        if (debug_action)FPS_ERROR_PRINT(" %s  >>>>> get action bitfield uri %s var %s<<--\n"
            , __func__
            , uri
            , var
        );
        //int fm = 0;
        // use featmap
        abf->featDict->showCj(cjix);
        // void showCj(cJSON* cjix)
        // {
        // for ( auto x : abf->featDict->featMap)
        // {
        //     //cJSON_AddNumberToObject(cjix,"fmIndex", fm++); 

        //     assFeat* af = x.second;
        //     switch (af->type) {
        //        case assFeat::AFLOAT:
        //             cJSON_AddNumberToObject(cjix, af->name.c_str(), af->valuedouble); 
        //             break;
        //        case assFeat::AINT:
        //             cJSON_AddNumberToObject(cjix,  af->name.c_str(), af->valueint); 
        //             break;
        //        case assFeat::ASTRING:
        //             cJSON_AddStringToObject(cjix,  af->name.c_str(), af->valuestring); 
        //             break;
        //         case assFeat::ABOOL:
        //            if(af->valuebool)
        //                 cJSON_AddTrueToObject(cjix, af->name.c_str()); 
        //             else
        //                 cJSON_AddFalseToObject(cjix, af->name.c_str()); 
        //             break;
        //        default:
        //             cJSON_AddStringToObject(cjix,   af->name.c_str(), "TypeNotDefined"); 
        //             break;
        //     }
        // }
        // }

        cJSON_AddItemToArray(cja, cjix);
        if (debug_action)FPS_ERROR_PRINT(" <<<<<<< get action var[%s]  bitfield done <<--\n", abf->var);

    }
    cJSON_AddItemToObject(cj, aact->name.c_str(), cja);

    if (0)FPS_ERROR_PRINT(" <<<<<<< get action done <<--\n");
    return cj;

}

assetVar::~assetVar() {
    delete aVal;
    if (lVal) delete lVal;
    if(extras)
        delete extras;
    //if (dbVal) delete dbVal;

    // for (auto x : actMap)
    // {
    //     delete x.second;
    // }
    //actMap.clear();
    // for (auto x : actVec)
    // {
    //     for (auto y : x.second)
    //     {
    //         delete y;
    //     }
    //     x.second.clear();
    // }

    // actVec.clear();
    //extras if (featDict) delete featDict;
    //if (optDict) delete optDict;
    // we also have to keep the order of the list intact. use a control vector as a base assetvar.. Uck but needs must.
    // we may need to load up a derived class called a ui class. to do this...
    //if (baseDict) delete baseDict;
    //if (optVec) delete optVec;
    // we need  named map of arrays for things like ui options
    // actually the list below will also work for keeping the order list.
    //std::map<std::string,std::vector<assetVar *>> optVec;
}


// still need to split this up
void assetVar::showvarExtrasCJ(cJSON* cjv, int opts)
{
    if(0)if (1||debug_action) FPS_ERROR_PRINT("%s >>  >>>>> looking for extras %p av %p [%s] opts 0x%04x link %p\n"
            , __func__, extras, this, name.c_str(), opts, linkVar);

    // char* tmp = cJSON_Print(cjv);
    // if (tmp)
    // {
    //     if (1) FPS_ERROR_PRINT(" >>>>> %s started with  -->%s<<--\n",__func__, tmp);
    //     free((void*)tmp);
    // }
    // if (!actMap.empty())
    // {
    //     cJSON* cjact = cJSON_CreateObject();
    //     for (auto& x : actMap)
    //     {

    //         cJSON* cja = getAction(x.second);
    //         if (debug_action) FPS_ERROR_PRINT("%s >>  <<<<< action got cj  -->%s<--  cja %p\n", __func__, x.first.c_str(), (void*)cja);

    //         if (cja)
    //         {
    //             if (debug_action) FPS_ERROR_PRINT(" >>>>> OK  action found\n");

    //             char* tmp = cJSON_PrintUnformatted(cja);
    //             if (tmp)
    //             {
    //                 if (0) FPS_ERROR_PRINT(" >>>>> show action found -->%s<<--\n", tmp);
    //                 free((void*)tmp);
    //             }
    //             cJSON_AddItemToObject(cjact, x.first.c_str(), cja);
    //         }
    //         else
    //         {
    //             FPS_ERROR_PRINT(" >>>>> HMMM no action found\n");
    //         }
    //     }
    //     cJSON_AddItemToObject(cjv, "actions", cjact);
    // }
    if(extras)
    {
        if (!extras->actVec.empty())
        {
            cJSON* cjact = cJSON_CreateObject();
            for (auto& x : extras->actVec)
            {
                cJSON* cjactar = cJSON_CreateArray();

                for (auto& y : x.second)
                {

                    cJSON* cja = getAction(y);
                    if (0||debug_action) FPS_ERROR_PRINT("%s >>  <<<<< action Vec got cj  -->%s<--  cja %p\n", __func__, x.first.c_str(), (void*)cja);

                    if (cja)
                    {
                        if (debug_action) FPS_ERROR_PRINT(" >>>>> OK  action found\n");

                        char* tmp = cJSON_PrintUnformatted(cja);
                        if (tmp)
                        {
                            if (0) FPS_ERROR_PRINT(" >>>>> show action found -->%s<<--\n", tmp);
                            free((void*)tmp);
                        }
                        cJSON_AddItemToArray(cjactar, cja);
                    }
                    else
                    {
                        FPS_ERROR_PRINT(" >>>>> HMMM no action found\n");
                    }
                }
                cJSON_AddItemToObject(cjact, x.first.c_str(), cjactar);

            }
            cJSON_AddItemToObject(cjv, "actions", cjact);
        }
        if (extras->featDict)
        {
            cJSON* cjfeat = cJSON_CreateObject();
            extras->featDict->showCj(cjfeat);
            cJSON_AddItemToObject(cjv, "params", cjfeat);

        }
    
        if (extras->optDict)
        {
            cJSON* cjfeat = cJSON_CreateObject();
            extras->optDict->showCj(cjfeat);
            cJSON_AddItemToObject(cjv, "options", cjfeat);

        }
        if(extras->useAlarms)
        {
            cJSON* cja = cJSON_CreateArray();
            cJSON_AddItemToObject(cjv, "options", cja);

            auto asize = extras->alarmVec.size();
        
            if (asize > 0)
            {
                for (auto aa : extras->alarmVec)
                {
                    if (aa)
                    {
                        cJSON* cjai = cJSON_CreateObject();
                        cJSON_AddStringToObject(cjai, "name", aa->almsg.c_str());
                        cJSON_AddNumberToObject(cjai, "return_value", aa->severity); ;
                        cJSON_AddItemToArray(cja, cjai);
                    }
                }
            }
        }        
    }
    // tmp = cJSON_Print(cjv);
    // if (tmp)
    // {
    //     if (1) FPS_ERROR_PRINT(" >>>>> %s found -->%s<<--\n",__func__, tmp);
    //     free((void*)tmp);
    // }
// if (baseDict)
    // {
    //     // This should take care of adding all the name:values in base dict to the existing cjson object
    //     baseDict->showCj(cjv);
    // }
}
// still need to split this up
void assetVar::showvarAlarmsCJ(cJSON* cjv, int opts)
{
    // this becomes a class
    if(extras)
    {
        if (extras->alarmVec.size() == 0)
        {
            if (extras->optVec)
            {
                extras->optVec->showCj(cjv);
            }
        }
        if (extras->featDict)
        {
            cJSON* cjfeat = cJSON_CreateObject();
            extras->featDict->showCj(cjfeat);
            cJSON_AddItemToObject(cjv, "params", cjfeat);

        }
    }

    // "bms_1":        {
    //         "alarms_1":     {
    //                 "value":        1,
    //                 "options":      [{
    //                                 "name": "HVAC Alarm - NO",
    //                                 "return_value": 1
    //                         }],
    //                 "enabled":      true,
    //                 "name": "Alarm Group_1",
    //                 "scaler":       0,
    //                 "type": "number",
    //                 "ui_type":      "alarm",
    //                 "unit": ""
    //         }

    //}
    if(extras)
    {
        int asize = (int)extras->alarmVec.size();
        if (asize > 0)
        {
            cJSON* cja = cJSON_CreateArray();

            for (auto aa : extras->alarmVec)
            {
                if (aa)
                {
                    cJSON* cjai = cJSON_CreateObject();
                    cJSON_AddStringToObject(cjai, "name", aa->almsg.c_str());
                    cJSON_AddNumberToObject(cjai, "return_value", aa->severity); ;
                    cJSON_AddItemToArray(cja, cjai);
                }
            }
            cJSON_AddItemToObject(cjv, "options", cja);
        }
        if (extras->baseDict)
        {
            // This should take care of adding all the name:values in base dict to the existing cjson object
            extras->baseDict->showCj(cjv);
        }

    }
    // else
    // {
    //     // this becomes a class
    //     if (optVec)
    //     {
    //         optVec->showCj(cjv);
    //     }

    // }

    // if (optDict)
    // {
    //     cJSON* cjfeat = cJSON_CreateObject();
    //     optDict->showCj(cjfeat);
    //     cJSON_AddItemToObject(cjv, "options", cjfeat);

    // }
    // this becomes a class
    // if (optVec)
    // {
    //     optVec->showCj(cjv);
    // }


}
// runs on alarm dest
void assetVar::setAlarm(asset_log* avAlarm)
{
    if(!extras)
    {
        extras = new assetExtras;
    }
    if(0) FPS_ERROR_PRINT("%s >> setting alarm in [%s:%s] msg [%s]\n"
                , __func__
                , comp.c_str()
                , name.c_str()
                , avAlarm->almsg.c_str()
                );
    // TODO search for alarm already posted
    //avAlarm->destIdx = (int)alarmVec.size();
    extras->alarmVec.push_back(avAlarm);
    extras->useAlarms = true;
    setVal(1);
    return;
}

asset_log* assetVar::getAlarm(assetVar* srcAv, const char* atype, int num)
{
    if(extras)
    {
        std::string stype = atype;
        for (auto al : extras->alarmVec)
        {
            if (al)
            {
                if ((al->srcAv == srcAv) && (al->altype == stype))
                {
                    if (--num == 0)
                    {
                        return al;
                    }
                }
            }

        }
    }
    return nullptr;
}

int assetVar::getNumAlarm(assetVar* srcAv, const char* atype)
{
    int num = 0;
    if(extras)
    {

        std::string stype = atype;
        for (auto al : extras->alarmVec)
        {
            if (al)
            {
                if ((al->srcAv == srcAv) && (al->altype == stype))
                {
                    num++;
                }
            }
        }
    }
    return num;
}

// this comes from the srcVar
// modify this to look for the alarm on the dest
int assetVar::clearAlarm(asset_log* avAlarm)
{
    if(extras)
    {
    if ((avAlarm->destIdx >= 0) && (avAlarm->destIdx < (int)extras->alarmVec.size()))
        {
            if (extras->alarmVec[avAlarm->destIdx] == avAlarm)
            {
                extras->alarmVec[avAlarm->destIdx] = nullptr;
                delete avAlarm;
            }
        }
        else
        {
            FPS_ERROR_PRINT("%s >> bad alarm index %d \n", __func__, avAlarm->destIdx);
        }
    }

    return 0;
}

int assetVar::clearAlarm(assetVar* destAv, const char* atype)
{
    //asset_log* avAlarm = alarmMaps[atype];
    asset_log* avAlarm = destAv->getAlarm(this, atype);
    if (!avAlarm)
    {
        return -1;
    }
    return destAv->clearAlarm(avAlarm);
}

// runs on dest
int assetVar::clearAlarms()
{
    if(extras)
    {
        extras->useAlarms = true;
    // notify the srcAv that its disconnected from the dest.
        for (auto x : extras->alarmVec)
        {
            if (x)
            {
                x->destAv = nullptr;
            }
        }
        extras->alarmVec.clear();
    }
    setVal(0);
    return 1;
}
// opts 0 :normal 1: naked 2:value only 
void assetVar::showvarValueCJ(cJSON* cj, int opts)
{
    if(linkVar)
    {
        linkVar->showvarValueOnlyCJ(cj, opts);
    }
    else
    {
        showvarValueOnlyCJ(cj, opts);
    }

    if (opts & 0x0010)  // extras
    {
        if(0)FPS_ERROR_PRINT(" %s >>  >>>>>>>opts 0x%04x name [%s] type %d atype %d\n"
            , __func__
            , opts
            , name.c_str()
            , type
            , aVal->type
            );
            // just add the basedict for now
        if(extras)
        {
            if (extras->baseDict)
            {
                // This should take care of adding all the name:values in base dict to the existing cjson object
                extras->baseDict->showCj(cj);
            }
        }
    }
}

// opts 0 :normal 1: naked 2:value only 
void assetVar::showvarValueOnlyCJ(cJSON* cj, int opts)
{
    if (0)
    {
        FPS_ERROR_PRINT(" %s >>  >>>>>>>name [%s] comp [%s] type %d aval %p opts %d\n"
            , __func__
            , name.c_str()
            , comp.c_str()
            , type
            , aVal
            , opts
        );
        FPS_ERROR_PRINT(" %s >>  >>>>>>>name [%s] type %d atype %d\n"
            , __func__
            , name.c_str()
            , type
            , aVal->type
        );
    }
    

    //if ((opts == 0)||(opts==3))
    if (opts & 0x0001)  // naked
    {
        getCjVal(&cj);
    }
    else
    {
        cJSON* cjv = cj;//cJSON_CreateObject();

        switch (aVal->type) {
        case AINT:
            cJSON_AddItemToObject(cjv, "value", cJSON_CreateNumber(aVal->valueint));
            break;
        case AFLOAT:
            cJSON_AddItemToObject(cjv, "value", cJSON_CreateNumber(aVal->valuedouble));
            break;
        case ABOOL:
            if (aVal->valuebool)
                cJSON_AddTrueToObject(cjv, "value");
            else
                cJSON_AddFalseToObject(cjv, "value");
            break;
        case ASTRING:
            if (0)
            {
                FPS_ERROR_PRINT("%s >>  ASTRING name [%s] type %d atype %d valstr [%s]\n"
                    , __func__
                    , name.c_str()
                    , type
                    , aVal->type
                    , aVal->valuestring
                );
            }
            cJSON_AddStringToObject(cjv, "value", aVal->valuestring ? aVal->valuestring : "No VAL");
            if (0)
            {
                FPS_ERROR_PRINT("%s >>  ASTRING name [%s] done \n", __func__, name.c_str());
            }
            break;
        default:
            if (1)
            {
                FPS_ERROR_PRINT("%s >>  %d <<<<<< DEFAULT name [%s] done \n", __func__, __LINE__, name.c_str());
            }
            break;
        }
    }
}

void assetVar::setParam(const char* pname, bool val)
{
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(!extras)
    {
        extras=new assetExtras;
    }
    if(!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParam(const char* pname, int val)
{
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(!extras)
    {
        extras=new assetExtras;
    }
    if(!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParam(const char* pname, double val)
{
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(!extras)
    {
        extras=new assetExtras;
    }
    if(!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParam(const char* pname, char* val)
{
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(!extras)
    {
        extras=new assetExtras;
    }
    if(!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

// template <class T>
// void assetVar::setParam(const char* pname, T val)
// {
//     if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
//         , __func__
//         , pname
//         );
//     if(!extras)
//     {
//         extras=new assetExtras;
//     }
//     if(!extras->baseDict)
//         extras->baseDict = new assetFeatDict;
//     extras->baseDict->setFeat(pname, val);
// }

int assetVar::getiParam(const char* pname)
{
    int val;
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );

    if(extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}

double assetVar::getdParam(const char* pname)
{
    double val;
    if (0)FPS_ERROR_PRINT(" %s >>   called here av name [%s] param [%s] extras %p \n"
        , __func__
        , name.c_str()
        , pname
        , extras
        );
    if(extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}
bool assetVar::getbParam(const char* pname)
{
    bool val;
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}
char* assetVar::getcParam(const char* pname)
{
    char* val;
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}

char* assetVar::getcAParam(const char* pname)
{
    char* val=nullptr;
    assetVar*av;
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(extras && extras->baseDict)
    {
        extras->baseDict->getFeat(pname, &av);
        if(av)
        {
            val = av->getcVal();
        }
    }
    return val;
}

assetVar* assetVar::getaParam(const char* pname)
{
    assetVar*av=nullptr;
    if (0)FPS_ERROR_PRINT(" %s >>   called here name [%s] \n"
        , __func__
        , pname
        );
    if(extras && extras->baseDict)
    {
        extras->baseDict->getFeat(pname, &av);
    }
    return av;
}

void assetVar::PubFunc(assetVar *av)
{
    if(!extras)
    {
        extras=new assetExtras;
    }
    extras->PubFunc = av;
}

void assetVar::SetPubFunc(assetVar *av)
{
    if(!extras)
    {
        extras=new assetExtras;
    }
    extras->SetPubFunc = av;
}

void assetVar::SetFunc(assetVar *av)
{
    if(!extras)
    {
        extras = new assetExtras;
    }
    extras->SetFunc = av;
}
// TODO fix this
// int assetVar::sendLog(const char *msg,...)
// {
//     int rc = 0;
//     if(extras)
//     {
//         // skip if not enabled
//         if (!getbVal())
//         {
//             return rc;
//         }

//         char* buffer=nullptr;
//         va_list args;
//         va_start (args, msg);
//         rc = vsnprintf (buffer, 0, msg, args);
//         va_end (args);   
//         if (rc > 0)
//         { 
//             buffer = (char *)calloc(rc,1); 
//             va_start (args, msg);
//             vsnprintf (buffer, rc, msg, args);
//             va_end (args);
//             if(extras->logfd>=0)
//             {
//                 write(extras->logfd, buffer, rc);
//             }
//             else
//             {
//                 FPS_ERROR_PRINT("%s>> log[%s]\n",__func__, buffer);
//             }
//             free(buffer);
//         }
//     }
//     return rc;
// }

int assetVar::sendLog(assetVar*av, const char *msg,...)
{
    int rc = 0;
    if(extras)
    {
        // skip if not enabled
        if (!getbVal())
        {
            return rc;
        }

        char* buffer=nullptr;
        va_list args;
        va_start (args, msg);
        rc = vsnprintf (buffer, 0, msg, args);
        va_end (args);   
        if (rc > 0)
        { 
            rc++;
            buffer = (char *)calloc(rc,1); 
            va_start (args, msg);
            vsnprintf (buffer, rc, msg, args);
            va_end (args);
            if(extras->logAlways || (extras->logfd< 0) )
            {
                FPS_ERROR_PRINT("%s\n", buffer);
            }
            if(extras->logfd >= 0)
            {
                write(extras->logfd, buffer, rc);
            }
            free(buffer);
        }
    }
    return rc;
}

int assetVar::flushLog(void)
{
    if(extras && (extras->logfd>0))
    {
        syncfs(extras->logfd);
    }
    return 0;
}

int assetVar::logAlways(bool flag)
{
    if(extras)
    {
        extras->logAlways = flag;
    }
    return 0;
}

int assetVar::openLog(const char* fname, int logDepth)
{
    char *sp = nullptr;
    char *tmpsp = nullptr;
    // TODO rotate old logs

    if(!extras)
    {
        extras = new assetExtras;
    }
    if(extras->logfd>=0)
    {
        close(extras->logfd);
    }
    if(extras->logFile)
    {
        free(extras->logFile);        
    }

    extras->logFile=strdup(fname);
    extras->logDepth = logDepth;
    if(fname)
    {
        // other options like db , ip may follow
        sp = (char*)fname;
        if(strncmp(fname,"file:", strlen("file:")) == 0)
        {
            sp += strlen("file:");
        }
        if (*sp != '/')
        {
            // in asset.h
            asprintf(&tmpsp,"%s/%s", DEFAULT_LOG_DIR, sp);
            sp = tmpsp;
        }

    }
    // TODO rotate old logs


    if(sp)extras->logfd  = open(sp, O_RDWR | O_CREAT | O_APPEND, 0644);  
    if(tmpsp)free(tmpsp);
    return 0;
}

int assetVar::closePerf(const char* fname)
{
    if(extras && (extras->logfd>0))
    {
        close(extras->logfd); 
        extras->logfd = -1;
    }
    return 0;
}


/***********************************************
 *                 VarsMap
 ***********************************************/
// VarsMap is this used ??
VarsMap::VarsMap() {
    pthread_mutex_init(&map_lock, nullptr);
}

VarsMap::~VarsMap()
{
    // find each comp
    nvarsmap::iterator x;
    std::map<std::string, assetVar*> vm;

    for (x = vmap.begin(); x != vmap.end();++x)
    {
        // for each var
        varmap* vm = vmap[x->first];

        for (auto y = vm->begin(); y != vm->end(); ++y)
        {
            free((void*)y->second);
        }
        vm->clear();
    }
    vmap.clear();
}

// move to the rest ofthem
/***********************************************
 *                 assetExtras
 ***********************************************/
assetExtras::assetExtras()
{
    depth = 0; cval = 0;
    RunFunc = nullptr;
    SetFunc = nullptr;
    SetPubFunc = nullptr;
    GetFunc = nullptr;
    PubFunc = nullptr;
    featDict = nullptr;
    baseDict = nullptr;
    optDict = nullptr;
    optVec = nullptr;
    // these are used in  config functions (onSet)
    aa = nullptr;
    abf = nullptr;
    abNum = -1;
    IsDiff = false;
    valChanged = false;
    ui_type = 0;
    compFunc = nullptr;
    useAlarms = false;

    logFile = nullptr;
    logDepth = 0;
    logfd = -1;
    logAlways = false;

}


assetExtras::~assetExtras() {
    for (auto x : actVec)
    {
        for (auto y : x.second)
        {
            delete y;
        }
        x.second.clear();
    }
    actVec.clear();

    for (auto x : logVec)
    {
        delete x;
    }

    logVec.clear();
    if(logFile)free(logFile);

    if(logfd >= 0)
    {
        close(logfd);
    }

    if (featDict) delete featDict;
    if (optDict) delete optDict;
    // we also have to keep the order of the list intact. use a control vector as a base assetvar.. Uck but needs must.
    // we may need to load up a derived class called a ui class. to do this...
    if (baseDict) delete baseDict;
    if (optVec) delete optVec;
    // we need  named map of arrays for things like ui options
    // actually the list below will also work for keeping the order list.
    //std::map<std::string,std::vector<assetVar *>> optVec;
}

/**************************************************************************************************************************************************************/

/******************************************************
 *              
 *                 asset.h
 *    
 ******************************************************/

/***********************************************
 *                 assetUri
 ***********************************************/
assetUri::assetUri(const char* uri, const char* var)
{
    Uri = strdup(uri);
    origuri = strdup(uri);
    origvar = nullptr;
    if(var)origvar = strdup(var);
    nfrags = getNfrags();
    Var = nullptr;
    Param = nullptr;
    setup();
}

assetUri::~assetUri()
{
    free((void*)Uri);
    free((void*)origuri);
    if(origvar)free((void*)origvar);
    uriVec.clear();
}

void assetUri::setup(void)
{
    char *sp = strstr(Uri,(char*)":");
    if(sp)
    {
        *sp++ = 0;
        Var = sp;
    }
    else
    {
        Var = origvar;
    }
    if(Var)
    {
        sp = strstr(Var,(char *)"@");
        if(sp)
        {
            *sp++ = 0;
            Param = sp;
        }
    }
    else
    {
        sp = strstr(Uri,(char *)"@");
        if(sp)
        {
            *sp++ = 0;
            Param = sp;
        }

    }  
}

int assetUri::setupUriVec()
{
    if(uriVec.size() > 0)
        return (int)uriVec.size();
    char *sp = Uri;
    char *sp1;
    sp++;
    do
    {
        sp1 = strstr(sp,(char*)"/");
        if(sp1)
        {
            *sp1++ = 0;
            uriVec.push_back(sp);
            if(0)FPS_ERROR_PRINT("%s >> push back [%s]\n",__func__, sp);
            sp = sp1;
        }
    } while(sp1);
    if(sp!=Uri)
    {
        uriVec.push_back(sp);
        if(0)FPS_ERROR_PRINT("%s >> push back last [%s]\n",__func__, sp);

    }
    return (int)uriVec.size();
}
// was pull_one_uri
char* assetUri::pullOneUri(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    return uriVec[idx];
}

//char* pull_pfrag(const char* uri, int idx);
char* assetUri::pullPfrag(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    if (idx == 0) return strdup(Uri);
    std::string res= "";
    int i = 0;
    while(i<idx && i<ix)
    {
        res += "/";
        res +=uriVec[i++];
    }

    char* sp = (char*)strdup(res.c_str());
    return sp;
}

char* assetUri::pullPvar(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    if (idx == 0) return nullptr;

    std::string res= "";
    while(idx<ix)
    {
        res += "/";
        res +=uriVec[idx];
        idx++;
    }
    char* spr = (char*)res.c_str();

    char* sp = (char*)strdup(spr+1);
    return sp;
}

//char* pull_first_uri(const char* uri, int n = 1)
char* assetUri::pullFirstUri(int n)
{
    return pullPfrag(n);
}

//char* pull_last_uri(const char* uri, int n = 1)
char* assetUri::pullLastUri(int n)
{
    int ix = setupUriVec();
    if((ix - n) >= 0)
        return (char *)strdup(uriVec[ix-n]);
    return nullptr;
}

//char* pull_uri(const char* uri, int idx)
char* assetUri::pullUri(int idx)
{
    // needs to be freed
    return pullPfrag(idx);
}

int assetUri::getNfrags()
{
    int nfrags = 0;
    char* sp = origuri;
    while (*sp)
    {
        if (*sp++ == '/') nfrags++;
    }
    return nfrags;
}

/***********************************************
 *                 asset
 ***********************************************/
asset::asset() :asset("asset") {
}

asset::asset(const char* _name) {
    name = _name;
    run_init = nullptr;
    run_wakeup = nullptr;
    vm = &defvm;
    p_fims = nullptr;
    am = nullptr;
    vecs = nullptr;
}

asset::~asset()
{
    std::cout << "asset_delete :" << name << "\n";
}

void asset::setAm(asset_manager* _am) {
    am = _am;
}

void asset::setName(const char* _name) {
    name = _name;
}

void asset::cfgwrite(const char* fname, const char* aname)
{
    //VarMapUtils vm;

    cJSON* cj = getConfig();
    vm->write_cjson(fname, cj);
    cJSON_Delete(cj);
}

// to be developed .. the comman will be a fims message to be sent to all assets by the asset manager
const char* asset::get_command(const char* dest, const char* cmd)
{
    std::string rstr = "{\"asset_name\":";
    std::cout << "asset_dest :" << dest << " got  command:\n" << cmd << "\n";
    rstr.append(name);
    rstr.append(",\"cmd\":\"");
    rstr.append(cmd);
    rstr.append("\"}");
    return strdup(rstr.c_str());
}

// configure the asset
// use a list of reps to modify the file
void asset::configure(const char* fname, std::vector<std::pair<std::string, std::string>>* reps, asset_manager* am, asset* ai)
{
    //VarMapUtils vm;
    cJSON* cjbase = vm->get_cjson(fname, reps);
    cJSON* cj = nullptr;
    if (cjbase) cj = cjbase->child;
    //const char* vname;
    while (cj)
    {
        // uri - cj->string
        // uri->body - cj->child
        //FPS_ERROR_PRINT(" cj->string [%s] child [%p]\n", cj->string, (void *) cj->child);
        char* body = cJSON_Print(cj);
        FPS_ERROR_PRINT(" %s >> cj->string [%s] child [%p] body \n[%s]\n"
            , __func__, cj->string, (void*)cj->child, body);

        char* buf = vm->fimsToBuffer("set", cj->string, nullptr, body);
        free((void*)body);
        fims_message* msg = vm->bufferToFims(buf);
        free((void*)buf);
        cJSON* cjb = nullptr;
        vm->processFims(*vmap, msg, &cjb, am, ai);
        vm->free_fims_message(msg);

        buf = cJSON_Print(cjb);
        if (cjb) cJSON_Delete(cjb);

        FPS_ERROR_PRINT("%s >>  configured [%s]\n", __func__, buf);
        free((void*)buf);

        cj = cj->next;
    }
    if (cjbase) cJSON_Delete(cjbase);
}

cJSON* asset::getConfig(const char* uri, const char* var)
{
    //VarMapUtils vm;
    if (vmap)
        return  vm->getMapsCj(*vmap, uri, var);
    return nullptr;
}

bool asset::free_message(fims_message* message)
{
    // TODO manage memory better
    if (message == nullptr)
        return false;
    delete(message);
    return true;
}

void asset::cleanup(void)
{
    for (auto& x : amap)
    {
        delete x.second;
    }
    amap.clear();
}

varmap* asset::getBmap()
{
    return &amap;
}

void asset::setVmap(varsmap* _vmap)
{
    vmap = _vmap;
}
void asset::setPmap(varsmap* _vmap)
{
    pmap = _vmap;
}

void asset::setVm(VarMapUtils* _vm)
{
    vm = _vm;
}

int asset::Send(const char* method, const char*uri, const char*rep, const char* body)
{
    //p_fims->Send("pub", comp, nullptr, dvar);
    int ival = p_fims->Send(method, uri, rep, body);
    return ival;
}
//asset_manager

/***********************************************
 *                 asset_manager
 ***********************************************/
asset_manager::asset_manager() {
    run_init = nullptr;
    run_wakeup = nullptr;
    reload = 2; // force an init.
    vm = &defvm;
    vecs = nullptr;
}

asset_manager::asset_manager(const char* _name) : asset_manager() {
    // run_init = nullptr;
    // run_wakeup = nullptr;
    setName(_name);

}

asset_manager::~asset_manager()
{
    FPS_ERROR_PRINT(" asset manager running cleanup\n");
    cleanup();
}

void asset_manager::setVmap(varsmap* _vmap)
{
    if (0)FPS_ERROR_PRINT("%s >> asset manager setting vmap\n", __func__);
    vmap = _vmap;
}
void asset_manager::setPmap(varsmap* _vmap)
{
    pmap = _vmap;
}

varsmap* asset_manager::getVmap()
{
    if (0)FPS_ERROR_PRINT("%s >> asset manager getting vmap\n", __func__);
    return vmap;
}

void asset_manager::setAm(asset_manager* _am) 
{
    am = _am;
}

void asset_manager::setName(const char* _name) 
{
    name = _name;
}
//  "/assets/bms":        {
//                 "bms_1":   { 
//                                    "template":"bms_catl_template.json".
//                                    "subs":[
//                                       {"replace":"@@BMS_ID@@","with":"bms_1"},
//                                       {"replace":"@@BMS_IP@@","with":"192.168.1.114"}
//                                       ]
//                                   }
//            }
void asset_manager::debugConfig(asset* pc, const char* dmsg)
{
    // this a test for our config.
    cJSON* cjbm = pc->getConfig();
    char* res = cJSON_Print(cjbm);
    FPS_ERROR_PRINT("%s%s\n <<< done\n", dmsg, res);
    free((void*)res);
    cJSON_Delete(cjbm);
}

// TODO this looks very common
void asset_manager::assconfigure(varsmap* vmap, const char* fname, const char* aname)
{
    //VarMapUtils vm; // use the asset_manager one

    //vm->getReps(const char *fname, const char *aname,)
    cJSON* cjbase = vm->get_cjson(fname, nullptr);

    char* assname = nullptr;
    asprintf(&assname, "/assets/%s", aname);

    //cJSON* cj = cjbase->child;
    cJSON* cja = cJSON_GetObjectItem(cjbase, assname);
    cja = cja->child;
    free((void*)assname);

    while (cja)
    {
        cJSON* cjsi;
        cJSON* cjt = cJSON_GetObjectItem(cja, "template");
        if (0)FPS_ERROR_PRINT(" %s >> found asset [%s]\n", __func__, cja->string);
        if (0)FPS_ERROR_PRINT(" %s >> found asset [%s] template [%s]\n", __func__, cja->string, cjt->valuestring ? cjt->valuestring : "No template");

        cJSON* cjs = cJSON_GetObjectItem(cja, "subs");
        if (0)FPS_ERROR_PRINT(" %s   >> found asset [%s] subs %p isArray %s\n"
            , __func__, cja->string, cjs
            , cJSON_IsArray(cjs) ? "true" : "false"
        );
        if (cJSON_IsArray(cjs))
        {
            std::vector<std::pair<std::string, std::string>> reps;

            cJSON_ArrayForEach(cjsi, cjs)
            {
                cJSON* cjsr = cJSON_GetObjectItem(cjsi, "replace");
                cJSON* cjsw = cJSON_GetObjectItem(cjsi, "with");

                if (cjsr && cjsw && cjsr->valuestring && cjsw->valuestring)
                {
                    if (1)FPS_ERROR_PRINT(" %s   >> found asset [%s] sub from  [%s] to [%s]\n", __func__, cja->string
                        , cjsr->valuestring
                        , cjsw->valuestring);
                    reps.push_back(std::make_pair(cjsr->valuestring, cjsw->valuestring));
                }
            }
            asset* ass = addInstance(cja->string);
            ass->setVmap(vmap);
            ass->setVm(vm);
            ass->p_fims = p_fims;

            ass->configure(cjt->valuestring, &reps);
        }
        cja = cja->next;
    }

    //assname = nullptr;
    asprintf(&assname, "/links/%s", aname);
    cja = cJSON_GetObjectItem(cjbase, assname);
    if (1)FPS_ERROR_PRINT(" %s >> seeking links  [%s] in file [%s] cja %p\n", __func__, assname, fname, (void*) cja);
    if(cja)
    {
        if (1)FPS_ERROR_PRINT(" %s >> found links  [%s]\n", __func__, cja->string);
        amConfig(vmap, cja, this);
    }
    // configure_vmap

}


asset* asset_manager::addAsset(cJSON* cja, cJSON* cjt,
    std::vector<std::pair<std::string, std::string>>& reps, asset_manager* am)
{
    asset* ass = addInstance(cja->string);
    ass->setVmap(vmap);
    ass->vecs = vecs;
    ass->setVm(vm);
    ass->configure(cjt->valuestring, &reps, am, ass);
    return ass;

}
// TODO tidy this up
int asset_manager::amConfig(varsmap* vmap, cJSON* cj, asset_manager* am)
{
    char* body = cJSON_Print(cj);
    if (1)FPS_ERROR_PRINT(" %s >> cj->string [%s] child [%p] body \n[%s]\n"
        , __func__, cj->string, (void*)cj->child, body);

    char* buf = vm->fimsToBuffer("set", cj->string, nullptr, body);
    free((void*)body);
    fims_message* msg = vm->bufferToFims(buf);
    free((void*)buf);
    cJSON* cjb = nullptr;
    vm->processFims(*vmap, msg, &cjb, am, nullptr);
    vm->free_fims_message(msg);

    buf = cJSON_Print(cjb);
    if (cjb) cJSON_Delete(cjb);

    if (1)FPS_ERROR_PRINT("%s >>  configured [%s]\n [%s]\n", __func__, cj->string, buf);
    return 0;
}
//Note sysVec is for Uri ordering
//This is the asset_manager config we need the wake up function and teh asset manager pointer
void asset_manager::configure(varsmap* vmap, const char* fname, const char* aname, std::vector<std::string>* sysVec, bool(*assWake)(asset*, int), asset_manager* am)
{
    //VarMapUtils vm; // use the asset_manager one

    //vm->getReps(const char *fname, const char *aname,)
    cJSON* cjbase = vm->get_cjson(fname, nullptr);
    if (cjbase == nullptr)
    {
        if (1)FPS_ERROR_PRINT(" %s >> error in file [%s]\n", __func__, fname);
        return;

    }
    if (1)FPS_ERROR_PRINT(" %s >> incoming vmap [%p]\n", __func__, vmap);

    // More UI hacks
    if (sysVec)
    {
        //sysVec->push_back(aname);
        // HACK for UI
        std::string sname = aname;
        sname += "/summary";
        sysVec->push_back(sname);
        FPS_ERROR_PRINT(" %s >> addded [%s] to sysVec size %d \n", __func__, sname.c_str(), (int)sysVec->size());
    }

    // char* body = cJSON_Print(cj);
    //     FPS_ERROR_PRINT(" %s >> cj->string [%s] child [%p] body \n[%s]\n"
    //         , __func__, cj->string, (void *) cj->child, body);

    //     char* buf = vm->fimsToBuffer("set", cj->string, nullptr , body);
    //     free((void *)body);
    //     fims_message* msg = vm->bufferToFims(buf);
    //     free((void *)buf);
    //     cJSON *cjb = nullptr;
    //     vm->processFims(*vmap, msg,  &cjb, am, ai);
    //     vm->free_fims_message(msg);

    //     buf = cJSON_Print(cjb);
    //     if(cjb) cJSON_Delete(cjb);

    //     FPS_ERROR_PRINT("%s >>  configured [%s]\n",__func__, buf);

    cJSON* cji = cjbase->child;
    while (cji)
    {
        FPS_ERROR_PRINT("%s >>  Base child [%s]\n",__func__, cji->string);
        amConfig(vmap, cji, am);

        cji = cji->next;
    }

    //FPS_ERROR_PRINT("%s >>  configured [%s]\n",__func__, buf);

    char* sumname = nullptr;
    asprintf(&sumname, "/assets/%s/summary", aname);
    //cJSON* cj = cjbase->child;

    cJSON* cjs = cJSON_GetObjectItem(cjbase, sumname);
    if (cjs)
    {
        // amConfig(vmap, cjs, am);

        // char* cjtmp = cJSON_Print(cjs);
        // if (cjtmp)
        // {
        //     if (1)FPS_ERROR_PRINT(" %s >> found [%s] cjc >>%s<< \n", __func__, sumname, cjtmp);
        //     free((void*)cjtmp);
        // }
        //cJSON_Delete(cjs);
    }
    if (sumname) free((void*)sumname);

    char* cfgname = nullptr;
    asprintf(&cfgname, "/config/%s", aname);
    //cJSON* cj = cjbase->child;

    cJSON* cjc = cJSON_GetObjectItem(cjbase, cfgname);
    if (cjc)
    {
        amConfig(vmap, cjc, am);
        char* cjtmp = cJSON_Print(cjc);
        if (cjtmp)
        {
            if (1)FPS_ERROR_PRINT(" %s >> found [%s] cjc >>%s<< \n", __func__, cfgname, cjtmp);
            free((void*)cjtmp);
        }
        //cJSON_Delete(cjc);
    }
    if (cfgname) free((void*)cfgname);

    char* assname = nullptr;
    asprintf(&assname, "/assets/%s", aname);


    //cJSON* cj = cjbase->child;
    cJSON* cja = cJSON_GetObjectItem(cjbase, assname);

    if (cja && cja->child)
        cja = cja->child;
    if (assname) free((void*)assname);

    while (cja)
    {
        cJSON* cjsi;
        cJSON* cjt = cJSON_GetObjectItem(cja, "template");
        if (cjt)
        {
            std::string sname = aname;
            sname += "/";
            sname += cja->string;
            //if (sysVec)sysVec->push_back(cja->string);
            if (sysVec)sysVec->push_back(sname);
            if (0)FPS_ERROR_PRINT(" %s >> found asset [%s]\n", __func__, cja->string);
            if (0)FPS_ERROR_PRINT(" %s >> found asset [%s] template [%s]\n", __func__, cja->string, cjt->valuestring ? cjt->valuestring : "No template");

            cJSON* cjs = cJSON_GetObjectItem(cja, "subs");
            if (0)FPS_ERROR_PRINT(" %s   >> found asset [%s] subs %p isArray %s\n"
                , __func__, cja->string, cjs
                , cJSON_IsArray(cjs) ? "true" : "false"
            );
            if (cJSON_IsArray(cjs))
            {
                std::vector<std::pair<std::string, std::string>> reps;

                cJSON_ArrayForEach(cjsi, cjs)
                {
                    cJSON* cjsr = cJSON_GetObjectItem(cjsi, "replace");
                    cJSON* cjsw = cJSON_GetObjectItem(cjsi, "with");

                    if (cjsr && cjsw && cjsr->valuestring && cjsw->valuestring)
                    {
                        if (1)FPS_ERROR_PRINT(" %s   >> found asset [%s] sub from  [%s] to [%s]\n", __func__, cja->string
                            , cjsr->valuestring
                            , cjsw->valuestring);
                        reps.push_back(std::make_pair(cjsr->valuestring, cjsw->valuestring));
                    }
                }
                asset* ass = addAsset(cja, cjt, reps, am);
                //TODO fix duplicate calls to addAsset addINSTANCE
                ass->run_wakeup = assWake;
                ass->setVmap(vmap);
                ass->setVm(vm);
                ass->p_fims = p_fims;
                ass->vecs = am->vecs;

                //TODO add any Pubs into Vecs
                int ccntai = 0;
                vm->getVList(*ass->vecs, *ass->vmap, ass->amap, ass->name.c_str(), "Pubs", ccntai);


                // pcs* pcs = addInstance(cja->string);
                // pcs->setVmap(vmap);
                // pcs->configure(cjt->valuestring, &reps);
            }
        }
        cja = cja->next;
    }
    assname = nullptr;
    asprintf(&assname, "/links/%s", aname);
    cja = cJSON_GetObjectItem(cjbase, assname);
    if (0)FPS_ERROR_PRINT(" %s >> seeking links  [%s] in file [%s] cja %p\n", __func__, assname, fname, (void*) cja);
    if(cja)
    {
        if (1)FPS_ERROR_PRINT(" %s >> found links  [%s]\n", __func__, cja->string);
        //amConfig(vmap, cja, am);
    }
    free((void*)assname);
    // This wont work .. put them inito the template file . Theywill be inserted multiple tines but that's OK
    // asprintf(&assname, "/components/%s", aname);
    // asprintf(&assname, "/components");
    // cja = cJSON_GetObjectItem(cjbase, assname);
    // if (0)FPS_ERROR_PRINT(" %s >> seeking components  [%s] in file [%s] cja %p\n", __func__, assname, fname, (void*) cja);
    // if(cja)
    // {
    //     if (1)FPS_ERROR_PRINT(" %s >> found components  [%s]\n", __func__, cja->string);
    //     amConfig(vmap, cja, am);
    // }
    // free((void*)assname);

}

// send a command to one or all the assets.
const char* asset_manager::send_command(const char* dest, const char* cmd)
{
    const char* res;
    std::string rstr = "";
    if (strcmp(dest, "all") == 0)
    {
        rstr.append("["); // its a list
        for (auto it = assetMap.begin(); it != assetMap.end(); ++it)
        {
            if (it != assetMap.begin())
                rstr.append(",");
            res = (it->second)->get_command(it->first.c_str(), cmd);
            rstr.append(res);

            std::cout << " Manager sent command  " << cmd << " to : " << it->first.c_str() << " res: [" << res << "] rstr: [" << rstr << "]\n";
            free((void*)res);
        }
        rstr.append("]");

    }
    else
    {
        auto it = assetMap.find(dest);
        if (it != assetMap.end()) {
            res = assetMap[dest]->get_command(dest, cmd);
            rstr.append(res);
            std::cout << " Manager sent command  " << cmd << " to : " << dest << " res: [" << res << "] rstr: [" << rstr.c_str() << "]\n";
            free((void*)res);
        }
        else
        {
            std::cout << " Manager dest  [" << dest << "] not in asset map\n";
            for (auto const& x : assetMap) {
                std::cout << "Item Name :[" << x.first << "]\n";
            }
        }
    }
    return rstr.c_str();
}

// a pure virtual function
// this has to be done in the target class to get the full asset type
// but do we need it
//virtual asset* addInstance(const char * name) = 0;
void asset_manager::mapInstance(asset* item, const char* _name)
{
    if (_name)
        item->setName(_name);
    auto it = assetMap.find(item->name);
    if (it == assetMap.end()) {
        FPS_ERROR_PRINT("%s >> mapped instance %s OK\n", __func__, item->name.c_str());
        assetMap[item->name] = item;
    }
    else
    {
        FPS_ERROR_PRINT("%s >> ERROR mapped instance %s FAILED\n", __func__, item->name.c_str());
    }
}



asset* asset_manager::addInstance(const char* _name)
{
    asset* item = new asset(_name);
    // if (item) {
    //     item->setAm(this);
    // }
    std::cout << " added asset instance " << _name << " OK\n";
    mapInstance(item, _name);
    item->am = this;

    return item;
}

asset* asset_manager::getInstance(const char* _name)
{
    auto it = assetMap.find(_name);
    if (it != assetMap.end()) {
        return assetMap[_name];
    }
    return nullptr;
}

int asset_manager::getNumAssets()
{
    return assetMap.size();
}
// set up the thread data systems.
void asset_manager::setupChannels()
{
    t_data.message_chan = &msgchan;
    m_data.message_chan = &msgchan;
    f_data.fims_chan = &fimschan;
}

cJSON* asset_manager::getConfig(const char* uri, const char* var)
{
    //VarMapUtils vm;
    if (vmap)
        return  vm->getMapsCj(*vmap, uri, var);
    return nullptr;
}
// sets up / configs amap and vars

bool asset_manager::runChildren(int wakeup)
{
    //return true;
    if (0)FPS_ERROR_PRINT("%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  looking for child Asset Manager\n", __func__, name.c_str());

    if (1)
    {
        for (auto ix : assetManMap)
        {
            if (0)FPS_ERROR_PRINT("%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  running for Asset Manager [%s]   \n", __func__, name.c_str(), ix.first.c_str());

            asset_manager* am2 = ix.second;  //am->getManAsset("bms");
            if (am2->run_wakeup)
            {
                if (0)FPS_ERROR_PRINT("%s >>>>>>>>> %s Manager Loop Wakeup >>>>>>>>>>> running for Asset Manager [%s] \n", __func__, name.c_str(), ix.first.c_str());
                // first trigger the wakeup there is no thread in the lower leel managers
                    //am2->wakechan.put(wakeup);
                am2->run_wakeup(am2, wakeup);
            }
            else
            {
                FPS_ERROR_PRINT("%s >>>>>>>>> %s Manager Loop NO Wakeup >>>>>>>>>>> running for Asset Manager [%s] \n", __func__, name.c_str(), ix.first.c_str());
            }

            //am2->wakechan.put(wakeup);
        }
    }
    if (1)
    {
        // now do the assets
        for (auto ix : assetMap)
        {
            asset* ass = ix.second; //am->getManAsset("bms");
            if (0)FPS_ERROR_PRINT("%s >>>>>>>>>%s ASSETS >>>>>>>>>>> running for Asset [%s] \n", __func__, name.c_str(), ix.first.c_str());
            // TODO no need to set up the assets with channels
            //TODO ass->wakechan.put(wakeup);
            if (ass->run_wakeup)
                ass->run_wakeup(ass, wakeup);
        }
    }
    return true;
}
// this is the main wake up loop 
// we are cutting down the complexit of this since we have developed the "cascade " concept.
void asset_manager::manager_loop()
{
    int wakeup;
    // char *item3;
    // fims_message * msg;
    // int tnum = 0;
    char* pname;
    asprintf(&pname, "/asset/%s", name.c_str());
    if (0)FPS_ERROR_PRINT("%s  >>>>>>>>    MANAGER LOOP Starting for [%s]\n", __func__, name.c_str());
    // you could use setvar to make sure its there
    // OOPS this look common again
    // any wake up comes here
    while (wakechan.get(wakeup, true)) {
        // then service the other channels 
        //
        if (0)FPS_ERROR_PRINT("%s  %2.3f MANAGER LOOP Wakeup for [%s] wval %d\n", __func__, vm->get_time_dbl(), name.c_str(), wakeup);

        // a wakeup of 0 means service the others
        // a wakeup if 1 means process the asset
        // a wakeup of 2 means pub the asset
        if (run_wakeup)
        {
            if (0)FPS_ERROR_PRINT("%s  %2.3f MANAGER LOOP Run Wakeup for [%s] wval %d\n", __func__, vm->get_time_dbl(), name.c_str(), wakeup);

            // SURELY this should all be in the wakeup code....
            // now do the under managers
            // cut out the auto stuff for now. It was too complex
            if (wakeup != 0)  // dont pass on controller level wakeups 
            {
                run_wakeup(this, wakeup);

                if (0)
                {
                    for (auto ix : assetManMap)
                    {
                        if (0)FPS_ERROR_PRINT("%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  running for Asset Manager [%s] \n", __func__, name.c_str(), ix.first.c_str());

                        asset_manager* am2 = ix.second;  //am->getManAsset("bms");
                        if (am2->run_wakeup)
                        {
                            if (0)FPS_ERROR_PRINT("%s >>>>>>>>> %s Manager Loop Wakeup >>>>>>>>>>> running for Asset Manager [%s] \n", __func__, name.c_str(), ix.first.c_str());
                            // first trigger the wakeup there is no thread in the lower leel managers
                            //am2->wakechan.put(wakeup);
                            am2->run_wakeup(am2, wakeup);
                            am2->runChildren(wakeup);
                        }
                        else
                        {
                            FPS_ERROR_PRINT("%s >>>>>>>>> %s Manager Loop NO Wakeup >>>>>>>>>>> running for Asset Manager [%s] \n", __func__, name.c_str(), ix.first.c_str());
                        }

                        //am2->wakechan.put(wakeup);
                    }
                }
                if (0)
                {

                    // now do the assets
                    for (auto ix : assetMap)
                    {
                        asset* ass = ix.second; //am->getManAsset("bms");
                        if (1)FPS_ERROR_PRINT("%s >>>>>>>>>%s ASSETS >>>>>>>>>>> running for Asset [%s] \n", __func__, name.c_str(), ix.first.c_str());
                        // TODO no need to set up the assets with channels
                        //TODO ass->wakechan.put(wakeup);
                        if (ass->run_wakeup)
                            ass->run_wakeup(ass, wakeup);
                    }
                }
                if (wakeup == -1)
                {
                    // quit
                    FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, name.c_str());
                    running = 0;
                    break;//return false;
                }
            }
        }
    }
    if (pname)free((void*)pname);
}

// manager time loop
void asset_manager::man_timer_loop()
{
    chan_data* td = &t_data;
    FPS_ERROR_PRINT("%s %2.3f MANAGER LOOP %s POLL WAKEUPS DELAY %d \n", __func__, vm->get_time_dbl(), name.c_str(), td->delay);
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);

        //wakeval=1;  // process scan
        if (td->count++ > 10)
        {
            //wakeval = WAKE_LEVEL_PUB;  // run pubs
            td->wake_up_chan->put(WAKE_LEVEL_PUB);
            td->count = 0;
        }
        if (td->count >= 0)
        {
            if (0)FPS_ERROR_PRINT("%s %2.3f MANAGER LOOP %s POLL WAKEUPS \n", __func__, vm->get_time_dbl(), name.c_str());
            td->wake_up_chan->put(WAKE_LEVEL1);
            td->wake_up_chan->put(WAKE_LEVEL2);
            td->wake_up_chan->put(WAKE_LEVEL3);
            td->wake_up_chan->put(WAKE_LEVEL4);
        }
    }

    //pthread_exit(nullptr);
}

// asset timer loop
// Take the timed out asset from the top of the timer queue and throw it at the input channel.
// Queue upthe timer with the next entry
// 
// class Compare
// {
// public:
//     bool operator()(timeVar* t1, timeVar * t2)
//     {
//         return (t1->tt > t2->tt);
//     }
// }


// std::priority_queue<assetVar*, std::vector<assetVar*>, Compare > tqueue;

// // here is a simple test 
// void test_tqueue ()
// {
//     double c_time = get_time_dbl();
//     tqueue.push(new timeVar(c_time+ 0.5,5));
//     tqueue.push(new timeVar(c_time+ 0.1,1));
//     tqueue.push(new timeVar(c_time+ 0.8,8));
//     tqueue.push(new timeVar(c_time+ 1.5,15));

//     while (!tqueue.empty())
//     {
//         printf("xtt %f somestuff %d\n",tqueue.top()->tt,tqueue.top()->somestuff);
//         delete tqueue.top();
//         tqueue.pop();
//     }
// }
// manager time loop

void asset_manager::ass_timer_loop()
{
    chan_data* td = &t_data;
    FPS_ERROR_PRINT("%s %2.3f ASSET LOOP %s POLL WAKEUPS DELAY %d \n", __func__, vm->get_time_dbl(), name.c_str(), td->delay);
    while (*td->run)
    {
        // Note we could do that anyway with the main loop.
        // TODO get next delay from top of timer list of avars ordered by time delay 
        //
        // We'll have a wake up FD as well (socketpair perhaps) to allow a retest of the queue after a new top guy appears 
        poll(nullptr, 0, td->delay);

        //wakeval=1;  // process scan
        if (td->count++ > 10)
        {
            //wakeval = WAKE_LEVEL_PUB;  // run pubs
            td->wake_up_chan->put(WAKE_LEVEL_PUB);
            td->count = 0;
        }
        if (td->count >= 0)
        {
            if (0)FPS_ERROR_PRINT("%s %2.3f MANAGER LOOP %s POLL WAKEUPS \n", __func__, vm->get_time_dbl(), name.c_str());
            td->wake_up_chan->put(WAKE_LEVEL1);
            td->wake_up_chan->put(WAKE_LEVEL2);
            td->wake_up_chan->put(WAKE_LEVEL3);
            td->wake_up_chan->put(WAKE_LEVEL4);
            //td->wake_up_chan->put(WAKE_TIMER);
        }
    }

    //pthread_exit(nullptr);
}

// virtual void man_timer_loop()
// {
//     chan_data* td = &t_data;
//     int wakeval;
//     while (*td->run)
//     {
//         poll(nullptr,0,td->delay);

//         wakeval=1;  // process scan
//         if (td->count++ > 10)
//         {
//             wakeval = 2;  // run pubs
//             td->count = 0;
//         }
//         td->wake_up_chan->put(wakeval);
//     }

//     //pthread_exit(nullptr);
// }

void asset_manager::timer_loop()
{
    chan_data* td = &t_data;
    FPS_ERROR_PRINT("%s %2.3f TIMER LOOP %s POLL WAKEUPS DELAY %d \n", __func__, vm->get_time_dbl(), name.c_str(), td->delay);
    while (*td->run)
        //int wakeval;
        while (*td->run)
        {
            poll(nullptr, 0, td->delay);

            //wakeval=1;  // process scan
            if (td->count++ == 10)
            {
                //wakeval = WAKE_LEVEL_PUB;  // run pubs
                td->wake_up_chan->put(WAKE_LEVEL_PUB);
                td->count = 0;
            }
            //if (td->count == 1)
            {
                if (0)FPS_ERROR_PRINT("%s %2.3f TIMER LOOP %s POLL WAKEUPS \n", __func__, vm->get_time_dbl(), name.c_str());

                td->wake_up_chan->put(WAKE_LEVEL1);
                // cut the clutter down for now
                // td->wake_up_chan->put(WAKE_LEVEL2);
                // td->wake_up_chan->put(WAKE_LEVEL3);
                // td->wake_up_chan->put(WAKE_LEVEL4);
            }
        }

    //pthread_exit(nullptr);
}

// this is the same as all others 
// put in lib

void  asset_manager::message_loop()
{
    char* mdata;
    chan_data* td = &m_data;
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);

        asprintf(&mdata, " this is message number %d", td->count++);
        td->message_chan->put(mdata);
        td->wake_up_chan->put(0);
    }
    //pthread_exit(nullptr);
}

// needs cname , subs
void asset_manager::fims_loop()
{
    chan_data* td = &f_data;

    //char uri[16][64];
    // const char* subs[] = {
    //     "/components", 
    //     "/assets/pcs_1",
    //     "/controls/pcs_1"
    //     }
    // TODO
    //const char **subs = getSubs("pcs_1");
    //int sublen = sizeof td->subs / sizeof td->subs[0];
    int sublen = td->numSubs;// / sizeof td->subs[0];
    int cattempt = 0;
    fims* p_fims = new fims();

    while (!p_fims->Connect((char*)td->name)) {
        poll(nullptr, 0, 1000);
        FPS_ERROR_PRINT("%s >> name %s waiting to connect to FIMS attempt %d\n", __func__, name.c_str(), cattempt++);
    }

    FPS_ERROR_PRINT("%s >> name %s afer connect to FIMS attempts %d sublen %d subs %p\n"
        , __func__, name.c_str(), cattempt++, sublen, (void*)td->subs);
    if (sublen > 15)
        sublen = 15;
    p_fims->Subscribe((const char**)td->subs, sublen);

    while (*td->run)
    {
        fims_message* msg = p_fims->Receive_Timeout(100000);
        if (msg)
        {
            if (strcmp(msg->method, "get") == 0)
            {
                if (0)FPS_ERROR_PRINT("%s >> %2.3f  name %s  GET uri  [%s]\n"
                    , __func__, vm->get_time_dbl(), name.c_str(), msg->uri);
            }

            td->fims_chan->put(msg);
            td->wake_up_chan->put(0);   // but this did not get serviced immediareiy
            if (run_wakeup)
            {
                run_wakeup(this, 0);
            }

        }
    }

    //pthread_exit(nullptr);
}

void asset_manager::cleanup(void)
{
    for (auto& x : amap)
    {
        delete x.second;
    }
    amap.clear();
}

//run the main_loop every 100 mS 
void asset_manager::run_manager(fims* _p_fims)
{
    p_fims = _p_fims;
    // t_data.run = &running;
    // t_data.count = 0;
    // t_data.delay = period;
    // t_data.wake_up_chan = &wakechan;
    FPS_ERROR_PRINT("%s >>>>STARTING MANAGER LOOP for %s\n", __func__, name.c_str());
    manager_thread = std::thread(&asset_manager::manager_loop, this);
    FPS_ERROR_PRINT("%s >>>>>MANAGER LOOP STARTED for %s\n", __func__, name.c_str());
    return;// t_data.cthread;
    //return run(&t_data, (void *(*)(void*))timer_loop,  running, period,  &wakechan);
}

//run the timer_loop every 100 mS 
void asset_manager::run_timer(int period)
{
    t_data.run = &running;
    t_data.count = 0;
    t_data.delay = period;
    t_data.wake_up_chan = &wakechan;
    t_data.cthread = std::thread(&asset_manager::timer_loop, this);
    return;// t_data.cthread;
    //return run(&t_data, (void *(*)(void*))timer_loop,  running, period,  &wakechan);
}
//run the timer_loop every 100 mS 
void asset_manager::run_message(int period)
{
    m_data.run = &running;
    m_data.count = 0;
    m_data.delay = period;
    m_data.wake_up_chan = &wakechan;
    m_data.cthread = std::thread(&asset_manager::message_loop, this);
    //return run(&m_data, (void *(*)(void*))message_loop,  running, period,  &wakechan);
    return;//m_data.cthread;
}


// run the message loop eery 1.5 seconds
//pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running, 1500,  &pcs_man->wakechan);

// the fims system will get pubs and create a varsMap for  the items.
//pcs_man->run_fims(&pcs_man->f_data, fims_loop,    &running, 1500,  &pcs_man->wakechan);
void asset_manager::run_fims(int period, char** subs, const char* name, int numSubs)
{
    FPS_ERROR_PRINT(" %s  >> %s  numSubs %d\n", __func__, name, numSubs);
    f_data.subs = subs;
    f_data.numSubs = numSubs;
    f_data.name = name;
    f_data.run = &running;
    f_data.count = 0;
    f_data.delay = period;
    f_data.wake_up_chan = &wakechan;
    f_data.cthread = std::thread(&asset_manager::fims_loop, this);
    return;// f_data.cthread;
    //return run(&f_data, (void *(*)(void*))fims_loop,  running, period,  &wakechan);
}

// this will 


varmap* asset_manager::getAmap()
{
    return &amap;
}
//TODO
// asset_manager* getAsset(const char * name)
// {
//     auto ix = assetManMap.find(name);
//     if ( ix != assetManMap.end())
//     {
//         return assetManMap[name];
//     }
//     return nullptr;
// }

bool asset_manager::addManAsset(asset_manager* am, const char* name)
{
    assetManMap[name] = am;
    return true;
}

asset_manager* asset_manager::getManAsset(const char* name)
{
    auto ix = assetManMap.find(name);
    if (ix != assetManMap.end())
    {
        return assetManMap[name];
    }
    return nullptr;
}

void asset_manager::cfgwrite(const char* fname, const char* aname)
{

    cJSON* cj = nullptr;
    if (aname != nullptr)
    {
        //vm->getReps(const char *fname, const char *aname,)
    //cJSON* cjbase = vm->get_cjson(fname, nullptr);
    //    cj = vm->getConfig(aname);
    }
    else
    {
        //  cj = vm->getConfig();
    }
    if (cj)
    {
        vm->write_cjson(fname, cj);
        cJSON_Delete(cj);
    }
}

void asset_manager::setVm(VarMapUtils* _vm)
{
    vm = _vm;
}

// thse guys are the wrappers to enable runing sub managers mmanaers and / or children.
int asset_manager::cascadeAI(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am
    , int(*runAI)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am))
{
    return runAI(vmap, amap, aname, p_fims, am);
}
// call this is a method to pass control to sub managers 
// specify the call backs for submanagers  (runAM) or children (runAI) 
// either can be nullptr allowing a cascade to children only or to sub managers only.
int asset_manager::cascadeAM(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am
    , int(*runAM)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
    , int(*runAI)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
)
{
    if (am)
    {
        // assets are in assetMap sub managers are in assetManMap
        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;

            if (0)FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] \n ", __func__, amc->name.c_str());
            if (runAM)runAM(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            cascadeAM(vmap, amc->amap, amc->name.c_str(), p_fims, amc, runAM, runAI);

        }
        // run each of the asst INstances
        for (auto ix : am->assetMap)
        {
            asset* amc = ix.second;

            if (0)FPS_ERROR_PRINT("%s >>>>>> cascading function to >>>> Asset [%s] \n ", __func__, amc->name.c_str());

            //runAI(vmap, amc->amap,amc->name.c_str(), p_fims, amc);
            if (runAI)cascadeAI(vmap, amc->amap, amc->name.c_str(), p_fims, amc, runAI);

        }
    }
    return 0;
}

int asset_manager::Send(const char* method, const char*uri, const char*rep, const char* body)
{
    //p_fims->Send("pub", comp, nullptr, dvar);
    int ival = p_fims->Send(method, uri, rep, body);
    return ival;
}

/**************************************************************************************************************************************************************/

/******************************************************
 *              
 *                 VarMapUtils.h
 *    
 ******************************************************/

/***********************************************
 *                 VarMapUtils
 ***********************************************/

VarMapUtils::VarMapUtils()
{
    sysVec = nullptr;
    alarmId = 0;
    set_base_time();
}
VarMapUtils::~VarMapUtils(){}

assetVar*VarMapUtils::runActFuncfromCj(varsmap &vmap, assetVar *av, assetAction* aa)
{
    // for now run the sets directly
    //std::map<int,assetBitField *> bitmap;
    //int aval = av->aVal->valueint;
    char *strval;
    assetVal*aVal = av->aVal;
    if(av->linkVar)
    {
        aVal = av->linkVar->aVal;
    }
    if(aVal->valuestring)
    {
        strval = (char *)strdup(aVal->valuestring);
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

        if(0)FPS_ERROR_PRINT(" %s >> running a Function av [%s] func [%s] am %p ai %p enable [%s]\n"
                ,__func__, av->name.c_str()
                , func, av->am, av->ai
                , enable ? "true":"false"
                );
        if(av->ai)
        {
            if(0)FPS_ERROR_PRINT(" %s >> running a Function  for an asset instance  [%s] \n",__func__, av->ai->name.c_str() );
        }
        else if(av->am)
        {
            myAvfun_t amFunc;
            void *res1 = nullptr;
            if(av->am->vm)
            {
                // ess could be the amap feature 
                // ODO maybe use the amap field instead of hard coding ess here
                //res1 = av->am->vm->getFunc(vmap, "ess", func );
                res1 = av->am->vm->getFunc(vmap, av->am->name.c_str(), func);  // added change here -> should run func for arbitrary asset managers
                if(res1)
                {
                    amFunc = reinterpret_cast<myAvfun_t> (res1);
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

int VarMapUtils::testRes(const char* tname, varsmap& vmap, const char* method, const char* uri, const char* body, const char* res, asset_manager* am, asset* ai)
{
    int rc = 1;
    char* tmp;
    // this is our map utils factory
    VarMapUtils vm;
    cJSON* cjr = nullptr;//cJSON_CreateObject();
    const char* replyto = "/mee";
    char* buf = vm.fimsToBuffer(method, uri, replyto, body);
    //free((void *)body);
    fims_message* msg = vm.bufferToFims(buf);
    free((void*)buf);

    runFimsMsg(vmap, msg, nullptr,&cjr);
    //processFims(vmap, msg, &cjr, am, ai);
    free_fims_message(msg);
    tmp = cJSON_PrintUnformatted(cjr);
    if (tmp)
    {
        if (!res)
        {
            printf("\n%s\tOk reply >>%s<<\n\n", tname, tmp);

        }
        else if (strcmp(res, tmp) == 0)
        {
            //printf(" PASSED \n");
            printf("\n%s\tOk PASSED reply >>%s<<\n\n", tname, tmp);
        }
        else
        {
            unsigned int ires = strlen(res);
            unsigned int itmp = strlen(tmp);
            unsigned int iuse = itmp;

            if (iuse > ires)
            {
                iuse = ires;
            }

            for (unsigned int i = 1; i < iuse; i++)
            {
                if (strncmp(res, tmp, i) != 0)
                {
                    printf(" test failed at loc %d \ntmp[%s] \nres[%s] \n"
                        , i
                        , &tmp[i - 1]
                        , &res[i - 1]
                    );
                    break;


                }
            }

            printf("\n%s\tFAILED reply >>%s<<\n\n", tname, tmp);
            rc = 0;
        }
        free((void*)tmp);
    }
    cJSON_Delete(cjr);
    return rc;
}

assetVar* VarMapUtils::runActValfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    // TODO map these
    if (aa)
    {
        if (0) FPS_ERROR_PRINT(" %s >> #######on Set action aa->name [%s]\n", __func__, aa->name.c_str());
        av->aa = aa;
        if (strcmp(aa->name.c_str(), "bitfield") == 0)
        {
            av = runActBitFieldfromCj(vmap, av, aa);
        }
        else if (strcmp(aa->name.c_str(), "bitset") == 0)
        {
            av = runActBitSetfromCj(vmap, av, aa);
        }
        else if (strcmp(aa->name.c_str(), "enum") == 0)
        {
            av = runActEnumfromCj(vmap, av, aa);
        }
        else if (strcmp(aa->name.c_str(), "remap") == 0)
        {
            av = runActRemapfromCj(vmap, av, aa);
        }
        else if (strcmp(aa->name.c_str(), "func") == 0)
        {
            av = runActFuncfromCj(vmap, av, aa);
        }
        else if (strcmp(aa->name.c_str(), "limits") == 0)
        {
            av = runActLimitsfromCj(vmap, av, aa);
        }
        else if (strcmp(aa->name.c_str(), "bitmap") == 0)
        {
            av = runActBitMapfromCj(vmap, av, aa);
        }


    }
    return av;
}

assetVar* VarMapUtils::runActBitMapfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    // for now run the sets directly
    //std::map<int,assetBitField *> bitmap;
        assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

    int aval = (int)aVal->valuedouble;
    char* sval = aVal->valuestring;
    //double dval = av->aVal->valuedouble;
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        av->abf = abf;
        //double inValue = abf->getFeat("inValue", &inValue);
        double mask = abf->getFeat("mask", &mask);
        int shift = abf->getFeat("shift", &shift);
        char* uri = abf->getFeat("uri", &uri);
        int bit = abf->getFeat("bit", &bit);
        
        bool gotsval = abf->gotFeat("svalue");
        char* svalue = abf->getFeat("svalue", &svalue);

        bool gotnval = abf->gotFeat("nvalue");
        int nvalue = abf->getFeat("nvalue", &nvalue);

        int outval = 0;

        assetVar* avout = getVar(vmap, uri, nullptr);
        if(avout)
        {
            outval = av->getiVal();
        }
        if(gotsval)
        {
            bool neg = false;
            if(sval[0] == '!')
            {
                neg = true;
                sval++;
            }
            if(strcmp(sval,svalue)==0)
            {
                if(neg)
                {
                    outval &= ~(int)(1<<bit);
                }
                else
                {
                    outval |= (int)(1<<bit);
                }
            }
        }
        else if(gotnval)
        {
            bool neg = false;
            if(aval < 0)
            {
                neg = true;
                aval = -aval;
            }
            if(aval == nvalue)
            {
                if(neg)
                {
                    outval &= ~(int)(1<<bit);
                }
                else
                {
                    outval |= (int)(1<<bit);
                }
            }
        }
        else
        {
            bit = (int)aval;
            bit &=(int)mask;

            if(shift > 0)
            {
                bit = bit << shift;
                mask = (int)mask << shift;
            }
            outval &= ~(int)mask;
            outval |= (int)bit;

        }

        if(avout)
        {
            avout->setVal((int)outval);
        }

        if (1)FPS_ERROR_PRINT(" %s >> ######on Set BitMap action av %p mask 0x%04x hbit 0x%04x  shift %d aval %d masked val %d uri [%s] outval 0x%04x\n"
            , __func__
            , av
            , (int)mask
            , (int)bit
            , shift
            , aval
            , (int)aval & (int)mask
            , uri ? uri : "NoUri"
            , outval 
            );
    }
    return av;
}

// added inValue+ inValue- 
assetVar* VarMapUtils::runActEnumfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    // for now run the sets directly
    //std::map<int,assetBitField *> bitmap;
    assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

    double aval = aVal->valuedouble;
    bool fired = false;
    //double dval = av->aVal->valuedouble;
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        av->abf = abf;
        bool gotRange = abf->gotFeat("useRange");

        double inValue = abf->getFeat("inValue", &inValue);
        double inValuePlus = abf->getFeat("inValue+", &inValuePlus);
        double inValueNeg  = abf->getFeat("inValue-", &inValueNeg);
        double mask = abf->getFeat("mask", &mask);
        int shift = abf->getFeat("shift", &shift);
        char* uri = abf->getFeat("uri", &uri);
        char* var = abf->getFeat("var", &var);    // var can be null if we use /comp:var
        char* outValue = abf->getFeat("outValue", &outValue);

        int bit = (int)inValue;
        if(shift > 0)
        {
            bit = bit << shift;
            mask = (int)mask << shift;
        }

        if (0)FPS_ERROR_PRINT(" %s >> ######on Set Enum action inValue %d mask 0x%04x hbit 0x%04x  shift %d aval %f masked val %d uri [%s] var [%s] tmpval [%s] outValue[%s]\n"
            , __func__
            , (int)inValue
            , (int)mask
            , (int)bit
            , shift
            , aval
            , (int)aval & (int)mask
            , uri ? uri : "NoUri"
            , var ? var : "NoVar"
            , abf->getTmpval()
            , outValue ? outValue : "noOutvalue"
        );

        if(!gotRange)
        {
            fired = (((int)aval & (int)mask) == (int)bit);
        }
        else
        {
            fired = ((aval < (inValue + inValuePlus)) && (aval >(inValue-inValueNeg)));
        }

        if (fired)
        {
            cJSON* cjov = cJSON_Parse(abf->getTmpval());
            if (0)FPS_ERROR_PRINT(" %s >> Mask Check OK tmpval [%s] cjov %p , uri [%s], var [%s]\n"
                , __func__
                , abf->getTmpval()
                , (void*)cjov
                , uri
                , var ? var : "NoVar"
            );

            if (cjov)
            {
                setValfromCj(vmap, uri, var, cjov);
                cJSON_Delete(cjov);
            }
            //setValfromCj(vmap, uri, var, cJSON_Parse(abf->getTmpval()));
            if (0) //FPS_ERROR_PRINT // send fims
            {
                char* stmp;
                asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"body\":%s}", uri, abf->getTmpval());
                if (0)FPS_ERROR_PRINT(" #######Set action [%s]\n", stmp);
                free((void*)stmp);
            }
        }
    }
    return av;
}

assetVar* VarMapUtils::runActBitFieldfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    // for now run the sets directly
    //std::map<int,assetBitField *> bitmap;
    assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

    int aval = (int)aVal->valuedouble;

    //av->aVal->valueint =  (int)av->aVal->valuedouble;
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        int bit = abf->getFeat("bit", &bit);
        double inValue = abf->getFeat("inValue", &inValue);
        double mask = abf->getFeat("mask", &mask);
        char* uri = abf->getFeat("uri", &uri);
        char* var = abf->getFeat("var", &var);
        //char* oval   = abf->getFeat("outValue", &oval);
        //double outValue  = abf->getFeat("outValue", &outValue);
        cJSON* cjov = abf->getFeat("outValue", &cjov);
        char* sstemp = nullptr;
        if (cjov)
        {
            sstemp = cJSON_PrintUnformatted(cjov);
        }

        //int bit = (int)inValue;
        if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >> ######on Set Bitfield action bit %d mask 0x%04x hbit 0x%04x  value %d  bit value %d masked val %d uri [%s] var [%s] sstmp[%s] tmpval [%s]\n"
            , __func__
            , (int)bit
            , (int)mask
            , (int)bit
            , aval
            , (int)(1 << bit)
            , (int)(aval & (1 << bit))
            , uri
            , var
            , sstemp ? sstemp : "no outValuecj"
            , abf->getTmpval()
        );

        if (aval & (1 << (int)bit))
        {
            cJSON* cjt = cjov;
            if (cjt)
            {
                //assetVar* setOldValfromCj(varsmap &vmap, const char* comp, const char* var, cJSON* cj)
                if (0)FPS_ERROR_PRINT(" %s >> ####### before setValfromCj  [%s:%s] body [%s] \n", __func__, uri, var, sstemp);

                assetVar* avs = setValfromCj(vmap, uri, var, cjt);

                char* stmp;
                asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"var\":%s,\"body\":%s}", uri, var, sstemp);
                if (avs)
                {
                    if (0)FPS_ERROR_PRINT(" %s >> #######Set  var [%s:%s] body [%s] avs->name[%s] setFunc %p \n", __func__, uri, var, sstemp, avs->name.c_str()
                    , (void*)avs->extras?av->extras->SetFunc:nullptr);
                }
                else
                {
                    if (0)FPS_ERROR_PRINT(" %s >> #######Set  var [%s:%s] body [%s] NOAVS \n", __func__, uri, var, sstemp);
                }
                free((void*)stmp);
                //cJSON_Delete(cjt);
            }
            else
            {
                char* stmp;
                asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"var\":%s,\"body\":%s}", uri, var, abf->getTmpval());
                if (0)FPS_ERROR_PRINT(" %s >> #######Set action [%s] cjit %p \n", __func__, stmp, (void*)cjt);
                free((void*)stmp);
            }


            if (0) // sendfims
            {
                char* stmp;
                asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"var\":%s,\"body\":%s}", uri, var, abf->getTmpval());
                if (0)FPS_ERROR_PRINT(" %s >> #######Set action [%s]\n", __func__, stmp);
                free((void*)stmp);
            }
        }
        if (sstemp)
        {
            free((void*)sstemp);
        }
        if (cjov)
        {
            cJSON_Delete(cjov);
        }
    }
    return av;
}

assetVar* VarMapUtils::runActBitSetfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

    bool bval = (int)aVal->valuebool;
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        int bit = abf->getFeat("bit", &bit);
        int aval = 0;
        //double inValue = abf->getFeat("inValue", &inValue);
        //double mask = abf->getFeat("mask", &mask);
        char* uri = abf->getFeat("uri", &uri);
        char* var = abf->getFeat("var", &var);
        bool soloBit = abf->getFeat("soloBit", &soloBit);
        assetVar* av = getVar(vmap, uri, var);
        if (av)
        {
            aval = av->getVal(aval);
        }
        else
        {
            av = setVal(vmap, uri, var, aval);
        }
        //int bit = (int)inValue;
        if (0 || setvar_debug)FPS_ERROR_PRINT(
            " %s >> ######on Set BitSet action bit %d value %s  input val 0x%04x  output val 0x%04x "
            " uri [%s] var [%s] soloBit [%d]\n"
            , __func__
            , (int)bit
            //, (int)mask
            //, (int)bit
            , bval ? "true" : "false"
            , (int)(1 << bit)
            , bval ? (int)(aval | (1 << bit)) : (int)(aval & ~(1 << bit))
            , uri
            , var
            , soloBit
        );

        if (bval)
        {
            // set the bit
            if (soloBit)
                aval = bit ? 1 << (bit - 1) : 0;
            else if (bit)
                aval |= 1 << (bit - 1);
        }
        else
        {
            // clear the bit
            if (!soloBit)
                aval = aval & ~(1 << bit);
        }
        setVal(vmap, uri, var, aval);
    }
    return av;
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

void VarMapUtils::write_cjson(const char* fname, cJSON* cj)
{
    FILE* fp = nullptr;
    fp = fopen(fname, "w");
    if (fp == nullptr)
    {
        FPS_ERROR_PRINT("Failed to open file %s\n", fname);
        return;
    }

    char* res = cJSON_Print(cj);
    if (res)
    {
        size_t bytes_written = fwrite(res, 1, strlen(res), fp);
        FPS_ERROR_PRINT(" %s >> Wrote %d bytes to  file %s\n", __func__, (int)bytes_written, fname);
        free((void*)res);
    }
    fclose(fp);
}

// get a file and replace string patterns found 
char* VarMapUtils::get_cjfile(const char* fname, std::vector<std::pair<std::string, std::string>>* reps)
{
    FILE* fp = nullptr;

    if (fname == nullptr)
    {
        FPS_ERROR_PRINT(" Failed to get the path of the config file. \n");
        return nullptr;
    }

    fp = fopen(fname, "r");
    if (fp == nullptr)
    {
        FPS_ERROR_PRINT("Failed to open file %s\n", fname);
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    long unsigned file_size = ftell(fp);
    rewind(fp);

    // create Configuration_file and read file in Configuration_file
    char* config_file = (char*)calloc(1, file_size + 1);
    if (config_file == nullptr)
    {
        FPS_ERROR_PRINT("Memory allocation error\n");
        fclose(fp);
        return nullptr;
    }

    size_t bytes_read = fread(config_file, 1, file_size, fp);
    fclose(fp);
    if (bytes_read != file_size)
    {
        FPS_ERROR_PRINT("Read size error.\n");
        free((void*)config_file);
        return nullptr;
    }
    config_file[bytes_read] = 0;

    if (reps)
    {
        std::string cstr = config_file;
        for (int i = 0; i < (int)reps->size(); i++)
        {
            auto x = reps->at(i);
            std::string fstr = x.first;
            std::string tstr = x.second;
            std::size_t start_pos = 0;
            FPS_ERROR_PRINT(" %s >> Replacing [%s] with [%s] in file [%s]\n", __func__, fstr.c_str(), tstr.c_str(), fname);

            while ((start_pos = cstr.find(fstr, start_pos)) != std::string::npos)
            {
                cstr.replace(start_pos, fstr.length(), tstr);
                start_pos += tstr.length(); // ...
            }
        }

        free((void*)config_file);
        config_file = strdup(cstr.c_str());
    }
    return config_file;
}

cJSON* VarMapUtils::get_cjson(const char* fname, std::vector<std::pair<std::string, std::string>>* reps)
{
    char* config_file = get_cjfile(fname, reps);
    cJSON* cj = nullptr;
    if (config_file)
    {
        cj = cJSON_Parse(config_file);
        free((void*)config_file);
    }
    if (cj == nullptr)
        FPS_ERROR_PRINT("Invalid JSON object in file\n");
    return cj;
}

//TODO use assetURI --> DONE
assetVar* VarMapUtils::makeVar(varsmap& vmap, const char* comp, const char* var, assetList* alist)
{
    assetUri my(comp,var);
    if(0) FPS_ERROR_PRINT("%s >> makeVar   comp [%s] var [%s] my.Uri [%s] my.Var[%s] my.Param [%s]\n"
            , __func__
            , comp
            , var
            , my.Uri
            , my.Var
            , my.Param
            );

    assetVar* av = nullptr;
    bool tval = true;
    av = new assetVar(my.Var, my.Uri, tval);
    vmap[my.Uri][my.Var] = av;
    return av;
}

// create a duplicate link to the same av
//TODO use assetURI --> DONE
assetVar* VarMapUtils::makeAVar(varsmap& vmap, const char* comp, const char* var, assetVar* av)
{
    assetUri my(comp, var);
    if(0) FPS_ERROR_PRINT("%s >> makeVar for Av input av %p\n", __func__, (void*)av);
    if(0) FPS_ERROR_PRINT("%s >> makeAVar   comp [%s] var [%s] my.Uri [%s] my.Var[%s] my.Param [%s] av %p\n"
            , __func__
            , comp
            , var
            , my.Uri
            , my.Var
            , my.Param
            , (void*)av
    );

    vmap[my.Uri][my.Var] = av;
    return av;
}

// new code we want the linked var exposed 
// TODO use AssetURI --> DONE
assetVar* VarMapUtils::replaceAv(varsmap& vmap, const char* comp, const char* var, assetVar* av)
{
    assetUri my(comp, var);

    if(0) FPS_ERROR_PRINT("%s >> replaceAv   comp [%s] var [%s] my.Uri [%s] my.Var[%s] my.Param [%s] av %p\n"
            , __func__
            , comp
            , var
            , my.Uri
            , my.Var
            , my.Param
            , (void*)av
    );

    assetVar* oldAv = getVar(vmap, comp, var);
    if (oldAv)
    {
        if(0) FPS_ERROR_PRINT(" %s >> NOT removing comp [%s] var [%s] old AV comp[%s] name[%s] old  %p new %p\n"
            , __func__
            , comp
            , var
            , oldAv->comp.c_str()
            , oldAv->name.c_str()
            , (void*)oldAv
            , (void*)av
        );
        //TODO is this an orphan now 
        //delete oldAv;
    }
    vmap[my.Uri][my.Var] = av;
    return av;
}



void VarMapUtils::setFunc(varsmap& vmap, const char* aname, const char* fname, void* func)
{
    char* comp;
    char* fval;

    asprintf(&comp, "/functions/%s", aname);
    asprintf(&fval, "%p", func);
    if (fval && comp)
    {
        setVal(vmap, (const char*)comp, fname, fval);
    }
    if (comp)free((void*)comp);
    if (fval)free((void*)fval);
}
    //void setAmFunc(vmap, "comp", "/alarms", ass_man, void*func);
    //vm->setAmFunc(vmap, "comp", "/alarms", aname, ass_man, (void*)&dummy_bms_alarm);

void VarMapUtils::setAmFunc(varsmap& vmap, const char* aname, const char *fname, const char* amname, asset_manager* am, void* func)
{
    char* comp;
    char* cname;
    char* fval;

    asprintf(&comp, "/functions/%s", aname);
    asprintf(&cname, "%s/%s", fname, amname);
    asprintf(&fval, "%p", func);
    if (fval && comp)
    {
        if (0)FPS_ERROR_PRINT("%s >>  Func created ->>[%s:%s]\n", __func__, comp, fname);
        assetVar* av = setVal(vmap, (const char*)comp, cname, fval);
        av->am = am;
    }
    if (cname)free((void*)cname);
    if (comp)free((void*)comp);
    if (fval)free((void*)fval);
}

void* VarMapUtils::getFunc(varsmap& vmap, const char* aname, const char* fname, assetVar*avi)
{
    char* comp;
    char* var = nullptr;
    void* res = nullptr;
    // If we use a module or shared library  then do that instead

    asprintf(&comp, "/functions/%s", aname);
    if (comp)
    {
        assetVar* av = getVar(vmap, comp, fname);
        if (!av)
            var = nullptr;
        else
        {
            assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;
            var = aVal->valuestring;
            // transfer the am to th seeking avi
            if(avi && (avi->am == nullptr))
                avi->am = av->am;
        }
    }
    if (var)
    {
        sscanf(var, "0x%x", (unsigned int*)&res);

        if (0)FPS_ERROR_PRINT(" %s value recovered [%s] res %p\n"
            , __func__
            , var
            , res);

    }
    if (comp)free((void*)comp);
    return (void*)res;
}



// template <class T>
// assetVar*setVal2(varsmap &vmap, const char* comp, const char* var, T &value)
// {

//     // only set it if we can find it
//     assetVar* av = setVal(vmap, comp, var, value);
//     if(av)
//     {
//         av->setVal(value);
//         return av;
//     }

//     return av;

// }

const char* VarMapUtils::getdefLink(const char* name, const char* base, const char* var, char* buf, int blen)
{
    snprintf(buf, blen, "%s/%s:%s", base, name, var);
    return (const char*)buf;
}
//vm.setVal2(vmap, link,"AcContactor",               sAcContactor);
//vm.setVal3(vmap, link,"AcContactor",               "/status", name.c_str());
//vm.setVal3(vmap, "/link/bms_1","AcContactor",               "/status", "AcContactor";
// assetVar*setVal3(varsmap &vmap, const char* comp, const char*var,  const char* base, const char * name)
// {
//     char buf[1024];
//     const char* value = getdefLink(name, base, var, buf, sizeof(buf));
//     return setVal2(vmap, comp, var, value);
// }
// links to the value from the config
// does not set a value
//HeartBeat                 = linkVal(vmap, link, "HeartBeat",                     ival);

template <class T>
assetVar* VarMapUtils::linkVal(varsmap& vmap, const char* comp, const char* var, T& defvalue)
{
    //printf(" %s looking for comp [%s] var [%]\n",__func__,comp,var);
    // this gets the link
    // comp may have a : so remove it
    char* comp1 = strdup(comp);
    char* sp = strstr(comp1, ":");
    if (sp)
        *sp = 0;

    assetVar* av = getVar(vmap, comp1, var);
    assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

    if (0)FPS_ERROR_PRINT(" %s >>looking for comp [%s] comp1 [%s] var [%s] got %p %s\n"
        , __func__
        , comp
        , comp1
        , var
        , (void*)av
        , av ? aVal->valuestring : "noval"
    );
    free((void*)comp1);
    if (av)
    {
        if (aVal->valuestring)
        {
            char* comp2 = aVal->valuestring;
            char* comp3 = strdup(comp2);
            char* sp = strstr(comp3, ":");
            if (sp)
                *sp = 0;

            av = getVar(vmap, comp3, var);
            if (0)FPS_ERROR_PRINT(" %s >>now looking for linked  comp3 [%s] var [%s] got %p\n"
                , __func__, comp3, var, (void*)av);
            if (!av)
            {
                av = makeVar(vmap, comp3, var, defvalue);
                if (0)FPS_ERROR_PRINT(" %s >>now created linked  comp [%s] var [%s] av now  %p\n"
                    , __func__, comp3, var, (void*)av);
            }
            free((void*)comp3);
            return av;
        }
        else
        {
            if (1)FPS_ERROR_PRINT(" %s >>rogue link comp [%s] var [%s] skipped\n", __func__, comp, var);
        }


    }
    // we cant find it so make it and set a default value
    av = makeVar(vmap, comp, var, defvalue);
    return av;

}
//creates a vlink from vname to lname
// this means that vname has its assetVal linked to lname using the linkVar thing
// "/vlinks/ess": {
//     "MinCellVolt": {
//     "value": "/site/ess:MinCellVolt",
//     "vlink": "/components/catl_mbmu_control_r:mbmu_min_cell_voltage"
// }


void VarMapUtils::setVLinks(varsmap& vmap, const char* aname)
{
    for (auto& x : vmap)
    {
        if (strncmp(x.first.c_str(),"/vlinks", strlen("/vlinks")) == 0)
        {
            for (auto& y : x.second)
            {

                const char*vname = y.second->getcVal();
                const char*vlink = y.second->getcParam("vlink");

                if (0)FPS_ERROR_PRINT("\n%s >>looking to link vname [%s] to  [%s]\n"
                        , __func__
                        , vname
                        , vlink
                );
                if(vname && vlink)
                {
                    setVLink(vmap, vname, vlink);
                }
            }
        }
    }
    
}


//amap["HandleLoadRequest"]      = vm.setVL(vmap, aname, "/controls",  "HandleLoadRequest",         reload);
assetVar* VarMapUtils::setVLink(varsmap& vmap, const char* vname, const char* vlink)
{
    //char* avar = var;
    //bool var = false;
    assetVar* av = getVar(vmap, vname, nullptr);
    // target assetvar  /components/pcs_pe:pmode_control
    assetVar* link = getVar(vmap, vlink, nullptr);

    if (0) FPS_ERROR_PRINT("\n%s >>looking to link vname [%s] %p to  [%s] %p\n"
        , __func__
        , vname
        , av
        , vlink
        , link
    );
    if (av && link)
    {
        if(1)av->linkVar = link;
    }
    else
    {
        if (0) FPS_ERROR_PRINT("\n %s >>Unable to link vname [%s] %p to  [%s] %p\n"
            , __func__
            , vname
            , av
            , vlink
            , link
        );
    }
    return av;
}



// links to the value from the config
// does not set a value
//HeartBeat                 = linkVal(vmap, link, "HeartBeat",                     ival);
//amap["HandleLoadRequest"]      = vm.setLinkVal(vmap, aname, "/controls",  "HandleLoadRequest",         reload);
template <class T>
assetVar* VarMapUtils::setLinkVal(varsmap& vmap, const char* aname, const char* cname, const char* var, T& defvalue)
{
    //printf(" %s looking for comp [%s] var [%]\n",__func__,comp,var);
    // this gets the link
    // comp may have a : so remove it
    // char *comp1 = strdup(comp);
    // char *sp = strstr(comp1,":");
    // if(sp)
    //  *sp = 0;

    char* linkcomp = nullptr;
    char* linkvar = nullptr;
    char* comp3 = nullptr;
    char* acomp = nullptr;
    //char* avar = var;

    asprintf(&linkcomp, "/links/%s", aname);
    // linked assetvar  /links/pcs
    asprintf(&acomp, "%s/%s", cname, aname);
    // linked assetvar  /links/pcs
    assetVar* lav = getVar(vmap, linkcomp, var);
    // target assetvar  /components/pcs_pe:pmode_control
    assetVar* tav = nullptr;
    assetVar* aav = nullptr;
    // = getVar(vmap, acomp, var);

    // assetVal* laVal = lav->linkVar?lav->linkVar->aVal:lav->aVal;

    if (0)FPS_ERROR_PRINT("\n\n %s >>looking for linkcomp [%s] cname [%s] var [%s] got %p %s\n"
        , __func__
        , linkcomp
        , cname
        , var
        , (void*)lav
        , lav ? lav->aVal->valuestring : "noval"
    );
    if (!lav)
    {
        // aname = bms_1
        // cname = /params
        // var = LoadSetpoint
        // create default /links/bms_1/LoadSetpoint with a string  value of /params/bms_1:LoadSetpoint
        asprintf(&linkvar, "%s/%s:%s", cname, aname, var);
        lav = makeVar(vmap, linkcomp, var, linkvar);
        lav->setVal(linkvar);
        // laVal = lav->linkVar?lav->linkVar->aVal:lav->aVal;
        // if (0)FPS_ERROR_PRINT(" %s >>created linkcomp [%s]  var [%s] linkvar [%s] got %p %s\n"
        //     , __func__
        //     , linkcomp
        //     , var
        //     , linkvar
        //     , (void*)lav
        //     , lav ? laVal->valuestring : "noval"
        // );

    }

    if (lav)
    {
        if (0)FPS_ERROR_PRINT(" %s >>found linkcomp [%s]  var [%s] linkvar [%s] got %p %s\n"
            , __func__
            , linkcomp
            , var
            , linkvar
            , (void*)lav
            , lav ? lav->aVal->valuestring : "noval"
        );


        // now see if the taget variable exists
        // /components/pcs_pe:pmode_control
        if (lav->aVal->valuestring)
        {
            char* comp2 = lav->aVal->valuestring;
            comp3 = strdup(comp2);
            char* sp = strstr(comp3, ":");
            if (sp)
            {
                *sp = 0;
                sp++;
            }
            else
            {
                sp = (char*)var;
            }
            tav = getVar(vmap, comp3, sp);
            if (0)FPS_ERROR_PRINT(" %s >>now looking for link target comp3 [%s] var [%s] got %p\n"
                , __func__, comp3, sp, (void*)tav);

            // no target to var so make one
            if (!tav)
            {
                tav = makeVar(vmap, comp3, sp, defvalue);
                if (0)FPS_ERROR_PRINT(" %s >> created target  comp [%s] var [%s] av now  %p\n"
                    , __func__, comp3, sp, (void*)tav);
            }
        }
        // now look for /controls/pcs:pmode
        aav = getVar(vmap, acomp, var);
        if (0)FPS_ERROR_PRINT(" %s >>now looking for amap  [%s] var [%s] got %p\n"
            , __func__, acomp, var, (void*)aav);
        if (!aav)
        {
            aav = makeAVar(vmap, acomp, var, tav);
            if (0)FPS_ERROR_PRINT(" %s >>now creating var for  amap  [%s] var [%s] got %p\n"
                , __func__, acomp, var, (void*)aav);
        }
        else
        {
            if (aav != tav)
            {
                if (0)FPS_ERROR_PRINT(" %s >>now replacing aVar for  amap  [%s] var [%s]  %p  with target av %p\n"
                    , __func__, acomp, var, (void*)aav, (void*)tav);
                // amap assetvar /controls/pcs/pmode  ( will have the same av as the taget av)
                aav = replaceAv(vmap, acomp, var, tav);
            }
            else
            {
                if (0)FPS_ERROR_PRINT(" %s >> same var >>>> NOT replacing aVvar for  amap  [%s] var [%s]  with target av\n"
                    , __func__, acomp, var);
            }
        }
    }
    else
    {
        FPS_ERROR_PRINT(" %s >>rogue link comp [%s] var [%s] skipped\n", __func__, linkcomp, var);
    }

    if (0)FPS_ERROR_PRINT(" %s >> link comp [%s] var [%s] completed\n", __func__, linkcomp, var);
    if (linkcomp)free((void*)linkcomp);
    if (linkvar)free((void*)linkvar);
    if (comp3)free((void*)comp3);
    if (acomp)free((void*)acomp);

    return tav;
}

assetVar* VarMapUtils::runActLimitsfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

    double inVal = aVal->valuedouble;

    assetBitField* afd = aa->getBitField(0);
    double low = afd->getFeat("low", &low);
    double high = afd->getFeat("high", &high);
    double aval = inVal;

    if (0 || setvar_debug)FPS_ERROR_PRINT(
        " %s >> ###### av [%s] value [%f] LOW [%f] HIGH [%f]\n"
        , __func__
        , av->name.c_str()
        , inVal
        , low
        , high
    );

    if (inVal < low)
        aval = low;
    else if (inVal > high)
        aval = high;

    av->setVal(aval);

    return av;
}


assetVar* VarMapUtils::runActRemapfromCj(varsmap& vmap, assetVar* av, assetAction* aa)
{
    // for now run the sets directly
    //std::map<int,assetBitField *> bitmap;
    //int aval = av->aVal->valueint;
    char* strval;
    if (0)FPS_ERROR_PRINT(" %s >> #######Remap action started av [%s] cvalue [%s]  int %d bool [%s]\n"
            , __func__
            , av->name.c_str()
            , av->getcVal()
            , av->getiVal()
            , av->getbVal()? "true":"false"
            );
    assetVal*aVal = av->aVal;
    if(av->linkVar)
    {
        aVal = av->linkVar->aVal;
    }
    if (aVal->valuestring)
    {
        strval = (char*)strdup(aVal->valuestring);
    }
    else
    {
        strval = (char*)strdup("NoVal");
    }
    // we can have several remap functions.
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        bool enable = true;
        char* ensp = nullptr;    abf->getFeat("enable", &ensp);
        if (0)FPS_ERROR_PRINT(" %s >> #######Remap action started enable [%s]\n"
            , __func__
            , ensp
            );
        if (ensp)
        {
            assetVar* av = getVar(vmap, (const char*)ensp, nullptr);
            if (0)FPS_ERROR_PRINT(" %s >> #######Remap action started enable av [%p]\n"
                , __func__
                , av
                );
            if (av)
            {
                enable = av->getbVal();
            }
        }

        double scale = 1.0; abf->getFeat("scale", &scale);
        double offset = 0;   abf->getFeat("offset", &offset);
        char* uri = nullptr;   abf->getFeat("uri", &uri);
        char* var = nullptr;   abf->getFeat("var", &var);
        // oops we need to know the invalue type
        // but we could use the cJSON 
        cJSON* cjav = nullptr;
        cJSON* cjiv = nullptr;

        if (abf->gotFeat("inValue"))
        {
            cjiv = abf->getFeat("inValue", &cjiv);
        }
        cjav = aVal->getValCJ(scale, offset);
        bool match = true;
        if(cjiv)
        {
            char* cav = cJSON_Print(cjav);
            char* civ = cJSON_Print(cjiv);

            // no need to set value if they a already the same 
            match = cJSON_Compare(cjiv, cjav);
            if (0)FPS_ERROR_PRINT(" %s >> #######Remap action enable [%s] looking for match cjav \n[%s] cjiv \n[%s] match [%s]\n"
                , __func__
                , enable ? "true":"false"
                , cav
                , civ
                , match?"true":"false"
                );
            free((void*)cav);
            free((void*)civ);
        }
        else
        {
            char* cav = cJSON_Print(cjav);
            match = false;
            if (0)FPS_ERROR_PRINT(" %s >> #######Remap action enable [%s] looking for match cjav \n[%s]  match [%s]\n"
                , __func__
                , enable ? "true":"false"
                , cav
                , match?"true":"false"
                );
            free((void*)cav);
        }

        if (enable && !match)
        {

            assetUri auri(uri, var);
            if(auri.Param == nullptr)
            {
                if (0)FPS_ERROR_PRINT(" %s >> ####### Setting Value Uri [%s] Var [%s] Param [%s] \n", __func__, auri.Uri , auri.Var, auri.Param);
                setValfromCj(vmap, uri, var, cjav);
            }
            else
            {
                {
                    char* cav = cJSON_Print(cjav);
                    //match = false;
                    if (0)FPS_ERROR_PRINT(" %s >> #######Remap action on Param , enable [%s]  cjav >>%s<<  match [%s]\n"
                        , __func__
                        , enable ? "true":"false"
                        , cav
                        , match?"true":"false"
                        );
                    free((void*)cav);
                }
                //if (0)FPS_ERROR_PRINT(" %s >> ####### Setting Param Uri [%s] Var [%s] Param [%s] \n", __func__, auri.Uri , auri.Var, auri.Param);
                setParamfromCj(vmap, auri.Uri, auri.Var, auri.Param, cjav);
            }
            
            if (1)
            {
                // TODO use this to set a FIMS message
                char* scj = cJSON_PrintUnformatted(cjav);
                char* stmp;
                asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"var\":\"%s\",%s}"
                    , uri, var, scj);
                if (0)FPS_ERROR_PRINT(" %s >> ####### Param [%s] Remap action uri [%s] var [%s] [%s]\n", __func__, auri.Param, uri, var ? var : "noVar", stmp);
                free((void*)stmp);
                free((void*)scj);
            }
            if(cjav)cJSON_Delete(cjav);
            if(cjiv)cJSON_Delete(cjiv);
            //delete auri;
        }
    }
    if (0)FPS_ERROR_PRINT(" %s >> #######Remap action completed av [%s] \n", __func__, av->name.c_str());

    free((void*)strval);
    return av;
}


assetVar* VarMapUtils::setActVecfromCj(varsmap& vmap, assetVar* av)//,  cJSON *cj)
{
    //the value has already been set
    // now run the actions
    if(!av->extras)
    {
        av->extras = new assetExtras;
    }
    auto aa = av->extras->actVec["onSet"];
    for (auto x : aa)
    {
        runActValfromCj(vmap, av, x);
    }
    return av;
}

// handles the complexity of value = as well as naked ? sets
assetVar* VarMapUtils::setActBitMapfromCj(assetVar* av, assetAction* aact, cJSON* cjbf, cJSON* cj)
{
    cJSON* cji;
    //cJSON* cjbfm = cJSON_GetObjectItem(cjbf, "bitmap");

    if (cJSON_IsArray(cjbf))
    {
        cJSON_ArrayForEach(cji, cjbf)
        {
            // cJSON* cjmask = cJSON_GetObjectItem(cji, "mask");
            // cJSON* cjbit = cJSON_GetObjectItem(cji, "bit");
            // cJSON* cjuri = cJSON_GetObjectItem(cji, "uri");
            // cJSON* cjvar = cJSON_GetObjectItem(cji, "var");
            // cJSON* cjval = cJSON_GetObjectItem(cji, "bvalue");
            // assetBitField* abf = nullptr;
            if (0)
            {
                char* stmp = cJSON_PrintUnformatted(cji);
                FPS_ERROR_PRINT(" %s >>Whole Feat       >>%s<< child %p \n", __func__, stmp, (void*)cji->child);
                free((void*)stmp);
            }
            aact->addBitField(cji);
        }
    }
    return av;
}

// handles the complexity of value = as well as naked ? sets
// assetVar* setActMapfromCj(assetVar* av, const char* act, const char* opt, cJSON* cjbf, cJSON* cj)
// {
//     if (0)FPS_ERROR_PRINT(" %s >>  act [%s]  btype [%s]\n", __func__, act, opt);

//     assetAction* aact;
//     //Sets up the actMap dict for on set actions
//     // we need a vector under this  
//     // new assetVec
//     // assetVec push_back assetAction
//     if (!av->actMap[act])
//     {
//         if (0)FPS_ERROR_PRINT("%s >>  setting new actMap entry for act [%s] opt [%s]  \n"
//             , __func__
//             , act
//             , opt
//         );

//     }
//     aact = av->actMap[act] = new assetAction(opt);
//     //cJSON *cji;
//     //setActMapfromCj >>  setting act [onSet] opt [limits]   av->actMap size 1
//     if (0)FPS_ERROR_PRINT("%s >>  setting act [%s] opt [%s]   av->actMap size %d\n"
//         , __func__
//         , act
//         , opt
//         , (int)av->actMap.size()
//     );
//     av = setActBitMapfromCj(av, aact, cjbf, cj);
//     // av->actMap[act]>push_back(aact);

//     return av;
// }
// // handles the complexity of value = as well as naked ? sets
assetVar* VarMapUtils::setActVecfromCj(assetVar* av, const char* act, const char* opt, cJSON* cjbf, cJSON* cj)
{
    if (0)FPS_ERROR_PRINT(" %s >>  act [%s]  btype [%s]\n", __func__, act, opt);

    assetAction* aact;
    //Sets up the actMap dict for on set actions
    // we need a vector under this  
    // new assetVec
    // assetVec push_back assetAction
    if(!av->extras)
    {
        av->extras = new assetExtras;
    }
    if (av->extras->actVec.find(act) == av->extras->actVec.end())
    {
        if (0)FPS_ERROR_PRINT("%s >>  setting new actVec entry for act [%s] opt [%s]  \n"
            , __func__
            , act
            , opt
        );
        //av->actVec[act] = std::vector<assetAction *>;
    }
    aact =  new assetAction(opt);
    av->extras->actVec[act].push_back(aact);
    //cJSON *cji;
    //setActMapfromCj >>  setting act [onSet] opt [limits]   av->actMap size 1
    if (0)FPS_ERROR_PRINT("%s >>  setting act [%s] opt [%s]   av->extras->actVec[act] size %d\n"
        , __func__
        , act
        , opt
        , (int)av->extras->actVec[act].size()
    );
    av = setActBitMapfromCj(av, aact, cjbf, cj);
    // av->actMap[act]>push_back(aact);

    return av;
}

// TODO (DONE) remove layer connects to getting the bitmap 
assetVar* VarMapUtils::setActOptsfromCj(assetVar* av, const char* act, const char* opt, cJSON* cj)
{
    cJSON* cjact = cJSON_GetObjectItem(cj, act);
    if (cjact)
    {
        if (0)FPS_ERROR_PRINT("%s >>  act [%s] opt [%s] type %d \n", __func__, act, opt, cjact->type);
        if(cjact->type != cJSON_Array)
        {
            cJSON* cjopt = cJSON_GetObjectItem(cjact, opt);
            if (cjopt)
            {
                //av = setActMapfromCj(av, act, opt, cjopt, cj);
                av = setActVecfromCj(av, act, opt, cjopt, cj);
            }
        }
        else
        {
            cJSON* cji;
            cJSON_ArrayForEach(cji, cjact)
            {
                cJSON* cjopt = cJSON_GetObjectItem(cji, opt);
                if (cjopt)
                {
                    av = setActVecfromCj(av, act, opt, cjopt, cj);
                }
            }
        }
    }
    return av;
}

// sets up the options field for the action
assetVar* VarMapUtils::setActOptsfromCj(assetVar* av, const char* act, cJSON* cj)
{
    if (0)FPS_ERROR_PRINT("%s >>  starting for av->name [%s], act [%s] \n"
            , __func__
            , av->name.c_str()
            , act
            //, opt
            );
    if(!av->extras)
    {
        av->extras = new assetExtras;
    }
    // if we already have actions then delete the stack
    if (av->extras->actVec.find(act) != av->extras->actVec.end())
    {
        if (0)FPS_ERROR_PRINT("%s >>  clearing  actVec entry for act [%s] \n"
            , __func__
            , act
            //, opt
        );
        //delete av->actVec[act] = 
        //std::vector<assetAction *>;
        // this deletes ALL old actions 
        while (av->extras->actVec[act].size() > 0 )
        {
            assetAction* aa = av->extras->actVec[act].back();
            if(aa) delete aa;                //
            av->extras->actVec[act].pop_back();
        }
    }
    // this crashes it but why are we still trying to set an action ...
    //{"name":"Clear Faults",
    //  "value":"Init",
    //  "unit":"",
    //  "scaler":0,
    // "enabled":false,
    // "ui_type":"control",
    // "type":"enum_button",
    //  "actions":{
    //               "onSet":{
    //                     "remap":[{"bit":0,"uri":"/alarms/bms:alarms"}]
    //                }
    //  },
    // "options":[
    //         {"name":"Clear Faults","return_value":"Clear"}
    //         ]
    // }
    //"actions":{"onSet":{"remap":[{"bit":0,"uri":"/alarms/bms:alarms"}]}},   "options":[{"name":"Clear Faults","return_value":"Clear"}]}]
    //"options":[{"name":"Clear Faults","return_value":"Clear"}]
    if (0)FPS_ERROR_PRINT(" %s >>  setting actions for name [%s] act [%s] \n\n", __func__, av->name.c_str(), act);
    cJSON* cji = cj;
    while(cji)
    {
        // hack to fix this
        
        if(0)FPS_ERROR_PRINT("%s >>     >> string [%s] child [%p] child->string[%s] child->child->string[%s] child->child->next %p\n"
        , __func__
        , cji->string
        , cji->child
        , cji->child->string
        , cji->child->child->string
        , cji->child->child->next
        );

        // dont walk past the actions
        if(strcmp(cji->string,"actions")!= 0)
        {
            if (0)FPS_ERROR_PRINT(" %s >>  break setting actions for name [%s] act [%s] \n\n", __func__, av->name.c_str(), act);
            break;
        }

        // NOT Array
        // "maint_mode": {
        // "name": "Maintenance Mode",
        // "value": false,
        // "unit": "",
        // "scaler": 0,
        // "enabled": true,
        // "ui_type": "control",
        // "type": "enum_slider",
        // "actions":{
        //     "onSet":[ <<<< cji->child->string
        //         {"remap":   <<<cji->child->child
        //             [
        //                 {"uri":"/assets/bms/summary:start@enabled"},
        //                 {"uri":"/assets/bms/summary:stop@enabled"},
        //                 {"uri":"/assets/bms/summary:enter_standby@enabled"},
        //                 {"uri":"/assets/bms/summary:exit_standby@enabled"}
        //             ]
        //         }
        //     ]
        // },
        // ARRAY 
        // "@@BMS_ID@@_warning_23": {
        //     "value": 0,
        //     "note":"Sbmu Warning table without degrees reg 3 Appendix 4",
        //     "actions": {
        //         "onSet": {
        //             "enum": [
        //                 { "shift": 0,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:TMS_communications", "outValue": "Normal"},
        //                 { "shift": 0,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:TMS_communications", "outValue": "Fault"},
        //                 { "shift": 2,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:TMS_mode_conflict", "outValue": "Normal"},
        //                 { "shift": 2,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:TMS_mode_conflict", "outValue": "Fault"},
        //                 { "shift": 3,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:temp_sensor_alarm", "outValue": "Normal"},
        //                 { "shift": 3,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:temp_sensor_alarm", "outValue": "Fault"},
        //                 { "shift": 4,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:smoke_sensor_alarm", "outValue": "Normal"},
        //                 { "shift": 4,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:smoke_sensor_alarm", "outValue": "Fault"},
        //                 { "shift": 5,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:aerosol_state", "outValue": "Normal"},
        //                 { "shift": 5,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:aerosol_state", "outValue": "Fault"},
        //                 { "shift": 6,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:aerosol_close", "outValue": "Normal"},
        //                 { "shift": 6,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:aerosol_close", "outValue": "Fault"},
        //                 { "shift": 7,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:aerosol_open", "outValue": "Normal"},
        //                 { "shift": 7,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:aerosol_open", "outValue": "Fault"},
        //                 { "shift": 8,"mask": 1,"inValue": 0,"uri": "/fault/@@BMS_ID@@:rack_door", "outValue": "Normal"},
        //                 { "shift": 8,"mask": 1,"inValue": 1,"uri": "/fault/@@BMS_ID@@:rack_door", "outValue": "Fault"}

        //             ]
        //         }
        //     }
        // }

        // "start_grad_p": {
        //     "value": 10.0,
        //     "actions": {
        //         "onSet": {
        //             "limits": [{"low": 0.1,"high": 3000.0}],
        //             "remap": [{"bit": 0, "uri": "/components/pe_pcs","var": "pcs_start_grad_p"}]
        //         }
        //     }
        // }


        if(strcmp(cji->child->string, act)==0)
        {
            cJSON* cji2 = cji->child->child;
            if(cji2->type == cJSON_Array)
            {
                if(1)FPS_ERROR_PRINT("%s >>     >> >> act [%s] cji2 is an Array  \n"
                        , __func__
                        , act
                );
                cJSON* cjii;
                cJSON_ArrayForEach(cjii, cji2)
                {
                    char* tmp = cJSON_PrintUnformatted(cjii);
                    if (1) FPS_ERROR_PRINT("%s >> ..%s<<\n", __func__, tmp);
                    free((void *)tmp);

                }
                cji2=nullptr;  // We dont handle this yet

            }
            else if(cji2->type == cJSON_Object)
            {
                if(0)FPS_ERROR_PRINT("%s >>     >> >> act [%s] cji2 is an Object child func [%s]\n"
                        , __func__
                        , act
                        , cji2->child->string
                );
                cji2 = cji2->child;

            }

            while(cji2)
            {
                if(0)FPS_ERROR_PRINT("%s >>     >> >> act [%s] cji2 string [%s] child [%p]\n"
                , __func__
                , act
                , cji2->string
                , cji2->child
                );
                setActOptsfromCj(av, act, cji2->string, cj);

                cji2 = cji2->next;
            }
        }

        cji = cji->next;
    }

    if (0)FPS_ERROR_PRINT(" %s >>  done setting actions for name [%s] act [%s] \n\n", __func__, av->name.c_str(), act);

    // setActOptsfromCj(av, act, "bitfield", cj);
    // setActOptsfromCj(av, act, "enum", cj);
    // setActOptsfromCj(av, act, "func", cj);
    // setActOptsfromCj(av, act, "bitset", cj);
    // setActOptsfromCj(av, act, "limits", cj);
    // setActOptsfromCj(av, act, "remap", cj);
    return av;
}

// handles the complexity of value = as well as naked ? sets
// looks for onSet, onGet, onPub
// TODO preserve action order 

assetVar* VarMapUtils::setActfromCj(assetVar* av, cJSON* cj)
{
    setActOptsfromCj(av, "onSet", cj);
    setActOptsfromCj(av, "onGet", cj);
    setActOptsfromCj(av, "onPub", cj);

    return av;
}

// TODO lock vmap , create unlocked option
// handles the complexity of value = as well as naked ? sets
// int uiObject = 0 ; asset type
// uIobject = 1 means uiType
// uiObject = 2 means uIalarm / fault
// 

//assetVar* xxsetOldValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject = 0);
// How do we designate a alarm/fault object  We see ui_type as an alarm in loadmap 


assetVar* VarMapUtils::setParamfromCj(varsmap& vmap, const char* comp, const char* var, const char *param, cJSON* cj, int uiObject)
{
    assetVar* av = getVar(vmap, comp, var);
    if (av)
    {
        // ohboy we have to run setParam lets hope we have a cj options
        av->setParam(param, cj);
    }
    return av;

}
// TODO use assetURI --> DONE
assetVar* VarMapUtils::getVar(varsmap& vmap, const char* comp, const char* var)
{
    assetUri my(comp, var);
    if(0) FPS_ERROR_PRINT("%s >> getVar   comp [%s] var [%s] my.Uri [%s] my.Var[%s] my.Param [%s\n"
            , __func__
            , comp
            , var
            , my.Uri
            , my.Var
            , my.Param
    );

    assetVar* av = nullptr;

    if (0)FPS_ERROR_PRINT("%s looking for comp [%s] var [%s]\n"
        , __func__
        , my.Uri
        , my.Var);
    if (vmap.size() > 0)
    {
        auto ic = vmap.find(my.Uri);
        if (ic == vmap.end())
        {
            if (0)FPS_ERROR_PRINT("%s NOTE created comp [%s] var [%s]\n"
                , __func__
                , my.Uri
                , my.Var);

        }
        if (ic != vmap.end())
        {
            if (0)FPS_ERROR_PRINT("%s found comp [%s] looking for var [%s] size %d\n"
                , __func__
                , my.Uri
                , my.Var
                , (int)vmap[my.Uri].size()
            );
            // for (auto it : vmap[mycomp])
            // {
            //     if(0)FPS_ERROR_PRINT("%s found comp [%s] looking for var [%s] found [%s]\n"
            //         , __func__
            //         , mycomp
            //         , myvar
            //         ,it.first.c_str()
            //         );
            //     if(strcmp(myvar,it.first.c_str()) == 0)
            //     {
            //         av = vmap[mycomp][myvar];
            //         break;
            //     }

            // }  
            if (my.Var)
            {
                auto iv = vmap[my.Uri].find(my.Var);
                if (iv != vmap[my.Uri].end())
                {
                    av = vmap[my.Uri][my.Var];
                }
            }
        }
    }
    return av;

}

// TODO test comp /a/b:c  ok working
template <class T>
T VarMapUtils::getVar(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetVar* av = getVar(vmap, comp, var);
    if (av)
    {
        return av->getVal(value);
    }
    return value;
}

// rework 
// given a list or URI's populate an incoming cj with the data  adding individual uirs if needed  
// options 0x0100 will produce a segmented list
//   for example inuri /assets   
//               uri   /assets/bms/summary
// will produce {"bms":{"summary":{<data>}}}
//
int VarMapUtils::baseVec(std::string& bs, std::vector<std::string>& buri, std::vector<std::string>& turi)
{
    int rc = 0;
    if (0) FPS_ERROR_PRINT(" %s >> sizes buri [%d] turi [%d] \n"
        , __func__
        , (int)buri.size()
        , (int)turi.size()
    );
    while ((rc < (int)buri.size()) && (rc < (int)turi.size()))
    {
        if (buri[rc] != turi[rc])
        {
            if (0) FPS_ERROR_PRINT(" %s >> uri [%s] furi [%s] break rc %d\n"
                , __func__
                , buri[rc].c_str()
                , turi[rc].c_str()
                , rc);
            //rc = 0;  // flag difference
            break;
        }
        bs += "/";
        bs += buri[rc];
        rc++;
    }
    if (( rc==(int)buri.size()) &&
        ( rc ==(int)turi.size())&& (rc > 0))
    {
        if (0) FPS_ERROR_PRINT(" %s >> uri [%s] furi [%s] identical  rc %d\n"
            , __func__
            , buri[rc-1].c_str()
            , turi[rc-1].c_str()
            , rc);
        rc = 0; // flag identical
    }

    return rc;
}

int VarMapUtils::uriSplit(std::vector<std::string>& uriVec, const char* _uri)
{
    int nfrags = 0;
    std::string uri = _uri;
    std::string key = "/";

    std::size_t startf;
    std::size_t endf = 0;
    do
    {
        if (endf == 0)
        {
            startf = 1;
        }
        else
        {
            std::string furi = uri.substr(startf, (endf - startf));
            if (0) FPS_ERROR_PRINT(" %s >> uri [%s] furi [%s]\n", __func__, _uri, furi.c_str());
            startf = endf + 1;
            uriVec.push_back(furi);
            nfrags++;
        }

    } while ((endf = uri.find(key, startf)) != std::string::npos);
    std::string furi = uri.substr(startf, (endf - startf));
    uriVec.push_back(furi);

    if (0) FPS_ERROR_PRINT(" %s >> last >> uri [%s] furi [%s]\n", __func__, _uri, furi.c_str());
    return nfrags;
}

cJSON* VarMapUtils::createUriListCj(varsmap& vmap, std::string& bs, const char* inuri, cJSON* incj, int options, std::vector<std::string>& uriVec)
{
    // split uri up into strings
    std::vector<std::string> inVec;
    if (incj == nullptr)
    {
        incj = cJSON_CreateObject();
    }
    //int infrags = 
    uriSplit(inVec, inuri);   //  inVec /status/bms   uriVec /status/bms_1,2,3,4   etc
    for (auto& x : uriVec)
    {
        const char* myuri = x.c_str();
        if (0) FPS_ERROR_PRINT(" %s >> uri [%s] uriVec [%s] opts 0x%04x\n", __func__, inuri, myuri, options);
        assetList* alist = getAlist(vmap, myuri);

        if (0)FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj map for [%s] assetList %p  cj %p\n"
            , __func__
            , myuri
            , alist
            , (void*)incj
            );
        // we only need do this for options 0x0100
        if(!(options & 0x0100))
        {
            cJSON *cjii = cJSON_CreateObject();

            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    av = alist->avAt(ix++);
                    if (av) av->showvarCJ(cjii, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                    y.second->showvarCJ(cjii, options);
                    if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                    //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                }
            }
            cJSON_AddItemToObject(incj, myuri,cjii);
            continue;
        }
        std::vector<std::string> uVec;
        //int infrags = 
        uriSplit(uVec, x.c_str());
        std::string bsx;
        int bvec = baseVec(bsx, inVec, uVec);
        bs = bsx;
        if (0) FPS_ERROR_PRINT(" %s >> inuri [%s] uriVec [%s] opts 0x%04x bvec %d bs[%s]\n", __func__, inuri, x.c_str()
                , options, bvec, bs.c_str());
        cJSON* cji = incj;
        cJSON* cjii = nullptr;
        // we had a match but got more than one answer
        // need to step back and show the kids
        if(bvec == 0)
        {
            if(uVec.size()== 1)
            {
                cjii = cji;
            }
            else
            {
                cjii = cJSON_CreateObject();
            }

            // now run getMapsCj on x.c_str() into cjii
            if (0) FPS_ERROR_PRINT(" %s >> running getMapsCj [%s] into base node alist %p cji->string [%s]\n"
                    , __func__, x.c_str(), (void *)alist
                    , cji->string?cjii->string:"noname"
                    );
            //cjii = cJSON_CreateObject();

            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    av = alist->avAt(ix++);
                    if (av) av->showvarCJ(cjii, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                    y.second->showvarCJ(cjii, options);
                    if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                    //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                }
            }

            if(uVec.size() > 1)
            {
//                   cjii = cji;
                cJSON_AddItemToObject(cji,uVec[uVec.size()-1].c_str(),cjii);

            }
            
            


        }
        else if (bvec < (int)uVec.size())
        {
            if (0) FPS_ERROR_PRINT(" %s >> bvec small %d need to find / create trees\n", __func__, bvec);
            while (bvec < (int)uVec.size())
            {
                if (0) FPS_ERROR_PRINT(" %s >> bvec small %d  find / create tree [%s] inuri [%s] \n"
                        , __func__, bvec, uVec[bvec].c_str(), inuri);
                cjii = cJSON_GetObjectItem(cji, uVec[bvec].c_str());
                if (!cjii)
                {
                    if (0) FPS_ERROR_PRINT(" %s >> bvec small %d  create tree [%s] inuri [%s] \n"
                        , __func__, bvec, uVec[bvec].c_str(), inuri);

                    cjii = cJSON_CreateObject();
                    cJSON_AddItemToObject(cji, uVec[bvec].c_str(), cjii);
                }
                // build tree
                cji = cjii;
                bvec++;
            }
            // now run getMapsCj on x.c_str() into cjii
            if (0) FPS_ERROR_PRINT(" %s >> running getMapsCj [%s ] into end node\n", __func__, x.c_str());
            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    av = alist->avAt(ix++);
                    if (av) av->showvarCJ(cjii, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                    y.second->showvarCJ(cjii, options);
                    if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                    //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                }
            }
        }
        else
        {
            // now run getMapsCj on x.c_str() into cjii
            if (0) FPS_ERROR_PRINT(" %s >> running getMapsCj [%s] into end node alist %p\n"
                    , __func__, x.c_str(), (void *)alist);
            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    av = alist->avAt(ix++);
                    if (av) av->showvarCJ(cji, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                    y.second->showvarCJ(cji, options);
                    if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                    //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                }
            }
        }
    }
    if(0)
    {
        char* tmp = cJSON_Print(incj);
        if(tmp)
        {
            FPS_ERROR_PRINT("%s >> incj at end \n>> %s<<\n", __func__, tmp);
            free((void*)tmp);
        }
    }
    return incj;
}

// 1/ create a list of stuff in varsmap
// 2/ put items in sysvec order ( if you have a sysVec)
// 3/ find  matches to the uri
//std::vector<std::string> nVec;
int VarMapUtils::createAssetListCj(varsmap& vmap, const char* uri, std::vector<std::string>* sysVec, int opts, std::vector<std::string>& nVec)
{
    std::vector<std::string> xVec;
    std::vector<std::string> yVec;
    std::vector<std::string>* yVecp;
    std::vector<std::string> zVec;

    std::string yuri = uri;
    std::size_t found = yuri.find("/",1);
    std::string ystart = uri;//"/"+ yuri.substr(0,found+1);

    for (auto& x : vmap)
    {
        if (process_fims_debug) FPS_ERROR_PRINT(" %s >> xVec push_back [%s]\n", __func__, x.first.c_str());
        xVec.push_back(x.first);
    }
    if (0) FPS_ERROR_PRINT(" %s >> xVec size [%d] uri [%s] \n", __func__, (int)xVec.size(), uri);
    yVecp = &xVec;

    // now extract it from sysVec if we have one and we are running a uri quesy
    if (sysVec && (opts&0x0100))
    {
        for (auto& y : *sysVec)
        {
            // y == whole string
            // yend == last /thing
            found = y.find_last_of("/");
            //std::cout << " path: " << str.substr(0,found) << '\n';
            std::string yend = y.substr(found + 1);

            if (yend == "summary")
                yend = y;

            if (0) FPS_ERROR_PRINT(" %s >> sysvec y [%s] ystart [%s] yend [%s]\n"
                    , __func__
                    , y.c_str()
                    , ystart.c_str()
                    , yend.c_str()
                    );
            for (auto z : xVec)
            {
                if (0) FPS_ERROR_PRINT(" %s >> sysvec y [%s] z [%s]\n", __func__, y.c_str(), z.c_str());

                if ((y == z) || (z.find(yend) != std::string::npos))
                {
                    //if(z[0]!='_')
                    {
                        if (0) FPS_ERROR_PRINT(" %s >> >> >> yVec push_back [%s]\n", __func__, z.c_str());
                        yVec.push_back(z);

                    }
                }
            }
            //nVec.push_back(x.first);
        }
        yVecp = &yVec;

        if (0) FPS_ERROR_PRINT(" %s >> yVec size [%d]\n", __func__, (int)yVec.size());
    }
    for (auto& y : *yVecp)
    {
        std::string suri = uri;
        auto x = y.find(suri);
        if (0) FPS_ERROR_PRINT(" %s >> >> >> suri [%s] yVec option [%s] x %d\n"
                , __func__, suri.c_str(), y.c_str(), (int)x);
        if (x == 0)
        {
            if (0) FPS_ERROR_PRINT(" %s >> >> >> >>nVec push_back [%s] suri [%s]\n", __func__, y.c_str(), suri.c_str());
            nVec.push_back(y);
        }
    }
    int rc = (int)nVec.size();
    if (0) FPS_ERROR_PRINT(" %s >> nVec size [%d]\n", __func__, rc);
    return rc;
}

// How do we designate a alarm/fault object  We see ui_type as an alarm in loadmap 
asset_log* assetVar::sendAlarm(assetVar* destAv, const char* atype, const char* msg, int severity)
{
    asset_log* avAlarm;
    // if (alarmMaps[atype] != nullptr)
    // {
    //     avAlarm = alarmMaps[atype];
    //     destAv->clearAlarm(avAlarm);
    //     //delete avAlarm;
    // }
    avAlarm = new asset_log((void*)this, (void*)destAv, atype, msg, severity);
    avAlarm->aVal = new assetVal;
    if(linkVar)
    {
        avAlarm->log_time = 0.0;
        if (linkVar->aVal)
        {
            *((assetVal*)avAlarm->aVal) = *linkVar->aVal;
            avAlarm->log_time = linkVar->aVal->setTime;
        }

    }
    else
    {
        if (aVal)
            *((assetVal*)avAlarm->aVal) = *aVal;
        avAlarm->log_time = aVal ? aVal->setTime : 0.0;
    }
    avAlarm->name = name;
    avAlarm->comp = comp;

    //alarmMaps[atype] = avAlarm;
    
    if (0)FPS_ERROR_PRINT("%s >> calling setAlarm in destAv [%s:%s]\n",__func__, destAv->comp.c_str(), destAv->name.c_str());
    destAv->setAlarm(avAlarm);
    return avAlarm;
}

//uiObject 
// if its name look for name= no objet
// all other items have objects "active_thing":{or they are params
// 
// How do we designate a alarm/fault object  We see ui_type as an alarm in loadmap 
// does NOT correctly decode a bool parm 
assetVar* VarMapUtils::setValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject)
{
    assetList*alist = nullptr;
    //bool newAv = false;

    setvar_debug = 0;
    if ((setvar_debug) || !var)
    {
        if (0 || setvar_debug)
        {
            char* tcj = nullptr;
            if (cj) tcj = cJSON_PrintUnformatted(cj);
            FPS_ERROR_PRINT("%s  >> >>Seeking Variable comp 1 [%s] var [%s] cj is %p [%s]\n"
                , __func__
                , comp
                , var
                , (void*)cj
                , cj ? tcj : "NoCj");
            if (tcj) free((void*)tcj);
        }
    }
    assetUri my(comp, var);
    //auri.Uri;

    // see if we have an alist
    alist = getAlist(vmap, my.Uri);

    if(0) FPS_ERROR_PRINT("%s  >> >>Seeking Variable comp 2 [%s] var [%s]  param [%s] uiObject %d alist %p\n"
                , __func__
                , my.Uri
                , my.Var?my.Var:" no Var"
                , my.Param?my.Param:" no Param"
                , uiObject
                , alist
                );

    // //Now process the list of objects.

    assetVar *av = nullptr;
    cJSON* cjval = cJSON_GetObjectItem(cj, "value");
    cJSON* cjname = cJSON_GetObjectItem(cj, "name");
    if(cjname && !cjval)
    {
        uiObject = 1;
    }
    if ((uiObject > 0) || setvar_debug)FPS_ERROR_PRINT("%s  >> >>Suspected UI Object  comp [%s] my.Uri [%s] uiObject %d\n"
            , __func__
            , comp
            , my.Uri
            , uiObject
            );

    if (uiObject > 0)
    {
         alist = getAlist(vmap, my.Uri);
         // //Now process the list of objects.

        if (!alist)
        {
            //newAv = true;
            alist = setAlist(vmap, my.Uri);

            FPS_ERROR_PRINT("%s >>  new assetList for [%s] \n"
                        , __func__
                        , my.Uri
                        );
        }
    }
//     if (alist && av)
//     {
//         alist->add(av);
//         av = nullptr;
//     }
    // unsigned int ix = aList.size();
    // for (unsigned int i = 0; i < ix; i++)
    // {
    //     if (aList[i] == av)
    //         return;
    // }
    // just a URI , process the list of assetsVars
    if((my.Var == nullptr) && (cjval == nullptr) && (cjname == nullptr))
    {
        cJSON*cj3 = cj;
        if (cj->child)
            cj3 = cj3->child;
        while(cj3)
        {
            if(1)FPS_ERROR_PRINT("%s >>  new assetVar ##1  [%s:%s] uiObject %d \n"
                        , __func__
                        , my.Uri
                        , cj3->string
                        , uiObject
                        );

            av = setValfromCj(vmap, my.Uri, cj3->string, cj3, uiObject);
            if (alist && av)
            {
                alist->add(av);
                 av = nullptr;
             }

            cj3 = cj3->next;
        }
        return av;
    }
    // try it this way 
    // no my.Var means no av which is used later to set value
    // we can search for an existing av anyway if not we'll have to make it
    // but we still have to solve for my.Var being nullptr
    if(my.Var)
    {
        av = getVar(vmap, my.Uri, my.Var);
    }
    if (av)
    {
        if (0 || setvar_debug)FPS_ERROR_PRINT("%s >> >>Careful here Setting Old Vari comp [%s] var [%s] param [%s] cj is %p  extras [%p] cjisObject [%s]\n"
            , __func__
            , my.Uri
            , my.Var
            , my.Param?my.Param:"no Param"
            , (void*)cj
            , av->extras
            , cJSON_IsObject(cj)?"true":"false"
            );
        // this does not setup detiled components .. but note we may want to update params.
        if(my.Param)
        {
            setParamfromCj(vmap, my.Uri, my.Var, my.Param, cj);
            return av;
        }
    }
    else
    {
        if (0 || setvar_debug)FPS_ERROR_PRINT("%s >>  >>Setting New Variable comp [%s] var [%s] Param [%s] cj is %p \n"
            , __func__
            , my.Uri
            , my.Var ? my.Var:"No Var"
            , my.Param ? my.Param:"no Param"
            , (void*)cj);
    }

    // we have to add a new variable perhaps we need special flags to enable this

    // this sections adds a new variable, looks for a single value
    //if(!av)
    //{
    // examine what we have 
    if (cJSON_IsObject(cj))
    {
        cJSON* cjval = cJSON_GetObjectItem(cj, "value");
        cJSON* cjname = cJSON_GetObjectItem(cj, "name");
        cJSON* cjui_type = cJSON_GetObjectItem(cj, "ui_type");
        bool ui_control = false;
        bool ui_alarm = false;
        bool ui_status = false;
        const char* ui_type = "None";

        if (cjui_type)
        {
            if (strcmp(cjui_type->valuestring, "control") == 0)
            {
                ui_control = true;
                ui_type="control";
            }
            if (strcmp(cjui_type->valuestring, "alarm") == 0)
            {
                ui_alarm = true;
                ui_type="alarm";
            }
            if (strcmp(cjui_type->valuestring, "status") == 0)
            {
                ui_status = true;
                ui_type="status";
            }
        }
        if(av == nullptr)
        {
            if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >> Adding a new Variable from OBJECT comp [%s] var [%s] cj is %p cjval is %p cjname is %p cjui_type is %p\n"
                , __func__
                , my.Uri
                , my.Var? my.Var:"No Var"
                , (void*)cj
                , (void*)cjval ? cjval : nullptr
                , (void*)cjname ? cjname : nullptr
                , (void*)cjui_type ? cjui_type : nullptr
                );
        }
        else
        {
            if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >> Using existing Variable from OBJECT comp [%s] var [%s] cj is %p cjval is %p cjname is %p cjui_type is %p\n"
                , __func__
                , my.Uri
                , my.Var? my.Var:"No Var"
                , (void*)cj
                , (void*)cjval ? cjval : nullptr
                , (void*)cjname ? cjname : nullptr
                , (void*)cjui_type ? cjui_type : nullptr
                );

        }
        //av = nullptr;
        // HMM ui_controls do NOT have a value if they are boolean ....
        // So we have to create one to receive the incoming data
        // UI Alarms we have to show the options vec as alarms.
        // test for no value in cj
        // we did not have a value but we still may have params .. its just like that !!!
        if (!cjval)
        {
            //bool vv = cJSON_IsTrue(cjval);  ??
            if (!av && my.Var)
            {
                bool vv = false;
                av = makeVar(vmap, my.Uri, my.Var, vv);
                av->ui_type = ui_control ? 1 : ui_alarm ?2:ui_status?3:0;
                if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >>Adding a new ui control [%s]  OBJECT Uri [%s] var [%s]\n"
                    , __func__
                    , ui_type 
                    , my.Uri
                    , my.Var?my.Var:"No Var"
                    );
                if (alist && av)
                {
                    alist->add(av);
                    //av = nullptr;
                }
            }
            // this is a special for the ui name  cjname->string is used instead of my.Var
            else if (!av && cjname)
            {
                av = setValfromCj(vmap, my.Uri, cjname->string, cjname, uiObject);
                if(av)av->setNaked = true;
                if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >>Adding name  ui  [%s]  OBJECT Uri [%s] var [%s] val [%s] \n"
                        , __func__
                        , ui_type
                        , my.Uri
                        , cjname->string
                        , cjname->valuestring
                        );
                // this adds the name in at the top
                if (alist && av)
                {
                    alist->add(av);
                    //av = nullptr;
                }
                cJSON*cji = cjname->next;
                while(cji)
                {
                    av = setValfromCj(vmap, my.Uri, cji->string, cji, uiObject);
                    if(1)FPS_ERROR_PRINT(" %s >> added uiObject [%s] \n", __func__, cji->string);
                    cji = cji->next;
                }
            }
            // debug show alist
            if(alist)
            {
                unsigned int ix = alist->size();
                for (unsigned int i = 0; i < ix; i++)
                {
                    if (alist->avAt(i))
                    {
                        if(1)FPS_ERROR_PRINT("  %s >>  alist item [%d] [%s] \n", __func__, i, alist->avAt(i)->name.c_str());
                    }
                }
            }
        }
        // did we find a value ??
        // if so, we have to create the right kind of av
        // perhaps we need makeVar(vmap, uri,var,cj);
        if (cjval)
        {
            // Did not work 
            // debug later
            // if (!av)
            // {
            //     av = makeVar(vmap, my.Uri, my.Var, cjval);
            // }
            // if(av)
            // {
            //     av->setVal(cjval);
            // }

            if (cJSON_IsNumber(cjval))
            {
                double vv = cjval->valuedouble;
                av = setVal(vmap, my.Uri, my.Var, vv);
                //if(!av)
                //    av = makeVar(vmap, my.Uri, my.Var, vv);
                //av->setVal(vv);
            }
            else if (cJSON_IsBool(cjval))
            {
                bool vv = cJSON_IsTrue(cjval);
                av = setVal(vmap, my.Uri, my.Var, vv);
                // if(!av)
                //     av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            else if (cJSON_IsString(cjval))
            {
                const char* vv = cjval->valuestring;
                av = setVal(vmap, my.Uri, my.Var, vv);
                // if(!av)
                //     av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            // add it to any running alist
            if (alist && av)
            {
                alist->add(av);
                //av = nullptr;
            }
        }

        // right if we had a name but No Value or uiObject is set then we must add params to the current av.
        // no need to keep order for params ....

        // else
        // {
        //     // may need to for this for all base vars
        //     if(/*uiOject &&*/cjname && !cjval)
        //     {
        //         if (cJSON_IsString(cjname))
        //         {
        //             const char *vv = cjname->valuestring;
        //             av = makeVar(vmap, mycomp, cjname->string, vv);
        //             av->setVal(vv);
        //             if(uiObject)av->setNaked = true;

        //             if(0||setvar_debug)FPS_ERROR_PRINT(" %s >> Adding a new  OBJECT comp [%s] varname [%s] vv [%s] \n"
        //                 , __func__
        //                 , mycomp
        //                 , cjname->string
        //                 , vv
        //             );

        //         } 
        //     }
        // }

        // now look for these dudes
        cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
        cJSON* cjparam = cJSON_GetObjectItem(cj, "params");
        cJSON* cjopts = cJSON_GetObjectItem(cj, "options");
        //cJSON* 
        cjname = cJSON_GetObjectItem(cj, "name");

        if (setvar_debug)FPS_ERROR_PRINT(" %s >>Adding a new Variable from OBJECT actions [%p] params[%p] options[%p]]\n"
            , __func__
            , (void*)cjact
            , (void*)cjparam
            , (void*)cjopts
        );
        // what about setfunc
        if (cjact)
        {
            if (setvar_debug)FPS_ERROR_PRINT("%s >> >>  new Adding actions starting for [%s] av [%p]\n", __func__, var, (void*)av);
            if (av)
            {
                // this will delete old actions as well
                setActfromCj(av, cjact);  // note this must delete any old actions
            }
            if (setvar_debug)FPS_ERROR_PRINT("%s >> >>  new Adding actions done for [%s] av [%p]\n", __func__, var, (void*)av);

        }
        // may need to add these to basedict
        if (cjparam)
        {
            if (0)FPS_ERROR_PRINT("%s >>Adding params for [%s] av [%p]\n", __func__, var, (void*)av);
            if (av)
            {
                if(!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (!av->extras->featDict)
                {
                    av->extras->featDict = new assetFeatDict;
                }
                av->extras->featDict->addCj(cjparam);
            }
        }
//        if (ui_control || ui_alarm || (uiObject > 0))  // TODO we may do this for any object now !!!
        {
            if (0)FPS_ERROR_PRINT("%s >>Adding base params starting for [%s] av [%p]\n", __func__, var, (void*)av);
            if (av)
            {
                if (!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (!av->extras->baseDict)
                {
                    av->extras->baseDict = new assetFeatDict;
                }
                // do not add value as a baseparam
                av->extras->baseDict->addCj(cj, 1/*uiObject*/, false /*skip name */);

            }
            if (0)FPS_ERROR_PRINT("%s >>Adding base params done for [%s] av [%p]\n", __func__, var, (void*)av);

        }
        if (cjopts)
        {
            if (0)FPS_ERROR_PRINT("%s >>Adding options starting for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);
            // it may be an array
            // in any case addCj should handle it.
            if (av)
            {
                if (ui_control)
                    av->ui_type = 1;
                if (ui_alarm)
                    av->ui_type = 2;
                if (ui_status)
                    av->ui_type = 3;

                if(!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (cjopts->type == cJSON_Array)
                {
                    if (0)FPS_ERROR_PRINT("%s >>Adding options Vec for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);
                    if (!av->extras->optVec)
                    {
                        if (0)FPS_ERROR_PRINT("%s >>New Options Vec  for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);
                        av->extras->optVec = new assetOptVec;
                    }
                    av->extras->optVec->addCj(cjopts);
                }
                else
                {
                    if (!av->extras->optDict)
                    {
                        av->extras->optDict = new assetFeatDict;
                    }
                    av->extras->optDict->addCj(cjopts);
                }
            }
            if (0)FPS_ERROR_PRINT("%s >>Adding options done for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);

        }
        return av;
    }
    // NOT an object   but we still may need to trigger actions
    // will setVal do that o we have to use varmaputils::setval
    //assetVar* VarMapUtils::setValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject)
    //assetVar* VarMapUtils::setVal(varsmap& vmap, const char* comp, const char* var, T& value)

    //
    if (0 || setvar_debug)FPS_ERROR_PRINT("%s  >>NOT an OBJECT Var comp [%s] var [%s] cj is %p type %d\n"
        , __func__
        , my.Uri
        , my.Var? my.Var:"No Var"
        , (void*)cj
        , cj?cj->type:-1
        );
    // again we need makeVar(vmap,uri,var,cj)

    if (cJSON_IsNumber(cj))
    {
        double vv = cj->valuedouble;
        // if(!av)
        //     av = makeVar(vmap, my.Uri, my.Var, vv);
        av = setVal(vmap, my.Uri, my.Var, vv);
    }
    else if (cJSON_IsBool(cj))
    {
        bool vv = cJSON_IsTrue(cj);
        av = setVal(vmap, my.Uri, my.Var, vv);
    }
    else if (cJSON_IsString(cj))
    {
        const char* vv = cj->valuestring;
        av = setVal(vmap, my.Uri, my.Var, vv);
    }
    else
    {
        // can we ever get here ????
        // Yes if we have an array to deal with
        cJSON* cjval = cJSON_GetObjectItem(cj, "value");
        cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
        cJSON* cjparam = cJSON_GetObjectItem(cj, "params");
        cJSON* cjopts = cJSON_GetObjectItem(cj, "options");

        //cJSON_DetachItemFromObject(cj,"value");
        //cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
        // TODO add more options here like deadband
        // test turn it off
        //cjact = nullptr;    
        //if (setvar_debug)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (0|| setvar_debug) FPS_ERROR_PRINT("%s >> NOT AN OBJECT  setValfrom_Cj >>Adding NEW cj >>%s<<\n cjval %p \n cjact %p\n"
                , __func__
                , tmp
                , (void*)cjval
                , (void*)cjact
            );
            free((void*)tmp);
        }
        assetVar* av = nullptr;
        /// 
        if (cJSON_IsArray(cj))
        {
            int asize = cJSON_GetArraySize(cj);
            if(0)FPS_ERROR_PRINT(" setValfrom_Cj >>Setval for cj  array size %d\n"
                , asize
                //, (void *)cjact                        
            );

            if (asize == 1)
            {
                //"ems_cmd":[{"value":0,"string":"Initial"}]
                cJSON* cji = cJSON_GetArrayItem(cj, 0);
                if (cji)
                    cjval = cJSON_GetObjectItem(cji, "string");
            }
        }
        // This is called when we have an array not an object.
        if (cjval && my.Var)
        {
            if (0)FPS_ERROR_PRINT("%s  >>FUNNY STUFF Adding CJ value for [%s]\n", __func__, var);
            av = setValfromCj(vmap, my.Uri, my.Var, cjval, uiObject);
            //cJSON_Delete(cjval);
        }
        if (cjact)
        {
            if (1)FPS_ERROR_PRINT("%s  >>FUNNY STUFF Adding actions for [%s] av [%p]\n", __func__, var, (void*)av);
            if (av)
            {
                setActfromCj(av, cjact);
            }
        }

        if (cjparam)
        {
            if (1)FPS_ERROR_PRINT("%s >>FUNNY STUFF Adding params for [%s] av [%p]\n", __func__, var, (void*)av);
            if (av)
            {
                if(!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (!av->extras->featDict)
                {
                    av->extras->featDict = new assetFeatDict;
                }
                av->extras->featDict->addCj(cjparam);
            }
        }
        if (cjopts)
        {
            if (1)FPS_ERROR_PRINT("%s >> FUNNY STUFF Adding options for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);
            // it may be an array
            // in any case addCj should handle it.
            if (av)
            {

                if(!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (cjopts->type == cJSON_Array)
                {
                    if (1)FPS_ERROR_PRINT("%s >>FUNNY STUFF Adding options for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);

                    if (!av->extras->optVec)
                    {
                        if (1)FPS_ERROR_PRINT("%s  >> FUNNY STUFF Adding New options for [%s] av [%p] cjopts type %d\n", __func__, var, (void*)av, cjopts->type);
                        av->extras->optVec = new assetOptVec;
                    }
                    av->extras->optVec->addCj(cjopts);
                }
                else
                {
                    if (!av->extras->optDict)
                    {
                        av->extras->optDict = new assetFeatDict;
                    }
                    av->extras->optDict->addCj(cjopts);
                }
            }
        }
        // NOW we need to deal with actions ...

    }


    //if (mycomp != comp)free((void*)mycomp);
    return av;
}// How do we designate a alarm/fault object  We see ui_type as an alarm in loadmap 

int VarMapUtils::vListPartialSendFims(varsmap& vmap, std::vector<std::string>& vx, const char* method, fims* p_fims, const char* uri)
{
    char* msg;
    cJSON* cj = nullptr;

    //if(0)FPS_ERROR_PRINT("%s >> pmap size %d \n", __func__,  (int)vecs.size());
    for (auto ix : vx)
    {
        char* newuri = (char*)ix.c_str();
        //cj = cJSON_CreateObject();
        int found = 0;
        int opts = 0;
        if (method)
        {
            if (strcmp(method, "pub") == 0) // force pub opts
                opts = 0;//0x0100;
        }

        //cj = loadAssetList(vmap, newuri, found, opts);
        if (found == 0)
        {
            if(cj)cJSON_Delete(cj);
            cj = getVmapCJ(vmap[newuri]);
            if (0)FPS_ERROR_PRINT("%s >> pmap ix.first [%s] cj [%p] opts 0x%04x\n", __func__, ix.c_str(), cj, opts);
        }

        //cJSON *cj = getVmapCJ(vmap[ix.c_str()]);
        if (0)FPS_ERROR_PRINT("%s >> pmap ix.first [%s] cj [%p] \n", __func__, ix.c_str(), cj);
        if (cj->child)
            msg = cJSON_PrintUnformatted(cj->child);
        else
            msg = cJSON_PrintUnformatted(cj);

        cJSON_Delete(cj);
        if (0)FPS_ERROR_PRINT("%s >> pmap ix.first [%s] msg [%s] \n", __func__, ix.c_str(), msg);
        if (msg)
        {
            if (strcmp(method, "get") == 0) // need a uri for a get
            {
                if (uri && strlen(uri) > 0)
                {
                    p_fims->Send(method, uri, ix.c_str(), msg);
                }
            }
            else
            {
                const char* dest = ix.c_str();
                if (dest && strlen(dest) > 0)
                {
                    if (0)FPS_ERROR_PRINT("%s >> comp [%s] msg [%s] \n", __func__, dest, msg);
                    p_fims->Send(method, dest, nullptr, msg);
                }
            }
            free((void*)msg);
        }
    }
    return 0;
}


int VarMapUtils::vListSendFims(varsmap& vmap, const char* method, fims* p_fims, const char* uri, bool sim, assetVar* avd)
{
    if (!p_fims && !avd)
    {
        if (1)FPS_ERROR_PRINT("%s >> No Fims Connection  \n", __func__);
        return -1;
    }
    char* msg;
    for (auto ix : vmap)
    {
        cJSON* cj = getVmapCJ(ix.second);// Add flag for naked ,value or full dict  var: val var:value:val full = var:{value:val ,scale: scale } html ???
        msg = sim ? cJSON_PrintUnformatted(cj->child ? cj->child : cj) : cJSON_PrintUnformatted(cj);
        cJSON_Delete(cj);
        if (msg)
        {
            // TODO not sending any gets et
            if (strcmp(method, "get") == 0) // need a uri for a get
            {
                if (uri && strlen(uri) > 0)
                {
                    char* atmp;
                    asprintf(&atmp, "%s/%s", uri, ix.first.c_str());
                    if (atmp)
                    {
                        p_fims->Send(method, atmp, nullptr, nullptr);
                        free(atmp);
                    }
                }
            }
            else
            {
                const char* dest = ix.first.c_str();
                if (dest != nullptr && strlen(dest) > 0)
                {
                    if (0)FPS_ERROR_PRINT("%s >> comp [%s] msg [%s] \n", __func__, dest, msg);
                    if(p_fims)
                        p_fims->Send(method, dest, nullptr, msg);
                }

                if(avd)
                    avd->setVal(msg);

            }
            free((void*)msg);
        }
    }
    return 0;
}

//
// new wrinkle send to another assetvar
int VarMapUtils::sendAssetVar(assetVar* av, fims* p_fims,  const char* meth, const char* comp, const char* var, assetVar* avd)
{
    cJSON* cj = nullptr;

    const char *svar = var?var:av->name.c_str();
    const char *scomp = comp?comp:av->comp.c_str();
    const char *smeth = meth?meth:"pub";

    // get the value from the assetVar
    av->getCjVal(&cj);
    if(cj && cj->child)
    {
        // we may want to send the value out under a different name
        cJSON* cjval = cJSON_Duplicate(cj->child, true);
        cJSON* cjsend = cJSON_CreateObject();
        cJSON_AddItemToObject(cjsend, svar, cjval);
        char* spsend  = cJSON_PrintUnformatted(cjsend);
        cJSON_Delete(cjsend);
        if(spsend)
        {
            if(0) FPS_ERROR_PRINT(" %s >> meth [%s] comp [%s] msg [%s]\n"
                    ,__func__
                    , smeth
                    , scomp
                    , spsend
                    );
            if(p_fims)
                p_fims->Send(smeth, scomp, nullptr, spsend);
            if(avd)
                avd->setVal(spsend);
            free(spsend);
        }
    }
    if(cj)cJSON_Delete(cj);
    return 0;
}

// TODO make this work
// uri could be /a/b/c@p?a=3,b=45   ..
// uri could be /a/b:c@p?a=3,b=45   ..

cJSON* VarMapUtils::checkSingleUri(varsmap& vmap, int& single, char** vName, char** cName, const char* method, const char* uri, const char* body)
{
    assetUri my(uri);
    if (1)FPS_ERROR_PRINT("%s >> we're working on this one uri [%s] nfrags %d my.Uri [%s] my.Var [%s] my.Param [%s]\n", 
            __func__, uri, my.nfrags, my.Uri, my.Var, my.Param );
    //single = 0;
    cJSON* cj = nullptr;
    char* vv1 = my.pullPfrag(my.nfrags-1); // /a/b/c  -> /a/b
    char* vv2 = my.pullPvar(my.nfrags-1);  // /a/b/c  ->c

    if (strcmp(method, "get") == 0)
    {
        // Why ?? single |= 0x0100;   // lets try anyway.
        auto ix = vmap.find(my.Uri);
        if (ix != vmap.end())
        {
            if (process_fims_debug)FPS_ERROR_PRINT("%s >> Table found for  get uri  [%s]\n", __func__, uri);
        }
        else
        {
            if (process_fims_debug)FPS_ERROR_PRINT("%s >> Table NOT found for  get uri  [%s]\n", __func__, uri);
        }

        if (ix == vmap.end())
        {
            if (1)FPS_ERROR_PRINT("%s >> possible get single uri [%s] vv1 [%s] vv2 [%s] \n"
                            , __func__, uri, vv1, vv2);

            //single |= 1;
            // rework uri to be one less
            // /x/y/z  => /x/y  with a ar of Z
            //int parts = get_nfrags(uri);
            char* varName = vv2; //pull_pfrag(uri, parts);
            char* uriName = vv1; //pull_uri(uri, parts);
            if (0)FPS_ERROR_PRINT("%s >> possible get single (0x%04x) uri  [%s] comp [%s] len %ld var [%s]\n"
                            , __func__, single, uri, uriName, strlen(uriName), varName);
           
            if (strlen(uriName) == 0)
            {
                *cName = strdup(uri);
            }
            else
            {
                *cName = strdup(uriName);
            }

            *vName = varName?strdup(varName):nullptr;
            auto ix2 = vmap.find(*cName);
            if (ix2 == vmap.end())
            {
                // No luck finding table
                if (1)FPS_ERROR_PRINT("%s >> Did not find  comp single uri  [%s] comp [%s] var [%s]\n", __func__, uri, *cName, varName);
                free((void*)*cName);
                if (*vName)free((void*)*vName);
                *vName = nullptr;
                *cName = strdup(uri);

                if(vv1)free(vv1);
                if(vv2)free(vv2);
                return nullptr;
            }
            auto ix3 = vmap[*cName].find(varName);
            if (ix3 == vmap[*cName].end())
            {
                // No luck finding var in table
                if (0)FPS_ERROR_PRINT("%s >> Did not find  var single uri  [%s] comp [%s] var [%s]\n", __func__, uri, uriName, varName);
                if(vv1)free(vv1);
                if(vv2)free(vv2);
                return nullptr;
            }
            single |= 0x0001; // TODO proper mask
            if(vv1)free(vv1);
            if(vv2)free(vv2);
            return nullptr;
        }
        else
        {
            *vName = nullptr;
            *cName = strdup(uri);
            // Why
            //single |= 0x0100; // TODO proper mask
            if(vv1)free(vv1);
            if(vv2)free(vv2);

            return nullptr;
        }

    }
    else   // Set or Pub
    {
        auto ix = vmap.find(uri);
        if (ix == vmap.end())
        {
            if (0)printf("possible single uri  [%s]\n", uri);
            //single |= 1;
        }
        else
        {
            if (0)printf("%s >> possible single uri  [%s]\n", __func__, uri);
        }

        if (body) cj = cJSON_Parse(body);
        if (!cj)
        {
            if (0)printf(" cj failed def single\n");
            single |= 0x0001;
            single |= 0x0100;
        }
        else
        {
            // look for name and ! value
            if (0)printf("%s >> cj OK, may not be single body [%s] type %d\n", __func__, body, cj->type);
            if (cj->type == cJSON_Object)
            {
                cJSON* cji = cJSON_GetObjectItem(cj, "value");
                cJSON* cjn = cJSON_GetObjectItem(cj, "name");

                if (cji)
                {
                    if (0)printf("%s >>cj OK  found value, confirmed SINGLE  uri [%s] body [%s] type %d\n"
                        , __func__, uri, body, cj->type);
                    single |= 0x0001;
                    single |= 0x0010;
                }
                else if (cjn)
                {
                    if (0)FPS_ERROR_PRINT("%s >>cj no value but found Name , possible uiasset  uri [%s] body [%s] type %d\n"
                        , __func__, uri, body, cj->type);
                    // this means that it will have naked params and asset params
                    //This is a candidate for a UI table
                    // Ui objects have naked assetVars and uiassetVars
                    // a naked setvar is always a string value pair
                    single |= 0x1000;

                }
                // we'll let it go anyway
                single |= 0x0100;
            }
            else if (cj->type == cJSON_True)
            {
                if (0)printf("cj OK  found true confirmed SINGLE  body [%s] type %d\n", body, cj->type);
                single |= 0x0101;
            }
            else if (cj->type == cJSON_False)
            {
                if (0)printf("cj OK  found false confirmed SINGLE  body [%s] type %d\n", body, cj->type);
                single |= 0x0101;
            }
            else if (cj->type == cJSON_Number)
            {
                if (0)printf("cj OK  found number confirmed SINGLE  body [%s] type %d\n", body, cj->type);
                single |= 0x0101;
            }
            else if (cj->type == cJSON_String)
            {
                if (0)printf("cj OK  found string confirmed SINGLE  body [%s] type %d\n", body, cj->type);
                single |= 0x0101;
            }
            else
            {
                single |= 0x0100;
            }
        }
        if(vv1)free(vv1);
        if(vv2)free(vv2);
        vv1=nullptr;
        vv2=nullptr;

    }

    // if we are a single get a new URI
    // get num parts
    // get the last part
    // remove the last part and we have a comp plus a var name
    if (single & 0x0001)
    {
        char* vv1 = my.pullPfrag(my.nfrags-1); // /a/b/c  -> /a/b
        char* vv2 = my.pullPvar(my.nfrags-1);  // /a/b/c  ->c
        
        if (1)FPS_ERROR_PRINT("%s >>  got single, varName [%s] uriName [%s]\n", __func__, vv2, vv1);
        *vName = vv2;
        *cName = vv1;
    }
    else
    {
        *vName = nullptr;
        *cName = strdup(uri);
    }
    //if(cj)cJSON_Delete(cj);
    return cj;
}

// check for a blocked URI
// also filter options
bool VarMapUtils::checkedBlockedUri(varsmap& vmap, char** cName, int& opts, const char* method, const char* uri)
{
    bool blocked = false;
    const char* ess = "/ess";
    const char* naked = "/naked";
    const char* full = "/full";
    const char* components = "/components";
    const char* site = "/site";
    char* sp = (char*)uri;

    // block gets on components // bypass with /ess/components
    if((strcmp(method, "get")== 0) &&  (strncmp(uri, components, strlen(components)) == 0))
    {
        blocked = true;
        return blocked;
    }
    // block pubs on site // bypass with /ess/site
    if((strcmp(method, "pub")== 0) &&  (strncmp(uri, site, strlen(site)) == 0))
    {
        blocked = true;
        return blocked;
    }
    if (strncmp(uri, ess, strlen(ess)) == 0)
    {
        blocked = false;
        sp += strlen(ess);
    }
    if(!blocked)
    {
        if (strncmp(sp, naked, strlen(naked)) == 0)
        {
            sp += strlen(naked);
            opts |= 0x0001;
            //return cj; 
        }
        if (strncmp(sp, full, strlen(full)) == 0)
        {
            sp += strlen(full);
            opts |= 0x0010;
            opts &= ~0x0001;  // this one wins
            //return cj; 
        }
    }
    
    *cName = strdup(sp);
    bool tblocked = true;
    const char* blockeduri = nullptr;
    if (tblocked)
    {
        tblocked = false;   // default to not blocked

        // This may be deprecated
        if (strcmp(method, "set") == 0)
        {
            blockeduri = "/sets/blocked";
        }
        else if (strcmp(method, "pub") == 0)
        {
            blockeduri = "/pubs/blocked";
        }
        else if (strcmp(method, "get") == 0)
        {
            blockeduri = "/gets/blocked";
        }
        if (blockeduri)
        {
            if (0)FPS_ERROR_PRINT(" %s checking blocked uri [%s] \n", __func__, blockeduri);

            auto ix = vmap.find(blockeduri);
            if (ix != vmap.end())
            {
                for (auto iy : vmap[blockeduri])
                {
                    // skip leading '/' value [%s]
                    if (0)FPS_ERROR_PRINT(" %s >>>> checking uri [%s] buri [%s] name [%s]  bval [%s]\n"
                            , __func__, uri, blockeduri, iy.first.c_str(), iy.second->getbVal()?"true":"false" );

                    if (strncmp(uri, iy.first.c_str(), strlen(iy.first.c_str())) == 0)
                    {
                        tblocked = iy.second->getbVal();
                        //break;
                    }
                }
            }
        }
    }
    if(0)
    {
        if (strcmp(method, "pub") != 0)
        {

            if (1)FPS_ERROR_PRINT(" %s checked uri [%s] buri [%s] tblocked [%s] blocked [%s]\n"
                    , __func__, uri 
                    , blockeduri
                    , tblocked?"true":"false"
                    , blocked?"true":"false"
                                    );
        }
    }

    if(tblocked)
    {
        blocked =  true;
    }


    if (blocked)
    {
        if (0)FPS_ERROR_PRINT("%s >> blocked uri  method [%s] uri [%s] ->[%s]\n"
            , __func__, method, uri, *cName);
    }
    else
    {
        if (0)FPS_ERROR_PRINT("%s >> NOT blocked method [%s] uri [%s] ->[%s]\n"
            , __func__, method, uri, *cName);
    }

    return blocked;
}

// the main fims message processor
bool VarMapUtils::runFimsMsg(varsmap& vmap, fims_message* msg, fims* p_fims, cJSON** cjri)
{
    cJSON* cjr = nullptr;
    char* vName = nullptr;  // new var name
    char* newUri = nullptr;   // new comp (uri) name
    char* cName2 = nullptr;   // new comp (uri) name
    //std::string newBody;   // new comp (uri) name
    int opts = 0;

    if (0||process_fims_debug)FPS_ERROR_PRINT("%s >> before  checkBlocked  uri [%s]\n", __func__, msg->uri);
    bool reject = checkedBlockedUri(vmap, &newUri, opts, msg->method, msg->uri);
    if (strcmp(msg->method, "pub") != 0)
    {
        if (0 || process_fims_debug)FPS_ERROR_PRINT("%s >> after checkBlocked  method [%s] newUri [%s] reject [%s] opts 0x%04x\n"
            , __func__
            , msg->method
            , newUri
            , reject ? "Rejected" : "OK to run"
            , opts
        );
    }

    if (strcmp(msg->method, "get") == 0)
    {
        if (0 || process_fims_debug)FPS_ERROR_PRINT("%s >> after checkBlocked  method [%s] orig uri [%s] newUri [%s] reject [%s] opts 0x%04x\n"
            , __func__
            , msg->method
            , msg->uri
            , newUri
            , reject ? "Rejected" : "OK to run"
            , opts
        );
        process_fims_debug = 1;
    }
    else
    {
        process_fims_debug = 0;
    }        
    if (!reject)
    {
        int single=opts;
        // send in the new uri
        cJSON* cj = checkSingleUri(vmap, single, &vName, &cName2, msg->method, newUri, msg->body);

        if (0 ||process_fims_debug)FPS_ERROR_PRINT("%s  after checkSingle single 0x%04x vName [%s] cName2 [%s] newUri [%s] newBod [%s] cj %p\n"
            , __func__
            , single, vName, cName2, newUri
            , msg->body, (void*)cj);
        // single 0x0100 must be set if this one is valid

        if (single & 0x1000)
        {
            if (process_fims_debug)FPS_ERROR_PRINT("%s  ready for get/load uivars 0x%04x vName [%s] cName2 [%s] newUri [%s] newBod [%s] cj %p\n"
                , __func__
                , single, vName, cName2, newUri
                , msg->body, (void*)cj);
            char* newBod = (char*)msg->body;
            if (strcmp(msg->method, "get") == 0)
                cjr = getUimap(vmap, single, cName2, vName);
            else
                cjr = loadUimap(vmap, single, cName2, vName, newBod);
            if (0 || process_fims_debug)FPS_ERROR_PRINT("%s  ready for get/load uivars 0x%04x vName [%s] cName2 [%s] newUri [%s]  cj %p\n"
                , __func__
                , single, vName, cName2, newUri
                , (void*)cjr);

        }
        else if (single & 0x0100)
        {
            if (0 ||process_fims_debug)FPS_ERROR_PRINT("%s  ready for get/load method [%s] single 0x%04x vName [%s] cName2 [%s] newUri [%s] newBod [%s] cj %p\n"
                , __func__
                , msg->method
                , single, vName, cName2, newUri
                , msg->body, (void*)cj);
            char* newBod = (char*)msg->body;  // for single adjust    /x/y/x 123 => /x/y '{z:{value"123}}'
            if (strcmp(msg->method, "get") == 0)
                cjr = getVmap(vmap, single, cName2, vName, single);//opts);
            else
                cjr = loadVmap(vmap, single, cName2, vName, newBod);

            if (0 || process_fims_debug)FPS_ERROR_PRINT("%s  after get/load uivars 0x%04x vName [%s] cName2 [%s] newUri [%s]  cjr %p\n"
                , __func__
                , single, vName, cName2, newUri
                , (void*)cjr);
            //vm.processRawMsg(vmap, msg->method, cName2.c_str(), msg->replyto, newBod, &cjr);
            if (0)FPS_ERROR_PRINT("%s >> cj result %p\n", __func__, (void*)cjr);
            // if (cjr)
            // {
            //     char* tmp = cJSON_PrintUnformatted(cjr);
            //     if (tmp)
            //     {
            //         if (0)FPS_ERROR_PRINT("%s >> cj result \n%s\n", __func__, tmp);
            //         if (p_fims)
            //         {
            //             if (msg->replyto)
            //             {
            //                 p_fims->Send("set", msg->replyto, nullptr, tmp);
            //             }
            //         }
            //         else
            //         {
            //             FPS_ERROR_PRINT("%s >> cj result \n%s\n", __func__, tmp);
            //         }
            //         free((void*)tmp);
            //     }
            //     cJSON_Delete(cjr);
            // }
        }
        else
        {
            // TODO we need an option to remove the baseuri
            // 
            if (strcmp(msg->method, "get") == 0)
                cjr = getVmap(vmap, single, cName2, vName, single, msg->uri);//opts);

        }
        // for now limit the scope of this to gets and sets
        if ((strcmp(msg->method, "get") == 0) || (strcmp(msg->method, "set") == 0))
        {
            if(cjr && cjri)
            {
                *cjri = cjr;
                cjr = nullptr;
            }
            if (cjr)
            {
                char* tmp = cJSON_PrintUnformatted(cjr);
                if (tmp)
                {
                    if (0)FPS_ERROR_PRINT("%s >> cj result \n%s\n", __func__, tmp);
                    if (p_fims)
                    {
                        if (msg->replyto)
                        {
                            p_fims->Send("set", msg->replyto, nullptr, tmp);
                        }
                    }
                    else
                    {
                        FPS_ERROR_PRINT("%s >> cj result \n%s\n", __func__, tmp);
                    }
                    free((void*)tmp);
                }
                cJSON_Delete(cjr);
            }
        }
        if (cj)cJSON_Delete(cj);
        if (p_fims)p_fims->free_message(msg);
    }
    if (newUri)free((void*)newUri);
    if (cName2)free((void*)cName2);
    if (vName)free((void*)vName);
    process_fims_debug = 0;
    return true;
}

void VarMapUtils::processMsgGet(varsmap& vmap, const char* method, const char* uri, const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    assetUri my(uri);

    // TODO allow 3 level base uris /assets/bms/bms_1   .. maybe
    int nfrags = my.nfrags;//get_nfrags(uri);
    //int nbase = 0;
    int opts = 0;
    // hack for assets
    if (strncmp(uri, "/assets", strlen("/assets")==0))
    {
        opts |= 0x0100;
    }

    if (1)FPS_ERROR_PRINT("%s >> we're working on this one uri [%s] nfrags %d my.Uri [%s] my.Var [%s] my.Param [%s]\n", 
            __func__, uri, nfrags, my.Uri, my.Var, my.Param );
    
    *cjr = getMapsCj(vmap, my.Uri, my.Var, opts);


    // if (nfrags == nbase + 1)
    // {
    //     char* smap = pull_pfrag(uri, nbase);// pfrags[0]);
    //     if (0)FPS_ERROR_PRINT("%s >> we're working on this one uri [%s] nfrags %d smap [%s]\n", __func__, uri, nfrags, smap);
    //     *cjr = getMapsCj(vmap, smap, nullptr, opts);
    //     if (smap) free((void*)smap);
    // }
    // else if (nfrags == nbase + 2)
    // {
    //     if (0)FPS_ERROR_PRINT("%s >>  running with uri [%s] nfrags %d\n", __func__, uri, nfrags);
    //     *cjr = getMapsCj(vmap, uri, nullptr, opts);
    //     if (0)FPS_ERROR_PRINT("%s >> returned from uri [%s] nfrags %d\n", __func__, uri, nfrags);

    // }
    // // this limits up to 3 levels....
    // else if (nfrags >= nbase + 3)
    // {
    //     char* svar = nullptr;
    //     char* smap = pull_pfrag(uri, nbase + 1);// pfrags[0]);
    //     char* ssys = pull_pfrag(uri, nbase + 2);//msg->pfrags[1]);
    //     char* asuri;
    //     char* sysnaked = nullptr;
    //     asprintf(&asuri, "/%s/%s", smap, ssys);

    //     svar = pull_pfrag(uri, 3);//->pfrags[2]);
    //     if (nfrags > nbase + 3)
    //     {
    //         sysnaked = pull_pfrag(uri, nbase + 3);//msg->pfrags[1]);
    //         opts = 1;
    //     }
    //     if (1)
    //     {
    //         FPS_ERROR_PRINT("%s  we got svar as [%s] sysnaked [%s]\n", __func__, svar, sysnaked ? sysnaked : "dontknow");
    //         FPS_ERROR_PRINT("%s  we got asuri as [%s]\n", __func__, asuri);
    //     }
    //     *cjr = getMapsCj(vmap, asuri, svar, opts);
    //     if (smap) free((void*)smap);
    //     if (ssys) free((void*)ssys);
    //     if (asuri) free((void*)asuri);
    //     if (svar) free((void*)svar);
    //     if (sysnaked) free((void*)sysnaked);
    //    }
}


// this is a little different
// we have naked assetVars
void VarMapUtils::processMsgSetPubUi(varsmap& vmap, const char* method, const char* uri, int& single, const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    setTime();
    int nfrags = get_nfrags(uri);

    if (0)FPS_ERROR_PRINT("%s >> we're working on this one uri [%s] nfrags %d \n", __func__, uri, nfrags);


    if (!cjr)
    {
        FPS_ERROR_PRINT("%s >> requires a cJson reply var\n", __func__);
        return;
    }
    //char* smap = pull_pfrag(uri, 0);// pfrags[0]);

    cJSON* cj = cJSON_Parse(body);
    assetVar* av;
    if (cj)
    {
        if (*cjr == nullptr)
        {
            *cjr = cJSON_CreateObject();
        }
        if (0 || process_fims_debug)FPS_ERROR_PRINT("%s >> method %s [%s] cj->type %d , cj->child %p cj->next %p\n"
            , __func__
            , method
            , cj->string
            , cj->type
            , cj->child
            , cj->next);
        cJSON* cjrx = cj;
        if (cj->child)
            cjrx = cj->child;

        while (cjrx)
        {
            //if this has a child it is a normal ui assetvar
            // the uiasset var will have naked params etc.  
            // else it is a naked assetvar
            if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >> uri [%s] string [%s] cj->type %d (str %d), cj->child %p cj->next %p\n"
                , __func__
                , uri
                , cjrx->string
                , cjrx->type, cJSON_String
                , cjrx->child
                , cjrx->next);

            //setval from cj must accept naked params
            if (cjrx->child)
            {
                av = setValfromCj(vmap, uri, cjrx->string, cjrx->child, true);
                if (am && av)
                    av->am = am;
                if (ai && av)
                    av->ai = ai;
            }
            else
            {
                av = setValfromCj(vmap, uri, cjrx->string, cjrx, true);
                if (am && av)
                    av->am = am;
                if (ai && av)
                    av->ai = ai;
            }

            cJSON* cji2 = cJSON_Duplicate(cjrx, true);
            cJSON_AddItemToObject(*cjr, cjrx->string, cji2);

            cjrx = cjrx->next;
        }
        cJSON_Delete(cj);
    }
    return;
}

// TODO use assetUri
//char* vv1 = my.pullPfrag(my.nfrags-1); // /a/b/c  -> /a/b
//    char* vv2 = my.pullPvar(my.nfrags-1);  // /a/b/c  ->c
void VarMapUtils::processMsgSetPub(varsmap& vmap, const char* method, const char* uri, int& single, const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    assetUri my(uri);

    setTime();
    if (!cjr)
    {
        FPS_ERROR_PRINT("%s >> requires a cJson reply var\n", __func__);
        return;
    }

    // char* smap = pull_pfrag(uri, 0);// pfrags[0]);

    // if (strcmp(smap, "test") == 0)
    // {
    //     if (0)FPS_ERROR_PRINT("%s >> method %s [%s]  body [%s]\n"
    //         , __func__
    //         , method
    //         , uri
    //         , body ? body : "NoBody"
    //     );
    //     if (smap) free((void*)smap);
    //     return;
    // }
    // if (smap) free((void*)smap);
    cJSON* cj = cJSON_Parse(body);
    if (cj)
    {
        // Evaluate this at the head of the table 
        // watch out for ui_type"control" they also may have a name but no value...
        int uiObject = 0;
        cJSON* cjname = cJSON_GetObjectItem(cj, "name");
        cJSON* cjvalue = cJSON_GetObjectItem(cj, "value");
        if (cj->child && cjname && !cjvalue)
        {
            FPS_ERROR_PRINT("%s >> detected  a uiObject  name [%s]\n", __func__, cjname->valuestring ? cjname->valuestring : " No Name found");
            uiObject = 1;
        }

        if (*cjr == nullptr)
        {
            *cjr = cJSON_CreateObject();
        }
        if (0 || process_fims_debug)FPS_ERROR_PRINT("%s >> method %s [%s] cj->type %d , cj->child %p cj->next %p\n"
            , __func__
            , method
            , cj->string
            , cj->type
            , cj->child
            , cj->next);
        cJSON* cjrx = cj;
        assetVar* av = nullptr;
        if (uiObject > 0)
        {
            av = setValfromCj(vmap, my.Uri, cjname->string, cjname, uiObject);

            if (am && av)
                av->am = am;
            if (ai && av)
                av->ai = ai;

            if (av)
            {
                av->setNaked = true;
            }
            FPS_ERROR_PRINT("%s >> set  uiObject  name [%s] av %p naked [%s]\n"
                , __func__
                , cjname->valuestring ? cjname->valuestring : " No Name found"
                , av
                , av ? av->setNaked ? "true" : "false" : "no AV"
            );
        }
        // hefre we go  with a config list headed into     the uri table

        if (cj->child)
            cjrx = cj->child;

        assetList* alist = nullptr;
        bool newAv = false;

        if (uiObject > 0)
        {
            alist = getAlist(vmap, my.Uri);
            // //Now process the list of objects.

            if (!alist)
            {
                newAv = true;
                alist = setAlist(vmap, my.Uri);

                FPS_ERROR_PRINT("%s >>  new assetList\n"
                    , __func__
                );
            }
        }
        if (alist && av)
        {
            alist->add(av);
            av = nullptr;
        }
        // // we may need to clean out the old avec but for now just leave it. 
        // TODO keep the order for uiObjects in a vector 
        while (cjrx)
        {

            // todo if uiObject populate order list...
            // todo what happened to the ui controls ??   they had name but no value .... allow name and ui_type = control
            // todo why is "name" corrupted ?? see above
            av = setValfromCj(vmap, my.Uri, cjrx->string, cjrx, uiObject);
            if (av)
            {
                if (am && av)
                    av->am = am;
                if (ai && av)
                    av->ai = ai;

                if (alist && newAv) alist->add(av);
            }

            if (0 || setvar_debug)FPS_ERROR_PRINT(" %s >> uri [%s] string [%s] cj->type %d (str %d), cj->child %p cj->next %p av %p\n"
                , __func__
                , uri
                , cjrx->string
                , cjrx->type, cJSON_String
                , cjrx->child
                , cjrx->next
                , av
            );
            cJSON* cji2 = cJSON_Duplicate(cjrx, true);
            cJSON_AddItemToObject(*cjr, cjrx->string, cji2);

            cjrx = cjrx->next;
        }
        cJSON_Delete(cj);
        //// no no no dont delete this 
        //if(alist) delete alist;
    }
    return;
}

// use assetUri
//char* vv1 = my.pullPfrag(my.nfrags-1); // /a/b/c  -> /a/b
//char* vv2 = my.pullPvar(my.nfrags-1);  // /a/b/c  ->c
void VarMapUtils::processMsgSetReply(varsmap& vmap, const char* method, const char* uri, const char* replyto, const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    assetUri my(uri);

    //char* var = nullptr;
    //char* uri = strdup(inuri);
    int nfrags = my.nfrags;
    cJSON* cj = cJSON_Parse(body);
    // look for var in last frag

    // The var may be the last frag
    //if (my.Var && nfrags > 2)
    //{
    //    var = pull_pfrag(uri, nfrags);
    //}

    if (*cjr == nullptr)
    {
        *cjr = cJSON_CreateObject();
    }

    assetVar* av = nullptr;
    cJSON* cji = nullptr;
    if (cj)
    {
        if (0)FPS_ERROR_PRINT(" %s >>> uri [%s] method %s uri [%s] frgs %d body [%s] va [%s] cj->string [%s] cj->type %d , cj->child %p cj->next %p\n"
            , __func__
            , uri
            , method
            , my.Uri
            , nfrags
            , body
            , my.Var ? my.Var : "noVar"
            , cj->string
            , cj->type
            , (void*)cj->child
            , (void*)cj->next);
        if (my.Var)
        {
            if (0)FPS_ERROR_PRINT("%s >>> [%s] cj->type %d , cj->child %p cj->next %p\n"
                , __func__
                , my.Var
                , cj->type
                , cj->child
                , cj->next);

            av = setValfromCj(vmap, my.Uri, my.Var, cj, false);

            if (am && av)
                av->am = am;
            if (ai && av)
                av->ai = ai;

            if (1) FPS_ERROR_PRINT(" MSGSetReply we got a VAR set with a replyto [%s]\n", replyto ? replyto : "No Reply");
            cJSON* cji2 = cJSON_Duplicate(cj, true);
            cJSON_AddItemToObject(*cjr, my.Var, cji2);
            //free((void*)var);
        }
        else
        {
            cji = cj;
            int uiObject = 0;
            // look for "registers"
            cJSON* cjname = cJSON_GetObjectItem(cj, "name");
            cJSON* cjvalue = cJSON_GetObjectItem(cj, "value");
            if (cj->child && cjname && !cjvalue)
            {
                uiObject = 1;
            }

            if (cj->child)
                cji = cj->child;
            // this processes the list 
            // if this is  UI object we can detect it here.
            // before we head off with the kids


            while (cji)
            {
                if (0)FPS_ERROR_PRINT("%s >> [%s] cj->type %d , cj->child %p cj->next %p\n"
                    , __func__
                    , cji->string
                    , cji->type
                    , cji->child
                    , cji->next);

                av = setValfromCj(vmap, my.Uri, cji->string, cji, uiObject);
                if (am && av)
                    av->am = am;
                if (ai && av)
                    av->ai = ai;

                if (1)FPS_ERROR_PRINT("%s >> MSGSetReply we got a Multi set with a replyto [%s]\n", __func__, replyto);
                cJSON* cji2 = cJSON_Duplicate(cji, true);
                cJSON_AddItemToObject(*cjr, cji->string, cji2);
                cji = cji->next;
            }
        }
        cJSON_Delete(cj);
    }
//    if (uri) free((void*)uri);
    return;
}

void VarMapUtils::addCjFrags(cJSON* cj, const char* uri, cJSON* junk)
{
    assetUri my(uri);
    int nfrags = my.nfrags;
    //char* suri = strdup(my.Uri);
//ToDo use my.urivec
    cJSON* cji = cj;
    cJSON* cjf = cj;
    for (int i = 0 ; i <nfrags ;i++)
    {
        if (nfrags == 1)
        {
            cjf = junk;
        }
        else
        {
            cjf = cJSON_GetObjectItem(cji, my.uriVec[i]);
        }

        if (!cjf)
        {
            cjf = cJSON_CreateObject();
        }
        cJSON_AddItemToObject(cji, my.uriVec[i], cjf);

        //nfrags--;
        //suri += (strlen(firsturi));
        cji = cjf;
    }
    //free((void*)suri);
}


bool VarMapUtils::cJSON_Compare(cJSON *a, cJSON*b)
{
    if(!a->child ) return false;
    if(!b->child ) return false;
    a = a->child;
    b = b->child;
    
    if(0)FPS_ERROR_PRINT(" %s >> a->type  %d b->type %d \n", __func__, a->type, b->type);
    if(a->type == b->type)
    {
        if((a->type == cJSON_False ) || (a->type == cJSON_True)) return true; 
        else if(a->type == cJSON_Number ) 
        {
            if (std::abs(a->valuedouble-b->valuedouble) < 0.001) return true;
        }
        else if(a->type == cJSON_String) 
        {
            if(strcmp(a->valuestring, b->valuestring) == 0) return true;
        }
    }
    return false;
}
// so here's the thing  rules for set

// simple /set a/b/c {"component":{"value":1234},.....}  Yes
// naked /set a/b/c {"component":1234, ...}  YES
// single    set /a/b/c/component 1234        NO                           /a/b/c/component is mot a table /a/b/c is sor set/add component to /a/b/c
// perhaps       /a/b/c/component '{"value":1234}'    YES

//   get     /a/b/c/component      NO Error
//   get     /a/b/c        Ok but we should search anyway  Error

// get /a/b/c/ should return just /a/b/c

// TODO lock varmap    
// get one or get them all
// added assetList concept for uiObjects to keep order

// global search  given a uri get a list of comps that match it
// /assets  -> /assets/ess /assets/bms /assets/bms_1    etc 

// opts 0x0000     default , full comps , value
// opts 0x0001     default , full comps , naked
// opts 0x0010     dump object , full comps , value
// opts 0x0100     dump object , reduced , value


cJSON* VarMapUtils::getMapsCj(varsmap& vmap, const char* inuri, const char* var, int opts, const char* origuri , cJSON* cji)
{
    if (process_fims_debug)FPS_ERROR_PRINT("%s >> getting cj maps uri [%s] var [%s] opts 0x%04x\n"
        , __func__
        , inuri ? inuri : "noURI"
        , var ? var : "noVar"
        , opts
    );
    int found = 0;
    cJSON* cj = nullptr;  //cJSON_CreateObject();
    char* uri = (char*)inuri;
// get /assets/bms should retun
//     {
// "summary": {...},
// "bms_1": {...},
// "bms_2": {...},
// "bms_3": {...},
// "bms_4": {...},
// )
// path 1 uri and no var 
// NOTE this will return the table name as the first object.

    if (uri && !var)
    {

        std::vector<std::string> nVec;
        std::string bs;
        //int rc = 
        // Hack for assets
        // if (strncmp(uri, "/assets", strlen("/assets")==0))
        // {
        //     opts |= 0x0100;
        // }
        createAssetListCj(vmap, uri, sysVec, opts, nVec);
        cJSON* cjm = createUriListCj(vmap, bs, uri, nullptr, opts, nVec);
        {
            if (process_fims_debug)
            {
                char* tmp = cJSON_Print(cjm);

                FPS_ERROR_PRINT("%s >> used createUriList from uri [%s] basevec [%s] tmp \n>>%s<<\n", __func__, uri, bs.c_str(), tmp);
                //if (uri != inuri) free((void*)uri);
                if (tmp) free((void*)tmp);
            }
            if(bs.length() > 0)
            {
                cJSON* cjx = cJSON_CreateObject();
                cJSON_AddItemToObject(cjx, bs.c_str(), cjm);
                cjm = cjx;
            }
            if (uri != inuri) free((void*)uri);
            return cjm;
            //cJSON_Delete(cjm);
        }


    }
    // path 2 we have a uri and a var
    else if (uri && var)
    {
        auto x = vmap.find(uri);
        if (x != vmap.end())
        {
            auto y = vmap[uri].find(var);
            if (y != vmap[uri].end())
            {
                found++;
                cJSON* cji = cJSON_CreateObject();
                if (0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] found %d opts 0x%04x\n", __func__, uri, y->first.c_str(), found, opts);
                // TODO need opts here 
                if (opts & 0x0010)
                {
                    if (0)FPS_ERROR_PRINT(" %s >> showvarCJ  for uri [%s] var [%s] found %d opts 0x%04x\n", __func__, uri, y->first.c_str(), found, opts);
                    y->second->showvarCJ(cji, opts);
                }
                else
                {
                    y->second->showvarValueCJ(cji, opts);
                }
                // char* tmp = cJSON_PrintUnformatted(cji);
                // if(tmp)
                // {
                //     if(0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] as [%s] \n", __func__, uri, y->first.c_str(), tmp);
                //     free((void *)tmp);
                // }
                cJSON* cj = cji;
                // naked
                if (opts & 0x0001 && cji->child)
                {
                    cj = cJSON_Duplicate(cji->child, true);
                    // tmp = cJSON_PrintUnformatted(cji->child);
                    // if(tmp)
                    // {
                    //     if(0)FPS_ERROR_PRINT(" %s >> final  cj child for uri [%s] var [%s] as [%s] \n", __func__
                    //             , uri
                    //             , var?var:"noVar"
                    //             , tmp);
                    //     free((void *)tmp);
                    // }
                    cJSON_Delete(cji);
                }
                if (uri != inuri) free((void*)uri);
                return cj;

            }
        }
    }
    else
        // get all the objects
    {
        cJSON* cj = cJSON_CreateObject();

        if (0)FPS_ERROR_PRINT(" %s >>  get them all opts [0x%04x] \n"
            , __func__
            , opts
        );
        int found = 0;

        for (auto& x : vmap)
        {
            if(0)FPS_ERROR_PRINT(" %s >>  get them all comp [%s] \n"
                , __func__
                , x.first.c_str()
                );

            //cJSON* cjx = loadAssetList(vmap, x.first.c_str(), found, opts);
            if (0)FPS_ERROR_PRINT(" %s >>  get them all comp [%s]  assetlists \n"
                , __func__
                , x.first.c_str()
            );

            cJSON* cji = cJSON_CreateObject();
            // assetList* alist = getAlist(vmap, x.first.c_str());

            // if (0)FPS_ERROR_PRINT(" %s >>       >>>>>>>>>>>>>>>>>>>>query alist >>>>>>>>>>>>>>>>>>>>>>>>>>getting cj for comp [%s]  alist %p\n"
            //     , __func__
            //     , x.first.c_str()
            //     , (void*)alist
            // );
            // if (alist)
            // {
            //     unsigned int ix = 0;
            //     assetVar* av;
            //     do
            //     {
            //         av = alist->avAt(ix++);
            //         if (av) av->showvarCJ(cji, opts);
            //     } while (av);
            //     found++;
            // }
            //else
            {
                for (auto& y : vmap[x.first])
                {
                    if (y.second != nullptr)
                    {
                        if(0) FPS_ERROR_PRINT(" %s >>       NOTE running  for var [%s:%s]  av %p cji %p\n", __func__
                            , x.first.c_str(), y.first.c_str(), y.second, cji);
                        
                        y.second->showvarCJ(cji, opts);
                        found++;
                    }
                    else
                    {
                        FPS_ERROR_PRINT(" %s >>       NOTE no map for var [%s] \n", __func__, y.first.c_str());
                    }
                }
            }
            // shoud really compare x.first.c.str() with cji->string
            //cJSON_AddItemToObject(cj, x.first.c_str(), cji->child?cji->child:cji);
            // if (opts & 0x0001)
            // {
            //     // TODO get showvarCJ to work properly for naked stuff
            //     cJSON* cjii = cJSON_Duplicate(cji->child, true);
            //     cJSON_AddItemToObject(cj, x.first.c_str(), cjii);
            //     cJSON_Delete(cji);
            // }
            // else
            {
                cJSON_AddItemToObject(cj, x.first.c_str(), cji);

            }
            found++;
        }
        // if (cj)
        // {
        //     char* tmp = cJSON_PrintUnformatted(cj);
        //     if (tmp)
        //     {
        //         if (0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] as [%s] \n", __func__
        //             , uri
        //             , var ? var : "noVar"
        //             , tmp);
        //         free((void*)tmp);
        //     }
        // }
        if (uri != inuri) free((void*)uri);
        return cj;
    }
    if (0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<got cj %p maps  found [%d]\n", (void*)cj, found);
    if (found == 0)
    {
        cJSON_Delete(cj);
        cj = nullptr;
    }
    if (cj)
    {
        char* tmp = cJSON_PrintUnformatted(cj);
        if (tmp)
        {
            if (0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] as [%s] \n", __func__
                , uri
                , var ? var : "noVar"
                , tmp);
            free((void*)tmp);
        }
    }
    if (uri != inuri) free((void*)uri);
    return cj;
}


// gets a version of an assetlist
int VarMapUtils::getAssetListVersion(varsmap& vmap, const char* alistname, const char* alistVersion)
{
    int rc = 1;
    char* aload;
    asprintf(&aload, "saved_configs/%s/%s", alistname, alistVersion);
    if (aload)
    {
        // now load 
        configure_vmap(vmap, aload);
        free((void*)aload);
        rc = 0;
    }
    return rc;
}

//TODO lock varmap
// get one or get them all
void VarMapUtils::getMapsVm(varsmap& vmap, varsmap& vmr, const char* uri, const char* var)
{
    if (0)FPS_ERROR_PRINT("%s >> getting vm maps uri [%s] var [%s] \n"
        , __func__
        , uri ? uri : "noURI"
        , var ? var : "noVar"
    );

    if (uri && !var)
    {
        auto x = vmap.find(uri);
        if (x != vmap.end())
        {
            //auto x = vmap.find(uri);
            for (auto y : vmap[uri])
            {
                vmr[x->first.c_str()][y.first.c_str()] = y.second;
            }
        }
    }
    else if (uri && var)
    {
        auto x = vmap.find(uri);
        if (x != vmap.end())
        {
            //                auto x = vmap.find(uri);
            auto y = vmap[uri].find(var);
            if (y != vmap[uri].end())
            {
                //                  auto y = vmap[uri].find(var);
                vmr[x->first.c_str()][y->first.c_str()] = y->second;
            }
        }
    }
    else
        // get all the objects
    {
        for (auto& x : vmap)
        {

            for (auto& y : vmap[x.first])
            {
                if (y.second != nullptr)
                {
                    vmr[x.first.c_str()][y.first.c_str()] = y.second;

                }
                else
                {
                    printf("       NOTE no map for var [%s] \n", y.first.c_str());
                }
            }
        }
    }
    return;
}

bool VarMapUtils::strMatch(const char* str, const char* key)
{
    //char* key2=nullptr;
    if (0)printf("  Match test  for str [%s] key [%s] \n", str, key);

    //char rep = 0;
    int mlen = 0;
    if (key) mlen = strlen(key);
    if (mlen == 0 || (mlen > 0 && key[0] == '*'))
    {
        if (0)printf(" >>>> Match true  for str [%s] key [%s] \n", str, key);
        return true;
    }
    if (!str)
    {
        FPS_ERROR_PRINT(" %s >>>>  no str for match  key [%s] \n", __func__, key);
        return false;
    }
    if (strncmp(str, key, mlen) == 0)
    {
        if (0)printf("     >>>> Match true  for str [%s] key [%s] mlen %d \n", str, key, mlen);
        return true;
    }

    // rep = key[mlen-1];
    // if (rep == '*')
    // {
    //     key2 = strdup(key);
    //     mlen--;
    //     key2[mlen] = 0;
    //     if (strncmp(str, key2, mlen)== 0)
    //     {
    //         Match = true;
    //         if(0)printf(" >>>> Match true  for str [%s] key [%s] \n", str, key);

    //     }
    //     free((void *)key2);
    // }
    // else
    // {
    //     if (strcmp(str, key)== 0)
    //     {
    //         Match = true;
    //         if(0)printf("  >>>> Match true  for str [%s] key [%s] \n", str, key);
    //     }
    // }
    return false;
}
// TODO lock varmap
    // get one or get them all
cJSON* VarMapUtils::getCompsCj(varsmap& vmap, const char* key, const char* var)
{
    if (0)FPS_ERROR_PRINT("%s >> getting cj comps key [%s] var [%s] \n"
        , __func__
        , key ? key : "noKey"
        , var ? var : "noVar"
    );

    cJSON* cj = cJSON_CreateObject();

    if (key && !var)
    {
        for (auto& x : vmap)
        {
            //if (strncmp(x.first.c_str(), key, strlen(key))== 0)
            if (strMatch(x.first.c_str(), (char*)key))
            {
                if (0)printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj map for [%s] \n", x.first.c_str());

                cJSON_AddStringToObject(cj, x.first.c_str(), "none");
                if (0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<got cj map for [%s] \n", x.first.c_str());
            }

        }
    }
    else if (key && var)
    {
        for (auto& x : vmap)
        {
            if (strMatch(x.first.c_str(), (char*)key))
            {
                cJSON* cjx = cJSON_CreateObject();

                for (auto& y : vmap[x.first.c_str()])
                {

                    if (y.second != nullptr)
                    {
                        if (strMatch(y.first.c_str(), (char*)var))
                        {
                            cJSON* cjy = cJSON_CreateObject();

                            y.second->showvarValueCJ(cjy);
                            cJSON_AddItemToObject(cjx, y.first.c_str(), cjy);
                        }
                    }
                }
                cJSON_AddItemToObject(cj, x.first.c_str(), cjx);
            }
        }
    }
    else
        // get all the objects
    {
        for (auto& x : vmap)
        {
            if (0)printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj map for [%s] \n", x.first.c_str());

            cJSON_AddStringToObject(cj, x.first.c_str(), "none");
            if (0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<got cj map for [%s] \n", x.first.c_str());

        }
    }
    if (0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<got cj maps\n");

    return cj;
}

void VarMapUtils::getCompsVm(varsmap& vmap, varsmap& rmap, const char* key, const char* var)
{
    varmap rvmap;
    if (key && !var)
    {
        for (auto& x : vmap)
        {
            if (strMatch(x.first.c_str(), (char*)key))
            {
                rmap[x.first] = rvmap;
            }
        }
    }
    else if (key && var)
    {
        for (auto& x : vmap)
        {
            if (strMatch(x.first.c_str(), (char*)key))
            {
                for (auto& y : vmap[x.first.c_str()])
                {
                    if (y.second != nullptr)
                    {
                        if (strMatch(y.first.c_str(), (char*)var))
                        {
                            rmap[x.first][y.first] = y.second;
                        }
                    }

                }
            }
        }
    }
    else
        // get all the objects
    {
        for (auto& x : vmap)
        {
            for (auto& y : vmap[x.first.c_str()])
            {
                if (y.second != nullptr)
                {
                    rmap[x.first][y.first] = y.second;
                }
            }
        }
    }
    return;
}


void VarMapUtils::processRawMsg(varsmap& vmap, const char* method, const char* uri, const char* replyto, const char* body,
    cJSON** cjr, asset_manager* am, asset* ai)
{
    int single = 0;
    if (strcmp(method, "set") == 0)
    {
        if (replyto != nullptr)
            return processMsgSetReply(vmap, method, uri, replyto, body, cjr, am, ai);
        else
            return processMsgSetPub(vmap, method, uri, single, body, cjr, am, ai);
    }
    else if (strcmp(method, "pub") == 0)
        return processMsgSetPub(vmap, method, uri, single, body, cjr, am, ai);

    else if (strcmp(method, "get") == 0)
        return processMsgGet(vmap, method, uri, body, cjr, am, ai);

    return;
}

// all these assume nfrags == 2 except get
    // vmaps is really a name space , vm is an accessor to that name space
void VarMapUtils::processFims(varsmap& vmap, fims_message* msg, cJSON** cjr, asset_manager* am, asset* ai)
{
    return processRawMsg(vmap, msg->method, msg->uri, msg->replyto, msg->body, cjr, am, ai);
}

void VarMapUtils::free_fims_message(fims_message* msg)
{
    if (msg)
    {
        if (msg->method) free((void*)msg->method);
        if (msg->uri) free((void*)msg->uri);
        if (msg->replyto) free((void*)msg->replyto);
        if (msg->body) free((void*)msg->body);
        if (msg->pfrags) free((void*)msg->pfrags);

        free(msg);
    }
}
// turns body: method: replyto: uri: into a fims message ( we also need to go the other way.
fims_message* VarMapUtils::bufferToFims(const char* buffer)
{
    // pull out fields for return
    cJSON* root = cJSON_Parse(buffer);
    if (root == nullptr)
    {
        FPS_ERROR_PRINT("%s: Failed to parse message.\n", program_invocation_short_name);
        return nullptr;
    }

    fims_message* message = (fims_message*)malloc(sizeof(fims_message));

    if (message == nullptr)
    {
        FPS_ERROR_PRINT("%s: Allocation error.\n", program_invocation_short_name);
        return nullptr;
    }
    //    return nullptr;
    message->uri = nullptr;
    message->method = nullptr;
    message->replyto = nullptr;
    message->body = nullptr;
    message->nfrags = 0;
    message->pfrags = nullptr;

    cJSON* body = cJSON_GetObjectItem(root, "body");
    if (body != nullptr)
    {
        if (body->valuestring == nullptr)
        {
            message->body = cJSON_PrintUnformatted(body);
        }
        else
        {
            message->body = strdup(body->valuestring);
        }
    }


    cJSON* method = cJSON_GetObjectItem(root, "method");
    if (method != nullptr)
    {
        message->method = strdup(method->valuestring);
    }

    cJSON* replyto = cJSON_GetObjectItem(root, "replyto");
    if (replyto != nullptr)
    {
        message->replyto = strdup(replyto->valuestring);
    }

    //return nullptr;
    cJSON* uri = cJSON_GetObjectItem(root, "uri");
    if (uri != nullptr)
    {
        message->uri = strdup(uri->valuestring);
        // build pfrags
        int count = 0;
        int offset[MAX_URI_DEPTH];
        for (int i = 0; message->uri[i] != '\0' && count < MAX_URI_DEPTH; i++)
        {
            if (message->uri[i] == '/')
            {
                offset[count] = i;
                count++;
            }
        }
        message->nfrags = count;
        if (count > 0 && count < MAX_URI_DEPTH)
            message->pfrags = (char**)calloc(count, sizeof(char*));
        else
        {
            FPS_ERROR_PRINT("%s: Invalid number of segments in URI", program_invocation_short_name);
        }
        for (int i = 0; i < count; i++)
        {
            message->pfrags[i] = message->uri + (offset[i] + 1);
        }
    }

    cJSON_Delete(root);
    return message;
}

// This function will create a  FIMS message buffer
char* VarMapUtils::fimsToBuffer(const char* method, const char* uri, const char* replyto, const char* body)
{
    if (method == nullptr || uri == nullptr)
    {
        FPS_ERROR_PRINT("%s: Can't transmit message without method or uri.\n", program_invocation_short_name);
        return nullptr;
    }
    // build json object
    cJSON* message = cJSON_CreateObject();
    if (message == nullptr)
    {
        FPS_ERROR_PRINT("%s: Memory allocation error.\n", program_invocation_short_name);
        return nullptr;
    }
    cJSON_AddStringToObject(message, "method", method);
    cJSON_AddStringToObject(message, "uri", uri);
    if (replyto != nullptr)
        cJSON_AddStringToObject(message, "replyto", replyto);
    if (body != nullptr)
        cJSON_AddStringToObject(message, "body", body);

    // create message buffer
    char* tmp_str = cJSON_PrintUnformatted(message);
    cJSON_Delete(message);
    return tmp_str;
}

void VarMapUtils::clearVm(varmap& vmap)
{
    for (auto& y : vmap)
    {
        printf(" deleteing [%s]\n", y.first.c_str());
        delete y.second;
    }
    vmap.clear();
}


void VarMapUtils::clearVmap(varsmap& vmap)
{
    for (auto& x : vmap)
    {
        for (auto& y : vmap[x.first])
        {
            delete y.second;
        }
        vmap[x.first].clear();
    }
    vmap.clear();
}


// configure_vmap
// use a list of reps to modify the file
int VarMapUtils::configure_vmap(varsmap& vmap, const char* fname, std::vector<std::pair<std::string, std::string>>* reps, asset_manager* am, asset* ai)
{
    int rc = 0;
    cJSON* cjbase = get_cjson(fname, reps);
    if (!cjbase)
        rc = -1;
    cJSON* cj = nullptr;
    if (cjbase) cj = cjbase->child;
    //const char* vname;
    while (cj)
    {
        // uri - cj->string
        // uri->body - cj->child
        //FPS_ERROR_PRINT(" cj->string [%s] child [%p]\n", cj->string, (void *) cj->child);
        char* body = cJSON_Print(cj);
        FPS_ERROR_PRINT(" %s >> cj->string [%s] child [%p] body \n[%s]\n"
            , __func__, cj->string, (void*)cj->child, body);

        char* buf = fimsToBuffer("set", cj->string, nullptr, body);
        free((void*)body);
        fims_message* msg = bufferToFims(buf);
        free((void*)buf);
        cJSON* cjb = nullptr;
        processFims(vmap, msg, &cjb, am, ai);
        free_fims_message(msg);

        buf = cJSON_Print(cjb);
        if (cjb) cJSON_Delete(cjb);

        FPS_ERROR_PRINT("%s >>  configured [%s]\n", __func__, buf);
        free((void*)buf);
        cj = cj->next;
    }
    if (cjbase) cJSON_Delete(cjbase);
    return rc;
}

void VarMapUtils::CheckLinks(varsmap& vmap, varmap& amap, const char* aname)
{    // vmap["/links/bms_1"]

    auto ix = vmap.find(aname);
    if (ix != vmap.end())
    {
        // if this works no need to run the init function below
        FPS_ERROR_PRINT("%s >>  We found our links , we should be able to set up our link amap\n", __func__);
        for (auto iy : ix->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                assetVar* av = iy.second;
                assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

                FPS_ERROR_PRINT("%s >> lets link [%s] to [%s]\n"
                    , __func__
                    , iy.first.c_str()
                    , aVal->valuestring
                );
                // for example lets link [AcContactor] to the var defined for [/status/bms_1:AcContactor]

                // amap[iy.first] = vm.getVar (vmap, y.second->aVal->valuestring);//  getVar(varsmap &vmap, const char* comp, const char* var=nullptr)

        //amap["AcContactor"]               = linkVal(vmap, link, "AcContactor",            fval);
            }
        }
    }

}
// TODO add timeout
// move to lib
template<class T>
bool VarMapUtils::valueChanged(T one, T theother, T deadb)
{
    T diff = one - theother;

    if (diff > deadb || diff < -deadb)
        return true;
    else
        return false;
}
template<class T>
bool VarMapUtils::valueChanged(T one, T theother)
{
    T diff = one - theother;

    if (diff)
        return true;
    else
        return false;
}

// return when one or the other has changed
// or when we hae passed a time
// TODO test timeout
template<class T>
bool VarMapUtils::valueChanged(assetVar* one, assetVar* theother, assetVar* deadb, T vtype, double timeout)
{
    bool resp;
    //T dval;
    resp = valueChanged(one->getVal(vtype), theother->getVal(vtype),
        deadb->getVal(vtype));
    if (!resp)
    {
            assetVal* aVal = one->linkVar?one->linkVar->aVal:one->aVal;

        if (timeout > 0.0 && aVal->getsTime() + timeout > getTime())
        {
            return true;
        }
    }
    return resp;
}
template<class T>
bool VarMapUtils::valueChangednodb(assetVar* one, assetVar* theother, T vtype, double timeout)
{
    bool resp;
    //T dval;
    resp = valueChanged(one->getVal(vtype), theother->getVal(vtype));
    if (!resp)
    {
        assetVal* aVal = one->linkVar?one->linkVar->aVal:one->aVal;
        if (timeout > 0.0 && aVal->getsTime() + timeout > getTime())
        {
            return true;
        }
    }
    return resp;
}


int VarMapUtils::getSubCount(char* scopy)
{
    int i = 0;
    char* sp = scopy;
    while (*sp && *sp == ' ')sp++;
    if (*sp)
        i++;
    while (*sp)
    {
        if (*sp == ',')
        {
            if (0)FPS_ERROR_PRINT(" %s >> found comma in [%s] ccnt %d\n", __func__, sp, i);
            i++;
        }
        sp++;
    }
    if (0)FPS_ERROR_PRINT(" %s >> found comma in [%s] ccnt %d\n", __func__, scopy, i);

    return i;
}

int VarMapUtils::showList(char** subs, const char* aname, int& ccnt)
{
    int i;
    for (i = 0; i < ccnt;i++)
    {
        FPS_ERROR_PRINT("%s subs [%s] [%d] = [%s]\n", __func__, aname, i, subs[i]);
    }
    return i;
}

int VarMapUtils::addListToVec(vecmap& vecs, char** subs, const char* vname, int& ccnt)
{
    int i;
    for (i = 0; i < ccnt;i++)
    {
        vecMapAddChar(vecs, vname, subs[i]);
        //            printf("subs [%d] = [%s]\n",i, subs[i]);
    }
    return i;
}

int VarMapUtils::clearList(char** subs, const char* aname, int& ccnt)
{
    int i;
    for (i = 0; i < ccnt;i++)
    {
        free((void*)subs[i]);
    }
    free((void*)subs);
    return 0;
}

void VarMapUtils::getVList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt)
{
    char** slist = getList(vecs, vmap, amap, aname, vname, ccnt);
    clearList(slist, aname, ccnt);
}

char** VarMapUtils::getVmapList(vecmap& vecs, varsmap& vmap, int& ccnt)
{
    char** vars = nullptr;
    ccnt = 0;
    for (auto x : vmap)
    {
        for (auto y : vmap[x.first])
        {
            ccnt++;
        }
    }
    if (ccnt > 0)
    {
        vars = (char**)calloc(ccnt + 1, sizeof(char*));
        ccnt = 0;
        for (auto x : vmap)
        {
            for (auto y : vmap[x.first])
            {
                asprintf(&vars[ccnt++], "%s:%s", x.first.c_str(), y.first.c_str());
            }
        }
    }
    return vars;
}
// sets up the internal structure for an assetList from a string right now but we'll add the template file option real soon.
int VarMapUtils::setupAssetList(varsmap& vmap, const char* aname, const char* alistname, const char* alistVersion, asset_manager* am)
{
    int ccnt = 0;
    vecmap vecs;  // may not be used
    varmap amap;
    // amap not used here
    char** assets = getList(vecs, vmap, amap, aname, alistname, ccnt);

    // then look for a template file
    if (ccnt == 0)
    {
        varsmap tmap;
        char* aload = nullptr;
        asprintf(&aload, "saved_configs/%s/%s", alistname, "template");
        if (aload)
        {
            configure_vmap(tmap, aload);
            assets = getVmapList(vecs, tmap, ccnt);
            free((void*)aload);
        }
    }

    if (0)FPS_ERROR_PRINT(" %s >> seeking [%s] Found ccnt as %d\n", __func__, alistname, ccnt);
    if (ccnt > 0)
    {
        if (0)showList(assets, aname, ccnt);
        assetList* alist = setAlist(vmap, alistname);
        //now we need to add the assets mentioned.  we make have to make the assets.
        //
        //one for each ccnt in assets
        //
        for (int iy = 0; iy < ccnt; iy++)
        {
            double dval = 0.0;  // this will set up the av a a double 
            assetVar* av = setVal(vmap, assets[iy], nullptr, dval);
            alist->add(av);
        }
        clearList(assets, nullptr, ccnt);
    }
    return ccnt;
}

//char** getListStr(vecmap &vecs, varsmap &vmap, varmap &amap, const char* aname, const char* vname,  int &ccnt, char* dbsubs)
char** VarMapUtils::getListStr(vecmap& vecs, varsmap& vmap, const char* vname, int& ccnt, char* dbsubs)
{
    char** sret = nullptr;
    char* scopy;

    ccnt = 0;
    if (dbsubs)
    {
        FPS_ERROR_PRINT(" %s >> recovered [%s] as [%s]\n", __func__, vname, dbsubs);
        char* sp = dbsubs;
        while (*sp && *sp == ' ')sp++;
        scopy = strdup(sp);

        ccnt = getSubCount(scopy);
        sret = (char**)calloc(ccnt + 1, sizeof(char*));
        sp = scopy;
        int idx = 0;
        int retcc = 0;
        while (idx < ccnt)
        {
            while (*sp && (*sp == ',' || *sp == ' '))sp++;
            if (strlen(sp) > 0)
            {
                sret[retcc] = strdup(sp);
                sp = sret[retcc];
                while (*sp && *sp != ',' && *sp != ' ')sp++;
                *sp++ = 0;
                retcc++;
            }
            idx++;
        }
        free((void*)scopy);
        // addListToVec(vecs, sret, aname, retcc);
        // showList(sret,aname, retcc);

        sret[retcc] = nullptr;
        ccnt = retcc;
    }
    return sret;
}

char** VarMapUtils::getList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt)
{
    char** sret;

    char* dbsubs = nullptr;
    amap[vname] = setLinkVal(vmap, aname, "/config", vname, dbsubs);
    dbsubs = amap[vname]->getVal(dbsubs);

    //sret = getListStr(vecs, vmap, amap, aname, vname,  ccnt, dbsubs);
    sret = getListStr(vecs, vmap, vname, ccnt, dbsubs);
    if (sret)
    {
        if (0)FPS_ERROR_PRINT(" %s >> found list string dbsubs [%s] ccnt [%d]\n", __func__, dbsubs, ccnt);
        addListToVec(vecs, sret, vname, ccnt);
        showList(sret, aname, ccnt);
    }
    else
    {
        ccnt = 0;
        FPS_ERROR_PRINT(" %s >>>>>> No List recovered for [%s] \n", __func__, vname);
    }
    return sret;
}


char** VarMapUtils::getDefList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt)
{
    char** sret;

    char* dbsubs = nullptr;
    amap[vname] = setLinkVal(vmap, aname, "/config", vname, dbsubs);
    dbsubs = (char*)"/components, /assets, /params ";
    sret = getListStr(vecs, vmap, vname, ccnt, dbsubs);
    if (sret)
    {
        addListToVec(vecs, sret, aname, ccnt);
        showList(sret, aname, ccnt);
    }
    else
    {
        ccnt = 0;
        FPS_ERROR_PRINT(" %s >>>>>> No dbsubs recovered for [%s] \n", __func__, vname);
    }
    return sret;
}

//vecMap utils    template<class T>

// given a name and a type create or add an entry
template<class T>
void VarMapUtils::vecMapAddEntry(vecmap& vecm, const char* name, T val)
{
    auto ix = vecm.find(name);
    if (ix == vecm.end())
    {
        vecm[name] = (new std::vector<T>);
    }
    std::vector<T>* ve = vecm[name];
    ve->push_back(val);

}

void VarMapUtils::vecMapAddChar(vecmap& vecm, const char* name, const char* val)
{
    return vecMapAddEntry(vecm, name, std::string(val));
}
// user has to decode vector type
// vecMapAddEntry(vecm, "Subs",std::string("/assets"));
// void * ve = vecMapGetVec(vecm, "Subs");
// std::vector<std::string> ve = (std::vector<std::string>)*vx;

template<class T>
std::vector<T>* VarMapUtils::vecMapGetVec(vecmap& vecm, const char* name, T val)
{
    auto ix = vecm.find(name);
    if (ix != vecm.end())
    {
        return vecm[name];
    }
    return nullptr;
}

void VarMapUtils::testvecMap(void)
{
    vecmap vcmap;
    vecMapAddChar(vcmap, "test", "testchar1");
    vecMapAddChar(vcmap, "test", "testchar2");
    std::vector<std::string>* vx = vecMapGetVec(vcmap, "test", std::string("dummy"));
    int idx = 0;
    for (auto ix : *vx)
    {
        printf(" entry %d is [%s]\n", idx++, ix.c_str());
    }

}

void VarMapUtils::showvecMap(vecmap& vcmap, const char* key)
{
    //vecMapAddChar(vcmap,"test","testchar1");
    //vecMapAddChar(vcmap,"test", "testchar2");
    if (key)
    {
        std::vector<std::string>* vx = vecMapGetVec(vcmap, key, std::string("dummy"));
        int idx = 0;
        if (vx)
        {
            for (auto ix : *vx)
            {
                FPS_ERROR_PRINT("%s key [%s] > entry [%d] is [%s]\n", __func__, key, idx++, ix.c_str());
            }
        }
        else
        {
            FPS_ERROR_PRINT(" %s >> no entries for key [%s]", __func__, key);
        }
    }
    else
    {
        for (auto x : vcmap)
        {
            showvecMap(vcmap, x.first.c_str());
        }
    }


}
// do this for singles

cJSON* VarMapUtils::loadVmap(varsmap& vmap, int single, const char* comp, const char* var, const char* body)
{
    char* xsp = nullptr;
    cJSON* cjr = nullptr;
    if (single & 1)
    {
        if ((single && 0x0010) == 0)
        {
            asprintf(&xsp, "{\"%s\":{\"value\":%s}}", var, body);
        }
        else
        {
            asprintf(&xsp, "{\"%s\":%s}", var, body);
        }
        if(0)FPS_ERROR_PRINT(" %s >> running single value [%s]\n", __func__, xsp);
        processMsgSetPub(vmap, "set", comp, single, xsp, &cjr);
    }
    else
    {
        processMsgSetPub(vmap, "set", comp, single, body, &cjr);
    }

    if (xsp)free((void*)xsp);

    return cjr;
}

// if we supply a baseUri it needs to be removed from the component name 
cJSON* VarMapUtils::getVmap(varsmap& vmap, int& single, const char* key, const char* var, int opts, const char *baseUri)
{
    //varsmap vmr;
    //get matching components 
    // does not work yet getCompsVm(vmap, vmr, key, var);

    // Hack for assets
    if(strncmp(key, "/assets", strlen("/assets"))==0)
    {
        opts |= 0x0100;
    }

    if (process_fims_debug)FPS_ERROR_PRINT("%s >> RUNNING  key   [%s] var [%s] single 0x%04x opts 0x%04x baseUri %s\n"
            , __func__, key, var, single, opts, baseUri?baseUri:"No BaseURI");
    cJSON* cj = nullptr;

    //TODO merge single and opts
    if (single & 0x0001) 
    {
        if (opts & 0x0010)
        {
            opts &=~0x0001;
        }

        if (process_fims_debug)FPS_ERROR_PRINT("%s >> Running getMapsCj key   [%s] var [%s] opts 0x%04x\n", __func__, key, var, opts);
        cj = getMapsCj(vmap, key, var, opts);
    }
    else
    {
        // TODO dont like this
        if (single & 0x1000)
        {
            opts |= 0x0001;
        }
        if (process_fims_debug)FPS_ERROR_PRINT("%s >> Running getMapsCj key   [%s] var nullptr opts 0x%04x\n", __func__, key, opts);
        cJSON* cjall = getMapsCj(vmap, key, nullptr, opts);
        if (0)FPS_ERROR_PRINT("%s >> Ran key   [%s] var [%s] single 0x%04x cjall %p \n", __func__, key, var, single, (void*)cjall);

        if (cjall)
        {
            if (opts & 0x0001)
            {
                cj = cjall;
            }
            else
            {
                cJSON* cjd = cJSON_DetachItemFromObject(cjall, key);

                if (cjd)
                {
                    // char* tmpall = cJSON_PrintUnformatted(cjd);                  
                        // if(tmpall)
                    // {
                    //     if(0)FPS_ERROR_PRINT("%s >>  JUST DATA key   [%s] data [%s] \n", __func__, key, tmpall);
                    //     free((void*)tmpall);
                    // }
                    cj = cjd;
                    cJSON_Delete(cjall);

                }
                else
                {
                    cj = cjall;
                }

            }
        }

    }
    return cj;
}
// This is the UI map work. 
// do this for singles
cJSON* VarMapUtils::loadUimap(varsmap& vmap, int single, const char* comp, const char* var, const char* body)
{
    char* xsp = nullptr;
    cJSON* cjr = nullptr;
    // Dummy case ( I think)
    if (single & 0x0001)
    {
        if ((single && 0x0010) == 0)
        {
            asprintf(&xsp, "{\"%s\":{\"value\":%s}}", var, body);
        }
        else
        {
            asprintf(&xsp, "{\"%s\":%s}", var, body);
        }
        processMsgSetPubUi(vmap, "set", comp, single, xsp, &cjr);

    }
    else
    {
        processMsgSetPubUi(vmap, "set", comp, single, body, &cjr);
    }

    if (xsp)free((void*)xsp);

    return cjr;
}

cJSON* VarMapUtils::getUimap(varsmap& vmap, int& single, const char* key, const char* var)
{
    //varsmap vmr;
    //get matching components 
    // does not work yet getCompsVm(vmap, vmr, key, var);

    if (0)FPS_ERROR_PRINT("%s >> RUNNING  key   [%s] var [%s] single 0x%04x\n", __func__, key, var, single);
    int opts = 0;
    cJSON* cj = nullptr;
    // {
    //     cJSON* cjx = getMapsCj(vmap, key, var);

    //     char* tmp = cJSON_PrintUnformatted(cjx);
    //     if(0)FPS_ERROR_PRINT("%s >> full item   [%s] got [%p] tmp [%p]\n", __func__, key, (void *)cjx, (void *)tmp);
    //     if(tmp)
    //     {
    //         if(0)FPS_ERROR_PRINT("%s >> full item key   [%s] tmpall [%s] \n", __func__, key, tmp);
    //         free((void*)tmp);
    //     }
    //     cJSON * cjd = cJSON_DetachItemFromObject(cjx, key);
    //     if(cjd)
    //     {
    //         char* tmpall = cJSON_PrintUnformatted(cjd);

    //         if(tmpall)
    //         {
    //             if(0)FPS_ERROR_PRINT("%s >>  full item JUST DATA key   [%s] data [%s] \n", __func__, key, tmpall);
    //             free((void*)tmpall);
    //         }
    //     }

    //     cJSON_Delete(cjd);
    //     cJSON_Delete(cjx);
    // }
    if (single & 0x0001)
    {
        cj = getMapsCj(vmap, key, var);
    }
    else
    {
        // TODO dont like this
        if (single & 0x1000)
        {
            opts = 1;
        }
        cJSON* cjall = getMapsCj(vmap, key, nullptr, opts);
        if (cjall)
        {
            if (opts == 1)
            {
                cj = cjall;
            }
            else
            {
                cJSON* cjd = cJSON_DetachItemFromObject(cjall, key);

                if (cjd)
                {
                    // char* tmpall = cJSON_PrintUnformatted(cjd);                  
                        // if(tmpall)
                    // {
                    //     if(0)FPS_ERROR_PRINT("%s >>  JUST DATA key   [%s] data [%s] \n", __func__, key, tmpall);
                    //     free((void*)tmpall);
                    // }
                    cj = cjd;
                }
                cJSON_Delete(cjall);
            }
        }

    }
    return cj;
}

//int CheckReload(varsmap& vmap, varmap& amap, const char* aname, const char* fname, void* func = nullptr);

// standard check reload function
int VarMapUtils::CheckReload(varsmap& vmap, varmap& amap, const char* aname, const char* fname, void*func)
{
    int reload;
    assetVar* av = amap[fname];

    if (!av || (reload = av->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }
    if (reload == 0)
    {
        //reload = 0;
        amap[fname] = setLinkVal(vmap, aname, "/reload", fname, reload);
    }
    if(func)
    {
        setFunc(vmap, aname, fname, func);
    }

    return reload;
}
//setAlarm(varsmap &vmap,"/assets" ,"bms_1","alarms_1","Battery Voltage Alarm", 2);
int VarMapUtils::setAlarm(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* atext, int retval)
{
    char* body;
    char* auri;
    cJSON* cj = nullptr;
    asprintf(&body, "{\"value\":1,\"ui_type\":\"alarm\",\"options\":[{\"id\":%d,\"time\":%f,\"name\":\"%s\",\"return_value\":%d}]}"
        , alarmId++
        , get_time_dbl()
        , atext
        , retval
    );
    asprintf(&auri, "/%s/%s"
        , base
        , aname
    );

    if (0)FPS_ERROR_PRINT("%s >> Alarm uri [%s] Body  [%s]\n", __func__, auri, body?body:"aint got no body");

    if (body)
    {
        cj = cJSON_Parse(body);
    }
    if (auri && cj)
    {
        setValfromCj(vmap, auri, vname, cj, 1);
    }
    if (auri)free((void*)auri);
    if (body)free((void*)body);
    if (cj)cJSON_Delete(cj);
    return 0;
}
//av.sendAlarm(vmap,  "srcUri", "destAv","atype","msg, severity)
asset_log* VarMapUtils::sendAlarm(varsmap& vmap, const char* srcUri, const char* destUri, const char* atype, const char* msg, int severity)
{
    assetVar* srcAv = getVar(vmap, srcUri, nullptr);
    if (!srcAv)
    {
        int ival = 1;
        srcAv = makeVar(vmap, srcUri, nullptr, ival);
    }
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to make var for [%s]\n", __func__, srcUri);
        return nullptr;
    }
    return sendAlarm(vmap, srcAv, destUri, atype, msg, severity);
}

//av.sendAlarm(vmap, srcAv, "atype", "destAv", "msg", severity)
asset_log* VarMapUtils::sendAlarm(varsmap& vmap, assetVar* srcAv, const char* destUri, const char* atype, const char* msg, int severity)
{
    assetVar* destAv = getVar(vmap, destUri, nullptr);
    if (!destAv)
    {
        int ival = 1;
        destAv = makeVar(vmap, destUri, nullptr, ival);
    }
    if (0)FPS_ERROR_PRINT("%s >> Alarm dest Uri [%s] av %p\n", __func__, destUri,destAv);

    return srcAv->sendAlarm(destAv, atype, msg, severity);
}


int VarMapUtils::clearAlarm(varsmap& vmap, assetVar* srcAv, assetVar* destAv, const char* atype, const char* msg, int severity)
{
    return  srcAv->clearAlarm(destAv, atype);
}

int VarMapUtils::clearAlarm(varsmap& vmap, const char* srcUri, const char* destUri, const char* atype, const char* msg, int severity)
{
    assetVar* srcAv = getVar(vmap, srcUri, nullptr);
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to find srcVar for [%s]\n", __func__, srcUri);
        return -1;
    }
    return clearAlarm(vmap, srcAv, destUri, atype, msg, severity);
}

int VarMapUtils::clearAlarm(varsmap& vmap, assetVar* srcAv, const char* destUri, const char* atype, const char* msg, int severity)
{
    assetVar* destAv = getVar(vmap, destUri, nullptr);
    if (!destAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to find destVar for [%s]\n", __func__, destUri);
        return -1;
    }
    return clearAlarm(vmap, srcAv, destAv, atype, msg, severity);
}

int VarMapUtils::clearAlarms(varsmap& vmap, const char* destUri)
{
    assetVar* destAv = getVar(vmap, destUri, nullptr);
    if (!destAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to find destVar for [%s]\n", __func__, destUri);
        return -1;
    }
    return destAv->clearAlarms();
}


template <class T>
void VarMapUtils::setParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname, T val)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to make var for [%s]\n", __func__, suri.c_str());
        return;
    }
    return srcAv->setParam(pname, val);
}


int VarMapUtils::getiParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        int val = 1;
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to make var for [%s]\n", __func__, suri.c_str());
        return 0;
    }
    return srcAv->getiParam(pname);
}

double VarMapUtils::getdParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        double val = 1;
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to make var for [%s]\n", __func__, suri.c_str());
        return 0;
    }
    return srcAv->getdParam(pname);
}

bool VarMapUtils::getbParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;


    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        bool val = true;
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to make var for [%s]\n", __func__, suri.c_str());
        return 0;
    }
    return srcAv->getbParam(pname);
}
char* VarMapUtils::getcParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;


    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        char* val = (char*)"noVal";
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)FPS_ERROR_PRINT("%s >> falied to make var for [%s]\n", __func__, suri.c_str());
        return 0;
    }
    return srcAv->getcParam(pname);
}

int VarMapUtils::setAmFunc(varsmap &vmap,varmap &amap, const char* aname, fims* p_fims
            , asset_manager*am,const char*vname
                ,int (*func)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am))
{

    assetFunc* SetAmCmd =(assetFunc *) new assetFunc(aname);
    SetAmCmd    ->setupRamFunc(func, vmap, amap, aname, p_fims, am);
    if(!amap[vname]->extras)
    {
        amap[vname]->extras = new assetExtras;
    }
    amap[vname] ->extras-> SetFunc = (assetVar*)SetAmCmd;
    return 0;
}

// this is for the site interface
bool VarMapUtils::loadSiteMap(varsmap & vmap, cJSON* cjss)
{
    bool ret = false;
    cJSON* cji;
    cJSON* cjid;
    cJSON* cjuri;
    cJSON* cjregs = cJSON_GetObjectItem(cjss, "registers");
    if(cjregs)
    {
        FPS_ERROR_PRINT("%s >> found registers \n", __func__);
        cji = cjregs->child;
        while (cji)
        {
            FPS_ERROR_PRINT("%s >> found register item [%s]\n", __func__, cji->string);
            cji = cji->next;
        }
        cJSON* cjinput = cJSON_GetObjectItem(cjregs, "input_registers");
        cJSON* cjhold = cJSON_GetObjectItem(cjregs, "holding_registers");
        if(cjinput)
        {
            ret = true;
            if(0)FPS_ERROR_PRINT("%s >> found input registers type %d child %p\n", __func__, cjinput->type, cjinput->child);
            cji = cjinput->child;
            while (cji)
            {
                //char*tmp = nullptr;
                //tmp = cJSON_PrintUnformatted(cji);
                //FPS_ERROR_PRINT("%s >> found input register [%s]\n", __func__, tmp);
                cjid = cJSON_GetObjectItem(cji, "id");
                cjuri = cJSON_GetObjectItem(cji, "uri");
                if(cjid && cjuri && cjid->valuestring&&cjuri->valuestring)
                {
                    if(1) FPS_ERROR_PRINT("%s >> found input register id [%s] uri [%s]\n", __func__, cjid->valuestring, cjuri->valuestring);
                    int ival = 0;
                    //vm.
                    setVal(vmap,cjuri->valuestring, cjid->valuestring, ival);
                }
                //if(tmp)free(tmp);
                cji = cji->next;
            }
        }
        if(cjhold)
        {
            ret = true;
            if(0) FPS_ERROR_PRINT("%s >> found holding registers type %d child %p\n", __func__, cjinput->type, cjinput->child);
            cji = cjhold->child;
            while (cji)
            {
                //char*tmp = nullptr;
                //tmp = cJSON_PrintUnformatted(cji);
                //FPS_ERROR_PRINT("%s >> found input register [%s]\n", __func__, tmp);
                cjid = cJSON_GetObjectItem(cji, "id");
                cjuri = cJSON_GetObjectItem(cji, "uri");
                if(cjid && cjuri && cjid->valuestring&&cjuri->valuestring)
                {
                    if (1)FPS_ERROR_PRINT("%s >> found holding register id [%s] uri [%s]\n", __func__, cjid->valuestring, cjuri->valuestring);
                    int ival = 0;
                    //vm.
                    setVal(vmap, cjuri->valuestring, cjid->valuestring, ival);
                }
                //if(tmp)free(tmp);
                cji = cji->next;
            }
        }
    }
    return ret;
}

assetList* VarMapUtils::setAlist(varsmap& vmap, const char* uri)
{

    assetList* alist = nullptr;
    char* tmp = nullptr;
    // // no leading "/" so its hard to find 
    asprintf(&tmp, "_%s", uri);
    if (tmp)
    {
        alist = new assetList(tmp);
        FPS_ERROR_PRINT(" %s >> creating  alist [%s]\n", __func__, tmp);
        assetVar* av = makeVar(vmap, (const char*)tmp, (const char*)"assetList", alist);
        if (av)
        {
            av->aVar = (assetVar*)alist;
        }
        FPS_ERROR_PRINT(" %s >> creating  alist [%s] av[%p] alist[%p]\n", __func__, tmp, av, alist);
        free((void*)tmp);
    }
    return alist;
}
// get the asset list if we have one
assetList* VarMapUtils::getAlist(varsmap& vmap, const char* uri)
{
    assetList* alist = nullptr;
    char* tmp = nullptr;
    // // no leading "/" so its hard to find 
    asprintf(&tmp, "_%s", uri);
    if (tmp)
    {
        assetVar* av = getVar(vmap, tmp, "assetList");
        if (av)
        {
            alist = (assetList*)av->aVar;
        }
        if (0)FPS_ERROR_PRINT(" %s >> looking for alist [%s] av [%p] alist [%p]\n"
            , __func__
            , tmp
            , av
            , alist
        );

        free((void*)tmp);
    }
    return alist;
}

// Time stuff
long int VarMapUtils::get_time_us()
{
    long int ltime_us;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
    return ltime_us;
}

double VarMapUtils::get_time_dbl()
{
    return  (double)get_time_us() / 1000000.0 - g_base_time;
}

void VarMapUtils::set_base_time()
{
    g_base_time = get_time_dbl();
}

// do this before a var update
void VarMapUtils::setTime()
{
    g_setTime = get_time_dbl();
}
// do this before a var update
double VarMapUtils::getTime()
{
    return g_setTime;// = get_time_dbl();
}
// tnow = (time_t)(timeNow + g_base_time)
    // tm *ltm = localtime(&tnow);

    // const char* method = "set";
    // const char* replyto=nullptr;
    // const char* uri = "/asset/bms_1/date";

    // // Use asprintf, then free
    // char* body;
    // asprintf(&body, "{\"date_yr\":%d,\"date_mon\":%d,\"date_day\":%d,\"date_hr\"
    // :%d,\"date_min\":%d,\"date_sec\":%d}", ltm->tm_year + 1900, ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

time_t VarMapUtils::getTNow(double tNow)
{
    time_t tnow = time(nullptr); //(time_t)(tNow + g_base_time);
    return tnow;
}

varsmap* VarMapUtils::createVlist()
{
    varsmap* vl = new varsmap;
    return vl;
}

cJSON* VarMapUtils::getVmapCJ(varmap& vm)
{

    cJSON* cji = cJSON_CreateObject();

    for (auto iy : vm)
    {
        if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, iy.first.c_str());
        iy.second->showvarCJ(cji);
        if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, iy.first.c_str());
    }
    return cji;
}

bool VarMapUtils::notMissing(varmap& amap, const char* func, const char* vname)
{
    if (amap.find(vname) == amap.end())
    {
        FPS_ERROR_PRINT("%s  [%s] is Missing \n", func, vname);
        return false;
    }
    return true;
}

int VarMapUtils::vListAddVar(varsmap& vmap, assetVar* av, const char* comp)
{
    // TODO tidy up
    // allow /comp/c/v:name /components/ess/some/system:varname
    //if(vmap.find(av->comp) == vmap.end())
    if (av)
    {
        if (comp)
            vmap[comp][av->name] = av;
        else
            vmap[av->comp][av->name] = av;
    }
    return 0;
}

int VarMapUtils::addVlist(varsmap* vl, assetVar* av, const char* comp)
{
    vListAddVar(*vl, av, comp);
    return 0;
}
// new wrinkle send to another assetvar
int VarMapUtils::sendVlist(fims* p_fims, const char* method, varsmap* vl, bool sim, assetVar* avd)
{
    vListSendFims(*vl, method, p_fims, nullptr, sim, avd);
    return 0;
}

void VarMapUtils::clearVmapxx(varsmap& vmap, bool delAv)
{
    for (auto& x : vmap)
    {
        for (auto& y : vmap[x.first])
        {
            // only delete the vars you own..
            assetVar* av = y.second;

            if ((av->comp == x.first) && (av->name == y.first))
            {
                if(0)FPS_ERROR_PRINT("%s >> running .... YES deleting [%s:%s]\n"
                    , __func__
                    , x.first.c_str()
                    , y.first.c_str()
                );
                //vmap[x.first][y.first].clear();

                if(delAv)delete y.second;
            }
        }
        assetList* alist = getAlist(vmap, x.first.c_str());
        if(alist)
        {
            delete alist; 
        }
        vmap[x.first].clear();
    }
    vmap.clear();
}

void VarMapUtils::clearVlist(varsmap* vl)
{
    if (0)FPS_ERROR_PRINT("%s >> running ....\n", __func__);
    clearVmapxx(*vl, false);
    delete vl;
}

// These are to be removed
char* VarMapUtils::xpull_pfrag(const char* uri, int idx)
{
    char* sp = (char*)uri;
    while (idx)
    {
        if (*sp++ == '/') idx--;
    }

    char* comp_id = strdup(sp);
    char* comp_end = strstr(comp_id, "/");
    if (comp_end)
        *comp_end = 0;
    return comp_id;
}

char* VarMapUtils::xpull_first_uri(const char* uri, int n)
{
    char* comp = strdup(uri);
    char* sp = (char*)comp;
    sp++;
    while (*sp)
    {
        if (*sp++ == '/')
        {
            if (n > 1)
            {
                n--;
            }
            else
            {
                break;
            }

        }
    }
    if (*sp)
    {
        *--sp = 0;
    }
    return comp;
}

char* VarMapUtils::xpull_last_uri(const char* uri, int n)
{
    int nf = get_nfrags(uri);
    nf -= n;

    char* comp = nullptr;
    char* sp = (char*)uri;
    sp++;
    while (*sp)
    {
        if (*sp++ == '/')
        {
            if (nf > 1)
            {
                nf--;
            }
            else
            {
                //sp++;
                break;
            }
        }
    }
    if (*sp)
    {
        comp = strdup(sp);
    }
    return comp;
}

char* VarMapUtils::xpull_uri(const char* uri, int idx)
{
    char* comp = strdup(uri);
    char* sp = (char*)comp;
    while (*sp && idx)
    {
        if (*sp++ == '/') idx--;
    }
    if (*sp)
    {
        *--sp = 0;
    }
    return comp;
}

char* VarMapUtils::xpull_one_uri(const char* uri, int idx)
{
    char* comp = strdup(uri);
    char* res = nullptr;
    char* sp = (char*)comp;
    char* spt = nullptr;
    while (*sp && idx)
    {
        if (*sp++ == '/') idx--;
    }
    spt = sp;

    while (*sp && *sp != '/')
    {
        sp++;
    }

    if (*sp)
    {
        *sp = 0;
    }
    if (spt)
    {
        res = strdup(spt);
    }
    if (comp)free((void*)comp);
    return res;
}

int VarMapUtils::get_nfrags(const char* uri)
{
    int nfrags = 0;
    const char* sp = uri;
    while (*sp)
    {
        if (*sp++ == '/') nfrags++;
    }
    return nfrags;
}


// end of varmaputils not quite

void assetList::add(assetVar* av)
{
    if(0)FPS_ERROR_PRINT("%s >> 2 aList %p add av [%s] size %d\n",__func__, &aList, av->name.c_str(), (int)aList.size());

    unsigned int ix = aList.size();
    for (unsigned int i = 0; i < ix; i++)
    {
        if (aList[i] == av)
            return;
    }
    aList.push_back(av);
}
int assetList::size(void)
{
    return (int) aList.size();
}
assetVar* assetList::avAt(unsigned int ix)
{
    if(0)FPS_ERROR_PRINT("%s >> 1 aList %p ix %d \n",__func__, &aList, ix);
    //if(0)FPS_ERROR_PRINT("%s >> 1 name [%s] aList %p ix %d \n",__func__, name.c_str(), &aList, ix);
    if(0)FPS_ERROR_PRINT("%s >> 2 aList %p size %d\n",__func__, &aList, (int)aList.size());
    if (ix < aList.size())
        return aList.at(ix);
    return nullptr;
}

template <class T>
assetVar* VarMapUtils::makeVar(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetVar* av = nullptr;

    // todo use assetUri
    assetUri my(comp, var);

    if (my.Var)
    {
        av = new assetVar(my.Var, my.Uri, value);
        vmap[my.Uri][my.Var] = av;
        if(0)FPS_ERROR_PRINT("%s >> seeking compFunc for Av input av [%s:%s]\n", __func__, my.Uri, my.Var);
        void* compFunc = getFunc(vmap, "comp", my.Uri, av);//   go find this when we make the var
        if(compFunc)
        {
            if(!av->extras)
            {
                av->extras = new assetExtras;
            }
            av->extras->compFunc = compFunc;

            if(0)FPS_ERROR_PRINT("%s >> FOUND compFunc for Av %p input av [%s:%s]\n", __func__, av, av->comp.c_str(), av->name.c_str());
        }

        if (0)FPS_ERROR_PRINT(">> %s >> created [%s:%s] av %p\n"
            , __func__
            , my.Uri
            , my.Var
            , (void*)av
        );

    }
    else
    {
        if (1)FPS_ERROR_PRINT(">> %s >> FAILED [%s:%s]\n"
            , __func__
            , comp
            , var
        );
    }

    return av;
}
template <class T>
assetVar* VarMapUtils::setVal(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetUri my(comp,var);
    
    assetVar* av = nullptr;


    if (my.Var)
    {
        av = getVar(vmap, my.Uri, my.Var);
        if (!av)
        {
            av = makeVar(vmap, my.Uri, my.Var, value);
        }
        // now we need to do this

        if(my.Param)
        {
            if (0)FPS_ERROR_PRINT("%s >> running set Param for [%s:%s@%s] (av->extras %p)\n"
                    , __func__, my.Uri, my.Var, my.Param
                    , av->extras
                    );
            av->setParam(my.Param, value);
            return av;
        }

        av->setVal(value);
        assetVal*aVal = av->aVal;
        if(av->linkVar)
        {
            aVal = av->linkVar->aVal;
        }
        // TODO check isDIFF
        if (0 && av->extras )FPS_ERROR_PRINT("%s >> >> running  for [%s:%s@%s] (av->extras %p) compFunc %p\n"
                    , __func__, my.Uri, my.Var, my.Param
                    , av->extras
                    , av->extras?av->extras->compFunc:0
                    );
      
        if(av->extras)
        {
            if (av->extras->SetFunc)
            {
                if (0)FPS_ERROR_PRINT("%s >> running SetFunc for [%s:%s]\n", __func__, my.Uri, my.Var);
                setTime();
                assetFunc* af = (assetFunc*)av->extras->SetFunc;
                if (af->ramFunc)
                {
                    af->ramFunc(*af->vmap, *af->amap, af->aname, af->p_fims, af->am);
                }
                else if (af->raiFunc)
                {
                    af->raiFunc(*af->vmap, *af->amap, af->aname, af->p_fims, af->ai);
                }
                else if (af->ravFunc)
                {
                    af->ravFunc(*af->vmap, *af->amap, af->aname, af->p_fims, av);
                }

            }
            if (av->extras->actVec.size() > 0)
            {
                if (av->extras->actVec.find("onSet") != av->extras->actVec.end())
                {
                    if (0)FPS_ERROR_PRINT("%s >> After Setting  value for [%s] aval (float) %f  (int) %d (char) [%s]\n"
                        , __func__
                        , my.Var
                        , aVal->valuedouble
                        , aVal->valueint
                        , aVal->valuestring ? aVal->valuestring : "noval"
                    );

                    //TODO should  not force this
                    //aVal->valueint = (int)av->aVal->valuedouble;

                    if (0)FPS_ERROR_PRINT("%s >> setActValfrmoCj onSet value for [%s]\n", __func__, my.Var);
                    setActVecfromCj(vmap, av);//, cjact);
                }
            }
            if(av->extras->compFunc)
            {
                typedef int (*myCompFunc_t)(varsmap &vmap, assetVar* av);
                myCompFunc_t fcn = myCompFunc_t(av->extras->compFunc);
                fcn(vmap, av);
            }
        }
    }
    return av;
}

#include <chrono>
#include <ctime>
int forceTemplates()
{
    using namespace std::chrono;

   //ess_man = new asset_manager("ess_controller");
    varsmap vmap;
    VarMapUtils vm;
    asset_manager *am = new asset_manager("ess");
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

    //vm->setFunc(vmap, "ess", "run_init", (void*)&run_ess_init);
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
    system_clock::time_point now = system_clock::now();
    time_t tnow = system_clock::to_time_t(now);
    tm *local_tm = localtime(&tnow);
    char tbuffer[80];
    strftime (tbuffer,80,"%c.",local_tm);
    am->amap["timeString"]           = am->vm->setLinkVal(vmap, "ess", "/status", "timeString", tbuffer);
    return 0;
}

//Each assetvar can process logs.. Just like alarms but they are sent or a file object.
// a short buffer of say 32 entries are kept im memory
// amap["log"]->setLog(const char *fname, int memSize);
// amap["log"]->sendLog(vmap, av, varargs ...);
// amap["log"]->flushLog()
// amap["log"]->openLog(fname)
// amap["log"]->closePerf(fname)


// void PrintFError ( const char * format, ... )
// {
//   char buffer[256];
//   va_list args;
//   va_start (args, format);
//   vsprintf (buffer,format, args);
//   perror (buffer);
//   va_end (args);
// }
// void PrintFError ( const char * format, ... )
// {
//   char *buffer;
//   va_list args;
//   va_start (args, format);
//   rc = vsnprintf (buffer,0,format, args);
//   if rc > 0 buffer= (char *)malloc(rc); 
//   va_start (args, format);
//   vsnprintf (buffer, rc,format, args);
//   //perror (buffer);
//   va_end (args);
//}
#endif
