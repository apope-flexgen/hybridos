/*

new Scheduler
p. wilshire
02/09/2021

How it works.
The system uses ScheduleItems as points in time when we need to do something.
Here is the structure
    double refTime;     // used to set priority  actual run time is tNow + (refTime *runCnt)
    double runTime;     // earliest time to run 
    double repTime;     // if non zero causes repeated runs
    int runCnt;         // counts how many times this item has run
    char *id;           // used to identify the schedItem
    char *aname;        // asset name to use 
    char* targname;     // the name of the target assetVar
    char *fcnname;      // (optional) the name of the assetvar to be passed to the function
    assetVar* targav;   // target assetvar    
    assetVar* fcnav;    // (optional) asset var to be used by the target   

A list of thse items is kept by the scheduler (schliist) with the first one to run ( lowest run time ) at the head of the list.

The scheduler runs in a the main schedule thread. This thread uses channels to get notifications of:
   fims messages arriving
   new sched items to be added to the list
   other messages ( not used yet)

When the scheduler runs it looks for any item on the (top of the) schlist and sends the current time to the target av.
It will also set the Param of the fcnAV if specified.
The target av will get the new time message and run any functions associated with the target aV
The fcnAv can be used to allow the same target av (and function) to run with different assetVars. 

As each item on the schlist is processed it is checked for a (repTime) repeat time.
If specified the repTime is added to the runTime and the schedItem is placed back in the schlist.

As the scheduler works down the list it finally comes to the end or to a schedItem with a time in the future.
If there is nothing on the list the sched delay is set to 1 second...
If there is another item in the list , the sched delay is set to the difference between tNow and the next runTime.

The loop is rerun with a channel timedGet using the delay as a max time to wait for an incoming wakeup on the
wake channel.
If nothing else sends a wakeup then this would be time to run the next entry.

An exernal process can decide to add an item to the schlist. It does that by creating the schedItem and 
sending that to the reqChan together with a wakeup to the wakeChan.
The scheduler will wake up immediately and put the schedItem into the schlist and then run any past due items 
or reset the wait delay.


A fims thread is also set up to continually read fims messages. 
As each message is received it is dequeed from the incoming fims socket and sent to the fimsChan together 
with a wakeReq to cause the scheduler to immediately wake up and process the fims message.

All message handling and schedule operations are handled by a single thread. So no locking is required on the variable map.

The system sets up its "default" or hard coded operation before reading in any config files.
The incoming config files can change this default operation any incoming fims message can also change the scheduler 
operation.

There are a number of "helper" functions used to facilitate this setup.

1/ The scheduler runs by sending a message ( current time) to a designated target assetVar. 
   That assetVar needs to be present if the schedule operation wants to do anything.
2/ Having set up the assetVar it needs to be given some actions to perform when the value is set.
3/ Having set up the target assetVar the scheduler needs to be told that this is a schedule point  
   with an expected runTime.


Here is an example sequence using the helper functions.

int SetupDefaultSched(scheduler*sched, asset_manager* am)
{

    am->vm->setFunc(*am->vmap, "ess", "AddSchedItem",    (void*)&AddSchedItem);
    am->vm->setFunc(*am->vmap, "ess", "CheckMonitorVar", (void*)&CheckMonitorVar);
    am->vm->setFunc(*am->vmap, "ess", "EssSystemInit",   (void*)&EssSystemInit);
    am->vm->setFunc(*am->vmap, "ess", "Every1000mS",     (void*)&Every1000mS);
    am->vm->setFunc(*am->vmap, "ess", "Every100mSP1",    (void*)&Every100mSP1);
    am->vm->setFunc(*am->vmap, "ess", "Every100mSP2",    (void*)&Every100mSP2);
    am->vm->setFunc(*am->vmap, "ess", "Every100mSP3",    (void*)&Every100mSP3);
    am->vm->setFunc(*am->vmap, "ess", "FastPub",         (void*)&FastPub);
    am->vm->setFunc(*am->vmap, "ess", "SlowPub",         (void*)&SlowPub);

    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "essSystemInit", "EssSystemInit", "ess", 0.200, 0.200 , 0.000);
    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "every1000mS" ,  "EverySecond",   "ess", 0.250, 0.250 , 1.000);
    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "every100mSP1" , "Every100mS_P1", "ess", 0.251, 0.251 , 0.100);
    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "every100mSP2" , "Every100mS_P2", "ess", 0.252, 0.252 , 0.100);
    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "every100mSP3" , "Every100mS_P3", "ess", 0.253, 0.253 , 0.100);
    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "fastPub" ,      "FastPub",       "ess", 0.254, 0.254 , 0.050);
    sched->setupSchedVar(*am->vmap, am, "/sched/ess", "slowPub" ,      "SlowPub",       "ess", 0.255, 0.255 , 2.000);

    cJSON* cja;

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "EssSystemInit", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "essSystemInit", "onSet", "func", cja); cJSON_Delete(cja);

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "Every1000mS", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "every1000mS", "onSet", "func", cja); cJSON_Delete(cja);

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "Every100mS_P1", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "every100mSP1", "onSet", "func", cja); cJSON_Delete(cja);

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "Every100mS_P2", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "every100mSP2", "onSet", "func", cja); cJSON_Delete(cja);

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "Every100mS_P3", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "every100mSP3", "onSet", "func", cja); cJSON_Delete(cja);

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "FastPub", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "fastPub", "onSet", "func", cja); cJSON_Delete(cja);

    cja = cJSON_CreateArray(); sched->addSaction(*am->vmap, "ess", "SlowPub", "/status/ess:status",   am, cja);
    sched->setupSchedItemActions(*am->vmap, am, "/sched/ess", "slowPub", "onSet", "func", cja); cJSON_Delete(cja);
 
    // this makes /schedule/ess:add_item activate a schedule item when it is sent some details
    sched->setupSchedFunction(*am->vmap, "/schedule/ess", "add_item", "AddSchedItem", am);


    // now use it to create the sched items and activate the schedule vars
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "essSystemInit");
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "every1000mS");
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "every100mSP1");
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "every100mSP3");
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "every100mSP3");
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "fastPub");
    sched->activateSchedItem(*am->vmap, "/schedule/ess", "/sched/ess", "slowPub");
    return 0;
}

This is the resulting stuff in the varsmap.
 setValfromCj >> after setting value for [
     
     
/sched/ess: {
	"essSystemInit":	{
		"value:0,"id":	"EssSystemInit", "aname":"ess", "enabled":true,"refTime":0.2,"runTime":0.2, "repTime":0,
        "actions":{"onSet":	[{"func":[
            {"amap":	"ess","func":	"EssSystemInit","var":	"/status/ess:status"}
            ]}]}},
	"every1000mS":	{
		"value":0,"id":	"EverySecond","aname":"ess","enabled":true,"refTime":	0.25,"runTime":	0.25,"repTime":	1,
        "actions":{"onSet":	[{"func":	[
            {"amap":	"ess","func":	"Every1000mS","var":	"/status/ess:status"}
            ]}}}},
	"every100mSP1":	{
		"value":0,"id":	"Every100mS_P1","aname":"ess","enabled":true,"refTime":	0.251,"runTime":0.251,"repTime":0.1,
        "actions":{"onSet":	[{"func":	[
            {"amap":	"ess","func":	"Every100mS_P1","var":	"/status/ess:status"}
            ]}}}},
	"every100mSP2":	{
		"value":0,"id":	"Every100mS_P2","aname":"ess","enabled":true,"refTime":	0.252,"runTime":0.252,"repTime":0.1,
        "actions":{"onSet":	[{"func":	[
            {"amap":	"ess","func":	"Every100mS_P2","var":	"/status/ess:status"}
            ]}}}},
	"every100mSP3":	{
		"value":0,"id":	"Every100mS_P3","aname":"ess","enabled":true,"refTime":	0.253,"runTime":0.253,"repTime":0.1,
        "actions":{"onSet":	[{"func":	[
            {"amap":	"ess","func":	"Every100mS_P3","var":	"/status/ess:status"}
            ]}}}},
	"fastPub":	{
		"value":0,"id":	"FastPub","aname":"ess","enabled":true,"refTime":	0.254,"runTime":0.254,"repTime":0.05,
        "actions":{"onSet":	[{"func":	[
            {"amap":	"ess","func":	"FastPub","var":	"/status/ess:status"}
            ]}}}},
	"slowPub":	{
		"value":0,"id":	"SlowPub","aname":"ess","enabled":true,"refTime":	0.255,"runTime":0.255,"repTime":2,
        "actions":{"onSet":	[{"func":	[
            {"amap":	"ess","func":	"SastPub","var":	"/status/ess:status"}
            ]}}}}
    }
 /schedule/ess:{
	"add_item":	{
		"value":	"test string",
		"actions":{"onSet":	[{"func":	[
            { "func":	"AddSchedItem", "amap":	"ess"}
            ]}]}}
    }

Here is the list of sched items 
 sched id [EssSystemInit] count [0] aname [ess] uri [/sched/ess:essSystemInit] refTime 0.200 runTime 0.200000 repTime 0.000 
 sched id [EverySecond]   count [0] aname [ess] uri [/sched/ess:every1000mS]   refTime 0.250 runTime 0.250000 repTime 1.000 
 sched id [Every100mS_P1] count [0] aname [ess] uri [/sched/ess:every100mSP1]  refTime 0.251 runTime 0.251000 repTime 0.100 
 sched id [Every100mS_P2] count [0] aname [ess] uri [/sched/ess:every100mSP2]  refTime 0.252 runTime 0.252000 repTime 0.100 
 sched id [Every100mS_P3] count [0] aname [ess] uri [/sched/ess:every100mSP3]  refTime 0.253 runTime 0.253000 repTime 0.100 
 sched id [ FastPub]      count [0] aname [ess] uri [/sched/ess:fastPub]       refTime 0.254 runTime 0.254000 repTime 0.050 
 sched id [ slowPub]      count [0] aname [ess] uri [/sched/ess:slowPub]       refTime 0.255 runTime 0.255000 repTime 2.000 




wip for final integration.....



leading nicely into sequencing...

* new scheduler test code
* anyone can add a schedule event , the new event  may change the timeout.
* and events 
* take a variable and give it some parameters
*
* set /sched/ess:essStateCheck  '{"value":true,"refTime":0.253, "runTime": 1.25,"repTime":0.5,"var":/status/ess:poweron","function":"CheckEssStatus"}
*
* then flag the variable for the scheduler.
* set /scheduler/ess:add_item  '{"value":/status/ess:essStateCheck"}
* the add_item call will cause the function "CheckEssStatus" to be scheduled at a time greater than tNow plus 1.25 seconds calcuated from refTime + repTime * n.
*   for example     
*   set this value in place /sched/ess:essStateCheck  '{"value":true, "runTime": 1.25,"repTime":0.0,"var":/status/ess:poweron","function":"CheckEssStatus"}
*   then call 
*                set /scheduler/ess:add_item  '{"value":/status/ess:essStateCheck"}
*          at time 25.3 seconds 
* the function CheckEssStatus using /status/ess:poweron will be run once at tNow + 1.25 seconds (26.58 SECS).
*
*   set this value in place /sched/ess:essPowerCheck  '{"value":true, "refTime":0.51, "runTime": 1.25,"repTime":0.5,"var":/status/ess:poweron","function":"CheckEssStatus"}
*   then call 
*                set /scheduler/ess:add_item  '{"value":/status/ess:essStateCheck"}
*   at time 30.12 seconds.
* the system will will call  
*        the function CheckEssStatus using /status/ess:poweron will be run every 0.5 seconds  starting  at 31.51 Seconds
*
* the repeating function can be stopped immediately be setting the enabled Param false. and also setting the repTime to 0.0
* nah we can do better than that.
*
*                set /scheduler/ess:add_item  '{"value":/status/ess:essStateCheck", "runTime":1.5, "repTime":0}
*                means 
*                       run once in 1.5 seconds
*                set /scheduler/ess:add_item  '{"value":/status/ess:essStateCheck", "runTime":1.5, "repTime":0.1}
*                       run every 0.1 seconds in 1.5 seconds
* 
*/

