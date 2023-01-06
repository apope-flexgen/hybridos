/*
*
* an attempt to clean up the ess start up system.
* phil wilshire
*/

#include "asset.h"
#include "testUtils.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

VarMapUtils defvm;
VarMapUtils* vm = &defvm;
int running = 0;

// this is a map of local variables as known to the asset 
varmap* amap;
void changeTestValue(int newValue) {

}
void run_ess_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>EMS>>>>>>>>>>>%s running for EMS Manager\n", __func__);
}
void run_bms_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager\n", __func__);
}
void run_pcr_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>PCR>>>>>>>>>>>%s running for PCR Manager\n", __func__);
}
void run_dcr_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>DCR>>>>>>>>>>>%s running for DCR Manager\n", __func__);
}

bool run_bms_wakeup(asset_manager* am, int wakeup)
{
    return true;
}
bool run_pcr_wakeup(asset_manager* am, int wakeup)
{
    return true;
}
bool run_dcr_wakeup(asset_manager* am, int wakeup)
{
    return true;
}
void run_bms_asset_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager\n", __func__);
}
void run_pcr_asset_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>PCR>>>>>>>>>>>%s running for PCR Manager\n", __func__);
}
void run_dcr_asset_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>DCR>>>>>>>>>>>%s running for DCR Manager\n", __func__);
}

bool run_bms_asset_wakeup(asset_manager* am, int wakeup)
{
    return true;
}
bool run_pcr_asset_wakeup(asset_manager* am, int wakeup)
{
    return true;
}
bool run_dcr_asset_wakeup(asset_manager* am, int wakeup)
{
    return true;
}

// We have presently have two wakeup messages sent  
bool run_ess_wakeup(asset_manager* am, int wakeup)
{
    //char * item3;
    fims_message* msg;
    if (wakeup != 0)
    {
        if (1)FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    }
    if (am->reload != 2)
    {
        if (1)FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running init for %s Manager wval %d \n", __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
        am->run_init(am);
    }

    if (wakeup == -1)
    {
        // quit
        FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, am->name.c_str());
        running = 0;
        //break;//
        return false;
    }

    if (wakeup == 0)
    {
        if (0)std::cout << am->name << ">> MANAGER LOOP  process another channel\n";
        //break;
    }
    // now process all the events

    //Do the manager first
    if (wakeup == WAKE_LEVEL1)
    {
        am->vm->setTime();
    }

    if (wakeup == WAKE_LEVEL_PUB)
    {
        if (1)std::cout << am->name << " >>publish status\n";
        //am->vm->vListPartialSendFims(*am->vmap, *am->pmap, "pub", am->p_fims);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false)) {
        if (0)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  %s  >>>>BEFORE FIMS MESSAGE  method [%s] replyto[%s]  uri [%s]\n"
            , __func__
            , vm->get_time_dbl()
            , am->name.c_str()
            , msg->method
            , msg->replyto ? msg->replyto : "No Reply"
            , msg->uri
        );
        am->vm->runFimsMsg(*am->vmap, msg, am->p_fims);//fims* p_fims)
    }
    return true;
}

