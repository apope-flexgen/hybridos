//============================================================================
// Name        : FIMS_Server.cpp
// Author      : John Calcagni
// Version     :
// Copyright   : Your copyright notice
// Description : FlexGen Internal Messaging Service Server - This program
//               is meant to listen for incoming connections. Then each of
//               those connections will have a thread started to listen to
//               the data they send and forward to all the other connections
//               that have subscribed to it. 
//============================================================================

/* OS Includes */
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
/* C Standard Library Dependencies */
#include <cerrno>
#include <cstdio>
#include <cstdlib>
/* C++ Standard Library Dependencies */
#include <stack>
#include <iostream>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "fps_utils.h" // fims_addon requires local search path
#include "libfims_internal.hpp"
#include <fims_server.h>
#include "libaes.h"
#include "gen_time.h"
#include "defer.hpp"

#ifdef FPS_TEST_MODE
#include "fims_test.h"
#endif

volatile bool running = true;
volatile int sock_fd = 0;

pthread_mutex_t subscription_mutex;
pthread_cond_t  subscription_lock_cv;
pthread_cond_t  subscription_reading_cv;

subscription_lock sub_lock;

subscription_hash *subc_table[HASH_TABLE_SIZE];

client_info connections[MAX_CONNECTS];

// base mask will hold all connections that are subscribed to all
uint32_t pub_only_base_mask[SUB_MASK_SIZE];
uint32_t non_pub_base_mask[SUB_MASK_SIZE];

extern uint8_t* g_aesKey;


