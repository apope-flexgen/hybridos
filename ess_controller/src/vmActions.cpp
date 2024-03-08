// vmActions.cpp
// extract of the actions stuff
// p. wilshire
// 11/2/2021
// 11/8/2021  rework after doing review / docs
// 12/3/2021  looking for enum bug

#include "asset.h"
#include "assetVar.h"
#include "varMapUtils.h"
#include "formatters.hpp"

assetVar* setValfromAf(VarMapUtils*vm, varsmap& vmap, assetBitField* abf,  const char* uri
                , const char* var, assFeat*af, bool debug);

assetVar* setLocalValfromAf(VarMapUtils*vm, varsmap& vmap, const char* uri
                , const char* var, assFeat*af, bool debug);

assetVar* setOutValue(VarMapUtils*vm , varsmap& vmap, assetBitField* abf, assetVar* av, assFeat* outaf, bool debug);

bool setupAbf(VarMapUtils*vm, varsmap& vmap, assetBitField*abf, assetVar* av, bool debug);

void runAbfFuncAv(VarMapUtils* vm, varsmap &vmap, assetBitField* abf,  assetVar* av, bool trigger, bool debug);
bool getAbfFunction(VarMapUtils* vm, varsmap &vmap, assetBitField* abf,  assetVar* av, char *essName, bool debug);




// 1.1.0 rework
/**
 * @brief Parses what it can from the action item caches things that will not change and updates those that can change 
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var
 * @param debug local debug flag
 */    
 // bugfix may need to set val from param
assetVar* setAval(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    //assetVar*inAv = av;
    bool gotav   = abf->gotFeat("inAv");
    char *inuri = nullptr;
    if(gotav && !abf->inAvOK)
    {
        inuri = abf->getFeat("inAv", &inuri);
        if(debug) FPS_PRINT_INFO(" seeking  inAv [{}]  curr [{}]"
                , inuri
                , fmt::ptr(abf->inAv)
                );

        if(inuri)
        {
            assetUri my(inuri, nullptr);   // new 10.2 force my.Var to be found
            if(my.Param)
            {
                if(abf->inParam)
                {
                    free(abf->inParam);
                }
                abf->inParam = strdup(my.Param);
            }
            if(my.Var)
            {
                abf->inAv = vm->getVar(vmap, my.Uri, my.Var);
            }
            else
            {
                if(debug) FPS_PRINT_ERROR(" no var found in inAv [{}]"
                    , inuri
                    );
                abf->inAv = nullptr; 
                abf->inAvOK = true; // dont mess with this again
            }
        }
        // it was lost and now its found
        if(abf->inAv)
            abf->inAvOK = true;
    }
    if(debug && abf->inParam) FPS_PRINT_INFO(" setting inAv from inuri [{}] param [{}]", inuri, abf->inParam);
    if (!abf->inAv)
    {
        if(debug) FPS_PRINT_INFO(" setting inAv to av");
        abf->inAv = av;
    }
    // this was a bug... we may need the param
    //if(inuri)
    abf->setFeatfromAv("aVal", abf->inAv, abf->inParam);
    //else
    //    abf->setFeatfromAv("aVal", abf->inAv);
    if(0|| debug)
    {
        double adval ;
        double avdval ;
        avdval = abf->inAv->getdVal();
        adval = abf->getFeat("aVal",&adval);
        FPS_PRINT_INFO(" gotav [{}] aVal feat [{:2.3f}] dval [{:2.3f}] from [{}]"
                , gotav, adval, avdval, abf->inAv->getfName());
    }

    // save the aVal assFeat
    // TODO abf->getAf("aVal")
    abf->avaf = abf->getFeat("aVal");

    // apply scale and offset to the value
    // TODO abf->getAfType("aVal")
    auto  type = abf->getFeatType("aVal");
    abf->scale = 1.0;
    abf->offset = 0;
    if((type == (int)assFeat::AINT) || (type == (int)assFeat::AFLOAT))
    {
        double adval = abf->getFeat("aVal", &adval);
        double adval2 = abf->getFeat("aVal", &adval2);
        if(abf->gotFeat("scale"))
        {
            abf->scale = abf->getFeat("scale", &abf->scale);
            adval2 *= abf->scale;
        }

        if(abf->gotFeat("offset"))
        {
            abf->offset = abf->getFeat("offset", &abf->offset);
            adval2 -= abf->offset;
        }
        if(0|| debug)
        {
            FPS_PRINT_INFO(" scale [{}] offset [{}] adval [{:2.3f}] adval2(adj)  [{:2.3f}] from [{}]"
                , abf->scale, abf->offset, adval, adval2, abf->inAv->getfName());
        }
        abf->setFeat("aVal", adval2);
        double adval3 = abf->getFeat("aVal", &adval3);
        if(0|| debug)
        {
            FPS_PRINT_INFO("                                     adval3(adj)  [{:2.3f}] from [{}]"
                ,  adval3, abf->inAv->getfName());
        }

    }
    return abf->inAv;
}

// 1.1.0 rework
// set up "inValue" from inVar
// returns the  assFeat if there is one
/**
 * @brief Sets up the optional inValue from either a direct  value or an indirect value from inVar  inValue
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */    
 // 10.2 allow this to be a param.
assFeat* setinValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    bool gotav = abf->gotFeat("inVar");
    bool gotinv = abf->gotFeat("inValue");
    if(gotav && !abf->invAv)
    {
        char *inuri = abf->getFeat("inVar", &inuri);

        if(inuri)
        {
            assetUri my(inuri, nullptr);   // new 10.2 force my.Var to be found
            if(my.Param)
            {
                if(abf->invParam)
                {
                    free(abf->invParam);
                }
                abf->invParam = strdup(my.Param);
            }
            abf->invAv = vm->getVar(vmap, inuri, nullptr);
            if(!abf->invAv)
            {
                FPS_PRINT_ERROR(" Error setting up inVaR inuri {} No invaV", inuri?inuri:" No inuri");
            }
        }
    }
    if(debug)FPS_PRINT_INFO(" setting up inValue gotav {}  got inv {}", gotav, gotinv);
    if(gotav && abf->invAv)abf->setFeatfromAv("inValue", abf->invAv, abf->invParam);
    if(debug)FPS_PRINT_INFO(" done setting up inValue");

    if (abf->gotFeat("inValue"))
    {
        if(debug)FPS_PRINT_INFO(" used inValue [{}]",
                abf->getFeat("inValue")->valueint);

        return abf->getFeat("inValue");
    }
    return nullptr;
}

// 1.1.0 rework
// set up "outValue" from outVar
// returns the  assFeat
// also handles outTime
/**
 * @brief Sets up the optional outValue from either a direct value or an indirect value from outVar  outValue
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param
assFeat* getoutValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    bool gottime   = abf->gotFeat("outTime");
    bool gotav   = abf->gotFeat("outVar");
    if(gotav && !abf->outAv)
    {
        char *inuri = abf->getFeat("outVar", &inuri);
        if(inuri)
        {
            assetUri my(inuri, nullptr);   // new 10.2 force my.Var to be found
            if(my.Param)
            {
                if(abf->outParam)
                {
                    free(abf->outParam);
                }
                abf->outParam = strdup(my.Param);
            }
            abf->outAv = vm->getVar(vmap, inuri, nullptr);
            if(!abf->outAv)
            {
                FPS_PRINT_ERROR(" Error setting up outVar inuri {} No invaV", inuri?inuri:" No inuri");
            }
        }
        //if(inuri)abf->outAv = vm->getVar(vmap, inuri, nullptr);
    }
    if(gottime)
    {
        double tNow = vm->get_time_dbl();
        double tVal = abf->getFeat("outTime", &tVal);
        tVal += tNow;
        abf->setFeat("outValue", tVal);
        abf->outAv =  nullptr;  // forget about this one
    }
    else
    {
        if(abf->outAv)abf->setFeatfromAv("outValue", abf->outAv, abf->outParam);
    }    

    if (abf->gotFeat("outValue"))
        return abf->getFeat("outValue");
    return nullptr;
}

// 1.1.0 rework
// set up "outNValue" from outNVar (option) used when a match is missed
// returns the  assFeat
/**
 * @brief Sets up the optional outNValue from either a direct value or an indirect value from outNVar  outNValue
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param
assFeat* getoutNValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    bool gottime   = abf->gotFeat("outTime");
    bool gotav   = abf->gotFeat("outNVar");
    if(gotav)
    {
        assetVar* inAv = nullptr;
        char *inuri = abf->getFeat("outNVar", &inuri);
        if(inuri)inAv = vm->getVar(vmap, inuri, nullptr);
        if(inAv)abf->setFeatfromAv("outNValue", inAv);
    }
    if(gottime)
    {
        double tNow = vm->get_time_dbl();
        double tVal = abf->getFeat("outTime", &tVal);
        tVal += tNow;
        abf->setFeat("outNValue", tVal);
    }    

    return abf->getFeat("outNValue");
}

// 1.1.0 rework
// set up "ignValue" from ignVar
// returns the  assFeat
/**
 * @brief Sets up the optional ignoreValue from either a direct value or an indirect value from ignoreVar  ignValue
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param.

assFeat* getignValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{

    bool gotav   = abf->gotFeat("ignoreVar");
    if(gotav & !abf->ignAv)
    {
        char *inuri = abf->getFeat("ignoreVar", &inuri);
        if(inuri)abf->ignAv = vm->getVar(vmap, inuri, nullptr);
    }
    if(abf->ignAv)abf->setFeatfromAv("ignoreValue", abf->ignAv);
    return abf->getFeat("ignoreValue");
}

/**
 * @brief Sets up the optional var uri for some older func
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
assetVar* getVarAv(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{

    bool gotvar   = abf->gotFeat("var");
    if(gotvar & !abf->varAv)
    {
        char *varuri = abf->getFeat("var", &varuri);
        if(varuri)abf->varAv = vm->getVar(vmap, varuri, nullptr);
    }
    return abf->varAv;
}
// 1.1.0 rework
// set up high from highVar
// returns the  assFeat
/**
 * @brief Sets up the optional highValue from either a direct value or an indirect value from highVar or high
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param
assFeat* gethighValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    bool gotav   = abf->gotFeat("highVar");
    if(gotav)
    {
        assetVar* inAv = nullptr;
        char *inuri = abf->getFeat("highVar", &inuri);
        if (inuri)inAv = vm->getVar(vmap, inuri, nullptr);
        if(inAv)abf->setFeatfromAv("high", inAv);
    }
    return abf->getFeat("high");
}

// 1.1.0 rework
// set up high from highVar
// returns the  assFeat
/**
 * @brief Sets up the optional lowValue from either a direct value or an indirect value from lowVar or low
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param
assFeat* getlowValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{

    bool gotav   = abf->gotFeat("lowVar");
    if(gotav)
    {
        assetVar* inAv = nullptr;
        char *inuri = abf->getFeat("lowVar", &inuri);
        if(inuri)inAv = vm->getVar(vmap, inuri, nullptr);
        if(inAv)abf->setFeatfromAv("low", inAv);
    }
    return abf->getFeat("low");
    return nullptr;
}
// 1.1.0 rework
// set up "uriValue" from uri
// returns the  assFeat
/**
 * @brief Sets up uriValue assFeat from the uri
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param
assFeat* geturiValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    bool gotav   = abf->gotFeat("uri");
    if(gotav)
    {
        assetVar* inAv = nullptr;
        char *inuri = abf->getFeat("uri", &inuri);
        if(inuri)inAv = vm->getVar(vmap, inuri, nullptr);
        if(inAv)abf->setFeatfromAv("uriValue", inAv);
    }
    return abf->getFeat("uriValue");
}

// 1.1.0 rework
// set  "uriValue" from value in assFeat
// returns the  outAv
/**
 * @brief Sets up uri Value from assFeat from the uriVal object
 *
 * @param vm  VarMapUtils object
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  assetBitField structure
 * @param av   incoming assetVar var ( not used)
 * @param debug local debug flag
 */
 // 10.2 allow this to be a param
