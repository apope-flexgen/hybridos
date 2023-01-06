/*
* channel event test code
* the timer thread can be woken up with a message to add a new event to the timer list
* the new event may change the timeout 
* and events 
*/

#include <pthread.h>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#include <asset.h>
#include <fims/libfims.h>
#include "channel.h"
#include "varMapUtils.h"
#include "chrono_utils.hpp"



class run_req {
    public:
    run_req(double _t_shot, double _t_rep, const char* _fname, const char* _args)
    {
        t_rep = _t_rep;
        t_shot = _t_shot;
        fname=_fname?strdup(_fname):nullptr;
        args=_args?strdup(_args):nullptr;    
    }
    ~run_req()
    {
        if (fname) {
            printf("deleting [%s]\n", fname);
            free(fname);
        }
        if (args) free(args);
    }

    void show(int idx)
    {
        printf(" [%02d] t_shot [%2.3f] t_rep [%2.3f] func[%s] args[%s]\n"
        , idx
        , t_shot
        , t_rep
        , fname
        , args
        );
    }
    double t_shot;  // set if a single shot
    double t_rep;   // set if repeating
    char* fname;
    char* args;
};


typedef struct chan2_data_t {
    channel<int> *wake_up_chan;
    channel<char*> *message_chan;
    channel<run_req*>*run_chan;
    int count;
    int delay;
    int *run;
    run_req* rr;
    //int fd[2];
    pthread_t thread;

} chan2_data;

// int add_rreq(std::vector<run_req*>&rreq, run_req*rn)
// {

//     std::vector<run_req*>::iterator it;
//     it = rreq.begin();
//     while( it != rreq.end())
//     {
//         if ((*it)->t_shot>rn->t_shot)
//         {
//             rreq.insert(it, rn);
//             return rreq.size();
//         }
//         it++;

//     }
//     rreq.push_back(rn);
//     return rreq.size();
// }

int add_rreq(std::vector<run_req*>&rreq, run_req* rr)
{
    std::vector<run_req*>::iterator it;
    it = rreq.begin();
    while( it != rreq.end())
    {
        if ((*it)->t_shot>rr->t_shot)
        {
            rreq.insert(it, rr);
            return rreq.size();
        }
        it++;

    }
    rreq.push_back(rr);
    return rreq.size();

}

void show_reqs(std::vector<run_req*>&rreq)
{
    int idx = 0;
    for ( auto rr : rreq)
    {
        rr->show(idx++);
    }
    return;
}

void clear_reqs(std::vector<run_req*>&rreq)
{
    int idx = rreq.size();
    while (idx--)
    {
        delete(rreq.front());
        rreq.erase(rreq.begin());

    }
    return;
}

run_req* pull_req(std::vector<run_req*>&rreq, double t_now)
{
    run_req* rn = nullptr;
    if (rreq.size() == 0)
        return rn;

    rn = rreq.front();
    
    if(rn->t_shot<=t_now)
    {
        //rreq.erase(it);

        if (rn->t_rep > 0.0)
        {
            //rn->t_shot+=rn->t_rep;
            add_rreq(rreq, new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->fname,rn->args));
        }
    }
    return rn;
}

int xxmain(int argc, char*argv[])
{
    //run_req rr(25,      2,"rr one","rr 2");
    //rr.show(1);

    std::vector<run_req *> rreqs;

    rreqs.reserve(64);
//              t_shot t_rep   func        args
    add_rreq(rreqs, new run_req(40,    0,      "single40", "arg1 arg2 arg3"));
    add_rreq(rreqs, new run_req(20,    60,     "rep20at60","arg1 arg2 arg3"));
    add_rreq(rreqs, new run_req(30,    70,     "rep30at70","arg1 arg2 arg3"));

    printf("first 3\n");
    show_reqs(rreqs);
    
    add_rreq(rreqs, new run_req(25,    0,     "single25","arg1 arg2 arg3"));
    printf("add one\n");

    show_reqs(rreqs);
    // so this becomes the timer loop picking off the next task
    // function + assetvat with a vmap
    printf("running list\n");

    run_req* rr;
    double t_now = 0.0;
    while (t_now < 200)
    {
        //rr = pull_req(rreqs, t_now);
        if (rreqs.size() > 0)
        {
            rr = rreqs.front();
    
            if(rr->t_shot<=t_now)
            {
                printf("got one at %2.3f\n", t_now);
                rr->show(((int)t_now));
                rreqs.erase(rreqs.begin());
                if (rr->t_rep > 0.0)
                {
                    rr->t_shot+=rr->t_rep;
                    add_rreq(rreqs, rr);// new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->fname,rn->args));
                }
                else
                {
                    delete rr;
                }
            }
        }
        if (rreqs.size() > 0)
        {
            rr = rreqs.front();
            printf("  >>>new delay = %f \n", rr->t_shot - t_now);
        }

        t_now += 5;
    }

    printf("after 200\n");

    show_reqs(rreqs);

    clear_reqs(rreqs);

    printf(" Git branch = [%s]\n",GITBRANCH);
    
    return 0;
}

