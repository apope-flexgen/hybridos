#include <gtest/gtest.h>
#include <mutex>
#include "gcom_config.h"
#include "gcom_iothread.h"

// Test when reconnect_delay is positive
TEST(DelayConnectTest, ReconnectDelayPositive)
{
    cfg myCfg;
    myCfg.reconnect_delay = 10.0;
    ThreadControl tc;
    DelayConnect(myCfg, tc);
    double tNow = get_time_double();
    ASSERT_NEAR(tc.tConnect, tNow + myCfg.reconnect_delay, 0.001);
}

// Test when reconnect_delay is zero
TEST(DelayConnectTest, ReconnectDelayZero)
{
    cfg myCfg;
    myCfg.reconnect_delay = 0.0;
    ThreadControl tc;
    DelayConnect(myCfg, tc);
    double tNow = get_time_double();
    ASSERT_NEAR(tc.tConnect, tNow, 0.001);
}

// Test when reconnect_delay is negative
TEST(DelayConnectTest, ReconnectDelayNegative)
{
    cfg myCfg;
    myCfg.reconnect_delay = -5.0;
    ThreadControl tc;
    DelayConnect(myCfg, tc);
    double tNow = get_time_double();
    ASSERT_NEAR(tc.tConnect, tNow + myCfg.reconnect_delay, 0.001);
}