/******************************************************************************/
// catch any signals for exiting to clean up nicely
void signal_handler (int sig)
{
    running = false;
    close(sock_fd);
    sock_fd = 0;
    // loop though the sockets for each thread and close them
    for(int i = 0; i < MAX_CONNECTS; i++)
    {
        if (connections[i].sock != 0)
        {
            int sock = connections[i].sock;
            connections[i].sock = 0;
            // when the socket is shutdown, the connection thread is responsible for closing the socket
            shutdown(sock, SHUT_RDWR);
        }
    }
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

/******************************************************************************/


//read only lock of subscriptions allows multiple threads to read simultaneously
void lock_subscriptions_ro()
{
    pthread_mutex_lock(&subscription_mutex);

    while(sub_lock.lock == true)
        //conditional wait for lock to free
        pthread_cond_wait(&subscription_lock_cv, &subscription_mutex);

    sub_lock.reading++;
    pthread_mutex_unlock(&subscription_mutex);
}

//free read only lock of subscriptions
void unlock_subscriptions_ro()
{
    pthread_mutex_lock(&subscription_mutex);
    sub_lock.reading--;
    pthread_cond_signal(&subscription_reading_cv);
    pthread_mutex_unlock(&subscription_mutex);
}

// lock subscriptions for editing
void lock_subscriptions()
{
    pthread_mutex_lock(&subscription_mutex);

    while(sub_lock.lock == true)
        //conditional wait for lock to free
        pthread_cond_wait(&subscription_lock_cv, &subscription_mutex);

    sub_lock.lock = true;

    while(sub_lock.reading > 0)
        // conditional wait for read == 0
        pthread_cond_wait(&subscription_reading_cv, &subscription_mutex);

    pthread_mutex_unlock(&subscription_mutex);
}

// clear lock on subscriptions for editing
void unlock_subscriptions()
{
    pthread_mutex_lock(&subscription_mutex);
    sub_lock.lock = false;
    pthread_cond_signal(&subscription_lock_cv);
    pthread_mutex_unlock(&subscription_mutex);
}


// simple hash function to return a number from the input string
// assumes the string in null terminated
unsigned int strtohash(const char* in_str)
{
    unsigned int total = 0;
    for(int i = 0; in_str[i] != 0; i++)
    {
        char c = in_str[i];
        total += (i+1) * (int)((c >= 'a' && c <= 'z') ? (c - 'a' + 40) :
                               (c >= 'A' && c <= 'Z') ? (c - 'A' + 14) :
                               (c >= '-' && c <= '9') ? (c - '-' + 1 ) : 0);
    }
    return total;
}

// This checks to see if the node has a subscription or is a root to another uri
bool is_active_node(subscription_hash* cur)
{
    //combine all masks to see if any connections are subscribed to this uri
    uint32_t total_mask = cur->non_pub_conn_mask[0] | cur->pub_only_conn_mask[0];
    for (int i = 1; i < SUB_MASK_SIZE; i++)
    {
        total_mask |= cur->non_pub_conn_mask[i] | cur->pub_only_conn_mask[i];
    }

    // If another sockets is using this uri then its active
    if (total_mask != 0)
        return true;

    // check if this is a root for another uri
    for(int i = 0; i < SUB_HASH_SIZE; i++)
    {
        if (cur->next[i] != NULL)
            return true;
    }
    return false;
}

bool valid_uri(const fims::str_view uri)
{
    if (!uri.data)
        return false;
    if (uri.data[0] != '/')
        return false;

    for (uint8_t i = 0; i < uri.size; ++i)
    {
        if (uri.data[i] < '-' || (uri.data[i] > '9' && uri.data[i] < 'A') || (uri.data[i] > 'Z' && uri.data[i] < 'a' && uri.data[i] != '_') ||uri.data[i] > 'z')
        {
            FPS_ERROR_PRINT("Bad Uri  [%s] @ %d [%c]\n", uri.data, i, uri.data[i]);
            return false;
        }
    }
    //FPS_ERROR_PRINT("Good Uri  [%s] \n", uri.data);
    return true;
}

/******************************************************************************/

// This function will add all the frags needed in the table to represent the uri
// At the leaf the mask will be set to represent a subscription
// This function assumes that the uri passed in is valid
// Inserts subscription to hash only; does not change client info
void insert_subscription(const char* uri, int thread_id, bool pub_only)
{
    FPS_DEBUG_PRINT("%s for  [%s]\n", __FUNCTION__, uri);
    // Special case to deal with subscribe to all
    if (strcmp(uri, "/") == 0)
    {
        if (pub_only)
            pub_only_base_mask[thread_id / 32] |= 1 << (thread_id % 32);
        else
            non_pub_base_mask[thread_id / 32] |= 1 << (thread_id % 32);
        return;
    }

    char *uri_tok, *pfrag, *str_tok;
    unsigned int hash;
    str_tok = NULL;
    uri_tok = strdup(uri);
    //Add first frag into uri hash table
    pfrag = strtok_r(uri_tok, "/", &str_tok);
    hash = strtohash(pfrag) % HASH_TABLE_SIZE;
    FPS_DEBUG_PRINT("Hash for %s = %d\n", pfrag, hash);
    subscription_hash* cur = subc_table[hash];
    if (cur == NULL || strcmp(pfrag, cur->name) < 0)
    {
        // insert into head of table
        FPS_DEBUG_PRINT("Adding %s to head of hash table in front of %s.\n", pfrag, ((cur == NULL) ? "null" : subc_table[hash]->name));
        cur = new subscription_hash;
        memset(cur, 0, sizeof(subscription_hash));
        cur->name = strdup(pfrag);
        //move current head to first collision (still works if current head is NULL)
        cur->collision = subc_table[hash];
        subc_table[hash] = cur;
    }
    else if (strcmp(pfrag, cur->name) != 0)
    {
        FPS_DEBUG_PRINT("Collision occurred inserting %s into hash table.\n", pfrag);
        // resolve collision or if match found
        while(cur->collision != NULL && strcmp(pfrag, cur->collision->name) > 0)
        {
            FPS_DEBUG_PRINT("Resolving collision, comparing existing %s with new segment %s.\n", cur->name, pfrag);
            cur = cur->collision;
        }
        if (cur->collision == NULL || strcmp(pfrag, cur->collision->name) != 0)
        {
            FPS_DEBUG_PRINT("Adding %s to into collision list after %s and before %s.\n", pfrag, cur->name, ((cur->collision == NULL)?"null":cur->collision->name));
            subscription_hash* insert_temp = new subscription_hash;
            memset(insert_temp, 0, sizeof(subscription_hash));
            insert_temp->name = strdup(pfrag);
            insert_temp->collision = cur->collision;
            cur->collision = insert_temp;
        }
        else
        {
            FPS_DEBUG_PRINT("Found %s in collision list.\n", pfrag);
        }
        cur = cur->collision;
    }
    else
    {
        FPS_DEBUG_PRINT("Found %s at head of hash table.\n", pfrag);
    }
    pfrag = strtok_r(NULL, "/", &str_tok);

    // add remaining frags to hash
    while(pfrag != NULL)
    {
        hash = strtohash(pfrag) % SUB_HASH_SIZE;
        FPS_DEBUG_PRINT("Hash for %s = %d\n", pfrag, hash);
        subscription_hash* sub_cur = cur->next[hash];
        if (sub_cur == NULL || strcmp(pfrag, sub_cur->name) < 0)
        {
            // insert into head of subtable
            FPS_DEBUG_PRINT("Adding %s into head of %s hash subtable in front of %s.\n", pfrag, cur->name, ((sub_cur == NULL) ? "null" : cur->next[hash]->name) );
            sub_cur = new subscription_hash;
            memset(sub_cur, 0, sizeof(subscription_hash));
            sub_cur->name = strdup(pfrag);
            sub_cur->collision = cur->next[hash];
            cur->next[hash] = sub_cur;
        }
        else if (strcmp(pfrag, sub_cur->name) != 0)
        {
            FPS_DEBUG_PRINT("Collision occurred adding %s into %s hash subtable.\n", pfrag, cur->name);
            // resolve collision or if match found
            while(sub_cur->collision != NULL && strcmp(pfrag, sub_cur->collision->name) > 0)
            {
                FPS_DEBUG_PRINT("Resolving collision, comparing existing %s with new segment %s.\n", sub_cur->name, pfrag);
                sub_cur = sub_cur->collision;
            }
            if (sub_cur->collision == NULL || strcmp(pfrag, sub_cur->collision->name) != 0)
            {
                FPS_DEBUG_PRINT("Adding %s into collision list for %s after %s before %s.\n", pfrag, cur->name, sub_cur->name, ((sub_cur->collision == NULL)?"null":sub_cur->collision->name));
                subscription_hash* insert_temp = new subscription_hash;
                memset(insert_temp, 0, sizeof(subscription_hash));
                insert_temp->name = strdup(pfrag);
                insert_temp->collision = sub_cur->collision;
                sub_cur->collision = insert_temp;
            }
            else
            {
                FPS_DEBUG_PRINT("Found %s in collision list of %s.\n", pfrag, cur->name);
            }
            sub_cur = sub_cur->collision;
        }
        else
        {
            FPS_DEBUG_PRINT("Found %s at head of %s hash subtable.\n", pfrag, cur->name);
        }
        cur = sub_cur;
        pfrag = strtok_r(NULL, "/", &str_tok);
    }
    // add subscription to leaf of uri
    if (pub_only)
        cur->pub_only_conn_mask[thread_id/32] |= 1 << (thread_id % 32);
    else
        cur->non_pub_conn_mask[thread_id/32] |= 1 << (thread_id % 32);
    free(uri_tok);
}

// Removes subscription from hash only; does not touch client info.
void remove_subscription(const char* uri, int thread_id)
{
    // special case to deal with subscribe to all
    if (strcmp(uri, "/") == 0)
    {
        //remove from base mask
        pub_only_base_mask[thread_id/32] &= ~(1 << (thread_id % 32));
        non_pub_base_mask[thread_id/32] &= ~(1 << (thread_id % 32));
        return;
    }

    std::stack<subscription_hash*> voided_entries;
    char *uri_tok, *pfrag, *str_tok;
    unsigned int hash;
    uri_tok = strdup(uri);
    str_tok = NULL;
    // Build stack of pointer to frag entries for uri
    pfrag = strtok_r(uri_tok, "/", &str_tok);
    hash = strtohash(pfrag) % HASH_TABLE_SIZE;
    subscription_hash* cur = subc_table[hash];
    // traverse if collision
    while(cur != NULL && strcmp(pfrag, cur->name) > 0)
        cur = cur->collision;
    if (cur != NULL && strcmp(pfrag, cur->name) == 0)
    {
        voided_entries.push(cur);
        pfrag = strtok_r(NULL, "/", &str_tok);
    }
    else
    {
        // failed to find uri
        FPS_ERROR_PRINT("No currently subscribed uri found with matching base %s.\n", pfrag);
        free(uri_tok);
        return;
    }

    while(pfrag != NULL)
    {
        hash = strtohash(pfrag) % SUB_HASH_SIZE;
        cur = cur->next[hash];
        // traverse if collision
        while(cur != NULL && strcmp(pfrag, cur->name) > 0)
            cur = cur->collision;
        if (cur != NULL && strcmp(pfrag, cur->name) == 0)
            voided_entries.push(cur);
        else
        {
            // failed to find uri
            FPS_ERROR_PRINT("Tried to remove uri %s that is not currently subscribed.\n", uri);
            free(uri_tok);
            uri_tok = NULL;
            return;
        }
        pfrag = strtok_r(NULL, "/", &str_tok);
    }
    free(uri_tok);
    uri_tok = NULL;

    // voided_entries contains pointers to each subscription_hash in the path of URI
    // Start working way down stack to clean up no longer active paths
    // remove subscription for thread
    if ((cur->non_pub_conn_mask[thread_id/32] & 1 << (thread_id % 32)) != 0)
        cur->non_pub_conn_mask[thread_id/32] ^= 1 << (thread_id % 32);
    else if ((cur->pub_only_conn_mask[thread_id/32] & 1 << (thread_id % 32)) != 0)
        cur->pub_only_conn_mask[thread_id/32] ^= 1 << (thread_id % 32);
    else
        // No subscription matching thread at URI
        // should not get here so proceed for now
        FPS_ERROR_PRINT("No subscription for URI trying to remove.\n");
    voided_entries.pop();
    while(cur != NULL && voided_entries.empty() != true)
    {
        subscription_hash *cur_up = NULL;
        if (is_active_node(cur) == false)
        {
            // clean up and remove cur
            cur_up = voided_entries.top();
            hash = strtohash(cur->name) % SUB_HASH_SIZE;
            if (cur == cur_up->next[hash])
            {
                cur_up->next[hash] = cur->collision;
                free(cur->name);
                delete cur;
                cur = NULL;
            }
            else
            {
                subscription_hash *temp;
                temp = cur_up->next[hash];
                while (temp != NULL && temp->collision != cur)
                    temp = temp->collision;
                if (temp == NULL)
                {
                    //Error should never reach here
                }
                else // if (temp->collision == cur)
                {
                    temp->collision = cur->collision;
                    free(cur->name);
                    delete cur;
                    cur = NULL;
                }
            }
        }
        else
        {
            // Keep the rest of the URI since still needed
            break;
        }
        cur = cur_up;
        voided_entries.pop();
    }
    //clean up main table entry
    if (cur != NULL && voided_entries.empty())
    {
        if (is_active_node(cur) == false)
        {
            // clean up and remove cur
            hash = strtohash(cur->name) % HASH_TABLE_SIZE;
            if (cur == subc_table[hash])
            {
                subc_table[hash] = cur->collision;
                free(cur->name);
                delete cur;
                cur = NULL;
            }
            else
            {
                subscription_hash *temp;
                temp = subc_table[hash];
                while (temp != NULL && temp->collision != cur)
                    temp = temp->collision;
                if (temp == NULL)
                {
                    //Error should never reach here
                }
                else //if (temp->collision == cur)
                {
                    temp->collision = cur->collision;
                    free(cur->name);
                    delete cur;
                    cur = NULL;
                }
            }
        }
    }
}

// builds a mask of connections who are subscribed to uri
void get_connections_for_uri(const fims::str_view uri, uint32_t * thread_mask, bool pub)
{
    if (thread_mask == NULL)
        return;
    // copy base mask in for connections subscribed to all
    memcpy(thread_mask, non_pub_base_mask, sizeof(uint32_t) * SUB_MASK_SIZE);
    if (pub)
    {
        for (int i = 0; i < SUB_MASK_SIZE; i++)
            thread_mask[i] |= pub_only_base_mask[i];
    }
    char *uri_tok, *pfrag, *tok_str;
    unsigned int hash;
    uri_tok = strndup(uri.data, uri.size);
    tok_str = NULL;
    pfrag = strtok_r(uri_tok, "/", &tok_str);
    hash = strtohash(pfrag) % HASH_TABLE_SIZE;
    subscription_hash* cur = subc_table[hash];
    // traverse if collision
    while(cur != NULL && strcmp(pfrag, cur->name) > 0)
        cur = cur->collision;
    if (cur != NULL && strcmp(pfrag, cur->name) == 0)
    {
        for(int i = 0; i < SUB_MASK_SIZE; i++)
        {
            thread_mask[i] |= (pub) ? cur->pub_only_conn_mask[i] | cur->non_pub_conn_mask[i] :
            cur->non_pub_conn_mask[i];
        }
        pfrag = strtok_r(NULL, "/", &tok_str);
    }
    else
    {
        free(uri_tok);
        return;
    }

    while(pfrag != NULL)
    {
        hash = strtohash(pfrag) % SUB_HASH_SIZE;
        cur = cur->next[hash];
        // traverse if collision
        while(cur != NULL && strcmp(pfrag, cur->name) > 0)
            cur = cur->collision;
        if (cur != NULL && strcmp(pfrag, cur->name) == 0)
        {
            for(int i = 0; i < SUB_MASK_SIZE; i++)
            {
                thread_mask[i] |= (pub) ? cur->pub_only_conn_mask[i] | cur->non_pub_conn_mask[i] :
                cur->non_pub_conn_mask[i];
            }
        }
        else
        {
            free(uri_tok);
            return;
        }
        pfrag = strtok_r(NULL, "/", &tok_str);
    }
    free(uri_tok);
}

void clean_up_connection(client_info* info)
{
    // if socket is not currently close, close it
    if (info->sock != 0)
    {
        int sock = info->sock;
        info->sock = 0;
        shutdown(sock, SHUT_RDWR);
        close(sock);
    }
    // if there's at least one subscription, loop through and remove them all
    // Need lock to remove the subscriptions from the hash table; clientinfo subscriptions
    // are removed as well because it's more efficient to do so inside the critical section
    // even though it increases its length.
    if (info->subscriptions[0] != NULL)
    {
        lock_subscriptions();
        for (int i = 0; i < MAX_SUBSCRIPTIONS && info->subscriptions[i] != NULL; i++)
        {
            remove_subscription(info->subscriptions[i], info->thread_id);
            delete(info->subscriptions[i]);
            info->subscriptions[i] = NULL;
            info->pub_only[i] = false;
        }
        unlock_subscriptions();
    }
}

/******************************************************************************/

// each socket will create a thread to handle io from socket
void *receive_messages(void* args)
{
    auto*             info = static_cast<client_info*>(args);
    Receiver_Bufs<MAX_MESSAGE_SIZE - Meta_Data_Info::Buf_Len - sizeof(Meta_Data_Info), 0> receiver_bufs;
    auto& meta_data = receiver_bufs.meta_data;
    uint32_t          thread_mask[SUB_MASK_SIZE];
    FPS_DEBUG_PRINT("%s thread starting  sock %d info %p\n", info->process_name, info->sock, (void *) info);

    while (running == true && info->sock != 0)
    {
        FPS_DEBUG_PRINT("%s waiting for messages\n", info->process_name);
        FPS_DEBUG_PRINT("Waiting for message on socket %d\n", info->sock);
        const bool recv_ret = recv_raw_message(info->sock, receiver_bufs);
        auto err = errno;
        if (!recv_ret)
        {
            //Typically caused by a connection closing by application
            FPS_ERROR_PRINT("\"%s\",  Socket %d, thread %d: closed or error reading. rc %ld\n"
                    , info->process_name, info->sock, info->thread_id, (long int)err);
            clean_up_connection(info);
            break;
        }

        // valid meta data check (sub doesn't require "uri" therefore that check isn't here):
        if (meta_data.method_len == 0 || meta_data.process_name_len == 0)
        {
            FPS_ERROR_PRINT("\"%s\": Message missing method %d , and/or process_name %d \n"
                    , info->process_name
                    , meta_data.method_len
                    , meta_data.process_name_len
                    );
            continue;
        }

        const auto method      = receiver_bufs.get_method_view();
        const auto uri         = receiver_bufs.get_uri_view();
        const auto process_name = receiver_bufs.get_process_name_view();

        // add subscription
        // TODO consider making full method in mutex
        // currently there is a risk of incoming message for already subscribed uri
        // sending out when message is expecting response string
        if (strncmp(method.data, "sub", 3) == 0)
        {
            // extract out sub info (decryption if necessary):
            void* data = nullptr;
            defer { if (data && g_aesKey) free(data); };
            data = decrypt_buf(receiver_bufs);

            if (!data)
            {
                const char errstr[] = "Subscribe did not have data.";
                aes_send_raw_message(info->sock, nullptr, 0, nullptr, 0, nullptr, 0, "fims_server", sizeof("fims_server") - 1, nullptr, 0, (void*) errstr, sizeof(errstr) - 1);
                FPS_DEBUG_PRINT("sender name: %.*s, %s\n", (int) process_name.size, process_name.data , errstr);
                continue;
            }

            auto* data_ptr        = reinterpret_cast<uint8_t*>(data);
            // first byte is sub_amount:
            const auto sub_amount = data_ptr[0];
            // sub info array:
            auto* sub_info_ptr    = reinterpret_cast<Sub_Info*>(data_ptr + sizeof(sub_amount));
            // actual strings array:
            auto* sub_str_ptr     = reinterpret_cast<const char*>(data_ptr + sizeof(sub_amount) + sizeof(Sub_Info) * sub_amount);

            //FPS_ERROR_PRINT("sender name: %.*s, sub_amount  = %d\n", (int) process_name.size, process_name.data , sub_amount);
            if (sub_amount == 0)
            {
                const char errstr[] = "No uris to subscribe to.";
                aes_send_raw_message(info->sock, nullptr, 0, nullptr, 0, nullptr, 0, "fims_server", sizeof("fims_server") - 1, nullptr, 0, (void*) errstr, sizeof(errstr) - 1);
                FPS_ERROR_PRINT("sender name: %.*s, err = %s\n", (int) process_name.size, process_name.data , errstr);
                continue;
            }
            if (sub_amount > MAX_SUBSCRIPTIONS)
            {
                const char errstr[] = "Too many Uri's to subscribe.";
                aes_send_raw_message(info->sock, nullptr, 0, nullptr, 0, nullptr, 0, "fims_server", sizeof("fims_server") - 1, nullptr, 0, (void*) errstr, sizeof(errstr) - 1);
                FPS_ERROR_PRINT("sender name: %.*s, err = %s\n", (int) process_name.size, process_name.data , errstr);
                continue;
            }

            // loop through body array, extracting all new subscriptions
            fims::str_view sub_requests[MAX_SUBSCRIPTIONS];
            bool pub_requests[MAX_SUBSCRIPTIONS];
            bool uris_valid = true;
            std::size_t curr_subs_idx = 0;
            fims::str_view err_sub_str;
            for (int i = 0; i < sub_amount; ++i)
            {
                const auto curr_sub_info = sub_info_ptr[i];
                const auto curr_sub = fims::str_view{sub_str_ptr + curr_subs_idx, curr_sub_info.str_size};
                curr_subs_idx += curr_sub_info.str_size;
                sub_requests[i] = curr_sub;
                pub_requests[i] = curr_sub_info.pub_only;

                if (!valid_uri(sub_requests[i]))
                {
                    err_sub_str = sub_requests[i];
                    uris_valid = false;
                    break;
                }
            }
            if (uris_valid == false)
            {
                const char errstr[] = "Invalid Uri: %.*s";
                char* fmt_buf = (char*) malloc(sizeof(errstr) - 1 + err_sub_str.size);
                defer { if (fmt_buf) free(fmt_buf); };
                const auto send_amount = asprintf(&fmt_buf, errstr, (int) err_sub_str.size, err_sub_str.data);
                aes_send_raw_message(info->sock, nullptr, 0, nullptr, 0, nullptr, 0, "fims_server", sizeof("fims_server") - 1, nullptr, 0, (void*) fmt_buf, send_amount);
                FPS_ERROR_PRINT("sender name: %.*s, %s\n", (int) process_name.size, process_name.data , fmt_buf);
                continue;
            }
            // no errors send "SUCCESS":
            const char success_str[] = "SUCCESS";
            bool send_ret = aes_send_raw_message(info->sock, nullptr, 0, nullptr, 0, nullptr, 0, "fims_server", sizeof("fims_server") - 1, nullptr, 0, (void*) success_str, sizeof(success_str) - 1);
            if (!send_ret) {
                FPS_ERROR_PRINT("\"%s\",  Socket %d, thread %d: error sending success reply to subscription request. rc %d\n", 
                    info->process_name, info->sock, info->thread_id, errno);
            }

            // check if subscriptions exist and clean them up
            char* add_subscriptions[MAX_SUBSCRIPTIONS];
            bool  add_pubs[MAX_SUBSCRIPTIONS];
            char* remove_subscriptions[MAX_SUBSCRIPTIONS];
            char* already_subscribed[MAX_SUBSCRIPTIONS];
            bool  already_pubs[MAX_SUBSCRIPTIONS];
            int needs_added, needs_removed, already_sub;
            needs_added = needs_removed = already_sub = 0;

            // if a new subscription is already in the list of subscriptions
            for(int i = 0; i < sub_amount; i++)
            {
                bool found = false;
                for(int j = 0; info->subscriptions[j] != NULL; j++)
                {
                    if ((strncmp(info->subscriptions[j], sub_requests[i].data, sub_requests[i].size) == 0)
                         && (strlen(info->subscriptions[j]) == sub_requests[i].size))
                    {
                        // if we find it and it's the same type of subscription it was before,
                        // we're already subscribed. Otherwise, we need to remove and re-add
                        // the subscription with the other pub_only boolean.
                        if (info->pub_only[j] == pub_requests[i])
                        {
                            // Existing subscriptions already has uri from update
                            already_subscribed[already_sub] = info->subscriptions[j];
                            already_pubs[already_sub] = info->pub_only[j];
                            already_sub++;
                            found = true;
                        }
                        break;
                    }
                }
                if (found == false)
                {
                    // URI in update was not previously subscribed
                    add_subscriptions[needs_added] = strndup(sub_requests[i].data, sub_requests[i].size);
                    add_pubs[needs_added] = pub_requests[i];
                    needs_added++;
                }
            }
            // check to see if any currently subscribed URIs are going away
            for(int i = 0; info->subscriptions[i] != NULL; i++)
            {
                bool found = false;
                for(int j = 0; j < already_sub; j++)
                {
                    if (info->subscriptions[i] == already_subscribed[j])
                    {
                        // still in subscribe list, do nothing
                        found = true;
                        break;
                    }
                }
                if (found == false)
                {
                    // item in existing subscriptions not in update
                    remove_subscriptions[needs_removed] = info->subscriptions[i];
                    needs_removed++;
                }
            }

            memset(info->subscriptions, 0, sizeof(char*) * MAX_SUBSCRIPTIONS);
            for(int i = 0; i < already_sub; i++)
            {
                info->subscriptions[i] = already_subscribed[i];
                info->pub_only[i] = already_pubs[i];
            }

            lock_subscriptions();
            for(int i = 0; i < needs_removed; i++)
            {
                remove_subscription(remove_subscriptions[i], info->thread_id);
                free(remove_subscriptions[i]);
            }
            for(int i = 0; i < needs_added; i++)
            {
                insert_subscription(add_subscriptions[i], info->thread_id, add_pubs[i]);
                info->subscriptions[already_sub + i] = add_subscriptions[i];
                info->pub_only[already_sub + i] = add_pubs[i];
            }
            unlock_subscriptions();
        }
        else // not a sub so send message to incoming connections
        {
            if (meta_data.uri_len == 0)
            {
                FPS_ERROR_PRINT("uri missing from message\n");
                continue;
            }
            struct iovec resend_bufs[] = {
                { (void*) &meta_data,                         sizeof(Meta_Data_Info) },
                { (void*) receiver_bufs.data_buf,             meta_data.get_total_bytes() }
            };
            // Get all subscribed thread and send message to each
            lock_subscriptions_ro();
            get_connections_for_uri(uri, thread_mask, strncmp(method.data, "pub", 3) == 0);
            unlock_subscriptions_ro();

            for(int i = 0; i < SUB_MASK_SIZE; i++)
            {
                int value = i*32;

                for(uint32_t j = thread_mask[i]; j > 0; j>>=1, value++)
                {
                    // if current value is selected in mask send to connections
                    if (((j & 1) == 1) && value != info->thread_id)
                    {
                        //msg_sent = true;
                        FPS_DEBUG_PRINT("Socket %d sending message to Socket %d\n", info->sock, connections[value].sock);
                        // NOTE this send out the original incoming buffer
                        // NOTE(WALKER): this no longer has MSG_DONTWAIT flag for resending out the data to the appropriate person
                        if (connections[value].sock == 0)
                        {
                            continue;
                        }
                        auto rc = writev_nonblock(connections[value].sock, resend_bufs, sizeof(resend_bufs) / sizeof(*resend_bufs));
                        if (rc <= 0)
                        {
                            if (errno == EWOULDBLOCK)
                            {
                                timespec current_time;
                                clock_gettime(CLOCK_MONOTONIC, &current_time);
                                // Send a message if we've been at least 2 minutes without a buffer drop
                                if (current_time.tv_sec - connections[value].last_time.tv_sec >= 120)
                                {
                                    lock_subscriptions_ro();
                                    get_connections_for_uri(fims::str_view{"/events", sizeof("/events") - 1}, thread_mask, false);
                                    unlock_subscriptions_ro();
                                    const char err_fmt_str[] = "Process %s: Message dropped due to full buffer\n";
                                    void* ermsg = (char *)malloc(sizeof(err_fmt_str) + Meta_Data_Info::Max_Meta_Data_Str_Size);
                                    defer { if (ermsg) free(ermsg); };
                                    snprintf((char *)ermsg, sizeof(err_fmt_str) + Meta_Data_Info::Max_Meta_Data_Str_Size, err_fmt_str, connections[value].process_name);
				                    FPS_ERROR_PRINT("%s", (char*)ermsg);
                                    uint32_t elen = strlen((char *)ermsg);
                                    void* to_send = encrypt(ermsg, elen);
                                    defer { if (to_send && g_aesKey) free(to_send); };
                                }
                                // we've just had a message drop so update the last_time a message dropped to now
                                connections[value].last_time = current_time;
                            }
                            else
                            {
                                FPS_ERROR_PRINT("Failed to send message due to: %s\n", strerror(errno));
                                //clean up connection for lost server. This should remove our subscription to this connection
                                clean_up_connection(&connections[value]);
                            }
                        }
                    }
                }
            }
        }
    }
    FPS_DEBUG_PRINT("Exit thread %d info %p 0\n", info->thread_id, (void *) info);
    pthread_exit((void*)0);
}

/******************************************************************************/

// wait for new connections and create a thread to handle socket
int wait_for_connection()
{
    struct sockaddr_un name;
    int ret;

    // setup socket for internal unix socket
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sock_fd == -1)
    {
        FPS_ERROR_PRINT("Failed to make socket.\n");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(struct sockaddr_un));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind(sock_fd, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));
    if (ret == -1)
    {
        FPS_ERROR_PRINT("Failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }

    ret = listen(sock_fd, BACKLOG);
    if (ret == -1)
    {
        FPS_ERROR_PRINT("Unable to listen on socket.\n");
        exit(EXIT_FAILURE);
    }

    Handshake handshake;
    struct iovec handhsake_bufs[] = {
        { (void*) &handshake, sizeof(Handshake) }
    };

    // wait for connection and start thread to handle
    while (running == true)
    {
        // setup handshake data:
        handshake.fims_data_layout_version = FIMS_DATA_LAYOUT_VERSION;
        handshake.max_message_size = MAX_MESSAGE_SIZE;

        FPS_DEBUG_PRINT("waiting for new socket\n");
        int new_sock;
        new_sock = accept(sock_fd, NULL, NULL);

        if (new_sock > 0)
        {
            // commence handshake with the application:
            if (writev_nonblock(new_sock, handhsake_bufs, sizeof(handhsake_bufs) / sizeof(*handhsake_bufs)) <= 0)
            {
                FPS_ERROR_PRINT("Failed to send handshake to new connection on socket %d.\n", new_sock);
                shutdown(new_sock, SHUT_RDWR);
                close(new_sock);
                continue;
            }
            if (readv(new_sock, handhsake_bufs, sizeof(handhsake_bufs) / sizeof(*handhsake_bufs)) <= 0)
            {
                FPS_ERROR_PRINT("Failed to receive handshake back from new connection on socket %d.\n", new_sock);
                shutdown(new_sock, SHUT_RDWR);
                close(new_sock);
                continue;
            }
            if (handshake.fims_data_layout_version != FIMS_DATA_LAYOUT_VERSION) // server has different layout interpretation than the application
            {
                FPS_ERROR_PRINT("application's fims_data_layout_version (got: %d) is different from the server's (should be: %d) on socket %d.\n", handshake.fims_data_layout_version, FIMS_DATA_LAYOUT_VERSION, new_sock);
                shutdown(new_sock, SHUT_RDWR);
                close(new_sock);
                continue;
            }
            if (handshake.max_message_size != MAX_MESSAGE_SIZE)
            {
                FPS_ERROR_PRINT("application's max_message_size (got: %d) is different from the server's (should be: %d) on socket %d.\n", handshake.fims_data_layout_version, FIMS_DATA_LAYOUT_VERSION, new_sock);
                shutdown(new_sock, SHUT_RDWR);
                close(new_sock);
                continue;
            }

            // Check the current socket send buffer size
            int sndBufferSize;
            socklen_t sockOptSize = sizeof(sndBufferSize);
            getsockopt(new_sock, SOL_SOCKET, SO_SNDBUF, &sndBufferSize, &sockOptSize);

            // Set the send buffer size if the current size is less than the fims max message size
            if (sndBufferSize < MAX_MESSAGE_SIZE)
            {
                sndBufferSize = MAX_MESSAGE_SIZE;
                sockOptSize = sizeof(sndBufferSize);
                if (setsockopt(new_sock, SOL_SOCKET, SO_SNDBUF, &sndBufferSize, sockOptSize) == -1) 
                {
                    FPS_ERROR_PRINT("%s >> Failed to set the socket send buffer as %d for sock %d, errno <%d>(%s)", __func__, sndBufferSize, new_sock, errno, strerror(errno));
                }
            }

            // find first available position in array
            int first_available = 0;
            for(; first_available < MAX_CONNECTS && connections[first_available].sock != 0; first_available++);

            if (first_available == MAX_CONNECTS)
            {
                FPS_ERROR_PRINT("No available connections.\n");
                shutdown(new_sock, SHUT_RDWR);
                close(new_sock);
                continue;
            }

            // make sure we join the previous thread at that connection
            // when thread exits it will close socket but cannot rejoin to free resources
            if (connections[first_available].thread != 0)
            {
                pthread_join(connections[first_available].thread, NULL);
                connections[first_available].thread = 0;
            }

            // set information for this connection
            connections[first_available].sock = new_sock;
            connections[first_available].thread_id = first_available;
            // set the connecting process' name here
            // NOTE(WALKER): this directly sends the data from the socket straight into the name buf now
            struct iovec recv_name_buf[] {
                { (void*) connections[first_available].process_name, sizeof(connections[first_available].process_name) - 1 }
            };
            const auto bytes_read = readv(new_sock, recv_name_buf, sizeof(recv_name_buf) / sizeof(*recv_name_buf));
            //FPS_ERROR_PRINT("XXX setting  sock %d process name len : %d\n", (int)new_sock, (int)bytes_read);
            if (bytes_read > 0)
            {
                connections[first_available].process_name[bytes_read] = '\0'; // null terminate char array
                FPS_DEBUG_PRINT("starting up connection to: \"%s\"\n", connections[first_available].process_name);
                clock_gettime(CLOCK_MONOTONIC, &(connections[first_available].last_time));
                connections[first_available].last_time.tv_sec -= 120;
                ret = pthread_create(&connections[first_available].thread, NULL, receive_messages, &connections[first_available]);
            }
            else 
            {
                ret = -1;
            }

            // start new thread to handle the new connection
            if (ret)
            {
                FPS_ERROR_PRINT("Failed to start thread, error %d\n", ret);
                connections[first_available].thread = 0;
                continue;
            }
        }
    }
    if (sock_fd != 0)
    {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
    }
    return 0;
}

/******************************************************************************/

int main(int argc, char **argv)
{
    #ifdef FPS_TEST_MODE
        test_main(argc, argv);
        return 0;
    #endif

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);


    //initialize our connections array
    memset(connections, 0, sizeof(struct client_info) * MAX_CONNECTS);

    // remove the socket in case program did not clean up properly on close
    unlink(SOCKET_NAME);
    

    g_aesKey = NULL; 
    loadAesKey(AESKEY_FILE);
    if (g_aesKey == NULL) 
    {
        FPS_RELEASE_PRINT(" Note: No Key in File : %s\n", AESKEY_FILE);
    }

    pthread_mutex_init(&subscription_mutex, NULL);
    pthread_cond_init(&subscription_lock_cv, NULL);
    pthread_cond_init(&subscription_reading_cv, NULL);

    wait_for_connection();

    // join all the threads
    for(int i = 0; i < MAX_CONNECTS; i++)
    {
        if (connections[i].thread != 0)
            pthread_join(connections[i].thread, NULL);
    }
    pthread_mutex_destroy(&subscription_mutex);
    pthread_cond_destroy(&subscription_lock_cv);
    pthread_cond_destroy(&subscription_reading_cv);
    unlink(SOCKET_NAME);
   	return 0;
}
