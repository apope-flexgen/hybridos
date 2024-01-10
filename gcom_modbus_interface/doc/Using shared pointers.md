Using shared pointers
p. wishire
01_09_2023

Below is an example demonstrating the use of `std::weak_ptr` on a structure. In this example, we have a structure named `Data` that holds an integer value. We create a `std::shared_ptr` to manage an instance of `Data` and then create a `std::weak_ptr` that observes the `Data` instance. The example demonstrates how to safely access the `Data` instance through the `std::weak_ptr` even if the `std::shared_ptr` may have been reset or destroyed.

```cpp
#include <iostream>
#include <memory>

struct Data {
    int value;

    Data(int v) : value(v) { 
        std::cout << "Data constructed with value: " << value << std::endl;
    }

    ~Data() { 
        std::cout << "Data destroyed with value: " << value << std::endl;
    }
};

int main() {
    std::shared_ptr<Data> sharedPtr = std::make_shared<Data>(100);
    std::weak_ptr<Data> weakPtr = sharedPtr; // weakPtr observing sharedPtr

    // Accessing Data through weak_ptr
    if (auto tempSharedPtr = weakPtr.lock()) { // lock() returns shared_ptr
        std::cout << "Shared pointer is still valid. Data value: " << tempSharedPtr->value << std::endl;
    } else {
        std::cout << "Shared pointer has been reset or destroyed." << std::endl;
    }

    // Resetting shared_ptr, Data is destroyed as reference count drops to zero
    sharedPtr.reset();

    // Attempt to access Data through weak_ptr after shared_ptr has been reset
    if (auto tempSharedPtr = weakPtr.lock()) { // Attempt to lock and obtain shared_ptr
        std::cout << "Data value: " << tempSharedPtr->value << std::endl;
    } else {
        std::cout << "Shared pointer has been reset or destroyed. Cannot access Data." << std::endl;
    }

    return 0;
}
```

### Expected Output:
```
Data constructed with value: 100
Shared pointer is still valid. Data value: 100
Data destroyed with value: 100
Shared pointer has been reset or destroyed. Cannot access Data.
```

### Explanation:
1. A `std::shared_ptr<Data>` is created, managing a `Data` instance with the value `100`.
2. A `std::weak_ptr<Data>` is created, observing the `Data` instance.
3. We check if the `weak_ptr` can be locked (i.e., if the `Data` instance is still alive) and access the `Data` successfully.
4. The `shared_ptr` is reset, destroying the `Data` instance because its reference count drops to zero.
5. We attempt to lock the `weak_ptr` again, but this time it fails because the `Data` instance has been destroyed. The program safely reports that the `Data` cannot be accessed.



A `std::unique_ptr` is a smart pointer that owns and manages another object through a pointer and disposes of that object when the `unique_ptr` goes out of scope. A `unique_ptr` is not copyable, and ownership can only be transferred through move semantics.

Below is an example illustrating the use of `std::unique_ptr` with the `Data` structure:

```cpp
#include <iostream>
#include <memory>

struct Data {
    int value;

    Data(int v) : value(v) { 
        std::cout << "Data constructed with value: " << value << std::endl;
    }

    ~Data() { 
        std::cout << "Data destroyed with value: " << value << std::endl;
    }
};

int main() {
    std::unique_ptr<Data> uniquePtr = std::make_unique<Data>(100); // uniquePtr owns Data

    // Accessing Data through unique_ptr
    std::cout << "Data value: " << uniquePtr->value << std::endl;

    // Transfer ownership of Data to another unique_ptr
    std::unique_ptr<Data> anotherUniquePtr = std::move(uniquePtr); // Ownership transferred

    // uniquePtr no longer owns Data, anotherUniquePtr now owns Data
    if (uniquePtr) {
        std::cout << "uniquePtr still owns Data." << std::endl;
    } else {
        std::cout << "uniquePtr no longer owns Data." << std::endl;
    }

    // Data is destroyed when anotherUniquePtr goes out of scope at the end of the program
    return 0;
}
```

### Expected Output:
```
Data constructed with value: 100
Data value: 100
uniquePtr no longer owns Data.
Data destroyed with value: 100
```

### Explanation:
1. A `std::unique_ptr<Data>` is created, owning a `Data` instance with the value `100`.
2. We access the `Data` value through `uniquePtr`.
3. Ownership of the `Data` instance is transferred from `uniquePtr` to `anotherUniquePtr` using `std::move`.
4. We check and confirm that `uniquePtr` no longer owns the `Data` instance.
5. The `Data` instance is automatically destroyed when `anotherUniquePtr` goes out of scope at the end of the program, calling the destructor of `Data`.