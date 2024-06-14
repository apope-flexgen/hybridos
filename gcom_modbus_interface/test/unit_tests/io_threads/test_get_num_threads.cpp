#include <gtest/gtest.h>
#include <gmock/gmock.h>  // For mock objects
#include "gcom_iothread.h"

extern ThreadControl threadControl;

// Test when num_connected_threads is 0
TEST(GetNumThreadsTestTwoParam, ZeroThreads)
{
    cfg myCfg;
    ThreadControl tc;
    tc.num_connected_threads = 0;

    EXPECT_EQ(GetNumThreads(myCfg, tc), 0);
}

// Test when num_connected_threads is greater than 0
TEST(GetNumThreadsTestTwoParam, PositiveThreads)
{
    cfg myCfg;
    ThreadControl tc;
    tc.num_connected_threads = 5;

    EXPECT_EQ(GetNumThreads(myCfg, tc), 5);
}

// Test when num_connected_threads is negative (assuming it should not be negative, but including for completeness)
TEST(GetNumThreadsTestTwoParam, NegativeThreads)
{
    cfg myCfg;
    ThreadControl tc;
    tc.num_connected_threads = -3;

    EXPECT_EQ(GetNumThreads(myCfg, tc), -3);
}

// Test when num_connected_threads is 0
TEST(GetNumThreadsTestOneParam, ZeroThreads)
{
    cfg myCfg;
    threadControl.num_connected_threads = 0;

    EXPECT_EQ(GetNumThreads(&myCfg), 0);
}

// Test when num_connected_threads is greater than 0
TEST(GetNumThreadsTestOneParam, PositiveThreads)
{
    cfg myCfg;
    threadControl.num_connected_threads = 5;

    EXPECT_EQ(GetNumThreads(&myCfg), 5);
}

// Test when num_connected_threads is negative (assuming it should not be negative, but including for completeness)
TEST(GetNumThreadsTestOneParam, NegativeThreads)
{
    cfg myCfg;
    threadControl.num_connected_threads = -3;

    EXPECT_EQ(GetNumThreads(&myCfg), -3);
}