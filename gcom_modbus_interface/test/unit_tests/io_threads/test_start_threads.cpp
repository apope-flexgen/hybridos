#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include "gcom_iothread.h"

extern ThreadControl threadControl;

TEST(StartThreadsTest, StartsResponseThread)
{
    threadControl.ioThreadPool.clear();
    cfg myCfg;

    StartThreads(1, "127.0.0.1", 502, 10.0, 5.0, myCfg);

    EXPECT_EQ(threadControl.ioThreadPool.size(), 2);

    StopThreads(myCfg, false);
}

TEST(StartThreadsTest, StartsLocalThread)
{
    threadControl.ioThreadPool.clear();
    cfg myCfg;

    StartThreads(1, "127.0.0.1", 502, 10.0, 5.0, myCfg);

    ASSERT_EQ(threadControl.ioThreadPool.size(), 2);
    EXPECT_TRUE(threadControl.ioThreadPool[0]->is_local);

    StopThreads(myCfg, false);
}

TEST(StartThreadsTest, StartsMultipleThreads)
{
    threadControl.ioThreadPool.clear();
    cfg myCfg;

    StartThreads(3, "127.0.0.1", 502, 10.0, 5.0, myCfg);

    EXPECT_EQ(threadControl.ioThreadPool.size(), 4);

    StopThreads(myCfg, false);
}