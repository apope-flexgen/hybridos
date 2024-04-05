#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"
#include "testUtils.h"

VarMapUtils defvm;
VarMapUtils* vm = &defvm;
int running = 0;

// this is a map of local variables as known to the asset
varmap* amap;
void changeTestValue(int newValue) {}
void run_bms_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager\n", __func__);
}

// We have presently have two wakeup messages sent
bool run_bms_wakeup(asset_manager* am, int wakeup)
{
    // char * item3;
    fims_message* msg;
    if (wakeup != 0)
    {
        if (1)
            FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__,
                            am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    }
    if (am->reload != 2)
    {
        if (1)
            FPS_ERROR_PRINT(
                "%s %2.3f >>>>>>>>>>>>>>>>>>>>> running init for %s "
                "Manager wval %d \n",
                __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
        am->run_init(am);
    }

    if (wakeup == -1)
    {
        // quit
        FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, am->name.c_str());
        running = 0;
        // break;//
        return false;
    }

    if (wakeup == 0)
    {
        if (0)
            std::cout << am->name << ">> MANAGER LOOP  process another channel\n";
        // break;
    }
    // now process all the events

    // Do the manager first
    if (wakeup == WAKE_LEVEL1)
    {
        am->vm->setTime();
    }

    if (wakeup == WAKE_LEVEL_PUB)
    {
        if (1)
            std::cout << am->name << " >>publish status\n";
        // am->vm->vListPartialSendFims(*am->vmap, *am->pmap, "pub", am->p_fims);
    }
    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        if (0)
            FPS_ERROR_PRINT(
                " %s >> >>>>>>>>>>>  %2.3f  %s  >>>>BEFORE FIMS MESSAGE  "
                "method [%s] replyto[%s]  uri [%s]\n",
                __func__, vm->get_time_dbl(), am->name.c_str(), msg->method, msg->replyto ? msg->replyto : "No Reply",
                msg->uri);
        am->vm->runFimsMsg(*am->vmap, msg, am->p_fims);  // fims* p_fims)
    }
    return true;
}

int main(int argc, char* argv[])
{
    // varMapUtils vm;
    system("mkdir -p run_configs");
    // this is our main data map
    varsmap vmap;
    // VarMapUtils vm;

    asset_manager* bms_man = new asset_manager("bms_man");
    varmap* amap;

    asset* bm;
    vecmap vecs;

    bms_man->setVmap(&vmap);
    bms_man->vm = vm;  //??
    bms_man->run_init = run_bms_init;
    bms_man->run_wakeup = run_bms_wakeup;
    bm = bms_man->addInstance("bms_1");

    amap = bm->getBmap();

    bm->setVmap(&vmap);

    bms_man->debugConfig(bm, "Single register to change, very basic test!");
    bm->configure("configs/test_bms_connect.json");
    bms_man->running = 1;
    fims* p_fims = new fims;

    // p_fims->Connect((char *)"fimsEssTest");
    bool connectOK = p_fims->Connect((char*)"FimsBMsTest");
    if (!connectOK)
    {
        FPS_ERROR_PRINT("%s >> unable to connect to fims server. All is lost.\n", __func__);
        running = 0;
    }

    bms_man->p_fims = p_fims;
    bms_man->running = 1;
    // eeded for the controller, manager and possibly assets
    bms_man->setupChannels();

    // the timer generates  channel wakes every 100 mS
    int ccnt = 0;
    char** subs2 = vm->getDefList(vecs, vmap, bms_man->amap, "bms", "Subs", ccnt);
    bms_man->run_timer(100);
    bms_man->run_fims(1500, (char**)subs2, "essMan", ccnt);
    // run the message loop eery 1.5 seconds

    // the message  generates test message  channel wakes every 1500 mS
    // ess_man->run_message( 1500);
    vm->CheckLinks(vmap, *amap, "/links/bms_1");
    bms_man->cfgwrite("run_configs/bms_man_bms_connect_after_links.json");
    int secs = 0;
    while (bms_man->running)
    {
        poll(nullptr, 0, 1000);
        secs++;
        if (secs == 30)
        {
            const char* fname = "run_configs/bms_4_after_30 seconds.json";
            // this a test for our config with links
            cJSON* cjbm = nullptr;
            cjbm = vm->getMapsCj(vmap);
            vm->write_cjson(fname, cjbm);
        }
    }
    printf("ESS >> Shutting down\n");
}