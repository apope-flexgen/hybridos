## Dev-Ops Ess_controller Introduction

P. Wilshire 
12/08/2021

Really meant for anyone else who needs more details


# The Do nothing controller. 

     It just sits there and thinks about stuff.

    ess_controller -x

    other options 

    ess_controller -v
    ess_controller -h
```
 usage: ess_controller <options>
 name:    [ess]
 config:  [ess_config_risen_sungrow]
 dir:     [configs/ess_controller]
 subs:    [:/components:/system:/site:/reload:/misc2:]
 fimsdir: [/usr/local/bin/] old [false] new [true]
 options:
 -?   print help
 -h   print help
 -v   print version info
 -x   simdbi  triggers cfg load
 -s   sets the subs list <:/ess:/components:/site:>
 -c   initial config file <ess_config_risen_sungrow>
 -n   sets the default name <ess>
 -d   sets the config dir <deprecated>
 -f   sets thefims binary dir <deprecated>
```

```code
fims_send -m get -r /$$ -u /ess/system | jq
```

# We have actions 

```json
{
  "/system/actions": {
    "bitfield": {
      "value": "decode value bits  into a number  of different values"
    },
    "bitmap": {
      "value": "use a bitmap to set the output variable"
    },
    "bitset": {
      "value": "set or clear a bit in an output var"
    },
    "enum": {
      "value": "decode a value into a number of different values"
    },
    "func": {
      "value": "run a func using an assetVar as an argument"
    },
    "limit": {
      "value": "set limits on a value"
    },
    "remap": {
      "value": "forward a value to a different uri"
    }
  }
}
```

# we have functions

```json
{
  "/system/functions": {
    "AggregateManager": {
      "value": true
    },
    "CalculateVar": {
      "value": true
    },
    "CheckDbiResp": {
      "value": true
    },
    "CheckDbiVar": {
      "value": true
    },
    "CheckMonitorVar": {
      "value": true
    },
    "CheckMonitorVar_v2": {
      "value": true
    },
    "CheckTableVar": {
      "value": true
    },
    "DeratePower": {
      "value": true
    },
    "DumpConfig": {
      "value": true
    },
    "EssSystemInit": {
      "value": true
    },
    "Every1000mS": {
      "value": true
    },
    "Every100mSP1": {
      "value": true
    },
    "Every100mSP2": {
      "value": true
    },
    "Every100mSP3": {
      "value": true
    },
    "FastPub": {
      "value": true
    },
    "GPIOCalcResponse": {
      "value": true
    },
    "HandleCmd": {
      "value": true
    },
    "HandlePowerCmd": {
      "value": true
    },
    "HandlePowerEst": {
      "value": true
    },
    "HandlePowerRackEst": {
      "value": true
    },
    "HandleSchedItem": {
      "value": true
    },
    "HandleSchedLoad": {
      "value": true
    },
    "LoadClient": {
      "value": true
    },
    "LoadConfig": {
      "value": true
    },
    "LoadServer": {
      "value": true
    },
    "LogDebug": {
      "value": true
    },
    "LogError": {
      "value": true
    },
    "LogInfo": {
      "value": true
    },
    "LogIt": {
      "value": true
    },
    "LogWarn": {
      "value": true
    },
    "MakeLink": {
      "value": true
    },
    "MathMovAvg": {
      "value": true
    },
    "RunAllALists": {
      "value": true
    },
    "RunAllLinks": {
      "value": true
    },
    "RunAllVLinks": {
      "value": true
    },
    "RunLinks": {
      "value": true
    },
    "RunMonitor": {
      "value": true
    },
    "RunMonitorList": {
      "value": true
    },
    "RunPub": {
      "value": true
    },
    "RunSched": {
      "value": true
    },
    "RunScript": {
      "value": true
    },
    "RunSysVec": {
      "value": true
    },
    "RunSystemCmd": {
      "value": true
    },
    "RunTpl": {
      "value": true
    },
    "RunVLinks": {
      "value": true
    },
    "SchedItemOpts": {
      "value": true
    },
    "SendClearFaultCmd": {
      "value": true
    },
    "SendDb": {
      "value": true
    },
    "SendDbiVar": {
      "value": true
    },
    "SetDbiDoc": {
      "value": true
    },
    "SetMapping": {
      "value": true
    },
    "ShutdownBMS": {
      "value": true
    },
    "ShutdownPCS": {
      "value": true
    },
    "SimHandleBms": {
      "value": true
    },
    "SimHandleHeartbeat": {
      "value": true
    },
    "SimHandlePcs": {
      "value": true
    },
    "SimHandleSbmu": {
      "value": true
    },
    "SlewVal": {
      "value": true
    },
    "SlowPub": {
      "value": true
    },
    "SlowPubOne": {
      "value": true
    },
    "StartupBMS": {
      "value": true
    },
    "StartupPCS": {
      "value": true
    },
    "StopSched": {
      "value": true
    },
    "TestTriggerFunc": {
      "value": true
    },
    "UpdateSysTime": {
      "value": true
    },
    "process_sys_alarm": {
      "value": true
    }
  }
}
```

