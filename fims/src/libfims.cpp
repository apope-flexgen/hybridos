/*
 * libFIMS.cpp
 *
 *  Created on: Jun 12, 2018
 *      Author: jcalcagni
 */

/* OS Includes */
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>  /* htons ntohs */
/* C Standard Library Dependencies */
#include <cerrno>
#include <cstdio>
#include "pwd.h"
/* C++ Standard Library Dependencies */
#include <string>
#include <iostream>
#include <fstream>
#include <limits>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "libfims_internal.hpp"

extern uint8_t* g_aesKey;

// aliases:
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

void* encrypt(void* buf, uint32_t &length)
{
    static constexpr uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint32_t nlength;
    unsigned char* nbuf;

    if (g_aesKey == NULL) return buf;
    nlength = length;

    while (nlength % 16 != 0)
        nlength++;
    nlength += 32;
    nbuf = (unsigned char *) malloc(nlength);
    nlength =
        aesEncrypt ((unsigned char *)buf, (int)length, (unsigned char *)g_aesKey
                    , (unsigned char *)iv, nbuf);
    length = nlength;

    return nbuf;
}

void* decrypt(void* buf, uint32_t &length, uint32_t maxlen)
{
    static constexpr uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    if (g_aesKey == NULL) return buf;

    // NOTE(WALKER): the + 32 is for padding bytes due to certain json parsers needing it (like simdjson, that way they don't have to worry about it) 
    unsigned char* nbuf = (unsigned char *) malloc(length + 32);
    int nlen = aesDecrypt((unsigned char*) buf, length
                        , (unsigned char *)g_aesKey
                        , (unsigned char *)iv, nbuf);
    if(nlen >= (int) maxlen)
        nlen = maxlen;
    memset(nbuf + nlen, 0, 32); // set null terminated byte and 32 padding bytes to '\0'

    length = (uint32_t) nlen;
    return nbuf;
}

// NOTE(WALKER): data is generic and therefore we require a contiguous block of memory for it
// however, if we want to change that for the future we need to think of a good way to
// get more iovec's into the iovec array for writev so we don't have to memcpy
// this might be important in the future if we want to start passing around
// data that isn't a json string
bool send_raw_message(int connection, 
                        const char* method,       u8 method_len, 
                        const char* uri,          u8 uri_len,
                        const char* replyto,      u8 replyto_len,
                        const char* process_name, u8 process_name_len,
                        const char* username,     u8 username_len,
                        void* data,               u32 data_len) noexcept
{
    Meta_Data_Info meta_data;
    iovec bufs[MAX_IOVEC_BUFS];
    bufs[0].iov_base = (void*) &meta_data;
    bufs[0].iov_len  = sizeof(Meta_Data_Info);
    int bufs_counter = 1;
    if (method)
    {
        meta_data.method_len        = method_len;
        bufs[bufs_counter].iov_base = (void*) method;
        bufs[bufs_counter].iov_len  = method_len;
        ++bufs_counter;
    }
    if (uri)
    {
        meta_data.uri_len           = uri_len;
        bufs[bufs_counter].iov_base = (void*) uri;
        bufs[bufs_counter].iov_len  = uri_len;
        ++bufs_counter;
    }
    if (replyto)
    {
        meta_data.replyto_len       = replyto_len;
        bufs[bufs_counter].iov_base = (void*) replyto;
        bufs[bufs_counter].iov_len  = replyto_len;
        ++bufs_counter;
    }
    if (process_name)
    {
        meta_data.process_name_len   = process_name_len;
        bufs[bufs_counter].iov_base = (void*) process_name;
        bufs[bufs_counter].iov_len  = process_name_len;
        ++bufs_counter;
    }
    if (username)
    {
        meta_data.username_len     = username_len;
        bufs[bufs_counter].iov_base = (void*) username;
        bufs[bufs_counter].iov_len  = username_len;
        ++bufs_counter;
    }
    if (data)
    {
        meta_data.data_len          = data_len;
        bufs[bufs_counter].iov_base = (void*) data;
        bufs[bufs_counter].iov_len  = data_len;
        ++bufs_counter;
    }
    // send data out:
    const auto bytes_written = writev(connection, bufs, bufs_counter);
    return bytes_written > 0;
}

