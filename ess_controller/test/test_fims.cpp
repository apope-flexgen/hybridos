/*
 * channel test with fims
 * sets up a fims listner and streams in the data
 * also used for testing process fims.
 */

// #include <pthread.h>
// #include <iostream>
// #include <poll.h>
// #include <cstring>
// #include <queue>
// #include <vector>
// #include <fims/libfims.h>
#include "asset.h"

#include "channel.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

//#include "assetVar.h"

// typedef struct chan_data_t {
//     channel<int> *wake_up_chan;
//     channel<char*> *message_chan;
//     channel<fims_message*> *fims_chan;
//     int count;
//     int delay;
//     int *run;
//     pthread_t thread;

// } chan_data;

// // template ??
// template<class T>
// class  tchan_data {
//     channel<int> *wake_up_chan;
//     channel<T> *message_chan;
//     int count;
//     int delay;
//     int *run;
//     pthread_t thread;

// };


void *fims_loop(void *data)
{
    const char* subs[] = {
        "/components", 
        "/assets/bms_1"
        };
    int sublen = sizeof subs / sizeof subs[0];

    fims* p_fims = new fims();
    p_fims->Connect((char *)"fimsTest");
    p_fims->Subscribe(subs, sublen);

    chan_data* td = (chan_data*) data;
    while (*td->run)
    {
        fims_message* msg = p_fims->Receive_Timeout(100000);
        if(msg)
        {
            td->fims_chan->put(msg);
            td->wake_up_chan->put(0);
        }
    }
    delete p_fims;
    pthread_exit(nullptr);
}

void *timer_loop(void *data)
{
    chan_data* td = (chan_data*) data;
    while (*td->run)
    {
        poll(nullptr,0,td->delay);
        td->wake_up_chan->put(td->count++);
    }
    pthread_exit(nullptr);
}

void *message_loop(void *data)
{
    char *mdata = nullptr;
    chan_data* td = (chan_data*) data;
    while (*td->run)
    {
        //if(mdata)free((void *)mdata);
        asprintf(&mdata, " this is message number %d", td->count++);
        poll(nullptr, 0, td->delay);
        td->message_chan->put(strdup(mdata));
        td->wake_up_chan->put(0);
        if(mdata)free((void *)mdata);
        mdata = nullptr;
    }
    //if(mdata)free((void *)mdata);
    pthread_exit(nullptr);
}

typedef void*(* vLoop)(void *args);

int run(chan_data* c_data, vLoop c_loop, int *rflag, int delay, channel<int> *wake)
{
    c_data->run = rflag;
    c_data->count = 0;
    c_data->delay = delay;
    c_data->wake_up_chan = wake;
    return pthread_create(&c_data->thread, nullptr, c_loop, c_data);
}

int sendTestMessage(fims *p_fims, int tnum)
{
    const char* method;
    const char* replyto=nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;

    switch (tnum) {
        case 1:
            {
                method = "set";
                replyto = "/test/foo";
                uri="/components/test_1";
                body="{\"var_set_one\":21}";
            }
            break;
        case 2:
            {
                method = "set";
                //replyto = "/test/foo";
                uri="/components/test_2";
                body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
            }
            break;
        case 3:
            {
                method = "set";
                replyto = "/test/foo_2";
                uri="/components/test_2";
                body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
            }
            break;
        case 4:
            {
                method = "set";
                replyto = "/test/foo_4";
                uri="/components/test_3";
                body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
            }
            break;
        case 5:
            {
                method = "get";
                replyto = "/test/foo_5";
                uri="/components/test_3";
                //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
            }
            break;
        case 6:
            {
                method = "get";
                replyto = "/test/foo_6";
                uri="/components/test_3/var_set_two";
                //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
            }
            break;
 
        default:
            break;
        }
    if(1)
        if(uri)
            p_fims->Send(method, uri, replyto, body);
    return 0;
}

