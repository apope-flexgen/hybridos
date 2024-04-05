/*
 * ess test , opens up the master config and creates all the peripherals
 * bms , pcr , drc
 * Starts all those threads if needed
 * handles the interface between hos ( via modbus server pubs) and hte
 * controllers. It has registers and links. g++ -std=c++11 -g -o ./test_ess -I
 * ./include test/test_ess.cpp -lpthread -lcjson -lfims
 */

#include "../test/release_fcns.cpp"
#include "asset.h"
#include "channel.h"
#include "chrono_utils.hpp"
#include "monitor.h"

// set this to 0 to stop
volatile int running = 1;
asset_manager* ess_man = nullptr;

void signal_handler(int sig)
{
    running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    if (ess_man)
    {
        ess_man->running = 0;
        ess_man->wakechan.put(-1);
    }
    signal(sig, SIG_DFL);
}

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
            uri = "/components/test_1";
            body = "{\"var_set_one\":21}";
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
            uri = "/assets/bms_1";
            body = "{\"ctrlword1\":{\"value\":3}}";
        }
        break;
        case 8:
        {
            method = "set";
            replyto = "/test/foo_8";
            uri = "/assets/bms_1";
            body = "{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
        }
        break;
        case 9:
        {
            method = "set";
            replyto = "/test/foo_9";
            uri = "/components/catl_ems_bms_rw";
            body = "{\"ems_test_status\":{\"value\":\"Running\"}}";
        }
        break;
        default:
            break;
    }
    if (uri)
        p_fims->Send(method, uri, replyto, body);
    return 0;
}

// this is our map utils factory
VarMapUtils defvm;
VarMapUtils* vm = &defvm;

// this is a map of local variables as known to the asset
varmap* amap;
std::vector<std::string> sysVec;
// thse guys are the wrappers to promote the cascade
// int cascadeAI(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// asset* am
//         ,  int(*runAI)(varsmap &vmap, varmap &amap, const char* aname, fims*
//         p_fims, asset* am) )
// {
//     return runAI(vmap, amap,aname, p_fims, am);
// }

// int cascadeAM(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// asset_manager* am
//         ,  int(*runAM)(varsmap &vmap, varmap &amap, const char* aname, fims*
//         p_fims, asset_manager* am) ,  int(*runAI)(varsmap &vmap, varmap
//         &amap, const char* aname, fims* p_fims, asset* am)
//         )
// {
//     if (am)
//     {
//         // assets are in assetMap managers are in assetManMap
//         for (auto ix : am->assetManMap)
//         {
//             asset_manager* amc = ix.second;

//             if(0)FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager
//             [%s] \n ",__func__, amc->name.c_str());
//             if(runAM)runAM(vmap, amc->amap,amc->name.c_str(), p_fims, amc);
//             cascadeAM(vmap, amc->amap,amc->name.c_str(),p_fims,amc,runAM,
//             runAI);

//         }
//         for (auto ix : am->assetMap)
//         {
//             asset* amc = ix.second;

//             if(0)FPS_ERROR_PRINT("%s >>>>>> cascading function to >>>> Asset
//             [%s] \n ",__func__, amc->name.c_str());

//             //runAI(vmap, amc->amap,amc->name.c_str(), p_fims, amc);
//             if(runAI)cascadeAI(vmap, amc->amap,amc->name.c_str(),p_fims, amc,
//             runAI);

//         }
//     }
//     return 0;
// }

void run_bms_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager\n", __func__);
}

void run_pcs_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>PCS>>>>>>>>>>>%s running for PCS Manager\n", __func__);
}

