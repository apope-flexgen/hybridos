# simple demo of a charge / discharge system  using the ess_controller
# p wilshire 03-28-2022


## Introduction


This "simple" system introduces a number of techniques used in the ess_controller to manage and control systems.

It is a simple exercise to create something that will react to "Charge/Discharge" requests.

Once we get that working we'll add a few bells and whistles to make in look like a "real" system.

This is also a concept exercise to test current features and valuate how the architcture is planned to work in the very near future ( 10.3 release) 

## Basic System

First define the basic system. We'll take an abstract entity  no need to define units at this stage.

```
{
    "/status/bms" : {
        "ChargeRate": 0,            # Current Charge Increment / Decrement
        "Max": 3000,            # Max Charge Capacity
        "Min": 125,             # Min 
        "CurrentCap":500,       # Current Charge Capacity  ( units don't matter at this time)
        "Status" :"Idle"
    }
}
```

You can add this into a "blank" ess controller as follows.

```
fims_send -m set -r /$$ -u /ess/status/bms '{
      "ChargeRate": 0,      
        "enable":false,
        "Max": 3000,        
        "Min": 125,          
        "CurrentCap":500,   
        "Status" :"Idle"
}'
```

Test this with 

```
fims_send -m get -r /$$ -u /ess/full/status/bms  | jq
{
  "ChargeRate": {
    "value": 0
  },
  "CurrentCap": {
    "value": 500
  },
  "Max": {
    "value": 3000
  },
  "Min": {
    "value": 125
  },
  "Status": {
    "value": "Idle"
  },
  "enable": {
      "value": false
  }
}
```


## Basic Operation

We want to set the Status to be "Idle" (0 Charge inc/ dec ), "Charging"  or "Discharging"

Lets create some "simple" rules to let this happen. 

```
{
    "CheckChargeCharging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} > 0",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Charging"    }
                        ]
                    }
                ]}
        },
    "CheckChargeDischarging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} < 0",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true, "uri": "/status/bms:Status",                "outValue": "Discharging"   },
                        { "inValue": true, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Discharging"   }
                        ]
                    }
                ]}
        },
    "CheckChargeIdle": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} == 0",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Idle"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Idle"    }
                        ]
                    }
                ]}
        }

```

This is a bit much to type in so we'll create a small file to contain the setup.


```
cat CalculateVar#1.json
```

We can load this file 

```
fims_send -f /CalculateVar#1.json -m set  -u /ess/cfg/cfile/ess/calc_var_1

```


# First tests

* start a blank ess_controller in one terminal 

```
ess_controller -x
```

* load this file in another terminal.

```
fims_send -f ./CalculateVar#1.json -m set  -u /ess/cfg/cfile/ess/calc_var_1

```


Now we can simulate some of the system operation by simply writing true/false values to CheckChargeIdle, CheckChargeCharge , CheckCharge,Discharge


```
fims_send -m set  -r /$$ -u /ess/status/bms/CheckChargeCharging true

fims_send -m get  -r /$$ -u /ess/status/bms/Status
"Idle"

```

Hmm , did not seem to work.

Lets Check our designated test variable.

```
fims_send -m get  -r /$$ -u /ess/status/bms/CheckChargeCharging
false
```

Nah we want too see the "full" picture

```
sh-4.2# fims_send -m get  -r /$$ -u /ess/full/status/bms/CheckChargeCharging | jq
{
  "CheckChargeCharging": {
    "value": false,
    "debug": true,
    "enable": "/status/bms:enable",
    "expression": "{1} > 0",
    "ifChanged": false,
    "numVars": 1,
    "useExpr": true,
    "variable1": "/status/bms:ChargeRate",
    "actions": {
      "onSet": [
        {
          "remap": [
            {
              "inValue": true,
              "outValue": "Charging",
              "uri": "/status/bms:Status"
            },
            {
              "fims": "set",
              "inValue": true,
              "outValue": "Charging",
              "uri": "/status/bms:Status"
            }
          ]
        }
      ]
    }
  }
}
```

Notice we are missing an indicator that the "actions" triggered ( no aVal .. more about that later )
Notice that also we have an "/status/bms/enable" on the variable and its actions.

This value was set to "false" during the initial load.

Lets set it to true.

