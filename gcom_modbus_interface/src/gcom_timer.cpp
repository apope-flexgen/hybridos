// gcom_timer.cpp
// p. wilshire
// s .reynolds
// 11_08_2023
// self review 11_22_2023
// transitioned to std::set 11_26_2023

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <set>

#include "gcom_timer.h"
#include "logger/logger.h"

// Custom comparator used in a set
struct CompareTimerSet {
    bool operator()(const std::shared_ptr<TimeObject>& a, const std::shared_ptr<TimeObject>& b) const {
        if (a->runTime == b->runTime)
            return (a->id < b->id);
        return(a->runTime < b->runTime);

    }
};

std::set<std::shared_ptr<TimeObject>, CompareTimerSet> timerSet;

std::mutex timerMutex;
std::condition_variable timerCv;
bool stopThread = false;

void _addTimeObjToSet(std::shared_ptr<TimeObject> timeObj){
    timerSet.insert(timeObj);
    timerCv.notify_one();
}

void addTimeObjToSet(std::shared_ptr<TimeObject> timeObj, double offset, bool isSync) {
    std::lock_guard<std::mutex> lock(timerMutex);
    _addTimeObjToSet(timeObj);
}

std::shared_ptr<TimeObject> _findTimerSetByName(const std::set<std::shared_ptr<TimeObject>, CompareTimerSet>& timerSet, const std::string& name) {
    auto it = std::find_if(timerSet.begin(), timerSet.end(),
                           [&name](const std::shared_ptr<TimeObject>& obj) {
                               return obj->name == name;
                           });

    if (it != timerSet.end()) {
        return *it;
    } else {
        return nullptr; // or throw an exception, or handle the not-found case as needed
    }
}

std::shared_ptr<TimeObject> findTimerSetByName(const std::set<std::shared_ptr<TimeObject>, CompareTimerSet>& timerSet, const std::string& name) {
    std::lock_guard<std::mutex> lock(timerMutex);
    return _findTimerSetByName(timerSet, name);
}

