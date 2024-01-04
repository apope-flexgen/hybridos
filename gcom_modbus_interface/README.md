# Trouble installing?:
- everything is in C++17 so make sure to get an appropriate compiler for it.
- Install "devtoolset-9" through yum to get access to the gcc 9 compiler for C++17.

# Structure:

## Shared
- decode_encode_utils.hpp
    - This is a common header file that both the server and the client use for encoding and decoding raw registers over modbus
    - things like "scale", "shift", "size", etc. are in these functions
    - This file also includes the "runtime" structs that are used in both the client and the server
- Jval_buif.hpp
    - "tagged union" of types:
        - bool
        - uin64_t (u64)
        - int64_t (s64)
        - double  (f64)
    - used in both the server and the client for encoding and decoding purposes
    - helps to save space (16 byte struct) and keep track of the types of things after decoding
    - also used when outputting to fims (uses fmt formatters)
- decode_config_loader.hpp
    - part of the "config_loaders" namespace
    - this is a config loader struct used by both the client and the server
    - This helps to reduce code dupe when "parsing" a config on either the client or the server
    - This is because both server and client have "registers" arrays which share this common object for encoding and decoding
    - hence the "utils" file as well (so common parsing and functions)
- error_location.hpp
    - part of the "config_loaders" namespace
    - both client and server will tell you where in the config you went wrong and why
    - helpful for error checking as we "parse" the configs
    - both client and server have their own print out versions of this based on their config structure
    - should tell you array indices, last known ids, etc. so one can follow along in the config to find the point of failure
- simple_arena.hpp
    - A single time malloc byte array (using void* for data)
    - you "allocate" bytes at a time or multiple bytes using T*'s and "amount" (if the pointer points to the beginning of an array)
    - based on the type the correct amount of bytes are allocated and if more than one is needed it allocates that many
    - all of the "runtime" structs for the client use this arena (along with "string storage")
    - always "constructs" the type at the pointer as needed (calls emplace new operator to gaurantee garbage isn't viewed)
- string_storage_v2.hpp
    - This is simple the number of bytes allocated and a char*
    - This is used to store all the strings that the client and server need during runtime
    - This is helpful because all of the config information is constant, meaning you can store in a byte array and access it later (no heap allocation necessary)
    - This also helps to remove "duplicate" strings found in the config by having them point to the same string in storage
    - Instead of using const char* and a size, a "handle" is used (only 4 bytes) which uses indices to access the string in storage
    - This is known as "lazy evaluation" and saves a massive amount of space (4 bytes vs 16 bytes)
- shared_utils.hpp
    - basic aliases: like u8 for uint8_t, s32 for int, f64 for double, etc.
    - basic enums: like Holding, Input, etc. (register types)
## Client
- client_structs.hpp
    - These are the "runtime" structs for modbus_client.
    - client has "Main_Workspace" which contains:
        - the "arena"
        - "string storage"
        - a TSCNS clock (used as a replacement for "monotonic clock" from the OS) -> much more efficient at runtime and shared amongst all threads everywhere
        - 
    - This contains the structs which are meant to be allocated in the "arena"
    - It has functions to take in the "client configs" structs and allocate the necessary bytes
    - it then assigns off the "pointers" in the "main_workspace"

## Server
- WIP (to come in the future, nothing for now)
