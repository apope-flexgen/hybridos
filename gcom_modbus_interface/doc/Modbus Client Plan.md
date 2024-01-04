# Modbus Client Improvement Plan

**Author**: P. Wilshire  
**Date**: 09-02-2023

## Current Situation

- Non-modular.
- Overly complex.
- Excessive optimization.
- Incorrect error handling.
- Not fault-tolerant.
- Incomprehensible code layout.
- Not testable in a modular format.
- Poor logging.

The workspace system is currently not required. It may optimize the code, but the major bottleneck is the Modbus server we are communicating with. We read into an awkward layout config, then translate the config into a workspace, and then use obscure references into the workspace structure to run the system.

## Goals

1. **Read Config into Base Structure**: Keep the "as read" config. Don't throw it away and be left with just the workspace. Ideally, remove the workspace in phase 1.
   
2. **Extract From Config**:
   
   2.1. Given a single FIMS object, get the device id, type, size, and offset from the config.
   
   2.1.1. Given the type, size, offset, get the value from the decode buffer.
   
   2.1.2. Given the type, size, offset, and a new value, put the value into the decode buffer.
   
   2.1.2.1. Add all the bit/enum/scale stuff as dictated by the decode flags.
   
   2.2. Given a component, get a list of maps associated with that component. Each map will have a device_id (inherited from the component definition), type, offset, and number of registers ready for a Modbus read/write.

3. **Data Transfer from Modbus to Decode Data**:
   
   3.1. Given a decode buffer (as provided by a Modbus read operation) and the type and offset, num items data, extract the values, ids, etc., ready to send to FIMS.

4. **GCom Timer System**: Create a GCom timer system (code already exists) to provide pub, heartbeat, and watchdog timing (almost like the TMW system).
   
   4.1. Use the sync timer and set up a config item to enable the sync operation (delay the next pub cycle because the previous one took too long).

5. **Isolate Threads**: Isolate the FIMS_listen, IOthreads, and main process threads each into their own files.

6. **IOThread**:

   6.1. Define the clean interface to the IOthread.

   6.1.1. Give the thread a package containing the time_of_request, Modbus context, device_id, type, offset, num_items, and a decode buffer. Trigger the thread to run in input (read) or output (write) mode. When done, the thread hands the package back to the main processor.

   6.1.1.1. Handle errors properly: reconnect on timeout, try again if busy, give up if it's gone.

   6.1.2. IOthreads can have set requests and pub requests. Prioritize sets over pubs; we'll have the time_of request in place but will not use it yet.

   6.1.2.1. IOThreads will use MPMC queues. Provide test code to make sure this works.

   6.2. Get the IOthread to report status back to the main thread, including the modified decode_buffer in its package (pub_work).

7. **Main Thread**:

   7.1. Get the main thread to produce FIMS messages (either set/get replies or pub outputs). Also, add error reports using the GCom logger.

8. **FIMS Thread**:

   8.1. Decode FIMS_input, extract device_id, type, offset, size from the config.

   8.1.1. Prepare IOthread package or package for the main thread. Use the tools to access the config and work with the decode buffer to input or extract data from the buffer.

   8.2. Add options to get raw data or even query abstract device_id, type, offset, size. This means we bypass the config lookup and work with the data from the FIMS message. This concept is also a low-level test for the IOthread package. If the main thread cannot find the data in the config, there is no need to place the results in the config data; just report back, as directed, using the main thread to FIMS and drop the data.

9. **To be continued**:

