Modbus Device Ids


p. wilshire

01_19_2024

The Modbus Interface pProtocol includes an optional Device Id field.

When Using serial connections ( Modbus RTU) this device id allows for a proticular device to be identified in a system where many devices share the same input and output bus. The RS485 interface spec ensures that only one , selected, device is driving the shared output line at any one time.
In addition to the device ID the modbus protocol allows RTU devices to have identical register mappings with the device_id being used to differentiate between selected devices.
Modbus TCP is supposed to ignore device ID's.
Some of the vendor systems supplied to Flexgen use the device id to differentiate between almost identical pieces of equipment in deffernce to the Modbus Guidelines.
The FlexGen Modbus Client allowed the device Id field to be specified to allow it to communicate with Vendor Equiuipment that specified a device ID.
The Flexgen Modbus Server could also make use of the device ID field but , in the original release, the server could not permit identical register mappings under different device id's.

When simulating systems using duplicated mappings with different device ids special configs needed to be created that removed the duplicate register mappings.

THe 11.3 releaseof the Flexgen Modbus client/server software now allows the upgraded  register / device id  mappings to be used.

The interface is somewhat complex because we have to maintain compatibility with older configs but also allow the new functionality.
We also have to retain the original Modbus TCP Spec where device_ids were not used.

To make things simple, in many cases, you can ignore or not use device ids. This works well when Flexgen Modbis Clinats are connected to Flexgen modbus servers.
Even if you have to specify device Ids in out Modbus Client Configs, to interfce with Vendor supplied systems. The device Ids can simply be left out of the Modbus Server configs when systems are being simulated or tested.
The modbus Server will default to a device id of 255 which is like a wildcard.
The key here is the incidence of   duplicating device offset mappings using different ids.

If you have a complex device Id structure in the Modbus Clinet with no duplicated devide offsets its safe to leave hte device id out of the Server configuration when used for simulation.

If you need to add device ID's toe a Modbus Server Config then a minor change has been made to the register definitions.

Original Server definition.
Note that every register id and offset are unique within the register type name space.

```
{
   "system":
   {   "name":"SEL-2440",
       "protocol": "Modbus TCP",
       "id": "SEL-2440",
       "ip_address": "0.0.0.0",
       "xserver_delay": 510, 
       "port": 502
   },
    "registers": 
    {
        "holding_registers":
        [
            { "name":"Hold_1_1","id":"hold_1_1","offset":1400,                   "uri":"/components/sel_2440" },
            { "name":"Hold_1_2","id":"hold_1_2","offset":1401, "size":2,         "uri":"/components/sel_2440" },
            { "name":"Hold_1_4","id":"hold_1_4","offset":1403, "size":4,         "uri":"/components/sel_2440" },
            { "name":"Hold_2_1","id":"hold_2_1","offset":1407,                   "uri":"/components/sel_2440" },
            { "name":"Hold_2_2","id":"hold_2_2","offset":1408, "size":2,         "uri":"/components/sel_2440" },
            { "name":"Hold_2_4","id":"hold_2_4","offset":1410, "size":4,         "uri":"/components/sel_2440" }
        ],

        "discrete_inputs":
        [
          { "name":"fuse_monitoring",       "id":"fuse_monitoring",    "offset":390,"uri":"/components/sel_2440" },
          { "name":"surge_arrester",        "id":"surge_arrester",     "offset":393,"uri":"/components/sel_2440" },
          { "name":"door_latch",            "id":"door_latch",         "offset":394,"uri":"/components/sel_2440" },
          { "name":"disconnect_switch",     "id":"disconnect_switch",  "offset":395,"uri":"/components/sel_2440" },
          { "name":"spare_1",               "id":"spare_1",            "offset":396,"uri":"/components/sel_2440" },
          { "name":"e_stop",                "id":"e_stop",             "offset":397,"uri":"/components/sel_2440" },
          { "name":"fire_relay",            "id":"fire_relay",         "offset":398,"uri":"/components/sel_2440" },
          { "name":"trouble_relay",         "id":"trouble_relay",      "offset":399,"uri":"/components/sel_2440" },

          { "name":"fuse_monitoring_2",     "id":"fuse_monitoring_2",  "offset":490,"uri":"/components/sel_2440" },
          { "name":"surge_arrester_2",      "id":"surge_arrester_2",   "offset":493,"uri":"/components/sel_2440" },
          { "name":"door_latch_2",          "id":"door_latch_2",       "offset":494,"uri":"/components/sel_2440" },
          { "name":"disconnect_switch_2",   "id":"disconnect_switch_2","offset":495,"uri":"/components/sel_2440" },
          { "name":"spare_1",               "id":"spare_1_2",          "offset":496,"uri":"/components/sel_2440" },
          { "name":"e_stop",                "id":"e_stop_2",           "offset":497,"uri":"/components/sel_2440" },
          { "name":"fire_relay",            "id":"fire_relay_2",       "offset":498,"uri":"/components/sel_2440" },
          { "name":"trouble_relay",         "id":"trouble_relay_2",    "offset":499,"uri":"/components/sel_2440" }
    

      ],
      "input_registers":
       [
          { "name":"heartbeat_1","id":"heartbeat_1","offset":400,"uri":"/components/sel_2440" },
          { "name":"heartbeat_2","id":"heartbeat_2","offset":401,"uri":"/components/sel_2440" }       
       ]
    }    
}

```

