## DNP3 Events
P Wilshire 02-09-2022

# Defining Data types and Flags

# Static Data and Flags

The v10.1 DNP3 interface now handles server events. 
This means that the quality data from the server to the client can be included in the data packet. 
There are two basic types of data handlers in the server. 
Static data contains the current state of the variable. That state contains a value, and optional flags.


The configuration options dictate the data included with the object.

As a reminder the "normal" static data options available are:-

Group1Var1   Binary Input - Without Flags
Group1Var2   Binary Input - With Flags

Group30Var1  Analog Input 32 bit with flags
Group30Var2  Analog Input 16 bit with flags
Group30Var3  Analog Input 32 bit without flags
Group30Var4  Analog Input 16 bit without flags
Group30Var5  Analog Input single precision with flags (Float)   
Group30Var6  Analog Input double precision with flags (Double)

There are other data items including counters that are NOT included in this release.

The flags indicate a number of possible states that the system can determine for a particular variable.
 
 ONLINE(0x1) 
 RESTART(0x2) 
 COMM_LOST(0x4) 
 REMOTE_FORCED(0x8) 
 LOCAL_FORCED(0x10) 
 CHATTER_FILTER(0x20) 
 RESERVED(0x40),  
 STATE (0x80)

 Flags combine their states into a bitfield.

 This bitfield is decoded as a list of options for example "STATE,ONLINE"


# Event Handling

A variable can be added to the event system by adding an "evariation" configuration option.


Event handling is triggereed by specifying an e-variation for either analog or binary  inputs.

    Group32Var1 Analog Input Event - 32-bit With Flag
    Group32Var3 Analog Input Event - 32-bit With Flag and Time
    
    Group2Var1 Binary Input Event - Without Time
    Group2Var2 Binary Input Event - With Absolute Time
    Group2Var3 Binary Input Event - With Relative Time


Here is a configuration example.

```
{
        "id": "active_power",
        "offset": 3,
        "variation": "Group30Var5",
        "evariation":"Group32Var3",
        "format":"clothed",
        "events" : true,
        "name": "Site - Active Power Total Net",
        "uri": "/components/sel_735"
}
```

Once an event has been flagged in the config, the event is generated whenever the input state changes.

If an input is not triggered for any period of time the data channel will enter a "comms lost" state.
This condition is created by the FlexGen  code from a timeout , in seconds, contained in the system config.

```
"system": {
        "name": "FlexGen HybridOS",
        "protocol": "DNP3",
        "version": "1.2",
        "debug": 0,
        "ip_address": "0.0.0.0",
        "local_uri": "/local_server",
        "id": "test_dnp3_server",
        "port": 20000,
        "frequency": 5000,
        "timeout": 10.2
    },
```
This value will be applied to all data items, however, individual data items can override the system timeout, a timeout value of 0 removes the timeout.
```
{
    "id": "racks",
    "offset": 37,
    "variation":"Group30Var5",
    "name": "BESS Container #2 Number of Active Racks",
    "uri": "/assets/ess/ess_2",
    "timeout":0
}
```

The timeout specified , in seconds, determines how frequently the value must be updated at the dnp3 server input.
The value need not change but the value must be updated before the timeout period or the "COMM_LOST" state will be asserted.

Analog values can have a deadband provided that will limit the event generation.
The current value must exceed the last event value before an event is generated.
```
 {
    "id": "ess_chargeable_power",
    "offset": 13,
    "variation":"Group30Var5",
    "name": "Battery - Chargeable power",
    "uri": "/assets/ess/summary",
    "deadband":10
}
```

# Event Configuration

On the client (master) side several options control the event delivery.

the "events":true|false config item is used to trigger a special event output message when the client receives the event from the server.
```
"system": {
        "name": "FlexGen HybridOS",
        "protocol": "DNP3",
        "version": "1.2",
        "debug": 0,
        "ip_address": "0.0.0.0",
        "local_uri": "/local_server",
        "id": "test_dnp3_server",
        "port": 20000,
        "frequency": 5000,
        "timeout": 10.2,
        "events": true
    },
```


If the events item is set to true in the system config then any incoming event is published as a clothed body on the output.


Method:    pub
Uri:       /components/sel_735_event
ReplyTo:   null
Body:      {"active_power":{"value":125,"flags":1,"sflags":"ONLINE","stime":"2022-02-10 02:06:10.654","etime":"2022-02-10 02:06:10.114"}}
Timestamp: 2022-02-10 02:06:10.675481

