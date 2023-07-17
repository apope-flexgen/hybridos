DNP3 Interface Release Notes 
    pr 44 feature/test_crob_output
    notr this pr needs to be updated by pr's 45 and 46

P Wilshire 03_08_2023  initial entry

     
# Introduction

The hotfix for release 3.0.6 left a couple of items unfixed.
* 
* Crob input on Client system now handles full range of possible inputs  
    * ints 0,1,2,3,4,5
    * strings NUL,PULSE_ON,PULSE_OFF,LATCH_ON,LATCH_OFF
    * true , false  (by defaut mapped to LATCH_ON-3 LATCH_OFF-4)
* invalid or unknown values are either ignored or translated to "Unknown" (255)



## Crob configuration can be complex
The Crob MVP ( Minimum Viable Product ) Operation was to accept the numbers 3 or 4 as inputs and to produce true or false as outputs

```
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT 3     =>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT 4     =>    false
```

## Modified Crob single Uri naked input

```
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  true     =>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  false     =>    false

fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  LATCH_ON     =>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  LATCH_OFF     =>    false

```
## Modified Crob single Uri clothed input

```
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":3}'            =>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":4}'            =>    false

fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":true}'         =>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":false}'        =>    false

fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":"LATCH_ON"}'   =>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":"LATCH_OFF"}'  =>    false

```

## Modified Crob multi Uri naked input

```
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":true}'           =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":false}'          =>     flase
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":3}'              =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":4}'              =>     false
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":"LATCH_ON"}'     =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":"LATCH_OFF"}'    =>     false


```

## Modified Crob multi Uri clothed input

```
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":{"value":true}}'        =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":{"value":false}}'       =>     flase
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":{"value":3}}'           =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":{"value":4}}'           =>     false
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":{"value":"LATCH_ON"}}'  =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT":{"value":"LATCH_OFF"}}' =>     false

```


## Crob Client Configuration

### Basics 

The default configurations will work as before but the optional configuration items now work properly and make sense.

The problem with crob is that, when connected to a  customer client, you may not know what they are sending you.

The default operation is to convert "LATCH_ON"/"LATCH_OFF" from the customer to  a true/false outputs on our server.

Other possible input options from the customer client include "NUL" "PULSE_ON" and "PULSE_OFF".

These other options are, by default, ignored by the FlexGen server.

However, if needed, the server can be configured to control the server output from these values.

Options in the server config include:
 * (crob_int:true) output the raw crob value
 * (crob_string:true) the string crob value 
 * (crob_bool:true) translate any of the incoming crob values into a true or false output.
           (use the crob_true and crob_false config options in the server to specify the crob valuse to be used to produce true or false outputs)

When using the FlexGen DNP3 client with  a customer dnp3 server, the "set" input to a crob data poiint is now very flexible.

Any of the three input options are now decoded properly with (almost) no configuration required.

 * string "NUL","PULSE_ON","PULSE_OFF","LATCH_ON","LATCH_OFF" are all converted to "crob" values
 * integers 0,1,2,3,4  are converted into "crob" values
 * "true","false" default to "LATCH_ON" and "LATCH_OFF".
         config options for "crob_true" and "crob_false" can be used to translate different "crob" values into the true/false outputs.

This system should be much simpler to use, save a lot of questions and confusion from the integration team and be sufficiently flexible for all FlexGen customers' interfaces.

(This rework would will be used "as is" if we move to a different dnp3 transport system.)

## Crob details

A number of different inputs are possible for the Client, these are  controlled using different flags  on the  input  configuration.

The incoming "crob" value  can be a string , int or bool
The simplest (default) option is  Set with a "true" value  and the crob server output also is "true"



If we are using a bool, the meaning of true/false  can be  translated into a "crob" value if "crob_true" or "crob_false" are defined in the config.

If this client config items is given a "true" or "false" input. 

The actual server crob value is derived from the contents of "crob_true" and "crob_false"

```
{
    "id": "TESTCROB_BOOL_PULSE_ON",
    "crob_true":"PULSE_ON",
    "crob_false":"PULSE_OFF",
    "offset": 4,
    "name": "TEST_CROB_BOOL_PULSE_ON"
}

```

```
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_BOOL_PULSE_ON":{"value":true}}'        =>     true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_BOOL_PULSE_ON":{"value":false}}'       =>     flase
```

Note that the server side must have some config options set to correctly decode the incoming crob values.

 * Setting "crob_int":true will use  incoming crob value (0-5) 
 * Setting "crob_string":true will translate the incoming  crob value (0-5) to a string.
 * Setting "crob_bool":true will  decode the crob value into a true / false output
     The default is "LATCH_ON" (3) for true , "LATCH_OFF" (4) for false