# But it is doing something 

It's trying to get its config file

The -x command line option triggered this activity.

(This is a 10.1 feature under advanced development in 9.3)

```code
fims_send -m get -r /$$ -u /ess/full/config | jq
```

```json
{
  "/config/cfile": {
    "ess_config_risen_sungrow": {
      "value": false,
      "aname": "ess",
      "file": "ess_config_risen_sungrow",
      "reqCount": 5,
      "reqTimeout": 26.29939800000284
    }
  },
  "/config/control": {
    "ess_config": {
      "value": 634.2977030000184,
      "active": true,
      "amap": true,
      "debug": 0,
      "enabled": true,
      "endTime": 0,
      "fcn": "RunTarg",
      "reftime": 0,
      "repTime": 1,
      "runAfter": 0.7416690000100061,
      "runCnt": 634,
      "runEnd": 0,
      "runTime": 635.295179953,
      "runtime": 0.7416690000100061,
      "targ": "/config/control:ess_config",
      "uri": "/config/control:ess_config"
    }
  },
  "/config/system": {
    "ess": {
      "value": 0.31936700001824647
    }
  }
}
```


## add a simple config file.

Still does nothing but has some data 
We now have some data.

```code
fims_send -m set -r /$$ -u /ess/system/pcs '{"status":"Idle","voltage":0 ,"Errors":false}
```
```code
fims_send -m get -r /$$ -u /ess/system/pcs  | jq
```

```json
{
  "Errors": {
    "value": false
  },
  "status": {
    "value": "Idle"
  },
  "voltage": {
    "value": 0
  }
}
```



## Add actions to a variable.

We want a change in status to "Run" to set the volts to 480

```code
fims_send -m set -r /$$ -u /ess/system/pcs '
{ "status":{"value":"$$","actions":{"onSet":[{"remap":[{"inValue":"Run","uri":"/system/pcs:voltage","outValue":480}]}]}}}'
```

```code
fims_send -m set -r /$$ -u /ess/system/pcs | jq
```

```json
{
  "Errors": {
    "value": false
  },
  "actions": {
    "value": false
  },
  "status": {
    "value": "Init",
    "actions": {
      "onSet": [
        {
          "remap": [
            {
              "aVal": "Init",
              "inValue": "Run",
              "outValue": 480,
              "uri": "/system/pcs:voltage"
            }
          ]
        }
      ]
    }
  },
  "voltage": {
    "value": 0
  }
}
```



## Make changing the value do something.

```code
 fims_send -m set -r /$$ -u /ess/full/system/pcs/status '"Run"' 
```
```code
fims_send -m get -r /$$ -u /ess/full/system/pcs | jq
```
```json
{
  "Errors": {
    "value": false
  },
  "actions": {
    "value": false
  },
  "status": {
    "value": "Run",
    "actions": {
      "onSet": [
        {
          "remap": [
            {
              "aVal": "Run",
              "inValue": "Run",
              "outValue": 480,
              "uri": "/system/pcs:voltage"
            }
          ]
        }
      ]
    }
  },
  "voltage": {
    "value": 480
  }
}
```

Or we can just look at the voltage value
```code
fims_send -m get -r /$$ -u /ess/system/pcs/voltage | jq

480
```

```code
fims_send -m get -r /$$ -u /ess/full/system/pcs/voltage | jq
{
  "voltage": {
    "value": 480
  }
}
```
## Values normally have to change to trigger an action

set up ifChanged condition (its default is true)

```code
fims_send -m set -r /$$ -u /ess/system/pcs '{"voltage":22 }'
{"voltage":22}

fims_send -m set -r /$$ -u /ess/system/pcs/status@ifChanged 'true' | jq
{
  "status@ifChanged": true
}
```

now send th Run command again.

```code

fims_send -m set -r /$$ -u /ess/full/system/pcs/status '"Run"'| jq
{
  "status": "Run"
}
```
 Voltage has not changed 

```code
fims_send -m get -r /$$ -u /ess/full/system/pcs/voltage | jq
{
  "voltage": {
    "value": 22
  }
}
```

Now set the value change detection to false ( run on every set )