```
fims_send -m set  -r /$$ -u /ess/status/bms/enable true

```

Now set the different switches to true and check the value of "/status/bms/Status" each time.

```
fims_send -m set  -r /$$ -u /ess/full/status/bms/CheckChargeCharging true | jq
{
  "CheckChargeCharging": true
}

fims_send -m get  -r /$$ -u /ess/status/bms/Status | jq
"Charging"


fims_send -m set  -r /$$ -u /ess/full/status/bms/CheckChargeDischarging true | jq
{
  "CheckChargeDischarging": true
}
fims_send -m get  -r /$$ -u /ess/status/bms/Status | jq
"Discharging"


fims_send -m set  -r /$$ -u /ess/full/status/bms/CheckChargeIdle true | jq
{
  "CheckChargeIdle": true
}
fims_send -m get  -r /$$ -u /ess/status/bms/Status | jq
"Idle"

```

## Increasing the " automantion

We can cause the "Checks" to be run anytime the ChargeRate is changed.


Consider this action.


```
fims_send -m set -r /$$ -u /ess/status/bms '{
    "ChargeRate": {
            "debug": true,
            "value": 0,
            "ifChanged": true,
            "actions": { 
                "onSet":[
                    {
                    "func": 
                        [
                        { "inAv": "/status/bms:CheckChargeCharging",      "func":"CalculateVar"    },
                        { "inAv": "/status/bms:CheckChargeDischarging",   "func":"CalculateVar"    },
                        { "inAv": "/status/bms:CheckChargeIdle",          "func":"CalculateVar"    }
                        ]
                    }
                ]}
        }
}'

```

This means that every time the value of "ChargeRate" changes so does the evaluation of  of "/status/bms/Status".
The trick here is to use the "CalculateVar" function on each of the CheckCharging evaluators to auto detect the "true/false" value to send to trugger the change in state



## Automating the charge/discharge activity.

We now need to extend this simple model to simulate the system Charging  and Discharging.

We have an incoming charge/discharge value and we need to periodically apply that to the Charge Value.



## Adding a periodic operation.

The ess_controller has a versatile scheduler. Read the docs for more. 
Simply "running" a scheduled operation is achieved by writing the current system time to a selcted variable at a defined ( very precise ) rate.
Once "running" the schedule operation can be "Started", " Stopped" , "Repeated" etc at any time.


Here is a quick look at the running schedule list.

```
fims_send -m get  -r /$$ -u /ess/full/schlist | jq
{
  "ess_config": {
    "runTime": 2947.523433598415,
    "repTime": 0.1,
    "runs": 29425,
    "uri": "/config/control:ess_config",
    "func": "RunTarg"
  }
}
```

Take a look at the scheduled item, this is for the config loader system.

```
fims_send -m get  -r /$$ -u /ess/full/config/control/ess_config | jq
{
  "ess_config": {
    "value": 3298.024124145508,
    "active": true,
    "amap": true,
    "debug": 0,
    "enabled": true,
    "endTime": 0,
    "fcn": "RunTarg",
    "reftime": 0,
    "repTime": 0.1,                    <<<< periodic frequncy >>>>
    "runCnt": 32931,                   <<<  Run Count >>>
    "runEnd": 0,
    "runTime": 3298.123433598096,
    "targ": "/config/control:ess_config",
    "uri": "/config/control:ess_config"
  }
}
```


We can easily add another periodic operation, Well there is the "easy" way and the "hard" way.

Lets take the easy way.

# define a control var.

Anything will do

```
fims_send -m set -r /$$ -u /ess/sched/bms/controls ' {
        "runMon":{
            "value":0
        }
}'

```

# schedule that control var 

```
fims_send -m set -r /$$ -u /ess/system/commands '  {
        "run":{
            "value":"test",
            "uri":"/sched/bms/controls:runMon",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
}'
```

That's it, we are now sending the "current time" to "/sched/bms/controls:runMon" very second.

We can up that rate to every 100mS with the following command.

```
fims_send -m set -r /$$ -u /ess/sched/bms/controls/runMon@repTime 0.1 
```

Take a look at the schlist now.

