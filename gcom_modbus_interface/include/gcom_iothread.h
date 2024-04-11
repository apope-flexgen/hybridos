#ifndef GCOM_IOTHREAD_H
#define GCOM_IOTHREAD_H


// gcom thread headers
// p. wilshire
// 11_22_2023

#include <condition_variable>
#include <mutex>
#include <deque>
#include <atomic>
#include <future>
#include <fims/libfims.h>
#include <iostream>

//#include "rigtorp/SPSCQueue.h"
//#include "rigtorp/MPMCQueue.h"

//#include "semaphore.hpp" // Linux semaphore wrapper with helper functions (use PCQ_Semaphore)

#include "shared_utils.h"
#include "gcom_config.h"
#include "logger/logger.h"

enum class WorkTypes
{
    Noop,
    Set,
    Get,
    Poll
    // ... Add other types as needed
};

// Channel definition
template <typename T>
class ioChannel {
private:
    std::queue<T> queue;
    //std::deque<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void send(T&& message) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace(std::move(message));
        }
        cv.notify_one();
    }
    void sendb(T& message) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace(std::move(message));
        }
        cv.notify_one();
    }

    bool receive(T& message) {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.empty()) {
            cv.wait(lock);
        }

        message = std::move(queue.front());
        queue.pop();
        return true;
    }

    bool receive(T& message, double durationInSeconds) {
        std::unique_lock<std::mutex> lock(mtx);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(durationInSeconds));
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); })) {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false; // timed out without receiving a message
    }

    bool receive(T& message, const std::chrono::seconds& duration) {
        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); })) {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false; // timed out without receiving a message
    }
    bool peekpop(T& message) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!queue.empty()) {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false;
    }
};



// this is a last_in first_out channel , use the fsend / fsendb functions to populate the head of the queue
template <typename T>
class ioDeque {
private:
    //std::queue<T> queue;
    std::deque<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void send(T&& message) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace_back(std::move(message));
        }
        cv.notify_one();
    }
    void sendb(T& message) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace_back(std::move(message));
        }
        cv.notify_one();
    }
    void fsend(T&& message) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.push_front(std::move(message));
        }
        cv.notify_one();
    }
    void fsendb(T& message) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.push_front(std::move(message));
        }
        cv.notify_one();
    }

    bool receive(T& message) {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.empty()) {
            cv.wait(lock);
        }

        message = std::move(queue.front());
        queue.pop_front();
        return true;
    }

    bool receive(T& message, double durationInSeconds) {
        std::unique_lock<std::mutex> lock(mtx);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(durationInSeconds));
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); })) {
            message = std::move(queue.front());
            queue.pop_front();
            return true;
        }
        return false; // timed out without receiving a message
    }

    bool receive(T& message, const std::chrono::seconds& duration) {
        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); })) {
            message = std::move(queue.front());
            queue.pop_front();
            return true;
        }
        return false; // timed out without receiving a message
    }
    bool peekpop(T& message) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!queue.empty()) {
            message = std::move(queue.front());
            queue.pop_front();
            return true;
        }
        return false;
    }
};


struct IO_Work {
    // this is to allow the poll item to be collected
    int mynum;
    static int wnum;
    u64 use_count  = 0;
    static const uint16_t LOCAL            = 1 << 0;
    static const uint16_t POINT_ERROR      = 1 << 1;
    static const uint16_t POINT_DISCONNECT = 1 << 2;
    static const uint16_t REMOVE_GAP       = 1 << 3;

    IO_Work()
    {
        mynum = ++wnum;
        //std::cout << " IO Work Object " << mynum <<  " created"<< std::endl;

    };

    ~IO_Work()
    {
        printf("IO Work Object [%d]\tCount: %ld \tName: %s\t deleted\n", mynum, use_count, work_name.c_str());
    };
    double tNow;
    std::shared_ptr<cfg::pub_struct> pub_struct;
    std::string work_name;
    int work_id;       // 7 of 9
    int work_group;   // how many in the group
    int device_id;


    int threadId;
    cfg::Register_Types register_type;
    WorkTypes wtype;  //pub set etc
    int errno_code;
    int offset;
    int errors;
    int good;
    int start_register;
    int num_registers;
    std::vector<int> disabled_registers;

    std::vector<std::shared_ptr<struct cfg::io_point_struct>> io_points;
    std::shared_ptr<struct cfg::register_group_struct> register_groups;
    cfg::component_struct* component;

    double tStart;
    double tIo;
    double tDone;
    double tReceive;
    double tPool;
    double connect_time;
    uint8_t buf8[256];
    uint16_t buf16[256];
    uint16_t flags[256];
    std::string replyto;

