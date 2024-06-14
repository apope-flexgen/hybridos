#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <sstream>

#include "gcom_iothread.h"

extern ioChannel<std::shared_ptr<IO_Work>> io_poolChan;

TEST(ProcessGroupCallbackTest, ProcessesPubGroup)
{
    cfg myCfg;
    myCfg.connection.debug = false;
    myCfg.fims_connected = true;

    PubGroup pub_group;
    pub_group.key = "pub_key";
    pub_group.tNow = get_time_double();
    pub_group.done = false;
    pub_group.pub_struct = std::make_shared<cfg::pub_struct>();

    auto io_work = std::make_shared<IO_Work>();
    io_work->pub_struct = pub_group.pub_struct;
    io_work->good = 1;
    io_work->component = new cfg::component_struct();
    pub_group.pub_struct->component = io_work->component;
    pub_group.works.push_back(io_work);

    processGroupCallback(pub_group, myCfg);

    EXPECT_TRUE(pub_group.done);
    delete io_work->component;
}

TEST(ProcessGroupCallbackTest, ProcessesSetGroup)
{
    cfg myCfg;
    myCfg.connection.debug = false;
    myCfg.fims_connected = true;

    PubGroup pub_group;
    pub_group.key = "set_key";
    pub_group.tNow = get_time_double();
    pub_group.done = false;
    pub_group.pub_struct = std::make_shared<cfg::pub_struct>();

    auto io_work = std::make_shared<IO_Work>();
    io_work->pub_struct = pub_group.pub_struct;
    io_work->replyto = "/reply_uri";
    io_work->component = new cfg::component_struct();
    pub_group.pub_struct->component = io_work->component;
    pub_group.works.push_back(io_work);

    processGroupCallback(pub_group, myCfg);

    EXPECT_TRUE(pub_group.done);
    delete io_work->component;
}

TEST(ProcessGroupCallbackTest, ProcessesGetGroup)
{
    cfg myCfg;
    myCfg.connection.debug = false;
    myCfg.fims_connected = true;

    PubGroup pub_group;
    pub_group.key = "get_key";
    pub_group.tNow = get_time_double();
    pub_group.done = false;
    pub_group.pub_struct = std::make_shared<cfg::pub_struct>();

    auto io_work = std::make_shared<IO_Work>();
    io_work->pub_struct = pub_group.pub_struct;
    io_work->replyto = "/reply_uri";
    io_work->component = new cfg::component_struct();
    pub_group.pub_struct->component = io_work->component;
    pub_group.works.push_back(io_work);

    processGroupCallback(pub_group, myCfg);

    EXPECT_TRUE(pub_group.done);
    delete io_work->component;
}

TEST(ProcessGroupCallbackTest, HandlesHeartbeat)
{
    cfg myCfg;
    myCfg.connection.debug = false;
    myCfg.fims_connected = true;

    PubGroup pub_group;
    pub_group.key = "pub_key";
    pub_group.tNow = get_time_double();
    pub_group.done = false;
    pub_group.pub_struct = std::make_shared<cfg::pub_struct>();

    auto io_work = std::make_shared<IO_Work>();
    io_work->pub_struct = pub_group.pub_struct;
    io_work->good = 1;
    auto component = std::make_shared<cfg::component_struct>();
    io_work->component = component.get();
    component->heartbeat_enabled = true;
    component->heartbeat = new cfg::heartbeat_struct();
    pub_group.pub_struct->component = io_work->component;
    pub_group.works.push_back(io_work);

    processGroupCallback(pub_group, myCfg);

    EXPECT_TRUE(pub_group.done);
    delete component->heartbeat;
}
