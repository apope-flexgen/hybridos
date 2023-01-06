
#include "asset.h"
#include <fims/libfims.h>
#include <csignal>

#include "channel.h"
#include "varMapUtils.h"
#include "scheduler.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"


// question ? how do we enable the checkuri bypass to be "gpio" and not "ess"
// answer set up vm.uriroot

// Use this to trigger a pub
// fims_send -m set -r /$$ -u /gpio/control/gpio/getPubs true

// use this to enter sim mode.
// fims_send -m set -r /$$ -u /gpio/full/config/GPIOsim 1
// trigger a sim value

// use this to change the pub rate
// fims_send -m set -r /$$ -u /gpio/full/sched/gpio '{"schedGpioRwPub":{"value":"schedGpioRwPub","repTime":20}}'


asset_manager* ess_man = nullptr;
int run_secs = 0;
//volatile 
int running = 1;

void signal_handler(int sig)
{
    running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    if (ess_man)
    {
        ess_man->running = 0;
        if(ess_man->wakeChan)ess_man->wakeChan->put(-1);
    }
    signal(sig, SIG_DFL);
}


// this is the full ess controller under new management
// functions   
extern "C++"
{
    int  TestTriggerFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int  HandleSchedItem(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    //int AddSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int EssSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every100mSP3(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int Every1000mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int SlowPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

    int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    //CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
    int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
    int HandleSchedLoad(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
    int SetupGpioSched(scheduler*sched, asset_manager* am);

}

void fimsThread(scheduler *sc, fims* p_fims)
{
    double tNow =  sc->vm->get_time_dbl();
    int tick = 0;
    fims_message* msg = p_fims->Receive_Timeout(100);
    while (*sc->run)
    {
        bool fimsok = p_fims->Connected();
        // if( tNow > 35.0)
        // {
        //     FPS_ERROR_PRINT("%s >> TEST >> FIMS closed  \n"
        //         , __func__
        //      );
        //     p_fims->Close();
        //     fimsok = p_fims->Connected();
        // }
        if(!fimsok)
        {
            FPS_ERROR_PRINT("%s >> FIMS DISCONNECTED fimsok [%s]\n"
                , __func__
                , fimsok?"true":"false"
             );
             *sc->run = 0;
             //exit(0);
        }
        if(fimsok)
        {
            msg = p_fims->Receive_Timeout(1000000);
            // just for testing
            tNow =  sc->vm->get_time_dbl();
            if(0)FPS_ERROR_PRINT("%s >>Fims Tick %d msg %p p_fims %p time %2.3f\n"
                    , __func__
                    , tick
                    , msg
                    , p_fims
                    , tNow
                    );
            tick++;
            
        }
        if (msg)
        {
            // if (strcmp(msg->method, "get") == 0)
            // {
            //     if (1)FPS_ERROR_PRINT("%s >> %2.3f GET uri  [%s]\n"
            //         , __func__, sc->vm->get_time_dbl(), msg->uri);
            // }
            if(*sc->run) 
            {
                sc->fimsChan.put(msg);
                sc->wakeChan.put(tick);   // but this did not get serviced immediatey
            }
            else
            {
                if (p_fims)p_fims->free_message(msg);
                 //p_fims->delete
            }
        }
    }
    assetVar aV;
    tNow = sc->vm->get_time_dbl();
    aV.sendEvent("GPIO_CONTROLLER", p_fims,  Severity::Info, "Gpio controller shutting down at %2.3f", tNow);
    FPS_ERROR_PRINT("%s >> fims shutting down\n"
        , __func__
        );
    FPS_ERROR_PRINT("%s >> fims shutting down\n"
        , __func__
        );
    //if(p_fims)delete p_fims;
    sc->p_fims = nullptr;
}

void setupControls(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am, fims* p_fims)
{
    char * fimsname= (char*)"/sched/fims:dummy";
    assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);
    double dval = 0.0;

    if(!fimsav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_PRINT("%s >> Created Fims var [%s:%s] \n "
            , __func__
            , av->comp.c_str()
            , av->name.c_str()
        );
        fimsav = av;
    }
    fimsname= (char*)"/control/gpio:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!runav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_PRINT("%s >> Created Runvar var [%s:%s] \n "
            , __func__
            , av->comp.c_str()
            , av->name.c_str()
        );
        runav = av;
    }
    fimsname= (char*)"/control/gpio:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!stopav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_PRINT("%s >> Created Stop var [%s:%s] \n "
            , __func__
            , av->comp.c_str()
            , av->name.c_str()
        );
        stopav = av;
    }
}

