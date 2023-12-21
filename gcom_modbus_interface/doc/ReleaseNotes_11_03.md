###  Gcom Modbus Client Release Notes 11_03
p. wilshire
s. reynolds
    12_08_2023

# overrides
These are used for the command line to allow "temporary" adjustments to configs


### how to find available overrides

run the modbus_client use the help override 
```
gcom_modbus_client

```

This produces 

```
override options :

auto_disable:false           turn off the automatic invalid point disable feature
allow_multi_sets:false       allow set operations to be grouped together
force_multi_sets:false       force set operations to be grouped together
ip:172.17.0.3                override the config ip_address
port:503                     override the config port number
format:naked|clothed         override the config format
debug:true                   turn on debug options
debug_decode:true            turn on decode debug options
debug_connection:true        turn on connection debug options
debug_fims:true              turn on fims debug options
pub_coil:false               turn on pub output for coils
pub_holding:false            turn on pub output for holding
pub_input:true               turn pub output for input  ( this are normally turned on)
pub_discrete:true            turn pub output for discrete inputs ( this are normally turned on)
pub_sync:false               turn off pub sync (delays next pub request if current pub request takes too long)
sync_percent:10              set pub sync threshold percent (delays next pub request if current pub request takes too long)


```

The startup output will show the config options selected with a remark on any not understood.

```
[2023-11-16] [15:51:28.111] [0.0133622s] [PID 9365] [gcom_mb_client] [info    ] processing option [pub_discrete:false] ...Ok
[2023-11-16] [15:51:28.111] [0.0133723s] [PID 9365] [gcom_mb_client] [info    ] processing option [pub_input:true] ...Ok
[2023-11-16] [15:51:28.111] [0.0133842s] [PID 9365] [gcom_mb_client] [info    ] processing option [port:502] ...Ok
[2023-11-16] [15:51:28.111] [0.0133842s] [PID 9365] [gcom_mb_client] [info    ] processing option [foo:fum] ...unknown option

```


# auto_disable. 

The auto_disable option gives the user the ability to let the system stop trying to access data points on the server that are incorrectly set up.
 

## Setup 
 
### in config 
```
connection.auto_disable : true| false
```

### in overrides
```
build/release/gcom_modbus_client -c  configs/client/gcom_test_client_4.json  auto_disable:true|false
```


When auto_disable is enabled the point that produces an error on any access attepmt ( for example it is not defined in the server config)
will be removed from the remote system scan.

When auto_disable is disabled and no disabled points are given forced values, all points in the io_block containing the error point will not be in the output pub.

When auto_disable is disabled the point that is not defined in the server config will be present in the client pub but will use the forced value, it if has one.

Every time you try to read the "bad" point yoy will get an error mesage and the io_point group that the bad point belonds to will not be included in the pub output

### Log outout

```
thread_id 3 read input bits failed for [break_test_1] offset 299 err code 112345680 [Illegal data address]
thread_id 3 read input bits failed for [break_test_2] offset 300 err code 112345680 [Illegal data address]
```


If  an io_point group contains an invalid point that whole group will be removed from the pub output.


### Manual disable / forced values

This situation can be "fixed up" by manually diasbling the point 

```
fims_send -m set -r /$$ -u /components/comp_sel_2440/break_test_1/_disable 1
{"gcom":"Modbus Point Disable","status":"Success"}
```

Now the group will be split up into two groups with the disabled point not included.

The value of this point can be "forced" as follows. 
In which case the forced disabled point will appear inthe pub output.

```
fims_send -m set -r /$$ -u /components/comp_sel_2440/break_test_1/_force 1
{"gcom":"Modbus Point Force","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/break_test_1/_force 0
{"gcom":"Modbus Point Force","status":"Success"}

```

# Pub Sync

A configurable / override option allows a late response to a pub request to delay the next pub request.
This allows pubs to a slow responding server to stagger later pubs and not overload the system.
Pub requests sent to a sync'd pub  will have their base frequency diviced by two.
IE request 100 ms and , if no sync response is received that pub will be presented every 200 ms
If a pub request completes before the config delay time ( say 50ms) then the next pub will be started at the next scheduled interval.
If a pub request completes after the config delay time  the next pub request will be delayed.

Config options to set this up are called:
 "sync_percent":<int 20 - 80> 
