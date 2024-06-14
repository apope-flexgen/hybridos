#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "gcom_iothread.h"

extern std::map<std::string, PubGroup> pubGroups;

TEST(CheckPubGroupKeyTest, AddsNewKeyIfNotExists)
{
    std::string key = "test_key";
    auto io_work = std::make_shared<IO_Work>();

    bool result = check_pubgroup_key(key, io_work);

    EXPECT_TRUE(result);
    ASSERT_EQ(pubGroups.size(), 1);
    EXPECT_EQ(pubGroups[key].key, key);
    EXPECT_EQ(pubGroups[key].works.size(), 0);
    pubGroups.clear();
}

TEST(CheckPubGroupKeyTest, DoesNotAddKeyIfExists)
{
    std::string key = "test_key";
    auto io_work1 = std::make_shared<IO_Work>();
    auto io_work2 = std::make_shared<IO_Work>();

    pubGroups[key] = PubGroup(key, io_work1, false);

    bool result = check_pubgroup_key(key, io_work2);

    EXPECT_TRUE(result);
    ASSERT_EQ(pubGroups.size(), 1);
    EXPECT_EQ(pubGroups[key].works.size(), 0);
    pubGroups.clear();
}

TEST(CheckPubGroupKeyTest, HandlesEmptyKey)
{
    std::string key = "";
    auto io_work = std::make_shared<IO_Work>();

    bool result = check_pubgroup_key(key, io_work);

    EXPECT_TRUE(result);
    ASSERT_EQ(pubGroups.size(), 1);
    EXPECT_EQ(pubGroups[key].key, key);
    EXPECT_EQ(pubGroups[key].works.size(), 0);
    pubGroups.clear();
}

TEST(CheckPubGroupKeyTest, HandlesNullIOWork)
{
    std::string key = "test_key";
    std::shared_ptr<IO_Work> io_work = nullptr;

    bool result = check_pubgroup_key(key, io_work);

    EXPECT_TRUE(result);
    ASSERT_EQ(pubGroups.size(), 1);
    EXPECT_EQ(pubGroups[key].key, key);
    EXPECT_EQ(pubGroups[key].works.size(), 0);
    pubGroups.clear();
}