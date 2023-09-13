/*
* using Phil's old schedule tester to move function calling out of json files and into code base
* this code allows us to statically add functions into the scheduler
*/

#include "asset.h"
#include <fims/libfims.h>
#include <csignal>

#include "channel.h"
#include "varMapUtils.h"
#include "scheduler.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"

#include "formatters.hpp"

#include "gitinc.h"

namespace flex
{
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

asset_manager* ess_man = nullptr;
int run_secs = 0;
int running = 1;

void signal_handler(int sig)
{
    running = 0;
    FPS_PRINT_INFO("signal of type {} caught.", sig);
    if (ess_man)
    {
        ess_man->running = 0;
        if(ess_man->wakeChan)ess_man->wakeChan->put(-1);
    }
    signal(sig, SIG_DFL);
}

#define LOGDIR "/var/log/ess_controller"

VarMapUtils vm;
varsmap vmap;

schlist schreqs;
std::vector<char *> syscVec;
vecmap vecs;

varsmap funMap;
std::vector<char*>* syscpVec;

cJSON*getSchListcJ(schlist&schreqs);

cJSON*getSchList()
{
    return getSchListcJ(schreqs);
}

typedef void(* vLoopc)(void *args);

const char* FlexName = "ess";
char* essName = (char *)"ess";
const char* FlexDir = "../local/config_c2c/ess_controller";        // was "configs/ess_controller"

// the first sub needs to be the FlexName
const char* FlexSubs = ":/components:/assets:/system:/site:/reload:/misc2:";
const char* FlexConfig = "ess_init"; //nullptr; //"flex_controller.json";
const char* FlexCfile = nullptr;  //"ess_config"; //nullptr; //"flex_controller.json";

typedef struct chan2_data_t {

    channel<int> *wake_up_chan;
    channel<char*> *message_chan;
    channel<schedItem*>*reqChan;
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

            ////td->reqChan->put(av);
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
    int SetupEssSched(scheduler* sched, asset_manager* am);
}

// copied vec functions from ess_controller
const char **vecToList(std::vector<std::string>&subVec, int nkeys)
{
    const char ** vlist=nullptr;
    int i = 0;
    vlist = (const char**)malloc(sizeof(const char*) * (nkeys+1));
    if(nkeys>0)
    {
        for (i = 0; i<nkeys;i++)
        {
            vlist[i] = subVec[i].c_str();
        }
    }
    vlist[i] = nullptr;
    return vlist;
}

void showVecList(const char** vlist)
{
    int i=0;
    while (vlist[i])
    {
        FPS_PRINT_INFO("[{}] :: [{}]", i, vlist[i]);
        i++;
    }
    return;
}

void clearVecList(const char** vlist)
{
    free(vlist);
    return;
}

int  run_some_func(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
{
    double tNow = vm.get_time_dbl();

    FPS_PRINT_INFO(" >>> function {} running for av [{}:{}] at {}\n", 
                    __func__, av->comp.c_str(), av->name.c_str(), tNow
    );

    return 0;
}

int  CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av)
{
    // printf("\n***** >> function called %s NOW running for av [%s:%s]\n"
    //     , __func__
    //     , av->comp.c_str()
    //     , av->name.c_str()
    // );
    return 0;
}

// have to get it from value since the objects are not yet added
// this takes the assetVar and  creates the sched item and triggers the channel write
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
        , cj    //av->getcParam("function")
        , aV ? aV : "NoAv Here"
        , av->getcVal() ? av->getcVal() : " No string val"
    );

    if(cj)cJSON_Delete(cj);
    schedItem *as = nullptr;
    assetVar* avi=nullptr;

    double tNow = vm.get_time_dbl();

    avi = vm.getVar(vmap, aV, nullptr);
    if(avi && am)
    {
        as = new schedItem();

        as->av = avi;

        if(avi->am)
            am = avi->am;

        // this dummy function will simply send the current time to avi triggering its onSet actions
        // infact we'll do this anyway without a function
        as->setUp(avi->getcParam("id"), avi->name.c_str(), aV, "run_some_func", avi->getdParam("refTime"), avi->getdParam("runTime"),
                    avi->getdParam("repTime"), avi->getdParam("endTime"), avi->getcParam("targ")); 
        as->func = (char*)vm.getFunc(*am->vmap, "ess", "run_some_func");
    
    //TODO phase out the function here... just send the time to the scheduled assetVar  
    //let its actions list take care of things.
    // note we do need to insert /remove an action..
    // but the function operation is NOT important here
    // ->put(

        // we'll need the sched thread listening on these channels

        if (am->reqChan){      

            channel <schedItem*>* reqChan = (channel <schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                am->wakeChan->put(0);
            }
        }            // was originally if (am->reqChan)am->reqChan->put(as);

        if (am->wakeUpChan)am->wakeUpChan->put((int)(tNow * 1000));
        if(0) {printf("@@@@@@ %s>> activated %s   avi %p runTime %2.3f \n\n"
            , __func__
            , aV
            , avi
            , avi->getdParam("runTime")
            );
        }
            avi->setParam("active",true);
    }
    else
    {

    }
    return 0;
}

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
        printf("fims message:\nmethod: [%s] | uri: [%s] | replyto: [%s] | body: [%s]  | fims pfrags [1] [%s]\n"
                , msg->method       ? msg->method : "no method"
                , msg->uri          ? msg->uri : "no uri"
                , msg->replyto      ? msg->replyto : "no replyto"
                , msg->body         ? msg->body : "no body"
                // , msg->process_name ? msg->process_name : "no pname"         these dont work and cause faults -> process_name: [%s] | username: [%s] |
                // , msg->username     ? msg->username : "no username"
                , msg->nfrags>0     ? msg->pfrags[1] : "no pfrags"
        );
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
                printf(" %s >> got res [\n%s\n]\n", __func__, res);
                free(res);
            }
        }
        cJSON_Delete(cj);
    }
    free(rbuf);
    return 0;
}

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

