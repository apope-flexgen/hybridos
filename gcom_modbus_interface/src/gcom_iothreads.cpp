// gcom modbus thread code
// p. wilshire
// 11_22_2023
// self review 11_29_2023

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
#include <tuple>
#include <functional>
#include <algorithm>
#include <random>
#include <errno.h>  //For errno - the error number
#include <cxxabi.h>
#include <iostream>
#include <stdexcept>
#include <cfenv>  // For standard floating point functions
#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>

#ifndef FPS_TEST_MODE
#include <modbus/modbus.h>
#else
#include "mock_modbus.h"
#endif
#include "logger/logger.h"
#include "gcom_config.h"
#include "gcom_iothread.h"
#include "gcom_modbus_decode.h"
#include "gcom_fims.h"
#include "gcom_timer.h"
#include "gcom_utils.h"

int IO_Work::wnum = 0;

void addTimeStamp(std::stringstream& ss);
int check_socket_alive(std::shared_ptr<IO_Thread> io_thread, int timeout_sec, int timeout_usec);

using namespace std::chrono_literals;

struct ThreadControl;

std::mutex io_output_mutex;
extern std::mutex logger_mutex;

struct IO_Work;
// Global Channel Definitions
ioChannel<std::shared_ptr<IO_Work>> io_pollChan;       // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_setChan;        // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_localpollChan;  // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_localsetChan;   // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_responseChan;   // Thread picks up IO_work and processes it
ioChannel<std::shared_ptr<IO_Work>> io_poolChan;       // Response channel returns io_work to the pool
ioChannel<int> io_threadChan;                          // Thread Control
ioChannel<int> io_localthreadChan;                     // Thread Control

ThreadControl threadControl;

std::map<std::string, PubGroup> pubGroups;

int num_work = 0;
// TODO make this a config option
// if we reach this limt we have to  wait for io_work objects to be released.
// the socket timeouts should terminate all send / get requests.
// these only build up if the network is lagging

