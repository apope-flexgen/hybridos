//============================================================================
// Name        : FIMS_Test_Utilities.h
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Common utilitiy functions for tests of FIMS
//============================================================================

#ifndef FIMS_TEST_UTILITIES_H_
#define FIMS_TEST_UTILITIES_H_

#include "../include/fims_server.h"
#include "../include/libfims.h"
#include <errno.h>

#define RECV_MSG_SOCKET_1       1
#define RECV_MSG_SOCKET_2       2
#define RECV_MSG_SOCKET_3       3
#define LIB_FIMS_SOCKET         4

// READ locks are locked when a receive messages function is reading a buffer
// WRITE locks are locked when anyone is writing to a buffer, and unlocked when
// the buffer is empty
// REPLY locks are unlocked when a receive messages function writes to a buffer
// and are locked when we have the permission of a client to reply

/*
 * Lock states:
 * READ(L), REPLY(L), WRITE(U) - Initial state; means we can send
 * When we call send to send the client a message, the result will be
 * READ(U), REPLY(L), WRITE(L) - Message has been sent
 * Client reads message by calling read, the resulting state is
 * READ(L), REPLY(L), WRITE(U) - duplicate state!!! We have to know
 * whether we expect a reply from the client or not in order to use
 * this. If we don't expect a reply from the client, then we're free
 * to send another message to that client now. If we do, we wait on
 * REPLY lock, which will allow the client to call send, which will
 * unlock REPLY for us.
 * If we acquire REPLY, we don't call read to read the buffer, we read it
 * directly. Then when we are done reading it, we call clear_write_buffer,
 * which clears the buffer and unlocks write, giving us a state of
 * READ(L), REPLY(L), WRITE(U) again.
 */

pthread_mutex_t read_lock_1;
pthread_mutex_t write_lock_1;
pthread_mutex_t reply_lock_1;
pthread_mutex_t close_lock_1;
char write_buffer_1[MAX_MESSAGE_SIZE];
char compmsg_1[MAX_MESSAGE_SIZE];
pthread_mutex_t read_lock_2;
pthread_mutex_t write_lock_2;
pthread_mutex_t reply_lock_2;
pthread_mutex_t close_lock_2;
char write_buffer_2[MAX_MESSAGE_SIZE];
char compmsg_2[MAX_MESSAGE_SIZE];
pthread_mutex_t read_lock_3;
pthread_mutex_t write_lock_3;
pthread_mutex_t reply_lock_3;
pthread_mutex_t close_lock_3;
char write_buffer_3[MAX_MESSAGE_SIZE];
char compmsg_3[MAX_MESSAGE_SIZE];

char send_buffer[MAX_MESSAGE_SIZE];
char send_comp_buffer[MAX_MESSAGE_SIZE];
char recv_buffer[MAX_MESSAGE_SIZE];
bool closed;

ssize_t read(int sock, void *buffer, size_t size)
{
    int buffsize;
    switch (sock)
    {
    case RECV_MSG_SOCKET_1 :
        FPS_TEST_PRINT("Locking read_lock_1\n");
        pthread_mutex_lock(&read_lock_1);
        FPS_TEST_PRINT("Locked read_lock_1\n");
        strcpy((char *)buffer, write_buffer_1);
        buffsize = strlen(write_buffer_1);
        strcpy(write_buffer_1, "");
        FPS_TEST_PRINT("Unlocking write_lock_1: Read\n");
        pthread_mutex_unlock(&write_lock_1);
        break;
    case RECV_MSG_SOCKET_2 :
        FPS_TEST_PRINT("Locking read_lock_2\n");
        pthread_mutex_lock(&read_lock_2);
        FPS_TEST_PRINT("Locked read_lock_2\n");
        strcpy((char *)buffer, write_buffer_2);
        buffsize = strlen(write_buffer_2);
        strcpy(write_buffer_2, "");
        FPS_TEST_PRINT("Unlocking write_lock_2: Read\n");
        pthread_mutex_unlock(&write_lock_2);
        break;
    case RECV_MSG_SOCKET_3 :
        FPS_TEST_PRINT("Locking read_lock_3\n");
        pthread_mutex_lock(&read_lock_3);
        FPS_TEST_PRINT("Locked read_lock_3\n");
        strcpy((char *)buffer, write_buffer_3);
        buffsize = strlen(write_buffer_3);
        strcpy(write_buffer_3, "");
        FPS_TEST_PRINT("Unlocking write_lock_3: Read\n");
        pthread_mutex_unlock(&write_lock_3);
        break;
    }
    return buffsize;
}

ssize_t recv(int sock, void *buf, size_t len, int flags)
{
    switch (sock)
    {
    case LIB_FIMS_SOCKET :
        FPS_TEST_PRINT("FIMS Subscribe is receiving\n");
        if (strcmp(recv_buffer, "timeout") == 0)
            return 0;
        strcpy((char*)buf, recv_buffer);
        break;
    }
    return strlen((const char *)buf);
}