and "pub_sync": < bool true:false>
```
{
"components": [
        {
            "id": "sel_3530_fast_rtac",
            "frequency": 8,
            "offset_time": 0,
            "pub_sync": true,
            "sync_percent": 60,
         }
         }
]
}
```

# Uri Requests

Request options can be added to any uri to do the following

# enable/disable an  io_point
# force/unforce an  io_point
# change output format (naked/clothed/full)
# access local or remote data items 
    see last value received or forced value for local items
    see actual values on the server for remote requests,


# Fims Input 

## new  ( standardized) options  ( bool = true false "true" "false" 1 0, full single, and the old multi )

### All these single methods now work 

```
fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '{false}'
{"gcom":"Modbus Set Message","status":"Failed to decode"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '{"false"}'
{"gcom":"Modbus Set Message","status":"Failed to decode"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '"false"'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '"true"'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 1
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 0
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '{"value":1}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '{"value":0}'
{"gcom":"Modbus Set","status":"Success"}
```

### As well as these

```
fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_0":0}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_0":1}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_0":false}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_0":"true"}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_1": false, "coil_0":"true"}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_1": false, "coil_0":"false"}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_1": {"value":false}, "coil_0":"false"}'
{"gcom":"Modbus Set","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_1": {"value":true}, "coil_0":"false"}'
{"gcom":"Modbus Set","status":"Success"}

```


### We also can get all the data points with a single "get"

```
fims_send -m get -r /$$ -u /components/comp_sel_2440
```

```
{"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":1,"hold_2_2":0,"hold_2_4":0,"shold_1_1":0,"shold_1_2":0,"shold_1_4":0,"shold_2_1":0,"shold_2_2":0,"shold_2_4":0,"fhold_1_1":0.0000,"fhold_1_2":0.0000,"fhold_1_4":0.0000,"fshold_1_4":0.0000,"scale_1_1":4,"scale_1_2":4,"scale_1_4":4,"shift_1_1":100,"shift_1_2":100,"shift_1_4":100,"debounce_1":0,"test_id":0,"inv_1_16":0,"inv_1_4":0,"multi_1":0,"multi_2":0,"multi_3":0,"multi_4":0,"multi_5":0,"multi_6":0,"multi_7":0,"multi_8":0,"multi_9":0,"multi_10":0,"multi_11":0,"multi_12":0,"multi_13":0,"error_1":100,"input_0_1":0,"input_1_1":0,"input_2_1":0,"input_3_1":0,"sinput_4_1":0,"input_5_1":123,"finput_6_1":0.0000,"input_7_1":0,"input_0_2":4,"input_1_2":0,"input_2_2":0,"input_3_2":0,"sinput_4_2":0,"input_5_2":0,"finput_6_2":0.0000,"input_7_2":0,"input_0_4":0,"input_1_4":0,"input_2_4":0,"input_3_4":0,"sinput_4_4":0,"input_5_4":0,"finput_6_4":0.0000,"input_7_4":0,"enum_256_1":[{"value":0, "string":"some_string"}],"enum_257_2":[{"value":0, "string":"some_string"}],"enum_259_4":[{"value":0, "string":"some_string"}],"bsinput_0_4":0,"bsinput_1_4":0,"heartbeat":0,"heartbeat2":0,"ibits_1000_1":0,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"bf_2007_1":[],"bf_2008_2":[],"bf_2010_4":[],"pack_1_first":[{"value":0, "string":"undefined"}],"pack_1_second":[{"value":0, "string":"undefined"}],"pack_1_first":[{"value":0, "string":"undefined"}],"pack_1_second":[{"value":0, "string":"undefined"}],"coil_0":0,"coil_1":1,"coil_2":0,"coil_3":0,"coil_4":0,"coil_5":0,"coil_6":0,"coil_7":0,"surge_arrester":0,"fuse_monitoring":0,"door_latch":0,"disconnect_switch":0,"spare_1":0,"e_stop":0,"fire_relay":0,"trouble_relay":0,"break_test_1":0,"break_test_2":0}

```

## local / remote

### This is a new get option that uses the uri requests option

```
fims_send -m get -r /$$ -u /components/comp_sel_2440/_remote
```

### It provides ...