assetVar* seturiValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assFeat* af, bool debug)
{

    assetVar* outAv = nullptr;

    bool gotav   = abf->gotFeat("uri");
    if(gotav)
    {
        char *inuri = abf->getFeat("uri", &inuri);
        if(inuri)outAv = setValfromAf(vm, vmap,  abf, inuri, nullptr, af, debug);
    }
    return outAv;
}

// 1.2.0 preview
/**
 * @brief Sets the value of a Param from an assFeat object
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param vm  VarMapUtils object
 * @param uri  assetVar uri
 * @param var  assetVar var name
 * @param af assFeat object holding the value
 */       
assetVar* setParamfromAf(varsmap& vmap, VarMapUtils*vm, const char* uri
                , const char* var, assFeat*af, bool debug)
{
    assetVar* av=nullptr;
    if(!uri)
    {
        FPS_PRINT_ERROR("this needs a uri");
        return av;
    }
    assetUri my(uri, var);
    if(my.Uri && my.Var)
    {
        av = vm->getVar(vmap, my.Uri, my.Var);
        double dval = 0.0;
        if(!av)
        {
            av = vm->makeVar(vmap, my.Uri, my.Var, dval);
            FPS_PRINT_INFO(" created var for uri [{}] as [{}]"
                , my.Uri
                , av ? av->getfName():" no Av found"
                 );
            if(!av) return av;

        }
        if(my.Param)
        {
            if (my.index < 0)
            {
                //if(!av)av = vm->makeVar(vmap, my.Uri, my.Var, dval);
                if(af->type==assFeat::AINT)
                {
                    av->setParam(my.Param,af->valueint);
                }
                else if(af->type==assFeat::AFLOAT)
                {
                    av->setParam(my.Param,af->valuedouble);
                }
                else if(af->type==assFeat::ASTRING)
                {
                    av->setParam(my.Param,af->valuestring);
                }
                else if(af->type==assFeat::ABOOL)
                {
                    av->setParam(my.Param,af->valuebool);            
                }
                else
                {
                    if(1)FPS_PRINT_ERROR(" Unable to set param type {} for {}"
                        , af->type
                        , av->getfName()
                    );
                }
            }
            else // set an index value 
            {
                if(af->type==assFeat::ABOOL)
                {
                    if(av->extras)
                        av->extras->baseDict->setFeat(my.Param, my.index, af->valuebool);
                    //av->setParam(my.Param, my.index, af->valuebool);            
                }
                else
                {
                    if(1)FPS_PRINT_ERROR(" Unable to set index param type {} for {}"
                    , af->type
                    , av->getfName()
                    );
                }

            }
        }
    }
    return av;
}

//TODO move to assetVar.cpp
/**
 * @brief Sets the type of an assetVar, actions always setup type from the input value
 *
 * @param av  asssetVar pointer
 * @param type  forced type
 */    
void setaVType(assetVar* av, int type)
{
    if (av->linkVar)
       return setaVType(av->linkVar, type);

    switch (type)
    {
        case assetVar::AINT:
            av->aVal->type = assetVal::AINT;
            av->lVal->type = assetVal::AINT;
            av->type = assetVar::AINT;
            break;
        case assetVar::ABOOL:
            av->aVal->type = assetVal::ABOOL;
            av->lVal->type = assetVal::ABOOL;
            av->type = assetVar::ABOOL;
            break;
        case assetVar::AFLOAT:
            av->aVal->type = assetVal::AFLOAT;
            av->lVal->type = assetVal::AFLOAT;
            av->type = assetVar::AFLOAT;
            break;
        case assetVar::ASTRING:
            av->aVal->type = assetVal::ASTRING;
            av->lVal->type = assetVal::ASTRING;
            av->type = assetVar::ASTRING;
            break;
        default:
            break;

    }

}

/**
 * @brief Sets the fims value from an assFeat object
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param vm  VarMapUtils object
 * @param uri  assetVar uri can include a var / param but not an index yet
 * @param var  assetVar var name can be null
 * @param fmode "set" or "get" or null ("set"for set naked) 
 * @param af assFeat object holding the value
 */       
// TODO watch this will overwrite var
void setFimsfromAf(VarMapUtils*vm, varsmap& vmap, assetBitField* abf
                , const char* fmode, assFeat*af, bool debug)
{
    char* uri = nullptr;       if(abf->gotFeat("uri")) uri =abf->getFeat("uri", &uri);
    char* repto = nullptr;     if(abf->gotFeat("replyto")) repto =abf->getFeat("replyto", &repto);
    char* var = nullptr;       if(abf->gotFeat("var")) var =abf->getFeat("var", &var);
    char* fims_mode = nullptr; if(abf->gotFeat("mode")) var =abf->getFeat("mode", &fims_mode);
    if(!uri)
    {
        FPS_PRINT_ERROR(" error no uri for var [{}]"
            , var?var:"no Var"
            );
        return;
    }
    assetUri my(uri, var);
    bool is_get = strncmp("get",fmode, strlen(fmode)) == 0;
    if(!my.Var && !is_get)
    {
        FPS_PRINT_ERROR(" error no var for uri [{}] var [{}]"
            , uri
            , var?var:"no Var"
            );
        return;
    }

    std::string reptoStr;
    if(repto){
        reptoStr = std::string(repto);
        size_t pos = reptoStr.find("{essName}");
        if (pos != std::string::npos) {
            reptoStr.replace(pos, std::strlen("{essName}"), vm->getSysName(vmap));
        }
    }

    std::string sval;
    std::string uval = my.Uri;
    if(!my.Param && !is_get)
    {
        if(fims_mode) // any mode means naked 
        {
            uval = fmt::format(R"({}/{})", my.Uri, my.Var);
            if(af->type==assFeat::AINT)
            {
                sval = fmt::format(R"({})", af->valueint);
            }
            else if(af->type==assFeat::AFLOAT)
            {
                //I want   {"foo":{"value":1234}}
                sval = fmt::format(R"({})", af->valuedouble);
            }
            else if(af->type==assFeat::ASTRING)
            {
                sval = fmt::format(R"("{}")", af->valuestring);
            }
            else if(af->type==assFeat::ABOOL)
            {
                sval = fmt::format(R"({})", af->valuebool);
            }
            else
            {
                if(1)FPS_PRINT_ERROR(" Unable to set value type {} for {}"
                    , af->type
                    , uri
                    );
                    return;
            }
        }
        else
        {
            if(af->type==assFeat::AINT)
            {
                sval = fmt::format(R"({{"{}":{{"value":{}}}}})", my.Var, af->valueint);
            }
            else if(af->type==assFeat::AFLOAT)
            {
                //I want   {"foo":{"value":1234}}
                sval = fmt::format(R"({{"{}":{{"value":{}}}}})", my.Var, af->valuedouble);
            }
            else if(af->type==assFeat::ASTRING)
            {
                sval = fmt::format(R"({{"{}":{{"value":"{}"}}}})", my.Var, af->valuestring);
            }
            else if(af->type==assFeat::ABOOL)
            {
                sval = fmt::format(R"({{"{}":{{"value":{}}}}})", my.Var, af->valuebool);
            }
            else
            {
                if(1)FPS_PRINT_ERROR(" Unable to set value type {} for {}"
                    , af->type
                    , uri
                    );
                    return;
            }
        }
    }
    else if(my.Param) // set a param via do not disturb
    {
        if(af->type==assFeat::AINT)
        {
            sval = fmt::format(R"({{"{}":{{"value":"$$","{}":{}}}}})", my.Var, my.Param,af->valueint);

        }
        else if(af->type==assFeat::AFLOAT)
        {
            sval = fmt::format(R"({{"{}":{{"value":"$$","{}":{}}}}})", my.Var, my.Param,af->valuedouble);
        }
        else if(af->type==assFeat::ASTRING)
        {
            sval = fmt::format(R"({{"{}":{{"value":"$$","{}":"{}"}}}})", my.Var, my.Param,af->valuestring);
        }
        else if(af->type==assFeat::ABOOL)
        {
            sval = fmt::format(R"({{"{}":{{"value":"$$","{}":{}}}}})", my.Var, my.Param,af->valuebool);
        }
        else
        {
            if(1)FPS_PRINT_ERROR(" Unable to set value type {} for {}"
                , af->type
                , uri
                );
                return;
        }
    }
    if(debug)
    {
        FPS_PRINT_INFO(" sending fims output  {} for uri {} var {}"
            , sval
            , uval
            , var?var:"noVar"
            );
    }
    if(vm->p_fims)
    {
        // we already have the var in the message body
        if(fmode)
        {
            vm->p_fims->Send(fmode, uval.c_str(), reptoStr.c_str(), sval.c_str());
        }
        else
        {
            FPS_PRINT_ERROR(" no fmode for uri [{}] body [{}]"
                , uval
                , sval.c_str()
                );
        }
    }
    return;
}

