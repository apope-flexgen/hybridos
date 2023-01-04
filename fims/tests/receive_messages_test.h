//============================================================================
// Name        : Receive_Messages_Test.cpp
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Test of receive_messages function error handling.
//============================================================================

#include "fims_test_utilities.h"

class ReceiveMessagesTest : public testing::Test
{
public:

    virtual void SetUp()
    {
        pthread_mutex_lock(&read_lock_1);
        pthread_mutex_lock(&reply_lock_1);
        pthread_mutex_lock(&close_lock_1);
        connections[1].sock = 1;
        connections[1].thread_id = 1;
        pthread_create(&connections[1].thread, NULL, receive_messages, &connections[1]);
        pthread_mutex_lock(&read_lock_2);
        pthread_mutex_lock(&reply_lock_2);
        pthread_mutex_lock(&close_lock_2);
        connections[2].sock = 2;
        connections[2].thread_id = 2;
        pthread_create(&connections[2].thread, NULL, receive_messages, &connections[2]);
        pthread_mutex_lock(&read_lock_3);
        pthread_mutex_lock(&reply_lock_3);
        pthread_mutex_lock(&close_lock_3);
        connections[3].sock = 3;
        connections[3].thread_id = 3;
        pthread_create(&connections[3].thread, NULL, receive_messages, &connections[3]);
    }
    virtual void TearDown()
    {
        connections[1].sock = 0;
        connections[2].sock = 0;
        connections[3].sock = 0;
        connections[1].thread_id = 0;
        connections[2].thread_id = 0;
        connections[3].thread_id = 0;
        usleep(50000);
        pthread_mutex_unlock(&read_lock_1);
        pthread_mutex_unlock(&reply_lock_1);
        pthread_mutex_unlock(&write_lock_1);
        pthread_mutex_unlock(&close_lock_1);
        strcpy(write_buffer_1, "");
        strcpy(compmsg_1, "");
        pthread_mutex_unlock(&read_lock_2);
        pthread_mutex_unlock(&reply_lock_2);
        pthread_mutex_unlock(&write_lock_2);
        pthread_mutex_unlock(&close_lock_2);
        strcpy(write_buffer_2, "");
        strcpy(compmsg_2, "");
        pthread_mutex_unlock(&read_lock_3);
        pthread_mutex_unlock(&reply_lock_3);
        pthread_mutex_unlock(&write_lock_3);
        pthread_mutex_unlock(&close_lock_3);
        strcpy(write_buffer_3, "");
        strcpy(compmsg_3, "");
        clear_subscriptions();
    }
};

/**********************************************************************************/
// Message Failure

TEST_F(ReceiveMessagesTest, SendEmptyMessage)
{
    close_all_connections();
    EXPECT_EQ(connections[1].sock, 0);
    EXPECT_EQ(connections[2].sock, 0);
    EXPECT_EQ(connections[3].sock, 0);
}

// we don't currently cover whether remove subscription gets called
// on socket close
TEST_F(ReceiveMessagesTest, CorrectNumberOfRemovesOnClose)
{
    close_all_connections();
}

// we don't currently cover failing to parse the message because it
// uses fprintf, not send
TEST_F(ReceiveMessagesTest, FailToParseMessage)
{
    close_all_connections();
}

// this test doesn't actually cover receiving the missing method error
// because it uses fprintf, not send
TEST_F(ReceiveMessagesTest, SendMissingMethod)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "body", "body");
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);

    close_all_connections();
}

/**********************************************************************************/
// Sub Messages

TEST_F(ReceiveMessagesTest, MissingBodyForSubscribe)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "Message missing body for subscribe.");
    FPS_TEST_PRINT("We are reading write buffer\n");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    close_all_connections();
}

TEST_F(ReceiveMessagesTest, NoURIsToSubscribeTo)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    cJSON_AddStringToObject(message, "body", "");
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "No uris to subscribe to.");
    FPS_TEST_PRINT("We are reading write buffer\n");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    close_all_connections();
}