```
{"test_id":0,"inv_1_16":0,"inv_1_4":0,"multi_1":0,"multi_2":0,"multi_3":0,"multi_4":0,"multi_5":0,"multi_6":0,"multi_7":0,"multi_8":0,"multi_9":0,"multi_10":0,"multi_11":0,"multi_12":0,"multi_13":0,"debounce_1":0,"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":1,"hold_2_2":0,"hold_2_4":0,"shold_1_1":0,"shold_1_2":0,"shold_1_4":0,"shold_2_1":0,"shold_2_2":0,"shold_2_4":0,"fhold_1_1":0.0000,"fhold_1_2":0.0000,"fhold_1_4":0.0000,"fshold_1_4":0.0000,"scale_1_1":4,"scale_1_2":4,"scale_1_4":4,"shift_1_1":100,"shift_1_2":100,"shift_1_4":100,"error_1":100,"input_0_1":0,"input_1_1":0,"input_2_1":0,"input_3_1":0,"sinput_4_1":0,"input_5_1":123,"finput_6_1":0.0000,"input_7_1":0,"input_0_2":4,"input_1_2":0,"input_2_2":0,"input_3_2":0,"sinput_4_2":0,"input_5_2":0,"finput_6_2":0.0000,"input_7_2":0,"input_0_4":0,"input_1_4":0,"input_2_4":0,"input_3_4":0,"sinput_4_4":0,"input_5_4":0,"finput_6_4":0.0000,"input_7_4":0,"enum_256_1":[{"value":0, "string":"some_string"}],"enum_257_2":[{"value":0, "string":"some_string"}],"enum_259_4":[{"value":0, "string":"some_string"}],"bsinput_0_4":0,"bsinput_1_4":0,"heartbeat":0,"heartbeat2":0,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"bf_2007_1":[],"bf_2008_2":[],"bf_2010_4":[],"pack_1_first":[{"value":0, "string":"undefined"}],"ibits_1000_1":0,"pack_1_second":[{"value":0, "string":"undefined"}],"pack_1_first":[{"value":0, "string":"undefined"}],"pack_1_second":[{"value":0, "string":"undefined"}],"coil_0":0,"coil_1":1,"coil_2":0,"coil_3":0,"coil_4":0,"coil_5":0,"coil_6":0,"coil_7":0,"break_test_1":0,"break_test_2":0,"surge_arrester":0,"fuse_monitoring":0,"door_latch":0,"disconnect_switch":0,"spare_1":0,"e_stop":0,"fire_relay":0,"trouble_relay":0}
```

 fims_send -m get -r /$$  -u  /components/comp_sel_2440

```
{"hold_1_1":0,"hold_1_2":0,"hold_1_4":0,"hold_2_1":1,"hold_2_2":0,"hold_2_4":0,"shold_1_1":0,"shold_1_2":0,"shold_1_4":0,"shold_2_1":0,"shold_2_2":0,"shold_2_4":0,"fhold_1_1":0.0000,"fhold_1_2":0.0000,"fhold_1_4":0.0000,"fshold_1_4":0.0000,"scale_1_1":4,"scale_1_2":4,"scale_1_4":4,"shift_1_1":100,"shift_1_2":100,"shift_1_4":100,"test_id":0,"inv_1_16":0,"inv_1_4":0,"multi_1":0,"multi_2":0,"multi_3":0,"multi_4":0,"multi_5":0,"multi_6":0,"multi_7":0,"multi_8":0,"multi_9":0,"multi_10":0,"multi_11":0,"multi_12":0,"multi_13":0,"debounce_1":0,"error_1":100,"bsinput_0_4":0,"bsinput_1_4":0,"enum_259_4":[{"value":0, "string":"some_string"}],"input_0_1":0,"input_1_1":0,"input_2_1":0,"input_3_1":0,"sinput_4_1":0,"input_5_1":0,"finput_6_1":0.0000,"input_7_1":0,"input_0_2":0,"input_1_2":0,"input_2_2":0,"input_3_2":0,"sinput_4_2":0,"input_5_2":0,"finput_6_2":0.0000,"input_7_2":0,"input_0_4":0,"input_1_4":0,"input_2_4":0,"input_3_4":0,"sinput_4_4":0,"input_5_4":0,"finput_6_4":0.0000,"input_7_4":0,"enum_256_1":[{"value":0, "string":"some_string"}],"enum_257_2":[{"value":0, "string":"some_string"}],"heartbeat":0,"heartbeat2":0,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"some_string":false,"another_string":false,"yet_another_string":false,"etc.":false,"bf_2007_1":[],"bf_2008_2":[],"bf_2010_4":[],"ibits_1000_1":0,"pack_1_second":[{"value":0, "string":"undefined"}],"pack_1_first":[{"value":0, "string":"undefined"}],"pack_1_first":[{"value":0, "string":"undefined"}],"pack_1_second":[{"value":0, "string":"undefined"}],"coil_0":0,"coil_1":0,"coil_2":0,"coil_3":0,"coil_4":0,"coil_5":0,"coil_6":0,"coil_7":0,"surge_arrester":0,"fuse_monitoring":0,"door_latch":0,"disconnect_switch":0,"spare_1":0,"e_stop":0,"fire_relay":0,"trouble_relay":0,"break_test_1":0,"break_test_2":0,"watchdog_state":"INIT","component_connected":1}
```


