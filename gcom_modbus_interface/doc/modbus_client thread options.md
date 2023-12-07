modbus_client thread options
p. wilshire 
09_11_2023


The modbus target io operation has been refactored as a  standalone task.

This task is given an IO_Work structure to perform throuigh either  a set_work queue or a pub_work queue.

The thread is given a modbus context that cotains the connection information.
In addition the thread is given, using the IO_Work object, a device_id, a register type, a staring offset and a number of registers.
The thread's task is to execute a modbus_read or write on the registers determined in the IO_Work structure and collect the data into either  a byte buffer or a bit buffer ( these may be combied as the same buffer.)
having completed the I/O operation the task returns the IO_Work object to a reply queue for the main processing thread to digest.
The IO_Work object also contains the timing, error codes and the data buffer.


The current (test) system can have many threads all waiting for IO_Work objects from a common queue.
It is a MPMC queue so any thread can grab an IO_Work object and proceed to talk to the selected registers in a specific device_id in the modbus server

Some constraints may be needed to make a "real" system work.

Modbus Context. 
    - this is actually a connection between the client and server.
If the server can only have one connection that this, by nature, limits the number of threads we can have.  

The thead options are :-
1/ limit the max_number_of_connections ( or threads) but keep the multithreaded operation to allow any thread to service and IO_Work item.

2/ tie the  threads to specific device_ids.
    probably we would then have a set of set / pup queues for each device_id and have threads monitor their own specific queues.

3/ tie the threads to the component structures.
    one or more threads tied to each structure.
    same arrangement as (2) with specific set / pub queues for each component.
    In this case the incoming component contained in the fims message will dictate which ioqueues the IO_Work structure is sent to.


With options 2 or 3 we can still have more than one therad per device_id or component as permitted by the target hardware.



