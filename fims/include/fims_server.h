//============================================================================
// Name        : fims_server.h
// Author      : Sam Rappl
// Version     :
// Copyright   : Your copyright notice
// Description : Declarations for the FlexGen Internal Messaging Service Server -
//               declare functionality of FIMS_Server to improve interface for
//               automated testing and future refactors.
//============================================================================

#ifndef FIMS_SERVER_H_
#define FIMS_SERVER_H_

/* OS Includes */
/* C Standard Library Dependencies */
#include <stdint.h>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "fims.h"

#define BACKLOG 20
#define SUB_MASK_SIZE 16
#define MAX_CONNECTS (SUB_MASK_SIZE * 32)
#define HASH_TABLE_SIZE 1024
#define SUB_HASH_SIZE   32

// turns false if ctrl-C is pressed
extern volatile bool running;
extern volatile int sock_fd;

struct subscription_lock
{
    bool lock = false;
    int  reading  = 0;
};

// ensures multiple threads cannot subscribe at the same position
// in subscription table and overwrite each other
extern subscription_lock sub_lock;

/*
 * Paths are split into subdirectories and hashed into the subscription
 * table. For example, if a client subscribes to '/assets/generators/info'
 * 'assets' will be hashed into the subscription table. Then 'generators'
 * will be hashed into the subtable for 'assets'. Then 'info' will be
 * hashed into the subtable for 'generators'. The client's thread_id will
 * be ORed with the bit mask for 'info'.
 */
struct subscription_hash
{
    /*
    Each bit in these 128 bit masks corresponds to a client thread.
    pub_only is where the clients who subscribe to only publish
    messages from this URI are masked. non_pub is where the clients
    who subscribe to all messages from this URI are masked.
    */
    uint32_t                  pub_only_conn_mask[SUB_MASK_SIZE];
    uint32_t                  non_pub_conn_mask[SUB_MASK_SIZE];
    // Contains the pointers to subsequent directories in the URI.
    struct subscription_hash *next[SUB_HASH_SIZE];
    // Holds a pointer to the collision chain in the table if the name of
    // this subscription hash is not the name of the subdirectory you're looking
    // for.
    struct subscription_hash *collision;
    // The name of the subdirectory corresponding to this subscription hash.
    char*                     name;
};

extern subscription_hash *subc_table[HASH_TABLE_SIZE];

struct client_info
{
    pthread_t    thread;
    // used to identify this client from subscription hash bitmasks
    int          thread_id;
    // socket descriptor for communication
    volatile int sock;
    // list of plaintext subscription paths and corresponding pub subscriptions
    char        *subscriptions[MAX_SUBSCRIPTIONS];
    bool        pub_only[MAX_SUBSCRIPTIONS];
    // name of the client process for debugging if they drop messages
    // or for logging
    char         process_name[std::numeric_limits<decltype(Meta_Data_Info::process_name_len)>::max() + 1];
    timespec     last_time;
};

extern client_info connections[MAX_CONNECTS];

// base mask will hold all connections that are subscribed to all URIs
extern uint32_t pub_only_base_mask[SUB_MASK_SIZE];
extern uint32_t non_pub_base_mask[SUB_MASK_SIZE];

/**
 * Returns whether the URI is valid or not.
 */
bool valid_uri(const fims::str_view uri);

/**
 * Returns an int which is a hash of a string.
 */
unsigned int strtohash(const char* in_str);

/**
 * Adds all the frags of the URI into the table. Sets the mask at the leaf
 * of the URI to represent a subscription. This function assumes that the
 * URI passed in is valid.
 */
void insert_subscription(const char* uri, int thread_id, bool pub_only);

/**
 * Removes the given thread's subscription at the given URI. If the URI's mask
 * is empty after the thread's subscription has been removed, this function
 * cleans the path of the URI.
 */
void remove_subscription(const char* uri, int thread_id);

/**
 * Builds a mask of connections who are subscribed to the given URI.
 */ 
void get_connections_for_uri(const fims::str_view uri, uint32_t* thread_mask, bool pub);

/**
* Removes subscriptions from to a connection and closes socket
*/
void clean_up_connection(client_info* info);

/**
 * Handles I/O from socket.
 */
void* receive_messages(void* args);

/*
 * Waits for a socket to join then creates a thread to handle the connection
 * via receive_messages.
 */
int wait_for_connection();

#endif