bool aes_send_raw_message(int connection, 
                            const char* method,       u8 method_len, 
                            const char* uri,          u8 uri_len,
                            const char* replyto,      u8 replyto_len,
                            const char* process_name, u8 process_name_len,
                            const char* username,     u8 username_len,
                            void* data,               u32 data_len) noexcept
{
    if (data) data = encrypt(data, data_len);
    defer { if (data && g_aesKey) free(data); }; // if we have encryption then free it up
    return send_raw_message(connection, method, method_len, uri, uri_len, replyto, replyto_len, process_name, process_name_len, username, username_len, data, data_len);
}

bool recv_raw_message(int connection, Meta_Data_Info& meta_data, void* data_buf, uint32_t data_buf_len) noexcept
{
    struct iovec bufs[] = {
        { (void*) &meta_data, sizeof(Meta_Data_Info) },
        { (void*) data_buf,   data_buf_len }
    };
    const auto bytes_read = readv(connection, bufs, sizeof(bufs) / sizeof(*bufs));
    return bytes_read > 0;
}

void* decrypt_buf(Meta_Data_Info& meta_data, void* data_buf, uint32_t data_buf_len) noexcept
{
    if (meta_data.data_len == 0) return nullptr; // no data to decrypt
    return decrypt(reinterpret_cast<uint8_t*>(data_buf) + meta_data.get_meta_bytes(), meta_data.data_len, data_buf_len - Meta_Data_Info::Buf_Len);
}

std::size_t Meta_Data_Info::get_meta_bytes() const noexcept
{
    return (method_len + uri_len + replyto_len + process_name_len + username_len);
}
// This is used by server only (for resending purposes):
std::size_t Meta_Data_Info::get_total_bytes() const noexcept
{
    return get_meta_bytes() + data_len;
}

fims_message::fims_message()
{
    method      = nullptr;
    uri         = nullptr;
    replyto     = nullptr;
    process_name = nullptr;
    username   = nullptr;
    body        = nullptr;
    pfrags      = nullptr;
    nfrags      = 0;
}

fims_message::~fims_message()
{
    if (method)      free(method);
    if (uri)         free(uri);
    if (replyto)     free (replyto);
    if (process_name) free(process_name);
    if (username)   free(username);
    if (body)        free(body);
    if (pfrags)      delete[] pfrags;
}

fims::fims()
{
    p_process_name_len = 0;
    connection         = FIMS_CONN_CLOSED;
    p_process_name     = nullptr;
    loadAesKey(AESKEY_FILE);
}

fims::~fims()
{
    FPS_DEBUG_PRINT("Fims %p closing connection for [%s] prog [%s]\n", (void*)this, p_process_name, program_invocation_short_name);
    if(connection > 0)
        close(connection);
    if(p_process_name != NULL)
        free(p_process_name);
    connection = FIMS_CONN_CLOSED;
}

