### Ess Controller v 10.1  Interface

This document accompanies the v10.1_03 training video

## Introduction

This video shows how the data from the modbus or dnp3 interface is used by the ess_controller. 

The ess_controller will automatically accept all incoming data. 
The controller has to have subscribed to that data source in the initial config file.
You have to use the "blockeduris" system to ignore selected data sources.
For these uri's the fims data will be received but dropped without procesing. 

The site_controller interface, however, expects a defined data structure.
The ess_controller usesa specific load instrution to configure itsself to intrface site_controller by 
parsing the same modbus_server configuration file used by the site_controller
This ensures that the data maps match and cuts out the need for yet another config file.
In 10.2 we'll try to use the actual modbus_server config file from dbi. For now we ask for a copy.

For this initial training we'll use simulated modbus_data at first to get the system setup.
The live interface  will be introduced afer we are familiar with the data mapping process.
I strongly suggest that when setting up and testing the system we'll use simple (if possible) simulations of the real data to test things out.

## Getting started 

You need this version ( or a later version on the same branch)

```
 ess_controller -v
             git_branch: test/dual_risen_ess
             git_commit: 0d05b8c
             build:      v10.1-936

```
Make sure you are linked to the correct set of config files.

```
ls -l configs/dbi/
total 0
lrwxrwxrwx 1 root root   15 Mar  9 13:41 ess_controller -> ess_controller3
drwxrwxrwx 1 root root 4096 Mar  9 21:49 ess_controller1
drwxrwxrwx 1 root root 4096 Mar  9 21:49 ess_controller2
drwxrwxrwx 1 root root 4096 Mar  9 21:49 ess_controller3
```

These commands can help set up the correct config dir.

```
rm configs/dbi/ess_controller
ln -sf ess_controller3 configs/dbi/ess_controller
```

Run the ess_controller as follows.

```
 ess_controller  -f ess_file

[5.73837s  ] [info    ] [sendEvent      ] sending event  xxx >>>>>>>>>>>>>>>> [{"source":"ess","message":"ess starting  at 0.705","severity":1}]
[5.73867s  ] [info    ] [runConfig      ]  requesting cfile [ess_init]
[5.73892s  ] [info    ] [handleCfile    ]  Running Loader
[5.74019s  ] [info    ] [setupControls  ] Created Fims var [/sched/fims:dummy]
[5.7403s   ] [info    ] [setupControls  ] Created Runvar var [/control/ess:runTime]
[5.74031s  ] [info    ] [setupControls  ] Created Stop var [/control/ess:stopTime]
[5.74041s  ] [info    ] [schedThread    ] this is a replacement schedItem 0x1915340
[5.74051s  ] [info    ] [schedThread    ] schedItem deleted, seting repTime to 0.0

```


Load the demo configs.

```

sh  scripts/dual_bms/load_dual_bms.sh
sh  scripts/dual_bms/run_dual_bms.sh

 ```


 Check that its running

 ```
 fims_listen

Method:    pub
Uri:       /site/ess_ls
ReplyTo:   null
Body:      {"LsVal1":123,"LsVal2":4456,"chargeable_power":0,"dischargeable_power":0}
Timestamp: 2022-03-10 12:30:46.547513

Method:    pub
Uri:       /site/ess_ls
ReplyTo:   null
Body:      {"LsVal1":123,"LsVal2":4456,"chargeable_power":0,"dischargeable_power":0}
Timestamp: 2022-03-10 12:30:47.547429

```


Now we will work through the system.




## Sample Modbus Data

Here is a example of a small "modbus_client" data stream.

```
{
    "/components/ess/pcs_runing_info" :{
        "status":"init",
        "current": 295,
        "voltage": 1325,
        "alarms": 0,
        "faults":0
    }
}
```

That is providing that the fims subs are set up correctly.

```
fims_send -m get -r /$$ -u /ess/full/sysconfig/default/Subs | jq
{
  "Subs": {
    "value": ":/components/ess:/assets:/system/ess:/site/ess:"
  }
}
```

Good its all set up.

Simply "pub" this data with an ess_controller running and the data will arrive in the ess_controller.