/**
 * @brief Sets the value of an assetVar from an assFeat object
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param vm  VarMapUtils object
 * @param uri  assetVar uri
 * @param var  assetVar var name
 * @param af assFeat object holding the value
 * implies a force of the value type
 */       
 // 10.2 compact repeated code
assetVar* setValfromAf(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, const char* uri
                , const char* var, assFeat*af, bool debug)
{
    // bypass cascading actions
    if(abf)
    {
        if(abf->gotFeat("useAv"))
        {
            abf->useAv = abf->getFeat("useAv", &abf->useAv);
            if(abf->useAv)
                return setLocalValfromAf(vm, vmap, uri, var, af, debug);
        }
    }
    assetVar* av;
    if(!uri)
    {
        FPS_PRINT_ERROR(" error no uri for  var [{}]"
            , var?var:"no Var"
            );
        return nullptr;
    }
    assetUri my(uri, var);
    if(!my.Var)
    {
        FPS_PRINT_ERROR(" note: no var for uri [{}] var [{}]"
        , uri
        , var?var:"no Var"
        );
        return nullptr;
    }
    double dval = 0.0;

    assetVal* aVal = nullptr;
    if(my.Param)
    {
        if (debug) FPS_PRINT_INFO(" set Param [{}] for uri [{}]"
            , my.Param
            , uri
            );

        return setParamfromAf(vmap, vm, uri, var, af, debug);
    }
    else
    {
        av = vm->getVar(vmap, my.Uri, my.Var);
        // warning setting an index value does not trigger 
        // a vm->setval change .. yet
        if(my.index>=0)
        {
            if(af->type==assFeat::ABOOL)
            {
                if(!av)
                {
                    // this is broken 
                    av = vm->makeVar(vmap, my.Uri, my.Var, dval);
                }
                if(av)
                {
                    aVal= av->linkVar?av->linkVar->aVal:av->aVal;
                    aVal->setVal(my.index, af->valuebool);
                }
            }
        }
        else
        {
            //bool assetVal::setVal(int index, bool val)
            // dont forget useAV; see the next function
            //assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;

            if(af->type==assFeat::AINT)
            {
                av = vm->setVal(vmap, my.Uri, my.Var, af->valueint);

                if(0)FPS_PRINT_INFO(" setting INT value [{}] to av [{}]"
                    , af->valueint
                    , av->getfName()
                    );
                setaVType(av, (int)assetVar::AINT);
                setaVType(av, (int)assetVar::AFLOAT);

            }
            else if(af->type==assFeat::AFLOAT)
            {
                av = vm->setVal(vmap, my.Uri, my.Var, af->valuedouble);
                if(0)FPS_PRINT_INFO(" setting FLOAT value [{}] to av [{}]"
                    , af->valuedouble
                    , av->getfName()
                    );
                setaVType(av, (int)assetVar::AFLOAT);
            }
            else if(af->type==assFeat::ASTRING)
            {        
                if(debug)FPS_PRINT_INFO(" sending string [{}] to uri [{}] var [{}]"
                    , af->valuestring
                    , my.Uri
                    , my.Var
                    );
                av = vm->setVal(vmap, my.Uri, my.Var, af->valuestring);
                setaVType(av, (int)assetVar::ASTRING);
            }
            else if(af->type==assFeat::ABOOL)
            {
                av = vm->setVal(vmap, my.Uri, my.Var, af->valuebool);
                setaVType(av, (int)assetVar::ABOOL);
            }
            else
            {
                if(1)FPS_PRINT_ERROR(" Unable to set value type {} for {}"
                    , af->type
                    , uri
                    );
            }
        }
    }
    return av;
}

// this one avoids running the actions on the var
/**
 * @brief Sets the local value of an assetVar from an assFeat object
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param vm  VarMapUtils object
 * @param uri  assetVar uri
 * @param var  assetVar var name
 * @param af assFeat object holding the value
 */       
assetVar* setLocalValfromAf(VarMapUtils*vm, varsmap& vmap,  const char* uri
                , const char* var, assFeat*af, bool debug)
{
    assetVar* av = nullptr;
    if(!uri)
    {
        if(1)FPS_PRINT_ERROR(" no uri for {}"
            , var?var:"no Var"
            );
            return nullptr;

    }
    assetUri my(uri,var);
    av = vm->getVar(vmap,my.Uri, my.Var);
    double dval = 10.0;
    if(!av)av = vm->makeVar(vmap, my.Uri, my.Var, dval);
    if(!av)
    {
        if(1)FPS_PRINT_ERROR(" no av for uri {} var {}"
            , uri?uri:"no Uri"
            , var?var:"no Var"
            );
        return nullptr;

    }

    if(my.Param)
    {
        // params never trigger actions
        return setParamfromAf(vmap, vm, uri, var, af, debug);
    }
    else
    {
        if(my.index>=0)
        {
            if(af->type==assFeat::ABOOL)
            {
                assetVal* aVal= av->linkVar?av->linkVar->aVal:av->aVal;
                aVal->setVal(my.index, af->valuebool);
            }
        }
        else
        {
            // local set done when we detect useav
            if(af->type==assFeat::AINT)
            {
                av->setVal(af->valueint);
                setaVType(av, (int)assetVar::AINT);                
                setaVType(av, (int)assetVar::AFLOAT);                
            }
            else if(af->type==assFeat::AFLOAT)
            {
                av->setVal(af->valuedouble);
                setaVType(av, (int)assetVar::AFLOAT);
            }
            else if(af->type==assFeat::ASTRING)
            {
                av->setVal(af->valuestring);
                setaVType(av, (int)assetVar::ASTRING);
            }
            else if(af->type==assFeat::ABOOL)
            {
                av->setVal(af->valuebool);
                setaVType(av, (int)assetVar::ABOOL);
            }
            else
            {
                if(1)FPS_PRINT_ERROR(" Unable to set value type {} for {}:{}"
                    , af->type
                    , uri
                    , var
                    );
            }
        }
    }
    return av;
}

//runactbitma
//runActRemapfromCj
// converted to 1.1.0 
/**
 * @brief runs the remap operation from an array of remap assetitFields
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  group of assetActions
 * @param debug  debug flag
 */ 
 // TODO we don't use Cj anymore 
assetVar* VarMapUtils::runActRemapfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    if (debug)FPS_PRINT_INFO("#######Remap action started av [{}], value [{}] "
            , av->getfName()
            , av->getdVal()
            );
    char *essName = getSysName(vmap);
    //we can have several remap functions.
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        bool ldebug = false;

        if(abf->gotFeat("debug"))
        {
            ldebug = abf->getFeat("debug", &ldebug);
        }
        if (!setupAbf(this, vmap, abf, av, ldebug))
            continue;
        
        //assFeat* ignaf = getignValue(this, vmap, abf, av, debug);

        assFeat* outaf = abf->outaf;
        assFeat* avaf = nullptr;
        bool match = false;
        // nothing to compare to.
        if (!abf->inaf)
            match = true;

        avaf = abf->getFeat("aVal");
        
        if (avaf && abf->inaf)
        {
            match = assFeat_Compare(avaf, abf->inaf);
        }
        if(!match)
        {
            // allow outnvalue
            // move to setupAbf
            assFeat* outn = getoutNValue(this, vmap, abf, av, debug);
            if(outn)
            {
                match = true;
                outaf = outn;
            }
        }
        char* fmode = nullptr;      abf->getFeat("fims", &fmode);
        assetVar* tav = nullptr;
        char* func = nullptr;

        if (match)
        {
            if(!fmode)
            {
                if(ldebug)FPS_PRINT_INFO(" sending var to vmap " );
                //outav
                tav = setOutValue(this, vmap, abf, av, outaf, debug);
            }
            else
            {
                // not this does not discover a target assetVar
                if(ldebug)FPS_PRINT_INFO(" sending var to fims " );
                setFimsfromAf(this, vmap, abf, fmode, outaf, debug);
            }
            if(abf->gotFeat("func"))
            {
                if(!abf->fptr)
                {

                    func  = abf->getFeat("func", &func);
                    getAbfFunction(this, vmap, abf,  tav, essName, debug);
                }
                if(abf->fptr && tav)
                {
                    if (ldebug) FPS_PRINT_INFO("func [{}]-> [{}] using tav [{}]"
                        , func
                        , fmt::ptr(abf->fptr)
                        , tav?tav->getfName():"No tav");
                    bool xdebug = true;
                    bool trigger =  true;
                    runAbfFuncAv(this, vmap, abf,  tav, trigger, xdebug);
                }
            }
        }
    }
    if (debug)FPS_PRINT_INFO("#######Remap action completed av [{}]", av->getfName());
    return av;
}


