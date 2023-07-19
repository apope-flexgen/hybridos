### Overview 

## Metrics
Need a way to modify incoming fims data  as required by the system controllers.

# Operators designed in 

* Extents - max, min
* Aggregates - integrate
* Math - sum, product, average, rss
* Bit Manipulation- and, or, enum, bitfield, count, bitfieldpositioncount
* Logic - srff, compare, compareand, compareor
* Utility - echo, select
* Special - runtime, unicompare, pulse, currentTimeMilliseconds, compareMillisecondsToCurrentTime, millisecondsToRFC3339, combineInputsToArray

Metrics can take its data from multiple sources on FIMS and can also use other metrics as inputs to pipeline complex calculations.

Metrics are stored until overwritten (extents, math, bitwise, single input) or updated (aggregates).


# Operators in current use

| Operator  | /NCEMC/multiple-inputs/dnp3 | brp_ffra/tx10_sungrow| ameresco_sce/powercloud|
|-----------|-----------------------------|-----------------------|-------------------------| 
| echo       | 53                          | 397                  | 171                     |
| product    | 8                           | 44                   | 27                      |
| select     | 4                           | 114                  | 18                      |
| compare    | 20                          | 13                   | 8                       |
| compareor  | 2                           | 21                   | 6                       |
| compareand | 2                           | 0                    | 0                       |
| ne         | 16                          | 4                    | 2                       |
| gt         | 2                           | 14                   | 6                       |
| average    | 1                           | 3                    | 0                       |
| max        | 0                           | 6                    | 0                       |
| zulu       | 2                           | 2                    | 2                       |
| lt         | 2                           | 2                    | 5                       |
| sum        | 1                           | 20                   | 2                       |
| selectn    | 0                           | 24                   | 0                       |
| bitfield   | 2                           | 2                    | 2                       |


### Current Echo Operations
Echo has an option to translate dnp3 and modbus client config files into server and echo config files. 
Does echo support naked and clothed inputs ?
How does echo handle strings ?
Note that Echo only processes floats and booleans I think.
Echo does not maintain a "state"

* add
* subtract
* multiply
* divide
* and
* or
* not
* forward
* eq
* neq
* lt
* gt
* lte
* gte

## Notes
Common between all metrics objects:
*    id: the last frag of the output component
    *   operation: The action that you perform on the input uris
    *  inputs: all the uris that you use for the operation, will use the uris from top to bottom
* Optional fields:
    * value: a single value that allows you to have a constant
    * param: used mainly for bitfield, but contains a map[string][]interface{}

* Add:
    Allows the user to add uris together from top to bottom of the inputs list
* Subtract:
    Allows the user to subtract uris together from top to bottom of the inputs list
* Multiple:
    Allows the user to multiple uris together from top to bottom of the inputs list
* Divide:
    Allows the user to divide uris together from top to bottom of the inputs list
* Not:
    Inverts the polarity of the boolean value that you pass through, must be boolean
* And:
    Ands together the boolean values that are provided in the inputs fields
* Or:
    Ors together the boolean values that are provided in the inputs fields
* Compare:
    Will compare up to 2 uris in the inputs field, value can be used if there is only one uri in inputs

## Comments

* Is this the best way to perform these operations
* What is missing from fims_echo
* Is fims_echo fast enough
* What about cpu loading    
* How do the different configs differ

## Design Thoughts
* Define the problem
    * Transfer data from one object to another with the option to modify that data as dertermined by  other data items.
* Needs to be able to scale tx10 , tx100, tx1000, tx10000
* Find best ways to:
    * define data inputs, outputs and operations.
    * accept incoming data into a process.
    * process that data
    * output the results
    * test the whole system
* Find best coding methods for node, go , c++
* Evaluate systems for input, process, output


* Understand current Echo and Metrics codebase.
    * Are the code paths easy to follow.  
* Undertake some simple tests to evaluate performance.
* Design "Desired" config instructions. 
    * What do we want the config instructions to look like.
        * Clarify echo/remap path
        * Manage Chained calculations
        * Aggregations that cope with "disabled" components.
        * Can we use templates to allow repeat operations based on a different base address or number of similar components ?   (./config/twins/config/echo/flexgen_ess_nn_modbus_echo.json)

* Produce automated testing for operations.
* Produce automatic translation for any new system.
* how long does it take to produce metrics / echo files.

