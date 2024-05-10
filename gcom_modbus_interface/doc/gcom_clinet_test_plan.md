### GCOM Modbus Client Test Plan - System Testing

**Author:** P. Wilshire  
**Date:** 04/16/2024  

#### Objective:
This test plan is designed to verify the functionality and reliability of the `gcom_modbus_client` under various operational conditions. It assumes prior completion of interface testing has been completed.

#### Scope:
The scope includes testing configuration validation, connection handling, data publication under normal and stress conditions, error handling, and network and system stress testing.

#### Test Environment:
- **Test Configuration:** `max_num_connections` set to 1
- **Software:** `gcom_modbus_client` and `gcom_modbus_server`
- **Hardware:** Server and client systems capable of running the software and simulating network conditions.

#### Test Steps:

| Step No. | Action | Expected Outcome | Actual Outcome | Pass/Fail |
|----------|--------|------------------|----------------|-----------|
| **1** | Validate the client config setup with both good and bad configurations. | System should correctly read valid configurations and reject invalid ones. | | |
| **2.1** | Validate the server connection and disconnection using correct IP and port. | Client should successfully connect and disconnect without errors. | | |
| **2.2** | Ensure connect events are triggered, indicating connection made and/or lost. | Event logs should accurately reflect the connection status changes. | | |
| **2.3** | Add invalid data points (invalid offsets) to the client and ensure the system still connects and disables the bad points. | Client connects and invalid data points are disabled without affecting overall connectivity. | | |
| **3.1** | With client and server connected, ensure the client publishes data. | Data should be successfully published from the client to the server. | | |
| **3.2** | Add invalid data points to the client, connect with the server; ensure valid points are still published. | Client ignores invalid points and continues to publish valid data. | | |
| **3.3** | With the server disconnected, ensure publications stop and then restart upon server reconnection. | Client stops publishing when server is disconnected and resumes once reconnected. | | |

| **3.4** | Publish sample data on the server and ensure the client reflects these changes in its publication output. | Client publication output updates to reflect new data from the server. | | |
| **3.5** | Send invalid URIs to the server and check for correct response. | Server responds correctly to invalid URIs. | | |
| **4.1** | With client and server connected, verify that FIMS sets on the client result in FIMS sets from the server. | Actions initiated by the client are mirrored by the server correctly. | | |
| **4.2** | Send client sets with bad URIs and check server response. | Server correctly identifies and rejects bad URIs with appropriate error messages. | | |
| **4.3** | Send client sets with bad data names and check responses. | Server correctly handles bad data names with appropriate error messages. | | |
| **5.1** | Add delay value below transaction timeout (250 ms) to server response. | System operates normally, continues to publish data despite added delays. | | |
| **5.2** | Add delay value above transaction timeout (250 ms) to server response. | System should log connection attempts and fail to maintain a stable connection. | | |
| **5.3** | Reduce delay value below transaction timeout (200 ms) after a higher delay. | System recovers from high delay, re-establishing connection and resuming data publication. | | |
| **5.4** | Introduce a temporary delay value above transaction timeout (300 ms). | System temporarily loses connection but recovers, reconnecting and continuing operation. | | |
| **6.5** | Connect more than one client to the same server. | System logs the attempt but does not fail; handles multiple connections gracefully. | | |
| **6.7** | Add debounce to coil and holding registers and write set values at an interval faster than the debounce. | Server does not produce outputs more frequently than the debounce interval; some values may be skipped. | | |
| **7.1** | On the Client, disable an input value from the server. | The disabled point should not appear in the list of values published by the client. | | |
| **7.2** | On the client, enable an input value on the server. | The enabled point should appear in the list of values published by the client. | | |
| **7.3** | On the client, force an input value from the server; check client publication. |
The forced value should be correctly published by the client. | | |
| **8.1** | Use the "_full" request to view more details on an individual or set of multiple URIs. | The full data request should return detailed information about specified URIs. | | |
| **8.2** | Use the "_stats" request to view system communication stats. | The stats request should return detailed statistics of the system's communications. | | |
| **8.3** | Increase the pub frequency to 10 (mS), Observe Cpu Load, Client Error messages. | The Client should be able to keep up with the increased frequency. | | |
| **8.4** | Increase the pub frequency to 10 (mS), Apply set messages to the Client | The Server should be able to output the Set messages. | | |
| **9.1** | Set different device_ids on the client , Use no Id (default) on the server | The server should accept the different device_ids as long as there is no duplicated point offsets| | |
| **9.2** | Set different device_ids on the client , Set Multiple (matching) device_ids on the server | The server should accept the mapped device_ids even if there are duplicated point offsets but with different ids| | |
| **9.3** | Set different device_ids on the client , Set Multiple device_ids on the server with some ids not matching | The server should accept the matched, mapped device_ids even if there are duplicated point offsets but with different ids| | |
| **10.1** | Set up a heartbeat on the client with just a read uri. Supply a changing heartbeat to the server  | The client should show heartbeat state , The client pub should show "component_connected": true | | |
| **10.2** | Set up a heartbeat on the client with just a read uri. remove the changing heartbeat to the server  | The client should show heartbeat state , The client pub should show "component_connected": false | | |
| **10.3** | Set up a heartbeat on the client with a write uri and a read uri | The server should show the heartbeat returned from the client | | |



