#ifndef HANDLEPOWER_CPP
#define HANDLEPOWER_CPP

#include "asset.h"
#include "formatters.hpp"

char* strtime(const struct tm *timeptr);

/**
 * 
 */
 extern "C++" {

int HandlePowerCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av);
int HandlePowerInit(varsmap& vmap, VarMapUtils* vm, varmap& amap, const char* aname, fims* p_fims, const char * p , const char * b,int reload);
// int HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset*am);
int HandlePowerLimit(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av);
int CheckTableVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av1, assetVar* av2, assetVar* tblAv);
int DeratePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int EstimatePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int DerateRack(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int AggregateManager(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int initVec(varsmap &vmap, VarMapUtils* vm, varmap &amap,  const char* mname,double initVal, double defdepth);
int CheckValueChanged(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av, bool reset);

}
// run through all children finding max / min of a particular attrubute
//  For a given input value work through all the children for the same value 
//  /status/bms:MaxChargeCurrent   for example
// there must be /status/sbmu_xx:MaxChargeCurrent for all the children
// results are placed in Params
// use selectVal to pick what goes into the main value 
//
int AggregateManager(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    //return 0;
    bool debug = false;
    double dval = 0.0;
    //double ival = 0;
    asset_manager* am = aV->am;

    if(debug)FPS_PRINT_INFO(" Running  am ({}) for [{}] aname [{}]"
            , fmt::ptr(am), cstr{aV->getfName()}, aname); // spdlog
    if (!am || !am->vm)
    {
        FPS_PRINT_ERROR("no am ({}) (or vm) for [{}] aname [{}]"
            , fmt::ptr(am), aV->getfName(), aname); // spdlog
        return 0;
    }
    VarMapUtils* vm = am->vm;
    const char* vname = aV->name.c_str(); 

    // get the maxcharge / disharge current and volts for each rack
    if (!amap[vname])
    {
        FPS_PRINT_ERROR("no [{}] in  aname [{}]", vname, aname ); // spdlog
        amap[vname]    = vm->setLinkVal(vmap, am->name.c_str(), "/status", vname, dval);
    }
    if (1)
    {
        // selectVal picks the aggregate we want to propagate
        char * selectVal = nullptr;   
        if (aV->gotParam("selectVal"))
        {
            selectVal = aV->getcParam("selectVal");
        }
        if(!selectVal)
        {
            selectVal = (char *)"MinVal";   
        }
        auto tmpsel  = fmt::format("{}_{}",selectVal,vname);
        dval = 0.0;
        amap[vname]->setVal(dval);
        auto tmpmax  = fmt::format("{}_{}","MaxVal",vname);
        auto tmpmaxn = fmt::format("{}_{}","MaxName",vname);
        auto tmpmin  = fmt::format("{}_{}","MinVal",vname);
        auto tmpminn = fmt::format("{}_{}","MinName",vname);
        auto tmpcnt  = fmt::format("{}_{}","Num",vname);
        auto tmpena  = fmt::format("{}_{}","Enable",vname);

        double maxVal = 0.0;
        double minVal = 0.0;
        char* maxName = nullptr; 
        char* minName = nullptr; 
        int vcount = 0;
        for (auto& iy : am->assetMap)
        {
            asset* ami = iy.second;
            if (debug) FPS_PRINT_INFO(" Got asset instance {}. Running  now..", ami->name);

            if (!ami->amap[vname])
            {
                ami->amap[vname]= vm->setLinkVal(vmap, ami->name.c_str(), "/status", vname, dval);
            }
            assetVar* avi = ami->amap[vname];
            dval = avi->getdVal();
            bool enabled =  true;
            if (avi->gotParam(tmpena.c_str()))
            {
                enabled = (avi->getbParam(tmpena.c_str()));
            }
            if(enabled && dval > 0) 
            {
                if((maxVal == 0.0)||(dval > maxVal))
                {
                    maxVal = dval;
                    maxName = (char*)ami->name.c_str();
                    amap[vname]->setParam(tmpmaxn.c_str(), maxName);

                }
                if((minVal == 0.0) || (dval < minVal))
                {
                    minVal = dval;
                    minName = (char *)ami->name.c_str();
                    amap[vname]->setParam(tmpminn.c_str(), minName);

                }
                vcount++;
            }
        }
        amap[vname]->setParam(tmpmax.c_str(), maxVal);
        amap[vname]->setParam(tmpmin.c_str(), minVal);
        amap[vname]->setParam(tmpcnt.c_str(), vcount);

        if (amap[vname]->gotParam(tmpsel.c_str()))
        {
            amap[vname]->setVal(amap[vname]->getdParam(tmpsel.c_str()));
        }
        if(vcount > 0)
        {
            if(debug) FPS_PRINT_INFO("{} MaxVal {} in {} MinVal {} in {} select {}\n"
                    , cstr{aV->getfName()} , maxVal, cstr{maxName}, minVal, cstr{minName}, tmpsel);
        }
    }
    return 0;
}

int HandlePowerInit(varsmap& vmap, VarMapUtils* vm, varmap& amap, const char* aname, fims* p_fims, const char * pcsch, const char* bmsch, int reload)
{
    double dval = 0.0; 
    char* cval = nullptr;
    double P = 0;
    double Q = 0;
    double S = 0;
    double pf = 0;
    double PDMax = 0;
    double PCMax = 0;
    bool debug = true;
    if (debug) FPS_PRINT_INFO("RUNNING RELOAD {} for {}", reload, pcsch);
    // Rated power values may come from either configuration (/config/pcs) or hardware (/components/pcs_info -> /limits/pcs)
    // First we see if either exist for P, Q, S and pf
    auto lpcs = fmt::format("/limits/{}", pcsch);
    auto cfgpcs = fmt::format("/config/{}", pcsch);
    auto conpcs = fmt::format("/controls/{}", pcsch);

    assetVar* Pr = vm->getVar(vmap, lpcs.c_str(), "RatedActivePower");
    assetVar* Qr = vm->getVar(vmap, lpcs.c_str(), "RatedReactivePower");
    assetVar* Sr = vm->getVar(vmap, lpcs.c_str(), "RatedApparentPower");
    assetVar* pfr = vm->getVar(vmap, lpcs.c_str(), "RatedPowerFactor");
    assetVar* PC = vm->getVar(vmap, lpcs.c_str(), "ChargePowerLimit");
    assetVar* PD = vm->getVar(vmap, lpcs.c_str(), "DischargePowerLimit");
    assetVar* Pr_c = vm->getVar(vmap, cfgpcs.c_str(), "RatedActivePower");
    assetVar* Qr_c = vm->getVar(vmap, cfgpcs.c_str(), "RatedReactivePower");
    assetVar* Sr_c = vm->getVar(vmap, cfgpcs.c_str(), "RatedApparentPower");
    assetVar* pfr_c = vm->getVar(vmap, cfgpcs.c_str(), "RatedPowerFactor");
    assetVar* PC_c = vm->getVar(vmap, cfgpcs.c_str(), "MaxChargePower");
    assetVar* PD_c = vm->getVar(vmap, cfgpcs.c_str(), "MaxDischargePower");
    assetVar* PMaxSet_c = vm->getVar(vmap, cfgpcs.c_str(), "MaxActivePowerSetpoint");
    assetVar* Priority_c = vm->getVar(vmap, cfgpcs.c_str(), "PowerPriority");
    assetVar* Priority_ctrl = vm->getVar(vmap, conpcs.c_str(), "PowerPriority");
    if (!Priority_ctrl) FPS_PRINT_INFO("No Priority found for {}", pcsch);
    if (debug) FPS_PRINT_INFO("Done checking hw and config");
    
    // First look for limits from hardware
    // If HW limit wasn't found at the first run, don't create "_HW" entry in /limits
    if (Sr && (reload < 2 || amap["RatedApparentPower_HW"]))
    {
        S = Sr->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found Sr_hw [{}]", aname, S);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedApparentPower_HW");
        amap["RatedApparentPower_HW"]->setVal(S);
    }
    if (Pr && (reload < 2 || amap["RatedActivePower_HW"]))
    {
        P = Pr->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found Pr_hw [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedActivePower_HW");
        amap["RatedActivePower_HW"]->setVal(P);
    }
    if (Qr && (reload < 2 || amap["RatedReactivePower_HW"]))
    {
        Q = Qr->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found Qr_hw [{}]", aname, Q);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedReactivePower_HW");
        amap["RatedReactivePower_HW"]->setVal(Q);
    }
    if (pfr && (reload < 2 || amap["RatedPowerFactor_HW"]))
    {
        pf = pfr->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found pfr_hw [{}]", aname, pf);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedPowerFactor_HW");
        amap["RatedPowerFactor_HW"]->setVal(pf);
    }
    if (PC && (reload < 2 || amap["MaxChargePower_HW"]))
    {
        P = PC->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found PC_hw [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "MaxChargePower_HW");
        amap["MaxChargePower_HW"]->setVal(P);
    }
    if (PD && (reload < 2 || amap["MaxDischargePower_HW"]))
    {
        P = PD->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found PD_hw [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "MaxDischargePower_HW");
        amap["MaxDischargePower_HW"]->setVal(P);
    }
    if (Pr_c)
    {
        P = Pr_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found Pr_c [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedActivePower_Config");
        amap["RatedActivePower_Config"]->setVal(P);
    }
    if (Qr_c)
    {
        Q = Qr_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found Qr_c [{}]", aname, Q);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedReactivePower_Config");
        amap["RatedReactivePower_Config"]->setVal(Q);
    }
    if (Sr_c)
    {
        S = Sr_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found Sr_c [{}]", aname, S);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedApparentPower_Config");
        amap["RatedApparentPower_Config"]->setVal(S);
    }
    if (pfr_c)
    {
        pf = pfr_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found pfr_c [{}]", aname, pf);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "RatedPowerFactor_Config");
        amap["RatedPowerFactor_Config"]->setVal(pf);
    }
    if (PC_c)
    {
        P = PC_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found PC_c [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "MaxChargePower_Config");
        amap["MaxChargePower_Config"]->setVal(P);
        if (!amap["MaxChargePower_HW"])                 PC = PC_c;
    }
    if (PD_c)
    {
        P = PD_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found PD_c [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "MaxDischargePower_Config");
        amap["MaxDischargePower_Config"]->setVal(P);
        if (!amap["MaxDischargePower_HW"])              PD = PD_c;
    }
    if (PMaxSet_c)
    {
        P = PMaxSet_c->getdVal();
        if (debug) FPS_PRINT_INFO("{} Found PmaxSet_c [{}]", aname, P);
        linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "MaxActivePowerSetpoint_Config");
        amap["MaxActivePowerSetpoint_Config"]->setVal(P);
        if (!amap["RatedActivePower_HW"] && !amap["RatedActivePower_config"])        Pr = PMaxSet_c;
    }
    if (debug) FPS_PRINT_INFO("Done initializing vars");

    if (Priority_ctrl)    cval = Priority_ctrl->getcVal();
    if (Priority_c && !cval)
    {
        cval = Priority_c->getcVal();
        if (cval[0] == 'p' || cval[0] == 'q' || cval[0] == 'P' || cval[0] == 'Q')
        {
            if (debug) FPS_PRINT_INFO("{} Found Priority from config: {}", aname, cval);
            linkVals(*vm, vmap, amap, pcsch, "/limits", cval, "PowerPriority_config");
            linkVals(*vm, vmap, amap, pcsch, "/controls", cval, "PowerPriority");
            amap["PowerPriority_config"]->setVal(cval);
            amap["PowerPriority"]->setVal(cval);
        }
        else
        {
            cval = (char *) "P";
            if (debug) FPS_PRINT_INFO("{} Priority from config invalid. Setting power priority to default P {}", aname, cval);
            linkVals(*vm, vmap, amap, pcsch, "/controls", cval, "PowerPriority");
            amap["PowerPriority"]->setVal(cval);
        }
    }
    else if (!Priority_c || !cval || (cval[0] != 'p' && cval[0] != 'q' && cval[0] != 'P' && cval[0] != 'Q'))
    {
        cval = (char *) "P";
        if (debug) FPS_PRINT_INFO("{} No priority found in /config or /controls. Setting power priority for {} to default P {}", aname, pcsch, cval);
        linkVals(*vm, vmap, amap, pcsch, "/controls", cval, "PowerPriority");
        amap["PowerPriority"]->setVal(cval);
    }
    if (debug) FPS_PRINT_INFO("Done initializing Priority");

    P = -1;
    Q = -1;
    S = -1;
    pf = 0.75;              // Default power factor (from UL1741)
    if      (amap["RatedActivePower_HW"])                   P = Pr->getdVal();
    else if (amap["RatedActivePower_Config"])               P = Pr_c->getdVal();
    if      (amap["RatedReactivePower_HW"])                 Q = Qr->getdVal();
    else if (amap["RatedReactivePower_Config"])             Q = Qr_c->getdVal();
    if      (Sr && amap["RatedApparentPower_HW"])           S = Sr->getdVal();
    else if (Sr_c && amap["RatedApparentPower_Config"])     S = Sr_c->getdVal();
    if      (amap["RatedPowerFactor_HW"])                   pf = pfr->getdVal();
    else if (amap["RatedPowerFactor_Config"])               pf = pfr_c->getdVal();
    if (pf <= 0 || pf > 1)                                  pf = 0.75;
    if (debug) FPS_PRINT_INFO("Done initializing Ps and Qs");

    linkVals(*vm, vmap, amap, pcsch, "/limits", dval, "MaxPCSReactivePower", "RatedApparentPower");
    
    if (P > -1 && Q > -1 && S > -1)
    {
        if (P > S && !amap["RatedActivePower_HW"])          P = S;
        if (Q > S && !amap["RatedReactivePower_HW"])        Q = S;
        FPS_PRINT_INFO("Rated Active/Reactive/Apparent Powers set to {}/{}/{}", P, Q, S);
    }
    else if (P > -1)
    {
        if (Q > -1 && S == -1)
        {
            S = sqrt((P * P) + (Q * Q));
            FPS_PRINT_INFO("Rated Apparent Power set to {}", S);
        }
        else if (S > -1 && Q == -1)
        {
            if (S > P && !amap["RatedActivePower_HW"])
            {
                P = S;
                Q = sqrt((S * S) - (P * P));
                FPS_PRINT_INFO("Rated Active Power limited to S: {}", P);
            }
            else if (S > P)            Q = sqrt((S * S) - (P * P));
            else                       Q = sqrt(S * S * (1 - (pf * pf)));
            FPS_PRINT_INFO("Rated Reactive Power set to {}", Q);
        }
        else
        {
            S = P / pf;
            Q = sqrt(S * S * (1 - (pf * pf)));
            FPS_PRINT_INFO("Rated Apparent/Reactive Power set to {}/{}", S, Q);
        }
    }
    else if (S > 0)
    {
        P = S * pf;
        if (Q == -1 || Q > S)
        {
            Q = sqrt(S * S * (1 - (pf * pf)));
        }
        FPS_PRINT_INFO("Rated Active/Reactive Power set to {}/{}", P, Q);
    }
    else
    {
        if (PC || PD)
        {
            if (PC && PD)          P = -PC->getdVal() > PD->getdVal() ? -PC->getdVal() : PD->getdVal();
            else if (PC)           P = -PC->getdVal();
            else if (PD)           P = PD->getdVal();
            S = P / pf;
            Q = sqrt(S * S * (1 - (pf * pf)));
            FPS_PRINT_INFO("Rated Active/Reactive/Apparent Powers set to max charge/discharge powers {}/{}/{}", P, Q, S);
        }
        else if (Q > -1)
        {
            S = sqrt(Q * Q / (1 - (pf * pf)));
            P = S * pf;
            FPS_PRINT_INFO("Rated Active/apparent powers set using {} pf: {}/{}", pf, P, S);
        }
        else if (PMaxSet_c)
        {
            P = PMaxSet_c->getdVal();
            S = P / pf;
            Q = sqrt(S * S * (1 - (pf * pf)));
            FPS_PRINT_INFO("Rated Active power set to MaxActivePowerSetpoint: {}", P);
            FPS_PRINT_INFO("Rated reactive/apparent powers set using {} pf: {}/{}", pf, Q, S);
        }
        else FPS_PRINT_ERROR("PCS Config and hardware missing rated active/apparent/reactive power and/or power factor");
    }
    if (debug) FPS_PRINT_INFO("Done initing vars");
    
    amap["MaxPCSReactivePower"]->setVal(Q);
    amap["RatedApparentPower"]->setVal(S);

    if (PD)
    {
        PDMax = PD->getdVal();
        if (PDMax > P || PDMax < 0)      PDMax = P;
    }
    else PDMax = P;

    if (PC)
    {
        PCMax = PC->getdVal();
        if (PCMax < -P || PCMax > 0)     PCMax = -P;
    }
    else PCMax = -P;
    
    if (PMaxSet_c)
    {
        P = PMaxSet_c->getdVal();
        if (P < PDMax)                   PDMax = P;
        if (P < -PCMax)                  PCMax = -P;
    }
    if (debug) FPS_PRINT_INFO("Done setting max Ps");

    amap["MaxPCSDischargePower"] = vm->setLinkVal(vmap, pcsch, "/limits", "MaxPCSDischargePower", PDMax);
    amap["MaxPCSDischargePower"]->setVal(PDMax);
    amap["MaxPCSChargePower"] = vm->setLinkVal(vmap, pcsch, "/limits", "MaxPCSChargePower", PCMax);
    amap["MaxPCSChargePower"]->setVal(PCMax);

    FPS_PRINT_INFO("Rated Power for {}:", pcsch);
    FPS_PRINT_INFO("       Apparent Power  [{}]", amap["RatedApparentPower"]->getdVal());
    FPS_PRINT_INFO("       Reactive Power  [{}]", amap["MaxPCSReactivePower"]->getdVal());
    
    FPS_PRINT_INFO("Max Powers for {}:", pcsch);
    FPS_PRINT_INFO("       Charge Power    [{}]", amap["MaxPCSChargePower"]->getdVal());
    FPS_PRINT_INFO("       Discharge Power [{}]", amap["MaxPCSDischargePower"]->getdVal());

    return 1;
}

