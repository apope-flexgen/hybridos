P. Wilshire
11/04/2021

## Action Consolidation

# Introduction

The MVP version of the ess_conrtoller (v1.0.0) created a nnumber of special methods for handling variables.
The objective was to create a configurable , flexible way to  manipulate inputs and outputs as needed.

Data input arrives from the IO periperals in the for for json encoded messages.

```json
{
    "/components/pcs_info": {
        "status": 21,
        "soc":1000,
        "active_current":-23456
    }
}
```

This information needs to be decoded and dispatched to different parts of the controller.

Date generated by the controller needs to be decoded and diapatched to either the UI , the Site Controller or back to the IO peripheral.

For Example

```json
{
    "/controls/pcs":{
        "command":"Power On"
    }
}```

May need to be translated to a particular bit in an output variable destined for the PCS Controller.

The ess_controller takes the data maniplation operations provided by the Modbus Interface and Metrics systems to a new level.

The rapid development of the first cut of the ess_controller extended the "actions" concept with several simple nd a few more complex features as the controller concept was realised.
The result was a very fuctional but totally disorganised system.

Part of the 1.1.0 development entails streamlining this chaos into a cohrent organised feature.
This has the added benefit of creating a compact and maintainable code base.

## What is an action

When an external value is presented ("set") the controller reacts by translating that value in a number of different ways into other values dispatched throughout the system.
For Example the PCS status of 21 may mean that the DC Contactor is closed and the system is in "Standby" mode.

This can be presented by the PCS supplier as follows:

status bit [0]  1: DC contactor closed
status bit [0]  0: DC contactor open

status bit [4]  1: Mode E-Stop
status bit [5]  1: Mode Stop
status bit [6]  1: Mode StandBy
status bit [5]  1: Mode Run


Consider this control narrative 
If pcs_status bit 1  is true then set value DC Contactor status to closed.
If pcs_status bit 1  is false then set value DC Contactor status to open.

the ess _controller config for this is something like:

{"bit":1, "inValue":true , "uri":"/status/pcs:dc_contactor","outValue":"closed"},
{"bit":1, "inValue":false , "uri":"/status/pcs:dc_contactor","outValue":"open"}

and 

{"bit":7, "inValue":true , "uri":"/status/pcs:mode","outValue":"Run"},
{"bit":6, "inValue":true , "uri":"/status/pcs:mode","outValue":"Standby"},
{"bit":5, "inValue":true , "uri":"/status/pcs:mode","outValue":"Stop"},
{"bit":4, "inValue":true , "uri":"/status/pcs:mode","outValue":"E_Stop"},

Note that the evaluation order is sequential so the last action to be evaluated is the final answer.
( but there is more ...)

Compounding actions.
If the system gets an E-Stop status the dc_contactor needs to be opened immediately and the power taken to zero.

so extend the actions as follows.

{"bit":7, "inValue":true , "uri":"/status/pcs:mode","outValue":"Run"},
{"bit":6, "inValue":true , "uri":"/status/pcs:mode","outValue":"Standby"},
{"bit":5, "inValue":true , "uri":"/status/pcs:mode","outValue":"Stop"},
{"bit":4, "inValue":true , "uri":"/status/pcs:CurrentCmd","outValue":0},
{"bit":4, "inValue":true , "uri":"/status/pcs:dc_contactor","outValue":"open"},
{"bit":4, "inValue":true , "uri":"/status/pcs:mode","outValue":"E_Stop"},

So in this case the incoming E-Stop status has done most of the work to shut the system down.

But if we get an E-Stop status we do Not want to evaluate the contactor status or set the other modes.

To facilitate this each "action" component can be "enabled".

The estop bit bit becoming active sets estop_not_active to false turning off the evaluation of the other bits
When the estop bit is reset the other option evaluations are re enabled.

This way most, if not all, of the command and status decoding  can be set up in the sysem configuration.

This system is incredibly flexible, E-Stop sequence can be reworked like this


