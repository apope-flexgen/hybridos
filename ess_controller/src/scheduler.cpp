/*

new Scheduler
p. wilshire
02/09/2021

How it works.
The system uses ScheduleItems as points in time when we need to do something.
Here is the structure
    double refTime;     // used to set priority  actual run time is tNow +
(refTime *runCnt) double runTime;     // earliest time to run double repTime;
// if non zero causes repeated runs int runCnt;         // counts how many times
this item has run char *id;           // used to identify the schedItem char
*aname;        // asset name to use char* targname;     // the name of the
target assetVar char *fcnname;      // (optional) the name of the assetvar to be
passed to the function assetVar* targAv;   // target assetvar assetVar* fcnav;
// (optional) asset var to be used by the target

A list of thse items is kept by the scheduler (schliist) with the first one to
run ( lowest run time ) at the head of the list.

The scheduler runs in a the main schedule thread. This thread uses channels to
get notifications of: fims messages arriving new sched items to be added to the
list other messages ( not used yet)

When the scheduler runs it looks for any item on the (top of the) schlist and
sends the current time to the target av. It will also set the Param of the fcnAV
if specified. The target av will get the new time message and run any functions
associated with the target aV The fcnAv can be used to allow the same target av
(and function) to run with different assetVars.

As each item on the schlist is processed it is checked for a (repTime) repeat
time. If specified the repTime is added to the runTime and the schedItem is
placed back in the schlist.

As the scheduler works down the list it finally comes to the end or to a
schedItem with a time in the future. If there is nothing on the list the sched
delay is set to 1 second... If there is another item in the list , the sched
delay is set to the difference between tNow and the next runTime.

The loop is rerun with a channel timedGet using the delay as a max time to wait
for an incoming wakeup on the wake channel. If nothing else sends a wakeup then
this would be time to run the next entry.

An exernal process can decide to add an item to the schlist. It does that by
creating the schedItem and sending that to the reqChan together with a wakeup to
the wakeChan. The scheduler will wake up immediately and put the schedItem into
the schlist and then run any past due items or reset the wait delay.


A fims thread is also set up to continually read fims messages.
As each message is received it is dequeed from the incoming fims socket and sent
to the fimsChan together with a wakeReq to cause the scheduler to immediately
wake up and process the fims message.

All message handling and schedule operations are handled by a single thread. So
no locking is required on the variable map.

The system sets up its "default" or hard coded operation before reading in any
config files. The incoming config files can change this default operation any
incoming fims message can also change the scheduler operation.

There are a number of "helper" functions used to facilitate this setup.

1/ The scheduler runs by sending a message ( current time) to a designated
target assetVar. That assetVar needs to be present if the schedule operation
wants to do anything. 2/ Having set up the assetVar it needs to be given some
actions to perform when the value is set. 3/ Having set up the target assetVar
the scheduler needs to be told that this is a schedule point with an expected
runTime.


Here is an example sequence using the helper functions.

int SetupDefaultSched(scheduler*sched, asset_manager* am)
{

    am->vm->setFunc(*am->vmap, essName, "CheckMonitorVar",
(void*)&CheckMonitorVar); am->vm->setFunc(*am->vmap, essName, "EssSystemInit",
(void*)&EssSystemInit); am->vm->setFunc(*am->vmap, essName, "Every1000mS",
(void*)&Every1000mS); am->vm->setFunc(*am->vmap, essName, "Every100mSP1",
(void*)&Every100mSP1); am->vm->setFunc(*am->vmap, essName, "Every100mSP2",
(void*)&Every100mSP2); am->vm->setFunc(*am->vmap, essName, "Every100mSP3",
(void*)&Every100mSP3); am->vm->setFunc(*am->vmap, essName, "FastPub",
(void*)&FastPub); am->vm->setFunc(*am->vmap, essName, "SlowPub",
(void*)&SlowPub);

    return 0;
}


Here is the list of sched items
 sched id [EssSystemInit] count [0] aname [ess] uri [/sched/ess:essSystemInit]
refTime 0.200 runTime 0.200000 repTime 0.000 sched id [EverySecond]   count [0]
aname [ess] uri [/sched/ess:every1000mS]   refTime 0.250 runTime 0.250000
repTime 1.000 sched id [Every100mS_P1] count [0] aname [ess] uri
[/sched/ess:every100mSP1]  refTime 0.251 runTime 0.251000 repTime 0.100 sched id
[Every100mS_P2] count [0] aname [ess] uri [/sched/ess:every100mSP2]  refTime
0.252 runTime 0.252000 repTime 0.100 sched id [Every100mS_P3] count [0] aname
[ess] uri [/sched/ess:every100mSP3]  refTime 0.253 runTime 0.253000 repTime
0.100 sched id [ FastPub]      count [0] aname [ess] uri [/sched/ess:fastPub]
refTime 0.254 runTime 0.254000 repTime 0.050 sched id [ slowPub]      count [0]
aname [ess] uri [/sched/ess:slowPub]       refTime 0.255 runTime 0.255000
repTime 2.000




wip for final integration.....



leading nicely into sequencing...
*/

#include <thread>