// This function is called to make a connection to the server
// Must be called before any other operation can be performed
bool fims::Connect(const char* process_name)
{
    if (connection > 0) 
    {
        FPS_ERROR_PRINT("%s: Already connected to FIMS\n", program_invocation_short_name);
        return false;
    }

    static constexpr auto Max_Process_Name_Len = std::numeric_limits<decltype(Meta_Data_Info::process_name_len)>::max();

    if (process_name && strlen(process_name) > Max_Process_Name_Len)
    {
        FPS_ERROR_PRINT("%s: Desired process_name is more than the maximum of %d characters for FIMS\n", process_name, Max_Process_Name_Len);
        return false;
    }
    else if (strlen(program_invocation_short_name) > Max_Process_Name_Len)
    {
        FPS_ERROR_PRINT("%s: This program's name is more than the maximum of %d characters for FIMS\n", program_invocation_short_name, Max_Process_Name_Len);
        return false;
    }

    //create socket
    connection = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (connection == FIMS_CONN_CLOSED) 
    {
        FPS_ERROR_PRINT("%s: Failed to create a socket\n", program_invocation_short_name);
        return false;
    }

    // connect to server
    struct sockaddr_un remote;
    int return_val;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCKET_NAME, sizeof(remote.sun_path) - 1);
    return_val = connect(connection, (struct sockaddr *)&remote, sizeof(remote));
    if(return_val == FIMS_CONN_CLOSED)
    {
        FPS_ERROR_PRINT("%s: Failed to make connection to FIMS.\n", program_invocation_short_name);
        close(connection);
        connection = FIMS_CONN_CLOSED;
        return false;
    }

    // Check the current socket send buffer size
    int sndBufferSize;
    socklen_t sockOptSize = sizeof(sndBufferSize);
    getsockopt(connection, SOL_SOCKET, SO_SNDBUF, &sndBufferSize, &sockOptSize);

    // If the current socket send buffer size is less than the MAX_MESSAGE_SIZE defined for fims messages,
    // then set the max socket send buffer size to be at least as large as MAX_MESSAGE_SIZE in case
    // a fims message of considerable size is being passed to the send buffer
    //
    // Note: setsockopt for SO_SNDBUF can set the send buffer size in bytes up to the maximum buffer size
    // (defined in /proc/sys/net/core/wmem_max). wmem_max cannot be overwritten if the buffer size to set is
    // greater than wmem_max
    if (sndBufferSize < MAX_MESSAGE_SIZE)
    {
        // Set the max socket send buffer size
        sndBufferSize = MAX_MESSAGE_SIZE;
        sockOptSize = sizeof(sndBufferSize);
        if (setsockopt(connection, SOL_SOCKET, SO_SNDBUF, &sndBufferSize, sockOptSize) == -1) 
        {
            FPS_ERROR_PRINT("%s >> Failed to set the socket send buffer as %d for sock %d, errno <%d>(%s)", __func__, sndBufferSize, connection, errno, strerror(errno));
        }
    }

    //set socket to have 2 second timeout
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // commence handshake with the server and set data_layout version and max_message_size
    // IMPORTANT(WALKER): This is NOT subject to change
    // this "handshake" is important for reinterpretation layout versioning
    // of bytes between the server and applications
    // as well as between applications (in case we change things in the future)
    Handshake handshake;
    struct iovec handshake_bufs[] = {
        { (void*) &handshake, sizeof(handshake) }
    };

    if (readv(connection, handshake_bufs, sizeof(handshake_bufs) / sizeof(*handshake_bufs)) <= 0)
    {
        FPS_ERROR_PRINT("%s: Failed to receive handshake from the server.\n", program_invocation_short_name);
        close(connection);
        connection = FIMS_CONN_CLOSED;
        return false;
    }
    // set max_message_size
    max_message_size = handshake.max_message_size;

    const auto recv_layout_version = handshake.fims_data_layout_version;

    // setup handshake to send back:
    handshake.fims_data_layout_version = FIMS_DATA_LAYOUT_VERSION;

    // send handshake back to the server:
    if (writev(connection, handshake_bufs, sizeof(handshake_bufs) / sizeof(*handshake_bufs)) <= 0)
    {
        FPS_ERROR_PRINT("%s: Failed to send handshake back to the server.\n", program_invocation_short_name);
        close(connection);
        connection = FIMS_CONN_CLOSED;
        return false;
    }

    // Must have the same data layout version as the server:
    if (recv_layout_version != FIMS_DATA_LAYOUT_VERSION)
    {
        FPS_ERROR_PRINT("%s: Server's data layout version was %d instead of the expected version of %d. Cannot continue\n", program_invocation_short_name, recv_layout_version, FIMS_DATA_LAYOUT_VERSION);
        close(connection);
        connection = FIMS_CONN_CLOSED;
        return false;
    }

    // after handshake, send your process name to the server (other stuff that is subject to change)
    // this is the extra process information that the server needs:
    if(p_process_name) free(p_process_name);

    if (process_name == NULL)
        p_process_name = strdup(program_invocation_short_name); // NOTE(WALKER): This has been changed so everybody at least has a name when sending stuff around
    else
        p_process_name = strdup(process_name);
    // setup name:
    p_process_name_len = static_cast<u8>(strlen(p_process_name));

    struct iovec send_name_bufs[] = {
        { (void*) p_process_name, p_process_name_len }
    };

    // send out process name after welcome string:
    if (writev(connection, send_name_bufs, sizeof(send_name_bufs) / sizeof(*send_name_bufs)) <= 0)
    {
        FPS_ERROR_PRINT("%s, Could not send name to server.\n", program_invocation_short_name);
        return false;
    }

    //return socket to blocking
    tv.tv_sec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    return true;
}