{"bit":4, "inValue":true , "uri":"/status/pcs:mode","outValue":"E_Stop"},
{"bit":4, "inValue":true , "uri":"/controls/pcs:estop_not_active","outValue":false},
{"bit":4, "inValue":false , "uri":"/controls/pcs:estop_not_active","outValue":true},
{"enable":"/controls/pcs:estop_not_active","bit":4, "inValue":true , "uri":"/status/pcs:CurrentCmd","outValue":0},
{"enable":"/controls/pcs:estop_not_active","bit":4, "inValue":true , "uri":"/status/pcs:dc_contactor","outValue":"open"},
{"enable":"/controls/pcs:estop_not_active","bit":7, "inValue":true , "uri":"/status/pcs:mode","outValue":"Run"},
{"enable":"/controls/pcs:estop_not_active","bit":6, "inValue":true , "uri":"/status/pcs:mode","outValue":"Standby"},
{"enable":"/controls/pcs:estop_not_active","bit":5, "inValue":true , "uri":"/status/pcs:mode","outValue":"Stop"},


"pcs_status":
{"bit":4, "inValue":true , "uri":"/status/pcs:mode","outValue":"E_Stop"},
{"bit":4, "inValue":true , "uri":"/controls/pcs:estop_active","outValue":true},
{"bit":4, "inValue":false , "uri":"/controls/pcs:estop_not_active","outValue":true},
{"bit":4, "inValue":false , "uri":"/controls/pcs:estop_active","outValue":false},

"estop_active":

{"inAv":"/components/pcs:pcs_status","bit":4, "inValue":true , "uri":"/status/pcs:CurrentCmd","outValue":0},
{"inAv":"/components/pcs:pcs_status","bit":4, "inValue":true , "uri":"/status/pcs:dc_contactor","outValue":"open"},


"estop_not_active":

{"inAv":"/components/pcs:pcs_status","bit":7, "inValue":true , "uri":"/status/pcs:mode","outValue":"Run"},
{"inAv":"/components/pcs:pcs_status","bit":6, "inValue":true , "uri":"/status/pcs:mode","outValue":"Standby"},
{"inAv":"/components/pcs:pcs_status","bit":5, "inValue":true , "uri":"/status/pcs:mode","outValue":"Stop"},


The system allows you to "inject" "test" operations into it at run time to simulate and evaluate these sequences.


 Lets summarize what we have so far.

 1/ we can inspect an incoming value and react to the statuses of individual bits
   {"bit":4,"inValue":false .... }

2/ we can cause this evaluation to cause other variables in the system to have their values set to arbitrary values.
   {... "uri":"/status/pcs:mode","outValue":"Run"}

3/ we can enable or disable steps in the action sequences.
   { "enable":"/controls/pcs:estop_not_active" ...}

4/ we can cascade actions from one variable to another.
   "estop_not_active":
   {......}

5/ we can select a different variable as an input to a sequence step.
   {"inAv":"/components/pcs:pcs_status" ...}

There are other ways to do this as well but , depending  on your requirements, the "actions" way of doing this may well be the one to choose.

## Action line 

The Action line  is a json object that describes and action or operation associated with an action type.

An action line can have a number of options  and instructions that are used to define the action operation.
The simplst being the "remap" action.
```
   Send my value to another variable.

   {"uri": "/controls/bms:bm_oncmd"},
```

This action can be tested as follows

   fims_send -m set -r /$$ -u /ess/controls/bms/command  "Power On"

   fims_send -m get -r /$$ -u /ess/controls/bms/bm_oncmd  
            "Power On"


Note The action definition to make this happen is :

```json
{
 "/controls/bms":
   {
    "command":{ 
        "value":"dummytext",
        "actions": {
            "onSet": [{
                "remap": [
                  {"uri": "/controls/bms:bm_oncmd"}
                  ]
            }]
        }
    }
}
```

## Action Types

Actions are defined in the following types :