// #include <pthread.h>
// #include <iostream>
// #include <poll.h>
// #include <sys/socket.h>
// #include <poll.h>
// #include <unistd.h>
// #include <string.h>
// #include <vector>

#include <thread>

#include "asset.h"
#include <fims/libfims.h>
#include "channel.h"
#include "varMapUtils.h"
#include "scheduler.h"


VarMapUtils vm;
varsmap vmap;
std::vector<char *> syscVec;

#if defined _JUST_LIB
// use av->addSchedReq(schAvList*avlist)
schedItem::schedItem()
{
    refTime = 0.0;
    runTime = 0.0;
    repTime = 0.0;
    runCnt = 0;
    aname = nullptr;
    id = nullptr;
    av = nullptr;
    comp = nullptr;
    name = nullptr;

}

schedItem::~schedItem()
{
    FPS_ERROR_PRINT(" %s deleting this sched item %p \n", __func__, this);
    show();

    if(aname)    free(aname);
    if(id)       free(id);
    if(comp)     free(comp);
    if(name)     free(name);
}



// if tNow > 0 then pick the resched time after tNow
// else just repeat once.
int schedItem::reSched(double tNow)
{
    if (repTime == 0.0) return runCnt;
    if(tNow > 0.0)
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
    FPS_ERROR_PRINT(" sched id [%8s] count [%4d] aname [%8s] uri [%s:%s] refTime %2.3f runTime %2.6f repTime %2.3f \n"
    , id
    , runCnt
    , aname
    , comp
    , name?name:""
    , refTime
    , runTime
    , repTime
    );
}

void schedItem::setUp(const char * _id, const char* _aname, const char * _comp, const char* _name, double _refTime,
            double _runTime, double _repTime)
{
    assetUri my(_comp,_name);
    if(aname) free(aname);
    if(comp) free(comp);

    if(id) free(id);
    id  = strdup(_id);
    aname = strdup(_aname);
    comp = strdup(my.Uri);
    name = strdup(my.Var);
    refTime = _refTime;
    runTime = _runTime;
    repTime = _repTime;
    FPS_ERROR_PRINT(" %s setup a sched item %p \n", __func__, this);
    show();

    return;
}

scheduler::scheduler(const char *_name, int *_run )
{
   FPS_ERROR_PRINT(" hello scheduler [%s]\n", _name);
    name = _name;
    fLog = nullptr;
    uriLog = nullptr;
    run = _run;
    p_fims = nullptr;
    vm = nullptr;
};

scheduler::~scheduler()
{
    int ival = 21;
    schedItem * si;
    FPS_ERROR_PRINT(" bye scheduler [%s]\n", name.c_str());
    while(wakeChan.get(ival,false))
    {
        FPS_ERROR_PRINT("%s >> we got a wakeNotice .... %d\n", __func__, ival);
    }
    while(reqChan.get(si,false))
    {
        FPS_ERROR_PRINT("%s >> we got a schedItem %p not deleting it for now .... \n", __func__,  si);
        si ->show();
        delete si;
    }
}

void scheduler::showReqs(schlist&rreqs)
{
    for ( auto sr : rreqs)
    {
        sr->show();        
    }
    return;
}

void scheduler::clearReqs(schlist&rreqs)
{
    FPS_ERROR_PRINT("%s >> reqs at end size %d\n",__func__, (int)rreqs.size());
    for ( auto sr : rreqs)
    {
        FPS_ERROR_PRINT("%s >>  we got a schedItem %p not deleting it for now .... \n", __func__, sr);
        sr->show();
        //delete sr;        
    }
    return;
}

// followed by this config items
// set /schedule/ess:add_item 
//                '{"id":"some_id", 
//                 "aname":"asset_name", 
//                 "var":"/sched/ess:every100mS", 
//                 "nopefunction":"periodic_scan_100", 
//                 "function":"send_time_to_var", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'
// see also
//int setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item",am)
//setupSchedFinction
int scheduler::setupSchedFunction(varsmap& vmap, const char* uri, const char* oper, const char*func, asset_manager* am)
// int setup_sched_function(varsmap& vmap, const char* uri, const char* oper, const char*func, asset_manager* am)
{
    char*rbuf;

    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"%s\":{\"value\":\"test string\",\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"%s\",\"amap\":\"ess\"}]}]}}}}", uri, oper, func);
    fims_message *msg = am->vm->bufferToFims((const char *)rbuf);
    if(msg)
    {
        FPS_ERROR_PRINT("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
               , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
               , msg->method?msg->method:"no method"
               , msg->body?msg->body:"no body" );
    }
    else
    {
        FPS_ERROR_PRINT(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        return 0;
    }

    if(msg)
    {
        cJSON* cj =cJSON_CreateObject();
        am->vm->processFims(vmap, msg, &cj);
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                FPS_ERROR_PRINT(" %s >> got res [%s]\n"
                , __func__
                , res
                );
                free(res);
            }
        }
        cJSON_Delete(cj);
    }
    free(rbuf);
    assetVar* av = am->vm->getVar(vmap,uri, oper);
    av->am = am;
    am->vm->free_fims_message(msg);
    return 0;
}

fims* scheduler::fimsSetup(const char* subs[], int sublen, const char* name)
{
    //int sublen = sizeof subs / sizeof subs[0];
    FPS_ERROR_PRINT(" %s >> size of subs = %d\n", __func__, sublen);
    int attempt = 0;
    p_fims = new fims();

    while (!p_fims->Connect((char*)name)) 
    {
        poll(nullptr, 0, 1000);
        FPS_ERROR_PRINT("%s >> name %s waiting to connect to FIMS,attempt %d\n"
        , __func__
        , name
        , attempt++
        );
    }
    FPS_ERROR_PRINT("%s >> name %s connected to FIMS at attempt %d\n"
        , __func__
        , name
        , attempt
        );

    p_fims->Subscribe(subs, sublen);
    return p_fims;
}


// have to get it from value since the objects are not yet added
// BUGFIX done required
// this takes the assetVar and  creates the assetSched item and triggers the channel write
// things like repTime and runTime are left in place as params and used 
// in the sched list
// int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
// {
//     cJSON *cj = nullptr; //cJSON_Parse(av->getcVal());
//     cJSON *cji = nullptr;// cJSON_GetObjectItem(cj,"function");
//     if(av->getcVal())
//     {
//         cj = cJSON_Parse(av->getcVal());
//         if(cj)cji = cJSON_GetObjectItem(cj,"aV");
//     } 
//     char * aV = nullptr;
//     if(cji)
//     {
//         aV = cji->valuestring;
//     }
//     else if(cj)
//     {
//         aV = cj->valuestring;

//     }
//     else
//     {
//         aV=av->getcVal();
//     }
//     asset_manager* am = av->am;

//     FPS_ERROR_PRINT("\n\n***** %s >> running for  av [%s:%s] am %p cj %p  aV [%s] value [%s]\n\n"
//         , __func__
//         , av->comp.c_str()
//         , av->name.c_str()
//         , am
//         , cj//av->getcParam("function")
//         , aV?aV:"NoAv Here"
//         , av->getcVal()?av->getcVal():" No string val"
//     );
//     if(cj)cJSON_Delete(cj);
//     schedItem *as = nullptr;
//     assetVar* avi=nullptr;

//     //double tNow = vm.get_time_dbl();

//     avi = vm.getVar(*am->vmap, aV, nullptr);
//     if(avi && am)
//     {
//         as = new schedItem();
//         FPS_ERROR_PRINT("%s >> we added %p\n", __func__, as);


//         as->av = avi;

//         if(avi->am)
//             am = avi->am;

//         char* myid = avi->getcParam("id");
//         if (!myid) myid=(char *)"NoID2"; 

//         // this dummy function will simply send the current time to avi triggering its onSet actions
//         // infact we'll do this anyway without a function
//         as->setUp(myid, am->name.c_str(), avi->comp.c_str(), avi->name.c_str(),  avi->getdParam("runTime"),avi->getdParam("repTime") );
    
//         as->show();
//     //TODO phase out the function here... just send the time to the scheduled assetVar  
//     //let its actions list take care of things.
//     // note we do need to insert /remove an action..
//     // but the function operation is NOT important here
//     // ->put(