// This function will set the URI's that messages are forward for
// This function should not be called from loop processing send or receive messages
// TODO if you are already subscribed it is possible that you could receive a message wait for confirmation
bool fims::Subscribe(const char** uri_array, int count, bool* pub_array)
{
    // Ensure connection
    if(connection <= 0)
    {
        FPS_ERROR_PRINT("%s: Need to connect before calling subscribe.\n", program_invocation_short_name);
        return false;
    }

    if (count > MAX_SUBSCRIPTIONS)
    {
        FPS_ERROR_PRINT("%s: The number of subscriptions (currently: %d) exceeds the maximum of %d.\n", program_invocation_short_name, count, MAX_SUBSCRIPTIONS);
        return false;
    }

    u32 total_sub_bytes = sizeof(u8) + sizeof(Sub_Info) * count;
    for (int i = 0; i < count; ++i)
    {
        const auto len = strlen(uri_array[i]);
        if (len > Meta_Data_Info::Max_Meta_Data_Str_Size)
        {
            FPS_ERROR_PRINT("%s: subscription uri \"%s\" exceeds the maximum sub length of %d.\n", program_invocation_short_name, uri_array[i], Meta_Data_Info::Max_Meta_Data_Str_Size);
            return false;
        }
        total_sub_bytes += len;
    }

    void* send_data = malloc(total_sub_bytes);
    defer { free(send_data); };

    // setup send_data:
    auto* data_ptr = reinterpret_cast<u8*>(send_data);
    // how many subs we have:
    data_ptr[0] = static_cast<u8>(count);
    // sub info array:
    auto* sub_info_ptr = reinterpret_cast<Sub_Info*>(data_ptr + sizeof(u8));
    // start of char array:
    auto* str_ptr = reinterpret_cast<char*>(data_ptr + sizeof(u8) + sizeof(Sub_Info) * count);
    std::size_t curr_idx = 0;
    for (int i = 0; i < count; ++i)
    {
        new (&sub_info_ptr[i]) Sub_Info{false, 0};
        if (pub_array) sub_info_ptr[i].pub_only = pub_array[i];
        const auto len = strlen(uri_array[i]);
        sub_info_ptr[i].str_size = static_cast<u8>(len);
        memcpy(str_ptr + curr_idx, uri_array[i], len);
        curr_idx += len;
    }

    if (!aes_send_raw_message(connection, "sub", 3, nullptr, 0, nullptr, 0, p_process_name, p_process_name_len, nullptr, 0, send_data, total_sub_bytes))
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: Failed to send subscribe to server.\n", program_invocation_short_name);
        return false;
    }

    //set socket to have 2 second timeout
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    Receiver_Bufs<MAX_MESSAGE_SIZE - Meta_Data_Info::Buf_Len - sizeof(Meta_Data_Info), 0> receiver_bufs;
    auto& meta_data = receiver_bufs.meta_data;
    if (!recv_raw_message(connection, receiver_bufs))
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: No response from server within 2 seconds.\n", program_invocation_short_name);
        return false;
    }

    // decrypt:
    void* data = nullptr;
    defer { if (data && g_aesKey) free(data); };
    data = decrypt_buf(receiver_bufs);

    // size check
    if (!data || meta_data.data_len > receiver_bufs.get_max_expected_data_len())
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: The number of bytes received (currently: %d) is either 0 or exceeds the expected maximum of %d. Bytes are being cut off\n", program_invocation_short_name, receiver_bufs.meta_data.data_len, receiver_bufs.get_max_expected_data_len());
        return false;
    }

    // check confirmation string
    if (strncmp((const char*) data, "SUCCESS", sizeof("SUCCESS") - 1) != 0)
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: Did not get success message from server. Got \"%s\" instead\n", program_invocation_short_name, (const char*) data);
        return false;
    }

    //return socket to blocking
    tv.tv_sec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    return true;
}