```
fims_send -m get -r /$$ -u /ess/full/system/actions| jq
{
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
```
The flexibility in decoding and manpulating individual actions has provided a feature overlap in some cases
For example the bitset action has been just about replaced by this operation.

```
fims_send -m set  -u /ess/controls/bms/output_state[3] true
  " set bit 3 of the output state variable to a 1"
```


## Action Blocks

Actions are contained in action blocks attached to a variable.
Here is an example.

```json
{
 "/controls/bms":
   {
    "test_bitmap":{ 
        "value":0,
        "actions": {
            "onSet": [{
                "bitmap": [
                  {"bit": 1,"uri": "/controls/bms:bm_oncmd",       "outValue": true  },
                  {"bit": 2,"uri": "/controls/bms:bm_oncmd",       "outValue": false },
                  {"bit": 3,"uri": "/controls/bms:bm_offcmd",      "outValue": true },
                  {"bit": 4,"uri": "/controls/bms:bm_offcmd",      "outValue": false },
                  {"bit": 5,"uri": "/controls/bms:bm_acclosecmd", "outValue": true },
                  {"bit": 6,"uri": "/controls/bms:bm_acclosecmd", "outValue": false },
                  {"bit": 7,"uri": "/controls/bms:bm_acopencmd",  "outValue": true  },
                  {"bit": 8,"uri": "/controls/bms:bm_acopencmd",  "outValue": false }
                  ]
            }]
        }
    }
}
```

## Action block control

The whole action block can be controlled by "action control" parameters.
These can b general purpose , in that they are used in every action, or specific to a particuar action type.
For Example the "defaultUri" and "defaultVal" parameters are only used by the "enum" action.


```json
{
 "/controls/bms":
   {
    "test_bitmap":{ 
        "value":0,
 
        "note1":"=========start of action controls",
        "debug":true,
        "enable":"/controls/bms:enable_test_bitmap",
        "defaultUri":"/controls/bms:default_state",
        "defaultVal":"Unknown",
        "ifChanged":true,
        "resetChange":true,
        "note2":" =========end of action controls",

        "actions": {
            "onSet": [{
                "bitmap": [
                  {"bit": 1,"uri": "/controls/bms:bm_oncmd",     "outValue": true  },
                  {"bit": 2,"uri": "/controls/bms:bm_oncmd",     "outValue": false },
                  {"bit": 3,"uri": "/controls/bms:bm_offcmd",    "outValue": true },
                  {"bit": 4,"uri": "/controls/bms:bm_offcmd",     "outValue": false },
                  {"bit": 5,"uri": "/controls/bms:bm_acclosecmd", "outValue": true },
                  {"bit": 6,"uri": "/controls/bms:bm_acclosecmd", "outValue": false },
                  {"bit": 7,"uri": "/controls/bms:bm_acopencmd",  "outValue": true  },
                  {"bit": 8,"uri": "/controls/bms:bm_acopencmd",  "outValue": false }
                  ]
            }]
        }
    }
}
```

## Action Item control
Several of the Action Block control parameters can also be used inside an action Item.
control parameters defined inside an Action Item only apply to the individual item and not to the block as a whole.
Action Item Controls here is a selection of item control parameters 

```
   "debug":         - turns on debug for the action line
   "uri":           - define the target uri "/controls/bms:bm_oncmd",     
   "ruri":          - define the uri that defines the target uri route "<route">:bm_oncmd" see notes,     
   "bit":           - select the bit to be tested
   "mask":          - select only certain bits of the incoming value to be used in validatin tests
   "shift"          - shift the input value before masking ( eases definition spec)
   "outValue":      - select the output Value to be sent to the uri
   "inValue":       - select the input Value to be used in validation tests
   "enable":        - uri to be used to allow the action to be disabled
   "ifChanged":     - stops the action line from runing unless the value has changed.
   "aSkip":         - ( new in 1.2.0 ) skip the next "n" action lines n= -1 means skip th remainder of the block
   
```

