#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "gcom_config.h"
#include "gcom_iothread.h"

extern ioChannel<std::shared_ptr<IO_Work>> io_poolChan;

TEST(StashWorkTest, StashesWorkCorrectly)
{
    auto io_work = std::make_shared<IO_Work>();

    stashWork(io_work);

    EXPECT_TRUE(io_poolChan.peekpop(io_work));
}

TEST(StashWorkTest, HandlesNullIOWork)
{
    std::shared_ptr<IO_Work> io_work = nullptr;

    stashWork(io_work);

    EXPECT_TRUE(io_poolChan.peekpop(io_work));
}