```
fims_send -m pub -u /components/ess/pcs_running_info '{
   "status":"init",
        "current": 295,
        "voltage": 1325,
        "alarms": 0,
        "faults":0
}'

```


And check the result 

```
fims_send -m get -r /$$  -u /ess/components/ess/pcs_running_info  | jq
{
  "alarms": {
    "value": 0
  },
  "current": {
    "value": 295
  },
  "faults": {
    "value": 0
  },
  "status": {
    "value": "init"
  },
  "voltage": {
    "value": 1325
  }
}

```

This data has arrived in the ess_controller.

# Simple Data Handling


We are going to do a couple of things with our shiny new data.

Lets run an enum on faults. 
Assume the following:

* faults 
* 0 = no faults
* 1 = over temp fault
* 2 = comms failure
* 4 = door open

None or all of these faults may be present.


Define an "enum" action on the "/ess/components/ess/pcs_running_info:faults" variable
```
{
"/ess/components/ess/pcs_running_info":{
    "faults":{
        "value":0,        
        "actions":{"onSet":[{"enum":[
             {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs_enum:OverTemp",     "outValue": "Clear"},
             {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs_enum:CommsFailure", "outValue": "Clear"},
             {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs_enum:DoorOpen",     "outValue": "Clear"},
             {"shift": 0, "mask": 1, "inValue": 1, "uri": "/status/pcs_enum:OverTemp",       "outValue": "Fault"},
             {"shift": 1, "mask": 1, "inValue": 1, "uri": "/status/pcs_enum:CommsFailure",   "outValue": "Fault"},
             {"shift": 2, "mask": 1, "inValue": 1, "uri": "/status/pcs_enum:DoorOpen",       "outValue": "Fault"}
        ]}]}}
    }
}
```

Next you can send different values to the "/ess/components/ess/pcs_running_info:faults" variable and see the faults generated.


```
sh scripts/dual_bms/test_modbus.sh 0
{
  "/status/pcs_enum": {
    "CommsFailure": "Clear",
    "DoorOpen": "Clear",
    "OverTemp": "Clear"
  }
}


```


```
sh scripts/dual_bms/test_modbus.sh 1
{
    "/status/pcs_enum": {
    "CommsFailure": "Clear",
    "DoorOpen": "Clear",
    "OverTemp": "Fault"
  }
}


sh scripts/dual_bms/test_modbus.sh 2
{
    "/status/pcs_enum": {
    "CommsFailure": "Falut",
    "DoorOpen": "Clear",
    "OverTemp": "Fault"
  }
}

sh scripts/dual_bms/test_modbus.sh 4
{
  "/status/pcs_enum": {
    "CommsFailure": "Fault",
    "DoorOpen": "Fault",
    "OverTemp": "Fault"
  }
}

```

# Some points of interest for enum

* the /status/pcs_enum  items were created by the initial write of 0 to 
        "/ess/components/ess/pcs_running_info/faults"
* the faults latch and have to be reset by writing 0 to
        "/ess/components/ess/pcs_running_info/faults"
* multiple faults can be detected in the single variable.
* use shift to move all the bits to the left



## "remap" : the swiss army knife

The "enum" action is a efficient way to break apart and decode complex input values from pereipheral components.

The other major tool in handling variables in the "remap" action.

Basically the "remap" action is saying  this.

When you set a value (or a change a  value) to any variable I want to also change  the value in a different variable.