bool fims::Subscribe(const std::vector<std::string>& sub_array, bool pub_only)
{
    // Ensure connection
    if(connection <= 0)
    {
        FPS_ERROR_PRINT("%s: Need to connect before calling subscribe.\n", program_invocation_short_name);
        return false;
    }

    if (sub_array.size() > MAX_SUBSCRIPTIONS)
    {
        FPS_ERROR_PRINT("%s: The number of subscriptions (currently: %ld) exceeds the maximum of %d.\n", program_invocation_short_name, sub_array.size(), MAX_SUBSCRIPTIONS);
        return false;
    }

    u32 total_sub_bytes = sizeof(u8) + sizeof(Sub_Info) * sub_array.size();
    for (const auto& str : sub_array)
    {
        if (str.size() > Meta_Data_Info::Max_Meta_Data_Str_Size)
        {
            FPS_ERROR_PRINT("%s: subscription uri \"%s\" exceeds the maximum sub length of %d.\n", program_invocation_short_name, str.data(), Meta_Data_Info::Max_Meta_Data_Str_Size);
            return false;
        }
        total_sub_bytes += str.size();
    }

    void* send_data = malloc(total_sub_bytes);
    defer { free(send_data); };

    // setup send_data:
    auto* data_ptr = reinterpret_cast<u8*>(send_data);
    // how many subs we have:
    data_ptr[0] = static_cast<u8>(sub_array.size());
    // sub info array:
    auto* sub_info_ptr = reinterpret_cast<Sub_Info*>(data_ptr + sizeof(u8));
    // start of char array:
    auto* str_ptr = reinterpret_cast<char*>(data_ptr + sizeof(u8) + sizeof(Sub_Info) * sub_array.size());
    std::size_t curr_idx = 0;
    for (std::size_t i = 0; i < sub_array.size(); ++i)
    {
        const auto& str = sub_array[i];
        new (&sub_info_ptr[i]) Sub_Info{pub_only, static_cast<u8>(str.size())};
        memcpy(str_ptr + curr_idx, str.data(), str.size());
        curr_idx += str.size();
    }

    if (!aes_send_raw_message(connection, "sub", 3, nullptr, 0, nullptr, 0, p_process_name, p_process_name_len, nullptr, 0, send_data, total_sub_bytes))
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: Failed to send subscribe to server.\n", program_invocation_short_name);
        return false;
    }

    //set socket to have 2 second timeout
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    Receiver_Bufs<MAX_MESSAGE_SIZE - Meta_Data_Info::Buf_Len - sizeof(Meta_Data_Info), 0> receiver_bufs;
    auto& meta_data = receiver_bufs.meta_data;
    if (!recv_raw_message(connection, receiver_bufs))
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: No response from server within 2 seconds.\n", program_invocation_short_name);
        return false;
    }

    // decrypt:
    void* data = nullptr;
    defer { if (data && g_aesKey) free(data); };
    data = decrypt_buf(receiver_bufs);

    // size check and then decrypt:
    if (!data || meta_data.data_len > receiver_bufs.get_max_expected_data_len())
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: The number of bytes received (currently: %d) is either 0 or exceeds the expected maximum of %d. Bytes are being cut off\n", program_invocation_short_name, receiver_bufs.meta_data.data_len, receiver_bufs.get_max_expected_data_len());
        return false;
    }

    // check confirmation string
    if (strncmp((const char*) data, "SUCCESS", sizeof("SUCCESS") - 1) != 0)
    {
        close(connection);
        connection = FIMS_CONN_CLOSED;
        FPS_ERROR_PRINT("%s: Did not get success message from server. Got \"%s\" instead\n", program_invocation_short_name, (const char*) data);
        return false;
    }

    //return socket to blocking
    tv.tv_sec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    return true;
}

