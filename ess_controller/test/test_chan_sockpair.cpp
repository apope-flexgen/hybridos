/*
 * channel event test code
 * We want to say run this list of functions (each with its ownthis av) (in|every) 10 Seconds
 * run ({fun1, (av1,av2,av3)}, {fun2,{av1,av2,av3}}, "in"|"every" 0.5 secs )
* this will need varadic lists running ..
* and events
* the sock pair is used to wake a poll up before its time but I dont think we need that 
*/

#include <pthread.h>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include "chrono_utils.hpp"



//#include <asset.h>
#include <fims/libfims.h>
#include "channel.h"

typedef struct chan2_data_t {
    channel<int> *wake_up_chan;
    channel<char*> *message_chan;
    int count;
    int delay;
    int *run;
    int fd[2];
    pthread_t thread;

} chan2_data;

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
void *timer2_loop(void *data)
{
    int tick = 0;
    int rc;
    chan2_data* td = (chan2_data*) data;
    while (*td->run)
    {
        poll(nullptr,0,td->delay);
        if(++tick > 3)
        {
            rc = write(td->fd[0],"hi", strlen("hi"));
            printf(" Sock write rc = %d\n", rc);
            tick = 0;
        }
        td->wake_up_chan->put(td->count++);
    }
    pthread_exit(nullptr);
}
// slow loop listen to fd[1]
void *timer_loop(void *data)
{
    struct pollfd pfds[2];
    int rc;
    char msg[16];
    chan2_data* td = (chan2_data*) data;
    pfds[0].fd = td->fd[1];
    pfds[0].events=POLLIN;

    while (*td->run)
    {
        rc = poll(pfds,1,td->delay);
        printf(" poll wake rc = %d\n",rc);
        if (rc > 0)
        {
            rc = read(td->fd[1], msg,16);
            if(rc > 0)
            {
                msg[rc] = 0;
                printf(" Message received [%s]\n", msg);
            }

        }
        td->wake_up_chan->put(td->count++);
    }
    pthread_exit(nullptr);
}

void *message_loop(void *data)
{
    char *mdata;
    chan2_data* td = (chan2_data*) data;
    while (*td->run)
    {
        asprintf(&mdata, " this is message number %d", td->count++);
        poll(nullptr, 0, td->delay);
        td->message_chan->put(mdata);
        td->wake_up_chan->put(0);
    }
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
int main(int argc, char *argv[])
{
    chan2_data t_data;
    chan2_data t_data2;
    chan2_data m_data;

    channel <int> wakechan;
    //channel <std::string>channel2;
    channel <char *>msgchan;
    int rc;
    int fd[2];
// pid_t pid;
// char buf[256];
// ⋮
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if(rc < 0){
 	    printf("SOCKETPAIR ERROR %d\n", rc);
    }
    int wakeup;
    //std::string item2;
    char* item3;

    t_data.message_chan = &msgchan;
    t_data.fd[0] = fd[0];t_data.fd[1] = fd[1];
    
    t_data2.message_chan = &msgchan;
    t_data2.fd[0] = fd[0];t_data2.fd[1] = fd[1];

    m_data.message_chan = &msgchan;

    run(&t_data, timer_loop, &running, 2000, &wakechan);
    run(&t_data2, timer2_loop, &running, 1000, &wakechan);
    run(&m_data, message_loop, &running, 1500, &wakechan);

    // any wake up comes here
    while(wakechan.get(wakeup,true)) {
        // then service the components
        std::cout << " wakeup ival "<< wakeup << "\n";
        if(wakeup >= 10)
        {
            running = 0;
            break;
        }
        if(msgchan.get(item3,false)) {
            std::cout << " item3-> data  "<< item3 << "\n";
            free((void *) item3);
        }
    }

    pthread_join(t_data.thread,nullptr);
    std::cout << " t_data all done " << "\n";
    pthread_join(m_data.thread,nullptr);
    std::cout << "m_data all done " << "\n";
    return 0;
}