For example: 

 When I get an ESTOP  input I want to shut down the  PCS and BMS subsystems.

 The exact details and options available for this "remap" action  are extensive( but they are all documented)

 Writing a value to a selected variable that triggers the action. 
 There is an option ("ifChanged":true) to only trigger actions if the variable value has changed.

 We can then take the value of that variable, or any other designated input variable (inAv)
 optionally compare it with another value (inValue, inAv) and if it matches write either , the  
    the original incoming value or a selected output value (outAv, outValue, outTime) to a different uri, using a fims message if required.

 The whole process can be enabled by a true/false status of an "enable" variable.


 Here are a selection of the many remap options


 ```
 {
   "/status/pcs_1":{
      "FaultShutdown": {
        "value": false,
        "actions": { 
          "onSet":[{
              "remap":[
                  {"uri": "/status/pcs_1:FaultShutdown"},
                  {"uri": "/pcs_1/status/pcs_1:FaultShutdown", "fims":"set"},
                  {"inValue": true, "uri": "/status/pcs_1:FaultShutdownisTrue"},
                  {"inVar": "/status/pcs_1:FaultValue", "uri": "/status/pcs_1:FaultShutdownmatchFaultValue"},
                  {"inAv": "/status/pcs_1:FaultInValue", "uri": "/status/pcs_1:FaultinValue"},
                  {"inValue": true, "uri": "/status/pcs_1:HardShutdown", "outValue": false},
                  {"inValue": false, "uri": "/status/pcs_1:FullShutdownTime", "outTime": 5},
                  {"inValue": true, "uri": "/sched/pcs_1:schedShutdownPCS@endTime", "outValue": 0},
                  {"inValue": true, "uri": "/sched/pcs_1:schedShutdownPCS", "outValue": "schedShutdownPCS"},
                  {"enable": "/sched/pcs_1:FaultEnable", "uri": "/sched/pcs_1:schedShutdownESS", "outValue": "schedShutdownPCS"}
              ]
          }]
        }
      }
  }
}
```

They are ( as shown) :

  * send the incoming value to a different destination
  * send the incoming value to a different destination via fims
  * if the incoming value is "true" then send the incoming value to a different uri
  * if the incoming value matches the value  in the inVar  then send the incoming value to a different uri
  * send the value from a diffferent varaible (inAv) to a different uri ( can be combined with inValue)
  * if the incoming value is "true"  then send the specified outValue (or outTime) to a different uri
  * if the incoming value is "false"  then send the time plus 5 seconds to a different uri
  * if the incoming value is "true"  then send the specified outValue  to a different uri's param
  * if the incoming value is "true"  then trigger  the shutdown action task.
  * if the enable var is  "true"  then trigger  the shutdownEss action task.



Here it is running
```
fims_send -m set -r /$$ -u /ess/full/status/pcs_1/FaultShutdown '
{
    "value": false,
    "actions": {
      "onSet":[{
          "remap":[
              {"uri": "/status/pcs_1:FaultShutdown"},
              {"uri": "/status/pcs_1:FaultShutdown", "fims":"set"},
              {"inValue": true, "uri": "/status/pcs_1:FaultShutdownisTrue"},
              {"inVar": "/status/pcs_1:FaultValue", "uri": "/status/pcs_1:FaultShutdownmatchFaultValue"},
              {"inAv": "/status/pcs_1:FaultInValue", "uri": "/status/pcs_1:FaultinValue"},
              {"inValue": true, "uri": "/status/pcs_1:HardShutdown", "outValue": false},
              {"inValue": false, "uri": "/status/pcs_1:FullShutdownTime", "outTime": 5},
              {"inValue": true, "uri": "/sched/pcs_1:schedShutdownPCS@endTime", "outValue": 0},
              {"inValue": true, "uri": "/sched/pcs_1:schedShutdownPCS", "outValue": "schedShutdownPCS"},
              {"enable": "/sched/pcs_1:FaultEnable", "uri": "/sched/pcs_1:schedShutdownESS", "outValue": "schedShutdownPCS"}
          ]
      }]
    }
}'
```

