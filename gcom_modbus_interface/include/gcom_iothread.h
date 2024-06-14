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

// #include "rigtorp/SPSCQueue.h"
// #include "rigtorp/MPMCQueue.h"

// #include "semaphore.hpp" // Linux semaphore wrapper with helper functions (use PCQ_Semaphore)

#include "shared_utils.h"
#include "gcom_config.h"
#include "logger/logger.h"

#define EMBERROR 112345600

// function code is invalid      EMBXILFUN   112345679
// specified address is invalid. EMBXILADD   112345680
// data is invalid               EMBXILVAL   112345681
// Modbus slave is stopped.      EMBXSFAIL   112345682
// ACK response                  EMBXACK     112345683
// Modbus slave is busy          EMBXSBUSY   112345684
// Negative Ack response         EMBXNACK    112345685
// Memory parity error.          EMBXMEMPAR  112345686
// Bad gateway config            EMBXGPATH   112345687
// No response from target       EMBXGTAR    112345688
// CRC error.                    EMBBADCRC   112345689
// data is invalid.              EMBBADEXC   112345690
// bad data                      EMBBADDATA  112345691
// Reserved code                 EMBUNKEXC   112345692
// Exceeds max data size.        EMBMDATA    112345693
// Device ID different.          EMBBADSLAVE 112345694

#define BAD_DATA_ADDRESS 112345680
#define INVALID_DATA 112345691

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
class ioChannel
{
private:
    std::queue<T> queue;
    // std::deque<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void send(T&& message)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace(std::move(message));
        }
        cv.notify_one();
    }
    void sendb(T& message)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace(std::move(message));
        }
        cv.notify_one();
    }

    bool receive(T& message)
    {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.empty())
        {
            cv.wait(lock);
        }

        message = std::move(queue.front());
        queue.pop();
        return true;
    }

    bool receive(T& message, double durationInSeconds)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::duration<double>(durationInSeconds));
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); }))
        {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false;  // timed out without receiving a message
    }

    bool receive(T& message, const std::chrono::seconds& duration)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); }))
        {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false;  // timed out without receiving a message
    }
    bool peekpop(T& message)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!queue.empty())
        {
            message = std::move(queue.front());
            queue.pop();
            return true;
        }
        return false;
    }
};

// this is a last_in first_out channel , use the fsend / fsendb functions to populate the head of the queue
template <typename T>
class ioDeque
{
private:
    // std::queue<T> queue;
    std::deque<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void send(T&& message)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace_back(std::move(message));
        }
        cv.notify_one();
    }
    void sendb(T& message)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.emplace_back(std::move(message));
        }
        cv.notify_one();
    }
    void fsend(T&& message)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.push_front(std::move(message));
        }
        cv.notify_one();
    }
    void fsendb(T& message)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            queue.push_front(std::move(message));
        }
        cv.notify_one();
    }

    bool receive(T& message)
    {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.empty())
        {
            cv.wait(lock);
        }

        message = std::move(queue.front());
        queue.pop_front();
        return true;
    }

    bool receive(T& message, double durationInSeconds)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::duration<double>(durationInSeconds));
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); }))
        {
            message = std::move(queue.front());
            queue.pop_front();
            return true;
        }
        return false;  // timed out without receiving a message
    }

    bool receive(T& message, const std::chrono::seconds& duration)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, duration, [this] { return !queue.empty(); }))
        {
            message = std::move(queue.front());
            queue.pop_front();
            return true;
        }
        return false;  // timed out without receiving a message
    }
    bool peekpop(T& message)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!queue.empty())
        {
            message = std::move(queue.front());
            queue.pop_front();
            return true;
        }
        return false;
    }
};

struct IO_Work
{
    // this is to allow the poll item to be collected
    int mynum;
    static int wnum;
    u64 use_count = 0;
    static const uint16_t LOCAL = 1 << 0;
    static const uint16_t POINT_ERROR = 1 << 1;
    static const uint16_t POINT_DISCONNECT = 1 << 2;
    static const uint16_t REMOVE_GAP = 1 << 3;

    IO_Work()
    {
        mynum = ++wnum;
        // std::cout << " IO Work Object " << mynum <<  " created"<< std::endl;
    };

    ~IO_Work()
    {
#ifndef FPS_TEST_MODE
        printf("IO Work Object [%d]\tCount: %ld \tName: %s\t deleted\n", mynum, use_count, work_name.c_str());
#endif
    };
    double tNow;
    std::shared_ptr<cfg::pub_struct> pub_struct;
    std::string work_name;
    int work_id;     // 7 of 9
    int work_group;  // how many in the group
    int device_id;

    int threadId;
    cfg::Register_Types register_type;
    WorkTypes wtype;  // pub set etc
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
    bool local = false;        // bypass server , use or set local data
    bool erase_group = false;  // set true to force erase of a pubgroup
    bool off_by_one = false;
    int thread_num = 0;
    bool data_error = false;
    bool full = false;

    std::string func;
    // IO_Work(IO_Work&& other) noexcept  {};

    // IO_Work& operator=(IO_Work&& other) noexcept {
    //     return *this;
    // };

    ioChannel<std::shared_ptr<IO_Work>>* io_repChan;  // Thread picks up IO_work and processes it

    void clear_bufs()
    {
        memset(buf8, 0, sizeof(buf8));    // Set buf8 to 0
        memset(buf16, 0, sizeof(buf16));  // Set buf16 to 0
    };

