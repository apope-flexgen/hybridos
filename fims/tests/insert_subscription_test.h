//============================================================================
// Name        : Insert_Subscription_Test.cpp
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Test for insertion of subscription into subscription hash in
//               FIMS_Server.
//============================================================================

#include "fims_test_utilities.h"

class InsertSubscriptionTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        
    }

    virtual void TearDown()
    {
        // clear subc_table and its chains
        clear_subscriptions();
    }
};

TEST_F(InsertSubscriptionTest, InsertMonoSubscription)
{
    insert_subscription("/path", 65, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[65/32] & 1<<(65%32));
}

TEST_F(InsertSubscriptionTest, InsertTwoLevelPathSubscription)
{
    insert_subscription("/path/to", 88, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    hash = strtohash("to") % SUB_HASH_SIZE;
    cur = cur->next[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[88/32] & 1<<(88%32));
}

TEST_F(InsertSubscriptionTest, InsertMultiLevelPathSubscription)
{
    insert_subscription("/path/to/message", 122, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    hash = strtohash("to") % SUB_HASH_SIZE;
    cur = cur->next[hash];
    hash = strtohash("message") % SUB_HASH_SIZE;
    cur = cur->next[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[122/32] & 1<<(122%32));
}

TEST_F(InsertSubscriptionTest, InsertSubscriptionToChildPath)
{
    insert_subscription("/path", 37, 0);
    insert_subscription("/path/to", 47, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[37/32] & 1<<(37%32));
    EXPECT_FALSE(cur->non_pub_conn_mask[47/32] & 1<<(47%32));
    hash = strtohash("to") % SUB_HASH_SIZE;
    cur = cur->next[hash];
    EXPECT_FALSE(cur->non_pub_conn_mask[37/32] & 1<<(37%32));
    EXPECT_TRUE(cur->non_pub_conn_mask[47/32] & 1<<(47%32));
}

TEST_F(InsertSubscriptionTest, InsertSubscriptionWithIntersect)
{
    insert_subscription("/path", 44, 0);
    insert_subscription("/former/path", 76, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[44/32] & 1<<(44%32));
    hash = strtohash("former") % HASH_TABLE_SIZE;
    cur = subc_table[hash];
    hash = strtohash("path") % SUB_HASH_SIZE;
    cur = cur->next[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[76/32] & 1<<(76%32));
    EXPECT_FALSE(cur->non_pub_conn_mask[44/32] & 1<<(44%32));
}

TEST_F(InsertSubscriptionTest, InsertMultipleSubscriptionsToPath)
{
    insert_subscription("/path/to", 57, 0);
    insert_subscription("/path/to", 58, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    hash = strtohash("to") % SUB_HASH_SIZE;
    cur = cur->next[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[57/32] & 1<<(57%32));
    EXPECT_TRUE(cur->non_pub_conn_mask[58/32] & 1<<(58%32));
}

TEST_F(InsertSubscriptionTest, InsertMultipleSubscriptionsFromClient)
{
    insert_subscription("/path", 1, 0);
    insert_subscription("/directory", 1, 0);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[1/32] & 1<<(1%32));
    hash = strtohash("directory") % HASH_TABLE_SIZE;
    cur = subc_table[hash];
    EXPECT_TRUE(cur->non_pub_conn_mask[1/32] & 1<<(1%32));
}

TEST_F(InsertSubscriptionTest, InsertMultipleCollisionsFromSubMask)
{
    // all of the components work out to a hash of 11 causing them all to be inserted into the collision list
    insert_subscription("/components/sel_735", 22, false);
    insert_subscription("/components/sma_pv_6", 22, false);
    insert_subscription("/components/sma_pv_2", 22, false);
    insert_subscription("/components/sungrow_ess_1", 22, false);
    //verify list is alphabetical
    int hash = strtohash("components") % HASH_TABLE_SIZE;
    //verify hash function has not changed
    EXPECT_THAT(hash, 909);
    subscription_hash *cur = subc_table[hash];
    EXPECT_FALSE(cur == NULL);
    EXPECT_TRUE(strcmp(cur->name, "components") == 0);
    hash = strtohash("sel_735") % SUB_HASH_SIZE;
    EXPECT_THAT(hash, 11);
    hash = strtohash("sma_pv_2") % SUB_HASH_SIZE;
    EXPECT_THAT(hash, 11);
    hash = strtohash("sma_pv_6") % SUB_HASH_SIZE;
    EXPECT_THAT(hash, 11);
    hash = strtohash("sungrow_ess_1") % SUB_HASH_SIZE;
    EXPECT_THAT(hash, 11);
    cur = cur->next[hash];
    EXPECT_FALSE(cur == NULL);
    EXPECT_TRUE(strcmp(cur->name, "sel_735") == 0);
    cur = cur->collision;
    EXPECT_FALSE(cur == NULL);
    EXPECT_TRUE(strcmp(cur->name, "sma_pv_2") == 0);
    cur = cur->collision;
    EXPECT_FALSE(cur == NULL);
    EXPECT_TRUE(strcmp(cur->name, "sma_pv_6") == 0);
    cur = cur->collision;
    EXPECT_FALSE(cur == NULL);
    EXPECT_TRUE(strcmp(cur->name, "sungrow_ess_1") == 0);
    cur = cur->collision;
    EXPECT_TRUE(cur == NULL);
}


TEST_F(InsertSubscriptionTest, InsertPubOnlySubscription)
{
    insert_subscription("/path", 87, 1);
    int hash = strtohash("path") % HASH_TABLE_SIZE;
    subscription_hash *cur = subc_table[hash];
    EXPECT_TRUE(cur->pub_only_conn_mask[87/32] & 1<<(87%32));
    EXPECT_FALSE(cur->non_pub_conn_mask[87/32] & 1<<(87%32));
}

TEST_F(InsertSubscriptionTest, InsertSubToAll)
{
    insert_subscription("/", 54, 0);
    EXPECT_TRUE(non_pub_base_mask[54/32] & 1<<(54%32));
}