Modified mapping
Note that the register object has become a array of register objects , each group of registers can now have their own device id.
The type and offsets in different mapped groups can now have identical offset numbers , the ids , however, must still be unique.

```
    "registers": [   << this is now an array >>
    {
        "device_id": 1,
        <register type>: [
        ]
    },
    {
        "device_id": 2,
        <register type>: [
        ]

    }
    ]
```


Example config

```
{
   "system":
   {   "name":"SEL-2440",
       "protocol": "Modbus TCP",
       "id": "SEL-2440",
       "ip_address": "0.0.0.0",
       "xserver_delay": 510, 
       "port": 502,
       "device_id":1
   },
    "registers": [
    {
        "device_id":1,
        "holding_registers":
        [
            { "name":"Hold_1_1","id":"hold_1_1","offset":1400,                   "uri":"/components/sel_2440" },
            { "name":"Hold_1_2","id":"hold_1_2","offset":1401, "size":2,         "uri":"/components/sel_2440" },
            { "name":"Hold_1_4","id":"hold_1_4","offset":1403, "size":4,         "uri":"/components/sel_2440" }
    
        ],
        "discrete_inputs":
       [
          { "name":"fuse_monitoring",       "id":"fuse_monitoring",    "offset":390,"uri":"/components/sel_2440" },
          { "name":"surge_arrester",        "id":"surge_arrester",     "offset":393,"uri":"/components/sel_2440" },
          { "name":"door_latch",            "id":"door_latch",         "offset":394,"uri":"/components/sel_2440" },
          { "name":"disconnect_switch",     "id":"disconnect_switch",  "offset":395,"uri":"/components/sel_2440" },
          { "name":"spare_1",               "id":"spare_1",            "offset":396,"uri":"/components/sel_2440" },
          { "name":"e_stop",                "id":"e_stop",             "offset":397,"uri":"/components/sel_2440" },
          { "name":"fire_relay",            "id":"fire_relay",         "offset":398,"uri":"/components/sel_2440" },
          { "name":"trouble_relay",         "id":"trouble_relay",      "offset":399,"uri":"/components/sel_2440" }

      ],
      "input_registers":
       [
          { "name":"heartbeat_1","id":"heartbeat_1","offset":400,"uri":"/components/sel_2440" }
       ]
    },
    {
        "device_id":2,
        "holding_registers":
        [
            { "name":"Hold_2_1","id":"hold_2_1","offset":1407,                   "uri":"/components/sel_2440" },
            { "name":"Hold_2_2","id":"hold_2_2","offset":1408, "size":2,         "uri":"/components/sel_2440" },
            { "name":"Hold_2_4","id":"hold_2_4","offset":1410, "size":4,         "uri":"/components/sel_2440" }

        ],
        "discrete_inputs":
        [

          { "name":"fuse_monitoring_2",     "id":"fuse_monitoring_2",  "offset":390,"uri":"/components/sel_2440" },
          { "name":"surge_arrester_2",      "id":"surge_arrester_2",   "offset":393,"uri":"/components/sel_2440" },
          { "name":"door_latch_2",          "id":"door_latch_2",       "offset":394,"uri":"/components/sel_2440" },
          { "name":"disconnect_switch_2",   "id":"disconnect_switch_2","offset":395,"uri":"/components/sel_2440" },
          { "name":"spare_1",               "id":"spare_1_2",          "offset":396,"uri":"/components/sel_2440" },
          { "name":"e_stop",                "id":"e_stop_2",           "offset":397,"uri":"/components/sel_2440" },
          { "name":"fire_relay",            "id":"fire_relay_2",       "offset":398,"uri":"/components/sel_2440" },
          { "name":"trouble_relay",         "id":"trouble_relay_2",    "offset":399,"uri":"/components/sel_2440" }
        ],
        "input_registers":
        [
          { "name":"heartbeat_2","id":"heartbeat_2","offset":400,"uri":"/components/sel_2440" }       
        ]
    }
    ]
}


### Modbus Client Configuration

1. **Global Device ID** (`client_global_device_id`): A universal identifier for the client device.
2. **Component Array** (`client_component_array`):
   - **Device Type** (`device_type`): Specifies the type of device.
   - **Register Device ID** (`register_device_id`): Optional. Overrides the global device ID for specific registers.
   - **Register Type/Offset** (`register_type_offset`): Specifies the register type and its offset.

### Modbus Server Configuration

1. **Global Device ID** (`server_global_device_id`): A universal identifier for the server device.
2. **Register Array** (`server_register_array`):
   - **Register Type/Offset** (`register_type_offset`): Specifies the register type and its offset.
   - **Device ID Override** (`device_id_override`): Optional. Overrides the global device ID for specific registers.

### Test Matrix

test_dev1 device id 1 in both client , server no array used on server all points valid.
gcom_modbus_client -c configs/client/gcom_test_client_dev1.json
gcom_modbus_server -c configs/server/gcom_test_server_dev1.json

Connect OK

client events 
###########################################


Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread id 1 failed to connect to [172.17.0.3] on port [502]. Modbus error [Connection refused]","severity":1}
Timestamp:    2024-01-19 15:18:31.710333

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread id 1 Connecting to[172.17.0.3] on port [502]","severity":1}
Timestamp:    2024-01-19 15:18:42.408977

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread  id 1 successfully connected to [172.17.0.3] on port [502]","severity":1}
Timestamp:    2024-01-19 15:18:42.409026

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread id 1 Disconnecting from [172.17.0.3] on port [502]","severity":1}
Timestamp:    2024-01-19 15:19:10.808253

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread id 1 failed to connect to [172.17.0.3] on port [502]. Modbus error [Connection refused]","severity":1}
Timestamp:    2024-01-19 15:19:11.908416

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread id 1 Connecting to[172.17.0.3] on port [502]","severity":1}
Timestamp:    2024-01-19 15:19:19.408950

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"Modbus Client","message":"Modbus Client [SEL-2440] Thread  id 1 successfully connected to [172.17.0.3] on port [502]","severity":1}
Timestamp:    2024-01-19 15:19:19.409031



server events
#########################

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"source":"Modbus Server","message":"Modbus Server SEL-2440 detected a TCP client disconnect.\n","severity":1}
Timestamp:    2024-01-19 14:56:29.630493

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"source":"Modbus Server","message":"New connection from 172.17.0.2:6800 on interface SEL-2440","severity":1}
Timestamp:    2024-01-19 14:56:37.198046

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"source":"Modbus Server","message":"Modbus Server SEL-2440, listening for connections on 0.0.0.0 port 502.\n","severity":1}
Timestamp:    2024-01-19 14:57:14.258431

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"source":"Modbus Server","message":"New connection from 172.17.0.2:16016 on interface SEL-2440","severity":1}

<new>
Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"source":"Modbus Server","message":"Modbus Server SEL-2440 shutting down.\n","severity":1}
Timestamp:    2024-01-19 15:05:00.733467
Timestamp:    2024-01-19 14:57:14.597381



Client Fims Listen Pubs 
            OK

client sets
###########################

fims_send -m set -r /$$ -u /components/sel_2440/hold_1_1  1234
    {"gcom":"Modbus Set","status":"Success"}

    note should be 
    1234

server listen
###########################

    Method:       set
    Uri:          /components/sel_2440/hold_1_1
    ReplyTo:      (null)
    Process Name: SEL-2440
    Username:     root
    Body:         {"value":1234}
    Timestamp:    2024-01-19 14:48:17.783935


client set
###########################

fims_send -m set -r /$$ -u /components/sel_2440/hold_1_1  1234
            {"gcom":"Modbus Set","status":"Success"}

server listen
###########################

    Method:       set
    Uri:          /components/sel_2440/hold_1_1
    ReplyTo:      (null)
    Process Name: SEL-2440
    Username:     root
    Body:         {"value":1234}
    Timestamp:    2024-01-19 14:48:17.783935

client set
###########################

fims_send -m set -r /$$ -u /components/sel_2440/hold_1_2  12345678
            {"gcom":"Modbus Set","status":"Success"}

server listen
###########################
    Method:       set
    Uri:          /components/sel_2440/hold_1_2
    ReplyTo:      (null)
    Process Name: SEL-2440
    Username:     root
    Body:         {"value":12345678}
    Timestamp:    2024-01-19 14:50:27.865697

client set
###########################

fims_send -m set -r /$$ -u /components/sel_2440/hold_1_1  12345678
            {"gcom":"Modbus Set","status":"Success"}

server listen
###########################

    Method:       set
    Uri:          /components/sel_2440/hold_1_1
    ReplyTo:      (null)
    Process Name: SEL-2440
    Username:     root
    Body:         {"value":65535}
    Timestamp:    2024-01-19 14:50:35.854974





clinet gets
###########################

summary data for a component

fims_send -m get -r /$$ -u /components/sel_2440 | jq
{
  "hold_1_1": 0,
  "hold_1_2": 0,
  "hold_1_4": 0,
  "hold_2_1": 0,
  "hold_2_2": 0,
  "hold_2_4": 0,
  "heartbeat_1": 0,
  "surge_arrester_1": false,
  "fuse_monitoring_1": false,
  "door_latch_1": false,
  "disconnect_switch_1": false,
  "spare_1_1": false,
  "e_stop_1": false,
  "fire_relay_1": false,
  "trouble_relay_1": false
}

complete data for an unspecified  component

 fims_send -m get -r /$$ -u /components/sel_2440/trouble_relay/_full | jq
{
  "gcom": "Modbus Point Unknown",
  "status": "Falure"
}

complete data for a specified  component

 fims_send -m get -r /$$ -u /components/sel_2440/trouble_relay_1/_full | jq
"trouble_relay_1": {
    "name": "Trouble Relay (IN201)",
    "id": "trouble_relay_1",
    "raw_val": 0,
    "device_id": "1",
    "offset": 399,
    "size": 1,
    "enabled": true,
    "process_name": "",
    "username": "",
    "is_forced": false,
    "forced_val": 0,
    "float_val": 0,
    "update_time": 57.2006,
    "last_update_time": 56.2011,
    "register_type": "Discrete Inputs",
    "value": false,
    "raw_hex": "0x0",
    "starting_bit_pos": 0,
    "number_of_bits": 16,
    "bit_mask": "0xff",
    "shift": 0,
    "scale": 0,
    "normal_set": false,
    "invert_mask": "0x0",
    "care_mask": "0xffffffffffffffff",
    "uses_mask": false,
    "is_float": false,
    "is_signed": false,
    "is_byte_swap": false,
    "word_order": 0,
    "is_bit_string_type": false,
    "is_individual_bits": false,
    "is_bit_field": false,
    "packed_register": false,
    "bits_known": 0,
    "bits_unknown": 0,
    "bit_strings": [],
    "bit_str_known": [],
    "bit_str_num": [],
    "is_enum": false,
    "is_random_enum": false,
    "use_debounce": false,
    "use_deadband": false,
    "use_bool": true,
    "use_hex": false,
    "use_raw": false,
    "is_bit": false,
    "is_watchdog": false,
    "multi_write_op_code": false
  }

clinet pubs
##############################

Method:       pub
Uri:          /components/sel_2440
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":0,"hold_2_2":0,"hold_2_4":0,"heartbeat_1":0,"surge_arrester_1":false,"fuse_monitoring_1":false,"door_latch_1":false,"disconnect_switch_1":false,"spare_1_1":false,"e_stop_1":false,"fire_relay_1":false,"trouble_relay_1":false}
Timestamp:    2024-01-19 15:31:38.417751

Method:       pub
Uri:          /components2/alt_2440
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"heartbeat_2":0,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"INIT","component_connected":"false","Timestamp":"01-19-2024 15:31:38.617565"}
Timestamp:    2024-01-19 15:31:38.617861


server sets
###############################

    fims_send -m set -r/$$ -u /components/sel_2440 '{"heartbeat_2":20}'
    Receive Timeout.

server gets

   fims_send -m get -r /$$ -u /components/sel_2440
   Receive Timeout.

server pubs
   fims_send -m pub -u /components/sel_2440 '{"heartbeat_2":22}'

server listen
   onl gets sets from server

Client Heartbeat
###############################

Client Read Heartbeat
    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":0,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"INIT","component_connected":"false","Timestamp":"01-19-2024 15:31:38.617565"}
    Timestamp:    2024-01-19 15:31:38.617861

server 
>>>>>>>>          fims_send -m set -u /components/sel_2440 '{"heartbeat_2":23}'

    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":23,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"NORMAL","component_connected":"true","Timestamp":"01-19-2024 15:40:04.617234"}
    Timestamp:    2024-01-19 15:40:04.617490


server 
>>>>>>>>          fims_send -m set -u /components/sel_2440 '{"heartbeat_2":22}'

    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":22,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"NORMAL","component_connected":"true","Timestamp":"01-19-2024 15:40:07.616992"}
    Timestamp:    2024-01-19 15:40:07.617175



server 
>>>>>>>>          fims_send -m set -u /components/sel_2440 '{"heartbeat_2":20}'

    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":20,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"NORMAL","component_connected":"true","Timestamp":"01-19-2024 15:40:11.617300"}
    Timestamp:    2024-01-19 15:40:11.617571



>>>>>>> heartbeat frozen component connected : false

    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":20,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"FROZEN","component_connected":"false","Timestamp":"01-19-2024 15:40:17.617275"}
    Timestamp:    2024-01-19 15:40:17.617560


    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":20,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_state":"FROZEN","component_connected":"false","Timestamp":"01-19-2024 15:40:22.617351"}
    Timestamp:    2024-01-19 15:40:22.617595


    Method:       pub
    Uri:          /components2/alt_2440
    ReplyTo:      (null)
    Process Name: modbus_client_uri@SEL-2440
    Username:     root
    Body:         {"heartbeat_2":20,"surge_arrester_2":false,"fuse_monitoring_2":false,"door_latch_2":false,"disconnect_switch_2":false,"spare_1_2":false,"e_stop_2":false,"fire_relay_2":false,"trouble_relay_2":false,"heartbeat_tate":"FROZEN","component_connected":"false","Timestamp":"01-19-2024 15:40:23.617435"}
    Timestamp:    2024-01-19 15:40:23.617710


Client Read heartbeat with write uri
>>>>>>>>>>>> input to client
    fims_send -m pub -u /components/sel_2440 '{"heartbeat_2":22}'

>>>>>>>>>>>> output from server 
  simply increments the number regardless of heartbeat state
    Method:       set
    Uri:          /components/sel_2440/hold_2_1
    ReplyTo:      (null)
    Process Name: SEL-2440
    Username:     root
    Body:         {"value":23}
    Timestamp:    2024-01-19 15:55:28.004754

Server Heartbeat
    N/a Yet
Server Read Heartbeat 
    N/A yet


Testing device_id operation
        option 1 client no devid specified, server no devid specified, no multiple defs
        gcom_modbus_client -c configs/client/gcom_test_client_nodev.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_nodev.json
          -> OK

        option 2 client no devid specified, server devid specified, no multiple defs
        gcom_modbus_client -c configs/client/gcom_test_client_nodev.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_dev1.json
          -> OK

        option 3 client devid specified, server no devid specified, no multiple defs
        gcom_modbus_client -c configs/client/gcom_test_client_dev1.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_nodev.json
          -> OK

        option 4 client devid specified, server no devid specified, no multiple defs, use dev array
        gcom_modbus_client -c configs/client/gcom_test_client_dev1.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_nodeva.json
          -> OK
           gcom_modbus_server -c configs/server/gcom_test_server_nodeva.json
                System config complete: Setup register map.
                Failed to get object item 'device_id' as a number in 'devices' array object #0
                seg fault ( now fixed )

        option 5 client multiple devid specified, server no devid specified, no multiple defs
        gcom_modbus_client -c configs/client/gcom_test_client_dev2.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_nodev.json
          -> OK

        option 6 client multiple devid specified, server no devid specified, no multiple defs , array
        gcom_modbus_client -c configs/client/gcom_test_client_dev2.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_nodeva.json
          -> OK

        option 7 client multiple devid specified, server with one good devid specified, no multiple defs
        gcom_modbus_client -c configs/client/gcom_test_client_dev2.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_dev2.json
            -> was spams now fixed  client bug keeps writing does not disable point because it was a heartbeat

        option 8 client multiple devid specified, server with two devids specified, no multiple defs, array
        gcom_modbus_client -c configs/client/gcom_test_client_dev2.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_dev2a.json
            -> OK

        option 9 client multiple devid specified multiple defs, server with two devids specified, multiple defs, array
        gcom_modbus_client -c configs/client/gcom_test_client_dev2m.json
          -> gcom_modbus_server -c configs/server/gcom_test_server_dev2am.json
            -> OK