int max_work = 20000;
int max_work_wait = 5000;

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
bool OkToConnect(ThreadControl& tc, double tNow)
{
    std::lock_guard<std::mutex> lock2(tc.connect_mutex);
    if (tNow < tc.tConnect)
        return false;
    return true;
}

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
void DelayConnect(struct cfg& myCfg, ThreadControl& tc)
{
    double tNow = get_time_double();
    std::lock_guard<std::mutex> lock2(tc.connect_mutex);
    tc.tConnect = tNow + myCfg.reconnect_delay;
}

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
void handleDisconnect(struct cfg& myCfg, ThreadControl& tc, std::shared_ptr<IO_Thread> io_thread)
{
    // emit_event
    if (io_thread && io_thread->connected)
    {
        char message[1024];
        if (io_thread->myCfg && io_thread->myCfg->connection.is_RTU)
        {
            snprintf(message, 1024, "Disconnecting Modbus RTU client [%s]. Thread ID: %d. Device: [%s]",
                     io_thread->myCfg->connection.name.c_str(), io_thread->tid,
                     io_thread->myCfg->connection.device_name.c_str());
        }
        else if (io_thread->myCfg)
        {
            snprintf(message, 1024, "Disconnecting Modbus TCP client [%s]. Thread ID: %d. Host: [%s], Port: [%d]",
                     io_thread->myCfg->connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
        }

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);

        tc.num_connected_threads--;
        io_thread->connected = false;
    }
}

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
void handleConnect(struct cfg& myCfg, ThreadControl& tc, std::shared_ptr<IO_Thread> io_thread)
{
    if (!io_thread->connected)
    {
        tc.num_connected_threads++;
        io_thread->connected = true;
        char message[1024];
        if (myCfg.connection.is_RTU)
        {
            snprintf(message, 1024, "Modbus Client RTU  [%s] Thread id %d connecting to device [%s]",
                     myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str());
        }
        else
        {
            snprintf(message, 1024, "Modbus Client TCP [%s] Thread id %d connecting to[%s] on port [%d]",
                     myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
        }

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
    }
}

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
int GetNumThreads(struct cfg& myCfg, ThreadControl& tc)
{
    std::lock_guard<std::mutex> lock2(tc.connect_mutex);
    return tc.num_connected_threads;
}

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
int GetNumThreads(struct cfg* myCfg)
{
    std::lock_guard<std::mutex> lock2(threadControl.connect_mutex);
    return threadControl.num_connected_threads;
}

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
void formatThreadConnectionInfo(std::stringstream& ss, bool include_key)
{
    if (include_key)
    {
        ss << "\"thread_connection_info\": ";
    }
    ss << "[";
    bool first = true;
    for (auto& io_thread : threadControl.ioThreadPool)
    {
        // if(io_thread->is_local)
        //     continue;
        // if (io_thread->ctx != nullptr)
        {
            if (!first)
            {
                ss << ", ";
            }
            else
            {
                first = false;
            }
            ss << "{";
            ss << "\"id\":" << io_thread->tid << "";

            if (!io_thread->is_local)
            {
                ss << ",\"connected\":" << (io_thread->connected ? "true" : "false") << ",";
                if (io_thread->myCfg->connection.is_RTU)
                {
                    // snprintf(message, 1024, "Modbus Client [%s] Thread id %d disconnecting from [%s]",
                    // io_thread->myCfg->connection.name.c_str()
                    //                                                                         , io_thread->tid
                    //                                                                         ,
                    //                                                                         io_thread->myCfg->connection.device_name.c_str());
                    ss << "\"serial_device\": \"" << io_thread->myCfg->connection.device_name.c_str() << "\",";
                }
                else
                {
                    ss << "\"ip_address\": \"" << io_thread->ip.c_str() << "\",";
                    ss << "\"port\":" << io_thread->port << ",";
                }
                ss << "\"time_to_connect\":\"" << io_thread->connect_time << " ms\",";
                {
                    std::unique_lock<std::mutex> lock(io_thread->stat_mtx);

                    ss << "\"modbus_read_times\":";
                    io_thread->modbus_read_timer.showNum(ss);
                    ss << ",\"modbus_write_times\":";
                    io_thread->modbus_write_timer.showNum(ss);
                }
            }
            ss << ",\"num_jobs\":" << io_thread->jobs << ",";
            ss << "\"num_fails\":" << io_thread->fails << "";
            ss << "}";
        }
    }
    ss << "]";
}

// Specific handling for EBADF error
void handleEBADF(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                 struct cfg& myCfg)
{
    if (io_thread->ctx)
    {
        FPS_ERROR_LOG("Bad file descriptor (EBADF) error for modbus client thread #%d after %d tries", io_thread->tid,
                      io_tries);
    }
    handleDisconnect(myCfg, threadControl, io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Specific handling for EBADF error
void handleECONNRESET(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                      struct cfg& myCfg)
{
    if (io_thread->ctx)
    {
        FPS_ERROR_LOG("Connection reset by peer (ECONNRESET) error for modbus client thread #%d after %d tries",
                      io_thread->tid, io_tries);
    }
    handleDisconnect(myCfg, threadControl, io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Specific handling for ETIMEDOUT error
void handleETIMEDOUT(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                     struct cfg& myCfg)
{
    if (io_thread->ctx)
    {
        FPS_ERROR_LOG("Connection timed out (ETIMEOUT) error for modbus client thread #%d after %d tries",
                      io_thread->tid, io_tries);
    }
    if (!myCfg.connection.is_RTU)  // no need to disconnect if timed out
    {
        handleDisconnect(myCfg, threadControl, io_thread);
    }

    // fast retry on timed out
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// TODO use the proper connect function
//  115
void handleEINPROGRESS(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                       struct cfg& myCfg)
{
    if (io_thread->connect_fails < 5)
    {
        io_thread->connect_fails++;
        FPS_ERROR_LOG("Operation now in progress (EINPROGRESS) error for modbus client thread #%d after %d tries",
                      io_thread->tid, io_tries);
    }

    io_work->data_error = true;
    if (io_thread->wasConnected || io_thread->hadContext)
    {
        FPS_ERROR_LOG(
            "Operation now in progress (EINPROGRESS) error for modbus client thread #%d after %d tries. Thread #%d %s previously connected and %s a valid context. The thread is%s currently connected.",
            io_thread->tid, io_tries, io_thread->tid, (io_thread->wasConnected ? "was" : "was NOT"),
            (io_thread->hadContext ? "had" : "did NOT have"), (io_thread->connected ? "" : " NOT"));
    }

    handleDisconnect(myCfg, threadControl, io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// 88
void handleENOTSOCK(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                    struct cfg& myCfg)
{
    handleDisconnect(myCfg, threadControl, io_thread);

    if (io_thread->connect_fails < 1)
    {
        io_thread->connect_fails++;
        FPS_ERROR_LOG("Socket operation on non-socket (ENOTSOCK) error for modbus client thread #%d after %d tries",
                      io_thread->tid, io_tries);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

//    // INVALID_DATA 112345691 (EBADDATA)
// Caused by rc > 0 and rc != msg_length
// https://github.com/stephane/libmodbus/blob/5c14f13944ec7394a61a7680dcb462056c4321e3/src/modbus.c#L215
void handleINVALID_DATA(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                        struct cfg& myCfg)
{
    if (io_thread->ctx)
        modbus_flush(io_thread->ctx);

    FPS_ERROR_LOG(
        "Invalid data (EBADDATA) error for modbus client thread #%d after %d tries; Message data was likely out-of-sync with server; Register offset was between %d and %d; Ran flush\n",
        io_thread->tid, io_tries, io_work->offset, (io_work->offset + io_work->num_registers)

    );
}

// BAD_DATA_ADDRESS)
void handleBAD_DATA_ADDRESS(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                            struct cfg& myCfg)
{
    FPS_INFO_LOG(
        "Illegal data address (BAD_DATA_ADDRESS) for modbus client thread #%d after %d tries; Client or server is likely misconfigured; Register offset was between %d to %d; Aborting for now\n",
        io_thread->tid, io_tries, io_work->offset, (io_work->offset + io_work->num_registers));
}

// EMBBADEXC 112345691
void handleEMBBADEXC(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                     struct cfg& myCfg)
{
    // auto mberr =
    modbus_flush(io_thread->ctx);
    {
        FPS_ERROR_LOG("Invalid exception code (EMBBADEXC) for modbus client thread #%d after %d tries; Ran flush\n",
                      io_thread->tid, io_tries);
    }
}

// 22
void handleEINVAL(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                  struct cfg& myCfg)
{
    if (io_thread->ctx)
    {
        handleDisconnect(myCfg, threadControl, io_thread);
    }
    // std::cout<< std::dec << " error EINVAL 22:" << 0<<std::endl;
    io_work->data_error = true;

    if (io_thread->wasConnected || io_thread->hadContext)
    {
        FPS_ERROR_LOG(
            "Invalid argument (EINVAL) error for modbus client thread #%d after %d tries. Thread #%d %s previously connected and %s a valid context. The thread is%s currently connected.",
            io_thread->tid, io_tries, io_thread->tid, (io_thread->wasConnected ? "was" : "was NOT"),
            (io_thread->hadContext ? "had" : "did NOT have"), (io_thread->connected ? "" : " NOT"));
    }
}

// 32
void handleEPIPE(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                 struct cfg& myCfg)
{
    if (io_thread->ctx)
    {
        handleDisconnect(myCfg, threadControl, io_thread);
    }

    io_work->data_error = true;
    if (io_thread->wasConnected || io_thread->hadContext)
    {
        FPS_ERROR_LOG(
            "Broken pipe (EPIPE) error for modbus client thread #%d after %d tries. Thread #%d %s previously connected and %s a valid context. The thread is%s currently connected.",
            io_thread->tid, io_tries, io_thread->tid, (io_thread->wasConnected ? "was" : "was NOT"),
            (io_thread->hadContext ? "had" : "did NOT have"), (io_thread->connected ? "" : " NOT"));
    }
}

void handleDefaultError(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries)
{
    if (io_thread->wasConnected || io_thread->hadContext)
    {
        FPS_ERROR_LOG(
            "%s error for modbus client thread #%d after %d tries. Thread #%d %s previously connected and %s a valid context. The thread is%s currently connected.",
            modbus_strerror(io_work->errno_code), io_thread->tid, io_tries, io_thread->tid,
            (io_thread->wasConnected ? "was" : "was NOT"), (io_thread->hadContext ? "had" : "did NOT have"),
            (io_thread->connected ? "" : " NOT"));
        FPS_LOG_IT("thread_io_error");
    }
    // Default error handling
}

/**
 * @brief Translates a modbus error code from an io_work object into a human-readable error message.
 *
 * @param io_thread A pointer to a relevant io_thread
 * @param io_work The io_work object that experienced the error
 * @param io_tries The number of times the original action was attempted
 * @param myCfg The configuration for the modbus client
 * @param io_done Set to true if the connection has been terminated within the relevant handler
 */
void handleErrorCode(std::shared_ptr<IO_Thread>& io_thread, std::shared_ptr<IO_Work>& io_work, int& io_tries,
                     struct cfg& myCfg, bool& io_done)
{
    io_thread->fails++;
    if (0)
        std::cout << __func__ << " error code " << io_work->errno_code << std::endl;
    switch (io_work->errno_code)
    {
        case 0:
            if (!io_thread->ctx && !io_work->local)
            {
                io_work->data_error = true;
            }
            break;
        case ECONNRESET:
            handleECONNRESET(io_thread, io_work, io_tries, myCfg);
            break;
        case EBADF:
            handleEBADF(io_thread, io_work, io_tries, myCfg);
            break;
        case ETIMEDOUT:
            handleETIMEDOUT(io_thread, io_work, io_tries, myCfg);
            break;

        // Add other cases here
        case EPIPE:
            handleEPIPE(io_thread, io_work, io_tries, myCfg);
            break;
        case EINVAL:
            handleEINVAL(io_thread, io_work, io_tries, myCfg);
            break;
        case EMBBADEXC:
            handleEMBBADEXC(io_thread, io_work, io_tries, myCfg);
            break;
        case BAD_DATA_ADDRESS:
            handleBAD_DATA_ADDRESS(io_thread, io_work, io_tries, myCfg);
            io_done = true;
            break;
        case INVALID_DATA:
            handleINVALID_DATA(io_thread, io_work, io_tries, myCfg);
            io_done = true;
            break;
        case ENOTSOCK:
            handleENOTSOCK(io_thread, io_work, io_tries, myCfg);
            break;
        case EINPROGRESS:
            handleEINPROGRESS(io_thread, io_work, io_tries, myCfg);
            break;

        default:
            handleDefaultError(io_thread, io_work, io_tries);
            break;
    }
}

// ///////////////////////////////////////////////////////////////////////////////////
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
                       bool& io_done, int& io_tries, int& max_io_tries, bool debug)
{
    io_thread->wasConnected = io_thread->connected;
    io_thread->hadContext = (io_thread->ctx != nullptr);

    auto wasConnected = io_thread->connected;
    if (wasConnected)
    {
        if (0)
            std::cout << "<" << __func__ << "> was connected .. io_work " << io_work->mynum
                      << " error code :" << io_work->errno_code << " data_error :" << io_work->data_error << std::endl;

        if (io_thread->ctx == nullptr && io_thread->ip != "")
        {
            for (int i = 0; i < 2 && io_thread->ctx == nullptr; ++i)
            {
                double ctime;
                {
                    if (io_work->errno_code == ETIMEDOUT)
                    {
                        // if the error was a timeout we need to do a fast reconnect attempt
                        ctime = FastReconnectForThread(myCfg, io_thread, debug);
                    }
                    else
                    {
                        // else this will do
                        ctime = SetupModbusForThread(myCfg, io_thread, debug);
                    }
                }
                if (!io_thread->ctx)
                {
                    {
                        FPS_ERROR_LOG(
                            "Failed to allocate modbus connection context for thread #%d. Thread is%s connected after %d attempts.",
                            io_thread->tid, (io_thread->connected ? "" : "NOT"), io_tries);
                    }
                    io_thread->connected = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
                else
                {
                    FPS_INFO_LOG(
                        "Allocated connection context for thread #%d. Thread is%s connected after %d attempts. Time to connect: %2.3fs",
                        io_thread->tid, (io_thread->connected ? "" : "NOT"), io_tries, ctime);
                    io_work->cTime = ctime;
                }
            }
        }

        if (!io_thread->ctx && !io_work->local)
        {
            io_work->data_error = true;
            return;
        }
    }

    handleErrorCode(io_thread, io_work, io_tries, myCfg, io_done);
}

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
void local_write_registers(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        if (io_point)  // make sure it's not a nullptr (it shouldn't be, but just in case)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx);
                memcpy(io_point->reg16, &io_work->buf16[idx], io_point->size * 2);
            }
            idx += io_point->size;
        }
    }
    io_work->errors = idx;
}

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
void local_read_registers(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        if (io_point)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx);
                memcpy(&io_work->buf16[idx], io_point->reg16, io_point->size * 2);
            }
            idx += io_point->size;
        }
    }
    io_work->errors = idx;
}

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
void local_write_bits(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        if (io_point)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx);
                memcpy(io_point->reg8, &io_work->buf8[idx], io_point->size);
            }
            idx += io_point->size;
        }
    }
    io_work->errors = idx;
}

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
void local_read_bits(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        if (io_point)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx);
                memcpy(&io_work->buf8[idx], io_point->reg8, io_point->size);
            }
            idx += io_point->size;
        }
    }
    io_work->errors = idx;
}

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
void local_write_work(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if ((io_work->register_type == cfg::Register_Types::Holding) ||
        (io_work->register_type == cfg::Register_Types::Input))
        local_write_registers(myCfg, io_work, debug);
    else if ((io_work->register_type == cfg::Register_Types::Coil) ||
             (io_work->register_type == cfg::Register_Types::Discrete_Input))
        local_write_bits(myCfg, io_work, debug);
}

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
void local_read_work(struct cfg& myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if ((io_work->register_type == cfg::Register_Types::Holding) ||
        (io_work->register_type == cfg::Register_Types::Input))
        local_read_registers(myCfg, io_work, debug);
    else if ((io_work->register_type == cfg::Register_Types::Coil) ||
             (io_work->register_type == cfg::Register_Types::Discrete_Input))
        local_read_bits(myCfg, io_work, debug);
}

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
                         const char* func_name, int err)
{
    bool lost = false;
    auto wasConnected = io_thread->connected;
    auto hadContext = (io_thread->ctx != nullptr);

    if (err == ETIMEDOUT)
    {
        if (myCfg.connection.is_RTU)
        {
            if (io_thread->connection_timedout != true)
            {
                FPS_ERROR_LOG("RTU connection timed out, serial device [%s]", myCfg.connection.device_name);
            }
            io_work->data_error = true;
            lost = true;
        }
        io_thread->connection_timedout = true;
    }
    else
    {
        io_thread->connection_timedout = false;
    }

    if (err == EPIPE || err == EINVAL)
    {
        lost = true;
        // we need to disconnect
        if (io_thread->ctx)
        {
            handleDisconnect(myCfg, threadControl, io_thread);

            io_work->data_error = true;
            io_thread->connected = false;

            if (err == EPIPE)
            {
                FPS_ERROR_LOG(
                    "Broken pipe (EPIPE) error for modbus client thread #%d. Thread #%d %s previously connected and %s a valid context. The thread is%s currently connected.",
                    io_thread->tid, io_thread->tid, (wasConnected ? "was" : "was NOT"),
                    (hadContext ? "had" : "did NOT have"), (io_thread->connected ? "" : " NOT"));
            }
            else
            {
                FPS_ERROR_LOG(
                    "Invalid argument (EINVAL) error for modbus client thread #%d. Thread #%d %s previously connected and %s a valid context. The thread is%s currently connected.",
                    io_thread->tid, io_thread->tid, (wasConnected ? "was" : "was NOT"),
                    (hadContext ? "had" : "did NOT have"), (io_thread->connected ? "" : " NOT"));
            }
        }
    }
    return lost;
}

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
                                   std::shared_ptr<struct cfg::io_point_struct> io_point)
{
    if (io_point->errno_code != ECONNRESET)
    {
        return false;
    }

    io_thread->connect_reset++;
    FPS_INFO_LOG(
        "CONNECTION RESET for thread #%d. Enabled connect reset %d times for io_point [%s] (offset %d). Err code %d [%s] ",
        io_thread->tid, io_thread->connect_reset, io_point->id.c_str(), io_point->offset, io_point->errno_code,
        modbus_strerror(io_point->errno_code));

    if ((io_thread->tid > 1) && (io_thread->connect_reset > 10) && io_thread->is_enabled)
    {
        char message[1024];
        snprintf(
            message, 1024,
            "Point Error for thread #%d. Enabled connect reset %d times for io_point [%s] (offset %d). Err code %d [%s] ",
            io_thread->tid, io_thread->connect_reset, io_point->id.c_str(), io_point->offset, io_point->errno_code,
            modbus_strerror(io_point->errno_code));

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);

        io_thread->is_enabled = false;
    }
    return true;
}

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
                                std::shared_ptr<IO_Work> io_work, std::shared_ptr<struct cfg::io_point_struct> io_point)
{
    if ((io_point->errno_code != EPIPE) && (io_point->errno_code != ECONNRESET) && (io_point->errno_code != EINVAL) &&
        (io_point->errno_code != EAGAIN) && (io_point->errno_code != ETIMEDOUT))
    {
        return false;
    }

    if (!io_work->data_error)
    {
        FPS_ERROR_LOG(
            "Data Error for thread #%d at time [%2.3f]. Connected but I/O operation failed for io_point [%s] (offset %d%s). Err code %d [%s]",
            io_thread->tid, io_work->tNow, io_point->id.c_str(), io_point->offset,
            io_point->off_by_one ? ", point off_by_one" : "", io_point->errno_code,
            modbus_strerror(io_point->errno_code));
        io_work->data_error = true;
    }
    return true;
}

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
                                const char* func_name, int err)
{
    if (io_work->data_error)
    {
        return;
    }

    if (0)
        printf(
            "%s: Thread #%d is connected but I/O operation failed twice for io_point [%s] (offset %d%s%s, gap is %d). Err %d code %d [%s]. Auto disable is turned %s.\n",
            func_name, io_thread->tid, io_point->id.c_str(), io_point->offset,
            io_point->off_by_one ? ", point off_by_one" : "",
            io_point->is_disconnected ? "io_point still connected" : "io_point is diconnected", io_point->gap, err,
            io_point->errno_code, modbus_strerror(io_point->errno_code), myCfg.auto_disable ? "ON" : "OFF");

    if (io_point->errno_code == EMBBADEXC)
    {
        int io_tries = 0;
        handleEMBBADEXC(io_thread, io_work, io_tries, myCfg);
    }
}

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
                               const char* func_name, int err)
{
    if (!myCfg.auto_disable || (err == EPIPE) || (err == ECONNRESET) || (err == EAGAIN) || (err == EINVAL) ||
        (err == ETIMEDOUT) || (err == EMBBADEXC) || (err == EMBBADDATA) || (err == EMBXSBUSY))
    {
        if (0)
            FPS_INFO_LOG("%s: I/O operation failed for thread #%d for io_point [%s] (offset %d). Err %d code %d [% s] ",
                         func_name, io_thread->tid, io_point->id.c_str(), io_point->offset, err, io_point->errno_code,
                         modbus_strerror(io_point->errno_code));
        return;
    }

    if (0)
        std::cout << " setting point error , offset " << io_point->offset << std::endl;

    io_work->set_flag(io_point->offset, IO_Work::POINT_ERROR);

    if (!io_point->is_disconnected)
    {
        if (io_point->gap > 0)
        {
            io_work->set_flag(io_point->offset, IO_Work::REMOVE_GAP);

            FPS_INFO_LOG(
                "Point Error, thread_id %d failed for [%s] offset %d with gap [%d] err %d -> [%s]; gap removed",
                io_thread->tid, io_point->id.c_str(), io_point->offset, io_point->gap, err,
                modbus_strerror(io_point->errno_code));
        }
        else
        {
            io_work->set_flag(io_point->offset, IO_Work::POINT_DISCONNECT);
            FPS_INFO_LOG("Point Error, thread_id %d failed for [%s] offset %d err %d -> [%s]; point disconnected",
                         io_thread->tid, io_point->id.c_str(), io_point->offset, err,
                         modbus_strerror(io_point->errno_code));
        }
    }

    io_work->data_error = true;
}

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
                        std::shared_ptr<struct cfg::io_point_struct> io_point, const char* func_name, int err)
{
    io_point->errno_code = err;

    if (0)
        std::cout << __func__ << " point id [" << io_point->id << "] error [" << modbus_strerror(err) << "]"
                  << std::endl;

    handle_point_connection_reset(myCfg, io_thread, io_point);

    if (handle_point_common_errors(myCfg, io_thread, io_work, io_point))
    {
        return;
    }

    if (err >= EMBERROR)
    {
        handle_modbus_point_errors(myCfg, io_thread, io_work, io_point, func_name, err);
        return;
    }

    handle_point_auto_disable(myCfg, io_thread, io_work, io_point, func_name, err);
}

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
                        std::shared_ptr<IO_Work> io_work, bool debug)
{
    int result = 0;
    int offset = 0;
    io_work->good = 0;
    io_work->errors = 0;

    if (io_work->off_by_one)
        offset++;

    if (io_work->register_type == cfg::Register_Types::Holding)
    {
        result = modbus_read_registers(io_thread->ctx, io_work->offset - offset, io_work->num_registers,
                                       io_work->buf16);
        io_work->func = "read_registers";
    }
    else if (io_work->register_type == cfg::Register_Types::Coil)
    {
        result = modbus_read_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers, &io_work->buf8[0]);
        io_work->func = "read_bits";
    }
    else if (io_work->register_type == cfg::Register_Types::Input)
    {
        result = modbus_read_input_registers(io_thread->ctx, io_work->offset - offset, io_work->num_registers,
                                             io_work->buf16);
        io_work->func = "read_input_registers";
    }
    else if (io_work->register_type == cfg::Register_Types::Discrete_Input)
    {
        result = modbus_read_input_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers,
                                        io_work->buf8);
        io_work->func = "read_input_bits";
    }
    else
    {
        FPS_INFO_LOG("Get/Poll: unknown register type");
    }

    if (0)
        std::cout << __func__ << ">>> type [" << io_work->func << "] offset [" << io_work->offset << "] result "
                  << result << " errors " << io_work->errors << std::endl;

    if (result < 0)
    {
        // TODO move this
        auto err = errno;
        if (!has_lost_connection(myCfg, io_thread, io_work, io_work->func.c_str(), err))
        {
            // handle_point_error(myCfg, io_thread, io_work, io_point, "write register", err);
        }
        if (0)
            std::cout << __func__ << ">>> type [" << io_work->func << "] offset [" << io_work->offset << "] result "
                      << result << "] err " << modbus_strerror(err) << std::endl;

        io_work->errors = io_work->num_registers;
        io_work->good = 0;
    }
    else
    {
        io_work->good = 1;
    }

    return result;
}

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
                          std::shared_ptr<IO_Work> io_work, bool& io_done, bool debug)
{
    int result = 0;
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    int idx = 0;
    io_work->good = 0;
    io_work->errors = 0;
    for (auto io_point : io_work->io_points)
    {
        if (io_work->register_type == cfg::Register_Types::Holding)
        {
            result = modbus_read_registers(io_thread->ctx, io_point->offset - offset, io_point->size,
                                           &io_work->buf16[idx]);
        }

        // for (auto io_reg = 0; io_reg < io_point->size; io_reg++)
        // {
        //     errors = modbus_read_register(io_thread->ctx, io_point->offset - offset + io_reg, io_work->buf16[idx]);

        // }

        else if (io_work->register_type == cfg::Register_Types::Coil)
        {
            result = modbus_read_bits(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf8[idx]);
        }
        else if (io_work->register_type == cfg::Register_Types::Input)
        {
            result = modbus_read_input_registers(io_thread->ctx, io_point->offset - offset, io_point->size,
                                                 &io_work->buf16[idx]);
        }
        else if (io_work->register_type == cfg::Register_Types::Discrete_Input)
        {
            result = modbus_read_input_bits(io_thread->ctx, io_point->offset - offset, io_point->size,
                                            &io_work->buf8[idx]);
        }
        else
        {
            FPS_INFO_LOG("Get/Poll: unknown register type");
        }
        idx += io_point->size + io_point->gap;
        if (result < 0)
        {
            if (0)
                std::cout << __func__ << " id [" << io_point->id << "]" << std::endl;

            io_work->errors++;
            auto err = errno;
            io_work->errno_code = err;
            handleErrorCode(io_thread, io_work, io_tries, myCfg, io_done);
            if (!has_lost_connection(myCfg, io_thread, io_work, io_work->func.c_str(), err))
            {
                if (0)
                    std::cout << __func__ << " handle point error .. id [" << io_point->id << "]" << std::endl;

                handle_point_error(myCfg, io_thread, io_work, io_point, io_work->func.c_str(), err);
            }
        }
        else
        {
            io_work->good += result;
        }
    }
    return io_work->good;
}

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
                         std::shared_ptr<IO_Work> io_work, bool debug)
{
    bool setForceMulti = myCfg.force_multi_sets;  // get/setForceMulti forces the multi register instructions even for
                                                  // single register operations

    int result = 0;
    int offset = 0;
    io_work->good = 0;
    io_work->errors = 0;

    if (io_work->off_by_one)
        offset++;

    if (io_work->register_type == cfg::Register_Types::Holding)
    {
        result = modbus_write_registers(io_thread->ctx, io_work->offset - offset, io_work->num_registers,
                                        io_work->buf16);
        io_work->func = "write_registers";
    }
    else if (io_work->register_type == cfg::Register_Types::Coil)
    {
        if ((io_work->num_registers == 1) && !setForceMulti)
        {
            result = modbus_write_bit(io_thread->ctx, io_work->offset - offset, io_work->buf8[0]);
            io_work->func = "write_bit";
        }
        else
        {
            result = modbus_write_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers,
                                       &io_work->buf8[0]);
            io_work->func = "write_bits";
        }
    }
    else
    {
        FPS_INFO_LOG("Set: unknown register type");
    }
    if (result < 0)
    {
        // TODO move this
        auto err = errno;
        if (!has_lost_connection(myCfg, io_thread, io_work, io_work->func.c_str(), err))
        {
            // handle_point_error(myCfg, io_thread, io_work, io_point, "write register", err);
        }
        io_work->errors = io_work->num_registers;
        io_work->good = 0;
    }
    else
    {
        io_work->good = 1;
    }

    return result;
}

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
                           std::shared_ptr<IO_Work> io_work, bool& io_done, bool debug)
{
    bool setForceMulti = myCfg.force_multi_sets;  // get/setForceMulti forces the multi register instructions even for
                                                  // single register operations
    bool writeSingleRegisters = false;

    int result = 0;
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    int idx = 0;
    io_work->good = 0;
    io_work->errors = 0;
    for (auto io_point : io_work->io_points)
    {
        // if we are here we can write one point at a time and capture the status of that write attempt
        // or we can write one register at a time and force a failure if one of the resisters assiciated with the
        // configured
        //      point fails
        //   writeSingleRegisters forces us to write th data
        // we can also wish to write a single register but the vendor hardware does not accept single reister writes.
        //    in that case we always use write registers but with a count of 1.
        //         setForceMulti takes care of that operation
        if (io_work->register_type == cfg::Register_Types::Holding)
        {
            if ((io_point->size == 1) && !setForceMulti)
            {
                result = modbus_write_register(io_thread->ctx, io_point->offset - offset, io_work->buf16[idx]);
                io_work->func = "write_register";
            }
            else
            {
                if (!writeSingleRegisters)
                {
                    result = modbus_write_registers(io_thread->ctx, io_point->offset - offset, io_point->size,
                                                    &io_work->buf16[idx]);
                    io_work->func = "write_registers";
                }
                else
                {
                    for (auto io_reg = 0; io_reg < io_point->size; io_reg++)
                    {
                        result = modbus_write_register(io_thread->ctx, io_point->offset - offset + io_reg,
                                                       io_work->buf16[idx + io_reg]);
                        io_work->func = "write_single_registers";
                        if (result < 0)
                        {
                            // we got a failure we'll have to disconnect this point
                            break;
                        }
                    }
                }
            }
        }

        else if (io_work->register_type == cfg::Register_Types::Coil)
        {
            if ((io_point->size == 1) && !setForceMulti)
            {
                result = modbus_write_bit(io_thread->ctx, io_point->offset - offset, io_work->buf8[idx]);
                io_work->func = "write_bit";
            }
            else
            {
                result = modbus_write_bits(io_thread->ctx, io_point->offset - offset, io_point->size,
                                           &io_work->buf8[idx]);
                io_work->func = "write_bit";
            }
        }
        else
        {
            FPS_INFO_LOG("Set: unknown register type");
        }
        idx += io_point->size + io_point->gap;
        if (result < 0)
        {
            io_work->errors++;
            auto err = errno;
            io_work->errno_code = err;
            handleErrorCode(io_thread, io_work, io_tries, myCfg, io_done);
            if (!has_lost_connection(myCfg, io_thread, io_work, io_work->func.c_str(), err))
            {
                handle_point_error(myCfg, io_thread, io_work, io_point, io_work->func.c_str(), err);
            }
        }
        else
        {
            io_work->good += result;
        }
    }
    return io_work->good;
}

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
                   bool debug)
{
    double tNow = get_time_double();
    io_work->tIo = tNow;
    int io_tries = 0;
    int max_io_tries = 1;  // myCfg.max_io_tries;
    bool io_done = false;

    if (!io_work->local)
    {
        if (io_thread->connected && io_thread->ctx)
        {
            modbus_set_slave(io_thread->ctx, io_work->device_id);
        }
        else
        {
            io_done = true;
            io_work->data_error = true;
        }
    }

    // when not connected we can try one more time at the same timeout
    max_io_tries = 2;  // myCfg.max_io_tries;

    if (!io_work->local)
    {
        std::unique_lock<std::mutex> lock(io_thread->stat_mtx);

        if (io_work->wtype == WorkTypes::Set)
        {
        }
        else
        {
            io_thread->modbus_read_timer.start();
        }
    }
    io_work->threadId = io_thread->tid;
    io_thread->jobs++;

    while (!io_done && (io_tries < max_io_tries))
    {
        io_tries++;
        if (debug && io_tries > 1)
        {
            FPS_INFO_LOG(
                "thread_id %d running retry %d ; work type %d (%s)  offset %d num %d reg_type %d  (%s) debug %d",
                io_thread->tid, io_tries, (int)io_work->wtype, workTypeToStr(io_work->wtype).c_str(), io_work->offset,
                io_work->num_registers, /*(int)setForceMulti,*/
                (int)io_work->register_type, regTypeToStr(io_work->register_type).c_str(), debug);
        }

        io_work->errors = -2;     // flag a no go
        io_work->errno_code = 0;  // flag a no go
        // bool setForceMulti = myCfg.force_multi_sets; // get/setForceMulti forces the multi register instructions even
        // for single register operations bool getForceMulti = myCfg.force_multi_gets; bool setAllowMulti =
        // myCfg.allow_multi_sets; // get/setAllowMulti forces the single register instructions even for multi register
        // operations bool getAllowMulti = myCfg.allow_multi_gets;

        if (io_work->wtype == WorkTypes::Set)
        {
            {
                std::unique_lock<std::mutex> lock(io_thread->stat_mtx);
                io_thread->modbus_write_timer.start();
            }
            auto result = write_modbus_io_work(myCfg, io_tries, io_thread, io_work, debug);
            if (result < 0)
            {
                auto err = errno;
                if (!has_lost_connection(myCfg, io_thread, io_work, io_work->func.c_str(), err))
                {
                    write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, debug);
                }
            }
            {
                std::unique_lock<std::mutex> lock(io_thread->stat_mtx);
                io_thread->modbus_read_timer.snap();
            }
        }
        // read (Get) starts here
        else if ((io_work->wtype == WorkTypes::Get) || (io_work->wtype == WorkTypes::Poll))
        {
            if (io_work->local)
            {
                io_work->good = io_work->num_registers * 2;
                io_responseChan.send(std::move(io_work));
                return;
            }

            {
                std::unique_lock<std::mutex> lock(io_thread->stat_mtx);
                io_thread->modbus_read_timer.start();
            }

            auto result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, debug);
            if (result < 0)
            {
                auto err = errno;
                if (!has_lost_connection(myCfg, io_thread, io_work, io_work->func.c_str(), err))
                {
                    read_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, debug);
                }
            }
            {
                std::unique_lock<std::mutex> lock(io_thread->stat_mtx);
                io_thread->modbus_read_timer.snap();
            }
        }

        double tRun = get_time_double() - tNow;

        io_work->tRun = tRun;
        if (0)
            printf(">>>  errno_code thread_id %d   >> offset %d num %d errno_code %d \n", io_thread->tid,
                   io_work->offset, io_work->num_registers, io_work->errno_code);

        if (io_work->good > 0)
        {
            // got successful communication
            io_done = true;
        }
        else
        {
            // this will try a reconnect if not RTU
            handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);
        }
    }
    io_work->tDone = get_time_double();
    io_responseChan.send(std::move(io_work));
}

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
void ioThreadFunc(ThreadControl& control, struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread)
{
    std::shared_ptr<IO_Work> io_work;
    int signal;
    bool run = true;
    bool debug = false;
    double delay = 0.1;

    if (!io_thread->is_local)
    {
        FPS_INFO_LOG("IO_THREAD thread_id  %d started; enabled : %s ", io_thread->tid,
                     (io_thread->is_enabled ? "true" : "false"));
    }
    while (run && control.ioThreadRunning && io_thread->is_enabled)
    {
        if (!io_thread->is_local)
        {
            if (!io_thread->connected)
            {
                if ((myCfg.connection.serial_device == "") && (myCfg.connection.ip_address == ""))
                {
                    FPS_ERROR_LOG(
                        "Config error, no serial_device or ip_address ,thread_id [%d]: Unable to set up modbus.",
                        io_thread->tid);
                    run = false;
                    return;
                }

                for (int i = 0; i < 2 && !io_thread->connected; ++i)
                {
                    double ctime = SetupModbusForThread(myCfg, io_thread, debug);

                    if (ctime > 0.0)
                    {
                        io_thread->cTime = ctime;  // TODO use perf
                    }
                }
            }

            int num_threads = GetNumThreads(myCfg, threadControl);

            // this allows remote io_work objects to be collected in the case of a server disconnect
            // this causes problems
            // dont try to process stuff unless we are connected
            // or process stuff but discard io_work we accept and then become disconnected (error = 32)
            if ((io_thread->is_enabled) && (io_thread->connected || num_threads == 0))
            {
                if (io_threadChan.receive(signal, delay))
                {
                    // from testThread
                    if (signal == 2)
                    {
                        if (0)
                            printf(" >>>>>>>>>>>>>>>>%s testThread signal received \n", __func__);
                        if (check_socket_alive(io_thread, /*timeout_sec*/ 0, /* timeout_usec*/ 20) > 0)
                        {
                            if (0)
                                printf(" >>>>>>>>>>>>>>>>%s testThread shutting socket down \n", __func__);
                            CloseModbusForThread(io_thread, true);
                            handleDisconnect(myCfg, threadControl, io_thread);
                        }
                    }
                    // from killThread
                    if (signal == 3)
                    {
                        if (0)
                            printf(" >>>>>>>>>>>>>>>>%s killThread shutting socket down \n", __func__);
#ifndef FPS_TEST_MODE
                        CloseModbusForThread(io_thread, true);
#endif
                        handleDisconnect(myCfg, threadControl, io_thread);
                    }
                    if (signal == 0)
                        run = false;
                    if (signal == 1)
                    {
                        double stime = get_time_double();
                        std::string wtype = "set";
                        if (io_setChan.peekpop(io_work))
                        {
                            if (io_work->component)
                            {
                                double time_remaining =
                                    io_work->component->inter_message_delay -
                                    (stime - io_work->component->last_message_time) -
                                    0.000001;  // wait a microsecond less because of the time to do stuff
                                if (time_remaining > 0)
                                {
                                    std::thread delay_setWork_thread(setWorkWithDelay, io_work, time_remaining);
                                    delay_setWork_thread.detach();
                                    continue;
                                }
                                io_work->component->last_message_time = stime;
                            }
                            runThreadWork(myCfg, io_thread, io_work, debug);
                        }
                        if (io_pollChan.peekpop(io_work))
                        {
                            if (io_work->component)
                            {
                                double time_remaining =
                                    io_work->component->inter_message_delay -
                                    (stime - io_work->component->last_message_time) -
                                    0.000001;  // wait a microsecond less because of the time to do stuff
                                if (time_remaining > 0)
                                {
                                    std::thread delay_pollWork_thread(pollWorkWithDelay, io_work, time_remaining);
                                    delay_pollWork_thread.detach();
                                    continue;
                                }
                                io_work->component->last_message_time = stime;
                            }
                            wtype = "get";
                            runThreadWork(myCfg, io_thread, io_work, debug);
                        }
                        double etime = get_time_double();
                        if ((etime - stime) > io_thread->transfer_timeout)
                        {
                            // TODO do not spam this
                            FPS_ERROR_LOG(
                                "Transfer Timeout on %s thread id %d , work num %d time %2.3f elapsed (mS) %2.3f ",
                                wtype.c_str(), io_thread->tid, io_work->mynum, io_work->tNow, (etime - stime) * 1000);
                        }
                    }
                }
            }
        }
        else
        {
            if (io_localthreadChan.receive(signal, delay))
            {
                if (signal == 0)
                    run = false;

                if (io_localsetChan.peekpop(io_work))
                {
                    runThreadWork(myCfg, io_thread, io_work, debug);
                }
                if (io_localpollChan.peekpop(io_work))
                {
                    runThreadWork(myCfg, io_thread, io_work, debug);
                }
            }
        }
    }
    FPS_INFO_LOG("thread_id [%d]: Loop Completed; enabled [%s].", io_thread->tid,
                 (char*)io_thread->is_enabled ? "true" : "false");
    //    std::cout << " thread_id " << io_thread->tid << "   loop done; enabled :"<< io_thread->is_enabled <<
    //    std::endl;

    // TODO move to cfg.log_lock;
    if (!io_thread->is_local)
    {
#ifndef FPS_TEST_MODE
        CloseModbusForThread(io_thread, debug);
#endif
    }
    {
        char message[1024];

        snprintf(message, 1024, "thread_id %d stopping after %d jobs  %d fails, enabled %s ", io_thread->tid,
                 io_thread->jobs, io_thread->fails, (io_thread->is_enabled ? "true" : "false"));

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
    }
}

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
void processGroupCallback(struct PubGroup& pub_group, struct cfg& myCfg)
{
    // auto compsh = pub_group.component.lock();
    double tNow = get_time_double();
    bool debug = myCfg.connection.debug;
    bool found = false;
    // debug = true;
    if (debug)
        std::cout << " processing workgroup " << pub_group.key << " created at :" << pub_group.tNow
                  << " process time mS " << (tNow - pub_group.tNow) * 1000.0 << std::endl;

    std::string pub_key = "pub_";
    std::string set_key = "set_";
    std::string get_key = "get_";

    // if this is a pub response and we get io_work->errors == -3 that means we had no connection
    if (pub_group.key.substr(0, pub_key.length()) == pub_key)
    {
        // myCfg.performance_timers.pub_timer.start();
        pub_group.done = true;

        std::stringstream string_stream;
        std::string uri;
        bool first = true;
        int total_errors = 0;
        string_stream << "{";
        // cfg::pub_struct *mypub;
        std::shared_ptr<cfg::pub_struct> mypub = nullptr;

        for (auto io_work : pub_group.works)
        {
            uri = "/" + io_work->component->component_id + "/" + io_work->component->id;
            // set this for the first object
            if (mypub == nullptr)
            {
                mypub = io_work->pub_struct;
                if (0)
                    std::cout << " mypub completed name: " << io_work->work_name << " pending value :" << mypub->pending
                              << std::endl;
                // if we get one we can reset
                if (mypub->pending > 1)
                {
                    if (0)
                        std::cout << " mypub completed name: " << io_work->work_name
                                  << " pending value :" << mypub->pending << std::endl;
                    mypub->pend_timeout = 0.0;
                    mypub->kill_timeout = 0.0;
                    mypub->pending = 0;
                }
            }
            if (0)
                std::cout << " mypub completed xx 1 name: " << io_work->work_name << " data error "
                          << io_work->data_error << " errno code " << io_work->errno_code << std::endl;
            if (io_work->good == 0)
            {
                total_errors++;
            }
            else
            {
                // handle the comma
                if (first)
                {
                    first = false;
                }
                else
                {
                    string_stream << ",";
                }
                io_work->full = false;

                store_raw_data(io_work, myCfg.debug_decode);
                found = gcom_modbus_decode(io_work, string_stream, myCfg);
                if (0)
                    std::cout << " mypub completed xx 2 name: " << io_work->work_name
                              << " total_errors : " << total_errors << " string_stream [" << string_stream.str() << " ]"
                              << std::endl;
            }
        }

        // add the heartbeat / component connected fields
        // TODO make this optional
        // if (total_errors == 0)
        {
            cfg::component_struct* component = pub_group.pub_struct->component;
            bool send_time = false;
            std::string state_str;

            if (1 || myCfg.use_new_wdog)
            {
                if (component->heartbeat_enabled && component->heartbeat)
                {
                    send_time = true;
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        string_stream << ",";
                    }

                    state_str = component->heartbeat->state_str;
                    string_stream << "\"heartbeat_state\":\"" << state_str << "\"";

                    if (/*component->heartbeat->state_str == "INIT"||*/
                        component->heartbeat->state_str == "NORMAL")
                        state_str = "true";
                    else
                        state_str = "false";
                    // Bug 02_01_2024 use myhb->heartbeat_read_point
                    if (component->heartbeat->heartbeat_read_point != nullptr)
                    {
                        string_stream << ",\"modbus_heartbeat\":" << component->heartbeat->heartbeat_read_point->raw_val
                                      << "";
                    }
                    string_stream << ",\"component_connected\":" << state_str << "";
                    void hbSendWrite(cfg::heartbeat_struct * myhb);
                    hbSendWrite(component->heartbeat);
                }
                else
                {
                    int num_threads = GetNumThreads(myCfg, threadControl);
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        string_stream << ",";
                    }

                    if (num_threads > 0)
                        state_str = "true";
                    else
                        state_str = "false";
                    string_stream << "\"component_connected\":" << state_str << "";
                }

                // CURRENTLY NOT BEING USED SO I COMMENTED IT OUT
                // if (component->watchdog_enabled && component->watchdog){
                //     send_time = true;
                //     if (first)
                //     {
                //         first = false;
                //     }
                //     else
                //     {
                //         string_stream << ",";
                //     }

                //     std::string state_str = component->watchdog->state_str;
                //     if (component->watchdog->state_str == "INIT"||component->watchdog->state_str== "NORMAL")
                //         state_str = "true";
                //     else
                //         state_str = "false";
                //     string_stream << "\"component_connected\":" << state_str << "";
                // }
            }
            if (send_time)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    string_stream << ",";
                }
                addTimeStamp(string_stream);
            }

            string_stream << "}";
            if (total_errors == 0 || send_time)
            {
                if (myCfg.fims_gateway.Connected())
                {
                    send_pub(myCfg.fims_gateway, uri, string_stream.str());
                    myCfg.fims_connected = true;
                }
                else
                {
                    if (myCfg.fims_connected)
                    {
                        FPS_ERROR_LOG("fims not connected for output (pub) message");
                        myCfg.fims_connected = false;
                        myCfg.keep_running = false;
                    }
                }
            }

            mypub->pubStats.snap(mypub->pmtx);
            {
                std::lock_guard<std::mutex> lock(mypub->tmtx);
                mypub->comp_time = tNow;
            }

            // io_work->pub_struct = mypub;
            // myCfg.performance_timers.pub_timer.snap();
            if (myCfg.connection.debug)
            {
                std::cout << "Published to: ";
                std::cout << "\"" << uri << "\": " << string_stream.str() << std::endl;
            }
        }
        discardGroupCallback(pub_group, myCfg, true);
    }
    else if (pub_group.key.substr(0, set_key.length()) == set_key)
    {
        pub_group.done = true;
        std::stringstream string_stream;
        std::string uri;
        bool first = true;
        bool send = true;
        string_stream << "{";
        for (auto io_work : pub_group.works)
        {
            if (io_work->replyto == "")
            {
                send = false;
                break;
            }
            if (first)
            {
                uri = io_work->replyto;
                first = false;
            }
            else
            {
                string_stream << ",";
            }
            // if (io_work->errors > 0)
            //{

            found = gcom_modbus_decode(io_work, string_stream, myCfg);
            store_raw_data(io_work, false);
            // gcom_modbus_decode(io_work, string_stream, myCfg);
            //}
            if (debug)
                std::cout << __func__ << " offset :" << io_work->offset << " string so far [" << string_stream.str()
                          << "]"
                          << " found [" << found << "]" << std::endl;
        }
        string_stream << "}";

        if (send)
            send_set(myCfg.fims_gateway, uri, string_stream.str());
        discardGroupCallback(pub_group, myCfg, true);
    }
    else if (pub_group.key.substr(0, get_key.length()) == get_key)
    {
        // printf("\n\n get_key [%s]\n",pub_group.key.c_str());
        pub_group.done = true;
        std::stringstream ss;
        std::string uri;
        bool first = true;
        bool send = true;
        ss << "{";
        for (auto io_work : pub_group.works)
        {
            if (debug)
                std::cout << __func__ << " offset :" << io_work->offset << " get string so far [[" << ss.str() << "]]"
                          << std::endl;

            if (io_work->replyto == "")
            {
                send = false;
                if (debug)
                    std::cout << __func__ << "  no replyto " << std::endl;
                break;
            }
            else
            {
                if (debug)
                    std::cout << __func__ << "  found replyto [" << io_work->replyto << "]" << std::endl;
            }
            if (first)
            {
                uri = io_work->replyto;
                first = false;
            }
            else
            {
                ss << ",";
            }
            found = gcom_modbus_decode_debug(io_work, ss, myCfg, true, true);
            if (found && !io_work->local)
                local_write_work(myCfg, io_work, debug);
        }
        {
            // obscure bug but clear last comma if needed
            std::string str = ss.str();
            char lastChar;
            if (!str.empty())
            {
                lastChar = str.back();
                if (lastChar == ',')
                {
                    str.pop_back();
                    ss.str("");
                    ss.clear();
                    ss << str;
                }
            }
        }
        ss << "}";
        if (debug)
        {
            std::cout << __func__ << " found [" << found << "]"
                      << " get string at end [" << ss.str() << "]"
                      << " uri [" << uri << "]" << std::endl;
        }

        if (send)
        {
            if (debug)
                std::cout << " uri [" << uri << "] sent" << std::endl;
            if (myCfg.fims_gateway.Connected())
            {
                send_set(myCfg.fims_gateway, uri, ss.str());
                myCfg.fims_connected = true;
            }
            else
            {
                if (myCfg.fims_connected)
                {
                    FPS_ERROR_LOG("fims not connected for output (set) message");
                    myCfg.fims_connected = false;
                    myCfg.keep_running = false;
                }
            }
        }
        discardGroupCallback(pub_group, myCfg, true);
    }
}

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
void discardGroupCallback(struct PubGroup& pub_group, struct cfg& myCfg, bool ok)
{
    bool debug = false;
    double tNow = get_time_double();
    // std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> dropping pubgroup " << pub_group.key
    //             << " created at :" << pub_group.tNow << " time elapsed (ms) " << ((tNow-pub_group.tNow)* 1000.0)  <<
    //             std::endl;
    int num_groups = 0;
    int num_points = 0;
    std::shared_ptr<cfg::pub_struct> mypub = nullptr;

    for (auto io_work : pub_group.works)
    {
        mypub = io_work->pub_struct;
        num_groups++;
        num_points += io_work->num_registers;
        io_work->errno_code = 0;
        io_work->data_error = false;

        io_poolChan.send(std::move(io_work));
    }

    // we found a pubstruct so clear it
    if (mypub)
    {
        // TODO remove
        if (debug)
            std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> completed   pubgroup " << pub_group.key << " tNow " << tNow
                      << " start_time " << mypub->start_time << " elapsed_time " << tNow - mypub->start_time
                      << std::endl;

        std::lock_guard<std::mutex> lock(mypub->tmtx);

        mypub->comp_time = tNow;
    }

    pub_group.works.clear();
    if (myCfg.debug_pub)
    {
        if (ok)
        {
            std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> completing   pubgroup " << pub_group.key
                      << " created at :" << pub_group.tNow << " time elapsed (ms) "
                      << ((tNow - pub_group.tNow) * 1000.0) << " num_groups " << num_groups << " num points "
                      << num_points << std::endl;
        }
        else
        {
            std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> dropping   pubgroup " << pub_group.key
                      << " created at :" << pub_group.tNow << " time elapsed (ms) "
                      << ((tNow - pub_group.tNow) * 1000.0) << " num_groups " << num_groups << " num points "
                      << num_points << std::endl;
        }
    }
}

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
bool check_pubgroup_key(std::string key, std::shared_ptr<IO_Work> io_work)
{
    if (pubGroups.find(key) == pubGroups.end())
    {
        pubGroups[key] = PubGroup(key, io_work, false);
    }
    return true;
}