#### Additional Notes:
- The tester should ensure that each test is performed under controlled conditions and repeated where necessary to verify consistency.
- All test outcomes should be documented carefully, and any deviations from expected outcomes should be investigated thoroughly.

#### action scripts
| **1** | Validate the client config setup with both good and bad configurations.
Use the scripts from the git repo
good     gcom_modbus_client configs/client/gcom_test_client
bad      gcom_modbus_client configs/client/gcom_test_client_noip
      defaults to ip:172.3.0.2
bad      gcom_modbus_client configs/client/gcom_test_client_noport
    defaults to 502
bad      gcom_modbus_client configs/client/gcom_test_client_badjson
        [error   ] Error parsing json config: The JSON document has an improper structure: missing or superfluous commas, braces, missing keys, etc.

        [error   ] Unable to parse config file [configs/client/gcom_test_client_badjson.json]. Quitting.
        [error   ] ip_address "" isn't valid or can't be found from the local service file


| **2.1** | Validate the server connection and disconnection using correct IP and port.
2.1.1   Start client with good config   
              fims_server
              fims_listen
              gcom_modbus_client configs/client/gcom_test_client ip:172.10.0.3 
                client should run and produce events but no data pubs. 

Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread id 1 failed to connect TCP to [172.10.0.3] on port [502]. Modbus error [Operation now in progress]","severity":1} 
Timestamp:    2024-04-19 15:43:04.998158

        On antother container 
              fims_server
              fims_listen
              gcom_modbus_server configs/server/gcom_test_server
              client should run and produce events with data pubs


| **2.2** | Ensure connect events are triggered, indicating connection made and/or lost. | Event logs should accurately reflect the 
The pubs on the client should become.
timeout 5 fims_listen

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client TCP [SEL-2440] Thread id 1 connecting to[172.10.0.3] on port [502]","severity":1} 
Timestamp:    2024-04-19 15:46:55.528144

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread  id 1 successfully connected to [172.10.0.3] on port [502]","severity":1} 
Timestamp:    2024-04-19 15:46:55.528242

Method:       pub
Uri:          /components/comp_sel_2440
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":0,"hold_2_2":0,"hold_2_4":0,"hold_shift_1_1":-10000.0000,"hold_shift_1_2":-10000.0000,"hold_shift_1_4":-10000.0000,"hold_shift_2_1":-10000.0000,"hold_shift_2_2":-10000.0000,"hold_shift_2_4":-10000.0000,"hold_sc_1":0,"hold_sc_2":0,"hold_sc_4":0,"hold_shift_sc_1":-10000.0000,"hold_shift_sc_2":-10000.0000,"hold_shift_sc_4":-10000.0000,"heartbeat":0,"input_shift_1_1":-10000.0000,"input_shift_1_2":-10000.0000,"input_shift_1_4":-10000.0000,"input_shift_2_1":-10000.0000,"input_shift_2_2":-10000.0000,"input_shift_2_4":-10000.0000,"surge_arrester":false,"fuse_monitoring":false,"door_latch":false,"disconnect_switch":false,"spare_1":false,"e_stop":false,"fire_relay":false,"trouble_relay":false,"component_connected":true} 
Timestamp:    2024-04-19 15:46:56.324638

Method:       pub
Uri:          /components2/comp_alt_2440
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"heartbeat":0,"surge_arrester":false,"fuse_monitoring":false,"door_latch":false,"disconnect_switch":false,"spare_1":false,"e_stop":false,"fire_relay":false,"trouble_relay":false,"component_connected":true} 
Timestamp:    2024-04-19 15:46:56.524543

