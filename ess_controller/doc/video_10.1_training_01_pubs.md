### Ess Controller v 10.1  Pubs

This document accompanies the v10.1_01 training video

## Introduction

This video shows the preferred "method" of setting up the pub operations, specifically to the site controller.
There are other ways of seting up and running system commands but this is the "new" 10.1 option. 
It is intended as an example of how to set the system to publish data to , in this case, the site controller.

It covers the new "-f" command line option used to start the ess_controller for a  specific function. IN this case the main ess controller.
The dual risen system will, infact , have three ess_contrllers.
One acting as the main interface to the PCS, two bms units, the UI and the site controller.

Th two others will connect to each bms unit. This design will aim to 
allow (if possible)   the action of simply starting a third bms unit to cause the third unit to be integratd in the design.
In all cases NO source code changes to the v101 ess_controller are expected to make this  sort of system work.

## Running the controller

You'll need a file based config file.

Here is an example ...

```
/tmp/ess_controller/ess_temp_file.json
{
    "/sysconfig/default":
    {
        "Help": " This is the default system config file",
        "Subs":":/components/ess:/assets:/system/ess:/site/ess:",
        "Config":"ess_init",
        "EssName": "ess"
    }
    ,
    "/blockeduris/pub" :
    {
        "/assets/ess/summary":false,
        "/assets/ess/bms_1":true,
        "/assets/ess/bms_2":true
    },
    "/blockeduris/set" :
    {
        "/assets/ess/summary":true,
        "/assets/ess/bms_1":true,
        "/assets/ess/bms_2":true
    }
}
```

Note this is in a non-standard location , no problem we can force the use of this directory
But you must specify the directory first in the command line.

Typical command line 

```
ess_controller -d /tmp/ess_controller -f ess_temp_file

```

To get the full dual_bms system running the following sequence could be used.


```
ess_controller -d /usr/local/etc/configs/ess_controller -f ess_temp_file
ess_controller -d /usr/local/etc/configs/ess_controller -f bms_1_file
ess_controller -d /usr/local/etc/configs/ess_controller -f bms_2_file

```

# What the initial config file sets up

Thse are the important fields in the config file....

```
"Subs":      < thes are the default fims messages the ess_controller will listen to>
"Config":    < this is the first config file required by the system>
"EssName":   < this is the name usd to identify the system>
```

The Subs files will have the /<EssName>: item prepnded to the list of subs.
This is used to bypass the fims subs system and allow fims mssages to b snt dirctly to the EssName ess_controller.
After reciving a fims message to "/<EssName>/status/ess" the EssName prefix is removed and the internal message transformed to "/status/ess".


```
{
    "/sysconfig/default":
    {
        "Help": " This is the default system config file",
        "Subs":":/components/ess:/assets:/system/ess:/site/ess:",
        "Config":"ess_init",
        "EssName": "ess"
    }
}
```


## ess_init

This is the file that sets up the ess_controller configuration loader.

The ess_controller can have one or more configuration loaders for each part of the system. "PCS, BMS_1, BMS_2".

Thes loaders are included in the files delivered to the initial loader.

Here is a typical "ess_init"  segment.

```
{
    "/config/load": {
        "ess_controller": {
            "value":false,
            "file":"ess_controller",
            "aname":"ess",
            "final": "ess_final",
            "new_options": [
                { "file":"ess_manager",      "aname":"ess"   },
                { "xfile":"bms_manager",      "aname":"bms",   "pname":"ess"  },
                { "xfile":"bms_1_manager",    "aname":"bms_1", "pname":"bms"  },
                { "xfile":"bms_2_manager",    "aname":"bms_2", "pname":"bms"  },
                { "xfile":"pcs_manager",      "aname":"pcs",   "pname":"ess"  },
                { "xfile":"site_manager",     "aname":"site",  "pname":"ess"  }
            ]
        }
    }
}
```

By nature of this iniital design for the loader it expects  the "ess_controller"  component to arrive first  
Followed by a list of other components.

These components are normally supplied by the dbi system but the following script can be used to simulate that system.