```
fims_send -m get  -r /$$ -u /ess/full/schlist | jq
{
  "ess_config": {
    "runTime": 4035.8234335974253,
    "repTime": 0.1,
    "runs": 40308,
    "uri": "/config/control:ess_config",
    "func": "RunTarg"
  },
  "runMon": {
    "runTime": 4036.7060037,
    "repTime": 1,
    "runs": 3,
    "uri": "/sched/bms/controls:runMon",
    "func": "RunTarg"
  }
}
```

And take a peek at the "/sched/bms/controls:runMon" variable.

```
fims_send -m get  -r /$$ -u /ess/full/sched/bms/controls  | jq
{
  "runMon": {
    "value": 4321.706640005112,
    "active": true,
    "amap": true,
    "debug": 0,
    "enabled": true,
    "endTime": 0,
    "fcn": "RunTarg",
    "reftime": 0,
    "repTime": 1,
    "runCnt": 289,
    "runEnd": 0,
    "runTime": 4322.7060037,
    "targ": "/sched/bms/controls:runMon",
    "uri": "/sched/bms/controls:runMon"
  }
}
```


To make the system "do something" when it writes the current time to "/sched/bms/controls:runMon" we can assign actions to that varuable.

To stop the scheduling ot the  "/sched/bms/controls:runMon" variable you can do several things.

* set the endTime to a non zero value less than the current time. ( or a time in the future to delay the shutdown)
```
fims_send -m set  -r /$$ -u /ess/full/sched/bms/controls/runMon@endTime  4000
```

* set the repTime to  zero. The task will run one more time and then quit
```
fims_send -m set  -r /$$ -u /ess/full/sched/bms/controls/runMon@repTime  0
```

You can pause the scheduling of any attached actions 

* set enabled to false. The task will run but not triggerany actions.
```
fims_send -m set  -r /$$ -u /ess/full/sched/bms/controls/runMon@enabled  false
```

## What we want to do with the scheduler.

We want to see if the system is charging or discharging and modify the system charge based on the charge rate.

But .... we also want to make sure we do not exceed the Max or Min system charge.


Th sort of  operation we want to do with ChargeRate can be defined in two other variables triggered from a third one that work out if we are 
charging or discharging.

First detect if we are charging or discharging , I think we have already done that.

```
fims_send -m get  -r /$$ -u /ess/full/status/bms/CheckChargeCharging | jq
{
  "CheckChargeCharging": {
    "value": false,
    "debug": true,
    "enable": "/status/bms:enable",
    "expression": "{1} > 0",
    "ifChanged": false,
    "numVars": 1,
    "useExpr": true,
    "variable1": "/status/bms:ChargeRate",
    "actions": {
      "onSet": [
        {
          "remap": [
            {
              "inValue": true,
              "outValue": "Charging",
              "uri": "/status/bms:Status"
            },
            {
              "fims": "set",
              "inValue": true,
              "outValue": "Charging",
              "uri": "/status/bms:Status"
            }
          ]
        }
      ]
    }
  }
}
```


BUt now we want to add or subtract charge based on the charge rate.

We'll use two more variables for those operations.

```
{
"/status/bms": {
    "DischargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:CurrentCap",
            "variable2": "/status/bms:ChargeRate",
            "variable3": "/status/bms:Max",
            "variable4": "/status/bms:Min",
            "expression": "if (({1} > {4}), ({1} + {2}), {1})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        },
        "ChargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:CurrentCap",
            "variable2": "/status/bms:ChargeRate",
            "variable3": "/status/bms:Max",
            "variable4": "/status/bms:Min",
            "expression": "if (({1} < {3}), ({1} + {2}), {1})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        }
    }
}

```

We can modify CheckChargCharging to also control the ChargeCap / DischargeCap calulations.

```

CheckChargCharging ....
       "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:DischargeCap@enabled",  "outValue":false            },
                        { "inValue": true,  "uri": "/status/bms:ChargeCap@enabled",     "outValue":true             },
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Charging"    },
                        ]
                    }
                ]}

CheckChargDischarging ....
        "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
       
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",  "outValue":true             },
                        { "inValue": true, "uri": "/status/bms:Status",                "outValue": "Discharging"   },
                        { "inValue": true, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Discharging"   }
                        ]
                    }
                ]}
       
```

This really just enables or disables the actions of the ChargeCap / DischargeCap  variables with their calculations.
We need a way to schedule them periodically to adjust the system charge based on the charge / discharge rate.