int main(int argc, char* argv[]) {

    //varMapUtils vm;
    system("mkdir -p run_configs");
    // this is our master data map
    varsmap vmap;
    //VarMapUtils vm;

    varmap dummyvmap;
    //varMapUtils vm;
    asset_manager* ess_man = new asset_manager("ess_man");
    varmap* amap;

    vecmap vecs;

    varsmap pmap;  // for pubs just set this up for pubs will rework
    pmap["/status/ess"] = dummyvmap;
    pmap["/config/ess"] = dummyvmap;
    pmap["/params/ess"] = dummyvmap;

    vm->setFunc(vmap, "ess", "run_init", (void*)&run_ess_init);
    vm->setFunc(vmap, "ess", "run_wakeup", (void*)&run_ess_wakeup);
    vm->setFunc(vmap, "bms", "run_init", (void*)&run_bms_init);
    vm->setFunc(vmap, "bms", "run_wakeup", (void*)&run_bms_wakeup);
    vm->setFunc(vmap, "bms", "run_asset_init", (void*)&run_bms_asset_init);
    vm->setFunc(vmap, "bms", "run_asset_wakeup", (void*)&run_bms_asset_wakeup);

    vm->setFunc(vmap, "pcs", "run_init", (void*)&run_pcr_init);
    vm->setFunc(vmap, "pcs", "run_wakeup", (void*)&run_pcr_wakeup);
    vm->setFunc(vmap, "pcs", "run_asset_init", (void*)&run_pcr_asset_init);
    vm->setFunc(vmap, "pcs", "run_asset_wakeup", (void*)&run_pcr_asset_wakeup);


    vm->setFunc(vmap, "dcr", "run_init", (void*)&run_dcr_init);
    vm->setFunc(vmap, "dcr", "run_wakeup", (void*)&run_dcr_wakeup);
    vm->setFunc(vmap, "dcr", "run_asset_init", (void*)&run_dcr_asset_init);
    vm->setFunc(vmap, "dcr", "run_asset_wakeup", (void*)&run_dcr_asset_wakeup);

    ess_man->setVmap(&vmap);
    ess_man->vm = vm;  //??
    ess_man->run_init = run_ess_init;
    ess_man->run_wakeup = run_ess_wakeup;
    const char* cfgname = "configs/test_ess.json";// TODO move this into asset manager assconfig
    ess_man->configure(&vmap, cfgname, "ess");

    //bms_man->debugConfig(bm, "Single register to change, very basic test!");
    //bm->configure("configs/test_bms_connect.json");
    ess_man->running = 1;
    fims* p_fims = new fims;
    //p_fims->Connect((char *)"fimsEssTest");
    bool connectOK = p_fims->Connect((char*)"EssTest");
    if (!connectOK)
    {
        FPS_ERROR_PRINT("%s >> unable to connect to fims server. All is lost.\n", __func__);
        running = 0;
    }

    ess_man->running = 1;
    // needed for the controller, manager and possibly assets
    ess_man->setupChannels();

    // the timer generates  channel wakes every 100 mS
    int ccnt = 0;
    vm->getVList(vecs, vmap, ess_man->amap, "ess", "Blocks", ccnt); //??
    vm->getVList(vecs, vmap, ess_man->amap, "ess", "Pubs", ccnt);
    char** subs2 = vm->getList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    if (!subs2)
    {
        subs2 = vm->getDefList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    }
    //char ** subs2 = vm->getDefList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    ess_man->run_timer(100);
    ess_man->run_fims(1500, (char**)subs2, "essMan", ccnt);
    // run the message loop eery 1.5 seconds
    asset_manager* ass_man = nullptr;
    auto ixa = vmap.find("/assets/ess");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        printf(" ESS >>We found our assets, we should be able to set up our system\n");
        for (auto iy : ixa->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                printf(" lets run assets for  [%s] from [%s]\n"
                    , iy.first.c_str()
                    , iy.second->aVal->valuestring
                );
                const char* fname = iy.second->aVal->valuestring;
                if (iy.first == "bms" || iy.first == "pcs" || iy.first == "dcr")

                {
                    const char* aname = iy.first.c_str();
                    printf(" setting up a %s manager\n", aname);
                    // TODO get the names from the config file...
                    ass_man = new asset_manager(aname);
                    void* initfcn = vm->getFunc(vmap, aname, "run_init");
                    void* wakefcn = vm->getFunc(vmap, aname, "run_wakeup");
                    //TODO
                    //void *ainitfcn = vm->getFunc(vmap, aname,"run_asset_init" );
                    void* awakefcn = vm->getFunc(vmap, aname, "run_asset_wakeup");
                    myAmInit_t myMinit = myAmInit_t(initfcn);
                    myAmWake_t myMwake = myAmWake_t(wakefcn);
                    //myAssInit_t mybmsAinit = myAssInit_t(ainitfcn);
                    myAssWake_t myAwake = myAssWake_t(awakefcn);

                    //              ess_man->run_init = myessMinit;
                    //              ess_man->run_wakeup = myessMwake;

                    ass_man->run_init = myMinit;
                    ass_man->run_wakeup = myMwake;
                    ass_man->p_fims = p_fims;


                    ass_man->setVmap(&vmap);
                    ass_man->vm = vm;  //??

                    // now get the xxx_manager to configure itsself
                    // note you need sysVec to make UI work
                    ass_man->configure(&vmap, fname, aname, nullptr, myAwake);
                    // add it to the assetList
                    ess_man->addManAsset(ass_man, aname);

                    printf(" done setting up a %s manager varsmap must be fun now\n", aname);
                    // we should be able to do things like get status from the bms_manager.
                    // first see it it unmaps correctly.
                    ass_man->running = 1;
                    ass_man->setupChannels();
                }
            }
        }
    }
    ess_man->run_manager(p_fims);

    // the message  generates test message  channel wakes every 1500 mS
    //ess_man->run_message( 1500);
    amap = ess_man->getAmap();  //may not need this 
    vm->CheckLinks(vmap, *amap, "/links/bms_1");
    ess_man->cfgwrite("run_configs/test_ess_links.json");
    int secs = 0;
    while (ess_man->running)
    {
        poll(nullptr, 0, 1000);
        secs++;
        if (secs == 30)
        {
            const char* fname = "run_configs/test_ess_after_30 seconds.json";
            // this a test for our config with links
            cJSON* cjbm = nullptr;
            cjbm = vm->getMapsCj(vmap);
            vm->write_cjson(fname, cjbm);
        }
        //test::step();

    }
    printf("ESS >> Shutting down\n");
}