// This function will create and send a message to FIMS server
bool fims::Send(const char* method, const char* uri, const char* replyto, const char* body, const char* username)
{
    if (connection <= 0)
    {
        FPS_ERROR_PRINT("%s: Need to connect before calling send.\n", program_invocation_short_name);
        return false;
    }
    if (method == NULL || uri == NULL)
    {
        FPS_ERROR_PRINT("%s: Can't transmit message without method or uri.\n", program_invocation_short_name);
        return false;
    }

    // strlen calls (lame, but necessary):
    const auto method_len   = strlen(method);
    const auto uri_len      = strlen(uri);
    const auto replyto_len  = replyto ? strlen(replyto) : 0;
    const auto username_len = username ? strlen(username) : 0;
    const auto body_len     = body ? strlen(body) : 0;
    const auto total_bytes  = sizeof(Meta_Data_Info) + method_len + uri_len + replyto_len + p_process_name_len + username_len + body_len;

    // no more than 255 characters for method, uri, replyto, and username
    if (method_len > Meta_Data_Info::Max_Meta_Data_Str_Size 
            || uri_len > Meta_Data_Info::Max_Meta_Data_Str_Size 
                || replyto_len > Meta_Data_Info::Max_Meta_Data_Str_Size
                    || username_len > Meta_Data_Info::Max_Meta_Data_Str_Size)
    {
        FPS_ERROR_PRINT("one of method, uri, replyto, or username is more than 255 characters\n");
        return false;
    }
    if (total_bytes > max_message_size)
    {
        FPS_ERROR_PRINT("the number of bytes to send: %ld, is greater than the maximum bytes of %d\n", total_bytes, max_message_size);
        return false;
    }

    // send data out to the server:
    if (!aes_send_raw_message(connection, method, method_len, uri, uri_len, replyto, replyto_len, p_process_name, p_process_name_len, username, username_len, (void*) body, body_len))
    {
        if (errno == EAGAIN)
        {
            FPS_ERROR_PRINT("%s: Message send queue full, Server busy.\n", program_invocation_short_name);
            return false;
        }
        FPS_ERROR_PRINT("%s: Socket error.\n", program_invocation_short_name);
        close(connection);
        connection = FIMS_CONN_CLOSED;
        return false;
    }
    return true;
}

bool fims::Send(const str_view method, const str_view uri, const str_view replyto, const str_view username, const str_view body)
{
    if (connection <= 0)
    {
        FPS_ERROR_PRINT("%s: Need to connect before calling send.\n", program_invocation_short_name);
        return false;
    }
    if (!method.data || !uri.data)
    {
        FPS_ERROR_PRINT("%s: Can't transmit message without method or uri.\n", program_invocation_short_name);
        return false;
    }

    const auto total_bytes = sizeof(Meta_Data_Info) + method.size + uri.size + replyto.size + p_process_name_len + username.size + body.size;

    // no more than 255 characters for method, uri, and replyto
    if (method.size > Meta_Data_Info::Max_Meta_Data_Str_Size 
            || uri.size > Meta_Data_Info::Max_Meta_Data_Str_Size 
                || replyto.size > Meta_Data_Info::Max_Meta_Data_Str_Size
                    || username.size > Meta_Data_Info::Max_Meta_Data_Str_Size)
    {
        FPS_ERROR_PRINT("one of method, uri, replyto, or username is more than 255 characters\n");
        return false;
    }
    if (total_bytes > max_message_size)
    {
        FPS_ERROR_PRINT("the number of total bytes: %ld, is greater than the maximum bytes of %d\n", total_bytes, max_message_size);
        return false;
    }

    // send data out to the server:
    if (!aes_send_raw_message(connection, method.data, method.size, uri.data, uri.size, replyto.data, replyto.size, p_process_name, p_process_name_len, username.data, username.size, (void*) body.data, body.size))
    {
        if (errno == EAGAIN)
        {
            FPS_ERROR_PRINT("%s: Message send queue full, Server busy.\n", program_invocation_short_name);
            return false;
        }
        FPS_ERROR_PRINT("%s: Socket error.\n", program_invocation_short_name);
        close(connection);
        connection = FIMS_CONN_CLOSED;
        return false;
    }
    return true;
}

fims_message* fims::Receive_Timeout(int useconds)
{
    if(connection <= 0)
    {
        FPS_ERROR_PRINT("%s: proc [%s] fims %p Need to connect before calling receive.\n",  program_invocation_short_name, p_process_name, (void*)this);
        return NULL;
    }
    // set socket timeout
    struct timeval tv;
    tv.tv_sec  = useconds / 1000000;
    tv.tv_usec = useconds % 1000000;
    if(-1 == setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv))
    {
        FPS_ERROR_PRINT("%s: Failed to set timeout: %s.\n", program_invocation_short_name, strerror(errno));
        return NULL;
    }
    // call Receive
    fims_message* return_val = Receive();
    // return socket to blocking
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    return return_val;
}

// NOTE(WALKER): This is for username if we don't receive one (it is "null"):
const char *getMachineName()
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
        return pw->pw_name;
    }

    return "";
}