// rework for 1.1.0 
/**
 * @brief runs the limits operation from an array of limit assetitFields
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  group of assetActions
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::runActLimitsfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;

        bool ldebug = false;
        if(abf->gotFeat("debug"))
        {
            ldebug = abf->getFeat("debug", &ldebug);
        }

        if (!setupAbf(this, vmap, abf, av, ldebug))
            continue;
        // assFeat* ignaf = getignValue(this, vmap, abf, av, debug);
        gethighValue(this, vmap, abf, av, ldebug);
        getlowValue(this, vmap, abf, av, ldebug);
        
        double inVal = abf->getFeat("aVal",&inVal);

        double low = inVal;
        double high = inVal;
        //bool runit = false;
        if(abf->gotFeat("low"))
        {
            low = abf->getFeat("low",&low);
        }
        if(abf->gotFeat("high"))
        {
            high = abf->getFeat("high",&high);
        }

        double aval = inVal;

        if (ldebug)FPS_PRINT_INFO(
            "###### av [{}] inVal [{}] LOW [{}] HIGH [{}]"
            , av->getfName()
            , inVal
            , low
            , high
        );
        bool limit = false;
        if (inVal < low)
        {
            limit = true;
            aval = low;
        }
        else if (inVal > high)
        {
            limit = true;
            aval = high;
        }
        else
        {
            aval = inVal;
        }
        // TODO add logging
        if (ldebug || limit)FPS_PRINT_INFO(
            "###### av [{}] value [{}] limited to [{}] LOW [{}] HIGH [{}]"
            , av->getfName()
            , inVal
            , aval
            , low
            , high
        );
        /// this does not allow cascading.... but its OK. 
        if(aval != inVal)
        {
            av->setVal(aval);
        }
    }
    return av;
}

//void runAbfFunction(VarMapUtils* vm, varsmap &vmap, assetBitField* abf,  assetVar* av, char *essName, bool debug)
bool getAbfFunction(VarMapUtils* vm, varsmap &vmap, assetBitField* abf,  assetVar* av, char *essName, bool debug)
{
    bool ldebug = false;
    asset* ai= nullptr;
    asset_manager* am= nullptr;

    if(abf->gotFeat("debug"))
    {
        ldebug = abf->getFeat("debug", &debug);
    }

    if (!setupAbf(vm, vmap, abf, av, ldebug))
        return false;

    // now need to see if we can find a function    
    assetVar* inAv =  abf->inAv;
    //int ret = -1;

    if (!abf->fptr)
    {

        abf->isAi = false;
        abf->isAm = false;

        // use the var feat if we found one.
        if (abf->varAv)
            inAv =  abf->varAv;

        if (!inAv->am)
        {
            inAv->am = vm->getaM(vmap, essName);
        }

        inAv->abf = abf;

        char* func = nullptr;
        //TODO this could be cached
        if(abf->gotFeat("func"))
        {
            func = abf->getFeat("func", &func);
        }
        if(func)
        {
            if (ldebug) FPS_PRINT_INFO("seeking Function [{}] for assetVar [{}]"
                , func
                , inAv->getfName()
                );
        }
        char* amap = nullptr;
        if(abf->gotFeat("amap"))
        {
            amap = abf->getFeat("amap", &amap);
        }
        if(amap)
        {
            am = vm->getaM(vmap, amap);
        }
        if(!am)
        {
            ai = vm->getaI(vmap, amap);
            if(!ai)
            {
                am = vm->getaM(vmap, essName);
                if (ldebug) FPS_PRINT_INFO("seeking Function [{}] for essName [{}]"
                    , func
                    , essName
                );
            }
        }

        if(amap == nullptr)
        {
            if(inAv->am)amap = (char*)inAv->am->name.c_str();  
        }
        char* essamap = nullptr;
        if(inAv->am) 
            essamap = (char*)inAv->am->name.c_str();

        
        if(ai)
        {
            abf->isAi = true;
            abf->amapptr = (void*)ai;
            if (ldebug) FPS_PRINT_INFO("seeking Function for an asset instance [{}]"
                , ai->name
                );
            //myAvfun_t aiFunc;
            void *res1 = nullptr;
            res1 = vm->getFunc(vmap, ai->name.c_str(), func);  // added change here -> should run func for arbitrary asset instances
            //if(!res1)res1 = getFunc(vmap, ai->name.c_str() , func);  // added change here -> should run func for arbitrary asset instances
            if(!res1) res1 = vm->getFunc(vmap, essamap, func);  // added change here -> should run func for arbitrary asset managers
            if(!res1)res1 = vm->getFunc(vmap, essName, func);
            abf->fptr = res1;

            
        }
        else if(am)
        {
            abf->isAm = true;
            abf->amapptr = (void*)am;
            //typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
            //myAvfun_t amFunc;
            void *res1 = nullptr;
            // ess could be the amap feature 
            // DONE  use the amap
            res1 = vm->getFunc(vmap, am->name.c_str(), func);  // added change here -> should run func for arbitrary asset managers
            if(!res1) res1 = vm->getFunc(vmap, essamap, func);  // added change here -> should run func for arbitrary asset managers
            if(!res1) res1 = vm->getFunc(vmap, essName, func);
            abf->fptr = res1;
           
        }
    }
    return true;
}

void runAbfFuncAv(VarMapUtils* vm, varsmap &vmap, assetBitField* abf,  assetVar* av, bool trigger, bool debug)
{
    asset* ai= nullptr;
    asset_manager* am= nullptr;
    int ret = -1;

    if (abf->fptr)
    {
        if (abf->isAi)
        {
            ai = (asset*)abf->amapptr; 
            myAvfun_t aiFunc;
            aiFunc = reinterpret_cast<myAvfun_t> (abf->fptr);
            if(trigger)ret = aiFunc(vmap, ai->amap, ai->name.c_str(), vm->p_fims, av);
            if (debug) FPS_PRINT_INFO(
                "OK ran trigger [{}] ret {} for asset   [{}] av [{}]"
                , trigger
                , ret
                , ai->name
                , av->getfName()
                );             
        }
        if (abf->isAm)
        {
            am = (asset_manager*)abf->amapptr;
            myAvfun_t amFunc;
            amFunc = reinterpret_cast<myAvfun_t> (abf->fptr);
            //if (ldebug) FPS_PRINT_ERROR( " Found an am [{}] for this variable [{}] func [{}] ", am->name, inAv->getfName(), func);
            if(!av->am) av->am = am;
            if(trigger) ret = amFunc(vmap, am->amap, am->name.c_str(), vm->p_fims, av);
            if (debug) FPS_PRINT_INFO(
                "OK ran trigger [{}] ret {} for asset manager  [{}] av [{}]"
                , trigger
                , ret
                , am->name
                , av->getfName()
                );             
        }
        return;
    }

}

void runAbfFunction(VarMapUtils* vm, varsmap &vmap, assetBitField* abf,  assetVar* av, bool trigger, bool debug)
{
    asset* ai= nullptr;
    asset_manager* am= nullptr;
    int ret = -1;
    assetVar* inAv =  abf->inAv;
    // use the var feat if we found one.
    if (abf->varAv)
        inAv =  abf->varAv;


    inAv->abf = abf;

    if (abf->fptr)
    {
        if (abf->isAi)
        {
            ai = (asset*)abf->amapptr; 
            myAvfun_t aiFunc;
            aiFunc = reinterpret_cast<myAvfun_t> (abf->fptr);
            if(trigger)ret = aiFunc(vmap, ai->amap, ai->name.c_str(), vm->p_fims, inAv);
            if (debug) FPS_PRINT_INFO(
                "OK ran trigger [{}] ret {} for asset   [{}] av [{}]"
                , trigger
                , ret
                , ai->name
                , inAv->getfName()
                );             
        }
        if (abf->isAm)
        {
            am = (asset_manager*)abf->amapptr;
            myAvfun_t amFunc;
            amFunc = reinterpret_cast<myAvfun_t> (abf->fptr);
            //if (ldebug) FPS_PRINT_ERROR( " Found an am [{}] for this variable [{}] func [{}] ", am->name, inAv->getfName(), func);
            if(trigger) ret = amFunc(vmap, am->amap, am->name.c_str(), vm->p_fims, inAv);
            if (debug) FPS_PRINT_INFO(
                "OK ran trigger [{}] ret {} for asset manager  [{}] av [{}]"
                , trigger
                , ret
                , am->name
                , inAv->getfName()
                );             
        }
        return;
    }

}

// rework for 1.1.0 
/**
 * @brief runs the func operation from an array of assetitFields
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  group of assetActions
 * @param debug  debug flag
 */ 