    void set_bufs(int num_bufs, uint16_t* buf16_in, uint8_t* buf8_in)
    {
        if (num_bufs > 0)
        {
            if (buf8_in)
            {
                memcpy(buf8, buf8_in, num_bufs * sizeof(uint8_t));
            }
            else
            {
                memset(buf8, 0, sizeof(buf8));  // Set buf8 to 0
            }
            if (buf16_in)
            {
                memcpy(buf16, buf16_in, num_bufs * sizeof(uint16_t));
            }
            else
            {
                memset(buf16, 0, sizeof(buf16));  // Set buf8 to 0
            }
        }
    };
    // Method to set a flag
    void set_flag(size_t ioffset, uint16_t flag)
    {
        size_t myoffset = ioffset - offset;
        if (myoffset < 256)
        {
            flags[myoffset] |= flag;  // Set the flag bit using bitwise OR
        }
    }

    // Method to check if a flag is set
    bool get_flag(size_t ioffset, uint16_t flag) const
    {
        size_t myoffset = ioffset - offset;
        if (myoffset < 256)
        {
            return flags[myoffset] & flag;  // Test the flag bit using bitwise AND
        }
        return false;  // Return false if offset is out of bounds
    }
};

// extern int IO_Work::wnum = 0;

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

struct IO_Fims
{
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
    u32 fims_data_buf_len = FIMS_BUFFER_DEFAULT_LEN;
    u32 data_buf_len = FIMS_BUFFER_DEFAULT_LEN;
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
        is_request = false;
        FPS_INFO_LOG("IO_Fims create tId [%f]", tId);
        fims_data_buf_len = FIMS_BUFFER_DEFAULT_LEN;
        fims_input_buf_len = FIMS_BUFFER_DEFAULT_LEN;
        fims_input_buf = reinterpret_cast<uint8_t*>(calloc(1, fims_data_buf_len));
        // memset(&meta_data,0,sizeof(Meta_Data_Info));
    };

    IO_Fims(int buffer_len)
    {
        fid = io_fims_count++;
        jobs = 0;
        tId = get_time_double();
        is_request = false;
        FPS_INFO_LOG("IO_Fims create tId [%f]", tId);
        fims_data_buf_len = buffer_len;
        fims_input_buf_len = buffer_len;
        fims_input_buf = reinterpret_cast<uint8_t*>(calloc(1, fims_data_buf_len));
        // memset(&meta_data,0,sizeof(Meta_Data_Info));
    };
    ~IO_Fims()
    {
        printf("IO_Fims id: %d\ttId: %f\tuses: %ld\t total data: %ld\n", fid, tId, jobs, total_data);
        if (fims_input_buf)
        {
            free(fims_input_buf);
        }
    };

    bool set_fims_data_buf(unsigned int data_buf_len)
    {
        if (fims_input_buf)
        {
            free(fims_input_buf);
            fims_input_buf = nullptr;
        }

        std::cout << " set buff len to " << data_buf_len << std::endl;
        fims_input_buf = reinterpret_cast<uint8_t*>(malloc(data_buf_len));

        fims_input_buf_len = data_buf_len;  /// sys.fims_dependencies->data_buf_len;
        fims_data_buf_len = data_buf_len;   /// sys.fims_dependencies->data_buf_len;

        return true;
    };

    bool reset_fims_data_buf(unsigned int data_buf_len)
    {
        auto old_buf = fims_input_buf;
        fims_input_buf = reinterpret_cast<uint8_t*>(malloc(data_buf_len));

        if (old_buf)
        {
            std::cout << " set buff len to " << data_buf_len << std::endl;

            auto data_buf_orig = fims_data_buf_len;
            if (data_buf_orig > data_buf_len)
            {
                data_buf_orig = data_buf_len;
            }

            memcpy(fims_input_buf, old_buf, data_buf_orig);
            free(old_buf);
        }

        fims_input_buf_len = data_buf_len;  /// sys.fims_dependencies->data_buf_len;
        fims_data_buf_len = data_buf_len;   /// sys.fims_dependencies->data_buf_len;

        return true;
    };
};

// std::unique_lock<std::mutex> lock(*pg.pmtx);
//             io_thread->modbus_read_timer.start();
//                     io_thread->modbus_read_timer.showNum(ss);
//             io_thread->modbus_read_timer.snap();

struct PubGroup
{
    // Constructor that takes a key and a shared_ptr to IO_Work
    std::string key;
    std::shared_ptr<cfg::pub_struct> pub_struct;
    std::vector<std::shared_ptr<IO_Work>> works;
    int work_group = 0;  // size
    double tNow = 0.0;
    double tDone = 0.0;
    bool done = false;
    bool erase_group = false;

    PubGroup(){};
    ~PubGroup(){};

    PubGroup(std::string& _key, std::shared_ptr<IO_Work> io_work, bool save) : key(_key)
    {
        if (io_work != nullptr)
        {
            reset_group(key, io_work);
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
            pub_struct = io_work->pub_struct;
            work_group = io_work->work_group;
            tNow = io_work->tNow;
            erase_group = io_work->erase_group;
        }
        tDone = 0.0;
        done = false;
    };
};

struct IO_Work_group
{
    std::string key;
    std::vector<std::shared_ptr<IO_Work>> works;
    int work_group;  // size
    double tNow;
    bool done = false;
    bool erase_group = false;
};

struct ThreadControl;

struct IO_Thread
{
public:
    IO_Thread()
    {
        modbus_read_timer.set_label("Modbus Read Timer");
        modbus_write_timer.set_label("Modbus Write Timer");
        is_enabled = true;
        connect_reset = 0;
    };