// This will listen until a message is received on a uri process is listening on
// This will return a fim_message that needs to be deleted by calling free_message()
// when the caller is done with the message
fims_message* fims::Receive()
{
    if(connection <= 0)
    {
        FPS_ERROR_PRINT("%s: Need to connect before calling receive.\n", program_invocation_short_name);
        return NULL;
    }

    Receiver_Bufs<MAX_MESSAGE_SIZE - Meta_Data_Info::Buf_Len - sizeof(Meta_Data_Info), 0> receiver_bufs;
    auto& meta_data = receiver_bufs.meta_data;

    bool continue_receiving = true;
    bool has_timed_out = true;
    while (continue_receiving)
    {
        continue_receiving = false;
        if (!recv_raw_message(connection, receiver_bufs))
        {
            if (errno != EAGAIN)
            {
                // check if error is from timeout or socket closed on other end:
                continue_receiving = true;
                FPS_ERROR_PRINT("%s: Failed to read anything from socket errno %d.\n", program_invocation_short_name, errno);
                if (errno != EINTR)
                {
                    close(connection);
                    connection = FIMS_CONN_CLOSED;
                    return nullptr;
                }
            }
        }
        else
        {
            has_timed_out = false;
        }
    }

    if (has_timed_out) return nullptr; // we timed out on the socket (don't bother extracting out any info)

    // basic data check (did we recv more than we expected):
    if (meta_data.data_len > receiver_bufs.get_max_expected_data_len())
    {
        FPS_ERROR_PRINT("The number of bytes received (currently: %d) exceeds the expected maximum of %d. Bytes are being cut off\n", receiver_bufs.meta_data.data_len, receiver_bufs.get_max_expected_data_len());
        return nullptr;
    }

    // decryption stuff:
    void* data = nullptr;
    defer { if (data && g_aesKey) free(data); };
    data = decrypt_buf(receiver_bufs);

    fims_message *message = new fims_message();
    // method:
    message->method = strndup(receiver_bufs.get_method_data(), meta_data.method_len);
    // uri:
    message->uri = strndup(receiver_bufs.get_uri_data(), meta_data.uri_len);
    // replyto (optional):
    if (meta_data.replyto_len > 0) message->replyto = strndup(receiver_bufs.get_replyto_data(), meta_data.replyto_len);
    // process_name:
    message->process_name = strndup(receiver_bufs.get_process_name_data(), meta_data.process_name_len);
    // username (optional):
    if (meta_data.username_len > 0)
    {
        message->username = strndup(receiver_bufs.get_username_data(), meta_data.username_len);
    }
    else // set to machine name by default:
    {
        const char* machineName = getMachineName();
        message->username = strdup(machineName);
    }

    // actual data ("body" - optional):
    if (meta_data.data_len > 0)
    {
        if (!g_aesKey) // no encryption:
        {
            message->body = strndup(reinterpret_cast<const char*>(data), meta_data.data_len);
        }
        else // we have encryption -> don't need to strndup just cast data to char* (fims_message will free the malloc'd buffer)
        {
            message->body = reinterpret_cast<char*>(data);
            data = nullptr; // so defer call doesn't free the body data if we have decryption
        }
    }

    // build pfrags
    int count = 0;
    int offset[MAX_URI_DEPTH];
    for(int i = 0; message->uri[i] != '\0' && count < MAX_URI_DEPTH; i++)
    {
        if(message->uri[i] == '/')
        {
            offset[count] = i;
            count++;
        }
    }
    message->nfrags = count;
    if(count > 0 && count < MAX_URI_DEPTH)
        message->pfrags = new char*[count];
    else
    {
        FPS_ERROR_PRINT("%s: Invalid number of segments in URI", program_invocation_short_name);
    }
    for(int i = 0; i < count; i++)
    {
        message->pfrags[i] = message->uri + (offset[i] + 1);
    }

    return message;
}


void fims::Close()
{
    if(Connected())
        close(connection);
    connection = FIMS_CONN_CLOSED;
}

// Returns whether the server has any connections
bool fims::Connected() const noexcept
{
    return (connection > 0);
}

// tells the programmer if they are using encryption or not (also if they need to call decrypt_buf manually themselves if they use recv_raw_message after checking meta_data first)
bool fims::has_aes_encryption() const noexcept
{
    return g_aesKey != nullptr;
}

// This message will free a fim_message
bool fims::free_message(fims_message* message)
{
    // TODO manage memory better
    if(message == NULL)
        return false;
    delete(message);
    return true;
}

int fims::get_socket() const noexcept
{
    return connection;
}