#include "asset.h"
#include "channel.h"
#include "ess_utils.h"
#include "scheduler.h"
#include "varMapUtils.h"
#include <fims/libfims.h>

#include "formatters.hpp"

VarMapUtils vm;
varsmap vmap;
std::vector<char*> syscVec;

// use av->addSchedReq(schAvList*avlist)

schedItem::schedItem()
{
    av = nullptr;  // old target av
    // comp = nullptr;         // deprecated target av comp
    // name = nullptr;         // deprecated target av name now combined in
    // targname
    refTime = 0.0;  // used to set priority  actual run time is tNow + (refTime *runCnt)
    uri = nullptr;
    runTime = 0.0;    // earliest time to run
    repTime = 0.0;    // if non zero causes repeated runs
    endTime = 0.0;    // if non zero stops run at end time
    runCnt = 0;       // counts how many times this item has run
    id = nullptr;     // used to identify the schedItem
    aname = nullptr;  // asset name to use
    // targname = nullptr;     // the name of the target assetVar
    // fcnname = nullptr;      // (optional) the name of the assetvar to be passed
    // to the function
    targAv = nullptr;  // target assetvar
    // fcnav = nullptr;        // (optional) asset var to be used by the target
    func = nullptr;
    fast = 0;
    targaVp = nullptr;  // target assetvar *
    fcnptr = nullptr;   // used in fast mode pointer to function
    ai = nullptr;       // used in fast mode
    am = nullptr;       // used in fast mode
    runt = false;       // infast mode just send time to targaVp
    targ = nullptr;
    thread = false;
    update = false;
    updone = false;
}

schedItem::~schedItem()
{
    if (0)
    {
        FPS_PRINT_INFO("delete sched item {}", fmt::ptr(this));
        show();
    }
    if (id)
        free(id);
    if (aname)
        free(aname);
    if (uri)
        free(uri);
    if (targAv)
        free(targAv);
    if (func)
        free(func);
    if (0)
        FPS_PRINT_INFO("deleted sched item {}", fmt::ptr(this));
}

// if tNow > 0 then pick the resched time after tNow
// else just repeat once.
// the refTime allows us to configure relative priorities
int schedItem::reSched(double tNow)
{
    if (repTime == 0.0)
        return runCnt;
    if (tNow > 0.0)
    {
        runTime = refTime * runCnt;
        while (runTime < tNow)
        {
            runTime += repTime;
            runCnt++;
        }
    }
    else
    {
        runTime += repTime;
    }
    return runCnt;
}

void schedItem::show()
{
    FPS_PRINT_INFO(
        "sched id [{}] count [{}] aname [{}] uri [{}] refTime {:2.3f} "
        "repTime {:2.3f} endTime {:2.3f}",
        id ? id : "we-have-no-id", runCnt, aname, uri, refTime, repTime, endTime);
}

// void schedItem::setUp(const char * _id, const char* _aname, const char *
// _comp, const char* _name, double _refTime,
//    double _runTime, double _repTime, double _endTime, char*targaV)
// what about the function
void schedItem::setUp(const char* _id, const char* _aname, const char* _uri, const char* _func, double _refTime,
                      double _runTime, double _repTime, double _endTime, char* _targAv)
{
    if (aname)
        free(aname);
    if (uri)
        free(uri);
    if (targAv)
        free(targAv);
    targAv = nullptr;
    if (func)
        free(func);
    func = nullptr;

    targaVp = nullptr;  // target assetvar *
    fcnptr = nullptr;   // used in fast mode pointer to function
    ai = nullptr;       // used in fast mode
    am = nullptr;       // used in fast mode
    runt = false;       // infast mode just send time to targaVp
    targ = nullptr;
    fast = 0;

    if (id)
        free(id);
    id = strdup(_id);
    aname = strdup(_aname);
    uri = strdup(_uri);
    if (_targAv)
        targAv = strdup(_targAv);
    if (_func)
        func = strdup(_func);
    refTime = _refTime;
    runTime = _runTime;
    repTime = _repTime;
    endTime = _endTime;
    if (0)
        FPS_PRINT_INFO("setup a sched item {}", fmt::ptr(this));
    if (0)
        show();

    return;
}

void schedItem::putOnScheduler(std::string func, double _refTime, double _runTime, double _repTime, std::string targURI,
                               assetVar* aV)
{
    // create a new ID based on the comp and name of our aV
    std::string schedID = aV->comp + ":" + aV->name + "_schedItem";

    setUp((char*)schedID.c_str(), (char*)aV->am->name.c_str(), (char*)aV->comp.c_str(), (char*)func.c_str(), _refTime,
          _runTime, _repTime, 0, (char*)targURI.c_str());

    // configure the params the assetVar needs to run on the scheduler
    av = aV;                           // our sched item should run on this av
    av->setParam("runTime", _runTime);  // this tells the scheduler when to run
    av->setParam("repTime", _repTime);  // this is how often our func will run
    av->setParam("update", true);      // this flag allows the scheduler to run faster after its first pass

    // put our new schedItem on the scheduler and wake it up
    auto reqChan = (channel<schedItem*>*)aV->am->reqChan;
    reqChan->put(this);
    if (aV->am->wakeChan)
    {
        aV->am->wakeChan->put(0);
    }
}

