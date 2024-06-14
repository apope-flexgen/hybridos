#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "gcom_iothread.h"

extern ioChannel<std::shared_ptr<IO_Work>> io_localsetChan;
extern ioChannel<std::shared_ptr<IO_Work>> io_localpollChan;
extern ioChannel<int> io_threadChan;
extern modbus_t* modbus_new_tcp_ret;
extern int modbus_connect_ret;
extern int modbus_set_response_timeout_ret;
extern int modbus_set_error_recovery_ret;
extern ThreadControl threadControl;

TEST(IOThreadFuncTest, HandlesSignalToShutdown)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->is_enabled = true;
    io_thread->is_local = false;

    ThreadControl control;
    control.ioThreadRunning = true;
    io_threadChan.send(0);

    ioThreadFunc(control, myCfg, io_thread);

    EXPECT_EQ(io_thread->jobs, 0);
    EXPECT_EQ(io_thread->fails, 0);
}

TEST(IOThreadFuncTest, HandlesModbusSetupWithSuccess)
{
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    myCfg.connection.ip_address = "172.10.0.1";
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->is_enabled = true;
    io_thread->is_local = false;
    io_thread->connected = false;

    ThreadControl control;
    control.ioThreadRunning = true;
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;

    std::thread io_thread_func = std::thread(ioThreadFunc, std::ref(control), std::ref(myCfg), io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    control.ioThreadRunning = false;
    io_thread_func.join();

    EXPECT_TRUE(io_thread->connected);
    EXPECT_TRUE(modbus_new_tcp_ret != nullptr);
}

TEST(IOThreadFuncTest, HandlesModbusSetupNotEnoughInfo)
{
    cfg myCfg;
    myCfg.connection.serial_device = "";
    myCfg.connection.ip_address = "";
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->is_enabled = true;
    io_thread->is_local = false;
    io_thread->connected = false;

    ThreadControl control;
    control.ioThreadRunning = true;
    io_threadChan.send(0);

    std::thread io_thread_func = std::thread(ioThreadFunc, std::ref(control), std::ref(myCfg), io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    control.ioThreadRunning = false;
    io_thread_func.join();

    EXPECT_FALSE(io_thread->connected);
}

TEST(IOThreadFuncTest, ProcessesIOWork)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_thread->tid = 1;
    io_thread->is_enabled = true;
    io_thread->is_local = false;
    io_thread->connected = true;
    io_work->component = new cfg::component_struct();

    ThreadControl control;
    control.ioThreadRunning = true;
    int signal = 0;
    while (io_threadChan.peekpop(signal))
    {
    }
    pollWork(io_work);

    std::thread io_thread_func = std::thread(ioThreadFunc, std::ref(control), std::ref(myCfg), io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    control.ioThreadRunning = false;
    io_thread_func.join();

    EXPECT_EQ(io_thread->jobs, 1);
    delete io_work->component;
}

TEST(IOThreadFuncTest, HandlesLocalThread)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work_set = std::make_shared<IO_Work>();
    auto io_work_poll = std::make_shared<IO_Work>();
    io_thread->tid = 1;
    io_thread->is_enabled = true;
    io_thread->is_local = true;

    ThreadControl control;
    control.ioThreadRunning = true;
    io_work_set->local = true;
    io_work_poll->local = true;
    setWork(io_work_set);
    pollWork(io_work_poll);

    std::thread io_thread_func = std::thread(ioThreadFunc, std::ref(control), std::ref(myCfg), io_thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    control.ioThreadRunning = false;
    io_thread_func.join();

    EXPECT_EQ(io_thread->jobs, 2);
}