//         // we'll need the sched thread listening on these channels
//   // channel <int> *wakeChan;
//     // channel <schedItem*>*reqChan; // anyone can post run_reqs
//         if (am->reqChan)
//         {
//             channel <schedItem*>* reqChan = (channel <schedItem*>*)am->reqChan;
//             reqChan->put(as);
//         }
//         if (am->wakeChan)
//         {
//             channel <int>* wakeChan = (channel <int>*)am->wakeChan;
//             wakeChan->put(0);
//         }
//         else
//         {
//             FPS_ERROR_PRINT(" @@@@@ %s >> You need to set up am->reqChan and am->wakeChan   @@@@\n"
//                 , __func__
//                 );

//         }
//         //TODO let these rip if (am->wakeChan)am->wakeChan->put((int)(tNow * 1000));
//         //am->wakeUpChan->put(0) 
//         //am->wake_up
//         FPS_ERROR_PRINT("@@@@@@ %s>> activated %s   avi %p runTime %2.3f \n\n"
//             , __func__
//             , aV
//             , avi
//             , avi->getdParam("runTime")
//             );
//             avi->setParam("active",true);
//     }
//     else
//     {
//         FPS_ERROR_PRINT("@@@@@ %s did not find var to activate [%s]\n", __func__, aV?aV:"No aV");

//     }
//     return 0;
// }
int  scheduler::addSchedItem(varsmap &vmap, varmap &amap, const char* aname, assetVar* av)
{
    cJSON *cj = nullptr; //cJSON_Parse(av->getcVal());
    cJSON *cji = nullptr;// cJSON_GetObjectItem(cj,"function");
    if(av->getcVal())
    {
        cj = cJSON_Parse(av->getcVal());
        if(cj)cji = cJSON_GetObjectItem(cj,"aV");
    } 
    char * aV = nullptr;
    if(cji)
    {
        aV = cji->valuestring;
    }
    else if(cj)
    {
        aV = cj->valuestring;

    }
    else
    {
        aV=av->getcVal();
    }
    asset_manager* am = av->am;

    FPS_ERROR_PRINT("\n\n***** %s >> running for  av [%s:%s] am %p cj %p  aV [%s] value [%s]\n\n"
        , __func__
        , av->comp.c_str()
        , av->name.c_str()
        , am
        , cj//av->getcParam("function")
        , aV?aV:"NoAv Here"
        , av->getcVal()?av->getcVal():" No string val"
    );
    if(cj)cJSON_Delete(cj);
    schedItem *si = nullptr;
    assetVar* avi=nullptr;

    double tNow = vm->get_time_dbl();

    avi = vm->getVar(*am->vmap, aV, nullptr);
    if(avi && am)
    {
        si = new schedItem();

        si->av = avi;

        if(avi->am)
            am = avi->am;
        assetUri my(aV);
        char * myid = avi->getcParam("id");
        if(!myid)
            myid=(char*)"noId";
        si->setUp(myid, avi->name.c_str(), my.Uri, my.Var,
             avi->getdParam("refTime"),avi->getdParam("runTime"), avi->getdParam("repTime") );

        reqChan.put(si);
        wakeChan.put((int)(tNow * 1000));
        //am->wakeUpChan->put(0) 
        //am->wake_up
        FPS_ERROR_PRINT("@@@@@@ %s>> activated %s  id %s  avi %p runTime %2.3f \n\n"
            , __func__
            , aV
            , myid
            , avi
            , avi->getdParam("runTime")
            );
            avi->setParam("active",true);
    }
    return 0;
}

// -m set -u /schedule/ess '{"every100mS":{"value":0,"id":"some_id", "aname":"asset_name", "refTime":0.5,"runTime":0.5,"repTime":0.1}}'
// we'll have to add actions to this item to make it do anything
//    "actions":{"onSet":[{"func":[{"amap": "ess","func": "add_sched_item","var":"/status/ess/some_var"}]}]}
// example use:
//    cJSON * cja = cJSON_CreateArray();
//    addSaction(cja,"ess","CheckMonitorVar","/status/ess/maxTemp", cjacts);
//    setupSchedItemActions(vmap,"/schedule/ess","every100mS",cja);
//    cJSON_Delete(cja);
void scheduler::addSaction(varsmap& vmap, const char* amap, const char* func, const char*var, asset_manager* am, cJSON* cjacts)
{
 
    cJSON*cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj, "amap", amap);
    cJSON_AddStringToObject(cj, "func", func);
    cJSON_AddStringToObject(cj, "var", var);

    cJSON_AddItemToArray(cjacts, cj);
   //const char *myvar = nullptr;

    assetVar*av = vm->getVar(vmap, var, nullptr);
    double dval = 0.0;

    if(!av){
        assetUri my(var, nullptr);

        av = new assetVar(my.Var, my.Uri, dval);
        vmap[av->comp][av->name] = av;
    }
    if(am)
      av->am  = am;
}

