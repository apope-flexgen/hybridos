#include <gtest/gtest.h>
#include <memory>
#include "gcom_config.h"
#include "gcom_iothread.h"

TEST(MakeIOThreadTest, InitializesCorrectly)
{
    cfg myCfg;
    int idx = 1;
    const char* ip = "127.0.0.1";
    int port = 502;
    double connection_timeout = 10.0;
    double transfer_timeout = 5.0;

    std::shared_ptr<IO_Thread> io_thread = make_IO_Thread(idx, ip, port, connection_timeout, transfer_timeout, myCfg);

    ASSERT_NE(io_thread, nullptr);
    EXPECT_EQ(io_thread->tid, idx);
    EXPECT_EQ(io_thread->ip, ip);
    EXPECT_EQ(io_thread->port, port);
    EXPECT_EQ(io_thread->connection_timeout, connection_timeout);
    EXPECT_EQ(io_thread->transfer_timeout, transfer_timeout);
    EXPECT_EQ(io_thread->jobs, 0);
    EXPECT_EQ(io_thread->fails, 0);
    EXPECT_EQ(io_thread->myCfg, &myCfg);
}

TEST(MakeIOThreadTest, HandlesNullIp)
{
    cfg myCfg;
    int idx = 1;
    const char* ip = nullptr;
    int port = 502;
    double connection_timeout = 10.0;
    double transfer_timeout = 5.0;

    std::shared_ptr<IO_Thread> io_thread = make_IO_Thread(idx, ip, port, connection_timeout, transfer_timeout, myCfg);

    ASSERT_NE(io_thread, nullptr);
    EXPECT_EQ(io_thread->tid, idx);
    EXPECT_EQ(io_thread->ip, "");  // Should be empty
    EXPECT_EQ(io_thread->port, port);
    EXPECT_EQ(io_thread->connection_timeout, connection_timeout);
    EXPECT_EQ(io_thread->transfer_timeout, transfer_timeout);
    EXPECT_EQ(io_thread->jobs, 0);
    EXPECT_EQ(io_thread->fails, 0);
    EXPECT_EQ(io_thread->myCfg, &myCfg);
}