or
```
fims_send -m get -r /$$  -u  /components/comp_sel_2440 | jq
```

```
{
  "test_id": 0,
  "inv_1_16": 0,
  "inv_1_4": 0,
  "multi_1": 0,
  "multi_2": 0,
  "multi_3": 0,
  "multi_4": 0,
  "multi_5": 0,
  "multi_6": 0,
  "multi_7": 0,
  "multi_8": 0,
  "multi_9": 0,
  "multi_10": 0,
  "multi_11": 0,
  "multi_12": 0,
  "multi_13": 0,
  "hold_1_1": 0,
  "hold_1_2": 0,
  "hold_1_4": 0,
  "hold_2_1": 1,
  "hold_2_2": 0,
  "hold_2_4": 0,
  "shold_1_1": 0,
  "shold_1_2": 0,
  "shold_1_4": 0,
  "shold_2_1": 0,
  "shold_2_2": 0,
  "shold_2_4": 0,
  "fhold_1_1": 0,
  "fhold_1_2": 0,
  "fhold_1_4": 0,
  "fshold_1_4": 0,
  "scale_1_1": 4,
  "scale_1_2": 4,
  "scale_1_4": 4,
  "shift_1_1": 100,
  "shift_1_2": 100,
  "shift_1_4": 100,
  "debounce_1": 0,
  "error_1": 100,
  "enum_259_4": [
    {
      "value": 0,
      "string": "some_string"
    }
  ],
  "input_0_1": 0,
  "input_1_1": 0,
  "input_2_1": 0,
  "input_3_1": 0,
  "sinput_4_1": 0,
  "input_5_1": 0,
  "finput_6_1": 0,
  "input_7_1": 0,
  "input_0_2": 0,
  "input_1_2": 0,
  "input_2_2": 0,
  "input_3_2": 0,
  "sinput_4_2": 0,
  "input_5_2": 0,
  "finput_6_2": 0,
  "input_7_2": 0,
  "input_0_4": 0,
  "input_1_4": 0,
  "input_2_4": 0,
  "input_3_4": 0,
  "sinput_4_4": 0,
  "input_5_4": 0,
  "finput_6_4": 0,
  "input_7_4": 0,
  "enum_256_1": [
    {
      "value": 0,
      "string": "some_string"
    }
  ],
  "enum_257_2": [
    {
      "value": 0,
      "string": "some_string"
    }
  ],
  "bsinput_0_4": 0,
  "bsinput_1_4": 0,
  "heartbeat": 0,
  "heartbeat2": 0,
  "some_string": false,
  "another_string": false,
  "yet_another_string": false,
  "etc.": false,
  "bf_2007_1": [],
  "bf_2008_2": [],
  "bf_2010_4": [],
  "ibits_1000_1": 0,
  "pack_1_second": [
    {
      "value": 0,
      "string": "undefined"
    }
  ],
  "pack_1_first": [
    {
      "value": 0,
      "string": "undefined"
    }
  ],
  "coil_0": 0,
  "coil_1": 0,
  "coil_2": 0,
  "coil_3": 0,
  "coil_4": 0,
  "coil_5": 0,
  "coil_6": 0,
  "coil_7": 0,
  "surge_arrester": 0,
  "fuse_monitoring": 0,
  "door_latch": 0,
  "disconnect_switch": 0,
  "spare_1": 0,
  "e_stop": 0,
  "fire_relay": 0,
  "trouble_relay": 0,
  "break_test_1": 0,
  "break_test_2": 0,
  "watchdog_state": "INIT",
  "component_connected": 1
}
```