int setup_sched_function(varsmap& vmap, const char* uri, const char* oper, const char*func, asset_manager* am)
{
    char*rbuf;

    asprintf(&rbuf,
       "{\"method\":\"set\",\"uri\":\"%s\",\"body\": {\"%s\":{\"value\":\"test_string\",\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"%s\",\"amap\":\"ess\"}]}]}}}}", 
       uri, oper, func);

    // puts(rbuf);      // print was was just read into rbuf
        
    fims_message *msg = vm.bufferToFims((const char *)rbuf);
    if(msg)     // prints msg
    {
        printf("fims message:\nmethod: [%s] | uri: [%s] | replyto: [%s] | body: [%s]  | process_name: [%s] | username: [%s] | fims pfrags [1] [%s]\n"
                , msg->method       ? msg->method : "no method"
                , msg->uri          ? msg->uri : "no uri"
                , msg->replyto//      ? msg->replyto : "no replyto"
                , msg->body         ? msg->body : "no body"
                , msg->process_name ? msg->process_name : "no pname"
                , msg->username     ? msg->username : "no username"
                , msg->nfrags>0     ? msg->pfrags[1] : "no pfrags"
                );
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
        std::cout << std::endl;
        // FPS_PRINT_INFO(" heres the vmap:\n{}\n", vmap);
        vm.processFims(vmap, msg, &cj);         // this is where we are seg faulting!!!
        char *res=cJSON_Print(cj);
        {
            if(res)
            {
                printf(" %s >> got res [\n%s\n]\n" , __func__ , res);
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
    schedItem* as;
    assetVar* av = nullptr;
    int delay = 2000;
    if (schreqs.size() > 0)
    {
        int oneran = 0;
    
        while(runit)
        {
            as = nullptr;
            if (schreqs.size() > 0)
            {
                as = schreqs.front();
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
                printf("Running one at %2.3f name [%s:%s]  size %d \n", tNow, av->comp.c_str(), av->name.c_str(),(int)schreqs.size());
                vm->setVal(*vmap,av->comp.c_str(), av->name.c_str(),tNow);
                schreqs.erase(schreqs.begin());
                printf(" new size 1 %d\n", (int)schreqs.size());
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
                        // addAmSchedReq(as);// new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->fname,rn->args));
                        if (am->reqChan)
                        {
                            channel <schedItem*>* reqChan = (channel <schedItem*>*)am->reqChan;
                            reqChan->put(as);
                            if (am->wakeChan)
                            {
                                am->wakeChan->put(0);
                            }
                        }
                    }
                }
                printf(" new size 2 %d\n", (int)schreqs.size());
            }
        }
        if(oneran)printf("Done running at %2.3f   ran %d\n", tNow, oneran);

    }
    // now pick off the next delay
    if (schreqs.size() > 0)
    {
        tNow = vm->get_time_dbl();

        as = schreqs.front();
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

int runChans(asset_manager *am)     // TODO: check if this does all the same things as regular ess_c when putting things on reqChan
{
    running = 1;
    int delay = 1000; // ms
    int wakeup = 0;
    schedItem *as = nullptr;
    double tNow = 0.0;
    fims_message *msg = nullptr;
    // double tStart = vm.get_time_dbl();

    std::cout << "we're in the runChans now" << std::endl;
    int running_count = 0;
    while (running)
    {
        std::cout << running_count++ << std::endl;    // to only run while loop for a set period of time
        if (running_count >= 10) break;

        // this will wake up after delay
        am->wakeUpChan->timedGet(wakeup, delay);
        
        tNow = vm.get_time_dbl();
        printf("Tick   at %2.3f\n",tNow);
        show_AvReqs(schreqs);

        // handle an incoming avar run request .. avoids too many locks
        if(am->reqChan) {   // was originally if(am->reqChan->get(as, false))

            // printf("Adding  Sched Request [%s]  at %2.3f\n", as->id, tNow); //  as->id creates a segfault because as is a nullptr

            //delete as;
            // am->addAmSchedReq(as);       // <- original

            // new (copied from SchedFunctions.cpp)
            channel <schedItem*>* reqChan = (channel <schedItem*>*)am->reqChan;
            reqChan->put(as);
            if (am->wakeChan)
            {
                am->wakeChan->put(0);
            }
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (am->fimsChan->get(msg, false))
        {
            std::cout << "\n am->fimsChan->get(msg, false) | msg->" << msg << std::endl;
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

                vm.syscVec = &syscVec;
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
        delay = 200;    // how much time each while loop takes in miliseconds

        // int xdelay = am->getSchedDelay();
        // printf(" %s new delay = %d\n", __func__, xdelay);
        // if ((vm.get_time_dbl() - tStart)  > 15.0) running = 1;

    }
    return 0;

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
        FPS_PRINT_INFO("Created Fims var [{}:{}]"
            , av->comp
            , av->name
        );
        fimsav = av;
    }
    fimsname= (char*)"/control/ess:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!runav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_PRINT_INFO("Created Runvar var [{}:{}]"
            , av->comp
            , av->name
        );
        runav = av;
    }
    fimsname= (char*)"/control/ess:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    if(!stopav){
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_PRINT_INFO("Created Stop var [{}:{}]"
            , av->comp
            , av->name
        );
        stopav = av;
    }
    SetupGit(*vmap, sc->vm, GITBRANCH, GITCOMMIT, GITTAG, GITVERSION );

}

// thread functions
void fimsThread(scheduler *sc, fims* p_fims)
{
    double tNow =  sc->vm->get_time_dbl();
    int tick = 0;
    fims_message* msg = nullptr;

    while (*sc->run)
    {
        bool fimsok = p_fims->Connected();
        if(!fimsok)
        {
            FPS_ERROR_PRINT("%s >> FIMS DISCONNECTED fimsok [%s]\n"
                , __func__
                , fimsok?"true":"false"
                );
            *sc->run = 0;
            
             //exit(0);
        }
        //fims_message* 
        if(fimsok)
        {
            msg = p_fims->Receive_Timeout(1000000);
            // just for testing
            tNow =  sc->vm->get_time_dbl();
            if (0) FPS_PRINT_INFO("Fims Tick {} msg {} p_fims {}"
                    , tick
                    , fmt::ptr(msg)
                    , fmt::ptr(p_fims)
                    );
            tick++;
        }
        if (msg)
        {
            if (strcmp(msg->method, "get") == 0)
            {
                if (1) FPS_PRINT_INFO("GET uri [{}]",  msg->uri);
            }
            if(*sc->run) 
            {
                sc->fimsChan.put(msg);
                sc->wakeChan.put(0);   // but this does not get serviced immediatey -> is done in schedThread
                std::cout << " we just put something on the fimsChan!!" << std::endl;
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
    char* essName = sc->vm->getSysName(*sc->am->vmap); 
    aV.sendEvent(essName, p_fims,  Severity::Info, "%s shutting down at %2.3f", cstr{essName}, tNow);
    FPS_PRINT_INFO("fims shutting down");
    //if(p_fims)delete p_fims;
    sc->p_fims = nullptr;
}

void schedThread(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager* am, fims* p_fims)
{
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

    fimsname= (char*)"/control/ess:runTime";
    assetVar*runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    //double dval = 0.0;

    double runTime = runav->getdVal();
    fimsname= (char*)"/control/ess:stopTime";
    assetVar*stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    runTime = runav->getdVal();
    if(runTime < 15) {
        runTime = 15.0;
        runav->setVal(runTime);
    }
    double stopTime = stopav->getdVal();
    
    while (*sc->run)
    { 
        sc->wakeChan.timedGet(wakeup, delay);
        essPerf * essLog = new essPerf(am, (char*)am->name.c_str(), "SchedPerf", nullptr);
        tNow = sc->vm->get_time_dbl();
        if (0) FPS_PRINT_INFO("Sched Tick");
        if (0) sc->showReqs(*rreqs);
        stopTime = stopav->getdVal();
        runTime = runav->getdVal();
        if(stopTime>0 && ! stopSeen)
        {
            stopSeen = true;
            FPS_PRINT_INFO("Sched Tick stopTime set {:2.3f}", stopTime);
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
            FPS_PRINT_INFO("message -> data [{}]", cstr{cmsg});
            //use the message to set the next delay
            if(strcmp(cmsg,"quit")== 0)
            {
                FPS_PRINT_INFO("wakeup value to stop");
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
                if (0) FPS_PRINT_INFO("Servicing Sched Request {} id [{}] ({})  uri [{}] repTime {:2.3f}"
                        , fmt::ptr(si)
                        , fmt::ptr(si->id)
                        , fmt::ptr(si->id)
                        , si->uri
                        , si->repTime
                    );
                if (!si->id || (strcmp(si->id,"None")==0))
                {
                    FPS_PRINT_INFO("this is not a schedItem");
                    delete si;
                    si = nullptr;
                }
                
                if(si)
                {
                    schedItem* oldsi = sc->find(*rreqs, si);
                    if(oldsi)
                    {
                        FPS_PRINT_INFO("this is a replacement schedItem {} id {} uri {}", fmt::ptr(oldsi), cstr{oldsi->id}, cstr{oldsi->uri});
                        oldsi->repTime=0;
                        oldsi->endTime=0.1;
                         
                        FPS_PRINT_INFO("schedItem deleted, seting repTime to 0.0");
                    }
                    //else
                    {
                        if (0) FPS_PRINT_INFO("schedItem added {} id {} uri {}", fmt::ptr(si), cstr{si->id}, cstr{si->uri});
                        sc->addSchedReq(*rreqs, si);
                    }
                }
            }
            else
            {
                FPS_PRINT_INFO("got nullptr si request !!");
            }
        }

        // this gets incoming  fims messages
        if (sc->fimsChan.get(msg, false))
        {
            if(msg)
            {
                essPerf ePerf(am, (char*)am->name.c_str(), msg->uri, nullptr);
                sc->vm->schedaV = fimsav;
                if(0)sc->fimsDebug1(*vmap, msg, am);

                sc->vm->runFimsMsgAm(*vmap, msg, am, p_fims);
                if(0)sc->fimsDebug2(*vmap, msg, am);
            }
        }

        double ddelay = sc->getSchedDelay(*vmap, *rreqs);   // use this line when not testing
        // double ddelay = 200;    // delete this line after testing
        delete essLog;

        if(0)FPS_PRINT_INFO("new delay = {:2.6f}", ddelay);
        delay = ddelay;
        //if ((sc->vm->get_time_dbl() - tStart)  > 15.0) running = 0;
    }

    tNow = sc->vm->get_time_dbl();
    FPS_PRINT_INFO("shutting down");
    return;
}


void print_vmap(){
    cJSON* cj0 = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj0) 
    {
        char* res = cJSON_Print(cj0);
        if(res)
        {
            printf("\n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj0);
    }
}

int main_test_new_sched(int argc, char *argv[])         // most of the code here is copied directly from ess_controller
{
    std::cout << "\n STARTING MAIN_TEST_NEW_SCHED\n" << std::endl;

    syscpVec = new std::vector<char*>;
    vm.argc = argc;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    scheduler sched("essSched", &running);
    sched.vm = &vm;
    sched.vm->syscVec = &syscVec;

    vm.vmapp   = &vmap;
    vm.funMapp = &funMap;
    fims* p_fims = nullptr;

    vm.setFname(FlexDir);

    vm.setRunCfg(LOGDIR "/run_configs");

    asset_manager* ess_man = setupEssManager(&vm, vmap, vecs, syscpVec, FlexName, &sched);

    ess_man->wakeChan = &sched.wakeChan;
    ess_man->reqChan = (void*)&sched.reqChan;
    sched.am = ess_man;

    vm.setaM(vmap, FlexName, ess_man);
    vm.setSysName(vmap, FlexName);
    essName = vm.getSysName(vmap);

    double tNow = vm.get_time_dbl();

    asset_manager *am = new asset_manager("ess");
    am->vm = &vm;
    am->vmap = &vmap;

    // these 3 functions get set as /system/functions in the vmap
    vm.setFunc(vmap, "ess", "run_some_func", (void*)&run_some_func);
    vm.setFunc(vmap, "ess", "add_sched_item", (void*)&add_sched_item);
    vm.setFunc(vmap, "ess", "CheckMonitorVar", (void*)&CheckMonitorVar);

    SetupEssSched(&sched, ess_man);     // sets up all func from og ess_controller


    vm.setActions(vmap, FlexName);

    std::vector<std::string>subVec;
    int nsubkeys = 0;
    auto nSubs = fmt::format(":/{}{}", FlexName, FlexSubs);
    nsubkeys = vm.uriSplit(subVec, nSubs.c_str() , ":");
    const char **fsubs = vecToList(subVec, nsubkeys);
    showVecList(fsubs);

    int mysublen;
    char** mysubs = vm.getVecListbyName(*ess_man->vecs, "Subs", mysublen);
    FPS_PRINT_INFO("recovered  [{}]  Subs [{}] len  [{}]", FlexName, fmt::ptr(mysubs), mysublen);
    //fims* p_fims = nullptr;

    if((mysublen> 0) || (nsubkeys > 0))
    {
        if (1)FPS_PRINT_INFO(" flex_man >> No Subs found in flex config");
        if(nsubkeys == 0)
        {
            p_fims = sched.fimsSetup ((const char**)mysubs, mysublen, "FlexSched", vm.argc);
        }
        else
        {
            p_fims = sched.fimsSetup ((const char**)fsubs, nsubkeys, "FlexSched", vm.argc);
        }

        vm.clearList(mysubs, FlexName, mysublen);
        if (1)FPS_PRINT_INFO(" flex_man >> p_fims [{}]  sched [{}]"
            , fmt::ptr(p_fims)
            , fmt::ptr(sched.p_fims)
            );
    }
    else
    {
        if (1)FPS_PRINT_INFO(" flex_man >> No Subs found in flex config");
        return 0;
    }
    if(fsubs) free(fsubs);

    ess_man->p_fims = p_fims;
    vm.p_fims = p_fims;

    // idk what these do or if theyre needed    
    //chan2_data r_data;  // for new run reqs
    // these are the ain system level chanels
    channel <int> wakechan;
    channel <char*>msgchan;   // anyone can post messages 
    channel <schedItem*>reqChan; // anyone can post run_reqs
    channel <fims_message*>fimsChan; // anyone can post run_reqs
    
    schreqs.reserve(64);

    assetVar *av = vm.setVal(vmap, "/sched/ess", "single01", tNow);
    
    //av->addSchedReq(avreqs, tNow+0.1, 0.0);
    av->am = am;
    am->wakeUpChan = &wakechan;
    am->reqChan = &reqChan;
    am->fimsChan = &fimsChan;
    // am->schreqs  = &avreqs;

    schedItem *as = new schedItem();
    as->setUp("item_id", "item name", "/sched/ess:single01", "run_some_func", 0, 0.25, 0.1, 0.2, av->getcParam("targ") );
    as->func = (char*)vm.getFunc(*am->vmap, "ess", "run_some_func");
    as->av = vm.getVar(*am->vmap, "/sched/ess:single01", nullptr);

    // std::cout << "checking new schedItem that we just set up:" << std::endl;
    // as->show();

    // printf("\n\n running set up sched function ...\n");
    setup_sched_function(vmap, "/schedule/ess", "add_item", "add_sched_item", am);

    // std::cout << "we set up the function add_item! yay" << std::endl;

    //int setup_sched_item(varsmap& vmap, const char*name , const char* id, const char*aname, double runTime , double reptime)
    printf("\n\n running set up sched itemses ...\n");
    // setup_sched_item(vmap,"/schedule/ess","every100mS","ID100mS","ess", 0.5, 0.1);
    // setup_sched_item(vmap,"/schedule/ess","every50mS","ID50mS","ess", 0.5, 0.05);
    setup_sched_item(vmap,"/schedule/ess","every1S","ID1S","ess", 0.5, 1.0);

    std::cout << "every100ms, 50ms, and 1s items have been set up" << std::endl;

    // set actions for every 100mS item
    // cJSON * cja = cJSON_CreateArray();
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxTemp", am);
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxVolts", am);
    // add_action(cja,"ess","CheckMonitorVar","/status/ess:maxCurrent",am);
    // setup_sched_item_actions(vmap,"/schedule/ess","every100mS",cja);
    // cJSON_Delete(cja);

    //set actions for every 1S item
    cJSON * cja1 = cJSON_CreateArray();
    add_action(cja1,"ess","run_some_func","/status/ess:wiggle", am);
    setup_sched_item_actions(vmap,"/schedule/ess","every1S",cja1);
    cJSON_Delete(cja1);

    // now setting a value to "/schedule/ess:every100mS" shouldtrigger all the checkMonitorVar actions
    tNow = vm.get_time_dbl();
    // vm.setVal(vmap,"/schedule/ess:every100mS", nullptr, tNow);
    vm.setVal(vmap,"/schedule/ess:every1S", nullptr, tNow);

    printf("\n\n running activate sched item  ...\n");
    // activate_sched_item(vmap, "/schedule/ess", "/schedule/ess", "every100mS");
    activate_sched_item(vmap, "/schedule/ess", "/schedule/ess", "every1S");

    printf("\n\n vmap before we runChans ...\n");
    print_vmap();


    char* specific_essName = vm.getSysName(vmap);
    const auto config_name = fmt::format("/config/{}", specific_essName);

    // This setups the default logging dir,enable, timestamp, and amount

    assetVar* logging_dir_av  = vm.getVar(vmap, config_name.c_str(), "LogDir");
    if (!logging_dir_av) // set "default" to be "/var/log/ess_controller":
    {
        char* log_dir = (char*)"var/log/ess_controller";
        logging_dir_av = vm.makeVar(vmap, config_name.c_str(), "LogDir", log_dir);
    }

    assetVar* logging_enabled_av  = vm.getVar(vmap, config_name.c_str(), "logging_enabled");
    if (!logging_enabled_av) // set "default" to be false:
    {
        bool log_enabled = false;
        logging_enabled_av = vm.makeVar(vmap, config_name.c_str(), "logging_enabled", log_enabled);
    }

    assetVar* logging_timestamp_av  = vm.getVar(vmap, config_name.c_str(), "logging_timestamp");
    if (!logging_timestamp_av) // set "default" to be false:
    {
        bool log_timestamp = false;
        logging_timestamp_av = vm.makeVar(vmap, config_name.c_str(), "logging_timestamp", log_timestamp);
    }

    assetVar* logging_size_av = vm.getVar(vmap, config_name.c_str(), "logging_size");
    if (logging_size_av) // they have it in the config file:
    {
        setLoggingSize(vmap, vm);
    }
    else // setup default value and "actions":
    {
        int log_size = 64;
        logging_size_av = vm.makeVar(vmap, config_name.c_str(), "logging_size", log_size);
    }

    // !!!!!!!!!!!!!! TODO
    // what does runChans do?
    // runChans(am);   // is this the same as starting fimsThread and schedThread? -> this exists in scheduler.h (diff params)
    // printf("\n\n runchans done ...\n");


    std::cout << "\n starting threads" << std::endl;

    std::thread fThread(fimsThread, &sched, p_fims);    // thread works
    std::thread sThread(schedThread, &sched, &vmap, &schreqs, ess_man, p_fims);

    if(fThread.joinable()) fThread.join();
    if(sThread.joinable()) sThread.join();
    FPS_PRINT_INFO("threads done cleaning up ...");


    FPS_PRINT_INFO(" Printing final vmap:");
    cJSON* cj_2 = vm.getMapsCj(vmap, nullptr, nullptr, 0x00);
    if(cj_2) 
    {
        char* res = cJSON_Print(cj_2);
        if(res)
        {
            printf("\n%s\n", res);
            free(res);
        }
        cJSON_Delete(cj_2);
    }

    // deleting and clearing everything -> copied from og ess_controller    
    vm.clearVmap(vmap);
    vmap.clear();
    am->p_fims = nullptr;
    ess_man->p_fims = nullptr;
    for ( auto xx:vecs)
    {
        xx.second->clear();
        delete xx.second;
    }
    vecs.clear();
    delete ess_man;
    schreqs.clear();
    vm.amMap.clear();
    vm.aiMap.clear();
    if(p_fims)delete p_fims;   

    // uncommenting the lines below causes the program to abort 
    // for ( auto sr : schreqs)
    // {
    //     // sr->show();
    //     delete sr;        
    // }
    // delete am;
    // delete as;

    return 0;
}

// #define _MAIN
#if defined _MAIN
int main(int argc, char *argv[])
{
    // setting up stdout console sink:
    auto std_out = spdlog::stdout_color_st("stdout");
    std_out->set_level(spdlog::level::debug);

    // setting up stderr console sink:
    auto std_err = spdlog::stderr_color_st("stderr");
    std_err->set_level(spdlog::level::err);

    // setting the default logger to log to both:
    spdlog::set_default_logger(std::move(std_out));

    // setting up the elapsed time formatter for the global logger (similar to the way we have it for ESS Controller):
    // NOTE: please refer to this for help -> https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<elapsed_time_formatter>('*').set_pattern("[%-10*] [%^%-8l%$] [%-15!!] %v");
    spdlog::set_formatter(std::move(formatter));
   
    return  main_test_new_sched(argc, argv);
}
#endif

/*
to run this file

./ess_controller/build/release/test_new_sched
add >> output.txt to send output to text file

run pkill -f ess_controller to stop running

debugger
gdb ./ess_controller/build/release/test_new_sched

*/ 
