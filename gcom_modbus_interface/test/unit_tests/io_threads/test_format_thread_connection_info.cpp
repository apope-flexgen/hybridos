#include <gtest/gtest.h>
#include <gmock/gmock.h>  // For mock objects
#include "gcom_iothread.h"

extern ThreadControl threadControl;

// Test case for empty ioThreadPool
TEST(formatThreadConnectionInfoTest, EmptyThreadPool)
{
    std::stringstream ss;
    formatThreadConnectionInfo(ss, true);
    EXPECT_EQ(ss.str(), "\"thread_connection_info\": []");
}

// Test case for a single connected TCP client
TEST(formatThreadConnectionInfoTest, SingleConnectedTCPClient)
{
    cfg myCfg;
    myCfg.connection.is_RTU = false;
    myCfg.connection.device_name = "";
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->connected = true;
    io_thread->is_local = false;
    io_thread->port = 502;
    io_thread->ip = "192.168.1.1";
    io_thread->connect_time = 10.0;
    io_thread->jobs = 5;
    io_thread->fails = 0;
    io_thread->myCfg = &myCfg;
    threadControl.ioThreadPool.push_back(io_thread);

    std::stringstream ss;
    formatThreadConnectionInfo(ss, true);

    std::string expected =
        "\"thread_connection_info\": [{\"id\":1,\"connected\":true,\"ip_address\": \"192.168.1.1\",\"port\":502,\"time_to_connect\":\"10 ms\",\"modbus_read_times\":{\"Max\": 0,\"Min\": 0,\"Avg\": 0,\"Count\": 0},\"modbus_write_times\":{\"Max\": 0,\"Min\": 0,\"Avg\": 0,\"Count\": 0},\"num_jobs\":5,\"num_fails\":0}]";
    EXPECT_EQ(ss.str(), expected);
}