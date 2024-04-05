// #include<iostream>
// #include<sstream>
// #include<string>
// #include<map>
// #include<vector>
// #include<thread>

// #include <cjson/cJSON.h>
// #include <cstring>
// #include <csignal>
// #include <stdlib.h>
// #include <math.h>
// #include <limits.h>
// #include <time.h>

#define DBL_EPSILON 2.2204460492503131e-16
//#include <fims/libfims.h>
#include "channel.h"
#include "newAv2.h"
#include "newUtils.h"

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#endif

using namespace std;

extern "C" {
typedef int (*myAvfun_t)(AssetVar* vmap, AssetVar* amap, const char* aname, fims* p_fims, AssetVar* av);
int TestFunc(AssetVar* vmap, AssetVar* amap, const char* aname, fims* p_fims, AssetVar* av);
}

//  int TestFunc(AssetVar*vmap , AssetVar* amap, const char* aname, fims*
//  p_fims, AssetVar* av)
//  {
//      cout << __func__ << " Running for av ["<<
//      av->cstring <<"] "<<endl; return 0;
//  }

extern int run;

extern channel<int> wakeChan;
// channel <schedItem*>reqChan; // anyone can post run_reqs
extern channel<AssetVar*> reqChan;       // anyone can post run_reqs
extern channel<fims_message*> fimsChan;  // anyone can post run_reqs
extern channel<char*> msgChan;           // anyone can post run_reqs

void runFimsMsg(AssetVar* av, fims_message* msg, fims* p_fims);
int runAvFunc(AssetVar* vmap, AssetVar* sv, AssetVar* av, fims* p_fims);

// add a schedItem into the list of reqs at the proper slot
int addSchedReq(alist* sreqs, AssetVar* sv)
{
    if (!sv)
    {
        FPS_ERROR_PRINT(" %s >> assetVar NOT Setup\n", __func__);
        return -1;
    }
    if (!sv->gotParam("runTime"))
    {
        FPS_ERROR_PRINT(
            " %s >> assetVar [%s] does not have a \"runTime\" param "
            "use scheduler::setupSchedItem \n",
            __func__, sv->cstring);

        return -1;
    }

    double runTime = sv->getdParam("runTime");
    char* sp = nullptr;

    auto it = sreqs->begin();
    while (it != sreqs->end())
    {
        // use the av one to make sure
        if ((*it).first->getdParam("runTime") > runTime)
        {
            sreqs->insert(it, make_pair(sv, sp));
            return sreqs->size();
        }
        it++;
    }
    sreqs->push_back(make_pair(sv, sp));
    return sreqs->size();
}