scheduler::scheduler(const char* _name, int* _run)
{
    FPS_PRINT_INFO("hello scheduler [{}]", _name);
    name = _name;
    fLog = nullptr;
    uriLog = nullptr;
    run = _run;
    p_fims = nullptr;
    vm = nullptr;
}

scheduler::~scheduler()
{
    int ival = 21;
    schedItem* si;
    FPS_PRINT_INFO("shutdown scheduler start [{}]", name);
    while (wakeChan.get(ival, false))
    {
        if (0)
            FPS_PRINT_INFO("we got a wakeNotice ... {}", ival);
    }
    while (reqChan.get(si, false))
    {
        FPS_PRINT_INFO("we found a schedItem {} still on the reqChan. deleting it", fmt::ptr(si));
        si->show();
        delete si;
    }
    FPS_PRINT_INFO("shutdown scheduler [{}] done", name);
}

void scheduler::showReqs(schlist& rreqs)
{
    for (auto sr : rreqs)
    {
        sr->show();
    }
    return;
}

schedItem* scheduler::find(schlist& rreqs, schedItem* si)
{
    for (auto sr : rreqs)
    {
        if (strcmp(sr->id, si->id) == 0)
        {
            return sr;
        }
    }
    return nullptr;
}

void scheduler::clearReqs(schlist& rreqs)
{
    FPS_PRINT_INFO("reqs at end size {}", rreqs.size());
    for (auto sr : rreqs)
    {
        FPS_PRINT_INFO("we got a schedItem {}  And deleting it for now ...", fmt::ptr(sr));
        sr->show();
        delete sr;
    }
    return;
}

// followed by this config items
// set /schedule/ess:add_item
//                '{"id":"some_id",
//                 "aname":"asset_name",
//                 "var":"/sched/ess:every100mS",
//                 "value":"/sched/ess:every100mS",    // some value the var is
//                 used "runTime":0.5, "repTime":0.1
//                 }'
// see also
// int setup_sched_function(vmap, "/schedule/ess", "add_item",
// "add_sched_item",am)

// we may need to do this in the fimsthread !!otherwise gdb fails
fims* scheduler::fimsSetup(const char* subs[], int sublen, const char* name, int argc)
{
    // int sublen = sizeof subs / sizeof subs[0];
    FPS_PRINT_INFO("FIMSSETUP size of subs = {}", sublen);
    int attempt = 0;
    p_fims = new fims();

    while (!p_fims->Connect((char*)name))
    {
        if (attempt == 0)
        {
            if (argc != 2)
                system("/usr/local/bin/fims/fims_server&");
        }
        else
        {
            poll(nullptr, 0, 1000);
        }
        FPS_PRINT_INFO("name {} waiting to connect to FIMS, attempt {}", name, ++attempt);
    }
    FPS_PRINT_INFO("name {} connected to FIMS at attempt {}", name, attempt);

    bool subok = p_fims->Subscribe(subs, sublen);
    bool conok = p_fims->Connected();
    FPS_PRINT_INFO("name {} subscribed to FIMS [{}] connected [{}]", name, subok, conok);

    return p_fims;
}

// -m set -u /schedule/ess '{"every100mS":{"value":0,"id":"some_id",
// "aname":"asset_name", "refTime":0.5,"runTime":0.5,"repTime":0.1}}' we'll have
// to add actions to this item to make it do anything
//    "actions":{"onSet":[{"func":[{"amap": essName,"func":
//    "add_sched_item","var":"/status/ess/some_var"}]}]}
// example use:
//    cJSON * cja = cJSON_CreateArray();
//    addSaction(cja,essName,"CheckMonitorVar","/status/ess/maxTemp", cjacts);
//    setupSchedItemActions(vmap,"/schedule/ess","every100mS",cja);
//    cJSON_Delete(cja);
// confirmed
// possibly deprecated
void scheduler::addSaction(varsmap& vmap, const char* amap, const char* func, const char* var, const char* fvar,
                           asset_manager* am, cJSON* cjacts)
{
    cJSON* cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj, "amap", amap);
    cJSON_AddStringToObject(cj, "func", func);
    if (fvar)
        cJSON_AddStringToObject(cj, "var", fvar);
    else
        cJSON_AddStringToObject(cj, "var", var);

    cJSON_AddItemToArray(cjacts, cj);
    // const char *myvar = nullptr;

    assetVar* av = vm->getVar(vmap, var, nullptr);
    double dval = 0.0;

    if (!av)
    {
        assetUri my(var, nullptr);

        av = new assetVar(my.Var, my.Uri, dval);
        vmap[av->comp][av->name] = av;
        FPS_PRINT_INFO("Created Function var [{}:{}]", av->comp, av->name);
    }
    else
    {
        FPS_PRINT_INFO("Used Function var [{}:{}]", av->comp, av->name);
    }
    if (am)
        av->am = am;
}