And the result
```
fims_send -m get -r /$$ -u /ess/full/status/pcs_1  | jq
{
  "FaultShutdown": {
    "value": false,
    "actions": {
      "onSet": [
        {
          "remap": [
            {
              "aVal": false,
              "uri": "/status/pcs_1:FaultShutdown"
            },
            {
              "aVal": false,
              "fims": "set",
              "uri": "/status/pcs_1:FaultShutdown"
            },
            {
              "aVal": false,
              "inValue": true,
              "uri": "/status/pcs_1:FaultShutdownisTrue"
            },
            {
              "aVal": false,
              "inVar": "/status/pcs_1:FaultValue",
              "uri": "/status/pcs_1:FaultShutdownmatchFaultValue"
            },
            {
              "aVal": false,
              "inAv": "/status/pcs_1:FaultInValue",
              "uri": "/status/pcs_1:FaultinValue"
            },
            {
              "aVal": false,
              "inValue": true,
              "outValue": false,
              "uri": "/status/pcs_1:HardShutdown"
            },
            {
              "aVal": false,
              "inValue": false,
              "outTime": 5,
              "outValue": 20635.868649959564,
              "uri": "/status/pcs_1:FullShutdownTime"
            },
            {
              "aVal": false,
              "inValue": true,
              "outValue": 0,
              "uri": "/sched/pcs_1:schedShutdownPCS@endTime"
            },
            {
              "aVal": false,
              "inValue": true,
              "outValue": "schedShutdownPCS",
              "uri": "/sched/pcs_1:schedShutdownPCS"
            },
            {
              "aVal": false,
              "enable": "/sched/pcs_1:FaultEnable",
              "outValue": "schedShutdownPCS",
              "uri": "/sched/pcs_1:schedShutdownESS"
            }
          ]
        }
      ]
    }
  },
 
  "FaultShutdownmatchFaultValue": {
    "value": false
  },
  "FaultinValue": {
    "value": false
  },
  "FullShutdownTime": {
    "value": 20635.868649959564
  }
}

fims_send -m get -r /$$ -u /ess/full/sched/pcs_1  | jq
{
  "schedShutdownESS": {
    "value": "schedShutdownPCS"
  }
}
```

If the value is set as "true" we get even more variables

```
fims_send -m get -r /$$ -u /ess/full/sched/pcs_1  | jq
{
  "schedShutdownESS": {
    "value": "schedShutdownPCS"
  },
  "schedShutdownPCS": {
    "value": "schedShutdownPCS",
    "endTime": 0
  }
}
```

We also see an error message

```
[20635.9s  ] [error   ] [setOutValue    ] Warning possible loopbackvar [/status/pcs_1:FaultShutdown]
[21121.1s  ] [error   ] [setOutValue    ] Warning possible loopbackvar [/status/pcs_1:FaultShutdown]
```

It looks like the system is trying to cause a infinite loop by writing to itself . The ess_controler detected this and prevented the loop.


Here is a corrected set of actions, we can reload the built in actions for "FaultShutdown" from the command line.


```



fims_send -m set -r /$$ -u /ess/full/status/pcs_1/FaultShutdown '
{
    "value": false,
    "actions": {
      "onSet":[{
          "remap":[
              {"uri": "/status/pcs_1:FaultShutdownSeen"},
              {"uri": "/bms_1/status/pcs_1:FaultShutdown", "fims":"set"},
              {"inValue": true, "uri": "/status/pcs_1:FaultShutdownisTrue"},
              {"inVar": "/status/pcs_1:FaultValue", "uri": "/status/pcs_1:FaultShutdownmatchFaultValue"},
              {"inAv": "/status/pcs_1:FaultInValue", "uri": "/status/pcs_1:FaultinValue"},
              {"inValue": true, "uri": "/status/pcs_1:HardShutdown", "outValue": false},
              {"inValue": false, "uri": "/status/pcs_1:FullShutdownTime", "outTime": 5},
              {"inValue": true, "uri": "/sched/pcs_1:schedShutdownPCS@endTime", "outValue": 0},
              {"inValue": true, "uri": "/sched/pcs_1:schedShutdownPCS", "outValue": "schedShutdownPCS"},
              {"enable": "/sched/pcs_1:FaultEnable", "uri": "/sched/pcs_1:schedShutdownESS1", "outValue": "schedShutdownESS"},
              {"uri": "/sched/pcs_1:schedShutdownESS2", "outValue": "schedShutdownESS"}
          ]
      }]
    }
}'
```

Note that this set of actions also sent a fims message to "bms_1"

```
Method:    set
Uri:       /bms_1/status/pcs_1
ReplyTo:   null
Body:      {"FaultShutdown":{"value":true}}
Timestamp: 2022-03-09 08:16:09.185498
```


Here is a demo of the "enable" flag.

