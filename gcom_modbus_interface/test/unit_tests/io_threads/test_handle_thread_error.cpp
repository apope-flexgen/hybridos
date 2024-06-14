#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "gcom_iothread.h"

extern modbus_t* modbus_new_rtu_ret;
extern modbus_t* modbus_new_tcp_ret;
extern int modbus_connect_ret;
extern int modbus_set_response_timeout_ret;
extern int modbus_set_error_recovery_ret;
extern ThreadControl threadControl;

TEST(HandleThreadErrorTest, NotConnected)
{
    struct cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->cTime = 1000;
    io_thread->ctx = nullptr;
    io_thread->connected = false;
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;
    io_work->data_error = false;
    io_work->errno_code = -1;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_FALSE(io_thread->connected);
    EXPECT_FALSE(io_thread->wasConnected);
    EXPECT_FALSE(io_thread->hadContext);
    EXPECT_FALSE(io_work->data_error);
    EXPECT_EQ(io_thread->cTime, 1000);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
}

TEST(HandleThreadErrorTest, WasConnectedWithContext)
{
    struct cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->connected = true;
    io_thread->ctx = reinterpret_cast<modbus_t*>(1);  // Mock context
    io_thread->ip = "127.0.0.1";
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;
    io_thread->cTime = 1000;
    io_work->data_error = false;
    io_work->errno_code = -1;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_TRUE(io_thread->connected);
    EXPECT_TRUE(io_thread->wasConnected);
    EXPECT_TRUE(io_thread->hadContext);
    EXPECT_FALSE(io_work->data_error);
    EXPECT_EQ(io_thread->cTime, 1000);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
}

TEST(HandleThreadErrorTest, WasConnectedNoContextNoIP)
{
    struct cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->connected = true;
    io_thread->ctx = nullptr;
    io_thread->ip = "";
    io_thread->is_local = false;
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;
    io_thread->cTime = 1000;
    io_work->data_error = false;
    io_work->errno_code = -1;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_TRUE(io_thread->connected);
    EXPECT_TRUE(io_thread->wasConnected);
    EXPECT_FALSE(io_thread->hadContext);
    EXPECT_TRUE(io_work->data_error);
    EXPECT_EQ(io_thread->cTime, 1000);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
}

TEST(HandleThreadErrorTest, WasConnectedNoContextWithIPAndENeg1)
{
    struct cfg myCfg;
    myCfg.connection.is_RTU = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->myCfg = &myCfg;
    io_thread->connected = true;
    io_thread->ctx = nullptr;
    io_thread->ip = "127.0.0.1";
    io_work->errno_code = -1;
    modbus_connect_ret = -1;
    modbus_set_response_timeout_ret = -1;
    modbus_set_error_recovery_ret = -1;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_FALSE(io_thread->connected);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
}

TEST(HandleThreadErrorTest, WasConnectedNoContextWithIPAndETIMEDOUT)
{
    struct cfg myCfg;
    myCfg.connection.is_RTU = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->myCfg = &myCfg;
    io_thread->connected = true;
    io_thread->ctx = nullptr;
    io_thread->ip = "127.0.0.1";
    io_work->errno_code = ETIMEDOUT;
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_FALSE(io_thread->connected);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
}

TEST(HandleThreadErrorTest, ReconnectFailed)
{
    struct cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->connected = true;
    io_thread->ctx = nullptr;
    io_thread->ip = "127.0.0.1";
    io_work->errno_code = -1;
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;
    io_work->local = false;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_FALSE(io_thread->connected);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
    EXPECT_TRUE(io_work->data_error);
}

TEST(HandleThreadErrorTest, ReconnectSucceeded)
{
    struct cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    bool io_done = false;
    int io_tries = 1;
    int max_io_tries = 3;
    bool debug = true;

    io_thread->connected = true;
    io_thread->ctx = nullptr;
    io_thread->ip = "127.0.0.1";
    io_work->errno_code = -1;
    io_thread->is_enabled = true;
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;
    modbus_set_error_recovery_ret = 0;
    threadControl.tConnect = 0;

    handleThreadError(myCfg, io_thread, io_work, io_done, io_tries, max_io_tries, debug);

    EXPECT_TRUE(io_thread->connected);
    EXPECT_FALSE(io_done);
    EXPECT_EQ(io_tries, 1);
}