#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "gcom_iothread.h"

extern ioChannel<std::shared_ptr<IO_Work>> io_poolChan;
extern int num_work;
extern int max_work_wait;

TEST(MakeWorkTest, CreateNewIOWorkWhenPoolIsEmpty)
{
    std::shared_ptr<IO_Work> io_work_placeholder;
    while (io_poolChan.peekpop(io_work_placeholder))
    {
    }
    int device_id = 1;
    int offset = 10;
    bool off_by_one = false;
    int num_regs = 5;
    uint16_t u16bufs[5] = { 1, 2, 3, 4, 5 };
    uint8_t u8bufs[5] = { 6, 7, 8, 9, 10 };
    WorkTypes wtype = WorkTypes::Set;
    cfg::component_struct* component = nullptr;  // Replace with actual component if needed

    auto io_work = make_work(component, cfg::Register_Types::Holding, device_id, offset, off_by_one, num_regs, u16bufs,
                             u8bufs, wtype);

    ASSERT_NE(io_work, nullptr);
    EXPECT_EQ(io_work->device_id, device_id);
    EXPECT_EQ(io_work->offset, offset);
    EXPECT_EQ(io_work->off_by_one, off_by_one);
    EXPECT_EQ(io_work->num_registers, num_regs);
    EXPECT_EQ(io_work->register_type, cfg::Register_Types::Holding);
    EXPECT_EQ(io_work->wtype, wtype);
    EXPECT_FALSE(io_work->local);
    EXPECT_FALSE(io_work->data_error);
}

TEST(MakeWorkTest, ReuseExistingIOWorkFromPool)
{
    std::shared_ptr<IO_Work> io_work_placeholder;
    while (io_poolChan.peekpop(io_work_placeholder))
    {
    }
    auto existing_io_work = std::make_shared<IO_Work>();
    IO_Work* ptr = existing_io_work.get();
    io_poolChan.send(std::move(existing_io_work));
    int device_id = 2;
    int offset = 20;
    bool off_by_one = true;
    int num_regs = 3;
    uint16_t u16bufs[3] = { 11, 12, 13 };
    uint8_t u8bufs[3] = { 14, 15, 16 };
    WorkTypes wtype = WorkTypes::Get;
    cfg::component_struct* component = nullptr;  // Replace with actual component if needed

    auto io_work = make_work(component, cfg::Register_Types::Input, device_id, offset, off_by_one, num_regs, u16bufs,
                             u8bufs, wtype);

    ASSERT_NE(io_work, nullptr);
    EXPECT_EQ(io_work.get(), ptr);
    EXPECT_EQ(io_work->device_id, device_id);
    EXPECT_EQ(io_work->offset, offset);
    EXPECT_EQ(io_work->off_by_one, off_by_one);
    EXPECT_EQ(io_work->num_registers, num_regs);
    EXPECT_EQ(io_work->register_type, cfg::Register_Types::Input);
    EXPECT_EQ(io_work->wtype, wtype);
    EXPECT_FALSE(io_work->local);
    EXPECT_FALSE(io_work->data_error);
}

TEST(MakeWorkTest, FailsToGetIOWorkWithinMaxWaitTime)
{
    std::shared_ptr<IO_Work> io_work_placeholder;
    while (io_poolChan.peekpop(io_work_placeholder))
    {
    }
    num_work = 20001;
    max_work_wait = 0;
    int device_id = 3;
    int offset = 30;
    bool off_by_one = false;
    int num_regs = 4;
    uint16_t u16bufs[4] = { 17, 18, 19, 20 };
    uint8_t u8bufs[4] = { 21, 22, 23, 24 };
    WorkTypes wtype = WorkTypes::Poll;
    cfg::component_struct* component = nullptr;  // Replace with actual component if needed

    auto io_work = make_work(component, cfg::Register_Types::Coil, device_id, offset, off_by_one, num_regs, u16bufs,
                             u8bufs, wtype);

    ASSERT_EQ(io_work, nullptr);
    max_work_wait = 5000;
}