The "crob" value may also be set with the words PULSE_ON , PULSE_OFF, LATCH_ON, LATCH_OFF etc
Another option is to use an integer number.

The current system works as follows
* default

```
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT true
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT": true}'
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT": 1}'
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT": 2}'
fims_send -m set -u /sites/brp_sierra '{"TESTCROB_DEFAULT": 3}'  
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT 3         
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT 4         
```


# crob output options on the server

The dnp3_server crob output has several options.

The simplest is to use the boolean value for the crob output  
This option will default to using the LATCH_ON state (3) as the boolean value true
         and the LATCH_OFF state (4) as the boolean value false

Currently, the actual crob value gets returned on the client

```
fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT true
{"TESTCROB_DEFAULT":3}
```

and the defaut (fims_listen) output on the server

```
    Method:       set
    Uri:          /sites/brp_sierra/TESTCROB_DEFAULT
    ReplyTo:      (null)
    Process Name: DNP3_O_brp_sierra
    Username:     root
    Body:         true
    Timestamp:    2023-02-06 15:02:47.131933
```

Likewise for a false value , the actual crob value gets returned on the client

```
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT false
    {"TESTCROB_DEFAULT":4}
```


and the defaut (fims_listen) output on the server

```

    Method:       set
    Uri:          /sites/brp_sierra/TESTCROB_DEFAULT
    ReplyTo:      (null)
    Process Name: DNP3_O_brp_sierra
    Username:     root
    Body:         false
    Timestamp:    2023-02-06 15:03:56.402738

```

The server output can be modified in the server configuration.

```
    "crob_int":true,

```
```
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT false ===>

    Method:       set
    Uri:          /sites/brp_sierra/TESTCROB_DEFAULT
    ReplyTo:      (null)
    Process Name: DNP3_O_brp_sierra
    Username:     root
    Body:         4
    Timestamp:    2023-02-06 15:08:32.865238

    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT true ===>

    Method:       set
    Uri:          /sites/brp_sierra/TESTCROB_DEFAULT
    ReplyTo:      (null)
    Process Name: DNP3_O_brp_sierra
    Username:     root
    Body:         3
    Timestamp:    2023-02-06 15:09:35.446123
```

Notice that this mode also allows limited other crob values to be sent to the server 
```
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 0  ==> 0
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 1  ==> 1
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 2  ==> 2
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 3  ==> 3
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 4  ==> 4
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 5  ==> 255
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT 6  ==> 255
```

This other input formats also work.

```
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT NUL  ==> 0
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT PULSE_ON  ==> 1
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT PULSE_OFF  ==> 2
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT LATCH_ON  ==> 3
    fims_send -m set -r/$$ -u /sites/brp_sierra/TESTCROB_DEFAULT LATCH_OFF  ==> 4

```


The server output can be modified in the server configuration to output strings for the CROB state.

```
    "crob_string":true,

```

Example output

```
    Method:       set
    Uri:          /sites/brp_sierra/TESTCROB_DEFAULT
    ReplyTo:      (null)
    Process Name: DNP3_O_brp_sierra
    Username:     root
    Body:         "LATCH_ON"
    Timestamp:    2023-02-06 15:21:25.67110
```
# review of output options

## "crob_int":true

  Te output is the raw crob number based on the crob state coming from the client
  0 -> NUL
  1-> PULSE_ON
  2->PULSE_OFF
  3->LATCH_ON
  4->LATCH_OFF

## "crob_string":true

The output is a string related to the crob state coming from the controller
  0 -> "NUL"
  1-> "PULSE_ON"
  2-> "PULSE_OFF"
  3-> "LATCH_ON"
  4-> "LATCH_OFF"


  ## "crob_bool":true

  You will get a true/false output  related to the crob state coming from the controller

    3-> "LATCH_ON"  -> true
    4-> "LATCH_OFF" -> false

## "crob_bool" with "crob_true" and crob_false

You can use strings or numbers with the config keys "crob_true" and "crob_false" to define the crob state uses for true/false outputs. 

        "crob_true":1
        "crob_false":2
or

        "crob_true":"PULSE_ON"
        "crob_false":"PULSE_OFF"

Causes the "PULSE_ON" and "PULSE_OFF" crob states to be used to trigger   


# get uri options added 

The "get" uri has ways to override the configured format flags.
This is similar to the way the ess_controller works.
Note that , as yet, the "full" format does not return all the features of the datapoint .
It does,however, allow the user to look at the dnp3 data flags.

