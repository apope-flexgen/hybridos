#include <cstdint>

#include "fims/libfims.h"
#include "fims/defer.hpp"
#include "simdjson_noexcept.hpp"
#include "spdlog/fmt/fmt.h"


// aliases
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using f32 = float;
using f64 = double;


#define CONST static constexpr
CONST u32 MAX_MESSAGE_SIZE = 10000;


int main() {
    Receiver_Bufs<MAX_MESSAGE_SIZE, 16> receiver_bufs;
    auto& meta_data = receiver_bufs.meta_data;
    fims f;
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;


    if (!f.Connect("test")) {
        return EXIT_FAILURE;
    }
    f.Subscribe({"/components"});



    while (true) {
        if (!recv_raw_message(f.get_socket(), receiver_bufs)) {
            const auto curr_errno = errno;
            if (curr_errno == EAGAIN || curr_errno == EWOULDBLOCK) continue; // we just timed out
            // This is if we have a legitimate error (no timeout):
            return EXIT_FAILURE;
        }
        
        // meta data views:
        const auto method_view       = std::string_view{receiver_bufs.get_method_data(), meta_data.method_len};
        auto uri_view                = std::string_view{receiver_bufs.get_uri_data(), meta_data.uri_len};
        const auto replyto_view      = std::string_view{receiver_bufs.get_replyto_data(), meta_data.replyto_len};
        const auto process_name_view = std::string_view{receiver_bufs.get_process_name_data(), meta_data.process_name_len};

        // if (meta_data.data_len > receiver_bufs.get_max_expected_data_len()) {
        //     continue;
        // }

        // void* data = nullptr;
        // defer { if (data && f.has_aes_encryption()) free(data); };
        // data = decrypt_buf(receiver_bufs);

        // if (!data) {
        //     continue;
        // }

        // if (!f.has_aes_encryption()) {
        //     memset(reinterpret_cast<u8*>(data) + meta_data.data_len, '\0', Simdj_Padding);
        // }

        // // set success_str for replyto defer:
        // const auto success_str = std::string_view{reinterpret_cast<const char*>(data), meta_data.data_len};


        // if (const auto err = parser.iterate(reinterpret_cast<const char*>(data), meta_data.data_len, meta_data.data_len + Simdj_Padding).get(doc); err) {
        //     continue;
        // }

        // for (auto pair : doc.get_object()) {
        //     fmt::print("key: {}\n", pair.unescaped_key().value_unsafe());
        // }


        fmt::print("Method: {}\nURI: {}\nReplyTo: {}\nProcessName: {}\n", method_view, uri_view, replyto_view, process_name_view);
    }
}