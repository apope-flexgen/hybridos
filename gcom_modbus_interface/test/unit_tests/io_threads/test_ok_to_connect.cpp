#include <gtest/gtest.h>
#include <mutex>
#include "gcom_iothread.h"

// Test when tNow is less than tc.tConnect
TEST(OkToConnectTest, tNowLessThanTConnect)
{
    ThreadControl tc;
    tc.tConnect = 10.0;
    double tNow = 5.0;
    EXPECT_FALSE(OkToConnect(tc, tNow));
}

// Test when tNow is equal to tc.tConnect
TEST(OkToConnectTest, tNowEqualToTConnect)
{
    ThreadControl tc;
    tc.tConnect = 10.0;
    double tNow = 10.0;
    EXPECT_TRUE(OkToConnect(tc, tNow));
}

// Test when tNow is greater than tc.tConnect
TEST(OkToConnectTest, tNowGreaterThanTConnect)
{
    ThreadControl tc;
    tc.tConnect = 10.0;
    double tNow = 15.0;
    EXPECT_TRUE(OkToConnect(tc, tNow));
}