/*
 * ess test , opens up the master config and creates all the peripherals
 * bms , pcr , drc
 * Starts all those threads if needed
 * handles the interface between hos ( via modbus server pubs) and hte controllers.
 * It has registers and links.
 *  g++ -std=c++11 -g -o ./test_ess -I ./include test/test_ess.cpp -lpthread -lcjson -lfims
 */

#include "asset.h"
#include "channel.h"
#include "monitor.h"
//#include "ess.h"
//#include "bms.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"


// set this to 0 to stop
volatile int running = 1;
asset_manager* ess_man = nullptr;

void signal_handler (int sig)
{
    running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    if(ess_man)
    {
        ess_man->running = 0;
        ess_man->wakechan.put(-1);
    }
    signal(sig, SIG_DFL);
}


int sendTestMessage(fims *p_fims, int tnum)
{
    const char* method;
    const char* replyto=nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;

    switch (tnum) {
        case 1:
            {
                method = "set";
                replyto = "/test/foo";
                uri="/components/test_1";
                body="{\"var_set_one\":21}";
            }
            break;
        case 2:
            {
                method = "set";
                //replyto = "/test/foo";
                uri="/components/test_2";
                body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
            }
            break;
        case 3:
            {
                method = "set";
                replyto = "/test/foo_2";
                uri="/components/test_2";
                body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
            }
            break;
        case 4:
            {
                method = "set";
                replyto = "/test/foo_4";
                uri="/components/test_3";
                body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
            }
            break;
        case 5:
            {
                method = "get";
                replyto = "/test/foo_5";
                uri="/components/test_3";
                //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
            }
            break;
        case 6:
            {
                method = "get";
                replyto = "/test/foo_6";
                uri="/components/test_3/var_set_twox";
                //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
            }
            break;
        case 7:
            {
                method = "set";
                replyto = "/test/foo_7";
                uri="/assets/bms_1";
                body="{\"ctrlword1\":{\"value\":3}}";
            }
            break;
        case 8:
            {
                method = "set";
                replyto = "/test/foo_8";
                uri="/assets/bms_1";
                body="{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
            }
            break;
        case 9:
            {
                method = "set";
                replyto = "/test/foo_9";
                uri="/components/catl_ems_bms_rw";
                body="{\"ems_test_status\":{\"value\":\"Running\"}}";
            }
            break;
        default:
            break;
        }
    if(uri)
        p_fims->Send(method, uri, replyto, body);
    return 0;
}

//lot of notes here but we'll condense them 
// add a config uri or method 
// add an actions mode to the var
// onset will do the ctrlword translations.

//
// these are the ess commands from the hos_manager
//
// "ctrlword1[cfg]":[
//          0:0   { "field": "oncmd", "value": true },
//          0:1   { "field": "kacclosecmd", "value": true }
//          1:0   { "field": "offcmd", "value": true },
//          1:1   { "field": "kacopencmd", "value": true }
// "ctrlword2[cfg]":[
//          0:0   { "field": "kdcclosecmd", "value": true }
//          1:0   { "field": "kdcopencmd", "value": true }
// "ctrlword4[cfg]": [
//          0:0   { "field": "standbycmd", "value": true }
//          1:0   { "field": "oncmd", "value": true }
// we have control commands
// code (in go)
// // ContactorControl allows discrete control over DC and AC contactors
	// If not set, only Oncmd or Offcmd are needed
	
//  start_stop
// 
//  run_mode
//
//
// active_power_setpoint
// reactive_power_setpoint
// active_power_rising_slope (PCS)
// active_power_droop_slope (PCS)
// reactive_power_rising_slope (PCS)
// reactive_power_droop_slope (PCS)
//

// These are the ess responses
// bms_bus_voltage(array)
// bms_bus_current(array)
// ess_soc
// ess_soh
// ess_power
// operational_status (Standby Charging Discharging ( Fault))
// ess_current_chargeable_capacity   ( how much we can charge the thing)
// ess_current_dischargeable_capacity
// ess_circuit_breaker_control_word
// ess_circuit_breaker_status
// ess_maximum_charging_power_limit     
// ess_maximum_discharging_power_limit
// ess_alarms
// ess_warnings
//
// this is our map utils factory
VarMapUtils defvm;
VarMapUtils* vm = &defvm;

