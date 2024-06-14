#include <gtest/gtest.h>
#include <gmock/gmock.h>  // For mock objects
#include "gcom_iothread.h"

// Test decrementing num_connected_threads
TEST(handleDisconnectTest, DecrementNumConnectedThreads)
{
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    io_thread->connected = true;
    io_thread->myCfg = new cfg();
    io_thread->myCfg->connection.is_RTU = true;

    ThreadControl tc;
    tc.num_connected_threads = 3;

    handleDisconnect(*io_thread->myCfg, tc, io_thread);

    EXPECT_EQ(tc.num_connected_threads, 2);
    delete io_thread->myCfg;
}

// Test setting io_thread->connected to false
TEST(handleDisconnectTest, SetConnectedToFalse)
{
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    io_thread->connected = true;
    io_thread->myCfg = new cfg();
    io_thread->myCfg->connection.is_RTU = false;

    ThreadControl tc;
    tc.num_connected_threads = 1;

    handleDisconnect(*io_thread->myCfg, tc, io_thread);

    EXPECT_FALSE(io_thread->connected);

    delete io_thread->myCfg;
}

// TODO: Figure out how to use gmock with things like threads and mutexes in the structures

// Test that the message is correctly formatted for a Modbus RTU client
// TEST(handleDisconnectTest, ModbusRTUClientMessage)
//     // // EXPECT_CALL(*this, fps_info_log(testing::StrEq(
//     //                        "Disconnecting Modbus RTU client [RTU_Client]. Thread ID: 123. Device: [Device1]")));

// Test that the message is correctly formatted for a Modbus TCP client
// TEST(handleDisconnectTest, ModbusTCPClientMessage)
//     // EXPECT_CALL(*this,
//     //             mock_fps_info_log(testing::StrEq(
//     //                 "Disconnecting Modbus TCP client [TCP_Client]. Thread ID: 456. Host: [192.168.1.1], Port:
//     //                 [502]")));