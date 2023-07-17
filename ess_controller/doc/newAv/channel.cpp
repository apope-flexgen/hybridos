//channel.cpp
// concept from here:
//https://st.xorian.net/blog/2012/08/go-style-channel-in-c/
// p. wilshire
// last update 02/24/2021
// look in channel.h as well. 


#include <deque>
#include <thread>

template<class item>
class channel {
private:
    std::deque<item> queue;
    std::mutex m;
    std::condition_variable cv;
    bool closed = false;
public:
    // channel() : closed(false) { } // rule of zero
    void close() {
        std::unique_lock<std::mutex> lock(m);
        closed = true;
        cv.notify_all();
    }
    bool is_closed() {
        std::unique_lock<std::mutex> lock(m);
        return closed;
    }
    void put(const item& i) {
        std::unique_lock<std::mutex> lock(m);
        if(closed)
            throw std::logic_error("put to closed channel");
        queue.push_back(i);
        cv.notify_one();
    }
    bool get(item& out, bool wait = true) {
        std::unique_lock<std::mutex> lock(m);
        if(wait)
            cv.wait(lock, [&]() {
            return closed || !queue.empty();
        });
        if(queue.empty())
            return false;
        out = queue.front();
        queue.pop_front();
        return true;
    }
};