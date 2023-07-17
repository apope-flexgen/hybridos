### Notes on dual bms system.

p. wilshire 03/04/2022

## revisd site layout 

Uses 3 ess_controiller instances

```
        ess

    bms_1  bms_2          
```

## Introduction

The kpp/sierra system requires the use of two bms systems associated with a single PCS.
The ess_controller (V9.3 v10.1-2 may be better) , with one bms system can use up to 64% of a single cpu.
The v10.1 ess_controller is single threaded ( for speed ) by design.

"Multithreading"  is available by running more that one executible on the same computer (SuperMicro).

The first instance (ess_1) runs the first BMS and the main ESS / PCS interface.
The second instance (ess_2) runs the second BMS ( note we may call this instance bms_2).

Both executibles (and more if needed) can run on the same hardware, each in their own memory map.

Individual start up files are used to kick off each instance 

This config sets up ess_2 and  focuses its components / site interface on
/components/ess_2 and /site/ess_2

The fims_data from /assets is recieved and blocked (dropped) where it is not needed. 


```
{
    "/sysconfig/default": 
    {
        "Help": " This is the default system config file",
        "Subs":":/components/ess_1:/assets:/system/ess_1:/site/ess_1:/shared:ess_1",
        "Config":"ess_1_dual_init",
        "EssName": "ess_1"
    }
    ,
    "/blockeduris/pub" :
    {
        "/assets/ess/summary": false,
        "/assets/ess/ess_2":true
    },
    "/blockeduris/set" :
    {
        "/assets/ess/summary":false,
        "/assets/ess/ess_2":true
    }

}
```

This config sets up ess_2 and  focuses its components / site interface on
/components/ess_2 and /site/ess_2

The fims_data from /assets is recieved and blocked (dropped) where it is not needed. 

```
{
    "/sysconfig/default": 
    {
        "Help": " This is the default system config file",
        "Subs":":/components/ess_2:/assets:/system:/site/ess_2:/shared/ess_2:",
        "Config":"ess_2_dual_init",
        "EssName": "ess_2"
    }
    ,
    "/blockeduris/pub" :
    {
        "/components/ess_1":true,
        "/system/ess_1":true,
        "/assets/ess/summary":true,
        "/assets/ess_1":true
    },
    "/blockeduris/set" :
    {
        "/components/ess_1":true,
        "/system/ess_1":true,
        "/assets/ess/summary":true,
        "/assets/ess_1":true
    }

}
```

The fims subscriptions are split to allow the systems to receive input messages specifically designated for a particular ess instance.
THe usual subscription bypass is in effect.

```
fims_send -m set -r /$$ -u /ess_1/<any uri>
fims_send -m set -r /$$ -u /ess_2/<any uri>
```


## Modbus input data


# Bms systems

Individual modbus clients are required for each set of bms racks.
The output data provided for a single ess controller that would normally be published as follows

```
/components/bms_info
```
Is now dircted to individual ess_controllers

```
/components/ess_1/bms_info
/components/ess_2/bms_info

```


Here are examples.

```
{
    "fileInfo": {
        [...]
    },
    "connection": {
        [...]
    },
    "components": [
        {
            "id": "ess_1/ems_running_info",  << data for ess_1>>
 
            "frequency": 1000,
            "offset_time": 20,
            "device_id": 1,
            "registers": [
                {

                }
        }
}
```

```
{
    "fileInfo": {
        [...]
    },
    "connection": {
        [...]
    },
    "components": [
        {
            "id": "ess_2/ems_running_info",  << data for ess_2>>
 
            "frequency": 1000,
            "offset_time": 20,
            "device_id": 1,
            "registers": [
                {

                }
        }
}
```

# Pcs system.

To direct the data to ess_1 the ids for the pcs modbus client are also adjusted

```
{
    "fileInfo": {
    },
    "connection": {
        "name": "Sungrow PCS SC1200UD",
        "ip_address": "172.30.0.20",
        "port": 5031,
    },
    "components": [
        {
            "id": "ess_1/pcs_running_info"
        }
    ]
}
```

# Site ( modbus server )

The site modbus server may have to direct its outputs to individual ess controllers.
If so, replace 
   /site/ess_hs with /site/ess_1/ess_hs 
and
   /site/ess_ls with /site/ess_1/ess_ls

The second ess_controller may simply ignore /site and subscribe to /site/ess_2 for its controls   
In which case the first ess_controller will recieve status messages from the second controller and be responsible 
for adding the data from the second controller into the first controller.

Alarm and fault data may have to be routed as well. 
To provide a combined alarm / fault system the alarm and fault data from the second controller will have to be routd through the first controller.
The Alarm / Fault clear system will also have to be routed.

The Site Controller may have to accept different bitmaps to accomdate the second BMS information. 
This will be in the hands of the site controller team.

Note that the ess_controllers are flexible enough to present and format data as required by the site_controller. 

It may be simpler for the site_controller to open a second alarms/faults/controls set of variables for interacting with the second bms.
There are good reasons for leaving the first ess_controller as a primary and letting it output controls to the second ess_controller and recieve status and alarm /fault data.