// We have presently have two wakeup messages sent
bool run_bms_wakeup(asset_manager* am, int wakeup)
{
    char* item3;
    fims_message* msg;
    // std::cout << " bms >> wakeup ival "<< wakeup << "\n";
    // FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval
    // %d \n",__func__, am->vm->get_time_dbl(),
    // am->name.c_str(), wakeup);

    am->vm->setTime();

    if (am->reload != 2)
    {
        am->vm->set_base_time();  // Time();
        am->vm->setTime();

        am->run_init(am);
    }
    if (am->vm->get_time_dbl() > 50000.0)
    {
        am->vm->set_base_time();  // Time();
        am->vm->setTime();
    }

    if (0)
        FPS_ERROR_PRINT(
            "%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset "
            "Manager wval %d \n",
            __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    if (wakeup == 0)
    {
        // std::cout << am->name <<">> MANAGER LOOP  process another channel\n";
        // break;
    }
    // now process all the events

    // Do the manager first
    if (wakeup == WAKE_LEVEL1)
    {
        // std::cout << am->name <<" >>get_state\n";
        // HandleHeartBeat(*am->vmap, am->amap, "ess", am->p_fims);
        // std::cout << am->name << "  >>process_cmds\n";
        // HandleCmd(*am->vmap, am->amap, am->p_fims);
        // std::cout << am->name << "  >>process_power\n";
        HandleBMSChargeL1(*am->vmap, am->amap, "bms", am->p_fims, am);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up

        // now get all the managed assets , wake those up too
    }

    if (wakeup == WAKE_LEVEL3)
    {
        // std::cout << am->name <<" >>HanlePower\n";
        // HandlePower(*am->vmap, am->amap, "ess");
        // std::cout << am->name << "  >>process_cmds\n";
        // HandleCmd(*am->vmap, am->amap, am->p_fims);
        // std::cout << am->name << "  >>process_power\n";
        // HandlePower(*am->vmap, am->amap, am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up

        // now get all the managed assets , wake those up too
    }

    if (wakeup == WAKE_LEVEL_PUB)
    {
        // // std::cout << am->name << " >>publish status\n";
        // //ess_manager * ess = (ess_manager*)am;
        // //asset_manager* bm2 = am->getManAsset("bms");

        // // note this assumes that the bms is running threaded
        // // let the builtin functions do this bms ma not be running a manager
        // thread.
        // //bm2->wakechan.put(wakeup);
        // //publ
        // const char *pname ="Unknown";
        // cJSON* cjbm = am->getConfig();
        // // printf("%s >>>>>>>Publish cjbm %p\n", __func__,
        // (void *)cjbm); char* res = cJSON_Print(cjbm);
        // // printf("%s >>>>>>>Publish fims  %p\n", __func__,
        // (void *)am->p_fims); cJSON_Delete(cjbm);
        // // TODO fix name
        // am->p_fims->Send("pub", pname, nullptr, res);
        // free((void *)res) ;

        // //break;
    }

    if (am->msgchan.get(item3, false))
    {
        // sendTestMessage(p_fims, tnum);
        // std::cout << am->name << "  >>item3-> data  "<< item3 << " tnum :"<<
        // am->tnum<<"\n";
        am->tnum++;
        // sendTestMessage(p_fims, tnum);
        // std::cout << am->name << "  >>item3-> data  "<< item3 << " tnum :"<<
        // am->tnum<<"\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << am->name << "  >> fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        cJSON* cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->sysVec = &sysVec;
        am->vm->processFims(*am->vmap, msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (tmp)
            {
                am->p_fims->Send("set", msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);

        // free((void *) item3);
    }
    for (auto ix : am->assetManMap)
    {
        if (0)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  running for "
                "Asset Manager [%s] \n",
                __func__, am->name.c_str(), ix.first.c_str());

        asset_manager* am2 = ix.second;  // am->getManAsset("bms");
        if (am2->run_wakeup)
        {
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>>>>>>>> %s Manager Loop Wakeup >>>>>>>>>>> "
                    "running for Asset Manager [%s] \n",
                    __func__, am->name.c_str(), ix.first.c_str());
            // first trigger the wakeup there is no thread in the lower leel managers
            // am2->wakechan.put(wakeup);
            am2->run_wakeup(am2, wakeup);
        }
        else
        {
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>>>>>>>> %s Manager Loop NO Wakeup >>>>>>>>>>> "
                    "running for Asset Manager [%s] \n",
                    __func__, am->name.c_str(), ix.first.c_str());
        }

        // am2->wakechan.put(wakeup);
    }

    // // now do the assets
    // for (auto ix : am->assetMap)
    // {
    //     asset* ass = ix.second ; //am->getManAsset("bms");
    //     if(0)FPS_ERROR_PRINT("%s >>>>>>>>>%s ASSETS >>>>>>>>>>> running for
    //     Asset [%s] \n",__func__, am->name.c_str(),
    //     ix.first.c_str());
    //     //TODO ass->wakechan.put(wakeup); this will only work if the assets are
    //     threaded if (ass->run_wakeup)
    //         ass->run_wakeup(ass, wakeup);
    // }

    if (wakeup == -1)
    {
        // quit
        FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, am->name.c_str());
        running = 0;
        // break;//
        return false;
    }

    return true;
}

