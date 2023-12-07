#ifndef GCOM_PERF_H
#define GCOM_PERF_H

#include <string>
#include <map>
#include <limits>
#include <queue>
#include <mutex>
#include <sstream>

#include <condition_variable>
#include <vector>
#include <atomic>

// std::atomic<bool> terminate_thread(false);
// std::condition_variable termination_cv; // Condition variable for thread termination

void* record_thread_func(void* arg);
// pthread_t recordThread;
// bool perf_running = false;



// Forward declarations
class Stats;
class Perf;
class RecordQueue;

// Constants
const size_t BATCH_SIZE = 10;

// Record structure used in RecordQueue

class RecordQueue {
public:
    struct Record {
        std::string label;
        double duration;
    };

    //RecordQueue();

    void push(const std::string& label, double duration);
    bool pop(std::vector<Record>& records_batch);
    bool try_pop(std::vector<Record>& records_batch);
    void flush();
    std::queue<std::vector<Record>> batches;
    std::mutex mtx;
    std::condition_variable cv;

private:
    std::vector<Record> batch;
};

class Perf {
public:
    Perf(const std::string& label);
    ~Perf();

    void start();
    void snap();

private:
    std::string label;
    std::mutex mtx;
    double start_time;
    bool closed;
};

struct Stats {
    //std::mutex mtx;
    double start_time;
    std::string label;
    double max_duration = 0.0;
    double min_duration = std::numeric_limits<double>::max();
    double total_duration = 0.0;
    int count = 0;
    void set_label(std::string);
    void start();
    void start(std::mutex& mtx);
    void snap();
    void snap(std::mutex& mtx);
    void clear();
    void clear(std::mutex& mtx);
    void show();
    void show(std::mutex& mtx);
    void showNum(std::stringstream &ss, std::string _label); 
    void showNum(std::mutex& mtx, std::stringstream &ss, std::string _label); 
    void showNum(std::stringstream &ss); 
    void showNum(std::mutex& mtx,std::stringstream &ss); 


    void record_duration(double duration);
    void record_duration(std::mutex& mtx, double duration);
    void record_duration(int duration);
    void record_duration(std::mutex& mtx, int duration);

    bool started = false;
};

double get_time_double();

void run_perf();
void* record_thread_func(void* arg);
int test_perf(void);
void display_statistics();

#endif // GCOM_PERF_H