//
double getSchedDelay(AssetVar* vmap, alist* sreqs, fims* p_fims)
{
    double tNow = get_time_dbl();
    if (0)
        cout << __func__ << " Running at time " << tNow << endl;
    int runit = 1;
    AssetVar* av = NULL;
    double delay = 2.0;
    if (sreqs->size() > 0)
    {
        int oneran = 0;
        while (runit)
        {
            av = NULL;
            if (sreqs->size() > 0)
            {
                auto si = sreqs->front();
                av = si.first;
                // we could use as->runTime
                if (av->getdParam("runTime") > (tNow - 0.000001))  // allow pre sced
                {
                    av = NULL;
                    runit = 0;
                }
            }
            else
            {
                runit = 0;
            }
            if (av)
            {
                char* response = NULL;
                tNow = get_time_dbl();
                oneran++;
                double endTime = av->getdParam("endTime");
                double runTime = av->getdParam("runTime");
                bool endRun = (endTime > 0 && endTime < tNow);
                if (0)
                    FPS_ERROR_PRINT("%s >> Got one at %2.6f name [%s]  size %d \n", __func__, tNow, av->cstring,
                                    (int)sreqs->size());

                if (endRun)
                {
                    if (0)
                        FPS_ERROR_PRINT("%s >> EndSet for name [%s] at %2.3f \n", __func__, av->cstring, tNow);
                    // rreqs.erase(rreqs.begin());
                }

                if (endRun)
                {
                    runAvFunc(vmap, /*AssetVar* sv*/ nullptr, av, p_fims);

                    if (av->gotParam("response"))
                    {
                        response = (char*)av->getcParam("response");
                    }
                    if (response)
                    {
                        if (1)
                            FPS_ERROR_PRINT(
                                "          %s >> Response [%s] from %2.6f at "
                                "%2.6f elapsed mS %2.6f \n",
                                __func__, response, tNow, get_time_dbl(), (get_time_dbl() - tNow) * 1000.0);
                    }
                }
                // as->runCnt++;
                av->incParam("runCnt", 1);

                sreqs->erase(sreqs->begin());
                if (0)
                    FPS_ERROR_PRINT("%s >> repTime av %2.3f new size  %d\n", __func__,
                                    av->getdParam("repTime")
                                    //, as->repTime
                                    ,
                                    (int)sreqs->size());
                // could use as->repTime
                if (!endRun && av->getdParam("repTime") > 0.0)
                {
                    if (0)
                        FPS_ERROR_PRINT(" added av [%s] at %2.6f \n", av->cstring,
                                        av->getdParam("runTime") + av->getdParam("repTime"));
                    // Do we need a tNow check ??
                    av->incParam("runTime", av->getdParam("repTime"));
                    if ((endTime == 0) || (runTime < endTime))
                    {
                        // may need to check resched;
                        addSchedReq(sreqs,
                                    av);  // new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->args));
                    }
                    else
                    {
                        if (1)
                            FPS_ERROR_PRINT(
                                "%s >> deleted av  [%s] at %2.6f  runTime %2.3f "
                                "endTime %2.3f \n",
                                __func__, av->cstring, tNow, runTime, endTime);
                    }
                }
                if (0)
                    FPS_ERROR_PRINT(" new size 2 %d\n", (int)sreqs->size());
            }
        }
        if (0 && oneran)
            FPS_ERROR_PRINT("Done running at %2.6f   ran %d\n", tNow, oneran);
    }
    // now pick off the next delay
    if (sreqs->size() > 0)
    {
        tNow = get_time_dbl();

        auto si = sreqs->front();
        av = si.first;
        double newd = (av->getdParam("runTime") - tNow);
        delay = newd + 0.0005;
        if (0)
            FPS_ERROR_PRINT("  >>> calc new delay %3.6f = %3.6f \n", newd, delay);
    }
    if (delay < 0.0)
        delay = 0.0;
    if (delay > 2.0)
        delay = 2.0;
    // cout <<__func__ << " Completed " << delay<<endl;
    return delay;
}