## Design Flow (Go)
* logger [https://github.com/uber-go/zap]
* decode [https://yalantis.com/blog/speed-up-json-encoding-decoding]


* parse echo config add subs element
* simple fims receive listen to subs, create all vars as they arrive
    * note options allow the system to automatically expand to incude new variables or to make te system simply ignore new varaibles.

* create pub uris and vars from echo config.
* run pubs, use simple ticker for single pub rate expand to multi pub rates very quickly

* {not needed} Add ess scheduler create schditems sorted array at next time (float seconds ) concept but use a delay sshot ticker
* evaluate performance with the simple task. tx10 tx100 tx1000 tx10000
    * fine tune input parsing (simdjson for go)
    * optimize output parsing (spdlog for go )

* develop concept of calc blocks
    * attach blocks to pubitems
    * attach blocks to block inputs
* allow fee running input processor , use locks when processing blocks, unlock when pub since outputs are not touched by inputs



* we keep current metrics/echo configs but add an eval block



## Fims input

The Fims input is handled by a single  fims_connection which gets raw fims data  in the "body" component (not unmarshalled) 
The processing is then handed off to one of a group of worker threads.

The worker thread has to tasks.
* fims_decode unmarshall the body data
* fims_merge inject or merge the body into the vmap data , extend and metrics flags are used to allow the incoming data to extend the vmap and / or run any metrics.

The first task can be completed as soon as the fims data arrives. This gives us an unmarshalled object ready to be processed.
An average of 250 uSecs will be saved at this time. 
A go "input handler" thread is used for this operation.


The second task is time, or rather, sequence sensitive.
We must have all the incoming fims messages that arrived before the current message merged into the vmap before we can
merge any subsequent  object.
This task is performed by one or more processor threads.
Normally (in fact, probabably, always) there is one dedicated processor thread attached to a URI. 


The input handler runs as soon as the message arrives.
The Uri processor runs when all processing for prior messages have been completed.
See `fims input sync` for more details.

## Uri handling part 1

The key component in this system is the URI,
A URI has a list of ID's used to define an individual variable.
A Variable can have a value, name and a list of Parameters. In addition, a list of `Metrics` or operations can be attached to the `Pull` or `Push` processes associated with the Variable.


The original `metrics`  process had a list of `PublishUris` and then created a series of `Metrics` to produce each URI:ID combination 
This is an example of a `Pull` Operation. If we want to look at a value we need to get it from somewhere ans we may have to 
use one or more variables to help us get that value.

This system may introduce the concept or "SetUris". This is the process used by the ess_controller.
When an incoming value is given a (new?) value then we may have to `react` or  adjust other values in the system.
This is an example of a `Push` system.
In a simple `Push` system, output variables are simply placed in a collection of different URIs and the whole URI was published.

For example 
```
/components/bms_1/rack_01/ChargeCurrent was "sent" to /status/bms_1/rack_01/ChargeCurrent  and /status/ess_hs/bms_1_Rack_01_ChargeCurrent
                                         was multiplied by /status/bms_1/rack_01/Voltage   and sent to /status/bms_1/rack_01/Power
                                         etc.
```
The whole /status/ess_hs URI was published every 100mS.

A combination of `Push` and `Pull` systems provides the best solution for system flexibilty.
If an input variable needs processing and any resultant values also need futher procesing then use a `Push` process.
If an output variable is a `view` or an `observation` of the system for the benefit of upstream processes then a `Pull` peocess is the correct solution.

There may be no difference in the URI definition between `Pull` and `Push` peocessing. We'll try and design that difference out.

Some inputs will require no processing and can be simply passed through to a new output URI. The ess_controller called these `VLINKS`.
The same variable with all it `additions` under a different URI label.

Some inputs will require no processing on a value but will need to be published with a different set of `additions` are presented under a different URI.
The ess_controller called these `LINKS`.

The `fims_echo`  system recognised the `VLINK` operation and mapped inputs to outputs directly. The `echo 2` system will mimic that behaviour.
This allows zero cost mapping between components but , when multi threatding the system, we will have to redirect any read/write locks back to the original URI.
( NOte that the ess_controller retained the Original URI in the linked variable, /site/ess_hs/ChangeCurrent retained the original component:id names it was just mirrored under the /site/ess_hs output URI )


## Uri handling part 2 The URI Prefix

The system will recieve a flood of incoming URI's from its subscribed objects.
Each URI will have a number of ID's each of which will have values.

There are two main modes that can affect thes response to these incoming URIs

In configuration mode the system has to accept the incoming data as a map or set of instructions for future processing.
In run mode the system has to map the incoming data to its configured map and process data as required.

The system will also prepare a list of URI's to give to the fims_server to restrict the URI delivery to a defined set of options.
This list is produced by examining the configured URI list and producing a `subscription` list.

The system adds an extra item to this subscriptoin list , its own key . This is a technique used in the ess_controller to 
extend the subscription list to include additional URIs as needed.
The prefix can be removed by the input processer and the remainder of the URI then passed into the system.


So consider  that we are "echo_1" subscribed to :

```
/components/bms_1
/components/bms_2
/components/bms_3
/components/bms_4
/components/pcs_1
/components/pcs_2
/components/pcs_3

and 

/echo_1

```

If we get a message sent to any of the `/components/xxx` URIs the system will process those as "normal"
If we get a message sent to  the `/echo_1/config/components/ess` URI the system will process those as a "config" instruction.
The internal data map will be modified but no `metrics` actions will be performed.

If this new data map includes new subscriptions a new, additional, fims channel will be opened to allow those subscriptions to be received.
( A streach goal  will include a way to migrate all the subscriptions to the new fims channel)


Other prefix options will trigger other operations.

Here are some examples

```
/echo_1/perf/components/xxx                    show performance data related to a particular URI
/echo_1/config/metrics:add/components/xxx '{"id":"varname","metrics":[{"met1xxxx"}]}'     add or extend the metrics array for a  variable
/echo_1/config/metrics:rep/components/xxx '{"id":"varname","metrics":[{"met1xxxx"}]}'     replace the metrics array for a  variable

/echo_1/format:naked/components/xxx/<varname>     in a get , use naked format
/echo_1/format:clothed/components/xxx/<varname>     in a get , use clothed format
/echo_1/format:ui/components/xxx/<varname>     in a get , use ui format
/echo_1/format:config/components/xxx/<varname>     in a get , use config format, show metrics etc

... etc We'll extend this as required by ongoing design options.

```

## loaders and templates

The template scheme is useful to build up configurations that contain repeating elements , 75 ess controllers, for example.
Together with a template scheme you need a template loader.
This process executes the template process on one or more template files.

The URI prefix option is used to process this

/echo_1/ctmpl/<some name>    becomes the URI of a template file.
   It is saved as a raw text string and not unmarshalled

/echo_1/cload/<some name>    becomes a loader file.
   This contains instructions for loading config files and expanding templates

/echo_1/cfile/<some name> is a config file used in the loader process.
   This file is loaded in the vmap but it does contain template expansion details.

( The full ess_controller template system has been ported to go as part of the update tool development.)











## Fims Input Sync

Quick review on how I think this is done.

We have two threaded functions.
The worker thread is used to decode the fims messages and get them ready to merge into the vmap.
The processor thread sequentially merges object into the vmap.

When a fims message arrives it is sent to a worker thread.
The worker thread immediately sends a message to the merge thread saying "I'm Next".
This effectively sets up the correct sequence for the processor thread to manage the data.

The worker thread then unmarshalls the fims object into its temp store.
It then waits for a "send it" message from the processor thread.
This means that the processor thread has completed any current tasks and is ready for the next data object.  
Once the worker thread gets the "send it " message it sends the temp store to the processor thread.
The processor thread sends a response saying "got it". The worker thread can then terminate.

The processor thread has a queue of incoming "I'm next" messages.
As it accepts each item it sends a "send it" message to the worker thread and then waits for the worker thread response with the data.
Once it gets the data it can send back a "got it" message. 
The processor thread then processes the data and drops the temp object.
Once that data has been processed the processor thread can move on to the next "I'm Next" message from another  worker thread.


## Metrics blocks

The system uses a number of "metrics" blocks to handle set (Push) processing and "get" or "pub" (Pull) processing.
Each metrics block is identified by a uri and an id.
The variable defined by the uri/id combination will exist in the vmap.

A metrics block will have an operation (or function)
A metrics block will have a list of inputs.
A metrics block may have a list of outputs
A metrics block may have other params used in performing the function.
The controls available to a metrics block are "enable" (a Uri)  , "enabled" (a bool state) we may also have "output_enable".
The evaluation of a metrics block will normally update the Metrics URI + ID combination but optional outputs may be also be updated.

In the metrics.json configs, the metrics blocks were defined "inline"

```
"uri": "/components/magnolia",
            "naked": true,
            "metrics": [
                {
                    "id": "gen_net_mw",
                    "inputs": [
                        {
                            "uri": "/components/internal_magnolia",
                            "id": "discharging_flag"
                        },
                        {
                            "uri": "/components/magnolia",
                            "id": "gen_line_flows_mw"
                        }
                    ],
                    "operation": "select",
                    "param": {
                        "falseCase": 0
                    },
                    "initialInput": 0
                },
```
The metrics blocks can be defined in their own merit as well.

```
{
"uri": "/components/magnolia",
{
    "id": "gen_net_mw",
    "metrics": [
        {

        "enable":  {
                "uri": "/components/internal_magnolia",
                "id": "gen_net_mw_enable"
        },

        "inputs": [
            {
                "uri": "/components/internal_magnolia",
                "id": "discharging_flag"
            },
            {
                "uri": "/components/magnolia",
                "id": "gen_line_flows_mw"
            }
        ],
        "operation": "select",
        "param": {
            "falseCase": 0
        },
        "initialInput": 0
        }
    ]
    
}
```

Metrics objects remember the time when they were last processed ("_sys/last_processed" time).
When processing metrics in Pull mode, the input list is scanned and if any input uri has a "lastchanged" time greater than the "last processed" time then the metrics block will have to be re evaluated.
When Processing in Push mode, any metrics associated with any inputs will have to be processed, if needed,  and any output metrics processed also.

On a complex push or pull  the correct use of process time will eliminate any repeat processing for a metics component.

# SuperBlocks

When dealing with an array of  metrics  blocks for a given URI, the system creates a "super block" and collects all the inputs and outputs for each component , or metrics item in the block.
The collection can contain "internal" values that are NOT used outside that combined block operation.
The  collection can also contain external values that touch other uris. The system imposes read/write locks as needed to process these variables.
The metrics operation can produce values in one or more URI's that are distributed using system pubs. Direct outputs are also possible. These are issued as "set" messages once the whole block computation has completed for the URI.

# VLINKS
Metrics operations that are  "echo" commands will require no compute time. These values are simply `VLINK'd` together.

In this case the variable is represented as a pointer to the other value.
The go interface typing allows this to be easily incorporated the code. 






## Set/GetVar

These operations handle the setting or getting of a value or a param from a variable. 
An interface is returned to handle different var types.
Missing components, referenced in setVar, can be created as needed if the expand operation is enabled.
Any Metrics associated with the variable will be executed if the metrics flag is also enabled.

```
func setVarParam(vmap interface{}, flags int, uri, id , param string , value interface{}) (interface, error) {
    decode the uri, set up flags
    find the base uri , create one  if the noexpand flag is not set
    find the variable  (by ID ) , create one if the noexpand flag is not set, or return , we are not interested in this uri in this message.

    writelock the base uri 
    if param == value
    set valuechanged time if the value has changed
    set valuescanned time 
    set the  param in the object which must be a `map[string]interface{}` 
    see if the variable has any metrics
    write unlock component, run them if the metrics operation is enabled 

}
func setVar(vmap interface{}, flags int, uri, id string , value interface{}) (interface, error) {
    return setVarParam(vmap, flags, uri, id, "value", value)
}


func getVarParam(vmap interface{}, flags int, uri, id,  param string) (interface, error) {
    decode the uri, set up flags
    find the base uri, do not create one if missing 
    find the variable  (by ID ) , do not create one if missing
    if metrics and metrics are enabled , run get metrics
    writelock the base uri 
    get the param object in the variable object which must be a `map[string]interface{}` 
    writeunlock the base uri
}
func getVar(vmap interface{}, flags int, uri, id string ) (interface, error) {
    return getVarParam(vmap, flags, uri, id, "value")
}

```


## Example Metrics file

```
{
    "publishRate": 1000, // in ms, how often to publish all metrics
    "listenRate": 2, // in ms, how often to pull FIMS message
    "publishUris":[ // list of objects defining each of the uris
                    // that metrics publishes to and the associated metrics
        {   
            "uri": "/metrics/kpi", // baseURI of publish
            "metrics":[
                {
                    "id": "system_sum_diff_cell_temp", // appended to baseURI
                    "name": "System Sum/Diff Cell Temperature",
                    "scale": 1, // name, scale, unit, ui_type, type and 
                    "unit": "°C", // options are for the UI, if needed
                    "ui_type": "status", // if left off, they don't publish
                    "type": "number",
                    "options": [],
                    "inputs" : [ // list of uri/id pairs that make up 
                                 // the inputs to the operation. Some
                                 // operations are sensitive to order
                        {"uri":"/components/ess_1","id":"bms1_max_cell_temp"},
                        {"uri":"/components/ess_1","id":"bms2_max_cell_temp"},
                        {"uri":"/components/ess_1","id":"bms3_max_cell_temp"},
                        {"uri":"/components/ess_1","id":"bms4_max_cell_temp"}
                    ],
                    "operation":"sum", // refer to operations list
                    "param":{"operations":"+-+-"}
                    // the operations API list will define what parameters,
                    // if any, may be used for a particular operation
                }
            ]
        },
        {
            "uri": "/components/dio_echo",
            "naked": true, // if you want to publish naked, do so here
            "metrics": [
                {
                    "id": "di1",
                    "name": "Digital Input 1",
                    // unit and other UI should be left off when naked
                    "inputs": [ 
                        {"uri":"/components/ed588","id":"di1"} 
                    ],
                    // some operations allow their stored value to be
                    // `set` to another URI, all uri/id pairs in this list
                    // will get set with the same value
                    "outputs": [
                        {"uri":"/site/operation","id":"spareinput"}
                    ]
                }
            ]
        }
    ]
}
```
### How to define configs

### Metrics config
```
{
    "publishRate": 1000,
    "listenRate": 2,
    "publishUris": [
        {
            "uri": "/components/magnolia",
            "naked": true,
            "metrics": [
                {
                    "id": "gen_net_mw",
                    "inputs": [
                        {"uri": "/components/internal_magnolia","id": "discharging_flag"},
                        {"uri": "/components/magnolia","id": "gen_line_flows_mw"}
                    ],
                    "operation": "select","param": {"falseCase": 0},
                    "initialInput": 0
                },
                {
                    "id": "gen_net_mvar",
                    "inputs": [
                        {"uri": "/components/internal_magnolia","id": "discharging_flag"},
                        {"uri": "/components/magnolia","id": "gen_line_flows_mvar"}
                    ],
                    "operation": "select","param": {"falseCase": 0},
                    "initialInput": 0
                },
                {
                    "id": "gen_aux_mw",
                    "inputs": [
                        {"uri": "/components/internal_magnolia","id": "discharging_flag"},
                        {"uri": "/components/internal_magnolia","id": "gen_line_flows_mw_invert"}
                    ],
                    "operation": "select","param": {"trueCase": 0},
                    "initialInput": 0
                },
                {
                    "id": "gen_aux_mvar",
                    "inputs": [
                        {"uri": "/components/internal_magnolia","id": "discharging_flag"},
                        {"uri": "/components/internal_magnolia","id": "gen_line_flows_mvar_invert"}
                    ],
                    "operation": "select","param": {"trueCase": 0},
                    "initialInput": 0
                },
                {
                    "id": "discharging_flag",
                    "inputs": [
                        {"uri": "/sites/magnolia","id": "active_power"}
                    ],
                    "operation": "compareor","param": {"reference": 0,"operation": "gt"},
                    "initialInput": 0
                },
                {
                    "id": "gen_line_flows_mw",
                    "inputs": [
                        {"uri": "/sites/magnolia","id": "active_power" }
                    ],
                    "operation": "product", "param": {"gain": 0.001},
                    "initialInput": 0
                },
                {
                    "id": "gen_line_flows_mvar",
                    "inputs": [
                        {"uri": "/sites/magnolia","id": "reactive_power"}
                    ],
                    "operation": "product","param": {"gain": 0.001},
                    "initialInput": 0
                },
                {
                    "id": "gen_line_flows_mw_invert",
                    "inputs": [
                        {"uri": "/sites/magnolia","id": "active_power"}
                    ],
                    "operation": "product","param": {"gain": -0.001},
                    "initialInput": 0
                },
                {
                    "id": "gen_line_flows_mvar_invert",
                    "inputs": [
                        {"uri": "/sites/magnolia","id": "reactive_power"}
                    ],
                    "operation": "product","param": {"gain": -0.001},
                    "initialInput": 0
                }
            ]
        }
    ]
}
```

### Metrics Psuedo code


Two config options
* real code Note that this is the current favorite...

```

An Enum example

if (/sites/magnolia:active_power > 0) then {
    /components/magnolia:gen_net_mw   = /sites/magnolia:active_power * 0.001;
    /components/magnolia:gen_net_mvar = /sites/magnolia:reactive_power * 0.001;
    /components/magnolia:gen_aux_mw = 0;
    /components/magnolia:gen_aux_mvar = 0;
} else {
    /components/magnolia:gen_net_mw = 0;
    /components/magnolia:gen_net_mvar = 0;
    /components/magnolia:gen_aux_mw   = /sites/magnolia:active_power *-0.001;
    /components/magnolia:gen_aux_mvar = /sites/magnolia:reactive_power *-0.001;
}

A Select example

switch (/sites/magnolia:active_power) {
    case > 0:
       /components/magnolia:state = "Charging";
       ;;
    case < 0:
       /components/magnolia:state = "Discharging";
       ;;
    default:
       /components/magnolia:state = "Standby";
       ;;
}

An Echo example
[
    /components/magnolia:gen_gross_mv = /components/magnolia:gen_net_mv
    /components/magnolia:gen_gross_mvar = /components/magnolia:gen_net_mvar
]

A Product example
{
    /components/magnolia:max_operating_soc = 2.33 * /sites/magnolia:available_ess_num
}

A Sum example
{
   /components/magnolia:gen_basepoint_deviation = 
                         /components/magnolia:gen_net_mw - /components/magnolia:gen_updated_basepoint
  
}
Note **** what about disabled system
An Average example, (also max, min, mean, sum )
{
    /components/magnolia:avg_voltage_v = avg[
                /components/magnolia:grid_voltage_l1_l2 * -1, 
                /components/magnolia:grid_voltage_l2_l3, 
                /components/magnolia:grid_voltage_l3_l1
                ] 

}

Yes We'll do something for zulu and millisecondsToRFC3339.


```

* ess_style for reference

```
num_variables: 6
variable0:/sites/magnolia:active_power
variable1:/sites/magnolia:reactive_power
variable2:/components/magnolia:gen_net_mw
variable3:/components/magnolia:gen_net_mvar 
variable4:/components/magnolia:gen_aux_mw
variable5:/components/magnolia:gen_aux_mvar

if ({0} > 0 ) then ( 
    {2} = {0} * 0.001; 
    {3} = {1} * 0.001; 
    {4} = {0} * -0.001; 
    {5} = {1} * -0.001   
    ) else ( 
    {2} = 0.0; 
    {3} = 0.0; 
    {4} = 0.0;
    {5} = 0.0
    )
```
or      
```
    if (vars[0] > 0 ) then ( 
    vars[2] = vars[0] * 0.001; 
    vars[3] = vars[1] * 0.001; 
    vars[4] = vars[0] * -0.001; 
    vars[5] = vars[1] * -0.001   
    ) else ( 
    vars[2] = 0.0; 
    vars[3] = 0.0; 
    vars[4] = 0.0;
    vars[5] = 0.0
    )  
```



Looks like its all here
https://www.npmjs.com/package/ts-expression-evaluator



Break down a logical block
```
this is just an outline
const context = {
  vnames:[
    "/sites/magnolia:active_power",
    "/sites/magnolia:reactive_power",
    "/components/magnolia:gen_net_mw",
    "/components/magnolia:gen_net_mvar",
    "/components/magnolia:gen_aux_mw",
    "/components/magnolia:gen_aux_mvar"
 ],
 // note use getVars to translate vnames into vars objects
 // this is done once 
 // after then it all runs from "compiled" objects 

 vars :[],

 expr: " 
    if (/sites/magnolia:active_power > 0 ) then ( 
       /components/magnolia:gen_net_mw = /sites/magnolia:active_power * 0.001; 
       /components/magnolia:gen_net_mvar = /sites/magnolia:reactive_power * 0.001; 
       /components/magnolia:gen_aux_mw = 0.0;
       /components/magnolia:gen_aux_mvar = 0.0;
    ) else ( 
       /components/magnolia:gen_net_mw = 0.0; 
       /components/magnolia:gen_net_mvar = 0.0; 
       /components/magnolia:gen_aux_mw = /sites/magnolia:active_power * -0.001; 
       /components/magnolia:gen_aux_mvar = /sites/magnolia:reactive_power * -0.001;   
    )  
    "
};
```


### Blocks

* Blocks are organized in UriSets.
* Three URISet's are considered ,`PublishUris`, `InputUris`, `MonitorUris`
    * `PublishUris` are run periodically at a preconfigured interval. That can be enabled / disbled via fims messages.
    * `InputUris` is a list of Metrics Blocks "attached" to an input URI ( ess_controller actions). These are executed , if enabled, on input changes.
    * `MonitorUris` are lists of Metrics Blocks attached to arbrtrary URIs that are executed in a periodic manner to agggregate and monitor data.
       These operations will produce events and alarms. Effectively just like publishuris but ith no publish output.

* Each URIset  will have a ManagerThread (maybe).
* The ManagerThread will take care of extracting variables from the system map.
* One cycle will process all the blocks associated with an input or an output.
* outputs or fims "sets" can happen immediately not just at publish time
* Blocks can contain references to "internal" uris that , in turn, can have their own chain of blocks.
* Any block will only be executed once unless one of its input values have changed 
* Runaway detection will be in place
* Blocks can be disabled.
* bool, ints double, strings time/date all handled.
* Block status and results available via Fims.
* multi threadded block processing , use read/write locks on Uri handles
* save block state to disk like the current system (mdo).
* "built in" performance measurements and monitoring

### Variables

A URI has a component name and then one or more variables each identified by an ID.
A Variable is a map  of Parameters. One of which is a Value Object.
When setting a value of a component id the value is acually stored in a "value" object.
The Variable Map can contain named Parameters, Metrics/Alarm/Fault arrays.
The abstract go interface object is used here to allow a wide variety of data types to be used as the mappe component.
When one of these objects is included in a "pub" or "set"  operation the "value" object is formatted as either naked or clothed.
```
single naked
/components/comp1/id0 1234

multi naked
/components/comp1 : {
     "id0": 1234,
     "id1": 12345
     }

single clothed
/components/comp1/id0 {"value":1234}

multi clothed
/components/comp1 : {
     "id0": {"value":1234},
     "id1": {"value":12345}
     }
```

If a variable has additional parameters these can also be included in the output.

```
/components/comp1/id0 {
    "value":1234,
    "units":"DegC",
    "scaler": 10000
}

```

The format:full option is used to select the extra display options.
The "fmt" and "unmarshal"  go packages will have to be extended to handle the formatting options.

In the case of a `VLINK` either the "value" or , in the case of a `LINK` the whole value + param map will be pointers to the `linked` object .

```
object parameters
where PARAM interface{} types are string,float64,bool

URI                     ID                     PARAM
map[string]interface{}->map[string]interface{}->map[string]interface{}->string
map[string]interface{}->map[string]interface{}->map[string]interface{}->float64
map[string]interface{}->map[string]interface{}->map[string]interface{}->bool
                                                 

where PARAM interface{} types are *interface{}

gives us `VLINK'd` "value"
URI                     ID                     PARAM
map[string]interface{}->map[string]interface{}->map[string]interface{}->*interface{}


where IDinterface{} types are *interface{}

gives us `LINK'd` "value"
URI                     ID                      LINKED ID ( different name same Variable)
map[string]interface{}->map[string]interface{}->*interface{}

```

NOPE System params are saved in a special "_sys" PARAM which is used to hold timings and other system information.

The URI list is shadowed by a URI/ID manangement structure.
This contains the URI locks A map of IDs and performance data.
Metrics Alarms Faults Options are _meta data items contained in the shadow data objects. 
(Some special param names are reserved as arrays for metrics, alarms, faults and options. More reserved names may be needed.)


## Metrics Lists

The original metrics project collected URI/ID pairs together in `PublishUri` lists.
These items were run ( as a Pull process) when it came time to collect the data to be presented in a pub.
Metrics could be associated with an individual publish item. Once data has been collected the URI/ID items were used to generate a Publish message.

The echo 2 metrics has the option of multiple publish lists / times , normally High Speed and Low Speed publish items are required.

Note the echo 2 has the conscept os a sparse publish wher only changed data items are included in a publish operation.

Two other lists are available in the echo 2 project.

`InputUris` are  lists of Metrics that are executed when ever an incoming data item is received ( and , optionally,  its value changed). This is the `Push` process. These inputs can be considered as control or action URI's 

`MonitorUris` are lists of Metrics that are executed on a periodic basis. They do not result in a publish operation but are intended to track for alarms and faults and status changes.

   

### Performance testing 

Test using these config files [https://github.com/flexgen-power/config_twins/tree/TX100]


```
Start Twins 
    twins config_twins/twins/
```

This sends publishes to echo

``` 
run echo
    echo config_twins/echo_all.json
```

### Meeting 12_14_2022 notes

* create the metrics->echo config convertor
* continue to test echo speed , as is.
* add json decoder and retest speed.
* look at publish time.



## time monitor example 


```
package main


// tree /root/go/src/customjson/
// /root/go/src/customjson/
// `-- customjson.go

// we are using about 1000 data points
// cat simple.json | jq | wc -l
// 1074

import (
	"customjson"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"strings"
	"time"
)

// type OrderedMap struct {
// 	keys    []string
// 	JsonMap map[string]interface{}
// }\

func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
	log.Printf("%s took %s", name, elapsed)
}

func runUnmarshal(data []byte) (unmarshaled_data interface{}, err error) {
	defer timeTrack(time.Now(), "unmarshal")
	return customjson.Unmarshal(data)
}
func runMarshal(v interface{}) (output []byte) {
	defer timeTrack(time.Now(), "marshal")
	return customjson.Marshal(v)
}

func runjUnmarshal(data []byte, fx interface{}) (err error) {
	var f interface{}
	defer timeTrack(time.Now(), "json unmarshal")
	return json.Unmarshal(data, f)
}

func runjMarshal(v interface{}) (output []byte, err error) {
	defer timeTrack(time.Now(), "json marshal")
	return json.Marshal(v)
}
func myGet(f interface{}, path string) {
	path_arr := strings.Split(path, ".")
	for ix, key := range path_arr {
		fmt.Printf("key from path : %s  ", key)
		if ix == 0 {
			//arrInterface1, ok1 :=
			//_, ok1 := f.(map[string]interface{})[key].([]interface{})
			_, ok1 := f.(map[string]interface{})[key]
			fmt.Println(ok1)
		}
	}
}

func main() {
	var oMap customjson.OrderedMap
	file_byte, err := os.ReadFile("simple.json")
	if err != nil {
		fmt.Printf("Error reading in json file: %s", err)
	}
	//var oMap2 OrderedMap
	// file_byte2, err := os.ReadFile("simple2.json")
	// if err != nil {
	// 	fmt.Printf("Error reading in json file: %s", err)
	// }
	unmarshaled_data, _ := runUnmarshal(file_byte)
	oMap = unmarshaled_data.(customjson.OrderedMap)
	//oMap_out, _ := customjson.AddKeyValAfter(oMap, "field4.objectfield2", "key2_5", "5")
	//byte_out :=
	runMarshal(oMap)
	var f interface{}
	//err :=
	runjUnmarshal(file_byte, f)
	json.Unmarshal(file_byte, &f)
	runjMarshal(f)

	m := f.(map[string]interface{})
	for k, v := range m {
		switch vv := v.(type) {
		case string:
			fmt.Println(k, "is string", vv)
		case float64:
			fmt.Println(k, "is float64", vv)
		case []interface{}:
			fmt.Println(k, "is an array:")
			for i, u := range vv {
				fmt.Println(i, u)
			}
		case map[string]interface{}:
			fmt.Println(k, "is a map:")
		default:
			fmt.Println(k, "is of a type I don't know how to handle")
		}

	}
	myGet(f, "/components/bms_3a")
	myGet(f, "/components/bms_3ax")

	//fmt.Println(string(customjson.FormatJson(byte_out)))
}
```

## results
```
go build cj.go
sh-4.2# ./cj
2022/12/16 12:51:55 custom unmarshal took 38.474ms << Nah too slow>>
2022/12/16 12:51:55 marshal took 1.1059ms          << not too bad>>
2022/12/16 12:51:55 json unmarshal took 229µs      << this is good news >>
2022/12/16 12:51:55 json marshal took 958.8µs      << no problem this will be threaded>>

/components/tx_4 is a map:
/components/twins_sel_651r is a map:
/components/twins_m_bess_aux_acuvim is a map:
/components/bms_3a is a map:
/components/grid is a map:
/components/twins_sel_3530_rtac is a map:
/components/twins_apc_ups is a map:

// we can find things
key from path : /components/bms_3a  true
key from path : /components/bms_3ax  false

```

### More Design Notes
### Global Vars Map 

We may have one of these, worked well in other projects.
Easier to start with this than to have to create one later.
No Need to lock any maps I think the Update ( copy data from one map to the other)
Process will be super fast.


```
Here is the basic json object 
JsonMap map[string]interface{}
```

# we ay need some of this.

```
xx =find (vmap,uri) 
if !xx {
  if add == true{
  lock
   xx =add(vmap, uri)
   unlock
  }
}
return xx
```

### Locks
## the base uri tables can be locked 
This allows multiple fims threads to run and access the same uri space.
the go rwlocks are used

```
import (
. "sync"
"sync/atomic"
}

func reader(rwm *RWMutex, num_iterations int, activity *int32, cdone chan bool) {
	for i := 0; i < num_iterations; i++ {
		rwm.RLock()
		n := atomic.AddInt32(activity, 1)
		if n < 1 || n >= 10000 {
			rwm.RUnlock()
			panic(fmt.Sprintf("wlock(%d)\n", n))
		}
		for i := 0; i < 100; i++ {
		}
		atomic.AddInt32(activity, -1)
		rwm.RUnlock()
	}
	cdone <- true
}
func writer(rwm *RWMutex, num_iterations int, activity *int32, cdone chan bool) {
	for i := 0; i < num_iterations; i++ {
		rwm.Lock()
		n := atomic.AddInt32(activity, 10000)
		if n != 10000 {
			rwm.Unlock()
			panic(fmt.Sprintf("wlock(%d)\n", n))
		}
		for i := 0; i < 100; i++ {
		}
		atomic.AddInt32(activity, -10000)
		rwm.Unlock()
	}
	cdone <- true
}
```

simply locking a uri will create the lock structure if needed 

```use defer to auto unlock
UriReadLock (uri)
UriReadUnlock (uri)
UriReadTryLock (uri)
UriWriteLock (uri)
UriWriteUnlock (uri)
UriWriteTryLock (uri)
```



### GetUri
## concepts : mergejson has flas
 The mergejson function accepts an input interface{} and attempts to merge it into the vmap.
 It has a number of flags to control the merge.
 * extend  if the object is missing in the vmap, extend the vmap to include the object.
 * metrics if the object has a metrics component  then run those operations
 * format options
    * full
    * naked
    * clothed
    * ui
* uri the uri follows no more flag extraction

 /<baseuri>/extend/metrics/uri....


Function to split a URI up into uri and var 

Uses vmap to search for the split if the orig uri is not found
```
var = nullptr
xx = find(vmap,uri)
if ! xx {
    uri,var = split(uri)
    xx = find(vmap,uri)
}
return uri,var
```
## Profile Tool
* uses defer to collect performance and use data about a function + user.
* go profile useful but we need more
* collects max,min,avg, total time 
* allows reset
* examined by gets on uri/_perf
* We could produce a pprof file for the go tools.
```

func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
    //need to create the complete record in the base vmap
	log.Printf("%s took %s", name, elapsed)
}