bool run_pcs_wakeup(asset_manager* am, int wakeup)
{
    char* item3;
    fims_message* msg;

    if (am->reload != 2)
        am->run_init(am);

    if (0)
        FPS_ERROR_PRINT(
            "%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset "
            "Manager wval %d \n",
            __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    if (wakeup == 0)
    {
        // std::cout << am->name <<">> MANAGER LOOP  process another channel\n";
        // break;
    }
    // now process all the events

    if (wakeup == WAKE_LEVEL1)
    {
        GetPCSLimits(*am->vmap, am->amap, "pcs", am);
    }

    if (wakeup == WAKE_LEVEL_PUB)
    {
        // // std::cout << am->name << " >>publish status\n";
    }

    if (am->msgchan.get(item3, false))
    {
        am->tnum++;
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << am->name << "  >> fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        cJSON* cj = nullptr;
        // either here of the bms instance
        am->vm->processFims(*am->vmap, msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (tmp)
            {
                am->p_fims->Send("set", msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);
    }
    for (auto ix : am->assetManMap)
    {
        if (0)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  running for "
                "Asset Manager [%s] \n",
                __func__, am->name.c_str(), ix.first.c_str());

        asset_manager* am2 = ix.second;  // am->getManAsset("bms");
        if (am2->run_wakeup)
        {
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>>>>>>>> %s Manager Loop Wakeup >>>>>>>>>>> "
                    "running for Asset Manager [%s] \n",
                    __func__, am->name.c_str(), ix.first.c_str());
            am2->run_wakeup(am2, wakeup);
        }
        else
        {
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>>>>>>>> %s Manager Loop NO Wakeup >>>>>>>>>>> "
                    "running for Asset Manager [%s] \n",
                    __func__, am->name.c_str(), ix.first.c_str());
        }
    }

    if (wakeup == -1)
    {
        // quit
        FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, am->name.c_str());
        running = 0;
        return false;
    }

    return true;
}

bool run_bms_asset_wakeup(asset* am, int wakeup)
{
    if (am->vm->get_time_dbl() > 500)
    {
        am->vm->set_base_time();
        am->vm->setTime();
    }
    am->vm->setTime();

    if (0)
        FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n", __func__, am->name.c_str(), wakeup);
    // FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset
    // [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if (wakeup == WAKE_LEVEL1)
    {
        // if (BMSIsActive(*am->vmap, am->amap, am->name.c_str(), am))
        GetBMSStatus(*am->vmap, am->amap, am->name.c_str(), am);
    }
    else if (wakeup == WAKE_LEVEL2)
    {
        // HandleHeartBeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
        // GetBMSLimits(varsmap &vmap, varmap &amap, const char*aname, fims* p_fims)
    }

    return true;
}

