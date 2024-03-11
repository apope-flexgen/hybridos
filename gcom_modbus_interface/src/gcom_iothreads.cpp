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
#include <errno.h> //For errno - the error number
#include <cxxabi.h>
#include <iostream>
#include <stdexcept>
#include <cfenv> // For standard floating point functions


#include <modbus/modbus.h>
#include "logger/logger.h"
#include "gcom_config.h"
#include "gcom_iothread.h"
#include "gcom_modbus_decode.h"
#include "gcom_fims.h"
#include "gcom_timer.h"

int IO_Work::wnum = 0;

void discardGroupCallback(struct PubGroup &pub_group, struct cfg &myCfg, bool ok);
void addTimeStamp(std::stringstream &ss);
int check_socket_alive(std::shared_ptr<IO_Thread> io_thread, int timeout_sec, int timeout_usec);


// function code is invalid.EMBXILFUN  112345679
// specified address is invalid. EMBXILADD 112345680
// data is invalid               EMBXILVAL 112345681
// Modbus slave is stopped.      EMBXSFAIL 112345682
// ACK response                  EMBXACK 112345683
// Modbus slave is busy          EMBXSBUSY 112345684
// Negative Ack response         EMBXNACK 112345685
// Memory parity error.          EMBXMEMPAR 112345686
// Bad gateway config            EMBXGPATH 112345687
// No response from target       EMBXGTAR 112345688
// CRC error.                    EMBBADCRC 112345689
// data is invalid.              EMBBADDATA  112345690
// unexpected exception code     EMBBADEXC 112345691
// Reserved code                 EMBUNKEXC 112345692
// Exceeds max data size.        EMBMDATA 112345693
// Device ID different.          EMBBADSLAVE 112345694



#define BAD_DATA_ADDRESS 112345680
#define INVALID_DATA     112345681


// #include "oldmodbus_iothread.h"

using namespace std::chrono_literals;

struct ThreadControl;

std::mutex io_output_mutex;
extern std::mutex logger_mutex;


struct IO_Work;
// Global Channel Definitions
ioChannel<std::shared_ptr<IO_Work>> io_pollChan;     // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_setChan;      // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_localpollChan;     // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_localsetChan;      // Use Channel to send IO-Work to thread
ioChannel<std::shared_ptr<IO_Work>> io_responseChan; // Thread picks up IO_work and processes it
ioChannel<std::shared_ptr<IO_Work>> io_poolChan;     // Response channel returns io_work to the pool
ioChannel<int> io_threadChan;                        // Thread Control
ioChannel<int> io_localthreadChan;                        // Thread Control


ThreadControl threadControl;

// if OkToConnect(threadControl)
bool OkToConnect(ThreadControl &tc, double tNow)
{
    std::lock_guard<std::mutex> lock2(tc.connect_mutex);
    if (tNow < tc.tConnect)
        return false;
    return true;
}

void  DelayConnect(struct cfg &myCfg, ThreadControl &tc)
{
    double tNow = get_time_double();
    std::lock_guard<std::mutex> lock2(tc.connect_mutex);
    tc.tConnect = tNow + myCfg.reconnect_delay;
}

void  DisConnect(struct cfg &myCfg, ThreadControl &tc, std::shared_ptr<IO_Thread> io_thread)
{
    // emit_event
    if(io_thread->connected)
    {
 
        char message[1024];
        snprintf(message, 1024, "Modbus Client [%s] Thread id %d Disconnecting from [%s] on port [%d]",
                myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);

        tc.num_connected_threads--;
        io_thread->connected = false;
    }
}

void  SetConnect(struct cfg &myCfg, ThreadControl &tc,std::shared_ptr<IO_Thread> io_thread)
{
    if(!io_thread->connected)
    {
        tc.num_connected_threads++;
        io_thread->connected = true;
        char message[1024];
        snprintf(message, 1024, "Modbus Client [%s] Thread id %d Connecting to[%s] on port [%d]",
                myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);

    }
}

int GetNumThreads(struct cfg &myCfg, ThreadControl &tc)
{
    std::lock_guard<std::mutex> lock2(tc.connect_mutex);
    return tc.num_connected_threads;
}

int GetNumThreads(struct cfg *myCfg)
{
    std::lock_guard<std::mutex> lock2(threadControl.connect_mutex);
    return threadControl.num_connected_threads;

}