```
fims_send -m get -r /$$ -u /ess/full/sched/pcs_1  | jq
Receive Timeout.  << no data here yet  

<< enable the "flag">>
fims_send -m set -r /$$ -u /ess/full/sched/pcs_1/FaultEnable true  | jq
{  
  "FaultEnable": true
}

<< send the FaultShutdown message again>>
fims_send -m set -r /$$ -u /ess/full/status/pcs_1/FaultShutdown true
{"FaultShutdown":true}

<< inspect the data again >>
fims_send -m get -r /$$ -u /ess/full/sched/pcs_1  | jq
{
  "FaultEnable": {
    "value": true
  },
  "schedShutdownPCS": {
    "value": "schedShutdownPCS",
    "endTime": 0
  }
}

```

# Some points of interest for remap

* options range from simple to complex
* some protection for loop locks
* can remap based on values , enables, other variable values
* can select outValue from the inAv (possibly redirected), fixed (outValue,outTime), indirect (outVar) and you can send it via fims.
* values used can be fixed , from variable , from variable@param (v10.2)
* the whole action line can be disabled 

Remap can do complex operations. Keep it simple at first but then develop more, as needed and  test test test.


## "func" :  if you are still missing something

The "func" action can trigger one of the built_in functions embedded in the ess_controller.
There are many 
Here is a basic list,

Running one function can add more functions to the list at run time.
All functions are "compiled in" options, they cannot be changed outside of a hotfix, bugfix or new release.
Currently, once a function has been defined, it can remain in the system indefinitely.
There is a minimal overhead (code space only) in keeping older functions in place.


Here is a basic list.
More can be added , at runtime, by running other functions.
This allows the system to be upgraded  without affecting current running configurations.



```
{
  "/system/functions": {
    "AggregateManager": true,
    "CalculateVar": true,
    "CheckDbiResp": true,
    "CheckDbiVar": true,
    "CheckMonitorVar": true,
    "CheckMonitorVar_v2": true,
    "CheckTableVar": true,
    "DeratePower": true,
    "DumpConfig": true,
    "EssSystemInit": true,
    "Every1000mS": true,
    "Every100mSP1": true,
    "Every100mSP2": true,
    "Every100mSP3": true,
    "FastPub": true,
    "GPIOCalcResponse": true,
    "HandleCmd": true,
    "HandlePowerCmd": true,
    "HandlePowerEst": true,
    "HandlePowerRackEst": true,
    "HandleSchedItem": true,
    "HandleSchedLoad": true,
    "LoadClient": true,
    "LoadConfig": true,
    "LoadServer": true,
    "LogDebug": true,
    "LogError": true,
    "LogInfo": true,
    "LogIt": true,
    "LogWarn": true,
    "MakeLink": true,
    "MathMovAvg": true,
    "RunAllALists": true,
    "RunAllLinks": true,
    "RunAllVLinks": true,
    "RunLinks": true,
    "RunMonitor": true,
    "RunMonitorList": true,
    "RunPub": true,
    "RunSched": true,
    "RunScript": true,
    "RunSysVec": true,
    "RunSystemCmd": true,
    "RunTpl": true,
    "RunVLinks": true,
    "SchedItemOpts": true,
    "SendClearFaultCmd": true,
    "SendDb": true,
    "SendDbiVar": true,
    "SendTime": true,
    "SendTrue": true,
    "SetDbiDoc": true,
    "SetMapping": true,
    "ShutdownBMS": true,
    "ShutdownPCS": true,
    "SimHandleBms": true,
    "SimHandleHeartbeat": true,
    "SimHandlePcs": true,
    "SimHandleSbmu": true,
    "SlewVal": true,
    "SlowPub": true,
    "SlowPubOne": true,
    "StartupBMS": true,
    "StartupPCS": true,
    "StopSched": true,
    "TestTriggerFunc": true,
    "UpdateSysTime": true,
    "UpdateToDbi": true,
    "process_sys_alarm": true
  }
}
```

Too many(70+) to discuss in this session, we'll  cover some of these later.

For now this is  how we set up some "func" actions.