```code
fims_send -m set -r /$$ -u /ess/system/pcs/status@ifChanged 'false' | jq
{
  "status@ifChanged": false
}
```

This time the "Run" command sets the value.

```code
fims_send -m set -r /$$ -u /ess/full/system/pcs/status '"Run"'| jq
{
  "status": "Run"
}

fims_send -m get -r /$$ -u /ess/full/system/pcs/voltage | jq
{
  "voltage": {
    "value": 480
  }
}
```


Now add a "Stop" status to set the volts to 0

```code
fims_send -m set -r /$$ -u /ess/system/pcs '
{ "status":{"value":"$$","actions":{"onSet":[
    {"remap":[
        {"inValue":"Run","uri":"/system/pcs:voltage","outValue":480},
        {"inValue":"Stop","uri":"/system/pcs:voltage","outValue":0}
    ]}]}}}'
```
Now we can set the voltage to 480 or 0.

```code
fims_send -m get -r /$$ -u /ess/full/system/pcs/voltage | jq
{
  "voltage": {
    "value": 480
  }
}

fims_send -m set -r /$$ -u /ess/full/system/pcs/status '"Stop"'| jq
{
  "status": "Stop"
}

fims_send -m get -r /$$ -u /ess/full/system/pcs/voltage | jq
{
  "voltage": {
    "value": 0
  }
}
```



## Interesting data operations

  Remap - action , as shown already

  Link  - operation, make two "names" point to the same variable 

  UI    /assets/pcs/summary:voltage
  ESS   /system/pcs:voltage

```json
{
    "/links/pcs":{
            "seconds": {
            "value": "/components/pcsm_general:seconds",
            "link": "/status/pcs:seconds"  (10.1 feature)
        }

}
```

  VLink - operation, make to name have the same value. 

  ESS   /system/pcs:voltage
  SITE  /site/ess_1_hs:voltage

```json
{
    "/vlinks/pcs":{
            "voltage": {
            "value": "/assets/pcs/summary:Volts",
            "vlink": "/status/pcs:voltage"
        }

}
```


In 10.1 we will be able to create links and vlinks on config load but not yet

## Add attributes to a variable

This adds the "max_value" attribute to a variable.

```code
fims_send -m set -r /$$ -u /ess/full/system/pcs/voltage@max_value '490'
```

```code
fims_send -m get -r /$$ -u /ess/full/system/pcs/voltage  | jq
{
  "voltage": {
    "value": 0,
    "max_value": 490
  }
}
```


## Add a function (action)  to a variable

    Run some code when we write something.
    Set up a system/pcs:power variable.
    Then set up something to trigger a calculation.

```code
fims_send -m set -r /$$ -u /ess/system/pcs '
{   
    "power":123,
    "calcPower":{"value":0,"actions":{"onSet":[
    {"func":[
        {"inAv":"/system/pcs:power","func":"CalculateVar","amap":"pcs"}
    ]}]}}
}'
```
```code
fims_send -m get -r /$$ -u /ess/full/system/pcs/calcPower  | jq
```
```json
{
  "calcPower": {
    "value": 0,
    "actions": {
      "onSet": [
        {
          "func": [
            {
              "aVal": 0,
              "amap": "pcs",
              "func": "CalculateVar",
              "inAv": "/system/pcs:power"
            }
          ]
        }
      ]
    }
  }
}
```

In the system log we see the following 

```
[5240.31s  ] [info    ] [setupCalculateV] Setting up params for [/system/pcs:power]
[5240.37s  ] [warning ] [checkOperation ] operation [n/a] not defined for assetVar [/system/pcs:power]. Supported operations are:
    Addition           (+)
    Subtraction        (-)
    Multiplication     (*)
    Division           (/)
    Modulus Division   (%)
    Average            (avg)
    Percentage of      (pctOf)
    Maximum            (max)
    Minimum            (min)
    Square Root        (sqrt)
    Scale              (scale)
    And                (and)
    Or                 (or)
    Greater than       (>)
    Less than          (<)
    Value Changed Any  (valChangedAny)
    Value Changed All  (valChangedAll)
```


So we need to provide some sort of operation for the power calculation.

```code
fims_send -m set -r /$$ -u /ess/system/pcs '
{
        "power": {
            "value": 0,
            "numVars": 2,
            "variable1": "/status/pcs:BMSVoltage",
            "variable2": "/status/pcs:BMSCurrent",
            "scale": 1000,
            "operation": "*"
        }
}'
```