/**
 * @brief Displays the pubGroups at exit.
 *
 * This function iterates over the pubGroups container and prints the pubGroup key, tNow, and tDone values for each
 * entry.
 *
 * @return true if the operation was successful, false otherwise.
 */
bool show_pubgroup()
{
    FPS_INFO_LOG("Show pubGroups at exit ...");
    for (auto pubgroup : pubGroups)
    {
        FPS_INFO_LOG("Found  pubGroup key [%s]  tNow [%2.3f] tDone [%2.3f]", pubgroup.first.c_str(),
                     pubgroup.second.tNow, pubgroup.second.tDone);
    }
    return true;
}

/**
 * @brief Retrieves a pointer to the PubGroup associated with the given IO_Work.
 *
 * This function fetches the PubGroup associated with the specified IO_Work. If the
 * PubGroup does not already exist, it creates a new one using the work name as the key.
 *
 * @param io_work Shared pointer to the IO_Work for which the PubGroup is being retrieved.
 * @return Pointer to the PubGroup associated with the given IO_Work.
 */
PubGroup* get_pubgroup(std::shared_ptr<IO_Work> io_work)
{
    std::string key = io_work->work_name;
    check_pubgroup_key(key, io_work);
    return (&pubGroups[key]);
}

/**
 * @brief Retrieves a pointer to the PubGroup associated with the given key.
 *
 * This function fetches the PubGroup associated with the specified key. If the
 * PubGroup does not already exist, it creates a new one using the key.
 *
 * @param key Reference to the string key for which the PubGroup is being retrieved.
 * @return Pointer to the PubGroup associated with the given key.
 */
