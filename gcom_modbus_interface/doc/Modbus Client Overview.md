## Modbus Client Data Path Design Document
p. wilshire 
09_15_2023

### Note
This document discusses a proposal for a general data flow through the modbus client.
The specifics of the register encoding and decoding  will be identical to the original modbus system with all later enhancements.
THis document focusses on the way the system handles timed pub and on demand set requests.
At this stage no batching or filtering of the data is proposed.   

### **1. Configuration Layout**

#### **1.1 Overview**

The configuration is set through a file that defines an array of "components". Each of these components has specific attributes and associations that facilitate communication with the Modbus server.

#### **1.2 Component Attributes**

- **ID**: A unique identifier for the component.
- **DeviceID**: Optional device identifier.
- **Heartbeat**: Optional heartbeat mechanism to check the component's health.
- **Watchdog**: Optional mechanism to monitor and possibly recover from malfunctions.
- **Polling Frequency**: Rate at which the Modbus server is polled.
- **Offset Time**: Time offset for the polling.

#### **1.3 Registers**

Each component contains an array of "registers" which are individual data points or a set of data points. Each register has:

- **Starting Offset**: Position of the register.
- **Number of Registers**: Defines how many data points are in the register.
- **Data Size**: Some registers might have extended sizes, e.g., 2 or 4 words.
- **Limit**: The number of registers cannot exceed 125, typical for Modbus.
  
#### **1.4 Mapping Structures**

Once the configuration is loaded into the system, mapping structures are created for fast data retrieval. Key use-cases for these mappings include:

- Finding a data item by URI and ID.
- Locating a data item by device_id, register type, and offset.
- Generating a list of URIs to subscribe to.
- Organizing a list of publish requests by time and offset.

### **2. Client Publications (Pubs)**

#### **2.1 Overview**

The client is responsible for periodically retrieving `Input Registers` and `Holding Registers` from the server, decoding this data, and then publishing it according to configuration rules.

#### **2.2 Publication Request Process**

1. **Data Collection**: Gather device_ids, register types, starting offsets, and the number of registers.
2. **Request Creation**: Construct a request object.
3. **Timestamping**: Attach the current system time as an ID to the publication request.
4. **Sequence Numbering**: Assign a sequence number to each request object.
5. **IO Thread Processing**: Process the request, read data into a buffer within the request object.
6. **Publishing Queue**: After reading, or on error, the request object is added to a publication response queue.

#### **2.3 Main Thread Processing**

1. **Data Retrieval**: The main thread collects sets of request objects.
2. **Data Decoding**: Raw data is decoded and translated into an output format using configuration rules.
3. **Data Merging**: Data from various requests is combined into a merging map.
4. **Publication**: The map is serialized and published.

### **3. Client Set Requests**

#### **3.1 Overview**

Data from subscribed URIs is collected and processed to facilitate interactions with the Modbus server.

#### **3.2 Set Request Process**

1. **Data Collection**: Retrieve input from the subscribed URIs.
2. **Data Matching**: Match input data against the configuration map.
3. **Request Creation**: Construct data requests based on matches, sorted by device ids, register types, and offsets.
4. **Data Encoding**: Translate FIMS input data into Modbus register format.
5. **Binning**: Optimize requests by combining adjacent offsets of the same type into a request while respecting the Modbus limit.
6. **IO Thread Queueing**: Bin requests are placed in the IO thread queue.
7. **Error Handling**: Errors are reported back to the main thread.

#### **3.3 FIMS Reply Mechanisms**

1. **Immediate Reply**: Send a response as soon as the FIMS set request is received and queued in the IO thread.
2. **Post-Processing Reply**: Wait for the IO thread's response, then reply with potential error codes.

### **4. Ignored Requests**

Any set requests for data items not present in the configuration will be ignored.

### **5. Conclusion**

This document provides a structured design for the Modbus Client Data Path. The design takes into account both publications and set requests, providing a comprehensive overview of the system's processes and mechanisms. Future updates might delve deeper into the specifics of each component or provide illustrative examples to facilitate implementation.