//int scheduler::runChans(varsmap &vmap, schlist &rreqs, asset_manager* am)
void schedThread(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am, fims* p_fims)
{
    //scheduler *sc = (scheduler *) data;

    //int running = 1;
    double delay = 1.0; // Sec
    int wakeup = 0;
    schedItem *si = nullptr;
    double tNow = 0.0;
    fims_message *msg = nullptr;
    //double tStart = sc->vm->get_time_dbl();
    char* cmsg;
    bool triggered = false;
    bool stopped = false;
    bool stopSeen = false;

    setupControls(sc, vmap, rreqs, am, p_fims);

    char * fimsname= (char*)"/sched/fims:dummy";
    assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);

    fimsname= (char*)"/control/gpio:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    double runTime = runav->getdVal();
    fimsname= (char*)"/control/gpio:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    runTime = runav->getdVal();
    if(runTime < 15) {
        runTime = 15.0;
        runav->setVal(runTime);
    }
    double stopTime = stopav->getdVal();
    
    while (*sc->run)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the timeout specified
        //bflag = 
        sc->wakeChan.timedGet(wakeup, delay);
        essPerf * essLog = new essPerf(am, "gpio_test", "WakePerf", nullptr);
        tNow = sc->vm->get_time_dbl();
        if(0)FPS_ERROR_PRINT("%s >> Sched Tick   at %2.6f\n",__func__, tNow);
        if(0)sc->showReqs(*rreqs);
        stopTime = stopav->getdVal();
        runTime = runav->getdVal();
        if(stopTime>0 && ! stopSeen)
        {
            stopSeen = true;
            FPS_ERROR_PRINT("%s >> Sched Tick   at %2.6f stopTime set %2.3f \n",__func__, tNow, stopTime);
        }
        if( (runTime>0) && (tNow > runTime) && !triggered) 
        {
            //triggered = true;
            runav->setVal(0.0);

        }
        if(( stopTime > 0 ) && (tNow > stopTime) && !stopped) 
        {
            stopped = true;
            *sc->run = 0;
            //     sc->wakeChan.put(-1);   // but this did not get serviced immediatey
        }

        if(sc->msgChan.get(cmsg, false)) 
        {
            FPS_ERROR_PRINT("%s >> message -> data  [%s] \n", __func__, cmsg);
            //use the message to set the next delay
            if(strcmp(cmsg,"quit")== 0)
            {
                FPS_ERROR_PRINT("%s >> wakeup value  %2.3f time to stop \n",__func__,tNow);
                *sc->run = 0;
            }
            free((void *) cmsg);
        }
    
        // handle an incoming avar run request .. avoids too many locks
        if(sc->reqChan.get(si, false)) 
        {
            if(si) 
            {
                // look for id allready allocated
                FPS_ERROR_PRINT("%s >> Servicing  Sched Request %p id [%s] (%p)  uri [%s] repTime %2.3f at %2.3f\n"
                    , __func__
                    , si
                    , si->id
                    , si->id
                    , si->uri
                    , si->repTime
                    , tNow);
                if (!si->id || (strcmp(si->id,"None")==0))
                {
                    FPS_ERROR_PRINT("%s >> this is not a schedItem \n",__func__);
                    delete si;
                    si = nullptr;
                }

                if(si)
                {
                    schedItem* oldsi = sc->find(*rreqs, si);
                    if(oldsi)
                    {
                        FPS_ERROR_PRINT("%s >> this is a replacement schedItem %p \n",__func__, oldsi);
                        oldsi->repTime=0;
                        oldsi->endTime=0.1;
                        
                        FPS_ERROR_PRINT("%s >>  schedItem deleted, seting repTime to 0.0\n",__func__);
                    }
                    //else
                    {
                        FPS_ERROR_PRINT("%s >>  schedItem added  %p\n",__func__, si);
                        sc->addSchedReq(*rreqs, si);
                    }
                }
            }
            else
            {
                FPS_ERROR_PRINT("%s >> got nullptr si request !!\n",__func__);
            }
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (sc->fimsChan.get(msg, false))
        {
            if(msg)
            {
                essPerf ePerf(am, "ess_fims", msg->uri, nullptr);
                sc->vm->schedaV = fimsav;
                if(0)sc->fimsDebug1(*vmap, msg, am);

                //double tNow = vm->get_time_dbl();
                //sc->vm->syscVec = &syscVec;
                //bool runFims = true;

                //sc->vm->runFimsMsg(*vmap, msg, sc->p_fims);
                sc->vm->schedTime = tNow;

                sc->vm->runFimsMsgAm(*vmap, msg, am, p_fims);
                if(0)sc->fimsDebug2(*vmap, msg, am);
                //sc->p_fims->free_message(msg);
            }
        }

        //int xdelay = sc->getSchedDelay(*vmap, *rreqs);
        double ddelay = sc->getSchedDelay(*vmap, *rreqs);
        delete essLog;

        if(0)FPS_ERROR_PRINT("%s>> new delay = %2.6f\n", __func__, ddelay);
        delay = ddelay;
        //if ((sc->vm->get_time_dbl() - tStart)  > 15.0) running = 0;
    }

    tNow = sc->vm->get_time_dbl();
    FPS_ERROR_PRINT("%s >> shutting down at %2.6f\n"
        , __func__
        , tNow
        );
    return;
}
// may not need this
int loadAssetManagers(varsmap &vmap, asset_manager* ess_man, std::vector<char*>* syscpVec, fims* p_fims)
{
    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* ass_man = nullptr;
    
    auto ixa = vmap.find("/config/gpio/managers");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        FPS_ERROR_PRINT("%s >> GPIO >>We found our assets, we should be able to set up our system\n",__func__);
        for (auto iy : ixa->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                FPS_ERROR_PRINT("%s >> lets run assets for  [%s] from [%s]\n"
                    , __func__
                    , iy.first.c_str()
                    , iy.second->aVal->valuestring
                );
                // this utility gets the correct file name
                // looks out for the config directory in the args...and its smart about it 
                //const char* fname = vm->getFname(iy.second->aVal->valuestring);
                const char* fname = iy.second->aVal->valuestring;

                if (iy.first == "gpio" || iy.first == "site")
                {
                    const char* aname = iy.first.c_str();
                    FPS_ERROR_PRINT("%s >> setting up manager [%s]\n",__func__, aname);
                    ass_man = new asset_manager(aname);
                    
                    ass_man->p_fims = p_fims;
                    assetVar*Av = iy.second; 
                    Av->am = ass_man;

                    ass_man->setFrom(ess_man);

                    //ccnt = 0;
                    if (1)FPS_ERROR_PRINT(" %s >> running with vmap [%p]\n", __func__, &vmap);
                    if (1)FPS_ERROR_PRINT(" %s >> syscVec size [%d]\n", __func__, (int)syscpVec->size());
                    // now get the asset_manager to configure itsself
                    if(ass_man->configure(&vmap, fname, aname, syscpVec, nullptr, ass_man) < 0)
                    {
                        FPS_ERROR_PRINT(" %s >> error in [%s] config file [%s]\n", __func__, aname, fname);
                        exit(0);
                    }

                    //int ccntam = 0;
                    //FPS_ERROR_PRINT("%s >> setting up a %s manager pubs found %d \n", __func__, aname, ccntam);

                    // add it to the assetList
                    ess_man->addManAsset(ass_man, aname);
                    FPS_ERROR_PRINT(" done setting up a %s manager varsmap must be fun now\n", aname);
                    // we should be able to do things like get status from the bms_manager.
                    // first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset(aname);
                    if (bm2)
                    {
                        FPS_ERROR_PRINT(" @@@@@@@  found %s asset manager with %d assets\n", aname, bm2->getNumAssets());
                    }
                    ass_man->running = 1;
                }
            }
        }
    }
    return 0;
}