```
{
"/sched/essRun":{  
    "timer100mS": {
      "value":0,    
        "actions": {
          "onSet": [{"func": [
                      {"func":"UpdateSysTime","aname":"ess"},       
                      {"func":"RunInitCheck","aname":"ess"},
                      {"func":"RunComsCheck","aname":"ess"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_mbmu_control_r:mbmu_max_cell_voltage",     "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_mbmu_control_r:mbmu_min_cell_voltage",     "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_mbmu_control_r:mbmu_max_cell_temperature", "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_mbmu_control_r:mbmu_min_cell_temperature", "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_mbmu_control_r:mbmu_soc",                  "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_mbmu_control_r:mbmu_soh",                  "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_bms_ems_r:bms_heartbeat",                  "enable": true,"aname":"bms"},
                      {"func":"CheckMonitorVar","Av":"/components/catl_bms_ems_r:bms_timestamp",                  "enable": true,"aname":"bms"},   
                      {"func":"HandlePowerLimit","aname":"ess"},
                      {"func":"HandlePowerCmd","aname":"ess"}
        
                      ]
                  }
              ]
          }
      }
  }
}
```

## Global "enable"

But there is a problem.
All the functions try to run when the config is loaded.
Luckily you can add a global "enable" flag.


```
"enabledFaultShutdown": {
      "enable": "/controls/pcs:enableShutdown",
      "value": false,
      "actions": { 
        "onSet":[{
            "remap":[
                {"uri": "/status/pcs_2:FaultShutdown"},
```

Before loading this config segment , define the flag as false and none of the "actions" are run.

```
{
  "/controls/pcs_1":{
    "enableShutdown":false
  }
}
```

Then in the "ess_final" config you can simply tuen on the enable.
```
{
  "/controls/pcs_1":{
    "enableShutdown":true
  }
}
```
Once "enabled"  the "actions" can be triggered as follows

```
from command line 

 fims_send -m set -r /$$ -u /ess/full/status/pcs_1/enabledFaultShutdown true

or use a "remap" action.

```
## What is an "aname"

We'll cover the "aname" concept in a different session


## UpdateSysTime Demo

About that UpdateSysTime , lets run that.

```
fims_send -m set -r /$$ -u /ess/sched/essRun/setTime '
{
  "value":0,    
  "actions": {
      "onSet": [{"func": [
                      {"func":"UpdateSysTime","aname":"ess"}
      ]}]}     
}'

fims_send -m get -r /$$ -u /ess/naked/status/ess  | jq
{
  "/status/ess": {
    "Day": 9,
    "Heartbeat": 2,
    "Hour": 9,
    "Min": 16,
    "Month": 3,
    "Sec": 31,
    "Year": 2022,
    "build": "v10.1-909",
    "git_branch": "release/v10.2_base",
    "git_commit": "7f00570",
    "tNow": 25198.748963832855,
    "timeString": "Wed Mar  9 09:16:31 2022."
  }
}
```

# Some points of interest for func

* prebuilt into the system (as a shared lib)
* much reduced code effort to produce a new function. Minimal system impact on a hotfix, bugfix or release.
* easily attached to an variable
* can be used like plug in modules

## Lets not copy data all over the place.

The problem comes where you have a process where you have several thousand data points from a system that needs to be simply passed through the system

For example 

  ChargeCurrent arrives from the BMS  System, 
     it needs to be passed on to the site controller as a different data point.


The ess_controller fixes this situation in one of two ways.

# Vlinks

These are used where the same value is used by two different variables.
The two variables may have different params but they share the same value.

Consider these two values

```
{
  "/state/ess_hs": {
    "pcsCurrentCharge": {
      "value":231,
      "maxval":345,
      "minval":123
    } 
  }
}
```

```
{
  "/components/pcs_general_info": {
    "current_charge": {
      "value":1231
    } 
  }
}
```


One is being sent to the "site_controller" as part of the pub to that system.
This other is an input from the modbus_client connected to the pcs system.

In larger systems many thousands of variables may be linked together in this manner.

When you write a value to one it must be also written to the other.
Adding the code to copy data items is not difficult but maintianing that list is a non trivial task.
Much better if it was all done at configuration time.
Better even still if NO RUNTIME CODE was used to "copy" the data.