assetVar*VarMapUtils::runActFuncfromCj(varsmap &vmap, assetVar *av, assetAction* aa, bool debug)
{
    char *essName = getSysName(vmap);
    if (!av->am)
    {
         av->am = getaM(vmap, essName);
    }

    // This runs each function in the AbitMap
    // x.first is the entry number
    // this is the list of functions
    for (auto &x : aa->Abitmap)
    {
        // Set up the bitMap Number
        assetBitField* abf = x.second;
        bool trigger = true;
        trigger = getAbfFunction(this, vmap, abf, av, essName, debug);
        //if we have an invalue then this can disable trigger on a no match
        if(trigger)
        {
            if (abf->avaf && abf->inaf)
            {
                trigger = assFeat_Compare(abf->avaf, abf->inaf);
            }
            runAbfFunction(this, vmap, abf, av, trigger, debug);
        }
    }
    return av;
}

// TODO review this
/**
 * @brief runs the enum  operation 
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  an asset action item
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
bool checkEnumValue(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    bool triggered = false;
    if (abf->mask >= 0)
    {
        // do it by hand here
        double aVal = abf->getFeat("aVal", &aVal);
        double inValue = abf->getFeat("inValue", &inValue);
        int iaVal = (int)aVal;
        if(abf->mask>0)iaVal = (iaVal >> abf->shift) & abf->mask;
        double mVal = (double) iaVal;
        abf->setFeat("aVal", mVal);
        if (debug) FPS_PRINT_INFO(" #1 aVal [{:2.3f}] shift [{:d}] mask [{:04x}] iaVal [{:d}] inVal [{:d}] \n"
                    , aVal
                    , abf->shift
                    , abf->mask
                    , iaVal
                    , (int) inValue
                    );
        if (!abf->useRange)
        {
            if (iaVal == (int)inValue)
            {
                triggered  = true;
                if (debug) FPS_PRINT_INFO("#2 aVal [{:2.3f}] shift [{:d}] mask [{:04x}] iaVal [{:d}] inVal [{:d}] \n"
                    , aVal
                    , abf->shift
                    , abf->mask
                    , iaVal
                    , (int) inValue
                    );
            }
        }
        else
        {
            double rangeplus = 0.0;
            if(abf->gotFeat("inValue+"))
                rangeplus =  abf->getFeat("inValue+", &rangeplus);
            double rangeneg = 0.0;
            if(abf->gotFeat("inValue-"))
                rangeplus =  abf->getFeat("inValue-", &rangeneg);
            rangeplus += inValue;
            rangeneg = inValue - rangeneg;
            if ( (iaVal <= (int)rangeplus) && (iaVal >= (int)rangeneg))
            {
                triggered = true;
                if (debug) FPS_PRINT_INFO("aVal triggered [{:2.3f}] shift [{}] mask [{:04x}] iaVal [{}] inVal+ [{}] inVal- [{}] uri [{}]"
                    , aVal
                    , abf->shift
                    , abf->mask
                    , iaVal
                    , (int) rangeplus
                    , (int) rangeneg
                    , abf->uri
                    );
            }
        }
    }
    return triggered;
}

// TODO check consolidation of methods
/**
 * @brief sets the output value allws a ruri to redirect the root uri for the output 
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  an asset action item
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
assetVar* setOutValue(VarMapUtils*vm , varsmap& vmap, assetBitField* abf, assetVar* av, assFeat* outaf, bool debug)
{
    std::string turi = "";
    char* uri = abf->getFeat("uri", &uri);
    char* ruri = abf->getFeat("ruri", &ruri);
    char* var = abf->getFeat("var", &var);    // var can be null if we use /comp:var
    // dont cause a loop 
    if(uri == nullptr)
    {
        FPS_PRINT_ERROR("Error no Uri here  [{}]", av->getfName());
        return nullptr;
    }

    assetVar*tav = vm->getVar(vmap, uri, var);
    if((tav == av) && !abf->useAv)
    {
        if (abf->gotFeat("useAv"))
        {
            abf->useAv = abf->getFeat("useAv", &abf->useAv);
        }
        FPS_PRINT_ERROR("Warning possible loopbackvar [{}] check abf->useAv [{}]"
                , av->getfName()
                , abf->useAv
                );
    }
    
    if((tav == av) && !abf->useAv)
    {
        FPS_PRINT_ERROR("Warning possible loopbackvar [{}] no write"
                , av->getfName()
                );
        tav = nullptr;
    }
    else
    {
        // ruri 
        // get the value from uri which becomes the base cname
        // ruri is the destination var name
        // presume that var is crunched to null for this to work
        if(ruri)
        {
            if(debug)FPS_PRINT_INFO("Note possible ruri [{}]", av->getfName());
            assetVar* xav =  vm->getVar(vmap, uri, nullptr);
            if(xav) 
            {
                char* tmp = xav->getcVal();
                turi = fmt::format("{}:{}", tmp, ruri);
                uri = (char*)turi.c_str();
            }
        }
        tav = setValfromAf(vm, vmap, abf, uri, var, outaf, debug);
        if(debug)FPS_PRINT_INFO("####### >>>> Set uri [{}:{}] outaf  [{}]"
                    , uri ? uri:"no Uri"
                    , var ? var:" no Var"
                    , outaf?outaf->valuestring?outaf->valuestring:"no outaf string":"no outaf"
                    );
    }
    return tav;

}

// allow default uri 
// allow default value
// added inValue+ inValue 


/**
 * @brief runs the list of enum actions  
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  an asset action item
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::runActEnumfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    char* defUri = nullptr;
    if(av->gotParam("defUri"))
    {
        defUri = av->getcParam("defUri");
    }

    assFeat* defAf = nullptr;
    // TODO use new getParam  fixup
    //defAf = av->extras->baseDict->getFeat("defVal");
    if(av->gotParam("defVal"))
    {
        defAf = av->extras->baseDict->featMap["defVal"];
    }

    if(debug)FPS_PRINT_INFO("running >> debug [{}] av [{}]  value [{:2.3f}]"
        , debug
        , av->getfName()
        , av->getdVal()
        );
    // used to triggger the default action
    int fcount = 0;

    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        bool ldebug = false;
        assetVar* tav = nullptr;        
        if(abf->gotFeat("debug"))
        {
            ldebug = abf->getFeat("debug", &ldebug);
        }
        if (!setupAbf(this, vmap, abf, av, ldebug))
            continue;

        bool triggered = checkEnumValue(this, vmap, abf, av, ldebug);

        // TODO nval
        if(!triggered) continue;

        fcount++;
        tav = setOutValue(this, vmap, abf, abf->inAv, abf->outaf, ldebug);
        char *essName = getSysName(vmap);
        char *func = nullptr;
        if(abf->gotFeat("func"))
        {
            if(!abf->fptr)
            {

                func  = abf->getFeat("func", &func);
                getAbfFunction(this, vmap, abf,  tav, essName, ldebug);
            }
            if(abf->fptr)
            {
                if (debug) FPS_PRINT_INFO("func [{}]-> [{}] using tav [{}]"
                    , func
                    , fmt::ptr(abf->fptr)
                    , tav?tav->getfName():"No tav");
                bool xdebug = true;
                bool trigger =  true;
                runAbfFuncAv(this, vmap, abf,  tav, trigger, xdebug);
            }
        }
    }

    if (debug) FPS_PRINT_INFO("fcount {} using default uri [{}]", fcount, defUri?defUri:" no Def Uri");

    if((fcount==0) && defUri && defAf)
    {
        FPS_PRINT_INFO("using default uri [{}]", defUri);

        assetVar* tav = getVar(vmap, defUri, nullptr);
        if(tav == av)
        {
            FPS_PRINT_INFO("Warning possible loopbackvar [{}]", av->getfName());
        }
        else
        {
            //setvar_debug = 1;
            setValfromAf(this, vmap, nullptr, defUri, nullptr, defAf, debug);
            //setvar_debug = 0;
        }
    }

    return av;
}


/**
 * @brief parses the assetBitField and caches the result
 *
 * @param vm   VarMapUtils pointer 
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf  an asset action item
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
bool setupAbf(VarMapUtils*vm, varsmap& vmap, assetBitField*abf, assetVar* av, bool debug)
{
    bool ldebug = false;
    if(abf->gotFeat("debug"))
    {
        ldebug = abf->getFeat("debug", &debug);
    }

    if(!checkabfEnable(vm, vmap, abf, av, ldebug))
        return false;
    if(!checkabfChanged(vm, vmap, abf, av, ldebug))
        return false;

    abf->inAv = setAval(vm, vmap, abf, av, ldebug);
    if(ldebug)FPS_PRINT_INFO(" =====   inAv is now [{}]", 
            abf->inAv?abf->inAv->getfName():" no inAv");

    if(ldebug)FPS_PRINT_INFO(" **** initial avaf type [{}] (AINT [{}] AFLOAT [{}]) int [{}] double [{}] for av [{}] "
        , abf->avaf->type
        , assFeat::AINT
        , assFeat::AFLOAT
        , abf->avaf->valueint
        , abf->avaf->valuedouble
        , av->getfName()
        );
    abf->outaf = getoutValue(vm, vmap, abf, av, ldebug);
    abf->inaf = setinValue(vm, vmap, abf, av, ldebug);

    if (!abf->outaf)
    {
        if(ldebug)
        {
            double d = abf->getFeat("aVal", &d);
            FPS_PRINT_INFO(" setting outaf from aVal [{}] av [{}] "
                , d
                , av->getfName()
                );
        }

        abf->outaf = abf->getFeat("aVal");
    }

    abf->ignaf = getignValue(vm, vmap, abf, av, ldebug);
    if (abf->avaf && abf->ignaf)
    {
        if(vm->assFeat_Compare(abf->avaf, abf->ignaf, ldebug))
        {
            if(ldebug)
            {
                FPS_PRINT_INFO(" =====  skipping due to ignoreValue" );
                FPS_PRINT_INFO(" =====  avaf [{}]  ingnaf [{}] "
                    , abf->avaf->valuedouble 
                    , abf->ignaf->valuedouble 
                    );
            }
            return false;
        }
    }
    if(!abf->setup)
    {
        if(ldebug)FPS_PRINT_INFO(" av  [{}] running setup", av->getfName());
        //idx++;
        // 
        abf->useSet = true;
        if (abf->gotFeat("useSet"))
            abf->useSet   = abf->getFeat("useSet",&abf->useSet);

        abf->useRange = false;
        if(abf->gotFeat("useRange"))
            abf->useRange = abf->getFeat("useRange", &abf->useRange);


        abf->bit = 0;
        bool gotbit   = abf->gotFeat("bit");
        if(gotbit)
            abf->bit = abf->getFeat("bit", &abf->bit);

        abf->mask = 0;
        bool gotmask  = abf->gotFeat("mask");
        if(gotmask)
        {
            abf->mask = abf->getFeat("mask", &abf->mask);
        }
        abf->shift = 0;
        bool gotshift = abf->gotFeat("shift");
        if(gotshift)
        {
            abf->shift = abf->getFeat("shift", &abf->shift);
        }

        abf->uri = abf->getFeat("uri", &abf->uri);
        abf->var = abf->getFeat("var", &abf->var);
        if(abf->var && !abf->varAv)abf->varAv = getVarAv(vm, vmap, abf, av, ldebug);

        abf->setup = true;
    }
    abf->adval = 0;
    if(abf->inAv) abf->adval = abf->inAv->getdVal();        
    abf->aval = (int)abf->adval;
    abf->trigger = false;
    // then get outValue
    if(ldebug)
    {
        FPS_PRINT_INFO(" av  [{}] completed", av->getfName());
        FPS_PRINT_INFO(" OUTPUT outaf type [{}] (AINT [{}] AFLOAT [{}]) valueint [{}] valuedouble [{}] for av [{}] "
            , abf->outaf->type
            , assFeat::AINT
            , assFeat::AFLOAT
            , abf->outaf->valueint
            , abf->outaf->valuedouble
            , av->getfName()
            );
        FPS_PRINT_INFO(" INPUT avaf type [{}] (AINT [{}] AFLOAT [{}]) valueint [{}] valuedouble [{}] for av [{}] "
            , abf->avaf->type
            , assFeat::AINT
            , assFeat::AFLOAT
            , abf->avaf->valueint
            , abf->avaf->valuedouble
            , av->getfName()
            );
    }
    return true;    
}

// bitfield
/**
 * @brief runs the bitfield actions 
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  the  asset action item vector
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::runActBitFieldfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        bool ldebug = false;
        if(abf->gotFeat("debug"))
        {
            ldebug = abf->getFeat("debug", &debug);
        }

        if (!setupAbf(this, vmap, abf, av, ldebug))
            continue;
 
        if (abf->aval & (1 << (int)abf->bit))
        {
            abf->trigger = true;
        }
        
        if(abf->trigger)
        {
            if(abf->uri)
            {
                setValfromAf(this, vmap, abf, abf->uri
                        , abf->var, abf->outaf, debug);
            }
        }
    }

    return av;
}

// Bitset
// if true sets bits 
// if false clears bits
/**
 * @brief runs the bitset actions 
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  the  asset action item vector
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::runActBitSetfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        bool ldebug = false;
        if(abf->gotFeat("debug"))
        {
            ldebug = abf->getFeat("debug", &ldebug);
        }

        if (!setupAbf(this, vmap, abf, av, ldebug))
            continue;

        bool bval = abf->inAv->getbVal();
        
        abf->trigger = true;

        bool soloBit = false;
        if(abf->gotFeat("soloBit"))
        {
            soloBit = abf->getFeat("soloBit", &soloBit);
        }

        assetVar* outav = nullptr;
        if (!abf->uri)
        {
            FPS_PRINT_ERROR("needs a Uri for [{}]", av->getfName());
            continue;
        }

        outav = getVar(vmap, abf->uri, abf->var);
        if (outav)
        {
             abf->adval = outav->getdVal();
        }
        else
        {
            abf->adval = 0;
            FPS_PRINT_INFO("setting up  NEW outav for [{}:{}]", abf->uri, abf->var?abf->var:"no Var");
            outav = setVal(vmap, abf->uri, abf->var, abf->adval);
        }
       
        abf->aval = (int)abf->adval;
        int bit = abf->bit;
        if(abf->trigger)
        {   
            if (bval)
            {
                // set the bit
                if (soloBit)
                    abf->aval = bit ? 1 << (bit - 1) : 0;
                else if (bit)
                    abf->aval |= 1 << (bit - 1);
            }
            else
            {
                // clear the bit
                if (!soloBit)
                    abf->aval = abf->aval & ~(1 << (bit-1));
                else
                    abf->aval = 0;
            }
            abf->adval = (double)abf->aval;
            setVal(vmap, abf->uri, abf->var, abf->adval);
             if (ldebug)FPS_PRINT_INFO(
                "###### onSet BitSet action enable [{}] bit [{}] bval [{}]  bit val {:#04x} output val {:#04x}"
                " uri [{}] var [{}] soloBit [{}]"
                //, (int)mask
                //, (int)bit
                , abf->trigger
                , (int)bit
                , bval
                , (int)(1 << bit)
                , bval ? (int)(abf->aval | (1 << (bit-1))) : (int)(abf->aval & ~(1 << (bit-1)))
                , abf->uri
                , abf->var?abf->var:"no Var"
                , soloBit
                );
        }
    }
    return av;
}

// v 1.1.0

// may not be used 
// bool checkEnable(varsmap& vmap, VarMapUtils*vm,  assetVar* av,assetBitField* abf, bool debug)
// {
//     if (debug)FPS_PRINT_INFO(">> ####### [{}] action 1\n"
//             , av->getfName()
//             );
//     // check for inline enable or enable var 
//     bool enable = true;
//     char* enablevar = NULL;
//     if (abf->gotFeat("enable"))
//     {
//         enablevar = abf->getFeat("enable", &enablevar);
//         if (debug)FPS_ERROR_PRINT(" %s >> ####### [%s] action check enable av [%s]\n"
//             , __func__
//             , av->getfName()
//             , enablevar? enablevar:"no enable var"
//             );
//     }
//     if (enablevar)
//     {
//         assetVar* enav = vm->getVar(vmap, (const char*)enablevar, NULL);
//         if (enav)
//         {
//             enable = enav->getbVal();
//         }
//         if (!enable)
//         {
//             if (debug)FPS_ERROR_PRINT(" %s >> ####### action check enableav [%s] enable [%s]\n"
//                 , __func__
//                 , enablevar
//                 , enable?"true":"false"
//                 );

//             return false;
//         }
//     }
//     else
//     {
//         if (abf->gotFeat("enabled"))
//         {
//             bool enabled = abf->getFeat("enabled", &enabled);
//             if(!enabled)
//             {
//                 if (debug)FPS_ERROR_PRINT(" %s >> var [%s] not enabled  ignoring  \n"
//                     , __func__, av->getfName());
//                 return false;
//             }
//         }
//     }
//     return true;
// }

// todo remove this
// the outval is either the value of the main av
// or indirected by inVar
// bool getoutVal(varsmap& vmap, VarMapUtils*vm, cJSON** cjov, assetVar* av, assetVar**aV2, assetBitField* abf, bool debug)
// {
//     // TODO we could have an outVar here.
//     if (abf->gotFeat("outValue"))
//     {
//         *cjov = abf->getFeat("outValue", cjov);
//     }
//     else
//     {
//         *cjov = av->getValCJ(/*scale, offset*/);
//     }
//     return true;
// }