    int size;
    int bit;
    double response;
    bool test_mode = false;
    double tRun;
    double cTime;
    bool test_it = false;

    bool unforced = false;
    bool local = false; // bypass server , use or set local data
    bool erase_group = false; // set true to force erase of a pubgroup
    bool off_by_one =  false;
    int thread_num = 0;
    bool data_error =  false;
    bool full = false;

    std::string func;
    // IO_Work(IO_Work&& other) noexcept  {};

    // IO_Work& operator=(IO_Work&& other) noexcept {
    //     return *this;
    // };

    ioChannel<std::shared_ptr<IO_Work>>* io_repChan;  // Thread picks up IO_work and processes it

    void clear_bufs() {
        memset(buf8, 0, sizeof(buf8));    // Set buf8 to 0
        memset(buf16, 0, sizeof(buf16));  // Set buf16 to 0 
    };

    void set_bufs(int num_bufs,uint16_t* buf16_in, uint8_t* buf8_in ) {
        if (num_bufs > 0) {
            if (buf8_in) {
                memcpy(buf8, buf8_in, num_bufs* sizeof(uint8_t)); 
            }
            else {
                memset(buf8, 0, sizeof(buf8));    // Set buf8 to 0
            }
            if (buf16_in) {
                memcpy(buf16, buf16_in, num_bufs* sizeof(uint16_t)); 
            }
            else {
                memset(buf16, 0, sizeof(buf16));    // Set buf8 to 0
            }
        }
    };
    // Method to set a flag
    void set_flag(size_t ioffset, uint16_t flag) {
        size_t myoffset = ioffset - offset;
        if (myoffset < 256) {
            flags[myoffset] |= flag; // Set the flag bit using bitwise OR
        }
    }

    // Method to check if a flag is set
    bool get_flag(size_t ioffset, uint16_t flag) const {
        size_t myoffset = ioffset - offset;
        if (myoffset < 256) {
            return flags[myoffset] & flag; // Test the flag bit using bitwise AND
        }
        return false; // Return false if offset is out of bounds
    }
};


//extern int IO_Work::wnum = 0;

struct myMeta_Data_Info
{
    // constants:
    static constexpr auto Max_Meta_Data_Str_Size = std::numeric_limits<uint8_t>::max();
    static constexpr uint16_t Buf_Len = Max_Meta_Data_Str_Size * 5;

    // first X bytes of the data buffer for unencrypted meta_data info:
    uint8_t method_len = 0;
    uint8_t uri_len = 0;
    uint8_t replyto_len = 0;
    uint8_t process_name_len = 0;
    uint8_t username_len = 0;
    uint8_t spare_1 = 0;
    uint8_t spare_2 = 0;
    uint8_t spare_3 = 0;
    // last X bytes of the data buffer for receiving (we encrypt/decrypt this part of the data buffer):
    uint32_t data_len = 0;

    // helper functions:
    std::size_t get_meta_bytes() const noexcept;
    // This is used by server only (for resending purposes):
    std::size_t get_total_bytes() const noexcept;
};

#define FIMS_BUFFER_DEFAULT_LEN 100000

/// @brief 
// these structures get in a fims message and turn the body into an std::any map
static int io_fims_count = 0;

struct IO_Fims {
    double tId;
    u64 jobs = 0;
    u64 total_data = 0;
    int fid;    
    std::string work_name;
    std::string_view method_view;
    std::string_view uri_view;
    std::string_view replyto_view;
    std::string_view process_name_view;
    std::string_view username_view;

    struct myMeta_Data_Info meta_data;
    u32 fims_data_buf_len=FIMS_BUFFER_DEFAULT_LEN;
    u32 data_buf_len=FIMS_BUFFER_DEFAULT_LEN;
    u8* fims_input_buf;
    u8* fims_data_buf;
    u64 fims_data_len;
    size_t bytes_read;
    u64 fims_input_buf_len;
    std::any anyBody;
    bool is_request;
    struct Uri_req uri_req;
    int error;

    IO_Fims()
    {
        fid = io_fims_count++;
        jobs = 0;
        tId = get_time_double();
        is_request =  false;
        FPS_INFO_LOG("IO_Fims create tId [%f]", tId);
        fims_data_buf_len = FIMS_BUFFER_DEFAULT_LEN;
        fims_input_buf_len = FIMS_BUFFER_DEFAULT_LEN;
        fims_input_buf = reinterpret_cast<uint8_t *>(calloc(1,fims_data_buf_len));
        //memset(&meta_data,0,sizeof(Meta_Data_Info));
    };