Data used in certain operations can be extracted from other asset vars.

```
   "inAv":          - use this assetVar as the input source instead of the defined var
   "enable":        - use this assetVar as the input source instead of the defined var
   "inVar":         - use this assetVar to provide inValue
   "outVar":        - use this assetVar to provide outValue
```

The action control parameters are described here;

# debug  
(true:false)
Used to debug a whole action block and / or each individual action.

# uri
(/controls/bms:<some_uri>)
define the target destination uri for the action output  "/controls/bms:bm_oncmd"

# ruri

(/controls/bms:<some_uri_route>)
defines the component name for the target destination uri for the action output  "/controls/bms:bm_oncmd"

This is used in systems where two registers are used to define a control or a status.

The first register "bms_status_select" defines the source of the data value 

0: /components/bms_info:
1: /components/bms_rack01:
2: /components/bms_rack02:
3: /components/bms_rack03:
4: /components/bms_rack04:

The second register " bms_status" defines the working value

0: Idle
1: Stop
2: Standby
3: Run
4: Fault
5: E-Stop

So when the data arrives from the BMS (in this case)
we get

/components/bms_onfo:bms_status_select:   3
/components/bms_onfo:bms_status:   4

The result will be placing the value "Fault" into the
"/components/bms_rack03:bms_status" variable.

   "ruri":          - define the uri that defines the target uri route "<route">:bm_oncmd" see notes,     
   "bit":           - select the bit to be tested
   "mask":          - select only certain bits of the incoming value to be used in validatin tests
   "shift"          - shift the input value before masking ( eases definition spec)

# bit
      select the bit in inValue to be tested

# mask
      select only certain bits of the inValue to be used 
          
# shift 
        shift the input value before masking ( removes nasty logical value calcs )
        Often a manufacturer's spec will create a 4 bit block for status and then
        apply that same status  to different 4 bit blocks in the same register;

bits 0-3  bms_status
bits 4-7  rack01_status
bits 8-11  rack02_status
bits 12-15  rack03_status

This can be represented in the actions block 

```
         {"shift":0,"mask":4,"inValue": 0,"uri":"/controls/bms:bms_status","outValue": "Idle"  },
         {"shift":0,"mask":4,"inValue": 1,"uri":"/controls/bms:bms_status","outValue": "Stop"  },
         {"shift":0,"mask":4,"inValue": 2,"uri":"/controls/bms:bms_status","outValue": "Standby"  },
         {"shift":0,"mask":4,"inValue": 3,"uri":"/controls/bms:bms_status","outValue": "Run"  },
         {"shift":0,"mask":4,"inValue": 4,"uri":"/controls/bms:bms_status","outValue": "Fault"  },
         {"shift":0,"mask":4,"inValue": 5,"uri":"/controls/bms:bms_status","outValue": "E-Stop"  },

         {"shift":4,"mask":4,"inValue": 0,"uri":"/controls/bms:rack01_status","outValue": "Idle"  },
         {"shift":4,"mask":4,"inValue": 1,"uri":"/controls/bms:rack01_status","outValue": "Stop"  },
         {"shift":4,"mask":4,"inValue": 2,"uri":"/controls/bms:rack01_status","outValue": "Standby"  },
         {"shift":4,"mask":4,"inValue": 3,"uri":"/controls/bms:rack01_status","outValue": "Run"  },
         {"shift":4,"mask":4,"inValue": 4,"uri":"/controls/bms:rack01_status","outValue": "Fault"  },
         {"shift":4,"mask":4,"inValue": 5,"uri":"/controls/bms:rack01_status","outValue": "E-Stop"  },

         {"shift":8,"mask":4,"inValue": 0,"uri":"/controls/bms:rack02_status","outValue": "Idle"  },
         {"shift":8,"mask":4,"inValue": 1,"uri":"/controls/bms:rack02_status","outValue": "Stop"  },
         {"shift":8,"mask":4,"inValue": 2,"uri":"/controls/bms:rack02_status","outValue": "Standby"  },
         {"shift":8,"mask":4,"inValue": 3,"uri":"/controls/bms:rack02_status","outValue": "Run"  },
         {"shift":8,"mask":4,"inValue": 4,"uri":"/controls/bms:rack02_status","outValue": "Fault"  },
         {"shift":8,"mask":4,"inValue": 5,"uri":"/controls/bms:rack02_status","outValue": "E-Stop"  },

         {"shift":12,"mask":4,"inValue": 0,"uri":"/controls/bms:rack03_status","outValue": "Idle"  },
         {"shift":12,"mask":4,"inValue": 1,"uri":"/controls/bms:rack03_status","outValue": "Stop"  },
         {"shift":12,"mask":4,"inValue": 2,"uri":"/controls/bms:rack03_status","outValue": "Standby"  },
         {"shift":12,"mask":4,"inValue": 3,"uri":"/controls/bms:rack03_status","outValue": "Run"  },
         {"shift":12,"mask":4,"inValue": 4,"uri":"/controls/bms:rack03_status","outValue": "Fault"  },
         {"shift":12,"mask":4,"inValue": 5,"uri":"/controls/bms:rack03_status","outValue": "E-Stop"  },

```