extern "C++" {
    
    int SetupRwGpioSched(scheduler*sched, asset_manager* am);
    int GpioInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

#define LOGDIR "/var/log/gpio_controller"

//Here is a connection of nasty globals , --to be removed.

VarMapUtils vm;

varsmap vmap;
// deprecated
std::vector<char *> syscVec;

//typedef std::vector<schedItem*>schlist;
schlist schreqs;

//typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
varsmap funMap;

std::vector<char*>* syscpVec;

//typedef std::map<std::string, std::vector<std::string>*>vecmap;
vecmap vecs;

cJSON*getSchListcJ(schlist&schreqs);

cJSON*getSchList()
{
    return getSchListcJ(schreqs);
}

int main_test_new_gpio(int argc, char *argv[])
{
    syscpVec = new std::vector<char*>;
    if(0)FPS_ERROR_PRINT(" hello its me 1 %s ...\n", __func__);
    
    vm.argc = argc;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    vm.setSysName(vmap, "gpio");


    //double maxTnow = 15.0;
    // reserver some scheduler slots , it will add more if needed.
    scheduler sched("gpioSched", &running);
    sched.vm = &vm;
    sched.vm->syscVec = &syscVec;
    schreqs.reserve(64);
    FPS_ERROR_PRINT(" hello its me 2 %s ...\n", __func__);
    
    vm.vmapp   = &vmap;
    vm.funMapp = &funMap;
    int rc = 0;

    //read config dir
    if(argc > 1)
    {
        vm.setFname(argv[1]);
    }
    else
    {
        vm.setFname("configs/gpio_controller");
    }
    
    // vm.setRunLog(LOGDIR "/run_logs");
    vm.setRunCfg(LOGDIR "/run_configs");

    asset_manager* gpio_man = setupEssManager(&vm, vmap, vecs, syscpVec, "gpio", &sched);
    FPS_ERROR_PRINT(" hello its me 3 %s ...\n", __func__);

    vm.setaM(vmap,"gpio", gpio_man);
    // FlexPack thing moved here
    vm.setSysName(vmap, "gpio");
    
    // varmaputils cannot access scheduler.h yet
    gpio_man->wakeChan = &sched.wakeChan;
    gpio_man->reqChan = (void*)&sched.reqChan;
    sched.am = gpio_man;

    // set up functions
    SetupRwGpioSched(&sched, gpio_man);

    if (1)FPS_ERROR_PRINT(" %s >> gpio start load controller \n", __func__); 
    rc = loadEssConfig(&vm, vmap, "gpio_controller.json", gpio_man, &sched);
    if (1)FPS_ERROR_PRINT(" %s >> gpio done load controller rc %d \n", __func__, rc ); 

    //vm.showList(mysubs,"ess", mysublen);
    //return 0;

    //syscvec orders the comps for pub to UI interface
    if (1 && syscpVec->size() == 0)
    {
        syscpVec->push_back((char*)"gpio/summary");
 
        FPS_ERROR_PRINT(" %s >> HACK HACK to syscVec size %d \n", __func__,  (int)syscpVec->size());
    }
    
    // NOTE use syscpVec from now onwards
    if (1)FPS_ERROR_PRINT(" %s >> ess_man >> syscVec size [%d]\n", __func__, (int)syscpVec->size());

    // load up the other managers
    // next is the site controller
    // This loads in the system_controller interface 
    if(0){
        rc = loadSiteManager(&vm, vmap, "/config/gpio_server","gpio");
        if(rc<0)
        {
            if (1)FPS_ERROR_PRINT(" %s >> gpio_man >> Error in setting up site manager \n", __func__); 
            return 0;
        }
    }

    // setup fims with Subs from config
    int ccntam = 0;
    vm.getVList(*gpio_man->vecs, vmap, gpio_man->amap, "gpio", "Subs", ccntam);
    FPS_ERROR_PRINT("%s >> setting up [%s] manager Subs found %d rc %d \n", __func__, "gpio", ccntam, rc);
    //vm.showvecMap(*ess_man->vecs, "Subs");
    int mysublen;
    char** mysubs = vm.getVecListbyName(*gpio_man->vecs, "Subs", mysublen);
    FPS_ERROR_PRINT("%s >> recovered  [%s]  Subs %p found %d \n", __func__, "gpio", mysubs, mysublen);
    fims* p_fims = nullptr;
    if(mysublen> 0)
    {
        if (1)FPS_ERROR_PRINT(" %s >> ess_man >> Subs found in ess config \n", __func__); 

        p_fims = sched.fimsSetup ((const char**)mysubs, mysublen, "GpioSched", vm.argc);
        vm.clearList(mysubs, "gpio", mysublen);
        if (1)FPS_ERROR_PRINT(" %s >> ess_man >> p_fims %p  sched %p\n"
        , __func__
        , p_fims
        , sched.p_fims
        ); 
    }
    else
    {
       if (1)FPS_ERROR_PRINT(" %s >> ess_man >> No Subs found in gpio config \n", __func__); 
       return 0;
    }

    //p_fims = sched.p_fims;
    gpio_man->p_fims = p_fims;
    if (1)FPS_ERROR_PRINT(" %s >> ess_man >> p_fims %p\n", __func__, p_fims); 
    //return 0;
    // next we load the asset managers
    //int rc = 
    // may not need these
    //     loadAssetManagers(vmap, ess_man, syscpVec, p_fims);
    // show subs vecs and dump config to an initial file.
    debugSystemLoad(&vm, vmap, vecs, syscpVec, "gpio", LOGDIR);
    
    FPS_ERROR_PRINT("Gpio >>Setting vlinks\n");
    vm.setVLinks(vmap,"gpio");
    FPS_ERROR_PRINT("Gpio >>Done Setting vlinks\n");

    // no need to load schedule thie init does it all for us.

    //if (1)FPS_ERROR_PRINT(" %s >> gpio start load schedule \n", __func__); 
    //rc = loadEssConfig(&vm, vmap, "gpio_schedule.json", gpio_man, &sched);
    //if (1)FPS_ERROR_PRINT(" %s >> gpio done  load schedule rc %d \n", __func__, rc); 

    double tNow = vm.get_time_dbl();
    assetVar *aV = vmap["/config/gpio"]["gpios"];
    if (1)FPS_ERROR_PRINT(" %s >> gpio aV %p \n", __func__, aV); 
    if(aV)
    {

        aV->sendEvent("GPIO_CONTROLLER", p_fims,  Severity::Info, "Gpio starting  at %2.3f", tNow);
        GpioInit(vmap, gpio_man->amap, "gpio", p_fims, aV);
    }
    else
    {
        if (1)FPS_ERROR_PRINT(" %s >> gpio aV error %p \n", __func__, aV); 
        return 0;

    }
//    int GpioInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)

    std::thread fThread(fimsThread, &sched, p_fims);
    std::thread sThread(schedThread, &sched, &vmap, &schreqs, gpio_man, p_fims);

    if(fThread.joinable())
        fThread.join();
    FPS_ERROR_PRINT("%s >> fims thread done .... \n", __func__);
    if(sThread.joinable())
        sThread.join();
    FPS_ERROR_PRINT("%s >> sched thread done  .... \n", __func__);

    vm.clearVmap(vmap);
    
    vm.amMap.clear();

    //delete ess_man->p_fims;
    gpio_man->p_fims = nullptr;
    //vecm[name] = (new std::vector<T>);
    for (auto xx:vecs)
    {
        xx.second->clear();
        delete xx.second;
    }
    vecs.clear();
    delete gpio_man;
    int idx = 0;
    FPS_ERROR_PRINT("%s >> things found in syscpVec  .... \n", __func__);
    for ( auto x: *syscpVec)
    {
        FPS_ERROR_PRINT(" syscpVec[%d] [%s]\n", idx++, x);
    }
    FPS_ERROR_PRINT("%s >>  deleting  remaining sched items .... \n", __func__);
    for ( auto sr : schreqs)
    {
        //FPS_ERROR_PRINT("%s >>  we got a schedItem %p not deleting it for now .... \n", __func__, sr);
        sr->show();
        delete sr;        
    }
    schreqs.clear();

    vm.amMap.clear();
    vm.aiMap.clear();
    if(p_fims)delete p_fims;

    return 0;
}

// Please do NOT remove these, they are global extern variables for the GPIO controller.
namespace flex
{
    const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

int main(int argc, char *argv[])
{
    // setting up stdout console sink:
    auto std_out = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    std_out->set_level(spdlog::level::debug);

    // setting up stderr console sink:
    auto std_err = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
    std_err->set_level(spdlog::level::err);

    // setting the default logger to log to both:
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("", std::initializer_list<spdlog::sink_ptr>{std::move(std_out), std::move(std_err)}));

    // setting up the default console logger for the GPIO Controller from now on
    // offers safety and extensibilty, even more safety once we move to C++17 and up
    spdlog::set_level(spdlog::level::debug); // please change "tweakme.h" to get this

    // setting up the elapsed time formatter for the global logger (similar to the way we have it for ESS Controller):
    // NOTE: please refer to this for help -> https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<elapsed_time_formatter>('*').set_pattern("[%-8*] [%^%-8l%$] [%-15!!] %v");
    spdlog::set_formatter(std::move(formatter));

    return  main_test_new_gpio(argc, argv);
}