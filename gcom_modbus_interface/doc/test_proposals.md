# DNP3 / Modbus Test Concept Proposal

## Author: P. Wilshire
## Date: 10_09_2023

## Introduction
The document outlines a proposal for enhancing the testing mechanism for DNP3/Modbus protocols. Currently, our suite of test scripts is inefficient, and there's a need for improvement. 

## Proposal Overview

### Configurations Setup
- Establish configurations containing various settings and variations for the test environments.
  
### Test Environment
- Run server and client on different nodes. Nodes can either be SSH nodes or Docker containers.
  
### Test Execution

#### Server-Side
1. **FIMS Listener**: Start a FIMS Listener on the server with a timeout. The listener will log its output.
  
#### Client-Side
1. **Send Test Commands**: Send a series of sets to different test points using commands like:
    ```shell
    fims_send -m set -r /me -u /components/dnp3_master '{"testid":101}' 
    ```
2. **Output Verification**: The FIMS Listener log will contain test IDs. A Python script can scan these IDs to verify if the server set outputs match the expected values.
3. **Result Communication**: Smartly communicate expected results and test IDs through dummy FIMS messages to the server. The server will log these messages.

### Result Extraction
- Extract and analyze test results from the FIMS Listener log.

## Proposed Tools & Mechanisms

### Command Server
- A node or Python-based command server that listens for and responds to POST messages.
- It can execute commands, send or retrieve files to/from specific IPs.
  
### Command Relay Mechanism
- A relay tool running on every node (Docker container or SSH node).
- It accepts commands and decides whether to execute them locally or forward them to another IP.
- This mechanism simplifies communication between the Windows system and various nodes in the test network, facilitating command execution and data collection.

### Test Result Collection
- All test results are logged by FIMS Listener, and the pass/fail results are processed afterward.

### Test Setup (Client-Side)
1. Send server and client configurations.
2. Start the DNP3 server and client with respective configurations.
3. Start a FIMS Listener on the server.
4. Send test IDs and test sets to the server and client in a loop.
5. Stop the FIMS Listener on the server and retrieve the log file.
6. Analyze the log file using a Python script to extract test results.

## Demonstration
A demonstration will be prepared to illustrate the practical application of this proposal, emphasizing the relay's role and how the proposed system facilitates seamless communication and testing across different nodes.

## Conclusion
This proposal aims to streamline and enhance the DNP3/Modbus testing process through smart configurations, a relay mechanism, and automated result extraction and analysis. Feedback and further discussion on the proposal are welcome and necessary for refining and implementing the proposed system.