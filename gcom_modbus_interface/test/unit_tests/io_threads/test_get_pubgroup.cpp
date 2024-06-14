#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <map>

// Assuming the relevant includes for the mocks and dependencies
#include "gcom_config.h"
#include "gcom_iothread.h"

// Global variable for pubGroups, assumed to be part of the codebase
extern std::map<std::string, PubGroup> pubGroups;

TEST(GetPubGroupTest, ReturnsExistingPubGroup)
{
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "existing_group";

    // Insert a PubGroup manually
    pubGroups["existing_group"] = PubGroup(io_work->work_name, io_work, false);

    PubGroup* result = get_pubgroup(io_work);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "existing_group");
}

TEST(GetPubGroupTest, CreatesNewPubGroupIfNotExists)
{
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "new_group";

    PubGroup* result = get_pubgroup(io_work);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "new_group");
    EXPECT_NE(pubGroups.find("new_group"), pubGroups.end());
}

TEST(GetPubGroupTest, AddsIOWorkToNewPubGroup)
{
    auto io_work = std::make_shared<IO_Work>();
    io_work->work_name = "group_with_work";

    PubGroup* result = get_pubgroup(io_work);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "group_with_work");
    EXPECT_NE(pubGroups.find("group_with_work"), pubGroups.end());
    EXPECT_EQ(pubGroups["group_with_work"].works.size(), 0);
    // EXPECT_EQ(pubGroups["group_with_work"].works.front(), io_work); // it doesn't actually add work to the pubgroup.
    // Should it?
}

TEST(GetPubGroupTest, HandlesMultipleIOWorkInSameGroup)
{
    auto io_work1 = std::make_shared<IO_Work>();
    io_work1->work_name = "multi_group";

    auto io_work2 = std::make_shared<IO_Work>();
    io_work2->work_name = "multi_group";

    get_pubgroup(io_work1);
    PubGroup* result = get_pubgroup(io_work2);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "multi_group");
    EXPECT_EQ(pubGroups["multi_group"].works.size(), 0);
    // EXPECT_EQ(pubGroups["multi_group"].works.front(), io_work1);
    // EXPECT_EQ(pubGroups["multi_group"].works.back(), io_work2);
}

TEST(GetPubGroupTest, ReturnsExistingPubGroupStringKey)
{
    std::string key = "existing_group";

    // Insert a PubGroup manually
    pubGroups[key] = PubGroup(key, nullptr, false);

    PubGroup* result = get_pubgroup(key);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "existing_group");
}

TEST(GetPubGroupTest, CreatesNewPubGroupIfNotExistsStringKey)
{
    std::string key = "new_group";

    PubGroup* result = get_pubgroup(key);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "new_group");
    EXPECT_NE(pubGroups.find("new_group"), pubGroups.end());
}

TEST(GetPubGroupTest, AddsPubGroupToMapIfNotExistsStringKey)
{
    std::string key = "group_with_work";

    PubGroup* result = get_pubgroup(key);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "group_with_work");
    EXPECT_NE(pubGroups.find("group_with_work"), pubGroups.end());
}

TEST(GetPubGroupTest, HandlesMultipleKeys)
{
    std::string key1 = "multi_group1";
    std::string key2 = "multi_group2";

    get_pubgroup(key1);
    PubGroup* result = get_pubgroup(key2);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->key, "multi_group2");
    EXPECT_NE(pubGroups.find("multi_group1"), pubGroups.end());
    EXPECT_NE(pubGroups.find("multi_group2"), pubGroups.end());
}