int running = 1;
int main(int argc, char *argv[])
{
    chan_data t_data;
    chan_data m_data;
    chan_data f_data;

    channel <int> wakechan;
    //channel <std::string>channel2;
    channel <char *>msgchan;
    channel <fims_message *>fimschan;
    varsmap vmap;

    fims* p_fims = new fims;
    p_fims->Connect((char *)"fimsMain");

    int wakeup;
    //std::string item2;
    char* item3;
    fims_message* msg;

    t_data.message_chan = &msgchan;
    m_data.message_chan = &msgchan;
    f_data.fims_chan = &fimschan;
    class timeVar {
    public:
        timeVar( double t, int i) {
            tt = t;
            somestuff = i;
        }
        ~timeVar() {};
        bool greater(timeVar *tv)
        {
            return (tv->tt > tt );
        }
        double tt;
        int somestuff;
    };
    class Compare
    {
    public:
        bool operator()(timeVar* t1, timeVar * t2)
        {
            return (t1->tt > t2->tt);
        }
    };

    std::priority_queue<double, std::vector<double>, std::greater<double> > pqueue;
    pqueue.push(100.1);
    pqueue.push(102);
    pqueue.push(50.3);
    pqueue.push(25.4);
    while (!pqueue.empty())
    {
        printf("x %f\n",pqueue.top());
        pqueue.pop();

    }

    std::priority_queue<timeVar*, std::vector<timeVar*>, Compare > tqueue;
    tqueue.push(new timeVar(100.1,1100));
    tqueue.push(new timeVar(102,1102));
    tqueue.push(new timeVar(50.3,1103));
    tqueue.push(new timeVar(25.4,1102));
    while (!tqueue.empty())
    {
        printf("xtt %f\n",tqueue.top()->tt);
        delete tqueue.top();
        tqueue.pop();
    }

// timer loop need more expansion
// its job is to run a timer call back on timer requests
//  a timer request will have a delay ( from time now) and a call back function
// the timeer loop sleeps until the next callback is due.
// call backs are insterted in order.

    run(&t_data, timer_loop,   &running, 1000,  &wakechan);
    run(&m_data, message_loop, &running, 1500,  &wakechan);
    // the fims system will get pubs and create a varsMap from the items.

    run(&f_data, fims_loop,    &running, 1500,  &wakechan);

    VarMapUtils vm;
    int tnum = 0;
    // any wake up comes here
    while(wakechan.get(wakeup,true)) {
        // then service the components
        //
        std::cout << " wakeup ival "<< wakeup << "\n";
        if(wakeup >= 10)
        {
            running = 0;
            break;
        }
        if(msgchan.get(item3,false)) {
            sendTestMessage(p_fims, tnum++);

            std::cout << " item3-> data  "<< item3 << "\n";
            free((void *) item3);
        }

        if(fimschan.get(msg, false)) {
            std::cout << " fims_msg uri "<< msg->uri  << "\n";
            // we need to collect responses
            cJSON *cj = nullptr;
            vm.processFims(vmap, msg, &cj);
            if (cj)
            {
                char* tmp = cJSON_PrintUnformatted(cj);
                if(tmp)
                {
                    p_fims->Send("set",msg->replyto, nullptr, tmp);
                    free((void*)tmp);
                }
                cJSON_Delete(cj);
            }
            p_fims->free_message(msg);

            //free((void *) item3);
        }

    }
    // pull out last messages
    while (msgchan.get(item3,false)) 
    {
        //sendTestMessage(p_fims, tnum++);

        std::cout << " last item item3-> data  "<< item3 << "\n";
        free((void *) item3);
    }
    pthread_join(t_data.thread,nullptr);
    std::cout << " t_data all done " << "\n";
    pthread_join(m_data.thread,nullptr);
    std::cout << "m_data all done " << "\n";
    pthread_join(f_data.thread,nullptr);
    std::cout << "f_data all done " << "\n";
    cJSON *cj = vm.getMapsCj(vmap);
    char* res = cJSON_Print(cj);
    printf("Maps at end \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);
    delete p_fims;
    //VarMapUtils vm;
    vm.clearVmap(vmap);

    return 0;

}