// #include <sys/socket.h>
// #define DATA “Data to write to parent\n”
// ⋮
// int rc;
// int sv[2];
// pid_t pid;
// char buf[256];
// ⋮
// rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
// if(rc < 0){
// 	printf(“SOCKETPAIR ERROR = %d\n”, sock_errno());
// }
// ⋮
// pid = fork();
// if(pid == 0){ /* in the child */
// 	close(sv[0]);
// 	rc = write(sv[1], DATA, sizeof(DATA);
// 	⋮
// }
// else {
// 	close (sv[1]);
// 	rc = read(sv[0], buf, sizeof(buf));
// 	⋮
// }Copy code
//timer 2 runs faster and wakes up timer1
// nope noth this one 
// void *timer2_loop(void *data)
// {
//     int tick = 0;
//     int rc;
//     chan2_data* td = (chan2_data*) data;
//     while (*td->run)
//     {
//         // this delay comes from a queue
//         poll(nullptr,0,td->delay);
//         if(++tick > 3)
//         {
//             rc = write(td->fd[0],"hi", strlen("hi"));
//             printf(" Sock write rc = %d\n", rc);
//             tick = 0;
//         }
//         // this puts its self on the wake up queue 
//         td->wake_up_chan->put(td->count++);
//     }
//     pthread_exit(nullptr);
// }

// slow loop listen to fd[1]
void *timer_loop(void *data)
{
    //struct pollfd pfds[2];
    int rc;
    //char msg[16];
    chan2_data* td = (chan2_data*) data;
    //pfds[0].fd = td->fd[1];
    //pfds[0].events=POLLIN;

    while (*td->run)
    {
        rc = poll(nullptr,0,td->delay);
        if(0)printf(" poll wake rc = %d\n",rc);
        td->wake_up_chan->put(td->count++);
    }
    printf("%s >> done \n",__func__);
    pthread_exit(nullptr);
}

void *message_loop(void *data)
{
    char *mdata;
    chan2_data* td = (chan2_data*) data;
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);
        if(*td->run)
        {
            asprintf(&mdata, " this is message number %d", td->count++);
            printf("%s >> sending message [%s]\n", __func__, mdata);
            td->message_chan->put(mdata);
            td->wake_up_chan->put(0);
        }
    }
    printf("%s >> done \n",__func__);
    pthread_exit(nullptr);
}

typedef void*(* vLoop)(void *args);

int run(chan2_data*c_data, vLoop c_loop, int *rflag, int delay, channel<int> *wake)
{
    c_data->run = rflag;
    c_data->count = 0;
    c_data-> delay = delay;
    c_data->wake_up_chan = wake;
    return pthread_create(&c_data->thread,nullptr, c_loop, c_data);
}

