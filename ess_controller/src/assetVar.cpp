#ifndef ASSETVAR_CPP
#define ASSETVAR_CPP

#include "asset.h"
#include "assetVar.h"
#include "assetFunc.h"
#include "varMapUtils.h"
#include "formatters.hpp"

#define MAX_ALARM_SIZE 32
extern int setvar_debug;
extern int process_fims_debug;

extern double g_setTime;
extern int debug_action;
extern int setvar_debug_asset;

/******************************************************
 *
 *                 assetVar.h
 *
 ******************************************************/

/***********************************************
 *                 asset_log
 ***********************************************/
 static int log1count;
 static int log2count;
 
asset_log::asset_log() 
{
    srcAv = nullptr;
    destAv = nullptr;
    destIdx = -1;
    severity = -1;
    log_time = 0.0;
    aVal = nullptr;
    log1count++;

}

asset_log::asset_log(void* src, void* dest, const char* atype, const char* msg, int sev, double ltime)
{
    srcAv = src;
    destAv = dest;
    if (atype)altype = atype;
    almsg = msg;
    severity = sev;
    log_time = ltime;
    aVal = nullptr;
    log2count++;

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
    if (aVal) delete (assetVal*)aVal;
    //FPS_PRINT_INFO
    // auto xx = fmt::format(" asset alarm cleanup log_time [{:2.3f}] log1 {} log2"
    //     , log_time
    //     , log1count
    //     , --log2count
    //     );
    // printf("%s\n",xx.c_str());

    //FPS_PRINT_INFO("asset alarm cleanup");
}

/***********************************************
 *                 assFeat
 std::string name;
    //std::string fname;
    AFTypes type;
    double valuedouble;
    int valueint;
    char* valuestring;
    bool valuebool;
    assetVar *av;
    void* valuevoid;
 ***********************************************/
assFeat::assFeat(const char* _name, int val)
{
    valueint = val;
    valuedouble = val;
    type = AINT;
    name = _name;
    valuestring = nullptr;
    valuevoid = nullptr;
    valuebool = false;
    av = nullptr;
    lock = false;
    unlock = false;
}
assFeat::assFeat(const char* _name, double val)
{
    valuedouble = val;
    valueint = val;
    type = AFLOAT;
    name = _name;
    valuestring = nullptr;
    valuevoid = nullptr;
    valuebool = false;
    av = nullptr;
    lock = false;
    unlock = false;
}
assFeat::assFeat(const char* _name, bool val)
{
    valuebool = val;
    type = ABOOL;
    name = _name;
    valuestring = nullptr;
    valuevoid = nullptr;
    valuedouble = 0.0;
    valueint = 0;
    av = nullptr;
    lock = false;
    unlock = false;
}

assFeat::assFeat(const char* _name, const char* val)
{
    if(val)
    {
        valuestring = strdup(val);
    }
    else
    {
        valuestring = strdup("unknown");
        FPS_PRINT_ERROR(" Feat [{}] value unknown ", _name);

    }
    type = ASTRING;
    name = _name;
    valuevoid = nullptr;
    valuedouble = 0.0;
    valueint = 0;
    valuebool = false;
    av = nullptr;
    lock = false;
    unlock = false;
}
assFeat::assFeat(const char* _name, void* val)
{
    valuevoid = val;
    valuestring = nullptr;
    type = AVOID;
    name = _name;
    valuedouble = 0.0;
    valueint = 0;
    valuebool = false;
    av = nullptr;
    lock = false;
    unlock = false;
}
assFeat::assFeat(const char* _name, assetVar* val)
{
    valuestring = nullptr;//strdup(val);
    valuevoid = nullptr;
    type = AAVAR;
    name = _name;
    av = val;
    valuedouble = 0.0;
    valueint = 0;
    valuebool = false;
    lock = false;
    unlock = false;
}

assFeat::~assFeat()
{
    if (valuestring)free((void*)valuestring);
    // this may not work ...we may need to know the type
    if (valuevoid)free((void*)valuevoid);

}
assFeat::assFeat(const assFeat& other)
    : name(other.name), type(other.type), valuedouble(other.valuedouble), valueint(other.valueint), valuebool(other.valuebool), av(other.av), valuevoid(other.valuevoid)
{
    // if (valuestring)
    // {
    //     free((void*)valuestring);
    //     valuestring = nullptr;
    // }
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
        valuevoid = other.valuevoid;
        av = other.av;
        lock = other.lock;
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
assetFeatDict::assetFeatDict()
{
    tmpval = nullptr;
}

assetFeatDict::~assetFeatDict()
{
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
        FPS_PRINT_INFO("Feat >>{}<< child {}", cstr{ stmp }, fmt::ptr(cj->child));
        free((void*)stmp);
    }
    cJSON* cjic = cj->child;

    while (cjic)
    {
        if (0)
        {
            char* stmp2 = cJSON_PrintUnformatted(cjic);
            FPS_PRINT_INFO("stmp2 >>{}<< name [{}] child {} next {}"
                , stmp2
                , cstr{ cjic->string }
                , fmt::ptr(cjic->child)
                , fmt::ptr(cjic->next)
            );
            free((void*)stmp2);
        }
        //addFeat(cjic);
        if (uiObject
            && ((strcmp(cjic->string, "value") == 0)
                || (/*uiObject &&*/ (skipName && strcmp(cjic->string, "name") == 0))))
        {
            if (0) FPS_PRINT_INFO("skipping [{}] as a base param  its special", cstr{ cjic->string });
        }
        else
        {
            addFeat(cjic);
            // TODO after MVP review the cj outValue system do we use it ??
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
        FPS_PRINT_INFO("Features added");
        showFeat();
        FPS_PRINT_INFO("Features done");
    }
    return 0;
}


// test if we have a feature
//bool assetFeatDict::gotFeat(const char* name);
assFeat* assetFeatDict::getFeat(const char* name)
{
    if (featMap.find(name) != featMap.end())
    {
        return featMap[name];
    }
    return nullptr;
}

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

char* assetFeatDict::getcFeat(const char* name)
{
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        return af->valuestring;
    }
    return nullptr;
}