void timerSetThreadFunc() {
    std::unique_lock<std::mutex> lock(timerMutex);
    while (!stopThread) {
        if (!timerSet.empty()) {
            auto tNow = get_time_double();
            auto now = std::chrono::system_clock::now();
            //auto nowMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
            //double nowSeconds = nowMicroseconds / 1e6;  // Convert microseconds to seconds
            //auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now).time_since_epoch().count();
            // std::chrono::duration_cast<std::chrono::seconds>(
            //                std::chrono::system_clock::now().time_since_epoch())
            //                .count();
            std::shared_ptr<TimeObject> tObj = *timerSet.begin();
            double waitTime = tObj->runTime - tNow; 
            if(0)std::cout << " tNow " << tNow
                            << " TimerName " << tObj->name 
                            << " id " << tObj->id 
                            << " users " << tObj.use_count() 
                            << " runTime " << tObj->runTime 
                            << " repeatTime "  << tObj->repeatTime 
                            << " waitTime " << waitTime 
                            << " timerSet size " << timerSet.size() 
                            << std::endl;

            if (waitTime <= 0) {
                if (1 ||!tObj->sync || (tObj->sync && !tObj->run)) {
                    tObj->lastRun = tNow;
                    tObj->runs++;
                    tObj->run = true;

                    // Run the callback
                    auto callback = tObj->callback;
                    auto param = tObj->param;
                    auto name = tObj->name;
                    auto interval = tObj->repeatTime;
                    // reset sync if we need to 
                    // if (tObj->sync) {
                    //     if (tObj->run) {
                    //         if (tObj->lastRun + (tObj->repeatTime*2) <= tNow) {
                    //             tObj->run = false;                
                    //         }
                    //     }
                    // }
                    //timerSet.erase(timerSet.begin());
                    timerSet.erase(tObj);

                    tObj->runTime += interval;
                    // if (tObj->sync)
                    //     tObj->runTime += interval;

                    lock.unlock(); // Unlock while running the callback
                    if(callback)
                        callback(tObj, param);
                    lock.lock(); // Re-lock to insert back
                    //if((interval == 0.0)

                    if((interval> 0.0) 
                       && (tObj->stopTime == 0 || tObj->runTime < tObj->stopTime)
                       && (tObj->count == 0 || tObj->runs < tObj->count)
                       )
                    {
                        if(0)std::cout 
                            << " tNow " << tNow 
                            << " Adding back " << tObj->name 
                            << " id " << tObj->id 
                            << " users " << tObj.use_count() 
                            << " runTime " << tObj->runTime 
                            << " stopTime " << tObj->stopTime 
                            << " count " << tObj->count 
                            << " timerSet size " << timerSet.size() 
                            << std::endl;

                        //tObj->runTime += 0.0000001;

                        // auto insertres = timerSet.insert(tObj);
                        // if(!insertres.second)
                        // {
                        //     std::cout 
                        //     << " tNow " << tNow 
                        //     << " After Adding back " << tObj->name 
                        //     << " insertres " << insertres.second 
                        //     << " users " << tObj.use_count() 
                        //     << " runTime " << tObj->runTime 
                        //     << " stopTime " << tObj->stopTime 
                        //     << " count " << tObj->count 
                        //     << " timerSet size " << timerSet.size() 
                        //     << std::endl;
                        // }
                        timerSet.insert(tObj);
                        
                    }
                    else
                    {

                        std::cout << " Dropping " << tObj->name 
                            << " tNow " << tNow 
                            << " interval " << interval 
                            << " count " << tObj->count 
                            << " runs " << tObj->runs 
                            << " runTime " << tObj->runTime 
                            << " stopTime " << tObj->stopTime 
                            << std::endl;
                        //timerSet.erase(tObj);

                    }
                }
            } else {
                timerCv.wait_until(lock, now + std::chrono::duration<double>(waitTime));
                //timerCv.wait_until(lock, std::chrono::system_clock::now() + std::chrono::seconds(static_cast<long>(nextTimer.runtime - now)));
            }
        } else {
            timerCv.wait(lock);
        }
    }
    FPS_INFO_LOG("Timer Thread stopping");

}


std::condition_variable timerCV;
//std::atomic<bool> shouldTerminate;

bool timeInit = false;
std::mutex timeDoubleMutex;
std::chrono::system_clock::time_point baseTime;

double get_time_double() {
    std::lock_guard<std::mutex> lock(timeDoubleMutex);
    if (!timeInit)
        baseTime = std::chrono::system_clock::now();
    timeInit =  true;
    std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = currentTime - baseTime;
    return duration.count();
}

double get_basetime_double() {
    std::lock_guard<std::mutex> lock(timeDoubleMutex);
    if (!timeInit)
        baseTime = std::chrono::system_clock::now();
    timeInit =  true;
    std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = currentTime - baseTime;
    return duration.count();
}

//We can use get_time_double() for approximately 285 million years before you lose microsecond resolution.
//We  are safe using double to represent time with microsecond resolution for any foreseeable application!


void setTimeObject(const std::shared_ptr<TimeObject> newObj, double offset, bool isSync)
{
    newObj->offsetTime = offset;
    newObj->sync = isSync;

    auto origRunTime = newObj->runTime;
    auto tNow = get_time_double();
    if (newObj->repeatTime > 0 && tNow >= newObj->runTime) {
        // no matter when we request a timer, they are always sync'd to the same time 
        double nextRepeatTime = std::ceil(tNow / newObj->repeatTime ) * newObj->repeatTime;
        // Calculate the new run time based on repeated intervals and the offset
        double nextRunTime = nextRepeatTime + newObj->offsetTime;

        // update newObj.runTime
        newObj->runTime = nextRunTime;
    } else {
        // Only respect the initial runTime
        newObj->runTime = origRunTime + newObj->offsetTime;
    }

}

void addTimeObject(const std::shared_ptr<TimeObject> newObj, double offset, bool isSync) {
    std::lock_guard<std::mutex> lock(timerMutex);
    setTimeObject(newObj, offset, isSync);
    return _addTimeObjToSet(newObj);
}

