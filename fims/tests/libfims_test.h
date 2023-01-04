//============================================================================
// Name        : libFIMS_Test.h
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : FIMS library functions and utilities
//============================================================================

#ifndef LIBFIMS_TEST_H_
#define LIBFIMS_TEST_H_

#include "fims_test_utilities.h"

class MockFims : public fims
{
public:
    MockFims() { fims(); }
    bool Connect(char *process_name = NULL) { return (connection = 4); }
    void Close() { connection = 0; }
};

// There is little value in testing Connect, Connected, get_socket,
// and Close, because they are wrappers for an external library and
// the effort of mocking would not be worth the return.
class LibFIMSTest : public testing::Test {
public:
    MockFims *p_fims;
    virtual void SetUp()
    {
        p_fims = new MockFims();
        p_fims->Connect();
        closed = false;
    }

    virtual void TearDown()
    {
        p_fims->Close();
        delete p_fims;
        strcpy(send_buffer, "");
        strcpy(recv_buffer, "");
        closed = true;
    }
};

TEST_F(LibFIMSTest, SubscribeNotConnected)
{
    p_fims->Close();
    int count = 1;
    char **uri_array = new char*[count];
    bool *pub_array = new bool[count];
    uri_array[0] = (char*)"/path";
    pub_array[0] = false;
    EXPECT_FALSE(p_fims->Subscribe((const char**)uri_array, count, pub_array));
}

TEST_F(LibFIMSTest, SubscribeEmptyURIArray)
{
    int count = 0;
    char **uri_array = new char*[count];
    bool *pub_array = new bool[count];
    EXPECT_FALSE(p_fims->Subscribe((const char**)uri_array, count, pub_array));
    EXPECT_TRUE(strcmp(send_buffer, ""));
}

TEST_F(LibFIMSTest, SubscribeNoServerResponse)
{
    int count = 1;
    char **uri_array = new char*[count];
    bool *pub_array = new bool[count];
    uri_array[0] = (char*)"/path";
    pub_array[0] = false;
    EXPECT_FALSE(p_fims->Subscribe((const char**)uri_array, count, pub_array));
}

TEST_F(LibFIMSTest, SubscribeNoPubArray)
{
    int count = 1;
    char **uri_array = new char*[count];
    uri_array[0] = (char*)"/path";
    strcpy(recv_buffer, "SUCCESS");
    EXPECT_TRUE(p_fims->Subscribe((const char**)uri_array, count));
    strcpy(send_comp_buffer, "{\"method\":\"sub\",\"body\":[{\"uri\":\"/path\",\"pub_only\":false}]}");
    EXPECT_EQ(strncmp(send_buffer, send_comp_buffer, strlen(send_comp_buffer)), 0);
}

TEST_F(LibFIMSTest, SubscribeWithPubArray)
{
    int count = 1;
    char **uri_array = new char*[count];
    bool *pub_array = new bool[count];
    uri_array[0] = (char*)"/path";
    pub_array[0] = true;
    strcpy(recv_buffer, "SUCCESS");
    EXPECT_TRUE(p_fims->Subscribe((const char**)uri_array, count, pub_array));
    strcpy(send_comp_buffer, "{\"method\":\"sub\",\"body\":[{\"uri\":\"/path\",\"pub_only\":true}]}");
    EXPECT_EQ(strncmp(send_buffer, send_comp_buffer, strlen(send_comp_buffer)), 0);
}

TEST_F(LibFIMSTest, SubscribeToMultipleURIs)
{
    int count = 2;
    char **uri_array = new char*[count];
    bool *pub_array = new bool[count];
    uri_array[0] = (char*)"/path";
    pub_array[0] = true;
    uri_array[1] = (char*)"/other";
    pub_array[1] = false;
    strcpy(recv_buffer, "SUCCESS");
    EXPECT_TRUE(p_fims->Subscribe((const char**)uri_array, count, pub_array));
    strcpy(send_comp_buffer, "{\"method\":\"sub\",\"body\":[{\"uri\":\"/path\",\"pub_only\":true},{\"uri\":\"/other\",\"pub_only\":false}]}");
    EXPECT_EQ(strncmp(send_buffer, send_comp_buffer, strlen(send_comp_buffer)), 0);
}