// this is a map of local variables as known to the asset 
varmap *amap;

int HandleHeartBeat(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
{
    int reload;
    double dval = 0.0;
    VarMapUtils * vm = am ->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleHeartBeat      = amap[__func__];  

    
    if(reload < 2)
    {
        //reload = 0;
        amap["HeartBeat"]            = vm->setLinkVal(vmap, aname, "/status",    "HeartBeat",               dval);
        amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            amap["HeartBeat"]->setVal(0);
        }
        amap[__func__]->setVal(2);

    }
    // get the reference to the variable 
    assetVar* hb = amap["HeartBeat"];
    //double ival;
    dval = hb->getdVal();
    dval++;
    if(dval > 255) dval = 0;
    if(1)printf("HeartBeat %s val %f ", aname, dval);

    hb->setVal(dval);
    dval = hb->getdVal();
    if(1)printf("HeartBeat val after set %f\n", dval);

    vm->sendAssetVar(hb, p_fims);
    return dval;
}

// This is the pcs handle power function.

//#include "../src/pcs_functions.cpp"
// we have to make sure BMS has done its job first so run this at LEVEL3
int HandlePower(varsmap &vmap, varmap &amap, const char * aname, fims* p_fims, asset_manager *am)
{
    printf("Handling power: %s\n", aname);
    double dval;
    double Ppu;
    double Pcmd_dbl=0.0;
    int rc = 0;
    int reload;
    int ival;
    //printf("%s >> %s --- Running\n", __func__, aname);
    //        amap["HandleHeartBeat"]->setVal(2);

    VarMapUtils * vm = am ->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandlePower      = amap[__func__];  

    
    if(reload < 2)
    {
        amap["ActivePowerSetpoint"]     = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerSetpoint", dval);
        amap["ActivePowerDeadband"]     = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDeadband", dval);
        amap["maxChargePower"]          = vm->setLinkVal(vmap, "bms", "/status", "maxChargePower", dval);
        amap["maxDischargePower"]       = vm->setLinkVal(vmap, "bms", "/status", "maxDischargePower", dval);
        amap["ActivePowerCmd"]          = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerCmd", dval);
        amap["numActiveBms"]            = vm->setLinkVal(vmap, "bms", "/status", "numActiveBms", ival);
        if(reload == 0)
        {
            amap["ActivePowerSetpoint"]->setVal(0.0);
            amap["maxChargePower"]->setVal(0.0);
            amap["maxDischargePower"]->setVal(0.0);
        }
        reload = 2;
        amap[__func__]->setVal(reload);
    }
    assetVar * Pset = amap["ActivePowerSetpoint"];
    assetVar * PmaxC = amap["maxChargePower"];
    assetVar * PmaxD = amap["maxDischargePower"]; 
    assetVar * Pcmd = amap["ActivePowerCmd"];
    assetVar * numActBms = amap["numActiveBms"];

    Ppu = Pset->getdVal() / numActBms->getiVal();
    printf("PPU: %f, numActBms: %d ptr: %p\n", Ppu, numActBms->getiVal(), (void*)&amap);
    double dval1,dval2;
     
    if (Pset->valueChangedReset() 
     | PmaxC->valueChangedReset() 
     | PmaxD->valueChangedReset() 
     | numActBms->valueChangedReset())
    {
        if (Ppu > 0)
            Pcmd_dbl = Ppu > PmaxD->getdVal() ? PmaxD->getdVal()      : Ppu;
        else
            Pcmd_dbl = Ppu * -1 > PmaxC->getdVal() ? PmaxC->getdVal() * -1 : Ppu;
        Pcmd->setVal((double)(Pcmd_dbl * numActBms->getiVal()));
        rc++;
    }
    std::cout << "Pset: " << Pset->getdVal() << " PmaxC: " << PmaxC->getdVal() <<  " Pcmd: " << Pcmd->getdVal() << std::endl;
    return rc;   
}