Given the nature of the operations we would like a better way of running the scheduler  for this sort of operation.

## Introduction to the "Monitor" System.

The monitor system is a way of collecting a number of variables and functions together and running those functions against the selected variables on a periodic basis.

It is used to collect all the safety checks, command monitoring, command and status monitoring in the system.

We define a system "table"

```
"/schedule/monitor/bms": {
        "/status/bms:CheckChargeCharging":    { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:CheckChargeDisharging":  { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:CheckChargeIdle":        { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:ChargeCap":              { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:DischargeCap":           { "amap": "bms", "func":"CalculateVar"}
    }
}
```

And then arrange the scheduler to execute the RunMonitor function against all the entries in that table

```
"/system/commands": {
        "runMon":{
            "value":0,
            "aname":"bms",
            "debug":false,
            "monitor":"monitor",
            "enable":"/controls/bms:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"debug":false, "func":"RunMonitor","aname":"ess"}]}]}
        },
        "run":{
            "value":"test",
            "uri":"/system/commands:runMon",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
    }
```

The RunMonitor function simply runs the selected function , in this case CalculateVer with the asset var as an argument.

The advantage of doing this is that we dont have to write a value to an assetVar to trigg the functions.
We, also, can build up a list of operations to perform as the config files are loaded.

For example, if the PCS system wants the BMS system to monitor a particular variable it can specify items to be added to one of the "/schedule/monitor/bms"/ lists in its own config file.



In this case the CalculateVar is run against five assetVars

* CheckChargeCharge:    to check the charge / idle status
* CheckChargeDischarge: to check the discharge / idle status
* CheckChargeIdle:      to check the charge / idle status
* ChargeCap:            to process a positive charge rate 
* DischargeCap:         to process a negative discharge rate 

### The whole system 

