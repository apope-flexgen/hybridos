//============================================================================
// Name        : Remove_Subscription_Test.cpp
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Test for removal of subscriptions from subscription hash.
//============================================================================

#include "fims_test_utilities.h"

class RemoveSubscriptionTest : public testing::Test
{
public:
    char *path1;
    char *path2;
    char *path3;
    subscription_hash *path;
    subscription_hash *to;
    subscription_hash *of;
    subscription_hash *message;
    uint32_t thread_mask[SUB_MASK_SIZE];
    virtual void SetUp()
    {
        path1 = (char *)"/path/to";
        path2 = (char *)"/path/of";
        path3 = (char *)"/path/to/message";

        char *pfrag = (char*)"path";
        unsigned int hash = strtohash(pfrag) % HASH_TABLE_SIZE;
        path = new subscription_hash;
        memset(path, 0, sizeof(subscription_hash));
        path->name = strdup(pfrag);
        path->collision = subc_table[hash];
        subc_table[hash] = path;

        pfrag = (char*)"to";
        hash = strtohash(pfrag) % SUB_HASH_SIZE;
        to = new subscription_hash;
        memset(to, 0, sizeof(subscription_hash));
        to->name = strdup(pfrag);
        to->collision = path->next[hash];
        path->next[hash] = to;

        pfrag = (char*)"of";
        hash = strtohash(pfrag) % SUB_HASH_SIZE;
        of = new subscription_hash;
        memset(of, 0, sizeof(subscription_hash));
        of->name = strdup(pfrag);
        of->collision = path->next[hash];
        path->next[hash] = of;

        pfrag = (char*)"message";
        hash = strtohash(pfrag) % SUB_HASH_SIZE;
        message = new subscription_hash;
        memset(message, 0, sizeof(subscription_hash));
        message->name = strdup(pfrag);
        message->collision = to->next[hash];
        to->next[hash] = message;

        // Thread 1 is subbed to all messages
        non_pub_base_mask[1 / 32] |= 1 << (1 % 32);
        // Thread 2 is subbed to all publishes
        pub_only_base_mask[2 / 32] |= 1 << (2 % 32);
        // Thread 3 is subbed to PATH1
        to->non_pub_conn_mask[3 / 32] |= 1 << (3 % 32);
        // Thread 4 is subbed to PATH1 publishes
        to->pub_only_conn_mask[4 / 32] |= 1 << (4 % 32);
        // Thread 5 is subbed to PATH2
        of->non_pub_conn_mask[5 / 32] |= 1 << (5 % 32);
        // Thread 6 is subbed to PATH2 publishes
        of->pub_only_conn_mask[6 / 32] |= 1 << (6 % 32);
        // Thread 7 is subbed to PATH1 and PATH2
        to->non_pub_conn_mask[7 / 32] |= 1 << (7 % 32);
        of->non_pub_conn_mask[7 / 32] |= 1 << (7 % 32);
        // Thread 8 is subbed to PATH1 publishes and PATH2
        to->pub_only_conn_mask[8 / 32] |= 1 << (8 % 32);
        of->non_pub_conn_mask[8 / 32] |= 1 << (8 % 32);
        // Thread 9 is subbed to PATH1 and PATH2 publishes
        to->non_pub_conn_mask[9 / 32] |= 1 << (9 % 32);
        of->pub_only_conn_mask[9 / 32] |= 1 << (9 % 32);
        // Thread 10 is subbed to PATH1 publishes and PATH2 publishes
        to->pub_only_conn_mask[10 / 32] |= 1 << (10 % 32);
        of->pub_only_conn_mask[10 / 32] |= 1 << (10 % 32);
        // Thread 11 is subbed to PATH3
        message->non_pub_conn_mask[11 / 32] |= 1 << (11 % 32);
        // Thread 12 is subbed to PATH3 publishes
        message->pub_only_conn_mask[12 / 32] |= 1 << (12 % 32);
    }

    virtual void TearDown()
    {
        // clear subc_table and its chains
        clear_subscriptions();
    }
};

TEST_F(RemoveSubscriptionTest, RemoveExistingSubscription)
{
    remove_subscription(path1, 3);
    EXPECT_FALSE(to->non_pub_conn_mask[3 / 32] & 1 << (3 % 32));
}

TEST_F(RemoveSubscriptionTest, RemovePubOnlySubscription)
{
    remove_subscription(path1, 4);
    EXPECT_FALSE(to->pub_only_conn_mask[4 / 32] & 1 << (4 % 32));
}

TEST_F(RemoveSubscriptionTest, DoesntRemoveOtherSubscriptionsForThread)
{
    remove_subscription(path2, 7);
    EXPECT_FALSE(of->non_pub_conn_mask[7 / 32] & 1 << (7 % 32));
    EXPECT_TRUE(to->non_pub_conn_mask[7 / 32] & 1 << (7 % 32));
}

TEST_F(RemoveSubscriptionTest, InvalidSubscriptionRemovalAttempt)
{
    remove_subscription(path2, 11);
    EXPECT_FALSE(of->non_pub_conn_mask[11 / 32] & 1 << (1 % 32));
}

TEST_F(RemoveSubscriptionTest, RemoveSubToAll)
{
    remove_subscription("/", 1);
    EXPECT_FALSE(non_pub_base_mask[1 / 32] & 1 << (1 % 32));
}

TEST_F(RemoveSubscriptionTest, RemoveSubToAllPubs)
{
    remove_subscription("/", 2);
    EXPECT_FALSE(pub_only_base_mask[2 / 32] & 1 << (2 % 32));
}

TEST_F(RemoveSubscriptionTest, RemoveNestedSub)
{
    remove_subscription(path3, 11);
    EXPECT_FALSE(message->non_pub_conn_mask[11 / 32] & 1 << (11 % 32));
}