```
{"id": "dischargeable_energy", "offset": 2003, "scale": 10, "name": "Dischargeable Energy", "unit": "kWh", 
    "uri": "/site/ess_1/ess_ls" },
{"id": "chargeable_power", "offset": 2004, "name": "Chargeable Power", "unit": "kW", "signed": true, 
    "uri": "/site/ess_1/ess_ls" },
{"id": "dischargeable_power", "offset": 2005, "name": "Dischargeable Power", "unit": "kW", "signed": true, 
    "uri": "/site/ess_1/ess_ls" },
{"id": "system_state", "offset": 2006, "name": "System State", "uri": "/site/ess_1/ess_ls", "bit_strings": [ 
    "Stop", "Run", "Fault", "Standby" ] },
{"id": "com_status", "offset": 2007, "name": "Comms Status", "uri": "/site/ess_1/ess_ls", "bit_strings": 
     [ 
        "PCS Communication Offline", 
        "BMS_1 Communication Offline", 
        "EMS Communication Offline", 
        "FSS Communication Offline", 
        "BMS_1 Communication Offline", 
        ] },
{"id": "ems_alarms", "offset": 2008, "name": "External Management System Alarms", "uri": "/site/ess_1/ess_ls", "bit_strings": 
    [ 
        "FSS Fault Signal Abnormal", 
        "FSS Alarm Signal Abnormal", 
        "Lightning Protector Abnormal", 
        "Transformer High Temperature", 
        "Emergency Stop Triggered", 
        "DC Load Switch Open", 
        "Access Control Open", 
        "DC Fuse Abnormal" ] },
{"id": "ess_faults", "offset": 2009, "name": "ESS Faults", "uri": "/site/ess_1/ess_ls", "bit_strings": 
    [ 
        "BMS_1 Max Cell Voltage Threshold Exceeded", 
        "BMS_1 Min Cell Voltage Threshold Exceeded", 
        "BMS_1 Max Cell Temperature Threshold Exceeded", 
        "BMS_1 Min Cell Temperature Threshold Exceeded", 
        "BMS_1 Current Max Threshold Exceeded", 
        "BMS_1 Current Min Threshold Exceeded", 
        "BMS_1 Number of Closed-in Battery Racks Below Threshold", 
        "BMS_1 State of Health Below Threshold", 
        "PCS DC Voltage Threshold Exceeded",     
        "Number of PCS Modules Below Threshold", 
        "PCS Active Power Threshold Exceeded",
        "BMS_2 Max Cell Voltage Threshold Exceeded", 
        "BMS_2 Min Cell Voltage Threshold Exceeded", 
        "BMS_2 Max Cell Temperature Threshold Exceeded", 
        "BMS_2 Min Cell Temperature Threshold Exceeded", 
        "BMS_2 Current Max Threshold Exceeded", 
        "BMS_2 Current Min Threshold Exceeded", 
        "BMS_2 Number of Closed-in Battery Racks Below Threshold", 
        "BMS_2 State of Health Below Threshold"

 ] },
{"id": "ess_alarms", "offset": 2010, "name": "ESS Alarms", "uri": "/site/ess_1/ess_ls", "bit_strings": 
      [ "BMS Max Cell Voltage Threshold Exceeded", 
         "BMS Min Cell Voltage Threshold Exceeded", 
         "BMS Max Cell Temperature Threshold Exceeded", 
         "BMS Min Cell Temperature Threshold Exceeded", 
         "BMS Current Max Threshold Exceeded", 
         "BMS Current Min Threshold Exceeded", 
         "Number of Closed-in Battery Racks Below Threshold", 
         "BMS State of Health Below Threshold", 
         "PCS DC Voltage Threshold Exceeded", 
         "Number of PCS Modules Below Threshold", 
         "PCS Active Power Threshold Exceeded" ] },
```


## Data Interchange

The first ess_controller (ess_1) may have to combine data from itself with the second ess_controller (ess_2) to present a common data interface to the site controller.
The UI systems face a similar problem.

There are two answers to this :

1/ Route verything through the first ess_controller. Add accumulation operations where needed.
   The second ess_controller will pub or set data destined for the first ess_controller 
   (using the /ess_1 prefix or the shared data areas ).
   The remap or vlink operations will then route that data directly to the required destination.
   Where data needs to be processed before ( TotalChargeCurrent for example) the data triggers in the input data can be used to provide the accumulated totals.

   for example ess_1 has bms_1:ChargeCurrent and ess_2 has bms_2:ChargeCurrent.
   A write to either of those variables will update ess_1:TotalChargeCurrent as the sum of the two individual data items.
   ( The demo 720 test will show that)

2/ Send data directly to/from the second ess_controller.
   Some data may not need to be processed by each individual ess_controller.
   The second ess_controller can pub data directly to /site/ess_2/<somedata>
   This data will go out to the respecive modbus or UI interface without passing throuh the first ess_controller.
   Where data needs to be shared, the special shared data uris can be used between the systems.
   This does mean that a "shared" pub is required from each system.    
   Hopefully, the UI can also accept data directly from the second_ess controller either through gets or listening to the UI pubs.


   Note that when not needed the UI pubs could be turned off.
   We could keep a count of requesting UI instances  from the web_server and then turn off pubs when no UI is listening.

The ess_controllers can directly talk to each others data areas using the <ess_name> prefix.

Here is an example of the "alarm_clear" operation where ess_1 receives an alarm_clear signal and passes it on to the second ess_controller.


```
{
 "/faults/ess": {
        "clear_faults": {
            "value": "default",
            "type": "fault",
            "debug": true,
            "actions": {
                "onSet": [
                    {"remap": [{ "debug":false, "fims": "set", "uri": "/ess_2/faults/ess:clear_faults", "amap": "ess"}]},
                    {"func": [{"enable":"/sched/ess:enable","enabled":false, "func": "process_sys_alarm", "amap": "ess"}]}
                    ]
            }
        }
    }
}
```

