#ifndef LIBFIMS_INTERNAL_HPP
#define LIBFIMS_INTERNAL_HPP

/* OS Includes */
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>  /* htons ntohs */
/* C Standard Library Dependencies */
#include <cerrno>
#include <cstdio>
/* C++ Standard Library Dependencies */
#include <string>
#include <iostream>
#include <fstream>
#include <limits>
/* Local Internal Dependencies */
#include "fps_utils.h" // fims_addon requires local search path
#include "../include/libaes.h"
#include "../include/defer.hpp"
#include "../include/libfims.h"

// constants:
#define MAX_IOVEC_BUFS 7
#define MAX_MESSAGE_SIZE  924288

// aliases:
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

extern uint8_t* g_aesKey;

// First byte of subscribe is the number of subs
// second byte and onward is an array of these
// last array onward is the actual sub strings (non-null-terminated)
// layout is like this in the u8 array:
// u8 [[meta_data_bytes][sub_amount -> u8][Sub_Info_array][sub_strings]]
struct Sub_Info
{
    bool pub_only;
    uint8_t str_size;
};

// IMPORTANT(WALKER): NEVER EVER CHANGE THIS.
// this is the "handshake" that it sent between the server and a new client
// this will NEVER EVER EVER CHANGE in the future
// and should always be set in stone (otherwise we break ABI all over again)
// everything else (meta_data struct, data_layout inside the byte array) is subject to change
// for byte and alignment interpretation
struct Handshake
{
    u16 fims_data_layout_version;
    u32 max_message_size;
};

void* encrypt(void* buf, u32 &length);
void* decrypt(void* buf, u32 &length, u32 maxlen);

ssize_t writev_nonblock(int fd, iovec *iovec, size_t iovec_len);

bool send_raw_message(int connection, 
                        const char* method,       u8 method_len, 
                        const char* uri,          u8 uri_len,
                        const char* replyto,      u8 replyto_len,
                        const char* process_name, u8 process_name_len,
                        const char* username,     u8 username_len,
                        void* data,               u32 data_len) noexcept;

bool aes_send_raw_message(int connection, 
                            const char* method,       u8 method_len, 
                            const char* uri,          u8 uri_len,
                            const char* replyto,      u8 replyto_len,
                            const char* process_name, u8 process_name_len,
                            const char* username,     u8 username_len,
                            void* data,               u32 data_len) noexcept;
#endif
