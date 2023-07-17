DNP3_enable_state_events

P. Wilshire  03_21_2023


# State Change Event messages

## Overview

The OpenDNP3 System issues a message whenever a channel state change is detected.

These states relate to the data link to the system transitioning from Open (communicating) to Closed ( communications lost).
On statup the state will transition to Opening while the data link is waiting for an incoming connection.

These states are 


## Why allow it to be turned off ?

When working in a multi connection ( Fail Safe) mode , one Dnp3 Server may be talking to two Dnp3 Clients.
OpenDnp3 cannot do this by design , so when a second connection attempt is made from a different Client to the Server. 
The Server will drop the original connection and open a new connection to the new Client.

During this process the system will, normally, issue system events and log messages.






## Supress the events /messages

The "enable_state_events" system config component is used to turn off the event logging and error messages. 
If set false, the events and log messages are are not generated and the system silently switches from one connection to the other.
If the restart timeout is set to a value greater than the polling rate , it would seem that the two Clients can talk to the Server and receive  messages until the second Client attempt to connect to the Server.



```
"system": {
        "name": "FlexGen HybridOS",
        "protocol": "DNP3",
        "version": "0.2",
        "debug": 0,
        "id": "lopeno_dnp3_server",
        "ip_address": "172.17.0.3",
        "format": "naked",
        "port": 20000,
        "local_uri": "/local_server",
        "batchSets": "batchSets0",
        "batchPubs": "batchPubs0",
        "frequency": 5000,
        "enable_state_events": false
    },
```


## DNP3 Server Events/Log output 

```
        "enable_state_events": true (default)
```

These are the published events

```

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"source":"DNP3_server:lopeno_dnp3_server","message":"state change [OPENING]\n","severity":1}
Timestamp:    2023-03-22 13:48:14.797350

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"source":"DNP3_server:lopeno_dnp3_server","message":"state change [OPEN]\n","severity":1}
Timestamp:    2023-03-22 13:48:27.891108

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"source":"DNP3_server:lopeno_dnp3_server","message":"state change [CLOSED]\n","severity":1}
Timestamp:    2023-03-22 13:49:45.332801

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"source":"DNP3_server:lopeno_dnp3_server","message":"state change [SHUTDOWN]\n","severity":1}
Timestamp:    2023-03-22 13:49:45.333313


```

Here are the server log file entries .

TODO add time/date to these.

```
fps channel state change: [OPEN]
fps channel state change: [CLOSED]
fps channel state change: [OPENING]
fps channel state change: [OPEN]
```

## Server enable_state_events set to false


```
        "enable_state_events": false (config option)
```

You get no output.



## DNP3 Client Events/Log output 

```
        "enable_state_events": true (default)
```

These are the published events

```

Client Events during Stop and restart dnp3_server

```
Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [CLOSED]\n","severity":1}
Timestamp:    2023-03-22 14:02:09.726519

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [OPENING]\n","severity":1}
Timestamp:    2023-03-22 14:02:09.729709

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [OPEN]\n","severity":1}
Timestamp:    2023-03-22 14:02:24.732016
```


Client log output during stop and restart server.


```
    fps channel state change: [CLOSED]
    State Change message [Link Status Change [UNRESET]

    fps channel state change: [OPENING]",
    fps channel state change: [OPEN]",
    State Change message [Link Status Change [UNRESET]

```


Client Events during Stop and restart dnp3_client

```
Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [CLOSED]\n","severity":1}
Timestamp:    2023-03-22 14:08:35.066649

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [SHUTDOWN]\n","severity":1}
Timestamp:    2023-03-22 14:08:35.068053

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [OPENING]\n","severity":1}
Timestamp:    2023-03-22 14:09:01.509291

Method:       post
Uri:          /events
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"source":"DNP3_client:lopeno","message":"state change [OPEN]\n","severity":1}
Timestamp:    2023-03-22 14:09:01.509918

```

Client Log on startup

```
    fps channel state change: [OPENING]
    fps channel state change: [OPEN]
    State Change message [Link Status Change [UNRESET]
    Running [OnTaskComplete] result [FAILURE_BAD_RESPONSE]  start[1679494141510] elapsed [0] 
    Running [OnTaskComplete] result [FAILURE_BAD_RESPONSE]  start[1679494141512] elapsed [0]
```


## Client enable_state_events set to false


```
        "enable_state_events": false (config option)
```

You get no output for link status events.



