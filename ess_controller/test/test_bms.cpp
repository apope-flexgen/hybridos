/*
 * bms test , opens up a bms instance and runs the 
 * bms controller in a thread with its own fims interface
 * uses a timer to coordinate service and publish times.
 * adding a monior feature 
 *  we probably want a pub timer thread and a process timer thread  
 * the test messages can act as a sort of simulator
 * IE send the start command and the test message will reflect the status 
 */


#include "asset.h"
//#include "channel.h"
//#include "monitor.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

//#include "bms.h"

int bms_debug = 0;
// set this to 0 to stop
volatile int running = 1;
asset_manager* bms_man = nullptr;

void signal_handler (int sig)
{
    running = 0;
    if(bms_man)
        bms_man->running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}


// both the asset manager and the asset has a varsmap name space

int sendTestMessage(fims *p_fims, int tnum)
{
    const char* method;
    const char* replyto=nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;
    // turn it off for initial manual testing

    switch (tnum) {
        case 1:
            {
                method = "get";
                replyto = "/test/bms";
                uri="/params/bms_1";
                //body="{\"var_set_one\":21}";
            }
            break;
        // case 2:
        //     {
        //         method = "get";
        //         replyto = "/test/bms";
        //         uri="/params/bms_1/max_load_threshold";
        //         //body="{\"var_set_one\":21}";
        //     }
        //     break;
        // case 3:
        //     {
        //         method = "set";
        //         replyto = "/test/bms";
        //         uri="/params/bms_1";
        //         body="{\"max_load_threshold\":21000}";
        //     }
        //     break;
        // case 4:
        //     {
        //         method = "get";
        //         replyto = "/test/bms";
        //         uri="/params/bms_1/max_load_threshold";
        //         //body="{\"var_set_one\":21}";
        //     }
        //     break;

        // case 2:
        //     {
        //         method = "set";
        //         //replyto = "/test/foo";
        //         uri="/components/test_2";
        //         body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        //     }
        //     break;
        // case 3:
        //     {
        //         method = "set";
        //         replyto = "/test/foo_2";
        //         uri="/components/test_2";
        //         body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        //     }
        //     break;
        // case 4:
        //     {
        //         method = "set";
        //         replyto = "/test/foo_4";
        //         uri="/components/test_3";
        //         body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        //     }
        //     break;
        // case 5:
        //     {
        //         method = "get";
        //         replyto = "/test/foo_5";
        //         uri="/components/test_3";
        //         //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        //     }
        //     break;
        // case 6:
        //     {
        //         method = "get";
        //         replyto = "/test/foo_6";
        //         uri="/components/test_3/var_set_twox";
        //         //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        //     }
        //     break;
        // case 7:
        //     {
        //         method = "set";
        //         replyto = "/test/foo_7";
        //         uri="/assets/bms_1";
        //         body="{\"ctrlword1\":{\"value\":3}}";
        //     }
        //     break;
        // case 8:
        //     {
        //         method = "set";
        //         replyto = "/test/foo_8";
        //         uri="/assets/bms_1";
        //         body="{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
        //     }
        //     break;
        // case 9:
        //     {
        //         method = "set";
        //         replyto = "/test/foo_9";
        //         uri="/components/catl_ems_bms_rw";
        //         body="{\"ems_test_status\":{\"value\":\"Running\"}}";
        //     }
        //     break;
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
// these are the bms commands from the ess_manager
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


// if !e.ContactorControl {
	// 	if !e.On && e.Oncmd {
	// 		e.AcContactorCloseCmd, e.DcContactorCloseCmd = true, true
	// 	} else if e.On && e.Offcmd {
	// 		e.AcContactorOpenCmd, e.DcContactorOpenCmd = true, true
	// 	}
	// }
// this is the simulation
	// if !e.AcContactor && e.AcContactorCloseCmd {
	// 	e.AcContactor, e.AcContactorCloseCmd = true, false
	// } else if e.AcContactor && e.AcContactorOpenCmd {
	// 	e.AcContactor, e.AcContactorOpenCmd = false, false
	// 	e.Offcmd = true
	// }

	// if !e.DcContactor && e.DcContactorCloseCmd {
	// 	e.DcContactor, e.DcContactorCloseCmd = true, false
	// 	e.RacksInService = e.Racks
	// } else if e.DcContactor && e.DcContactorOpenCmd {
	// 	e.DcContactor, e.DcContactorOpenCmd = false, false
	// 	e.RacksInService = 0
	// 	e.Offcmd = true

	// }
//    more simulation
// Turn on if conditions allow it
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


// this is our map utils factory
VarMapUtils vm;

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
// char buf[1024];
//         const char* value = getdefLink(name, base, var, buf, sizeof(buf));
//         return setVal2(vmap, comp, var, value);
// when this is run inside the asset manager object aname will be given
// however the asset manager may want to tune this on behalf of one of the managed assets
// inwhich case the aname will be bms_1, bms_2 or what ever.
int HandleLoadRequest(varsmap &vmap, varmap &amap, const char *aname, fims* p_fims, asset_manager * am)
{
    double dval;
    bool  bval;
    char* cval = (char *) "Dummy"; 
    int reload;

    // setLinkVal 
    // 1/ looks for /links/<aname> and if so picks up the assetVar by reference to the link valuestring
    // for example /links/bms:maxLoadRequest -> /params/bms:maxLoadRequest this is a global for all bms units
    //             /links/bms_1:LoadSetpoint -> /controls/bms_1:LoadSetpoint this is a setpoint for unit bms_1 
    //             /links/ess:EStop -> /controls/ess:EStop this is a global command
    // setLinkval will look for an established link from the config file
    // if not found it will create a link called /links/<aname>  to the default agroup (/params/<aname>:<aval> etc)
    //   it will then look for the "linked to" variable for example /components/catl_bms_ess_01:bms_soc
    //     it will create  this variable ( with the given type) if needed
    //  thus the loop is closed we have a link and an associated variable. creates for us or predefined.
    // to force the links to be reevaluated then /controls/<anmae>:FunName should be set to 1
    // to cause a complete reset set reload to 0;
    //
    VarMapUtils * vm = am ->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleLoadRequest      = amap[__func__];  

    
    if(reload < 2)
    {
        //reload = 0;
        amap["maxLoadRequest"]         = vm.setLinkVal(vmap, "bms", "/params",    "maxLoadRequest",               dval);
        amap["LoadRequest"]            = vm.setLinkVal(vmap, "bms", "/controls",  "LoadRequest",                  dval);
        amap["LoadRequestDeadband"]    = vm.setLinkVal(vmap, "bms", "/params",    "LoadRequestDeadband",          dval);
        amap["lastLoadRequest"]        = vm.setLinkVal(vmap, "bms", "/controls",  "lastLoadRequest",              dval);
        amap["LoadSetpoint"]           = vm.setLinkVal(vmap, aname, "/controls",  "LoadSetpoint",                 dval);
        amap["LoadState"]              = vm.setLinkVal(vmap, aname, "/status",    "LoadState",                    cval);
        amap["StateResetCmd"]          = vm.setLinkVal(vmap, "bms", "/controls",  "StateResetCmd",                bval);
        amap["lastStateResetCmd"]      = vm.setLinkVal(vmap, "bms", "/status",    "lastStateResetCmd",            bval);
        //amap["EStop"]                  = vm.setLinkVal(vmap, "ess", "/controls",  "EStop",                        bval);

        amap[__func__]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            amap["LoadRequestDeadband"]->setVal(25.0);
            amap["lastLoadRequest"]->setVal(25.0);
            amap["LoadRequest"]->setVal(26.0);
            amap["StateResetCmd"]->setVal(false);
            amap["lastStateResetCmd"]->setVal(false);

            amap["LoadState"]->setVal("Init");
        }
    }

    assetVar* maxLoadRequest      = amap["maxLoadRequest"];  // all these will crash if the link vars are not set up correctly
    assetVar* LoadRequest         = amap["LoadRequest"];
    assetVar* LoadRequestDeadband = amap["LoadRequestDeadband"];
    assetVar* lastLoadRequest     = amap["lastLoadRequest"];
    assetVar* LoadSetpoint        = amap["LoadSetpoint"];
    assetVar* LoadState           = amap["LoadState"];
    assetVar* StateResetCmd       = amap["StateResetCmd"];
    assetVar* lastStateResetCmd   = amap["lastStateResetCmd"];
    //assetVar* EStop               = amap["EStop"];

    if(0)printf("%s >>>>> STATUS %s  (comp %s) %f to %f (deadband %f)\n"
                , __func__
                , LoadRequest->name.c_str()
                , LoadRequest->comp.c_str()
                , lastLoadRequest->getdVal()
                , LoadRequest->getdVal()
                , LoadRequestDeadband->getdVal()
                );

    if ( vm->valueChanged(LoadRequest, lastLoadRequest, LoadRequestDeadband ,dval, 0.0))
    {
        printf("%s >>>>> load value changed from %f to %f (deadband %f)\n"
                , __func__
                , lastLoadRequest->getdVal()
                , LoadRequest->getdVal()
                , LoadRequestDeadband->getdVal()
                );
        lastLoadRequest->setVal(LoadRequest->getdVal());
        if (LoadRequest->getdVal() < maxLoadRequest->getdVal())
        {
            LoadSetpoint->setVal(LoadRequest->getdVal());
            cval =  (char *)"Running" ;LoadState->setVal(cval);
        }
        else
        {
            LoadSetpoint->setVal(maxLoadRequest->getdVal());
            cval = (char *)"Limit"; LoadState->setVal(cval);
        }
        
    }

    if ( vm->valueChangednodb(LoadRequest, lastLoadRequest ,dval, 0.0))
    {
 
        if (abs(LoadRequest->getdVal()) < 2.0)
        {
            LoadSetpoint->setVal(LoadRequest->getdVal());
            cval =  (char *)"Standby" ;LoadState->setVal(cval);
        }
    }

    if ( vm->valueChangednodb(StateResetCmd, lastStateResetCmd, bval, 0.0))
    {
        lastStateResetCmd->setVal(StateResetCmd->getbVal());
        if(StateResetCmd->getbVal())
        {
            LoadSetpoint->setVal(0.0);
            cval =  (char *)"Reset" ;LoadState->setVal(cval);
        }
        else
        {
            cval =  (char *)"Standby" ;LoadState->setVal(cval);
        }
        
    }
    return 0;
}

int HandleCmd(varsmap &vmap, varmap &amap, const char *aname, fims* p_fims, asset_manager* am)
{
    int rc = 0;
    // Turn on if conditions allow it
    bool fval = false;
    //int ival;
    //double dval;
    //bool  bval;
    //char* cval = (char *) "Dummy"; 
    int reload;
    VarMapUtils * vm = am ->vm;
    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleCmd      = amap[__func__];  

    // setLinkVal 
    // 1/ looks for /links/<aname> and if so picks up the assetVar by reference to the link valuestring
    // for example /links/bms:maxLoadRequest -> /params/bms:maxLoadRequest this is a global for all bms units
    //             /links/bms_1:LoadSetpoint -> /controls/bms_1:LoadSetpoint this is a setpoint for unit bms_1 
    //             /links/ess:EStop -> /controls/ess:EStop this is a global command
    // setLinkval will look for an established link from the config file
    // if not found it will create a link called /links/<aname>  to the default agroup (/params/<aname>:<aval> etc)
    //   it will then look for the "linked to" variable for example /components/catl_bms_ess_01:bms_soc
    //     it will create  this variable ( with the given type) if needed
    //  thus the loop is closed we have a link and an associated variable. creates for us or predefined.
    // to force the links to be reevaluated then /controls/<anmae>:FunName should be set to 1
    // to cause a complete reset set reload to 0;
    //
    
    if(reload < 2)
    {
        //reload = 0;
        amap["On"]                 = vm.setLinkVal(vmap, "bms", "/controls",   "On",               fval);
        amap["OnCmd"]              = vm.setLinkVal(vmap, "bms", "/controls",   "OnCmd",            fval);
        amap["Off"]                = vm.setLinkVal(vmap, "bms", "/controls",   "Off",              fval);
        amap["OffCmd"]             = vm.setLinkVal(vmap, "bms", "/controls",   "OffCmd",           fval);
        amap["Standby"]            = vm.setLinkVal(vmap, "bms", "/controls",   "Standby",          fval);
        amap["StandbyCmd"]         = vm.setLinkVal(vmap, "bms", "/controls",   "StandbyCmd",       fval);
        amap["AcContactor"]        = vm.setLinkVal(vmap, "bms", "/controls",   "AcContactor",      fval);
        amap["DcContactor"]        = vm.setLinkVal(vmap, "bms", "/controls",   "DcContactor",      fval);
        //amap["EStop"]              = vm.setLinkVal(vmap, "ess", "/controls",   "EStop",            bval);

        HandleCmd->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            amap["On"]->setVal(true);
            amap["OnCmd"]->setVal(false);
            amap["Off"]->setVal(true);
            amap["OffCmd"]->setVal(false);
            amap["Standby"]->setVal(false);
            amap["StandbyCmd"]->setVal(false);
            amap["AcContactor"]->setVal(false);
            amap["DcContactor"]->setVal(false);
        }
    }

    assetVar* Off                   = amap["Off"];
    assetVar* OffCmd                = amap["OffCmd"];  // all these will crash if the link vars are not set up correctly
    assetVar* On                    = amap["On"];
    assetVar* OnCmd                 = amap["OnCmd"];
    assetVar* Standby               = amap["Standby"];
    assetVar* StandbyCmd            = amap["StandbyCmd"];
    assetVar* AcContactor           = amap["AcContactor"];
    assetVar* DcContactor           = amap["DcContactor"];
    //assetVar* EStop                 = amap["EStop"];

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
    bool OffCmdval = OffCmd->getbVal();
    //bool Offval    = Off->getbVal();

    bool OnCmdval  = OnCmd->getbVal();
    bool Onval     = On->getbVal();
    bool Standbyval = Standby->getbVal();

    bool AcContactorval = AcContactor->getbVal();
    bool DcContactorval = DcContactor->getbVal();

    if( OnCmdval && (!Onval || Standbyval) && AcContactorval && DcContactorval)
    {
        rc++;
        On->setVal(true);
        // Send On to device
        OnCmd->setVal(false);
        Standby->setVal(false);
    }
    else if (Onval && OffCmdval)
    {
        rc++;
        On->setVal(false);
        Off->setVal(true);
        // Send Off to device
        OffCmd->setVal(false);
        // Send Off to device
        Standby->setVal(false);
    }
    if (Onval && Standbyval)
    {
        rc++;
        Standby->setVal(true);
        // send standby to device
        StandbyCmd->setVal(false);
    }
    //TODO send stuff to FIMS
    return rc;
}


