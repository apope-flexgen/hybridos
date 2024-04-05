#ifndef ASSETVAR_HPP
#define ASSETVAR_HPP
/*
 * this contains most of the data manipulation code for handling the internal
 *  system data spaces.
 * This code is NOT ess_specific and can be used on other projects
 */

#include <cjson/cJSON.h>
#include <cstring>
#include <fims/libfims.h>
#include <iostream>
#include <malloc.h>
#include <map>
#include <pthread.h>
#include <string>
#include <vector>

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif

// DONE add double settime to asset val  ... done now have to add setval/time
// done but needs to call nm.setTime() DONE  but just to bitmap do the same for
// the assetVar for scaling etc add the Feat Dict to keep all this crap DONE
// allow reconfig.. read the file and change key to new DONE use dict for
// assfeat .. DONE add depth vector to allow history of asset val DONE scaling
// and offset on remap  note we can have multiple remaps with different scaling
// DONE Fix assfeat for enum and bitmap
// DONE use dict for assfeat display
// DONE   more testing needed allow comp:var    for example
// /system/components:active_setpoint DONE assetVar needs to know its component
// DONE rework and test enum and bitmap to latest spec
// DONE load configs from a file with multiple subs
// DONE add linkVal  only sets the value if it is not in the config
// DONE access setTime;

// TODO possible changed value callback for an asset
// TODO << >> operators for channel.
// TODO fix asset config  . we can have three layers /asset/bms/bms_1
// TODO add hos asset
// TODO Fix comp logic  only going with two levels for no but comp:var solves
// some problems
// TODO set/get AssetVar  detects 1 layer component and adds assetID  everything
// must now be an asset example /status:soc becomes /status/bms_4:soc
// TODO Deadband  "actions" , add a assDict into some new Params
// TODO Think about time
// TODO add timer concept to VM  setvalfortime
// TODO Assvar can hold sequences
// TODO Max min methods
// TODO unOrdered map

// TODO detect and use depth vector to allow history of asset val
// TODO add auto cjson detect to find nfrags on a set or get ..... hmm how do we
// do get
// TODO add fims set and get with notification
// TODO file set and get this allows auto save and recall
// TODO mongodb set and get ... same thing
// TODO limit checking on set against limits in variable names.  ( do it on
// remap) use max/min = /table:var  onMax onMin max/minerr/time = /table:var etc
// to map it...

// Fix comp logic
// the simple component mapping is like this

// {
// "/system/commands":{"start_stop":
//                    { "value":0,
//                       "actions":
//                         {"onSet":{"enum":
//                                         [
//                                           { "mask":3,  "inValue":0,
//                                           "outValue":"Start",
//                                           "uri":"/system/ess_controls:oncmd"},
//                                           { "mask":3,  "inValue":1,
//                                           "outValue": true,
//                                           "uri":"/system/ess_controls:dccontactor"},
//                                           { "mask":12,
//                                           "inValue":11,"outValue":"Stop",
//                                           "uri":"/system/ess_controls:offcmd"},
//                                           { "mask":12,
//                                           "inValue":12,"outValue": false,  "
//                                           uri":"/system/ess_controls:dccontactor"}
//                                          ]
//                                    }
//                            }
//                   }
//         }
// }

// active_power_setpoint - provides  a request for power generation.

//    This value , from the modbus_server will have to be adjusted ( scale and
//    possibly offset) and placed in an ess_control register. The remap is
//    enabled or disabled by the value of the "enable" uri. ( defaults to true
//    if missing)

// ```json
// {
// "/system/commands":{"active_power_setpoint":
//                        {"value":0,
//                            "signed":true,
//                            "actions":
//                   {"onSet":{"remap":
//                         [
//                            {
//                            "enable":"/system/ess_controls:active_power_enable","offset":0,
//                            "scale":10,
//                                                       "uri":"/system/ess_controls:active_power_cmd"}
//                         ]
//                       }
//                    }
//                }
//         }
// }
// an asset will have variables, states and parameters
// an asset can also have alarms and warnings
// subclasses of this class can have a fims sender so sets and gets can also use
// fims there can also be a a timer so that we can set a value  and then remove
// it after a time. The timer is run via a channel somewhere. we can also have a
// value history

static double g_setTime = 0.0;

// something other than name, register_id, scale and unit
class assFeat
{
public:
    assFeat(const char* _name, int val)
    {
        valueint = val;
        type = AINT;
        name = _name;
        valuestring = nullptr;
    };
    assFeat(const char* _name, double val)
    {
        valuedouble = val;
        type = AFLOAT;
        name = _name;
        valuestring = nullptr;
    };
    assFeat(const char* _name, bool val)
    {
        valuebool = val;
        type = ABOOL;
        name = _name;
        valuestring = nullptr;
    };

    assFeat(const char* _name, const char* val)
    {
        valuestring = strdup(val);
        type = ASTRING;
        name = _name;
    };
    ~assFeat()
    {
        if (valuestring)
            free((void*)valuestring);
    };

    enum AFTypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AEND
    };
    std::string name;
    std::string fname;
    AFTypes type;
    double valuedouble;
    int valueint;
    char* valuestring;
    bool valuebool;
};

// used for the bit mapping a bit single purposed but we may add a dict
class assetBitField
{
public:
    assetBitField(int _mask, int _bit, const char* _uri, const char* _var, char* tmp)
    {
        mask = _mask;
        bit = _bit;
        uri = strdup(_uri);
        var = _var ? strdup(_var) : nullptr;
        tmpval = nullptr;

        if (tmp)
            tmpval = strdup(tmp);
        // featMap["mask"] = new assFeat("mask",_mask);
        // featMap["bit"] = new assFeat("bit",_bit);
        // featMap["uri"] = new assFeat("uri",_uri);

        // featMap["bvalue"] = new assFeat("bvalue",tmp);

        // if(tmp)
        //    featMap["tmpval"] = new assFeat("tmpval",tmp);
    };