TEST_F(ReceiveMessagesTest, TooManyURIsToSubscribeTo)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    cJSON* body = cJSON_CreateArray();
    for (int i = 0; i < MAX_CONNECTS + 1; i++)
    {
        cJSON* tmp = cJSON_CreateObject();
        cJSON_AddStringToObject(tmp, "uri", "/path/to");
        cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
        cJSON_AddItemToArray(body, tmp);
    }
    cJSON_AddItemToObject(message, "body", body);
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "Too many Uri's to subscribe.");
    FPS_TEST_PRINT("We are reading write buffer\n");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    close_all_connections();
}

TEST_F(ReceiveMessagesTest, InvalidURI)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    cJSON* body = cJSON_CreateArray();
    cJSON* tmp = cJSON_CreateObject();
    cJSON_AddStringToObject(tmp, "uri", "/pa$th/t#o/me$$4g3");
    cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
    cJSON_AddItemToArray(body, tmp);
    cJSON_AddItemToObject(message, "body", body);
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "Invalid Uri.");
    FPS_TEST_PRINT("We are reading write buffer\n");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    close_all_connections();
}

// Test that insert_subscription gets called the number of times we expect
TEST_F(ReceiveMessagesTest, CorrectNumberOfInserts)
{
    close_all_connections();
}

// Insert subscriptions and then insert subscriptions again, ensuring that
// both insert_subscription and remove_subscription get called the number
// of times we expect
TEST_F(ReceiveMessagesTest, CorrectNumberOfRemoves)
{
    close_all_connections();
}

/**********************************************************************************/
// Non-Sub messages

// This test won't work yet because the server replies with a fprintf instead of a
// send
TEST_F(ReceiveMessagesTest, PubWithMissingURI)
{
    close_all_connections();
}

TEST_F(ReceiveMessagesTest, CorrectParametersForGetConnections)
{
    close_all_connections();
}

/**********************************************************************************/
// Integration Tests ***These will break when anything else breaks***

// Tests from here forward rely on the functionality of insertion of
// subscriptions
// They are integration tests and will fail when any other functionality
// in FIMS_Server fails
TEST_F(ReceiveMessagesTest, SubscriptionAndResponse)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    cJSON* body = cJSON_CreateArray();
    cJSON* tmp = cJSON_CreateObject();
    cJSON_AddStringToObject(tmp, "uri", "/path/to");
    cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
    cJSON_AddItemToArray(body, tmp);
    cJSON_AddItemToObject(message, "body", body);
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    FPS_TEST_PRINT("Waiting on reply_lock_1\n");
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "SUCCESS");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "pub");
    cJSON_AddStringToObject(message, "uri", "/path/to");
    msg = cJSON_PrintUnformatted(message);
    send(2, (const void *)msg, strlen(msg), 0);

    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, msg);
    FPS_TEST_PRINT("We are reading write buffer %s\n", write_buffer_1);
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    close_all_connections();
}

