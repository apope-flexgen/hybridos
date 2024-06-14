#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <cstring>
#include <mutex>

#include "gcom_iothread.h"

TEST(HasLostConnectionTest, HandlesTimeoutForRTUConnection)
{
    cfg myCfg;
    myCfg.connection.is_RTU = true;
    myCfg.connection.device_name = "TestDevice";

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->connected = true;
    io_thread->connection_timedout = false;
    io_thread->ctx = nullptr;
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->errno_code = ETIMEDOUT;
    io_work->data_error = false;

    bool result = has_lost_connection(myCfg, io_thread, io_work, "TESTunc", ETIMEDOUT);

    EXPECT_TRUE(result);
    EXPECT_TRUE(io_work->data_error);
    EXPECT_TRUE(io_thread->connection_timedout);
}

TEST(HasLostConnectionTest, HandlesNoError)
{
    cfg myCfg;
    myCfg.connection.is_RTU = false;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->connected = true;
    io_thread->connection_timedout = false;
    io_thread->ctx = reinterpret_cast<modbus_t*>(1);  // Mock context
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->errno_code = 0;  // No error
    io_work->data_error = false;

    bool result = has_lost_connection(myCfg, io_thread, io_work, "TESTunc", 0);

    EXPECT_FALSE(result);
    EXPECT_FALSE(io_work->data_error);
    EXPECT_TRUE(io_thread->connected);
    EXPECT_FALSE(io_thread->connection_timedout);
}

// TODO: Figure out mocking so I can actually have these test cases:
// TEST(HasLostConnectionTest, HandlesEPIPEError)
// {
//     cfg myCfg;
//     myCfg.connection.is_RTU = false;

//     auto io_thread = std::make_shared<IO_Thread>();
//     io_thread->connected = true;
//     io_thread->connection_timedout = false;
//     io_thread->ctx = reinterpret_cast<modbus_t*>(1);  // Mock context
//     io_thread->tid = 1;

//     auto io_work = std::make_shared<IO_Work>();
//     io_work->errno_code = EPIPE;
//     io_work->data_error = false;

//     bool result = has_lost_connection(myCfg, io_thread, io_work, "TESTunc", EPIPE);

//     EXPECT_TRUE(result);
//     EXPECT_TRUE(io_work->data_error);
//     EXPECT_FALSE(io_thread->connected);
// }

// TEST(HasLostConnectionTest, HandlesEINVAL)
// {
//     cfg myCfg;
//     myCfg.connection.is_RTU = false;

//     auto io_thread = std::make_shared<IO_Thread>();
//     io_thread->connected = true;
//     io_thread->connection_timedout = false;
//     io_thread->ctx = reinterpret_cast<modbus_t*>(1);  // Mock context
//     io_thread->tid = 1;

//     auto io_work = std::make_shared<IO_Work>();
//     io_work->errno_code = EINVAL;
//     io_work->data_error = false;

//     bool result = has_lost_connection(myCfg, io_thread, io_work, "TESTunc", EINVAL);

//     EXPECT_TRUE(result);
//     EXPECT_TRUE(io_work->data_error);
//     EXPECT_FALSE(io_thread->connected);
// }