// int scheduler::runChans(varsmap &vmap, schlist &rreqs, asset_manager* am)
// void schedThread(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager*
// am, fims* p_fims)
void schedThread(AssetVar* av, alist* sreqs, const char* aname, fims* p_fims)
{
    // scheduler *sc = (scheduler *) data;

    // int running = 1;
    double delay = 1.0;  // Sec
    // double ddelay = 1.0;
    int wakeup = 0;
    // schedItem *si = NULL;
    AssetVar* sv = NULL;
    double tNow = 0.0;
    fims_message* msg = NULL;
    // double tStart = sc->vm->get_time_dbl();
    char* cmsg;
    bool triggered = false;
    bool stopped = false;
    // bool stopSeen = false;

    // //setupControls(sc, vmap, rreqs, am, p_fims);

    // char * fimsname= (char*)"/sched/fims:dummy";
    // assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, NULL);

    // fimsname= (char*)"/control/gpio:runTime";
    // assetVar*runav = sc->vm->getVar(*vmap, fimsname, NULL);
    // //double dval = 0.0;

    double runTime = 0;  // runav->getdVal();
    // fimsname= (char*)"/control/gpio:stopTime";
    // assetVar*stopav = sc->vm->getVar(*vmap, fimsname, NULL);
    // runTime = runav->getdVal();
    // if(runTime < 15) {
    //     runTime = 15.0;
    //     runav->setVal(runTime);
    // }
    double stopTime = 0;  // stopav->getdVal();

    while (run)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the
        // timeout specified
        // bflag =
        wakeChan.timedGet(wakeup, delay);
        // essLogger * essLog = new essLogger(am, "gpio_test", "WakeLog", NULL);
        tNow = get_time_dbl();
        // if(0)FPS_ERROR_PRINT("%s >> Sched Tick   at
        // %2.6f\n",__func__, tNow); if(0)sc->showReqs(*rreqs);
        // stopTime = stopav->getdVal();
        // runTime = runav->getdVal();
        // if(stopTime>0 && ! stopSeen)
        // {
        //     stopSeen = true;
        //     FPS_ERROR_PRINT("%s >> Sched Tick   at %2.6f stopTime set %2.3f
        //     \n",__func__, tNow, stopTime);
        // }
        if ((runTime > 0) && (tNow > runTime) && !triggered)
        {
            // triggered = true;
            // runav->setVal(0.0);
        }
        if ((stopTime > 0) && (tNow > stopTime) && !stopped)
        {
            stopped = true;
            run = 0;
            //     sc->wakeChan.put(-1);   // but this did not get serviced immediatey
        }

        if (msgChan.get(cmsg, false))
        {
            FPS_ERROR_PRINT("%s >> message -> data  [%s] \n", __func__, cmsg);
            // use the message to set the next delay
            if (strcmp(cmsg, "quit") == 0)
            {
                FPS_ERROR_PRINT("%s >> wakeup value  %2.3f time to stop \n", __func__, tNow);
                run = 0;
            }
            free((void*)cmsg);
        }

        // handle an incoming avar run request .. avoids too many locks
        // the sv is an AssetVar
        if (reqChan.get(sv, false))
        {
            if (sv)
            {
                //         // look for id allready allocated
                FPS_ERROR_PRINT(
                    "%s >> Servicing  Sched Request %p id [%d] name[%s]  "
                    "runCnt %d repTime %2.3f at %2.3f\n",
                    __func__, sv, sv->id,
                    sv->cstring
                    //             , si->id
                    //             , si->uri
                    ,
                    sv->getiParam("runCnt"), sv->getdParam("repTime")

                                                 ,
                    tNow);

                sv->incParam("runCnt", 1);
                sv->setParam("id", sv->id);

                double runTime = tNow;
                double runAfter = sv->getdParam("runAfter");
                double refTime = sv->getdParam("refTime");
                double repTime = sv->getdParam("repTime");

                if (repTime > 0.0 && refTime > 0.0)
                {
                    double tCalc = (tNow + runAfter) - refTime;
                    int nreps = tCalc / repTime;
                    runTime = refTime + (repTime * (nreps + 1));
                }

                sv->setParam("runTime", runTime);
                string sch = "/sched/";
                sch += aname;

                av->makeLink(sch.c_str(), sv->cstring, sv);
                addSchedReq(sreqs, sv);
            }
            //         if (!si->id || (strcmp(si->id,"None")==0))
            //         {
            //             FPS_ERROR_PRINT("%s >> this is not a schedItem
            //             \n",__func__); delete si; si = NULL;
            //         }

            //         if(si)
            //         {
            //             schedItem* oldsi = sc->find(*rreqs, si);
            //             if(oldsi)
            //             {
            //                 FPS_ERROR_PRINT("%s >> this is a replacement schedItem
            //                 %p \n",__func__, oldsi);
            //                 oldsi->repTime=0; oldsi->endTime=0.1;

            //                 FPS_ERROR_PRINT("%s >>  schedItem deleted, seting
            //                 repTime to 0.0\n",__func__);
            //             }
            //             //else
            //             {
            //                 FPS_ERROR_PRINT("%s >>  schedItem added
            //                 %p\n",__func__, si);
            //                 sc->addSchedReq(*rreqs, si);
            //             }
            //         }
            //     }
            //     else
            //     {
            //         FPS_ERROR_PRINT("%s >> got NULL si request
            //         !!\n",__func__);
            //     }
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (fimsChan.get(msg, false))
        {
            if (msg)
            {
                // essLogger eLog(am, "ess_fims", msg->uri, NULL);
                // sc->vm->schedav = fimsav;
                // if(0)sc->fimsDebug1(*vmap, msg, am);

                // double tNow = vm->get_time_dbl();
                // sc->vm->syscVec = &syscVec;
                // bool runFims = true;

                runFimsMsg(av, msg, p_fims);
                // sc->vm->runFimsMsgAm(*vmap, msg, am, p_fims);
                // if(0)sc->fimsDebug2(*vmap, msg, am);
                cout << __func__ << " got a fims message " << endl;
                p_fims->free_message(msg);
            }
        }

        // //int xdelay = sc->getSchedDelay(*vmap, *rreqs);
        delay = getSchedDelay(av, sreqs, p_fims);
        // delete essLog;

        if (0)
            FPS_ERROR_PRINT("%s>> new delay = %2.6f\n", __func__, delay);
        // delay = 1000;//ddelay;
        // if ((sc->vm->get_time_dbl() - tStart)  > 15.0) running = 0;
    }

    tNow = get_time_dbl();
    FPS_ERROR_PRINT("%s >> shutting down at %2.6f\n", __func__, tNow);
    return;
}