// possibly deprecated use HandleSchedItem
int scheduler::setupSchedItemActions(varsmap& vmap, asset_manager* am, const char* uri, const char* name,
                                     const char* when, const char* act, cJSON* cjacts)
{
    char* acts = cJSON_PrintUnformatted(cjacts);
    char* rbuf;
    // uri example /schedule/ess
    // when = onSet
    // act = "func"
    asprintf(&rbuf,
             "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
             "{\"%s\":{\"value\":0.0, \"actions\":{\"%s\":[{\"%s\":%s}]}}}}",
             uri, name, when, act, acts);
    fims_message* msg = am->vm->bufferToFims((const char*)rbuf);
    {
        // debug code
        if (msg)
        {
            FPS_PRINT_INFO("fims nfrags {}, fims pfrags [1] [{}] fims method [{}] body [{}]", msg->nfrags,
                           msg->nfrags > 0 ? msg->pfrags[1] : "no pfrags", msg->method ? msg->method : "no method",
                           msg->body ? msg->body : "no body");
        }
        else
        {
            FPS_PRINT_INFO("no message from >>>{}<<<", rbuf);
            free(rbuf);
            free(acts);
            return 0;
        }
    }
    if (msg)
    {
        cJSON* cj = cJSON_CreateObject();
        am->vm->processFims(vmap, msg, &cj);
        char* res = cJSON_Print(cj);
        {
            if (res)
            {
                FPS_PRINT_INFO("got res [{}]", res);
                free(res);
            }
        }
        cJSON_Delete(cj);
        am->vm->free_fims_message(msg);
    }
    free(rbuf);
    free(acts);
    return 0;
}

// add a schedItem into the list of reqs at the proper slot
int scheduler::addSchedReq(schlist& sreqs, schedItem* si)
{
    assetVar* av = si->av;
    if (!av)
    {
        FPS_PRINT_ERROR("assetVar NOT Setup", NULL);
        return -1;
    }
    if (si->thread)
    {
        av->setParam("runTime", si->thread_runTime);
    }

    if (!av->gotParam("runTime"))
    {
        FPS_PRINT_ERROR(
            R"(assetVar [{}] does not have a "runTime" param use scheduler::setupSchedItem)", av->name);

        return -1;
    }

    double tshot = av->getdParam("runTime");

    auto it = sreqs.begin();
    while (it != sreqs.end())
    {
        // use the av one to make sure
        if ((*it)->av->getdParam("runTime") > tshot)
        {
            sreqs.insert(it, si);
            return sreqs.size();
        }
        it++;
    }
    sreqs.push_back(si);
    return sreqs.size();
}