func runUnmarshal(data []byte) (unmarshaled_data interface{}, err error) {
	defer timeTrack(time.Now(), "unmarshal")
	return customjson.Unmarshal(data)
}
```

## Fims input thread

# Current design
* Free running 
* First cut will accept all incoming data for a given URI and then pass through a filter of "required" data items.
* Second Cut will merge the unmarshall and filtering operation
* Needs profile tool  max, min, avg , total times  plus reset
* outputs time ordered change lists for each uri.
  * note the builtin json unmarshal maintains a state of the document so far and then adds components to that document if they are missing.
    I would suggest a refactor of that process to accept a fixed, current state and not process additions. Also kick out a change list of values that have changed.

# Data Input Formats

* needs to accept clothed and naked formats, single and multi vars possibly with and without params 
    * If the var is combined with the uri and simply has a value , it is single and naked.
    ```
    { "/components/bms_1/max_current": 3456 }
    ```
    * If the var is not combined with the uri and simply has a value , it is multi and naked.
    ```
    { "/components/bms_1":{"max_current": 3456 }}
    ```
    * If the var is combined with the uri and has a value object , it is single and clothed.
    ```
    { "/components/bms_1/max_current": {"value":3456 }}
    ```
    * If the var is not combined with the uri and  has a value object, it is multi and clothed.
    ```
    { "/components/bms_1":{"max_current":{"value": 3456 }}}
    ```

    * need to watch out for ui exceptions to these rules

Input processing of these different formats runs as follows

    Base objects are items associated with the uri that can have attributes or params including the one called "value"

* single and naked input 
    * give the uri an object , if needed, place the naked value into a "value" component of that base object.
     { "/components/bms_1/max_current": 3456 } ==>  {"/components/bms_1/max_current":{"value":3456}}
    * note that ess has this one too
     { "/components/bms_1/max_current@limit": 3456 } ==>  {"/components/bms_1/max_current":{"limit":3456}}


* single and clothed input with the word "value"
    * decode as is
        {"/components/bms_1/max_current":{"value":3456}}

* single and clothed input without the  word "value"
    * look for the value object in the base object , if found  then add the new item into the base object (ie add a param)
    {"/components/bms_1":{"max_current": 3456 }} ==>    {"/components/bms_1":{"max_current":{"value":3456}}}
    * maybe the ess will do this thanks to getUri
    {"/components/bms_1":{"max_current@limit": 5555 }} ==>    {"/components/bms_1":{"max_current":{"limit":5555}}}


* multi and naked input ( items are assumed to be base object)
    * give the uri an object , if needed, place the naked value into a "value" component of that object.
    {"/components/bms_1":{"max_current":{"value":3456}}} ==> {"/components/bms_1":{"max_current":{"value":3456}}}

    {"/components/bms_1":{"max_current":{"limit":5555}}} ==> {"/components/bms_1":{"max_current":{"limit":5555}}}

## Data Output Formats
* Sparse data output.

    In the same way that the system produces sparse input data, the system can withhold pubs of data that has not changed.
    Each data item or the whole pub uri can have an update time, or a time interval after which  the data is published regardless of its changed state. This feature is set to  to turn it off.
    This is to allow a process to sync up with the data.
    A fims sync command can be sent to the puburi object to force an output data sync.

    The rest of the time the data item is only inculded in the pub if it has changed.

* "naked"
    {"/components/bms_1":{"max_current":{"value":3456}}} ==> {"/components/bms_1":{"max_current":3456}}
    {"/components/bms_1":{"max_current":{"value":3456}}} ==> {"/components/bms_1/max_current":3456} ??

* "clothed" as is but no params
    {"/components/bms_1":{"max_current":{"value":3456}}} ==> {"/components/bms_1":{"max_current":{"value":3456}}}
    {"/components/bms_1":{"max_current":{"value":3456}}} ==> {"/components/bms_1/max_current":{"value":3456}}  ?? do  ever need / get this 

* "full" as is with params
    {"/components/bms_1":{"max_current":{"value":3456}}} ==> {"/components/bms_1":{"max_current":{"value":3456,"limit":5555}}}
* ui
   TODO
   See also 
   [https://flexgen.atlassian.net/wiki/spaces/API/pages/108724229/PowerCloud+FIMS+Format]



# Older concepts
* loads to vals and vars, this is read only by others , write only by thread
* processes gets (maybe)
* two vars lists one current , one for free use by others , they can manage thier own locks if needed
* accepts commands to sync or switch responds with a done message .. no other locks
* auto populates  nope use filter
* uses Stephanie's Merging Json Parser  nope too slow)
* accepts commands to block  unblock uri components  (nope uss filter now)
* has a subscribe list from config
* has its own fims connection can share with the pub threads

## Fims output thread(s)

* Free running produce an output list for a given uri on a predefined periodic basis.
* Fims control messages can enable or disable output blocks as required. 
    The frequency of the output can be changed. The output format can also be changed.

* Individual id items are evaluated using a series of operation blocks
* Multiple output blocks can be cascaded to produce the required output.
* Outputs from one block can be used as inputs to a following block.
* The blocks define a priority order created during setup with block 
    execution to be performed in priority order.
    If a one blocks output is used in as an input to a different block in the same
    uri calcualtion then that block willbe giver a high priority to ensure that it gets executed before the lower priority block ensureing that the results of its calculations are available for the lower order  block.

* These blocks can accept inputs from any incoming data source and 
    produce output data associated with internal, input or output uri's

* An output block examines the change lists for its input uris.

* Any changes in any input data items used in the evaluation of that block 
    will trigger a re evaluation of that block.
    The time of the re evaluation is moved to match the time of the change list entry.
    This requires a global change list ordered by time and uri.
* Any changes in data uri / values pairs produced by the operation block 
    will produce a new change item to be added to the global change list using the time reference of the triggered change.  

* Needs profile tool  max, min, avg , total  plus reset

## Input/output, in fact any var
* latest design 
    * note there is no vmap now we do have maps based on uris that contain values and timed change lists
    * each uri map will be defined by the output processing blocks 
    * only vars defined in that inital list will be accepted and parsed from the fims input.
    * special fims diagnostic "/echo_xxx/inputs/<uri>" access will be given to inspect and modify (add only) the uri list
    * Internal and Output lists ... perhaps

* older concepts
    * sits in the vmap defined by uri ( we may have input / output/ process vmaps with links to vals or vars)
    * each var has a list of set and get blocks referenced by pointers
    * more later

## Blocks
* overview
    * contains a block processing strucure
    * has a name/id
    * referenced in a "uri" list /echo_xxx/operations/<uri>/<var>/<operationid>
    * each "block" will have operator (method)
    * will have list of inputs, outputs, internal
    * eval or other params as needed
    * will have profile data
    * has execution time ( see later) 
    * will have enable / disable flag

* block priority placement
    * each block will have inputs and outputs defined
    * once all blocks are defined the system will define block order
    * any block whose outputs are used in another blocks inputs will be executed before that other block
       So start with a priority of "1"
       Any blocks whose outputs match my inputs will be given a priotity of mine +1
       move on to the next block
       at the end the highest priority block will be identified.


## Block Processor
* definitions stored in the vmap
* attached to an output uri/id
* has a block array
* sets start time
* executes all blocks BEFORE start time
* special blocks will snapshot input data from input buffers
* writes to any vars in the process space. ( may/will  need to discuss snap method)
* blocks can have inputs that have their own blocks so we recurse down the tree untilwe find an input with no blocks.
* have to protect against circular blocks ( I think the execution time will capture this)

## Config Processor
Current test protptype configtest.go

The config Processor accepts any predefined config file format:
  * Original metrics file
  * First Generation Echo Files
  * dnp3_interface 
  * modbus_client
  * Second Generations Echo Files

The config Processor produces:
  * a list of publish uris with their attendant data blocks.
  * a list of subscriptions
  * a list of input data items ordered by uri.
  * a series of compute blocks each with their own list of inputs and outputs.

## Basic Fims Input thread code
# note you will need /root/go/src/fims set up

```
tree /root/go/src/fims/
/root/go/src/fims/
|-- go_fims.go
|-- msg.go
`-- verification.go

we'll add the json marshal / unmarshal  to this

```
package main