// Bitmap
// if true sets bits 
// if false clears bits
/**
 * @brief runs the bitmap actions 
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  the  asset action item vector
 * @param debug  debug flag
 */ 

assetVar* VarMapUtils::runActBitMapfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    int idx = 0;
    if(debug)FPS_PRINT_INFO(" running ===== for [%s]\n"
            , av?av->getfName():" no Av"
            );

    for (auto& x : aa->Abitmap)
    {
        assetBitField* abf = x.second;
        bool ldebug = false;
        if(abf->gotFeat("debug"))
        {
            ldebug = abf->getFeat("debug", &debug);
        }

        if (!setupAbf(this, vmap, abf, av, ldebug))
            continue;

        idx++;
        double adval = abf->getFeat("aVal", &adval);
        geturiValue(this, vmap, abf, av, debug);
        double outdval =  0;
        if(abf->gotFeat("uriValue"))
            outdval = abf->getFeat("uriValue", &outdval); 
        int outval = (int)outdval;

        int bit = 0;
        int mask = 0;
        // if the masked and shifted aVal == inVal then set or clear the bit in the uri 
        if(abf->shift >= 0)
        {
            bit = abf->bit << abf->shift;
            mask = (int)abf->mask << abf->shift;
        }
        bool set = true;
        double indval =  0;
        int inval =  0;
        if (abf->inaf)
        {
            indval = abf->getFeat("inValue", &indval); 
            inval = (int)indval;
            set = (mask & int(inval) & bit);
            if(ldebug)
                FPS_PRINT_INFO(" calculating set [{}] #1  from  mask [{:04x}] aval [{:04x}] inval [{:04x}]\n"
                    , set ? "true":"false"
                    , mask  
                    , (int)adval
                    , inval
                );

        }
        else
        {
            set = (mask & int(adval) & bit);
            if(ldebug)
                FPS_PRINT_INFO(" calculating set [{}] #2 from  mask [{:04x}] aval [{:04x}] Aval [{:04x}]\n"
                    , set ? "true":"false"
                    , mask  
                    , (int)adval
                    , inval
                );

        }

        if (set)
        {
            int old_outval = outval; 
            if (abf->useSet)
            {
                outval |= (int)(1<<bit);
            }
            else
            {
                outval &= ~(int)(1<<bit);
            }
            outdval =  outval;
            abf->setFeat("uriValue", outdval); 
            assFeat* af =   abf->getFeat("outValue"); 
            seturiValue(this, vmap, abf, af, debug);
            if (ldebug)
            {
                char* uri = abf->getFeat("uri", &uri);

                FPS_PRINT_INFO("######1 idx [{}] BitMap uri [{}] adval [{}] ahex [{:04x}] "
                        "using shift mask and af bit [{:04x}] mask [{:04x}] set [{}]"
                    , idx
                    , uri?uri:"no Uri"
                    , (int)adval
                    , (int)adval
                    , bit
                    , mask
                    , set
                    );

                    FPS_PRINT_INFO("######1 idx [{}] shift [{}] aval [{:04x}] aval >>shift [{:04x}] "
                        " mask >> shift [{:04x}]  bit>>shift  [{}] old_outval [{}] -> outval[{}] af [{}]"
                    , idx
                    , abf->shift
                    , (int)adval
                    , (int)adval>>abf->shift
                    , abf->mask>>abf->shift
                    , bit>>abf->shift
                    , old_outval
                    , outval
                    , af?af->valuebool?"true":"false":"noaf"
                    );
            }
        }
    }
    return av;
}

