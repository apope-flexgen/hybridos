/*
 * channel timer test
 * basic code to set up a thread as a timer server.
 * the main thread will issue timer requests. The timer thread will call the main thread back at the requested time.
 * we can even find out how long it took to process the round trip.
 * The timer server will have 
 * A request channel 
 * A priority queue of timer requests with the closest one at the top
 * 
 */

// #include <pthread.h> 
#include <iostream>
#include <queue>
#include <vector>
// #include <poll.h>
//#include <asset.h>
#include <poll.h>
#include <time.h>

#include <fims/libfims.h>
#include "channel.h"
#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

// this is our map utils factory
VarMapUtils vm;

const char* sHeartBeat        = "/controls/bms_1:HeartBeat";

assetVar* HeartBeat;

int init(varsmap &vmap)
{
    int ival = 0;
    HeartBeat             = vm.linkVal(vmap, sHeartBeat,        nullptr, ival);
    return 0;
}

// this is the makings of a reverse prioity queue the smallest value ot tt will be the top of the queue
// normal priority queues do it the other way round.
// The compare function included in the queue definition reverses this.
// The timeVar will contain the details of the thing we want to do at the time out time.
// for now it just contains somestuff
// 
// this is what it may could look like in Go   
//  timerThing <- "send <somedata> in 5 seconds to <somedest>"

// notes on fims_send
//      fims* p_fims = new fims;
//      p_fims->Connect((char *)"somename");  
//      method = "set";
//      replyto = nullptr;//"/test/foo_6";
//      uri="/components/test_3/var_set_two";
//      body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
//      p_fims->Send(method, uri, replyto, body);


// notes on testing 
// test 1
// set up the tqueue buffer with requests at different times
// then run the system and collect the output  ?? how do we do that something like std::string result=""; result.append(last_test);

// test the output string against an expected output string
// note this may be tricky because the numbers will not be the same due to times .. have to think about that
// test 2 send multiple  requests to the message channel again inspect the output  

// this is all the get_time_dbl stuff .. there is better std::chron stuff now in c++ but we'll use the double for now.

double g_base_time = 0.0;

long int get_time_us()
{
    long int ltime_us;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
    return ltime_us;
}

double get_time_dbl()
{
    return  (double) get_time_us()/1000000.0 - g_base_time;
}

void set_base_time(void)
{
    g_base_time = get_time_dbl();
}

int sendAsset(assetVar*av,fims* p_fims)
{
    cJSON *cj = nullptr;
    HeartBeat->getCjVal(&cj);
    char* res = cJSON_Print(cj);
    //printf("Publish \n%s\n", res);
    cJSON_Delete(cj);
    p_fims->Send("pub", HeartBeat->comp.c_str(), nullptr, res);
    free((void *)res) ;
    return 0;
}

int HandleHeartBeat(fims* p_fims)
{
    int ival = HeartBeat->getiVal();
    ival++;
    if(ival > 255) ival = 0;

    HeartBeat->setVal(ival);
    sendAsset(HeartBeat, p_fims);
    return ival;
}

int HandleDateTime(fims *p_fims)
{
    // Jimmy - may need to look into assetVar.h in more detail and see how vars are stored/utilized

    time_t tnow = time(0);

    tm *ltm = localtime(&tnow);

    const char* method = "set";
    const char* replyto=nullptr;
    const char* uri = "/asset/bms_1/date";

    // Use asprintf, then free
    char* body;
    asprintf(&body, "{\"date_yr\":%d,\"date_mon\":%d,\"date_day\":%d,\"date_hr\":%d,\"date_min\":%d,\"date_sec\":%d}", ltm->tm_year + 1900, ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    
    p_fims->Send(method, uri, replyto, body);
    return 0;
}

class timeVar {
public:
    timeVar( double t, int i) {
        tt = t;
        somestuff = i;
        actions = nullptr;
    }
    
    // Added this constructor to allow timeVar to store and run a function (ex.: HandleHeartBeat)
    timeVar( double t, int i, int (*f)(fims *) ) {
        tt = t;
        somestuff = i;
        actions = nullptr;
        func = f;
    }
    ~timeVar() {};

    // Start running the task
    void run(fims* p_fims)
    {
        printf(" timer task running at %f somestuff %d actions >>%s<<\n"
        , tt ,somestuff
        , actions?actions:"noaction");
        
        // Run a function - results should be sent via fims
        func(p_fims);
    };

    // this is not used
    bool greater(timeVar *tv)
    {
        return (tv->tt > tt );
    }

    double tt;   //wake up time in seconds base on get_time_dbl()
    int somestuff;
    char* actions;
    int (*func)(fims *);  // Pointer to a function - executes whenever the timeVal run() is called
};

class Compare
{
public:
    bool operator()(timeVar* t1, timeVar * t2)
    {
        return (t1->tt > t2->tt);
    };
};

    
std::priority_queue<timeVar*, std::vector<timeVar*>, Compare > tqueue;

// here is a simple test 
void test_tqueue ()
{
    double c_time = get_time_dbl();
    tqueue.push(new timeVar(c_time+ 0.5,5));
    tqueue.push(new timeVar(c_time+ 0.1,1));
    tqueue.push(new timeVar(c_time+ 0.8,8));
    tqueue.push(new timeVar(c_time+ 1.5,15));

    while (!tqueue.empty())
    {
        printf("xtt %f somestuff %d\n",tqueue.top()->tt,tqueue.top()->somestuff);
        delete tqueue.top();
        tqueue.pop();
    }
}

// the  tqueue provides a queue of objects with the smallest value of tt at the top.
// tt is the time we want this thing to wake up and to the actions   



// this is a structure used to pass channel data to the running thread.  We will use this to send instructions ti the timer object
typedef struct mchan_data_t {
    channel<timeVar*> *wake_up_chan;
    channel<char*> *message_chan; // to be remoed
    int count;
    double dcount;
    int delay;
    int *run;
    int (*func)(fims *); //Pointer to a function - each time request will perform specific action
    pthread_t thread;

} mchan_data;

//  timerThing <- "send <somedata> in 5 seconds to <somedest>"

// this simply sends timer requests, TODO we need to run a action
void *timer_loop(void *data)
{
    mchan_data* td = (mchan_data*) data;
    while (*td->run)
    {
        poll(nullptr,0,td->delay);
        // give me a wake up time in the future new timeVar(get_time_dbl() +td->dcount, td_count));
        //td->wake_up_chan->put(new timeVar(get_time_dbl() +td->dcount, td->count));
        td->wake_up_chan->put(new timeVar(get_time_dbl() + td->delay * 1.0 / 1000, td->count, td->func));

        // also add to tqueue 
        // ** Jimmy - Not sure what dcount is used for here **
        td->dcount += 10.0/1000.0;
        td->dcount += 25;

    }
    pthread_exit(nullptr);
}

// we'll use this to send timer requests to the system
// maybe create a timeVat here as send that to yje system
void *message_loop(void *data)
{
    char *mdata1;
    char *mdata2;
    mchan_data* td = (mchan_data*) data;
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);
        asprintf(&mdata1, "\"timer\":{ \"action\":\"oneshot\",\"delay\":500,\"send\":\"/system/bss_1:start_off\",\"to\":\"/component/bss_1:system_start\"}");
        td->message_chan->put(mdata1); // HMM what happens if mdata is changed 
        asprintf(&mdata2, "\"timer\":{ \"action\":\"oneshot\",\"delay\":10,\"send\":\"/system/bss_1:start_on\",\"to\":\"/component/bss_1:system_start\"}");
        td->message_chan->put(mdata2); // HMM what happens if mdata is changed 
        //td->wake_up_chan->put(0);
    }
    pthread_exit(nullptr);
}