TEST_F(ReceiveMessagesTest, MessageDropEvent)
{
    // DB listener listening to events
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    cJSON* body = cJSON_CreateArray();
    cJSON* tmp = cJSON_CreateObject();
    cJSON_AddStringToObject(tmp, "uri", "/events");
    cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
    cJSON_AddItemToArray(body, tmp);
    cJSON_AddItemToObject(message, "body", body);
    char *event_db_msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)event_db_msg, strlen(event_db_msg), 0);
    FPS_TEST_PRINT("Locking reply_lock_1\n");
    pthread_mutex_lock(&reply_lock_1);
    FPS_TEST_PRINT("Locked reply_lock_1\n");
    strcpy(compmsg_1, "SUCCESS");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);
    // thread 1 READ(L), REPLY(L), WRITE(U)

    // another thread listening to a uri
    message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    body = cJSON_CreateArray();
    tmp = cJSON_CreateObject();
    cJSON_AddStringToObject(tmp, "uri", "/path/to");
    cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
    cJSON_AddItemToArray(body, tmp);
    cJSON_AddItemToObject(message, "body", body);
    char *msg = cJSON_PrintUnformatted(message);
    send(2, (const void *)msg, strlen(msg), 0);
    FPS_TEST_PRINT("Locking reply_lock_2\n");
    pthread_mutex_lock(&reply_lock_2);
    FPS_TEST_PRINT("Locked reply_lock_2\n");
    strcpy(compmsg_2, "SUCCESS");
    EXPECT_EQ(strncmp(write_buffer_2, compmsg_2, strlen(compmsg_2)), 0);
    clear_write_buffer(2);
    // thread 2 READ(L), REPLY(L), WRITE(U)
    FPS_TEST_PRINT("LOCK STATES ARE GOOD HERE\n");

    #define NUM_33_CHAR_MSG_TO_OVERFLOW_BUFFER 1986
    // In theory, more messages than those necessary to overflow the
    // buffer could be sent. In this case, the mocking implementation of
    // read and send involve locks, and these locks prevent more messages
    // from being sent while receive_messages cannot accept new ones
    // a third thread sending messages to the uri that are never read
    for (int i = 0; i < 1986; i++)
    {
        // Need to be in state READ(L), REPLY(L), WRITE(U) for thread 2 and 3
        message = cJSON_CreateObject();
        cJSON_AddStringToObject(message, "method", "pub");
        cJSON_AddStringToObject(message, "uri", "/path/to");
        msg = cJSON_PrintUnformatted(message);
        send(3, (const void *)msg, strlen(msg), 0);
        FPS_TEST_PRINT("Locking reply_lock_2\n");
        pthread_mutex_lock(&reply_lock_2);
        FPS_TEST_PRINT("Locked reply_lock_2\n");
        pthread_mutex_unlock(&write_lock_2);
    }

    // add time checking (make sure we only get 1 message every 2 min)
    FPS_TEST_PRINT("Locking reply_lock_1\n");
    pthread_mutex_lock(&reply_lock_1);
    FPS_TEST_PRINT("Locked reply_lock_1\n");
    FPS_TEST_PRINT("We are reading write buffer %s\n", write_buffer_1);
    EXPECT_NE(strncmp(write_buffer_1, "", strlen(write_buffer_1)), 0);
    clear_write_buffer(1);
    clear_write_buffer(2);

    close_all_connections();
}

TEST_F(ReceiveMessagesTest, OverwriteSubscriptionsTest)
{
    cJSON *message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    cJSON* body = cJSON_CreateArray();
    cJSON* tmp = cJSON_CreateObject();
    cJSON_AddStringToObject(tmp, "uri", "/path/to");
    cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
    cJSON_AddItemToArray(body, tmp);
    cJSON_AddItemToObject(message, "body", body);
    char *msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    FPS_TEST_PRINT("Waiting on reply_lock_1\n");
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "SUCCESS");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "sub");
    body = cJSON_CreateArray();
    tmp = cJSON_CreateObject();
    cJSON_AddStringToObject(tmp, "uri", "/path/of");
    cJSON_AddItemToObject(tmp, "pub_only", cJSON_CreateFalse());
    cJSON_AddItemToArray(body, tmp);
    cJSON_AddItemToObject(message, "body", body);
    msg = cJSON_PrintUnformatted(message);
    send(1, (const void *)msg, strlen(msg), 0);
    FPS_TEST_PRINT("Waiting on reply_lock_1\n");
    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, "SUCCESS");
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);

    message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "pub");
    cJSON_AddStringToObject(message, "uri", "/path/to");
    msg = cJSON_PrintUnformatted(message);
    send(2, (const void *)msg, strlen(msg), 0);

    message = cJSON_CreateObject();
    cJSON_AddStringToObject(message, "method", "pub");
    cJSON_AddStringToObject(message, "uri", "/path/of");
    msg = cJSON_PrintUnformatted(message);
    send(2, (const void *)msg, strlen(msg), 0);

    pthread_mutex_lock(&reply_lock_1);
    strcpy(compmsg_1, msg);
    FPS_TEST_PRINT("We are reading write buffer %s\n", write_buffer_1);
    EXPECT_EQ(strncmp(write_buffer_1, compmsg_1, strlen(compmsg_1)), 0);
    clear_write_buffer(1);



    close_all_connections();
}
