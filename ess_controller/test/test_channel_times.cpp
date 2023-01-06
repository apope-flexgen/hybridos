#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>

#include "testUtils.h"
#include "chrono_utils.hpp"

using namespace testUtils;

template<typename T>
class channel_list
{
private:
    std::deque<T> queue;
    std::mutex m;
    std::condition_variable cv;
    bool closed = false;

public:
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
            throw std::logic_error("put to closed channel");
        queue.push_back(in);
        cv.notify_one();
    }
    bool get(T& out, bool wait = true)
    {
        std::unique_lock<std::mutex> lock(m);
        if (wait)
            cv.wait(lock, [&]() {
            return closed || !queue.empty();
                });
        if (queue.empty())
            return false;
        out = queue.front();
        queue.pop_front();
        return true;
    }

    bool timedGet(T& out, int timeMs)
    {
        std::unique_lock<std::mutex> lock(m);
        auto now = std::chrono::system_clock::now();
        if (cv.wait_until(lock, now + std::chrono::milliseconds(timeMs), [&]() {return closed || !queue.empty(); }))
        {
        }
        else
        {
            return false;
        }

        if (queue.empty())
            return false;
        out = queue.front();
        queue.pop_front();
        return true;
    }
};

template<class T>
class channel_deque
{
private:
    std::deque<T> queue;
    std::mutex m;
    std::condition_variable cv;
    bool closed = false;

public:
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

    // what we want is put@time 
    void put(const T& in)
    {
        std::unique_lock<std::mutex> lock(m);
        if (closed)
            throw std::logic_error("put to closed channel");
        queue.emplace_back(in);
        cv.notify_one();
    }
    
    bool get(T& out, bool wait = true)
    {
        std::unique_lock<std::mutex> lock(m);
        if (wait)
            cv.wait(lock, [&]() {
            return closed || !queue.empty();
                });
        if (queue.empty())
            return false;
        out = queue.front();
        queue.pop_front();
        return true;
    }

    bool timedGet(T& out, int timeMs)
    {
        std::unique_lock<std::mutex> lock(m);
        auto now = std::chrono::system_clock::now();
        if (cv.wait_until(lock, now + std::chrono::milliseconds(timeMs), [&]() {return closed || !queue.empty(); }))
        {
        }
        else
        {
            return false;
        }

        if (queue.empty())
            return false;
        out = queue.front();
        queue.pop_front();
        return true;
    }
};

int main()
{
    {
        channel_list<int> chan;
        int temp = 0;
        Timer timer("channel_list");
        for (int i = 0; i < 1; ++i)
        {
            chan.put(i);
        }
        for (int j = 0; j < 1; ++j)
        {
            chan.get(temp);
        }
    }

    {
        channel_deque<int> chan;
        int temp = 0;
        Timer timer("channel_deque");
        for (int i = 0; i < 1; ++i)
        {
            chan.put(i);
        }
        for (int j = 0; j < 1; ++j)
        {
            chan.get(temp);
        }
    }
}
