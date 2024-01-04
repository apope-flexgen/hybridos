The Jconfig SON structure has a mix of nested objects, arrays, and primitive types. Based on your JSON structure, your `jsonToMap` and `jsonToAny` functions should be able to handle and convert the JSON into a `std::map<std::string, std::any>`.

Here's a brief overview of what the code does and how it aligns with your JSON structure:

1. The `fileInfo` and `connection` parts of the JSON are simple nested objects that get processed recursively by your code. 

2. The `components` part is an array of objects. Each object inside this array can contain further nested objects (`registers`) and arrays (`map`).

3. Primitive types like strings, numbers, and booleans are stored as `std::any`, allowing for a dynamic and varied container.

Given the recursive nature of your conversion functions, you should be able to represent the entire JSON structure within the `std::map<std::string, std::any>`. When you extract values from the map, you'll need to use `std::any_cast` to safely retrieve the underlying value.

However, a few things to be aware of:

- **Complex Retrieval**: While `std::any` is powerful, remember that every time you want to access a nested structure, you'll need to cast from `std::any` to the appropriate type. This can lead to more verbose and potentially error-prone code when accessing deeply nested structures.

- **Type Safety**: Using `std::any` means you lose some type safety. When you retrieve a value from the map, you have to know its type or handle potential type errors.

- **Memory Overhead**: While not usually an issue, the use of `std::any` introduces some memory overhead.

If you're just using this approach for configuration or one-off parsing, it should be perfectly fine. If you're doing high-performance tasks or dealing with a huge number of objects, you might want to consider a more specific structure or data type.

You might also want to consider creating specific data structures (e.g., structs or classes) that map more directly to your expected JSON format, which would give you type safety and more intuitive access patterns, but at the cost of flexibility.

That said, based on the provided functions and the JSON structure, it seems like your approach should work. If you're running into specific issues or have further questions, feel free to ask!