This output contains the following data
"flags"         : the raw flags data
"sflags"        : the decoded flags data
"stime"         : the local time when the last static data scan was received
"etime"         : either the time contained in the  dnp3 event data from the server
                : or the local time when the event was received



The "events" config entry  in the indidual data configuration will override any suystem level event configuration. 

```
 {
    "id": "ess_chargeable_power",
    "offset": 13,
    "variation":"Group30Var5",
    "name": "Battery - Chargeable power",
    "uri": "/assets/ess/summary",
    "deadband":10,
    "events" : true
}
```


Method:    pub
Uri:       /components/sel_735_event
ReplyTo:   null
Body:      {"active_power":{"value":125,"flags":1,"sflags":"ONLINE","stime":"2022-02-10 02:06:10.654","etime":"2022-02-10 02:06:10.114"}}
Timestamp: 2022-02-10 02:06:10.675481

In this output

  "value":    is the data item value
  "flags":    is the raw flag data received.
  "sflags":   is tehu decoded flag data
  "stime":    is the last tine the data item was recieved as part of a normal scheduled  scan.
  "etime":    is the last tine the data item was recieved as part of an event scan.
  "Timestamp": is the time the pub message was recieved by the fims listen tool.


The events data may also be presented in an individual data channel  by setting the format config flag to "clothed". 

Setting this flag for an indidual data item overrides the system level format configuration.


"format":"clothed"    selects the clothed  data format ( as in active_power in the example).
"format":"naked"      selects the naked  data format ( as in reactive_power in the example).


The format can be specified at the system level for the whole system and individual data items can override the "global" setting when needed.

Here is an example of a mixed data outout with "clothed" and "naked" data in the same message.

Method:    pub
Uri:       /components/sel_735
ReplyTo:   null
Body:      {"active_power":{"value":125,"flags":1,"sflags":"ONLINE","stime":"2022-02-10 02:06:10.654","etime":"2022-02-10 02:06:10.114"},"reactive_power":3400,"apparent_power":23000,"frequency":60.119998931884766,"Timestamp":"02-10-2022 02:06:10.675138"}
Timestamp: 2022-02-10 02:06:10.675481




# Event  Rate Limit.

Passing through the event data can introduce a high output data rate from the client.
The event rate limit will limit the number of event messages. The rate_limit specifies the minimum time between  event deliveries.

So given an event rate limit of 0.050 (50 mS) means that the system will wait 50 mS between successive event messages for any given data item.
Events will still be received by the system and event data collected in the local database but event message output will be limited by the rate limit.

A global event rate limit can be provided at the system level and overridden for an individual data item.
```
{
    "id": "active_power",
    "offset": 3,
    "variation":"Group30Var5",
    "evariation":"Group32Var3",
    "format":"clothed",
    "events" : true,
    "event_rate":0.005,
    "name": "Site - Active Power Total Net",
    "uri": "/components/sel_735"
}
```


# System Event Generation

The DNP3 system will post link state change events to the FlexGen Events system.

OPEN :      The link is open , dnp3 communications are flowing
CLOSED :    The link is closed. The restart system will attempt to restart.
OPENING :    The link is closed. The restart system will attempt to restart.
SHUTDOWN :   The link Channel has shutdown.


The following are examples of the event messages.

Method:    post
Uri:       /events
ReplyTo:   null
Body:      {"source":"OnStateChange","message":"DNP3  test_dnp3_client state change [OPEN]\n","severity":1}
Timestamp: 2022-02-10 20:52:35.602132

Method:    post
Uri:       /events
ReplyTo:   null
Body:      {"source":"OnStateChange","message":"DNP3  test_dnp3_client state change [CLOSED]\n","severity":1}
Timestamp: 2022-02-10 20:52:42.990842

Method:    post
Uri:       /events
ReplyTo:   null
Body:      {"source":"OnStateChange","message":"DNP3  test_dnp3_client state change [OPENING]\n","severity":1}
Timestamp: 2022-02-10 20:52:42.991010

Method:    post
Uri:       /events
ReplyTo:   null
Body:      {"source":"OnStateChange","message":"DNP3  test_dnp3_client state change [OPEN]\n","severity":1}
Timestamp: 2022-02-10 20:52:49.993025