```
# client looking at a CROB output value

fims_send -m get -r/$$ -u /local_client/naked/sites/brp_sierra/TESTCROB_DEFAULT
{"TESTCROB_DEFAULT":false}

fims_send -m get -r/$$ -u /local_client/clothed/sites/brp_sierra/TESTCROB_DEFAULT
{"TESTCROB_DEFAULT":{"value":false}}

fims_send -m get -r/$$ -u /local_client/full/sites/brp_sierra/TESTCROB_DEFAULT
{"TESTCROB_DEFAULT":{"value":false,"sflags":"Init","flags":1}}


# client looking at an analog input value

(default config value == clothed)
fims_send -m get  -r/$$ -u /sites/brp_sierra/U1_UGMW_GEN7
{"U1_UGMW_GEN7":{"value":0}}

fims_send -m get  -r/$$ -u /local_client/naked/sites/brp_sierra/U1_UGMW_GEN7
{"U1_UGMW_GEN7":0}

fims_send -m get  -r/$$ -u /local_client/clothed/sites/brp_sierra/U1_UGMW_GEN7
{"U1_UGMW_GEN7":{"value":0}}

fims_send -m get  -r/$$ -u /local_client/full/sites/brp_sierra/U1_UGMW_GEN7
{"U1_UGMW_GEN7":{"value":0,"sflags":"RESTART","flags":2,"stime":"2023-02-06 14:29:16.044","etime":"2023-02-06 14:29:16.044"}}

```

# Server response to pubs

The prefixed format for the server may not be working

```
 fims_send -m pub  -u /local_server/clothed/sites/brp_sierra '{"U1_UGMW_GEN7": 3344}'
```

The non prefixed format for the server is working

```
 fims_send -m pub  -u /sites/brp_sierra '{"U1_UGMW_GEN7": 3344}'
 fims_send -m pub  -u /sites/brp_sierra/U1_UGMW_GEN7 3344

```


# hotfix 3.0.6

The hotfix for release 3.0.6 is intended , primarily, to assist with the OpenDnp3 server stall caused by the incoming event buffer overflow.
This situation caused the Client to stop receiving and publishing data from the server under certain circumstances.
This problem is a known deficiency of the OpenDnp3 package used by the DNP3 Interface.
The problem is triggered by Direct Operate Functions  (sets) from the DNP3 client causing processing delays in the Server which prevent it responding to  client class scan  requests in a timely manner., the last value  This, in turn , prevents the input buffer from being emptied and triggers the unwanted behaviour.

This hotfix introduces batch mode operation  options for incomning data to the Client and outgoing data from the Server.

# Change Summary

* Batch mode
* late_pub_detection
* better format control
* crob output modes int|bool|string


# Batch Mode 

## Client 

This means that several individual set requests are combined into a single request within the client. After the "batch set" time has elapsed, the last value applied to the datapoint is sent to the Server from the client.

NOTE :: currently, only the last value sent to the Client is procesed in batch mode.

Batch mode also allows several individual data points to be collected into a single message to the server which increases the efficiency of the transaction.
In the Client a variable , "batchSet" is used to provide a time in milliseconds used to collect multiple set requests.
At the end of the time out period any pending data set operations are combined to a single DNP3 Operation.

This is a global variable, it can be set at run time, used to control the batch set operation.
Individual data points can "opt out" of the batch  flag set operation using a "notBatched"  and be dispatched from the client to the server immediately.

## Server 

The Server also has a "batch mode" option.
The same config flag is used to control this operation.
On the Server, multiple "set" commands from the Client are combined, if possible, into a single write operation.
The varaibles from a single Uri are combined with values received over the batch period.
This control can be set or cleared ( set to 0) at run time.

# Late Pub detection.

The Client records the time  a data packet is received from the server. When setting up the regular integrity scan  these packets should arrive at the requested scan frequency.

The Client checks that the time after the last data packet arrival does not exceed a maximum time limit.
The Client will issue an event when it first detects this situation.


Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"DNP3 Timeout [0.504] exceeded  [0.500]\n","severity":1}
Timestamp:    2023-02-03 10:47:43.776988

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"DNP3 Timeout recovered\n","severity":1}
Timestamp:    2023-02-03 10:47:07.256124

The pub timeout can be set in the system config or set at runtime with the following  fims command.
```
    fims_send -m set -u/local_client/_system '{"max_pub_delay":500}'
```

## Increase batch set time

Options are availabe to delay batched writes to the Server if the incoming scan responses (for pubs) are not received in a timely manner.
In addition to the event generation, the batch time can be extened to attempt to allow the server time to process the scan requests.
An optional variable "batchSetsMax" can be used to limit the extent of the batch set time. If not set, the automatic extending of the batch time will not be performed.

The delayed batch_set time  can be set in the system config or set at runtime with the following  fims command.
```
    fims_send -m set -u/local_client/_system '{"batchSets":100}'
    fims_send -m set -u/local_client/_system '{"batchSetsMax":1000}'

```