// confirmed
// todo possibly get a different vmap from the scheditem
double scheduler::getSchedDelay(varsmap& vmap, schlist& rreqs)
{
    double tNow = vm->get_time_dbl();
    int runit = 1;
    int fast = 0;
    int newfast = 0;
    schedItem* as;
    assetVar* av = nullptr;

    double delay = 2.0;
    if (rreqs.size() > 0)
    {
        int oneran = 0;
        while (runit)
        {
            as = nullptr;
            av = nullptr;
            if (rreqs.size() > 0)
            {
                as = rreqs.front();
                av = as->av;
                // we could use as->runTime
                if (av->getdParam("runTime") > (tNow - 0.000001))  // allow pre sced
                {
                    as = nullptr;
                    av = nullptr;
                    runit = 0;
                }
            }
            else
            {
                runit = 0;
            }
            if (av)
            {
                char* response = nullptr;
                tNow = vm->get_time_dbl();
                oneran++;
                double endTime = av->getdParam("endTime");
                bool endRun = (endTime > 0 && endTime < tNow);
                if (0)
                    FPS_PRINT_INFO("Got one with name [{}:{}] size {}", av->comp, av->name, rreqs.size());

                if (endRun)
                {
                    if (0)
                        FPS_PRINT_INFO("EndSet for name [{}:{}]", av->comp, av->name);
                    // rreqs.erase(rreqs.beginif ());
                }
                if (av->gotParam("fast"))
                {
                    newfast = av->getiParam("fast");
                    if (newfast != fast)
                    {
                        fast = newfast;
                    }
                }

                if (fast == 1)  // allow reset
                {
                    as->fast = 1;
                }

                if (!endRun && (!av->gotParam("enabled") || av->getbParam("enabled")))
                {
                    vm->schedaV = av;
                    vm->schedTime = tNow;
                    vm->schedActions = 0;

                    assetVar* targav = av;
                    if (as->fast == 2)
                    {
                        if (as->fcnptr && as->targaVp)
                        {
                            if (as->thread)
                            {
                                as->targaVp->setParam("runTime", as->thread_runTime);
                                as->targaVp->setParam("retSig", as->thread_retSig);
                            }
                            myAvfun_t amFunc = reinterpret_cast<myAvfun_t>(as->fcnptr);
                            if (as->am)
                            {
                                amFunc(vmap, as->am->amap, as->aname, as->am->p_fims, as->targaVp);
                            }
                            else if (as->ai)
                            {
                                amFunc(vmap, as->ai->amap, as->aname, as->ai->p_fims, as->targaVp);
                            }
                        }
                    }
                    else
                    {
                        char* myfcn = av->getcParam("fcn");
                        int debug = 0;
                        if (0)
                            FPS_PRINT_INFO("Running one with name [{}:{}] size {} func {}", av->comp, av->name,
                                           rreqs.size(), myfcn ? myfcn : "No FCN");
                        // this triggers the scheduled item
                        // we need to get the response from the operation
                        // pass the scheduler controlling av along to the function(s)
                        // eventually
                        if (!av->gotParam("fcn"))
                        {
                            // this is the main schedule point
                            // we have to set the start time here to help with race detection
                            if (0 || debug)
                                FPS_PRINT_INFO("Running  Time set for Av [{}] at [{:2.3f}]", av->getfName(), tNow);
                            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), tNow);
                        }
                        // TODO For discussion after MVP we may use as->func if it is
                        // defined
                        // TODO For discussion after MVP refine / define the use of schedAv
                        // (actAv) and targAv
                        // TODO For discussion after MVP move the action iterators into
                        // av->extras so we can use them to return to for sequences. (for
                        // varMapUtils really)

                        bool no_update = false;  // this flag is the default action the
                                                 // scheduler takes if no update param exists
                        char* fcn = as->func;
                        char* aname = as->aname;
                        // assetVar* targav = av;
                        char* targ = as->targAv;
                        if (av->gotParam("targ"))
                        {
                            targ = av->getcParam("targ");
                        }
                        if (targ)
                        {
                            targav = vm->getVar(vmap, targ, nullptr);

                            if (!targav)
                            {
                                if (1)
                                    FPS_PRINT_WARN("unable to find target Av [{}] using [{}]", as->targAv,
                                                   av->getfName());
                                targav = av;
                            }
                        }

                        // logic has been added to allow a schedItem that runs multiple
                        // times to run faster after the first iteration
                        if (av->gotParam("update"))
                        {
                            if (av->getbParam("update"))
                            {
                                // the flags for this logic have been moved from the av to the
                                // schedItem to account for the case where one av runs multiple
                                // schedItems
                                if (as->updone == false)
                                {
                                    // updone defaults to false, if its false here this is our
                                    // first run
                                    as->update = true;
                                    as->updone = true;
                                }
                                else if (as->updone == true)
                                {
                                    // updone latches to true, so we know this is our second+ run
                                    // and we have the fcnptr and targaVp that we need to run fast
                                    as->update = false;
                                }
                            }
                            else
                            {
                                // if the "update" param is false, default to running the "slow"
                                // way, using the "no_update" flag
                                no_update = true;
                            }
                        }
                        else
                        {
                            // if we dont have an update param, default to running the "slow"
                            // way, using the "no_update" flag
                            no_update = true;
                        }

                        if (av->gotParam("debug"))
                        {
                            debug = av->getiParam("debug");
                        }
                        if (av->gotParam("fcn"))
                        {
                            fcn = av->getcParam("fcn");
                        }
                        if (av->am)
                        {
                            aname = (char*)av->am->name.c_str();
                        }
                        if (av->gotParam("amap"))
                        {
                            aname = av->getcParam("amap");
                        }
                        as->runt = false;
                        if (av->gotParam("targ"))
                        {
                            as->targ = av->getcParam("targ");
                            if (debug)
                                FPS_PRINT_INFO("Running fcn [{}] for targ [{}]", fcn, as->targ);

                            if (as->targ)
                            {
                                as->targaVp = vm->getVar(vmap, as->targ, nullptr);
                                vm->schedaV = as->targaVp;  // vm->getVar(vmap, as->targ, nullptr);
                                vm->schedTime = tNow;
                                vm->schedActions = 0;

                                // we are done with this one no more tests
                            }
                        }
                        // runtarg simply sets the value of the schedAv or targ
                        // it then disables the fcn pointer
                        if (strcmp(fcn, "RunTarg") == 0)
                        {
                            if (vm->schedaV)
                            {
                                as->runt = true;
                                vm->setVal(vmap, vm->schedaV->getfName(), nullptr, tNow);
                                fcn = nullptr;
                            }
                            else
                            {
                                FPS_PRINT_ERROR("Cannot run  fcn [{}] no targ [{}] update [{}]", fcn, as->targ,
                                                as->update);
                                fcn = nullptr;
                            }
                        }

                        // this will run and set up
                        // as->fcnptr
                        // targav->am
                        // as->targaVp
                        // targav->ai
                        if (aname && as->targaVp && as->fcnptr && !as->update)
                        {
                            myAvfun_t amFunc = reinterpret_cast<myAvfun_t>(as->fcnptr);
                            if (as->targaVp->am)
                            {
                                av->setParam("fastRun_am", tNow);
                                amFunc(vmap, as->targaVp->am->amap, aname, as->targaVp->am->p_fims, as->targaVp);
                            }
                            else if (as->targaVp->ai)
                            {
                                av->setParam("fastRun_ai", tNow);
                                amFunc(vmap, as->targaVp->ai->amap, aname, as->targaVp->ai->p_fims, as->targaVp);
                            }
                        }

                        else if (fcn && aname && (as->update || no_update))
                        {
                            void* res1 = vm->getFunc(vmap, aname, fcn);
                            if (!res1)
                            {
                                res1 = vm->getFunc(vmap, vm->getSysName(vmap), fcn);
                            }
                            as->fcnptr = res1;

                            if (as->fcnptr)
                            {
                                myAvfun_t amFunc = reinterpret_cast<myAvfun_t>(as->fcnptr);

                                // scheduler needs to correct the environment
                                asset_manager* am = vm->getaM(vmap, aname);
                                asset* ai = vm->getaI(vmap, aname);
                                if (!am && !ai)
                                {
                                    if (1)
                                        FPS_PRINT_INFO("Unable to find am or ai, aname [{}] fcn [{}]", aname, fcn);
                                }
                                if (am)
                                {
                                    if (strcmp(av->am->name.c_str(), am->name.c_str()) != 0)
                                    {
                                        if (0)
                                            FPS_PRINT_INFO("Correcting av->am [{}] to [{}] aname [{}] fcn [{}]",
                                                           av->am->name, am->name, aname, fcn);
                                    }

                                    if (targav->am == nullptr)
                                        targav->am = am;
                                    if (am->vm == nullptr)
                                        am->vm = vm;
                                    if (0)
                                        FPS_PRINT_INFO(
                                            "Running aname [{}] fcn [{}] as->update "
                                            "[{}] no_update [{}]",
                                            aname, fcn, as->update, no_update);

                                    amFunc(vmap, am->amap, aname, am->p_fims, targav);
                                }
                                if (ai)
                                {
                                    if (targav->ai == nullptr)
                                        targav->ai = ai;
                                    if (ai->vm == nullptr)
                                        ai->vm = vm;
                                    amFunc(vmap, ai->amap, aname, ai->p_fims, targav);
                                }
                                as->targaVp = targav;
                            }
                        }
                        if (fast == 1)
                        {
                            av->setParam("fast", 2);
                        }
                    }
                    // Schedav ??
                    if (targav->gotParam("response"))
                    {
                        response = targav->getcParam("response");
                    }
                    if (response)
                    {
                        if (1)
                            FPS_PRINT_INFO("Response [{}] from {:2.6f} at {:2.6f} elapsed mS {:2.6f}", response, tNow,
                                           vm->get_time_dbl(), (vm->get_time_dbl() - tNow) * 1000.0);
                    }
                }

                if (!endRun && (!av->gotParam("enabled") || av->getbParam("enabled")))
                {
                    as->runCnt++;
                    av->setParam("runCnt", as->runCnt);
                }

                rreqs.erase(rreqs.begin());
                if (0)
                    FPS_PRINT_INFO("repTime av {:2.3f} as {:2.3f} new size {}", av->getdParam("repTime"), as->repTime,
                                   rreqs.size());
                // could use as->repTime
                if (!endRun && av->getdParam("repTime") > 0.0 && as->repTime > 0.0)
                {
                    if (0)
                        FPS_PRINT_INFO("added av [{}:{}] at {:2.6f}", av->comp, av->name,
                                       av->getdParam("runTime") + av->getdParam("repTime"));
                    // Do we need a tNow check ??
                    as->runTime = av->getdParam("runTime") + av->getdParam("repTime");
                    if ((as->endTime == 0) || (as->runTime < as->endTime))
                    {
                        av->setParam("runTime", as->runTime);
                        // may need to check resched;
                        addSchedReq(rreqs,
                                    as);  // new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->args));
                    }
                    else
                    {
                        if (1)
                            FPS_PRINT_INFO("deleted as [{}] runTime {:2.3f} endTime {:2.3f}", as->id, as->runTime,
                                           as->endTime);

                        delete as;
                    }
                }
                else
                {
                    delete as;
                }

                if (0)
                    FPS_PRINT_INFO("new size 2 {}", rreqs.size());
            }
        }
        if (0 && oneran)
            FPS_PRINT_INFO("Done running {}", oneran);
    }
    // now pick off the next delay
    if (rreqs.size() > 0)
    {
        tNow = vm->get_time_dbl();

        as = rreqs.front();
        av = as->av;
        double newd = (av->getdParam("runTime") - tNow);
        delay = newd + 0.0005;
        if (0)
            FPS_PRINT_INFO("calc new delay {:3.6f} = {:3.6f}", newd, delay);
    }
    if (delay < 0.0)
        delay = 0.0;
    if (delay > 2.0)
        delay = 2.0;
    return delay;
}