PubGroup* get_pubgroup(std::string& key)
{
    if (pubGroups.find(key) != pubGroups.end())
    {
        return &pubGroups[key];
    }

    FPS_INFO_LOG("No  pubGroup key [%s] found, creating it", key.c_str());
    pubGroups[key] = PubGroup(key, nullptr, false);

    return &pubGroups[key];
}

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
void processIOWorkResponse(std::shared_ptr<IO_Work> io_work, struct cfg& myCfg)
{
    double tNow = get_time_double();
    std::string key = io_work->work_name;  // + std::to_string(io_work->work_id);
    if (0)
        std::cout << __func__ << "[" << (tNow - 0.0) << "] "
                  << "\t io_work->tNow : " << io_work->tNow << "\t delay : " << (tNow - io_work->tNow)
                  << "\ttype get , io_work->work_name : " << key << " local " << (io_work->local ? "true" : "false")
                  << " work_group " << io_work->work_group << " work_id " << io_work->work_id << std::endl;

    bool debug = myCfg.debug_response;
    // we own the io_points in our io_work structures so handle all the errors and disconnects here
    // io_point->mtx

    for (auto io_point : io_work->io_points)
    {
        // std::unique_lock<std::mutex> lock(io_point->mtx);
        //  the io_point would not have been sent to the get/set queues if it was disconnected and not past the
        //  reconnect time.
        if (io_point->is_disconnected)
        {
            if (io_point->reconnect > 0.0 && tNow > io_point->reconnect)
            {
                io_point->is_disconnected = false;
                // io_point->reconnect  = 0.0; leave this as is to let the point be included if a get/set request comes
                // in
                //  while all of this is going on.
            }
        }
        // if we are preparing io_work queues and suddenly remove the gap we may have  an extra get/set group with a non
        // working gap. this will be a start up condition and we can live with that I think.
        if (io_work->get_flag(io_point->offset, IO_Work::REMOVE_GAP))
        {
            io_point->gap = 0;
        }
        if (io_work->get_flag(io_point->offset, IO_Work::POINT_DISCONNECT))
        {
            if (!io_point->is_disconnected)
            {
                io_point->is_disconnected = true;
                io_point->reconnect = tNow + 5.0;
            }
        }
        if (io_work->get_flag(io_point->offset, IO_Work::POINT_ERROR))
        {
            if (0)
                std::cout << " point error found on [" << io_point->id << "]" << std::endl;
        }
    }
    if (0)
        std::cout << __func__ << "[" << (tNow - 0.0) << "] "
                  << "\t io_work->tNow : " << io_work->tNow << "\t delay : " << (tNow - io_work->tNow)
                  << "\ttype get , io_work->work_name : " << key << " is_local ? "
                  << (io_work->local ? "true" : "false") << " work_group " << io_work->work_group << " work_id "
                  << io_work->work_id << std::endl;

    if (io_work->wtype == WorkTypes::Set)  //  || (io_work->wtype == WorkTypes::setForceMulti))
    {
        if (io_work->local)
        {
            if ((io_work->register_type == cfg::Register_Types::Holding) ||
                (io_work->register_type == cfg::Register_Types::Input))
            {
                local_write_registers(myCfg, io_work, debug);
            }
            else if ((io_work->register_type == cfg::Register_Types::Coil) ||
                     (io_work->register_type == cfg::Register_Types::Discrete_Input))
            {
                local_write_bits(myCfg, io_work, debug);
            }
        }
    }
    else if (io_work->wtype == WorkTypes::Get)  //  || (io_work->wtype == WorkTypes::setForceMulti))
    {
        if (debug)
            std::cout << __func__ << "[" << (tNow - 0.0) << "] "
                      << "\t io_work->tNow : " << io_work->tNow << "\t delay : " << (tNow - io_work->tNow)
                      << "\ttype get , io_work->work_name : " << key << " local " << (io_work->local ? "true" : "false")
                      << " work_group " << io_work->work_group << " work_id " << io_work->work_id << std::endl;

        if (io_work->local)
        {
            // Do the local read registers rather than using the modbus connection
            if ((io_work->register_type == cfg::Register_Types::Holding) ||
                (io_work->register_type == cfg::Register_Types::Input))
            {
                local_read_registers(myCfg, io_work, debug);
            }
            else if ((io_work->register_type == cfg::Register_Types::Coil) ||
                     (io_work->register_type == cfg::Register_Types::Discrete_Input))
            {
                local_read_bits(myCfg, io_work, debug);
            }
        }
    }

    // if there is no key then this operation is not part of a group so we dont have to collect all the objects
    if (key == "")
    {
        bool found = false;
        std::string reply;
        // if (debug)
        //     FPS_DEBUG_LOG
        if (0)
            printf("<%s> completing io_work, errors %d good %d data_error %d \ttNow: %f\treply: [%s]\n", __func__,
                   io_work->errors, io_work->good, io_work->data_error, io_work->tNow, io_work->replyto.c_str());
        // is it a set ?
        if (io_work->wtype == WorkTypes::Set)  //  || (io_work->wtype == WorkTypes::setForceMulti))
        {
            if (io_work->errors > 0 || io_work->data_error)
                reply = "{\"gcom\":\"Modbus Set\",\"status\":\"Failed\"}";
            else
                reply = "{\"gcom\":\"Modbus Set\",\"status\":\"Success\"}";
            if (io_work->local)
            {
                store_raw_data(io_work, false);
            }
            else
            {
                store_raw_data(io_work, false);
            }
        }

        if (io_work->wtype == WorkTypes::Get)  // || (io_work->wtype == WorkTypes::GetMulti))
        {
            if (io_work->local)
            {
                std::stringstream string_stream;
                found = gcom_modbus_decode_debug(io_work, string_stream, myCfg, false, false);

                if (debug)
                {
                    std::cout << __func__ << " found [" << found << "]"
                              << " get string at end [" << string_stream.str() << "]" << std::endl;
                }

                // reply = string_stream.str();
                if (found)
                {
                    reply = string_stream.str();
                }
                else
                {
                    reply = "{\"gcom\":\"Modbus Get\",\"status\":\"Failed disconnected data point\"}";
                }
                // std::cout << "it was a local get" << std::endl;
            }
            else
            {
                std::stringstream string_stream("");
                if (0)
                    std::cout << "it was a remote get ; data_error >> " << (io_work->data_error ? "true" : "false")
                              << std::endl;

                store_raw_data(io_work, false);
                found = gcom_modbus_decode_debug(io_work, string_stream, myCfg, false, false);
                if (debug)
                {
                    std::cout << __func__ << " found [" << found << "]"
                              << " get string at end [" << string_stream.str() << "]" << std::endl;
                }

                if (found)
                {
                    reply = string_stream.str();
                }
                else
                {
                    reply = "{\"gcom\":\"Modbus Get\",\"status\":\"Failed disconnected data point\"}";
                }
                // std::cout << "it was a remote get" << std::endl;
            }
        }

        // we could have a replyto
        if (io_work->replyto != "")
        {
            myCfg.fims_gateway.Send(fims::str_view{ "set", sizeof("set") - 1 },
                                    fims::str_view{ io_work->replyto.data(), io_work->replyto.size() },
                                    fims::str_view{ nullptr, 0 }, fims::str_view{ nullptr, 0 },
                                    fims::str_view{ reply.data(), reply.size() });
        }
        io_poolChan.send(std::move(io_work));
        return;
    }

    // we are working with a pubgroup or a get group
    bool pgret = check_pubgroup_key(key, io_work);

    if (debug)
        std::cout << " >> >>>>> tNow [" << tNow << " io_work tNow [" << io_work->tNow
                  << "] io_work seq:" << io_work->mynum << "] io_work regs: [" << io_work->num_registers
                  << "] errors :" << io_work->errors << " good :" << io_work->good
                  << " error_code :" << modbus_strerror(io_work->errno_code) << " received by resp channel "
                  << " key [" << key << "]"
                  << " pubgroup key  [" << (pgret ? "true" : "false") << "]" << std::endl;

    if ((!pgret) || (pubGroups.find(key) == pubGroups.end()))
    {
        FPS_INFO_LOG("io_work %d Pubgroup key not found [%s]. Discarding io_work", io_work->mynum, key.c_str());
        io_poolChan.send(std::move(io_work));
        return;
    }
    else
    {
        // if we get a new (later) tNow then discard and start again
        if (myCfg.debug_connection)
            FPS_DEBUG_LOG("Checking pubgroup %s size %d pub time %f must equal incoming time %f", key.c_str(),
                          (int)pubGroups[key].works.size(), pubGroups[key].tNow, io_work->tNow);
        if (io_work->tNow < pubGroups[key].tNow)
        {
            FPS_INFO_LOG("Discarding stale incoming io_work; current pubgroup id  %f is later than incoming id %f",
                         pubGroups[key].tNow, io_work->tNow);
            auto mypub = io_work->pub_struct;

            if (mypub && mypub->pending > 1)
                mypub->pending--;
            // todo decrement the pollcount
            io_poolChan.send(std::move(io_work));
            return;
        }

        // did we get a match if so add to the group
        else if (io_work->tNow == pubGroups[key].tNow)
        {
            if (myCfg.debug_connection)
                FPS_DEBUG_LOG("Adding incoming io_work; current pubgroup id %f is the same as id %f",
                              pubGroups[key].tNow, io_work->tNow);
            pubGroups[key].works.push_back(io_work);
            pubGroups[key].pub_struct = io_work->pub_struct;
            pubGroups[key].work_group = io_work->work_group;
        }
        else
        {
            // Callback: Discard group if it was not finished
            if (pubGroups[key].done == false)
            {
                discardGroupCallback(pubGroups[key], myCfg, false);
                auto mypub = io_work->pub_struct;
                if (mypub)
                {
                    if (debug)
                        std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> mypub aborted name: " << io_work->work_name
                                  << "\t pending value :" << mypub->pending << " work num :" << io_work->mynum
                                  << " error_code :" << io_work->errno_code << std::endl;
                }
                if (mypub && mypub->pending > 1)
                    mypub->pending--;
            }
            else
            {
                // this should cause the pubs to restart
                // this is done to indicate that we are receiving data on that io_work point
                auto mypub = io_work->pub_struct;
                // no need to announce the good news.
                if (0)
                    std::cout << " mypub completed name: " << io_work->work_name
                              << "\t pending value :" << mypub->pending << " work num :" << io_work->mynum
                              << " error_code :" << io_work->errno_code << std::endl;
                if (mypub && mypub->pending > 1)
                    mypub->pending--;
            }
            // reset the pubgroup
            pubGroups[key].reset_group(key, io_work);
            // push the new item onto the new group
            pubGroups[key].works.push_back(io_work);
        }
    }

    // check if we have all the items we need for the group
    if ((int)pubGroups[key].works.size() == (int)pubGroups[key].work_group)
    {
        // Callback: Process group
        pubGroups[key].done = true;
        pubGroups[key].tDone = tNow;
        if (myCfg.debug_connection)
            FPS_DEBUG_LOG("Completing group %s\ttnow: %f\t sync %f", key.c_str(), io_work->tNow,
                          pubGroups[key].pub_struct ? pubGroups[key].pub_struct->syncPct : 0.0);
        processGroupCallback(pubGroups[key], myCfg);
        if (pubGroups[key].pub_struct)
            syncTimeObjectByName(key, pubGroups[key].pub_struct->syncPct);
        if (pubGroups[key].erase_group == true)
        {
            if (myCfg.debug_connection)
                FPS_DEBUG_LOG("Erasing group %s\ttnow: %f", key.c_str(), io_work->tNow);
            pubGroups.erase(key);  // Optionally remove the group after processing
        }
    }
    else
    {
        if (myCfg.debug_connection)
            FPS_DEBUG_LOG("Still collecting  group  %s\tSize: %d out of %d\ttnow: %f", key.c_str(),
                          (int)pubGroups[key].works.size(), (int)pubGroups[key].work_group, io_work->tNow);
    }
}

