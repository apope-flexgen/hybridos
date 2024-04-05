/**
 * @file catl_fault_funcs_vec.cpp
 * @brief Test file that contains functions related to ESS fault protection for
 * CATL BMS
 * @version 0.1
 * @date 2020-11-12
 *
 */

#include "asset.h"
#include "assetFunc.cpp"
#include "assetFunc.h"
#include "chrono_utils.hpp"

/**
 * @brief Enum values representing the variables to use
 * for CATL
 *
 */
namespace CATL
{
enum
{
    eCheckValue,
    eValueOK,
    eValueSeen,
    eValueSeen_CAT1,
    eValueSeen_CAT2,
    eValueSeen_CAT3,
    eValueSeen_CAT4,

    eValueLimit_CAT1,
    eValueLimit_CAT2,
    eValueLimit_CAT3,
    eValueLimit_CAT4,
    eLimitEnabled_CAT1,
    eLimitEnabled_CAT2,
    eLimitEnabled_CAT3,
    eLimitEnabled_CAT4,

    eValueReset_CAT1,
    eValueReset_CAT2,
    eValueReset_CAT3,
    eValueReset_CAT4,
    eResetEnabled_CAT1,
    eResetEnabled_CAT2,
    eResetEnabled_CAT3,
    eResetEnabled_CAT4,

    eValueFault,
    eClearFaultCmd,
    eValueWarning,
    eValueErrTime,
    eValueResetTime,
    eErrTimeCfg,
    eResetTimeCfg,
    eTLast,
    eSim,
    eValue
};
};

/******************************************************************************************
 *                    Utility Functions
 ******************************************************************************************/

/******************************************************************************************
 *                    Helper Functions For Initializing Variables
 ******************************************************************************************/

/**
 * @brief Helper function used to initialize limit/reset values for testing CATL
 * battery cell over-voltage condition
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
void initializeCATLVars_HighVoltage(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                                    asset_manager* am)
{
    // Initialize limit value(s)
    double dval = 3.8;
    avmap[aname][vname][CATL::eValueLimit_CAT4]->setVal(dval);
    dval = 3.75;
    avmap[aname][vname][CATL::eValueLimit_CAT3]->setVal(dval);
    dval = 3.72;
    avmap[aname][vname][CATL::eValueLimit_CAT2]->setVal(dval);
    dval = 3.69;
    avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);

    avmap[aname][vname][CATL::eLimitEnabled_CAT4]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT3]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT2]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT1]->setVal(true);

    // Initialize reset value(s)
    dval = 3.34;
    avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);
    avmap[aname][vname][CATL::eValueReset_CAT2]->setVal(dval);
    avmap[aname][vname][CATL::eValueReset_CAT3]->setVal(dval);

    avmap[aname][vname][CATL::eResetEnabled_CAT4]->setVal(false);
    avmap[aname][vname][CATL::eResetEnabled_CAT3]->setVal(true);
    avmap[aname][vname][CATL::eResetEnabled_CAT2]->setVal(true);
    avmap[aname][vname][CATL::eResetEnabled_CAT1]->setVal(true);

    // Initialize reset/error time(s)
    dval = 3.0;
    avmap[aname][vname][CATL::eValueErrTime]->setVal(dval);
    avmap[aname][vname][CATL::eErrTimeCfg]->setVal(dval);

    dval = 1.0;
    avmap[aname][vname][CATL::eValueResetTime]->setVal(dval);
    avmap[aname][vname][CATL::eResetTimeCfg]->setVal(dval);
}

/**
 * @brief Helper function used to initialize limit/reset values for testing CATL
 * battery cell under-voltage condition
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
void initializeCATLVars_LowVoltage(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                                   asset_manager* am)
{
    // Initialize limit value(s)
    double dval = 2.66;
    avmap[aname][vname][CATL::eValueLimit_CAT4]->setVal(dval);
    dval = 2.7;
    avmap[aname][vname][CATL::eValueLimit_CAT3]->setVal(dval);
    dval = 2.73;
    avmap[aname][vname][CATL::eValueLimit_CAT2]->setVal(dval);
    dval = 2.75;
    avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);

    avmap[aname][vname][CATL::eLimitEnabled_CAT4]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT3]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT2]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT1]->setVal(true);

    // Initialize reset value(s)
    dval = 3.05;
    avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);
    avmap[aname][vname][CATL::eValueReset_CAT2]->setVal(dval);
    avmap[aname][vname][CATL::eValueReset_CAT3]->setVal(dval);

    avmap[aname][vname][CATL::eResetEnabled_CAT4]->setVal(false);
    avmap[aname][vname][CATL::eResetEnabled_CAT3]->setVal(true);
    avmap[aname][vname][CATL::eResetEnabled_CAT2]->setVal(true);
    avmap[aname][vname][CATL::eResetEnabled_CAT1]->setVal(true);

    // Initialize reset/error time(s)
    dval = 3.0;
    avmap[aname][vname][CATL::eValueErrTime]->setVal(dval);
    avmap[aname][vname][CATL::eErrTimeCfg]->setVal(dval);

    dval = 1.0;
    avmap[aname][vname][CATL::eValueResetTime]->setVal(dval);
    avmap[aname][vname][CATL::eResetTimeCfg]->setVal(dval);
}

/**
 * @brief Helper function used to initialize limit/reset values for testing CATL
 * battery cell high temperature (degrees Celcius) condition
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
void initializeCATLVars_HighTemp(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                                 asset_manager* am)
{
    // Initialize limit value(s)
    double dval = 50;
    avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);
    dval = 52;
    avmap[aname][vname][CATL::eValueLimit_CAT2]->setVal(dval);
    dval = 55;
    avmap[aname][vname][CATL::eValueLimit_CAT3]->setVal(dval);
    dval = 57;
    avmap[aname][vname][CATL::eValueLimit_CAT4]->setVal(dval);

    avmap[aname][vname][CATL::eLimitEnabled_CAT4]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT3]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT2]->setVal(true);
    avmap[aname][vname][CATL::eLimitEnabled_CAT1]->setVal(true);

    // Initialize reset value(s)
    dval = 48;
    avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);
    dval = 50;
    avmap[aname][vname][CATL::eValueReset_CAT2]->setVal(dval);
    dval = 53;
    avmap[aname][vname][CATL::eValueReset_CAT3]->setVal(dval);

    avmap[aname][vname][CATL::eResetEnabled_CAT4]->setVal(false);
    avmap[aname][vname][CATL::eResetEnabled_CAT3]->setVal(true);
    avmap[aname][vname][CATL::eResetEnabled_CAT2]->setVal(true);
    avmap[aname][vname][CATL::eResetEnabled_CAT1]->setVal(true);

    // Initialize error/reset time(s)
    dval = 3.0;
    avmap[aname][vname][CATL::eValueErrTime]->setVal(dval);
    avmap[aname][vname][CATL::eErrTimeCfg]->setVal(dval);

    dval = 3.0;
    avmap[aname][vname][CATL::eValueResetTime]->setVal(dval);
    avmap[aname][vname][CATL::eResetTimeCfg]->setVal(dval);

    // Initialize temperature value
    dval = 35;
    avmap[aname][vname][CATL::eValue]->setVal(dval);
}

/**
 * @brief Helper function used to initialize limit/reset values for testing CATL
 * battery cell low temperature (degrees Celcius) condition
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
void initializeCATLVars_LowTemp(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                                asset_manager* am)
{
    // Initialize limit value(s)
    double dval = 0;
    avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);

    avmap[aname][vname][CATL::eLimitEnabled_CAT4]->setVal(false);
    avmap[aname][vname][CATL::eLimitEnabled_CAT3]->setVal(false);
    avmap[aname][vname][CATL::eLimitEnabled_CAT2]->setVal(false);
    avmap[aname][vname][CATL::eLimitEnabled_CAT1]->setVal(true);

    // Initialize reset value(s)
    dval = 5;
    avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);

    avmap[aname][vname][CATL::eResetEnabled_CAT4]->setVal(false);
    avmap[aname][vname][CATL::eResetEnabled_CAT3]->setVal(false);
    avmap[aname][vname][CATL::eResetEnabled_CAT2]->setVal(false);
    avmap[aname][vname][CATL::eResetEnabled_CAT1]->setVal(true);

    // Initialize error/reset time(s)
    dval = 3.0;
    avmap[aname][vname][CATL::eValueErrTime]->setVal(dval);
    avmap[aname][vname][CATL::eErrTimeCfg]->setVal(dval);

    dval = 3.0;
    avmap[aname][vname][CATL::eValueResetTime]->setVal(dval);
    avmap[aname][vname][CATL::eResetTimeCfg]->setVal(dval);

    // Initialize temperature value
    dval = -15;
    avmap[aname][vname][CATL::eValue]->setVal(dval);
}

/**
 * @brief Helper function used to initialize limit/reset values for testing CATL
 * battery cell voltage difference condition
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
void initializeCATLVars_VoltDiff(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                                 asset_manager* am)
{
    double dval = 0.5;
    avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);

    dval = 0.45;
    avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);
}

/**
 * @brief Helper function used to initialize limit/reset values for testing CATL
 * battery cell temperature difference condition
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
void initializeCATLVars_TempDiff(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                                 asset_manager* am)
{
    double dval = 15;
    avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);

    dval = 10;
    avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);
}

/**
 * @brief Creates and initializes the list of asset vars for a specific variable
 * Ex.: MaxCellVolt is a variable that will be associated with a vector that
 * contains a collection of limit values, reset values, and other
 * variables/values to assist in monitoring and failure protection tasks
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param am the asset manager
 */