void exampleCallback(std::shared_ptr<TimeObject> t, void* p) {
    double rtime = get_time_double();
    std::cout << "Callback for :" << t->name <<" executed at : " << rtime << " with param: " << *((int*)p) << std::endl;
}

bool syncTimeObjectByName(const std::string& name, double late_margin_percent) {
    double tNow = get_time_double();
    double sTime, lTime;
    if(0)std::cout << __func__ << " syncTime " << name <<" Sync " << late_margin_percent<< std::endl;
    std::lock_guard<std::mutex> lock(timerMutex);
    auto tObj = _findTimerSetByName(timerSet, name);

    if(0)
    {
    if(tObj)
        std::cout << __func__ << " syncTime " << name <<" tObj " << (void*)tObj.get() << " use_sync " << tObj->sync<< std::endl;
    else
        std::cout << __func__ << " syncTime " << name << " no tObj found" << std::endl;
    }
    if (tObj && tObj->sync)
    {
        //if(1)std::cout << __func__ << " syncTime " << name <<" tObj " << (void*)tObj << std::endl;
        tObj->syncPct = late_margin_percent;
        timerSet.erase(tObj);
        sTime = tNow - tObj->lastRun;
        lTime = tObj->repeatTime * late_margin_percent;
        if (sTime > lTime) {
            tObj->runTime = tNow + tObj->repeatTime;
            tObj->total_sync_time+=sTime;
            tObj->total_sync_count++;
        }
        tObj->run = false;
        timerSet.insert(tObj);
        if(0)std::cout << __func__ << " snapTime " << sTime <<" limit " << lTime <<" Sync " << late_margin_percent<< std::endl;
        return true;

    }
    return false;
}

bool modifyTimeObjectByName(const std::string& name, double newRunTime, double newStopTime, double newRepeatTime) {
    std::lock_guard<std::mutex> lock(timerMutex);
    auto tObj = _findTimerSetByName(timerSet, name);
    if (tObj)
    {
        timerSet.erase(tObj);

        if(newRunTime > 0) tObj->runTime = newRunTime;
        if(newStopTime > 0 ) tObj->stopTime = newStopTime;
        if(newRepeatTime > 0) tObj->repeatTime = newRepeatTime;
        if((tObj->repeatTime > 0.0) && (tObj->stopTime != 0 && tObj->runTime < tObj->stopTime))
                timerSet.insert(tObj);
        return true;

    }
    return false;
}

void showTimerObjects() {
    std::lock_guard<std::mutex> lock(timerMutex);

    for (const auto& tPtr : timerSet) {
        std::cout << tPtr->name 
                << " runTime :"    << tPtr->runTime 
                << " repeatTime :" << tPtr->repeatTime 
                << " runs :"       << tPtr->runs 
                << " lastRun :"    << tPtr->lastRun 
                << " syncCount :"     << tPtr->total_sync_count 
                << " totalSyncTime :" << tPtr->total_sync_time 
                << " syncPercent :"   << tPtr->syncPct 
                << std::endl;

    }
}

void getTimerObjects(std::stringstream &ss) 
{
    std::lock_guard<std::mutex> lock(timerMutex);

    ss << "\"timers\": [";
    bool first = true;
    for (const auto& tPtr : timerSet) {
        if (first)first = false; else ss<<",";
        ss << "{\"name\":\""<< tPtr->name << "\""
                << ",\"runTime\" :"    << tPtr->runTime 
                << " ,\"repeatTime\" :" << tPtr->repeatTime 
                << ",\"runs\" :"       << tPtr->runs 
                << ",\"lastRun\" :"    << tPtr->lastRun 
                << ",\"syncCount\" :"      << tPtr->total_sync_count 
                << ",\"totalSyncTime\" :"  << tPtr->total_sync_time 
                << ",\"syncPct\" :"        << tPtr->syncPct 
                << "}";

    }
    ss << "]";
}

std::thread timerThread;
    
void runTimer()
{
    stopThread = false;
    timerThread = std::thread(timerSetThreadFunc);
}