Method:       pub
Uri:          /components/comp_sel_2440
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":0,"hold_2_2":0,"hold_2_4":0,"hold_shift_1_1":-10000.0000,"hold_shift_1_2":-10000.0000,"hold_shift_1_4":-10000.0000,"hold_shift_2_1":-10000.0000,"hold_shift_2_2":-10000.0000,"hold_shift_2_4":-10000.0000,"hold_sc_1":0,"hold_sc_2":0,"hold_sc_4":0,"hold_shift_sc_1":-10000.0000,"hold_shift_sc_2":-10000.0000,"hold_shift_sc_4":-10000.0000,"heartbeat":0,"input_shift_1_1":-10000.0000,"input_shift_1_2":-10000.0000,"input_shift_1_4":-10000.0000,"input_shift_2_1":-10000.0000,"input_shift_2_2":-10000.0000,"input_shift_2_4":-10000.0000,"surge_arrester":false,"fuse_monitoring":false,"door_latch":false,"disconnect_switch":false,"spare_1":false,"e_stop":false,"fire_relay":false,"trouble_relay":false,"component_connected":true} 
Timestamp:    2024-04-19 15:44:49.324471

| **2.3** | Add invalid data points (invalid offsets) to the client and ensure the system still connects and disables the bad points. | 
              gcom_modbus_client configs/client/gcom_test_client_badpoint ip:172.10.0.3 

 [172.10.0.3] on port [502]
[2024-04-19] [16:11:53.269] [2.21323s] [PID 116] [gcom_test_client_badpoint] [info    ] BAD_DATA_ADDRESS thread_id 1  tries 1 ; Bad Data Address in 1399 to 1442 , aborting for now

[2024-04-19] [16:11:53.269] [2.21329s] [PID 116] [gcom_test_client_badpoint] [info    ] Point Error, thread_id 1 failed for [hold_0_0] offset 1399 err 112345680 -> [Illegal data address]; point disconnected



| **3.4** | Publish sample data on the server and ensure the client reflects these changes in its publication output. | Client publication output updates to reflect new data from the server. | | |


on the server send 0
fims_send -m pub -u /components/sel_2440 '{"input_shift_1_1":0}'
on the client look at the shifted value
Process Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":0,"hold_2_2":0,"hold_2_4":0,"hold_shift_1_1":-10000.0000,"hold_shift_1_2":-10000.0000,"hold_shift_1_4":-10000.0000,"hold_shift_2_1":-10000.0000,"hold_shift_2_2":-10000.0000,"hold_shift_2_4":-10000.0000,"hold_sc_1":0,"hold_sc_2":0,"hold_sc_4":0,"hold_shift_sc_1":-10000.0000,"hold_shift_sc_2":-10000.0000,"hold_shift_sc_4":-10000.0000,"heartbeat":0,"input_shift_1_1":-10000.0000,"input_shift_1_2":-10000.00

on the server send 4567
on the client look at the modified shifted value

fims_send -m pub -u /components/sel_2440 '{"input_shift_1_1":4567}'
on the client
rocess Name: modbus_client_uri@SEL-2440 
Username:     root
Body:         {"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":0,"hold_2_2":0,"hold_2_4":0,"hold_shift_1_1":-10000.0000,"hold_shift_1_2":-10000.0000,"hold_shift_1_4":-10000.0000,"hold_shift_2_1":-10000.0000,"hold_shift_2_2":-10000.0000,"hold_shift_2_4":-10000.0000,"hold_sc_1":0,"hold_sc_2":0,"hold_sc_4":0,"hold_shift_sc_1":-10000.0000,"hold_shift_sc_2":-10000.0000,"hold_shift_sc_4":-10000.0000,"heartbeat":0,"input_shift_1_1":-5433.0000,"input_shift_1_2":-10000.0000,"input_shift_1_4":-10000.0000,"input_shif

| **3.5** | Send invalid URIs to the server and check for correct response. | Server responds correctly to invalid URIs. | | |
on the server
 fims_send -m pub -u /components/sel_2440 '{"xinput_shift_1_1":0}'
the server produces (spams sadly)
Bad id [xinput_shift_1_1] for uri : [/components/sel_2440]

on the server
fims_send -m pub -u /components/xsel_2440 '{"input_shift_1_1":0}
no response from the server which is ok


| **4.1** | With client and server connected, verify that FIMS sets on the client result in FIMS sets from the server. | Actions initiated by the client are mirrored by the server correctly. | | |

sh-4.2# fims_send -m set -r /$$ -u /components/comp_sel_2440/hold_1_1 23
{"gcom":"Modbus Set","status":"Success"}

Method:       set
Uri:          /components/sel_2440/hold_1_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":23}
Timestamp:    2024-04-19 17:25:18.186607

actually we got two of these  so we have a bug

on the client
fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"hold_1_1": 24}'
{"hold_1_1":24}

