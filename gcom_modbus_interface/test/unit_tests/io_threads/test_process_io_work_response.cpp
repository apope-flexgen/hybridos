#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "gcom_config.h"
#include "gcom_iothread.h"

extern std::map<std::string, PubGroup> pubGroups;

TEST(ProcessIOWorkResponseTest, HandlesLocalWriteRegisterWork)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "test_work";
    io_work->local = true;
    io_work->wtype = WorkTypes::Set;
    io_work->register_type = cfg::Register_Types::Holding;

    processIOWorkResponse(io_work, myCfg);

    // No assertions for void function
    // Just make sure it doesn't crash, I guess

    pubGroups.clear();
}

TEST(ProcessIOWorkResponseTest, HandlesLocalReadRegisterWork)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "test_work";
    io_work->local = true;
    io_work->wtype = WorkTypes::Get;
    io_work->register_type = cfg::Register_Types::Holding;

    processIOWorkResponse(io_work, myCfg);

    // No assertions for void function
    // Just make sure it doesn't crash, I guess

    pubGroups.clear();
}

TEST(ProcessIOWorkResponseTest, HandlesPubGroupKeyCheck)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "test_pubgroup";
    io_work->local = false;
    io_work->wtype = WorkTypes::Get;
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->pub_struct = std::make_shared<cfg::pub_struct>();

    processIOWorkResponse(io_work, myCfg);

    EXPECT_EQ(pubGroups.size(), 1);
    EXPECT_NE(pubGroups.find(io_work->work_name), pubGroups.end());

    pubGroups.clear();
}

TEST(ProcessIOWorkResponseTest, DiscardsStaleWork)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "test_pubgroup";
    io_work->tNow = 10.0;
    io_work->local = false;
    io_work->wtype = WorkTypes::Get;
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->pub_struct = std::make_shared<cfg::pub_struct>();

    pubGroups[io_work->work_name] = PubGroup(io_work->work_name, io_work, false);
    pubGroups[io_work->work_name].tNow = 20.0;

    processIOWorkResponse(io_work, myCfg);

    EXPECT_EQ(pubGroups.size(), 1);
    EXPECT_EQ(pubGroups[io_work->work_name].tNow, 20.0);

    pubGroups.clear();
}

TEST(ProcessIOWorkResponseTest, ProcessesGroupCallbackWhenWorkComplete)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "test_pubgroup";
    io_work->tNow = 10.0;
    io_work->local = false;
    io_work->work_group = 1;
    io_work->wtype = WorkTypes::Get;
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->pub_struct = std::make_shared<cfg::pub_struct>();

    pubGroups[io_work->work_name] = PubGroup(io_work->work_name, io_work, false);
    pubGroups[io_work->work_name].tNow = 10.0;
    pubGroups[io_work->work_name].work_group = 1;

    processIOWorkResponse(io_work, myCfg);

    EXPECT_TRUE(pubGroups[io_work->work_name].done);

    pubGroups.clear();
}