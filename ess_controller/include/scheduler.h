#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <vector>
#include <string>
#include <thread>

#include <cjson/cJSON.h>

#include "assetVar.h"
#include "channel.h"
#include "varMapUtils.h"

class schedItem
{
public:
    schedItem();
    ~schedItem();
    int runItem(assetVar* _av = nullptr);
    int runSched(double tNow = 0.0);
    int reSched(double tNow = 0.0);
    void setUp(const char* _id, const char* _aname, const char* _uri, const char* _func, double _refTime,
               double _runTime, double _repTime, double _endTime, char* _targAv);

    void setFromAv(assetVar* inav);
    cJSON* getCj();
    void show();

    // new param fast if missing or zero go slow no cache
    //                if 1 re eval cache
    //                if 2  run fast except for endTime
    assetVar* av;
    //    varmaputils* vmap;  pull it from the asset manager
    double refTime;     // used to set priority  actual run time is tNow + (refTime
                        // *runCnt)
    double runTime;     // earliest time to run
    double repTime;     // if non zero causes repeated runs
    double endTime;     // last time to run 0 to disable
    int runCnt;         // counts how many times this item has run
    char* id;           // used to identify the schedItem
    char* aname;        // asset name to use
    char* targAv;       // the name of the target assetVar
    char* func;         // (optional) the name of thefunction
    assetVar* targaVp;  // target assetvar *
    void* fcnptr;       // used in fast mode pointer to function
    asset* ai;          // used in fast mode
    asset_manager* am;  // used in fast mode
    bool runt;          // infast mode just send time to targaVp
    char* targ;         // target for runt
    int fast;
    char* uri;  // comp:name
    bool thread;
    double thread_runTime;
    int thread_retSig;
    bool update;
    bool updone;
};

typedef std::vector<schedItem*> schlist;
typedef void (*vLoopc)(void* args);

class scheduler
{
public:
    scheduler(const char* _name, int* _run);
    // {
    //     name = _name;
    //     fLog = nullptr;
    //     uriLog = nullptr;
    //     run = _run;
    //     p_fims = nullptr;
    //     schreqs = nullptr;
    //     vm = nullptr;
    //     vmap = nullptr;
    // };

    ~scheduler();

    channel<int> wakeChan;
    channel<schedItem*> reqChan;      // anyone can post run_reqs
    channel<fims_message*> fimsChan;  // anyone can post run_reqs
    channel<char*> msgChan;           // anyone can post run_reqs

    fims* p_fims;

    int* run;
    std::string name;

    std::thread fThread;
    std::thread sThread;
    std::thread mThread;

    VarMapUtils* vm;
    asset_manager* am;  // controlling asset_manager
    assetVar* schedaV;  // controlling assetVar

    // varsmap* vmap;

    void showReqs(schlist& rreqs);
    void clearReqs(schlist& rreqs);

    int addSchedReq(schlist& rreqs, schedItem* as);
    double getSchedDelay(varsmap& vmap, schlist& rreq);
    fims* fimsSetup(const char* subs[], int sublen, const char* name, int argc = 0);
    void fimsThread(void* data);
    void schedThread(void* data);
    void msgThread(void* data);
    // int runFimsThread(int delay);
    // int runSchedThread(int delay);
    // int runMsgThread(int delay);
    int addSchedItem(varsmap& vmap, varmap& amap, const char* aname, assetVar* av);
    int fimsDebug1(varsmap& vmap, fims_message* msg, asset_manager* am);
    int fimsDebug2(varsmap& vmap, fims_message* msg, asset_manager* am);
    void addSaction(varsmap& vmap, const char* amap, const char* func, const char* var, const char* fvar,
                    asset_manager* am, cJSON* cja);
    int setupSchedItemActions(varsmap& vmap, asset_manager* am, const char* uri, const char* name, const char* when,
                              const char* act, cJSON* cja);
    schedItem* setupSchedItem(varsmap& vmap, asset_manager* am, const char* uri, const char* name, const char* id,
                              const char* aname, double refTime, double runTime, double repTime);
    assetVar* setupSchedVar(varsmap& vmap, asset_manager* am, const char* uri, const char* name, const char* id,
                            const char* aname, double refTime, double runTime, double repTime);
    int activateSchedItem(varsmap& vmap, const char* uri, const char* auri, const char* name,
                          const char* cjtmp = nullptr);
    int setupSchedFunction(varsmap& vmap, const char* uri, const char* oper, const char* func, asset_manager* am);
    int runChans(varsmap& vmap, schlist& rreqs, asset_manager* am);
    // int addSchedReq(schlist& rreq, assetVar *av);
    // int addSchedReq(schlist& rreq, assetVar* av,double tshot, double trep);
    schedItem* find(schlist& rreqs, schedItem* si);

    essPerf* fLog;
    essPerf* uriLog;
};

int TestRunSchedOpts(varsmap& vmap, varmap& amap, const char* _aname, fims* p_fims, assetVar* aV);

std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace);

#endif
