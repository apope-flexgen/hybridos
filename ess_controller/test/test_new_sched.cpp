/*
* new scheduler test code
* anyone can add a schedule event , the new event  may change the timeout.
* and events 
* put the sched class in asset.h
* put the schedvec in the same place.
* value must be unique
* set /schedule/ess:essStateCheck  '{"value":true,"RunTime": 0.25,"RepTime":0.5,"aV":/status/ess:poweron","function":"CheckEssStatus"}
class assetSched {
    public:

    assetSched();
    ~assetSched();

    void setFromAv(assetVar* inav);
    cJSON* getCj();
    double runTime;
    double repTime;
    int runCnt;
    char *aname;
    char *function;
    void *func;
    char* aV;
    assetVar* av;

}
*/

// #include <pthread.h>
// #include <iostream>
// #include <poll.h>
// #include <sys/socket.h>
// #include <poll.h>
// #include <unistd.h>
// #include <string.h>
// #include <vector>

#include "asset.h"
#include <fims/libfims.h>
#include "channel.h"
#include "varMapUtils.h"
#include "chrono_utils.hpp"

VarMapUtils vm;
varsmap vmap;
std::vector<std::string> sysVec;


typedef std::vector<assetSchedItem*>schlist;
// when moved to an assetVar we expect params runTime and repTime

typedef void(* vLoopc)(void *args);

// TOdo tidy this up
typedef struct chan2_data_t {

    channel<int> *wake_up_chan;
    channel<char*> *message_chan;
    channel<assetSchedItem*>*reqAvChan;
    channel<fims_message*>*fimsChan;
    
    int count;
    int delay;
    int *run;
    assetVar* av;
    //int fd[2];
    fims* p_fims;
    std::thread thread;

} chan2_data;

// use av->addSchedReq(schAvList*avlist)


void show_AvReqs(schlist&rreq)
{
    //int idx = 0;
    for ( auto av : rreq)
    {
        av->show();
        // printf("[%02d] Av name [%s:%s]  runTime %2.3f  repTime %2.3f\n"
        // , idx++
        // , av->comp.c_str()
        // , av->name.c_str()
        // , av->getdParam("RunTime") 
        // , av->gotParam("RepTime")?av->getdParam("RepTime"):0.0 
        // );
    }
    return;
}


fims* fims_setup (chan2_data* td, const char* name)
{
    //const char **subs = getSubs("pcs_1");

    const char* subs[] = {
        "/ess", 
        "/control",
        "/sched"
         };

    int sublen = sizeof subs / sizeof subs[0];
    int attempt = 0;
    td->p_fims = new fims();

    while (!td->p_fims->Connect((char*)name)) 
    {
        poll(nullptr, 0, 1000);
        FPS_ERROR_PRINT("%s >> name %s waiting to connect to FIMS,attempt %d\n"
        , __func__
        , name
        , attempt++
        );
    }
    FPS_ERROR_PRINT("%s >> name %s connected to FIMS at attempt %d\n"
        ,   __func__
        , name
        , attempt
        );

    td->p_fims->Subscribe(subs, sublen);
    return td->p_fims;
}


// slow loop  just provide tick
void timer_loop(void *data)
{
    int rc;
    chan2_data* td = (chan2_data*) data;

    while (*td->run)
    {
        rc = poll(nullptr, 0, td->delay);
        if(0)printf(" poll wake rc = %d\n",rc);
        if(*td->run)
            td->wake_up_chan->put(td->count++);
    }
    printf("%s >> done \n",__func__);
    //pthread_exit(nullptr);
}

// this sends messages 
// NO we cant use vm in the thread
void message_loop(void *data)
{
    char *mdata;
    chan2_data* td = (chan2_data*) data;
    assetVar *av = nullptr;
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);
        if(*td->run)
        {
            double tNow = vm.get_time_dbl();
            asprintf(&mdata, " this is message number %d", td->count++);
            printf("%s >> sending message [%s]\n", __func__, mdata);
            td->message_chan->put(mdata);
            ////av = vm.getVar(vmap,"/sched/ess:state", nullptr);
            if(av)av->setParam("RunTime", tNow+0.1);
            //av->setParam("RepTime", 0.05);
            //av->setParam("RunFun", "TestRunFunc");

            ////td->reqAvChan->put(av);
            td->wake_up_chan->put(0);

        }
    }
    printf("%s >> done \n",__func__);
    //pthread_exit(nullptr);
}