import (
	"fims"
	"fmt"
	"log"
	"time"
)

var f fims.Fims
var fimsReceive chan fims.FimsMsg
var uris []string
var outputTicker *time.Ticker
var pubTicker = make(chan string)
var pubs = 0
var sets = 0
var gets = 0

func main() {

	fmt.Println("Echo test")
	setupFims()
	go publishRate("/hello/pub/test", 10, pubTicker) //Multiple Publish rate function

	for i := 0; i < 10000; i++ { //testing profiling
		select {
		case msg := <-fimsReceive:
			if msg.Method == "set" {
				sets++
				// read body into temp vmap
				// merge into be base vmap
				// need to use geturi to pick out the base and the source
				// Separating out the Uri for map traversal
				// base := strings.Replace(msg.Uri, "/"+msg.Frags[len(msg.Frags)-1], "", 1)
				//source := msg.Frags[len(msg.Frags)-1]
			} else if msg.Method == "pub" { // publish from an input device, most common use case
				pubs++
				// read body into temp vmap
				// merge into be base vmap
				//addToInput(msg)
			} else if msg.Method == "get" && msg.Replyto != "" {
				gets++
				// need to use geturi to pick out the base and the source for the get
				// unmarshall and send
				f.Send(fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   "Hello Kitty",
				})
			}
		case uri := <-pubTicker: //Channel for multi publish rates
			if !f.Connected() {
				// nope we need to try and pick up the connection
				s := fmt.Sprintf("No Fims for Pub # %s", uri)
				log.Fatal(s)
			}
			// note this may be their own threads
			// find pub object from the uri
			// s := fmt.Sprintf("Hello Kitty, Pub # %d", i)
			// f.Send(fims.FimsMsg{
			// 	Method: "pub",
			// 	Uri:    uri,
			// 	Body:   s,
			// 	})
		}
	}
}