// no need to be in the scheduler
int scheduler::fimsDebug1(varsmap& vmap, fims_message* msg, asset_manager* am)
{
    if (msg)
    {
        double msize = 0.0;
        if (msg->body)
            msize = (double)strlen(msg->body);
        if (0)
            FPS_PRINT_INFO("BEFORE FIMS MESSAGE method [{}] replyto [{}] uri [{}] msize {}", msg->method,
                           msg->replyto ? msg->replyto : "No Reply", msg->uri, msize);
        // bool bval = false;
        fLog = new essPerf(am, /*"ess_test"*/ (char*)am->name.c_str(), "fimsDebug1", nullptr);
        // if(msg)
        // char *ess_uri = nullptr;
        // asprintf(&ess_uri,"uri_%s",msg->uri);
        // uriLog = nullptr;
        // //this causes loads of problems
        // //ePerf_uri = new essPerf(am, "ess_uri", "msg->uri", nullptr);
        // free(ess_uri);

        const char* uri_meth = "unknown";
        if (strcmp(msg->method, "set") == 0)
        {
            uri_meth = "uri_set";
        }
        else if (strcmp(msg->method, "get") == 0)
        {
            uri_meth = "uri_get";
        }
        else if (strcmp(msg->method, "pub") == 0)
        {
            uri_meth = "uri_pub";
        }
        // essPerf ePerf2(am, "ess_test", uri_meth, nullptr);

        if (0)
            FPS_PRINT_INFO("vmap before fims [{}]...", uri_meth);

        if (0)
            FPS_PRINT_INFO("first level map", NULL);
        for (auto x : vmap)
        {
            if (0)
                FPS_PRINT_INFO("x.first [{}]", x.first);
            for (auto y : x.second)
            {
                if (0)
                    FPS_PRINT_INFO("x.first [{}] y.first [{}]", x.first, y.first);
            }
        }

        cJSON* cj = vm->getMapsCj(vmap, nullptr, nullptr, 0x00);
        if (cj)
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                if (0)
                    FPS_PRINT_INFO("Vmap {}", res);
                free(res);
            }
            cJSON_Delete(cj);
        }
    }
    return 0;
}