TEST_F(LibFIMSTest, SendNotConnected)
{
    p_fims->Close();
    char *method = (char*)"pub";
    char *uri = (char*)"/path";
    EXPECT_FALSE(p_fims->Send(method, uri, nullptr, nullptr));
}

TEST_F(LibFIMSTest, SendMethodNull)
{
    char *uri = (char*)"/path";
    EXPECT_FALSE(p_fims->Send(nullptr, uri, nullptr, nullptr));
}

TEST_F(LibFIMSTest, SendURINull)
{
    char *method = (char*)"pub";
    EXPECT_FALSE(p_fims->Send(method, nullptr, nullptr, nullptr));
}

TEST_F(LibFIMSTest, SendNullReplyToNullBody)
{
    char *method = (char*)"pub";
    char *uri = (char*)"/path";
    EXPECT_TRUE(p_fims->Send(method, uri, nullptr, nullptr));
    strcpy(send_comp_buffer, "{\"method\":\"pub\",\"uri\":\"/path\"}");
    EXPECT_EQ(strncmp(send_buffer, send_comp_buffer, strlen(send_comp_buffer)), 0);
}

TEST_F(LibFIMSTest, SendWithBody)
{
    char *method = (char*)"pub";
    char *uri = (char*)"/path";
    char *body = (char*)"hello world";
    EXPECT_TRUE(p_fims->Send(method, uri, nullptr, body));
    strcpy(send_comp_buffer, "{\"method\":\"pub\",\"uri\":\"/path\",\"body\":\"hello world\"}");
    EXPECT_EQ(strncmp(send_buffer, send_comp_buffer, strlen(send_comp_buffer)), 0);
}

TEST_F(LibFIMSTest, SendWithReplyTo)
{
    char *method = (char*)"pub";
    char *uri = (char*)"/path";
    char *replyto = (char*)"/reply";
    EXPECT_TRUE(p_fims->Send(method, uri, replyto, nullptr));
    strcpy(send_comp_buffer, "{\"method\":\"pub\",\"uri\":\"/path\",\"replyto\":\"/reply\"}");
    EXPECT_EQ(strncmp(send_buffer, send_comp_buffer, strlen(send_comp_buffer)), 0);
}

TEST_F(LibFIMSTest, ReceiveNotConnected)
{
    p_fims->Close();
    EXPECT_EQ(p_fims->Receive(), nullptr);
}

TEST_F(LibFIMSTest, ReceiveEmpty)
{
    EXPECT_EQ(p_fims->Receive(), nullptr);
}

TEST_F(LibFIMSTest, ReceiveFailedToParse)
{
    strcpy(recv_buffer, "T7issI5G!bberi}h");
    EXPECT_EQ(p_fims->Receive(), nullptr);
}

TEST_F(LibFIMSTest, ReceiveSuccess)
{
    strcpy(recv_buffer, "{\"method\":\"pub\",\"uri\":\"/path\",\"replyto\":\"/reply\"}");
    fims_message *msg = p_fims->Receive();
    ASSERT_NE(msg, nullptr);
}

// Timeout tests are difficult because receive mock is non-blocking
/*
TEST_F(LibFIMSTest, ReceiveTimeoutInTime)
{
    strcpy(recv_buffer, "{\"method\":\"pub\",\"uri\":\"/path\",\"replyto\":\"/reply\"}");
    fims_message *msg = p_fims->Receive_Timeout(100);
    ASSERT_NE(msg, nullptr);
}

TEST_F(LibFIMSTest, ReceiveTimeoutOutOfTime)
{
    strcpy(recv_buffer, "timeout");
    EXPECT_EQ(p_fims->Receive_Timeout(100), nullptr);
}*/

#endif