void* assetFeatDict::getFeat(const char* name, void** val)
{
    *val = nullptr;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        *val = af->valuevoid;
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
            // TODO after MVP get full data from an AAVAR Param not just the name
            if (af->av)
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

int assetFeatDict::getFeatType(const char* name)
{
    int featType = -1;
    if (featMap.find(name) != featMap.end())
    {
        assFeat* af = featMap[name];
        switch (af->type)
        {
        case assFeat::AINT:
            featType = assFeat::AINT;
            break;
        case assFeat::AFLOAT:
            featType = assFeat::AFLOAT;
            break;
        case assFeat::ASTRING:
            featType = assFeat::ASTRING;
            break;
        case assFeat::AAVAR:
            featType = assFeat::AAVAR;
            break;
        case assFeat::ABOOL:
            featType = assFeat::ABOOL;
            break;
        default:
            featType = -1;
        }
    }
    return featType;
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

void assetFeatDict::setFeat(const char* name, int idx, bool bval, assetVar* av)
{
    if (0) FPS_PRINT_INFO("name [{}] idx [{}] ", name, idx);
    if (featMap.find(name) == featMap.end())
    {
        int foo = 0;
        assFeat* af = nullptr;
        af = new assFeat(name, foo);
        if (af) featMap[name] = af;
    }
    if(av)
    {
        if(av->lock && !featMap[name]->unlock)
        {
            return;
        }
    }
    if(featMap[name]->lock) 
    {
        return;
    }

    if (bval)
    {
        featMap[name]->valueint |= (1 << idx);
    }
    else
    {
        featMap[name]->valueint &= ~(1 << idx);
    }

    if (0) FPS_PRINT_INFO("after name [{}] idx [{}] bval [{}] val {:#08x}"
        , name
        , idx
        , bval
        , featMap[name]->valueint
    );
}
void assetFeatDict::setFeat(const char* name, cJSON* cj, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = nullptr;
        if (cj->child) cj = cj->child;
        if (cJSON_IsTrue(cj) || cJSON_IsFalse(cj))
        {
            bool bval = cJSON_IsTrue(cj);
            af = new assFeat(name, bval);
        }
        else if (cJSON_IsString(cj))
        {
            af = new assFeat(name, cj->valuestring);
        }
        else if (cJSON_IsNumber(cj))
        {
            if (cj->valuedouble == (double)cj->valueint)
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
            FPS_PRINT_INFO("unmanaged cJSON string {} type {} child {}", cstr{ cj->string }, cj->type, fmt::ptr(cj->child));
        }

        if (af) featMap[name] = af;
    }
    else
    {
        if(av)
        {
            if(av->lock && !featMap[name]->unlock)
            {
                return;
            }
        }        
        if(featMap[name]->lock) 
        {
            return;
        }
        if (0) FPS_PRINT_INFO("name [{}] cJSON type {} child {}", name, cj->type, fmt::ptr(cj->child));
        if (cj->child) cj = cj->child;

        featMap[name]->valuedouble = cj->valuedouble;
        featMap[name]->valueint = cj->valueint;
        featMap[name]->valuebool = cJSON_IsTrue(cj);
        if (cj->valuestring)
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

void assetFeatDict::setFeat(const char* name, int idx, cJSON* cj, assetVar* av)
{
    if (0) FPS_PRINT_INFO("name [{}] idx [{}] cJSON type {}", name, idx, cj->type);
    if (featMap.find(name) == featMap.end())
    {
        int foo = 0;
        assFeat* af = nullptr;
        af = new assFeat(name, foo);
        if (af) featMap[name] = af;
    }
    if(av)
    {
        if(av->lock && !featMap[name]->unlock)
        {
            return;
        }
    }
    if(featMap[name]->lock) 
    {
        return;
    }

    if (cj->child) cj = cj->child;
    bool bval = false;
    if (cJSON_IsTrue(cj) || cJSON_IsFalse(cj))
    {
        bval = cJSON_IsTrue(cj);
        if (0) FPS_PRINT_INFO("before name [{}] idx [{}] bval [{}] val {:#08x}"
            , name
            , idx
            , bval
            , featMap[name]->valueint
        );

        if (bval)
        {
            featMap[name]->valueint |= (1 << idx);
        }
        else
        {
            featMap[name]->valueint &= ~(1 << idx);
        }

        if (0) FPS_PRINT_INFO("after name [{}] idx [{}] bval [{}] val {:#08x}"
            , name
            , idx
            , bval
            , featMap[name]->valueint
        );
    }
}

void assetFeatDict::setFeat(const char* name, assetVar* val, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);
        featMap[name] = af;
    }
    else
    {
        if(av)
        {
            if(av->lock && !featMap[name]->unlock)
            {
                return;
            }
        }
        if(featMap[name]->lock) 
        {
            return;
        }

        featMap[name]->av = val;
    }

    return;
}


void assetFeatDict::setFeatfromAv(const char* name, assetVar* av, const char* param)
{
    if(!av)
    {
        FPS_PRINT_ERROR("{}"," we need an av");
        return;

    }
    assetVar*realav =av->linkVar?av->linkVar:av;

    assetVal* aVal = realav->aVal;

    if(param)
    {
        if(0)FPS_PRINT_INFO("need to use the param [{}]",param);
        if(realav->extras->baseDict)
        {
            double dval = 0;
            assFeat* af = realav->extras->baseDict->getFeat(param);
            if(0)FPS_PRINT_INFO(" seeking param [{}] , got [{}]", param, fmt::ptr(af));
            if(af)
            {
                if(af->lock)
                {
                    return;
                }
                switch(af->type)
                {
                    case assFeat::AINT:
                        dval = (double) af->valueint;
                        setFeat(name, dval);
                        break;
                    case assFeat::AFLOAT:
                        dval = (double) af->valuedouble;
                        setFeat(name, dval);
                        break;
                    case assFeat::ABOOL:
                        setFeat(name, af->valuebool);
                        break;
                    case assFeat::ASTRING:
                        setFeat(name, af->valuestring);
                        break;
                    default:
                        FPS_PRINT_ERROR("DEFAULT Type  param [{}] name [{}] done", param, name);
                    break;
                }
                if(0)FPS_PRINT_INFO("Set name  [{}] from param  [{}] done", name, param);
            }
        }
        return;
    }

    //assFeat* af = nullptr;// new assFeat(name, aVal->valuedouble);
    double dval = 0;
    //int ival = 0;
    switch (aVal->type)
    {
    case assetVar::AINT:
        dval = (double) aVal->valueint;
        setFeat(name, dval);
        //setFeat(name, aVal->valueint);
        if(0)FPS_PRINT_INFO(" av [{}] INT value [{}]", av->getfName(), aVal->valueint);
        break;
    case assetVar::AFLOAT:
        //ival = (int)aVal->valuedouble;
        //setFeat(name, ival);
        setFeat(name, aVal->valuedouble);
        if(0)FPS_PRINT_INFO(" av [{}] FLOAT value [{}]", av->getfName(), aVal->valuedouble);
        break;
    case assetVar::ABOOL:
        setFeat(name, aVal->valuebool);
        //ival = (aVal->valuebool == true);
        //dval = (double)ival;
        //setFeat(name, dval);
        break;
    case assetVar::ASTRING:
        setFeat(name, aVal->valuestring);
        break;
    default:
        FPS_PRINT_ERROR("DEFAULT Type  name [{}]", name);
        break;
    }
    return;
}


void assetFeatDict::setFeat(const char* name, double val, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);
        featMap[name] = af;
    }
    if(av)
    {
        if(av->lock && !featMap[name]->unlock)
        {
            return;
        }
    }
    if(featMap[name]->lock) 
    {
        return;
    }

    featMap[name]->valuedouble = val;
    featMap[name]->valueint = val;

    return;
}
void assetFeatDict::setFeat(const char* name, int val, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        if (0) FPS_PRINT_INFO("set new int Feat [{}] value {}", name, val);
        assFeat* af = new assFeat(name, val);
        featMap[name] = af;
    }
    if(av)
    {
        if(av->lock && !featMap[name]->unlock)
        {
            return;
        }
    }
    if(featMap[name]->lock) 
    {
        return;
    }

    featMap[name]->valuedouble = val;
    featMap[name]->valueint = val;
 
    return;
}

void assetFeatDict::setFeat(const char* name, bool val, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);
        featMap[name] = af;
    }

    if(av)
    {
        if(av->lock && !featMap[name]->unlock)
        {
            return;
        }
    }
    if(featMap[name]->lock) 
    {
        return;
    }

    featMap[name]->valuebool = val;
    int ival = (val == true);
    featMap[name]->valueint = ival;
    featMap[name]->valuedouble = ival;

    return;
}

void assetFeatDict::setFeat(const char* name, const char* val, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);
        featMap[name] = af;
        if (0) FPS_PRINT_INFO("set up af >> {} valuestring {} [{}] for [{}] val [{}]"
            , fmt::ptr(af)
            , fmt::ptr(featMap[name]->valuestring)
            , featMap[name]->valuestring ? featMap[name]->valuestring : "No String"
            , name
            , val ? val : "no Val"
        );
    }
    else
    {
        if(av)
        {
            if(av->lock && !featMap[name]->unlock)
            {
                return;
            }
        }
        if(featMap[name]->lock) 
        {
            return;
        }

        // trap the "same" value update.
        char* nval = nullptr;
        if (featMap[name]->valuestring == val)
        {
            nval= strdup(val);
            val = nval;
        }


        if (featMap[name]->valuestring)
        {
            if (0) FPS_PRINT_INFO("clear up af >> {} valuestring {} [{}] for [{}] val [{}]"
                , fmt::ptr(featMap[name])
                , fmt::ptr(featMap[name]->valuestring)
                , featMap[name]->valuestring ? featMap[name]->valuestring : "No String"
                , name
                , val ? val : "no Val"
            );

            free((void*)featMap[name]->valuestring);
        }

        featMap[name]->valuestring = nullptr;
        if (val)
        {
            featMap[name]->valuestring = strdup(val);
            if (0) FPS_PRINT_INFO("reset af >> {} valuestring {} [{}] for [{}] val [{}]"
                , fmt::ptr(featMap[name])
                , fmt::ptr(featMap[name]->valuestring)
                , featMap[name]->valuestring ? featMap[name]->valuestring : "No String"
                , name
                , val ? val : "no Val"
            );

        }
        if(nval) free(nval);
    }

    return;
}

void assetFeatDict::setFeat(const char* name, void* val, assetVar* av)
{
    if (featMap.find(name) == featMap.end())
    {
        assFeat* af = new assFeat(name, val);
        featMap[name] = af;
    }
    if(av)
    {
        if(av->lock && !featMap[name]->unlock)
        {
            return;
        }
    }
    if(featMap[name]->lock) 
        {
            return;
        }

    featMap[name]->valuevoid = val;

    return;
}

template<class T>
void assetFeatDict::addFeat(const char* name, T val)
{
    if (featMap.find(name) != featMap.end())
    {
        if (featMap[name] != nullptr)
        {
            if (0) FPS_PRINT_INFO("note we are replacing param [{}] featmap {} valuestring {} [{}]"
                , name
                , fmt::ptr(featMap[name])
                , fmt::ptr(featMap[name]->valuestring)
                , featMap[name]->valuestring ? featMap[name]->valuestring : "no Val"
            );
            delete featMap[name];
        }
    }

    featMap[name] = new assFeat(name, val);
    if (0)FPS_PRINT_INFO("added  Feat [{}]  featmap {} valuestring {} [{}]"
        , name
        , fmt::ptr(featMap[name])
        , fmt::ptr(featMap[name]->valuestring)
        , featMap[name]->valuestring ? featMap[name]->valuestring : "no Val"
    );

}