int SetupCATLLimitsVec(varsmap& vmap, varmap& amap, avarmap& avmap, const char* aname, const char* vname,
                       asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;

    std::string mname = vname;
    std::string mvar = "Check" + mname;

    assetVar* CheckValue = amap[mvar.c_str()];
    // char *tval = (char *)" Asset Init";
    int reload = 0;
    bool bval = false;
    // int ival = -1;
    double dval = 0.0;
    if (!CheckValue || (reload = CheckValue->getiVal()) == 0)
    {
        reload = 0;
    }

    if (reload < 2)
    {
        if (1)
            FPS_ERROR_PRINT("%s >> %s [%s]--- Running  \n", __func__, aname, vname);

        if (!avmap[aname][vname].empty())
        {
            avmap[aname][vname].clear();
        }

        // note links must set these values

        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/reload", mvar.c_str(), reload);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "OK";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        mvar = mname + "Seen";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Seen_CAT1";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Seen_CAT2";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Seen_CAT3";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Seen_CAT4";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        // amap["ValueSeen"]             = vm->setLinkVal(vmap, aname, "/status",
        // "ValueSeen",  bval);  amap["ValueOK"]               =
        // vm->setLinkVal(vmap, aname, "/status", "ValueOK",  bval);
        mvar = mname + "Limit_CAT1";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Limit_CAT2";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Limit_CAT3";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Limit_CAT4";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        mvar = mname + "LimitEnabled_CAT1";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "LimitEnabled_CAT2";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "LimitEnabled_CAT3";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "LimitEnabled_CAT4";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        mvar = mname + "Reset_CAT1";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Reset_CAT2";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Reset_CAT3";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Reset_CAT4";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        mvar = mname + "ResetEnabled_CAT1";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetEnabled_CAT2";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetEnabled_CAT3";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetEnabled_CAT4";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        // amap["ValueLimit"]            = vm->setLinkVal(vmap, aname, "/preset",
        // "ValueLimit",  dval);  amap["ValueReset"]            =
        // vm->setLinkVal(vmap, aname, "/config", "ValueReset",  dval);
        mvar = mname + "Fault";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ClearFaultCmd";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/controls", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Warning";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), bval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        // amap["ValueFault"]           = vm->setLinkVal(vmap, aname, "/asset",
        // "ValueFault",  bval);
        mvar = mname + "ErrTime";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetTime";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        // amap["ValueErrTime"]       = vm->setLinkVal(vmap, aname, "/config",
        // "ValueErrTime",  dval);  amap["ValueResetTime"]     =
        // vm->setLinkVal(vmap, aname, "/config", "ValueResetTime",  dval);
        // amap["Value"]              = vm->setLinkVal(vmap, aname, "/status",
        // "Value",  dval);  eErrTimeCfg, eResetTimeCfg,
        mvar = mname + "ErrCfg";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "ResetCfg";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "TLast";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname + "Sim";
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/config", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);
        mvar = mname;
        amap[mvar.c_str()] = vm->setLinkVal(vmap, aname, "/status", mvar.c_str(), dval);
        avmap[aname][vname].push_back(amap[mvar.c_str()]);

        // eTLast,
        if (reload < 1)
        {
            avmap[aname][vname][CATL::eValueSeen]->setVal(false);
            avmap[aname][vname][CATL::eValueSeen_CAT4]->setVal(false);
            avmap[aname][vname][CATL::eValueSeen_CAT3]->setVal(false);
            avmap[aname][vname][CATL::eValueSeen_CAT2]->setVal(false);
            avmap[aname][vname][CATL::eValueSeen_CAT1]->setVal(false);

            avmap[aname][vname][CATL::eValueOK]->setVal(false);
            avmap[aname][vname][CATL::eValueFault]->setVal(false);
            avmap[aname][vname][CATL::eClearFaultCmd]->setVal(false);
            avmap[aname][vname][CATL::eValueWarning]->setVal(false);

            dval = 3.8;
            avmap[aname][vname][CATL::eValueLimit_CAT4]->setVal(dval);
            dval = 3.75;
            avmap[aname][vname][CATL::eValueLimit_CAT3]->setVal(dval);
            dval = 3.72;
            avmap[aname][vname][CATL::eValueLimit_CAT2]->setVal(dval);
            dval = 3.69;
            avmap[aname][vname][CATL::eValueLimit_CAT1]->setVal(dval);

            avmap[aname][vname][CATL::eLimitEnabled_CAT4]->setVal(false);
            avmap[aname][vname][CATL::eLimitEnabled_CAT3]->setVal(false);
            avmap[aname][vname][CATL::eLimitEnabled_CAT2]->setVal(false);
            avmap[aname][vname][CATL::eLimitEnabled_CAT1]->setVal(false);

            dval = 3.34;
            avmap[aname][vname][CATL::eValueReset_CAT4]->setVal(dval);
            dval = 3.34;
            avmap[aname][vname][CATL::eValueReset_CAT3]->setVal(dval);
            dval = 3.34;
            avmap[aname][vname][CATL::eValueReset_CAT2]->setVal(dval);
            dval = 3.34;
            avmap[aname][vname][CATL::eValueReset_CAT1]->setVal(dval);

            avmap[aname][vname][CATL::eResetEnabled_CAT4]->setVal(false);
            avmap[aname][vname][CATL::eResetEnabled_CAT3]->setVal(false);
            avmap[aname][vname][CATL::eResetEnabled_CAT2]->setVal(false);
            avmap[aname][vname][CATL::eResetEnabled_CAT1]->setVal(false);

            dval = 8.0;
            avmap[aname][vname][CATL::eValueErrTime]->setVal(dval);
            avmap[aname][vname][CATL::eErrTimeCfg]->setVal(dval);

            dval = 10.0;
            avmap[aname][vname][CATL::eValueResetTime]->setVal(dval);
            avmap[aname][vname][CATL::eResetTimeCfg]->setVal(dval);

            dval = 3.2;
            avmap[aname][vname][CATL::eValue]->setVal(dval);  // Seg fault occurs here
            dval = vm->get_time_dbl();
            avmap[aname][vname][CATL::eTLast]->setVal(dval);

            int ival = 0;
            avmap[aname][vname][CATL::eSim]->setVal(ival);
        }
        reload = 2;
        avmap[aname][vname][CATL::eCheckValue]->setVal(reload);
    }
    return 0;
}

