#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <chrono>
#include <thread>

// Assuming the relevant includes for the mocks and dependencies
#include "gcom_config.h"
#include "gcom_iothread.h"

extern ioChannel<std::shared_ptr<IO_Work>> io_responseChan;

TEST(ProcessIOWorkResponseThreadTest, ProcessesIOWorkCorrectly)
{
    cfg myCfg;
    ThreadControl control;
    std::thread thread([&]() { processIOWorkResponseThread(control, myCfg); });

    // Allow the thread to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    control.responseThreadRunning = false;
    thread.join();

    EXPECT_EQ(control.num_responses, 3);
    EXPECT_GT(control.tResponse, 0.0);
}

TEST(ProcessIOWorkResponseThreadTest, StopsRunningWhenFlagIsFalse)
{
    cfg myCfg;
    ThreadControl control;
    control.responseThreadRunning = false;
    processIOWorkResponseThread(control, myCfg);

    EXPECT_EQ(control.num_responses, 0);
    EXPECT_EQ(control.tResponse, 0.0);
}

TEST(ProcessIOWorkResponseThreadTest, UpdatesResponseTimeCorrectly)
{
    cfg myCfg;
    ThreadControl control;
    std::thread thread([&]() { processIOWorkResponseThread(control, myCfg); });
    std::shared_ptr<IO_Work> io_work = std::make_shared<IO_Work>();
    io_work->tStart = get_time_double();
    io_responseChan.send(std::move(io_work));

    // Allow the thread to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    control.responseThreadRunning = false;
    thread.join();

    EXPECT_GT(control.tResponse, 0.0);
}