// Connect and subscribe using fims
func setupFims() {
	var err error

	// subscribe to current process PID for config updates
	// pid := os.Getpid()
	// uris = append(uris, strconv.Itoa(pid))

	// subscribe to all inputs and outputs
	//for _, output := range cfg.Outputs {
	//	for _, input := range output.Inputs {
	//		if !findString(input.Uri, uris) {
	//			uris = append(uris, input.Uri)
	//		}
	//	}
	//	if !findString(output.Uri, uris) {
	//		uris = append(uris, output.Uri)
	//	}
	//}
	uris = append(uris, "/test/me")
	// connect to fims
	f, err = fims.Connect("echo_main")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	// subscribe on fims with slice of uris using variadic args
	err = f.Subscribe(uris...)
	if err != nil {
		log.Fatal("Unable to subscribe")
	}
	fmt.Print("subscribed to :")
	fmt.Println(uris)

	// launch go routine for receive channel
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)
}
```

func publishRate(uri string, rate float64, co chan<- string) {
	//var elapsed = 0
	var count = 0
	for {
		var tsleep = (time.Duration(rate) * time.Millisecond) //- elapsed
		time.Sleep(tsleep)
		// we may pull the data from the base vmap
		// process all the blocks
		// push data back into the base vmap
		// unmarshall and send
		s := fmt.Sprintf("Hello Kitty, Pub # %d", count)
		f.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    uri,
			Body:   s,
		})
		count++
		co <- uri
	}
}
```