int HandleBMSChargeL1(varsmap &vmap, varmap &amap, const char*aname, fims* p_fims, asset* am)
{
    double dval;
    int ival = 0;
    int rc = 0;
    int reload;
    VarMapUtils * vm = am ->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);

    assetVar* HandleBMSChargeL1      = amap[__func__];  
    printf("%s >> %s --- Running\n", __func__, aname);

    if(reload < 2)
    {
        amap["numActiveBms"]               = vm->setLinkVal(vmap, "bms", "/status", "numActiveBms", ival);
        amap["maxChargePower"]             = vm->setLinkVal(vmap, "bms", "/status", "maxChargePower", dval);
        amap["maxDischargePower"]          = vm->setLinkVal(vmap, "bms", "/status", "maxDischargePower", dval);
        reload = 2;
        amap[__func__]->setVal(reload);
        
    }
    
    assetVar * PmaxC     = amap["maxChargePower"];
    assetVar * PmaxD     = amap["maxDischargePower"];
    assetVar * numActBms = amap["numActiveBms"];
    
    ival = 0; numActBms->setVal(ival);
    printf("%s >> %s --- BMS Manager: max C [%f] max D [%f]\n", __func__, aname, PmaxC->getdVal(), PmaxD->getdVal());
    return rc;   

}

// run by each BMS asset

int HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char*aname, fims* p_fims, asset * am)
{
    double dval;
    double dval1;
    double dval2;
    int ival = 0;
    int rc = 0;
    int reload;
    VarMapUtils * vm = am ->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);

    assetVar* HandleBMSChargeL2      = amap[__func__];  
    printf("%s >> %s --- Running\n", __func__, aname);
    if(reload < 2)
    {
        amap["bmsMaxChargePower"]          = vm->setLinkVal(vmap, aname, "/variables", "bms_max_p_charge", dval);
        amap["bmsMaxDischargePower"]       = vm->setLinkVal(vmap, aname, "/variables", "bms_max_p_discharge", dval);
        amap["bmsStatus"]                  = vm->setLinkVal(vmap, aname, "/variables", "bms_status", ival);
        amap["maxChargePower"]             = vm->setLinkVal(vmap, "bms", "/status", "maxChargePower", dval);
        amap["maxDischargePower"]          = vm->setLinkVal(vmap, "bms", "/status", "maxDischargePower", dval);
        amap["numActiveBms"]               = vm->setLinkVal(vmap, "bms", "/status", "numActiveBms", ival);
        reload = 2;
        amap[__func__]->setVal(reload);
    }
    assetVar * PmaxC      = amap["maxChargePower"];
    assetVar * PmaxD      = amap["maxDischargePower"];
    assetVar * bmsPmaxC   = amap["bmsMaxChargePower"];
    assetVar * bmsPmaxD   = amap["bmsMaxDischargePower"];
    assetVar * numActBms  = amap["numActiveBms"];
    assetVar * bmsStatus  = amap["bmsStatus"];

    ival = bmsStatus->getiVal();
    printf("%s %s ival %d \n", __func__, aname, ival);
    if(ival == 0) printf("%s initializing\n", aname);
    else if (ival == 5) printf("%s faulted\n", aname);
    else
    {
        if (numActBms->getiVal() == 0)
        {
            PmaxC->setVal(bmsPmaxC->getdVal());
            PmaxD->setVal(bmsPmaxD->getdVal());
        }
        else {
            if ((dval = bmsPmaxC->getdVal()) < PmaxC->getdVal()) PmaxC->setVal(dval);
            if ((dval = bmsPmaxD->getdVal() )< PmaxD->getdVal()) PmaxD->setVal(dval);
        }
        ival = 1; numActBms->addVal(ival);
    }

    printf("%s --- Active BMS's: [%d] max C [%f] max D [%f]\n", aname, numActBms->getiVal(), PmaxC->getdVal(), PmaxD->getdVal());

    return rc;

}

