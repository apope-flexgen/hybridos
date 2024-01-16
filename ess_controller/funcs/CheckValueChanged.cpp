#ifndef CHECKDBIVAR_CPP
#define CHECKDBIVAR_CPP

#include "asset.h"

extern "C++"
{
    int CheckValueChanged(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av, bool reset=false);
}


/**
 * @brief Write a value to a variable say /limits/pcs:CheckChanged this function will then process all the other variables in that table
 *        to see if any values have changed
 *        if any have changed then set the value of /limits/pcs:CheckChanged to true , else st its value to false.
 *        The CheckChanged variable will not be included in the check list and its type will be coerced to bool.
 *   
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and dbi
 * @param av the assetVar referencing the dbi
 */
int CheckValueChanged(varsmap &vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av, bool noreset)
{
    int ret = 0;
    //VarMapUtils* vm = av->am->vm;
    bool debug = false;
    if(av->gotParam("debug"))
    {
        debug =av->getbParam("debug");
    }

    if(av->gotParam("noreset"))
    {
        noreset =av->getbParam("noreset");
    }

    if(debug)FPS_PRINT_INFO("checking  aname [{}] with assetVar [{}], noreset [{}]"
                    , aname, av->getfName(), noreset);
    bool changed = false;

    for (auto xx : vmap[av->comp])
    {
        if (!xx.second || (xx.second == av)) continue;
        bool xy = false;
        if(noreset)
        {
        // reset the change values on all
            xy = xx.second->valueChanged();
        }
        else
        {
            xy = xx.second->valueChangedReset();
        }

        if(debug)FPS_PRINT_INFO("checking  av [{}] changed [{}] with assetVar [{}]"
                , xx.second->getfName(), xy, av->getfName());

        if(xy)
        {
            changed = true;
            ret = 1;
            //break;
        }
    }
    av->setVal(changed);
    auto aVal = av->linkVar? av->linkVar->aVal:av->aVal;
    aVal->setType(assetVal::ABOOL);
    return ret;
}


#endif