// runs on pubs or sets when the assetVar has actions.
/**
 * @brief runs all the action lists associated with a pub or set operation  
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  the  asset action item vector
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::runActValfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    if (aa)
    {
        double tNow = 0.0;
        if(av->am)
        {
            //Todo build up a stack of actions executed for backtrace.
            // schedav, av, tRun
            tNow = get_time_dbl();
            double tRun = tNow-schedTime;
            schedActions++;
            if (schedActions == 1)
            {
                if(!schedaV)
                {
                    schedaV = av;
                }
                if(0)if (debug) FPS_PRINT_INFO("####### onSet action: Set up schedAv [{}] av [{}] tRun(mS) {:2.3f} loops {}"
                    , schedaV?schedaV->getfName():"no SchedAv"
                    , av->getfName()
                    , tRun * 1000.0
                    , schedActions
                    );
            }
            if(0)if (debug) FPS_PRINT_INFO("####### onSet action: schedAv [{}] av [{}] tRun(mS) {:2.3f} loops {}"
                , schedaV?schedaV->getfName():"no SchedAv"
                , av->getfName()
                , tRun * 1000.0
                , schedActions
                );
            if(schedActions> 128)  // Limit for Max Actions....  TODO provide test example
            {
                if (1||debug) FPS_PRINT_ERROR("####### onSet action: Max Loops schedAv [{}] av [{}] tRun(mS) {:2.3f} loops {}"
                    , schedaV?schedaV->getfName():"no SchedAv"
                    , av->getfName()
                    , tRun * 1000.0
                    , schedActions
                    );
                if ( schedaV ) 
                {
                    schedaV->setParam("enabled",false);
                }
                else
                {
                    av->setParam("enabled",false);
                }
                // reset here 
                //av->am->vm->schedActions = 0;
            }
        }
        if (0) FPS_PRINT_INFO("####### on Set action aa->name [{}]"
               , aa->name);
        av->aa = aa;
        if (aa->name == "bitfield")
        {
            // tested OK
            if (0) FPS_PRINT_INFO("Running action [{}]"
               , aa->name);
            av = runActBitFieldfromCj(vmap, av, aa, debug);
        }
        else if (aa->name == "bitset")
        {
            av = runActBitSetfromCj(vmap, av, aa, debug);
        }
        else if (aa->name == "enum")
        {
            av = runActEnumfromCj(vmap, av, aa, debug);
        }
        else if (aa->name == "remap")
        {
            av = runActRemapfromCj(vmap, av, aa, debug);
        }
        else if (aa->name == "func")
        {
            av = runActFuncfromCj(vmap, av, aa, debug);
        }
        else if (aa->name == "limits")
        {
            av = runActLimitsfromCj(vmap, av, aa, debug);
        }
        else if (aa->name == "bitmap")
        {
            av = runActBitMapfromCj(vmap, av, aa, debug);
        }
        // else if (aa->name == "clone")
        // {
        //     av = runActClonefromCj(vmap, av, aa, debug);
        // }
    }

    return av;
}


/**
 * @brief documents the action functions available
*
 * @param vmap the global data map shared by all assets/asset managers
 * @param fname  context name associated with the action
 */ 
void VarMapUtils::setActions(varsmap& vmap, const char* fname)
{
    setAction(vmap, fname, "bitfield", (void*)nullptr,"decode value bits  into a number  of different values");
    setAction(vmap, fname, "bitset", (void*)nullptr,"set or clear a bit in an output var");
    setAction(vmap, fname, "enum", (void*)nullptr,"decode a value into a number of different values");
    setAction(vmap, fname, "remap", (void*)nullptr,"forward a value to a different uri");
    setAction(vmap, fname, "limit", (void*)nullptr,"set limits on a value");
    setAction(vmap, fname, "func", (void*)nullptr,"run a func using an assetVar as an argument");
    setAction(vmap, fname, "bitmap", (void*)nullptr,"use a bitmap to set the output variable");
//    setAction(vmap, fname, "clone", (void*)nullptr,"expand a template into a mapped system");
}

/**
 * @brief compares two assFeat objects 
 *
 * @param a  the first assFeat 
 * @param b  the second assFeat 
 * @param debug  debug flag
 */ 
bool VarMapUtils::assFeat_Compare(assFeat *a, assFeat *b, bool debug)
{
    
    if(debug)FPS_PRINT_INFO("a->type {} b->type {} AFLOAT {} INT {} BOOL {}"
        , a->type
        , b->type
        , assFeat::AFLOAT
        , assFeat::AINT
        , assFeat::ABOOL
        );
    if(a->type == b->type)
    {
        if((a->type == assFeat::ABOOL ) && (a->valuebool == b->valuebool)) return true; 
        if(a->type == assFeat::AFLOAT) 
        {
            double dval = std::abs(a->valuedouble-b->valuedouble);
            if(debug)FPS_PRINT_INFO("dval  {}", dval);
            if ( dval < 0.001) return true;
        }
        if(a->type == assFeat::AINT) 
        {
            if (a->valueint == b->valueint ) return true;
        }
        else if(a->type == assFeat::ASTRING) 
        {
            if(strcmp(a->valuestring, b->valuestring) == 0) return true;
        }
    }
    return false;
}

// Deprecated and its gone !!!
// /**
//  * @brief compares two cJSON objects 
//  *
//  * @param a  the first object 
//  * @param b  the second object 
//  * @param debug  debug flag
//  */ 
// bool VarMapUtils::cJSON_Compare(cJSON *a, cJSON*b)
// {
//     if(!a->child ) return false;
//     if(!b->child ) return false;
//     a = a->child;
//     b = b->child;
    
//     if(0)FPS_PRINT_INFO("a->type {} b->type {}", a->type, b->type);
//     if(a->type == b->type)
//     {
//         if((a->type == cJSON_False ) || (a->type == cJSON_True)) return true; 
//         else if(a->type == cJSON_Number ) 
//         {
//             if (std::abs(a->valuedouble-b->valuedouble) < 0.001) return true;
//         }
//         else if(a->type == cJSON_String) 
//         {
//             if(strcmp(a->valuestring, b->valuestring) == 0) return true;
//         }
//     }
//     return false;
// }

/**
 * @brief check abf for an enable var  
 *
 * @param vm VarMapUtils handler
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf incoming asset Item
 * @param av  assetVar 
 * @param debug  debug flag
 */ 
bool checkabfEnable(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug)
{
    char* ensp = nullptr;
    bool enabled = true;

    // note slight anomaly.... we cannot turn this on or off
    if (abf->gotFeat("enable"))
    {
        auto  type = abf->getFeatType("enable");

        if(type == (int)assFeat::ASTRING)
        {
            ensp = abf->getFeat("enable", &ensp);
            if (debug) FPS_PRINT_INFO("#######  check enable av [{}] enable [{}]"
                , av->getfName()
                , ensp ? ensp : "no enable"
                );
            if (ensp)
            {
                if(!abf->enAv)
                    abf->enAv = vm->getVar(vmap, (const char*)ensp, nullptr);
        
                if (abf->enAv)
                {
                    enabled = abf->enAv->getbVal();
                }
                if (debug) FPS_PRINT_INFO("####### check enable value av [{}] enable [{}]"
                    , fmt::ptr(abf->enAv)
                    , enabled
                );
                if(enabled) return enabled;
            }
        }
        if(type == assFeat::ABOOL)
        {
            enabled = abf->getFeat("enable", &enabled);
            if(enabled) return enabled;            
        }

    }
    // this one must be a bool
    if (abf->gotFeat("enabled"))
    {
        enabled = abf->getFeat("enabled", &enabled);
        if(!enabled)
        {
            if (debug) FPS_PRINT_INFO("var [{}] not enabled ignoring"
                    , av->getfName() 
                    );
            return enabled;
        }
    }

    return enabled;
}

