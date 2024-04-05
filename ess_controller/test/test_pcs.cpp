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
#include "assetFunc.cpp"
#include "channel.h"
#include "chrono_utils.hpp"
#include "monitor.h"
#include "pcs.h"

// set this to 0 to stop
volatile int running = 1;

void signal_handler(int sig)
{
    running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

// this is really the manager loop
// TODO subscribe (name)
// TODO if we add subs to data then this is common code

// typedef void*(* vLoop)(void *args);
// move to the correct place later
// the uri defines the base map the var name defines the rest
// VarMapUtils* vmap = new VarMapUtils;
// this is for pubs
// what about sets with replytos
// what about pubs and sets with cj values as objets
// deal with the sets first

// both the asset manager and the asset has a varsmap name space
// adjust these to test the system
int sendTestMessage(fims* p_fims, int tnum)
{
    const char* method;
    const char* replyto = nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;

    switch (tnum)
    {
        case 1:
        {
            method = "set";
            replyto = "/test/foo";
            uri = "/components/pcs_1";
            body = "{\"start_system\":21}";
        }
        break;
        case 2:
        {
            method = "set";
            // replyto = "/test/foo";
            uri = "/components/test_2";
            body = "{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        }
        break;
        case 3:
        {
            method = "set";
            replyto = "/test/foo_2";
            uri = "/components/test_2";
            body = "{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        }
        break;
        case 4:
        {
            method = "set";
            replyto = "/test/foo_4";
            uri = "/components/test_3";
            body = "{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 5:
        {
            method = "get";
            replyto = "/test/foo_5";
            uri = "/components/test_3";
            // body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 6:
        {
            method = "get";
            replyto = "/test/foo_6";
            uri = "/components/test_3/var_set_twox";
            // body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 7:
        {
            method = "set";
            replyto = "/test/foo_7";
            uri = "/assets/pcs_1";
            body = "{\"start_stop\":{\"value\":3}}";
        }
        break;
        case 8:
        {
            method = "set";
            replyto = "/test/foo_8";
            uri = "/assets/pcs_1";
            body = "{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
        }
        break;
        case 9:
        {
            method = "set";
            replyto = "/test/foo_9";
            uri = "/components/catl_ems_bms_rw";
            body = "{\"pcs_test_status\":{\"value\":\"Running\"}}";
        }
        break;
        default:
            break;
    }
    if (uri)
        p_fims->Send(method, uri, replyto, body);
    return 0;
}

// lot of notes here but we'll condense them
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
// you could use setvar to make sure its there
// const char* ssstat = "/status/bss_1:start_stop";
// const char* ssvar = "/components/bss_1:start_stop";
// const char* sscatl = "/components/catl_ems_bms_rw:ems_cmd";
// const char* sscatl_pon = "Power on cmd";
// const char* sscatl_poff = "Power off cmd";
// const char* sscatl_init = "Initial";

// this is our map utils factory
VarMapUtils vm;

// this is a map of local variables as known to the asset
varmap* amap;

// this is done to make the config correct  and can be skipped when the config
// is set up you can use the config generated by this test as the basis of the
// real config move this to vm int setup(varsmap &vmap, const char* link)
// {
//     vm.setVal2(vmap, link,"AcContactor",               sAcContactor);
//     vm.setVal2(vmap, link,"AcContactorCloseCmd",       sAcContactorCloseCmd);
//     vm.setVal2(vmap, link,"AcContactorOpenCmd",        sAcContactorOpenCmd);

//     vm.setVal2(vmap, link,"DcContactor",               sDcContactor);
//     vm.setVal2(vmap, link,"DcContactorCloseCmd",       sDcContactorCloseCmd);
//     vm.setVal2(vmap, link,"DcContactorOpenCmd",        sDcContactorOpenCmd);

//     vm.setVal2(vmap, link,"On",                        sOn);
//     vm.setVal2(vmap, link,"OnCmd",                     sOnCmd);

//     vm.setVal2(vmap, link,"Off",                       sOff);
//     vm.setVal2(vmap, link,"OffCmd",                    sOffCmd);

//     vm.setVal2(vmap, link,"StandBy",                   sStandby);
//     vm.setVal2(vmap, link,"StandByCmd",                sStandbyCmd);
//     vm.setVal2(vmap, link,"ActivePowerSetpoint",       sActivePowerSetpoint);
//     vm.setVal2(vmap, link,"ReactivePowerSetpoint", sReactivePowerSetpoint);
//     vm.setVal2(vmap, link,"ActivePowerDeadband",       sActivePowerDeadband);
//     vm.setVal2(vmap, link,"ReactivePowerDeadband", sReactivePowerDeadband);
//     vm.setVal2(vmap, link,"lastActivePowerSetpoint",
//     slastActivePowerSetpoint); vm.setVal2(vmap,
//     link,"lastReactivePowerSetpoint", slastReactivePowerSetpoint);
//     vm.setVal2(vmap, link,"ActivePower",               sActivePower);
//     vm.setVal2(vmap, link,"ReactivePower",             sReactivePower);
//     vm.setVal2(vmap, link,"maxActivePower",            smaxActivePower);
//     vm.setVal2(vmap, link,"maxReactivePower",          smaxReactivePower);
//     vm.setVal2(vmap, link,"ReactivePowerCmd",         sReactivePowerCmd);
//     vm.setVal2(vmap, link,"ActivePowerCmd",            sActivePowerCmd);
//     vm.setVal2(vmap, link,"HeartBeat",                 sHeartBeat);

// }
// TODO move these into bms asset class
// actually it will read it all into a map amap
// inside a function you can get the asset
// assetVar * hbav = amap["HeartBeat"];

// // this (if needed is almost generic)
// int init(varsmap &vmap, varmap &amap, const char* link)
// {
//     bool fval = false;
//     int ival = 0;
//     double dval = 0.0;
//     double dbval = 5.0;
//     double dbmax = 50000.0;
//     amap["AcContactor"]               = vm.linkVal(vmap, link, "AcContactor",
//     fval); amap["AcContactorCloseCmd"]       = vm.linkVal(vmap, link,
//     "AcContactorCloseCmd",    fval); amap["AcContactorOpenCmd"]        =
//     vm.linkVal(vmap, link, "AcContactorOpenCmd",     fval);
//     amap["DcContactor"]               = vm.linkVal(vmap, link, "DcContactor",
//     fval); amap["DcContactorCloseCmd"]       = vm.linkVal(vmap, link,
//     "DcContactorCloseCmd",    fval); amap["DcContactorOpenCmd"]        =
//     vm.linkVal(vmap, link, "DcContactorOpenCmd",     fval);
//     amap["StandbyCmd"]                = vm.linkVal(vmap, link, "StandbyCmd",
//     fval); amap["Standby"]                   = vm.linkVal(vmap, link,
//     "Standby",                fval); amap["OnCmd"]                     =
//     vm.linkVal(vmap, link, "OnCmd",                  fval); amap["On"]
//     = vm.linkVal(vmap, link, "On",                     fval); amap["OffCmd"]
//     = vm.linkVal(vmap, link, "OffCmd",                 fval); amap["Off"]
//     = vm.linkVal(vmap, link, "Off",                    fval);
//     amap["ActivePowerSetpoint"]       = vm.linkVal(vmap, link,
//     "ActivePowerSetpoint",    dval); amap["ReactivePowerSetpoint"]     =
//     vm.linkVal(vmap, link, "ReactivePowerSetpoint",  dval);
//     amap["ActivePowerDeadband"]       = vm.linkVal(vmap, link,
//     "ActivePowerDeadband",    dbval); amap["ReactivePowerDeadband"]     =
//     vm.linkVal(vmap, link, "ReactivePowerDeadband",  dbval);
//     amap["lastActivePowerSetpoint"]   = vm.linkVal(vmap, link,
//     "lastActivePowerSetpoint",   dval); amap["lastReactivePowerSetpoint"] =
//     vm.linkVal(vmap, link, "lastReactivePowerSetpoint", dval);
//     amap["ActivePower"]               = vm.linkVal(vmap, link, "ActivePower",
//     dval); amap["ReactivePower"]             = vm.linkVal(vmap, link,
//     "ReactivePower",                 dval); amap["maxActivePower"]
//     = vm.linkVal(vmap, link, "maxActivePower",                dbmax);
//     amap["maxReactivePower"]          = vm.linkVal(vmap, link,
//     "maxReactivePower",              dbmax); amap["ReactivePowerCmd"]
//     = vm.linkVal(vmap, link, "ReactivePowerCmd",              dval);
//     amap["ActivePowerCmd"]            = vm.linkVal(vmap, link,
//     "ActivePowerCmd",                dbval); amap["HeartBeat"]
//     = vm.linkVal(vmap, link, "HeartBeat",                     ival);

// }

// generic move to lib
int sendAsset(assetVar* av, fims* p_fims)
{
    cJSON* cj = nullptr;
    av->getCjVal(&cj);
    char* res = cJSON_Print(cj);
    // printf("Publish \n%s\n", res);
    cJSON_Delete(cj);
    p_fims->Send("pub", av->comp.c_str(), nullptr, res);
    free((void*)res);
    return 0;
}

// generic move to lib
int HandleHeartBeat(varsmap& vmap, varmap& amap, fims* p_fims)
{
    // get the reference to the variable
    assetVar* hb = amap["HeartBeat"];
    int ival;
    hb->getiVal();
    ival++;
    if (ival > 255)
        ival = 0;

    hb->setVal(ival);
    sendAsset(hb, p_fims);
    return ival;
}

// this will change for the pcs
int HandleCmd(varsmap& vmap, varmap& amap, fims* p_fims)
{
    int rc = 0;
    // Turn on if conditions allow it
    bool fval = false;
    // int ival;
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
    bool OffCmdval = amap["OffCmd"]->getbVal();
    // bool Offval = amap["Off"]->getbVal();

    bool OnCmdval = amap["OnCmd"]->getbVal();
    bool Onval = amap["On"]->getbVal();
    bool Standbyval = amap["Standby"]->getbVal();

    bool AcContactorval = amap["AcContactor"]->getbVal();
    bool DcContactorval = amap["DcContactor"]->getbVal();

    if (OnCmdval && (!Onval || Standbyval) && AcContactorval && DcContactorval)
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
    if (amap["On"]->getbVal() && amap["StandbyCmd"]->getbVal())
    {
        rc++;
        amap["Standby"]->setVal(true);
        // send standby to device
        amap["StandbyCmd"]->setVal(false);
    }
    // TODO send stuff to FIMS
    return rc;
}
// TODO add timeout
// // move to lib
// template<class T>
// bool valueChanged( T one, T theother, T deadb)
// {
//     T diff = one-theother;
//     if (diff > deadb || diff < - deadb)
//        return true;
//     return false;
// }
// // return when one or the other has changed
// // or when we hae passed a time
// // TODO test timeout
// template<class T>
// bool valueChanged(assetVar* one, assetVar* theother, assetVar* deadb , T
// vtype, double timeout)
// {
//     bool resp;
//     T dval;
//     resp = valueChanged(one->getdVal(),
//     theother->getdVal(),deadb->getdVal()); if(!resp)
//     {
//         if(timeout > 0.0 &&  one->aVal->getsTime() + timeout > vm.getTime())
//         {
//             return true;
//         }
//     }
// }

int HandlePower(varsmap& vmap, varmap& amap, fims* p_fims)
{
    double dval;
    int rc = 0;
    if (vm.valueChanged(amap["ActivePowerSetpoint"], amap["lastActivePowerSetpoint"], amap["ActivePowerDeadband"], dval,
                        2.0))
    {
        amap["lastActivePowerSetpoint"]->setVal(amap["ActivePowerSetpoint"]->getdVal());
        if (abs(amap["ActivePowerSetpoint"]->getdVal()) < amap["maxActivePower"]->getdVal())
        {
            amap["ActivePowerCmd"]->setVal(amap["ActivePowerSetpoint"]->getdVal());
        }
        else
        {
            amap["ActivePowerCmd"]->setVal((amap["ActivePowerSetpoint"]->getdVal() > 0.0)
                                               ? amap["maxActivePower"]->getdVal()
                                               : -amap["maxActivePower"]->getdVal());
        }
        sendAsset(amap["ActivePowerCmd"], p_fims);
        rc++;
    }

    // another way to do this using local vars , prevents the dict lookup
    assetVar* RPSP = amap["ReactivePowerSetpoint"];
    assetVar* lastRPSP = amap["lastReactivePowerSetpoint"];
    assetVar* RPDB = amap["ReactivePowerDeadband"];
    assetVar* maxRP = amap["maxReactivePower"];
    assetVar* RPcmd = amap["ReactivePowerCmd"];

    if (vm.valueChanged(RPSP, lastRPSP, RPDB, dval, 2.0))
    {
        lastRPSP->setVal(RPSP->getdVal());
        if (abs(RPSP->getdVal()) < maxRP->getdVal())
        {
            RPcmd->setVal(RPSP->getdVal());
        }
        else
        {
            RPcmd->setVal((RPSP->getdVal() > 0.0) ? maxRP->getdVal() : -maxRP->getdVal());
        }
        sendAsset(RPcmd, p_fims);
        rc++;
    }
    // if ( valueChanged(amap["ReactivePowerSetpoint"],
    // amap["lastReactivePowerSetpoint"], amap["ReactivePowerDeadband"]
    // ,dval, 2.0))
    // {
    //     amap["lastReactivePowerSetpoint"]->setVal(
    //     amap["ReactivePowerSetpoint"]->getdVal()); if
    //     (abs(amap["ReactivePowerSetpoint"]->getdVal()) <
    //     amap["maxReactivePower"]->getdVal())
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

int main(int argc, char* argv[])
{
    varsmap vmap;

    // we'll have a pcs_manager but wont quite use it yet
    pcs_manager* pcs_man = new pcs_manager("pcs_man");

    pcs* pc;

    pcs_man->setVmap(&vmap);
    pc = pcs_man->addInstance("pcs_1");

    amap = pc->getBmap();
    // TODO this may be redundant
    pc->setVmap(&vmap);

    // right this thing is ready to go
    // Just for kicks read in the config file and see if it had our links in it
    // this should be the case after the first run.
    pc->configure("configs/cfg_and_links_pcs_1.json");
    pcs_man->debugConfig(pc, "Maps with links at beginning");
    pc->cfgwrite("configs/pcs_1_at_start.json");
    pcs_man->running = 1;
    // we should be able to set up the amap table from the links
    vm.CheckLinks(vmap, *amap, "/links/pcs_1");

    // pc->setup(vmap, "/links/pcs_1");
    pc->cfgwrite("configs/pcs_1_after_links.json");

    printf("pcs asset test OK\n");

    // this is a low level configure with no subs.
    pc->configure("configs/test_pcs.json");

    pc->cfgwrite("configs/pcs_1_after_test_pcs.json");

    // this sets up the amap for bss_running vars not needed if the links are in
    // the config....

    pc->initLinks();

    pc->cfgwrite("configs/pcs_1_afterInitlinks.json");

    // todo overwrite config with the last running config.
    // bm->configure("data/last_bss_1.json");
    pcs_man->debugConfig(pc, "Maps at beginning");

    // set up an outgoing fims connection.
    fims* p_fims = new fims;
    p_fims->Connect((char*)"fimsPcsTest");

    int wakeup;
    // std::string item2;
    char* item3;
    fims_message* msg;
    pcs_man->setupChannels();
    // timer loop need more expansion
    // its job is to run a timer call back on timer requests
    //  a timer request will have a delay ( from time now) and a call back
    //  function
    // the timeer loop sleeps until the next callback is due.
    // call backs are insterted in order.

    // run the timer_loop every 100 mS
    // pcs_man->run_timer(&pcs_man->t_data, pcs_man->timer_loop,   &running, 500,
    // &pcs_man->wakechan);
    pcs_man->run_timer(500);

    // run the message loop eery 1.5 seconds
    // pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running,
    // 1500,  &pcs_man->wakechan);

    pcs_man->run_message(1500);
    const char* subs[] = { "/components", "/assets/pcs_1", "/controls/pcs_1", "/assets/pcs", "/controls/pcs" };
    // the fims system will get pubs and create a varsMap for  the items.
    // pcs_man->run_fims(&pcs_man->f_data, fims_loop,    &running, 1500,
    // &pcs_man->wakechan);
    pcs_man->run_fims(1500, (char**)subs, "pcsMan");

    // manager_loop
    // pub_loop

    int tnum = 0;

    // any wake up comes here
    while (pcs_man->wakechan.get(wakeup, true))
    {
        // then service the other channels
        //
        std::cout << " wakeup ival " << wakeup << "\n";
        // a wakeup of 0 means service the others
        // a wakeup if 1 means process the asset
        // a wakeup of 2 means pub the asset
        if (wakeup == 0)
        {
            std::cout << " process another channel\n";
            // break;
        }

        if (wakeup == 1)
        {
            vm.setTime();
            std::cout << " get_state\n";
            HandleHeartBeat(vmap, *amap, p_fims);
            std::cout << " process_cmds\n";
            HandleCmd(vmap, *amap, p_fims);
            std::cout << " process_power\n";
            HandlePower(vmap, *amap, p_fims);
        }

        if (wakeup == WAKE_LEVEL_PUB)
        {
            std::cout << " publish status\n";

            // publ
            // TOOO  get this pub right
            // char* res = cJSON_Print(cjbm->child); perhaps

            cJSON* cjbm = pc->getConfig();
            char* res = cJSON_Print(cjbm);
            // printf("Publish \n%s\n", res);
            cJSON_Delete(cjbm);
            p_fims->Send("pub", "/status/pcs_1", nullptr, res);
            free((void*)res);

            // break;
        }

        if (wakeup == -1)
        {
            // quit
            // pcs_man->running = 0;
            break;
        }

        // this is the test message system
        if (pcs_man->msgchan.get(item3, false))
        {
            sendTestMessage(p_fims, tnum);
            std::cout << " item3-> data  " << item3 << " tnum :" << tnum << "\n";
            tnum++;
            sendTestMessage(p_fims, tnum);
            std::cout << " item3-> data  " << item3 << " tnum :" << tnum << "\n";
            tnum++;
            free((void*)item3);
        }

        // this gets in fims messages
        if (pcs_man->fimschan.get(msg, false))
        {
            std::cout << " fims_msg uri " << msg->uri << "\n";
            // we need to collect responses
            cJSON* cj = nullptr;
            // either here of the bms instance
            // bms_man->vmap
            vm.processFims(*pcs_man->vmap, msg, &cj);
            if (cj)
            {
                char* tmp = cJSON_PrintUnformatted(cj);
                if (tmp)
                {
                    p_fims->Send("set", msg->replyto, nullptr, tmp);
                    free((void*)tmp);
                }
            }
            p_fims->free_message(msg);

            // free((void *) item3);
        }
    }
    pcs_man->running = 0;

    pcs_man->t_data.cthread.join();
    std::cout << " t_data all done "
              << "\n";
    pcs_man->m_data.cthread.join();
    std::cout << "m_data all done "
              << "\n";
    pcs_man->f_data.cthread.join();
    std::cout << "f_data all done "
              << "\n";

    pc->cfgwrite("configs/pcs_1_attheend.json");
    cJSON* cj = vm.getMapsCj(*pcs_man->vmap);
    char* res = cJSON_Print(cj);
    printf("Maps at end \n%s\n", res);
    free((void*)res);
    cJSON_Delete(cj);

    // monitor M;
    // M.configure("configs/monitor.json");

    delete pcs_man;

    return 0;
}