// needs cname , subs
void fims_loop(void* data)
{
    chan2_data* td = (chan2_data*)data;
    fims_setup (td, "SchedTest");
    
    while (*td->run)
    {
        fims_message* msg = td->p_fims->Receive_Timeout(100);
        if (msg)
        {
            if (strcmp(msg->method, "get") == 0)
            {
                if (1)FPS_ERROR_PRINT("%s >> %2.3f GET uri  [%s]\n"
                    , __func__, vm.get_time_dbl(), msg->uri);
            }
            if(*td->run) 
            {
                td->fimsChan->put(msg);
                td->wake_up_chan->put(0);   // but this did not get serviced immediatey
                
            }
            else
            {
                if (td->p_fims)td->p_fims->free_message(msg);
                 //p_fims->delete
            }

        }
    }
    FPS_ERROR_PRINT("%s >> fims shutting down\n"
        , __func__
        );
    if(td->p_fims)delete td->p_fims;
    td->p_fims = nullptr;
    //pthread_exit(nullptr);
}

int run(chan2_data*c_data, vLoopc c_loop, int *rflag, int delay, channel<int> *wake)
{
    c_data->run = rflag;
    c_data->count = 0;
    c_data->delay = delay;
    c_data->wake_up_chan = wake;
    c_data->thread = std::thread(c_loop, c_data);
    return 1;
}

/// functions              
extern "C++" {
    int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
    int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
}


int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
{
    printf("\n\n***** %s >> some function called %s NOW running for av [%s:%s]\n\n"
        , __func__
        , __func__
        , av->comp.c_str()
        , av->name.c_str()
    );
    return 0;
}

int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
{
    printf("\n\n***** %s >> function called %s NOW running for av [%s:%s]\n\n"
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
int  add_sched_item(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
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

    printf("\n\n***** %s >> running for  av [%s:%s] am %p cj %p  aV [%s] value [%s]\n\n"
        , __func__
        , av->comp.c_str()
        , av->name.c_str()
        , am
        , cj//av->getcParam("function")
        , aV?aV:"NoAv Here"
        , av->getcVal()?av->getcVal():" No string val"
    );
    if(cj)cJSON_Delete(cj);
    assetSchedItem *as = nullptr;
    assetVar* avi=nullptr;

    double tNow = vm.get_time_dbl();

    avi = vm.getVar(*am->vmap, aV, nullptr);
    if(avi && am)
    {
        as = new assetSchedItem();

        as->av = avi;

        if(avi->am)
            am = avi->am;

        // this dummy function will simply send the current time to avi triggering its onSet actions
        // infact we'll do this anyway without a function
        as->setUp(avi->getcParam("id"), avi->name.c_str(), aV, "run_some_func", avi->getdParam("runTime"),avi->getdParam("repTime") );
        as->func = vm.getFunc(*am->vmap, "ess", "run_some_func");
    
    
    //TODO phase out the function here... just send the time to the scheduled assetVar  
    //let its actions list take care of things.
    // note we do need to insert /remove an action..
    // but the function operation is NOT important here
    // ->put(

        // we'll need the sched thread listening on these channels

        if (am->reqAvChan)am->reqAvChan->put(as);
        if (am->wakeUpChan)am->wakeUpChan->put((int)(tNow * 1000));
        //am->wakeUpChan->put(0) 
        //am->wake_up
        printf("@@@@@@ %s>> activated %s   avi %p runTime %2.3f \n\n"
            , __func__
            , aV
            , avi
            , avi->getdParam("runTime")
            );
            avi->setParam("active",true);
    }
    else
    {

    }
    return 0;
}
// set /schedule/ess:name
//                '{
//                 "id":"some_id", 
//                 "aname":"asset_name", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'
// we'll have to add actions to this item to make it do anything
// "actions":{"onSet":[{"func":[
//                             {"amap": "ess","func": "add_sched_item","var":"/status/ess/some_var"}
//]}]}
// cJSON * cja = cJSON_CreateArray();
// add_action(cja,"ess","CheckMonitorVar","/status/ess/maxTemp");
// add_action(cja,"ess","CheckMonitorVar","/status/ess/maxVolts");
// add_action(cja,"ess","CheckMonitorVar","/status/ess/maxCurrent");
// setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);
// cJSON_Delete(cja);
void add_action(cJSON *cja, const char* amap,const char* func,const char*var, asset_manager* am=nullptr)
{
    cJSON*cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj, "amap", amap);
    cJSON_AddStringToObject(cj, "func", func);
    cJSON_AddStringToObject(cj, "var", var);
    cJSON_AddItemToArray(cja,cj);
    assetVar*av = vm.getVar(vmap,var,nullptr);
    double dval = 0.0;

    if(!av){
        assetUri my(var, nullptr);

        av = new assetVar(my.Var, my.Uri, dval);
        vmap[av->comp][av->name] = av;
    }
    if(am)
      av->am  = am;
}

int setup_sched_item_actions(varsmap& vmap, const char* uri, const char*name , cJSON* cja)
{
    char*acts = cJSON_PrintUnformatted(cja);
    char*rbuf;
// uri example /schedule/ess
    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"%s\":{\"value\":0.0, \"actions\":{\"onSet\":[{\"func\":%s}]}}}}", uri, name, acts);
    fims_message *msg = vm.bufferToFims((const char *)rbuf);
    if(msg)
    {
        printf("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
               , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
               , msg->method?msg->method:"no method"
               , msg->body?msg->body:"no body" );
    }
    else
    {
        printf(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        free(acts);
        return 0;
    }
    if(msg)
    {
        cJSON* cj =cJSON_CreateObject();
        vm.processFims(vmap, msg, &cj);
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                printf(" %s >> got res [%s]\n"
                , __func__
                , res
                );
                free(res);
            }
        }
        cJSON_Delete(cj);
    }
    free(rbuf);
    free(acts);
    return 0;
}

