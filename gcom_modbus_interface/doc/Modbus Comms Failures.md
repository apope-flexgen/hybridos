## Modbus Communications Failures: A Discussion Document

**Author**: P. Wilshire  
**Date**: 25th September 2023

---

### Introduction

Modbus, as a widely-used communications protocol, can be prone to a range of failures during operations. Understanding these potential pitfalls and preparing for them can ensure smoother communications and faster troubleshooting. This document examines the common Modbus communication failures and proposes actionable recommendations to address them.

### Common Failures

1. **Connection Failures**:
    - **Cannot Connect**: The remote node does not respond. This can be verified using a ping test.
    - **Connection Refused**: The remote Modbus server is not running or its firewall is not allowing access to the designated port.

2. **Operational Failures**:
    - **Connection Timeout**: If a connection timeout is specified and is exceeded, the connection is deemed dead, necessitating re-establishment.
  
    - **Data Exchange Failures**: Once communication is established, clients might attempt to read or write data from the server. Failures here can manifest in several ways:
        1. Incorrect device ID.
        2. Requesting data from a bit or register offset that the server is not configured for.
        3. Delayed server response.
        4. Server responding with an error.
        5. Server being too occupied to service a request.
        6. Server going offline.

### Responses to Failures

1. **Incorrect Device ID**:
    - **System Configuration Issue**:
        - Report the error, disable the request for that section, and continue.
        - Report the error and halt the process; let `systemctl` attempt a restart.
        - Report the error without halting, continuously highlighting the configuration error.

2. **Unconfigured Data Point Request**:
    - **Data Configuration Issue**:
        - Report the error, disable the request for the data point, and proceed.
        - Report and halt the process; allow `systemctl` to attempt a restart.
        - Continuously report the configuration error without halting.

3. **Delayed Server Response**:
    - This is a significant issue we currently face where late incoming data offsets subsequent data packets.
    - Optimal Action: Execute a `modbus_flush` and retry the request, which might restore data synchronization with the server.

4. **Server Responds with Error**:
    - Depending on the nature of the error, the client can:
        - Flush and retry immediately.
        - Flush, delay, then retry.
        - Report the error, skip the data point for this scan cycle, and continue.
        - Report the error, close the context, and attempt to re-establish it.
        - After five unsuccessful attempts, terminate the program and let `systemctl` restart it.
        - Continuously report the error without halting.

5. **Server Overload**:
    - Delay briefly and retry.
    - After five such attempts, make an informed decision on further action.

6. **Server Goes Offline**:
    - Delay briefly and retry.
    - After five such attempts, make an informed decision on further action.

### Recommendations & Conclusions

1. **Distinguishing Between Errors**:
    It's crucial to differentiate between communication timeouts requiring a reconnect and message timeouts necessitating a `modbus_flush`.

2. **Unconfigured Points**:
    Significant time can be saved by identifying unconfigured points quickly. The current system, where commissioning or integration engineers manually deduce this, is inefficient.

3. **Unresolvable Failures**:
    For failures that can't be fixed, such as server outages or configuration mismatches, a clear "configuration failure" policy should be established.