/**
 * @brief Periodically check if the current value is greater than the limit
 * value by failure level
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param p_fims the fims object used for data interchange
 * @param am the asset manager
 */
int CheckCATLLimitsVec_HighVal(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims,
                               asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (1)
        FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    auto avx = avmap[aname].find(vname);
    if (avx == avmap[aname].end())
    {
        if (1)
            FPS_ERROR_PRINT("%s >> %s --- Running Setup  for [%s]\n", __func__, aname, vname);
        SetupCATLLimitsVec(vmap, am->amap, avmap, am->name.c_str(), vname, am);

        // Use helper function to initialize variables in the avmap before testing
        // for value greater than threshold condition
        if (1)
            initializeCATLVars_HighVoltage(vmap, am->amap, avmap, am->name.c_str(), vname, am);
        if (0)
            initializeCATLVars_HighTemp(vmap, am->amap, avmap, am->name.c_str(), vname, am);
    }

    std::vector<assetVar*> avec = avmap[aname][vname];

    assetVar* CheckValue = avec[CATgetVaL::eCheckValue];
    int reload = CheckValue->getiVal();
    // ValueSeen set by the assetVar holding the Valueage component value when it
    // exceeded ValueLimit ValueOK set by the assetVar holding the Valueage
    // component value when it went below ValueReset ValueFault is the assetVar
    // indicating that we have a fault. ErrTime / ResetTime are the working time
    // values
    if (reload < 2)
    {
        // SetupValueage
        reload = 2;
        CheckValue->setVal(reload);
    }

    // Check whether we have seen abnormal values (categorized by failure level)
    bool ValueSeen = avec[CATL::eValueSeen]->getbVal();
    bool ValueSeen_CAT1 = avec[CATL::eValueSeen_CAT1]->getbVal();
    bool ValueSeen_CAT2 = avec[CATL::eValueSeen_CAT2]->getbVal();
    bool ValueSeen_CAT3 = avec[CATL::eValueSeen_CAT3]->getbVal();
    bool ValueSeen_CAT4 = avec[CATL::eValueSeen_CAT4]->getbVal();

    // Check the fault/warning values
    bool ValueOK = avec[CATL::eValueOK]->getbVal();
    bool ValueFault = avec[CATL::eValueFault]->getbVal();
    bool ClearFaultCmd = avec[CATL::eClearFaultCmd]->getbVal();
    bool ValueWarn = avec[CATL::eValueWarning]->getbVal();

    // Check for limits values (categorized by failure level) and whether the
    // check for those limits are enabled
    double ValueLimit_CAT1 = avec[CATL::eValueLimit_CAT1]->getdVal();
    double ValueLimit_CAT2 = avec[CATL::eValueLimit_CAT2]->getdVal();
    double ValueLimit_CAT3 = avec[CATL::eValueLimit_CAT3]->getdVal();
    double ValueLimit_CAT4 = avec[CATL::eValueLimit_CAT4]->getdVal();

    bool LimitEnabled_CAT1 = avec[CATL::eLimitEnabled_CAT1]->getbVal();
    bool LimitEnabled_CAT2 = avec[CATL::eLimitEnabled_CAT2]->getbVal();
    bool LimitEnabled_CAT3 = avec[CATL::eLimitEnabled_CAT3]->getbVal();
    bool LimitEnabled_CAT4 = avec[CATL::eLimitEnabled_CAT4]->getbVal();

    // Check for reset values (categorized by failure level) and whether those
    // limits are enabled
    double ValueReset_CAT1 = avec[CATL::eValueReset_CAT1]->getdVal();
    double ValueReset_CAT2 = avec[CATL::eValueReset_CAT2]->getdVal();
    double ValueReset_CAT3 = avec[CATL::eValueReset_CAT3]->getdVal();
    double ValueReset_CAT4 = avec[CATL::eValueReset_CAT4]->getdVal();

    bool ResetEnabled_CAT1 = avec[CATL::eResetEnabled_CAT1]->getbVal();
    bool ResetEnabled_CAT2 = avec[CATL::eResetEnabled_CAT2]->getbVal();
    bool ResetEnabled_CAT3 = avec[CATL::eResetEnabled_CAT3]->getbVal();
    bool ResetEnabled_CAT4 = avec[CATL::eResetEnabled_CAT4]->getbVal();

    // Check the reset and error times
    double ValueErrTime = avec[CATL::eValueErrTime]->getdVal();
    double ValueResetTime = avec[CATL::eValueResetTime]->getdVal();
    double ErrTimeCfg = avec[CATL::eErrTimeCfg]->getdVal();
    double ResetTimeCfg = avec[CATL::eResetTimeCfg]->getdVal();

    // Check the simulator value, the last time value, and the current value being
    // observed
    double tLast = avec[CATL::eTLast]->getdVal();
    int Sim = avec[CATL::eSim]->getbVal();
    double Value = avec[CATL::eValue]->getdVal();

    assetVar* ValueAv = avec[CATL::eValue];
    assetVar* ValueSeenAv = avec[CATL::eValueSeen];
    assetVar* ValueSeenAv_CAT1 = avec[CATL::eValueSeen_CAT1];
    assetVar* ValueSeenAv_CAT2 = avec[CATL::eValueSeen_CAT2];
    assetVar* ValueSeenAv_CAT3 = avec[CATL::eValueSeen_CAT3];
    assetVar* ValueSeenAv_CAT4 = avec[CATL::eValueSeen_CAT4];

    assetVar* ValueOKAv = avec[CATL::eValueOK];
    assetVar* ValueFaultAv = avec[CATL::eValueFault];
    assetVar* ClearFaultCmdAv = avec[CATL::eClearFaultCmd];
    assetVar* ValueWarnAv = avec[CATL::eValueWarning];
    assetVar* ValueErrTimeAv = avec[CATL::eValueErrTime];
    assetVar* ValueResetTimeAv = avec[CATL::eValueResetTime];

    assetVar* tLastAv = avec[CATL::eTLast];
    assetVar* SimAv = avec[CATL::eSim];

    double tNow = vm->get_time_dbl();
    tLastAv->setVal(tNow);
    double tGap = tNow - tLast;
    double svalue = 0.01;

    // Checks if the current variable we're working with has a value that is
    // greater than the limit value. If so, we need to be aware of this. This will
    // help us transition to fault/warning state . Latch the value set
    //             ***
    // ***********************************************ValueLimit****************************
    //          *      *
    //        *          *
    // ***********************************************ValueReset****************************
    //     **              ***
    if (0)
        FPS_ERROR_PRINT(
            "%s >> Value [%s] [%f] Seen [%s] OK [%s] Fault [%s] "
            "Warning [%s] ErrTime %f  ResetTime %f eSim %d \n",
            __func__, ValueAv->name.c_str(), Value, ValueSeen ? "true" : "false", ValueOK ? "true" : "false",
            ValueFault ? "true" : "false", ValueWarn ? "true" : "false", ValueErrTime, ValueResetTime, Sim);
    //////////////////////////////////////////////////////////////////////////////////
    if (Sim == 1)
    {
        // Limit value CAT4
        if (Value < ValueLimit_CAT4 + 1)
        {
            ValueAv->addVal(svalue);
        }
        else
        {
            SimAv->setVal(2);
        }

        // The rest of the code blocks are commented out so that we can individually
        // test sampled values being greater than a specific fault level

        // Limit value CAT3
        // if (Value < ValueLimit_CAT3 + 1)
        // {
        //     ValueAv->addVal(svalue);
        // }
        // else
        // {
        //     SimAv->setVal(2);
        // }

        // // Limit value CAT2
        // if (Value < ValueLimit_CAT2 + 1)
        // {
        //     ValueAv->addVal(svalue);
        // }
        // else
        // {
        //     SimAv->setVal(2);
        // }

        // // Limit value CAT1
        // if (Value < ValueLimit_CAT1 + 1)
        // {
        //     ValueAv->addVal(svalue);
        // }
        // else
        // {
        //     SimAv->setVal(2);
        // }
    }
    if (Sim == 2)
    {
        // The rest of the code blocks are commented out so that we can individually
        // test temperature value being greater than a specific fault level

        //  Reset value CAT3
        if (Value > ValueReset_CAT3 - 1)
        {
            ValueAv->subVal(svalue);
        }
        else
        {
            SimAv->setVal(1);
        }

        // // // Reset value CAT2
        // if (Value > ValueReset_CAT2 - 1)
        // {
        //     ValueAv->subVal(svalue);
        // }
        // else
        // {

        //     SimAv->setVal(1);
        // }

        // // // Reset Value CAT1
        // if (Value > ValueReset_CAT1 - 1)
        // {
        //     ValueAv->subVal(svalue);
        // }
        // else
        // {

        //     SimAv->setVal(1);
        // }
    }

    double dval = 0.0;
    double dval1 = 0.0;

    if (Sim != 0)
    {
        if (1)
            FPS_ERROR_PRINT(
                "%s >> CAT1 Limit [%f] CAT2 Limit [%f] CAT3 Limit [%f] "
                "CAT4 Limit [%f] CAT1 Reset [%f] CAT2 Reset [%f] CAT3 "
                "Reset [%f] ErrTime %f  ResetTime %f \n",
                __func__, ValueLimit_CAT1, ValueLimit_CAT2, ValueLimit_CAT3, ValueLimit_CAT4, ValueReset_CAT1,
                ValueReset_CAT2, ValueReset_CAT3, ValueErrTime, ValueResetTime);

        if (1)
            FPS_ERROR_PRINT(
                "%s >> Value [%s] [%f]->[%f] Seen [%s] Seen_CAT1 [%s] Seen_CAT2 [%s] "
                "Seen_CAT3 [%s] Seen_CAT4 [%s] OK [%s] Fault [%s] ClearFaultCmd [%s] "
                "Warning [%s] \n",
                __func__, ValueAv->name.c_str(), ValueAv->getdLVal(), ValueAv->getdVal(), ValueSeen ? "true" : "false",
                ValueSeen_CAT1 ? "true" : "false", ValueSeen_CAT2 ? "true" : "false", ValueSeen_CAT3 ? "true" : "false",
                ValueSeen_CAT4 ? "true" : "false", ValueOK ? "true" : "false", ValueFault ? "true" : "false",
                ClearFaultCmd ? "true" : "false", ValueWarn ? "true" : "false");
    }

    // If the value is greater than the failure level limit value, set the
    // variable to indicate we have seen a value that is greater than the limit
    // value at this specific failure level
    if (LimitEnabled_CAT4 && (Value > ValueLimit_CAT4 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT4)
        {
            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(true);
        }
    }
    else if (LimitEnabled_CAT3 && (Value > ValueLimit_CAT3 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT3)
        {
            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(true);
            ValueSeenAv_CAT4->setVal(false);
        }
    }
    else if (LimitEnabled_CAT2 && (Value > ValueLimit_CAT2 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT2)
        {
            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(true);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }
    }
    else if (LimitEnabled_CAT1 && (Value > ValueLimit_CAT1 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT1)
        {
            ValueSeenAv_CAT1->setVal(true);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }
    }
    // Here, our current value is an acceptable value, so we'll need to
    // set the variables (ex.: ValueSeen) to false so we don't
    // accidentally jump to a fault/warning state
    else
    {
        ValueSeenAv->setVal(false);
        ValueSeenAv_CAT1->setVal(false);
        ValueSeenAv_CAT2->setVal(false);
        ValueSeenAv_CAT3->setVal(false);
        ValueSeenAv_CAT4->setVal(false);
    }

    // Increase the reset time whenever we have a value that is greater than the
    // failure level limit value
    if ((LimitEnabled_CAT4 && Value > ValueLimit_CAT4) || (LimitEnabled_CAT3 && Value > ValueLimit_CAT3) ||
        (LimitEnabled_CAT2 && Value > ValueLimit_CAT2) || (LimitEnabled_CAT1 && Value > ValueLimit_CAT1))
    {
        if (ValueResetTime < ResetTimeCfg)
        {
            // Adjust up Recovery time
            double resetTimeAdj = ValueResetTime + tGap;
            ValueResetTimeAv->setVal(resetTimeAdj <= ResetTimeCfg ? resetTimeAdj : ResetTimeCfg);
        }
    }

    // If the current variable is less than the reset value, then we need to be
    // aware of this. This will help us transition out of fault/warning state
    if (((ResetEnabled_CAT1 && Value < ValueReset_CAT1) || (ResetEnabled_CAT2 && Value < ValueReset_CAT2) ||
         (ResetEnabled_CAT3 && Value < ValueReset_CAT3)) &&
        !ValueFault)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);

            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }

        if (ValueErrTime < ErrTimeCfg)
        {
            // Adjust up Err time
            double errTimeAdj = ValueErrTime + tGap;
            ValueErrTimeAv->setVal(errTimeAdj <= ErrTimeCfg ? errTimeAdj : ErrTimeCfg);
        }
    }

    // If we're in a fault state, we'll need a clear
    // fault command to transition out of fault state
    else if (ValueFault && ClearFaultCmd)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);

            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }

        if (ValueErrTime < ErrTimeCfg)
        {
            // Adjust up Err time
            double errTimeAdj = ValueErrTime + tGap;
            ValueErrTimeAv->setVal(errTimeAdj <= ErrTimeCfg ? errTimeAdj : ErrTimeCfg);
        }
    }

    // Otherwise, we'll need to set the variable (ValueOK) to make sure we stay in
    // the fault/warning state (if we're ever in one)
    else
    {
        if (ValueOK)
            ValueOKAv->setVal(false);
    }

    // If we're in a fault state, make sure we have received a clear fault command
    // in order to move out of the fault state
    if (ValueFault)
    {
        // If we are faulted but now things are OK then reset fault
        if (ValueOK)
        {
            if (ValueResetTime > 0.0)
            {
                // decrease Reset time
                double resetTimeAdj = ValueResetTime - tGap;
                ValueResetTimeAv->setVal(resetTimeAdj > 0 ? resetTimeAdj : 0);
            }

            if (ValueResetTime <= 0.0)
            {
                ValueFaultAv->setVal(false);
                ClearFaultCmdAv->setVal(false);
                // CloseDCBreaker()

                // Reset Fault Alarm (AssetManager)
                // am->ResetFault(ValueageAv, tNow," Valueage Alarm" );
                // trigger Fault actions (assetManager)
                // am->trigger_wakeup(BMS_FAULT_RESET);
            }
        }
    }

    // If we're in a warning state, make sure the current voltage value is less
    // than the reset value for a set amount of time (cancellation/reset time). If
    // the current voltage value is still less than the reset value after the
    // reset time, transition out of warning state and reset the states
    else if (ValueWarn)
    {
        // If we are in a warning state but now things are OK then reset warning
        if (ValueOK)
        {
            // Continue to decrease reset time until we have reached the end of the
            // alloted time
            if (ValueResetTime > 0.0)
            {
                double resetTimeAdj = ValueResetTime - tGap;
                ValueResetTimeAv->setVal(resetTimeAdj > 0 ? resetTimeAdj : 0);
            }

            // Clear the warning state and reset variables
            if (ValueResetTime <= 0.0)
            {
                ValueWarnAv->setVal(false);
            }
        }
    }

    // Transition into fault state and perform fault-handling tasks if we are in
    // CAT4 failure level Otherwise, transition into warning state
    else
    {
        // if (ValueSeen && (tNow - ValueSeenAv->getSetTime()) > ValueErrTime)
        if (ValueSeen)
        {
            if (ValueErrTime > 0.0)
            {
                // decrease Err time
                double errTimeAdj = ValueErrTime - (tNow - tLast);
                ValueErrTimeAv->setVal(errTimeAdj > 0 ? errTimeAdj : 0);
            }
            if (ValueErrTime <= 0.0)
            {
                // If we see a CAT4 failure level, then transition to fault state
                if (ValueSeen_CAT4)
                {
                    ValueFaultAv->setVal(true);
                    ValueOKAv->setVal(false);
                    // ValueSeenAv->setVal(false);
                    // OpenDCBreaker()
                    // Create Fault Alarm (AssetManager)
                    // am->CreateFault(ValueageAv, tNow," Valueage Alarm" );
                    // trigger Fault actions (assetManager)
                    // am->trigger_wakeup(BMS_FAULT);
                }
                else
                {
                    // Transition to warning state
                    ValueWarnAv->setVal(true);
                    ValueOKAv->setVal(false);

                    // Do additional warning tasks here
                }
            }
        }
    }

    return 0;
}

