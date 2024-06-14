#include <gtest/gtest.h>

#include "gcom_iothread.h"

TEST(DiscardGroupCallbackTest, ClearsWorks)
{
    cfg myCfg;
    myCfg.debug_pub = false;

    PubGroup pub_group;
    pub_group.key = "test_key";
    pub_group.tNow = get_time_double();

    auto io_work = std::make_shared<IO_Work>();
    io_work->num_registers = 5;
    pub_group.works.push_back(io_work);

    discardGroupCallback(pub_group, myCfg, true);

    EXPECT_TRUE(pub_group.works.empty());
}

TEST(DiscardGroupCallbackTest, UpdatesPubStructTime)
{
    cfg myCfg;
    myCfg.debug_pub = false;

    PubGroup pub_group;
    pub_group.key = "test_key";
    pub_group.tNow = get_time_double();
    double original_time = pub_group.tNow;

    auto io_work = std::make_shared<IO_Work>();
    io_work->num_registers = 5;
    auto pub_struct = std::make_shared<cfg::pub_struct>();
    io_work->pub_struct = pub_struct;
    pub_group.works.push_back(io_work);

    discardGroupCallback(pub_group, myCfg, true);

    EXPECT_GT(pub_struct->comp_time, original_time);
}

TEST(DiscardGroupCallbackTest, ResetsIOWorkFields)
{
    cfg myCfg;
    myCfg.debug_pub = false;

    PubGroup pub_group;
    pub_group.key = "test_key";
    pub_group.tNow = get_time_double();

    auto io_work = std::make_shared<IO_Work>();
    io_work->num_registers = 5;
    io_work->errno_code = 1;
    io_work->data_error = true;
    pub_group.works.push_back(io_work);

    discardGroupCallback(pub_group, myCfg, true);

    EXPECT_EQ(io_work->errno_code, 0);
    EXPECT_FALSE(io_work->data_error);
}

TEST(DiscardGroupCallbackTest, HandlesDebugLoggingWhenOk)
{
    cfg myCfg;
    myCfg.debug_pub = true;

    PubGroup pub_group;
    pub_group.key = "test_key";
    pub_group.tNow = get_time_double();

    auto io_work = std::make_shared<IO_Work>();
    io_work->num_registers = 5;
    pub_group.works.push_back(io_work);

    testing::internal::CaptureStdout();
    discardGroupCallback(pub_group, myCfg, true);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("completing   pubgroup"), std::string::npos);
}

TEST(DiscardGroupCallbackTest, HandlesDebugLoggingWhenNotOk)
{
    cfg myCfg;
    myCfg.debug_pub = true;

    PubGroup pub_group;
    pub_group.key = "test_key";
    pub_group.tNow = get_time_double();

    auto io_work = std::make_shared<IO_Work>();
    io_work->num_registers = 5;
    pub_group.works.push_back(io_work);

    testing::internal::CaptureStdout();
    discardGroupCallback(pub_group, myCfg, false);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("dropping   pubgroup"), std::string::npos);
}