// no need to be in the scheduler
int scheduler::fimsDebug2(varsmap& vmap, fims_message* msg, asset_manager* am)
{
    UNUSED(am);
    if (msg)
    {
        if (0)
            FPS_PRINT_INFO("AFTER FIMS MESSAGE", NULL);
        if (0)
            FPS_PRINT_INFO("vmap after fims ...", NULL);
        if (0)
            FPS_PRINT_INFO("first level map", NULL);
        for (auto x : vmap)
        {
            if (0)
                FPS_PRINT_INFO("x.first [{}]", x.first);
            for (auto y : x.second)
            {
                if (0)
                    FPS_PRINT_INFO("x.first [{}] y.first [{}]", x.first, y.first);
            }
        }
        cJSON* cj = vm->getMapsCj(vmap, nullptr, nullptr, 0x00);
        if (cj)
        {
            char* res = cJSON_Print(cj);
            if (res)
            {
                if (0)
                    FPS_PRINT_INFO("Vmap {}", res);
                free(res);
            }
            cJSON_Delete(cj);
        }
    }
    {
        if (uriLog)
            delete uriLog;

        if (fLog)
            delete fLog;
        uriLog = nullptr;
        fLog = nullptr;
    }
    return 0;
}

cJSON* getSchListcJ(schlist& rreqs)
{
    cJSON* cj = nullptr;  // cJSON_CreateObject();

    if (rreqs.size() > 0)
    {
        cj = cJSON_CreateObject();
        for (auto sr : rreqs)
        {
            cJSON* cji = cJSON_CreateObject();
            cJSON_AddNumberToObject(cji, "runTime", sr->runTime);
            cJSON_AddNumberToObject(cji, "repTime", sr->repTime);
            cJSON_AddNumberToObject(cji, "runs", sr->runCnt);
            cJSON_AddStringToObject(cji, "uri", sr->uri);
            cJSON_AddStringToObject(cji, "func", sr->func);
            cJSON_AddItemToObject(cj, sr->id, cji);
        }
    }
    return cj;
}
// moved to scheduler
void scFimsThread(scheduler* sc, fims* p_fims)
{
    bool debug = false;
    double tNow = sc->vm->get_time_dbl();
    int tick = 0;
    fims_message* msg = p_fims->Receive_Timeout(100);
    while (*sc->run)
    {
        msg = p_fims->Receive_Timeout(1000000);
        if (debug)
        {
            // just for testing
            tNow = sc->vm->get_time_dbl();
            if (1)
                FPS_ERROR_PRINT("%s >>Fims Tick %d msg %p p_fims %p time %2.3f\n", __func__, tick, (void*)msg,
                                (void*)p_fims, tNow);
            tick++;
        }
        if (msg)
        {
            // TODO check for blocks here
            // if (strcmp(msg->method, "get") == 0)
            // {
            //     if (1)FPS_ERROR_PRINT("%s >> %2.3f GET uri  [%s]\n"
            //         , __func__, sc->vm->get_time_dbl(),
            //         msg->uri);
            // }
            if (*sc->run)
            {
                sc->fimsChan.put(msg);
                sc->wakeChan.put(tick);  // but this did not get serviced immediatey
            }
            else
            {
                if (p_fims)
                    p_fims->free_message(msg);
                // p_fims->delete
            }
        }
    }
    // assetVar aV;
    // tNow = sc->vm->get_time_dbl();
    // aV.sendEvent(flexPack, p_fims,  Severity::Info, "FlexPack shutting down at
    // %2.3f", tNow);
    FPS_ERROR_PRINT("%s >> fims shutting down\n", __func__);
    // if(p_fims)delete p_fims;
    sc->p_fims = nullptr;
}