### Also have the local option

```
fims_send -m get -r /$$ -u /components/comp_sel_2440/_local
```


### This works even when the server is not connected.

(So does the get /_remote but this is a bug)




# fims get with full object details.


fims_send -m get -r /$$ -u /components/comp_sel_2440/trouble_relay/_full

```
{"name": "Trouble Relay (IN201)", "id": "trouble_relay", "device_id": "3", "offset": 399, "size": 1, "forced": false, "enabled": true, "raw_val": 0, "register_type": "Discrete Inputs", "value": 1, "last_value": 1, "set_value": 1, "raw_hex": "0x0", "starting_bit_pos": 0, "number_of_bits": 16, "bit_mask": "0xff", "shift": 0, "scale": 0, "normal_set": false, "invert_mask": "0x0", "care_mask": "0xffffffffffffffff", "uses_mask": false, "is_float": false, "is_signed": false, "is_word_swap": false, "is_byte_swap": false, "word_order": 0, "is_float64": false, "is_bit_string_type": false, "is_individual_bits": false, "is_bit_field": false, "packed_register": false, "bits_known": 0, "bits_unknown": 0, "bit_strings": [], "bit_str_known": [], "bit_str_num": [], "is_enum": false, "is_random_enum": false, "is_forced": false, "is_enabled": true, "offtime": 0, "forced_val": 0, "use_debounce": false, "use_deadband": false, "float_val": 0, "last_float_val": 0, "last_raw_val": 0, "use_bool": false, "use_hex": false, "use_raw": false, "is_bit": false, "process_name": "", "username": "", "is_watchdog": false, "multi_write_op_code": false}
```
or 

```
{
  "name": "Trouble Relay (IN201)",
  "id": "trouble_relay",
  "device_id": "3",
  "offset": 399,
  "size": 1,
  "forced": false,
  "enabled": true,
  "raw_val": 0,
  "register_type": "Discrete Inputs",
  "value": 1,
  "last_value": 1,
  "set_value": 1,
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
  "is_word_swap": false,
  "is_byte_swap": false,
  "word_order": 0,
  "is_float64": false,
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
  "is_forced": false,
  "is_enabled": true,
  "offtime": 0,
  "forced_val": 0,
  "use_debounce": false,
  "use_deadband": false,
  "float_val": 0,
  "last_float_val": 0,
  "last_raw_val": 0,
  "use_bool": false,
  "use_hex": false,
  "use_raw": false,
  "is_bit": false,
  "process_name": "",
  "username": "",
  "is_watchdog": false,
  "multi_write_op_code": false
}
```

# Word order for when byte swap is not enough

You can change the order of  4 register values to suit the input data from a modbus server.
only used for size 4 registers.


```
"word_order": 1234,

"word_order": 3412,

"word_order": 1324,
```


#  events / watchdog / heartbeat options ( two level watchdogs Alarm and Fault)


## Config options to define watchdog and heartbeat.


```
"components": [
        {
	    "component_id":"components",
            "id": "comp_sel_2440",
            "heartbeat_enabled": true,
            "modbus_heartbeat_timeout_ms": 2000,
            "component_heartbeat_timeout_ms": 2000,
            "component_heartbeat_read_uri": "input_2_1",
            "component_heartbeat_write_uri": "hold_2_1",
            "component_heartbeat_max_value": 5000,
            "watchdog_uri": "input_2_1",
            "watchdog_enabled":true,
            "watchdog_alarm_timeout_ms":5000,
            "watchdog_fault_timeout_ms":10000,
            "watchdog_recovery_timeout_ms":5000,
            "watchdog_recovery_time_ms":10000,
            "watchdog_frequency_ms":1000,
            "frequency": 1000,
            "offset_time": 200,
            "device_id": 3
        }
        ]
```

### This is the log of the Watchdog 