// her we handle the incoming commands
int HandleCmd(varsmap &vmap, varmap &amap, fims* p_fims)
{
    int rc = 0;
    // Turn on if conditions allow it
    bool fval = false;
    //int ival;
	// if e.Oncmd && (!e.On || e.Standby) && e.AcContactor && e.DcContactor {
	// 	e.On = true
	// 	e.Oncmd = false
	// 	e.Standby = false
	// } else if e.On && e.Offcmd {
	// 	e.On = false
	// 	e.Offcmd = false
	// 	e.Standby = false
	// }
	// if e.On && e.StandbyCmd {
	// 	e.Standby = true
	// 	e.StandbyCmd = false
	// }
    bool OffCmdval = amap["OffCmd"]->getdVal();
    //bool Offval = amap["Off"]->getdVal();

    bool OnCmdval = amap["OnCmd"]->getdVal();
    bool Onval = amap["On"]->getdVal();
    bool Standbyval = amap["Standby"]->getdVal();

    bool AcContactorval = amap["AcContactor"]->getdVal();
    bool DcContactorval = amap["DcContactor"]->getdVal();

    if( OnCmdval && (!Onval || Standbyval) && AcContactorval && DcContactorval)
    {
        rc++;
        amap["On"]->setVal(true);
        // Send On to device
        amap["OnCmd"]->setVal(false);
        amap["Standby"]->setVal(false);
    }
    else if (Onval && OffCmdval)
    {
        rc++;
        amap["On"]->setVal(false);
        amap["Off"]->setVal(true);
        // Send Off to device
        amap["Offcmd"]->setVal(false);
        // Send Off to device
        amap["Standby"]->setVal(false);
    }
    if (amap["On"]->getdVal() && amap["StandbyCmd"]->getdVal())
    {
        rc++;
        amap["Standby"]->setVal(true);
        // send standby to device
        amap["StandbyCmd"]->setVal(false);
    }
    //TODO send stuff to FIMS
    return rc;
}
// TODO add timeout


// we'll move the variable set up here and loose  the othe setup code.
void run_ems_init(asset_manager*am)
{
    FPS_ERROR_PRINT(">>>>>>>>>ESS>>>>>>>>>>>%s running for ESS Manager\n",__func__);
    am->vm->setTime();
    double dval = 0.0;
    bool bval = true;
     if(am->reload < 2)
    {
        //reload = 0;
        am->amap["HandleHeartBeat"]      = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/config",    "HandleHeartBeat",          am->reload);
        am->amap["HeartBeat"]            = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status",    "HeartBeat",                dval);

        am->amap["AcContactor"]          = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status",     "AcContactor",              bval);
        am->amap["AcContactorCloseCmd"]  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",   "AcContactorCloseCmd",      bval);
        am->amap["AcContactorOpenCmd"]   = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",   "AcContactorOpenCmd",       bval);

        am->amap["DcContactor"]          = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status",     "DcContactor",              bval);
        am->amap["DcContactorCloseCmd"]  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",   "DcContactorCloseCmd",      bval);
        am->amap["DcContactorOpenCmd"]   = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",   "DcContactorOpenCmd",       bval);
        am->amap["OnCmd"]                = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",    "OnCmd",                   bval);
        am->amap["On"]                   = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status",      "On",                      bval);

        am->amap["OffCmd"]               = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",    "OffCmd",                  bval);
        am->amap["Off"]                  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status",      "Off",                     bval);

        am->amap["StandbyCmd"]           = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",    "StandbyCmd",              bval);
        am->amap["Standby"]              = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status",      "Standby",                 bval);

        am->amap["ActivePower"]          = am->vm->setLinkVal(*am->vmap, "pcs_mb_input",   "/components",  "ActivePower",             dval);
        am->amap["ActivePowerSetPoint"]  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",    "ActivePowerSetPoint",     dval);
        am->amap["ActivePowerDeadBand"]  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params",      "ActivePowerDeadband",     dval);

        am->amap["ReactivePower"]          = am->vm->setLinkVal(*am->vmap, "pcs_mb_input",   "/components", "ReactivePower",           dval);
        am->amap["ReactivePowerSetPoint"]  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",   "ReactivePowerSetPoint",   dval);
        am->amap["ReactivePowerDeadBand"]  = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params",     "ReactivePowerDeadband",   dval);

        am->amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if(am->reload == 0) // complete restart 
        {
            am->amap["HeartBeat"]->setVal(0);
        }
        am->reload = 2;
    }

}