int scheduler::setupSchedItemActions(varsmap& vmap, asset_manager*am, const char* uri, const char*name,
            const char*when, const char *act, cJSON* cjacts)
{
    char*acts = cJSON_PrintUnformatted(cjacts);
    char*rbuf;
    // uri example /schedule/ess
    // when = onSet
    // act = "func"
    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"%s\":{\"value\":0.0, \"actions\":{\"%s\":[{\"%s\":%s}]}}}}", uri, name, when, act, acts);
    fims_message *msg = am->vm->bufferToFims((const char *)rbuf);
    {
        // debug code
        if(msg)
        {
            FPS_ERROR_PRINT("fims  \n nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
                , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
                , msg->method?msg->method:"no method"
                , msg->body?msg->body:"no body" );
        }
        else
        {
            FPS_ERROR_PRINT(" no message from \n>>>%s<<<\n", rbuf);
            free(rbuf);
            free(acts);
            return 0;
        }
    }
    if(msg)
    {
        cJSON* cj =cJSON_CreateObject();
        am->vm->processFims(vmap, msg, &cj);
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                FPS_ERROR_PRINT(" %s >> got res [%s]\n"
                , __func__
                , res
                );
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

// sets up a schedule var
// set /schedule/ess:name'{"id":"some_id", "aname":"asset_name", "refTime":0.5, "runTime":0.5,"repTime":0.1}'
// we'll have to add actions to this item to make it do anything
int scheduler::setupSchedVar(varsmap& vmap, asset_manager*am, const char* uri, const char*name , const char* id, const char*aname
     , double refTime, double runTime , double repTime)
{
    char*rbuf;
    assetUri my(uri,name);

    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"%s\":{\"value\":0.0, \"id\":\"%s\","
       "\"aname\":\"%s\","
       "\"enabled\":true,"
       "\"refTime\":%f,"
       "\"runTime\":%f,"
       "\"repTime\":%f}}}", my.Uri, my.Var, id, aname, refTime, runTime, repTime);
    fims_message *msg = am->vm->bufferToFims((const char *)rbuf);
    if(!msg)
    {
        FPS_ERROR_PRINT(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        return 0;
    }
    cJSON* cj =cJSON_CreateObject();
    am->vm->processFims(vmap, msg, &cj, am);
    char *res=cJSON_Print(cj);
    {
        if(res)
        {
            FPS_ERROR_PRINT(" %s >> got res [%s]\n"
            , __func__
            , res
            );
            free(res);
        }
    }
    cJSON_Delete(cj);
    am->vm->free_fims_message(msg);

    free(rbuf);
    
    return 0;
}

// sets up a schedule item 
// set /schedule/ess:name'{"id":"some_id", "aname":"asset_name", "refTime":0.5, "runTime":0.5,"repTime":0.1}'
// we'll have to add actions to this item to make it do anything
schedItem* scheduler::setupSchedItem(varsmap& vmap, asset_manager*am, const char* uri, const char*name , const char* id, const char*aname
    ,double refTime , double runTime , double repTime)
{  
    setupSchedVar(vmap,am,uri,name,id,aname, refTime, runTime, repTime);
    schedItem *si = new schedItem();
    si->setUp(id, aname, uri, name, refTime, runTime, repTime );
    //si->func = vm->getFunc(*am->vmap, aname, "run_some_func");
    si->av = vm->getVar(*am->vmap, uri, name);
    return si;
}


// sched.activateSchedItem(vmap, "/schedule/ess", "/sched/ess", "every100mS");
//    sends the variable name (/sched/ess:every100ms) for activation to /schedule/ess/add_item 
//    which calls the function "add_sched_item" with the value string 
// this should be set up beforehand
// sched.setupSchedItem(*am->vmap, am, "/sched/ess", "every100mS" , "Every100mS", "ess", 0.25, 0.25 , 0.1);
int scheduler::activateSchedItem(varsmap& vmap, const char*uri, const char* turi, const char* name, const char*cjtmp)
{
    char*rbuf;

    cJSON* cji = nullptr;
    if (cjtmp)
    {
        cji = cJSON_Parse(cjtmp);
        FPS_ERROR_PRINT("%s >> cjtmp [%s] cji %p\n", __func__, cjtmp, cji);
    }
    if (!cji)
    {
        cji = cJSON_CreateObject();
    }
    asprintf(&rbuf, "%s:%s", turi, name);
    cJSON_AddStringToObject(cji, "value", rbuf);
    cJSON_AddStringToObject(cji, "var", rbuf);
    free(rbuf);
    char * cjbuf = cJSON_PrintUnformatted(cji);
    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"add_item\":%s}}", uri,  cjbuf);
    FPS_ERROR_PRINT("%s >> cjbuf [%s] rbuf [%s]\n", __func__, cjbuf, rbuf);

    free(cjbuf);
    fims_message *msg = vm->bufferToFims((const char *)rbuf);
    if(!msg)
    {
        FPS_ERROR_PRINT(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        return 0;
    }
    cJSON* cj =cJSON_CreateObject();
    vm->processFims(vmap, msg, &cj);
    char *res=cJSON_Print(cj);
    {
        if(res)
        {
            FPS_ERROR_PRINT(" %s >> got res [%s]\n"
            , __func__
            , res
            );
            free(res);
        }
    }
    cJSON_Delete(cj);
    cJSON_Delete(cji);
    vm->free_fims_message(msg);

    free(rbuf);
    return 0;
}


// add a schedItem into the list of reqs at the proper slot
int scheduler::addSchedReq(schlist &sreqs, schedItem* si)
{
    assetVar *av = si->av; 
    if(!av)
    {
        FPS_ERROR_PRINT(" %s >> assetVar NOT Setup\n"
            , __func__
            );
        return -1;

    }
    if(!av->gotParam("runTime"))
    {
        FPS_ERROR_PRINT(" %s >> assetVar [%s] does not have a \"runTime\" param use scheduler::setupSchedItem \n"
        , __func__
        , av->name.c_str()
        );

        return -1;
    }

    double tshot = av->getdParam("runTime");

    auto it = sreqs.begin();
    while( it != sreqs.end())
    {
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

double scheduler::getSchedDelay(varsmap &vmap, schlist& rreqs)
{        
    double tNow = vm->get_time_dbl();
    int runit = 1;
    schedItem* as;
    assetVar* av = nullptr;
    double delay = 2.0;
    if (rreqs.size() > 0)
    {
        int oneran = 0;
    
        while(runit)
        {
            as = nullptr;
            av = nullptr;
            if (rreqs.size() > 0)
            {
                as = rreqs.front();
                av = as->av;
                // we could use as->runTime
                if(av->getdParam("runTime") > (tNow -0.000001)) // allow pre sced
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
            if(av)
            {
                int resched = -1;
                tNow = vm->get_time_dbl();
                oneran++;
                if(!av->gotParam("enabled") || av->getbParam("enabled"))
                {
                    if(0)FPS_ERROR_PRINT("%s >> Running one at %2.6f name [%s:%s]  size %d \n", __func__, tNow, av->comp.c_str(), av->name.c_str(),(int)rreqs.size());
                    // this triggers the scheduled item
                    // we need to get the response from the operation 
                    // 
                    vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), tNow);
                    if(av->gotParam("resched"))
                    {
                        resched = av->getiParam("resched");
                    }
                    if(0)FPS_ERROR_PRINT("%s >> Response %d from %2.6f at %2.6f elapsed mS %2.6f \n"
                            , __func__
                            , resched
                            , tNow
                            , vm->get_time_dbl()
                            , (vm->get_time_dbl()-tNow) * 1000.0);
                }
                as->runCnt++;
                av->setParam("runCnt", as->runCnt);

                rreqs.erase(rreqs.begin());
                if(0)FPS_ERROR_PRINT("%s >> new size 1 %d\n", __func__, (int)rreqs.size());
                // could use as->repTime
                if(av->getdParam("repTime")> 0.0)
                {
                    if(0)FPS_ERROR_PRINT(" added av [%s:%s] at %2.6f \n"
                            , av->comp.c_str()
                            , av->name.c_str()
                            , av->getdParam("runTime")+av->getdParam("repTime")
                            );
                    as->runTime = av->getdParam("runTime")+av->getdParam("repTime");
                    av->setParam("runTime",as->runTime);
                    // may need to check resched;
                    addSchedReq(rreqs, as);// new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->args));    
                }
                else
                {
                    delete as;
                }

                if(0)FPS_ERROR_PRINT(" new size 2 %d\n", (int)rreqs.size());
            }
        }
        if(0&& oneran)FPS_ERROR_PRINT("Done running at %2.6f   ran %d\n", tNow, oneran);
    }
    // now pick off the next delay
    if (rreqs.size() > 0)
    {
        tNow = vm->get_time_dbl();

        as = rreqs.front();
        av = as->av;
        double newd = (av->getdParam("runTime") - tNow);
        delay = newd+0.0005;
        if(0)FPS_ERROR_PRINT("  >>> calc new delay %3.6f = %3.6f \n", newd, delay);
    }
    if(delay < 0.0)
        delay = 0.0;
    if(delay > 2.0)
        delay = 2.0;
    return delay;
}
int scheduler::fimsDebug1(varsmap& vmap, fims_message *msg, asset_manager* am)
{
    if(msg)
    {
        double tNow = vm->get_time_dbl();
        double msize = 0.0;
        if(msg->body)
            msize = (double)strlen(msg->body);
        if (1)FPS_ERROR_PRINT(" %s %2.3f   >>>>BEFORE FIMS MESSAGE  method [%s] replyto[%s]  uri [%s] msize %f\n"
            , __func__
            , tNow
            , msg->method
            , msg->replyto ? msg->replyto : "No Reply"
            , msg->uri
            , msize
            );
        //bool bval = false;
        fLog = new essPerf(am, "ess_test", "fimsLog2", nullptr);
        //if(msg)
        // char *ess_uri = nullptr;
        // asprintf(&ess_uri,"uri_%s",msg->uri);
        // uriLog = nullptr;
        // //this causes loads of problems
        // //ePerf_uri = new essPerf(am, "ess_uri", "msg->uri", nullptr);
        // free(ess_uri);
        
        const char *uri_meth = "unknown";
        if (strcmp(msg->method,"set") == 0)
        {
            uri_meth = "uri_set";
        }
        else if (strcmp(msg->method,"get") == 0)
        {
            uri_meth = "uri_get";
        }
        else if (strcmp(msg->method,"pub") == 0)
        {
            uri_meth = "uri_pub";
        }
        //essPerf ePerf2(am, "ess_test", uri_meth, nullptr);
        
        FPS_ERROR_PRINT(" vmap before fims  [%s]...\n", uri_meth );

        FPS_ERROR_PRINT(" first level map\n");
        for ( auto x: vmap)
        {
            FPS_ERROR_PRINT(" x.first [%s]\n", x.first.c_str());
            for (auto y: x.second)
            {
                FPS_ERROR_PRINT(" x.first [%s] y.first [%s]\n", x.first.c_str(), y.first.c_str());

            }
        }
        

        cJSON* cj = vm->getMapsCj(vmap, nullptr, nullptr, 0x00);
        if(cj) 
        {
            char* res = cJSON_Print(cj);
            if(res)
            {
                FPS_ERROR_PRINT(" Vmap \n%s\n", res);
                free(res);
            }
            cJSON_Delete(cj);
        }
    }
    return 0;
}

int scheduler::fimsDebug2(varsmap& vmap, fims_message *msg,asset_manager*am)
{
    if(msg)
    {
        if (0)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  <<<<AFTER FIMS MESSAGE \n"
            , __func__
            , vm->get_time_dbl()
            );
        FPS_ERROR_PRINT("%s >> vmap after fims  ...\n", __func__);
        FPS_ERROR_PRINT("%s >> first level map\n", __func__);
        for ( auto x: vmap)
        {
            FPS_ERROR_PRINT("%s >> x.first [%s]\n", __func__, x.first.c_str());
            for (auto y: x.second)
            {
                FPS_ERROR_PRINT("%s >> x.first [%s] y.first [%s]\n", __func__, x.first.c_str(), y.first.c_str());

            }
        }
        cJSON* cj =vm->getMapsCj(vmap, nullptr, nullptr, 0x00);
        if(cj) 
        {
            char* res = cJSON_Print(cj);
            if(res)
            {
                FPS_ERROR_PRINT("%s >> Vmap \n%s\n", __func__, res);
                free(res);
            }
            cJSON_Delete(cj);
        }
    }
    {    
        if(uriLog)delete uriLog;

        if(fLog)delete fLog; 
        uriLog = nullptr;
        fLog = nullptr;
    }
    return 0;
}
#endif

void fimsThread(scheduler *sc, const char *mysubs[], int sublen)
{
    //scheduler *sc = (scheduler *) data;
    double tNow =  sc->vm->get_time_dbl();
    int tick = 0;
    //sc->fimsSetup (mysubs, sublen, "SchedTest");
    
    while (*sc->run)
    {
        fims_message* msg = sc->p_fims->Receive_Timeout(1000000);
        if(1)
        {
            // just for testing
            tNow =  sc->vm->get_time_dbl();
            if(0)FPS_ERROR_PRINT("%s >>Fims Tick %d msg %p p_fims %p time %2.3f\n"
                    , __func__
                    , tick
                    , msg
                    , sc->p_fims
                    , tNow
                    );
            tick++;
            if(tick > 15) 
            {
                *sc->run = 0;
                sc->wakeChan.put(tick);   // but this did not get serviced immediatey
            }
        }
        if (msg)
        {
            if (strcmp(msg->method, "get") == 0)
            {
                if (1)FPS_ERROR_PRINT("%s >> %2.3f GET uri  [%s]\n"
                    , __func__, vm.get_time_dbl(), msg->uri);
            }
            if(*sc->run) 
            {
                sc->fimsChan.put(msg);
                sc->wakeChan.put(0);   // but this did not get serviced immediatey
            }
            else
            {
                if (sc->p_fims)sc->p_fims->free_message(msg);
                 //p_fims->delete
            }
        }
    }
    FPS_ERROR_PRINT("%s >> fims shutting down\n"
        , __func__
        );
    if(sc->p_fims)delete sc->p_fims;
    sc->p_fims = nullptr;
}


//int scheduler::runChans(varsmap &vmap, schlist &rreqs, asset_manager* am)
void schedThread(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am)
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

    while (*sc->run)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the timeout specified
        //bflag = 
        sc->wakeChan.timedGet(wakeup, delay);
        essPerf * essLog = new essPerf(am, "ess_test", "WakePerf", nullptr);
        tNow = sc->vm->get_time_dbl();
        if(0)FPS_ERROR_PRINT("%s >> Sched Tick   at %2.6f\n",__func__, tNow);
        if(0)sc->showReqs(*rreqs);

        if(sc->msgChan.get(cmsg, false)) {
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
        if(sc->reqChan.get(si, false)) {

            FPS_ERROR_PRINT("%s >> Really Adding  Sched Request [%s]  at %2.3f\n", __func__, si->id, tNow);
            //delete as;
            sc->addSchedReq(*rreqs, si);
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (sc->fimsChan.get(msg, false))
        {
            if(msg)
            {
                if(1)sc->fimsDebug1(*vmap, msg, am);

                //double tNow = vm->get_time_dbl();
                sc->vm->syscVec = &syscVec;
                //bool runFims = true;
                sc->vm->runFimsMsg(*vmap, msg, sc->p_fims);
                if(1)sc->fimsDebug2(*vmap, msg, am);
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



#if defined _MAIN
// functions              
extern "C" {
    int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
}


int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
{
    FPS_ERROR_PRINT("\n\n***** %s >> some function called %s NOW running for av [%s:%s]\n\n"
        , __func__
        , __func__
        , av->comp.c_str()
        , av->name.c_str()
    );
    return 0;
}
// have to get it from value since the objects are not yet added
// this takes the assetVar and  creates the assetSched item and triggers the channel write
// things like repTime and runTime are left in place as params and used 
// in the sched list

// this replaces the code for 
//  if(si2) rc = sched.addSchedReq(schreqs, si2);
// but does it by sending the si to the reqChan and a wake to the wakeChan
// the function process_sched_item is associate with set /schedule/ess:add_item
// the following config item does that.
//  "/schedule/ess": {
//        "note": "This sets up the method to add scheduler functions in the system",
//        "add_item": {
//            "value": 0,
//            "actions": {
//                "onSet": [{"func": [{"func": "add_sched_item"}]
//                    }]}},


// followed by this config items
// set /schedule/ess:add_item 
//                '{"id":"some_id", 
//                 "aname":"asset_name", 
//                 "var":"/sched/ess:every100mS", 
//                 "nopefunction":"periodic_scan_100", 
//                 "function":"send_time_to_var", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'
// see also
//int setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item",am)
//setupSchedFinction
// int scheduler::setupSchedFunction(varsmap& vmap, const char* uri, const char* oper, const char*func, asset_manager* am)
// // int setup_sched_function(varsmap& vmap, const char* uri, const char* oper, const char*func, asset_manager* am)
// {
//     char*rbuf;

//     asprintf(&rbuf,
//        "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
//        "{\"%s\":{\"value\":\"test string\",\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"%s\",\"amap\":\"ess\"}]}]}}}}", uri, oper, func);
//     fims_message *msg = am->vm->bufferToFims((const char *)rbuf);
//     if(msg)
//     {
//         FPS_ERROR_PRINT("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
//                , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
//                , msg->method?msg->method:"no method"
//                , msg->body?msg->body:"no body" );
//     }
//     else
//     {
//         FPS_ERROR_PRINT(" no message from \n>>>%s<<<\n", rbuf);
//         free(rbuf);
//         return 0;
//     }

//     if(msg)
//     {
//         cJSON* cj =cJSON_CreateObject();
//         am->vm->processFims(vmap, msg, &cj);
//         char *res=cJSON_Print(cj);
//         {
//             if(res)
//             {
//                 FPS_ERROR_PRINT(" %s >> got res [%s]\n"
//                 , __func__
//                 , res
//                 );
//                 free(res);
//             }
//         }
//         cJSON_Delete(cj);
//     }
//     free(rbuf);
//     assetVar* av = am->vm->getVar(vmap,uri, oper);
//     av->am = am;
//     am->vm->free_fims_message(msg);
//     return 0;
// }

// set /schedule/ess:add_item 
//                '{"id":"some_id", 
//                 "aname":"asset_name", 
//                 "var":"/sched/ess:every100mS", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'
int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    cJSON *cj = nullptr; //cJSON_Parse(av->getcVal());
    cJSON *cji = nullptr;// cJSON_GetObjectItem(cj,"function");
    cJSON *cjrun = nullptr;// cJSON_GetObjectItem(cj,"function");
    cJSON *cjrep = nullptr;// cJSON_GetObjectItem(cj,"function");

    if(av->getcVal())
    {
        cj = cJSON_Parse(av->getcVal());
        if(cj)
        {
            cji = cJSON_GetObjectItem(cj,"var");
            cjrun = cJSON_GetObjectItem(cj,"runTime");
            cjrep = cJSON_GetObjectItem(cj,"repTime");
        }
        FPS_ERROR_PRINT("%s >> string value [%s] cj %p cjrun %p cjrep %p\n"
            , __func__, av->getcVal(), cj, cjrun, cjrep);

        FPS_ERROR_PRINT("%s >> string value [%s] runTime %2.3f repTime %2.3f var [%s]\n"
            , __func__, av->getcVal(),av->getdParam("runTime"),av->getdParam("repTime"),av->getcParam("var")?av->getcParam("var"):"No Var");
    } 
    char * aV = nullptr;
    if(cji)
    {
        aV = cji->valuestring;
    }
    else if(cj)
    {
        aV = cj->valuestring;
    }
    else
    {
        aV=av->getcVal();
    }
    asset_manager* am = av->am;

    FPS_ERROR_PRINT("\n\n***** %s >> running for  av [%s:%s] am %p cj %p  aV [%s] value [%s]\n\n"
        , __func__
        , av->comp.c_str()
        , av->name.c_str()
        , am
        , cj//av->getcParam("function")
        , aV?aV:"NoAv Here"
        , av->getcVal()?av->getcVal():" No string val"
    );
    if(cj)cJSON_Delete(cj);
    schedItem *as = nullptr;
    assetVar* avi=nullptr;
    double tNow = vm.get_time_dbl(); 
    avi = vm.getVar(*am->vmap, aV, nullptr);
    if(avi && am)
    {
        as = new schedItem();
        FPS_ERROR_PRINT("%s >> we added %p\n", __func__, as);
        as->av = avi;

        if(avi->am)
            am = avi->am;

        char* myid = avi->getcParam("id");
        if (!myid) myid=(char *)"NoID2"; 

        // this dummy function will simply send the current time to avi triggering its onSet actions
        // infact we'll do this anyway without a function
        // we can get the default values or use new ones from the incoming request
        // after we are done we set all the incoming params used to < 0 
        double runTime = avi->getdParam("runTime");
        double refTime = avi->getdParam("refTime");
        double repTime = avi->getdParam("repTime");
        
        if (av->getdParam("refTime") >= 0.0)
        {
            refTime = av->getdParam("refTime");
            av->setParam("refTime", -1.0);
            avi->setParam("refTime", refTime);
        }

        if (av->getdParam("repTime") >= 0.0)
        {
            refTime = av->getdParam("repTime");
            av->setParam("repTime", -1.0);
            avi->setParam("repTime", repTime);
        }

        // if we specify a run time in the av data then this in an increment from tNow
        if (av->getdParam("runTime") >= 0.0)
        {
            runTime = av->getdParam("runTime");
            runTime+=tNow;
            av->setParam("runTime", -1.0);
            avi->setParam("runTime", runTime);
        }
        //void schedItem::setUp(const char * _id, const char* _aname, const char * _comp, const char* _name, double _refTime,
        //    double _runTime, double _repTime)
        as->setUp(myid, am->name.c_str(), avi->comp.c_str(), avi->name.c_str(), refTime, runTime, repTime);
    
        as->show();
        // we'll need the sched thread listening on these channels
        // channel <int> *wakeChan;
        // channel <schedItem*>*reqChan; // anyone can post run_reqs
        if (am->reqChan)
        {
            channel <schedItem*>* reqChan = (channel <schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                channel <int>* wakeChan = (channel <int>*)am->wakeChan;
                wakeChan->put(0);
            }
        }
        else
        {
            FPS_ERROR_PRINT(" @@@@@ %s >> You need to set up am->reqChan and am->wakeChan   @@@@\n"
                , __func__
                );
        }
        FPS_ERROR_PRINT("@@@@@@ %s>> activated %s   avi %p runTime %2.3f \n\n"
            , __func__
            , aV
            , avi
            , runTime
            );
            avi->setParam("active",true);
    }
    else
    {
        FPS_ERROR_PRINT("@@@@@ %s did not find var to activate [%s]\n", __func__, aV?aV:"No aV");
    }
    return 0;
}
// // todo create a req and send it to the channel in hte am->reqChan
// int  xxadd_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
// {
//     FPS_ERROR_PRINT("\n\n***** %s >> some function called %s NOW running for av [%s:%s]\n\n"
//         , __func__
//         , __func__
//         , av->comp.c_str()
//         , av->name.c_str()
//     );
//     return 0;
// }

int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
{
    FPS_ERROR_PRINT("***** %s >> NOW running for av [%s:%s]\n\n"
        , __func__
        , av->comp.c_str()
        , av->name.c_str()
    );
    return 0;
}

int running = 1;
varsmap funMap;
int main_test_new_sched(int argc, char *argv[])
{
    //double maxTnow = 15.0;

    vm.vmapp= &vmap;
    vm.funMapp = &funMap;
    //double tNow = vm.get_t
    double tNow = vm.get_time_dbl();
     
    // chan2_data t_data;  // timer ( do we need this)
    // chan2_data m_data;  // lets keep this 
    //chan2_data f_data;  // fims data 
    asset_manager *am = new asset_manager("ess");
    am->vm = &vm;
    am->vmap = &vmap;
    vm.setFunc(*am->vmap, "ess", "run_some_func", (void*)&run_some_func);
    vm.setFunc(*am->vmap, "ess", "add_sched_item", (void*)&add_sched_item);
    vm.setFunc(*am->vmap, "ess", "CheckMonitorVar", (void*)&CheckMonitorVar);

    void* res1 = vm.getFunc(*am->vmap, "ess", "run_some_func");

    FPS_ERROR_PRINT(" func %p res1 %p\n"
        ,(void*)&run_some_func 
        , res1
        );
    // tempo until we read in a config file
    
   //chan2_data r_data;  // for new run reqs
//    // these are the ain system level chanels
//     channel <int> wakechan;
//     channel <char*>msgchan;   // anyone can post messages 
//     channel <assetSchedItem*>reqChan; // anyone can post run_reqs
//     channel <fims_message*>fimsChan; // anyone can post run_reqs


    assetVar *av = vm.setVal(vmap, "/sched/ess", "single0_1", tNow);
    
    //av->addSchedReq(avreqs, tNow+0.1, 0.0);
    av->am = am;
    am->vm = &vm;
    // am->wakeUpChan = &wakechan;
    // am->reqChan = &reqChan;
    // am->fimsChan = &fimsChan;
    // am->schreqs  = &avreqs;

    // put thse in the asset_manager.

    // channel <int> *wakeUpChan;
    // channel <assetSchedItem*>*reqChan; // anyone can post run_reqs
    // channel <fims_message*>*fimsChan; // anyone can post run_reqs

// the function prcess_sched_item is associate with set /schedule/ess:add_item
// the following config item does that.
//  "/schedule/ess": {
//        "note": "This sets up the method to add scheduler functions in the system",
//        "add_item": {
//            "value": 0,
//            "actions": {
//                "onSet": [{"func": [{"func": "add_sched_item"}]
//                    }]}},
// followed by this config items
// set /schedule/ess:add_item 
//                '{"id":"some_id", 
//                 "aname":"asset_name", 
//                 "var":"/sched/ess:every100mS", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'


    // first test create a sched item

    // schedItem *si = new schedItem();
    // si->setUp("item_id", "item name", "/sched/ess","single0_1", "run_some_func", 0.25, 0.1 );
    // si->func = vm.getFunc(*am->vmap, "ess", "run_some_func");
    // si->av = vm.getVar(*am->vmap, "/sched/ess:single0_1", nullptr);

    // //if(res1) as->runItem(av);

    // FPS_ERROR_PRINT(" sched Item ...\n");
    // si->show();

    // delete si;

    // Second test , add it to a sched list 
    //a scheduler will do that
    int running = 1;
    scheduler sched("mysched", &running);
    //typedef std::vector<assetSchedItem*>schlist;
    schlist schreqs;
    schreqs.reserve(64);
    //sched.schreqs = &schreqs;
    sched.vm = &vm;
   //sched.vmap = &vmap;
    am->wakeChan = &sched.wakeChan;
    am->reqChan = (void*)&sched.reqChan;


    int rc = 0;
    //rc = sched.addSchedReq(si);
    //FPS_ERROR_PRINT(" add sched Item result 1 %d...\n", rc);
    schedItem* si = sched.setupSchedItem(*am->vmap, am, "/sched/ess", "single0_1" , "Single_Shot", "ess", 0.25, 0.25 , 0.0);

    if(si) rc = sched.addSchedReq(schreqs, si);
    FPS_ERROR_PRINT(" add sched Item result si %p  2 %d...\n", si,  rc);

    schedItem* si2 = nullptr;
    sched.setupSchedVar(*am->vmap, am, "/sched/ess", "every1S" , "EverySecond", "ess", 0.25, 0.25 , 1.0);
    sched.setupSchedVar(*am->vmap, am, "/sched/ess", "futureOneShot" , "FutureOne", "ess", 0.25, 0.25 , 0.0);
    //sched.cja = nullptr;
    cJSON* cja = cJSON_CreateArray();
    sched.addSaction(vmap, "ess", "CheckMonitorVar", "/status/ess:maxTemp",   am, cja);
    sched.addSaction(vmap, "ess", "CheckMonitorVar", "/status/ess:maxVolts",  am, cja);
    sched.addSaction(vmap, "ess", "CheckMonitorVar", "/status/ess:maxCurrent",am, cja);
    // TODO we are going to have to make this "non destructive"
    //sched.setupSchedItemActions(vmap, am, "/sched/ess", "every1S", "onSet@1", "func", cja);
    sched.setupSchedItemActions(vmap, am, "/sched/ess", "every1S", "onSet", "func", cja);
    cJSON_Delete(cja);
    // this makes /schedule/ess:add_item activate a schedule item when it is sent some details
    sched.setupSchedFunction(vmap, "/schedule/ess", "add_item", "add_sched_item", am);


    // now use it to create a schedule var
    sched.activateSchedItem(vmap, "/schedule/ess", "/sched/ess", "every100mS");
    sched.activateSchedItem(vmap, "/schedule/ess", "/sched/ess", "every1S");

    sched.activateSchedItem(vmap, "/schedule/ess", "/sched/ess", "futureOneShot", "{\"runTime\":3.45}");

//int activate_sched_item(vmap, "/schedule/ess", "sched/ess", "every100mS")
    //int activate_sched_item(vmap, "/schedule/ess", "sched/ess", "every100mS")

    //now writes to this /schedule/ess:add_item will set up a schedule operation.
    // once we have am->writeChan and am->reqChan setup..

    // add_action(cja,"ess","CheckMonitorVar","/status/ess/maxTemp");
// add_action(cja,"ess","CheckMonitorVar","/status/ess/maxVolts");
// add_action(cja,"ess","CheckMonitorVar","/status/ess/maxCurrent");
// setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);

    if(si2) rc = sched.addSchedReq(schreqs, si2);
    FPS_ERROR_PRINT(" add sched Item 2 result si %p  2 %d...\n", si2,  rc);


    // Now show the full list
    // TODO embed req list perhaps
    sched.showReqs(schreqs);

    //Now get the run delay
    int delay =     sched.getSchedDelay(vmap, schreqs);

    FPS_ERROR_PRINT(" sched delay = %d...\n", delay);
    poll(nullptr,0,delay+1);
    delay =     sched.getSchedDelay(vmap, schreqs);

    FPS_ERROR_PRINT(" sched delay 2 = %d...\n", delay);
    sched.showReqs(schreqs);


    //setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item", am);


    // //int setup_sched_item(varsmap& vmap, const char*name , const char* id, const char*aname, double runTime , double reptime)
    // FPS_ERROR_PRINT("\n\n running set up sched itemses ...\n");
    // setup_sched_item(vmap,"/schedule/ess","every100mS","ID100mS","ess", 0.5, 0.1);
    // setup_sched_item(vmap,"/schedule/ess","every50mS","ID50mS","ess", 0.5, 0.05);
    // setup_sched_item(vmap,"/schedule/ess","every1S","ID1S","ess", 0.5, 1.0);

    // cJSON * cja = cJSON_CreateArray();
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxTemp", am);
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxVolts", am);
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxCurrent",am);
    // setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);
    // cJSON_Delete(cja);


    // // now setting a value to "/schedule/ess:every100mS" shouldtrigger all the checkMonitorVar actions
    // tNow = vm.get_time_dbl();
    // vm.setVal(vmap,"/schedule/ess:every100mS", nullptr, tNow);
    // ///

    // FPS_ERROR_PRINT("\n\n running activate sched item  ...\n");
    // activate_sched_item(vmap, "/schedule/ess", "/schedule/ess", "every100mS");
    // // setup_sched_item(vmap,2);

    // FPS_ERROR_PRINT("\n\n vmap at end  2 ...\n");
    // cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    // if(cj) 
    // {
    //     char* res = cJSON_Print(cj);
    //     if(res)
    //     {
    //         FPS_ERROR_PRINT(" Vmap \n%s\n", res);
    //         free(res);
    //     }
    //     cJSON_Delete(cj);
    // }

 
    // runChans(am);
    FPS_ERROR_PRINT("\n\n test done ...\n");

    // next set up the fims listener thread.
    // std::thread
    const char* subs[] = {
        "/ess", 
        "/control",
        "/sched"
        };
    int sublen = sizeof subs / sizeof subs[0];
    sched.fimsSetup (subs, sublen, "SchedTest");
    //ess_man->p_fims = sched.p_fims;
    std::thread fThread(fimsThread, &sched, subs, sublen);
    std::thread sThread(schedThread, &sched, &vmap, &schreqs, am);
    fThread.join();
    sThread.join();

    sched.clearReqs(schreqs);

    FPS_ERROR_PRINT("\n\n vmap at end  2 ...\n");
    cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj) 
    {
        char* res = cJSON_Print(cj);
        if(res)
        {
            FPS_ERROR_PRINT(" Vmap \n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj);
    }

    vm.clearVmap(vmap);
    
    am->p_fims = nullptr;
   
    delete am;

    //delete si;
    if(si2) delete si2;
    return 0;

}

int main(int argc, char *argv[])
{
   return  main_test_new_sched(argc, argv);
}
#endif


/////////////////////// new ess_controller
#if defined _MAIN_ESS
// this is the full ess controller under new management
// functions              

extern "C" {
  //  int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
  //  int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
}


// int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
// {
//     FPS_ERROR_PRINT("\n\n***** %s >> some function called %s NOW running for av [%s:%s]\n\n"
//         , __func__
//         , __func__
//         , av->comp.c_str()
//         , av->name.c_str()
//     );
//     return 0;
// }
// have to get it from value since the objects are not yet added
// this takes the assetVar and  creates the assetSched item and triggers the channel write
// things like repTime and runTime are left in place as params and used 
// in the sched list

// this replaces the code for 
//  if(si2) rc = sched.addSchedReq(schreqs, si2);
// but does it by sending the si to the reqChan and a wake to the wakeChan
// the function process_sched_item is associate with set /schedule/ess:add_item
// the following config item does that.
//  "/schedule/ess": {
//        "note": "This sets up the method to add scheduler functions in the system",
//        "add_item": {
//            "value": 0,
//            "actions": {
//                "onSet": [{"func": [{"func": "add_sched_item"}]
//                    }]}},


// set /schedule/ess:add_item 
//                '{"id":"some_id", 
//                 "aname":"asset_name", 
//                 "var":"/sched/ess:every100mS", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'
// int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
// {
//     cJSON *cj = nullptr; //cJSON_Parse(av->getcVal());
//     cJSON *cji = nullptr;// cJSON_GetObjectItem(cj,"function");
//     if(av->getcVal())
//     {
//         cj = cJSON_Parse(av->getcVal());
//         if(cj)cji = cJSON_GetObjectItem(cj,"aV");
//     } 
//     char * aV = nullptr;
//     if(cji)
//     {
//         aV = cji->valuestring;
//     }
//     else if(cj)
//     {
//         aV = cj->valuestring;

//     }
//     else
//     {
//         aV=av->getcVal();
//     }
//     asset_manager* am = av->am;

//     FPS_ERROR_PRINT("\n\n***** %s >> running for  av [%s:%s] am %p cj %p  aV [%s] value [%s]\n\n"
//         , __func__
//         , av->comp.c_str()
//         , av->name.c_str()
//         , am
//         , cj//av->getcParam("function")
//         , aV?aV:"NoAv Here"
//         , av->getcVal()?av->getcVal():" No string val"
//     );
//     if(cj)cJSON_Delete(cj);
//     schedItem *as = nullptr;
//     assetVar* avi=nullptr;

//     //double tNow = vm.get_time_dbl();

//     avi = vm.getVar(*am->vmap, aV, nullptr);
//     if(avi && am)
//     {
//         as = new schedItem();
//         FPS_ERROR_PRINT("%s >> we added %p\n", __func__, as);


//         as->av = avi;

//         if(avi->am)
//             am = avi->am;

//         char* myid = avi->getcParam("id");
//         if (!myid) myid=(char *)"NoID2"; 

//         // this dummy function will simply send the current time to avi triggering its onSet actions
//         // infact we'll do this anyway without a function
//         as->setUp(myid, am->name.c_str(), avi->comp.c_str(), avi->name.c_str(),  avi->getdParam("runTime"),avi->getdParam("repTime") );
    
//         as->show();
//     //TODO phase out the function here... just send the time to the scheduled assetVar  
//     //let its actions list take care of things.
//     // note we do need to insert /remove an action..
//     // but the function operation is NOT important here
//     // ->put(

//         // we'll need the sched thread listening on these channels
//   // channel <int> *wakeChan;
//     // channel <schedItem*>*reqChan; // anyone can post run_reqs
//         if (am->reqChan)
//         {
//             channel <schedItem*>* reqChan = (channel <schedItem*>*)am->reqChan;
//             reqChan->put(as);
//         }
//         if (am->wakeChan)
//         {
//             channel <int>* wakeChan = (channel <int>*)am->wakeChan;
//             wakeChan->put(0);
//         }
//         else
//         {
//             FPS_ERROR_PRINT(" @@@@@ %s >> You need to set up am->reqChan and am->wakeChan   @@@@\n"
//                 , __func__
//                 );

//         }
//         //TODO let these rip if (am->wakeChan)am->wakeChan->put((int)(tNow * 1000));
//         //am->wakeUpChan->put(0) 
//         //am->wake_up
//         FPS_ERROR_PRINT("@@@@@@ %s>> activated %s   avi %p runTime %2.3f \n\n"
//             , __func__
//             , aV
//             , avi
//             , avi->getdParam("runTime")
//             );
//             avi->setParam("active",true);
//     }
//     else
//     {
//         FPS_ERROR_PRINT("@@@@@ %s did not find var to activate [%s]\n", __func__, aV?aV:"No aV");

//     }
//     return 0;
// }
// // todo create a req and send it to the channel in hte am->reqChan
// int  xxadd_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
// {
//     FPS_ERROR_PRINT("\n\n***** %s >> some function called %s NOW running for av [%s:%s]\n\n"
//         , __func__
//         , __func__
//         , av->comp.c_str()
//         , av->name.c_str()
//     );
//     return 0;
// }

// int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
// {
//     FPS_ERROR_PRINT("***** %s >> NOW running for av [%s:%s]\n\n"
//         , __func__
//         , av->comp.c_str()
//         , av->name.c_str()
//     );
//     return 0;
// }

int running = 1;

extern "C" {
    int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  UpdateSystemTime(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  UpdateSysTime(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int SetupDefaultSched(scheduler*sched, asset_manager* am);

}


//typedef std::vector<assetSchedItem*>schlist;
schlist schreqs;

//typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
varsmap funMap;

std::vector<char*>* syscpVec;

//typedef std::map<std::string, std::vector<std::string>*>vecmap;
vecmap vecs;

int main_test_new_ess(int argc, char *argv[])
{

    syscpVec = new std::vector<char*>;

    if (0 && syscpVec->size() == 0)
    {
        // // HACK for UI
        // std::string sname = "ess";
        // sname += "/summary";
        // syscVec.push_back(std::ref(sname));
        // FPS_ERROR_PRINT(" %s >> addded [%s] to syscVec size %d \n", __func__, sname.c_str(), (int)syscVec.size());
        // sname = "ess";
        // sname += "/ess_1";
        // syscVec.push_back(std::ref(sname));
        //syscVec.push_back(strdup("one"));
        //syscVec.push_back(strdup("two"));
        //syscVec.push_back((char*)strdup((char*)"one"));
        //syscVec.push_back((char*)strdup((char *)"two"));
        char *p = (char *)"one";
        syscpVec->push_back(p);
        p = (char *)"two";
        syscpVec->push_back(p);
        //syscVec.push_back((char*)"two");
    
        FPS_ERROR_PRINT(" %s >> HACK HACK addded syscVec size %d \n", __func__, (int)syscVec.size());
        for ( auto x: *syscpVec)
        {
            FPS_ERROR_PRINT(" we got [%s]\n", x);
        }
        return 0;
    }

    //double maxTnow = 15.0;
    // reserver some scheduler slots , it will add more if needed.
    int running = 1;
    scheduler sched("essSched", &running);
    sched.vm = &vm;

    schreqs.reserve(64);
    
    vm.vmapp   = &vmap;
    vm.funMapp = &funMap;
    //double tNow = vm.get_time_dbl();
    int rc = 0;

    //read config dir

    if(argc > 1)
    {
        vm.setFname(argv[1]);
    }
    else
    {
        vm.setFname("configs");
    }
    
    //vmap.clear();
    // TODO make these more flexible
    char* tmp;
    asprintf(&tmp,"mkdir -p /var/log/ess_controller/run_configs");
    system(tmp);
    free(tmp);

    asprintf(&tmp,"mkdir -p /var/log/ess_controller/run_logs");
    system(tmp);
    free(tmp);

    // TODO make this an asset manager
       //ess_man = new asset_manager("ess_controller");
    asset_manager* ess_man = new asset_manager("ess");

    ess_man->am = nullptr;
    ess_man->vm = &vm;
    ess_man->vecs = &vecs;         // used to store pubs , subs and blocks
    ess_man->syscVec = syscpVec;   // used to store UI publist


    ess_man->running = 1;
    vm.syscVec = syscpVec;

    ess_man->setVmap(&vmap);
    //ess_man->setPmap(&pmap); // pubs map 
    ess_man->running = 1;
    ess_man->reload = 0;
    ess_man->wakeChan = &sched.wakeChan;
    ess_man->reqChan = (void*)&sched.reqChan;

    //auto amap = ess_man->getAmap();  //may not need this 
    FPS_ERROR_PRINT(" Setting defaut Scheduler configuration\n");

    SetupDefaultSched(&sched, ess_man);


    FPS_ERROR_PRINT(" Getting initial configuration\n");
    char* cfgname = vm.getFname("ess_controller.json");
    if(cfgname == nullptr )
    {
        cfgname = vm.getFname("test_ess_config.json");
        FPS_ERROR_PRINT("%s >> ERROR Getting initial configuration   from [%s]\n", __func__, cfgname);
        //return (0);
    }
    FPS_ERROR_PRINT("%s >> Getting initial configuration   from [%s]\n", __func__, cfgname);


    rc = vm.configure_vmap(vmap, cfgname, nullptr, ess_man);
    if ( rc < 0)
    {
        FPS_ERROR_PRINT("%s >> Failed to parse initial configuration from [%s]\n", __func__, cfgname);
        exit (0);
    }
    
    if(cfgname)free(cfgname);

    if(0)
    {
        vm.clearVmap(vmap);
        
        ess_man->p_fims = nullptr;
    
        delete ess_man;
        return 0;
    }

    if(0)
    {
        vm.clearVmap(vmap);
        
        ess_man->p_fims = nullptr;
    
        delete ess_man;
        return 0;
    }

    //syscvec orders the comps for pub to UI interface
    if (1 && syscpVec->size() == 0)
    {
        syscpVec->push_back((char*)"ess/summary");
        syscpVec->push_back((char*)"ess/ess_1");
 
        FPS_ERROR_PRINT(" %s >> HACK HACK to syscVec size %d \n", __func__,  (int)syscpVec->size());
    }
    
    // NOTE use syscVec from now onwards
    if (1)FPS_ERROR_PRINT(" %s >> ess_man >> syscVec size [%d]\n", __func__, (int)syscpVec->size());
    if(0)
    {
        vm.syscVec = nullptr;
        vm.clearVmap(vmap);
        
        ess_man->p_fims = nullptr;
    
        delete ess_man;
        return 0;
    }
    // next is the site controller
    // This loads in the system_controller interface 
    //cJSON* cjsite = cJSON_GetObjectItem(cj, "/site/ess");
    auto ixs = vmap.find("/config/ess_server");
    if (ixs != vmap.end())
    {
        for (auto iy : ixs->second)
        {
            if (iy.second->name == "ess")
            {
                char* cfgname = vm.getFname(iy.second->getcVal());
                //char* cfgname = ; //aVal->valuestring;
                cJSON* cj = vm.get_cjson(cfgname);

                FPS_ERROR_PRINT("%s >> site_ess lets run site config for  [%s] from [%s] file [%s] cj %p\n"
                    , __func__
                    , iy.first.c_str()
                    , iy.second->name.c_str()
                    , cfgname
                    , cj                    
                    );
                // this is for the site interface
                if(cj)vm.loadSiteMap(vmap, cj);
                free(cfgname); // LEAK
                if(cj)cJSON_Delete(cj);
            }
        }     
    }
    else
    {
        if (1)FPS_ERROR_PRINT(" %s >> ess_man >> no site interface found \n", __func__);
    }
    // OK all well up to this point

    if(0)
    {
        vm.syscVec = nullptr;
        vm.clearVmap(vmap);
        
        ess_man->p_fims = nullptr;
    
        delete ess_man;
        return 0;
    }
    const char* subs[] = {
        "/ess", 
        "/control",
        "/sched"
        };
    int sublen = sizeof subs / sizeof subs[0];
    sched.fimsSetup (subs, sublen, "SchedTest");

    fims* p_fims = sched.p_fims;
    ess_man->p_fims = sched.p_fims;

    // I think next we load the asset managers

 // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* ass_man = nullptr;
    
    auto ixa = vmap.find("/config/ess/managers");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        FPS_ERROR_PRINT("%s >> ESS >>We found our assets, we should be able to set up our system\n",__func__);
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

                // TODO add digio
                if (iy.first == "ess" || iy.first == "bms" || iy.first == "pcs" || iy.first == "site")
                {
                    const char* aname = iy.first.c_str();
                    FPS_ERROR_PRINT("%s >> setting up manager [%s]\n",__func__, aname);
                    // TODO get the names from the config file...
                    ass_man = new asset_manager(aname);
                    
                    ass_man->p_fims = p_fims;
                    assetVar*Av = iy.second; 
                    Av->am = ass_man;
                    ass_man->setVmap(&vmap);
                    ass_man->vm = &vm;  //??
                    ass_man->am = ess_man;
                    ass_man->vecs = ess_man->vecs;
                    ass_man->syscVec = ess_man->syscVec;
                    ass_man->wakeChan = &sched.wakeChan;
                    ass_man->reqChan = (void*)&sched.reqChan;


                    //TODO add any Pubs into Vecs
                    //ccnt = 0;
                    if (1)FPS_ERROR_PRINT(" %s >> running with vmap [%p]\n", __func__, &vmap);
                    if (1)FPS_ERROR_PRINT(" %s >> syscVec size [%d]\n", __func__, (int)syscVec.size());
                    // now get the asset_manager to configure itsself
                    if(ass_man->configure(&vmap, fname, aname, syscpVec, nullptr, ass_man) < 0)
                    {
                        FPS_ERROR_PRINT(" %s >> error in [%s] config file [%s]\n", __func__, aname, fname);
                        exit(0);
                    }

                    //int ccntam = 0;
                    //TODO vm.getVList(*ess_man->vecs, vmap, ass_man->amap, aname, "Pubs", ccntam);
                    //FPS_ERROR_PRINT("%s >> setting up a %s manager pubs found %d \n", __func__, aname, ccntam);

                    // add it to the assetList
                    ess_man->addManAsset(ass_man, aname);
                    // TODO set up the channels for the slave asset managers
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
    // syscVec holds the assets in order

    // Call initFunc here to initialize functions for all assets/asset managers
    //initFuncs(ess_man);

    if (1)
    {
        FPS_ERROR_PRINT("%s >> list of assets in syscVec\n", __func__);
        for (int i = 0; i < (int)syscVec.size(); i++)
        {
            FPS_ERROR_PRINT("%s >> idx [%d] name [%s]\n", __func__, i, syscVec[i]);
        }
    }

    {
        const char* fname = "/var/log/ess_controller/run_configs/ess_4_after_assets.json";
        // this a test for our config with links
        cJSON* cjbm = nullptr;
        cjbm = vm.getMapsCj(vmap);
        vm.write_cjson(fname, cjbm);

        //cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        FPS_ERROR_PRINT("Maps (should be ) with links and assets  cjbm %p \n%s\n <<< done\n", (void *)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);
    }

    // We should be all setup run the threads here.
    // next set up the fims listener thread.
    // std::thread

    // const char* subs[] = {
    //     "/ess", 
    //     "/control",
    //     "/sched"
    //     };

    std::thread fThread(fimsThread, &sched, subs, sublen);
    std::thread sThread(schedThread, &sched, &vmap, &schreqs, ess_man);
    fThread.join();
    sThread.join();

    if(1)
    {
        FPS_ERROR_PRINT( " asset [%s]  mapsize %d\n", ess_man->name.c_str(), (int)ess_man->assetManMap.size());
        for ( auto x : ess_man->assetManMap)
        {
            FPS_ERROR_PRINT( " asset [%s]  %p name [%s]\n", x.first.c_str(), x.second, x.second->name.c_str());
        }
//        assetManMap[name] = am;
        
        ess_man->p_fims = nullptr;
        FPS_ERROR_PRINT("%s >>  >>>>>>>>>>>>>start of vecs\n", __func__);
        vm.showvecMap(vecs);
        FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of vecs\n", __func__);
        for ( auto xx:vecs)
        {
            xx.second->clear();
        }
        vecs.clear();
        delete ess_man;
        vm.syscVec = nullptr;
        //vm.clearVmap(vmap);
        return 0;
    }

//     // chan2_data t_data;  // timer ( do we need this)
//     // chan2_data m_data;  // lets keep this 
//     //chan2_data f_data;  // fims data 
//     asset_manager *am = new asset_manager("ess");
//     am->vm = &vm;
//     am->vmap = &vmap;
//     vm.setFunc(*am->vmap, "ess", "run_some_func", (void*)&run_some_func);
//     vm.setFunc(*am->vmap, "ess", "add_sched_item", (void*)&add_sched_item);
//     vm.setFunc(*am->vmap, "ess", "CheckMonitorVar", (void*)&CheckMonitorVar);

//     void* res1 = vm.getFunc(*am->vmap, "ess", "run_some_func");

//     FPS_ERROR_PRINT(" func %p res1 %p\n"
//         ,(void*)&run_some_func 
//         , res1
//         );
    

//     assetVar *av = vm.setVal(vmap, "/sched/ess", "single0_1", tNow);
    
//     //av->addSchedReq(avreqs, tNow+0.1, 0.0);
//     av->am = am;
//     am->vm = &vm;

//     int running = 1;
//     scheduler sched("mysched", &running);
//     schlist schreqs;
//     schreqs.reserve(64);
//     //sched.schreqs = &schreqs;
//     sched.vm = &vm;
//    //sched.vmap = &vmap;
//     am->wakeChan = (void*)&sched.wakeChan;
//     am->reqChan = (void*)&sched.reqChan;


//     int rc = 0;
//     //rc = sched.addSchedReq(si);
//     //FPS_ERROR_PRINT(" add sched Item result 1 %d...\n", rc);
//     schedItem* si = sched.setupSchedItem(*am->vmap, am, "/sched/ess", "single0_1" , "Single_Shot", "ess", 0.25, 0.25 , 0.0);

//     if(si) rc = sched.addSchedReq(schreqs, si);
//     FPS_ERROR_PRINT(" add sched Item result si %p  2 %d...\n", si,  rc);

//     schedItem* si2 = nullptr;
//     sched.setupSchedVar(*am->vmap, am, "/sched/ess", "every1S" , "EverySecond", "ess", 0.25, 0.25 , 1.0);
//     //sched.cja = nullptr;
//     cJSON* cja = cJSON_CreateArray();
//     sched.addSaction(vmap, "ess", "CheckMonitorVar", "/status/ess:maxTemp",   am, cja);
//     sched.addSaction(vmap, "ess", "CheckMonitorVar", "/status/ess:maxVolts",  am, cja);
//     sched.addSaction(vmap, "ess", "CheckMonitorVar", "/status/ess:maxCurrent",am, cja);
//     // TODO we are going to have to make this "non destructive"
//     //sched.setupSchedItemActions(vmap, am, "/sched/ess", "every1S", "onSet@1", "func", cja);
//     sched.setupSchedItemActions(vmap, am, "/sched/ess", "every1S", "onSet", "func", cja);
//     cJSON_Delete(cja);
//     // this makes /schedule/ess:add_item activate a schedule item when it is sent some details
//     sched.setupSchedFunction(vmap, "/schedule/ess", "add_item", "add_sched_item", am);


//     // now use it to create a schedule var
//     sched.activateSchedItem(vmap, "/schedule/ess", "/sched/ess", "every100mS");
//     sched.activateSchedItem(vmap, "/schedule/ess", "/sched/ess", "every1S");

// //int activate_sched_item(vmap, "/schedule/ess", "sched/ess", "every100mS")
//     //int activate_sched_item(vmap, "/schedule/ess", "sched/ess", "every100mS")

//     //now writes to this /schedule/ess:add_item will set up a schedule operation.
//     // once we have am->writeChan and am->reqChan setup..

//     // add_action(cja,"ess","CheckMonitorVar","/status/ess/maxTemp");
// // add_action(cja,"ess","CheckMonitorVar","/status/ess/maxVolts");
// // add_action(cja,"ess","CheckMonitorVar","/status/ess/maxCurrent");
// // setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);

//     if(si2) rc = sched.addSchedReq(schreqs, si2);
//     FPS_ERROR_PRINT(" add sched Item 2 result si %p  2 %d...\n", si2,  rc);


//     // Now show the full list
//     // TODO embed req list perhaps
//     sched.showReqs(schreqs);

//     //Now get the run delay
//     int delay =     sched.getSchedDelay(vmap, schreqs);

//     FPS_ERROR_PRINT(" sched delay = %d...\n", delay);
//     poll(nullptr,0,delay+1);
//     delay =     sched.getSchedDelay(vmap, schreqs);

//     FPS_ERROR_PRINT(" sched delay 2 = %d...\n", delay);
//     sched.showReqs(schreqs);


    //setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item", am);


    // //int setup_sched_item(varsmap& vmap, const char*name , const char* id, const char*aname, double runTime , double reptime)
    // FPS_ERROR_PRINT("\n\n running set up sched itemses ...\n");
    // setup_sched_item(vmap,"/schedule/ess","every100mS","ID100mS","ess", 0.5, 0.1);
    // setup_sched_item(vmap,"/schedule/ess","every50mS","ID50mS","ess", 0.5, 0.05);
    // setup_sched_item(vmap,"/schedule/ess","every1S","ID1S","ess", 0.5, 1.0);

    // cJSON * cja = cJSON_CreateArray();
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxTemp", am);
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxVolts", am);
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxCurrent",am);
    // setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);
    // cJSON_Delete(cja);


    // // now setting a value to "/schedule/ess:every100mS" shouldtrigger all the checkMonitorVar actions
    // tNow = vm.get_time_dbl();
    // vm.setVal(vmap,"/schedule/ess:every100mS", nullptr, tNow);
    // ///

    // FPS_ERROR_PRINT("\n\n running activate sched item  ...\n");
    // activate_sched_item(vmap, "/schedule/ess", "/schedule/ess", "every100mS");
    // // setup_sched_item(vmap,2);

    // FPS_ERROR_PRINT("\n\n vmap at end  2 ...\n");
    // cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    // if(cj) 
    // {
    //     char* res = cJSON_Print(cj);
    //     if(res)
    //     {
    //         FPS_ERROR_PRINT(" Vmap \n%s\n", res);
    //         free(res);
    //     }
    //     cJSON_Delete(cj);
    // }

 
    // runChans(am);
    FPS_ERROR_PRINT("\n\n test done ...\n");

    // // next set up the fims listener thread.
    // // std::thread
    // const char* subs[] = {
    //     "/ess", 
    //     "/control",
    //     "/sched"
    //     };
    // int sublen = sizeof subs / sizeof subs[0];
    // std::thread fThread(fimsThread, &sched, subs, sublen);
    // std::thread sThread(schedThread, &sched, &vmap, &schreqs, am);
    // fThread.join();
    // sThread.join();

    // sched.clearReqs(schreqs);

    FPS_ERROR_PRINT("\n\n vmap at end  2 ...\n");
    cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj) 
    {
        char* res = cJSON_Print(cj);
        if(res)
        {
            FPS_ERROR_PRINT(" Vmap \n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj);
    }

    vm.clearVmap(vmap);
    
    ess_man->p_fims = nullptr;
   
    delete ess_man;

    //delete si;
    //if(si2) delete si2;
    return 0;

}

int main(int argc, char *argv[])
{
   return  main_test_new_ess(argc, argv);
}
#endif