    IO_Fims(int buffer_len)
    {
        fid = io_fims_count++;
        jobs = 0;
        tId = get_time_double();
        is_request =  false;
        FPS_INFO_LOG("IO_Fims create tId [%f]", tId);
        fims_data_buf_len = buffer_len;
        fims_input_buf_len = buffer_len;
        fims_input_buf = reinterpret_cast<uint8_t *>(calloc(1,fims_data_buf_len));
        //memset(&meta_data,0,sizeof(Meta_Data_Info));
    };
    ~IO_Fims()
    {
        printf("IO_Fims id: %d\ttId: %f\tuses: %ld\t total data: %ld\n", fid, tId, jobs, total_data);
        if(fims_input_buf)
        {
           free(fims_input_buf);
        }
    };

    bool set_fims_data_buf(unsigned int data_buf_len)
    {
        if(fims_input_buf)
        {
            free(fims_input_buf);
            fims_input_buf = nullptr;
        }

        std::cout << " set buff len to " << data_buf_len << std::endl;
        fims_input_buf = reinterpret_cast<uint8_t *>(malloc(data_buf_len));

        fims_input_buf_len = data_buf_len; ///sys.fims_dependencies->data_buf_len;
        fims_data_buf_len = data_buf_len; ///sys.fims_dependencies->data_buf_len;

        return true;
    };

    bool reset_fims_data_buf(unsigned int data_buf_len)
    {
        auto old_buf = fims_input_buf;
        fims_input_buf = reinterpret_cast<uint8_t *>(malloc(data_buf_len));

        if(old_buf)
        {
            std::cout << " set buff len to " << data_buf_len << std::endl;

            auto data_buf_orig = fims_data_buf_len;
            if (data_buf_orig > data_buf_len )
            {
                data_buf_orig = data_buf_len;
            }

            memcpy(fims_input_buf, old_buf, data_buf_orig);
            free(old_buf);
        }

        fims_input_buf_len = data_buf_len; ///sys.fims_dependencies->data_buf_len;
        fims_data_buf_len = data_buf_len; ///sys.fims_dependencies->data_buf_len;

        return true;
    };


};


//std::unique_lock<std::mutex> lock(*pg.pmtx); 
//            io_thread->modbus_read_timer.start();
//                    io_thread->modbus_read_timer.showNum(ss);
//            io_thread->modbus_read_timer.snap();


struct PubGroup {
    // Constructor that takes a key and a shared_ptr to IO_Work
    std::string key;
    std::shared_ptr<cfg::pub_struct> pub_struct;
    std::vector<std::shared_ptr<IO_Work>> works;
    int work_group = 0;  // size
    double tNow = 0.0;
    double tDone = 0.0;
    bool done=false;
    bool erase_group=false;

    PubGroup() {
    };
    ~PubGroup() {
    };

    PubGroup(std::string& _key, std::shared_ptr<IO_Work> io_work, bool save)
        : key(_key) {
            if (io_work != nullptr)
            {
                reset_group(key,io_work);
            }
            // pub_struct =  io_work->pub_struct;
            // work_group = io_work->work_group; 
            // tNow = io_work->tNow; 
            // done = false; 
            // erase_group = io_work->erase_group;
            // if (save)
            //     works.push_back(io_work);

    };

    void reset_group(std::string& _key, std::shared_ptr<IO_Work> io_work)
    {
        if (io_work != nullptr)
        {
            pub_struct =  io_work->pub_struct;
            work_group = io_work->work_group; 
            tNow = io_work->tNow; 
            erase_group = io_work->erase_group;
        }
        tDone = 0.0;
        done = false; 

    };
};

struct IO_Work_group {
    std::string key;
    std::vector<std::shared_ptr<IO_Work>> works;
    int work_group;  // size
    double tNow;
    bool done=false;
    bool erase_group=false;
};


struct ThreadControl;

struct IO_Thread {

    IO_Thread(){
        modbus_read_timer.set_label("Modbus Read Timer");
        modbus_write_timer.set_label("Modbus Write Timer");
        is_enabled = true;
        connect_reset = 0;
    };

    alignas(64) modbus_t* ctx = nullptr;
    cfg *myCfg = nullptr;
    //xMain_Thread* main_work;
    int id;

    std::atomic<bool> keep_running;
    std::future<bool> thread_future; // IO Thread future
   
    Stats modbus_read_timer;
    Stats modbus_write_timer;
    
    std::condition_variable* cv;
    std::mutex stat_mtx;
    std::mutex* mtx;
    bool* ready;