typedef void*(* vLoop)(void *args);

// create a thread and run with it
int run(mchan_data*c_data, vLoop c_loop, int *rflag, int delay, channel<timeVar*> *wake, int (*func)(fims *))
{
    c_data->run = rflag;
    c_data->count = 0;
    c_data->dcount = 0.1;
    c_data-> delay = delay;
    c_data->wake_up_chan = wake;
    c_data->func = func; // Added this to allow thread to run an arbitrary task
    return pthread_create(&c_data->thread,nullptr, c_loop, c_data);
}

int running = 1;

void* timer_main(void *data)
{
    // Two different channel data structures for timer requests
    // Not sure if this is a good approach though - Jimmy
    mchan_data t_data_1;
    mchan_data t_data_2;
   // mchan_data m_data;  // not going to use this one

    // Timer request threads will share the same wakeup channel
    channel <timeVar *> wakechan;
   // channel <char *>msgchan;  // not going to use this one

    //double wakeup;
    //std::string item2;
    //char* item3;


    //  slight rework needed
    // t_data.message_chan = &msgchan;
    // m_data.message_chan = &msgchan;

    set_base_time();

    // Start timer request threads, each with different time delay and task function
    run(&t_data_1, timer_loop, &running, 100, &wakechan, &HandleHeartBeat);
    run(&t_data_2, timer_loop, &running, 1000, &wakechan, &HandleDateTime);
    //run(&m_data, message_loop, &running, 1500, &wakechan);

    // this lot needs to be rewitten
    double tnow;
    //double tdel;
    timeVar* tv;
    int tdelMs;

    // this is our main data map
    varsmap vmap;

    init(vmap);

    // Set up a fims connection 
    fims* p_fims = new fims;
    p_fims->Connect((char *)"fimsTimerChanTest");

    // this will run forever , we'll use a signal to turn it off... later
    while(running) 
    {
        double tdel;
        
        tnow = get_time_dbl();
        tdel = tnow+(10.0/1000.0);

        // do we have any expired timer requests
        if(!tqueue.empty())
        {
            tv = tqueue.top();
            //is the timeout past due 
            if(tv->tt < tnow )
            {
                // yes its time to run this task
                tv->run(p_fims);
                tqueue.pop();
                //std::cout<< "sending tdel " << tv->somestuff << "\n";
                if(!tqueue.empty())
                {
                    tv = tqueue.top();
                    // reset tdel remember the NEXT time to wake up is at the top of the queue
                    tdel = tv->tt;
                }
            }
            
        }
        tdelMs = int((tdel-tnow)*1000.0); 

        // this will return after the tdel timeout or when we get another timer request.
        // the tdel timeout is the time perion between now and the lowest value request at the top of the tqueue Ie the next time to run. 
        // it will block until the timeout or the message comes. 
        // if it returned true a new timer request was sent to the timer server.
        // push that request and the tqueue will bubble the next request to the top.

        if (wakechan.timedGet(tv, tdelMs)) {
            tqueue.push(tv);// ?? do we have to do this...it may have been done already
        }
        // we may not be able to se this...
        // this is the timer system input
        // if(msgchan.get(item3,false)) {
        //     // TODO this will create a timVar structure with a delay and action and add it ti tqueue
        //     //tqueue.push(new tv)
        //     //std::cout << " item3-> data  "<< item3 << "\n";
        //     free((void *) item3);
        // }
    }

    pthread_join(t_data_1.thread,nullptr);
    pthread_join(t_data_2.thread,nullptr);
    std::cout << " t_data all done " << "\n";
    // pthread_join(m_data.thread,nullptr);
    // std::cout << "m_data all done " << "\n";
    return 0;

}


int main(int argc, char *argv[])
{

    timer_main(nullptr);
    return 0;
    
}