// this one assumes channels are working..
bool run_ems_wakeup(asset_manager* am, int wakeup)
{
    if (0)
        FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__,
                        am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // then service the other channels
    //
    char* item3;
    fims_message* msg;
    // a wakeup of 0 means service the others
    // a wakeup if 1 means process the asset
    // a wakeup of 2 means pub the asset
    if (wakeup == 0)
    {
        // td::cout << " BMS >> process another channel\n";
        // break;
    }

    if (wakeup == 1)
    {
        am->vm->setTime();
        // HandleESSCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        GetESSLimits(*am->vmap, am->amap, "ess", am);  // am->name.c_str());
        HandlePower(*am->vmap, am->amap, "ess", am);
    }

    if (wakeup == 2)
    {
        std::cout << " TODO BMS >> publish status\n";
    }

    if (wakeup == -1)
    {
        // quit
        am->running = 0;
        return false;
    }

    // this is the testmessage
    if (am->msgchan.get(item3, false))
    {
        // sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        // sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << " BMS >>fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        cJSON* cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->processFims(*am->vmap, msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (tmp)
            {
                am->p_fims->Send("set", msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);
        // free((void *) item3);
    }
    return true;
}
// the main ess_controller start up
int main(int argc, char* argv[])
{
    varsmap vmap;
    vecmap vecs;
    system("mkdir -p run_configs");

    // TODO make this an asset manager
    ess_man = new asset_manager("ess_controller");

    ess_man->vmap = &vmap;  //??
    ess_man->vm = vm;       //??

    // new for UI ordering
    vm->sysVec = &sysVec;

    ess_man->run_wakeup = run_ems_wakeup;

    ess_man->setVmap(&vmap);
    ess_man->vecs = &vecs;
    ess_man->running = 1;

    amap = ess_man->getAmap();  // may not need this

    // TODO this may be redundant
    // ess_man->setVmap(&vmap);

    // right this thing is ready to go
    // Just for kicks read in the config file and see if it had our links in it
    // this should be the case after the first run.
    printf(" Getting initial configuration\n");

    const char* cfgname = "configs/test_ess_config.json";
    // ess_man->configure(vmap, cfgname , "ess");
    // varsmap* vmp = ess_man->getVmap();
    vm->configure_vmap(vmap, cfgname);
    // ess_man->configure(cfgname , nullptr);

    {
        // this a test for our config.
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        const char* fname = "run_configs/ess_1_base_ess.json";
        vm->write_cjson(fname, cjbm);
        // cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        printf("Maps (possibly) with links at beginning  cjbm %p \n%s\n <<< done\n", (void*)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);
    }
    // we should be able to set up the amap table from the links
    // TODO Fiish this
    vm->CheckLinks(vmap, *amap, "/links/ess");
    // bit ugly at the moment .. its a copy of the aset config

    // no dont do this now
    // ess_man->setup(vmap, "/links/ess");
    ess_man->cfgwrite("run_configs/ess_after_links.json");

    // this is just a check
    // vmap["/links/bms_1"]
    auto ix = vmap.find("/links/ess");
    if (ix != vmap.end())
    {
        // if this works no need to run the init function below
        printf(" We found our links , we should be able to set up our link amap\n");
        for (auto iy : ix->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                printf(" lets link [%s] to [%s]\n", iy.first.c_str(), iy.second->aVal->valuestring);
                // for example lets link [AcContactor] to the var defined for
                // [/status/bms_1:AcContactor] amap[iy.first] = vm->getVar (vmap,
                // y.second->aVal->valuestring);//  getVar(varsmap &vmap, const char*
                // comp, const char* var=nullptr)
                // amap["AcContactor"]               = vm->linkVal(vmap, link,
                // "AcContactor",            fval);
            }
        }
    }

    // nah not working yet
    // ess_man->cfgwrite("configs/justlinks_ess.json");

    printf("ess test OK so far\n");
    // this is a low level configure with no subs.
    // ess_man->configure("configs/test_ess.json");
    // todo mo
    cfgname = "configs/test_ess.json";  // TODO move this into asset manager assconfig
    ess_man->configure(&vmap, cfgname, "ess");
    // ess_man->configure(cfgname , (char *)"ess");
    // no worries at this stage

    // this sets up the amap for bss_running vars not needed if the links are in
    // the config....
    // ess_man->initLinks();

    // ess_man->cfgwrite("configs/cfg_and_links_ess.json");

    // todo overwrite config with the last running config.
    // bm->configure("data/last_bss_1.json");
    // set up an outgoing fims connection.
    fims* p_fims = new fims;

    // p_fims->Connect((char *)"fimsEssTest");
    bool connectOK = p_fims->Connect((char*)"FimsEssTest");
    if (!connectOK)
    {
        FPS_ERROR_PRINT("%s >> unable to connect to fims server. All is lost.\n", __func__);
        running = 0;
    }
    ess_man->p_fims = p_fims;
    ess_man->running = 1;

    // eeded for the controller, manager and possibly assets
    ess_man->setupChannels();

    // the timer generates  channel wakes every 500 mS
    ess_man->run_timer(100);

    // run the message loop eery 1.5 seconds
    // pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running,
    // 1500,  &pcs_man->wakechan);

    // the message  generates test message  channel wakes every 1500 mS
    // ess_man->run_message( 1500);

    const char* subs[] = { "/components", "/assets", "/params", "/status", "/controls", "/test", "/variables" };
    int sublen = sizeof(subs) / sizeof(subs[0]);

    // the fims system will get pubs and create a varsMap for  the items.
    // TODO restrict fims to known components

    ess_man->run_fims(1500, (char**)subs, "essMan", sublen);

    // the manager runs the channel receiver thread
    ess_man->run_manager(p_fims);

    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* bms_man = nullptr;
    asset_manager* pcs_man = nullptr;
    auto ixa = vmap.find("/assets/ess");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        printf(" ESS >>We found our assets, we should be able to set up our system\n");
        for (auto iy : ixa->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                printf(" lets run assets for  [%s] from [%s]\n", iy.first.c_str(), iy.second->aVal->valuestring);
                const char* fname = iy.second->aVal->valuestring;
                if (iy.first == "bms")
                {
                    printf(" setting up a bms manager\n");

                    bms_man = new asset_manager("bms_man");
                    bms_man->run_init = run_bms_init;
                    bms_man->run_wakeup = run_bms_wakeup;
                    bms_man->p_fims = p_fims;

                    bms_man->setVmap(&vmap);
                    bms_man->vecs = &vecs;
                    // now get the bms_manager to configure itsself
                    bms_man->configure(&vmap, fname, "bms", &sysVec, run_bms_asset_wakeup, ess_man);
                    // add it to the assetList
                    ess_man->addManAsset(bms_man, "bms");

                    printf(" done setting up a bms manager varsmap must be fun now\n");
                    // we should be able to do things like get status from the
                    // bms_manager. first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset("bms");
                    if (bm2)
                    {
                        printf(" @@@@@@@  found bms asset manager with %d bms assets\n", bm2->getNumAssets());
                    }
                    bms_man->running = 1;
                    bms_man->setupChannels();
                    // bms_man->run_timer(500);
                    bms_man->run_message(500);
                }
                else if (iy.first == "pcs")
                {
                    printf(" setting up a pcs manager\n");

                    pcs_man = new asset_manager("pcs_man");
                    pcs_man->run_init = run_pcs_init;
                    pcs_man->run_wakeup = run_pcs_wakeup;
                    pcs_man->p_fims = p_fims;

                    pcs_man->setVmap(&vmap);
                    pcs_man->vecs = &vecs;

                    pcs_man->setVmap(&vmap);
                    // now get the pcs_manager to configure itsself
                    pcs_man->configure(&vmap, fname, "pcs", &sysVec, nullptr, ess_man);
                    // add it to the assetList
                    ess_man->addManAsset(pcs_man, "pcs");

                    InitPCSLinks(*pcs_man->vmap, pcs_man->amap, "pcs", pcs_man);

                    printf(" done setting up a pcs manager varsmap must be fun now\n");
                    // we should be able to do things like get status from the
                    // pcs_manager. first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset("pcs");
                    if (bm2)
                    {
                        printf(" @@@@@@@  found pcs asset manager with %d pcs assets\n", bm2->getNumAssets());
                    }
                    pcs_man->running = 1;
                    pcs_man->setupChannels();
                    // pcs_man->run_timer(500);
                    pcs_man->run_message(500);
                }
            }
        }
    }
    {
        const char* fname = "configs/ess_4_cfg_links_assets_ess.json";
        // this a test for our config with links
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        vm->write_cjson(fname, cjbm);

        // cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        // printf("Maps (should be ) with links and assets  cjbm %p \n%s\n <<<
        // done\n", (void *)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);
    }
    int ival = 0;
    // pcs_man->amap["CtrlwordPMode"] =
    vm->setLinkVal(vmap, "pcs", "/controls", "ctrlword_pmode", ival);
    // pcs_man->amap["CtrlwordQMode"] =
    vm->setLinkVal(vmap, "pcs", "/controls", "ctrlword_qmode", ival);
    // FPS_ERROR_PRINT("Ctrlword PMODE comp [%s] var [%s]\n",
    // CtrlwordPMode->comp.c_str(), CtrlwordPMode->name.c_str());
    // ess_man->vmap = &vmap;
    if (ess_man->vmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->vmap);
        char* res = cJSON_Print(cj);
        printf("ESS >>Maps before run \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Maps before runskipped\n");
    }
    while (ess_man->running)
    {
        // printf("ESS >>Maps running\n");

        poll(nullptr, 0, 100);
    }
    printf("ESS >> Shutting down\n");
    pcs_man->running = 0;

    ess_man->t_data.cthread.join();
    std::cout << "ESS >>t_data all done "
              << "\n";
    ess_man->m_data.cthread.join();
    std::cout << "ESS >>m_data all done "
              << "\n";
    ess_man->f_data.cthread.join();
    std::cout << "ESS >>f_data all done "
              << "\n";

    if (ess_man->vmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->vmap);
        char* res = cJSON_Print(cj);
        printf("ESS >>Maps at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Maps at end skipped\n");
    }

    // monitor M;
    // M.configure("configs/monitor.json");

    delete ess_man;

    return 0;
}