### Var Storage

# we have to solve the naked / clothed / full var structure
* vars may need params
* need to recover parent ( use string)
* need to accept any input format but be consistent in the internal format
* so expand value on the input
* allow clothed/naked/full get and publish options


```
input var 
{
"/components/bms_3a/active_power": 2345
"/components/bms_3a" {"active_power": 2345}
"/components/bms_3a/active_power": '{"value":2345}'
"/components/bms_3a" {"active_power": {"value":2345}}

}

internal var
{
"/components/bms_3a" {"active_power": {"value":2345}}
}

pub/get options
{
naked single :- "/components/bms_3a/active_power": 2345
naked multi :- "/components/bms_3a" {"active_power": 2345}
clothed single:- "/components/bms_3a/active_power": '{"value":2345}'
clothed multi: "/components/bms_3a" {"active_power": {"value":2345}}
full single :- "/components/bms_3a/active_power": '{"value":2345, "parent":"/components/bms_3a"}'
full multi :- "/components/bms_3a":{"active_power": '{"value":2345, "parent":"/components/bms_3a"}}'
}
```

## TODO time the merge operation.
* done 12_19_2022  abount the same as the initial parse

## Change in plan .... no need to merge yet  but yes we do plus filter

New layout concepts ...
input parsing ... 1000 data points in 200 microseconds.
5,000,000  per second per thread.
Central Fims receiver thread will then drop fims messages to decode threads decode the uri and pick a thread assigned to that uri.
We need to filter the incoming messages to only accept designated variables and produce change lists based on time of message.