    bool wasConnected =  false;
    bool hadContext =  false;



    int port;
    int tid;
    int jobs;
    int fails = 0;
    int connect_fails = 0;
    int connect_reset = 0;

    std::thread thread;
    ThreadControl *thread_control;
    std::string ip;
    bool connected =  false;
    bool connection_timedout =  false;

    double connection_timeout;
    double transfer_timeout;
    int transaction_timeout;

    double connect_time;
    double transaction_time;

    double cTime; // TODO deprecatd
    bool is_local =  false;
    bool is_enabled = true;
};

struct ThreadControl
{
    ioChannel<std::shared_ptr<IO_Work>> *io_pollChan;     // Use Channel to send IO-Work to thread
    ioChannel<std::shared_ptr<IO_Work>> *io_setChan;      // Use Channel to send IO-Work to thread
    ioChannel<std::shared_ptr<IO_Work>> *io_responseChan; // Thread picks up IO_work and processes it
    ioChannel<std::shared_ptr<IO_Work>> *io_poolChan;     // Response channel returns io_work to the pool

    bool ioThreadRunning = true;
    bool responseThreadRunning = true;
    std::vector<std::shared_ptr<IO_Thread>> ioThreadPool;
    std::thread responseThread;

    int num_threads = 4;
    int num_responses = 0;
    double tResponse = 0.0;
    int num_connected_threads = 0;

    // may need a lock
    std::mutex connect_mutex;
    double tConnect = 0.0; // if connection fails set this to tNow plus 1 secong

    void startThreads()
    {
        // Initialize flags
        ioThreadRunning = true;
        responseThreadRunning = true;
    }

    void stopThreads()
    {
        ioThreadRunning = false;
        responseThreadRunning = false;
        for (auto &tp : ioThreadPool)
        {
            if (tp->thread.joinable())
            {
                tp->thread.join();
            }
        }
        // Send shutdown signal to threads
        // io_threadChan.send(0);
    }
};

void getConnectTimes(std::stringstream &ss, bool include_key);


void clearpollChan(bool debug);
bool startRespThread(struct cfg&);
bool stopRespThread(struct cfg&);
void sendpollChantoResp(bool debug);
void clearrespChan(bool debug);
std::shared_ptr<IO_Work> make_work(cfg::Register_Types register_type,  int device_id, int offset, bool off_by_one, int num_regs, uint16_t* u16bufs, uint8_t* u8bufs, WorkTypes wtype );
bool pollWork (std::shared_ptr<IO_Work> io_work);
bool setWork (std::shared_ptr<IO_Work> io_work);
WorkTypes strToWorkType(std::string roper, bool);
cfg::Register_Types strToRegType(std::string& register_type);
void test_io_point_multi(const char* ip, int port, int timeout, const char *oper, int device_id, const char *regtype, int offset, int value, int num_threads, struct cfg&, bool debug);
void test_io_point_single(const char* ip, int port, double timeout, const char *oper, int device_id, const char *regtype, int offset, int num_regs, int value, int num_threads,  struct cfg&, bool debug);
double SetupModbusForThread(std::shared_ptr<IO_Thread> io_thread, bool debug);
bool CloseModbusForThread(std::shared_ptr<IO_Thread> io_thread, bool debug);
std::shared_ptr<IO_Thread> make_IO_Thread(int idx, const char* ip, int port, double connection_timeout,double transfer_timeout, struct cfg& myCfg);
int test_find_bad_regs();
bool test_decode_raw();
bool StartThreads(struct cfg& myCfg, bool debug);
bool StopThreads(struct cfg& myCfg, bool debug);
bool StartThreads(int num_threads, const char *ip, int port, int connection_timeout, struct cfg& myCfg);
void runThreadWork(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work,  bool debug);

bool check_pubgroup_key(std::string key, std::shared_ptr<IO_Work> io_work);
int test_io_threads(struct cfg &myCfg);
void stashWork(std::shared_ptr<IO_Work> io_work);
void ioThreadFunc(ThreadControl &control, struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread);
void responseThreadFunc(ThreadControl &control, struct cfg &myCfg);
std::string regTypeToStr(cfg::Register_Types &register_type);
WorkTypes strToWorkType(std::string roper, bool debug = false);
std::string workTypeToStr(WorkTypes &work_type);
double get_time_double();
void randomDelay(int minMicroseconds, int maxMicroseconds);
double SetupModbusForThread(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug);

double FastReconnectForThread(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug);
WorkTypes strToWorkType(std::string roper, bool debug);







#endif