// set /schedule/ess:name
//                '{
//                 "id":"some_id", 
//                 "aname":"asset_name", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'
// we'll have to add actions to this item to make it do anything
int setup_sched_item(varsmap& vmap, const char* uri, const char*name , const char* id, const char*aname, double runTime , double repTime)
{
    char*rbuf;
// uri example /schedule/ess
    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"%s\":{\"value\":0.0, \"id\":\"%s\","
       "\"aname\":\"%s\","
       "\"runTime\":%f,\"repTime\":%f}}}", uri, name, id, aname, runTime, repTime);
    fims_message *msg = vm.bufferToFims((const char *)rbuf);
    if(msg)
    {
        printf("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
               , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
               , msg->method?msg->method:"no method"
               , msg->body?msg->body:"no body" );
    }
    else
    {
        printf(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        return 0;
    }
    if(msg)
    {
        cJSON* cj =cJSON_CreateObject();
        vm.processFims(vmap, msg, &cj);
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                printf(" %s >> got res [%s]\n"
                , __func__
                , res
                );
                free(res);
            }
        }
        cJSON_Delete(cj);
    }
    free(rbuf);
    return 0;
}


// set /schedule/ess:add_item 
//                '{
//                 "value":"/sched/ess:every100mS"
//                 }'
//int activate_sched_item(vmap, "/schedule/ess", "sched/ess", "every100mS")
//assetSchedItem *as = new assetSchedItem();
    // as->setUp("item_id", "item name", "/sched/ess:single0.1", "run_some_func", 0.25, 0.1 );
    // as->func = vm.getFunc(*am->vmap, "ess", "run_some_func");
    // as->av = vm.getVar(*am->vmap, "/sched/ess:single0.1", nullptr);

int activate_sched_item(varsmap& vmap, const char*uri, const char* auri, const char* name)
{
    char*rbuf;
    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"add_item\":{\"value\":\"%s:%s\"}}}", uri,  auri, name);
    fims_message *msg = vm.bufferToFims((const char *)rbuf);
    if(msg)
    {
        printf("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
               , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
               , msg->method?msg->method:"no method"
               , msg->body?msg->body:"no body" );
    }
    else
    {
        printf(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        return 0;
    }
    if(msg)
    {
        cJSON* cj =cJSON_CreateObject();
        vm.processFims(vmap, msg, &cj);
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                printf(" %s >> got res [%s]\n"
                , __func__
                , res
                );
                free(res);
            }
        }
        cJSON_Delete(cj);
    }
    free(rbuf);
    return 0;
}


//  "/schedule/ess": {
//        "note": "This sets up the method to add scheduler functions in the system",
//        "add_item": {
//            "value": 0,
//            "actions": {
//                "onSet": [{"func": [{"func": "add_sched_item"}]
//                    }]}},
//int setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item",am)

