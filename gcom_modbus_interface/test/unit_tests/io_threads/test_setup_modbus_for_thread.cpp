#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "gcom_iothread.h"

extern modbus_t* modbus_new_rtu_ret;
extern modbus_t* modbus_new_tcp_ret;
extern int modbus_connect_ret;
extern int modbus_set_response_timeout_ret;

TEST(SetupModbusForThreadTest, SuccessfulRTUConnection)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;
    myCfg.connection.is_RTU = true;
    myCfg.connection.device_name = "/dev/ttyS0";
    myCfg.connection.baud_rate = 9600;
    myCfg.connection.parity = 'N';
    myCfg.connection.data_bits = 8;
    myCfg.connection.stop_bits = 1;
    io_thread->connect_fails = 1;
    io_thread->connect_reset = 1;
    io_thread->connect_time = 10000;

    modbus_new_rtu_ret = new modbus_t();
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;

    double connect_time = SetupModbusForThread(myCfg, io_thread, debug);

    EXPECT_NEAR(connect_time, 0.0, 0.01);
    EXPECT_EQ(io_thread->ctx, modbus_new_rtu_ret);
    EXPECT_EQ(io_thread->connect_fails, 0);
    EXPECT_EQ(io_thread->connect_reset, 1);

    delete modbus_new_rtu_ret;
}

TEST(SetupModbusForThreadTest, SuccessfulTCPConnection)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    io_thread->ip = "127.0.0.1";
    io_thread->port = 502;

    io_thread->connect_fails = 1;
    io_thread->connect_reset = 1;
    io_thread->connect_time = 10000;

    modbus_new_tcp_ret = new modbus_t();
    modbus_connect_ret = 0;
    modbus_set_response_timeout_ret = 0;

    double connect_time = SetupModbusForThread(myCfg, io_thread, debug);

    EXPECT_NEAR(connect_time, 0.0, 0.01);
    EXPECT_EQ(io_thread->ctx, modbus_new_tcp_ret);
    EXPECT_EQ(io_thread->connect_fails, 0);
    EXPECT_EQ(io_thread->connect_reset, 1);

    delete modbus_new_tcp_ret;
}

TEST(SetupModbusForThreadTest, FailedRTUConnectionPath1)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;

    myCfg.connection.is_RTU = true;
    myCfg.connection.device_name = "/dev/ttyS0";
    myCfg.connection.baud_rate = 9600;
    myCfg.connection.parity = 'N';
    myCfg.connection.data_bits = 8;
    myCfg.connection.stop_bits = 1;
    io_thread->connect_fails = 1;
    io_thread->connect_reset = 1;
    io_thread->connect_time = 10000;

    modbus_new_rtu_ret = nullptr;
    modbus_connect_ret = 1;

    double connect_time = SetupModbusForThread(myCfg, io_thread, debug);

    EXPECT_EQ(connect_time, 0.0);
    EXPECT_EQ(io_thread->connect_time, 10000);
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_EQ(io_thread->connect_fails, 1);
    EXPECT_EQ(io_thread->connect_reset, 1);
}

TEST(SetupModbusForThreadTest, FailedRTUConnectionPath2)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;

    myCfg.connection.is_RTU = true;
    myCfg.connection.device_name = "/dev/ttyS0";
    myCfg.connection.baud_rate = 9600;
    myCfg.connection.parity = 'N';
    myCfg.connection.data_bits = 8;
    myCfg.connection.stop_bits = 1;
    io_thread->connect_fails = 1;
    io_thread->connect_reset = 1;
    io_thread->connect_time = 10000;

    modbus_new_rtu_ret = new modbus_t();
    modbus_connect_ret = 1;

    double connect_time = SetupModbusForThread(myCfg, io_thread, debug);

    EXPECT_EQ(connect_time, 0.0);
    EXPECT_EQ(io_thread->connect_time, 10000);
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_EQ(io_thread->connect_fails, 1);
    EXPECT_EQ(io_thread->connect_reset, 1);
}

TEST(SetupModbusForThreadTest, FailedTCPConnectionPath1)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    io_thread->ip = "127.0.0.1";
    io_thread->port = 502;
    io_thread->connect_fails = 1;
    io_thread->connect_reset = 1;
    io_thread->connect_time = 10000;

    modbus_new_rtu_ret = nullptr;
    modbus_connect_ret = 1;

    double connect_time = SetupModbusForThread(myCfg, io_thread, debug);

    EXPECT_EQ(connect_time, 0.0);
    EXPECT_EQ(io_thread->connect_time, 10000);
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_EQ(io_thread->connect_fails, 1);
    EXPECT_EQ(io_thread->connect_reset, 1);

    // delete modbus_new_rtu_ret; // deleted automatically
}

TEST(SetupModbusForThreadTest, FailedTCPConnectionPath2)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    io_thread->ip = "127.0.0.1";
    io_thread->port = 502;
    io_thread->connect_fails = 1;
    io_thread->connect_reset = 1;
    io_thread->connect_time = 10000;

    modbus_new_rtu_ret = new modbus_t();
    modbus_connect_ret = 1;

    double connect_time = SetupModbusForThread(myCfg, io_thread, debug);

    EXPECT_EQ(connect_time, 0.0);
    EXPECT_EQ(io_thread->connect_time, 10000);
    EXPECT_EQ(io_thread->ctx, nullptr);
    EXPECT_EQ(io_thread->connect_fails, 1);
    EXPECT_EQ(io_thread->connect_reset, 1);

    // delete modbus_new_rtu_ret; // deleted automatically
}

TEST(SetupModbusForThreadTest, ConnectionTimeoutError)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    io_thread->ip = "127.0.0.1";
    io_thread->port = 502;

    double result = SetupModbusForThread(myCfg, io_thread, debug);
    EXPECT_EQ(result, 0.0);
}

TEST(SetupModbusForThreadTest, SetResponseTimeoutError)
{
    bool debug = false;
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    io_thread->ip = "127.0.0.1";
    io_thread->port = 502;

    double result = SetupModbusForThread(myCfg, io_thread, debug);
    EXPECT_EQ(result, 0.0);
}