The config parser produces:
  * a list of publish uris with their attendant data blocks.
  * a list of subscriptions
  * a list of input data items ordered by uri.
  * a series of compute blocks each with their own list of inputs and outputs.

The input parser produces a direct map of the incoming object  and then passes through a filter where only selected items are monitored.
The parser plus filter then produces a change list of data items. This is passed to a list of change lists ordered by time for each input uri.


The thread allocation can be dynamic so maybe we simply have a thread pool and a master uri directory
When the publish operation wants to get data from a URI It will have to wait until that uri is unlocked.

worst case is a  100 system aggregation in which each one was locked so if they are serviced in series 20 millisecs to grab all the data.
Unless we queue resuests for each uri.
But they wont all be locked so the input method will either queue the request or simply pend a request for all URIs and then poll to pull them all in.

Lets say I have 100 uri's
As a thread services a URI it will take a lock, service and release 
Chances are only 10 will be locked at any time
publish data extract from each one wll be say 50uSecs per uri 2 usecs per point
We have a list of uris to get 
for X = 1 to 100 

have we got all URIs (list empty) -> yes -> done
pop URI[X] -> is URI[X] locked -> no lock , pull, unlock
                               -> no need leave on list
                
This will only have to cycle a few times to pull all the data.




### Notes on fims ui data

 * A UI control value. A UI control value is a JSON object. It must either have no member named `"value"`, or it must have a member named `"ui_type"` whose value is the string `"control"`.

        ```json
        {
            "ui_control_example_1": true
        },
        {
            "ui_control_example_2": true,
            "value": 1,
            "ui_type": "control"
        },
        {
            "clothed_example_1": true,
            "value": 1,
            "ui_type": "fault"
        }
        ```