/**
 * @brief Periodically check if the current value is less than the limit value
 * by failure level
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param p_fims the fims object used for data interchange
 * @param am the asset manager
 */
int CheckCATLLimitsVec_LowVal(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims,
                              asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (1)
        FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    auto avx = avmap[aname].find(vname);
    if (avx == avmap[aname].end())
    {
        if (1)
            FPS_ERROR_PRINT("%s >> %s --- Running Setup  for [%s]\n", __func__, aname, vname);
        SetupCATLLimitsVec(vmap, am->amap, avmap, am->name.c_str(), vname, am);

        // Use helper function to initialize variables in the avmap before testing
        // for value less than threshold condition
        if (1)
            initializeCATLVars_LowVoltage(vmap, am->amap, avmap, am->name.c_str(), vname, am);
        if (0)
            initializeCATLVars_LowTemp(vmap, am->amap, avmap, am->name.c_str(), vname, am);

        // Add more helper functions here
    }

    std::vector<assetVar*> avec = avmap[aname][vname];

    assetVar* CheckValue = avec[CATL::eCheckValue];
    int reload = CheckValue->getiVal();
    // ValueSeen set by the assetVar holding the Valueage component value when it
    // exceeded ValueLimit ValueOK set by the assetVar holding the Valueage
    // component value when it went below ValueReset ValueFault is the assetVar
    // indicating that we have a fault. ErrTime / ResetTime are the working time
    // values
    if (reload < 2)
    {
        // SetupValueage
        reload = 2;
        CheckValue->setVal(reload);
    }

    // Check whether we have seen abnormal values (categorized by failure level)
    bool ValueSeen = avec[CATL::eValueSeen]->getbVal();
    bool ValueSeen_CAT1 = avec[CATL::eValueSeen_CAT1]->getbVal();
    bool ValueSeen_CAT2 = avec[CATL::eValueSeen_CAT2]->getbVal();
    bool ValueSeen_CAT3 = avec[CATL::eValueSeen_CAT3]->getbVal();
    bool ValueSeen_CAT4 = avec[CATL::eValueSeen_CAT4]->getbVal();

    // Check the fault/warning values
    bool ValueOK = avec[CATL::eValueOK]->getbVal();
    bool ValueFault = avec[CATL::eValueFault]->getbVal();
    bool ClearFaultCmd = avec[CATL::eClearFaultCmd]->getbVal();
    bool ValueWarn = avec[CATL::eValueWarning]->getbVal();

    // Check for limits values (categorized by failure level) and whether the
    // check for those limits are enabled
    double ValueLimit_CAT1 = avec[CATL::eValueLimit_CAT1]->getdVal();
    double ValueLimit_CAT2 = avec[CATL::eValueLimit_CAT2]->getdVal();
    double ValueLimit_CAT3 = avec[CATL::eValueLimit_CAT3]->getdVal();
    double ValueLimit_CAT4 = avec[CATL::eValueLimit_CAT4]->getdVal();

    bool LimitEnabled_CAT1 = avec[CATL::eLimitEnabled_CAT1]->getbVal();
    bool LimitEnabled_CAT2 = avec[CATL::eLimitEnabled_CAT2]->getbVal();
    bool LimitEnabled_CAT3 = avec[CATL::eLimitEnabled_CAT3]->getbVal();
    bool LimitEnabled_CAT4 = avec[CATL::eLimitEnabled_CAT4]->getbVal();

    // Check for reset values (categorized by failure level) and whether those
    // limits are enabled
    double ValueReset_CAT1 = avec[CATL::eValueReset_CAT1]->getdVal();
    double ValueReset_CAT2 = avec[CATL::eValueReset_CAT2]->getdVal();
    double ValueReset_CAT3 = avec[CATL::eValueReset_CAT3]->getdVal();
    double ValueReset_CAT4 = avec[CATL::eValueReset_CAT4]->getdVal();

    bool ResetEnabled_CAT1 = avec[CATL::eResetEnabled_CAT1]->getbVal();
    bool ResetEnabled_CAT2 = avec[CATL::eResetEnabled_CAT2]->getbVal();
    bool ResetEnabled_CAT3 = avec[CATL::eResetEnabled_CAT3]->getbVal();
    bool ResetEnabled_CAT4 = avec[CATL::eResetEnabled_CAT4]->getbVal();

    // Check the reset and error times
    double ValueErrTime = avec[CATL::eValueErrTime]->getdVal();
    double ValueResetTime = avec[CATL::eValueResetTime]->getdVal();
    double ErrTimeCfg = avec[CATL::eErrTimeCfg]->getdVal();
    double ResetTimeCfg = avec[CATL::eResetTimeCfg]->getdVal();

    // Check the simulator value, the last time value, and the current value being
    // observed
    double tLast = avec[CATL::eTLast]->getdVal();
    int Sim = avec[CATL::eSim]->getbVal();
    double Value = avec[CATL::eValue]->getdVal();

    assetVar* ValueAv = avec[CATL::eValue];
    assetVar* ValueSeenAv = avec[CATL::eValueSeen];
    assetVar* ValueSeenAv_CAT1 = avec[CATL::eValueSeen_CAT1];
    assetVar* ValueSeenAv_CAT2 = avec[CATL::eValueSeen_CAT2];
    assetVar* ValueSeenAv_CAT3 = avec[CATL::eValueSeen_CAT3];
    assetVar* ValueSeenAv_CAT4 = avec[CATL::eValueSeen_CAT4];

    assetVar* ValueOKAv = avec[CATL::eValueOK];
    assetVar* ValueFaultAv = avec[CATL::eValueFault];
    assetVar* ClearFaultCmdAv = avec[CATL::eClearFaultCmd];
    assetVar* ValueWarnAv = avec[CATL::eValueWarning];
    assetVar* ValueErrTimeAv = avec[CATL::eValueErrTime];
    assetVar* ValueResetTimeAv = avec[CATL::eValueResetTime];

    assetVar* tLastAv = avec[CATL::eTLast];
    assetVar* SimAv = avec[CATL::eSim];

    double tNow = vm->get_time_dbl();
    tLastAv->setVal(tNow);
    double tGap = tNow - tLast;
    double svalue = 0.01;

    // Checks if the current variable we're working with has a value that is
    // greater than the limit value. If so, we need to be aware of this. This will
    // help us transition to fault/warning state . Latch the value set
    //             ***
    // ***********************************************ValueLimit****************************
    //          *      *
    //        *          *
    // ***********************************************ValueReset****************************
    //     **              ***
    if (0)
        FPS_ERROR_PRINT(
            "%s >> Value [%s] [%f] Seen [%s] OK [%s] Fault [%s] "
            "Warning [%s] ErrTime %f  ResetTime %f eSim %d \n",
            __func__, ValueAv->name.c_str(), Value, ValueSeen ? "true" : "false", ValueOK ? "true" : "false",
            ValueFault ? "true" : "false", ValueWarn ? "true" : "false", ValueErrTime, ValueResetTime, Sim);
    //////////////////////////////////////////////////////////////////////////////////
    if (Sim == 1)
    {
        // Limit value CAT4
        if (Value < ValueLimit_CAT4 + 1)
        {
            ValueAv->addVal(svalue);
        }
        else
        {
            SimAv->setVal(2);
        }

        // The rest of the code blocks are commented out so that we can individually
        // test sampled values being greater than a specific fault level

        // Limit value CAT3
        // if (Value < ValueLimit_CAT3 + 1)
        // {
        //     ValueAv->addVal(svalue);
        // }
        // else
        // {
        //     SimAv->setVal(2);
        // }

        // // Limit value CAT2
        // if (Value < ValueLimit_CAT2 + 1)
        // {
        //     ValueAv->addVal(svalue);
        // }
        // else
        // {
        //     SimAv->setVal(2);
        // }

        // // Limit value CAT1
        // if (Value < ValueLimit_CAT1 + 1)
        // {
        //     ValueAv->addVal(svalue);
        // }
        // else
        // {
        //     SimAv->setVal(2);
        // }
    }
    if (Sim == 2)
    {
        // The rest of the code blocks are commented out so that we can individually
        // test temperature value being greater than a specific fault level

        //  Reset value CAT3
        if (Value > ValueReset_CAT3 - 1)
        {
            ValueAv->subVal(svalue);
        }
        else
        {
            SimAv->setVal(1);
        }

        // // // Reset value CAT2
        // if (Value > ValueReset_CAT2 - 1)
        // {
        //     ValueAv->subVal(svalue);
        // }
        // else
        // {

        //     SimAv->setVal(1);
        // }

        // // // Reset Value CAT1
        // if (Value > ValueReset_CAT1 - 1)
        // {
        //     ValueAv->subVal(svalue);
        // }
        // else
        // {

        //     SimAv->setVal(1);
        // }
    }

    double dval = 0.0;
    double dval1 = 0.0;

    if (Sim != 0)
    {
        if (1)
            FPS_ERROR_PRINT(
                "%s >> CAT1 Limit [%f] CAT2 Limit [%f] CAT3 Limit [%f] "
                "CAT4 Limit [%f] CAT1 Reset [%f] CAT2 Reset [%f] CAT3 "
                "Reset [%f] ErrTime %f  ResetTime %f \n",
                __func__, ValueLimit_CAT1, ValueLimit_CAT2, ValueLimit_CAT3, ValueLimit_CAT4, ValueReset_CAT1,
                ValueReset_CAT2, ValueReset_CAT3, ValueErrTime, ValueResetTime);
        if (1)
            FPS_ERROR_PRINT(
                "%s >> Value [%s] [%f]->[%f] Seen [%s] Seen_CAT1 [%s] Seen_CAT2 [%s] "
                "Seen_CAT3 [%s] Seen_CAT4 [%s] OK [%s] Fault [%s] ClearFaultCmd [%s] "
                "Warning [%s] \n",
                __func__, ValueAv->name.c_str(), ValueAv->getdLVal(), ValueAv->getdVal(), ValueSeen ? "true" : "false",
                ValueSeen_CAT1 ? "true" : "false", ValueSeen_CAT2 ? "true" : "false", ValueSeen_CAT3 ? "true" : "false",
                ValueSeen_CAT4 ? "true" : "false", ValueOK ? "true" : "false", ValueFault ? "true" : "false",
                ClearFaultCmd ? "true" : "false", ValueWarn ? "true" : "false"

            );
    }

    // If the value is greater than the failure level limit value, set the
    // variable to indicate we have seen a value that is greater than the limit
    // value at this specific failure level
    if (LimitEnabled_CAT4 && (Value < ValueLimit_CAT4 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT4)
        {
            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(true);
        }
    }
    else if (LimitEnabled_CAT3 && (Value < ValueLimit_CAT3 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT3)
        {
            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(true);
            ValueSeenAv_CAT4->setVal(false);
        }
    }
    else if (LimitEnabled_CAT2 && (Value < ValueLimit_CAT2 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT2)
        {
            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(true);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }
    }
    else if (LimitEnabled_CAT1 && (Value < ValueLimit_CAT1 && !ValueFault))
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT1)
        {
            ValueSeenAv_CAT1->setVal(true);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }
    }
    // Here, our current value is an acceptable value, so we'll need to
    // set the variables (ex.: ValueSeen) to false so we don't
    // accidentally jump to a fault/warning state
    else
    {
        ValueSeenAv->setVal(false);
        ValueSeenAv_CAT1->setVal(false);
        ValueSeenAv_CAT2->setVal(false);
        ValueSeenAv_CAT3->setVal(false);
        ValueSeenAv_CAT4->setVal(false);
    }

    // Increase the reset time whenever we have a value that is greater than the
    // failure level limit value
    if ((LimitEnabled_CAT4 && Value < ValueLimit_CAT4) || (LimitEnabled_CAT3 && Value < ValueLimit_CAT3) ||
        (LimitEnabled_CAT2 && Value < ValueLimit_CAT2) || (LimitEnabled_CAT1 && Value < ValueLimit_CAT1))
    {
        if (ValueResetTime < ResetTimeCfg)
        {
            // Adjust up Recovery time
            double resetTimeAdj = ValueResetTime + tGap;
            ValueResetTimeAv->setVal(resetTimeAdj <= ResetTimeCfg ? resetTimeAdj : ResetTimeCfg);
        }
    }

    // If the current variable is less than the reset value, then we need to be
    // aware of this. This will help us transition out of fault/warning state
    if (((ResetEnabled_CAT1 && Value > ValueReset_CAT1) || (ResetEnabled_CAT2 && Value > ValueReset_CAT2) ||
         (ResetEnabled_CAT3 && Value > ValueReset_CAT3)) &&
        !ValueFault)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);

            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }

        if (ValueErrTime < ErrTimeCfg)
        {
            // Adjust up Err time
            double errTimeAdj = ValueErrTime + tGap;
            ValueErrTimeAv->setVal(errTimeAdj <= ErrTimeCfg ? errTimeAdj : ErrTimeCfg);
        }
    }

    // If we're in a fault state, we'll need a clear
    // fault command to transition out of fault state
    else if (ValueFault && ClearFaultCmd)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);

            ValueSeenAv_CAT1->setVal(false);
            ValueSeenAv_CAT2->setVal(false);
            ValueSeenAv_CAT3->setVal(false);
            ValueSeenAv_CAT4->setVal(false);
        }

        if (ValueErrTime < ErrTimeCfg)
        {
            // Adjust up Err time
            double errTimeAdj = ValueErrTime + tGap;
            ValueErrTimeAv->setVal(errTimeAdj <= ErrTimeCfg ? errTimeAdj : ErrTimeCfg);
        }
    }

    // Otherwise, we'll need to set the variable (ValueOK) to make sure we stay in
    // the fault/warning state (if we're ever in one)
    else
    {
        if (ValueOK)
            ValueOKAv->setVal(false);
    }

    // If we're in a fault state, make sure we have received a clear fault command
    // in order to move out of the fault state
    if (ValueFault)
    {
        // If we are faulted but now things are OK then reset fault
        if (ValueOK)
        {
            if (ValueResetTime > 0.0)
            {
                // decrease Reset time
                double resetTimeAdj = ValueResetTime - tGap;
                ValueResetTimeAv->setVal(resetTimeAdj > 0 ? resetTimeAdj : 0);
            }

            if (ValueResetTime <= 0.0)
            {
                ValueFaultAv->setVal(false);
                ClearFaultCmdAv->setVal(false);
                // CloseDCBreaker()

                // Reset Fault Alarm (AssetManager)
                // am->ResetFault(ValueageAv, tNow," Valueage Alarm" );
                // trigger Fault actions (assetManager)
                // am->trigger_wakeup(BMS_FAULT_RESET);
            }
        }
    }

    // If we're in a warning state, make sure the current voltage value is less
    // than the reset value for a set amount of time (cancellation/reset time). If
    // the current voltage value is still less than the reset value after the
    // reset time, transition out of warning state and reset the states
    else if (ValueWarn)
    {
        // If we are in a warning state but now things are OK then reset warning
        if (ValueOK)
        {
            // Continue to decrease reset time until we have reached the end of the
            // alloted time
            if (ValueResetTime > 0.0)
            {
                double resetTimeAdj = ValueResetTime - tGap;
                ValueResetTimeAv->setVal(resetTimeAdj > 0 ? resetTimeAdj : 0);
            }

            // Clear the warning state and reset variables
            if (ValueResetTime <= 0.0)
            {
                ValueWarnAv->setVal(false);
            }
        }
    }

    // Transition into fault state and perform fault-handling tasks if we are in
    // CAT4 failure level Otherwise, transition into warning state
    else
    {
        // if (ValueSeen && (tNow - ValueSeenAv->getSetTime()) > ValueErrTime)
        if (ValueSeen)
        {
            if (ValueErrTime > 0.0)
            {
                // decrease Err time
                double errTimeAdj = ValueErrTime - (tNow - tLast);
                ValueErrTimeAv->setVal(errTimeAdj > 0 ? errTimeAdj : 0);
            }
            if (ValueErrTime <= 0.0)
            {
                // If we see a CAT4 failure level, then transition to fault state
                if (ValueSeen_CAT4)
                {
                    ValueFaultAv->setVal(true);
                    ValueOKAv->setVal(false);
                    // ValueSeenAv->setVal(false);
                    // OpenDCBreaker()
                    // Create Fault Alarm (AssetManager)
                    // am->CreateFault(ValueageAv, tNow," Valueage Alarm" );
                    // trigger Fault actions (assetManager)
                    // am->trigger_wakeup(BMS_FAULT);
                }
                else
                {
                    // Transition to warning state
                    ValueWarnAv->setVal(true);
                    ValueOKAv->setVal(false);

                    // Do additional warning tasks here
                }
            }
        }
    }

    return 0;
}