/*
 * // links will be to modbus client on the vmap (/status/modbus_client/mod_1? - don't know what this is)
 *  amap["CurrentFault"]               = vm.linkVal(vmap, link, "CurrentFault",            fval);
 *  amap["ActivePowerSetpoint"]        = vm.linkVal(vmap, link, "ActivePowerSetpoint",    fval);
 * }
 * 
 * assetVar * CF = amap["CurrentFault"];
 * assetVar * APSP = amap["ActivePowerSetpoint"];
 * 
 * read input {
 *  if(CF->getdVal()) {
 *      bool faultMode = true;
 *  }
 *  
 * }
 * 
 * output logic {
 *  if(faultMode) {
 *     // Do faultMode logic here:
 *      APSP->setVal(False); // Turn off active power set point
 *  } 
 * }
 * 
 */


int HandlePower(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims)
{

    double dval;
    int rc = 0;
    if ( vm.valueChanged(amap["ActivePowerSetpoint"], amap["lastActivePowerSetpoint"], amap["ActivePowerDeadband"] ,dval, 2.0))
    {
        amap["lastActivePowerSetpoint"]->setVal(  amap["ActivePowerSetpoint"]->getdVal());
        if (abs(amap["ActivePowerSetpoint"]->getdVal()) < amap["maxActivePower"]->getdVal())
        {
            amap["ActivePowerCmd"]->setVal(amap["ActivePowerSetpoint"]->getdVal());
        }
        else
        {
            amap["ActivePowerCmd"]->setVal((amap["ActivePowerSetpoint"]->getdVal()>0.0)?amap["maxActivePower"]->getdVal()
                                :-amap["maxActivePower"]->getdVal());
        }
        //vm.sendAssetVar(amap["ActivePowerCmd"], p_fims);
        rc++;
    }

    // another way to do this using local vars , prevents the dict lookup
    assetVar * RPSP = amap["ReactivePowerSetpoint"];
    assetVar * lastRPSP = amap["lastReactivePowerSetpoint"];
    assetVar * RPDB = amap["ReactivePowerDeadband"];
    assetVar * maxRP = amap["maxReactivePower"];
    assetVar * RPcmd = amap["ReactivePowerCmd"];

    if ( vm.valueChanged(RPSP, lastRPSP, RPDB ,dval, 2.0))
    {
        lastRPSP->setVal(  RPSP->getdVal());
        if (abs(RPSP->getdVal()) < maxRP->getdVal())
        {
            RPcmd->setVal(RPSP->getdVal());
        }
        else
        {
            RPcmd->setVal((RPSP->getdVal()>0.0)?maxRP->getdVal()
                                                        :-maxRP->getdVal());
        }
        //vm.sendAssetVar(RPcmd, p_fims);
        rc++;
    }
    // if ( valueChanged(amap["ReactivePowerSetpoint"], amap["lastReactivePowerSetpoint"], amap["ReactivePowerDeadband"] ,dval, 2.0))
    // {
    //     amap["lastReactivePowerSetpoint"]->setVal(  amap["ReactivePowerSetpoint"]->getdVal());
    //     if (abs(amap["ReactivePowerSetpoint"]->getdVal()) < amap["maxReactivePower"]->getdVal())
    //     {
    //         amap["ReactivePowerCmd"]->setVal(amap["ReactivePowerSetpoint"]->getdVal());
    //     }
    //     else
    //     {
    //         amap["ReactivePowerCmd"]->setVal((amap["ReactivePowerSetpoint"]->getdVal()>0.0)?amap["maxReactivePower"]->getdVal()
    //                             :-amap["maxReactivePower"]->getdVal());
    //     }
    //     sendAsset(amap["ReactivePowerCmd"], p_fims);
    //     rc++;
    // }

    return rc;   
}
   