void stopTimer()
{
    stopThread = true;
    timerCv.notify_one();
    // Cleanup
    timerThread.join(); 
    
}

void setBaseTime()
{
    baseTime = std::chrono::system_clock::now();
}

std::shared_ptr<TimeObject> createTimeObject(std::string timer_name, double timer_runTime, double timer_stopTime, 
    double timer_repeatTime, int timer_count, void (*timer_callback)(std::shared_ptr<TimeObject>,void*), void* timer_callbackParams)
{
    double offset = 0.0;
    auto tobj = std::make_shared<TimeObject>(timer_name, timer_runTime, timer_stopTime,
         timer_repeatTime, timer_count, offset, timer_callback, timer_callbackParams);
    //timeObjects.emplace_back(tobj);

    return tobj;
}

std::shared_ptr<TimeObject> createOffsetTimeObject(std::string timer_name, double timer_runTime, double timer_stopTime, 
         double timer_repeatTime, int timer_count, int timer_offset, void (*timer_callback)(std::shared_ptr<TimeObject>,void*), void* timer_callbackParams)
{
    auto tobj = std::make_shared<TimeObject>(timer_name, timer_runTime, timer_stopTime, 
            timer_repeatTime, timer_count, timer_offset, timer_callback, timer_callbackParams);
    //timeObjects.emplace_back(tobj);
    return tobj;
}

int test_timer() 
{
    setBaseTime();
    std::cout << __func__ << " Running "<< std::endl;

    double start = get_time_double();

    int exampleParam = 42;
    int exampleParam1 = 11;
    int exampleParam2 = 12;
    int exampleParam3 = 13;
    int exampleParam5 = 15;
    int exampleParam6 = 16;

    auto obj1 = std::make_shared<TimeObject>("obj1", 15.0,   0,    0.1,  0,   exampleCallback, &exampleParam1);
    auto obj2 = std::make_shared<TimeObject>("obj2", 1.0,   0,    1.0,    0,   exampleCallback, &exampleParam2);
    auto obj3 = std::make_shared<TimeObject>("obj3", 10.0,   0,    0.001,    10,   exampleCallback, &exampleParam3);
    auto obj5 = std::make_shared<TimeObject>("obj5", 1.0,   0,    0.2,    0,   exampleCallback, &exampleParam5);
    auto obj6 = std::make_shared<TimeObject>("obj6", 1.0,   0,    0.2,    0,   exampleCallback, &exampleParam6);

    addTimeObject(obj1, 0.0, false);
    addTimeObject(obj2, 0.0, true);
    addTimeObject(obj3, 0.01, false);

    showTimerObjects();

    auto obj4 = std::make_shared<TimeObject>("obj4", 19.0,   0,  0, 1, exampleCallback, &exampleParam);
    addTimeObject(obj4,0.0, false);
    modifyTimeObjectByName("obj4", 2.0, 0, 0);
    showTimerObjects();

    std::thread timerThread;
    timerThread = std::thread(timerSetThreadFunc);
    
    // Simulate some external activity
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    double tNow = get_time_double();

    std::cout << " syncing obj2 now after " << tNow<< " seconds, should reset offset " << std::endl;
    syncTimeObjectByName("obj2", 0.5);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    tNow = get_time_double();
    std::cout << " syncing obj2 now after " << tNow << " seconds, should not offset" << std::endl;
    syncTimeObjectByName("obj2", 0.5);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Let's wait for a while to observe the behavior
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "adding obj5" << std::endl;
    addTimeObject(obj5, 0.0, false);

    std::cout << "sleeping for 5 secs" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "killing threads" << std::endl;
    stopThread = true;
    timerCv.notify_one();
    //std::cout << "killed threads" << std::endl;
    // Cleanup

    timerThread.join(); // In a real-world scenario, we'd have a mechanism to signal the timer thread to exit and then join.

    showTimerObjects();
    double end = get_time_double();

    std::cout << " duration secs :" << (end - start)  << std::endl;

    return 0;
}


#ifdef MYTEST

int main() {
     return test_timer();
}
#endif