```
[2023-11-15] [18:18:49.425] [4621.02s] [PID 8692] [gcom_mb_client] [info    ] Watchdog [input_2_1] has entered the [NORMAL] state.
[2023-11-15] [18:18:54.425] [4626.02s] [PID 8692] [gcom_mb_client] [warning ] Watchdog [input_2_1] has entered the [ALARM] state.
[2023-11-15] [18:18:55.425] [4627.02s] [PID 8692] [gcom_mb_client] [info    ] Watchdog [input_2_1] has entered the [RECOVERY] state.
[2023-11-15] [18:18:56.425] [4628.02s] [PID 8692] [gcom_mb_client] [error   ] Watchdog [input_2_1] has entered the [FAULT] state.
```

### Here are examples of event outputs

```
Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"my_client","message":"Watchdog [input_2_1] has entered the [FAULT] state.","severity":4}
Timestamp:    2023-11-15 18:23:20.425387

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"my_client","message":"Watchdog [input_2_1] has entered the [RECOVERY] state.","severity":2}
Timestamp:    2023-11-15 18:23:21.425232

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"source":"my_client","message":"Watchdog [input_2_1] has entered the [FAULT] state.","severity":4}
Timestamp:    2023-11-15 18:23:22.425439
```

### Use these commands to set the value for the reflective watchdog

```
fims_send -m pub -u /components/sel_2440 '{"input_2_1":4}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":5}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":6}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":7}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":8}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":9}'
```

# heartbeat

Config settings

```
"component_heartbeat_max_value": 500,

"component_heartbeat_timeout_ms": 2000,
"xcomponent_heartbeat_read_uri": "input_2_1",
"component_heartbeat_write_uri": "hold_2_1",
```

Output 

```

Method:       set
Uri:          /components/sel_2440/hold_2_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":20}
Timestamp:    2023-11-15 18:27:42.446745

Method:       set
Uri:          /components/sel_2440/hold_2_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":21}
Timestamp:    2023-11-15 18:27:44.446247
```

Use this config to get a reflective heartbeat.


```
"component_heartbeat_max_value": 500,

"component_heartbeat_timeout_ms": 2000,
"component_heartbeat_read_uri": "input_2_1",
"component_heartbeat_write_uri": "hold_2_1",
```

you need to set the input value on the server to get the reflective value back from the client.

Server input

```
fims_send -m pub -u /components/sel_2440 '{"input_2_1":6}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":7}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":8}'
fims_send -m pub -u /components/sel_2440 '{"input_2_1":9}'
```

Server Output after a short delay.

```
Method:       set
Uri:          /components/sel_2440/hold_2_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":7}
Timestamp:    2023-11-15 18:35:13.163134

Method:       set
Uri:          /components/sel_2440/hold_2_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":8}
Timestamp:    2023-11-15 18:35:15.162767

Method:       set
Uri:          /components/sel_2440/hold_2_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":9}
Timestamp:    2023-11-15 18:35:17.163160

Method:       set
Uri:          /components/sel_2440/hold_2_1
ReplyTo:      (null)
Process Name: SEL-2440
Username:     root
Body:         {"value":10}
Timestamp:    2023-11-15 18:35:19.163205
```

## Stats output


This is a config option

In the "connection" section   set the value for the stats uri ( the default value is shown)


```   
  "stats_pub_uri":"/stats/modbus_client", 
```

Use this command to monitor stats.

```
fims_listen -u /stats
```

Typical results.

```
Method:       pub
Uri:          /stats/modbus_client
ReplyTo:      (null)
Process Name: modbus_client_uri@SEL-2440
Username:     root
Body:         {"config_filename":"configs/client/gcom_test_client_4.json","pid":8692,"git_build":"543","git_commit":"3829d25","git_tag":"11.2.0-alpha.1","thread_connection_info": [{"id":0,"connected":true,"port":502,"time_to_connect":"0.00022575 ms","num_jobs":84,"num_fails":26}, {"id":1,"connected":true,"port":502,"time_to_connect":"0.000283476 ms","num_jobs":95,"num_fails":29}, {"id":2,"connected":true,"port":502,"time_to_connect":"0.000262313 ms","num_jobs":93,"num_fails":15}, {"id":3,"connected":true,"port":502,"time_to_connect":"0.000307936 ms","num_jobs":110,"num_fails":18}],"modbus_read_times":{"Max": 0.00166047,"Min": 3.99e-07,"Avg": 0.000336461,"Count": 239},"modbus_write_times":{"Max": 1.232e-06,"Min": 1.232e-06,"Avg": 1.232e-06,"Count": 1},"multi_pub_message_prep_times":{"Max": 0.000272579,"Min": 9.262e-06,"Avg": 0.000103025,"Count": 57},"total_pub_time_including_mb_poll":{"pub_components_comp_alt_2440":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0},"pub_components_comp_sel_2440":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0}},"fims_message_stats":{"Max": 0,"Min": 0,"Avg": 0,"Count": 5},"heartbeat_values":{"hb_components_comp_sel_2440":1},"heartbeat_stats":{"hb_components_comp_sel_2440":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0}},"watchdog_states":{"wd_components_comp_sel_2440_input_2_1":"INIT"},"watchdog_stats":{"wd_components_comp_sel_2440_input_2_1":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0}}}
```