    alignas(64) modbus_t* ctx = nullptr;
    cfg* myCfg = nullptr;
    // xMain_Thread* main_work;
    int id;

    std::atomic<bool> keep_running;
    std::future<bool> thread_future;  // IO Thread future

    Stats modbus_read_timer;
    Stats modbus_write_timer;

    std::condition_variable* cv;
    std::mutex stat_mtx;
    std::mutex* mtx;
    bool* ready;

    bool wasConnected = false;
    bool hadContext = false;

    int port;
    int tid;
    int jobs;
    int fails = 0;
    int connect_fails = 0;
    int connect_reset = 0;

    std::thread thread;
    ThreadControl* thread_control;
    std::string ip;
    bool connected = false;
    bool connection_timedout = false;

    double connection_timeout;
    double transfer_timeout;
    int transaction_timeout;

    double connect_time;
    double transaction_time;

    double cTime;  // TODO deprecatd
    bool is_local = false;
    bool is_enabled = true;
};

struct ThreadControl
{
    ioChannel<std::shared_ptr<IO_Work>>* io_pollChan;      // Use Channel to send IO-Work to thread
    ioChannel<std::shared_ptr<IO_Work>>* io_setChan;       // Use Channel to send IO-Work to thread
    ioChannel<std::shared_ptr<IO_Work>>* io_responseChan;  // Thread picks up IO_work and processes it
    ioChannel<std::shared_ptr<IO_Work>>* io_poolChan;      // Response channel returns io_work to the pool

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
    double tConnect = 0.0;  // if connection fails set this to tNow plus 1 secong

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
        for (auto& tp : ioThreadPool)
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

/**
 * @brief Determines if it is okay to connect based on the current time.
 *
 * This function checks whether the current time `tNow` is greater than or equal to the
 * connection time `tc.tConnect`. It uses a mutex to ensure thread safety while accessing
 * the `tConnect` member of the `ThreadControl` struct.
 *
 * @param tc A reference to a `ThreadControl` object containing the connection time and mutex.
 * @param tNow The current time to compare against the connection time.
 * @return `true` if `tNow` is greater than or equal to `tc.tConnect`, `false` otherwise.
 */
bool OkToConnect(ThreadControl& tc, double tNow);

/**
 * @brief Delays the connection by updating the connection time based on the current time
 * and the reconnect delay from the configuration.
 *
 * This function calculates the new connection time `tc.tConnect` by adding the current time
 * `tNow` to the reconnect delay specified in the configuration `myCfg.reconnect_delay`. It
 * uses a mutex to ensure thread safety while updating the `tConnect` member of the `ThreadControl`
 * struct.
 *
 * @param myCfg A reference to a `cfg` object containing the reconnect delay.
 * @param tc A reference to a `ThreadControl` object to update the connection time.
 */
void DelayConnect(struct cfg& myCfg, ThreadControl& tc);

/**
 * @brief Disconnects a client and updates relevant information.
 *
 * This function disconnects the client associated with the given `io_thread`. If the client is
 * currently connected, it emits a disconnect event, decrements the number of connected threads,
 * and updates the `connected` flag of the `io_thread` to `false`.
 *
 * @param myCfg The configuration containing connection details.
 * @param tc The thread control object managing thread-related information.
 * @param io_thread The IO thread to disconnect.
 */
void handleDisconnect(struct cfg& myCfg, ThreadControl& tc, std::shared_ptr<IO_Thread> io_thread);

/**
 * @brief Connects a client and updates relevant information.
 *
 * This function connects the client associated with the given `io_thread`. If the client is
 * currently disconnected, it emits a connect event, increments the number of connected threads,
 * and updates the `connected` flag of the `io_thread` to `true`.
 *
 * @param myCfg The configuration containing connection details.
 * @param tc The thread control object managing thread-related information.
 * @param io_thread The IO thread to connect.
 */
void handleConnect(struct cfg& myCfg, ThreadControl& tc, std::shared_ptr<IO_Thread> io_thread);

/**
 * @brief Gets the number of connected threads from the ThreadControl object.
 *
 * This function retrieves the number of connected threads from the ThreadControl object
 * `tc` by locking the `connect_mutex` to ensure thread safety while accessing the
 * `num_connected_threads` member.
 *
 * @param myCfg A reference to a cfg object (not used in this function).
 * @param tc A reference to a ThreadControl object containing the number of connected threads.
 * @return An integer representing the number of connected threads.
 */
int GetNumThreads(struct cfg& myCfg, ThreadControl& tc);

/**
 * @brief Gets the number of connected threads from the global ThreadControl object.
 *
 * This function retrieves the number of connected threads from the global ThreadControl object
 * `threadControl` by locking the `connect_mutex` to ensure thread safety while accessing the
 * `num_connected_threads` member.
 *
 * @param myCfg A pointer to a cfg object (not used in this function).
 * @return An integer representing the number of connected threads.
 */
int GetNumThreads(struct cfg* myCfg);

/**
 * @brief Formats thread connection information into a JSON-like string.
 *
 * This function formats thread connection information from the global `threadControl.ioThreadPool`
 * into a JSON-like string. If `include_key` is true, it includes a key "thread_connection_info"
 * at the beginning of the string. Each thread's information includes its ID, connection status,
 * IP address or serial device name, port (if applicable), time to connect, number of jobs, number
 * of failures, modbus read times, and modbus write times.
 *
 * @param ss The stringstream to store the formatted information.
 * @param include_key Flag to indicate whether to include the "thread_connection_info" key.
 */
void formatThreadConnectionInfo(std::stringstream& ss, bool include_key);

/**
 * @brief Handles errors encountered by an IO thread and attempts to recover the connection.
 *
 * This function processes errors encountered by an IO thread during its operation.
 * If the thread was previously connected, it attempts to reconnect based on the error code.
 * It updates the connection status and logs relevant information about the error and recovery attempts.
 *
 * @param myCfg Configuration structure containing the necessary settings for the operation.
 * @param io_thread The IO thread that encountered an error.
 * @param io_work The IO work associated with the thread, containing error details and connection info.
 * @param io_done Flag indicating whether IO operations are complete.
 * @param io_tries The number of attempts made to perform the IO operation.
 * @param max_io_tries The maximum number of allowed attempts for the IO operation.
 * @param debug Flag indicating whether debug information should be printed.
 */
void handleThreadError(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work,
                       bool& io_done, int& io_tries, int& max_io_tries, bool debug);

/**
 * @brief Writes data to local Modbus registers from the provided buffer.
 *
 * This function writes data from the buffer in the `io_work` object to the
 * local Modbus registers defined in `io_points`. The data is copied while
 * holding a lock on each `io_point` to ensure thread safety.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_work Shared pointer to the `IO_Work` object containing the data
 *                and `io_points` to be written to.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void local_write_registers(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Reads data from local Modbus registers into the provided buffer.
 *
 * This function reads data from the local Modbus registers defined in `io_points` and
 * copies the data into the buffer in the `io_work` object. The data is copied while
 * holding a lock on each `io_point` to ensure thread safety.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_work Shared pointer to the `IO_Work` object containing the buffer
 *                and `io_points` to be read from.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void local_read_registers(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Writes data to local Modbus bit registers from the provided buffer.
 *
 * This function writes data from the buffer in the `io_work` object to the
 * local Modbus bit registers defined in `io_points`. The data is copied while
 * holding a lock on each `io_point` to ensure thread safety.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_work Shared pointer to the `IO_Work` object containing the data
 *                and `io_points` to be written to.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void local_write_bits(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Reads data from local Modbus bit registers into the provided buffer.
 *
 * This function reads data from the local Modbus bit registers defined in `io_points` and
 * copies the data into the buffer in the `io_work` object. The data is copied while
 * holding a lock on each `io_point` to ensure thread safety.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_work Shared pointer to the `IO_Work` object containing the buffer
 *                and `io_points` to be read from.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void local_read_bits(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Writes data to local Modbus registers or bits based on the register type.
 *
 * This function determines the type of registers or bits to write based on the
 * `register_type` in the `io_work` object. It delegates the writing task to either
 * `local_write_registers` or `local_write_bits` depending on the register type.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_work Shared pointer to the `IO_Work` object containing the data
 *                and register type information.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void local_write_work(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Reads data from local Modbus registers or bits based on the register type.
 *
 * This function determines the type of registers or bits to read based on the
 * `register_type` in the `io_work` object. It delegates the reading task to either
 * `local_read_registers` or `local_read_bits` depending on the register type.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_work Shared pointer to the `IO_Work` object containing the data
 *                and register type information.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void local_read_work(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Checks and handles lost connections based on the error code.
 *
 * This function determines if a connection has been lost based on the provided error code.
 * It handles timeouts for RTU connections and disconnects for specific errors such as
 * EPIPE and EINVAL. If a connection is determined to be lost, appropriate logging and
 * cleanup actions are taken.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and error information.
 * @param func_name Name of the function where the error occurred.
 * @param err The error code to be checked.
 * @return true if the connection is lost, false otherwise.
 */
bool has_lost_connection(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work,
                         const char* func_name, int err);

/**
 * @brief Handles connection reset errors for a specific Modbus IO point.
 *
 * This function processes connection reset errors (`ECONNRESET`) for a specific Modbus IO point.
 * It increments the connection reset counter for the IO thread and logs the error.
 * If the connection reset count exceeds a threshold, the function disables the IO thread and emits an event.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_point Shared pointer to the io_point_struct representing the specific IO point.
 * @return `true` if the error code is `ECONNRESET`, `false` otherwise.
 */
bool handle_point_connection_reset(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread,
                                   std::shared_ptr<struct cfg::io_point_struct> io_point);

/**
 * @brief Handles common errors for a specific Modbus IO point.
 *
 * This function processes common errors that occur for a specific Modbus IO point during an I/O operation.
 * It checks if the error code is one of the common error codes and logs the error if it is the first occurrence.
 * The function sets the `data_error` flag in the `io_work` object and returns `true` if the error is one of the common
 * errors.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and error information.
 * @param io_point Shared pointer to the io_point_struct representing the specific IO point.
 * @return `true` if the error code is one of the common errors, `false` otherwise.
 */
bool handle_point_common_errors(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread,
                                std::shared_ptr<IO_Work> io_work,
                                std::shared_ptr<struct cfg::io_point_struct> io_point);

/**
 * @brief Handles specific Modbus point errors during an I/O operation.
 *
 * This function processes errors that occur for a specific Modbus IO point during an I/O operation.
 * If the `data_error` flag is already set in the `io_work` object, the function returns immediately.
 * For specific error codes, additional handling is performed, such as calling `handleEMBBADEXC` for the
 * `EMBBADEXC` error code.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and error information.
 * @param io_point Shared pointer to the io_point_struct representing the specific IO point.
 * @param func_name Name of the function where the error occurred.
 * @param err The error code to be handled.
 */
void handle_modbus_point_errors(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread,
                                std::shared_ptr<IO_Work> io_work, std::shared_ptr<struct cfg::io_point_struct> io_point,
                                const char* func_name, int err);

/**
 * @brief Automatically disables an IO point based on the error and configuration settings.
 *
 * This function handles the automatic disabling of an IO point based on the error code and
 * configuration settings. If auto-disable is enabled in the configuration and the error code
 * is not in the list of errors that should not trigger auto-disable, the function sets the
 * appropriate flags and logs the error. It also manages the removal of gaps and disconnection
 * of the IO point if necessary.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and error information.
 * @param io_point Shared pointer to the io_point_struct representing the specific IO point.
 * @param func_name Name of the function where the error occurred.
 * @param err The error code to be handled.
 */
void handle_point_auto_disable(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread,
                               std::shared_ptr<IO_Work> io_work, std::shared_ptr<struct cfg::io_point_struct> io_point,
                               const char* func_name, int err);

/**
 * @brief Handles errors for a specific IO point within a Modbus connection.
 *
 * This function processes errors that occur for a specific IO point during a Modbus operation.
 * It handles different error codes, updates the connection state, logs errors, and potentially
 * disables the IO thread or IO point based on the error type and configuration settings.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and error information.
 * @param io_point Shared pointer to the io_point_struct representing the specific IO point.
 * @param func_name Name of the function where the error occurred.
 * @param err The error code to be handled.
 */
void handle_point_error(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work,
                        std::shared_ptr<struct cfg::io_point_struct> io_point, const char* func_name, int err);

/**
 * @brief Reads Modbus data based on the configuration and updates the IO work object.
 *
 * This function reads Modbus data based on the register type specified in the `io_work` object.
 * It handles different types of registers (holding, coil, input, and discrete input) and updates the
 * `io_work` object with the results. If an error occurs during the read operation, it checks for a lost
 * connection and updates the `io_work` object accordingly.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_tries Reference to the number of I/O attempts.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and buffer for the read data.
 * @param debug Boolean flag indicating if debug information should be logged.
 * @return The result of the Modbus read operation (0 for success, negative for error).
 */
int read_modbus_io_work(struct cfg& myCfg, int& io_tries, std::shared_ptr<IO_Thread> io_thread,
                        std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Reads Modbus data points based on the configuration and updates the IO work object.
 *
 * This function reads Modbus data points for each IO point in the `io_work` object.
 * It handles different types of registers (holding, coil, input, and discrete input) and updates the
 * `io_work` object with the results. If an error occurs during the read operation, it checks for a lost
 * connection, handles the error, and updates the `io_work` object accordingly.
 *
 * Called if we have a connection and the first attempt to read points fails.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_tries Reference to the number of I/O attempts.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and buffer for the read data.
 * @param io_done Reference to a boolean flag indicating if the I/O operation is done.
 * @param debug Boolean flag indicating if debug information should be logged.
 * @return The total number of successful reads.
 */
int read_modbus_io_points(struct cfg& myCfg, int& io_tries, std::shared_ptr<IO_Thread> io_thread,
                          std::shared_ptr<IO_Work> io_work, bool& io_done, bool debug);

/**
 * @brief Writes Modbus data based on the configuration and updates the IO work object.
 *
 * This function writes Modbus data based on the register type specified in the `io_work` object.
 * It handles different types of registers (holding and coil) and updates the `io_work` object with the results.
 * If an error occurs during the write operation, it checks for a lost connection and updates the `io_work` object
 * accordingly.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_tries Reference to the number of I/O attempts.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and buffer for the write data.
 * @param debug Boolean flag indicating if debug information should be logged.
 * @return The result of the Modbus write operation (0 for success, negative for error).
 */
int write_modbus_io_work(struct cfg& myCfg, int& io_tries, std::shared_ptr<IO_Thread> io_thread,
                         std::shared_ptr<IO_Work> io_work, bool debug);

/**
 * @brief Writes Modbus data points based on the configuration and updates the IO work object.
 *
 * This function writes Modbus data points for each IO point in the `io_work` object.
 * It handles different types of registers (holding and coil) and updates the `io_work` object with the results.
 * If an error occurs during the write operation, it checks for a lost connection, handles the error, and updates the
 * `io_work` object accordingly.
 *
 *  Called if we have a connection and the first attempt to write points fails.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_tries Reference to the number of I/O attempts.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and buffer for the write data.
 * @param io_done Reference to a boolean flag indicating if the I/O operation is done.
 * @param debug Boolean flag indicating if debug information should be logged.
 * @return The total number of successful writes.
 */
int write_modbus_io_points(struct cfg& myCfg, int& io_tries, std::shared_ptr<IO_Thread> io_thread,
                           std::shared_ptr<IO_Work> io_work, bool& io_done, bool debug);

/**
 * @brief Executes Modbus I/O work for a given thread. When the thread first starts, it tries to connect.
 *
 * This function executes Modbus I/O work (read or write) for a given thread and updates the IO work object.
 * It handles setting up the Modbus context, performing the I/O operations, retrying on failure, and handling errors.
 * If an error occurs, it checks for a lost connection, handles the error, and retries the operation if necessary.
 *
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 * @param io_work Shared pointer to the IO_Work object containing the work details and buffer for the I/O data.
 * @param debug Boolean flag indicating if debug information should be logged.
 */
void runThreadWork(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work,
                   bool debug);

/**
 * @brief Main function for the IO thread, handling Modbus communication and various signals.
 *
 * This function serves as the main loop for the IO thread, managing Modbus communication and responding to signals.
 * It handles connecting to Modbus devices, processing I/O work, and managing thread states based on received signals.
 * The function logs important events and errors, and ensures proper shutdown of the thread and Modbus connection.
 *
 * @param control Reference to the ThreadControl object managing the thread state.
 * @param myCfg Reference to the configuration object.
 * @param io_thread Shared pointer to the IO_Thread object representing the current thread.
 */
void ioThreadFunc(ThreadControl& control, struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread);

/**
 * @brief Processes the callback for a group of I/O operations, handling publication, setting, and getting data.
 *
 * This function processes a group of I/O operations represented by `pub_group`. It handles different types of
 * operations based on the key prefix: publication, setting, or getting data. The function decodes the I/O work,
 * stores raw data, and sends the results to the appropriate destination (typically producing a fims output message).
 *
 * @param pub_group Reference to the PubGroup structure containing the group of I/O operations.
 * @param myCfg Reference to the configuration object.
 */
void processGroupCallback(struct PubGroup& pub_group, struct cfg& myCfg);

/**
 * @brief Discards the work in the given publication group, resetting their state and potentially logging the operation.
 *
 * This function processes each work item in the provided publication group, resetting their error states and returning
 * them to the I/O pool. If a publication structure is associated with the work items, it updates its completion time.
 * Additionally, it can log debug information about the operation depending on the configuration.
 *
 * @param pub_group Reference to the publication group to be discarded.
 * @param myCfg Reference to the configuration object.
 * @param ok Indicates whether the operation was successful, influencing the debug logging output.
 */
void discardGroupCallback(struct PubGroup& pub_group, struct cfg& myCfg, bool ok);

/**
 * @brief Checks if a publication group with the specified key exists, and creates it if it does not.
 *
 * This function checks if a publication group identified by the given key exists in the `pubGroups` map.
 * If the key is not found, it creates a new publication group with the key and associates it with the provided I/O
 * work.
 *
 * TODO: Find a way to look for an empty key
 *
 * @param key The key identifying the publication group.
 * @param io_work Shared pointer to the I/O work to be associated with the publication group if it is created.
 * @return Always returns true.
 */
bool check_pubgroup_key(std::string key, std::shared_ptr<IO_Work> io_work);

/**
 * @brief Displays the pubGroups at exit.
 *
 * This function iterates over the pubGroups container and prints the pubGroup key, tNow, and tDone values for each
 * entry.
 *
 * @return true if the operation was successful, false otherwise.
 */
bool show_pubgroup();

/**
 * @brief Retrieves a pointer to the PubGroup associated with the given IO_Work.
 *
 * This function fetches the PubGroup associated with the specified IO_Work. If the
 * PubGroup does not already exist, it creates a new one using the work name as the key.
 *
 * @param io_work Shared pointer to the IO_Work for which the PubGroup is being retrieved.
 * @return Pointer to the PubGroup associated with the given IO_Work.
 */
PubGroup* get_pubgroup(std::shared_ptr<IO_Work> io_work);

/**
 * @brief Retrieves a pointer to the PubGroup associated with the given key.
 *
 * This function fetches the PubGroup associated with the specified key. If the
 * PubGroup does not already exist, it creates a new one using the key.
 *
 * @param key Reference to the string key for which the PubGroup is being retrieved.
 * @return Pointer to the PubGroup associated with the given key.
 */
PubGroup* get_pubgroup(std::string& key);

/**
 * @brief Processes the response work item, handling errors, managing connections, and updating publication groups.
 *
 * This function handles the processing of a response work item. It checks the state of I/O points, updates their
 * connection status, processes local read/write operations, and manages publication groups. It also sends replies
 * to the FIMS gateway if necessary.
 *
 * This is the main receiver of the io_work objects. They all come back to a common thread to make sure we have
 * thread-safe access to the data points. The local read / writes also occur here.
 *
 * @param io_work Shared pointer to the I/O work item to be processed.
 * @param myCfg Reference to the configuration object.
 */
void processIOWorkResponse(std::shared_ptr<IO_Work> io_work, struct cfg& myCfg);

/**
 * @brief Function to handle response processing in a separate thread.
 *
 * This function runs in a loop while the response thread is active. It receives
 * IO_Work objects from the response channel, processes them, and updates response statistics.
 *
 * @param control Reference to the ThreadControl object managing the thread state and statistics.
 * @param myCfg Reference to the configuration object.
 */
void processIOWorkResponseThread(ThreadControl& control, struct cfg& myCfg);

/**
 * @brief Stashes an IO_Work object by sending it back to the io_poolChan channel.
 *
 * This function moves the given `io_work` shared pointer into the `io_poolChan` channel.
 * This is used for managing the lifecycle of IO_Work objects, typically stashing them
 * for later retrieval or processing.
 *
 * @param io_work A shared pointer to the IO_Work object that needs to be stashed.
 *
 * @note The function uses `std::move` to transfer ownership of the `io_work` object.
 */
void stashWork(std::shared_ptr<IO_Work> io_work);

/**
 * @brief Creates and initializes a new IO_Work object.
 *
 * This function creates and initializes an IO_Work object, either by reusing an available one from the pool or
 * by creating a new one if the pool is empty. The function sets various fields of the IO_Work object based on
 * the input parameters.
 *
 * @param component Pointer to the component structure associated with the work.
 * @param register_type The type of register involved in the work.
 * @param device_id The ID of the device for which the work is being created.
 * @param offset The register offset for the work.
 * @param off_by_one Flag indicating whether to adjust for off-by-one errors.
 * @param num_regs The number of registers involved in the work.
 * @param u16bufs Pointer to the buffer holding 16-bit register values.
 * @param u8bufs Pointer to the buffer holding 8-bit register values.
 * @param wtype The type of work (e.g., Set, Get, Poll).
 * @return A shared pointer to the newly created or reused IO_Work object.
 * @retval nullptr if a new IO_Work object could not be obtained from the pool within the maximum wait time.
 */
std::shared_ptr<IO_Work> make_work(cfg::component_struct* component, cfg::Register_Types register_type, int device_id,
                                   int offset, bool off_by_one, int num_regs, uint16_t* u16bufs, uint8_t* u8bufs,
                                   WorkTypes wtype);

/**
 * @brief Sends a signal to the I/O thread channel to indicate a test action.
 *
 * This function sends the signal `2` to the `io_threadChan`, which is used to trigger a specific
 * test-related action in the I/O thread. It always returns `true` indicating the signal was sent.
 *
 * @return true Always returns true indicating the signal was sent.
 */
bool testThread();

/**
 * @brief Sends a signal to the I/O thread channel to indicate termination.
 *
 * This function sends the signal `3` to the `io_threadChan`, which is used to trigger the termination
 * of the I/O thread. It always returns `true` indicating the signal was sent.
 *
 * @return true Always returns true indicating the signal was sent.
 */
bool killThread();

/**
 * @brief Sends I/O work to the appropriate polling channel and signals the thread.
 *
 * This function sends the given `io_work` to either the local or remote polling channel
 * based on the `local` attribute of `io_work`. It then sends a signal to the corresponding
 * thread channel to indicate that new work is available.
 *
 * @param io_work A shared pointer to an `IO_Work` object that represents the I/O work to be processed.
 * @return true Always returns true indicating the operation was initiated.
 *
 * @note This function might be enhanced to return a status indicating success or failure in the future.
 */
bool pollWork(std::shared_ptr<IO_Work> io_work);

/**
 * @brief Sends I/O work to the appropriate setting channel and signals the thread.
 *
 * This function sends the given `io_work` to either the local or remote setting channel
 * based on the `local` attribute of `io_work`. It then sends a signal to the corresponding
 * thread channel to indicate that new work is available.
 *
 * @param io_work A shared pointer to an `IO_Work` object that represents the I/O work to be processed.
 * @return true Always returns true indicating the operation was initiated.
 */
bool setWork(std::shared_ptr<IO_Work> io_work);

/**
 * @brief Delays the execution of setWork by a specified duration.
 *
 * This function introduces a delay specified in seconds before calling the `setWork` function.
 * It puts the current thread to sleep for the duration of the delay.
 *
 * @param io_work A shared pointer to an `IO_Work` object that represents the I/O work to be processed.
 * @param delay_seconds The duration of the delay in seconds before the `io_work` is processed.
 * @return true Always returns true indicating the operation was initiated after the delay.
 *
 * @note This function might be enhanced to return a status indicating success or failure in the future.
 */
bool setWorkWithDelay(std::shared_ptr<IO_Work> io_work, double delay_seconds);

/**
 * @brief Delays the execution of pollWork by a specified duration.
 *
 * This function introduces a delay specified in seconds before calling the `pollWork` function.
 * It puts the current thread to sleep for the duration of the delay.
 *
 * @param io_work A shared pointer to an `IO_Work` object that represents the I/O work to be processed.
 * @param delay_seconds The duration of the delay in seconds before the `io_work` is processed.
 * @return true Always returns true indicating the operation was initiated after the delay.
 *
 * @note This function might be enhanced to return a status indicating success or failure in the future.
 */
bool pollWorkWithDelay(std::shared_ptr<IO_Work> io_work, double delay_seconds);

/**
 * @brief Starts the response processing thread.
 *
 * This function creates a new thread that runs the `processIOWorkResponseThread` function.
 * The thread is responsible for processing I/O work responses.
 *
 * @param myCfg A reference to the configuration structure used by the thread.
 * @return true Always returns true indicating the thread was successfully started.
 *
 * @note This function might be enhanced to return a status indicating success or failure in the future.
 */
bool startProcessIOWorkResponseThread(struct cfg& myCfg);

/**
 * @brief Creates and initializes a new IO_Thread object.
 *
 * This function creates a shared pointer to an `IO_Thread` object and initializes it with the given parameters.
 * The thread ID, IP address, port, connection timeout, and transfer timeout are set according to the provided
 * arguments.
 *
 * @param idx The thread ID to assign to the new `IO_Thread`.
 * @param ip A pointer to a C-string representing the IP address to assign to the new `IO_Thread`. If `nullptr`, the IP
 * address is left empty.
 * @param port The port number to assign to the new `IO_Thread`.
 * @param connection_timeout The connection timeout value to assign to the new `IO_Thread`.
 * @param transfer_timeout The transfer timeout value to assign to the new `IO_Thread`.
 * @param myCfg A reference to the configuration structure used by the `IO_Thread`.
 * @return std::shared_ptr<IO_Thread> A shared pointer to the newly created and initialized `IO_Thread`.
 */
std::shared_ptr<IO_Thread> make_IO_Thread(int idx, const char* ip, int port, double connection_timeout,
                                          double transfer_timeout, struct cfg& myCfg);

/**
 * @brief Starts multiple I/O threads and a response thread for handling I/O operations.
 *
 * This function initializes and starts a specified number of I/O threads for handling
 * Modbus communication and a single response thread for processing I/O responses.
 * It also starts a local thread which does not require a connection.
 *
 * @param num_threads The number of I/O threads to start.
 * @param ip The IP address for the Modbus connection.
 * @param port The port number for the Modbus connection.
 * @param connection_timeout The timeout duration for establishing a connection.
 * @param transfer_timeout The timeout duration for data transfer operations.
 * @param myCfg A reference to the configuration struct.
 * @return true if the threads are started successfully, false otherwise.
 */
bool StartThreads(int num_threads, const char* ip, int port, double connection_timeout, double transfer_timeout,
                  struct cfg& myCfg);

/**
 * @brief Starts multiple I/O threads and a response thread for handling I/O operations.
 *
 * This function initializes and starts a specified number of I/O threads for handling
 * Modbus communication and a single response thread for processing I/O responses.
 * It also starts a local thread which does not require a connection.
 *
 * @param myCfg The configuration used to determine max_num_connections, ip_address, port, connection_timeout, and
 * transfer_timeout.
 * @param debug Flag indicating whether to enable debug logging.
 * @return true if the threads are started successfully, false otherwise.
 */
bool StartThreads(struct cfg& myCfg, bool debug);

/**
 * @brief Stops all I/O threads and the response thread.
 *
 * This function stops all currently running I/O threads and the response thread.
 * It first calls the `stopThreads` method on the global `threadControl` object to signal
 * all threads to stop. Then it joins the response thread to ensure it has finished execution.
 * The function logs messages indicating the status of the thread stopping process.
 *
 * @param myCfg Reference to the configuration structure.
 * @param debug Boolean flag indicating whether to enable debug logging.
 * @return Returns true indicating that the threads were stopped successfully.
 */
bool StopThreads(struct cfg& myCfg, bool debug);

/**
 * @brief Waits for a socket to become ready for writing.
 *
 * This function waits for a Modbus socket to become ready for writing within a specified timeout period.
 * It uses the `select` system call to monitor the socket and determine if it is ready for writing.
 * If the socket is ready within the timeout period, the function returns 0. Otherwise, it returns -1.
 *
 * @param ctx Pointer to the Modbus context.
 * @param timeout_sec Timeout period in seconds to wait for the socket to become ready.
 * @return Returns 0 if the socket is ready for writing within the timeout period, or -1 if the socket is invalid,
 *         the connection failed, or the timeout period expired.
 */
int wait_socket_ready(modbus_t* ctx, int timeout_sec);

/**
 * @brief Checks if the Modbus socket is alive by monitoring for readability or exceptions.
 *
 * This function uses the select system call to monitor the specified socket
 * for readability and exceptions within the provided timeout period. It returns
 * the result of the select call, indicating whether the socket is alive and readable.
 *
 * @param io_thread A shared pointer to the IO_Thread object containing the Modbus context.
 * @param timeout_sec The timeout period in seconds.
 * @param timeout_usec The timeout period in microseconds.
 * @return int Returns a positive value if the socket is readable, 0 if it timed out,
 * or -1 if the socket is invalid.
 */
int check_socket_alive(std::shared_ptr<IO_Thread> io_thread, int timeout_sec, int timeout_usec);

/**
 * @brief Attempts a fast reconnection for a Modbus thread after a timeout.
 *
 * This function handles the reconnection process for a Modbus thread, either RTU or TCP, and updates the thread's
 * context accordingly. It performs the following steps:
 * 1. Deletes the old context (if any).
 * 2. Creates a new Modbus context based on the connection type (RTU or TCP).
 * 3. Sets the response timeout for the new context.
 * 4. Attempts to connect to the Modbus server.
 * 5. Handles connection failures and emits relevant events.
 * 6. Updates the thread's context and connection status on successful connection.
 *
 * This function is meant to stop the system from building up a whole bunch of jobs during a full reconnect.
 *
 * @param myCfg Reference to the configuration structure.
 * @param io_thread Shared pointer to the IO_Thread structure.
 * @param debug Boolean flag indicating whether debugging information should be printed.
 * @return The time taken to establish the connection, or 0.0 if the connection failed.
 */
double FastReconnectForThread(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug);

/**
 * @brief Sets up the Modbus connection for a given IO thread.
 *
 * This function establishes a Modbus connection for a specified IO thread. Depending on the configuration, it creates
 * either an RTU or TCP Modbus context, attempts to connect, and sets the response timeout and error recovery mode.
 *
 * @param myCfg The configuration structure containing connection details and parameters.
 * @param io_thread A shared pointer to the IO_Thread object which holds the context and connection details.
 * @param debug A boolean flag to indicate whether debug information should be printed.
 * @return The time taken to establish the connection in seconds, or 0.0 if the connection failed.
 *
 * @note This function relies on several other functions to handle specific aspects of the connection process,
 *       such as `OkToConnect`, `DelayConnect`, `handleDisconnect`, and `handleConnect`.
 */
double SetupModbusForThread(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug);

/**
 * @brief Closes the Modbus connection for a given IO thread.
 *
 * This function closes the Modbus connection associated with the specified IO thread.
 * If the connection is an RTU connection, it logs a message indicating the disconnection
 * from the RTU device. If it is a TCP connection, it logs a message indicating the disconnection
 * from the TCP device and port.
 *
 * @param io_thread A shared pointer to the IO_Thread object whose Modbus connection needs to be closed.
 * @param debug A boolean indicating whether debug mode is enabled.
 * @return true if the Modbus connection was successfully closed.
 */
bool CloseModbusForThread(std::shared_ptr<IO_Thread> io_thread, bool debug);

#endif