int running = 1;
int main_test_chan_timeeq(int argc, char *argv[])
{
    double maxTnow = 15.0;
    VarMapUtils vm;

    chan2_data t_data;  // timer ( do we need this)
    //chan2_data t_data2;
    chan2_data m_data;  // lets keep this 
    chan2_data r_data;  // for new run reqs

    channel <int> wakechan;
    //channel <std::string>channel2;
    channel <char*>msgchan;   // anyone can post messages 
    channel <run_req*>runchan; // anyone can post run_reqs
    
    //int rc;
    //int fd[2];
    std::vector<run_req *> rreqs;

    rreqs.reserve(64);
    double tNow = vm.get_time_dbl();

    // pre load some run reqs 
    add_rreq(rreqs, new run_req(tNow+0.1,    0,      "single40", "arg1 arg2 arg3"));
    add_rreq(rreqs, new run_req(tNow+0.2,    0.2,     "rep20at60","arg1 arg2 arg3"));
    add_rreq(rreqs, new run_req(tNow+1.0,    1.0,     "rep30at70","arg1 arg2 arg3"));
    add_rreq(rreqs, new run_req(tNow+0.5,     0.5,     "single25","arg1 arg2 arg3"));
    // 
    printf( " reqs at the start\n");
    show_reqs(rreqs);
  

// pid_t pid;
// char buf[256];
// // ⋮
//     rc = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
//     if(rc < 0){
//  	    printf("SOCKETPAIR ERROR %d\n", rc);
//     }
    int wakeup;
    //std::string item2;
    char* item3;
    run_req* rr;

    t_data.message_chan = &msgchan;
    //t_data.fd[0] = fd[0];t_data.fd[1] = fd[1];
    
    // t_data2.message_chan = &msgchan;
    // t_data2.fd[0] = fd[0];t_data2.fd[1] = fd[1];

    m_data.message_chan = &msgchan;
    
    r_data.run_chan = &runchan;
      
    // timer loop sends us wakeups every 2 secs
    run(&t_data, timer_loop, &running, 2000, &wakechan);

    // timer2 loop sends us wakeups every 1 secs
    if(0)run(&r_data, timer_loop, &running, 1000, &wakechan);

    // message loop sends us wakeups and messages every 1.5 secs
    run(&m_data, message_loop, &running, 1500, &wakechan);

    // todo fims

    // forget the socket pair we'll use two threads.
    // one is the main wake up getting scheduled wakeups and fims messages

    // the second also uses channels but that gets scheduling messages  Do this every xyz seconds or di this once now or do this in .2 seconds.
    // this is the loop that will work out when to wake up the first loop.
    // at last we get to use our message channel.



    // here is what we want to do
    // run every, this is a periodic thing 
    //run_every(100, request_chan, function.....)
    // run once, this is a one_shot thing 
    //run_in(100, request, function.....)

    // any wake up comes here
    // if we use a timed get here we will either wake up on the time out
    // or wake early
    // wakeup would be 0 for run next message or 'n' MS to set the new sleep time.
    // we'll put a structure in wakeup
    //
    // 
    //bool done = false;
    bool bflag = false;
    int delay = 100; // ms
    wakeup = 0;
    while (running)
    {
        // this will wake up after delay regardless
        bflag = wakechan.timedGet(wakeup, delay);
        if(bflag)
        {
            if(0)std::cout << " wakeup bflag true ival "<< wakeup << "\n";
            //this is a true wake up so wake up the main scheduler
            // pop rhe first time off the list and have at it. This also sets the next delay
        }
        else
        {
            if(0)std::cout << " wakeup bflag false ival "<< wakeup << "\n";
        }
        // register the wakeup and quit after 30 
        // std::cout << " wakeup value "<< wakeup << "\n";
        // if(wakeup >= 30)
        // {
        //     std::cout << " wakeup value "<< wakeup << " time to stop\n";
        //     running = 0;
        //     break;
        // }

        // add a queue channel to add the run item
        // the delay is calculated from the lowest item in the runvec.
        // make this sorted soon 
        // now look to see what else is needed
        tNow = vm.get_time_dbl();

        if(msgchan.get(item3,false)) {
            std::cout << " message item3-> data  "<< item3 << "\n";
            //use the message to set the next delay
            free((void *) item3);
            if(tNow>maxTnow)
            {
                std::cout << " wakeup value "<< wakeup << " time to stop\n";
                running = 0;
            }
        }

        if(runchan.get(rr, false)) {
            printf("Adding one at %2.3f\n", tNow);
            rr->show(0);
            add_rreq(rreqs, rr);
        }

        // now look at the tasks and run one if needed
        // 
         //rr = pull_req(rreqs, t_now);
        //tNow = vm.get_time_dbl();
        if (rreqs.size() > 0)
        {
            rr = rreqs.front();
            if(rr->t_shot<=tNow)
            {
                printf("Running  one at %2.3f  :: ", tNow);
                rr->show(((int)tNow* 1000));
                rreqs.erase(rreqs.begin());
                if (rr->t_rep > 0.0)
                {
                    rr->t_shot+=rr->t_rep;
                    add_rreq(rreqs, rr);// new run_req(rn->t_shot+rn->t_rep, rn->t_rep,rn->fname,rn->args));
                }
                else
                {
                    delete rr;
                }
            }
        }
        // now pick off the next delay
        if (rreqs.size() > 0)
        {
            tNow = vm.get_time_dbl();

            rr = rreqs.front();
            delay = (int)((rr->t_shot - tNow)*1000);
            if(0)printf("  >>>new delay = %d \n", delay);
            if(delay < 0)
                delay = 0;
            if(delay > 200)
                delay = 200;

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

    pthread_join(t_data.thread,nullptr);
    std::cout << " t_data all done " << "\n";
    pthread_join(m_data.thread,nullptr);
    std::cout << "m_data all done " << "\n";
    printf("at end\n");
    show_reqs(rreqs);
    clear_reqs(rreqs);
    return 0;
}

#if defined _MAIN
int main(int argc, char *argv[])
{
   return  main_test_chan_timeeq(argc, argv);
}
#endif

    