```
{
    "pname":"ess",
    "amname":"bms",
    "/controls/bms":{
        "enable":true
    },
    "/status/bms": {
        "Cap": 1,
        "CurrentCap": {
            "value":200,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "fims":"set", "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        },
        "Charge": 0,
        "Max": 3000,
        "Min": 125,

        "CheckChargeCharging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} > 0",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:DischargeCap@enabled",  "outValue":false            },
                        { "inValue": true,  "uri": "/status/bms:ChargeCap@enabled",     "outValue":true             },
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Charging"    },
                        { "inValue": false, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": false, "uri": "/status/bms:DischargeCap@enabled",  "outValue":true             },
                        { "inValue": false, "uri": "/status/bms:Status",                "outValue": "Discharging"   },
                        { "inValue": false, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Discharging"   }
                        ]
                    }
                ]}
        },
        "CheckChargeCharging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} > 0",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:DischargeCap@enabled",  "outValue":false            },
                        { "inValue": true,  "uri": "/status/bms:ChargeCap@enabled",     "outValue":true             },
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Charging"    },
                        ]
                    }
                ]}
        },
        "CheckChargeDischarging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} < 0 ",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",  "outValue":true             },
                        { "inValue": true, "uri": "/status/bms:Status",                "outValue": "Discharging"   },
                        { "inValue": true, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Discharging"   }
                        ]
                    }
                ]}
        },
        "CheckChargeIdle": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} == 0 ",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",  "outValue":false             },
                        { "inValue": true, "uri": "/status/bms:Status",                "outValue": "Idle"   },
                        { "inValue": true, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Idle"   }
                        ]
                    }
                ]}
        },

        "DischargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:CurrentCap",
            "variable2": "/status/bms:Charge",
            "variable3": "/status/bms:Max",
            "variable4": "/status/bms:Min",
            "expression": "if (({1} > {4}), ({1} + {2}), {1})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        },
        "ChargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:CurrentCap",
            "variable2": "/status/bms:Charge",
            "variable3": "/status/bms:Max",
            "variable4": "/status/bms:Min",
            "expression": "if (({1} < {3}), ({1} + {2}), {1})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        }
    },

    "/schedule/monitor/bms": {
        "/status/bms:CheckChargeIdle":         { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:CheckChargeCharge":       { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:CheckChargeDischarge":    { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:ChargeCap":               { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:DischargeCap":            { "amap": "bms", "func":"CalculateVar"}
    },

    "/system/commands": {
        "runMon":{
            "value":0,
            "aname":"bms",
            "debug":false,
            "monitor":"monitor",
            "enable":"/controls/bms:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"debug":false, "func":"RunMonitor","aname":"ess"}]}]}
        },
        "run":{
            "value":"test",
            "uri":"/system/commands:runMon",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
    }
}'

echo $ess_init > /tmp/ess_init
fims_send -f /tmp/ess_init -m set  -u /ess/cfg/cfile/ess/ess_init 


exit 0

# first set up a basic system 
fims_send -m set  -u /ess/cfg/cfile/ess/bms_init '{
    "pname":"ess",
    "amname":"bms",
    "/status/bms": {
        "MaxCapacity":3000,
        "CurrentCapacity":0,
        "Status":"Init",
        "Soc":0,
        "Voltage":0,
        "Power":0
    },
    "/config/bms": {
        "MaxCapacity":3000,
        "MaxChargeCurrent":-2000,
        "MaxDischargeCurrent":2000,
        "Voltage":0,
        "Power":0
    },
    "/controls/bms": {
        "Start":false,
        "Stop":false,
        "Standby":false,
        "ChargeCurrent":0,
        "DischargeCurrent":0
    },

    "/assets/bms":{
        "CurrentState":"init",
        "SOC": 0,
        "Voltage":0,
        "Current":0,
        "Power":0
    }
}'

#
# next set up some reactions to the start / stop commands
#
fims_send -m set  -u /ess/cfg/cfile/ess/bms_controls '{
    "/status/bms": {
        "test2": {
            "value":2,
            "actions":{
                "onSet":[
                {
                    "xremap":
                    [
                        {"fims":"set","uri":"/status/bms:test2"}
                    ]
                }
            ]}
        },
        "test3": 12,
        "test": {
            "value":0,
            "debug":true,
            "useExpr": false,
            "includeCurrVal":true,
            "numVars": 2,
            "variable1": "/status/bms:test2",
            "variable2": "/status/bms:test3",
            "expression": "{1} + {2}",
            "operation":"+",

            "actions":{
                "onSet":[
                {
                    "remap":
                    [
                        {"enable":false, "fims":"set","uri":"/status/bms:test"},
                        {"enable":false, "uri":"/status/bms:test2"}
                    ]
                },
                {
                    "func":
                    [
                        {"debug":false, "func":"CalculateVar"}
                    ]
                }
            ]
            }

        },

        "StartCount": {
            "value":0,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:Status",
            "variable2": "/status/bms:StartCount",
            "expression": "(({1} == Starting) and ({2} < 10)) {2} + 1 ",
            "actions":{"onSet":[
                {"remap":
                    [
                        {"enable":"/status/bms:startComplete","uri":"/status/bms:Status","outValue":"Running"},
                        {"fims":"set","uri":"/status/bms:StartCount"}
                    ]
                }
                ]
            }
        }
    },
    "/controls/bms": {
        "enable":false,
        "Start": {
            "value":false,
            "actions":{"onSet":[{"remap":[{"uri":"/status/bms:Status","outValue":"Starting"}]}]}
        },
        "Stop": {
            "value":false,
            "actions":{"onSet":[{"remap":[{"uri":"/status/bms:Status","outValue":"Stopping"}]}]}
        },
        "Standby": {
            "value":false,
            "actions":{"onSet":[{"remap":[{"uri":"/status/bms:Status","outValue":"Shutting_Down"}]}]}
        }

    }
}'

fims_send -m get -r /$$  -u /ess/naked/config/cfile | jq

#
# now we set up some dynamic operations.
fims_send -m set  -u /ess/cfg/cfile/ess/bms_monitor '{
    "pname":"ess",
    "amname":"bms",

    "/schedule/monitor/bms": {
        "/status/bms:StartCount":          { "enable": false,  "amap": "bms", "func":"CalculateVar"}
    },

    "/system/commands": {
        "runMon":{
            "value":0,
            "aname":"bms",
            "monitor":"monitor",
            "enable":"/controls/bms:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"func":"RunMonitor","aname":"ess"}]}]}
        },
        "run":{
            "value":"test",
            "uri":"/system/commands:runMon",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"xRunSched"}]}]}
        },
        "runTest":{
            "value":"test",
            "uri":"/status/bms:test",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
    }
}'