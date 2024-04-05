#ifndef CHANNEL_HPP
#define CHANNEL_HPP

// channel.h
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "fims/libfims.h"
// using namespace std::chrono_literals;

template <typename T>
class channel
{
private:
    std::deque<T> queue;
    std::mutex m;
    std::condition_variable cv;
    bool closed = false;

public:
    // channel() : closed(false) {} // this is not needed, default constructor
    // will do.
    void close()
    {
        std::unique_lock<std::mutex> lock(m);
        closed = true;
        cv.notify_all();
    }

    bool is_closed()
    {
        std::unique_lock<std::mutex> lock(m);
        return closed;
    }

    void put(const T& in)
    {
        std::unique_lock<std::mutex> lock(m);
        if (closed)
        {
            throw std::logic_error("put to closed channel");
        }
        queue.emplace_back(in);
        cv.notify_one();
    }

    bool get(T& out, bool wait = true)
    {
        std::unique_lock<std::mutex> lock(m);
        if (wait)
        {
            cv.wait(lock, [&]() { return closed || !queue.empty(); });
        }
        if (queue.empty())
        {
            return false;
        }
        out = queue.front();
        queue.pop_front();
        return true;
    }
    // this is used for the incoming timer requests
    // if we returned false we timed out with no new requests
    // if we returned true process the new request.
    // add so it we returned with no messages get the management task to pop the
    // ltest off the timer queue and run it if we rerurned with a message that add
    // the message to the timer queue and calculate the new time out deprecated ..
    // we use the double one below now.
    bool timedGet(T& out, int timeMs)
    {
        std::unique_lock<std::mutex> lock(m);
        auto now = std::chrono::system_clock::now();
        if (cv.wait_until(lock, now + std::chrono::milliseconds(timeMs), [&]() { return closed || !queue.empty(); }))
        {
            // waiting finished preform action
        }
        else
        {
            // we have a new item that has been added to the queue
            // we got woken up to recalc the next wake up time
            // but still return false the outer program will reevalate the top item in
            // the queue
            return false;
        }

        if (queue.empty())
        {
            return false;
        }
        out = queue.front();
        queue.pop_front();
        return true;
    }
    // this is used for the incoming timer requests
    // if we returned false we timed out with no new requests
    // if we returned true process the new request.
    // add so it we returned with no messages get the management task to pop the
    // ltest off the timer queue and run it if we rerurned with a message that add
    // the message to the timer queue and calculate the new time out
    bool timedGet(T& out, double timeS)
    {
        std::unique_lock<std::mutex> lock(m);
        auto now = std::chrono::system_clock::now();
        if (timeS > 1.0)
        {
            int timeMs = static_cast<int>(timeS * 1000.0);
            if (cv.wait_until(lock, now + std::chrono::milliseconds(timeMs),
                              [&]() { return closed || !queue.empty(); }))
            {
                // waiting finished preform action
            }
            else
            {
                // we have a new item that has been added to the queue
                // we got woken up to recalc the next wake up time
                // but still return false the outer program will reevalate the top item
                // in the queue
                return false;
            }
        }
        else
        {
            int timeUs = static_cast<int>(timeS * 1000000.0);
            // TODO(root): work out how [&](){....} ( enclosure)  works...
            if (cv.wait_until(lock, now + std::chrono::microseconds(timeUs),
                              [&]() { return closed || !queue.empty(); }))
            {
                // waiting finished preform action
            }
            else
            {
                // we have a new item that has been added to the queue
                // we got woken up to recalc the next wake up time
                // but still return false the outer program will reevalate the top item
                // in the queue
                return false;
            }
        }
        if (queue.empty())
        {
            return false;
        }
        out = queue.front();
        queue.pop_front();
        return true;
    }
};

/*
channel <int> wakechan
channel <std::string>channel2

int wakeup;
std::string item2;
// any wake up comes here
if(wakechan.get(wakeup,true)) {
    // then service the components
  // ...
   if(channel2.get(item2,false)) {
     // ...
  }
}
*/
// TODO(root): deprecated remove after MVP
typedef struct chan_data_t
{
    channel<int>* wake_up_chan;
    channel<char*>* message_chan;
    channel<fims_message*>* fims_chan;
    int count;
    int delay;
    volatile int* run;
    char** subs;
    int numSubs;
    const char* name;
    pthread_t thread;
    std::thread cthread;

} chan_data;

// template ??
template <typename T>
class tchan_data
{
    channel<int>* wake_up_chan;
    channel<T>* message_chan;
    int count;
    int delay;
    volatile bool* run;
    char** subs;
    int numSubs;
    char* name;
    pthread_t thread;
    std::thread cthread;
};
typedef void* (*runLoop)(void* args);

#endif
