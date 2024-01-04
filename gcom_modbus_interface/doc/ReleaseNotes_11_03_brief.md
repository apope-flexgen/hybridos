rief###  Gcom Modbus Client Release Notes 11_03 
p. wilshire
    12_14_2023


# Modbus Client shutdown.
The client will no longer shutdown when it fails to connect to a server. The cleint will keep trying to connect and publishes an  event should the connection succeed.
The client will shutdown if it cannot read a config file or if the fims_service  is not running.

# Invalid data points
The client , by default, disables any invalid data points ( data points not defined in the server configuration).

The config option   in the "connection" section


```
"auto_disable":false
```
is used to override this default mode.

Data points are collected together into groups of sequential data points, organized by device_id, data_type  in the config, if a group contains an invalid data point, that whole group will be invalidated and the data will not appear in the output pubs.

With the default config

```
"auto_disable":true
```

Any invalid data points are "skipped" or "disabled"  and the original group broken up to bypass the invalid points.
These invalid points are probably missing in the server or the server config.
The Client output pubs will not show any values for the "disabled" points.
The Client log output will show details of the invalid points. The clinet point data can also be inspected.

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

### Manual disable / forced values

Any I/O point can be  manually diasbled the point 

```
fims_send -m set -r /$$ -u /components/comp_sel_2440/break_test_1/_disable 1
{"gcom":"Modbus Point Disable","status":"Success"}
```

The group containing that disabled point will be split up into two groups with the disabled point not included.

The value of this point can be "forced". 
This causes the disabled point will appear in the pub output with the forced value.

```
fims_send -m set -r /$$ -u /components/comp_sel_2440/break_test_1/_force 1
{"gcom":"Modbus Point Force","status":"Success"}

fims_send -m set -r /$$ -u /components/comp_sel_2440/break_test_1/_force 0
{"gcom":"Modbus Point Force","status":"Success"}

```
# Fims encryption
This option is not yet available.


# Overrides
These are used for the command line to allow "temporary" adjustments to configs


## how to find available overrides

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
sync_percent:50              set pub sync threshold percent (delays next pub request if current pub request takes too long)


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
and 

  "pub_sync": < bool true:false>

For example:


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

A  flexible (standardized) set of options are now used for fims data input
    ( bool = true, false, "true", "false", 1 or  0)

fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 '"false"'
{"gcom":"Modbus Set","status":"Success"}

When operating on a single value the uri can hold the variable name.

```
fims_send -m set -r /$$ -u /components/comp_sel_2440/coil_0 1
{"gcom":"Modbus Set","status":"Success"}
```
Or the "multi" definition with the variable name in the body can be used.

```
fims_send -m set -r /$$ -u /components/comp_sel_2440 '{"coil_0":0}'
{"gcom":"Modbus Set","status":"Success"}
```


# Examine  full io_point details.

The client will display full io_details when the /_full request is used

fims_send -m get -r /$$ -u /components/comp_sel_2440/trouble_relay/_full

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
...
}
```


## Config options to define  heartbeat.


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
            "frequency": 1000,
            "offset_time": 200,
            "device_id": 3
        }
        ]
```


# heartbeat

you need to set the input value on the server to get the reflective value back from the client.

Config settings

```
"heartbeat_enabled": true,
"modbus_heartbeat_timeout_ms": 1000,
"component_heartbeat_max_value": 500,
"component_heartbeat_timeout_ms": 2000,
"component_heartbeat_read_uri": "input_2_1",
(
    option
    "component_heartbeat_write_uri": "hold_2_1",
)
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
 fims_listen -u /stats | grep "Body" | sed 's/Body: *//' | jq

```
'{
    "config_filename":"/home/docker/configs/modbus/client/sel_2440_modbus_client_nohb.json",
    "pid":317,
    "git_build":"350",
    "git_commit":"6005c0e",
    "git_tag":"v11.3.0-beta.3",
    "thread_connection_info": [
        {"id":0,"num_jobs":0,"num_fails":0}, 
        {"id":1,"connected":true,"ip_address": "172.17.0.3","port":502,"time_to_connect":"0.000236 ms",
                "modbus_read_times":{"Max": 0.510773,"Min": 3.39e-07,"Avg": 0.00457439,"Count": 112},
                "modbus_write_times":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0},
                "num_jobs":112,
                "num_fails":0}],
        "pub_time":{
                "pub_components_comp_sel_2440":
                    {"Max": 0.511335,"Min": 0.000331498,"Avg": 0.102708,"Count": 5}
                    },
        "fims_message_stats":{"Max": 0,"Min": 0,"Avg": 0,"Count": 0},
        "fims_get_errors":0,
        "fims_set_errors":0,
        "fims_pub_errors":0,
        "timers": [
            {"name":"pub_components_comp_sel_2440","runTime" :58.02 ,"repeatTime" :1,"runs" :56,"lastRun" :57.0201,"syncCount" :0,"totalSyncTime" :0,"syncPct" :0}
            ]
}'
```



# Limited log messages for unknown io_points and invalid methods

The system will only show the first 16  log messages for invalid data points and invalid fims messages.
The stats will keep a running count of all such messages but only the first 16 will be shown.
The client will have to be restarted to reset the count.