/**
 * @brief Function to handle response processing in a separate thread.
 *
 * This function runs in a loop while the response thread is active. It receives
 * IO_Work objects from the response channel, processes them, and updates response statistics.
 *
 * @param control Reference to the ThreadControl object managing the thread state and statistics.
 * @param myCfg Reference to the configuration object.
 */
void processIOWorkResponseThread(ThreadControl& control, struct cfg& myCfg)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    while (control.responseThreadRunning)
    {
        if (io_responseChan.receive(io_work, delay))
        {
            io_work->tReceive = get_time_double();

            // Collate batches response_received_work
            processIOWorkResponse(io_work, myCfg);

            double tNow = get_time_double();
            {
                auto tEnd = io_work->tStart;
                auto duration = tNow - tEnd;
                control.num_responses++;
                control.tResponse += duration;
            }
            // io_poolChan.send(std::move(io_work));
        }
    }
}

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
void stashWork(std::shared_ptr<IO_Work> io_work)
{
    // possibly TODO use dequw
    io_poolChan.send(std::move(io_work));
}

/**
 * @brief Creates and initializes a new IO_Work object.
 *
 * This function creates and initializes an IO_Work object, either by reusing an available one from the pool or by
 * creating a new one if the pool is empty. The function sets various fields of the IO_Work object based on the input
 * parameters.
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
                                   WorkTypes wtype)
{
    std::shared_ptr<IO_Work> io_work;
    std::string repto("");
    int work_wait = 0;
    // myCfg.max_work_wait; // after this we give up

    if (!io_poolChan.peekpop(io_work))
    {  // Assuming receive will return false if no item is available.
        // std::cout << " create an io_work object "<< std::endl;
        if (num_work < max_work)
        {
            // myCfg.
            num_work++;
            io_work = std::make_shared<IO_Work>();
            if (0)
                std::cout << " create an io_work object number  " << num_work << std::endl;
        }
        else
        {
            while (!io_poolChan.peekpop(io_work))
            {
                work_wait++;
                if (work_wait > max_work_wait)
                {
                    std::cout << " failed to get a released io_work buffer " << std::endl;
                    // myCfg.keep_fims_running = false;
                    return nullptr;
                }
                // we've got to get one sometime or have we
                std::this_thread::sleep_for(10ms);
            }
        }
    }
    io_work->use_count++;
    memset(io_work->flags, 0, sizeof(io_work->flags));
    io_work->io_points.clear();
    // Modify io_work data if necessary here
    io_work->tStart = get_time_double();
    io_work->device_id = device_id;
    io_work->register_type = register_type;
    io_work->offset = offset;
    io_work->component = component;
    // io_work->start_offset = offset;
    io_work->num_registers = num_regs;

    io_work->set_bufs(num_regs, u16bufs, u8bufs);
    io_work->wtype = wtype;
    io_work->test_mode = false;

    io_work->local = false;
    io_work->data_error = false;
    io_work->replyto = repto;
    io_work->work_name = repto;
    io_work->off_by_one = off_by_one;
    io_work->full = false;
    io_work->pub_struct = nullptr;

    return (io_work);
}

/**
 * @brief Sends a signal to the I/O thread channel to indicate a test action.
 *
 * This function sends the signal `2` to the `io_threadChan`, which is used to trigger a specific
 * test-related action in the I/O thread. It always returns `true` indicating the signal was sent.
 *
 * @return true Always returns true indicating the signal was sent.
 */
