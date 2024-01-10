SmartPointers
p. wilshire
09_21_2023



### Example IO_Work strucure
class IO_Work {
    public:
    Register_Types reg_type;
    int device_id;
    int offset;
    int num_registers;

    int errno_code;
    int errors;
    double response;
    doiuble tNow;

    u16 u16_buff[MODBUS_MAX_READ_REGISTERS];  // receive into this (holding and input)
    u8  u8_buff[MODBUS_MAX_READ_REGISTERS];   // receive into this (coil and discrete input)
    
    IO_Work() noexcept {
        memset(u16_buff, 0, sizeof (u16_buff));
        memset(u8_buff, 0, sizeof (u8_buff));
    };

    clear_bufs() noexcept {
        memset(u16_buff, 0, sizeof (u16_buff));
        memset(u8_buff, 0, sizeof (u8_buff));
    };

    IO_Work(IO_Work&& other) noexcept  {};

    IO_Work& operator=(IO_Work&& other) noexcept {
        return *this;
    };
};



The io_work structure is uses to tell the IOThreads what to do.


Each `io_work` should be unique and have its own data.
A pool of unused `io_work`  structures is maintained by the system but when that pool is empty new structures will be creates and used.

A special construct `std::unique_ptr` creates a smart pointer to th io_work object:



```cpp
Channel<std::unique_ptr<IO_Work>> pollChan;      // Use Channel to send IO-Work to thread
Channel<std::unique_ptr<IO_Work>> responseChan;   // Thread picks uo IO_work and processe it
Channel<std::unique_ptr<IO_Work>> poolChan;       // response channel returns io_work to the pool
book queue_work(RegisterTypes reg_type, device_id, offset, num_regs) {


    std::unique_ptr<IO_Work> io_work;
    if(!poolChan.receive(io_work,false)) {
        io_work = std::make_unique<IO_Work>();
    }
    // Modify io_work data if necessary here
    io_work->tNow = get_time_double();
    io_work->device_id = device_id;
    io_work->reg_type = reg_type
    io_work->offset = offset; 
    io_work->num_registers = num_regs; 

    io_work->clear_bufs();
    pollChan.send(std::move(io_work));
}
```

This way, each `io_work` is unique, and you don't need to worry about manual memory management since `std::unique_ptr` will handle it. 
When the receiving end is done with an `IO_Work`, the `std::unique_ptr` will automatically delete it or push it onto a PoolChan.
We're allowing C++'s smart pointers to manage the memory for you.

Here's a breakdown of what happens:

1. Try to recover an io_work object from the pool of used items.
    if needed  use `std::make_unique<IO_Work>()` create a new instance of `IO_Work` and returns a `std::unique_ptr` that owns this new instance.

2. You can then modify the data within `io_work` (as you did with `io_work->offset = offset;`).

3. When you send `io_work` to the `pollChan` using `std::move(io_work)`, you're transferring ownership of the `IO_Work` instance to the channel. 
   After this line, `io_work` no longer owns the `IO_Work` instance, and its value becomes null.

4. Inside the channel, the `std::unique_ptr` will continue to manage the lifetime of the `IO_Work` instance. When the channel or whatever mechanism consumes the messages from the channel is done with the `IO_Work` and the corresponding `std::unique_ptr` goes out of scope or is overwritten, it will automatically delete the `IO_Work` instance, freeing the memory.

Using `std::unique_ptr` this way ensures that the memory is managed automatically, and you don't have to call `delete` manually. It's a safer approach that helps prevent memory leaks and other memory-related issues.
When the receiving process retrieves the `std::unique_ptr<IO_Work>` from the channel, it will now own that pointer and the associated memory. When that `std::unique_ptr` goes out of scope or is explicitly reset or replaced in the receiving process, the memory for the `IO_Work` instance will be automatically deallocated.


```cpp
std::unique_ptr<IO_Work> received_work;

while (true) {
    if (pollChan.receive(received_work)) {
        // Do something with received_work
        // ...

        // When this loop iteration ends, or if received_work is assigned a new value,
        // the memory for the previous IO_Work will be automatically freed.
        responseChan.send(std::move(received_work));

    }
}

while (true) {
    if (response.receive(received_work)) {
        // Do something with received_work
        // ...

        // When this loop iteration ends, or if received_work is assigned a new value,
        // the memory for the previous IO_Work will be automatically freed.
        poolChan.send(std::move(received_work));

    }
}

```


As long as the received data is  handled and stored  as `std::unique_ptr<IO_Work>`, the memory management is taken care of automatically. No manual `delete` is necessary, and the risk of memory leaks is minimized. This is one of the key benefits of using smart pointers in C++.


This design  employs a typical pattern found in high-performance systems where you reuse objects to avoid the overhead of frequent allocations and deallocations. This pattern, commonly known as Object Pooling, can be especially useful in systems where performance is crucial.


```cpp
Channel<std::unique_ptr<IO_Work>> pollChan;      // Use Channel to send IO-Work to thread
Channel<std::unique_ptr<IO_Work>> responseChan;  // Thread picks up IO_work and processes it
Channel<std::unique_ptr<IO_Work>> poolChan;      // Response channel returns io_work to the pool
Channel<int> threadChan;                         // Thread Control 

bool queue_work(RegisterTypes reg_type, device_id, offset, num_regs) {
    std::unique_ptr<IO_Work> io_work;
    if (!poolChan.receive(io_work)) {  // Assuming receive will return false if no item is available.
        io_work = std::make_unique<IO_Work>();
    }

    // Modify io_work data if necessary here
    io_work->tNow = get_time_double();
    io_work->device_id = device_id;
    io_work->reg_type = reg_type;
    io_work->offset = offset; 
    io_work->num_registers = num_regs; 

    io_work->clear_bufs();
    pollChan.send(std::move(io_work));
    threadChan.send(1);

    return true;  // You might want to return a status indicating success or failure.
}

// io_thread
std::unique_ptr<IO_Work> io_received_work;
int signal;
double delay;
bool run = true;
delay = 1,0; // delay in seconds
while(true) {
    if  (threadChan.receive(signal,delay))
    {
        if(signal == 0) run = false;
        if(signal ==1 ) {

            if (pollChan.receive(io_received_work)) {
                // Do something with io_received_work
                // ...

                responseChan.send(std::move(io_received_work));
            }
        }
    }
}

// response thread
std::unique_ptr<IO_Work> response_received_work;
while (true) {
    if (responseChan.receive(response_received_work)) {
        // Do something with response_received_work
        // ...

        poolChan.send(std::move(response_received_work));
    }
}
```

Your code flow suggests:

1. `queue_work` tries to get an `IO_Work` from the pool.
2. If the pool is empty, it creates a new `IO_Work`.
3. The `IO_Work` is then processed in the `io_thread`.
4. Once processed, it is sent to the `response thread`.
5. After the response processing, the `IO_Work` is returned to the pool for reuse.

This approach looks good for reusing `IO_Work` objects and minimizing dynamic allocations. As long as you always handle `IO_Work` using `std::unique_ptr`, the memory management will be automatic and safe.