int setup_sched_function(varsmap& vmap, const char* uri, const char* oper, const char*func, asset_manager* am)
{
    char*rbuf;

    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\":"
       "{\"%s\":{\"value\":\"test string\",\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"%s\",\"amap\":\"ess\"}]}]}}}}", uri, oper, func);
    fims_message *msg = vm.bufferToFims((const char *)rbuf);
    if(msg)
    {
        printf("fims  nfrags %d   , fims pfrags [1] [%s] fims method [%s] body [%s]\n"
               , msg->nfrags, msg->nfrags>0?msg->pfrags[1]:"no pfrags"
               , msg->method?msg->method:"no method"
               , msg->body?msg->body:"no body" );
    }
    else
    {
        printf(" no message from \n>>>%s<<<\n", rbuf);
        free(rbuf);
        return 0;
    }
    if(msg)
    {
        cJSON* cj =cJSON_CreateObject();
        vm.processFims(vmap, msg, &cj);
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                printf(" %s >> got res [%s]\n"
                , __func__
                , res
                );
                free(res);
            }
        }
        cJSON_Delete(cj);
    }
    free(rbuf);
    assetVar* av = vm.getVar(vmap,"/schedule/ess:add_item", nullptr);
    av->am = am;
    return 0;
}

// this just runs the chans untill we get done ( say 15 seconds)
int asset_manager::getSchedDelay()
{        
    double tNow = vm->get_time_dbl();
    int runit = 1;
    assetSchedItem* as;
    assetVar* av = nullptr;
    int delay = 2000;
    if (schreqs->size() > 0)
    {
        int oneran = 0;
    
        while(runit)
        {
            as = nullptr;
            if (schreqs->size() > 0)
            {
                as = schreqs->front();
                av = as->av;
                if(av->getdParam("runTime") > tNow)
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
                tNow = vm->get_time_dbl();
                oneran++;
                printf("Running one at %2.3f name [%s:%s]  size %d \n", tNow, av->comp.c_str(), av->name.c_str(),(int)schreqs->size());
                vm->setVal(*vmap,av->comp.c_str(), av->name.c_str(),tNow);
                schreqs->erase(schreqs->begin());
                printf(" new size 1 %d\n", (int)schreqs->size());
                if (av->gotParam("repTime"))
                {
                    if(av->getdParam("repTime")> 0.0)
                    {
                        printf(" added av [%s:%s] at %2.3f \n"
                            , av->comp.c_str()
                            , av->name.c_str()
                            , av->getdParam("runTime")+av->getdParam("repTime")
                            );

                        av->setParam("runTime",( av->getdParam("runTime")+av->getdParam("repTime")));
                        addAmSchedReq(as);// new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->fname,rn->args));
                    }
                }
                printf(" new size 2 %d\n", (int)schreqs->size());
            }
        }
        if(oneran)printf("Done running at %2.3f   ran %d\n", tNow, oneran);

    }
    // now pick off the next delay
    if (schreqs->size() > 0)
    {
        tNow = vm->get_time_dbl();

        as = schreqs->front();
        av = as->av;
        delay = (int)((av->getdParam("runTime") - tNow)*1000);
        if(0)printf("  >>>new delay = %d \n", delay);
    }
    if(delay < 0)
        delay = 0;
    if(delay > 2000)
        delay = 2000;
    return delay;
}

