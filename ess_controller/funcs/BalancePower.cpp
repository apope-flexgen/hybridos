#ifndef BALANCEPOWER_CPP
#define BALANCEPOWER_CPP

#include "asset.h"
#include "formatters.hpp"
#include "calculator.hpp"
#include "ess_utils.hpp"

extern "C++" 
{
    int BalancePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

/**
 * @brief Initializes the parameters to be used in the power balancing function
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to initialize parameters for
 */
void setupBalancePowerParams(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    FPS_PRINT_INFO("Setting up params for [{}:{}]", av->comp, av->name);

    // Last adjustment factor
    if (!av->gotParam("ratio"))
    {
        double dval = 0;
        av->setParam("ratio", dval);
    }
    av->setParam("lastRatio", av->getdParam("ratio"));

    // Maximum allowable difference (default to 50 for voltage difference threshold)
    if (!av->gotParam("threshold"))
    {
        double dval = 50;
        av->setParam("threshold", dval);
    }
    // Minimum time between power command adjustments. Default 15s (15000 ms). 
    if (!av->gotParam("timeoutMs"))
    {
        double dval = 15000;
        av->setParam("timeoutMs", dval);
    }
    // DC Current threshold
    if (!av->gotParam("dcCurrentThreshold"))
    {
        double dval = 0;
        av->setParam("dcCurrentThreshold", dval);
    }
    // The factor that determines how much the output power will change
    if (!av->gotParam("scaleFactor"))
    {
        double dval = 0.7;
        av->setParam("scaleFactor", dval);
    }
    // Initialize the assetVar operand parameters to be used for calculation if these do not exist
    //VarMapUtils* vm = av->am->vm;
    // if (!av->gotParam("numVars"))
    //     av->setParam("numVars", 0);

    if (!av->gotParam("tLast"))
    {
        double dval = 0;
        av->setParam("tLast", dval);
    }
    if(!av->gotParam("dcVoltageA"))
    {
        FPS_PRINT_WARN("assetVar [{}] does not have param dcVoltageA or param value is null", av->name);
    }

    if(!av->gotParam("dcVoltageB"))
    {
        FPS_PRINT_WARN("assetVar [{}] does not have param dcVoltageB or param value is null", av->name);
    }
}

/**
 * @brief Adjusts the input AC power command using the voltage balancing strategy
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the assetVar to initialize parameters for
 */
int BalancePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{

    bool debug = false;
    bool disabled = false; 
    bool reset = false;
    bool error = false;
    if (av->gotParam("debug"))
        debug = av->getbParam("debug");

    if(av->gotParam("reset"))
        reset = av->getbParam("reset");

    if(!av)
    {
        if(debug)FPS_ERROR_PRINT("[%s] av is null. Terminating voltage balancing", __func__);
        return -1;
    }
    else if(!av->am && !error)
    {
        if(debug)FPS_ERROR_PRINT("[%s] am is null. Terminating voltage balancing", __func__);
        return -1;
    }
	VarMapUtils* vm = av->am->vm;
    if(!vm && !error)
    {
        if(debug)FPS_ERROR_PRINT("[%s] Error accessing VarMapUtils. Terminating voltage balancing", __func__);
        return -1;
    }

    essPerf ePerf(av->am, aname, __func__);
    std::string reloadStr = "BalancePower_" + av->name;
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        FPS_PRINT_DEBUG("reload first run {}", aname, reload);
        setupBalancePowerParams(vmap, amap, aname, av);

        reload = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), reload);
        amap[reloadStr]->setVal(reload);
    }


    if (!checkEnable(vm, vmap, av, debug))
        disabled = true; 
    
    if (av->gotParam("enabled"))
        disabled = (!av->getbParam("enabled")) || disabled;
    
    assetVar* bmsVoltageA = ESSUtils::getAvFromParam(vmap, amap, av, "bmsInputDcA");
    if (!bmsVoltageA)
    {
        if(debug)FPS_PRINT_WARN("dcVoltageA [{}] in av [{}] does not exist", cstr{av->getcParam("bmsInputDcA")}, av->getfName());
        error = true;
    }

    assetVar* bmsVoltageB = ESSUtils::getAvFromParam(vmap, amap, av, "bmsInputDcB");
    if (!bmsVoltageB)
    {
        if(debug)FPS_PRINT_WARN("dcVoltageB [{}] in av [{}] does not exist", cstr{av->getcParam("bmsInputDcB")}, av->getfName());
        error = true;
    }

    assetVar* pCmdVar = ESSUtils::getAvFromParam(vmap, amap, av, "pCmdVar");
    if (!pCmdVar)
    {
        if(debug)FPS_PRINT_WARN("Active power command variable [{}] in av [{}] does not exist", cstr{av->getcParam("pCmdVar")}, av->getfName());
        return -1;
    }

    assetVar* qCmdVar = ESSUtils::getAvFromParam(vmap, amap, av, "qCmdVar");
    if (!qCmdVar)
    {
        if(debug)FPS_PRINT_WARN("Reactive power command variable [{}] in av [{}] does not exist", cstr{av->getcParam("qCmdVar")}, av->getfName());
        return -1;
    }

    assetVar* dcCurrentVar = ESSUtils::getAvFromParam(vmap, amap, av, "dcCurrentVar");
    if (!dcCurrentVar)
    {
        if(debug)FPS_PRINT_WARN("DC current variable [{}] in av [{}] does not exist", cstr{av->getcParam("dcCurrentVar")}, av->getfName());
        error = true;
    }

    assetVar* pOutputAVar = ESSUtils::getAvFromParam(vmap, amap, av, "pOutputAVar");
    if (!pOutputAVar)
    {
        if(debug)FPS_PRINT_WARN("Side A output active power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pOutputAVar")}, av->getfName());
        return -1;
    }

    assetVar* pOutputBVar = ESSUtils::getAvFromParam(vmap, amap, av, "pOutputBVar");
    if (!pOutputBVar)
    {
        if(debug)FPS_PRINT_WARN("Side B output active power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pOutputBVar")}, av->getfName());
        return -1;
    }

    assetVar* qOutputAVar = ESSUtils::getAvFromParam(vmap, amap, av, "qOutputAVar");
    if (!qOutputAVar)
    {
        if(debug)FPS_PRINT_WARN("Side A output reactive power variable [{}] in av [{}] does not exist", cstr{av->getcParam("qOutputAVar")}, av->getfName());
        return -1;
    }

    assetVar* qOutputBVar = ESSUtils::getAvFromParam(vmap, amap, av, "qOutputBVar");
    if (!qOutputBVar)
    {
        if(debug)FPS_PRINT_WARN("Side B output reactive power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pOutputBVar")}, av->getfName());
        return -1;
    }
    
    assetVar* pcsMaxChrgPwrA = ESSUtils::getAvFromParam(vmap, amap, av, "pcsMaxChrgPwrA");
    if (!pcsMaxChrgPwrA)
    {
        if(debug)FPS_PRINT_WARN("Side A PCS maximum charge power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pcsMaxChrgPwrA")}, av->getfName());
        error = true;
    }

    assetVar* pcsMaxDschgPwrA = ESSUtils::getAvFromParam(vmap, amap, av, "pcsMaxDschgPwrA");
    if (!pcsMaxDschgPwrA)
    {
        if(debug)FPS_PRINT_WARN("Side A PCS maximum discharge power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pcsMaxDschgPwrA")}, av->getfName());
        error = true;
    }

    assetVar* pcsMaxChrgPwrB = ESSUtils::getAvFromParam(vmap, amap, av, "pcsMaxChrgPwrB");
    if (!pcsMaxChrgPwrB)
    {
        if(debug)FPS_PRINT_WARN("Side B PCS maximum charge power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pcsMaxChrgPwrB")}, av->getfName());
        error = true;
    }

    assetVar* pcsMaxDschgPwrB = ESSUtils::getAvFromParam(vmap, amap, av, "pcsMaxDschgPwrB");
    if (!pcsMaxDschgPwrB)
    {
        if(debug)FPS_PRINT_WARN("Side B PCS maximum discharge power variable [{}] in av [{}] does not exist", cstr{av->getcParam("pcsMaxDschgPwrB")}, av->getfName());
        error = true;
    }

    double bmsAEnergy;
    assetVar* bmsAEnergyVar = ESSUtils::getAvFromParam(vmap, amap, av, "bmsAEnergy");
    if (!bmsAEnergyVar)
    {
        if(debug)FPS_PRINT_WARN("Side A battery energy variable [{}] in av [{}] does not exist", cstr{av->getcParam("bmsAEnergy")}, av->getfName());
        bmsAEnergy = 0.0;
    } 
    else
    {
        bmsAEnergy = bmsAEnergyVar->getdVal();
    }

    double bmsBEnergy;
    assetVar* bmsBEnergyVar = ESSUtils::getAvFromParam(vmap, amap, av, "bmsBEnergy");
    if (!bmsAEnergyVar)
    {
        if(debug)FPS_PRINT_WARN("Side B battery energy variable [{}] in av [{}] does not exist", cstr{av->getcParam("bmsBEnergy")}, av->getfName());
        bmsBEnergy = 0.0;
    } 
    else
    {
        bmsBEnergy = bmsBEnergyVar->getdVal();
    }

    double pCmd                = pCmdVar->getdVal()/2; //half command to each side as default.
    double qCmd                = qCmdVar->getdVal()/2;
    double threshold           = av->getdParam("threshold");
    double ratio               = av->getdParam("ratio");
    double lastRatio           = av->getdParam("lastRatio");
    double timeoutMs           = av->getdParam("timeoutMs");
    double scaleFactor         = av->getdParam("scaleFactor");
    double deadband            = av->getdParam("deadband");
    double dcCurrentThreshold  = av->getdParam("dcCurrentThreshold");
    double tLast               = av->getdParam("tLast");
    double pOutputA            = pCmd; //initialize to even split
	double pOutputB            = pCmd;
	double qOutputA            = qCmd;
	double qOutputB            = qCmd;

    if(threshold <= 0)
    {
        if(debug)FPS_PRINT_WARN("Threshold must be greater than Zero. No balancing will occur");
        error = true;
    }

    if(reset || disabled || error)
    {
        ratio = 0;
        lastRatio = 0;
        if(debug)FPS_PRINT_INFO("Ratio reset reached. Flags: RESET: {} DISABLED: {} ERROR: {}\n", reset, disabled, error);
        reset = false;

        av->setParam("ratio", ratio);
        av->setParam("lastRatio", av->getdParam("ratio"));
        av->setParam("reset", reset);

        vm->setVal(vmap, pOutputAVar->comp.c_str(), pOutputAVar->name.c_str(), pCmd);//even split
        vm->setVal(vmap, pOutputBVar->comp.c_str(), pOutputBVar->name.c_str(), pCmd);
        vm->setVal(vmap, qOutputAVar->comp.c_str(), qOutputAVar->name.c_str(), qCmd);
        vm->setVal(vmap, qOutputBVar->comp.c_str(), qOutputBVar->name.c_str(), qCmd);
        return -1;
    }

    double deltaV              = bmsVoltageA->getdVal() - bmsVoltageB->getdVal();
    double inputDCCurrent      = dcCurrentVar->getdVal();
    double tnow                = vm->get_time_dbl();
    double maxChrgPwrA         = pcsMaxChrgPwrA->getdVal();
    double maxDschrgPwrA       = pcsMaxDschgPwrA->getdVal(); 
    double maxChrgPwrB         = pcsMaxChrgPwrB->getdVal();
    double maxDschrgPwrB       = pcsMaxDschgPwrB->getdVal();

    if(scaleFactor > 1)
        scaleFactor = 1;
    else if(scaleFactor < 0)
        scaleFactor = 0;

    //Now the fun begins
    if ((std::abs(deltaV) >= deadband) && (std::abs(inputDCCurrent) >= dcCurrentThreshold) && (tnow - tLast >= timeoutMs/1000))
    {
        ratio = lastRatio + (scaleFactor * deltaV / threshold);
        av->setParam("tLast", tnow);
        if (debug)FPS_PRINT_INFO("ratio = lastRatio + scaleFactor * deltaV / threshold = {}", ratio);

        if (ratio > 1)
            ratio = 1;
        else if (ratio < -1)
            ratio = -1;

        av->setParam("ratio", ratio);
        av->setParam("lastRatio", av->getdParam("ratio")); 

        if (debug)FPS_PRINT_INFO("RATIO UPDATED. New ratio: [{}]   Last ratio: [{}]", ratio, lastRatio);  
    }

    double totalEnergy = bmsAEnergy + bmsBEnergy;
    double pSplitA = (bmsAEnergy / totalEnergy) * pCmd * 2; //We made pcmd be half of the total command earlier as a default value, so undo that here. 
    double pSplitB = (bmsBEnergy / totalEnergy) * pCmd * 2;
    if(bmsAEnergy < 0.0) bmsAEnergy = 0.0;
    if(bmsBEnergy < 0.0) bmsBEnergy = 0.0;
    if((bmsAEnergy == 0.0) || (bmsBEnergy == 0.0)) //not configured, or incorrect reading. 
    {
        if(debug)FPS_PRINT_INFO("Battery energies not found or incorrectly read. Side A energy [{}], Side B energy [{}]", bmsAEnergy, bmsBEnergy);
        pSplitA = pCmd; //even split
        pSplitB = pCmd;
    }
    if(debug)FPS_PRINT_INFO("pSplitA: [{}]   pSplitB:[{}]", pSplitA, pSplitB); 
    pOutputA = pSplitA + std::abs(pCmd) * ratio;
    pOutputB = pSplitB - std::abs(pCmd) * ratio;
    qOutputA = qCmd + std::abs(qCmd) * ratio;
    qOutputB = qCmd - std::abs(qCmd) * ratio;

    if(debug)FPS_PRINT_INFO("pOutputA:  [{}]  pOutputB: [{}]", pOutputA, pOutputB); 
    //Check for violations of power limits and adjust. 
    //If we have to choose between violating power limits and violating voltage balance limits then we conservatively allow
    //voltage to unbalance instead of violating PCS power limits. 
    if((pOutputA > maxDschrgPwrA) || (pOutputB > maxDschrgPwrB) || (pOutputA < maxChrgPwrA) || (pOutputB < maxChrgPwrB))
    {
        if((pOutputA+pOutputB) > (maxDschrgPwrA + maxDschrgPwrB))
        {
            pOutputA = maxDschrgPwrA;
            pOutputB = maxDschrgPwrB;
        }
        else if ((pOutputA + pOutputB) < (maxChrgPwrA + maxChrgPwrB))
        {
            pOutputA = maxChrgPwrA;
            pOutputB = maxChrgPwrB;
        }
        else
        {
            if(pOutputA > maxDschrgPwrA)
            {
                pOutputB = pOutputB + (pOutputA - maxDschrgPwrA);
                pOutputA = maxDschrgPwrA;
            }
            else if(pOutputB > maxDschrgPwrB)
            {
                pOutputA = pOutputA + (pOutputB - maxDschrgPwrB);
                pOutputB = maxDschrgPwrB;
            }
            else if(pOutputA < maxChrgPwrA)
            {
                pOutputB = pOutputB + (pOutputA - maxChrgPwrA);
                pOutputA = maxChrgPwrA;
            }
            else if(pOutputB < maxChrgPwrB)
            {
                pOutputA = pOutputA + (pOutputB - maxChrgPwrB);
                pOutputB = maxChrgPwrB;
            }
        }
    }

    vm->setVal(vmap, pOutputAVar->comp.c_str(), pOutputAVar->name.c_str(), pOutputA);
    vm->setVal(vmap, pOutputBVar->comp.c_str(), pOutputBVar->name.c_str(), pOutputB);
    vm->setVal(vmap, qOutputAVar->comp.c_str(), qOutputAVar->name.c_str(), qOutputA);
    vm->setVal(vmap, qOutputBVar->comp.c_str(), qOutputBVar->name.c_str(), qOutputB);

    if (debug)
        {
            FPS_PRINT_INFO("av: [{}]", av->getfName());
            FPS_PRINT_INFO("deltaV calculated: [{}]. inputDCCurrent: [{}]. tnow: [{}]. tLast: [{}]", deltaV, inputDCCurrent, tnow, tLast);
            FPS_PRINT_INFO("ScaleFactor: [{}]. Delta voltage threshold: [{}]", scaleFactor, threshold);
            FPS_PRINT_INFO("Ratio: [{}]", ratio);
            FPS_PRINT_INFO("Input power commands. Active: [{} kW]. Reactive [{} kW]", pCmd*2, qCmd*2);
            FPS_PRINT_INFO("Output active power commands. Side A: [{} kW]. Side B: [{} kW]", pOutputA, pOutputB);
            FPS_PRINT_INFO("Output rective power commands. Side A: [{} kW]. Side B: [{} kW]", qOutputA, qOutputB);
        }
    return 0;
}

#endif