void assetFeatDict::addFeat(cJSON* cj)
{
    switch (cj->type)
    {
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
    int len = 0;
    char* stmp = nullptr;
    for (auto x : featMap)
    {
        assFeat* af = x.second;
        switch (af->type)
        {
        case assFeat::AFLOAT:
            len = asprintf(&stmp, "%s->[%f]", af->name.c_str(), af->valuedouble);
            break;
        case assFeat::AINT:
            len = asprintf(&stmp, "%s->[%d]", af->name.c_str(), af->valueint);
            break;
        case assFeat::ASTRING:
            len = asprintf(&stmp, "%s->[%s]", af->name.c_str(), af->valuestring);
            break;
        case assFeat::ABOOL:
            if (af->valuebool)
                len = asprintf(&stmp, "%s->[true]", af->name.c_str());
            else
                len = asprintf(&stmp, "%s->[true]", af->name.c_str());
            break;
        default:
            len = asprintf(&stmp, "Unknown");
            break;
        }
        if (len > 0) FPS_DEBUG_PRINT(" Feature>>%s\n", stmp);
        if (stmp)free(stmp);
    }

}

void assetFeatDict::showCj(cJSON* cjix)
{
    char* tmp = nullptr;
    int len = 0;
    for (auto x : featMap)
    {
        assFeat* af = x.second;
        switch (af->type)
        {
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
            len = asprintf(&tmp, "av::%s/%s", af->av->comp.c_str(), af->av->name.c_str());
            cJSON_AddStringToObject(cjix, af->name.c_str(), tmp);
            if (len && tmp) free(tmp);
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
// assetBitField::assetBitField(int _mask, int _bit, const char* _uri, const char* _var, char* tmp)
// {
//     mask = _mask;
//     bit = _bit;
//     uri = strdup(_uri);
//     var = _var ? strdup(_var) : nullptr;
//     featDict = new assetFeatDict();
//     if (tmp)
//     {
//         featDict->tmpval = strdup(tmp);
//     }

// }

assetBitField::assetBitField(cJSON* cj)
{
    mask = 0;
    bit = 0;
    uri = nullptr;
    var = nullptr;
    
    enAv = nullptr;
    inAv = nullptr;
    inParam = nullptr;
    inAvOK = false;  // if we have an inaV and we can find it 
    ignAv = nullptr;
    outAv = nullptr;
    outParam = nullptr;
    invAv = nullptr;
    invParam = nullptr;
    varAv = nullptr;
    useAv =  false;
    
    avaf = nullptr;
    scale = 1.0;
    offset =  0.0;
    useSet = false;
    setup = false;
    //tmpval = nullptr;
    featDict = new assetFeatDict();
    if(cj)featDict->addCj(cj);
    amapptr = nullptr;
    fptr = nullptr;

}

assetBitField::~assetBitField()
{
    //if (uri)free((void*)uri);
    //if (var)free((void*)var);
    if(inParam) free((void*)inParam);
    if(invParam) free((void*)invParam);
    if(outParam) free((void*)outParam);
    delete featDict;

}

//template<class T>
int assetBitField::getFeat(const char* name, int* val)
{
    return featDict->getFeat(name, val);

}

double assetBitField::getFeat(const char* name, double* val)
{
    return featDict->getFeat(name, val);

}

bool assetBitField::getFeat(const char* name, bool* val)
{
    return featDict->getFeat(name, val);
}

char* assetBitField::getFeat(const char* name, char** val)
{
    return featDict->getFeat(name, val);
}

char* assetBitField::getcFeat(const char* name)
{
    return featDict->getcFeat(name);
}

void* assetBitField::getFeat(const char* name, void** val)
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

assFeat* assetBitField::getFeat(const char* name)
{
    return featDict->getFeat(name);
}

int assetBitField::getFeatType(const char* name)
{
    return featDict->getFeatType(name);
}

void assetBitField::setFeatfromAv(const char* name, assetVar* Av, const char*param)
{
    return featDict->setFeatfromAv(name, Av, param);
}

void assetBitField::setFeat(const char* name, double dval)
{
    return featDict->setFeat(name, dval);
}

void assetBitField::setFeat(const char* name, int dval)
{
    return featDict->setFeat(name, dval);
}

void assetBitField::setFeat(const char* name, bool dval)
{
    return featDict->setFeat(name, dval);
}

void assetBitField::setFeat(const char* name, const char* dval)
{
    return featDict->setFeat(name, dval);
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
    // 0817 stop loadconfig crash
    if(aname)
    {
        name = aname;
    }
    else
    {
        name = "UnKnown";
    }
    idx = 0;
}

assetAction::~assetAction()
{
    for (auto& x : Abitmap)
    {
        delete Abitmap[x.first];
    }
    Abitmap.clear();
}

// TODO after MVP add all assetAction  into a Feat Dict
assetBitField* assetAction::assetAction::addBitField(cJSON* cjbf)
{
    int idd = idx++;
    if (0)
    {
        char* tmp = cJSON_PrintUnformatted(cjbf);
        FPS_PRINT_INFO("Adding bitfield at [{}] [{}]", idd, cstr{ tmp });
        free((void*)tmp);
    }
    Abitmap[idd] = (new assetBitField(cjbf));
    return Abitmap[idd];
}

void assetAction::showBitField(int show)
{
    for (auto& x : Abitmap)
    {
        if (show)FPS_PRINT_INFO("Bitfield [{}] name [{}]"
            , x.first
            , name
        );
        assetBitField* abf = x.second;
        for (auto& y : abf->featDict->featMap)
        {
            if (show)FPS_PRINT_INFO("Bitfield [{}] feature[{}]"
                , x.first
                , y.first
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
    if (0)FPS_PRINT_INFO("3 delete aList {} {}", fmt::ptr(&aList), name);
}

const char* assetList::getName()
{
    return name.c_str();
}

/***********************************************
 *                 assetOptVec
 ***********************************************/
assetOptVec::assetOptVec()
{
    cjopts = nullptr;
    name = "options";
}

assetOptVec::~assetOptVec()
{
    if (cjopts)
    {
        cJSON_Delete(cjopts);
    }
}

void assetOptVec::showCj(cJSON* cj)
{
    cJSON* cjo = cJSON_Duplicate(cjopts, true);
    cJSON_AddItemToObject(cj, name.c_str(), cjo);
}

// we may need to get the child object
void assetOptVec::addCj(cJSON* cj)
{
    if (0)FPS_PRINT_INFO("adding options array [{}] child {} type {}"
        , cj->string ? cj->string : " no String"
        , fmt::ptr(cj->child)
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
    valuebool = false;
    valuestring = nullptr;
    av = nullptr;
}

assetVal::assetVal(int val) :assetVal()
{
    valuedouble = val;
    valueint = val;
}

assetVal::assetVal(double val) :assetVal()
{
    valuedouble = val;
    valueint = (int)val;
    type = AFLOAT;
}

assetVal::assetVal(const char* val) :assetVal()
{
    valuedouble = 0;
    valueint = 0;
    type = ASTRING;
}

assetVal::assetVal(bool val) :assetVal()
{
    valueint = (val == true);
    valuedouble = valueint;
    valuebool = val;
    type = ABOOL;
}

assetVal::assetVal(assetVar* val) :assetVal()
{
    av = val;
    type = AAVAR;
}

assetVal::~assetVal()
{
    if (valuestring != nullptr)
    {
        free((void*)valuestring);
    }

}

bool assetVal::getVal(bool val)
{
    val = valuebool;
    return val;
}

int assetVal::getVal(int val)
{
    val = valueint;
    return val;
}

double assetVal::getVal(double val)
{
    val = valuedouble;
    return val;
}

char* assetVal::getVal(char* val)
{
    val = valuestring;
    return val;
}

assetVar* assetVal::getVal(assetVar** val)
{
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
    valueint = val;
    valuedouble = val;
    setTime = g_setTime;
}

void assetVal::setVal(double val)
{
    valuedouble = val;
    valueint = (int)val;
    setTime = g_setTime;

}

void assetVal::setVal(bool val)
{
    if (0)FPS_PRINT_INFO("setVal bool called"
    );
    valueint = val;
    valuebool = val;
    setTime = g_setTime;
}

void assetVal::setVal(const char* val)
{
    const char* sval=nullptr;
    if (valuestring == val)
    {
        if (1)FPS_PRINT_INFO("setVal char  called same value  [{}]", val);
        sval = val;
        valuestring = nullptr;
    }

    if (valuestring)
    {
        free((void*)valuestring);
        valuestring = nullptr;
    }

    if (val)
    {
        valuestring = strdup(val);
    }
    if(sval)
    {
        free((void*)sval);
    }
    setTime = g_setTime;
}


void assetVal::setVal(char* val)
{
    if (valuestring)
    {
        free((void*)valuestring);
    }

    if (val)
    {
        valuestring = strdup(val);
    }
    setTime = g_setTime;
}

bool assetVal::setVal(cJSON* cj)
{
    if (0)FPS_PRINT_INFO("running, cjson type {}", cj->type);
    if (cJSON_IsBool(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO("body is a cjson bool value [{}]", cJSON_IsTrue(cj));
        setVal((bool)cJSON_IsTrue(cj));
        return true;
    }
    else if (cJSON_IsNumber(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO("body is a cjson numerical value [{}]", cj->valuedouble);
        setVal(cj->valuedouble);
        return true;
    }
    else if (cJSON_IsString(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO("body is a cjson string value [{}]", cstr{ cj->valuestring });
        setVal(cj->valuestring);
        return true;
    }
    else if (cJSON_IsObject(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO("body is a cjson Object try child [{}]", fmt::ptr(cj->child));
        return setVal(cj->child);
        return true;
    }
    else
    {
        FPS_PRINT_INFO("body [{}] type {} cannot be simply decoded", cstr{ cj->string }, cj->type);
    }
    return false;

}
// gets an indexed bit as a bool
bool assetVal::getbVal(int index)
{
    bool ret = false;
    if (0)FPS_PRINT_INFO("getbVal index {} {:#08x} called"
        , index
        , 1 << index
    );
    if (index < 0) return ret;
    if (index > 31) return ret;

    unsigned int nval = (1 << index);
    ret = ((unsigned int)valueint & nval) > 0;
    return ret;
}

// set a bool as an indexed bit
bool assetVal::setVal(int index, bool val)
{
    if (0)FPS_PRINT_INFO("setVal index {} {:#08x} bool [{}] called"
        , index
        , 1 << index
        , val
    );
    if (index < 0) return false;
    if (index > 31) return false;
    unsigned int nval = (1 << index);
    if (val)
    {
        valueint |= nval;
    }
    else
    {
        valueint &= ~nval;
    }
    valuedouble = (double)valueint;
    valuebool = val;
    setTime = g_setTime;
    return val;
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
    if (other.valuestring)
    {
        valuestring = strdup(other.valuestring);
    }
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

cJSON* assetVar::getValCJ()
{
    cJSON* cj = nullptr;
    assetVal* av = linkVar ? linkVar->aVal : aVal;
    if (av)
        cj = av->getValCJ();
    return cj;
}

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
    cval = 0;
    aVal = nullptr;
    lVal = nullptr;
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
    // removed for MVP 
    //users = 0;
    fname = nullptr;
    linkVar = nullptr;
    depth = 0;
    lock = false; //PSW

}


assetVar::assetVar(const char* _name, int val) :assetVar()
{
    name = _name;
    type = AINT;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    extras = nullptr;
}

assetVar::assetVar(const char* _name, double val) :assetVar()
{
    if (_name) name = _name;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    type = AFLOAT;
    extras = nullptr;

}

assetVar::assetVar(const char* _name, const char* val) :assetVar()
{
    name = _name;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    type = ASTRING;
    extras = nullptr;

}

assetVar::assetVar(const char* _name, bool val) :assetVar()
{
    name = _name;
    aVal = new assetVal(val);
    lVal = new assetVal(val);
    type = ABOOL;
    extras = nullptr;
}

assetVar::assetVar(const char* _name, cJSON* cjval) :assetVar()
{
    if (cJSON_IsNumber(cjval))
    {
        double vv = cjval->valuedouble;
        assetVar(_name, vv);
    }
    else if (cJSON_IsBool(cjval))
    {
        bool vv = cJSON_IsTrue(cjval);
        assetVar(_name, vv);
    }
    else if (cJSON_IsString(cjval))
    {
        const char* vv = cjval->valuestring;
        assetVar(_name, vv);
    }
    else
    {
        bool vv = false;
        assetVar(_name, vv);
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
    extras = nullptr;
    depth = 0;
}


const char* assetVar::getName(void)
{
    return name.c_str();
}

char* assetVar::getfName()
{
    if (!fname)
    {
        int len = 0;
        len = asprintf(&fname, "%s:%s", comp.c_str(), name.c_str());
        if (len) len = 0; // compiler      
    }
    return fname;
}

double assetVar::getLastSetDiff(double tnow)
{
    if (linkVar)
    {
        return linkVar->getLastSetDiff(tnow);
    }

    return tnow - aVal->setTime;
}

double assetVar::getLastChgDiff(double tnow)
{
    if (linkVar)
    {
        return linkVar->getLastChgDiff(tnow);
    }

    return tnow - aVal->chgTime;
}

double assetVar::getSetTime(void)
{
    if (linkVar)
    {
        return linkVar->getSetTime();
    }
    return aVal->setTime;
}

double assetVar::getChgTime(void)
{
    if (linkVar)
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

bool assetVar::getbVal(int index)
{
    if (0)FPS_PRINT_INFO("getVal called here");
    if (linkVar)
    {
        return linkVar->getbVal(index);
    }

    bool val = false;
    if (aVal)
        val = aVal->getbVal(index);
    return val;
}

template <class T>
T assetVar::getVal(T val)
{
    T nVal;
    if (!aVal)
    {
        FPS_PRINT_ERROR("-- missing aVal !!!!");
        aVal = new assetVal(val);
    }
    if (!lVal)
    {
        FPS_PRINT_ERROR("-- missing lVal !!!!");
        lVal = new assetVal(val);
    }

    if (linkVar)
    {
        nVal = linkVar->getVal(val);
    }
    else
    {
        // lock it here if we need to 
        nVal = aVal->getVal(val);
    }
    return nVal;
}

//template <class T>
int assetVar::addVal(int val)
{
    int nval;
    if (linkVar)
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

double assetVar::addVal(double val)
{
    // lock it here
    double nval;
    if (linkVar)
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
T assetVar::subVal(T val)
{
    // lock it here
    T nval = val;
    if (linkVar)
    {
        val = linkVar->subVal(val);
    }
    if (aVal->IsNumeric())
    {
        nval = aVal->getVal(nval);
        setVal(nval - val);
        val = (nval - val);
    }
    return val;
}

template <class T>
T assetVar::getLVal(T val)
{
    // lock it here
    T nVal;
    if (linkVar)
    {
        nVal = linkVar->getLVal(val);
    }
    else
    {
        // lock it here if we need to 
        nVal = lVal->getVal(val);
    }
    return nVal;
}

//template <class T>
bool assetVar::setVal(bool val)
{
    if (0)FPS_PRINT_INFO("setVal called here"
    );
    if (linkVar)
    {
        return linkVar->setVal(val);
    }
    if(lock)
    {
        return false;
    }
    bool mydiff = false;
    mydiff = valueIsDiff(val);
    if (0)FPS_PRINT_INFO("setVal called here diff {}"
        , mydiff
    );

    if (mydiff)
    {
        // bugfix 08162021   maintain type 
        lVal->type = aVal->type;
        assetVal* tv = lVal;
        lVal = aVal;
        aVal = tv;
        aVal->setVal(val);
        valChanged = true;
        IsDiff = true;
        aVal->chgTime = aVal->setTime;
    }

    return mydiff;
}

bool assetVar::setVal(int index, bool val)
{
    if (0)FPS_PRINT_INFO("setVal called here"
    );
    if (linkVar)
    {
        return linkVar->setVal(index, val);
    }
    if(lock)
    {
        return false;
    }
    if (aVal)aVal->setVal(index, val);

    return val;
}

bool assetVar::setVal(int val)
{
    if (0)FPS_PRINT_INFO("setVal called here"
    );
    if (linkVar)
    {
        return linkVar->setVal(val);
    }
    if(lock)
    {
        return false;
    }
    bool diff = valueIsDiff(val);
    if (0)FPS_PRINT_INFO("setVal called here diff {}"
        , diff
    );
    if (diff)
    {
        // bugfix 08162021   maintain type 
        lVal->type = aVal->type;
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

bool assetVar::setVal(double val)
{
    if (0)FPS_PRINT_INFO("setVal double called here"
    );
    if (linkVar)
    {
        return linkVar->setVal(val);
    }
    if(lock)
    {
        return false;
    }
    if (depth)
    {
        setVecVal(val, depth);
        //vdepth
    }
    bool diff = valueIsDiff(val);
    if (0)FPS_PRINT_INFO("setVal diff {}"
        , diff
    );
    if (diff)
    {
        // bugfix 08162021   maintain type 
        lVal->type = aVal->type;
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

bool assetVar::setVal(const char* val)
{
    if(!lVal->valuestring)
    {
        if (1)FPS_PRINT_INFO("setVal setting lVal->valuestring to  [{}]", val);
        lVal->valuestring = strdup (val);
    }
    if(!aVal->valuestring)
    {
        if (1)FPS_PRINT_INFO("setVal setting aVal->valuestring to  [{}]", val);
        aVal->valuestring = strdup (val);
    }

    if (0)FPS_PRINT_INFO("setVal called here  ; av [{}] val [{}] aval [{}] lastval [{}]"
                , getfName()
                , val
                , aVal->valuestring?aVal->valuestring:"no aval" 
                , lVal->valuestring?lVal->valuestring:"no lval" 
                );
    if (linkVar)
    {
        return linkVar->setVal(val);
    }
    if(lock)
    {
        return false;
    }
    bool diff = valueIsDiff(val);
    if (0)FPS_PRINT_INFO("setVal av [{}] called here diff {}", getfName(), diff);
    if (diff)
    {
        // fix for same value pointer update.
        char* nval = nullptr;
        if(lVal->valuestring == val)
        {
            if (1)FPS_PRINT_INFO("setVal av [{}] called here val [{}] lval [{}] "
                , getfName(), val, lVal->valuestring);
            nval = strdup(val);
            val = nval;
        }

        // bugfix 08162021   maintain type 
        lVal->type = aVal->type;
        assetVal* tv = lVal;
        lVal = aVal;
        aVal = tv;
        aVal->setVal(val);
        valChanged = true;
        IsDiff = true;
        aVal->chgTime = aVal->setTime;
        if(nval) free(nval);

    }

    return diff;
}

template <class T>
void assetVar::setFimsVal(T val, fims* p_fims, const char* acomp)
{
    if (p_fims)
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
        {
            auto rc  = p_fims->Send("pub", acomp, nullptr, res);
            if (rc <= 0)
            {
                FPS_PRINT_ERROR("send event failed rc = {}", rc);
                exit(0);
            }
        }
        free(res);
    }
    return;

}
// we dont test against the lval and val we simple see if we have had a setval since last reset
// this tests if we have had a value change since the last value reset.
// this is done at the assetVar level now
//template < class T>
bool assetVar::valueChanged(int reset)
{
    if (linkVar)
    {
        return linkVar->valueChanged(reset);
    }
    bool ret = valChanged;
    if (reset == 1)
        valChanged = false;
    return ret;
}

bool assetVar::valueChangedReset()
{
    if (linkVar)
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
    if (linkVar)
    {
        return linkVar->resetChanged(val);
    }
    valChanged = false;
}


//template <class T>
bool assetVar::valueIsDiff(bool val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(val);
    }

    return valueIsDiff(dbV, val);
}

bool assetVar::valueIsDiff(int val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(val);
    }

    return valueIsDiff(dbV, val);
}

bool assetVar::valueIsDiff(double val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(dbV, val);
    }

    return valueIsDiff(dbV, val);
}

bool assetVar::valueIsDiff(const char* val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(val);
    }

    return valueIsDiff(dbV, val);
}

bool assetVar::valueIsDiff(double db, const char* val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }

    if (!aVal->valuestring && !val) return false;
    if (aVal->valuestring && !val) return true;
    if (!aVal->valuestring && val) return true;
    if (0) FPS_PRINT_INFO("assetVar.h old [{}] aVal [{}] aValvs [{}]"
        , val, fmt::ptr(aVal), fmt::ptr(aVal->valuestring)
    );
    if (0) FPS_PRINT_INFO("assetVar.h old [{}] new [{}] strcmp [{}] diff [{}]"
        , val, aVal->valuestring
        , strcmp(aVal->valuestring, val)
        , strcmp(aVal->valuestring, val) != 0

    );
    return (strcmp(aVal->valuestring, val));
}

bool assetVar::valueIsDiff(double db, bool val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }
    return(aVal->valuebool != val);
}

bool assetVar::valueIsDiff(double db, int val)
{
    if (linkVar)
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
    if (linkVar)
    {
        return linkVar->valueIsDiff(db, val);
    }
    // auto diff = db ? db->valuedouble : DEF_DEADBAND;
    if (!aVal)
    {
        FPS_PRINT_INFO("Hmm no Aval for [{}] maybe we need one", name);
        //return true;
    }
    if (std::abs(aVal->valuedouble - val) > db)
    {
        return true;
    }
    return false;
}

template <class T>
T assetVar::valueDiff(T val)
{
    if (linkVar)
    {
        return linkVar->valueIsDiff(val);
    }
    T nval = val;
    if (!aVal->IsString())
        val = lVal->getVal(val) - aVal->getVal(val);

    return nval;
}

//Set a deadband for floats
void assetVar::setDbVal(double val)
{
    if (linkVar)
    {
        return linkVar->setDbVal(val);
    }
    dbV = val;
}

double assetVar::getDbVal() {
    if(linkVar)
    {
        return linkVar->getDbVal();
    }
    return dbV;
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
    return ax;
}

template <class T>
void assetVar::setLVal(T val)
{
    if (linkVar)
    {
        linkVar->setLVal(val);
    }

    lVal->setVal(val);
    IsDiff = true;
}


void assetVar::setVal(assFeat*af, bool force)
{
    assetVal*lval = linkVar?linkVar->aVal:aVal;
    if(lock)
    {
        return;
    }
    switch (af->type)
    {
        case assFeat::AINT:
            if(force)lval->type=assetVal::AINT;
            setVal(af->valueint);
            break;
        case assFeat::AFLOAT:
            if(force)lval->type=assetVal::AFLOAT;
            setVal(af->valuedouble);
            break;
        case assFeat::ABOOL:
            if(force)lval->type=assetVal::ABOOL;
            setVal(af->valuebool);
            break;
        case assFeat::ASTRING:
            if(force)lval->type=assetVal::ASTRING;
            setVal(af->valuestring);
            break;
        default:
            setVal(af->valuedouble);
            break;
    }    
}

bool assetVar::setCjVal(cJSON* cj, bool forceType)
{
    // these are all good for linkVars
    if (0) FPS_PRINT_INFO("setCjVal called for av [{}]", getfName());
    if (cJSON_IsBool(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO(
            "body child is a cjson bool value [{}]"
            , cJSON_IsTrue(cj));
        //if(forceType) 
        return setVal((bool)cJSON_IsTrue(cj));
    }
    else if (cJSON_IsNumber(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO("body child is a cjson numerical value [{}]", cj->valuedouble);
        return setVal(cj->valuedouble);
    }
    else if (cJSON_IsString(cj))
    {
        if (setvar_debug_asset)FPS_PRINT_INFO("body child is a cjson string value [{}]", cstr{ cj->valuestring });
        return setVal(cj->valuestring);
    }
    else
    {
        FPS_PRINT_INFO("body child [{}] cannot be simply decoded", cstr{ cj->string });
    }
    return false;
}
// bool assetVar::setCjVal(cJSON* cj)
// {
//     // these are all good for linkVars
//     if (0)FPS_PRINT_INFO(" %s >> setCjVal  called here \n"
//         , __func__
//     );
//     if (cJSON_IsBool(cj))
//     {
//         if (setvar_debug_asset)FPS_PRINT_INFO("%s >>   body child is a cjson bool value [%s]\n", __func__, cJSON_IsTrue(cj) ? "true" : "false");
//         return setVal((bool)cJSON_IsTrue(cj));
//     }
//     else if (cJSON_IsNumber(cj))
//     {
//         if (setvar_debug_asset)FPS_PRINT_INFO("%s >> body child is a cjson numerical value [%f]\n", __func__, cj->valuedouble);
//         return setVal(cj->valuedouble);
//     }
//     else if (cJSON_IsString(cj))
//     {
//         if (setvar_debug_asset)FPS_PRINT_INFO("%s >> body child is a cjson string value [%s]\n", __func__, cj->valuestring);
//         return setVal(cj->valuestring);
//     }
//     else
//     {
//         FPS_PRINT_INFO("%s >> body child [%s] cannot be simply decoded\n", __func__, cj->string);
//     }
//     return false;
// }

void assetVar::getCjVal(cJSON** cj)
{
    if (linkVar)
    {
        linkVar->getCjVal(cj);
    }

    if (*cj == nullptr)
        *cj = cJSON_CreateObject();
    cJSON* cjv = *cj;

    switch (aVal->type)
    {
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
            FPS_PRINT_INFO("ASTRING name [{}] type {} atype {} valstr [{}]"
                , name
                , type
                , aVal->type
                , cstr{ aVal->valuestring }
            );
        }
        cJSON_AddStringToObject(cjv, name.c_str(), aVal->valuestring ? aVal->valuestring : "No VAL");
        if (0)
        {
            FPS_PRINT_INFO("ASTRING name [{}] done", name);
        }
        break;

    default:
        if (1)
        {
            FPS_PRINT_INFO("DEFAULT name [{}] done", name);
        }
        break;
    }
}

// TODO after MVP Rework single and options for showVarcJ  to make more sense
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

void assetVar::showvarCJ(cJSON* cj, int opts, const char* showas)
{
    if (0) FPS_PRINT_INFO(">>>>>>> name [{}] comp [{}] av {} extras {} linkvar {} type {} atype {} opts {:#04x} ui_type {} naked [{}]"
        , name.c_str()
        , comp.c_str()
        , fmt::ptr(this)
        , fmt::ptr(extras)
        , fmt::ptr(linkVar)
        , type
        , aVal->type
        , opts
        , ui_type
        , setNaked
    );
    if(!showas)
    {
        showas = name.c_str();
    }
    cJSON* cjv = cJSON_CreateObject();
    if (setNaked) opts |= 0x0001;
    if (linkVar)
    {
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
        if (0) FPS_PRINT_INFO(">>>>> showvarExtras");
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
    else if (opts & 0x10000) // assets
    {
        showvarAlarmsCJ(cjv, opts);
    }
    // TODO after MVP get showvarCJ to work properly for naked stuff
    if (opts & 0x0001)
    {
        cJSON* cjii = cJSON_Duplicate(cjv->child, true);
        cJSON_AddItemToObject(cj, showas, cjii);
        cJSON_Delete(cjv);
    }
    else
    {
        cJSON_AddItemToObject(cj, showas, cjv);
    }

    return;
}

cJSON* assetVar::getAction(assetAction* aact)
{
    if (debug_action) FPS_PRINT_INFO(">>>>> get action start, name >>{}<<--", aact->name);

    cJSON* cj = cJSON_CreateObject();
    cJSON* cja = cJSON_CreateArray();
    for (auto& x : aact->Abitmap)
    {
        cJSON* cjix = cJSON_CreateObject();
        if (debug_action)FPS_PRINT_INFO(">>>>> get action bitfield start abf [{}]<<--", x.first);

        assetBitField* abf = x.second;
        char* uri = abf->getcFeat("uri");
        char* var = abf->getcFeat("var");

        if (debug_action)FPS_PRINT_INFO(">>>>> get action bitfield uri {} var {}<<--"
            , cstr{ uri }
            , cstr{ var }
        );
        //int fm = 0;
        // use featmap
        abf->featDict->showCj(cjix);

        cJSON_AddItemToArray(cja, cjix);
        if (debug_action)FPS_PRINT_INFO("<<<<<<< get action var [{}] bitfield done <<--{}", cstr{ abf->var });
    }

    cJSON_AddItemToObject(cj, aact->name.c_str(), cja);

    if (0) FPS_PRINT_INFO("<<<<<<< get action done <<--");
    return cj;

}

assetVar::~assetVar()
{
    if (0)FPS_PRINT_INFO("DELETING av [{}:{}]"
        , comp
        , name
    );

    if (aVal) delete aVal;
    if (lVal) delete lVal;
    if (extras)
        delete extras;
    if (fname)free(fname);
}

// still need to split this up
void assetVar::showvarExtrasCJ(cJSON* cjv, int opts)
{
    if (0) if (1 || debug_action) FPS_PRINT_INFO(">>>>> looking for extras {} av {} [{}] opts {:#04x} link {}"
        , fmt::ptr(extras), fmt::ptr(this), name.c_str(), opts, fmt::ptr(linkVar));

    if (extras)
    {
        if (0) FPS_PRINT_INFO("<<<<< OpeVec {} name [{}]"
            , fmt::ptr(extras->optVec)
            , extras->optVec ? extras->optVec->name : "No Optvec name"
        );

        if (!extras->actVec.empty())
        {
            cJSON* cjact = cJSON_CreateObject();
            for (auto& x : extras->actVec)
            {
                cJSON* cjactar = cJSON_CreateArray();

                for (auto& y : x.second)
                {
                    cJSON* cja = getAction(y);
                    if (0 || debug_action) FPS_PRINT_INFO("<<<<< action Vec got cj  -->{}<-- cja {}", x.first, fmt::ptr(cja));

                    if (cja)
                    {
                        if (debug_action) FPS_PRINT_INFO(">>>>> OK action found");

                        char* tmp = cJSON_PrintUnformatted(cja);
                        if (tmp)
                        {
                            if (0) FPS_PRINT_INFO(">>>>> show action found -->{}<<--", fmt::ptr(tmp));
                            free((void*)tmp);
                        }
                        cJSON_AddItemToArray(cjactar, cja);
                    }
                    else
                    {
                        FPS_PRINT_INFO(">>>>> HMMM no action found");
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
        // add display of options
        if (extras->optVec && extras->optVec->cjopts)
        {
            cJSON* cjfeat = cJSON_Duplicate(extras->optVec->cjopts, true);
            cJSON_AddItemToObject(cjv, extras->optName.c_str(), cjfeat);
        }

        if (extras->optDict)
        {
            cJSON* cjfeat = cJSON_CreateObject();
            extras->optDict->showCj(cjfeat);
            cJSON_AddItemToObject(cjv, extras->optName.c_str(), cjfeat);

        }
        if (extras->useAlarms)
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
}

// still need to split this up
void assetVar::showvarAlarmsCJ(cJSON* cjv, int opts)
{
    // this becomes a class
    if (extras)
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

    if (extras)
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

}

// runs on alarm dest
void assetVar::setAlarm(asset_log* avAlarm)
{
    if (!extras)
    {
        extras = new assetExtras;
    }
    if (0) FPS_PRINT_INFO("setting alarm in [{}] msg [{}]"
                , getfName()
                , avAlarm->almsg
                );
// TODO after MVP possibly search for alarm already posted
    int alsize = (int)extras->alarmVec.size();
    // TODO getParam alarmsize
    int alsizeMax = MAX_ALARM_SIZE;
    if(gotParam("MaxAlarmSize"))
    {
        alsizeMax = getiParam("MaxAlarmSize");
    }
    if (alsize > alsizeMax)
    {
        //setvecval
        if(1) FPS_PRINT_INFO("limiting alarm size");
        auto aval = extras->alarmVec.front();
        extras->alarmVec.erase(extras->alarmVec.begin());
        delete aval;
    }

    extras->alarmVec.push_back(avAlarm);
    extras->useAlarms = true;
    setVal(1);
    return;
}

asset_log* assetVar::getAlarm(assetVar* srcAv, const char* atype, int num)
{
    if (extras)
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
    if (extras)
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
    if (extras)
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
            FPS_PRINT_INFO("bad alarm index {}", avAlarm->destIdx);
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
    if (extras)
    {
        extras->useAlarms = true;
        // notify the srcAv that its disconnected from the dest.
        for (auto x : extras->alarmVec)
        {
            if (x)
            {
                x->destAv = nullptr;
                delete x;
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
    if (linkVar)
    {
        linkVar->showvarValueOnlyCJ(cj, opts);
    }
    else
    {
        showvarValueOnlyCJ(cj, opts);
    }

    if (opts & 0x0010)  // extras
    {
        if (0) FPS_PRINT_INFO(">>>>>>> opts {:#04x} name [{}] type {} atype {}"
            , opts
            , name
            , type
            , aVal->type
        );
        // just add the basedict for now
        if (extras)
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
        FPS_PRINT_INFO(">>>>>>> name [{}] comp [{}] type {} aval {} opts {}"
            , name
            , comp
            , type
            , fmt::ptr(aVal)
            , opts
        );
        FPS_PRINT_INFO(">>>>>>> name [{}] type {} atype {}"
            , name
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
        
        switch (aVal->type)
        {
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
                FPS_PRINT_INFO("ASTRING name [{}] type {} atype {} valstr [{}]"
                    , name
                    , type
                    , aVal->type
                    , cstr{ aVal->valuestring }
                );
            }
            cJSON_AddStringToObject(cjv, "value", aVal->valuestring ? aVal->valuestring : "No VAL");
            if (0)
            {
                FPS_PRINT_INFO("ASTRING name [{}] done", name);
            }
            break;
        default:
            if (1)
            {
                FPS_PRINT_INFO("<<<<<< DEFAULT name [{}] done", name);
            }
            break;
        }
    }
}

void assetVar::setcParam(const char* pname, const char* val)
{
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp
            , name
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, (char*)val);
}

void assetVar::setParam(const char* pname, bool val)
{
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0)FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp
            , name
            , fmt::ptr(extras)
        );

    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParam(const char* pname, int val)
{
    if (0)FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0)FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp.c_str()
            , name.c_str()
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParam(const char* pname, double val)
{
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp.c_str()
            , name.c_str()
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParam(const char* pname, char* val)
{
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp
            , name
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

//sadly void* also services cJSON
void assetVar::setParam(const char* pname, void* val)
{
    if (0) FPS_PRINT_INFO("void* called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp.c_str()
            , name.c_str()
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeat(pname, val);
}

void assetVar::setParamfromCj(const char* pname, cJSON* val)
{
    if (0) FPS_PRINT_INFO("cj called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp.c_str()
            , name.c_str()
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    assetFeatDict* afd = extras->baseDict;
    assFeat* af = afd->getFeat(pname);
    if(af)
    {
        if((lock && !af->unlock) || af->lock) //if assetVar is locked but parameter is unlocked, should still allow parameter to set. 
        {
            if(1)FPS_PRINT_INFO("Parameter [{}] not set, assetvar is locked\n",pname);
            return;
        }
    }
    else if(lock)
    {
        if(1)FPS_PRINT_INFO("Parameter [{}] not set, assetvar is locked\n",pname);
        return;
    }
    afd->setFeat(pname, val);
}

void assetVar::setParamfromAv(const char* pname, assetVar* av)
{
    if (0) FPS_PRINT_INFO("called here pname [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp.c_str()
            , name.c_str()
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    extras->baseDict->setFeatfromAv(pname, av);
}

void assetVar::setParamIdxfromCj(const char* pname, int idx, cJSON* val)
{
    if (0) FPS_PRINT_INFO("cj called here name [{}]"
        , pname
    );
    if (!extras)
    {
        extras = new assetExtras;
        if (0) FPS_PRINT_INFO("called here pname [{}] av {} comp [{}] name [{}] extras {}"
            , pname
            , fmt::ptr(this)
            , comp
            , name
            , fmt::ptr(extras)
        );
    }
    if (!extras->baseDict)
        extras->baseDict = new assetFeatDict;
    if (idx >= 0)
    {
        extras->baseDict->setFeat(pname, idx, val);
    }
    else
    {
        extras->baseDict->setFeat(pname, val);
    }
}

int assetVar::getiParam(const char* pname) const
{
    int val = 0;
    if (0)FPS_PRINT_INFO("called here name [{}]"
        , pname
    );

    if (extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}

double assetVar::getdParam(const char* pname) const
{
    double val = 0.0;
    //char* fname = (char *)((assetVar*)this->getfName());
    if (0) FPS_PRINT_INFO("av name [{}:{}] param [{}] extras {}"
        , comp
        , name
        , pname
        , fmt::ptr(extras)
    );
    if (extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}

bool assetVar::getbParam(const char* pname) const
{
    bool val = false;
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}

char* assetVar::getcParam(const char* pname) const
{
    char* val = nullptr;
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);

    return val;
}

cJSON* assetVar::getCjParam(const char* pname, int options)
{
    cJSON* val = nullptr;
    if (extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    // check for naked bit
    if (val)
    {
        if (!options || (options & 1))
        {
            cJSON* cj = cJSON_GetObjectItem(val, "value");
            if (cj)
            {
                cj = cJSON_DetachItemFromObject(val, "value");
                cJSON_Delete(val);
                val = cj;
            }
        }
    }
    return val;
}

void* assetVar::getvParam(const char* pname)
{
    void* val;
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (extras && extras->baseDict)
        extras->baseDict->getFeat(pname, &val);
    return val;
}

char* assetVar::getcAParam(const char* pname)
{
    char* val = nullptr;
    assetVar* av;
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (extras && extras->baseDict)
    {
        extras->baseDict->getFeat(pname, &av);
        if (av)
        {
            val = av->getcVal();
        }
    }
    return val;
}

bool assetVar::gotParam(const char* pname)
{
    bool val = false;
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );

    if (extras && extras->baseDict)
        val = extras->baseDict->gotFeat(pname);
    return val;
}

int assetVar::setVecDepth(int vdepth)
{
    double adval = 0.0;
    if (!extras)
    {
        extras = new  assetExtras;
    }

    // clear it all out
    while (vdepth > 0 && ((int)extras->valVec.size() >= vdepth))
    {
        adval = extras->valVec.front();
        extras->valVec.erase(extras->valVec.begin());
        if ((int)extras->valVec.size() >= extras->valDepth)
        {
            FPS_PRINT_INFO(" changing vec size  [{}] vec size {} valDepth {} adval {:2.3f}"
            , cstr{getfName()}
            , extras->valVec.size()
            , extras->valDepth
            , adval
            );
        }
    }

    if (vdepth >= 0)
    {
        extras->valDepth = vdepth;
        depth = vdepth;
        setParam("depth",depth);
    }
    return depth;
}

int assetVar::getVecDepth()
{
    return depth;
}

double assetVar::setVecVal(double dval, int vdepth)
{
    double adval = 0.0;
    if (!extras)
    {
        extras = new  assetExtras;
    }
    if(0)FPS_PRINT_INFO(" before testing vec size  [{}] vec size {} vdepth {} valDepth {} dval {:2.3f}"
            , cstr{getfName()}
            , extras->valVec.size()
            , vdepth
            , extras->valDepth
            , dval
            );
    if (gotParam("hold") && getbParam("hold"))
    {
        return dval;
    }

    while (depth > 0 && extras->valDepth> 0 && ((int)extras->valVec.size() >= extras->valDepth))
    {
        adval = extras->valVec.front();
        extras->valVec.erase(extras->valVec.begin());
        if ((int)extras->valVec.size() > extras->valDepth)
        {
            FPS_PRINT_INFO(" reducing vec size  [{}] vec size {} valDepth {} adval {:2.3f}"
            , cstr{getfName()}
            , extras->valVec.size()
            , extras->valDepth
            , adval
            );
        }
    }

   
    extras->valVec.push_back(dval);
    if (vdepth > 0)
    {
        extras->valDepth = vdepth;
        depth = vdepth;
    }
    if(0)FPS_PRINT_INFO(" after reducing vec size  [{}] vec size {} valDepth {} dval {:2.3f}"
            , cstr{getfName()}
            , extras->valVec.size()
            , extras->valDepth
            , dval
            );
    if (0) FPS_PRINT_INFO("called here name [{}] size {} adval {:2.3f}"
        , cstr{getfName()}
        , extras->valVec.size()
        , dval
    );
    return adval;
}

double assetVar::getVecdVal(int vdepth)
{
    double adval = 0.0;
    if (!extras)
    {
        return 0.0;
    }
    if (vdepth >= depth)
    {
        return 0.0;
    }

    if ((int)extras->valVec.size() > vdepth)
    {
        adval = extras->valVec.at(extras->valVec.size() - vdepth - 1);
    }
    else
    {
        return 0.0;
    }

    if (0) FPS_PRINT_INFO("called here name [{}] size {} depth {} vdepth {} adval {:2.3f}"
        , cstr{getfName()}
        , extras->valVec.size()
        , depth
        , vdepth
        , adval
    );
    return adval;
}

int assetVar::getVecVals(int depth, double& avval, double& minval,
    double& maxval, double& spval, bool debug)
{
    if (0) FPS_PRINT_INFO("called here name [{}] extras {} depth {} size {}"
        , cstr{getfName()}
        , fmt::ptr(extras)
        , depth
        , extras?extras->valVec.size():0
    );
    //debug = true;
    int rc = 0;
    if (extras)
    {
        avval = 0.0;
        spval = 0.0;
        minval = extras->valVec.front();
        maxval = extras->valVec.front();
        int idx = 0;
        if (debug)FPS_PRINT_INFO("####################");
        if (debug)FPS_PRINT_INFO("running  name [{}] depth {} x [{:2.3f}]"
            , cstr{getfName()}
            , depth
            , minval
        );

        for (auto x : extras->valVec)
        {
            idx++;
            if (idx <= depth)
            {
                rc++;
                avval += x;
                if (x < minval) minval = x;
                if (x > maxval) maxval = x;
                if (debug) FPS_PRINT_INFO(" var [{}] idx [{}] depth [{}] val(x) [{:2.3f}] avval [{:2.3f}]"
                    , cstr{getfName()}
                    , idx
                    , depth
                    , x
                    , avval / idx
                    );
            }
        }
        spval = maxval - minval;
        if (debug)FPS_PRINT_INFO("called here name [{}] size {} avval {:2.3f}"
            , cstr{getfName()}
            , rc
            , avval
        );

        if (depth > 0 && idx > 0 && extras->valVec.size()>0)
        {
            if(idx <= depth)
            {
                avval /= idx;
            }
            else
            {
                avval /= depth;
            }
        }
    }
    return rc;
}

assetVar* assetVar::getaParam(const char* pname)
{
    assetVar* av = nullptr;
    if (0) FPS_PRINT_INFO("called here name [{}]"
        , pname
    );
    if (extras && extras->baseDict)
    {
        extras->baseDict->getFeat(pname, &av);
    }
    return av;
}

void assetVar::PubFunc(assetVar* av)
{
    if (!extras)
    {
        extras = new assetExtras;
    }
    extras->PubFunc = av;
}

void assetVar::SetPubFunc(assetVar* av)
{
    if (!extras)
    {
        extras = new assetExtras;
    }
    extras->SetPubFunc = av;
}

void assetVar::SetFunc(assetVar* av)
{
    if (!extras)
    {
        extras = new assetExtras;
    }
    extras->SetFunc = av;
}

namespace flex
{
    template<typename E>
    constexpr auto getEnumVal(E anEnum) -> typename std::underlying_type<E>::type
    {
        static_assert(std::is_enum<E>::value, "requires an enum to use this function.");
        return static_cast<typename std::underlying_type<E>::type>(anEnum);
    }
}

int assetVar::sendEvent(const char* source, fims* p_fims, Severity severity, const char* msg, ...)
{
    int rc = 0;
    if (p_fims)
    {
        char* buffer = nullptr;
        va_list args;
        va_start(args, msg);
        rc = vsnprintf(buffer, 0, msg, args);
        va_end(args);
        if (rc > 0)
        {
            va_list args2;

            rc++;
            buffer = (char*)malloc(rc);
            memset(buffer, 0, rc);
            //buffer[0] =0;
            va_start(args2, msg);
            vsnprintf(buffer, rc, msg, args2);
            va_end(args2);
            rc = strlen(buffer);
            cJSON* cj = cJSON_CreateObject();
            if (cj)
            {
                cJSON_AddStringToObject(cj, "source", source);
                cJSON_AddStringToObject(cj, "message", buffer);
                cJSON_AddNumberToObject(cj, "severity", static_cast<double>(flex::getEnumVal(severity)));
                char* body = cJSON_PrintUnformatted(cj);
                if (body)
                {
                    FPS_PRINT_INFO("sending event  xxx >>>>>>>>>>>>>>>> [{}]"
                                , cstr{body});

                    //p_fims->Send("set", "/events", nullptr, body);
                    rc = p_fims->Send("post", "/events", nullptr, body);
                    free(body);
                    if (rc < 0)
                    {
                        FPS_PRINT_INFO("send event failed rc = {} size = {}"
                        , rc
                        , strlen(body)
                        );
                        exit(0);
                    }
                }
                cJSON_Delete(cj);
            }
            free(buffer);
        }
    }
    else
    {
        FPS_PRINT_INFO("no p_fims [{}]", name);
    }
    return rc;
}

// int assetVar::sendLog(assetVar*av, const char *msg,...)
// {
//     int rc = 0;
//     if(extras)
//     {
//         // skip if not enabled
//         if (!getbParam("enablePerf"))
//         {
//             FPS_PRINT_INFO("%s>> log not enabled for %s\n",__func__, name.c_str());
//             return rc;
//         }

//         char* buffer=nullptr;
//         va_list args;
//         va_start (args, msg);
//         rc = vsnprintf (buffer, 0, msg, args);
//         va_end (args);   
//         if (rc > 0)
//         { 
//             va_list args2;

//             rc++;
//             buffer = (char *)malloc(rc); 
//             memset(buffer,0,rc);
//             //buffer[0] =0;
//             va_start (args2, msg);
//             vsnprintf (buffer, rc, msg, args2);
//             va_end (args2);
//             if(extras->logAlways || (extras->logfd< 0) )
//             {
//                 FPS_PRINT_INFO("LOG>>%s\n", buffer);
//             }
//             if(extras->logfd >= 0)
//             {
//                 rc = strlen(buffer);
//                 write(extras->logfd, buffer, rc);
//             }
//             free(buffer);
//         }
//     }
//     else
//     {
//         FPS_PRINT_INFO("%s>> no extras for %s\n",__func__, name.c_str());

//     }
//     return rc;
// }

// int assetVar::flushLog(void)
// {
//     if(extras && (extras->logfd>0))
//     {
//         syncfs(extras->logfd);
//     }
//     return 0;
// }

// int assetVar::logAlways(bool flag)
// {
//     if(extras)
//     {
//         extras->logAlways = flag;
//     }
//     return 0;
// }

// int assetVar::openLog(char* logDir, const char* fname, int logDepth)
// {
//     char *sp = nullptr;
//     char *tmpsp = nullptr;

//     if(!extras)
//     {
//         extras = new assetExtras;
//     }
//     // dont open again if we are already open
//     if(extras->logFile && (strcmp(extras->logFile, fname) == 0))
//     {
//         if(extras->logfd>=0)
//         {
//             return (extras->logfd);
//         }
//         else
//         {
//             free(extras->logFile);
//         }
//     }

//     if(extras->logfd>=0)
//     {
//         close(extras->logfd);
//     }

//     if(fname)
//     {
//         extras->logFile=strdup(fname);
//         extras->logDepth = logDepth;
//         // other options like db , ip may follow
//         sp = (char*)fname;
//         if(strncmp(fname,"file:", strlen("file:")) == 0)
//         {
//             sp += strlen("file:");
//         }
//         if (*sp != '/')
//         {
//             // in asset.h
//             asprintf(&tmpsp,"%s/%s", logDir, sp);
//             sp = tmpsp;
//         }
//         FPS_PRINT_INFO("%s >> opening new log file [%s]\n"
//             , __func__
//             , sp);
//     }
//     if(sp)extras->logfd  = open(sp, O_RDWR | O_CREAT | O_APPEND, 0644);  
//     if(tmpsp)free(tmpsp);
//     return 0;
// }

// int assetVar::closePerf(const char* fname)
// {
//     if(extras && (extras->logfd>0))
//     {
//         close(extras->logfd); 
//         extras->logfd = -1;
//     }
//     return 0;
// }

// add this assetVar to the schAvlist
// where ever that is
// it has to have an param of "RunTime"
// this should be sometime slightly in the future
int assetVar::addSchedReq(schAvlist& rreq)
{
    if (!gotParam("RunTime"))
    {
        FPS_PRINT_INFO("assetVar [{}] does not have a RunTime param"
            , name
        );
        return 0;
    }
    double tshot = getdParam("RunTime");

    auto it = rreq.begin();
    while (it != rreq.end())
    {
        if ((*it)->getdParam("RunTime") > tshot)
        {
            rreq.insert(it, this);
            return rreq.size();
        }
        it++;
    }
    rreq.push_back(this);
    return rreq.size();
}

int assetVar::addSchedReq(schAvlist& rreq, double tshot, double trep)
{
    setParam("RunTime", tshot);
    setParam("RepTime", trep);
    return addSchedReq(rreq);

}



/***********************************************
 *                 VarsMap
 ***********************************************/
 // VarsMap is this used ??
VarsMap::VarsMap()
{
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
 int depth;
    int cval;
    ATypes type;

    std::map<std::string, std::vector<assetAction*>>actVec;   //list of actions for each category
    assetFeatDict* featDict;
    assetFeatDict* optDict;
    assetFeatDict* baseDict;
    assetOptVec* optVec;
    assetVar* SetPubFunc;
    assetVar* SetFunc;
    assetVar* GetFunc;
    assetVar* PubFunc;
    assetVar* RunFunc;
    void* monFunc;

    void* compFunc;    // used to trigger on component ident

    assetVar* aVar;
    asset_manager* am;
    asset* ai;
    assetAction* aa;
    assetBitField* abf;
    int abNum;   // bit map sequence number
    bool IsDiff;
// TODO after MVP ,loose IsDiff in extras
    bool valChanged;
    int ui_type;
    bool useAlarms;

    std::vector<asset_log*> alarmVec;
    std::vector<asset_log*> logVec;
    int logfd;
    char *logFile;
    int logDepth;
    bool logAlways;
 ***********************************************/
assetExtras::assetExtras()
{
    //depth = 0; 
    cval = 0;
    type = AEND;
    featDict = nullptr;
    optDict = nullptr;
    baseDict = nullptr;
    optVec = nullptr;

    SetPubFunc = nullptr;
    SetFunc = nullptr;
    GetFunc = nullptr;
    PubFunc = nullptr;
    RunFunc = nullptr;
    compFunc = nullptr;
    monFunc = nullptr;

    aVar = nullptr;
    am = nullptr;
    ai = nullptr;
    // these are used in  config functions (onSet)
    aa = nullptr;
    abf = nullptr;
    abNum = -1;
    IsDiff = false;
    valChanged = false;
    ui_type = 0;
    useAlarms = false;
    valDepth = 0;

    // logfd = -1;
    // logFile = nullptr;
    // logDepth = 0;
    // logAlways = false;

}

assetExtras::~assetExtras()
{
    for (auto x : actVec)
    {
        for (auto y : x.second)
        {
            delete y;
        }
        x.second.clear();
    }
    actVec.clear();

    // for (auto x : logVec)
    // {
    //     delete x;
    // }

    // logVec.clear();
    // if(logFile)free(logFile);

    // if(logfd >= 0)
    // {
    //     close(logfd);
    // }
    //alarmVec
    auto asize = alarmVec.size();

    if (asize > 0)
    {
        for (auto aa : alarmVec)
        {
            if (aa)
            {
                delete aa;
            }
        }
        alarmVec.clear();
    }
    if (featDict) delete featDict;
    if (optDict) delete optDict;
    // we also have to keep the order of the list intact. use a control vector as a base assetvar.. Uck but needs must.
    // we may need to load up a derived class called a ui class. to do this...
    if (baseDict) delete baseDict;
    if (optVec) delete optVec;
    if (SetFunc) delete (assetFunc*)SetFunc;
    //if (pubFunc) delete pubFunc;
    //if (getFunc) delete getFunc;

    // we need  named map of arrays for things like ui options
    // actually the list below will also work for keeping the order list.
    //std::map<std::string,std::vector<assetVar *>> optVec;
}

// How do we designate a alarm/fault object  We see ui_type as an alarm in loadmap 
asset_log* assetVar::sendAlarm(assetVar* destAv, const char* atype, const char* msg, int severity)
{
    asset_log* avAlarm = nullptr;

    avAlarm = new asset_log((void*)this, (void*)destAv, atype, msg, severity);
    avAlarm->aVal = new assetVal;
    if (linkVar)
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

    if (1) FPS_PRINT_INFO("calling setAlarm [{}:{}] in destAv [{}]", comp, name, destAv->getfName());
    destAv->setAlarm(avAlarm);
    return nullptr;//avAlarm;
}

#include <chrono>
#include <ctime>

// this is needde to force the compiler to expand templates
// the function is not used it jjust needs to be there.
int forceAvTemplates()
{
    using namespace std::chrono;

    //ess_man = new asset_manager("ess_controller");
    varsmap vmap;
    VarMapUtils vm;
    asset_manager* am = new asset_manager("test");
    assetVar* av;
    am->am = nullptr;
    am->running = 1;
    char* cval = (char*)"1234";
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
    av->setLVal((char*)cval);
    av->valueDiff(ival);
    av->valueDiff(dval);

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
    bval = am->vm->valueChanged(dval, dval);
    //bval = am->vm->valueChanged(dval,ival);
    bval = am->vm->valueChanged(av, av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, bval, dval);
    //bval = am->av->valueChangednodb(dval,dval);
    //bval = am->av->valueChangednodb(dval,bval);
    // av->setParam("d1",dval);
    // av->setParam("b1",bval);
    // av->setParam("c1",tval);
    //assetBitField bf(1, 2, nullptr, nullptr, nullptr);
    assetBitField bf2(nullptr);
    ival = bf2.getFeat("d", &ival);
    cval = bf2.getFeat("d", &cval);

    system_clock::time_point now = system_clock::now();
    time_t tnow = system_clock::to_time_t(now);
    tm* local_tm = localtime(&tnow);
    char tbuffer[80];
    strftime(tbuffer, 80, "%c.", local_tm);
    am->amap["timeString"] = am->vm->setLinkVal(vmap, "test", "/status", "timeString", tbuffer);
    av->sendAlarm(av, nullptr, nullptr, 1);

    return 0;
}
/**************************************************************************************************************************************************************/
#endif