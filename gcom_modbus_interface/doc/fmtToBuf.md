The `fmt::memory_buffer` in the [fmt library](https://fmt.dev/latest/index.html) is a buffer with a fast in-memory formatting API. You can use it similarly to how you would use a `std::ostringstream` but with the `fmt::format_to` function, which writes formatted data directly to the buffer.

Below is an example that demonstrates how you might use the `fmt::memory_buffer` in your function. This example is quite general, as I don't have the exact details of your `cfg::map_struct` and `cfg` structures, or how you want to format the data:

```cpp
#include <fmt/core.h>
#include <fmt/format.h>

using u16 = uint16_t;
using u8 = uint8_t;

namespace cfg {

struct map_struct {
    // Example fields, you need to replace these with your actual fields
    int field1;
    double field2;
};

} // namespace cfg

void decode_to_string(u16* regs16, u8* regs8, struct cfg::map_struct& item, fmt::memory_buffer& buf, struct cfg& myCfg) {
    // Here you'll need to decode the data in regs16, regs8, item, and myCfg as needed
    // This is just an example, adjust accordingly to your actual decoding logic

    // Example of how to write to buf using fmt::format_to
    fmt::format_to(buf, "field1: {}\n", item.field1);  // Replace with your actual field
    fmt::format_to(buf, "field2: {}\n", item.field2);  // Replace with your actual field
    
    // Add formatted data for regs16 and regs8 as necessary
    // For example:
    fmt::format_to(buf, "regs16[0]: {}\n", regs16[0]);  // Adjust index as needed
    fmt::format_to(buf, "regs8[0]: {}\n", regs8[0]);    // Adjust index as needed
    
    // Add formatted data for myCfg as necessary
    // For example (assuming myCfg has a field named config_field):
    // fmt::format_to(buf, "myCfg.config_field: {}\n", myCfg.config_field);  // Adjust field name as needed

    // After populating buf, you can convert it to a string if needed
    std::string result = fmt::to_string(buf);

    // Then you can output or use result as needed
    // std::cout << result;
}

int main() {
    // Example usage, replace with your actual data
    u16 regs16[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    u8 regs8[10] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    cfg::map_struct item {42, 3.14};  // Replace with your actual initialization
    struct cfg myCfg;                 // Replace with your actual initialization
    fmt::memory_buffer buf;

    decode_to_string(regs16, regs8, item, buf, myCfg);
    std::cout << fmt::to_string(buf) << std::endl;  // Output the formatted string
}
```

In this example:

1. The `fmt::format_to` function is used to write formatted data to the `fmt::memory_buffer`.
2. You'll need to replace the `cfg::map_struct` and `cfg` structures and their fields with your actual structures and fields.
3. Adjust the `decode_to_string` function to use the correct indexes and fields for `regs16`, `regs8`, `item`, and `myCfg` based on your actual data and formatting requirements.

Remember to link against the fmt library and include the appropriate header files. The exact steps might vary depending on your build system and the version of the fmt library you are using. 
Ensure you have the fmt library installed and configured correctly in your project.
In this situation, you would need to modify the `cfg::map_struct` to have an `std::any` field, and then handle it appropriately within the `decode_to_string` function. Handling `std::any` can be tricky because it can hold any type, so you need to be sure of what types you expect and handle them accordingly.

Below is an example:

Firstly, update the `cfg::map_struct`:

```cpp
#include <any>

namespace cfg {

struct map_struct {
    int field1;
    double field2;
    std::any field3;
};

} // namespace cfg
```

Next, modify the `decode_to_string` function to handle the `std::any` field:

```cpp
#include <fmt/core.h>
#include <fmt/format.h>
#include <any>

using u16 = uint16_t;
using u8 = uint8_t;

// ... (cfg::map_struct definition) ...

void decode_to_string(u16* regs16, u8* regs8, struct cfg::map_struct& item, fmt::memory_buffer& buf, struct cfg& myCfg) {
    // ... (Other code) ...

    fmt::format_to(buf, "field1: {}\n", item.field1);
    fmt::format_to(buf, "field2: {}\n", item.field2);
    
    // Handling field3 (std::any type)
    try {
        // Try casting to int and formatting
        fmt::format_to(buf, "field3 (int): {}\n", std::any_cast<int>(item.field3));
    } catch(const std::bad_any_cast&) {
        try {
            // Try casting to double and formatting
            fmt::format_to(buf, "field3 (double): {}\n", std::any_cast<double>(item.field3));
        } catch(const std::bad_any_cast&) {
            // Handle other expected types here
            // ...
            // If none of the expected types, write an error or default message
            fmt::format_to(buf, "field3: unknown or unsupported type\n");
        }
    }
    
    // ... (Other code) ...
}
```

In this example:

1. `std::any` is used to hold a field of any type in `cfg::map_struct`.
2. Within `decode_to_string`, attempts are made to cast the `std::any` to expected types (int and double in this example), and if the cast is successful, the value is formatted to the buffer.
3. If none of the casts succeed, a default or error message is written to the buffer.

Adjust the casting and formatting code to handle the actual types you expect to store in the `std::any` field. You can add more `try`/`catch` blocks or use another mechanism to handle different types.