/**
 * @brief runs all the action lists associated with a pub or set operation  
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param aa  the  asset action item vector
 * @param debug  debug flag
 */ 
// get value changed status
// TODO cache this
/**
 * @brief check that the value has changed
 *
 * @param vm VarMapUtils handler
 * @param vmap the global data map shared by all assets/asset managers
 * @param abf incoming asset Item
 * @param av  assetVar 
 * @param debug  debug flag
 */ 
bool checkabfChanged( VarMapUtils*vm, varsmap& vmap,  assetBitField* abf, assetVar* av, bool debug)
{
    bool ifChanged = false;
    bool vChanged  = false;  
    bool enable = true;

    if (abf->gotFeat("ifChanged"))
    {
        ifChanged = abf->getFeat("ifChanged", &ifChanged);
    }
    if (ifChanged)
    {
        vChanged = av->valueChanged();
        if(vChanged)
        {
            if (debug)FPS_PRINT_INFO(" var [{}] tested and changed \n"
                ,  av->getfName() );
            enable = true;
        }
        else
        {
            enable = false;
            if (debug)FPS_PRINT_INFO(" var [{}] tested and unchanged ignoring \n"
                ,  av->getfName());
        }
    }
    return enable;
}

// check av for change
/**
 * @brief check that the value has changed
 *
 * @param vm VarMapUtils handler
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  assetVar 
 * @param debug  debug flag
 */ 
bool checkChanged(VarMapUtils*vm, varsmap& vmap, assetVar* av, bool debug)
{
    bool ifChanged = false;
    bool vChanged  = false;  
    bool enable = true;

    if (av->gotParam("ifChanged"))
    {
        ifChanged = av->getbParam("ifChanged");
    }
    if (ifChanged)
    {
        vChanged = av->valueChanged();
        if(vChanged)
        {
            if (debug)FPS_PRINT_INFO("  var [{}] tested and changed \n"
                , av->getfName() );
            enable = true;
        }
        else
        {
            enable = false;
            if (debug)FPS_PRINT_INFO("  var [{}] tested and unchanged ignoring \n"
                , av->getfName());
        }
    }
    return enable;  
}


// check av for an enable var 
/**
 * @brief checks that this action item is enabled
 *
 * @param vm  VarMapUtils pointer 
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
bool checkEnable(VarMapUtils*vm, varsmap& vmap, assetVar* av, bool debug)
{
    bool enabled = true;
    char* ensp = nullptr;
    if (av->gotParam("enabled"))
    {
        if(!av->getbParam("enabled"))
        {
            if (debug) FPS_PRINT_INFO("####### Set action av [{}] NOT enabled"
                , av->getfName()
                );
            enabled = false;
        }
    }
    if (av->gotParam("enable"))
    {
        if(av->extras->baseDict->getFeatType("enable") == (int)assFeat::ASTRING)
        {
            ensp = av->getcParam("enable");
            if (debug)FPS_PRINT_INFO("####### enable av [{}] ensp [{}]"
                , av->getfName()
                , ensp? ensp:"no enable Av"
                );
        
            if (ensp)
            {
                assetVar* enav = vm->getVar(vmap, (const char*)ensp, nullptr);
                if (enav)
                {
                    enabled = enav->getbVal();
                    if(!enabled)
                    {
                        if (debug)FPS_PRINT_INFO("####### enable av [{}] not true"
                            , enav->getfName()
                            );
                    }
                }
            }
        }
        else
        {
            if(av->extras->baseDict->getFeatType("enable") == (int)assFeat::ABOOL)
                enabled = av->getbParam("enable");
        }
    }
    return enabled;
} 


/**
 * @brief  runs all the actions 
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::setActVecfromCj(varsmap& vmap, assetVar* av)//,  cJSON *cj)
{
    //the value has already been set
    // now run the actions
    if(!av->extras)
    {
        av->extras = new assetExtras;
    }
    bool ifChanged = false;
    bool resetChange = true;
    bool debug = false;
    if (av->gotParam("debug"))
    {
        debug = av->getbParam("debug");
    }
    if (av->gotParam("resetChange"))
    {
        resetChange = av->getbParam("resetChange");
    }
    if (!checkEnable(this, vmap, av, debug))
    {
        return av;
    }
    ifChanged = checkChanged(this, vmap, av, debug);
    if(ifChanged)
    {
        if (av->extras->actVec.find("onSet")!= av->extras->actVec.end())
        {
            auto aa = av->extras->actVec["onSet"];
            for (auto x : aa)
            {
                runActValfromCj(vmap, av, x, debug);
            }
        }
    }
    // resetchange unless reset change is false
    if (resetChange)
    {
        resetChange = av->valueChangedReset();
        if (0) FPS_PRINT_INFO("#######  resetChange [{}] on  av [{}]"
            
            , resetChange
            , av->getfName()
            );
    }
    if (0) FPS_PRINT_INFO("#######  resetChange [{}] on  av [{}]"        
            , resetChange
            , av->getfName()
            );
    return av;
}

// // handles the complexity of value = as well as naked ? sets
// loads the assetBitfields from the incoming Json object
/**
 * @brief  decodes the actions from the incoming cJSON actions object
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param av  incoming assetVar* av
 * @param debug  debug flag
 */ 
assetVar* VarMapUtils::setActBitMapfromCj(assetVar* av, assetAction* aact, cJSON* cjbf, cJSON* cj)
{
    cJSON* cji;
    //cJSON* cjbfm = cJSON_GetObjectItem(cjbf, "bitmap");

    if (cJSON_IsArray(cjbf))
    {
        cJSON_ArrayForEach(cji, cjbf)
        {
            aact->addBitField(cji);
        }
    }
    return av;
}

// some test code to us keyids to replace strings.

bool cJSON_ArrayToId(const cJSON* const item, std::vector<std::string>* idVec);
bool cJSON_ObjectToId(const cJSON* const item, std::vector<std::string>* idVec);

int cJSON_getKeyId(const char* item, std::vector<std::string>*idVec)
{
    //printf("%s seeking key for [%s]\n",__func__, item);
    std::string key = item;
    int idx = 0;
    for (auto x : *idVec)
    {
        if (x == key) return idx;
        idx++;
    }
    idVec->push_back(key);
    return idx;
}

int cJSON_showKeyIds(std::string&res, std::vector<std::string>*idVec)
{
    int idx = 0;
    for (auto x : *idVec)
    {
        res += fmt::format("idx [{}]\t [{}]\n", idx, x);
        idx++;
    }
    return idx;
}

bool cJSON_ValueToId(const cJSON* const item, std::vector<std::string>* idVec)
{

    switch ((item->type) & 0xFF)
    {
        case cJSON_NULL:
            return true;

        case cJSON_False:
            return true;

        case cJSON_True:
            return true;

        case cJSON_Number:
            return true; //print_number(item, output_buffer);

        case cJSON_Raw:
        {
            return true;
        }

        case cJSON_String:
            return true; //print_string(item, output_buffer);

        case cJSON_Array:
            return cJSON_ArrayToId(item, idVec);

        case cJSON_Object:
            return cJSON_ObjectToId(item, idVec);

        default:
            return false;
    }
    return true;
}

bool cJSON_ArrayToId(const cJSON* const item, std::vector<std::string>* idVec)
{
    cJSON *current_element = item->child;
    while (current_element != NULL)
    {
        if (!cJSON_ValueToId(current_element, idVec)) //print_value
        {
            return false;
        }
        
        current_element = current_element->next;
    }
    return true;
}

//replace item->string with an id.
// save the ids in the idvec
//  
bool cJSON_ObjectToId(const cJSON* const item, std::vector<std::string>* idVec)
{
    cJSON *current_item = item->child;

    while (current_item)
    {
        /* replace  key */
        if(current_item->string)
        {
            int keyId = cJSON_getKeyId((const char*)current_item->string, idVec);
            if(keyId>0)
            {
                free(current_item->string);
                asprintf(&current_item->string,"%d",keyId);
            }
        }

        /* print value */
        if (!cJSON_ValueToId(current_item, idVec))
        {
            //printf("%s quitting\n",__func__);

            return false;
        }
        current_item = current_item->next;
    }
    //printf("%s done\n",__func__);

    return true;
}
/* turn strings into ids */
bool cJSON_StringsToId(cJSON* item, std::vector<std::string>* idVec)
{
    //printf("%s running type [%d]\n",__func__, item->type);

    switch ((item->type) & 0xFF)
    {
        case cJSON_NULL:
            return true;

        case cJSON_False:
            return true;

        case cJSON_True:
            return true;

        case cJSON_Number:
            return true;//print_number(item, output_buffer);

        case cJSON_Raw:
            return true;//print_number(item, output_buffer);

        case cJSON_String:
            return true;//print_string(item, output_buffer);

        case cJSON_Array:
            return cJSON_ArrayToId(item, idVec); //print_array

        case cJSON_Object:
            return cJSON_ObjectToId(item, idVec);// print_object

        default:
            return false;
    }
}