/**
 * @brief Periodically check if the difference between the current value and the
 * previous value is greater/less than the threshold value
 *
 * @param vmap the global data map all assets/asset managers have access to
 * @param avmap the list of asset vars for a specific variable
 * @param aname the asset name
 * @param vname the variable name
 * @param p_fims the fims object used for data interchange
 * @param am the asset manager
 */
int CheckCATLLimitsVec_ValDiff(varsmap& vmap, avarmap& avmap, const char* aname, const char* vname, fims* p_fims,
                               asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    vm->setTime();
    //    VarMapUtils * vm = am->vm;
    if (1)
        FPS_ERROR_PRINT("%s >> %s --- Running  for [%s]\n", __func__, aname, vname);
    auto avx = avmap[aname].find(vname);
    if (avx == avmap[aname].end())
    {
        if (1)
            FPS_ERROR_PRINT("%s >> %s --- Running Setup  for [%s]\n", __func__, aname, vname);
        SetupCATLLimitsVec(vmap, am->amap, avmap, am->name.c_str(), vname, am);

        // Use helper function to initialize variables in the avmap before testing
        // for CATL voltage/temperature difference condition
        if (1)
            initializeCATLVars_VoltDiff(vmap, am->amap, avmap, am->name.c_str(), vname, am);
        if (0)
            initializeCATLVars_TempDiff(vmap, am->amap, avmap, am->name.c_str(), vname, am);
    }

    std::vector<assetVar*> avec = avmap[aname][vname];

    assetVar* CheckValue = avec[CATL::eCheckValue];
    int reload = CheckValue->getiVal();
    // ValueSeen set by the assetVar holding the Valueage component value when it
    // exceeded ValueLimit ValueOK set by the assetVar holding the Valueage
    // component value when it went below ValueReset ValueFault is the assetVar
    // indicating that we have a fault. ErrTime / ResetTime are the working time
    // values
    if (reload < 2)
    {
        // SetupValueage
        reload = 2;
        CheckValue->setVal(reload);
    }

    // first check for latched limits.
    bool ValueSeen = avec[CATL::eValueSeen]->getbVal();
    bool ValueSeen_CAT1 = avec[CATL::eValueSeen_CAT1]->getbVal();

    bool ValueOK = avec[CATL::eValueOK]->getbVal();
    bool ValueWarn = avec[CATL::eValueWarning]->getbVal();
    double Value = avec[CATL::eValue]->getdVal();
    // double lastValue = avec[CATL::eValue]->getLVal(lastValue);

    // For testing purposes. Need to find a better way to test for value change
    double lastValue = 3.2;  // value for voltage
    // double lastValue = 10;      // value for temperature

    // double  ValueLimit = avec[CATL::eValueLimit]->getdVal();
    double ValueLimit_CAT1 = avec[CATL::eValueLimit_CAT1]->getdVal();
    double ValueReset_CAT1 = avec[CATL::eValueReset_CAT1]->getdVal();
    double ValueErrTime = avec[CATL::eValueErrTime]->getdVal();
    double ValueResetTime = avec[CATL::eValueResetTime]->getdVal();
    double ErrTimeCfg = avec[CATL::eErrTimeCfg]->getdVal();
    double ResetTimeCfg = avec[CATL::eResetTimeCfg]->getdVal();
    double tLast = avec[CATL::eTLast]->getdVal();
    int Sim = avec[CATL::eSim]->getbVal();

    assetVar* ValueAv = avec[CATL::eValue];
    assetVar* ValueSeenAv = avec[CATL::eValueSeen];
    assetVar* ValueSeenAv_CAT1 = avec[CATL::eValueSeen_CAT1];

    assetVar* ValueOKAv = avec[CATL::eValueOK];
    assetVar* ValueWarnAv = avec[CATL::eValueWarning];
    // assetVar* ErrTimeAv = avec[eErrTimeCfg];
    // assetVar* ResetTimeAv = avec[eResetTimeCfg];
    assetVar* ValueErrTimeAv = avec[CATL::eValueErrTime];
    assetVar* ValueResetTimeAv = avec[CATL::eValueResetTime];
    assetVar* tLastAv = avec[CATL::eTLast];
    assetVar* SimAv = avec[CATL::eSim];

    double tNow = vm->get_time_dbl();
    tLastAv->setVal(tNow);
    double tGap = tNow - tLast;
    double svalue = 0.001;

    // Checks if the difference between the current value and the previous value
    // is greater than the threshold. If so, we need to be aware of this. This
    // will help us transition to warning state . Latch the value set
    //             ***
    // ***********************************************ValueLimit****************************
    //          *      *
    //        *          *
    // ***********************************************ValueReset****************************
    //     **              ***
    if (0)
        FPS_ERROR_PRINT(
            "%s >> Value [%s] [%f] Seen [%s] OK [%s] Warning [%s] "
            "ErrTime %f  ResetTime %f eSim %d \n",
            __func__, ValueAv->name.c_str(), Value, ValueSeen ? "true" : "false", ValueOK ? "true" : "false",
            ValueWarn ? "true" : "false", ValueErrTime, ValueResetTime, Sim);
    //////////////////////////////////////////////////////////////////////////////////

    // TODO: need a better way to simulate value difference

    // Simulate test for voltage difference
    if (Sim == 1)
    {
        // // Voltage limit CAT1
        if (std::abs(Value - lastValue) < ValueLimit_CAT1 + 0.1)
        {
            ValueAv->addVal(svalue);
        }
        else
        {
            SimAv->setVal(2);
        }
    }

    if (Sim == 2)
    {
        if (std::abs(Value - lastValue) > ValueLimit_CAT1 - 0.1)
        {
            ValueAv->subVal(svalue);
        }
        else
        {
            SimAv->setVal(1);
        }
    }

    // // Simulate test for temperature difference
    // if (Sim == 1)
    // {
    //     // // Temperature limit CAT1
    //     if (std::abs(Value - lastValue) < ValueLimit_CAT1 + 0.1)
    //     {
    //         ValueAv->addVal(svalue);
    //     }
    //     else
    //     {
    //         SimAv->setVal(2);
    //     }

    // }

    // if (Sim == 2)
    // {

    //     if (std::abs(Value - lastValue) > ValueLimit_CAT1 - 0.1)
    //     {
    //         ValueAv->subVal(svalue);
    //     }
    //     else
    //     {
    //         SimAv->setVal(1);
    //     }

    // }

    // If simlulator is active, report current state of the variables used
    if (Sim != 0)
    {
        if (1)
            FPS_ERROR_PRINT("%s >> CAT1 Limit [%f] Reset Value [%f] ErrTime %f  ResetTime %f \n", __func__,
                            ValueLimit_CAT1, ValueReset_CAT1, ValueErrTime, ValueResetTime);
        if (1)
            FPS_ERROR_PRINT(
                "%s >> Value [%s] [%f]->[%f] Diff [%f] Seen [%s] "
                "Seen_CAT1 [%s] OK [%s] Warning [%s] \n",
                __func__, ValueAv->name.c_str(), lastValue, Value, std::abs(Value - lastValue),
                ValueSeen ? "true" : "false", ValueSeen_CAT1 ? "true" : "false", ValueOK ? "true" : "false",
                ValueWarn ? "true" : "false");
    }

    // If the value is less than the failure level limit value, set the variable
    // to indicate we have seen a value that is greater than the limit value at
    // this specific failure level
    if (std::abs(Value - lastValue) > ValueLimit_CAT1)
    {
        if (!ValueSeen)
        {
            ValueSeenAv->setVal(true);
            ValueOKAv->setVal(false);
        }
        if (!ValueSeen_CAT1)
        {
            ValueSeenAv_CAT1->setVal(true);
        }

        // Increase the reset time whenever we have a difference value that is
        // greater than the failure level limit value
        if (ValueResetTime < ResetTimeCfg)
        {
            // Adjust up Recovery time
            double resetTimeAdj = ValueResetTime + tGap;
            ValueResetTimeAv->setVal(resetTimeAdj <= ResetTimeCfg ? resetTimeAdj : ResetTimeCfg);
        }
    }
    // Here, the difference is an acceptable value, so we'll need to
    // set the variables (ex.: ValueSeen) to false so we don't
    // accidentally jump to a fault/warning state
    else
    {
        ValueSeenAv->setVal(false);
        ValueSeenAv_CAT1->setVal(false);
    }

    // If the current variable is less than the reset value, then we need to be
    // aware of this. This will help us transition out of fault state
    if (std::abs(Value - lastValue) < ValueReset_CAT1)
    {
        if (!ValueOK)
        {
            ValueOKAv->setVal(true);
            ValueSeenAv->setVal(false);
            ValueSeenAv_CAT1->setVal(false);
        }
        if (ValueErrTime < ErrTimeCfg)
        {
            // Adjust up Err time
            double errTimeAdj = ValueErrTime + tGap;
            ValueErrTimeAv->setVal(errTimeAdj <= ErrTimeCfg ? errTimeAdj : ErrTimeCfg);
        }
    }

    // Otherwise, we'll need to set the variable (ValueOK) to make sure we stay in
    // the fault/warning state (if we're ever in one)
    else
    {
        if (ValueOK)
            ValueOKAv->setVal(false);
    }

    // If we're in a warning state, make sure the current voltage value is less
    // than the reset value for a set amount of time (cancellation/reset time). If
    // the current voltage value is still less than the reset value after the
    // reset time, transition out of warning state and reset the states
    if (ValueWarn)
    {
        // If we are in a warning state but now things are OK then reset warning
        if (ValueOK)
        {
            // Continue to decrease reset time until we have reached the end of the
            // alloted time
            if (ValueResetTime > 0.0)
            {
                double resetTimeAdj = ValueResetTime - tGap;
                ValueResetTimeAv->setVal(resetTimeAdj > 0 ? resetTimeAdj : 0);
            }

            // Clear the warning state and reset variables
            if (ValueResetTime <= 0.0)
            {
                ValueWarnAv->setVal(false);
            }
        }
    }

    // Transition into warning state if the difference between the current value
    // and the previous value remains greater than the threshold value after a set
    // amount of time
    else
    {
        if (ValueSeen)
        {
            if (ValueErrTime > 0.0)
            {
                // decrease Err time
                double errTimeAdj = ValueErrTime - (tNow - tLast);
                ValueErrTimeAv->setVal(errTimeAdj > 0 ? errTimeAdj : 0);
            }
            if (ValueErrTime <= 0.0)
            {
                // Transition to warning state
                ValueWarnAv->setVal(true);
                ValueOKAv->setVal(false);

                // Do additional warning tasks here
            }
        }
    }

    return 0;
}