int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    bool debug = false;
    double dval= 0.0;
    bool bval = false;
    char* cval = nullptr;
    int rc = 0;
    int reload = 0;
    asset_manager * am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    const char *bmsch = (const char*)"bms";
    const char *pcsch = (const char*)"pcs";
    const char *essch = (const char*)"ess";
    if (aV->gotParam("bms"))
    {
        bmsch = aV->getcParam("bms");
    }
    if (aV->gotParam("pcs"))
    {
        pcsch = aV->getcParam("pcs");
    }
    if (aV->gotParam("ess"))
    {
        essch = aV->getcParam("ess");
    }
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    auto relname = fmt::format("{}_{}", __func__, essch).c_str() ;
    assetVar* hpAv = amap[relname];
    if (!hpAv || (reload = hpAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, essch, "/reload", reload, relname);
        hpAv = amap[relname];
    
        linkVals(*vm, vmap, amap, essch, "/limits", dval, "MaxChargePower", "MaxDischargePower", "MaxReactivePower", "MaxApparentPower");

        linkVals(*vm, vmap, amap, pcsch, "/controls", dval, "ActivePowerSetpoint", "ReactivePowerSetpoint", "ActivePowerCmdReal", "ReactivePowerCmdReal");
        linkVals(*vm, vmap, amap, pcsch, "/controls", cval, "PowerPriority");
        linkVals(*vm, vmap, amap, pcsch, "/limits", bval, "PCSLimitsChanged");
        linkVals(*vm, vmap, amap, pcsch, "/config", bval, "PCSConfigChanged");
        
        linkVals(*vm, vmap, amap, bmsch, "/limits", dval, "ChargePowerLimit", "DischargePowerLimit");
        
        HandlePowerInit(vmap, vm, amap, aname, p_fims, pcsch, bmsch, reload);

        if (reload == 0)
        {
            amap["ActivePowerSetpoint"]->setParam("PpLimited", false);
            amap["ActivePowerSetpoint"]->setParam("PnLimited", false);
            amap["ReactivePowerSetpoint"]->setParam("QpLimited", false);
            amap["ReactivePowerSetpoint"]->setParam("QnLimited", false);
        }
        reload = 2;
        hpAv->setVal(reload);
    }
    
    if (debug) FPS_PRINT_INFO("Running reload {} essch {} for {}", reload, essch, aV->getfName());

    // Update /pcs/limits with HandlePowerLimit if anything in the table or anything in the config changes
    if (CheckValueChanged(vmap, amap, essch, p_fims, amap["PCSConfigChanged"], false)
      | CheckValueChanged(vmap, amap, essch, p_fims, amap["PCSLimitsChanged"], false))
    {
        if (debug) FPS_PRINT_INFO("Limits Changed  essch {} aname {}  aV {}", essch, aname, aV->getfName());
        HandlePowerInit(vmap, vm, amap, aname, p_fims, pcsch, bmsch, reload);
        bval = true;
        asset_manager* am1 = vm->getaM(vmap, (const char*)aname);
        asset_manager* am2 = vm->getaM(vmap, (const char*)essch);
        varmap *amap1 = nullptr;
        varmap *amap2 = nullptr;
        if(am1)
        {
            amap1 = am1->getAmap();
        }
        if(am2)
        {
            amap2 = am2->getAmap();
        }
        if (debug) FPS_PRINT_INFO("Amap check  amap {} aname {}->{}->{}  essch {}->{}->{} "
                    , fmt::ptr(&amap)
                    , aname, fmt::ptr(am1), fmt::ptr(amap1)
                    , essch, fmt::ptr(am2), fmt::ptr(amap2)
                    );
        
    }

    if (amap["ChargePowerLimit"]->valueChangedReset() | amap["DischargePowerLimit"]->valueChangedReset() || bval)
    {
        if (debug) FPS_PRINT_INFO("MaxBMSChargePower [{}] MaxBMSDischargePower [{}] RatedApparentPower [{}]"
                    , amap["ChargePowerLimit"]->getdVal()
                    , amap["DischargePowerLimit"]->getdVal()
                    , amap["RatedApparentPower"]->getdVal()
                    );

        double Qmax_PCS = amap["MaxPCSReactivePower"]->getdVal();
        double Smax_PCS = amap["RatedApparentPower"]->getdVal();
        double Pmax_D_PCS = amap["MaxPCSDischargePower"]->getdVal();
        double Pmax_C_PCS = amap["MaxPCSChargePower"]->getdVal();
        double Pmax_D_BMS = amap["DischargePowerLimit"]->getdVal();
        double Pmax_C_BMS = amap["ChargePowerLimit"]->getdVal();
        double Smax_D = Smax_PCS;
        double Smax_C = Smax_PCS;
        
        if (Pmax_D_BMS < 0)           Pmax_D_BMS = 0;
        if (Pmax_C_BMS > 0)           Pmax_C_BMS *= -1;
        if (Qmax_PCS > Smax_PCS)      Qmax_PCS = Smax_PCS;

        //Compare BMS limits to PCS, take minimum
        if (Pmax_D_BMS > Pmax_D_PCS)
        {
            amap["MaxDischargePower"]->setVal(Pmax_D_PCS);
            Smax_D = sqrt((Pmax_D_PCS * Pmax_D_PCS) + (Qmax_PCS * Qmax_PCS));
        }
        else
        {
            amap["MaxDischargePower"]->setVal(Pmax_D_BMS);
            Smax_D = sqrt((Pmax_D_BMS * Pmax_D_BMS) + (Qmax_PCS * Qmax_PCS));
        }
        if (Pmax_C_BMS < Pmax_C_PCS)
        {
            amap["MaxChargePower"]->setVal(Pmax_C_PCS);
            Smax_C = sqrt((Pmax_C_PCS * Pmax_C_PCS) + (Qmax_PCS * Qmax_PCS));
        }
        else
        {
            amap["MaxChargePower"]->setVal(Pmax_C_BMS);
            Smax_C = sqrt((Pmax_C_BMS * Pmax_C_BMS) + (Qmax_PCS * Qmax_PCS));
        }

        if (Smax_C > Smax_D)
        {
            if (Smax_D > Smax_PCS)      amap["MaxApparentPower"]->setVal(Smax_PCS);
            else                        amap["MaxApparentPower"]->setVal(Smax_D);
        }
        else
        {
            if (Smax_C > Smax_PCS)      amap["MaxApparentPower"]->setVal(Smax_PCS);
            else                        amap["MaxApparentPower"]->setVal(Smax_C);
        }
        
        amap["MaxReactivePower"]->setVal(Qmax_PCS);
        
        if (debug) FPS_PRINT_INFO("Max Charge Power [{}] kW Max Discharge Power [{}] kW Qmax [{}] kVAr Smax [{}] kVA "
                            , amap["MaxChargePower"]->getdVal()
                            , amap["MaxDischargePower"]->getdVal()
                            , amap["MaxReactivePower"]->getdVal()
                            , amap["MaxApparentPower"]->getdVal()
                            );
    }

    if (amap["MaxChargePower"]->valueChangedReset() | amap["MaxDischargePower"]->valueChangedReset() | amap["MaxReactivePower"]->valueChangedReset()
      | amap["ActivePowerSetpoint"]->valueChangedReset() | amap["ReactivePowerSetpoint"]->valueChangedReset()
      | amap["MaxApparentPower"]->valueChangedReset() | amap["PowerPriority"]->valueChanged())
    {
        char* clval = amap["PowerPriority"]->getcLVal();
        cval = amap["PowerPriority"]->getcVal();
        if (cval[0] != 'P' && cval[0] != 'Q' && cval[0] != 'p' && cval[0] != 'q')       amap["PowerPriority"]->setVal(clval);
        amap["PowerPriority"]->valueChangedReset();

        if (debug) FPS_PRINT_INFO("MaxChargePower [{}] MaxDischargePower [{}] MaxReactivePower [{}] MaxApparentPower [{}] \
                                    PowerPriority [{}] ActivePowerSetpoint [{}] ReactivePowerSetpoint [{}]"
                            , amap["MaxChargePower"]->getdVal()
                            , amap["MaxDischargePower"]->getdVal()
                            , amap["MaxReactivePower"]->getdVal()
                            , amap["MaxApparentPower"]->getdVal()
                            , amap["PowerPriority"]->getcVal()
                            , amap["ActivePowerSetpoint"]->getdVal()
                            , amap["ReactivePowerSetpoint"]->getdVal()
                            );

        // Start with setpoints, coerce them into range first based on BMS/PCS derived limits, then Apparent power limit
        double Pcmd_lim = amap["ActivePowerSetpoint"]->getdVal();
        double Qcmd_lim = amap["ReactivePowerSetpoint"]->getdVal();

        double Pmax_C = amap["MaxChargePower"]->getdVal();
        double Pmax_D = amap["MaxDischargePower"]->getdVal();
        double Qmax = amap["MaxReactivePower"]->getdVal();
        double Smax = amap["MaxApparentPower"]->getdVal();

        cval = nullptr;
        std::string msg;

        if (Pcmd_lim > Pmax_D)
        {
            if (!amap["ActivePowerSetpoint"]->getbParam("PpLimited"))
            {
                if (debug) FPS_PRINT_INFO("Pset [{}] kW limited to Max Discharge Power [{}] kW", Pcmd_lim, Pmax_D);
                tm* local_tm = vm->get_local_time_now();
                ESSLogger::get().info("Max discharge kW limited to [{}]", Pmax_D);
                msg = fmt::format("[{}] kW setpoint [{}] limited to Discharge power limit [{}] at [{}]",
                    __func__, Pcmd_lim, Pmax_D, strtime(local_tm));
                aV->sendEvent(aname, am->p_fims, Severity::Info, msg.c_str());
                
                amap["ActivePowerSetpoint"]->setParam("PpLimited", true);
                amap["ActivePowerSetpoint"]->setParam("PnLimited", false);
            }
            Pcmd_lim = Pmax_D;
        }
        else if (Pcmd_lim < Pmax_C)
        {
            if (!amap["ActivePowerSetpoint"]->getbParam("PnLimited"))
            {
                if (debug) FPS_PRINT_INFO("Pset [{}] kW limited to Max Charge Power [{}] kW", Pcmd_lim, Pmax_C);
                tm* local_tm = vm->get_local_time_now();
                ESSLogger::get().info("Max charge kW limited to [{}]", Pmax_C);
                msg = fmt::format("[{}] kW setpoint [{}] limited to Charge power limit [{}] at [{}]",
                    __func__, Pcmd_lim, Pmax_C, strtime(local_tm));
                aV->sendEvent(aname, am->p_fims, Severity::Info, msg.c_str());

                amap["ActivePowerSetpoint"]->setParam("PpLimited", false);
                amap["ActivePowerSetpoint"]->setParam("PnLimited", true);
            }
            Pcmd_lim = Pmax_C;
        }
        else
        {
            amap["ActivePowerSetpoint"]->setParam("PpLimited", false);
            amap["ActivePowerSetpoint"]->setParam("PnLimited", false);
        }
        
        if (Qcmd_lim > Qmax)
        {
            if (!amap["ReactivePowerSetpoint"]->getbParam("QpLimited"))
            {
                if (debug) FPS_PRINT_INFO("Qset [{}] kVAr limited to Max Reactive Power [{}] kVAr", Qcmd_lim, Qmax);
                tm* local_tm = vm->get_local_time_now();
                ESSLogger::get().info("Max reactive kVAr limited to [{}]", Qmax);
                msg = fmt::format("[{}] kVAr setpoint [{}] limited to reactive power limit [{}] at [{}]",
                    __func__, Qcmd_lim, Qmax, strtime(local_tm));
                aV->sendEvent(aname, am->p_fims, Severity::Info, msg.c_str());
                
                amap["ReactivePowerSetpoint"]->setParam("QpLimited", true);
                amap["ReactivePowerSetpoint"]->setParam("QnLimited", false);
            }
            Qcmd_lim = Qmax;
        }
        else if (Qcmd_lim < (Qmax * -1))
        {
            if (!amap["ReactivePowerSetpoint"]->getbParam("QnLimited"))
            {
                if (debug) FPS_PRINT_INFO("Qset [{}] kVAr limited to Max Reactive Power [{}] kVAr", Qcmd_lim, Qmax);
                tm* local_tm = vm->get_local_time_now();
                ESSLogger::get().info("Max reactive kVAr limited to -[{}]", Qmax);
                msg = fmt::format("[{}] kVAr setpoint [{}] limited to reactive power limit -[{}] at [{}]",
                    __func__, Qcmd_lim, Qmax, strtime(local_tm));
                aV->sendEvent(aname, am->p_fims, Severity::Info, msg.c_str());
                
                amap["ReactivePowerSetpoint"]->setParam("QpLimited", false);
                amap["ReactivePowerSetpoint"]->setParam("QnLimited", true);
            }
            Qcmd_lim = Qmax * -1;
        }
        else
        {
            amap["ReactivePowerSetpoint"]->setParam("QpLimited", false);
            amap["ReactivePowerSetpoint"]->setParam("QnLimited", false);
        }
        
        if (debug) FPS_PRINT_INFO("After PCS/BMS limits: Pcmd_lim [{}] kW Qcmd_lim [{}] kVAr Smax [{}] kVA", Pcmd_lim, Qcmd_lim, Smax);

        // If S from P and Q requests is too large, decrease P or Q accordingly depending on power priority (P priority by default)
        double Scmd_lim = sqrt((Pcmd_lim * Pcmd_lim) + (Qcmd_lim * Qcmd_lim));
        if (Scmd_lim > Smax)
        {
            if (debug) FPS_PRINT_INFO("S derived from Pcmd and Qcmd is greater than Smax [{}]", Smax);
            cval = amap["PowerPriority"]->getcVal();
            tm* local_tm = vm->get_local_time_now();
            if (cval[0] == 'Q' || cval[0] == 'q')
            {
                if (Pcmd_lim >= 0) Pcmd_lim = sqrt((Smax * Smax) - (Qcmd_lim * Qcmd_lim));
                else               Pcmd_lim = -sqrt((Smax * Smax) - (Qcmd_lim * Qcmd_lim));
                ESSLogger::get().info("Max apparent kVA limited to [{}] under Q priority", Smax);
                msg = fmt::format("[{}] Apparent power limit [{}] exceeded by setpoint [{}]. Active power limited to [{}] at [{}]",
                    __func__, Smax, Scmd_lim, Pcmd_lim, strtime(local_tm));
            }
            else
            {
                if (Qcmd_lim >= 0) Qcmd_lim = sqrt((Smax * Smax) - (Pcmd_lim * Pcmd_lim));
                else               Qcmd_lim = -sqrt((Smax * Smax) - (Pcmd_lim * Pcmd_lim));
                ESSLogger::get().info("Max apparent kVA limited to [{}] under P priority", Smax);
                msg = fmt::format("[{}] Apparent power limit [{}] exceeded by setpoint [{}]. Reactive power limited to [{}] at [{}]",
                    __func__, Smax, Scmd_lim, Qcmd_lim, strtime(local_tm));
            }
            if (debug) FPS_PRINT_INFO("Scmd [{}] kVA limited to Max Apparent Power [{}] kVA", Scmd_lim, Smax);
            aV->sendEvent(aname, am->p_fims, Severity::Info, msg.c_str());
        }
        
        if (debug) FPS_PRINT_INFO("After S limiting: Pcmd [{}] kW Qcmd [{}] kVAr S [{}] kVA", Pcmd_lim, Qcmd_lim, sqrt((Pcmd_lim * Pcmd_lim) + (Qcmd_lim * Qcmd_lim)));
        if (debug) FPS_PRINT_INFO("After S limiting: Pcmd [{}] kW Qcmd [{}] kVAr", amap["ActivePowerCmdReal"]->getdVal(), amap["ReactivePowerCmdReal"]->getdVal());

        // Active/ReactivePowerCmdReal holds power command to hardware in kW
        // Sending command to hardware is done via configuration
        if (amap["ActivePowerCmdReal"]->getdVal() != Pcmd_lim)
        {
            amap["ActivePowerCmdReal"]->setVal(Pcmd_lim);
            if (debug) FPS_PRINT_INFO("Pcmd set to {} kW", amap["ActivePowerCmdReal"]->getdVal());
            ESSLogger::get().info("Pcmd set to [{}]", amap["ActivePowerCmdReal"]->getdVal());
        }
        if (amap["ReactivePowerCmdReal"]->getdVal() != Qcmd_lim)
        {
            amap["ReactivePowerCmdReal"]->setVal(Qcmd_lim);
            if (debug) FPS_PRINT_INFO("Qcmd set to {} kVAr", amap["ReactivePowerCmdReal"]->getdVal());
            ESSLogger::get().info("Qcmd set to [{}]", amap["ReactivePowerCmdReal"]->getdVal());
        }
    }

    return rc;
}

int EstimatePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    double dval = 0.0;
    bool bval = false;
    int rc = 0;
    int reload = 0;
    asset_manager * am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);
    const char *bmsch = (const char*)"bms";
    const char *pcsch = (const char*)"pcs";
    if (aV->gotParam("bms"))
    {
        bmsch = aV->getcParam("bms");
    }
    if (aV->gotParam("pcs"))
    {
        bmsch = aV->getcParam("pcs");
    }

    assetVar* epAv = amap[__func__];
    if (!epAv || (reload = epAv->getiVal()) == 0)
    {
        if (0) FPS_ERROR_PRINT("%s >> Running reload %d \n", __func__, reload);
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        epAv = amap[__func__];
        linkVals(*vm, vmap, amap, bmsch, "/status", dval, "MaxBMSDischargePowerRunning", "MaxBMSChargePowerRunning", "MaxBMSDischargePower", "MaxBMSChargePower", "MaxBMSDischargePowerOffset", "MaxBMSChargePowerOffset");
        linkVals(*vm, vmap, amap, bmsch, "/status", dval, "MaxBMSDischargeIV", "MaxBMSChargeIV", "MaxBMSChargePowerEstFiltIn", "MaxBMSDischargePowerEstFiltIn");
        linkVals(*vm, vmap, amap, pcsch, "/status", bval, "Derate");
        linkVals(*vm, vmap, amap, pcsch, "/status", bval, "ActivePowerSetpoint");

        if (reload == 0)
        {
            if (!amap["MaxBMSDischargePower"]->gotParam("done"))      amap["MaxBMSDischargePower"]->setParam("done", false);
            if (!amap["MaxBMSChargePower"]->gotParam("done"))         amap["MaxBMSChargePower"]->setParam("done", false);
            if (!amap["MaxBMSDischargePower"]->gotParam("active"))    amap["MaxBMSDischargePower"]->setParam("active", false);
            if (!amap["MaxBMSChargePower"]->gotParam("active"))       amap["MaxBMSChargePower"]->setParam("active", false);
            amap["MaxBMSDischargePowerOffset"]->setVal(0);
            amap["MaxBMSChargePowerOffset"]->setVal(0);
            amap["MaxBMSDischargePower"]->setVal(amap["MaxBMSDischargeIV"]->getdVal());
            amap["MaxBMSChargePower"]->setVal(amap["MaxBMSChargeIV"]->getdVal());
            amap["MaxBMSDischargePower"]->setDbVal(10);
            amap["MaxBMSChargePower"]->setDbVal(10);
            if (1) FPS_ERROR_PRINT("%s >> Running reload %d \n", __func__, reload);
        }
        reload = 2;
        epAv->setVal(reload);
    }

    if (!amap["Derate"]->getbParam("Hold"))
    {
        // When filtered max power peaks for the first time, use that power offset as the max
        if (!amap["MaxBMSDischargePower"]->getbParam("done"))
        {
            if (amap["ActivePowerSetpoint"]->getdVal() >= 0)
            {
                if (amap["MaxBMSDischargePowerRunning"]->valueChangedReset())
                {
                    if (amap["MaxBMSDischargePower"]->gotParam("MaxPowerSent") && amap["MaxBMSDischargePower"]->getbParam("MaxPowerSent"))
                    {
                        if (amap["MaxBMSDischargePowerRunning"]->getdVal() <= amap["MaxBMSDischargePowerRunning"]->getdLVal())
                        {
                            amap["MaxBMSDischargePowerOffset"]->setVal(amap["MaxBMSDischargeIV"]->getdVal() - amap["MaxBMSDischargePowerRunning"]->getdVal());
                            amap["MaxBMSDischargePower"]->setParam("done", true);
                            if (1) FPS_ERROR_PRINT("%s >> Max BMS Discharge Power Offset set to %f\n", __func__, amap["MaxBMSDischargePowerOffset"]->getdVal());
                        }
                    }
                    amap["MaxBMSDischargePower"]->setVal(amap["MaxBMSDischargePowerRunning"]->getdVal());
                }
                if (!amap["MaxBMSDischargePower"]->getbParam("active"))
                {
                    double lVal = amap["MaxBMSDischargePower"]->getdParam("lastActiveVal");
                    initVec(vmap, vm, amap, "MaxBMSDischargePowerEstFiltIn", lVal, 120);

                    //vm->setVecDepth(vmap, amap, "MaxBMSDischargePowerEstFiltIn","vecAv",120, &lVal);
                    amap["MaxBMSDischargePower"]->setParam("active", true);
                }
            }
            else if (amap["MaxBMSDischargePower"]->getbParam("active"))
            {
                amap["MaxBMSDischargePower"]->setParam("lastActiveVal", amap["MaxBMSDischargePower"]->getdVal());
                amap["MaxBMSDischargePower"]->setParam("active", false);
            }
            else    amap["MaxBMSDischargePower"]->setVal(amap["MaxBMSDischargePower"]->getdParam("lastActiveVal"));
        }
        else    amap["MaxBMSDischargePower"]->setVal(amap["MaxBMSDischargeIV"]->getdVal() - amap["MaxBMSDischargePowerOffset"]->getdVal());

        if (!amap["MaxBMSChargePower"]->getbParam("done"))
        {
            if (amap["ActivePowerSetpoint"]->getdVal() <= 0)
            {
                if (amap["MaxBMSChargePowerRunning"]->valueChangedReset())
                {
                    if (amap["MaxBMSChargePower"]->gotParam("MaxPowerSent") && amap["MaxBMSChargePower"]->getbParam("MaxPowerSent"))
                    {
                        if (amap["MaxBMSChargePowerRunning"]->getdVal() >= amap["MaxBMSChargePowerRunning"]->getdLVal())
                        {
                            amap["MaxBMSChargePowerOffset"]->setVal(amap["MaxBMSChargeIV"]->getdVal() - amap["MaxBMSChargePowerRunning"]->getdVal());
                            amap["MaxBMSChargePower"]->setParam("done", true);
                            if (1) FPS_ERROR_PRINT("%s >> Max BMS Charge Power Offset set to %f\n", __func__, amap["MaxBMSChargePowerOffset"]->getdVal());
                        }
                    }
                    amap["MaxBMSChargePower"]->setVal(amap["MaxBMSChargePowerRunning"]->getdVal());
                }
                if (!amap["MaxBMSChargePower"]->getbParam("active"))
                {
                    double lVal = amap["MaxBMSChargePower"]->getdParam("lastActiveVal");
                    initVec(vmap, vm, amap, "MaxBMSChargePowerEstFiltIn", lVal, 120);

                    //vm->setVecDepth(vmap, amap, "MaxBMSChargePowerEstFiltIn", "vecAv", 120, &lVal);
                    amap["MaxBMSChargePower"]->setParam("active", true);
                }
            }
            else if (amap["MaxBMSChargePower"]->getbParam("active"))
            {
                amap["MaxBMSChargePower"]->setParam("lastActiveVal", amap["MaxBMSChargePower"]->getdVal());
                amap["MaxBMSChargePower"]->setParam("active", false);
            }
            else    amap["MaxBMSChargePower"]->setVal(amap["MaxBMSChargePower"]->getdParam("lastActiveVal"));
        }
        else    amap["MaxBMSChargePower"]->setVal(amap["MaxBMSChargeIV"]->getdVal() - amap["MaxBMSChargePowerOffset"]->getdVal());
    }
    // else
    // {
    //     amap["MaxBMSDischargePower"]->setVal(amap["MaxDeratedDischargeCmd"]->getdVal());
    //     amap["MaxBMSChargePower"]->setVal(amap["MaxDeratedChargeCmd"]->getdVal());
    // }

    return rc;

}

int DeratePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    FPS_PRINT_INFO("WE'RE DERATING :P:P:P:P:");
    double dval= 0.0;
    bool bval = false;
    int rc = 0;
    int reload = 0;
    asset_manager * am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    assetVar* dpAv = amap[__func__];
    if (!dpAv || (reload = dpAv->getiVal()) == 0)
    {
        if (0) FPS_ERROR_PRINT("%s >> Running reload %d \n", __func__, reload);
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }
    const char *bmsch = (const char*)"bms";
    const char *pcsch = (const char*)"pcs";
    if (aV->gotParam("bms"))
    {
        bmsch = aV->getcParam("bms");
    }
    if (aV->gotParam("pcs"))
    {
        bmsch = aV->getcParam("pcs");
    }

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        dpAv = amap[__func__];
        linkVals(*vm, vmap, amap, pcsch, "/status", dval, "Derate");
        linkVals(*vm, vmap, amap, pcsch, "/status", dval, "SystemState");
        linkVals(*vm, vmap, amap, bmsch, "/status", bval, "OverCurrentD1", "OverCurrentD2", "UnderCurrentD1", "UnderCurrentD2");

        if (reload == 0)
        {
            // amap["Derate"]->setDbVal(0.001);
            if (!amap["Derate"]->gotParam("Increment"))      amap["Derate"]->setParam("Increment", 0);
            if (!amap["Derate"]->gotParam("Decrement"))      amap["Derate"]->setParam("Decrement", 0);
            if (!amap["Derate"]->gotParam("OverCurrent"))    amap["Derate"]->setParam("OverCurrent", false);
            if (!amap["Derate"]->gotParam("Recovery"))       amap["Derate"]->setParam("Recovery", false);
            if (!amap["Derate"]->gotParam("Hold"))           amap["Derate"]->setParam("Hold", false);
            if (!amap["Derate"]->gotParam("Done"))           amap["Derate"]->setParam("Done", false);
            if (!amap["Derate"]->gotParam("LastDerate"))     amap["Derate"]->setParam("LastDerate", 1.0);
            if (!amap["Derate"]->gotParam("HoldDerate"))     amap["Derate"]->setParam("HoldDerate", 1.0);
        }
    }
    
    if (!strcmp(amap["SystemState"]->getcVal(), "Running"))
    {
        double NewDerate = amap["Derate"]->getdVal();
        // |Over/UnderCurrentD2| must be greater than |Over/UnderCurrentD1|
        if (amap["OverCurrentD1"]->getbVal() || amap["UnderCurrentD1"]->getbVal())
        {
            if (amap["OverCurrentD2"]->getbVal() || amap["UnderCurrentD2"]->getbVal())
            {
                if (!amap["Derate"]->getbParam("OverCurrent"))
                {
                    if (1) FPS_ERROR_PRINT("%s >> Active Power Derating starting\n", __func__);
                    const auto now = flex::get_time_dbl();
                    ESSLogger::get().info("Active Power Derating starting");
                    assetVar* temp_av = amap["Derate"];
                    if (temp_av) temp_av->sendEvent(aname, am->p_fims, Severity::Info, "Active Power Derating starting at time %2.3f"
                        , now.count()
                        );
                    amap["Derate"]->setParam("OverCurrent", true);
                    amap["Derate"]->setParam("Recovery", false);
                    amap["Derate"]->setParam("Hold", false);
                    amap["Derate"]->setParam("Done", false);
                }
            }
            // If we're not in Recovery, keep decrementing
            if (!amap["Derate"]->getbParam("Recovery"))
            {
                NewDerate -= amap["Derate"]->getdParam("Decrement");
                if (NewDerate > 0.0)
                {
                    if (1) FPS_ERROR_PRINT("%s >> NewDerate [%f%%]\n", __func__, NewDerate);
                    amap["Derate"]->setVal(NewDerate);
                    amap["Derate"]->setParam("LastDerate", NewDerate);
                }
                else
                {
                    if ((int) amap["Derate"]->getdVal() * 1000 != 0)
                    {
                        if (1) FPS_ERROR_PRINT("%s >> Active Power Derating set to 0\n", __func__);
                        const auto now = flex::get_time_dbl();
                        ESSLogger::get().info("Active Power Derating set to 0%");
                        assetVar* temp_av = amap["Derate"];
                        if (temp_av) temp_av->sendEvent(aname, am->p_fims, Severity::Info, "Active Power Derating set to 0 at time %2.3f"
                            , now.count()
                            );
                    }
                    amap["Derate"]->setVal(0.0);
                    amap["Derate"]->setParam("LastDerate", 0.0);
                }
            }
            // If we've recovered back up to the lower limit, hold
            else if (!amap["Derate"]->getbParam("Hold") && amap["Derate"]->getdVal() < 1.0)
            {
                if (1) FPS_ERROR_PRINT("%s >> Active Power Derating holding at [%f]\n", __func__, amap["Derate"]->getdVal());
                const auto now = flex::get_time_dbl();
                ESSLogger::get().info("Active Power Derating holding at [{}]",
                    amap["Derate"]->getdVal());
                assetVar* temp_av = amap["Derate"];
                if (temp_av) temp_av->sendEvent(aname, am->p_fims, Severity::Info, "Active Power Derating holding at [%f] at time %2.3f"
                    , amap["Derate"]->getdVal()
                    , now.count()
                    );
                amap["Derate"]->setParam("Hold", true);
                amap["Derate"]->setParam("HoldDerate", amap["Derate"]->getdVal());
                double temp_db = amap["Derate"]->getdVal();
                amap["Derate"]->setVal(0.0);
                amap["Derate"]->setVal(temp_db);
            }
        }
        // If we're below the lower limit, recover or do nothing
        else
        {
            NewDerate += amap["Derate"]->getdParam("Increment");
            if (amap["Derate"]->getbParam("OverCurrent") || !amap["Derate"]->getbParam("Recovery"))
            {
                if (1) FPS_ERROR_PRINT("%s >> Active Power Derating recovering at [%f]\n", __func__, NewDerate);
                const auto now = flex::get_time_dbl();
                ESSLogger::get().info("Active Power Derating recovering at [{}]",
                    NewDerate);
                assetVar* temp_av = amap["Derate"];
                if (temp_av) temp_av->sendEvent(aname, am->p_fims, Severity::Info, "Active Power Derating recovering at [%f] at time %2.3f"
                    , NewDerate
                    , now.count()
                    );
                amap["Derate"]->setParam("Recovery", true);
                amap["Derate"]->setParam("OverCurrent", false);
            }
            if (NewDerate < 1.0)
            {
                if (1) FPS_ERROR_PRINT("%s >> NewDerate [%f%%]\n", __func__, NewDerate);
                amap["Derate"]->setVal(NewDerate);
                amap["Derate"]->setParam("LastDerate", NewDerate);
            }
            else if (!amap["Derate"]->getbParam("Done"))
            {
                if (1) FPS_ERROR_PRINT("%s >> Active Power no longer Derating\n", __func__);
                const auto now = flex::get_time_dbl();
                ESSLogger::get().info("Active Power no longer Derating");
                assetVar* temp_av = amap["Derate"];
                if (temp_av) temp_av->sendEvent(aname, am->p_fims, Severity::Info, "Active Power no longer Derating at time %2.3f"
                        , now.count()
                        );
                amap["Derate"]->setParam("Done", true);
                amap["Derate"]->setVal(1.0);
                amap["Derate"]->setParam("LastDerate", 1.0);
            }
            if (amap["Derate"]->getbParam("Hold"))      amap["Derate"]->setParam("Hold", false);
        }
    }
    else if (amap["Derate"]->getdVal() < 1.0)
    {
        amap["Derate"]->setParam("LastDerate", amap["Derate"]->getdVal());
        amap["Derate"]->setVal(1.0);
    }

    return rc;
}

#endif