Now we get 
```
[6814.51s  ] [info    ] [runActFuncfromC] seeking Function [CalculateVar] for an assetVar [/system/pcs:power]
[6814.55s  ] [warning ] [getOperands    ] assetVar in parameter [variable1] of assetVar [power] does not exist
[6814.58s  ] [warning ] [getOperandsDbl ] Unable to retrieve list of operands for [/system/pcs:power]
```

We have to add the following

```
fims_send -m set -r /$$ -u /ess/status/pcs '
{
        "BMSVoltage": 1300,
        "BMSCurrent": 280
}'
```

  Finally trigger the calculation 

```code
fims_send -m set -r /$$ -u /ess/full/system/pcs/calcPower 25 | jq
{
  "calcPower": 25
}
```

```code
fims_send -m get -r /$$ -u /ess/full/system/pcs/power  | jq
```

Here is the output ( note the scaling divied the answer by 1000)
```json
{
  "power": {
    "value": 364,
    "expression": "n/a",
    "includeCurrVal": false,
    "numVars": 2,
    "operation": "*",
    "scale": 1000,
    "useExpr": false,
    "variable1": "/status/pcs:BMSVoltage",
    "variable2": "/status/pcs:BMSCurrent"
  }
}
```

 This operation can be extended to include more complex expressions.

```
{
    "/status/bms": {
      "UnderCurrentAlarm": {
            "value": 0,
            "numVars": 4,
            "variable1": "DCClosed",
            "variable2": "PCSDCClosed",
            "variable3": "MaintModeEnabled",
            "variable4": "OffFault",
            "expression": "({1} or {4}) and not {2} and {3}",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/summary:stop@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/bms/summary:stop@enabled", "outValue":true}
                    ]
                }]
            }
        }
    }
}
```


## Schedule a function periodically.

    There are two main reasons to run periodic tasks
      1/ to repeatedly check on components
      2/ to perform an action ov a period of time.

    The ess_controller has a complex , prioity based scheduler that executes a simple function.
    It sends the current (relative since start) time to a seleted assetVar.

    To do this it needs a controller assetVar that handles the schduling details and a target 
    assetVar to recive the time and then trigger actions and functions to perform at that time.
    (These could be combined as a single assetVar)

    The simple scheduler is not exposed in the config space by default.
    The more complex config based scheduler interface can be included in config_file

    Lets start with adding  assetVars called "Run" and "Stop" to provide an interface to the scheduler.

```code
fims_send -m set -r /$$ -u /ess/system/commands '
{
    "Run":{
        "value":"test",
        "help": "run a schedule var",
        "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}},
    "Stop":{
        "value":"test",
        "help": "stop a schedule var",
        "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"StopSched"}]}]}}
                   
}'
```

  We can then start sending the current time to "/control/pubs:pubBmsHs".

```code
fims_send -m set -r /$$ -u /ess/system/commands/Run '
{
    "value":0,
    "amap::"ess",
    "uri":"/control/pubs:pubBmsHs",
    "every":0.5,"offset":0,"debug":false
}'
```
  Then inspect the /contol/pubs system

```code
fims_send -m get -r /$$ -u /ess/full/control/pubs | jq
```
```json
{
  "pubBmsHs": {
    "value": 1243.779251000029,
    "active": true,
    "amap": true,
    "debug": 0,
    "enabled": true,
    "endTime": 0,
    "fcn": "RunTarg",
    "reftime": 0,
    "repTime": 0.5,
    "runCnt": 28,
    "runEnd": 0,
    "runTime": 1244.278214168,
    "targ": "/control/pubs:pubBmsHs",
    "uri": "/control/pubs:pubBmsHs"
  }

```
But the variable is doing nothing at the moment.
Lets make it publish some data.

```code
fims_send -m set -r /$$ -u /ess/full/control/pubs '
{ 
    "pubBmsHs":{"value":"$$","table":"/site/bms_hs",
                "enabled":true, "actions":{
                    "onSet":[{"func":[{"amap":"ess","func":"RunPub"}]}]}
                    }
}'
```

We can check the output with fims_listen

```
fims_listen
Method:  pub
Uri:     /site/bms_hs
ReplyTo: (null)
Body:    {"/site/bms_hs":{"value":"uri /site/bms_hs nonexistent"}
Timestamp:   2021-12-09 08:26:08.115546
```

This tells us we need some data in the /site/bms_hs table

```code
fims_send -m set -r /$$ -u /ess/full/site/bms_hs '
{ 
    "speed":1234,
    "Status":"Running",
    "errors":false
}'
```

## Modbus  Data input 

## More complex operations 
    Wake Monitor
    Handle Commands
    Pubs

## Data Output Format
    naked
    clothed
    full
   (ui, 10.1)