```
#!/bin/sh
# p wilshire 03-07-2022
# script to send start up files to the triple ess_controllers.
 
fims_send -m set -f configs/dbi/ess_controller1/ess_init.json        -u /ess/cfg/cfile/ess/ess_init
fims_send -m set -f configs/dbi/ess_controller1/ess_controller.json  -u /ess/cfg/cfile/ess/ess_controller
fims_send -m set -f configs/dbi/ess_controller1/ess_manager.json     -u /ess/cfg/cfile/ess/ess_manager
#                { "file":"bms_manager",      "aname":"bms",   "pname":"ess"  },
#                { "file":"bms_1_manager",    "aname":"bms_1", "pname":"bms"  },
#                { "file":"bms_2_manager",    "aname":"bms_2", "pname":"bms"  },
#                { "file":"pcs_manager",      "aname":"pcs",   "pname":"ess"  },
#                { "file":"site_manager",     "aname":"site",  "pname":"ess"  }

```



## Ess Controller segment

This config segment contains some essential data used by the ess_controller

This data will have to be changed for different instances of the controller running on the same system.

```
{
 "/config/ess": {
    "LogDir": "/var/log/ess_controller",
    "essCommsFaultTimeout":        {"value": 10},
    "essHeartbeatFaultTimeout":    {"value": 15},
    "essCommsAlarmTimeout":        {"value": 7},
    "essCommsRecoverTimeout":      {"value": 5},
    "AlarmDestination":            {"value": "/assets/ess/summary:alarms"},
    "FaultDestination":            {"value": "/assets/ess/summary:faults"}
  },

  "/logs/ess": {
    "CheckAmCommsLog": {"value": "file:CheckAmCommsLog.txt","enableLog": true },
    "MonitorVarLog":   {"value": "file:MonitorVarLog.txt",  "enableLog": true}
  }
}
```

The rest of this config segemnt is used to define the UI interface for the controller.
Here is a sample

```
{
    "/assets/ess/summary": {
        "name":"ESS Summary",
        "version":             {"name": "Version",             "value": "version","enabled": true},
        "max_charge_power":    {"name": "Max Charge Power",    "value": 0,        "enabled": false},
        "max_discharge_power": {"name": "Max Discharge Power", "value": 0,        "enabled": false}
    }
}

```


## Ess Manager segment

This is the segment that contains the operation controls for the system.

In addition to defining variables used by the controller , the scheduler , data mapping , publish , monitoring and command/control sequnces can all be in this segment.
The loader allows multiple segments to be defined , some like templates, have special functions.

In particular, this demo, will focus on the part of the config segment used to get the "/site/ess_hs" and "/site/ess_ls" pubs running.

# Ess Scheduler

This is a slightly complex beast. Many options are built into it.
To ease the interface to the scheduler the system has some "Built In" commands that can be activated to trigger the scheduler.
Why do we need the scheduler ?
Well, we want to pub "/site/ess_hs" and "/site_ess_ls"  data at predefined intervals. We dont want to hard code the schedule intervals , let the config options do it all.

The Simplified Scheduler interface has to be set up. In this example we have decided to put these functions into the "/system/commands" data area ( which we create just by defining it in the config file).

Here is a config file segment.

```
"/system/commands":{
    "run": {
        "help": "run a schedule var needs the uri to be set ",
        "value":0,
        "uri":"/control/ess:runMonBms",
        "every":0.1,
        "offset":0,
        "debug":false,
        "ifChanged":false, "enabled":true, 
        "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
  
      }
}
```

In this case the scheduler is being asked to send the current time , in seconds, to "/control/ess:runMonBms" every 0.1 seconds.

To use the same "/system/commands:run" command for different variables the config system would have to send different data to the same "/system/commands:run" uri.
Normally a json file cannot do that.

```
{
    "/system/commands":{
        "run": {"value":0,"uri":"/sched/ess:pubSiteHs","every":0.1,"offset":0},
        "run": {"value":0,"uri":"/sched/ess:pubSiteLs","every":1.0,"offset":0}
    }
}

```

So the answer is to create another build-in function to do that.

```
{
    "/system/commands":{
        "runOpts": {
        "value":true,
        "enabled":false,
        "targav":"/system/commands:run",
        "new_options": [
          {"uri":"/sched/ess:pubSiteHs","aname":"ess","value":0,"every":0.1,"offset":0},
          {"uri":"/sched/ess:pubSiteLs","aname":"ess","value":0,"every":1.0,"offset":0}
        ],
        "actions":{"onSet":[{"func":[{"func":"SchedItemOpts"}]}]}
      }
    }
}
```


When we write a value to "/system/commands:runOpts@enabled true" followed by "/system/commands:runOpts true"
The repeated sets to "/system/commands:run" will be executed and the different components activated.

