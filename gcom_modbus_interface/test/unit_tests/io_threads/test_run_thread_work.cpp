#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <cstring>

#include "gcom_iothread.h"

TEST(RunThreadWorkTest, HandlesDisconnectedThread)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_thread->connected = false;
    io_thread->ctx = nullptr;  // Mock context
    io_work->device_id = 1;
    io_work->local = false;
    io_work->wtype = WorkTypes::Set;

    // EXPECT_CALL(io_responseChan, send(testing::_)).Times(1);

    runThreadWork(myCfg, io_thread, io_work, false);

    EXPECT_TRUE(io_work->data_error);
}

TEST(RunThreadWorkTest, HandlesLocalWork)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->local = true;
    io_work->num_registers = 10;
    io_work->wtype = WorkTypes::Get;

    // EXPECT_CALL(io_responseChan, send(testing::_)).Times(1);

    runThreadWork(myCfg, io_thread, io_work, false);

    EXPECT_EQ(io_work->good, 20);  // num_registers * 2
}

// TODO: Figure out mocking

// TEST(RunThreadWorkTest, HandlesModbusReadWork)
// {
//     cfg myCfg;
//     auto io_thread = std::make_shared<IO_Thread>();
//     auto io_work = std::make_shared<IO_Work>();
//     io_thread->connected = true;
//     io_thread->ctx = nullptr;  // Mock context
//     io_work->device_id = 1;
//     io_work->local = false;
//     io_work->wtype = WorkTypes::Get;

//     EXPECT_CALL(io_responseChan, send(testing::_)).Times(1);

//     runThreadWork(myCfg, io_thread, io_work, false);

//     EXPECT_EQ(io_work->threadId, io_thread->tid);
//     EXPECT_EQ(io_thread->jobs, 1);
//     EXPECT_TRUE(io_work->tDone > 0);
// }

// TEST(RunThreadWorkTest, HandlesModbusPollWork)
// {
//     cfg myCfg;
//     auto io_thread = std::make_shared<IO_Thread>();
//     auto io_work = std::make_shared<IO_Work>();
//     io_thread->connected = true;
//     io_thread->ctx = nullptr;  // Mock context
//     io_work->device_id = 1;
//     io_work->local = false;
//     io_work->wtype = WorkTypes::Poll;

//     EXPECT_CALL(io_responseChan, send(testing::_)).Times(1);

//     runThreadWork(myCfg, io_thread, io_work, false);

//     EXPECT_EQ(io_work->threadId, io_thread->tid);
//     EXPECT_EQ(io_thread->jobs, 1);
//     EXPECT_TRUE(io_work->tDone > 0);
// }

// TEST(RunThreadWorkTest, ExecutesModbusIOWork)
// {
//     cfg myCfg;
//     auto io_thread = std::make_shared<IO_Thread>();
//     auto io_work = std::make_shared<IO_Work>();
//     io_thread->connected = true;
//     io_thread->ctx = nullptr;  // Mock context
//     io_work->device_id = 1;
//     io_work->local = false;
//     io_work->wtype = WorkTypes::Set;

//     EXPECT_CALL(io_responseChan, send(testing::_)).Times(1);

//     runThreadWork(myCfg, io_thread, io_work, false);

//     EXPECT_EQ(io_work->threadId, io_thread->tid);
//     EXPECT_EQ(io_thread->jobs, 1);
//     EXPECT_TRUE(io_work->tDone > 0);
// }