We still get two sets from the server
this works too
fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"hold_1_1": 24, "hold_1_2":3456}'
{"hold_1_1":24,"hold_1_2":3456}

| **4.2** | Send client sets with bad URIs and check server response. | Server correctly identifies and rejects bad URIs with appropriate error messages. | | |
fims_send -m set -r /$$ -u /components/comp_sel_2440x '{"hold_1_1": 24, "hold_1_2":3456}'
Receive Timeout. because the uri is not processed by modbus_client
and we only  decode  valid points
fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"xhold_1_1": 24, "hold_1_2":3456}'
{"hold_1_2":3456}

| **4.3** | Send client sets with bad data names and check responses. | Server correctly handles bad data names with appropriate error messages. | | |
fims_send -m set -r /$$ -u /components/comp_sel_2440
{"gcom":"Modbus Set Message","status":"Failed to decode"}

| **4.x** | Send client sets no body . | Server correctly handles with appropriate error 
h-4.2# fims_send -m set -r /$$ -u /components/comp_sel_2440
{"gcom":"Modbus Set Message","status":"Failed to decode"}

[2024-04-19] [17:23:33.971] [4161.34s] [PID 127] [gcom_test_client] [error   ] parser error [Empty: no JSON found]
runProcessFims "SEL-2440": failed with a message len 100000



| **5.2** | Add delay value above transaction timeout (250 ms) to server response. | System should log connection attempts and fail to maintain a stable connection. | | |

on the server
sh-4.2# fims_send -m pub -u /interfaces/SEL-2440/_bug '{"delay":260}'

[2024-04-19] [17:40:41.903] [5189.27s] [PID 127] [gcom_test_client] [error   ] Transfer Timeout on get thread id 1 , work num 14 time 5185.400 elapsed (mS) 200.325
[2024-04-19] [17:40:42.104] [5189.47s] [PID 127] [gcom_test_client] [error   ] Transfer Timeout on get thread id 1 , work num 15 time 5185.400 elapsed (mS) 200.367
[2024-04-19] [17:40:42.504] [5189.87s] [PID 127] [gcom_test_client] [error   ] Default thread_id 1  tries 1 errno_code 11 [Resource temporarily unavailable] wasConnected true hadContext true isConnected true
[2024-04-19] [17:40:42.504] [5189.87s] [PID 127] [gcom_test_client] [error   ] Data Error thread_id 1  io_work tNow [5186.200] connected but io failed for [hold_1_1] offset 1400  err code 11 [Resource temporarily unavailable] point off_by_one [false]
[2024-04-19] [17:40:43.506] [5190.87s] [PID 127] [gcom_test_client] [error   ] ETIMEOUT error; thread_id 1  try 1
[2024-04-19] [17:40:43.506] [5190.87s] [PID 127] [gcom_test_client] [info    ] Modbus Client TCP [SEL-2440] Thread id 1 disconnecting from [172.10.0.3] on port [502]
[2024-04-19] [17:40:43.717] [5191.08s] [PID 127] [gcom_test_client] [error   ] Default thread_id 1  tries 1 errno_code 11 [Resource temporarily unavailable] wasConnected true hadContext true isConnected false
[2024-04-19] [17:40:44.318] [5191.68s] [PID 127] [gcom_test_client] [error   ] INVALID_DATA thread_id 1  tries 1 ; Invalid Data Value in 1400 to 1442 , ran flush


| **5.3** | Reduce delay value below transaction timeout (200 ms) after a higher delay. | System recovers from high delay, re-establishing connection and resuming data publication. | | |
on the server
sh-4.2# fims_send -m pub -u /interfaces/SEL-2440/_bug '{"delay":160}'

| **5.4** | Introduce a temporary delay value above transaction timeout (300 ms). | System temporarily loses connection but recovers, reconnecting and continuing operation. | | |

