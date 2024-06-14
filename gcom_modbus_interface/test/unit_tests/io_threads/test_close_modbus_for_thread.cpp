#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "mock_modbus.h"
#include "gcom_iothread.h"

TEST(CloseModbusForThreadTest, SuccessfulClose_TCP)
{
    cfg myCfg;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    io_thread->ctx = new modbus_t();
    io_thread->myCfg = &myCfg;

    // Set the connection configuration
    io_thread->myCfg->connection.is_RTU = false;
    io_thread->ip = "192.168.0.1";
    io_thread->port = 502;

    // Call the function to test
    bool result = CloseModbusForThread(io_thread, false);

    // Verify results
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_TRUE(result);
}

TEST(CloseModbusForThreadTest, NoContext)
{
    // No context to close
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    io_thread->ctx = nullptr;

    // Call the function to test
    bool result = CloseModbusForThread(io_thread, false);

    // Verify results
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_TRUE(result);
}

TEST(CloseModbusForThreadTest, SuccessfulClose_RTU)
{
    cfg myCfg;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    io_thread->ctx = new modbus_t();
    io_thread->myCfg = &myCfg;

    // Set the connection configuration
    io_thread->myCfg->connection.is_RTU = true;
    io_thread->myCfg->connection.device_name = "/dev/ttyS0";

    // Call the function to test
    bool result = CloseModbusForThread(io_thread, false);

    // Verify results
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_TRUE(result);
}