# ifChanged.
(true:false)
Used to control a whole action block and / or each individual action.

So far, in the pcs_status example,  every input to pcs_status causes the actions to be evaluated.
This may not be the best thing todo if this value is being sent to the system on a periodic basis.

the ifChanged logic is used to only execute a set of astion instructions if a difference has been detected in the 
incoming value.
Continually sending 21 to pcs_status will cause the operations defined in the action to be re valuated despite the vaule not having changed.

The ess_controller can specify if an individual action is executed only if the value has changed , or , a whole sequence of actions can be skipped if the value has not changed.

The "ifChanged" flag is used for this operation. If this flag is set to "true" 
then the action will only be executed if the variable has changed its value.

If the "ifChanged" flag is missing , or it is "false" 
then the action will be executedreardless of the variable value change.

If the "ifChanged" flag is set to "true" at the beginning of a st of action sequences. 
The system has the option to reset the value changed status. 
The "resetChange" flag controls this , if set to "false" the valueChanged flag is not automatically reset.
  
# ignoreVal
# ignoreVar

# inValue
This is used in certain actions to specify a value used to compare with the incoming  value to trigger an action.
In this example when we get an input value of 5  then the rack03_status to "E-Stop"
```
   {"inValue": 5,"uri":"/controls/bms:rack03_status","outValue": "E-Stop"  },
```

# inNValue (future 1.2.0 will be revised)
This is used in certain actions to specify a value used to compare with the incoming value to trigger an action.
In this example when we get an input value anything other than  5  then the rack03_status to "Fault"
```
   {"inNValue": 5,"uri":"/controls/bms:rack03_status","outValue": "Fault"  },
```

# outValue
This is used in certain actions to specify a value used as an output for the action.
In this example when we get an input value of 5  then the rack03_status to the outValue ("E-Stop") 
```
   {"inValue": 5,"uri":"/controls/bms:rack03_status","outValue": "E-Stop"  },
```

# outNValue (1.2.0 will be revised)
This is used in certain actions to specify a value used as an output when the inValue is not matched.
In this example when we get an input value anything other than  5  then the rack03_status to "Fault"
```
   {"inValue": 5,"uri":"/controls/bms:rack03_status","outNValue": "Fault"  },
```

# enable
This is a variable used to enable or disable and action block or an action line.


## Indirect values
The action item can also use indirect values for items such as selected AV , inValue, enable, outValue.

The scheduler ( covered elsewere ) works by sending a time value to selected variables.
The action in these cases will often use the "inAv" key to say. "Every 100ms do this action on this other var".

In this case the "inAv" key is used to select the aV to be used instead of the time value sent to the "scheduled" aV.
The "inAv" key will place the indirectly derived value  into "inAval" action iem key.