ssize_t send(int sock, const void *buf, size_t len, int flags)
{
    int msg_size = strlen((const char *)buf);
    if (flags == MSG_DONTWAIT)
        FPS_TEST_PRINT("Client ");
    else
        FPS_TEST_PRINT("We are ");
    switch (sock)
    {
    case RECV_MSG_SOCKET_1 :
        if (strlen(write_buffer_1) + msg_size > MAX_MESSAGE_SIZE)
        {
            FPS_TEST_PRINT("BUFFER OVERFLOW\n");
            errno = EWOULDBLOCK;
        }
        FPS_TEST_PRINT("locking write_lock_1\n");
        pthread_mutex_lock(&write_lock_1);
        FPS_TEST_PRINT("Locked write_lock_1\n");
        strcat(write_buffer_1, (char *)buf);
        if (flags == MSG_DONTWAIT)
        {
            FPS_TEST_PRINT("Unlocking reply_lock_1\n");
            pthread_mutex_unlock(&reply_lock_1);
        }
        else
        {
            FPS_TEST_PRINT("Unlocking read_lock_1\n");
            pthread_mutex_unlock(&read_lock_1);
        }
        break;
    case RECV_MSG_SOCKET_2 :
        if (strlen(write_buffer_2) + msg_size > MAX_MESSAGE_SIZE)
        {
            FPS_TEST_PRINT("BUFFER OVERFLOW\n");
            errno = EWOULDBLOCK;
        }
        FPS_TEST_PRINT("locking write_lock_2\n");
        pthread_mutex_lock(&write_lock_2);
        FPS_TEST_PRINT("Locked write_lock_2\n");
        strcat(write_buffer_2, (char *)buf);
        if (flags == MSG_DONTWAIT)
        {
            FPS_TEST_PRINT("Unlocking reply_lock_2\n");
            pthread_mutex_unlock(&reply_lock_2);
        }
        else
        {
            FPS_TEST_PRINT("Unlocking read_lock_2\n");
            pthread_mutex_unlock(&read_lock_2);
        }
        break;
    case RECV_MSG_SOCKET_3 :
        if (strlen(write_buffer_3) + msg_size > MAX_MESSAGE_SIZE)
        {
            FPS_TEST_PRINT("BUFFER OVERFLOW\n");
            errno = EWOULDBLOCK;
        }
        FPS_TEST_PRINT("locking write_lock_3\n");
        pthread_mutex_lock(&write_lock_3);
        FPS_TEST_PRINT("Locked write_lock_3\n");
        strcat(write_buffer_3, (char *)buf);
        if (flags == MSG_DONTWAIT)
        {
            FPS_TEST_PRINT("Unlocking reply_lock_3\n");
            pthread_mutex_unlock(&reply_lock_3);
        }
        else
        {
            FPS_TEST_PRINT("Unlocking read_lock_3\n");
            pthread_mutex_unlock(&read_lock_3);
        }
        break;
    case LIB_FIMS_SOCKET :
        FPS_TEST_PRINT("FIMS Subscribe is sending\n");
        strcpy(send_buffer, (char *)buf);
        break;
    }
    if (errno == EWOULDBLOCK)
        return -1;
    else
        return strlen((const char *)buf);
}

int setsockopt(int sock, int level, int option_name, void *option_value, socklen_t option_len)
{
    return 0;
}

// we use clear_write_buffer when we've read the write_buffer
void clear_write_buffer(int sock)
{
    switch (sock)
    {
    case RECV_MSG_SOCKET_1 :
        strcpy(write_buffer_1, "");
        FPS_TEST_PRINT("Unlocking write_lock_1: Clear write buffer\n");
        pthread_mutex_unlock(&write_lock_1);
        break;
    case RECV_MSG_SOCKET_2 :
        strcpy(write_buffer_2, "");
        FPS_TEST_PRINT("Unlocking write_lock_2: Clear write buffer\n");
        pthread_mutex_unlock(&write_lock_2);
        break;
    case RECV_MSG_SOCKET_3 :
        strcpy(write_buffer_3, "");
        FPS_TEST_PRINT("Unlocking write_lock_3: Clear write buffer\n");
        pthread_mutex_unlock(&write_lock_3);
        break;
    }
}

int close(int sock)
{
    switch (sock)
    {
    case RECV_MSG_SOCKET_1 :
        FPS_TEST_PRINT("Client 1 is closing the connection\n");
        pthread_mutex_unlock(&close_lock_1);
        break;
    case RECV_MSG_SOCKET_2 :
        FPS_TEST_PRINT("Client 2 is closing the connection\n");
        pthread_mutex_unlock(&close_lock_2);
        break;
    case RECV_MSG_SOCKET_3 :
        FPS_TEST_PRINT("Client 3 is closing the connection\n");
        pthread_mutex_unlock(&close_lock_3);
        break;
    case LIB_FIMS_SOCKET :
        FPS_TEST_PRINT("FIMS Subscribe is closing the connection\n");
        closed = true;
        break;
    }
    return 0;
}

void close_all_connections()
{
    send(RECV_MSG_SOCKET_1, (const void *)"", 0, 0);
    send(RECV_MSG_SOCKET_2, (const void *)"", 0, 0);
    send(RECV_MSG_SOCKET_3, (const void *)"", 0, 0);
    // wait until you have the lock that says you can read the close variable
    FPS_TEST_PRINT("We are waiting on receive_messages 1 to close\n");
    pthread_mutex_lock(&close_lock_1);
    FPS_TEST_PRINT("We are waiting on receive_messages 2 to close\n");
    pthread_mutex_lock(&close_lock_2);
    FPS_TEST_PRINT("We are waiting on receive_messages 3 to close\n");
    pthread_mutex_lock(&close_lock_3);
    FPS_TEST_PRINT("All receive_message threads closed\n");
}

void clear_subscriptions()
{
    for (int i = 0; i < SUB_MASK_SIZE; i++)
    {
        pub_only_base_mask[i] = 0;
        non_pub_base_mask[i] = 0;
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        subc_table[i] = 0;
    }
}

#endif /* FIMS_TEST_UTILITIES_H_ */