bool testThread()
{
    io_threadChan.send(2);
    return true;
}

/**
 * @brief Sends a signal to the I/O thread channel to indicate termination.
 *
 * This function sends the signal `3` to the `io_threadChan`, which is used to trigger the termination
 * of the I/O thread. It always returns `true` indicating the signal was sent.
 *
 * @return true Always returns true indicating the signal was sent.
 */
bool killThread()
{
    io_threadChan.send(3);
    return true;
}

/**
 * @brief Sends I/O work to the appropriate polling channel and signals the thread.
 *
 * This function sends the given `io_work` to either the local or remote polling channel
 * based on the `local` attribute of `io_work`. It then sends a signal to the corresponding
 * thread channel to indicate that new work is available.
 *
 * @param io_work A shared pointer to an `IO_Work` object that represents the I/O work to be processed.
 * @return true Always returns true indicating the operation was initiated.
 */
bool pollWork(std::shared_ptr<IO_Work> io_work)
{
    if (io_work)
    {
        if (io_work->local)
        {
            io_localpollChan.send(std::move(io_work));
            io_localthreadChan.send(1);
        }
        else
        {
            io_pollChan.send(std::move(io_work));
            io_threadChan.send(1);
        }
    }
    return true;  // You might want to return a status indicating success or failure.
}

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
bool setWork(std::shared_ptr<IO_Work> io_work)
{
    if (io_work)
    {
        if (io_work->local)
        {
            io_localsetChan.send(std::move(io_work));
            io_localthreadChan.send(1);
        }
        else
        {
            io_setChan.send(std::move(io_work));
            io_threadChan.send(1);
        }
    }
    return true;  // You might want to return a status indicating success or failure.
}

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
bool setWorkWithDelay(std::shared_ptr<IO_Work> io_work, double delay_seconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int64_t>(delay_seconds * 1000000)));
    return setWork(io_work);
}

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
bool pollWorkWithDelay(std::shared_ptr<IO_Work> io_work, double delay_seconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int64_t>(delay_seconds * 1000000)));
    return pollWork(io_work);
}

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
bool startProcessIOWorkResponseThread(struct cfg& myCfg)
{
    threadControl.responseThread = std::thread(processIOWorkResponseThread, std::ref(threadControl), std::ref(myCfg));
    return true;
}

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
                                          double transfer_timeout, struct cfg& myCfg)
{
    // IO_Thread io_thread;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    // Actually start the threads here
    io_thread->tid = idx;
    io_thread->jobs = 0;
    io_thread->fails = 0;
    io_thread->ip = "";
    io_thread->myCfg = &myCfg;

    if (ip)
        io_thread->ip = ip;
    io_thread->port = port;
    io_thread->connection_timeout = connection_timeout;
    io_thread->transfer_timeout = transfer_timeout;
    return (io_thread);
}

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
                  struct cfg& myCfg)
{
    // threadControl.io_pollChan     = &io_pollChan;
    // threadControl.io_responseChan = &io_responseChan;
    // threadControl.io_poolChan     = &io_poolChan;

