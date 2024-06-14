#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "gcom_iothread.h"

extern int modbus_get_socket_ret;

TEST(WaitSocketReadyTest, InvalidSocket)
{
    modbus_t* ctx = nullptr;
    modbus_get_socket_ret = -1;
    int result = wait_socket_ready(ctx, 1);
    EXPECT_EQ(result, -1);
}

TEST(WaitSocketReadyTest, SocketReady)
{
    modbus_t* ctx = nullptr;
    modbus_get_socket_ret = 1;
    int result = wait_socket_ready(ctx, 1);
    EXPECT_EQ(result, 0);
}

TEST(WaitSocketReadyTest, SocketNotReady)
{
    modbus_t* ctx = nullptr;
    modbus_get_socket_ret = 0;
    int result = wait_socket_ready(ctx, 1);
    EXPECT_EQ(result, -1);
}