    assetBitField(cJSON* cj)
    {
        mask = 0;
        bit = 0;
        uri = nullptr;
        var = nullptr;
        tmpval = nullptr;
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
                FPS_ERROR_PRINT(" %s >>   stmp2 >>%s<< string %s child %p next %p \n", __func__, stmp2,
                                (void*)cjic->string, (void*)cjic->child, (void*)cjic->next);
                free((void*)stmp2);
            }
            addFeat(cjic);
            if (strcmp(cjic->string, "outValue") == 0)
            {
                if (tmpval)
                    free((void*)tmpval);
                tmpval = cJSON_PrintUnformatted(cjic);
            }
            cjic = cjic->next;
        }
        if (0)
        {
            FPS_ERROR_PRINT(" %s >>   Features added \n", __func__);
            showFeat();

            FPS_ERROR_PRINT(" %s >>   Features done \n", __func__);
        }
    };

    ~assetBitField()
    {
        if (uri)
            free((void*)uri);
        if (var)
            free((void*)var);
        if (tmpval)
            free((void*)tmpval);
        for (auto x : featMap)
        {
            delete x.second;
        }
        featMap.clear();
    };

    int getFeat(const char* name, int* val)
    {
        *val = 0;
        if (featMap.find(name) != featMap.end())
        {
            assFeat* af = featMap[name];
            *val = af->valueint;
        }
        return *val;
    };

    double getFeat(const char* name, double* val)
    {
        *val = 0.0;
        if (featMap.find(name) != featMap.end())
        {
            assFeat* af = featMap[name];
            *val = af->valuedouble;
        }
        return *val;
    };

    char* getFeat(const char* name, char** val)
    {
        *val = nullptr;
        if (featMap.find(name) != featMap.end())
        {
            assFeat* af = featMap[name];
            *val = af->valuestring;
        }
        return *val;
    };

    void getBit(int val, cJSON** cj)
    {
        if ((val & mask) == bit)
        {
        }
    };

    template <class T>
    void addFeat(const char* name, T val)
    {
        featMap[name] = new assFeat(name, val);
    };

    void addFeat(cJSON* cj)
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
                addFeat(cj->string, true);
                break;
            default:
                // asprintf(&stmp,"Unknown");
                break;
        }
    };

    void showFeat()
    {
        for (auto x : featMap)
        {
            assFeat* af = x.second;
            char* stmp;
            switch (af->type)
            {
                case assFeat::AFLOAT:
                    asprintf(&stmp, "%s->[%f]", af->name.c_str(), af->valuedouble);
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

    // TODO store all these in a Feat Dict
    int atype;
    int mask;
    int bit;
    char* uri;
    char* var;
    char* tmpval;
    std::map<std::string, assFeat*> featMap;
};

// TODO add fims, file, mdb, timed set  etc
class assetAction
{
public:
    assetAction(const char* aname)
    {
        name = aname;
        idx = 0;
    };

    ~assetAction()
    {
        for (auto& x : Abitmap)
        {
            delete Abitmap[x.first];
        }
        Abitmap.clear();
    };

    // TODO add all this stuff into a Feat Dict
    assetBitField* addBitField(int mask, int bit, const char* uri, const char* var, cJSON* cjv)
    {
        char* tmp = cJSON_PrintUnformatted(cjv);
        if (0)
            FPS_ERROR_PRINT(" Added bitfield for [%s] bit %d cjv %p tmp [%s]\n", var, bit, (void*)cjv, tmp);
        Abitmap[bit] = (new assetBitField(mask, bit, uri, var, tmp));
        free((void*)tmp);
        return Abitmap[bit];
    };
    assetBitField* addBitField(cJSON* cjv)
    {
        int idd = idx++;
        if (0)
        {
            char* tmp = cJSON_PrintUnformatted(cjv);
            FPS_ERROR_PRINT(" %s >> Adding bitfield at [%d] [%s] \n", __func__, idd, tmp);
            free((void*)tmp);
        }

        Abitmap[idd] = (new assetBitField(cjv));
        return Abitmap[idd];
    };
    int idx;

    std::map<int, assetBitField*> Abitmap;
    std::string name;
};

// TODO
// the assetVal allows us to keep the lastval and a mini history if needed.

class assetVal
{
    enum ATypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AEND
    };

public:
    assetVal()
    {
        valuedouble = 0.0;
        valueint = 0;
        type = AINT;
        valuestring = nullptr;
    };

    // assetVal(double val)
    // {
    //     valuedouble=val;
    //     valueint=(int)val;
    //     type=AFLOAT;
    //     valuestring = nullptr;
    // };

    // assetVal(const char* val)
    // {
    //     valuedouble=0;
    //     valueint=0;
    //     type=ASTRING;
    //     valuestring =strdup(val);
    // };
    // assetVal(bool val)
    // {
    //     valuedouble=0;
    //     valueint=(val==true);
    //     valuedouble=valueint;
    //     valuebool = val;
    //     type=ABOOL;
    //     valuestring = nullptr;
    // };
    // may cause havoc with the template
    assetVal(int val) : assetVal()
    {
        valuedouble = val;
        valueint = val;
        type = AINT;
    };

    assetVal(double val) : assetVal()
    {
        valuedouble = val;
        valueint = (int)val;
        type = AFLOAT;
    };

    assetVal(const char* val) : assetVal()
    {
        valuedouble = 0;
        valueint = 0;
        type = ASTRING;
    };
    assetVal(bool val) : assetVal()
    {
        valueint = (val == true);
        valuedouble = valueint;
        valuebool = val;
        type = ABOOL;
    };

    ~assetVal()
    {
        if (valuestring != nullptr)
            free((void*)valuestring);
    }

    virtual bool getVal(bool& val)
    {
        val = valuebool;
        return val;
    };
    virtual int getVal(int& val)
    {
        val = valueint;
        return val;
    };
    virtual double getVal(double& val)
    {
        val = valuedouble;
        return val;
    };
    virtual char* getVal(char*& val)
    {
        val = valuestring;
        return val;
    };

    virtual void setVal(int val)
    {
        valueint = val;
        setTime = g_setTime;
    };

    virtual void setVal(double val)
    {
        valuedouble = val;
        setTime = g_setTime;
    };

    virtual void setVal(bool val)
    {
        valuebool = val;
        setTime = g_setTime;
    };

    virtual void setVal(const char* val)
    {
        if (valuestring)
        {
            free((void*)valuestring);
        }
        valuestring = strdup(val);
        setTime = g_setTime;
    };

    virtual bool setVal(cJSON* cj)
    {
        if (cJSON_IsBool(cj))
        {
            if (0)
                FPS_ERROR_PRINT(" body child is a cjson bool value [%s]\n", cJSON_IsTrue(cj) ? "true" : "false");
            setVal(cJSON_IsTrue(cj));
            return true;
        }
        else if (cJSON_IsNumber(cj))
        {
            if (0)
                FPS_ERROR_PRINT(" body child is a cjson numerical value [%f]\n", cj->valuedouble);
            setVal(cj->valuedouble);
            return true;
        }
        else if (cJSON_IsString(cj))
        {
            if (0)
                FPS_ERROR_PRINT(" body child is a cjson string value [%s]\n", cj->valuestring);
            setVal(cj->valuestring);
            return true;
        }
        else
        {
            if (0)
                FPS_ERROR_PRINT(" body child [%s] cannot be simply decoded\n", cj->string);
        }
        return false;
        setTime = g_setTime;
    };

    virtual void setType(ATypes t) { type = t; }
    virtual double getsTime() { return setTime; }
    virtual cJSON* getValCJ()
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
    virtual cJSON* getValCJ(double scale, double offset)
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

    ATypes type;
    double valuedouble;
    int valueint;
    char* valuestring;
    bool valuebool;
    double setTime;
};

// this is the assetvar we love
class assetVar
{
public:
    enum ATypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AEND
    };
    assetVar()
    {
        depth = 0;
        cval = 0;
        aVals = nullptr;
        setDepth(1);
    }

    assetVar(const char* _name, int val) : assetVar()
    {
        name = _name;
        aVal = new assetVal(val);
        type = AINT;
    };
    assetVar(const char* _name, double val) : assetVar()
    {
        name = _name;
        aVal = new assetVal(val);
        type = AFLOAT;
    };

    assetVar(const char* _name, const char* val) : assetVar()
    {
        name = _name;
        aVal = new assetVal(val);
        type = ASTRING;
    };

    assetVar(const char* _name, bool val) : assetVar()
    {
        name = _name;
        aVal = new assetVal(val);
        type = ABOOL;
    };

    template <class T>
    assetVar(const char* _name, const char* _comp, T val) : assetVar(_name, val)
    {
        comp = _comp;
    }

    ~assetVar()
    {
        delete aVal;
        int i;
        for (i = 0; i < depth; i++)
        {
            if (aVals[i])
                delete aVals[i];
        }
        free((void*)aVals);
        for (auto x : actMap)
        {
            delete x.second;
        }
        actMap.clear();
    }

    virtual const char* getName(void) { return name.c_str(); }

    template <class T>
    T getVal(T& val)
    {
        aVal->getVal(val);
        return val;
    };

    template <class T>
    void setVal(T val)
    {
        aVal->setVal(val);
    };

    virtual bool setCjVal(cJSON* cj)
    {
        if (cJSON_IsBool(cj))
        {
            FPS_ERROR_PRINT(" body child is a cjson bool value [%s]\n", cJSON_IsTrue(cj) ? "true" : "false");
            setVal(cJSON_IsTrue(cj));
            return true;
        }
        else if (cJSON_IsNumber(cj))
        {
            FPS_ERROR_PRINT(" body child is a cjson numerical value [%f]\n", cj->valuedouble);
            setVal(cj->valuedouble);
            return true;
        }
        else if (cJSON_IsString(cj))
        {
            FPS_ERROR_PRINT(" body child is a cjson string value [%s]\n", cj->valuestring);
            setVal(cj->valuestring);
            return true;
        }
        else
        {
            FPS_ERROR_PRINT(" body child [%s] cannot be simply decoded\n", cj->string);
        }
        return false;
    }

    void setDepth(int de)
    {
        int dummy = 0;
        assetVal** oldv = aVals;
        aVals = (assetVal**)calloc(de, sizeof(assetVal*));
        if (depth > 0)
        {
            if (de > depth)
            {
                int i;
                for (i = 0; i < depth; i++)
                {
                    aVals[i] = oldv[i];
                }
                for (; i < de; i++)
                {
                    aVals[i] = new assetVal(dummy);
                    aVals[i]->setType(aVal->type);
                }
            }
            // go smaller ??   copy from cval backwards then delete the rest
            else
            {
                if (cval < de)
                {
                    int i;
                    // just leave the rest they are lost for now
                    for (i = cval; i > 0; i--)
                    {
                        aVals[i] = oldv[i];
                    }
                }
                else
                {
                    int i;
                    int id = de - 1;
                    for (i = cval; i > cval - de; i--)
                    {
                        aVals[i] = oldv[id--];
                    }
                    /* code */
                }
            }
        }
        if (oldv)
            free((void*)oldv);
    }

    virtual void showvarValueCJ(cJSON* cj)
    {
        if (0)
        {
            FPS_ERROR_PRINT("ShowVarValueCJ  >>>>>>>name [%s] comp [%s] type %d aval %p\n", name.c_str(), comp.c_str(),
                            type, aVal);
            FPS_ERROR_PRINT("ShowVarValueCJ  >>>>>>>name [%s] type %d atype %d\n", name.c_str(), type, aVal->type);
        }
        cJSON* cjv = cj;  // cJSON_CreateObject();

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
                    FPS_ERROR_PRINT("%s >>  ASTRING name [%s] type %d atype %d valstr [%s]\n", __func__, name.c_str(),
                                    type, aVal->type, aVal->valuestring);
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
    // gets '{name:value}
    virtual void getValCJ(cJSON** cj)
    {
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
                    FPS_ERROR_PRINT("%s >>  ASTRING name [%s] type %d atype %d valstr [%s]\n", __func__, name.c_str(),
                                    type, aVal->type, aVal->valuestring);
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

    virtual void showvarCJ(cJSON* cj)
    {
        if (0)
            printf("ShowVarCJ  >>>>>>>name [%s] type %d atype %d\n", name.c_str(), type, aVal->type);
        cJSON* cjv = cJSON_CreateObject();
        showvarValueCJ(cjv);

        cJSON_AddItemToObject(cj, name.c_str(), cjv);
        if (0)
            printf("ShowVarCJ  <<<<<<<name [%s] type %d atype %d\n", name.c_str(), type, aVal->type);

        if (!actMap.empty())
        {
            cJSON* cjact = cJSON_CreateObject();
            for (auto& x : actMap)
            {
                cJSON* cja = getAction(x.second);
                if (0)
                    FPS_ERROR_PRINT(" <<<<< action got cj  -->%s<--  cja %p\n", x.first.c_str(), (void*)cja);

                if (cja)
                {
                    if (0)
                        FPS_ERROR_PRINT(" >>>>> OK  action found\n");

                    char* tmp = cJSON_PrintUnformatted(cja);
                    if (tmp)
                    {
                        if (0)
                            FPS_ERROR_PRINT(" >>>>> show action found -->%s<<--\n", tmp);
                        free((void*)tmp);
                    }
                    cJSON_AddItemToObject(cjact, x.first.c_str(), cja);
                }
                else
                {
                    FPS_ERROR_PRINT(" >>>>> HMMM no action found\n");
                }
            }
            cJSON_AddItemToObject(cjv, "actions", cjact);
        }
        return;
    };

    cJSON* getAction(assetAction* aact)
    {
        if (0)
            FPS_ERROR_PRINT(" %s >> >>>>> get action start, name >>%s<<--\n", __func__, aact->name.c_str());

        cJSON* cj = cJSON_CreateObject();
        cJSON* cja = cJSON_CreateArray();
        for (auto& x : aact->Abitmap)
        {
            cJSON* cjix = cJSON_CreateObject();
            if (0)
                FPS_ERROR_PRINT(" >>>>> get action bitfield start <<--\n");

            assetBitField* abf = x.second;
            char* uri = abf->getFeat("uri", &uri);
            char* var = abf->getFeat("var", &var);

            if (0)
                FPS_ERROR_PRINT(" %s  >>>>> get action bitfield uri %s var %s<<--\n", __func__, uri, var);
            // int fm = 0;
            // use featmap
            for (auto x : abf->featMap)
            {
                // cJSON_AddNumberToObject(cjix,"fmIndex", fm++);

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

            cJSON_AddItemToArray(cja, cjix);
            if (0)
                FPS_ERROR_PRINT(" <<<<<<< get action var[%s]  bitfield done <<--\n", abf->var);
        }
        cJSON_AddItemToObject(cj, aact->name.c_str(), cja);

        if (0)
            FPS_ERROR_PRINT(" <<<<<<< get action done <<--\n");
        return cj;
    };

public:
    int depth;
    int cval;
    std::string name;   // this is really id
    std::string fname;  // this is the name
    std::string unit;
    std::string register_id;  //?
    std::string comp;         // new we  set comp
    double scale;             //?
    ATypes type;
    assetVal* aVal;    // current assetval
    assetVal** aVals;  // list of past vals includes this val
    std::map<std::string, assFeat*> featMap;
    std::map<std::string, assetAction*> actMap;  // list of actions
};

typedef std::map<std::string, pthread_mutex_t*> locksmap;
// this is the old varsmap
typedef std::map<std::string, std::map<std::string, assetVar*>> varslist;
// this now has components of "vars","links", "func" etc
typedef std::map<std::string, varslist*> varsmap;

typedef std::map<std::string, assetVar*> varmap;
typedef std::map<std::string, varmap*> nvarsmap;

// this is the accessor
class VarsMap
{
public:
    pthread_mutex_t map_lock;
    locksmap amap;

    nvarsmap vmap;

    VarsMap() { pthread_mutex_init(&map_lock, nullptr); };

    ~VarsMap()
    {
        // find each comp
        nvarsmap::iterator x;
        std::map<std::string, assetVar*> vm;

        for (x = vmap.begin(); x != vmap.end(); ++x)
        {
            // for each var
            varmap* vm = vmap[x->first];

            for (auto y = vm->begin(); y != vm->end(); ++y)
            {
                free((void*)y->second);
            }
            vm->clear();
            free((void*)amap[x->first]);
        }
        vmap.clear();
    };

    // find  comp with or with out the map_lock already set
    varmap* findComp(const char* comp, bool lockit = true)
    {
        if (lockit)
            pthread_mutex_lock(&map_lock);
        auto iv = vmap.find(comp);
        if (lockit)
            pthread_mutex_unlock(&map_lock);
        if (iv == vmap.end())
        {
            return nullptr;
        }
        return vmap[comp];
    };

    // with lockit = false we assume that the map_lock is taken
    // we also return with amap[comp] locked
    // comp is in fact already modified to remove ':'

    varmap* findAddComp(const char* comp, bool lockit = true)
    {
        varmap* mvar;

        if (lockit)
            pthread_mutex_lock(&map_lock);
        auto iv = vmap.find(comp);
        if (iv == vmap.end())
        {
            pthread_mutex_t* mylock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
            pthread_mutex_init(mylock, nullptr);

            // pthread_mutex_lock(&map_lock);
            mvar = new varmap;

            vmap[comp] = mvar;
            amap[comp] = mylock;
        }

        if (!lockit)
            pthread_mutex_lock(amap[comp]);
        if (lockit)
            pthread_mutex_unlock(&map_lock);

        return mvar;
    };
    // may not be used
    varmap* addComp(const char* comp, bool lockit = true)
    {
        pthread_mutex_t* mylock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mylock, nullptr);
        pthread_mutex_lock(&map_lock);
        varmap* mvar = new varmap;

        vmap[comp] = mvar;
        amap[comp] = mylock;
        pthread_mutex_unlock(&map_lock);
        return mvar;
    };

    // finds a comp / asset pair optionally returns with the component locked
    assetVar* findAsset(const char* comp, const char* var, bool lockit = false)
    {
        assetVar* avar = nullptr;
        char* mycomp = (char*)comp;
        char* myvar = (char*)var;

        if (var == nullptr)
        {
            // break up uri into comp:var
            mycomp = strdup(comp);
            myvar = strstr(mycomp, ":");
            if (myvar)
            {
                *myvar = 0;
                myvar++;
            }
        }
        // TODO use findVar
        auto nmap = findComp(mycomp);
        if (nmap != nullptr)
        {
            pthread_mutex_lock(amap[mycomp]);
            auto av = nmap->find(myvar);

            if (av == nmap->end())
            {
                pthread_mutex_unlock(amap[mycomp]);
                if (mycomp != (char*)comp)
                {
                    free((void*)mycomp);
                }

                return avar;
            }
            if (!lockit)
                pthread_mutex_unlock(amap[mycomp]);
            avar = av->second;
        }
        if (mycomp != (char*)comp)
        {
            free((void*)mycomp);
        }
        return avar;
    };
    template <class T>
    assetVar* findAddAsset(const char* comp, const char* var, T val)
    {
        assetVar* avar;

        char* mycomp = (char*)comp;
        char* myvar = (char*)var;

        if (var == nullptr)
        {
            // break up uri into comp:var
            mycomp = strdup(comp);
            myvar = strstr(mycomp, ":");
            if (myvar)
            {
                *myvar = 0;
                myvar++;
            }
        }

        pthread_mutex_lock(&map_lock);

        auto nmap = findAddComp(mycomp, false);
        auto av = nmap->find(myvar);
        if (av == nmap->end())
        {
            avar = new assetVar(myvar, val);
            nmap->insert(std::make_pair(myvar, avar));
            pthread_mutex_unlock(amap[mycomp]);
        }

        avar = av->second;

        pthread_mutex_unlock(&map_lock);
        if (mycomp != (char*)comp)
        {
            free((void*)mycomp);
        }

        return avar;
    };
};

// pull in the process fims stuff
// accessor class for VarMapUtils
class VarMapUtils
{
public:
    VarMapUtils() { set_base_time(); };
    VarMapUtils(varsmap& vm) : VarMapUtils()
    {
        vm["vars"] = new varslist;
        vm["links"] = new varslist;
        vm["func"] = new varslist;
    };

    ~VarMapUtils(){};

public:
    double g_base_time;
    void clearVm(varmap& vmap)
    {
        for (auto& y : vmap)
        {
            printf(" deleteing [%s]\n", y.first.c_str());
            delete y.second;
        }
        vmap.clear();
    }

    void clearVlist(varslist& vl)
    {
        for (auto& x : vl)
        {
            printf(" deleting vl.first [%s]\n", x.first.c_str());
        }
        vl.clear();
    }

    void clearVmap(varsmap& vmap)
    {
        for (auto& x : vmap)
        {
            printf(" deleting varlist [%s]\n", x.first.c_str());
            varslist* vl = x.second;
            clearVlist(*vl);
            delete x.second;
        }
        vmap.clear();
    }

    long int get_time_us()
    {
        long int ltime_us;
        timespec c_time;
        clock_gettime(CLOCK_MONOTONIC, &c_time);
        ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
        return ltime_us;
    }

    double get_time_dbl() { return (double)get_time_us() / 1000000.0 - g_base_time; }

    void set_base_time(void) { g_base_time = get_time_dbl(); }

    // do this before a var update
    void setTime(void) { g_setTime = get_time_dbl(); }
    // do this before a var update
    double getTime(void)
    {
        return g_setTime;  // = get_time_dbl();
    }
#if 0
    void write_cjson(const char* fname, cJSON*cj)
    {
        FILE *fp = nullptr;
        fp = fopen(fname, "w");
        if(fp == nullptr)
        {
            FPS_ERROR_PRINT("Failed to open file %s\n", fname);
            return;
        }

        char* res = cJSON_Print(cj);
        size_t bytes_written = fwrite(res, strlen(res), 1, fp);

        FPS_ERROR_PRINT(" %s >> Wrote %d bytes to  file %s\n",__func__, bytes_written, fname);
        fclose(fp);
        free((void *)res) ;
    }
    // get a file and replace string patterns found 
    char* get_cjfile(const char* fname, std::vector<std::pair<std::string, std::string>> *reps = nullptr)
    {
        FILE *fp = nullptr;

        if (fname == nullptr)
        {
            FPS_ERROR_PRINT(" Failed to get the path of the config file. \n");
            return nullptr;
        }

        fp = fopen(fname, "r");
        if(fp == nullptr)
        {
            FPS_ERROR_PRINT("Failed to open file %s\n", fname);
            return nullptr;
        }

        fseek(fp, 0, SEEK_END);
        long unsigned file_size = ftell(fp);
        rewind(fp);

        // create Configuration_file and read file in Configuration_file
        char *config_file = (char *)calloc(1, file_size+1);
        if(config_file == nullptr)
        {
            FPS_ERROR_PRINT("Memory allocation error\n");
            fclose(fp);
            return nullptr;
        }

        size_t bytes_read = fread(config_file, 1, file_size, fp);
        fclose(fp);
        if(bytes_read != file_size)
        {
            FPS_ERROR_PRINT("Read size error.\n");
            free((void*)config_file);
            return nullptr;
        }
        config_file[bytes_read] = 0;

        if(reps)
        {
            std::string cstr = config_file;
            for(int i = 0 ; i < reps->size(); i++)
            {
                auto x = reps->at(i); 
                std::string fstr = x.first;
                std::string tstr = x.second;
                std::size_t start_pos = 0; 
                FPS_ERROR_PRINT(" %s >> Replacing [%s] with [%s] in file [%s]\n", __func__, fstr.c_str(), tstr.c_str(), fname);
    
                while((start_pos = cstr.find(fstr, start_pos)) != std::string::npos) 
                {
                     cstr.replace(start_pos, fstr.length(), tstr);
                     start_pos += tstr.length(); // ...
                }
            }

            free((void *)config_file);
            config_file = strdup(cstr.c_str());
        }
        return config_file;
    }

    cJSON* get_cjson(const char* fname, std::vector<std::pair<std::string, std::string>> *reps = nullptr)
    {
        char* config_file = get_cjfile(fname, reps);
        cJSON* cj = cJSON_Parse(config_file);
        free((void*)config_file);
        if(cj == nullptr)
            FPS_ERROR_PRINT("Invalid JSON object in file\n");
        return cj;
    }

    char *pull_pfrag(const char *uri, int idx)
    {
        char *sp = (char *)uri;
        while(idx)
        {
            if (*sp++ == '/') idx--;
        }

        char* comp_id = strdup(sp);
        char* comp_end = strstr(comp_id,"/");
        if(comp_end)
            *comp_end = 0;
        return comp_id;
    }

    int get_nfrags(const char* uri)
    {
        int nfrags = 0;
        const char* sp = uri;
        while (*sp)
        {
            if (*sp++ == '/') nfrags++;
        }
        return nfrags;
    }

    template <class T>
    assetVar*makeVar(varsmap &vmap, const char* comp, const char* var,T &value)
    {
        char* mycomp = (char *)comp;
        char* myvar = (char *)var;
        assetVar* av = nullptr;
    
        if(var == nullptr)
        {
            // break up uri into comp:var
            mycomp = strdup(comp);
            myvar = strstr(mycomp,":");
            if(myvar)
            {
                *myvar = 0;
                myvar++;
            }
        }

        av = new assetVar(myvar, mycomp, value);
        vmap[mycomp][myvar] = av;

        if (mycomp != (char *)comp)
            free((void *)mycomp);
        return av;
    }

    template <class T>
    assetVar*setVal(varsmap &vmap, const char* comp, const char* var, T &value)
    {

        assetVar* av = getVar(vmap, comp, var);
        if(av)
        {
            av->setVal(value);
            return av;
        }
        // we cant find it so make it
        av = makeVar(vmap, comp, var, value);
        return av;

    }

    // links to the value from the config
    // does not set a value
    template <class T>
    assetVar*linkVal(varsmap &vmap, const char* comp, const char* var, T &defvalue)
    {

        assetVar* av = getVar(vmap, comp, var);
        if(av)
        {
            //
            return av;
        }
        // we cant find it so make it and set a default value
        av = makeVar(vmap, comp, var, defvalue);
        return av;

    }
    // TODO rework from latest spec
    assetVar*runActBitFieldfromCj(varsmap &vmap, assetVar *av, assetAction* aa)
    {
        // for now run the sets directly
        //std::map<int,assetBitField *> bitmap;
        int aval = (int)av->aVal->valuedouble;
        //av->aVal->valueint =  (int)av->aVal->valuedouble;
        for (auto &x : aa->Abitmap)
        {
            assetBitField* abf = x.second;  
            double inValue  = abf->getFeat("inValue", &inValue);
            double mask = abf->getFeat("mask", &mask);
            char* uri   = abf->getFeat("uri", &uri);
            char* var   = abf->getFeat("var", &var);
            int bit = (int)inValue;
            if(0)FPS_ERROR_PRINT(" %s >> ######on Set Bitfield action bit %d mask 0x%04x hbit 0x%04x  aval %d masked val %d uri [%s] var [%s] tmpval [%s]\n"
                    ,__func__
                    , (int)bit
                    , (int)mask
                    , (int)bit
                    , aval
                    , (int)aval & (int)mask
                    , uri
                    , var
                    , abf->tmpval
                );
            if(aval & (1 << (int)bit))
            {
                cJSON* cjt = cJSON_Parse(abf->tmpval);
                if(cjt)
                {
                    setValfromCj(vmap, uri, var, cjt);
                    cJSON_Delete(cjt);
                }

                if(0) // sendfims
                {
                    char* stmp;
                    asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"body\":%s}", uri,var,abf->tmpval);
                    FPS_ERROR_PRINT(" %s >> #######Set action [%s]\n",__func__, stmp);
                    free((void*) stmp);
                }
            }
        }
        return av;
    }

// TODO rework from latest spec
    assetVar*runActEnumfromCj(varsmap &vmap, assetVar *av, assetAction* aa)
    {
        // for now run the sets directly
        //std::map<int,assetBitField *> bitmap;
        int aval = (int)av->aVal->valuedouble;
        double dval = av->aVal->valuedouble;
        for (auto &x : aa->Abitmap)
        {
            assetBitField* abf = x.second;
            double inValue = abf->getFeat("inValue", &inValue);
            double mask = abf->getFeat("mask", &mask);
            char* uri = abf->getFeat("uri", &uri);
            char* var = abf->getFeat("var", &var);    // var can be null if we use /comp:var
            char* outValue = abf->getFeat("outValue", &outValue);
            int bit = (int)inValue;

            if(0)FPS_ERROR_PRINT(" %s >> ######on Set Enum action bit %d mask 0x%04x hbit 0x%04x  aval %d masked val %d uri [%s] var [%s] tmpval [%s] outValue[%s]\n"
                    ,__func__
                    , (int)bit
                    , (int)mask
                    , (int)bit
                    , aval
                    , (int)aval & (int)mask
                    , uri
                    , var?var:"NoVar"
                    , abf->tmpval
                    , outValue?outValue:"noOutvalue"
            );
            
            if(((int)aval & (int)mask) == (int)bit)
            {
                cJSON* cjov = cJSON_Parse(abf->tmpval);
                if(cjov)
                {
                    setValfromCj(vmap, uri, var, cjov);
                    cJSON_Delete(cjov);
                }
                //setValfromCj(vmap, uri, var, cJSON_Parse(abf->tmpval));
                if(0) //FPS_ERROR_PRINT // send fims
                {
                    char* stmp;
                    asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"body\":%s}", uri, abf->tmpval);
                    FPS_ERROR_PRINT(" #######Set action [%s]\n",stmp);
                    free((void*) stmp);
                }
            }
        }
        return av;    
    }

// new from spec // done
// {
// "/commands[/asset_id]":{"active_power_setpoint":
//                        {"value":0,
//                            "signed":true,
//                            "actions":
//                   {"onSet":{"remap":
//                         [
//                            { "enable":"/controls/ess_controls:active_power_enable","offset":0,  "scale":10, 
//                                                       "uri":"/cmds/ess_controls:active_power_cmd"}
//                         ]
//                       }
//                    }
//                }
//         }
// }
    // TODO work out how to use local asset vars here
    assetVar*runActRemapfromCj(varsmap &vmap, assetVar *av, assetAction* aa)
    {
        // for now run the sets directly
        //std::map<int,assetBitField *> bitmap;
        int aval = av->aVal->valueint;
        char *strval;
        if(av->aVal->valuestring)
        {
                strval = (char *)strdup(av->aVal->valuestring);
        }
        else
        {
            strval =(char*)strdup("NoVal");
        }
        
        for (auto &x : aa->Abitmap)
        {
            assetBitField* abf = x.second;
            bool enable = true;
            char* ensp = nullptr;    abf->getFeat("enable", &ensp);
            if(ensp)
            {
                assetVar*av = getVar(vmap, (const char *)ensp, nullptr);
                if(av)
                {
                    enable = av->getVal(enable);
                }
            }

            double scale = 1.0; abf->getFeat("scale",&scale);
            double offset= 0;   abf->getFeat("offset",&offset);
            char* uri = nullptr;   abf->getFeat("uri", &uri);
            char* var = nullptr;   abf->getFeat("var", &var);

            if(enable)
            {
                cJSON* cjav = av->aVal->getValCJ(scale, offset);
                setValfromCj(vmap, uri, var, cjav);
                if(0)
                {
                    // TODO use this to set a FIMS message
                    char* scj = cJSON_PrintUnformatted(cjav);
                    char* stmp;
                    asprintf(&stmp, "{\"method\":\"set\", \"uri\":\"%s\",\"var\":\"%s\",\"value\":%s}"
                            , uri, var, scj);
                    if(0)FPS_ERROR_PRINT(" %s >> #######Set action [%s]\n",__func__, stmp);
                    free((void*) stmp);
                    free((void*) scj);
                }
                cJSON_Delete(cjav);
            }           
        }
        free((void*)strval);
        return av;
    }

    assetVar*runActValfromCj(varsmap &vmap, assetVar *av, assetAction* aa)
    {
        if (aa)
        {
            if(0) FPS_ERROR_PRINT(" %s >> #######on Set action aa->name [%s]\n",__func__, aa->name.c_str());

            if (strcmp(aa->name.c_str(),"bitfield") == 0)
            {
                av = runActBitFieldfromCj(vmap, av, aa);
            }
            else if (strcmp(aa->name.c_str(),"enum") == 0)
            {
                av = runActEnumfromCj(vmap, av, aa);
            }
            else if (strcmp(aa->name.c_str(),"remap") == 0)
            {
                av = runActRemapfromCj(vmap, av, aa);
            }
        }
        return av;
    }
    
    assetVar*setActValfromCj(varsmap &vmap, assetVar *av)//,  cJSON *cj)
    {
        //the value has already been set
        // now run the actions
        assetAction* aa = av->actMap["onSet"];
        return runActValfromCj(vmap, av, aa);
    }

    // handles the complexity of value = as well as naked ? sets
    assetVar* setActBitMapfromCj(assetVar *av,  assetAction* aact, cJSON* cjbf, cJSON *cj)
    {
        cJSON *cji;
        //cJSON* cjbfm = cJSON_GetObjectItem(cjbf, "bitmap");

        if(cJSON_IsArray(cjbf))
        {
            cJSON_ArrayForEach(cji, cjbf)
            {
                // cJSON* cjmask = cJSON_GetObjectItem(cji, "mask");
                // cJSON* cjbit = cJSON_GetObjectItem(cji, "bit");
                // cJSON* cjuri = cJSON_GetObjectItem(cji, "uri");
                // cJSON* cjvar = cJSON_GetObjectItem(cji, "var");
                // cJSON* cjval = cJSON_GetObjectItem(cji, "bvalue");
                // assetBitField* abf = nullptr;
                if(0)
                {
                    char* stmp = cJSON_PrintUnformatted(cji);
                    FPS_ERROR_PRINT(" %s >>Whole Feat       >>%s<< child %p \n", __func__, stmp,(void *) cji->child);
                    free((void *)stmp);
                }
                aact->addBitField(cji);
            }
        }                       
        return av;
    }

    // handles the complexity of value = as well as naked ? sets
    assetVar*setActBitMapIffromCj(assetVar *av,  const char *act, const char *btype, cJSON* cjbf, cJSON *cj)
    {
        if(0)FPS_ERROR_PRINT( " %s >>  act [%s]  btype [%s]\n",__func__, act, btype);

        assetAction * aact;
        aact = av->actMap[act] = new assetAction(btype);
        //cJSON *cji;
        //FPS_ERROR_PRINT(" setActfromCj running array cjonset %p\n", (void *) cjonset);
        av = setActBitMapfromCj(av, aact, cjbf, cj);
        return av;
    }

    // TODO (DONE) remove layer connects to getting the bitmap 
    assetVar*setActOptsfromCj(assetVar *av,  const char* act, const char *opt, cJSON *cj)
    {
        if(0)FPS_ERROR_PRINT( " %s >>  act [%s] opt [%s] \n",__func__, act, opt);

        cJSON* cjact = cJSON_GetObjectItem(cj, act);
        if (cjact)
        {
            cJSON* cjopt = cJSON_GetObjectItem(cjact, opt);
            if(cjopt)
            {
                av = setActBitMapIffromCj(av,  act, opt, cjopt, cj);
            }
        }
        return av;
    }

    // looks for bitield enum
    assetVar*setActOptsfromCj(assetVar *av,  const char* act, cJSON *cj)
    {
        if(0)FPS_ERROR_PRINT( " %s >>  act [%s] \n",__func__, act);
        setActOptsfromCj(av,  act, "bitfield", cj);
        setActOptsfromCj(av,  act, "enum", cj);
        setActOptsfromCj(av,  act, "remap", cj);
        return av;
    }

    // handles the complexity of value = as well as naked ? sets
    // looks for onSet, onGet, onPub
    // TODO preserve action order 
    assetVar*setActfromCj(assetVar *av,  cJSON *cj)
    {
        setActOptsfromCj(av,  "onSet", cj);
        setActOptsfromCj(av,  "onGet", cj);
        setActOptsfromCj(av,  "onPub", cj);

        return av;
    }

    // TODO lock vmap , create unlocked option
    // handles the complexity of value = as well as naked ? sets

    assetVar* setOldValfromCj(varsmap &vmap,const char* comp, const char* var, cJSON* cj)
    {
        assetVar* av = nullptr;
        if(0)
        {
            char *tmp = cJSON_PrintUnformatted(cj);
            FPS_ERROR_PRINT(" %s Cj >>Setval for comp [%s] var  [%s] cjval >>%s<< \n"
                , __func__
                , comp?comp:"NoComp"
                , var?var:"NoVar"
                , tmp?tmp:"NoCjTmp"
                //, (void *)cjact                        
                );
            free((void*)tmp);
        }
        av = getVar(vmap, comp, var);

        if(    cJSON_IsNumber(cj)
            || cJSON_IsBool(cj)
            || cJSON_IsString(cj))
        {
            if (cJSON_IsNumber(cj))
            {
                double vv = cj->valuedouble;
                // should we use setVar here ??
                av->setVal(vv);
            }
            else if (cJSON_IsBool(cj))
            {
                bool vv = cJSON_IsTrue(cj);
                av->setVal(vv);
            }
            else if (cJSON_IsString(cj))
            {
                const char *vv = cj->valuestring;
                av->setVal(vv);
            }
            if (av->actMap.size() > 0)
            {
                if(av->actMap.find("onSet") != av->actMap.end())
                {
                    //TODO should we  force this yes bitfields need it
                    //av->aVal->valueint =  (int)av->aVal->valuedouble;

                    if(0)FPS_ERROR_PRINT(" >>Setting  act onSet value for [%s]\n",var);
                    setActValfromCj(vmap, av);//, cjact);
                }

            }
        }
        else
        {
            cJSON* cjval = cJSON_GetObjectItem(cj, "value");
            //cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
            if(0)
            {
                char * tmp = cJSON_PrintUnformatted(cj);
                FPS_ERROR_PRINT("%s ..2  setValfrom Cj.value >>Setval for cj >>%s<<\n type %d \n cjval %p \n"
                ,__func__
                ,tmp
                , cj->type
                , (void *)cjval
                //, (void *)cjact                        
                );
                free((void*)tmp);
            }
            if(cJSON_IsArray(cj))
            {
                int asize = cJSON_GetArraySize(cj) ;
                FPS_ERROR_PRINT(" setValfrom Cj >>Setval for cj  array size %d\n"
                    , asize
                    //, (void *)cjact                        
                    );
                if(asize == 1)
                {
                    //"ems_cmd":[{"value":0,"string":"Initial"}]
                    cJSON*cji = cJSON_GetArrayItem(cj, 0);
                    if(cji)
                        cjval = cJSON_GetObjectItem(cji, "string");
                }

            }
            if (cjval)
            {
                if(0)FPS_ERROR_PRINT(" >>Setting  value for [%s]\n",var);
                av = getVar(vmap, comp, var);
                if(av)
                {
                    av->setVal(cjval);
                }

                if(0)FPS_ERROR_PRINT(" >>After Setting  value for [%s] aval (f) %f  () %d \n"
                        , var  
                        , av->aVal->valuedouble
                        , av->aVal->valueint
                        );

            }
            // this should not be here after config but we may consider it as an idea
            if(av)
            {
                if (av->actMap.size() > 0)
                {
                    if(av->actMap.find("onSet") != av->actMap.end())
                    {
                        //TODO should  not force this
                        av->aVal->valueint =  (int)av->aVal->valuedouble;

                        FPS_ERROR_PRINT(" >>Setting  act onSet value for [%s]\n",var);
                        setActValfromCj(vmap, av);//, cjact);
                    }

                }
            } 
        }
        return av;
    }

    assetVar*setValfromCj(varsmap &vmap, const char* comp, const char* var, cJSON* cj)
    {
        if(0 || !var)
        { 
            char *tcj = nullptr;
            if(cj) tcj = cJSON_PrintUnformatted(cj);
            FPS_ERROR_PRINT("%s>> >>Seeking Variable comp [%s] var [%s] cj is %p [%s]\n"
                    , __func__
                    , comp
                    , var
                    , (void*)cj
                    , cj?tcj:"NoCj");
            if(tcj) free((void*)tcj);
        }
        // char* mycomp  = (char*)comp;
        // char *myvar  = (char*)var;

        // if(!var) 
        // {
        //     // break up uri into comp:var
        //     mycomp = strdup(comp);
        //     myvar = strstr(mycomp,":");
        //     if(myvar)
        //     {
        //         *myvar = 0;
        //         myvar++;
        //     }
        // }

        assetVar *av = getVar(vmap, comp, var);
        if(av)
        {
            return setOldValfromCj(vmap, comp, var, cj);
        }

        if(0)FPS_ERROR_PRINT(" >>Adding a new Variable comp [%s] var [%s] cj is %p\n"
                    , comp
                    , var
                    , (void*)cj);
        // this sections adds a new variable
        if (cJSON_IsNumber(cj))
        {
            double vv = cj->valuedouble;
            av = makeVar(vmap, comp, var, vv);
        }
        else if (cJSON_IsBool(cj))
        {
            bool vv = cJSON_IsTrue(cj);
            av = makeVar(vmap, comp, var, vv);
        }
        else if (cJSON_IsString(cj))
        {
            const char *vv = cj->valuestring;
            av = makeVar(vmap, comp, var, vv);
        }
        else
        {
            cJSON* cjval = cJSON_GetObjectItem(cj, "value");
            cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
            // TODO add more options here like deadband
            // test turn it off
            //cjact = nullptr;        
            char *tmp = cJSON_PrintUnformatted(cj);
            if(0) FPS_ERROR_PRINT(" setValfrom_Cj >>Adding NEW cj >>%s<<\n cjval %p \n cjact %p\n"
                , tmp
                , (void *)cjval
                , (void *)cjact                        
                );
            free((void*)tmp);
            assetVar* av = nullptr;
            if(cJSON_IsArray(cj))
            {
                int asize = cJSON_GetArraySize(cj) ;
                FPS_ERROR_PRINT(" setValfrom_Cj >>Setval for cj  array size %d\n"
                        , asize
                        //, (void *)cjact                        
                        );
                if(asize == 1)
                {
                    //"ems_cmd":[{"value":0,"string":"Initial"}]
                    cJSON*cji = cJSON_GetArrayItem(cj, 0);
                    if(cji)
                        cjval = cJSON_GetObjectItem(cji, "string");
                }
            }
            if (cjval)
            {
                if(0)FPS_ERROR_PRINT(" >>Adding CJ value for [%s]\n",var);
                av = setValfromCj(vmap, comp, var, cjval);
                //cJSON_Delete(cjval);
            }
            if (cjact)
            {
                if(0)FPS_ERROR_PRINT(" >>Adding actions for [%s] av [%p]\n",var, (void*)av);
                if(av)
                {
                    setActfromCj(av, cjact);
                }
            }

                        // todo set the rest
            // cJSON* cjval = cJSON_GetObjectItem(cj, "value");
            // if (cjval)
            // {
            //     setValfrom Cj(vmap, comp, var, cjval);
            //             // todo set the rest of the "features"
            // } else {
            //     const char *vv = cJSON_PrintUnformatted(cj);
            //     vmap[comp][var] = new assetVar(var, vv);
            //     free((void*)vv);
            // }
        }

        return av;
    }

    assetVar*getVar(varsmap &vmap, const char* comp, const char* var=nullptr)
    {
        char* mycomp = (char *)comp;
        char* myvar = (char *)var;

        assetVar* av = nullptr;
    
        if(var == nullptr)
        {
            // break up uri into comp:var
            mycomp = strdup(comp);
            myvar = strstr(mycomp,":");
            if(myvar)
            {
                *myvar = 0;
                myvar++;
            }
        }

        if(0)FPS_ERROR_PRINT("%s lookng for comp [%s] var [%s]\n"
                       , __func__
                        , mycomp
                        , myvar);

        auto ic = vmap.find(mycomp);
        if (ic != vmap.end())
        {
            auto iv = vmap[mycomp].find(myvar);
            if (iv != vmap[mycomp].end())
            {
                av = vmap[mycomp][myvar];
            }
        }
        if (mycomp != (char *)comp)
            free((void *)mycomp);
        return av;

    }

// TODO test comp /a/b:c  ok working
    template <class T>
    T getVar(varsmap &vmap, const char* comp, const char* var, T &value)
    {
        assetVar* av = getVar(vmap,comp,var);
        if (av)
        {
            return av->getVal(value);
        }
        return value;
    }

    // TODO lock varmap
    // get one or get them all
    cJSON* getMapsCj(varsmap &vmap , const char* uri = nullptr, const char* var = nullptr)
    {
        if(0)FPS_ERROR_PRINT("%s >> getting cj maps uri [%s] var [%s] \n"
            , __func__
            , uri ? uri:"noURI"
            , var?var:"noVar"
            );

        cJSON *cj = cJSON_CreateObject();
 
        if (uri&&!var)
        {
            auto x = vmap.find(uri);
            if( x != vmap.end() )
            {
                cJSON *cji = cJSON_CreateObject();

                for (auto &y : vmap[uri])
                {
                    if(0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                    y.second->showvarCJ(cji);
                    if(0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());

                }
                cJSON_AddItemToObject(cj, uri, cji);
            }
        }
        else if(uri && var)
        {
            auto x = vmap.find(uri);
            if( x != vmap.end() )
            {
                auto y = vmap[uri].find(var);
                if( y != vmap[uri].end() )
                {
                    //cJSON *cji = cJSON_CreateObject();

                    if(0)printf(" getting cj for uri [%s] var [%s] \n", uri, y->first.c_str());
                    y->second->showvarValueCJ(cj);
                    //cJSON_AddItemToObject(cj, var, cji);
                }
            }
        }
        else
        // get all the objects
        {
            for (auto &x : vmap)
            {
                if(0)printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj map for [%s] \n", x.first.c_str());
                cJSON *cji = cJSON_CreateObject();

                for (auto &y : vmap[x.first])
                {
                    if(0)printf("       >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj for var [%s]  map %p\n"
                                , y.first.c_str()
                                , (void *)y.second
                                );

                                
                    if(y.second != nullptr)
                    {    
                        y.second->showvarCJ(cji);
                    }
                    else
                    {
                            printf("       NOTE no map for var [%s] \n", y.first.c_str());
                    }
                        
                    if(0)printf("  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<     got cj for var [%s] \n", y.first.c_str());

                }
                cJSON_AddItemToObject(cj, x.first.c_str(), cji);
                if(0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<got cj map for [%s] \n", x.first.c_str());

            }
        }
        if(0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<got cj maps\n");

        return cj;
    }

//TODO check this out get /a/b:c perhaps
    void processMsgGet(varsmap &vmap, const char* method, const char* uri, const char* body, cJSON **cjr)
    {
        // TODO allow 3 level base uris /assets/bms/bms_1   .. maybe
        int nfrags = get_nfrags(uri);
        int nbase = 0;
        if(0)FPS_ERROR_PRINT("%s >> we're working on this one uri [%s] nfrags %d\n", __func__, uri, nfrags);
 
        if(nfrags == nbase+1)
        {
            *cjr = getMapsCj(vmap);
        }
        else if(nfrags == nbase+2)
        {
            if(0)FPS_ERROR_PRINT("%s >>  running with uri [%s] nfrags %d\n", __func__, uri, nfrags);
            *cjr = getMapsCj(vmap, uri);
            if(0)FPS_ERROR_PRINT("%s >> returned from uri [%s] nfrags %d\n", __func__, uri, nfrags);

        }
        else if(nfrags == nbase + 3)
        {
            char *svar = nullptr;
            char* smap = pull_pfrag(uri, nbase+1);// pfrags[0]);
            char* ssys = pull_pfrag(uri, nbase +2);//msg->pfrags[1]);
            char* asuri;
            asprintf(&asuri,"/%s/%s", smap, ssys);

            svar = pull_pfrag(uri,3);//->pfrags[2]);
            if(0)
            {
                FPS_ERROR_PRINT("%s  we got svar as [%s]\n", __func__, svar);
                FPS_ERROR_PRINT("%s  we got asuri as [%s]\n", __func__, asuri);
            }
            *cjr = getMapsCj(vmap, asuri, svar);
            if(smap) free((void *)smap);
            if(ssys) free((void *)ssys);
            if(asuri) free((void *)asuri);
            if(svar) free((void *)svar);
        }
    }

    void processMsgSetPub(varsmap &vmap, const char* method, const char* uri, const char* body,  cJSON **cjr)
    {
        cJSON*cj = cJSON_Parse(body);
        if(cj)
        {
            if (*cjr == nullptr);
                *cjr = cJSON_CreateObject();
            if(0)FPS_ERROR_PRINT("%s >> method %s [%s] cj->type %d , cj->child %p cj->next %p\n"
                    , __func__
                    , method
                    , cj->string
                    , cj->type
                    , cj->child
                    , cj->next);
            cJSON* cjrx = cj;
            if(cj->child)
                cjrx = cj->child;
            while(cjrx)
            {
                if(0)FPS_ERROR_PRINT(" %s >> [%s] cj->type %d , cj->child %p cj->next %p\n"
                    , __func__
                    , cjrx->string
                    , cjrx->type
                    , cjrx->child
                    , cjrx->next);
                setValfromCj(vmap, uri, cjrx->string, cjrx);
                cJSON *cji2 = cJSON_Duplicate(cjrx, true);
                cJSON_AddItemToObject(*cjr, cjrx->string, cji2);

                cjrx = cjrx->next;
            }
            cJSON_Delete(cj);
        }
        return;
    }

    void processMsgSetReply(varsmap &vmap, const char* method, const char* uri, const char* replyto, const char* body,  cJSON **cjr)
    {
        int nfrags = get_nfrags(uri);
        char *var = nullptr;
        cJSON*cj = cJSON_Parse(body);

        if(nfrags == 3)
            var = pull_pfrag(uri,3);
        

        if (*cjr == nullptr);
            *cjr = cJSON_CreateObject();

        cJSON* cji= nullptr;
        if(cj)
        {
            if(0)printf(" %s >>> method %s uri [%s] body [%s] cj->string [%s] cj->type %d , cj->child %p cj->next %p\n"
                    , __func__
                    , method
                    , uri
                    , body
                    , var?var:"noVar"
                    , cj->string
                    , cj->type
                    , cj->child
                    , cj->next);
            if(var)
            {
                if(0)printf("%s >>> [%s] cj->type %d , cj->child %p cj->next %p\n"
                    , __func__
                    , var
                    , cj->type
                    , cj->child
                    , cj->next);
                
                setValfromCj(vmap, uri, var, cj);
                if(0) FPS_ERROR_PRINT(" MSGSetReply we got a set with a replyto [%s]\n",replyto);
                cJSON *cji2 = cJSON_Duplicate(cj, true);
                cJSON_AddItemToObject(*cjr, var, cji2);
                free((void *)var);

            }
            else
            {

                cji = cj;
                if(cj->child)
                    cji = cj->child;

                while(cji)
                {
                    if(0)FPS_ERROR_PRINT(" [%s] cj->type %d , cj->child %p cj->next %p\n"
                        , cji->string
                        , cji->type
                        , cji->child
                        , cji->next);
                    
                    setValfromCj(vmap, uri, cji->string, cji);
                    if(0)FPS_ERROR_PRINT(" MSGSetReply we got a set with a replyto [%s]\n",replyto);
                    cJSON *cji2 = cJSON_Duplicate(cji, true);
                    cJSON_AddItemToObject(*cjr, cji->string, cji2);
                    cji = cji->next;
                }
            }
            cJSON_Delete(cj);
        }
        return;
    }

    void processRawMsg(varsmap &vmap, const char *method, const char* uri, const char*replyto, const char *body,
                        cJSON **cjr)
    {
        if (strcmp(method, "set")== 0)
        {
            if(replyto != nullptr)
                return processMsgSetReply(vmap, method, uri, replyto, body, cjr);
            else
                return processMsgSetPub(vmap, method, uri, body, cjr);
        }
        else if (strcmp(method, "pub")== 0)
            return processMsgSetPub(vmap, method, uri, body, cjr);
            
        else if (strcmp(method, "get")== 0)
            return processMsgGet(vmap, method, uri, body, cjr);
            
        return;
    }
    
 // all these assume nfrags == 2 except get
    // vmaps is really a name space , vm is an accessor to that name space
    void processFims(varsmap &vmap, fims_message* msg,  cJSON **cjr)
    {
        return processRawMsg(vmap, msg->method, msg->uri, msg->replyto , msg->body ,cjr);
    }

    void free_fims_message (fims_message *msg)
    {
        if(msg)
        {
            if(msg->method) free((void*)msg->method);
            if(msg->uri) free((void*)msg->uri);
            if(msg->replyto) free((void*)msg->replyto);
            if(msg->body) free((void*)msg->body);
            if(msg->pfrags) free((void*)msg->pfrags);
            
            free(msg);
        }
    }
    // turns body: method: replyto: uri: into a fims message ( we also need to go the other way.
    fims_message* bufferToFims(const char *buffer)
    {
    // pull out fields for return
        cJSON* root = cJSON_Parse(buffer);
        if(root == nullptr)
        {
            FPS_ERROR_PRINT("%s: Failed to parse message.\n", program_invocation_short_name);
            return nullptr;
        }

        fims_message *message = (fims_message*)malloc(sizeof (fims_message));

        if(message == nullptr)
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

        cJSON* body    = cJSON_GetObjectItem(root, "body");
        if(body != nullptr)
        {
            if (body->valuestring == nullptr)
            {
                message->body    = cJSON_PrintUnformatted(body);
            }
            else
            {
                message->body    = strdup(body->valuestring);
            }
        }
        

        cJSON* method  = cJSON_GetObjectItem(root, "method");
        if(method  != nullptr)
        {
            message->method  = strdup(method->valuestring);
        }

        cJSON* replyto = cJSON_GetObjectItem(root, "replyto");
        if(replyto != nullptr)
        {
            message->replyto = strdup(replyto->valuestring);
        }
        
        //return nullptr;
        cJSON* uri  = cJSON_GetObjectItem(root, "uri");
        if(uri     != nullptr)
        {
            message->uri     = strdup(uri->valuestring);
            // build pfrags
            int count = 0;
            int offset[MAX_URI_DEPTH];
            for(int i = 0; message->uri[i] != '\0' && count < MAX_URI_DEPTH; i++)
            {
                if(message->uri[i] == '/')
                {
                    offset[count] = i;
                    count++;
                }
            }
            message->nfrags = count;
            if(count > 0 && count < MAX_URI_DEPTH)
                message->pfrags = (char **) calloc(count, sizeof(char *)) ;
            else
            {
                FPS_ERROR_PRINT("%s: Invalid number of segments in URI", program_invocation_short_name);
            }
            for(int i = 0; i < count; i++)
            {
                message->pfrags[i] = message->uri + (offset[i] + 1);
            }
        }

        cJSON_Delete(root);
        return message;
    }

    // This function will create a  FIMS message buffer
    char* fimsToBuffer(const char* method, const char* uri, const char* replyto, const char* body)
    {
        if(method == nullptr || uri == nullptr)
        {
            FPS_ERROR_PRINT("%s: Can't transmit message without method or uri.\n", program_invocation_short_name);
            return nullptr;
        }
        // build json object
        cJSON *message = cJSON_CreateObject();
        if(message == nullptr)
        {
            FPS_ERROR_PRINT("%s: Memory allocation error.\n", program_invocation_short_name);
            return nullptr;
        }
        cJSON_AddStringToObject(message, "method", method);
        cJSON_AddStringToObject(message, "uri", uri);
        if(replyto != nullptr)
            cJSON_AddStringToObject(message, "replyto", replyto);
        if(body != nullptr)
            cJSON_AddStringToObject(message, "body", body);

        // create message buffer
        char* tmp_str = cJSON_PrintUnformatted(message);
        cJSON_Delete(message);
        return tmp_str;
    }

    

    int testRes(const char* tname, varsmap &vmap, const char* method, const char* uri,const char* body, const char* res )
    {
        int rc = 1;
        char *tmp;
        // this is our map utils factory
        VarMapUtils vm;
        cJSON* cjr = nullptr;//cJSON_CreateObject();
        const char*replyto = "/mee";
        vm.processRawMsg(vmap, method, uri, replyto, body, &cjr);
        tmp = cJSON_PrintUnformatted(cjr);
        if(strcmp(res,tmp)==0)
        {
            //printf(" PASSED \n");
            printf("%s\tPASSED reply >>%s<<\n", tname, tmp);
        }
        else
        {
            printf("%s\tFAILED reply >>%s<<\n", tname, tmp);
            rc =0;
        }
        free((void*)tmp);
        cJSON_Delete(cjr);
        return rc;
    }
#endif
};

// char *aprune_pfrag(const char *pfrag)
// {
//    char* comp_id = strdup(pfrag);
//    char* comp_end = strstr(comp_id,"/");
//    if(comp_end)
//        *comp_end = 0;
//    return comp_id;
// }

#endif
