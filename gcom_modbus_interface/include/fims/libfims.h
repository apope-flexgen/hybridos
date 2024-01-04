/*
 * libfims.h
 *
 *  Created on: Jun 12, 2018
 *      Author: jcalcagni
 */

#ifndef LIBFIMS_H_
#define LIBFIMS_H_

 /* OS Includes */
 /* C Standard Library Dependencies */
 /* C++ Standard Library Dependencies */
#include <cstdlib>
#include <stdint.h>
#include <limits>
#include <vector>
#include <string>

/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "fims.h"

#define MAX_URI_DEPTH 64
#define FIVE_SECONDS 5000000
// #define LEN_SIZE sizeof(uint16_t)

struct Meta_Data_Info
{
    // constants:
    static constexpr auto Max_Meta_Data_Str_Size = std::numeric_limits<uint8_t>::max();
    static constexpr uint16_t Buf_Len = Max_Meta_Data_Str_Size * 5;

    // first X bytes of the data buffer for unencrypted meta_data info:
    uint8_t method_len = 0;
    uint8_t uri_len = 0;
    uint8_t replyto_len = 0;
    uint8_t process_name_len = 0;
    uint8_t username_len = 0;
    // last X bytes of the data buffer for receiving (we encrypt/decrypt this part of the data buffer):
    uint32_t data_len = 0;

    // helper functions:
    std::size_t get_meta_bytes() const noexcept;
    // This is used by server only (for resending purposes):
    std::size_t get_total_bytes() const noexcept;
};

class fims_message
{
public:
    fims_message();
    ~fims_message();

    char* method;
    char* uri;
    char* replyto;
    char* body;
    char* process_name;
    char* username;
    char** pfrags;
    unsigned int nfrags;
};

class fims
{
public:
    // helper struct for new send function overload:
    // reduces strlen calls (for example the programmer might have an std::string with size info attached to the const char* or an std::string_view)
    struct str_view
    {
        const char* data;
        std::size_t size;
    };

    fims();
    ~fims();
    // these functions are not thread safe
    bool Connect(const char* process_name = nullptr);
    void Close();
    void initAes();
    bool Subscribe(const char** uri_array, int count, bool* pub_array = nullptr);
    bool Subscribe(const std::vector<std::string>& sub_array, bool pub_only = false);
    // these messages are thread safe
    bool Send(const char* method, const char* uri, const char* replyto, const char* body, const char* username = nullptr);
    bool Send(const str_view method, const str_view uri, const str_view replyto, const str_view username, const str_view body);
    fims_message* Receive();
    fims_message* Receive_Timeout(int useconds);
    static bool free_message(fims_message* message);
    bool Connected() const noexcept;
    bool has_aes_encryption() const noexcept;
    int  get_socket() const noexcept;
    uint32_t get_max_message_size() const noexcept;
protected:
    decltype(Meta_Data_Info::process_name_len) p_process_name_len;
    uint32_t max_message_size;
    int connection;
    char* p_process_name;
};

// data is received into these buffers:
template<uint32_t Max_Expected_Data_Len, uint8_t Padding_Bytes>
struct Receiver_Bufs
{
    Meta_Data_Info meta_data;
    // first X bytes of this data_buf contain the meta_data bytes. Last X bytes contain the encrypted/unencrypted "body"/data (the actual message data)
    uint8_t data_buf[Meta_Data_Info::Buf_Len + Max_Expected_Data_Len + Padding_Bytes];

    // helper functions:
    const char* get_method_data() const noexcept
    {
        return reinterpret_cast<const char*>(data_buf);
    }
    const char* get_uri_data() const noexcept
    {
        return get_method_data() + meta_data.method_len;
    }
    const char* get_replyto_data() const noexcept
    {
        return get_uri_data() + meta_data.uri_len;
    }
    const char* get_process_name_data() const noexcept
    {
        return get_replyto_data() + meta_data.replyto_len;
    }
    const char* get_username_data() const noexcept
    {
        return get_process_name_data() + meta_data.process_name_len;
    }
    fims::str_view get_method_view() const noexcept
    {
        return fims::str_view{get_method_data(), meta_data.method_len};
    }
    fims::str_view get_uri_view() const noexcept
    {
        return fims::str_view{get_uri_data(), meta_data.uri_len};
    }
    fims::str_view get_replyto_view() const noexcept
    {
        return fims::str_view{get_replyto_data(), meta_data.replyto_len};
    }
    fims::str_view get_process_name_view() const noexcept
    {
        return fims::str_view{get_process_name_data(), meta_data.process_name_len};
    }
    fims::str_view get_username_view() const noexcept
    {
        return fims::str_view{get_username_data(), meta_data.username_len};
    }
    void* get_message_data() noexcept
    {
        return data_buf + meta_data.get_meta_bytes();
    }
    constexpr uint32_t get_max_expected_data_len() const noexcept
    {
        return Max_Expected_Data_Len;
    }
};

// receiver helper functions:
bool  recv_raw_message(int connection, Meta_Data_Info& meta_data, void* data_buf, uint32_t data_buf_len) noexcept;
void* decrypt_buf(Meta_Data_Info& meta_data, void* data_buf, uint32_t data_buf_len) noexcept;

// This receives the raw message without decryption:
template<uint32_t Max_Expected_Data_Len, uint8_t Padding_Bytes>
bool recv_raw_message(int connection, Receiver_Bufs<Max_Expected_Data_Len, Padding_Bytes>& receiver_bufs) noexcept
{
    return recv_raw_message(connection, receiver_bufs.meta_data, receiver_bufs.data_buf, sizeof(receiver_bufs.data_buf) - Padding_Bytes);
}

// this decrypts the buffer (you can check meta_data information before decryption for extra performance -> also no strndups):
template<uint32_t Max_Expected_Data_Len, uint8_t Padding_Bytes>
void* decrypt_buf(Receiver_Bufs<Max_Expected_Data_Len, Padding_Bytes>& receiver_bufs) noexcept
{
    return decrypt_buf(receiver_bufs.meta_data, receiver_bufs.data_buf, sizeof(receiver_bufs.data_buf) - Padding_Bytes);
}

#endif /* LIBFIMS_H_ */