void getConnectTimes(std::stringstream &ss, bool include_key)
{
    if (include_key)
    {
        ss << "\"thread_connection_info\": ";
    }
    ss << "[";
    bool first = true;
    for (auto &io_thread : threadControl.ioThreadPool)
    {
        // if(io_thread->is_local)
        //     continue;
        //if (io_thread->ctx != nullptr)
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
            
            if(!io_thread->is_local)
            {
                ss << ",\"connected\":"                    << (io_thread->connected?"true":"false") << ",";
                ss << "\"ip_address\": \""                << io_thread->ip.c_str()<< "\",";
                ss << "\"port\":" << io_thread->port      << ",";
                ss << "\"time_to_connect\":\""            << io_thread->connect_time << " ms\",";
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
void handleEBADF(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
        //int mberr = 0;
    if (io_thread->ctx)
    {
        FPS_ERROR_LOG("thread_id %d  try %d  ctx %p  EBADF ",
                        io_thread->tid, io_tries,
                        (void *)io_thread->ctx
                        );
    }
    DisConnect(myCfg, threadControl, io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Specific handling for EBADF error
void handleECONNRESET(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    if (io_thread->ctx)
    {
        FPS_ERROR_LOG("thread_id %d  try %d  ctx %p  ECONNRESET ",
                        io_thread->tid, io_tries,
                        (void *)io_thread->ctx
                        );
    }
    DisConnect(myCfg, threadControl, io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Specific handling for ETIMEDOUT error
void handleETIMEDOUT(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    //int mberr = 0;
    if (io_thread->ctx)
    {
        FPS_ERROR_LOG("thread_id %d  try %d  timeout ",
                        io_thread->tid, io_tries
                        //(void *)io_thread->ctx,
                        );
    }

    DisConnect(myCfg, threadControl, io_thread);
    // fast retry on timed out
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

//TODO use the proper connect function
// 115
void handleEINPROGRESS(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    if ( io_thread->connect_fails < 5)
    {
        io_thread->connect_fails++;
        FPS_ERROR_LOG("thread_id %d  try %d  EINPROGRESS",
                    io_thread->tid, io_tries
        );
    }
    if(io_work->data_error == false)
    {
        io_work->data_error = true;
        if (io_thread->wasConnected || io_thread->hadContext)
        {
            FPS_ERROR_LOG("thread_id %d  tries %d  broken pipe errno_code %d was Connected %s had Context %s isConnected %s"
                , io_thread->tid, io_tries
                , io_work->errno_code
                , (io_thread->wasConnected?"true":"false")
                , (io_thread->hadContext?"true":"false")
                , (io_thread->connected?"true":"false")
                        //(void *)io_thread->ctx,
                );
        }
    }


    DisConnect(myCfg, threadControl, io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}


// 88
void handleENOTSOCK(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    int mberr = 0;
    DisConnect(myCfg, threadControl, io_thread);


    if ( io_thread->connect_fails < 1)
    {
        io_thread->connect_fails++;
        FPS_INFO_LOG("thread_id %d  tries %d ENOTSOCK %p  mberr %d",
                        io_thread->tid, io_tries,
                        (void *)io_thread->ctx,
                        mberr);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

//    // INVALID_DATA 112345681
void handleINVALID_DATA(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    FPS_INFO_LOG("thread_id %d  tries %d ; Invalid Data Value in %d to %d , aborting for now",
                    io_thread->tid, io_tries,
                    io_work->offset,
                    (io_work->offset + io_work->num_registers)

                    //(void *)io_thread->ctx,
    );
}

// BAD_DATA_ADDRESS)
void handleBAD_DATA_ADDRESS(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    FPS_INFO_LOG("thread_id %d  tries %d ; Bad Data Address in %d to %d , aborting for now",
                    io_thread->tid, io_tries,
                    io_work->offset,
                    (io_work->offset + io_work->num_registers)

                    //(void *)io_thread->ctx,
    );
}

// EMBBADEXC 112345691
void handleEMBBADEXC(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    auto mberr = modbus_flush(io_thread->ctx);
    {
        FPS_INFO_LOG("thread_id %d  tries %d flush   ctx %p mberr %d",
                        io_thread->tid, io_tries,
                        (void *)io_thread->ctx,
                        mberr);
    }
}

// 22
void handleEINVAL(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work,int &io_tries, struct cfg &myCfg)
{
    
    if(io_thread->ctx)
    {
        DisConnect(myCfg, threadControl, io_thread);
    }
    //std::cout<< std::dec << " error EINVAL 22:" << 0<<std::endl;
    io_work->data_error = true;

    if(io_thread->wasConnected || io_thread->hadContext)
    {

        FPS_ERROR_LOG("thread_id %d  tries %d  was_connected %s hadContext %s connected %s error 22 EINVAL"
                    , io_thread->tid
                    , io_tries
                    , (io_thread->wasConnected?"true":"false")
                    , (io_thread->hadContext?"true":"false")
                    , (io_thread->connected?"true":"false")
                    //(void *)io_thread->ctx,
                    );
    }
}

// 32
void handleEPIPE(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg)
{
    if(io_thread->ctx)
    {
        DisConnect(myCfg, threadControl, io_thread);

    }
    if(io_work->data_error == false)
    {
        io_work->data_error = true;
        if (io_thread->wasConnected || io_thread->hadContext)
        {
            FPS_ERROR_LOG("thread_id %d  tries %d  broken pipe errno_code %d was Connected %s had Context %s isConnected %s"
                , io_thread->tid, io_tries
                , io_work->errno_code
                , (io_thread->wasConnected?"true":"false")
                , (io_thread->hadContext?"true":"false")
                , (io_thread->connected?"true":"false")
                        //(void *)io_thread->ctx,
                );
        }
    }
}    

void handleDefaultError(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries)
{
    if(io_thread->wasConnected || io_thread->hadContext)
    {
     
        FPS_INFO_LOG("thread_id %d  tries %d errno_code %d wasConnected %s hadContext %s isConnected %s"
                    , io_thread->tid, io_tries
                     //(void *)io_thread->ctx,
                    , io_work->errno_code
                    , (io_thread->wasConnected?"true":"false")
                    , (io_thread->hadContext?"true":"false")
                    , (io_thread->connected?"true":"false")
                    );
        FPS_LOG_IT("thread_io_error");
            
    }
    // Default error handling
}


void handleErrorCode(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> &io_work, int &io_tries, struct cfg &myCfg,  bool &io_done)
{
    io_thread->fails++;
    if(0)std::cout << __func__ << " error code "<<io_work->errno_code << std::endl;
    switch (io_work->errno_code)
    {
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

void tryReconnectIfNeeded(std::shared_ptr<IO_Thread> &io_thread, std::shared_ptr<IO_Work> io_work, int &io_tries, struct cfg &myCfg, bool debug)
{
    if (io_thread->connected && io_thread->ctx == nullptr && !io_thread->ip.empty())
    {
        for (int i = 0; i < 2 && io_thread->ctx == nullptr; ++i)
        {
            double ctime = SetupModbusForThread(myCfg, io_thread, debug);
            if (!io_thread->ctx)
            {
                if(1)FPS_ERROR_LOG("thread_id %d  connect %s try %d Connection attempt failed ",
                                io_thread->tid, (io_thread->connected?"true":"false"), io_tries);
                io_thread->connected = false;


                //LOG_ERROR("Connection attempt failed", io_thread->tid, io_tries);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            else
            {
                FPS_INFO_LOG("thread_id %d  connect %s try %d Connection attempt passed, ctime %2.3f ",
                                io_thread->tid, (io_thread->connected?"true":"false"), io_tries, ctime);
                io_work->cTime = ctime;

                //LOG_INFO("Connection attempt passed", io_thread->tid, io_tries, ctime);
            }
        }
    }
}

// may not be used
void new_runThreadError(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool &io_done, int &io_tries, int &max_io_tries, bool debug)
{
    io_thread->wasConnected = io_thread->connected;
    io_thread->hadContext = (io_thread->ctx != nullptr);

    if(io_thread->wasConnected)
    {
        tryReconnectIfNeeded(io_thread, io_work, io_tries, myCfg, debug);
    }
        // if (!io_thread->ctx && !io_work->local)
        // {
        //     io_work->data_error = true;
        //     return;
        // }

    handleErrorCode(io_thread, io_work, io_tries, myCfg, io_done);
}

// ///////////////////////////////////////////////////////////////////////////////////
void runThreadError(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool &io_done, int &io_tries, int &max_io_tries, bool debug)
{
    io_thread->wasConnected = io_thread->connected;
    io_thread->hadContext = (io_thread->ctx != nullptr);

    auto wasConnected = io_thread->connected;
    //auto hadContext = (io_thread->ctx!= nullptr); 
    // if it was connected try to reconnect
    if(wasConnected)
    {
        if(0)
            std::cout << " was connected .. io_work " << io_work->mynum 
                        << " error code :" << io_work->errno_code
                        << std::endl;

        if (io_thread->ctx == nullptr && io_thread->ip != "")
        {
            for (int i = 0; i < 2 && io_thread->ctx == nullptr; ++i)
            {
                double ctime;
                {
                    //if (debug)
                    //    std::cout << " xxx getting modbus ctx #1 " << std::endl;
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
                    if (0)
                        std::cout << " was connected .. io_work " << io_work->mynum << "   done ModbusSetup; connect_time :" << ctime << std::endl;
                }
                if (!io_thread->ctx)
                {
                    {
                        if(1)FPS_ERROR_LOG("thread_id %d  connect %s try %d Connection attempt failed ",
                                io_thread->tid, (io_thread->connected?"true":"false"), io_tries);
                    }
                    io_thread->connected = false;

                    //printf("thread_id %d  connect %s try %d Connection attempt failed ",
                    //            io_thread->tid, (io_thread->connected?"true":"false"), io_tries);
                    // int mberr = 0;
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
                else
                {
                    FPS_INFO_LOG("thread_id %d  connect %s try %d Connection attempt passed, ctime %2.3f ",
                                io_thread->tid, (io_thread->connected?"true":"false"), io_tries, ctime);
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

    io_thread->fails++;
    if(0)
        std::cout << " was not connected .. io_work " << io_work->mynum 
                << "   local  :" << io_work->local 
                << "   data_error  :" << io_work->data_error 
                << "   error code  :" << io_work->errno_code 
                << std::endl;
    switch (io_work->errno_code)
    {
        case 0:
            if (!io_thread->ctx && !io_work->local)
            {
                io_work->data_error = true;
            }
            //handleEBADF(io_thread, io_work, io_tries, myCfg);
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

// in all of these we'll tackle the single / multi option, the off by one  and the auto disable
// the forced / unforced / debounce value option should have been taken care of already
// the local / remote io_work option will be handled here

void local_write_registers(struct cfg &myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        {
            std::unique_lock<std::mutex> lock(io_point->mtx); 
            memcpy(io_point->reg16, &io_work->buf16[idx], io_point->size * 2);
        }
        idx += io_point->size;
    }
    io_work->errors = idx;
}

void local_read_registers(struct cfg &myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        {
            std::unique_lock<std::mutex> lock(io_point->mtx); 
            memcpy(&io_work->buf16[idx], io_point->reg16, io_point->size * 2);
        }
        idx += io_point->size;
    }
    io_work->errors = idx;
}

void local_write_bits(struct cfg &myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        {
            std::unique_lock<std::mutex> lock(io_point->mtx); 
            memcpy(io_point->reg8, &io_work->buf8[idx], io_point->size);
        }
        idx += io_point->size;
    }
    io_work->errors = idx;
}

void local_read_bits(struct cfg &myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    int idx = 0;
    for (auto io_point : io_work->io_points)
    {
        {
            std::unique_lock<std::mutex> lock(io_point->mtx); 
            memcpy(&io_work->buf8[idx], io_point->reg8, io_point->size);
        }
        idx += io_point->size;
    }
    io_work->errors = idx;
}

void local_write_work(struct cfg &myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if ((io_work->register_type == cfg::Register_Types::Holding) || (io_work->register_type == cfg::Register_Types::Input))
        local_write_registers(myCfg, io_work, debug);
    else if ((io_work->register_type == cfg::Register_Types::Coil) || (io_work->register_type == cfg::Register_Types::Discrete_Input))
        local_write_bits(myCfg, io_work, debug);
}

void local_read_work(struct cfg &myCfg, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if ((io_work->register_type == cfg::Register_Types::Holding) || (io_work->register_type == cfg::Register_Types::Input))
        local_read_registers(myCfg, io_work, debug);
    else if ((io_work->register_type == cfg::Register_Types::Coil) || (io_work->register_type == cfg::Register_Types::Discrete_Input))
        local_read_bits(myCfg, io_work, debug);
}

bool handle_lost_connection(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work
                        , const char *func_name, int err)
{
    bool lost = false;
    auto wasConnected = io_thread->connected;
    auto hadContext = (io_thread->ctx!= nullptr); 

    if ( err == EPIPE || err == EINVAL)
    {
        lost = true;
        // we need to disconnect
        if (io_thread->ctx)
        {
            DisConnect(myCfg, threadControl, io_thread);

            io_work->data_error = true;
            io_thread->connected = false;
            {

                FPS_ERROR_LOG("thread_id %d  errno_code %d wasConnected %s hadContext %s isConnected %s"
                    , io_thread->tid
                    , io_work->errno_code
                    , (wasConnected?"true":"false")
                    , (hadContext?"true":"false")
                    , (io_thread->connected?"true":"false")
                    );
                }
        }
    }
    return lost;
}

void handle_point_error(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work
                        , std::shared_ptr<struct cfg::io_point_struct> io_point, const char *func_name, int err)
{
    io_point->errno_code = err;
    // do not disable thread id 1
    if (io_point->errno_code == ECONNRESET) 
    {
        io_thread->connect_reset++;
        if(0)FPS_INFO_LOG(
                    "thread_id %d %s enabled connect reset now %d,for [%s] offset %d  err code %d [%s]"
                    , io_thread->tid, func_name, io_thread->connect_reset, io_point->id.c_str()
                    , io_point->offset, io_point->errno_code,  modbus_strerror(io_point->errno_code));

        if((io_thread->tid > 1) && (io_thread->connect_reset> 10))
        {
            if(io_thread->is_enabled)
            {
                if(0)FPS_INFO_LOG(
                    "thread_id %d %s enabled but connect reset now %d, thread now disabled ,for [%s] offset %d  err code %d [%s]"
                    , io_thread->tid, func_name, io_thread->connect_reset, io_point->id.c_str()
                    , io_point->offset, io_point->errno_code,  modbus_strerror(io_point->errno_code));

                char message[1024];
                snprintf(message, 1024, 
                    "thread_id %d %s enabled but connect reset now %d, thread now disabled ,for [%s] offset %d  err code %d [%s]"
                        , io_thread->tid, func_name, io_thread->connect_reset, io_point->id.c_str()
                        , io_point->offset, io_point->errno_code,  modbus_strerror(io_point->errno_code));

                FPS_INFO_LOG("%s", message);
                emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);

                io_thread->is_enabled = false;
            }

        }
    }
    if ((io_point->errno_code == EPIPE) 
        || (io_point->errno_code == ECONNRESET) 
        || (io_point->errno_code == EINVAL)
        || (io_point->errno_code == EAGAIN)
        || (io_point->errno_code == ETIMEDOUT)
        )
    {
        // if we get one of these then the rest of the batch will fail.

        if(!io_work->data_error)
        {
            FPS_INFO_LOG("thread_id %d %s connected but io failed for [%s] offset %d  err code %d [%s] point off_by_one [%s]"
                , io_thread->tid, func_name, io_point->id.c_str()
                , io_point->offset, io_point->errno_code,  modbus_strerror(io_point->errno_code)
                , io_point->off_by_one?"true":"false");
            io_work->data_error = true;
        }
    }

    if(err >= 112345600)
    {
        // have we already failed
        if (!io_work->data_error)
        {

            FPS_INFO_LOG("thread_id %d %s connected but io failed 2 for [%s] offset %d  err %d code %d [%s] point off_by_one [%s] disconnected [%s] auto disable [%s] point_gap %d"
                , io_thread->tid, func_name, io_point->id.c_str(), io_point->offset, err, io_point->errno_code,  modbus_strerror(io_point->errno_code)
                                , io_point->off_by_one?"true":"false"
                                , io_point->is_disconnected?"true":"false"
                                , myCfg.auto_disable?"true":"false"
                                , io_point->gap
                                );

            io_work->data_error = true;
            if (io_point->errno_code == 112345691)
            {
                int io_tries = 0;
                handleEMBBADEXC(io_thread, io_work, io_tries, myCfg);
            }
        }
    }

    //#define EMBBADEXC 112345691  invalid data
    //#define EMBBADEXC 112345684 busy

    // only disable point if not one of these errors
    if (myCfg.auto_disable && !(
                                (err == EPIPE) 
                                || (err == ECONNRESET) 
                                || (err == EAGAIN)     // unavaliable
                                || (err == EINVAL) 
                                || (err == ETIMEDOUT) 
                                || (err == 112345691)  // Invalid Data
                                || (err == 112345684)
                                                                ))
    {
        // note this may be redundant
        if(io_point->is_disconnected == false) 
        {
            if(io_point->gap > 0)
            {
                FPS_INFO_LOG ("thread_id %d %s failed for [%s] offset %d with gap [%d] err %d -> [%s]; gap removed  ", 
                    io_thread->tid, func_name, io_point->id.c_str(), io_point->offset, io_point->gap, err, modbus_strerror(io_point->errno_code));
                io_point->gap = 0;
            } 
            else
            {

//            io_point->is_enabled = false;
                io_point->is_disconnected = true;
                double tNow = get_time_double();
                io_point->reconnect = tNow + 5.0;


                FPS_INFO_LOG 
                        ("thread_id %d %s failed for [%s] offset %d err %d -> [%s] point disconnected", 
                        io_thread->tid, func_name, io_point->id.c_str(), io_point->offset, err, modbus_strerror(io_point->errno_code));
            }
        }
    }
    else
    {
        // this will spam so dont send it
        if(0)FPS_INFO_LOG("thread_id %d %s failed for [%s]  offset %d err %d code %d [%s]", 
                io_thread->tid, func_name, io_point->id.c_str(), io_point->offset, err, io_point->errno_code, modbus_strerror(io_point->errno_code));
    }
}


void io_write_registers(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers * 2;
        return;
    }
    int offset = 0;
    bool allowMulti = myCfg.allow_multi_sets;
    if (io_work->off_by_one)
        offset++;
    if (allowMulti)
    {
        io_work->errors = modbus_write_registers(io_thread->ctx, io_work->offset - offset, io_work->num_registers, io_work->buf16);
    }
    auto err = errno;

    if (!handle_lost_connection(myCfg, io_thread, io_work, "write registers", err))
    {

        if (!allowMulti || io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_write_registers(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf16[idx]);
                if (errors < 0)
                {
                    err = errno;
                    handle_point_error(myCfg, io_thread, io_work, io_point, "write registers", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}

void io_write_register(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers * 2;
        return;
    }

    int offset = 0;
    int idx = 0;
    int errors = 0;
    io_work->errors = 0;
    if (io_work->off_by_one)
        offset++;
    // there should only be one register
    for (auto io_point : io_work->io_points)
    {
        if (io_point->multi_write_op_code || io_point->size > 1)
        {
            errors = modbus_write_registers(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf16[idx]);
            // std::cout << "write_registers errors " << errors << std::endl;
        }
        else
        {
            errors = modbus_write_register(io_thread->ctx, io_point->offset - offset, io_work->buf16[idx]);
            // std::cout << "io_point ["<<  io_point->id<< "] write_register error " << errors << std::endl;
        }
        auto err = errno;
        if (!handle_lost_connection(myCfg, io_thread, io_work, "write register", err))
        {

            if (errors < 0)
            {
                handle_point_error(myCfg, io_thread, io_work, io_point, "write register", err);
            }
            else
            {
                io_work->errors += errors;
            }

            idx += io_point->size;
        }
        else
        {
            break;
        }
    }
}

void io_read_registers(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers * 2;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;

    io_work->errors = modbus_read_registers(io_thread->ctx, io_work->offset - offset, io_work->num_registers, io_work->buf16);
    auto err = errno;
    if (!handle_lost_connection(myCfg, io_thread, io_work, "read registers", err))
    {

        if (io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_read_registers(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf16[idx]);

                if (errors < 0)
                {
                    err = errno;
                    handle_point_error(myCfg, io_thread, io_work, io_point, "read registers", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}


void io_read_input_registers(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    // if(io_work->local)
    // {
    //     FPS_INFO_LOG("thread_id %d Get >> Input : offset %d num %d local %s "
    //                                 , io_thread->tid, io_work->offset, io_work->num_registers, (io_work->local?"true":"false"));
    // }

    if (io_work->local)
    {
        io_work->errors = io_work->num_registers * 2;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;

    io_work->errors = modbus_read_input_registers(io_thread->ctx, io_work->offset - offset, io_work->num_registers, io_work->buf16);
    auto err = errno;
    if(io_work->errors < 0)
    {
        if (!handle_lost_connection(myCfg, io_thread, io_work,  "read input registers", err))
        {
            if (io_work->errors < 0)
            {
                io_work->errno_code = errno;
                // FPS_INFO_LOG("thread_id %d first read input_registers failed  offset %d  num %d off_by_one %d  err %s"
                //                 , io_thread->tid, io_work->offset, io_work->num_registers , offset, modbus_strerror(io_work->errno_code)) ;
                io_work->errors = 0;
                int idx = 0;
                int errors;
                for (auto io_point : io_work->io_points)
                {
                    errors = modbus_read_input_registers(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf16[idx]);
                    if (errors < 0)
                    {
                        err = errno;
                        handle_point_error(myCfg, io_thread, io_work, io_point, "read input registers", err);
                    }
                    else
                    {
                        io_work->errors += errors;
                    }
                    idx += io_point->size;
                }
            }
        }
    }
}

void io_read_bit(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    io_work->errors = modbus_read_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers, &io_work->buf8[0]);
    auto err = errno;
    
    if (!handle_lost_connection(myCfg, io_thread, io_work, "read bits", err))
    {

        if (io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_read_bits(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf8[idx]);
                if (errors < 0)
                {
                    err = errno;
                    handle_point_error(myCfg, io_thread, io_work, io_point, "read bits", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}

void io_write_bit(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    io_work->errors = modbus_write_bit(io_thread->ctx, io_work->offset - offset, io_work->buf8[0]);
    auto err = errno;
    if (!handle_lost_connection(myCfg, io_thread, io_work, "write bit", err))
    {
        if (io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_write_bit(io_thread->ctx, io_point->offset - offset, io_work->buf8[idx]);
                if (errors < 0)
                {
                    err = errno;
                    handle_point_error(myCfg, io_thread, io_work, io_point, "write bit", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}

void io_write_bits(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    io_work->errors = modbus_write_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers, io_work->buf8);
    auto err = errno;
    if (!handle_lost_connection(myCfg, io_thread, io_work, "write bits", err))
    {
    
        if (io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_write_bits(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf8[idx]);
                if (errors < 0)
                {
                    err = errno;
                    handle_point_error(myCfg, io_thread, io_work, io_point, "write bits", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}

void io_read_bits(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    io_work->errors = modbus_read_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers, io_work->buf8);
    auto err = errno;
    if (!handle_lost_connection(myCfg, io_thread, io_work, "read bits", err))
    {
        if (io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_read_bits(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf8[idx]);
                if (errors < 0)
                {
                    err = errno;
                    handle_point_error(myCfg, io_thread, io_work, io_point, "read bits", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}

void io_read_input_bits(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    if (io_work->local)
    {
        io_work->errors = io_work->num_registers;
        return;
    }
    int offset = 0;
    if (io_work->off_by_one)
        offset++;
    io_work->errors = modbus_read_input_bits(io_thread->ctx, io_work->offset - offset, io_work->num_registers, io_work->buf8);
    auto err = errno;
    if (!handle_lost_connection(myCfg, io_thread, io_work, "read input bits", err))
    {

        if (io_work->errors < 0)
        {
            io_work->errors = 0;
            int idx = 0;
            int errors;
            for (auto io_point : io_work->io_points)
            {
                errors = modbus_read_input_bits(io_thread->ctx, io_point->offset - offset, io_point->size, &io_work->buf8[idx]);
                err = errno;
                if (errors < 0)
                {
                    handle_point_error(myCfg, io_thread, io_work, io_point, "read input bits", err);
                }
                else
                {
                    io_work->errors += errors;
                }
                idx += io_point->size;
            }
        }
    }
}


// this is to process an io_work object
// When the thread starts it tries to connect 
// 
void runThreadWork(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, std::shared_ptr<IO_Work> io_work, bool debug)
{
    double tNow = get_time_double();
    io_work->tIo = tNow;
    int io_tries = 0;
    // int max_io_tries = 10;
    int max_io_tries = 1;//myCfg.max_io_tries;
    bool io_done = false;

    if(io_work->local)
    {
        //io_done = true;
    }
    else
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


    // if(!io_work->local)
    // {
    //     if (!io_thread->connected)
    //     {
    //         //runThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);
    //         io_done = true;
    //         io_work->data_error = true;
    //     }
    // }
    // if (io_thread->ctx && io_thread->connected)
    // {
    //     modbus_set_slave(io_thread->ctx, io_work->device_id);
    // }
    // else
    // {
    //     if(!io_work->local)
    //          io_work->data_error = true;
    //     io_done = true;
    // }

    // TODO check this
    // when connected the modbus timeout is set to transfer time
    // if the modbus transaction times out the modbus is disconnected 
    // we can try a reconnect at the same transfer time but if that fails then we need to use the delayed 
    // connect concept.
    // a network glitch will be either short or longer.
    // the timeout could also be caused by a sluggish modbus server.


    // when not connected we can try one more time at the same timeout 
    max_io_tries = 2;// myCfg.max_io_tries;
    
    if(!io_work->local)
    {
        std::unique_lock<std::mutex> lock(io_thread->stat_mtx); 

        if (io_work->wtype == WorkTypes::Set)
        {
            io_thread->modbus_write_timer.start();
        }
        else
        {
            io_thread->modbus_read_timer.start();
        }
    }
    io_work->threadId = io_thread->tid;
    io_thread->jobs++;
    if(0)printf("thread_id %d   >> offset %d num %d errors %d \n"
                                    , io_thread->tid, io_work->offset, io_work->num_registers, io_work->errors);


    while (!io_done && (io_tries < max_io_tries))
    {
        io_tries++;
        if(io_tries > 1)
        {
            FPS_INFO_LOG("thread_id %d running retry %d ; work type %d (%s)  offset %d num %d reg_type %d  (%s) ",
                io_thread->tid, io_tries, (int)io_work->wtype, workTypeToStr(io_work->wtype).c_str(), io_work->offset,
                io_work->num_registers, /*(int)setMulti,*/ (int)io_work->register_type, regTypeToStr(io_work->register_type).c_str());
        }

        io_work->errors = -2; // flag a no go
        bool setMulti = myCfg.force_multi_sets;
 
        if (io_work->wtype == WorkTypes::Set)
        {
            if (0)
            {
                FPS_INFO_LOG("thread_id %d Set >> work type %d (%s)  offset %d num %d multi %d reg_type %d  (%s) ",
                             io_thread->tid, (int)io_work->wtype, workTypeToStr(io_work->wtype).c_str(), io_work->offset,
                             io_work->num_registers, (int)setMulti, (int)io_work->register_type, regTypeToStr(io_work->register_type).c_str());
            }
            if (io_work->register_type == cfg::Register_Types::Coil)
            {
                if ((io_work->num_registers == 1) && !setMulti)
                {
                    io_write_bit(myCfg, io_thread, io_work, debug);
                        // io_work->errors = modbus_write_bit(io_thread->ctx, io_work->offset, io_work->buf8[0]);
                }
                else
                {
                    io_write_bits(myCfg, io_thread, io_work, debug);
                        // io_work->errors = modbus_write_bits(io_thread->ctx, io_work->offset, io_work->num_registers, io_work->buf8);
                }
            }
 
            else if (io_work->register_type == cfg::Register_Types::Holding)
            {
                if ((io_work->num_registers == 1) && !setMulti)
                {
                    io_write_register(myCfg, io_thread, io_work, debug);
                }
                else
                {
                    io_write_registers(myCfg, io_thread, io_work, debug);
                }
                if (0)
                {
                    FPS_INFO_LOG("thread_id %d Set >> Holding : offset %d errors %d ", io_thread->tid, io_work->offset, io_work->errors);
                }
            }
            else
            {
                FPS_INFO_LOG("Set: unknown register type");
            }
        }
        else if ((io_work->wtype == WorkTypes::Get) || (io_work->wtype == WorkTypes::Poll))
        {
            if (io_work->register_type == cfg::Register_Types::Holding)
            {

                io_read_registers(myCfg, io_thread, io_work, debug);
                
                // io_work->errors = modbus_read_registers(io_thread->ctx, io_work->offset, io_work->num_registers, io_work->buf16);
                if (0)
                {
                    FPS_INFO_LOG("thread_id %d Get >> Holding : errors %d ", io_thread->tid, io_work->errors);
                }
            }
            else if (io_work->register_type == cfg::Register_Types::Coil)
            {
                if (io_work->num_registers == 1)
                {
                    io_read_bit(myCfg, io_thread, io_work, debug);
                }
                else
                {
                    io_read_bits(myCfg, io_thread, io_work, debug);
                }
                // io_work->errors = modbus_read_bits(io_thread->ctx, io_work->offset, io_work->num_registers, io_work->buf8);
            }
            else if (io_work->register_type == cfg::Register_Types::Input)
            {
                io_read_input_registers(myCfg, io_thread, io_work, debug);
                // io_work->errors = modbus_read_input_registers(io_thread->ctx, io_work->offset, io_work->num_registers, io_work->buf16);
                if (myCfg.connection.debug)
                {
                    FPS_INFO_LOG("thread_id %d Get >> Input : offset %d num %d errors %d "
                                    , io_thread->tid, io_work->offset, io_work->num_registers, io_work->errors);
                }
            }
            else if (io_work->register_type == cfg::Register_Types::Discrete_Input)
            {
                io_read_input_bits(myCfg, io_thread, io_work, debug);
            }
            else
            {
                FPS_INFO_LOG("Get/Poll: unknown register type");
            }

            //FPS_INFO_LOG
            if(0)printf("thread_id %d Get/Poll >> offset %d num %d errors %d \n"
                                    , io_thread->tid, io_work->offset, io_work->num_registers, io_work->errors);
        }
        // test mode random delay from 100 to 500 uSecs
        else if (io_work->wtype == WorkTypes::Noop)
        {
            io_done = true;

            randomDelay(100, 500);
            io_work->errors = 1;
        }
        else
        {
            FPS_INFO_LOG("Get/Poll: unknown work type");
        }

        double tRun = get_time_double() - tNow;
        #ifdef FPS_DEBUG_MODE
        if ((io_work->work_name == "test_io_point_single") && (io_work->test_it))
        {
            // make it worse if we are testing
            std::cout << "adding an extra 1200ms to try to make this late\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        }
        #endif
        if (io_work->errors >= 0)
        {
            io_done = true;
        }

        io_work->errno_code = errno;
        io_work->tRun = tRun;

        if (debug || io_work->errors < 0)
        {
            if(io_work->errno_code == EPIPE)
                io_done = true;

            if (io_work->errors < 0)
            {
                FPS_ERROR_LOG("thread_id %d  connected %s local %s try %d type %d offset %d error %d code  %d -> %s "
                            , io_thread->tid
                            , io_thread->connected? "true":"false"
                            , io_work->local? "true":"false"
                            , io_tries
                            , (int)io_work->register_type
                            , io_work->offset
                              //(void *)io_thread->ctx,
                            , io_work->errors
                            , io_work->errno_code
                            , modbus_strerror(io_work->errno_code));
            }
            else
            {
                if (myCfg.connection.debug)
                    FPS_INFO_LOG("thread_id %d Success, try %d type %d offset %d num %d tRun(mS) %f inreg [%d] inbit [%d] ", io_thread->tid, io_tries, (int)io_work->register_type, io_work->offset, io_work->errors, tRun * 1000.0, io_work->buf16[0], io_work->buf8[0]);
            }
        }
        if ((io_work->errors > 0) || (io_work->errors == -2))
        { // got successful communication
            // io_tries = max_io_tries;
            io_done = true;
        }
        else
        {
            // if (io_work->test_range) return false
            //std::cout << " runthreaderror for io_work num " << io_work->mynum << " "<<std::endl;
            // this will try a reconnect
            runThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);
        }
    }
    io_work->tDone = get_time_double();

    //TODO cannot use this approach .. we have the io_work timer here we'll have to send collect the results in the response thread ( single thread)
    if(!io_work->local)
    {
        std::unique_lock<std::mutex> lock(io_thread->stat_mtx); 
        if (io_work->wtype == WorkTypes::Set)
        {
            io_thread->modbus_write_timer.snap();
 
        }
        else
        {
            io_thread->modbus_read_timer.snap();
        }
    }
    if(0)std::cout << " io_work : " << io_work->mynum
            << " sent to resp channel"
            << std::endl;

    io_responseChan.send(std::move(io_work));
}


// thread function
void ioThreadFunc(ThreadControl &control, struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread)
{
    // int id = tid;
    std::shared_ptr<IO_Work> io_work;
    int signal;
    bool run = true;
    bool debug = true;
    double delay = 0.1;

    if (!io_thread->is_local)
    {
        std::cout << " thread_id " << io_thread->tid << "   loop started; enabled :"<< io_thread->is_enabled << std::endl;
    }
    while (run && control.ioThreadRunning && io_thread->is_enabled)
    {
        if (!io_thread->is_local)
        {
            if (!io_thread->connected)
            {
                if ((myCfg.connection.serial_device == "") && (myCfg.connection.ip_address == ""))
                {

                    FPS_ERROR_LOG("thread_id [%d]: Config problem. Unable to set up modbus.", io_thread->tid);
                    run = false;
                    return;
                }
            
                for (int i = 0; i < 2 && !io_thread->connected; ++i)
                {
                    double ctime = SetupModbusForThread(myCfg, io_thread, debug);
                    if(0)std::cout << " thread_id " << io_thread->tid << "   done  attempt 2 ModbusSetup; connect_time :"<< ctime << std::endl;

                    if(ctime > 0.0)
                    {
                        if(0)std::cout << " thread_id " << io_thread->tid << "   done ModbusSetup; connect_time :"<< ctime << std::endl;
                        io_thread->cTime = ctime; // TODO use perf
                    }
                }
            }

            int num_threads = GetNumThreads(myCfg, threadControl);

            // this allows remote io_work objects to be collected in the case of a server disconnect
            // this causes problems 
            // dont try to process stuff unless we are connected
            // or process stuff but discard io_work we accept and then become disconnected (error = 32)

            if((io_thread->is_enabled) && (io_thread->connected || num_threads == 0))
            {
                if (io_threadChan.receive(signal, delay))
                {
                    // from testThread
                    if (signal == 2)
                    {
                       if(0)printf(" >>>>>>>>>>>>>>>>%s testThread signal received \n",__func__);
                        if (check_socket_alive(io_thread, /*timeout_sec*/ 0 , /* timeout_usec*/ 20) > 0)
                        {
                            if(0)printf(" >>>>>>>>>>>>>>>>%s testThread shutting socket down \n",__func__);
                            CloseModbusForThread(io_thread, true);
                            DisConnect(myCfg, threadControl, io_thread);
                        }
                    }
                    // from killThread
                    if (signal == 3)
                    {
                        if(0)printf(" >>>>>>>>>>>>>>>>%s killThread shutting socket down \n",__func__);
                        CloseModbusForThread(io_thread, true);
                        DisConnect(myCfg, threadControl, io_thread);
                    }
                    if (signal == 0)
                        run = false;
                    if (signal == 1)
                    {

                        if (io_setChan.peekpop(io_work))
                        {
                            runThreadWork(myCfg, io_thread, io_work, debug);
                        }
                        if (io_pollChan.peekpop(io_work))
                        {
                            double stime = get_time_double();
                            if(0)std::cout << "Start  io_work " << io_work->mynum 
                                << " Thread_connected " << io_thread->connected
                                << " num_threads " << num_threads
                                << std::endl;
                            runThreadWork(myCfg, io_thread, io_work, debug);
                            
                            double etime = get_time_double();
                            if ((etime - stime) > io_thread->transfer_timeout)
                            {
                                //io_thread->transfer_timeout += 0.1;
                                std::cout << "Done  io_work " << io_work->mynum 
                                    << " Thread_connected " << io_thread->connected
                                    << " num_threads " << num_threads
                                    << " elapsed mS " << (etime - stime) * 1000 
                                    << " transaction time  mS " << io_thread->transfer_timeout * 1000 
                                    << std::endl;
                            }

                        }
                    }
                }
                // else
                // {
                //     if (check_socket_alive(io_thread, /*timeout_sec*/ 0 , /* timeout_usec*/ 20) > 0)
                //     {
                //         CloseModbusForThread(io_thread, true);
                //     }

                // }
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
    FPS_INFO_LOG("thread_id [%d]: Loop Completed; enabled [%s].", io_thread->tid, (char*)io_thread->is_enabled?"true":"false");
//    std::cout << " thread_id " << io_thread->tid << "   loop done; enabled :"<< io_thread->is_enabled << std::endl;

    // TODO move to cfg.log_lock;
    if (!io_thread->is_local)
    {
        CloseModbusForThread(io_thread, debug);
    }
    {
        char message[1024];

        snprintf(message, 1024, 
                    "thread_id %d stopping after %d jobs  %d fails, enabled %s "
                    , io_thread->tid, io_thread->jobs, io_thread->fails, (io_thread->is_enabled?"true":"false"));

        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);

    }

}

void clearpollChan(bool debug)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    if (debug)
    {
        std::cout << " Clearing pollChan" << std::endl;
    }
    while (io_pollChan.receive(io_work, delay))
    {
        // runThreadWork(io_thread, io_work, debug);
        if (debug)
        {
            std::cout
                << "  start " << io_work.get()->offset
                << "  number " << io_work.get()->num_registers
                << std::endl;
        }
    }
    if (debug)
    {
        std::cout << " Cleared pollChan" << std::endl;
    }
}

void clearsetChan(bool debug)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    if (debug)
    {
        std::cout << " Clearing setChan" << std::endl;
    }
    while (io_setChan.receive(io_work, delay))
    {
        // runThreadWork(io_thread, io_work, debug);
        if (debug)
        {
            std::cout
                << "  start " << io_work.get()->offset
                << "  number " << io_work.get()->num_registers
                << std::endl;
        }
    }
    if (debug)
    {
        std::cout << " Cleared setChan" << std::endl;
    }
}

void clearrespChan(bool debug)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    if (debug)
    {
        std::cout << " Clearing respChan" << std::endl;
    }
    while (io_responseChan.receive(io_work, delay))
    {
        // runThreadWork(io_thread, io_work, debug);
        if (debug)
        {
            std::cout
                << "  start " << io_work.get()->offset
                << "  number " << io_work.get()->num_registers
                << std::endl;
        }
    }
    if (debug)
    {
        std::cout << " Cleared responseChan" << std::endl;
    }
}
// fixes tStart
// test function we'll have to set up some test data
//
void sendpollChantoResp(bool debug)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    if (debug)
    {
        std::cout << " Pulling  pollChan" << std::endl;
    }
    while (io_pollChan.receive(io_work, delay))
    {
        io_work.get()->tStart = get_time_double();

        // io_responseChan.send(io_work);
        // runThreadWork(io_thread, io_work, debug);
        if (debug)
        {
            std::cout
                << "  start " << io_work.get()->offset
                << "  number " << io_work.get()->num_registers
                << std::endl;
        }
        io_responseChan.send(std::move(io_work));
    }
    if (debug)
    {
        std::cout << " Moved to resp channel" << std::endl;
    }
}

// here is the collector for the in flight pubgroups.
// struct PubGroup {
//     std::string key;
//     std::vector<std::shared_ptr<IO_Work>> works;
//     int work_group;
//     double tNow;
//     bool done;
//     bool erase;
// };

std::map<std::string, PubGroup> pubGroups;

// this decodes the io_work io_points once we have collected all the io_work objects
// and produce the fims output message

void processGroupCallback(struct PubGroup &pub_group, struct cfg &myCfg)
{
    // auto compsh = pub_group.component.lock();
    double tNow = get_time_double();
    bool debug = myCfg.connection.debug;
    // debug = true;
    if (debug)
        std::cout << " processing workgroup " << pub_group.key
                  << " created at :" << pub_group.tNow
                  << " process time mS " << (tNow - pub_group.tNow) * 1000.0
                  << std::endl;


    std::string pub_key = "pub_";
    std::string set_key = "set_";
    std::string get_key = "get_";

// if this is a pub response and we get io_work->errors == -3 that means we had no connection
    if (pub_group.key.substr(0, pub_key.length()) == pub_key)
    {
        //myCfg.performance_timers.pub_timer.start();
        pub_group.done = true;

        std::stringstream string_stream;
        std::string uri;
        bool first = true;
        int total_errors  = 0;
        string_stream << "{";
        //cfg::pub_struct *mypub;
        std::shared_ptr<cfg::pub_struct> mypub=nullptr;

        for (auto io_work : pub_group.works)
        {
            uri = "/" + io_work->component->component_id + "/" + io_work->component->id;
            // set this for the first object
            if (mypub == nullptr)
            {
                mypub = io_work->pub_struct;
                if(0)std::cout << " mypub completed name: " << io_work->work_name << " pending value :" << mypub->pending << std::endl;
                // if we get one we can reset
                if(mypub->pending > 1)
                {
                    if(0)std::cout << " mypub completed name: " << io_work->work_name << " pending value :" << mypub->pending << std::endl;
                    mypub->pend_timeout = 0.0;
                    mypub->kill_timeout = 0.0;
                    mypub->pending = 0;
                }
            }

            if (io_work->data_error)
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
                gcom_modbus_decode(io_work, string_stream, myCfg);
            }
        }

        // add the heartbeat / component connected fields
        // TODO make this optional 
        //if (total_errors == 0)
        {

            cfg::component_struct *component = pub_group.pub_struct->component;
            bool send_time = false;
            std::string state_str; 

            if(1||myCfg.use_new_wdog)
            {
                if (component->heartbeat_enabled && component->heartbeat){
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
                    string_stream << "\"heartbeat_state\":\"" << state_str <<"\"";

                    if (/*component->heartbeat->state_str == "INIT"||*/
                        component->heartbeat->state_str== "NORMAL")
                        state_str = "true";
                    else
                        state_str = "false";
                    //Bug 02_01_2024 use myhb->heartbeat_read_point
                    string_stream << ",\"modbus_heartbeat\":" << component->heartbeat->heartbeat_read_point->raw_val << "";
                    string_stream << ",\"component_connected\":" << state_str << "";
                    void hbSendWrite(cfg::heartbeat_struct *myhb);
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
            if(send_time)
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
                if ( myCfg.fims_gateway.Connected() ) {
                    send_pub(myCfg.fims_gateway, uri, string_stream.str());
                    myCfg.fims_connected = true;

                }
                else
                {
                    if (myCfg.fims_connected)
                    {
                        FPS_ERROR_LOG(" fims not connected for pub message");
                        myCfg.fims_connected =  false;
                        myCfg.keep_running = false;

                    }
                }
            }
            mypub->pubStats.snap(mypub->pmtx);

            //io_work->pub_struct = mypub;
            //myCfg.performance_timers.pub_timer.snap();
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
            //if (io_work->errors > 0)
            //{

                gcom_modbus_decode(io_work, string_stream, myCfg);
                store_raw_data(io_work, false);
                //gcom_modbus_decode(io_work, string_stream, myCfg);
            //}
            if (debug)
                std::cout << __func__ << " offset :" << io_work->offset << " string so far [[" << string_stream.str() << "]]" << std::endl;
        }
        string_stream << "}";

        if (send)
            send_set(myCfg.fims_gateway, uri, string_stream.str());
        discardGroupCallback(pub_group, myCfg, true);

    }
    else if (pub_group.key.substr(0, get_key.length()) == get_key)
    {
        pub_group.done = true;
        std::stringstream ss;
        std::string uri;
        bool first = true;
        bool send = true;
        ss << "{";
        for (auto io_work : pub_group.works)
        {
            if (debug)
                std::cout << __func__ << " offset :" << io_work->offset << " get string so far [[" << ss.str() << "]]" << std::endl;

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
            gcom_modbus_decode_debug(io_work, ss, myCfg, true, true);
            if (!io_work->local)
                local_write_work(myCfg, io_work, debug);
        }
        ss << "}";
        if (debug)
        {
            std::cout << __func__ << " get string at end [[" << ss.str() << "]]" << std::endl;
            std::cout << " uri [[" << uri << "]]" << std::endl;
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
                    FPS_ERROR_LOG(" fims not connected for set message");
                    myCfg.fims_connected = false;
                    myCfg.keep_running = false;

                }
            }
        }
        discardGroupCallback(pub_group, myCfg, true);
    }
}

// handles the io_work objects and puts them back in the pool
void discardGroupCallback(struct PubGroup &pub_group, struct cfg &myCfg, bool ok)
{
    double tNow = get_time_double();
    //std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> dropping pubgroup " << pub_group.key 
    //            << " created at :" << pub_group.tNow << " time elapsed (ms) " << ((tNow-pub_group.tNow)* 1000.0)  << std::endl;
    int num_groups = 0;
    int num_points = 0;
    for (auto io_work : pub_group.works)
    {
        num_groups++;
        num_points += io_work->num_registers;
        io_poolChan.send(std::move(io_work));
    }
    pub_group.works.clear();
    if(myCfg.debug_pub)
    {
        if (ok)
        {
            std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> completing   pubgroup " << pub_group.key 
                << " created at :" << pub_group.tNow 
                << " time elapsed (ms) " << ((tNow-pub_group.tNow)* 1000.0)  
                << " num_groups " << num_groups 
                << " num points " << num_points<< std::endl;
        }
        else
        {
            std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> dropping   pubgroup " << pub_group.key 
                << " created at :" << pub_group.tNow 
                << " time elapsed (ms) " << ((tNow-pub_group.tNow)* 1000.0)  
                << " num_groups " << num_groups 
                << " num points " << num_points<< std::endl;
        }
    }
}

// creates a new pubgroup key if needed
bool check_pubgroup_key(std::string key, std::shared_ptr<IO_Work> io_work)
{
    if ((!io_work->local) && (!io_work->data_error)){
        if (pubGroups.find(key) == pubGroups.end())
        {
            pubGroups[key] = PubGroup(key, io_work, false);

            FPS_INFO_LOG("Creating new pubGroup key [%s]", key.c_str());
        }
        return true;
    }
    return false;
}

// this is the main receiver of the io_work objects.
// they all come back to a common thread to make sure we have thread safe acced to the data points
// The local read / writes also occur here 
void processRespWork(std::shared_ptr<IO_Work> io_work, struct cfg &myCfg)
{

    std::string key = io_work->work_name; // + std::to_string(io_work->work_id);
    //std::cout << __func__ << " io_work->work_name " << key <<  std::endl;

    bool debug = myCfg.connection.debug;

    if (io_work->wtype == WorkTypes::Set) //  || (io_work->wtype == WorkTypes::SetMulti))
    {
        if (io_work->local)
        {
            if (debug)
                std::cout << "it was a local set" << std::endl;
            if ((io_work->register_type == cfg::Register_Types::Holding) || (io_work->register_type == cfg::Register_Types::Input))
            {
                local_write_registers(myCfg, io_work, debug);
            }
            else if ((io_work->register_type == cfg::Register_Types::Coil) || (io_work->register_type == cfg::Register_Types::Discrete_Input))
            {
                local_write_bits(myCfg, io_work, debug);
            }
        }
    }
    else if (io_work->wtype == WorkTypes::Get) //  || (io_work->wtype == WorkTypes::SetMulti))
    {
        if(0)std::cout << __func__ << " type get , io_work->work_name : " << key 
                << " work_group " << io_work->work_group
                << std::endl;

        if (io_work->local)
        {
            // if (debug ||( io_work->offset == 294))
            std::cout << "it is a local get" << std::endl;
            if ((io_work->register_type == cfg::Register_Types::Holding) || (io_work->register_type == cfg::Register_Types::Input))
            {
                local_read_registers(myCfg, io_work, debug);
            }
            else if ((io_work->register_type == cfg::Register_Types::Coil) || (io_work->register_type == cfg::Register_Types::Discrete_Input))
            {
                local_read_bits(myCfg, io_work, debug);
            }
        }
    }

    // if there is no key then this operation is not part of a group so we dont have to collect all the objects
    if (key == "")
    {
        std::string reply;
        if (debug)
            FPS_DEBUG_LOG("<%s> completing io_work, errors %d\ttNow: %f\treply: [%s]", __func__,
                                       io_work->errors, io_work->tNow, io_work->replyto.c_str());
        // is it a set ?
        if (io_work->wtype == WorkTypes::Set) //  || (io_work->wtype == WorkTypes::SetMulti))
        {

            // TODO we need the name and value here
            // std::stringstream string_stream;
            // gcom_modbus_decode_debug(io_work, string_stream, myCfg, true, false);
            // reply = "{\"gcom\":\"Modbus Set\",\"status\":\"Success\","+string_stream.str()+"}";

            // std::cout << " set result :" << reply << std::endl; 

            if (io_work->errors < 0)
                reply = "{\"gcom\":\"Modbus Set\",\"status\":\"Failed\"}";
            else
                reply = "{\"gcom\":\"Modbus Set\",\"status\":\"Success\"}";
            if (io_work->local)
            {
                store_raw_data(io_work, false);
                // local_write_registers
            }
            else
            {
                store_raw_data(io_work, false);
            }
        }

        if (io_work->wtype == WorkTypes::Get) // || (io_work->wtype == WorkTypes::GetMulti))
        {
            if (io_work->local)
            {
                std::stringstream string_stream;
                gcom_modbus_decode_debug(io_work, string_stream, myCfg, false, false);
                reply = string_stream.str();
                // std::cout << "it was a local get" << std::endl;
            }
            else
            {

                std::stringstream string_stream("");
                if(0)std::cout << "it was a remote get ; data_error >> " << (io_work->data_error?"true":"false")<< std::endl;

                store_raw_data(io_work, false);
                gcom_modbus_decode_debug(io_work, string_stream, myCfg, false, false);
                reply = string_stream.str();
                // std::cout << "it was a remote get" << std::endl;
            }
        }

        // we could have a replyto
        if (io_work->replyto != "")
        {
            myCfg.fims_gateway.Send(
                fims::str_view{"set", sizeof("set") - 1},
                fims::str_view{io_work->replyto.data(), io_work->replyto.size()},
                fims::str_view{nullptr, 0},
                fims::str_view{nullptr, 0},
                fims::str_view{reply.data(), reply.size()});
        }
        io_poolChan.send(std::move(io_work));
        return;
    }

    // we are woring with a pubgroup
    if (0)std::cout << "io_work :" << io_work->mynum
        << " error :" << io_work->errors
        << " error_code :" << io_work->errno_code
        << " received by resp channel "
        << " key " << key
        << " tNow " << io_work->tNow
        << std::endl;

    bool pgret = check_pubgroup_key(key, io_work);

    if ((!pgret) || (pubGroups.find(key) == pubGroups.end()))
    {
        FPS_INFO_LOG("io_work %d Pubgroup key not found [%s]. Discarding io_work", io_work->mynum, key.c_str());
        io_poolChan.send(std::move(io_work));
        return;
    }
    else
    {
        // if we get a new (later) tNow then discard and start again
        if (myCfg.connection.debug)
            FPS_DEBUG_LOG("Checking pubgroup %s size %d time %f against incoming %f",  
                            key.c_str(), (int)pubGroups[key].works.size(), pubGroups[key].tNow, io_work->tNow);
        if (io_work->tNow < pubGroups[key].tNow)
        {
            FPS_INFO_LOG("Discarding stale incoming io_work; current pubgroup id  %f is later than incoming id %f",pubGroups[key].tNow, io_work->tNow);
            auto  mypub =  io_work->pub_struct;

            if(mypub && mypub->pending > 1)
                mypub->pending--;
            // todo decrement the pollcount
            io_poolChan.send(std::move(io_work));
            return;
        }

        // did we get a match if so add to the group
        else if (io_work->tNow == pubGroups[key].tNow)
        {
            if (myCfg.connection.debug)
                FPS_DEBUG_LOG("Adding incoming io_work; current pubgroup id %f is the same as id %f", pubGroups[key].tNow, io_work->tNow);
            pubGroups[key].works.push_back(io_work);
            pubGroups[key].pub_struct = io_work->pub_struct;
        }
        else
        {
            // Callback: Discard group if it was not finished
            if (pubGroups[key].done == false)
            {
                discardGroupCallback(pubGroups[key], myCfg, false);
                auto  mypub =  io_work->pub_struct;
                if(0)std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> mypub aborted name: " << io_work->work_name 
                            << "\t pending value :" << mypub->pending 
                            << " work num :" << io_work->mynum
                            << " error_code :" << io_work->errno_code
                            << std::endl;
                if(mypub && mypub->pending > 1)
                    mypub->pending--;
            }
            else
            {
                // this should cause the pubs to restart
                // this is done to indicate that we are receiving data on that io_work point
                auto  mypub =  io_work->pub_struct;
                // no need to announce the good news.
                if(0)std::cout << " mypub completed name: " << io_work->work_name 
                                << "\t pending value :" << mypub->pending 
                                << " work num :" << io_work->mynum
                                << " error_code :" << io_work->errno_code
                                << std::endl;
                if(mypub && mypub->pending > 1)
                    mypub->pending--;
            }
            // reset the pubgroup
            pubGroups[key].reset_group(key, io_work);
            //push the new item onto the new group
            pubGroups[key].works.push_back(io_work);
        }
    }

    // chack if we have all the items we need for the group
    if ((int)pubGroups[key].works.size() == (int)pubGroups[key].work_group)
    {
        // Callback: Process group
        pubGroups[key].done = true;
        if (myCfg.connection.debug)
            FPS_DEBUG_LOG("Completing group %s\ttnow: %f\tsyncPct: %f"
                        , key.c_str(), io_work->tNow, pubGroups[key].pub_struct?pubGroups[key].pub_struct->syncPct:0.0);
        processGroupCallback(pubGroups[key], myCfg);
        if (pubGroups[key].pub_struct)
            syncTimeObjectByName(key, pubGroups[key].pub_struct->syncPct); 
        if (pubGroups[key].erase_group == true)
        {
            if (myCfg.connection.debug)
                FPS_DEBUG_LOG("Erasing group %s\ttnow: %f", key.c_str(), io_work->tNow);
            pubGroups.erase(key); // Optionally remove the group after processing
        }
    }
    else
    {
        if (myCfg.connection.debug)
            FPS_DEBUG_LOG("Still collecting  group  %s\tSize: %d out of %d\ttnow: %f",key.c_str(), (int)pubGroups[key].works.size(), (int)pubGroups[key].work_group, io_work->tNow);
    }
}

// Response Thread Function
void responseThreadFunc(ThreadControl &control, struct cfg &myCfg)
{
    std::shared_ptr<IO_Work> io_work;
    double delay = 0.1;
    while (control.responseThreadRunning)
    {
        if (io_responseChan.receive(io_work, delay))
        {
            io_work->tReceive = get_time_double();

            // Collate batches response_received_work
            processRespWork(io_work, myCfg);

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

void stashWork(std::shared_ptr<IO_Work> io_work)
{
   // TODO use dequw
    io_poolChan.send(std::move(io_work));
 
}

void buffer_work(cfg::Register_Types register_type, int device_id, int offset, bool off_by_one, int num_regs)
{
    std::shared_ptr<IO_Work> io_work;

    io_work = std::make_shared<IO_Work>();

    // Modify io_work data if necessary here
    io_work->tStart = get_time_double();
    io_work->io_points.clear();
    io_work->local = false;

    io_work->device_id = device_id;
    io_work->register_type = register_type;
    io_work->offset = offset;
    // io_work->start_offset = offset;
    io_work->num_registers = num_regs;

    io_work->test_mode = false;
    io_work->num_registers = num_regs;
    io_work->off_by_one = off_by_one;

    // TODO use dequw
    io_poolChan.send(std::move(io_work));

    return; // (io_work);
}

// TODO possibly limit these , look at gcom_fims.cpp
// std::shared_ptr<IO_Work> make_work_from_point(cfg::Register_Types register_type, int device_id,
//                                    int offset, bool off_by_one,  int num_regs, uint16_t *u16bufs, uint8_t *u8bufs, WorkTypes wtype)
// {

//     std::shared_ptr<IO_Work> io_work;
//     std::string repto("");

//     if (!io_poolChan.peekpop(io_work))
//     { // Assuming receive will return false if no item is available.
//         // std::cout << " create an io_work object "<< std::endl;
//         io_work = std::make_shared<IO_Work>();
//     }
//     io_work->use_count++;

//     io_work->io_points.clear();
//     // Modify io_work data if necessary here
//     io_work->tStart = get_time_double();
//     io_work->device_id = device_id;
//     io_work->register_type = register_type;
//     io_work->offset = offset;
//     // io_work->start_offset = offset;
//     io_work->num_registers = num_regs;

//     io_work->set_bufs(num_regs, u16bufs, u8bufs);
//     io_work->wtype = wtype;
//     io_work->test_mode = false;

//     io_work->local = false;
//     io_work->data_error = false;
//     io_work->replyto = repto;
//     io_work->work_name = repto;
//     io_work->off_by_one = off_by_one;
//     io_work->full = false;


//     return (io_work);
// }

// TODO possibly limit these , look at gcom_fims.cpp


// std::shared_ptr<IO_Fims> make_fims(struct cfg &myCfg)
// {
//     std::shared_ptr<IO_Fims> io_fims;
//     int fims_wait = 0;
//     int max_fims_wait = myCfg.max_fims_wait; // after this we give up

//     while (!io_fimsChan.peekpop(io_fims))
//     { // Assuming receive will return false if no item is available.
//         if(myCfg.num_fims < myCfg.max_fims)
//         {
//             io_fims = std::make_shared<IO_Fims>(myCfg.connection.data_buffer_size);
//             myCfg.num_fims++;
//             return (io_fims);
//         }
//         fims_wait++;
//         if (fims_wait> max_fims_wait)
//         {
//             std::cout << " failed to get a released io_fims buffer " << std::endl;
//             myCfg.keep_fims_running = false;
//             return nullptr;
//         }
//         // we've got to get one sometime or have we
//         std::this_thread::sleep_for(10ms);
//     }

//     return (io_fims);
// }

int num_work = 0;
// TODO make this a config option
// if we reach this limt we have to  wait for io_work objects to be released.
// the socket timeouts should terminate all send / get requests.
// these only build up if the network is lagging

int max_work = 20000;


std::shared_ptr<IO_Work> make_work(cfg::Register_Types register_type, int device_id,
                                   int offset, bool off_by_one,  int num_regs, uint16_t *u16bufs, uint8_t *u8bufs, WorkTypes wtype)
{

    std::shared_ptr<IO_Work> io_work;
    std::string repto("");
    int work_wait = 0;
    int max_work_wait = 5000; //myCfg.max_work_wait; // after this we give up


    if (!io_poolChan.peekpop(io_work))
    { // Assuming receive will return false if no item is available.
        // std::cout << " create an io_work object "<< std::endl;
        if(num_work < max_work)
        {  
            //myCfg.
            num_work++;
            io_work = std::make_shared<IO_Work>();
            if(0)std::cout << " create an io_work object number  "<< num_work <<  std::endl;
        }
        else
        {
            while(!io_poolChan.peekpop(io_work))
            {
                work_wait++;
                if (work_wait> max_work_wait)
                {
                    std::cout << " failed to get a released io_work buffer " << std::endl;
                    //myCfg.keep_fims_running = false;
                    return nullptr;
                }
                // we've got to get one sometime or have we
                std::this_thread::sleep_for(10ms);
            }
        }
    }
    io_work->use_count++;

    io_work->io_points.clear();
    // Modify io_work data if necessary here
    io_work->tStart = get_time_double();
    io_work->device_id = device_id;
    io_work->register_type = register_type;
    io_work->offset = offset;
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

bool testThread()
{
    io_threadChan.send(2);
    return true;
}

bool killThread()
{
    io_threadChan.send(3);
    return true;
}

// bool pollWork (std::shared_ptr<IO_Work> io_work) {
// bool setWork (std::shared_ptr<IO_Work> io_work) {
bool pollWork(std::shared_ptr<IO_Work> io_work)
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

    return true; // You might want to return a status indicating success or failure.
}

bool setWork(std::shared_ptr<IO_Work> io_work)
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
    return true; // You might want to return a status indicating success or failure.
}



// I tink this is only used in a test mode 
bool queue_work(cfg::Register_Types register_type, int device_id, int offset, bool off_by_one, int num_regs, uint16_t *u16bufs, uint8_t *u8bufs, WorkTypes wtype)
{

    std::shared_ptr<IO_Work> io_work;

    if (!io_poolChan.receive(io_work, 0))
    { // Assuming receive will return false if no item is available.
        io_work = std::make_shared<IO_Work>();
    }

    // Modify io_work data if necessary here
    io_work->io_points.clear();
    io_work->local = false;

    io_work->tStart = get_time_double();
    io_work->device_id = device_id;
    io_work->register_type = register_type;
    io_work->offset = offset;
    io_work->num_registers = num_regs;
    io_work->off_by_one = off_by_one;

    io_work->set_bufs(num_regs, u16bufs, u8bufs);
    io_work->wtype = wtype;
    io_work->test_mode = false;
    io_work->pub_struct = nullptr;


    io_pollChan.send(std::move(io_work));
    io_threadChan.send(1);

    return true; // You might want to return a status indicating success or failure.
}

bool startRespThread(struct cfg &myCfg)
{
    threadControl.responseThread = std::thread(responseThreadFunc, std::ref(threadControl), std::ref(myCfg));
    return true;
}

bool stopRespThread(struct cfg &myCfg)
{
    threadControl.responseThreadRunning = false;
    threadControl.responseThread.join();
    return true;
}

std::shared_ptr<IO_Thread> make_IO_Thread(int idx, const char *ip, int port, double connection_timeout, double transfer_timeout, struct cfg &myCfg)
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

// thread callback manages thread connections  
// test code not used yet
void threadCallback(std::shared_ptr<TimeObject>t, void *p)
{
    //bool debug = false;
    //std::lock_guard<std::mutex> lock2(cb_output_mutex);
    //std::shared_ptr<IO_Thread> io_thread
    std::shared_ptr<IO_Thread> mythread = std::shared_ptr<IO_Thread>(static_cast<IO_Thread *>(p), [](IO_Thread *) {});
    double tNow = get_time_double();

    if (t->repeatTime == 5.0)
       t->repeatTime = 0.1;
    else
       t->repeatTime = 5.0;

    std::cout   << " Thread timer running for id " <<  mythread->tid 
                << " name :" << t->name 
                << " at " << tNow 
                << " state: "  << std::string(mythread->connected?"connected":"disconnected")
                << " repeat: "  << t->repeatTime
                << std::endl; 

    //  if the thread is disconnected and enabled then we'll try to connect


}


bool StartThreads(int num_threads, const char *ip, int port, double connection_timeout, double transfer_timeout, struct cfg &myCfg)
{

    // threadControl.io_pollChan     = &io_pollChan;
    // threadControl.io_responseChan = &io_responseChan;
    // threadControl.io_poolChan     = &io_poolChan;

    {
        double tNow = get_time_double();
        FPS_INFO_LOG("IO Threads starting at %f ", tNow);
        // std::cout << " test_io_threads running  at "<< tNow << std::endl;
    }

    // Start the response thread
    startRespThread(myCfg);

    // start the local thread no connection needed.
    std::shared_ptr<IO_Thread> io_thread = make_IO_Thread(0, ip, port, connection_timeout, transfer_timeout, myCfg);
    io_thread->is_local = true;
    io_thread->thread = std::thread(ioThreadFunc, std::ref(threadControl), std::ref(myCfg), io_thread);
    threadControl.ioThreadPool.push_back(io_thread); // std::thread (ioThreadFunc, std::ref(systemControl), io_thread));
    io_thread->tid = 0;

    // int num_threads = 4;
    for (int i = 0; i < num_threads; ++i)
    {
        // IO_Thread io_thread;
        std::shared_ptr<IO_Thread> io_thread = make_IO_Thread(i+1, ip, port, connection_timeout, transfer_timeout, myCfg);

        // create thread timer
        double tNow = get_time_double();

        // std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
        // // Actually start the threads here
        io_thread->tid = i+1;

        {
            //double tNow = get_time_double();
            FPS_INFO_LOG("thread_id %d starting at %f ", i, tNow);
        }

        io_thread->thread = std::thread(ioThreadFunc, std::ref(threadControl), std::ref(myCfg), io_thread);

        threadControl.ioThreadPool.push_back(io_thread); // std::thread (ioThreadFunc, std::ref(systemControl), io_thread));

        // std::string thstr = "timer_thread_" + std::to_string(i+1);
        // std::shared_ptr<TimeObject> obj1 = createTimeObject(thstr,                 // name
        //                                                     tNow,                   // start time (initial startup time)
        //                                                     0,                           // stop time - 0 = don't stop
        //                                                     5.0, // how often to repeat
        //                                                     0,                           // count - 0 = don't stop
        //                                                     threadCallback,                 // callback
        //                                                     (void *)io_thread.get());         // callback params
        // addTimeObject(obj1, 0.0, true);

    }
    // std::thread responseThread(responseThreadFunc, std::ref(*this));
    {
        double tNow = get_time_double();
        FPS_INFO_LOG("All io_threads running at %f ", tNow);
        FPS_LOG_IT("startup");
    }
    return true;
}

bool StartThreads(struct cfg &myCfg, bool debug)
{
    return StartThreads(myCfg.connection.max_num_connections, myCfg.connection.ip_address.c_str(),
                   myCfg.connection.port, myCfg.connection.connection_timeout, myCfg.connection.transfer_timeout, myCfg);
}

bool StopThreads(struct cfg &myCfg, bool debug)
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

int test_io_threads(struct cfg &myCfg)
{

    int num_threads = 4;
    StartThreads(num_threads, nullptr, 0, 2.0, 0.5, myCfg);


    // Simulate the sending of work
    for (int i = 0; i < 10; ++i)
    {
        queue_work(cfg::Register_Types::Holding, i, 0, false, 5, nullptr, nullptr, WorkTypes::Noop);
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Sleep for a bit before sending the next
    }
    {
        std::lock_guard<std::mutex> lock2(io_output_mutex);
        double tNow = get_time_double();
        std::cout << " jobs queued  at " << tNow << std::endl;
    }

    // Signal the io_thread to stop after some time
    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::lock_guard<std::mutex> lock2(io_output_mutex);
        double tNow = get_time_double();
        std::cout << " done sleeping " << tNow << std::endl;
    }

    // io_threadChan.send(0);

    threadControl.stopThreads();
    {
        FPS_INFO_LOG("All threads stopping ");
    }

    threadControl.responseThread.join();
    // Signal the io_thread to stop after some time
    std::this_thread::sleep_for(std::chrono::seconds(2));
    {
        std::lock_guard<std::mutex> lock2(io_output_mutex);
        double tNow = get_time_double();
        std::cout << " final sleep " << tNow << std::endl;
    }

    {
        FPS_INFO_LOG("IO Threads stopping.");
        FPS_LOG_IT("shutdown");
    }
    return 0;
}

#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>

int wait_socket_ready(modbus_t *ctx, int timeout_sec)
{
    int socket_fd = modbus_get_socket(ctx);
    if (socket_fd == -1)
    {
        return -1; // Invalid socket
    }

    fd_set write_set;
    struct timeval timeout;

    FD_ZERO(&write_set);
    FD_SET(socket_fd, &write_set);
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    int result = select(socket_fd + 1, NULL, &write_set, NULL, &timeout);
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

    return -1; // Connection failed or timed out
}

int check_socket_alive(std::shared_ptr<IO_Thread> io_thread, int timeout_sec, int timeout_usec)
{

    int socket_fd = modbus_get_socket(io_thread->ctx);
    if (socket_fd == -1)
    {
        return -1; // Invalid socket
    }

    fd_set except_set, read_set;
    struct timeval timeout;

    FD_ZERO(&except_set);
    FD_ZERO(&read_set);
    FD_SET(socket_fd, &except_set);
    FD_SET(socket_fd, &read_set);
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = timeout_usec;

    int result = select(socket_fd + 1, &read_set, NULL, &except_set, &timeout);
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
        auto foo_read = FD_ISSET(socket_fd,&read_set);
        //auto foo_except = FD_ISSET(socket_fd,&except_set);
        //printf( " socket test result %d read %d except %d\n", result, foo_read, foo_except);
        // if we get a read , in this case it is an error
        return foo_read;
    }

    return 0; // Connection failed or timed out
}

// new function to run a fast reconnect after a timeout
// this stops the system building up a whole bunch of jobs during a full reconnect 
double FastReconnectForThread(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug)
{
    double tNow = get_time_double();

    // make sure the old context is deleted
    modbus_t *ctx = nullptr; // modbus_new_tcp(io_thread->ip.c_str(), io_thread->port);
 
    if (( io_thread->tid != 1) && ( !io_thread->is_enabled))
    {
        return 0.0;
    }
    
    if (myCfg.connection.is_RTU)
    {
        ctx = modbus_new_rtu(
            myCfg.connection.serial_device.c_str(),
            myCfg.connection.baud_rate,
            myCfg.connection.parity,
            myCfg.connection.data_bits,
            myCfg.connection.stop_bits);
    }
    else
    {
        ctx = modbus_new_tcp(io_thread->ip.c_str(), io_thread->port);
    }


    if (!ctx)
    {
        DelayConnect(myCfg, threadControl);
        DisConnect(myCfg, threadControl, io_thread);
        char message[1024];
        snprintf(message, 1024, "Modbus Client [%s]  Thread id %d failed to create modbus context to [%s] on port [%d]",
                myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
        {
            //std::lock_guard<std::mutex> lock(logger_mutex);

            FPS_ERROR_LOG("%s", message);
        }
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
        //io_thread->connected = false;
        return 0.0;
    }

    // this has to be fast
    uint32_t to_sec = (uint32_t)io_thread->transfer_timeout; // 2-10 seconds timeout
    uint32_t to_usec = (uint32_t)((io_thread->transfer_timeout-to_sec)*1000000.0);         
     
    auto mberrto = modbus_set_response_timeout(ctx, to_sec, to_usec);
    std::cout << " Transfer Timeout  " << io_thread->transfer_timeout<< "  to_sec  " << to_sec << " to_usec:" << mberrto << std::endl;

    auto mberr = modbus_connect(ctx);

    if (mberr != 0)
    {
        auto err = errno;
        //std::cout << std::dec << " Connect modbus " << io_thread->ip << "  port " << io_thread->port << " Error mberr:" << mberr << std::endl;
        modbus_free(ctx);
        DelayConnect(myCfg, threadControl);
        io_thread->ctx = nullptr;
        DisConnect(myCfg, threadControl, io_thread);
        if(io_thread->connect_fails < 2)
        {
            io_thread->connect_fails++;
            char message[1024];
            snprintf(message, 1024, "Modbus Client [%s] Thread id %d failed to connect to [%s] on port [%d]. Modbus error [%s]"
                            , myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port, modbus_strerror(err));
            FPS_ERROR_LOG("%s", message);
            emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
        }
        return 0.0;
    }

    modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)MODBUS_ERROR_RECOVERY_PROTOCOL);

    io_thread->ctx = ctx;
    SetConnect(myCfg, threadControl, io_thread);

    //io_thread->connected = true;
    io_thread->connect_time = get_time_double() - tNow;
    char message[1024];
    snprintf(message, 1024, "Modbus Client [%s] Thread  id %d successfully connected to [%s] on port [%d]"
        , myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);

    FPS_INFO_LOG("%s", message);
    emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
    io_thread->connect_fails = 0;
    io_thread->connect_reset = 0; //get_time_double() - tNow;
    return (io_thread->connect_time);
}

double SetupModbusForThread(struct cfg &myCfg, std::shared_ptr<IO_Thread> io_thread, bool debug)
{
    double tNow = get_time_double();
    if (( io_thread->tid != 1) && ( !io_thread->is_enabled))
    {
        return 0.0;
    }
    if (OkToConnect(threadControl, tNow))
    {
        uint32_t to_sec = (uint32_t)io_thread->connection_timeout; // 2-10 seconds timeout
        uint32_t to_usec = (uint32_t)((io_thread->connection_timeout-to_sec)*1000000.0);                            // 0 microsecond
        io_thread->ctx = nullptr;
        modbus_t *ctx = nullptr; // modbus_new_tcp(io_thread->ip.c_str(), io_thread->port);
        if (myCfg.connection.is_RTU)
        {
            ctx = modbus_new_rtu(
                myCfg.connection.serial_device.c_str(),
                myCfg.connection.baud_rate,
                myCfg.connection.parity,
                myCfg.connection.data_bits,
                myCfg.connection.stop_bits);
        }
        else
        {
            ctx = modbus_new_tcp(io_thread->ip.c_str(), io_thread->port);
        }


        if (!ctx)
        {
            DelayConnect(myCfg, threadControl);
            DisConnect(myCfg, threadControl, io_thread);
            char message[1024];
            snprintf(message, 1024, "Modbus Client [%s]  Thread id %d failed to create modbus context to [%s] on port [%d]",
                 myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
            {
                //std::lock_guard<std::mutex> lock(logger_mutex);

                FPS_ERROR_LOG("%s", message);
            }
            emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
            //io_thread->connected = false;
            return 0.0;
        }

        auto mberr = modbus_connect(ctx);
        if(0)std::cout << std::dec 
                    << " Connect modbus " << io_thread->ip 
                    << "  port " << io_thread->port 
                    << " Error mberr:" << mberr << std::endl;

        if (mberr != 0)
        {
            auto err = errno;
            //std::cout << std::dec << " Connect modbus " << io_thread->ip << "  port " << io_thread->port << " Error mberr:" << mberr << std::endl;
            modbus_free(ctx);
            DelayConnect(myCfg, threadControl);
            io_thread->ctx = nullptr;
            DisConnect(myCfg, threadControl, io_thread);
            if(io_thread->connect_fails < 2)
            {
                io_thread->connect_fails++;
                char message[1024];
                snprintf(message, 1024, "Modbus Client [%s] Thread id %d failed to connect to [%s] on port [%d]. Modbus error [%s]"
                                , myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port, modbus_strerror(err));
                FPS_ERROR_LOG("%s", message);
                emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
            }
            return 0.0;
        }

        wait_socket_ready(ctx, 1);

        if (modbus_set_response_timeout(ctx, to_sec, to_usec) == -1)
        {
            //std::cout << "{ \"status\":\"error\" , \"message\":\" Unable to set timeout  to " << io_thread->ip << "  port " << io_thread->port << "\"}" << std::endl;
            modbus_free(ctx);
            DelayConnect(myCfg, threadControl);
            io_thread->ctx = nullptr;
            DisConnect(myCfg, threadControl, io_thread);
            char message[1024];
            snprintf(message, 1024, "Modbus Client [%s] Thread id %d failed to create set response timeout to [%s] on port [%d]"
                            , myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
 
            FPS_ERROR_LOG("%s", message);
            emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
            return 0.0;
        }

        modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)MODBUS_ERROR_RECOVERY_PROTOCOL);

        io_thread->ctx = ctx;
        SetConnect(myCfg, threadControl, io_thread);

        //io_thread->connected = true;
        io_thread->connect_time = get_time_double() - tNow;
        char message[1024];
        snprintf(message, 1024, "Modbus Client [%s] Thread  id %d successfully connected to [%s] on port [%d]"
            , myCfg.connection.name.c_str(), io_thread->tid, io_thread->ip.c_str(), io_thread->port);
        FPS_INFO_LOG("%s", message);
        emit_event(&myCfg.fims_gateway, "Modbus Client", message, 1);
        io_thread->connect_fails = 0;
        
        to_sec = 0;//io_thread->run_timeout; // 2-10 seconds timeout
        to_usec = 100000;                            // 0 microsecond
        to_sec = (uint32_t)io_thread->transfer_timeout; // 2-10 seconds timeout
        to_usec = (uint32_t)((io_thread->transfer_timeout-to_sec)*1000000.0);         
        modbus_set_response_timeout(ctx, to_sec, to_usec);
        // TODO check for errors

        return (io_thread->connect_time);
    }
    return 0.0;
}



bool CloseModbusForThread(std::shared_ptr<IO_Thread> io_thread, bool debug)
{
    auto ctx = io_thread->ctx;
    io_thread->ctx = nullptr;
    if(ctx)
    {
        modbus_close(ctx);
        modbus_free(ctx);
        char message[1024];
        snprintf(message, 1024, "Modbus Client [%s] disconnected from [%s] on port [%d]", io_thread->myCfg->connection.name.c_str(), io_thread->ip.c_str(), io_thread->port);
        FPS_INFO_LOG("%s", message);
        emit_event(&io_thread->myCfg->fims_gateway, "Modbus Client", message, 1);
    
    }
    return true;
}


// this is the old code
void test_io_point_single(const char *ip, int port, double connection_timeout, const char *oper, int device_id, const char *regtype, int offset, int num_regs,
                          int value, int num_threads, struct cfg &myCfg, bool debug)
{
    // auto thread_debug = debug;
    // int num_threads = 4;
    int num_points = 1;

    // TODO fix these
    std::string register_type_str(regtype);
    std::string roper(oper);
    // int num_regs = 1;
    auto register_type = strToRegType(register_type_str);
    auto cfgreg_type = myCfg.typeFromStr(register_type_str);
    std::cout << " cfg Register Type" << myCfg.typeToStr(cfgreg_type) << std::endl;
    auto work_type = strToWorkType(oper);

    uint16_t u16bufs[130]; // MAX_MODBUS_NUM_REGS
    uint8_t u8bufs[130];
    for (int i = 0; i < num_regs; ++i)
    {
        u16bufs[i] = value;
        u8bufs[i] = value;
    }

    StartThreads(num_threads, ip, port, connection_timeout, 0.5, myCfg);

    std::this_thread::sleep_for(100ms);

    for (int j = 0; j < 10; ++j)
    {
        for (int i = 0; i < num_points; ++i)
        {
            double tNow = get_time_double();
            u16bufs[0] = i;
            // std::shared_ptr<IO_Work> make_work(cfg::Register_Types register_type, int device_id, int offset, int num_regs, uint16_t* u16bufs, uint8_t* u8bufs, WorkTypes wtype ) {

            auto io_work = make_work(register_type, device_id, offset, false,num_regs, u16bufs, u8bufs, work_type);
            io_work->test_mode = true;
            io_work->work_group = num_points;
            io_work->work_name = std::string("test_io_point_single");
            io_work->tNow = tNow;

            if (j == 4)
                io_work->test_it = true;
            else
                io_work->test_it = false;

            pollWork(io_work);
            std::cout << " Test sleeping for 1 second" << std::endl;
            std::this_thread::sleep_for(1000ms);
            // std::cout << " Test io_work errno_code  "<< (int)io_work->errno_code<< std::endl;
            if (io_work->errno_code == BAD_DATA_ADDRESS)
            {
                std::cout << " Test io_work errno_code  " << (int)io_work->errno_code << " Skipping Test" << std::endl;
                j = 10;
            }
        }
    }
    std::cout << " Now sleeping for 2 seconds" << std::endl;
    std::this_thread::sleep_for(2000ms);

    // close context
    // TODO may need locks here
    // CloseModbusCtx(num_threads, ip , port);

    io_localthreadChan.send(0);

    for (int i = 0; i < num_threads; ++i)
    {
        io_threadChan.send(0);
    }

    threadControl.stopThreads();
    {
        FPS_INFO_LOG("all threads stopping ");
    }

    threadControl.responseThread.join();
    // Signal the io_thread to stop after some time
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // {
    //     std::lock_guard<std::mutex> lock2(io_output_mutex);
    //     double tNow = get_time_double();
    //     std::cout << " final sleep "<< tNow << std::endl;
    // }

    {
        FPS_INFO_LOG("IO Threads stopping.");
        FPS_LOG_IT("shutdown");
    }

}

void test_io_point_multi(const char *ip, int port, int connection_timeout, const char *oper, int device_id, const char *regtype, int offset, int value, int num_threads, struct cfg &myCfg, bool debug)
{
    // auto thread_debug = debug;
    // int num_threads = 4;
    int num_points = 10;

    // TODO fix these
    std::string register_type_str(regtype);
    std::string roper(oper);
    int num_regs = 1;
    auto register_type = strToRegType(register_type_str);

    auto work_type = strToWorkType(oper);

    uint16_t u16bufs[130]; // MAX_MODBUS_NUM_REGS
    uint8_t u8bufs[130];
    for (int i = 0; i < num_regs; ++i)
    {
        u16bufs[i] = value;
        u8bufs[i] = value;
    }


    StartThreads(num_threads, ip, port, connection_timeout, 0.5, myCfg);
    // SetUPModbusCtx(num_threads, ip , port, connection_timeout);

    std::this_thread::sleep_for(100ms);
    for (int i = 0; i < num_points; ++i)
    {
        u16bufs[0] = i;
        queue_work(register_type, device_id, offset, false, num_regs, u16bufs, u8bufs, work_type);
    }

    std::this_thread::sleep_for(10000ms);


    io_localthreadChan.send(0);
    // this wont work
    for (int i = 0; i < num_threads; ++i)
    {
        io_threadChan.send(0);
    }

    threadControl.stopThreads();
    {
        FPS_INFO_LOG("all threads stopping ");
    }

    threadControl.responseThread.join();

    FPS_INFO_LOG("IO Threads stopping.");
    FPS_LOG_IT("shutdown");

}