    double tNow = get_time_double();
    FPS_INFO_LOG("IO Threads starting at %f ", tNow);

    // Start the response thread
    startProcessIOWorkResponseThread(myCfg);

    // start the local thread no connection needed.
    std::shared_ptr<IO_Thread> io_thread = make_IO_Thread(0, ip, port, connection_timeout, transfer_timeout, myCfg);
    io_thread->is_local = true;
    io_thread->thread = std::thread(ioThreadFunc, std::ref(threadControl), std::ref(myCfg), io_thread);
    threadControl.ioThreadPool.push_back(
        io_thread);  // std::thread (ioThreadFunc, std::ref(systemControl), io_thread));
    io_thread->tid = 0;

    // int num_threads = 4;
    for (int i = 0; i < num_threads; ++i)
    {
        // IO_Thread io_thread;
        std::shared_ptr<IO_Thread> io_thread = make_IO_Thread(i + 1, ip, port, connection_timeout, transfer_timeout,
                                                              myCfg);

        // create thread timer
        double tNow = get_time_double();

        // std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
        // // Actually start the threads here
        io_thread->tid = i + 1;

        FPS_INFO_LOG("thread_id %d starting at %f ", i, tNow);

        io_thread->thread = std::thread(ioThreadFunc, std::ref(threadControl), std::ref(myCfg), io_thread);

        threadControl.ioThreadPool.push_back(
            io_thread);  // std::thread (ioThreadFunc, std::ref(systemControl), io_thread));

        // std::string thstr = "timer_thread_" + std::to_string(i+1);
        // std::shared_ptr<TimeObject> obj1 = createTimeObject(thstr,                 // name
        //                                                     tNow,                   // start time (initial startup
        //                                                     time) 0,                           // stop time - 0 =
        //                                                     don't stop 5.0, // how often to repeat 0, // count - 0 =
        //                                                     don't stop threadCallback,                 // callback
        //                                                     (void *)io_thread.get());         // callback params
        // addTimeObject(obj1, 0.0, true);
    }
    // std::thread responseThread(processIOWorkResponseThread, std::ref(*this));
    tNow = get_time_double();
    FPS_INFO_LOG("All io_threads running at %f ", tNow);
    FPS_LOG_IT("startup");
    return true;
}

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
bool StartThreads(struct cfg& myCfg, bool debug)
{
    return StartThreads(myCfg.connection.max_num_connections, myCfg.connection.ip_address.c_str(),
                        myCfg.connection.port, myCfg.connection.connection_timeout, myCfg.connection.transfer_timeout,
                        myCfg);
}

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
bool StopThreads(struct cfg& myCfg, bool debug)
{
    threadControl.stopThreads();
    {
        FPS_INFO_LOG("All io_threads stopped");
    }

    threadControl.responseThread.join();
    {
        FPS_INFO_LOG("Response thread stopped");
    }
    return true;
}

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
int wait_socket_ready(modbus_t* ctx, int timeout_sec)
{
    int socket_fd = modbus_get_socket(ctx);
    if (socket_fd == -1)
    {
        return -1;  // Invalid socket
    }

    fd_set write_set;
    struct timeval timeout;

    FD_ZERO(&write_set);
    FD_SET(socket_fd, &write_set);
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

#ifndef FPS_TEST_MODE
    int result = select(socket_fd + 1, NULL, &write_set, NULL, &timeout);
#else
    int result = socket_fd;
    (void)write_set;
    (void)timeout;
#endif
    if (result > 0)
    {
        // // Check if connection was successful
        // int optval;
        // socklen_t optlen = sizeof(optval);
        // if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == 0) {
        //     if (optval == 0) {
        //         return 0; // Connection successful
        //     }
        // }
        return 0;
    }

    return -1;  // Connection failed or timed out
}

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
int check_socket_alive(std::shared_ptr<IO_Thread> io_thread, int timeout_sec, int timeout_usec)
{
    int socket_fd = modbus_get_socket(io_thread->ctx);
    if (socket_fd == -1)
    {
        return -1;  // Invalid socket
    }

    fd_set except_set, read_set;
    struct timeval timeout;

    FD_ZERO(&except_set);
    FD_ZERO(&read_set);
    FD_SET(socket_fd, &except_set);
    FD_SET(socket_fd, &read_set);
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = timeout_usec;
#ifndef FPS_TEST_MODE
    int result = select(socket_fd + 1, &read_set, NULL, &except_set, &timeout);
#else
    int result = socket_fd;
    (void)except_set;
    (void)timeout;
#endif
    // printf( " %s socket test result %d \n", __func__, result);

    if (result > 0)
    {
        // // Check if exception occured
        // int optval;
        // socklen_t optlen = sizeof(optval);
        // if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == 0) {
        //     if (optval == 0) {
        //         return 0; // Connection successful
        //     }
        // }
        auto foo_read = FD_ISSET(socket_fd, &read_set);
        // auto foo_except = FD_ISSET(socket_fd,&except_set);
        // printf( " socket test result %d read %d except %d\n", result, foo_read, foo_except);
        //  if we get a read , in this case it is an error
        return foo_read;
    }

    return 0;  // Connection failed or timed out
}

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
double FastReconnectForThread(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug)
{
    double tNow = get_time_double();

    // make sure the old context is deleted
    modbus_t* ctx = nullptr;

    if ((io_thread->tid != 1) && (!io_thread->is_enabled))
    {
        return 0.0;
    }

    if (myCfg.connection.is_RTU)
    {
        ctx = modbus_new_rtu(myCfg.connection.device_name.c_str(), myCfg.connection.baud_rate, myCfg.connection.parity,
                             myCfg.connection.data_bits, myCfg.connection.stop_bits);
    }
    else
    {
        ctx = modbus_new_tcp(io_thread->ip.c_str(), io_thread->port);
    }
    if (!ctx)
    {
        DelayConnect(myCfg, threadControl);
        handleDisconnect(myCfg, threadControl, io_thread);
        char message[1024];
        if (myCfg.connection.is_RTU)
        {
            snprintf(message, 1024, "Modbus Client [%s]  Thread id %d failed to create modbus RTU context to [%s]",
                     myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str());
        }
        else
        {
            snprintf(message, 1024,
                     "Modbus Client [%s]  Thread id %d failed to create modbus TCP context to [%s] on port [%d]",
                     myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
        }
        {
            FPS_ERROR_LOG("%s", message);
        }
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
        // io_thread->connected = false;
        return 0.0;
    }

    // this has to be fast
    uint32_t to_sec = (uint32_t)io_thread->transfer_timeout;  // 2-10 seconds timeout
    uint32_t to_usec = (uint32_t)((io_thread->transfer_timeout - to_sec) * 1000000.0);

    auto mberrto = modbus_set_response_timeout(ctx, to_sec, to_usec);
#ifndef FPS_TEST_MODE
    std::cout << " Transfer Timeout  " << io_thread->transfer_timeout << "  to_sec:  " << to_sec
              << " to_usec:" << mberrto << std::endl;
#else
    (void)mberrto;
#endif
    auto mberr = modbus_connect(ctx);

    if (mberr != 0)
    {
        auto err = errno;
        // std::cout << std::dec << " Connect modbus " << io_thread->ip << "  port " << io_thread->port << " Error
        // mberr:" << mberr << std::endl;
        modbus_free(ctx);
        DelayConnect(myCfg, threadControl);
        io_thread->ctx = nullptr;
        handleDisconnect(myCfg, threadControl, io_thread);
        if (io_thread->connect_fails < 2)
        {
            io_thread->connect_fails++;
            char message[1024];
            if (myCfg.connection.is_RTU)
            {
                snprintf(message, 1024,
                         "Modbus Client [%s]  Thread id %d failed to create modbus context to [%s]. Modbus error [%s]",
                         myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str(),
                         modbus_strerror(err));
            }
            else
            {
                snprintf(
                    message, 1024,
                    "Modbus Client [%s] Thread id %d failed to create modbus context to [%s] on port [%d]. Modbus error [%s]",
                    myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port,
                    modbus_strerror(err));
            }
            FPS_ERROR_LOG("%s", message);
            emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
        }
        return 0.0;
    }

    modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)MODBUS_ERROR_RECOVERY_PROTOCOL);

    io_thread->ctx = ctx;
    handleConnect(myCfg, threadControl, io_thread);

    // io_thread->connected = true;
    io_thread->connect_time = get_time_double() - tNow;
    char message[1024];
    if (myCfg.connection.is_RTU)
    {
        snprintf(message, 1024, "Modbus Client [%s] Thread  id %d successfully connected to [%s]",
                 myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str());
    }
    else
    {
        snprintf(message, 1024, "Modbus Client [%s] Thread  id %d successfully connected to [%s] on port [%d]",
                 myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
    }

    FPS_INFO_LOG("%s", message);
    emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
    io_thread->connect_fails = 0;
    io_thread->connect_reset = 0;  // get_time_double() - tNow;
    return (io_thread->connect_time);
}

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
double SetupModbusForThread(struct cfg& myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug)
{
    double tNow = get_time_double();
    if ((io_thread->tid != 1) && (!io_thread->is_enabled))
    {
        return 0.0;
    }
    if (OkToConnect(threadControl, tNow))
    {
        uint32_t to_sec = (uint32_t)io_thread->connection_timeout;                            // 2-10 seconds timeout
        uint32_t to_usec = (uint32_t)((io_thread->connection_timeout - to_sec) * 1000000.0);  // 0 microsecond
        io_thread->ctx = nullptr;
        modbus_t* ctx = nullptr;
        if (myCfg.connection.is_RTU)
        {
            ctx = modbus_new_rtu(myCfg.connection.device_name.c_str(), myCfg.connection.baud_rate,
                                 myCfg.connection.parity, myCfg.connection.data_bits, myCfg.connection.stop_bits);
        }
        else
        {
            ctx = modbus_new_tcp(io_thread->ip.c_str(), io_thread->port);
        }

        if (!ctx)
        {
            DelayConnect(myCfg, threadControl);
            handleDisconnect(myCfg, threadControl, io_thread);
            char message[1024];
            if (myCfg.connection.is_RTU)
            {
                snprintf(message, 1024, "Modbus Client [%s] Thread  id %d failed to create modbus RTU context to [%s]",
                         myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str());
            }
            else
            {
                snprintf(message, 1024,
                         "Modbus Client [%s] Thread  id %d failed to create modbus TCP context to [%s] on port [%d]",
                         myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
            }

            FPS_ERROR_LOG("%s", message);
            emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
            // io_thread->connected = false;
            return 0.0;
        }

        auto mberr = modbus_connect(ctx);
        if (0)
            std::cout << std::dec << " Connect modbus " << io_thread->ip << "  port " << io_thread->port
                      << " Error mberr:" << mberr << std::endl;

        if (mberr != 0)
        {
            auto err = errno;
            // std::cout << std::dec << " Connect modbus " << io_thread->ip << "  port " << io_thread->port << " Error
            // mberr:" << mberr << std::endl;
            modbus_free(ctx);
            DelayConnect(myCfg, threadControl);
            io_thread->ctx = nullptr;
            handleDisconnect(myCfg, threadControl, io_thread);
            if (io_thread->connect_fails < 2)
            {
                io_thread->connect_fails++;
                char message[1024];
                if (myCfg.connection.is_RTU)
                {
                    snprintf(message, 1024,
                             "Modbus Client [%s] Thread id %d failed to connect RTU to [%s]. Modbus error [%s]",
                             myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str(),
                             modbus_strerror(err));
                }
                else
                {
                    snprintf(
                        message, 1024,
                        "Modbus Client [%s] Thread id %d failed to connect TCP to [%s] on port [%d]. Modbus error [%s]",
                        myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port,
                        modbus_strerror(err));
                }
                FPS_ERROR_LOG("%s", message);
                emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
            }
            return 0.0;
        }

        wait_socket_ready(ctx, 1);

        if (modbus_set_response_timeout(ctx, to_sec, to_usec) == -1)
        {
            // std::cout << "{ \"status\":\"error\" , \"message\":\" Unable to set timeout  to " << io_thread->ip << "
            // port " << io_thread->port << "\"}" << std::endl;
            modbus_free(ctx);
            DelayConnect(myCfg, threadControl);
            io_thread->ctx = nullptr;
            handleDisconnect(myCfg, threadControl, io_thread);
            char message[1024];
            if (myCfg.connection.is_RTU)
            {
                snprintf(message, 1024, "Modbus Client [%s] Thread  id %d failed to set RTU response timeout as [%s]",
                         myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str());
            }
            else
            {
                snprintf(message, 1024,
                         "Modbus Client [%s] Thread  id %d failed to set TCP response timeout as [%s] on port [%d]",
                         myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
            }

            FPS_ERROR_LOG("%s", message);
            emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
            return 0.0;
        }

        modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)MODBUS_ERROR_RECOVERY_PROTOCOL);

        io_thread->ctx = ctx;
        handleConnect(myCfg, threadControl, io_thread);

        // io_thread->connected = true;
        io_thread->connect_time = get_time_double() - tNow;
        char message[1024];
        if (myCfg.connection.is_RTU)
        {
            snprintf(message, 1024, "Modbus Client [%s] Thread  id %d successfully connected to [%s]",
                     myCfg.connection.name.c_str(), io_thread->tid, myCfg.connection.device_name.c_str());
        }
        else
        {
            snprintf(message, 1024, "Modbus Client [%s] Thread  id %d successfully connected to [%s] on port [%d]",
                     myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
        }
        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
        io_thread->connect_fails = 0;

        to_sec = 0;                                      // io_thread->run_timeout; // 2-10 seconds timeout
        to_usec = 100000;                                // 0 microsecond
        to_sec = (uint32_t)io_thread->transfer_timeout;  // 2-10 seconds timeout
        to_usec = (uint32_t)((io_thread->transfer_timeout - to_sec) * 1000000.0);
        modbus_set_response_timeout(ctx, to_sec, to_usec);
        // TODO check for errors

        return (io_thread->connect_time);
    }
    return 0.0;
}

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
bool CloseModbusForThread(std::shared_ptr<IO_Thread> io_thread, bool debug)
{
    auto ctx = io_thread->ctx;
    io_thread->ctx = nullptr;
    if (ctx)
    {
        modbus_close(ctx);
        modbus_free(ctx);
        char message[1024];
        if (io_thread->myCfg->connection.is_RTU)
        {
            snprintf(message, 1024, "Modbus Client [%s] disconnected from [%s]",
                     io_thread->myCfg->connection.name.c_str(), io_thread->myCfg->connection.device_name.c_str());
        }
        else
        {
            snprintf(message, 1024, "Modbus Client [%s] disconnected from [%s] on port [%d]",
                     io_thread->myCfg->connection.name.c_str(), io_thread->ip.c_str(), io_thread->port);
        }

        FPS_INFO_LOG("%s", message);
        emit_event(&io_thread->myCfg->fims_gateway, "Modbus Client", message, 1);
    }
    return true;
}