sh-4.2# fims_send -m pub -u /interfaces/SEL-2440/_bug '{"cdelay":2000}' connect delay of 2000 mS
sh-4.2# fims_send -m pub -u /interfaces/SEL-2440/_bug '{"sdelay":380}' single (temp) delay of 380 mS
sh-4.2# fims_send -m pub -u /interfaces/SEL-2440/_bug '{"delay":200}' continuous delay of 200 mS 
This test plan is structured to cover a comprehensive range of functionalities and stress conditions to ensure that the `gcom_modbus_client` operates reliably under various operational scenarios.


| **8.1** | Use the "_full" request to view more details on an individual or set of multiple URIs. | The full data request should return detailed information about specified URIs. | | |
fims_send -m get -r /$$ -u /components/comp_sel_2440
{"hold_1_1":24,"hold_1_2":3456,"hold_1_4":0,"hold_2_1":0,"hold_2_2":0,"hold_2_4":0,"hold_shift_1_1":-10000.0000,"hold_shift_1_2":-10000.0000,"hold_shift_1_4":-10000.0000,"hold_shift_2_1":-10000.0000,"hold_shift_2_2":-10000.0000,"hold_shift_2_4":-10000.0000,"hold_sc_1":0,"hold_sc_2":0,"hold_sc_4":0,"hold_shift_sc_1":-10000.0000,"hold_shift_sc_2":-10000.0000,"hold_shift_sc_4":-10000.0000,"heartbeat":0,"input_shift_1_1":-10000.0000,"input_shift_1_2":-10000.0000,"input_shift_1_4":-10000.0000,"input_shift_2_1":-10000.0000,"input_shift_2_2":-10000.0000,"input_shift_2_4":-10000.0000,"surge_arrester":false,"fuse_monitoring":false,"door_latch":false,"disconnect_switch":false,"spare_1":false,"e_stop":false,"fire_relay":false,"trouble_relay":false}

fims_send -m get -r /$$ -u /components/comp_sel_2440/_full

You should see loads of data

try a single variable
sh-4.2# fims_send -m get -r /$$ -u /components/comp_sel_2440/hold_1_2
3456
try again will the _full request

sh-4.2# fims_send -m get -r /$$ -u /components/comp_sel_2440/hold_1_2/_full
{"name": "Hold_1_2", "id": "hold_1_2", "raw_val": 3456, "device_id": "2", "offset": 1401, "size": 2, "enabled": true, "disconnected": false, "process_name": "fims_send", "username": "", "is_forced": false, "forced_val": 0, "float_val": 3456, "update_time": 5688.59, "last_update_time": 5688.22, "register_type": "Holding", "value": 0, "raw_hex_val": "0xd800", "last_value": 3456, "set_value": 3456, "starting_bit_pos": 0, "number_of_bits": 32, "bit_mask": "0x3ff", "shift": 0, "scale": 0, "normal_set": true, "invert_mask": "0x0", "care_mask": "0xffffffffffffffff", "uses_mask": false, "off_by_one": false, "is_byte_swap": false, "word_order": 0, "is_float": false, "is_signed": false, "is_bit_string_type": false, "is_individual_bits": false, "is_bit_field": false, "packed_register": false, "bits_known": 0, "bits_unknown": 0, "bit_strings": [], "bit_str_known": [], "bit_str_num": [], "is_enum": false, "is_random_enum": false, "use_debounce": false, "use_deadband": false, "use_bool": true, "use_hex": false, "use_raw": false, "is_bit": false, "is_watchdog": false, "multi_write_op_code": false}