Here is a summary of all the indirect Variable options.

# inAvar
Place the value from inAvar into inAval rather than the incoming av's Value.
# inVar
Place the value from inVal into inValue instead of a constant value.
# enable
Place the bool val into the enabled key.
# outVar
Place the value from outVar into outValue rather than a constat value.

## Value definitions
The value defition can also be simple or progressively more complex.
The same definition format is used for input and  output values.

"/controls/bms:MaxCurrent" --  get or set the value  of the component.

"/controls/bms:MaxCurrent@MaxTime" --  get or set the "MaxTime" Param of the MaxCurrent component.

"/controls/bms:MaxCurrent[2]" --  get or set the value  of bit [2] of the component.

"/controls/bms:MaxCurrent@MaxTime[2]" --  get or set bit [2] of  the "MaxTime" Param of the MaxCurrent component.

## Action Definitions

Actions are defined in the following types :

```
fims_send -m get -r /$$ -u /ess/full/system/actions| jq
{
  "bitfield": {
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
```

# bitfield
  Decode input value bits  into a number  of different outValues for different uris
  If the selected bit is "set" and the input value is not ignored then set the uri assetvar to the outvalue.
  (Currently) does use ignorevalue but does not check against invalue for a match.



# bitset
  Needs review
  (Currently) does not redirect input value to avar.
  allows a combination of "uri" and "var" to determine the output Av
  reads the current output value
  If the input value  is true set the bit in the output value.
  If the input value  is false clear the bit in the output value.
  If the soloBit option is set then the whole output value is set to the single bit value.
  (Currently) does use ignorevalue but does not check against invalue for a match.


# bitmap
   This one is a real mess. we'll probably deprecate it.
   the rework completed in 1.1.0 it the template for the 
   Basically map a string value to an output bit.
   Needs review and rework.
   If the input value is a string and equals inValue then set  bit inoutval.
   If a negate is detected then clear the bit.
   If the input value is a number then do the same comparison.
   ... rework
   Take the (redirected) assetVar , compare with a (redirected) inValue and if true set(or clear) the bit in the output uri. uses new af action to cause set or clear
   If the input value is the same as th ignore value then discard.

  
# enum
    This is one of the real workhorses.
   Uses mask and values and indirect input vars , asset vars and output vars.
   Mask and shift the (redirected ) assetVar , compare the result with the (redirected) inValue, and set the (redirected) outValue in to the var selected by the uri ( or uri and ruri combination).
   This one also has the concept on numerical ranges and the idea of being in a range or not.

# remap
   Another workhorse.
   Take the (redirected) asset Var , compare with a (redirected) invalue and if true set the (redirected) outvalue to the uri ( or uri plus ruri) combination.
  Also has the concept of not equal.

# limit
  Take the (redirected) assetVar  , compare with (redirected) max / min limits and place the result  in the uri.

# func
  Another workhorse
  Apply the (redirected) assetVar to  a function. ( possibly use an inValue  to enable )


## Basic Scheduling

The Scheduler simply sends the current time to an assetVar.
That assetVar can have actions associated with it.
In the simple case the assetVar runs a function with all the debug and enable options applied.
in the most simple case the system can be asked to trigger the schedule operation to an assetVar every x seconds.
The schedule condition is assigned a control var to allow this operation to be rescheduled ( give a different time period) , paused, stopped, restarted, or given a different priority.
More details are in the scheduler documantation.

# Pub scheduling
The scheduler sends the current time every pub period to a control assetVar.
The control assetVar will run a Pub function (naked or otherwize) based on a table of values ( /site/ess_hs)

# Monitor Sceduling
The scheduler sends the current time every monitor  period to a control assetVar.
The control assetVar will run a max/Min value checking, dbi access checking etc on a list of values

# Control Scheduling   
The scheduler sends the current time every pub period to a control assetVar.
The control assetVar will run a control function  for a particular component.