Consider this operation, defined in one of the configuration files.

```
{
  "/vlinks/pcs" : {
    "pcsCurrentChange" : {
      "value": "/site/ess_hs:pcsCurrentCharge",
      "vlink": "/components/pcs_general_info:current_charge",
      "ltype":"double"
    }
  }
}
```

After loading all the variables the two variables can be forced to share a common value.
This command can be added in the ess_manager segment

```
{
  "/system/commands: {
      "vlink":{
          "value":"test",
          "help": "vlink two var values",
          "ifChanged":false, "enabled":true,
          "actions":{"onSet":[{"func":[{"func":"RunAllVLinks"}]}]}}}
}
```

This command can be added in the ess_final segment

```
{
  "/system/commands: {
      "vlink":{
          "value":"pcs"
      }
  }
}
```

Once the vlink has been made. The two variables share a common value but enjoy indiviual params.


``` 

fims_send -m get -r /$$ -u /ess/full/site/ess_hs/pcsCurrentCharge
{"pcsCurrentCharge":{"value":0}}

fims_send -m set -r /$$ -u /ess/full/components/pcs_general_info/current_charge  345
{"current_charge":345}

fims_send -m get -r /$$ -u /ess/full/site/ess_hs/pcsCurrentCharge
{"pcsCurrentCharge":{"value":345}}



Didn't happen ??? ....take a look at ess_final.json and work out why not

Once the link is "set up" the following code demonstrates its operation. 


fims_send -m set -r /$$ -u /ess/full/components/pcs_general_info/current_charge@param '"test"'

fims_send -m get -r /$$ -u /ess/full/components/pcs_general_info/current_charge | jq
{
  "current_charge": {
    "value": 345,
    "param": "test"
  }
}
 
fims_send -m get -r /$$ -u /ess/full/site/ess_hs/pcsCurrentCharge  | jq
{
  "pcsCurrentCharge": {
    "value": 345
  }
}

```

# Links

These are used where the same value is given two different variable names.
The two variables will share all values, params and actions.

Consider these two values

```
{
  "/site/ess_hs": {
    "maxCurrentCharge": {
      "value":231,
      "maxval":345,
      "minval":123
    } 
  }
}
```

```
{
  "/components/pcs_general_info": {
    "max_current_charge": {
      "value":1231
    } 
  }
}
```

Here is the system command the runs the links
```
{
  "/system/commands: {
      "link":{
          "value":"test",
          "help": "link two var values",
          "ifChanged":false, "enabled":true,
          "actions":{"onSet":[{"func":[{"func":"RunAllLinks"}]}]}}}
}
```

Here is the part of one of the config files that sets up the link

```
{
  "/links/pcs" : {
    "pcsCurrentChange" : {
      "value": "/site/ess_hs:maxCurrentCharge",
      "lval": "/components/pcs_general_info:max_current_charge",
      "aname":"pcs"
    }
  }
}
```
Or as a command 

```
 fims_send -m set -r /$$ -u /ess/full/links/pcs/pcsCurrentChange '{"value": "/site/ess_hs:maxCurrentCharge","ltype":"double","aname":"pcs","link":"/components/pcs_general_info:max_current_charge"}'
```

Next trigger the link action.

```
from the command line 

fims_send -m set -r /$$ -u /ess/system/commands/link '"pcs"'

```

In "ess_final.json" we can activate this operation.

```
 "/system/commands":{
       "link" :{
        "value":"pcs",
        "enabled":true
       }
   },
```

## Conclusion 

This is only an iniital introduction to the "interfaces" systems available with the ess_controller. 
We have yet to discuss monitoring , alarms and control interfaces.

The ess_controller team will have to come up with initial designs for new designs and new components.
The Integration team should be able to support and adjust those designs using a selection of tools defined in this and later 
training sessions.

As we work through new designs, "Dual_bms", "Dc Coupled" etc,  new "helper" commands and functions can be introduced.
The modular system design allows progress to be made and operations simplified without affecting older systems.

An example is the "linkv10.2" it does not work quite yet but it can exist alongside the older "link" command.


