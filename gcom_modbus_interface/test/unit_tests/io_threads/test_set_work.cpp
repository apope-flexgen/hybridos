#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "gcom_config.h"
#include "gcom_iothread.h"

extern ioChannel<std::shared_ptr<IO_Work>> io_setChan;       // Use Channel to send IO-Work to thread
extern ioChannel<std::shared_ptr<IO_Work>> io_localsetChan;  // Use Channel to send IO-Work to thread
extern ioChannel<int> io_threadChan;                         // Thread Control
extern ioChannel<int> io_localthreadChan;                    // Thread Control

TEST(SetWorkTest, SendsToLocalSetChannel)
{
    std::shared_ptr<IO_Work> io_work_placeholder;
    while (io_localsetChan.peekpop(io_work_placeholder))
    {
    }
    int signal = 0;
    while (io_localthreadChan.peekpop(signal))
    {
    }
    auto io_work = std::make_shared<IO_Work>();
    IO_Work* io_work_ptr = io_work.get();
    io_work->local = true;

    bool result = setWork(io_work);

    EXPECT_TRUE(result);
    io_localsetChan.peekpop(io_work_placeholder);
    EXPECT_EQ(io_work_placeholder.get(), io_work_ptr);  // io_work should be moved
    io_localthreadChan.peekpop(signal);
    EXPECT_EQ(signal, 1);
}

TEST(SetWorkTest, SendsToRemoteSetChannel)
{
    std::shared_ptr<IO_Work> io_work_placeholder;
    while (io_setChan.peekpop(io_work_placeholder))
    {
    }
    int signal = 0;
    while (io_threadChan.peekpop(signal))
    {
    }
    auto io_work = std::make_shared<IO_Work>();
    IO_Work* io_work_ptr = io_work.get();
    io_work->local = false;

    bool result = setWork(io_work);

    EXPECT_TRUE(result);
    io_setChan.peekpop(io_work_placeholder);
    EXPECT_EQ(io_work_placeholder.get(), io_work_ptr);  // io_work should be moved
    io_threadChan.peekpop(signal);
    EXPECT_EQ(signal, 1);
}

TEST(SetWorkTest, HandlesNullIOWork)
{
    std::shared_ptr<IO_Work> io_work = nullptr;

    bool result = setWork(io_work);

    EXPECT_TRUE(result);
}