or

```
 fims_listen -u /stats | grep "Body" | sed 's/Body: *//' | jq
```

```
{
  "config_filename": "configs/client/gcom_test_client_4.json",
  "pid": 8692,
  "git_build": "543",
  "git_commit": "3829d25",
  "git_tag": "11.2.0-alpha.1",
  "thread_connection_info": [
    {
      "id": 0,
      "connected": true,
      "port": 502,
      "time_to_connect": "0.00022575 ms",
      "num_jobs": 304,
      "num_fails": 49
    },
    {
      "id": 1,
      "connected": true,
      "port": 502,
      "time_to_connect": "0.000283476 ms",
      "num_jobs": 311,
      "num_fails": 56
    },
    {
      "id": 2,
      "connected": true,
      "port": 502,
      "time_to_connect": "0.000262313 ms",
      "num_jobs": 305,
      "num_fails": 46
    },
    {
      "id": 3,
      "connected": true,
      "port": 502,
      "time_to_connect": "0.000307936 ms",
      "num_jobs": 353,
      "num_fails": 18
    }
  ],
  "modbus_read_times": {
    "Max": 0.00208837,
    "Min": 3.99e-07,
    "Avg": 0.00043644,
    "Count": 641
  },
  "modbus_write_times": {
    "Max": 1.232e-06,
    "Min": 1.232e-06,
    "Avg": 1.232e-06,
    "Count": 1
  },
  "multi_pub_message_prep_times": {
    "Max": 0.000335435,
    "Min": 9.184e-06,
    "Avg": 0.000100442,
    "Count": 219
  },
  "total_pub_time_including_mb_poll": {
    "pub_components_comp_alt_2440": {
      "Max": 0,
      "Min": 0,
      "Avg": 0,
      "Count": 0
    },
    "pub_components_comp_sel_2440": {
      "Max": 0,
      "Min": 0,
      "Avg": 0,
      "Count": 0
    }
  },
  "fims_message_stats": {
    "Max": 0,
    "Min": 0,
    "Avg": 0,
    "Count": 5
  },
  "heartbeat_values": {
    "hb_components_comp_sel_2440": 1
  },
  "heartbeat_stats": {
    "hb_components_comp_sel_2440": {
      "Max": 0,
      "Min": 0,
      "Avg": 0,
      "Count": 0
    }
  },
  "watchdog_states": {
    "wd_components_comp_sel_2440_input_2_1": "INIT"
  },
  "watchdog_stats": {
    "wd_components_comp_sel_2440_input_2_1": {
      "Max": 0,
      "Min": 0,
      "Avg": 0,
      "Count": 0
    }
  }
}

```

# optional heartbeat status when connection is lost

 The component_connected status data can be still included in the pub output when a server is disconnected.
 no other data will be included in the pub output.



# Limited log messages for unknown io_points and invalid methods

The system will only show the first 16  log messages for invalid data points and invalid fims messages.
The stats will keep a running count of all such messages but only the first 16 will be shown.
The client will have to be restarted to reset the count.

# Timer Delayed Sync total.

The timers for pubs etc can delay the start of the next sequence based in the time taken to complete any request.
This means that if a pub is requested every 100 milliSeconds and a request takes more than 50 milliseconds to service the next pub request will be delayed by upto a complete request cycle ( up to 200ms).
This stops the incidence of server response  or network delays being compounded by requesting pubs too close together.
The _stats shown for the timers will show the number of sync delays and the accumulated sync delay time.