int main(int argc, char *argv[])
{
    // chan_data t_data;  // time channel
    // chan_data m_data;  // message channel
    // chan_data f_data;  // fims channel 

    // channel <int> wakechan;         // this is for wakeups
    // channel <char *>msgchan;       // this is for messages ( will probably be fims sort of messages)
    // channel <fims_message *>fimschan;  // this is for real (external) fims messages

    // this is our main data map
    varsmap vmap;


    // we'll have a bms_manager but wont quite use it yet
    bms_man = new asset_manager("bms_man");
 
    asset* bm;

    bms_man->setVmap(&vmap);
    bm = bms_man->addInstance("bms_1");

    amap = bm->getBmap();
    // TODO this may be redundant
    bm->setVmap(&vmap);

    //right this thing is ready to go
    // Just for kicks read in the config file and see if it had our links in it
    // this should be the case after the first run.
    bm->configure("configs/test_bms_initial.json");
    bms_man->debugConfig(bm, "Maps with links at beginning");
    bm->cfgwrite("configs/bms_1_at_start.json");
    bms_man->running = 1;
    // we should be able to set up the amap table from the links
    vm.CheckLinks(vmap, *amap, "/links/bms_1");

    //bm->setup(vmap, "/links/bms_1");
    bm->cfgwrite("configs/bms_1_after_links.json");
    // we should be able to set up the amap table from the links
    //reload = 0;

    // this is a low level configure with no subs.
    //bm->configure("configs/test_bss.json");

    // this sets up the amap for bss_running vars not needed if the links are in the config....

    bm->initLinks();
    bm->cfgwrite("configs/bms_1_afterInitlinks.json");

    // todo overwrite config with the last running config.
    //bm->configure("data/last_bss_1.json");
    bms_man->debugConfig(bm, "Maps at beginning");
    bm->cfgwrite("configs/bms_1_cfg_and_links.json");

    // set up an outgoing fims connection. 
    fims* p_fims = new fims;
    p_fims->Connect((char *)"fimsBmsTest");

    int wakeup;
    //std::string item2;
    char* item3;
    fims_message* msg;
    bms_man->setupChannels();
// timer loop need more expansion
// its job is to run a timer call back on timer requests
//  a timer request will have a delay ( from time now) and a call back function
// the timeer loop sleeps until the next callback is due.
// call backs are insterted in order.

//run the timer_loop every 100 mS 
//pcs_man->run_timer(&pcs_man->t_data, pcs_man->timer_loop,   &running, 500,  &pcs_man->wakechan);
    bms_man->run_timer(500);

// run the message loop eery 1.5 seconds
//pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running, 1500,  &pcs_man->wakechan);

    bms_man->run_message( 1500);
    int ccnt = 0;
//char** getSubs(varsmap &vmap, varmap &amap, const char* aname, const char* vname,  int &ccnt)
    vecmap vecs;

    vm.getVList(vecs, vmap, *amap, "bms", "Blocks", ccnt);
    vm.getVList(vecs, vmap, *amap, "bms", "Pubs", ccnt);
    char** dbsubs = vm.getList(vecs, vmap, *amap, "bms", "Subs", ccnt);

    //char** dbsubs = vm.getList(vmap, *amap, "bms", "Subs", ccnt);
    // vm.addSub(dbsubs, "bms",   ccnt, scnt);
    // vm.addSub(dbsubs, "bms_1", ccnt, scnt);
    // vm.addSub(dbsubs, "bms_2", ccnt, scnt);
    // vm.showSubs(dbsubs, "bms", ccnt, scnt);

    // const char* subs[] = {
    //         "/components", 
    //         "/assets/bms",
    //         "/assets/bms_1",
    //         "/controls/bms_1",
    //         "/status/bms_1",
    //         "/params/bms_1",
    //         "/params/bms",
    //         "/controls/bms",
    //         "/test/bms",
    //         "/status/bms",
    //         "/controls/bms"
    //         };
// the fims system will get pubs and create a varsMap for  the items.
//pcs_man->run_fims(&pcs_man->f_data, fims_loop,    &running, 1500,  &pcs_man->wakechan);
    bms_man->run_fims(1500, dbsubs, "bmsMan", ccnt);

// manager_loop
//pub_loop
   
    int tnum = 0;
    // you could use setvar to make sure its there

    // any wake up comes here
    while(bms_man->wakechan.get(wakeup,true)) {
        // then service the other channels 
        //
        if(bms_debug)std::cout << " BMS >> wakeup ival "<< wakeup << "\n";
        // a wakeup of 0 means service the others
        // a wakeup if 1 means process the asset
        // a wakeup of 2 means pub the asset
        // we can add special wakeups for things like ESTOP Fast Requests etc
        if(wakeup == 0)
        {
            if(bms_debug)std::cout << " BMS >> process another channel\n";
            //break;
        }
        ///  got some big plans for messages and wakeups
        // we really have two events (poll : 1 pub :2 )
        // but we could (will ) set up more event groups to force an immediate response.
        // 
        //if(wakeup == LoadRequestWakeUp) future option
        if(wakeup == 1) //POLL EVENT
        {
            vm.setTime();
            if(1 ||bms_debug)std::cout << " BMS >> get_state\n";

            HandleLoadRequest(vmap, *amap, "bms", p_fims);

            if(bms_debug)std::cout << " BMS >> get_state\n";
            HandleHeartBeat(vmap, *amap, "bms", p_fims);
            if(bms_debug)std::cout << " BMS >>process_cmds\n";
            HandleCmd(vmap, *amap, "bms", p_fims);
            if(bms_debug)std::cout << " BMS >> process_power\n";
            HandlePower(vmap, *amap, "bms", p_fims);
        }

        if(wakeup == WAKE_LEVEL_PUB) //PUB EVENT
        {
            if(bms_debug)std::cout << " BMS >> publish status\n";

            // publ
            
            cJSON* cjbm = bm->getConfig("/status/bms_1");
            char* res = cJSON_Print(cjbm);
            //printf("Publish \n%s\n", res);
            cJSON_Delete(cjbm);
            p_fims->Send("pub", "/asset/bms/bms_1", nullptr, res);
            free((void *)res) ;
            bms_man->wakechan.put(4);// cause wake event 4
            //break;
        }

        // we cant issue the event "send" yet but when this all runs as a thread inside the bms_manager object 
        // we'll have direct access to the wakeup and message channels
        // then a fims set to /controls/bms_1:wakeup '{"value":3}' will cause this to happen 
        // bms_man->wakechan->put(wakeval);
        // once we do this the assetVars will have to have locking but no big deal on that.

        if(wakeup == 3) // HANDLESETPOINT REQUEST EVENT do it now dont wait for a poll
        {
            vm.setTime();
            if(1 ||bms_debug)std::cout << " BMS >> wake event 3\n";

            HandleLoadRequest(vmap, *amap, "bms", p_fims);
        }
        if(wakeup == 4) // HANDLE TEST REQUEST EVENT do it now dont wait for a poll
        {
            //vm.setTime();
            if(1 ||bms_debug)std::cout << " BMS >> >>>>>>>>wake event 4\n";

            //HandleLoadRequest(vmap, *amap, "bms", p_fims);
        }

        if(wakeup == -1) //QUIT
        {
            // quit
            running = 0;
            break;

        }
        // this is the testmessage
        if(bms_man->msgchan.get(item3,false)) {
            sendTestMessage(p_fims, tnum);
            if(bms_debug)std::cout << " BMS >>item3-> data  "<< item3 << " tnum :"<< tnum<<"\n";
            tnum++;
            sendTestMessage(p_fims, tnum);
            if(bms_debug)std::cout << " BMS >>item3-> data  "<< item3 << " tnum :"<< tnum<<"\n";
            tnum++;
            free((void *) item3);
        }

        // this gets in fims messages
        if(bms_man->fimschan.get(msg, false)) {
            if(msg->body)
            {
                std::cout << " BMS >>fims_msg method :"<<msg->method<<" uri :"<< msg->uri  << " body :"<<msg->body<<"\n";
            }
            else
            {
                std::cout << " BMS >>fims_msg method :"<<msg->method<<" uri :"<< msg->uri  << " body :"<< "NoBody" <<"\n";
            }

            std::cout << " BMS >>fims_msg uri "<< msg->uri  << "\n";
            std::cout << " BMS >>fims_msg method "<< msg->method  << "\n";
            // we need to collect responses
            cJSON *cj = nullptr;
            // either here of the bms instance
            //bms_man->vmap  
            if (msg->method != nullptr)
            {
                std::cout << " BMS >> process fims message..." << "\n";

                vm.processFims(*bms_man->vmap,  msg, &cj);
                if (cj && msg->replyto)
                {
                    
                    std::cout << " BMS >> process fims reply..." << "\n";

                    char* tmp = cJSON_PrintUnformatted(cj);
                    if(tmp)
                    {
                        p_fims->Send("set",msg->replyto, nullptr, tmp);
                        free((void*)tmp);
                    }
                }
                if(cj)cJSON_Delete(cj);

            }
            p_fims->free_message(msg);

            //free((void *) item3);
        }

    }

    bms_man->t_data.cthread.join();
    std::cout << " BMS >> t_data all done " << "\n";
    bms_man->m_data.cthread.join();
    std::cout << "BMS >> m_data all done " << "\n";
    bms_man->f_data.cthread.join();
    std::cout << "BMS >> f_data all done " << "\n";

    bm->cfgwrite("configs/bms_1_attheend.json");
    cJSON *cj = vm.getMapsCj(*bms_man->vmap);
    char *res = cJSON_Print(cj);
    printf("BMS >>Maps at end \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);

    //monitor M;
    //M.configure("configs/monitor.json");
    
    delete bms_man;

    return 0;

}
