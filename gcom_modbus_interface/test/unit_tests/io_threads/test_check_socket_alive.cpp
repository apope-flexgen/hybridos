#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <unistd.h>

#include "gcom_iothread.h"

extern int modbus_get_socket_ret;

TEST(CheckSocketAliveTest, InvalidSocket)
{
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    modbus_get_socket_ret = -1;

    int result = check_socket_alive(io_thread, 1, 0);

    EXPECT_EQ(result, -1);
}

TEST(CheckSocketAliveTest, SocketTimeout)
{
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    modbus_get_socket_ret = 0;

    int result = check_socket_alive(io_thread, 1, 0);
    EXPECT_EQ(result, 0);
}

TEST(CheckSocketAliveTest, SocketReadable)
{
    std::shared_ptr<IO_Thread> io_thread = std::make_shared<IO_Thread>();
    modbus_get_socket_ret = 1;

    int result = check_socket_alive(io_thread, 1, 0);
    EXPECT_EQ(result, 1);
}