void run_bms_init(asset_manager*am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager\n",__func__);
}

// We have presently have two wakeup messages sent  
bool run_bms_wakeup(asset_manager*am, int wakeup)
{
    char * item3;
    fims_message * msg;
    // std::cout << " bms >> wakeup ival "<< wakeup << "\n";
    // FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n",__func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);

    if (am->reload!= 2)
      am->run_init(am);

    FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset Manager wval %d \n",__func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    if(wakeup == 0)
    {
        //std::cout << am->name <<">> MANAGER LOOP  process another channel\n";
        //break;
    }
    // now process all the events

    //Do the manager first
    if(wakeup == WAKE_LEVEL1)
    {
        
        // std::cout << am->name <<" >>get_state\n";
        // HandleHeartBeat(*am->vmap, am->amap, "ess", am->p_fims);
        // std::cout << am->name << "  >>process_cmds\n";
        //HandleCmd(*am->vmap, am->amap, am->p_fims);
        // std::cout << am->name << "  >>process_power\n";
        HandleBMSChargeL1(*am->vmap, am->amap, "bms", am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up 
    
        
        // now get all the managed assets , wake those up too

    }

    if(wakeup == WAKE_LEVEL3)
    {
        
        // std::cout << am->name <<" >>HanlePower\n";
        HandlePower(*am->vmap, am->amap, "ess", am->p_fims);
        // std::cout << am->name << "  >>process_cmds\n";
        //HandleCmd(*am->vmap, am->amap, am->p_fims);
        // std::cout << am->name << "  >>process_power\n";
        //HandlePower(*am->vmap, am->amap, am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up 
    
        
        // now get all the managed assets , wake those up too

    }

    if(wakeup == WAKE_LEVEL_PUB)
    {
        // // std::cout << am->name << " >>publish status\n";
        // //ess_manager * ess = (ess_manager*)am;
        // //asset_manager* bm2 = am->getManAsset("bms");

        // // note this assumes that the bms is running threaded
        // // let the builtin functions do this bms ma not be running a manager thread.
        // //bm2->wakechan.put(wakeup);
        // //publ
        // const char *pname ="Unknown";
        // cJSON* cjbm = am->getConfig();
        // // printf("%s >>>>>>>Publish cjbm %p\n", __func__, (void *)cjbm);
        // char* res = cJSON_Print(cjbm);
        // // printf("%s >>>>>>>Publish fims  %p\n", __func__, (void *)am->p_fims);
        // cJSON_Delete(cjbm);
        // // TODO fix name
        // am->p_fims->Send("pub", pname, nullptr, res);
        // free((void *)res) ;

        // //break;
    }
    
    if(am->msgchan.get(item3,false)) {
        //sendTestMessage(p_fims, tnum);
        // std::cout << am->name << "  >>item3-> data  "<< item3 << " tnum :"<< am->tnum<<"\n";
        am->tnum++;
        //sendTestMessage(p_fims, tnum);
        // std::cout << am->name << "  >>item3-> data  "<< item3 << " tnum :"<< am->tnum<<"\n";
        am->tnum++;
        free((void *) item3);
    }

    // this gets in fims messages
    if(am->fimschan.get(msg, false)) {
        std::cout << am->name << "  >> fims_msg uri "<< msg->uri  << "\n";
        // we need to collect responses
        cJSON *cj = nullptr;
        // either here of the bms instance
        //bms_man->vmap  
        am->vm->processFims(*am->vmap,  msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if(tmp)
            {
                am->p_fims->Send("set",msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);

        //free((void *) item3);
    }
    for (auto ix : am->assetManMap)
    {
        if(0)FPS_ERROR_PRINT("%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  running for Asset Manager [%s] \n",__func__, am->name.c_str(), ix.first.c_str());
                        
        asset_manager* am2 = ix.second;  //am->getManAsset("bms");
        if (am2->run_wakeup)
        {
            if(0)FPS_ERROR_PRINT("%s >>>>>>>>> %s Manager Loop Wakeup >>>>>>>>>>> running for Asset Manager [%s] \n",__func__, am->name.c_str(), ix.first.c_str());
            // first trigger the wakeup there is no thread in the lower leel managers
                //am2->wakechan.put(wakeup);
            am2->run_wakeup(am2, wakeup);
        }
        else
        {
            FPS_ERROR_PRINT("%s >>>>>>>>> %s Manager Loop NO Wakeup >>>>>>>>>>> running for Asset Manager [%s] \n",__func__, am->name.c_str(), ix.first.c_str());
        }
                    
        //am2->wakechan.put(wakeup);
    }

    // now do the assets
    for (auto ix : am->assetMap)
    {
        asset* ass = ix.second ; //am->getManAsset("bms");
        if(0)FPS_ERROR_PRINT("%s >>>>>>>>>%s ASSETS >>>>>>>>>>> running for Asset [%s] \n",__func__, am->name.c_str(), ix.first.c_str());
        // TODO no need to set up the assets with channels
        //TODO ass->wakechan.put(wakeup);
        if (ass->run_wakeup)
            ass->run_wakeup(ass, wakeup);
    }

    if(wakeup == -1)
    {
        // quit
        FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, am->name.c_str());
        running = 0;
        //break;//
        return false;
    }


    return true;
}


bool run_bms_asset_wakeup(asset*am, int wakeup)
{
    FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n",__func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    //FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if(wakeup == WAKE_LEVEL1)
    {
        // HandleHeartBeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
        //HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char*aname, fims* p_fims)
    }
    else if(wakeup == WAKE_LEVEL2)
    {
        HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
    }

    return true;
}

// this one assumes channels are working..
bool run_ems_wakeup(asset_manager*am, int wakeup)
{
    // FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n",__func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // then service the other channels 
    //
    char * item3;
    fims_message * msg;
    // a wakeup of 0 means service the others
    // a wakeup if 1 means process the asset
    // a wakeup of 2 means pub the asset
    if(wakeup == 0)
    {
        //td::cout << " BMS >> process another channel\n";
        //break;
    }

    if(wakeup == 1)
    {
        am->vm->setTime();
        //std::cout << " BMS >> get_state\n";
        // HandleHeartBeat(*am->vmap, am->amap, "bms", am->p_fims);
        //HandleHeartBeat(vmap, *amap, p_fims);
        //std::cout << " BMS >>process_cmds\n";
        //HandleCmd(vmap, *amap, p_fims);
        //std::cout << " BMS >> process_power\n";
        //HandlePower(vmap, *amap, p_fims);
    }

    if(wakeup == 2)
    {
        std::cout << " TODO BMS >> publish status\n";

        // publ
        // for each instance
        // cJSON* cjbm = bm->getConfig();
        // char* res = cJSON_Print(cjbm);
        // //printf("Publish \n%s\n", res);
        // cJSON_Delete(cjbm);
        // p_fims->Send("pub", "/status/bms", nullptr, res);
        // free((void *)res) ;

        //break;
    }

    if(wakeup == -1)
    {
        // quit
        am->running = 0;
        return false;

    }

    // this is the testmessage
    if(am->msgchan.get(item3,false)) 
    {
        //sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  "<< item3 << " tnum :"<< am->tnum<<"\n";
        am->tnum++;
        //sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  "<< item3 << " tnum :"<< am->tnum<<"\n";
        am->tnum++;
        free((void *) item3);
    }

    // this gets in fims messages
    if(am->fimschan.get(msg, false)) 
    {
        std::cout << " BMS >>fims_msg uri "<< msg->uri  << "\n";
        // we need to collect responses
        cJSON *cj = nullptr;
        // either here of the bms instance
        //bms_man->vmap  
        am->vm->processFims(*am->vmap,  msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if(tmp)
            {
                am->p_fims->Send("set",msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);
        //free((void *) item3);
    }
    return true;
}
// the main ess_controller start up
int main(int argc, char *argv[])
{
    
    varsmap vmap;

 // TODO make this an asset manager
    ess_man = new asset_manager("ess_controller");

    ess_man->vmap = &vmap;  //??
    ess_man->vm = vm;  //??

    ess_man->run_init = run_ems_init;
    ess_man->run_wakeup = run_ems_wakeup;

    ess_man->setVmap(&vmap);
    ess_man->running = 1;

    amap = ess_man->getAmap();  //may not need this 

    // TODO this may be redundant
    //ess_man->setVmap(&vmap);

    //right this thing is ready to go
    // Just for kicks read in the config file and see if it had our links in it
    // this should be the case after the first run.
    printf(" Getting initial configuration\n");
    
    const char* cfgname = "configs/test_ess_config.json";
    //ess_man->configure(vmap, cfgname , "ess");
    //varsmap* vmp = ess_man->getVmap();
    vm->configure_vmap(vmap, cfgname);
    //ess_man->configure(cfgname , nullptr);

    {
        // this a test for our config.
        cJSON *cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        const char* fname =  "configs/ess_1_base_ess.json";
        vm->write_cjson(fname, cjbm);
        //cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        printf("Maps (possibly) with links at beginning  cjbm %p \n%s\n <<< done\n", (void *)cjbm, res);
        free((void *)res) ;
        cJSON_Delete(cjbm);
    }
    // we should be able to set up the amap table from the links
    //TODO Fiish this 
    vm->CheckLinks(vmap, *amap, "/links/ess");
    // bit ugly at the moment .. its a copy of the aset config    
 
    // no dont do this now
    //ess_man->setup(vmap, "/links/ess");
    ess_man->cfgwrite("configs/ess_after_links.json");

   // this is just a check
    // vmap["/links/bms_1"]
    auto ix = vmap.find("/links/ess");
    if (ix != vmap.end()) 
    {

        // if this works no need to run the init function below
        printf(" We found our links , we should be able to set up our link amap\n");
        for (auto iy : ix->second)
        {
            if(iy.second->type == assetVar::ASTRING)
            {
                printf(" lets link [%s] to [%s]\n"
                        , iy.first.c_str()
                        , iy.second->aVal->valuestring
                        );
                        // for example lets link [AcContactor] to the var defined for [/status/bms_1:AcContactor]
                        // amap[iy.first] = vm->getVar (vmap, y.second->aVal->valuestring);//  getVar(varsmap &vmap, const char* comp, const char* var=nullptr)
                   //amap["AcContactor"]               = vm->linkVal(vmap, link, "AcContactor",            fval);
            }
        }
    } 

    // nah not working yet   
    //ess_man->cfgwrite("configs/justlinks_ess.json");

    printf("ess test OK so far\n");
    // this is a low level configure with no subs.
    //ess_man->configure("configs/test_ess.json");
    // todo mo
    cfgname = "configs/test_ess.json";// TODO move this into asset manager assconfig
    ess_man->configure(&vmap, cfgname , "ess");
    //ess_man->configure(cfgname , (char *)"ess");
        // no worries at this stage

    // this sets up the amap for bss_running vars not needed if the links are in the config....
    //ess_man->initLinks();


    //ess_man->cfgwrite("configs/cfg_and_links_ess.json");

    // todo overwrite config with the last running config.
    //bm->configure("data/last_bss_1.json");
    // set up an outgoing fims connection. 
    fims* p_fims = new fims;

    //p_fims->Connect((char *)"fimsEssTest");
    bool connectOK = p_fims->Connect((char *)"FimsEssTest");
    if (!connectOK)
    {
       FPS_ERROR_PRINT("%s >> unable to connect to fims server. All is lost.\n",__func__ ); 
       running = 0;
    }
    ess_man->p_fims = p_fims;
    ess_man->running = 1;

    // eeded for the controller, manager and possibly assets
    ess_man->setupChannels();

    // the timer generates  channel wakes every 500 mS
    ess_man->run_timer(100);


// run the message loop eery 1.5 seconds
//pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running, 1500,  &pcs_man->wakechan);

    // the message  generates test message  channel wakes every 1500 mS
    // ess_man->run_message( 1500);

    const char* subs[] = {
            "/components", 
            "/assets",
            "/params",
            "/status",
            "/controls",
            "/test",
            "/variables"
            };
    int sublen = sizeof(subs)/ sizeof(subs[0]);

    // the fims system will get pubs and create a varsMap for  the items.
    // TODO restrict fims to known components

    ess_man->run_fims(1500, (char **)subs, "essMan",sublen);

    // the manager runs the channel receiver thread
    ess_man->run_manager(p_fims); 

    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* bms_man = nullptr;
    auto ixa = vmap.find("/assets/ess");
    if (ixa != vmap.end()) 
    {
        // if this works no need to run the init function below
        printf(" ESS >>We found our assets, we should be able to set up our system\n");
        for (auto iy : ixa->second)
        {
            if(iy.second->type == assetVar::ASTRING)
            {
                printf(" lets run assets for  [%s] from [%s]\n"
                        , iy.first.c_str()
                        , iy.second->aVal->valuestring
                        );
                const char* fname = iy.second->aVal->valuestring;
                if (iy.first == "bms") 
                {
                    printf(" setting up a bms manager\n");
                    
                    bms_man = new asset_manager("bms_man");
                    bms_man->run_init = run_bms_init;
                    bms_man->run_wakeup = run_bms_wakeup;
                    bms_man->p_fims = p_fims;
 

                    bms_man->setVmap(&vmap);
                    // now get the bms_manager to configure itsself
                    bms_man->configure(&vmap, fname, "bms",run_bms_asset_wakeup);
                    // add it to the assetList
                    ess_man->addManAsset(bms_man, "bms");

                    printf(" done setting up a bms manager varsmap must be fun now\n");
                    // we should be able to do things like get status from the bms_manager.
                    // first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset("bms");
                    if(bm2)
                    {
                        printf(" @@@@@@@  found bms asset manager with %d bms assets\n", bm2->getNumAssets());
                    }
                    bms_man->running = 1;
                    bms_man->setupChannels();
                    //bms_man->run_timer(500);
                    bms_man->run_message(500);


                    //bms_man->run_manager(p_fims); 
                    //bms_man->man_wakechan.put(2);
                    // const char* subs[] = {
                    //     "/components", 
                    //     "/assets/bms_1",
                    //     "/controls/bms_1",
                    //     "/test/bms",
                    //     "/assets/bms",
                    //     "/controls/bms"
                    //         };
                    // bms_man->run_fims(1500, (char**)subs, "bmsMan");

                    // TODO we have to run the bms_manager to get it talking to the varsmap
                    //first lets sort out the ess_manager's vma problem 
                 }
            }
        }
    } 
    {
        const char* fname =  "configs/ess_4_cfg_links_assets_ess.json";
        // this a test for our config with links
        cJSON *cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        vm->write_cjson(fname, cjbm);

        //cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        //printf("Maps (should be ) with links and assets  cjbm %p \n%s\n <<< done\n", (void *)cjbm, res);
        free((void *)res) ;
        cJSON_Delete(cjbm);
    
    }
//    ess_man->vmap = &vmap;
    if(ess_man->vmap)
    {
        cJSON *cj = vm->getMapsCj(*ess_man->vmap);
        char *res = cJSON_Print(cj);
        printf("ESS >>Maps before run \n%s\n", res);
        free((void *)res) ;
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Maps before runskipped\n");

    }
    while(ess_man->running)
    {
        poll(nullptr,0,100);
    }
    printf("ESS >> Shutting down\n");
    bms_man->running = 0; 


    ess_man->t_data.cthread.join();
    std::cout << "ESS >>t_data all done " << "\n";
    ess_man->m_data.cthread.join();
    std::cout << "ESS >>m_data all done " << "\n";
    ess_man->f_data.cthread.join();
    std::cout << "ESS >>f_data all done " << "\n";
    

    if(ess_man->vmap)
    {
        cJSON *cj = vm->getMapsCj(*ess_man->vmap);
        char *res = cJSON_Print(cj);
        printf("ESS >>Maps at end \n%s\n", res);
        free((void *)res) ;
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Maps at end skipped\n");

    }
    

    //monitor M;
    //M.configure("configs/monitor.json");
    
    delete ess_man;

    return 0;

}
