/*
 * channel test
 */

#include <iostream>
#include <poll.h>
#include <pthread.h>
//#include <asset.h>
#include "channel.h"
#include "chrono_utils.hpp"
#include <fims/libfims.h>

// typedef struct chan_data_t {
//     channel<int> *wake_up_chan;
//     channel<char*> *message_chan;
//     int count;
//     int delay;
//     int *run;
//     pthread_t thread;

// } chan_data;

void* timer_loop(void* data)
{
    chan_data* td = (chan_data*)data;
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);
        td->wake_up_chan->put(td->count++);
    }
    pthread_exit(nullptr);
}

void* message_loop(void* data)
{
    char* mdata;
    chan_data* td = (chan_data*)data;
    while (*td->run)
    {
        asprintf(&mdata, " this is message number %d", td->count++);
        poll(nullptr, 0, td->delay);
        if (*td->run)
        {
            td->message_chan->put(mdata);
            td->wake_up_chan->put(0);
        }
        else
        {
            free(mdata);
        }
    }
    pthread_exit(nullptr);
}

typedef void* (*vLoop)(void* args);

int run(chan_data* c_data, vLoop c_loop, int* rflag, int delay, channel<int>* wake)
{
    c_data->run = rflag;
    c_data->count = 0;
    c_data->delay = delay;
    c_data->wake_up_chan = wake;
    return pthread_create(&c_data->thread, nullptr, c_loop, c_data);
}

int running = 1;
int main(int argc, char* argv[])
{
    chan_data t_data;
    chan_data m_data;

    channel<int> wakechan;
    // channel <std::string>channel2;
    channel<char*> msgchan;

    int wakeup;
    // std::string item2;
    char* item3;

    t_data.message_chan = &msgchan;
    m_data.message_chan = &msgchan;

    run(&t_data, timer_loop, &running, 1000, &wakechan);
    run(&m_data, message_loop, &running, 1500, &wakechan);

    // any wake up comes here
    while (wakechan.get(wakeup, true))
    {
        // then service the components
        std::cout << " wakeup ival " << wakeup << "\n";
        if (wakeup >= 10)
        {
            running = 0;
            break;
        }
        if (msgchan.get(item3, false))
        {
            std::cout << " item3-> data  " << item3 << "\n";
            free((void*)item3);
        }
    }

    pthread_join(t_data.thread, nullptr);
    std::cout << " t_data all done "
              << "\n";
    pthread_join(m_data.thread, nullptr);
    std::cout << "m_data all done "
              << "\n";
    return 0;
}