int runChans(asset_manager *am)
{
    int running = 1;
    int delay = 1000; // ms
    int wakeup = 0;
    assetSchedItem *as = nullptr;
    double tNow = 0.0;
    fims_message *msg = nullptr;
    double tStart = vm.get_time_dbl();

    while (running)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the timeout specified
        //bflag = 
        am->wakeUpChan->timedGet(wakeup, delay);
        
        tNow = vm.get_time_dbl();
        printf("Tick   at %2.3f\n",tNow);
        show_AvReqs(*am->schreqs);

        // if(msgchan.get(item3, false)) {
        //     printf("%s  message item3-> data  [%s] \n", __func__,item3);
        //     //use the message to set the next delay
        //     free((void *) item3);
        //     if(tNow>maxTnow)
        //     {
        //         printf("%s >> wakeup value  %2.3f time to stop \n",__func__,tNow);
        //         running = 0;
        //     }
        // }
    
        // handle an incoming avar run request .. avoids too many locks
        if(am->reqAvChan->get(as, false)) {

            printf("Adding  Sched Request [%s]  at %2.3f\n", as->id, tNow);
            //delete as;
            am->addAmSchedReq(as);
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (am->fimsChan->get(msg, false))
        {
            if(msg)
            {
                double tNow = vm.get_time_dbl();
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
                essPerf* ePerf = nullptr;
                ePerf = new essPerf(am, "ess_test", "fimsLog2", nullptr);
                //if(msg)
                char *ess_uri = nullptr;
                asprintf(&ess_uri,"uri_%s",msg->uri);
                essPerf *ePerf_uri = nullptr;
                //this causes loads of problems
                //ePerf_uri = new essPerf(am, "ess_uri", "msg->uri", nullptr);
                free(ess_uri);
                
                // assetVar* logav = getLogAv(am, "ess", "fimsLog");
                // bool bval = logav->getbVal();
                // if(bval)
                //     tNow = am->vm->get_time_dbl();

                // // we need to collect responses
                // //cJSON *cj = nullptr;
                // // either here of the bms instance
                // //bms_man->vmap  
                // assetVar* uriav = nullptr;
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
                
                {
                    printf(" vmap before fims  [%s]...\n", uri_meth );

                    printf(" first level map\n");
                    for ( auto x: vmap)
                    {
                        printf(" x.first [%s]\n", x.first.c_str());
                        for (auto y: x.second)
                        {
                            printf(" x.first [%s] y.first [%s]\n", x.first.c_str(), y.first.c_str());

                        }
                    }
                    

                    cJSON* cj =vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
                    if(cj) 
                    {
                        char* res = cJSON_Print(cj);
                        if(res)
                        {
                            printf(" Vmap \n%s\n", res);
                            free(res);
                        }
                        cJSON_Delete(cj);
                    }
                }

                vm.sysVec = &sysVec;
                bool runFims = true;
                if(runFims)vm.runFimsMsg(vmap, msg, am->p_fims);//am->p_fims);//fims* p_fims)

                if (0)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  <<<<AFTER FIMS MESSAGE \n"
                    , __func__
                    , vm.get_time_dbl()
                    );
                {
                    printf(" vmap after fims  ...\n");
                    printf(" first level map\n");
                    for ( auto x: vmap)
                    {
                        printf(" x.first [%s]\n", x.first.c_str());
                        for (auto y: x.second)
                        {
                            printf(" x.first [%s] y.first [%s]\n", x.first.c_str(), y.first.c_str());

                        }
                    }
                    cJSON* cj =vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
                    if(cj) 
                    {
                        char* res = cJSON_Print(cj);
                        if(res)
                        {
                            printf(" Vmap \n%s\n", res);
                            free(res);
                        }
                        cJSON_Delete(cj);
                    }
                }
                
                if(!runFims)am->p_fims->free_message(msg);
                if(ePerf_uri)delete ePerf_uri; 
                if(ePerf)delete ePerf; 
            }
        }
        delay = 2000;

        int xdelay = am->getSchedDelay();
        printf(" %s new delay = %d\n", __func__, xdelay);
        if ((vm.get_time_dbl() - tStart)  > 15.0) running = 0;

    }
    return 0;

}
int running = 1;
int main_test_new_sched(int argc, char *argv[])
{
    //double maxTnow = 15.0;

    vm.vmapp= &vmap;
    double tNow = vm.get_time_dbl();
     
    chan2_data t_data;  // timer ( do we need this)
    chan2_data m_data;  // lets keep this 
    chan2_data f_data;  // fims data 
    asset_manager *am = new asset_manager("ess");
    am->vm = &vm;
    am->vmap = &vmap;
    vm.setFunc(*am->vmap, "ess", "run_some_func", (void*)&run_some_func);
    vm.setFunc(*am->vmap, "ess", "add_sched_item", (void*)&add_sched_item);
    vm.setFunc(*am->vmap, "ess", "CheckMonitorVar", (void*)&CheckMonitorVar);

    void* res1 = vm.getFunc(*am->vmap, "ess", "run_some_func");

    printf(" func %p res1 %p\n"
        ,(void*)&run_some_func 
        , res1
        );
    // tempo until we read in a config file
    
   //chan2_data r_data;  // for new run reqs
   // these are the ain system level chanels
    channel <int> wakechan;
    channel <char*>msgchan;   // anyone can post messages 
    channel <assetSchedItem*>reqAvChan; // anyone can post run_reqs
    channel <fims_message*>fimsChan; // anyone can post run_reqs
    
    schlist avreqs;

    avreqs.reserve(64);

    assetVar *av = vm.setVal(vmap, "/sched/ess", "single0.1", tNow);
    
    //av->addSchedReq(avreqs, tNow+0.1, 0.0);
    av->am = am;
    am->wakeUpChan = &wakechan;
    am->reqAvChan = &reqAvChan;
    am->fimsChan = &fimsChan;
    am->schreqs  = &avreqs;

    // put thse in the asset_manager.

    // channel <int> *wakeUpChan;
    // channel <assetSchedItem*>*reqAvChan; // anyone can post run_reqs
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
//                 "nopefunction":"periodic_scan_100", 
//                 "function":"send_time_to_var", 
//                 "runTime":0.5,
//                 "repTime":0.1
//                 }'

    assetSchedItem *as = new assetSchedItem();
    as->setUp("item_id", "item name", "/sched/ess:single0.1", "run_some_func", 0.25, 0.1 );
    as->func = vm.getFunc(*am->vmap, "ess", "run_some_func");
    as->av = vm.getVar(*am->vmap, "/sched/ess:single0.1", nullptr);

    if(res1) as->runItem(av);

    as->show();

    printf("\n\n running set up sched function ...\n");
    setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item", am);


    //int setup_sched_item(varsmap& vmap, const char*name , const char* id, const char*aname, double runTime , double reptime)
    printf("\n\n running set up sched itemses ...\n");
    setup_sched_item(vmap,"/schedule/ess","every100mS","ID100mS","ess", 0.5, 0.1);
    setup_sched_item(vmap,"/schedule/ess","every50mS","ID50mS","ess", 0.5, 0.05);
    setup_sched_item(vmap,"/schedule/ess","every1S","ID1S","ess", 0.5, 1.0);

    cJSON * cja = cJSON_CreateArray();
    add_action(cja,"ess","CheckMonitorVar","/status/ess:maxTemp", am);
    add_action(cja,"ess","CheckMonitorVar","/status/ess:maxVolts", am);
    add_action(cja,"ess","CheckMonitorVar","/status/ess:maxCurrent",am);
    setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);
    cJSON_Delete(cja);


    // now setting a value to "/schedule/ess:every100mS" shouldtrigger all the checkMonitorVar actions
    tNow = vm.get_time_dbl();
    vm.setVal(vmap,"/schedule/ess:every100mS", nullptr, tNow);
    ///

    printf("\n\n running activate sched item  ...\n");
    activate_sched_item(vmap, "/schedule/ess", "/schedule/ess", "every100mS");
    // setup_sched_item(vmap,2);

    printf("\n\n vmap at end  2 ...\n");
    cJSON* cj = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj) 
    {
        char* res = cJSON_Print(cj);
        if(res)
        {
            printf(" Vmap \n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj);
    }

 
    runChans(am);
    printf("\n\n runchans done ...\n");

    vm.clearVmap(vmap);
    
    am->p_fims = nullptr;
   
    delete am;

    delete as;
    return 0;
#if 0
    av = vm.setVal(vmap, "/sched/ess", "single0.1", tNow);
    av->addSchedReq(avreqs, tNow+0.1, 0.0);
    av->am = am;

    av = vm.setVal(vmap, "/sched/ess","rep20at60", tNow);
    av->addSchedReq(avreqs, tNow+0.2, 0.2);
    av->am = am;

    av = vm.setVal(vmap, "/sched/ess","rep30at70", tNow);
    av->addSchedReq(avreqs, tNow+1.0, 1.0);
    av->am = am;
    av = vm.setVal(vmap, "/sched/ess","single0.5", tNow);
    av->addSchedReq(avreqs, tNow+0.5, 0.0);
    av->am = am;
    av = vm.setVal(vmap, "/sched/ess","state", tNow);
    av->am = am;
    av->addSchedReq(avreqs, tNow+0.2, 0);

    // pre load some run reqs 
    //addSchedReq(rreqs, new schedReq(tNow+0.1,      0,     "/sched/ess:single0.1", "onSet"));
    //addSchedReq(rreqs, new schedReq(tNow+0.2,    0.2,     "/sched/ess:rep20at60", "onSet"));
    //addSchedReq(rreqs, new schedReq(tNow+1.0,    1.0,     "/sched/ess:rep30at70", "init"));
    //addSchedReq(rreqs, new schedReq(tNow+0.5,    0.0,     "/sched/ess:single0.5", "onSet"));
    // 
    printf( " sched reqs at the start\n");
    show_AvReqs(avreqs);
    //set_reqs_av(rreqs, &vm);


    int wakeup;
    //std::string item2;
    char* item3;

    fims_message* msg = nullptr;

    t_data.message_chan = &msgchan;
    m_data.message_chan = &msgchan;
    m_data.reqAvChan = &reqAvChan;
    f_data.fimsChan = &fimsChan;
    //am->p_fims = fims_setup (&f_data, "FimsTest");
    am->vmap = &vmap;

        //r_data.reqChan = &reqChan;
      
    // timer loop sends us wakeups every 2 secs
    run(&t_data, timer_loop, &running, 2000, &wakechan);

    // message loop sends us wakeups and messages every 1.5 secs
    run(&m_data, message_loop, &running, 1500, &wakechan);

    // fims
    run(&f_data, fims_loop, &running, 100, &wakechan);

    // 
    //bool done = false;
    //bool bflag = false;
    int delay = 100; // ms
    wakeup = 0;
    while (running)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the timeout specified
        //bflag = 
        wakechan.timedGet(wakeup, delay);
        
        tNow = vm.get_time_dbl();

        if(msgchan.get(item3, false)) {
            printf("%s  message item3-> data  [%s] \n", __func__,item3);
            //use the message to set the next delay
            free((void *) item3);
            if(tNow>maxTnow)
            {
                printf("%s >> wakeup value  %2.3f time to stop \n",__func__,tNow);
                running = 0;
            }
        }
    
        // handle an incoming avar run request .. avoids too many locks
        if(reqAvChan.get(av, false)) {

            printf("Adding Av Sched Request [%s]  at %2.3f\n", av->name.c_str(), tNow);
            av->addSchedReq(avreqs);
        }

        // fims
        // this gets incoming  fims messages
        if (fimsChan.get(msg, false))
        {
            if(msg)
            {
                double tNow = vm.get_time_dbl();
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
                essPerf* ePerf = nullptr;
                ePerf = new essPerf(am, "ess_test", "fimsLog2", nullptr);
                //if(msg)
                char *ess_uri = nullptr;
                asprintf(&ess_uri,"uri_%s",msg->uri);
                essPerf *ePerf_uri = nullptr;
                //this causes loads of problems
                //ePerf_uri = new essPerf(am, "ess_uri", "msg->uri", nullptr);
                free(ess_uri);
                
                // assetVar* logav = getLogAv(am, "ess", "fimsLog");
                // bool bval = logav->getbVal();
                // if(bval)
                //     tNow = am->vm->get_time_dbl();

                // // we need to collect responses
                // //cJSON *cj = nullptr;
                // // either here of the bms instance
                // //bms_man->vmap  
                // assetVar* uriav = nullptr;
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
                
                {
                    printf(" vmap before fims  [%s]...\n", uri_meth );

                    printf(" first level map\n");
                    for ( auto x: vmap)
                    {
                        printf(" x.first [%s]\n", x.first.c_str());
                        for (auto y: x.second)
                        {
                            printf(" x.first [%s] y.first [%s]\n", x.first.c_str(), y.first.c_str());

                        }
                    }
                    

                    cJSON* cj =vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
                    if(cj) 
                    {
                        char* res = cJSON_Print(cj);
                        if(res)
                        {
                            printf(" Vmap \n%s\n", res);
                            free(res);
                        }
                        cJSON_Delete(cj);
                    }
                }

                vm.sysVec = &sysVec;
                bool runFims = true;
                if(runFims)vm.runFimsMsg(vmap, msg, f_data.p_fims);//am->p_fims);//fims* p_fims)

                if (0)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  <<<<AFTER FIMS MESSAGE \n"
                    , __func__
                    , vm.get_time_dbl()
                    );
                {
                    printf(" vmap after fims  ...\n");
                    printf(" first level map\n");
                    for ( auto x: vmap)
                    {
                        printf(" x.first [%s]\n", x.first.c_str());
                        for (auto y: x.second)
                        {
                            printf(" x.first [%s] y.first [%s]\n", x.first.c_str(), y.first.c_str());

                        }
                    }
                    cJSON* cj =vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
                    if(cj) 
                    {
                        char* res = cJSON_Print(cj);
                        if(res)
                        {
                            printf(" Vmap \n%s\n", res);
                            free(res);
                        }
                        cJSON_Delete(cj);
                    }
                }
                
                if(!runFims)f_data.p_fims->free_message(msg);
                if(ePerf_uri)delete ePerf_uri; 
                if(ePerf)delete ePerf; 
            }
        }

        // now look at the tasks and run one if needed
        // 
         //rr = pull_req(rreqs, t_now);
        tNow = vm.get_time_dbl();
        int runit = 1;
        if (avreqs.size() > 0)
        {
            int oneran = 0;
            
            while(runit)
            {
                av = nullptr;
                if (avreqs.size() > 0)
                {
                    av = avreqs.front();
                    if(av->getdParam("RunTime") > tNow)
                    {
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
                    oneran++;
                    printf("Running one at %2.3f name [%s:%s]  size %d \n", tNow, av->comp.c_str(), av->name.c_str(),(int)avreqs.size());
                    av->setVal(tNow);
                    avreqs.erase(avreqs.begin());
                    printf(" new size 1 %d\n", (int)avreqs.size());
                    if (av->gotParam("RepTime"))
                    {
                        if(av->getdParam("RepTime")> 0.0)
                        {
                            printf(" added av [%s:%s] at %2.3f \n"
                            , av->comp.c_str()
                            , av->name.c_str()
                            , av->getdParam("RunTime")+av->getdParam("RepTime"));
                            
                            av->addSchedReq(avreqs, av->getdParam("RunTime")+av->getdParam("RepTime"), av->getdParam("RepTime"));
                        //rr->addSchedReq(rreqs, &vm);// new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->fname,rn->args));
                        }
                    }
                    printf(" new size 2 %d\n", (int)avreqs.size());

                    // else
                    // {
                    //     delete rr;
                    // }
                }
            }
            if(oneran)printf("Done running at %2.3f   ran %d\n", tNow, oneran);

        }
        // now pick off the next delay
        if (avreqs.size() > 0)
        {
            tNow = vm.get_time_dbl();

            av = avreqs.front();
            delay = (int)((av->getdParam("RunTime") - tNow)*1000);
            if(0)printf("  >>>new delay = %d \n", delay);

        }
        if(delay < 0)
            delay = 0;
        if(delay > 200)
            delay = 200;
        // if(tNow>maxTnow)
        // {
        //     std::cout << " wakeup value "<< wakeup << " time to stop\n";
        //     running = 0;
        // }
    }

    //pthread_join(t_data.thread,nullptr);

    t_data.thread.join();
    std::cout << " t_data all done " << "\n";
    //pthread_join(m_data.thread,nullptr);
    m_data.thread.join();
    std::cout << "m_data all done " << "\n";

    f_data.thread.join();
    std::cout << "fims all done " << "\n";

    printf( " sched reqs at the end\n");
    show_AvReqs(avreqs);

    printf("at end\n");
    show_AvReqs(avreqs);
    avreqs.clear();
    //clear_reqs(rreqs);
    printf(" vmap at end  1 ...\n");
    printf(" first level map\n");
    for ( auto x: vmap)
    {
        printf(" x.first [%s]\n", x.first.c_str());
        for (auto y: x.second)
        {
            assetVar *av = y.second;
            printf(" x.first [%s] y.first [%s] av %p name [%s] value %2.3f action [%s]\n"
                , x.first.c_str()
                , y.first.c_str()
                , av
                , av->name.c_str()
                , av->getdVal()
                , av->gotParam("action")?av->getcParam("action"):"NoAction"
                );

        }
        cJSON* cjx =vm.getMapsCj(vmap, x.first.c_str(), nullptr, 0x10000);
        if(cjx) 
        {
            char* res = cJSON_Print(cjx);
            if(res)
            {
                printf(" Vmap \n%s\n", res);
                free(res);
            }
            cJSON_Delete(cjx);
        }

    }
    cJSON* cj =vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj) 
    {
        char* res = cJSON_Print(cj);
        if(res)
        {
            printf(" Vmap \n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj);
    }
    printf(" vmap at end  2 ...\n");
    cj =vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj) 
    {
        char* res = cJSON_Print(cj);
        if(res)
        {
            printf(" Vmap \n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj);
    }

    vm.clearVmap(vmap);
    
    am->p_fims = nullptr;

    delete am;

    return 0;

#endif
}

#if defined _MAIN
int main(int argc, char *argv[])
{
   return  main_test_new_sched(argc, argv);
}
#endif

    