### Meeting 12_19_2022 notes

# base state
* Threaded design progressing
* grule rule engine
    [https://github.com/hyperjumptech/grule-rule-engine/blob/master/docs/en/Tutorial_en.md]
* antlr 
    [http://lab.antlr.org/]
* basic metrics->echo->echo2  config convertor ... in progress
* speed tests done 
* json merging decoder first cut.
* 

# notes
* keep thread time order in transfer from fims to vars
    This means that high speed high priority threads will have to be routed through a single queue.
    To maintain time order.
    Consider a double buffer where we extract a list of changes .
    Possibly consider a combined unmarshal and populate into  temp buffer.
    We can make do with the umarshall function as is and the merge into  temo buffer (about 500uSecs) and then extract a list of changes on the way and only transfer those into locked memory.
    May be something else we can do with this as well.
    We can expect 150 high speed threads ( 100 data items not 1000 as in our standard test.) Still take about 5mS to process into a copy of the main buffer.
    But , depending on the change list, we could just pass that round.
    Have to run some tests on simulated data and site recorded data from sce I hope.


* Justin's concepts look good.
* template expansion
* list of inputs / outputs with short name per block
* recursive block evaluation , time stamp to stop blocks repeating.
* wild card [ess_*] within the scope of a block 
* debug with names expanded.

# questions 
* diff expander ( collect list of changed points)
* (do we want to use one) rule engine.
* save state ( could be a special output block)
* immediate sets ( not discussed but yes)  
* maintain obscure rules. yes
* expanded debug. ( names and values)



## research work 12_19_2022
* continue with the metrics rule expander.
    * for a publish uri create an echo list of inputs and outputs.
* think about the uri buffers.