## "/sched/xxx" control vars

Now we have the designated control vars "/sched/ess:pubSite{H|L}s" set up to be triggered  periodically by the scheduler.

Its time to make these variables do something.

We do that by adding an "action:onSet" structure to the variables. This is one of the control mechanisms available in the ess_controller. 

(The 10.1 release moves a lot more of the system to running entirely from config statements. 
We will be transitioning the older 9.2 systems (tx100) to the newer format over time. However, the original 9.2 format ( rigid timings and system model ) are still supported. 
In fact both systems can run at the same time. 
The 9.2 release will not trigger an initial dbi based load but the 10.1 loader system is still in place.)

Let's define the "/sched/ess:pubSitexx" variables.

```
{
    "/sched/ess": {
        "pubSiteHs" : {
            "value":0,
            "mode":"naked",
            "table":"/site/ess_hs",
            "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}
    },
        "pubSiteLs" : {
            "value":0,
            "mode":"naked",
            "table":"/site/ess_ls",
            "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}
        } 
    }
}
```

These config segments can be added into the ess_manager.json config component."

Hopefully, their function is self explanitory.
The "built-in" function "RunPub" is executed with "params" "table" and "mode" defined.

NOTE an additional param "sendas" is also available to allow the actual pub data name to be changed from "/site/ess_ls" to a different name.

```
{
    "/sched/ess": {
        "pubSiteHs" : {
            "value":0,
            "mode":"naked",
            "table":"/site/ess_hs",
            "sendas":"/site/ess_1/ess_hs",
            "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}
    },
        "pubSiteLs" : {
            "value":0,
            "mode":"naked",
            "table":"/site/ess_ls",
            "sendas":"/site/ess_1/ess_ls",
            "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}
        } 
    }
}
```

Last thing we need is some dummy data. As a temporary measure we'll set up some.
The "site_controller" interface is rigidly defined from the modbus_server config file.
In fact the actual modbus config file can be used to define the interface. 
This is covered in a later session.

As a stop gap measure simply add some dummy data in the ess_manager config segment.
We could even cause that data to "magically" appear in ess_final.


```
{
    "/site/ess_hs": {
        "HsVal1":123,
        "HsVal2":4456
    },
  
    "/site/ess_ls": {
        "LsVal1":123,
        "LsVal2":4456
  }
}
```


## ESS Final segment

This is the ess_final config item.


```
{
    "/doc/99ess": {
        "notes1":" This is the end of the ess loader process",
        "notes2":" Run the SchedItemOpts list attached to runOpts ",
        "notes3":" ... ",
        "notes4":"     you can also add any other required options to the runOpts variable  at this time"

    },

    "/site/ess_hs": {
        "HsVal1":123,
        "HsVal2":4456
    },
  
    "/site/ess_ls": {
        "LsVal1":123,
        "LsVal2":4456
    },

   "/system/commands":{
       "runOpts": {
        "value":false,
        "enabled":true
       }
   },
   "/sched/ess":{
        "pubSiteHs":{
            "value":0,
            "enabled":false
        }
    }
}
```

## The result

Here is system output observed using fims_listen.

```
fims_listen

Method:    pub
Uri:       /site/ess_ls
ReplyTo:   null
Body:      {"LsVal1":123,"LsVal2":4456}
Timestamp: 2022-03-08 18:58:27.394029

Method:    pub
Uri:       /site/ess_ls
ReplyTo:   null
Body:      {"LsVal1":123,"LsVal2":4456}
Timestamp: 2022-03-08 18:58:28.393928
```


## Conclusion

So in this  training session we have covered the following:

* starting the ess_controller using the new "-f" command line option.
* setting the directory ("-d") to use for the file config in the same command line (before the "-f" option) 
* contents of the initial file config , setting the ess_controller name, setting up fims Subscriptions, defining the ess_init segment.
* using a script to simulate the action of the dbi based loader.
* Using the "ess_init" to define a "loader" for the rest of the system.
* "simple" loader options to load the interface spec and the management spec.
* Defining a system control to run the scheduler.
* Defining a system control to run a (config file) defined list (SchedItemOpts) of schedule target variables.
* Setting up one or more  "publish" control variables to use with the  "RunPub" built in function.
* Enabling and Disabling the pub functions.
* setting up some dummy pub data
* redirecting the pub function to a different Url.

# Next 

We'll cover the way assets data gets to appear on the UI.