// decodes all the fields from aV and runs a direct action sched
void SetupSched(varsmap& vmap, VarMapUtils* vm, char* schedVar, assetVar* aV, asset_manager* pm)
{
    double tNow = vm->get_time_dbl();

    if (!pm)
    {
        FPS_PRINT_ERROR("no pm found", NULL);
    }

    assetVar* avs = vm->getVar(vmap, schedVar, nullptr);
    if (!avs)
    {
        double dval = 0.0;
        avs = vm->makeVar(vmap, schedVar, nullptr, dval);
    }
    double offset = 0.0;
    double every = 1.0;  // get from param
    if (aV->gotParam("every"))
    {
        every = aV->getdParam("every");
    }
    if (aV->gotParam("offset"))
    {
        offset = aV->getdParam("offset");
    }

    avs->am = pm;
    avs->setParam("uri", schedVar);
    avs->setParam("every", every);
    avs->setParam("offset", offset);

    char* schedUri = aV->getcParam("uri");     // this is the schedule control uri
    char* schedFcn = aV->getcParam("fcn");     // we're going to run this function
    char* schedTarg = aV->getcParam("targ");   // with this aV
    char* schedId = aV->getcParam("schedid");  // this must be unique for each scheditem
    // char* schedamap = aV->getcParam("amap");
    double refTime = aV->getdParam("refTime");  // this is the start time
    double repTime = aV->getdParam("every");    // this sets repTime
    double runTime = aV->getdParam("runTime");  // this is the time it was last run
    // double repTime = aV->getdParam("repTime");
    // double runAfter = aV->getdParam("runAfter");
    double runFor = aV->getdParam("runFor");    // this sets endtime
    double endTime = aV->getdParam("endTime");  // this is the endTime
    int debug = aV->getiParam("debug");         // debuf on or off
    double startTime = tNow;

    if (aV->gotParam("refTime") && aV->gotParam("every"))
    {
        startTime = refTime;
    }
    std::cout << " got start time " << startTime << std::endl;
    // runAfter will default to 0
    // runTime will default to 0
    if (aV->gotParam("every") && repTime > 0.0)
    {
        // TODO find a better way to do this
        while (startTime < tNow)
        {
            startTime += repTime;
        }
    }

    runTime = startTime;

    if (!schedTarg)
        schedTarg = (char*)"/test/mySched:Targ";

    if (!schedFcn)
        schedFcn = (char*)"mySchedFcn";

    if (!schedUri)
        schedUri = (char*)"/default/sched:mySchedUri";

    if (!schedId)
        schedId = (char*)"bmsmon";  // need to override this to get unique scheditem
                                    // in schlist

    if (schedFcn)
        avs->setParam("fcn", schedFcn);
    if (schedId)
        avs->setParam("id", schedId);
    avs->setParam("targ", schedTarg);
    avs->setParam("runTime", runTime);
    avs->setParam("repTime", every);
    if (aV->gotParam("runFor") && runFor > 0.0)
    {
        endTime = runTime + runFor;
        avs->setParam("runEnd", (runTime + runFor));
        avs->setParam("endTime", endTime);
    }
    else
    {
        double dval = 0.0;
        avs->setParam("runEnd", dval);
        avs->setParam("endTime", dval);
    }

    // set up and run scheduler
    schedItem* as = nullptr;
    if (aV && aV->am)
    {
        as = new schedItem();

        // if (debug) FPS_PRINT_INFO("created new schedItem {}", fmt::ptr(as)); //
        // spdlog
        as->av = avs;
        // //if (avs->am)
        // {
        //     am = pm;//avs->am;
        // }
        // this dummy function will simply send the current time to avi triggering
        // its onSet actions in fact we'll do this anyway without a function
        // void schedItem::setUp(const char * _id, const char* _aname, const char *
        // _uri, const char* _func, double _refTime,
        //    double _runTime, double _repTime, double _endTime, char*targaV)

        std::cout << " try as->setup " << startTime << std::endl;

        // pm name is null, deref err
        as->setUp(schedId, pm->name.c_str(), schedUri, schedFcn, refTime, runTime, repTime, endTime, schedTarg);
        std::cout << " done as->setup " << startTime << std::endl;

        if (debug)
            as->show();

        if (pm->reqChan)
        {
            channel<schedItem*>* reqChan = (channel<schedItem*>*)pm->reqChan;
            reqChan->put(as);
            if (pm->wakeChan)
            {
                pm->wakeChan->put(0);
            }

            avs->setParam("active", true);
            avs->setParam("enabled", true);
        }
        else
        {
            delete as;
            as = nullptr;
        }
    }
}

int TestRunSchedOpts(varsmap& vmap, varmap& amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(_aname);
    UNUSED(p_fims);
    VarMapUtils* vm = aV->am->vm;
    double tNow = vm->get_time_dbl();
    std::cout << ">>>> running func " << __func__ << " with aV [" << aV->getfName() << "] time :" << tNow << std::endl;

    return 0;
}

std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}