# New Config Items

Name         default       runtime 
"batchSet"      0          yes    /sysname/_system '{"batchSet":value}'
"batchSetMax"   0          yes    /sysname/_system '{"batchSetMax":value}'
"maxPubTime     0          yes    /sysname/_system '{"maxPubTime":value}'
"notBatched"    false      yes    /component/uri   '{ "<name>":{"_system":"notBatched","_value":true}}'
"event_buffer"  100        no      config system   

## event_buffer 

This sets the size of the event_buffer in the Server. It defaults to 100 but can be set between 0 and 512.
If you have several points in an input uri that may change value between pubs then set this value higher.

# Older Config Items

"frequency"     500        yes    /sysname/_system '{"frequency":value}'


# output formats

## naked

The variable is sent as a  name: value pair  

## clothed

The variable is sent as a named object with a "value" attribute  '("vname":"{"value":1234})

Variable config 
                {
                    "id": "U1_UGMW_GEN7",
                    "offset": 0,
                    "format": "clothed",
                    "variation": "Group30Var2",
                    "name": "U1 UNIT GROSS MEGAWATTS"
                },

pub output

Method:       pub
Uri:          /sites/brp_sierra
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {
    "52_2_LOW_SIDE_CIRCUIT_BREAKER":false,
    "U1_UNIT_LOCAL_REMOTE_CONTROL":true,
    "U1_UNIT_AUTHORITY_SWITCH_ISO":false,
    "GOV_BLOCK":false,
    "BESS_IDLE":true,
    "BESS_NOT_IDLE":true,
    "U1_UGMW_GEN7":{"value":1},   <<< clothed format
    "U1_UGMV_GEN7":{"value":200,"sflags":"RESTART","flags":2} <<< full format
    "U1_UOLL_GENX":3,"U1_UOHL_GENX":1,"U1_CTLFDBK_GENX":4,"GOV_DRP":0,"GOV_DB":4,"OPER_RR":2,"NUMBER_OF_ONLINE_INVERTORS":0,"SOC_GEN_MWHX":1,"MXENERGY_GEN_MWHX":0,"Timestamp":"02-02-2023 22:37:52.954105"}
Timestamp:    2023-02-02 22:37:52.954372
The variable is sent as a named object with a "value" and other attributes  '("vname":"{"value":1234})



This is a get on a clothed varible

sh-4.2# fims_send -m get -r /$$  -u /sites/brp_sierra/U1_UGMW_GEN7
{"U1_UGMW_GEN7":{"value":0}}

# server cmd output differs between batched mode and direct mode 

Note the singleton set output changes when using batchSets 
```
not batched naked output
   /sites/brp_sierra/U1_UGMW_GEN7 2345
```

```
batched naked output
/sites/brp_sierra {"U1_UGMW_GEN7", 2345}
```
```
batched /notbatched clothed output
/sites/brp_sierra {"U1_UGMW_GEN7", {"value":2345}}
```

# events 
The source now contains information about the DNP3 client or server that originated the event.

sh-4.2# fims_listen -u /events

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"state change [OPENING]\n","severity":1}
Timestamp:    2023-02-03 10:38:38.685966

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"state change [OPEN]\n","severity":1}
Timestamp:    2023-02-03 10:38:38.687803

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"state change [CLOSED]\n","severity":1}
Timestamp:    2023-02-03 10:39:05.810489

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"state change [OPENING]\n","severity":1}
Timestamp:    2023-02-03 10:39:05.810643

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_brp_sierra
Username:     root
Body:         {"source":"DNP3_client:brp_sierra","message":"state change [OPEN]\n","severity":1}
Timestamp:    2023-02-03 10:39:36.815967

* The source field now shows the application and associated id  that produces the  message



# Server set output

    Current DNP3 

"format":"clothed"
Method:       set
Uri:          /sites/docs_road/manual_ess_kw_cmd_dnp3
ReplyTo:      (null)
Process Name: DNP3_O_ncemc_fleetmanager_dnp3_server
Username:     root
Body:         {"value":3456}


"format":"naked" ( default)
Method:       set
Uri:          /sites/docs_road/manual_ess_kw_cmd_dnp3
ReplyTo:      (null)
Process Name: DNP3_O_ncemc_fleetmanager_dnp3_server
Username:     root
Body:         3456


Warning , current batched sets give you this 
But you may get this
"format":"naked" ( default)
Method:       set
Uri:          /sites/docs_road
ReplyTo:      (null)
Process Name: DNP3_O_ncemc_fleetmanager_dnp3_server
Username:     root
Body:         '{"manual_ess_kw_cmd_dnp3":3456}'


* missing 
* class designation for binary data.
  The ability to associate binary data points to a chosen class (1,2,3) has been added.