| **8.2** | Use the "_stats" request to view system communication stats. | The stats request should return detailed statistics of the system's communications. | | |
fims_send -m get -r /$$ -u /components/comp_sel_2440/hold_1_2/_stats
{"config_filename":"configs/client/gcom_test_client","pid":127,"git_build":"567","git_commit":"caf2ecb","git_tag":"v12.0.0-alpha.3","thread_connection_info": [{"id":0,"num_jobs":0,"num_fails":0}, {"id":1,"connected":true,"ip_address": "172.10.0.3","port":502,"time_to_connect":"0.000130256 ms","modbus_read_times":{"Max": 3.82732,"Min": 4.8318e-05,"Avg": 0.00293601,"Count": 28791},"modbus_write_times":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0},"num_jobs":28797,"num_fails":42}],"pub_time":{"pub_components_comp_alt_2440":{"Max": 0.512575,"Min": 0.000265255,"Avg": 0.00556248,"Count": 5753},"pub_components_comp_sel_2440":{"Max": 0.852272,"Min": 0.000345274,"Avg": 0.00809313,"Count": 5753}},"fims_message_stats":{"Max": 34,"Min": 0,"Avg": 5.93333,"Count": 15},"fims_get_errors":1,"fims_set_errors":0,"fims_pub_errors":0,"timers": [{"name":"pub_components_comp_sel_2440","runTime" :5762.2 ,"repeatTime" :1,"runs" :5760,"lastRun" :5761.2,"syncCount" :0,"totalSyncTime" :0,"syncPct" :0},{"name":"pub_components_comp_alt_2440","runTime" :5762.4 ,"repeatTime" :1,"runs" :5760,"lastRun" :5761.4,"syncCount" :0,"totalSyncTime" :0,"syncPct" :0}],"heartbeat_values":{}}

these are minor config changes

| **8.3** | Increase the pub frequency to 10 (mS), Observe Cpu Load, Client Error messages. | The Client should be able to keep up with the increased frequency. | | |
| **8.4** | Increase the pub frequency to 10 (mS), Apply set messages to the Client | The Server should be able to output the Set messages. | | |


"components": [
        {
	    "component_id":"components",
            "id": "comp_sel_2440",
            "heartbeat_enabled": false,
            "modbus_heartbeat_timeout_ms": 200,
            "component_heartbeat_timeout_ms": 200,
            "frequency": 1000,
            "offset_time": 200,
            "device_id": 2,



| **9.1** | Set different device_ids on the client , Use no Id (default) on the server | The server should accept the different device_ids as long as there is no duplicated point offsets| | |
see the _devx client configs

"connection": {
        "device name": "conn_sel_2440",
        "name": "SEL-2440",
        "device protocol": "Modbus TCP",
        "device protocol version": "6.0 WIP",
        "device id string": "sel_2440",
        "ip_address1": "192.168.112.20",
        "ip_address": "172.17.0.3",
        "port": 502,
        "device_id":1, < global>
        "max_num_connections":1,
        "data bits (8 typical)": "Columns marked in green are REQUIRED"
    },
    "components": [
        {
	    "component_id":"components",
            "id": "sel_2440",
            "heartbeat_enabled": false,
            "modbus_heartbeat_timeout_ms": 200,
            "component_heartbeat_timeout_ms": 200,
            "component_heartbeat_read_uri": "life_signal",
            "component_heartbeat_write_uri": "life",
            "frequency": 1000,
            "offset_time": 200,
            "device_id": 1,


| **9.2** | Set different device_ids on the client , Set Multiple (matching) device_ids on the server | The server should accept the mapped device_ids even if there are duplicated point offsets but with different ids| | |
| **9.3** | Set different device_ids on the client , Set Multiple device_ids on the server with some ids not matching | The server should accept the matched, mapped device_ids even if there are duplicated point offsets but with different ids| | |



| **10.1** | Set up a heartbeat on the client with just a read uri. Supply a changing heartbeat to the server  | The client should show heartbeat state , The client pub should show "component_connected": true | | |

"connection": {
        "device name": "conn_sel_2440",
        "name": "SEL-2440",
        "device protocol": "Modbus TCP",
        "device protocol version": "6.0 WIP",
        "device id string": "sel_2440",
        "ip_address1": "192.168.112.20",
        "ip_address": "172.17.0.3",
        "port": 502,
        "max_num_connections":1,
    },
    "components": [
        {
	    "component_id":"components",
            "id": "sel_2440",
            "heartbeat_enabled": true,
            "modbus_heartbeat_timeout_ms": 200,
            "component_heartbeat_timeout_ms": 2000,
            "component_heartbeat_read_uri": "life_signal",
            "component_heartbeat_write_uri": "life",


These set up the heartbeat configs
pub the value on the server read uri and the incremented value should be "set" on the server write uri.
Stop setting the heartbeat on the server and component_connected should go false on the client pub.

| **10.2** | Set up a heartbeat on the client with just a read uri. remove the changing heartbeat to the server  | The client should show heartbeat state , The client pub should show "component_connected": false | | |
| **10.3** | Set up a heartbeat on the client with a write uri and a read uri | The server should show the heartbeat returned from the client | | |



Thats about it .. sorry it still needs some tidy up.
