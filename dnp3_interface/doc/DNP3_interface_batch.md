DNP3_interface

P. Wilshire  02_22_2023
             02_27_2023  added batchPubs


# BatchSets


## Client -> server operation.

The "BatchSets" option has been introduced in the 3.0.9 and later versions of the dnp3_interface.
It operates in slightly different ways in the DNP3 client and server code.
This section defines the operation of the setting in the dnp3 server system.0  


The DNP3_Client communicates with the server using fims "set" messages.

These are in the form of single or multi variable messages in either "clothed" or "naked" format.

Here are some examples.

```

fims_send -m set -r/$$ -u /sites/lopeno/watchdog_pet  23456

```

This is a single naked message. It is also a bit of a nightmare.

why ----- ?
(see below)

```

fims_send -m set -r/$$ -u /sites/lopeno '{"watchdog_pet":  23456}'

```

This is another , easier to handle , single naked message but it follows a "better" structure in the form of a "topic"  /components/pcs and an item "active_power" .
The message for the topic is in the form of a json object.

```

fims_send -m set -r/$$ -u /sites/lopeno '{"watchdog_pet": {"value":23456}}'

```

This is a single clothed message.
The naked message assumes thst the component of the item to be addressed by the message is the "value".


When sent to a DNP3_client these messages will result in a conversation wth the server resulting in a single value being sent to the server.

The server reeived this message in a complex callback loop provided by openDNP3.
The "MVP" operation was to immediately repeat the "set" action as a fims_message coming from the server.

You can send multiple set requests to the dnp3_client with a "multi" fims message.

```

fims_send -m set -r/$$ -u /sites/lopeno '{
                      "watchdog_pet":  23456,
                      "fr_response_enable_mask":456,
                      "fr_pfr_up_active_cmd_mw": 7845
                      }'


```

Sending this to the DNP3_client will result in a single transmission to the dnp3_server with three components referenced ( assuming that all 3 are in the same output group).


Once the message gets to the dnp3_server the async OpenDNP3 system requires a custom handler to be created to plug in our fims / database hooks.

In the case of the multi "set" message sent to the server, three individual fims_set messages would be issued on the server.

```
No BatchSet option.

DNP3 Client Input

fims_send -m set -r/$$ -u /sites/lopeno '{
                      "watchdog_pet":  23456,
                      "fr_response_enable_mask":456,
                      "fr_pfr_up_active_cmd_mw": 7845
                      }'



DNP3 Server Output (default)
Method:       set
Uri:          /features/active_power/uf_pfr_active_cmd_kw_dnp3
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         7845000
Timestamp:    2023-02-27 19:06:12.499653

Method:       set
Uri:          /features/active_power/fr_enable_mask_dnp3
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         456
Timestamp:    2023-02-27 19:06:12.499737

Method:       set
Uri:          /features/site_operation/watchdog_pet
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         23456
Timestamp:    2023-02-27 19:06:12.499776



```

The translation of the messages from multi format to multiple single format messages can overload some of the downstream systems.




This system config item ( which defaults to 0/disabled ) defers the output of the "set" message from the server for a short , configurable, time period.
Once that time period has expired, all incoming "set" messages from the are issued as a series of multi set messages grouped by base uri.

Even if the "set" messages are received from different object groups from the client they are "batched" togeter based on the output uri defined in the configuration.



```
With BatchSet option set to a non zero value in the SERVER config.

DNP3 Client Input

fims_send -m set -r/$$ -u /sites/lopeno '{
                      "watchdog_pet":  23456,
                      "fr_response_enable_mask":456,
                      "fr_pfr_up_active_cmd_mw": 7845
                      }'


```

DNP3 Server Output (default) two messages one for each uri group.

```
Method:       set
Uri:          /features/site_operation
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"watchdog_pet":23456}
Timestamp:    2023-02-27 19:31:46.611796

Method:       set
Uri:          /features/active_power
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"fr_enable_mask_dnp3":456,"uf_pfr_active_cmd_kw_dnp3":7845000}
Timestamp:    2023-02-27 19:24:08.362881

```


With the config option set to 100 , groups of  output "sets" will be output in 100Ms intervals.

Note that this option can be set at run time with a "pub" into the dnp3_server.


```
fims_send -m pub -u /local_server/_system '{"batchSets": 200}'

```
This is assuming that /local_server  is the "local_uri" in the dnp3_server config..

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
        "batchSets": 200,
        "batchPubs": 10000,
        "frequency": 5000,
        "batchPubss": 1000
    }
```

## DNP3 Client side batch sets


The batchSets option can also be used in the  dnp3_client config.

In the DNP3_client as the system processes incoming sets they are sent to the server as soon as they are received by the client in the fims "set" message.
They are already "batched" in the sense that all messages in a single fims message are processed as sets to the server after each fims message has been received.
With a multiple of incoming set messages an overload condition can be reached wher the stream of clinet sets can cause the server to stop sending pubs.
In fact so many sets are scheduled on the client that the Clinet does not get a chance to issue the scan request to the server that causes the clinet stop receiving pub data from the server.
The effect of this is, on the clinet, when some service or process starst tat send out multiple set messages to the clinet then pubs stop coming out of the clinet. Stop sendinf the sets to the clinet and pubs return.

In this case , the Clinet will not issue sets to the Server as soon as the incoming fims_message is received. The clinet will wait for the batchSets interval before relaying the set operation to the server.
This does mean that only the last fims_send set message for any given uri will be serviced during the batch operation.

```
With BatchSet option set to a non zero value in the CLIENT config.

DNP3 Client Input


fims_send -m set -r/$$ -u /local_client/sites/lopeno '{
                      "watchdog_pet":  10000,
                      "fr_response_enable_mask":1456,
                      "fr_pfr_up_active_cmd_mw": 17845
                      }'
fims_send -m set -r/$$ -u /local_client/sites/lopeno '{
                      "watchdog_pet":  110000,
                      "fr_response_enable_mask":2456,
                      "fr_pfr_up_active_cmd_mw": 17845
                      }'
fims_send -m set -r/$$ -u /local_client/sites/lopeno '{
                      "watchdog_pet":  23456,
                      "fr_response_enable_mask":456,
                      "fr_pfr_up_active_cmd_mw": 7845
                      }'

DNP3 Server Output (default) batchSets 0

Method:       set
Uri:          /features/active_power/fr_enable_mask_dnp3
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         456
Timestamp:    2023-02-27 19:35:58.783401

Method:       set
Uri:          /features/active_power/uf_pfr_active_cmd_kw_dnp3
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         7845000
Timestamp:    2023-02-27 19:35:58.783423

Method:       set
Uri:          /features/site_operation
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"watchdog_pet":23456}
Timestamp:    2023-02-27 19:36:15.332504


```         

## override.

SO what happens if you want an emergency set to bypass the batchSets process ?

Individual data points can be configured as "nonBatched"

```
   "nonBatched": true,
```

These data points will bypass the whole batched operation and be dispatched and serviced as soon as they are sent to the Client , or as soon as the server recieves theDNP3 message from the client.



## Testing

## Client Testing

* Set batchSets to zero in both server and client configs.

```
"system": {
    "name": "FlexGen HybridOS",
    "protocol": "DNP3",
    "version": "0.2",
    "debug": 0,
    "id": "lopeno_dnp3_server",
    "ip_address": "172.17.0.5",
    "format": "naked",
    "port": 20000,
    "local_uri": "/local_server",
    "batchSets": 0,
    "frequency": 5000
},
```
* Create a multiple "set" message for the same variable  

```
#!/bin/sh
# multi set demo for dnp3_client_309.json and dnp3_server_309.json
fims_send -m set  -u /sites/lopeno/watchdog_pet 1
fims_send -m set  -u /sites/lopeno/watchdog_pet 2 
fims_send -m set  -u /sites/lopeno/watchdog_pet 3 
fims_send -m set  -u /sites/lopeno/watchdog_pet 4
```

With no batchSets configured, the "multi set" script will cause multiple sets to be generated within the DNP3 system as each set input is processed.

```
Method:       set
Uri:          /features/site_operation/watchdog_pet
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         1
Timestamp:    2023-02-23 09:08:30.84445

Method:       set
Uri:          /features/site_operation/watchdog_pet
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         2
Timestamp:    2023-02-23 09:08:30.86494

Method:       set
Uri:          /features/site_operation/watchdog_pet
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         3
Timestamp:    2023-02-23 09:08:30.89183

Method:       set
Uri:          /features/site_operation/watchdog_pet
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         4
Timestamp:    2023-02-23 09:08:30.91996

```

With batchSets disabled (set to zero), the "multi set" script will cause multiple sets to be generated within the DNP3 system as each set input is processed.


```

Method:       set
Uri:          /features/site_operation/watchdog_pet
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         4
Timestamp:    2023-02-23 09:09:31.101874

```


Note that this is a client side batch_sets configuration.
The throttling and collection of sets is performed in the dnp3_client. 




* Server Testing

You get the same result when setting the batchSets config on the server.
Or do we....

```
Method:       set
Uri:          /features/site_operation
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"watchdog_pet":4}  << note this output format has changed>>
Timestamp:    2023-02-23 09:15:51.800894
```


The server will combine sets for the same uri into a single "set" output.

Client input

```
#!/bin/sh
# multi set demo for dnp3_client_309.json and dnp3_server_309.json
fims_send -m set  -u /sites/lopeno/fr_response_enable_mask 1
fims_send -m set  -u /sites/lopeno/fr_response_enable_mask 12
fims_send -m set  -u /sites/lopeno/fr_response_enable_mask 1234
fims_send -m set  -u /sites/lopeno/fr_response_enable_mask 12345
fims_send -m set  -u /sites/lopeno/fr_response_enable_mask 123456
fims_send -m set  -u /sites/lopeno/fr_baseload_cmd_mw 6
```

Server Output
```
Method:       set
Uri:          /features/active_power
ReplyTo:      (null)
Process Name: DNP3_O_lopeno_dnp3_server
Username:     root
Body:         {"fr_enable_mask_dnp3":123456,"fr_baseload_cmd_kw_dnp3":6000}
Timestamp:    2023-02-23 09:27:58.350688
```



## BatchPubs (DNP3 Server)

The batchPubs operation acts solely on the dnp3_server.

The dnp3 server's normal operation is to monitor incoming fims "Pub" messages on its subscribed uri's.
These messages are translated into events updating the OpenDnp3 Server internal data opbjects.
Setting the "batchPubs" time period (in mS). Defers the internal OpenDNP3 server internal update to the batchPubs interval.
This means that if the server is connected to a system that issues pub messages every 10 mS, and the dnp3_server has a batchPubs rate set to 100mS.
The internal OpenDnp3 data objects will only update at the slower, 100mS rate rather than the faster 10mS pub rate.

This operation can be controlled dynamically after the server has started up , or in the config file.

```
Turn on dnp3 server batch Pubs operation
fims_send -m pub -u /local_server/_system '{  "batchPubs": 200}'
```
```
Turn off dnp3 server batch Pubs operation
fims_send -m pub -u /local_server/_system '{  "batchPubs": 0}'
```


# Testing


A new flag, batchPubDebug, needed to test / debug this feature.

```
fims_send -m pub -u /local_server/_system '{  "batchPubDebug": 0}'
```


The testing for this feature is simplified by using the debug flag.


With the debug flag turned on, the system provides messages when each batchPub operation is performed and includes details of each data point to be updated.

```
fims_send -m pub -u /local_server/_system '{  "batchPubDebug": 1}'
fims_send -m pub -u /local_server/_system '{  "batchPubs": 2000}'
fims_send -m pub -u /components/sel_735 " {
>         \"active_power\":1000, 
>         \"reactive_power\":1000
>      }"

```

```
 Time [75.697] batch_pubs [2.000] =======> running  next_batch_pub_time [77.697]
 Time [75.697] Checking  value_set   cfg [0] type: [analog] size [47]
 Time [75.697] Checking  value_set   cfg [0] type: [binary] size [22]
 Time [77.697] batch_pubs [2.000] =======> running  next_batch_pub_time [79.697]
 Time [77.697] Checking  value_set   cfg [0] type: [analog] size [47]
 Time [77.697] ===> found   value_set 1  name [active_power] varcount [1]  << variable set vaue detected >>
 Time [77.697] Checking  value_set   cfg [0] type: [binary] size [22]
 Time [79.717] batch_pubs [2.000] =======> running  next_batch_pub_time [81.717]
 Time [79.717] Checking  value_set   cfg [0] type: [analog] size [47]
 Time [79.717] Checking  value_set   cfg [0] type: [binary] size [22]
```

To test, set a slightly slower batchPubs period (1000 - 5000). Issue pubs at a faster rate to the server and make sure that the individual pubs sent to the server are captured at the batchPub time period.


Here is a typical script to issue pubs to the server.
/home/docker/git4/dnp3_interface/scripts/test_dnp3_batchPubs.sh

```
runSinglePub () {
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":1000, 
        \"reactive_power\":1000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":2000, 
        \"reactive_power\":2000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":3000, 
        \"reactive_power\":3000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":4000, 
        \"reactive_power\":4000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":5000, 
        \"reactive_power\":5000
     }"
} 

```
The batchpubs debug flag will turn on console output to detail operations related to the batch operations.



With batchPubs set to 0, there is no batch management on pubs , each pub sent to the server generated a data update.

To see the results on the client , set the frequency to 200.

```
"system": {
        "name": "FlexGen HybridOS",
        "protocol": "DNP3",
        "version": "0.2",
        "ip_address": "172.17.0.3",
        "port": 20000,
        "base_uri": "/sites",
        "id": "lopeno",
        "debug": 0,
        "local_uri": "/local_client",
        "frequency": 200,
        "respTime": 2000,
        "batchSets": 0,
        "maxElapsed": 100
    }
```

Run the runSinglePub  script on the server  
record fims_listen on the client

```
timeout 6 fims_listen -u /pub/test > /tmp/fims.out 2>&1&
```

The results are:

```
Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":5,"Timestamp":"02-28-2023 16:19:51.783066"}
Timestamp:    2023-02-28 16:19:51.783472

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:19:51.983963"}
Timestamp:    2023-02-28 16:19:51.984495

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":2,"reactive_power":2,"Timestamp":"02-28-2023 16:19:52.186050"}
Timestamp:    2023-02-28 16:19:52.186616

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":3,"Timestamp":"02-28-2023 16:19:52.386186"}
Timestamp:    2023-02-28 16:19:52.386470

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":4,"reactive_power":4,"Timestamp":"02-28-2023 16:19:52.587050"}
Timestamp:    2023-02-28 16:19:52.587583

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":5,"Timestamp":"02-28-2023 16:19:52.790568"}
Timestamp:    2023-02-28 16:19:52.791175

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,signal of type 15 caught.


```

Now set the batchPubs option on the server.
Turn off dnp3 server batch Pubs operation
fims_send -m pub -u /local_server/_system '{  "batchPubs": 2000}'


The server will only use the value of the last pub received , for a given datapoint at the batchPub time.
  

Run the runSinglePub  script on the server  
record fims_listen on the client

```
timeout 6 fims_listen -u /pub/test > /tmp/fims.out 2>&1&
```

The results are:
```
Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":5,"Timestamp":"02-28-2023 16:24:38.287057"}
Timestamp:    2023-02-28 16:24:38.287392

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:38.487453"}
Timestamp:    2023-02-28 16:24:38.487724

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:38.690397"}
Timestamp:    2023-02-28 16:24:38.690805

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:38.892346"}
Timestamp:    2023-02-28 16:24:38.892732

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:39.94751"}
Timestamp:    2023-02-28 16:24:39.094968

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:39.296506"}
Timestamp:    2023-02-28 16:24:39.296846

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:39.498580"}
Timestamp:    2023-02-28 16:24:39.498894

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:39.700612"}
Timestamp:    2023-02-28 16:24:39.700989

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:39.902944"}
Timestamp:    2023-02-28 16:24:39.903406

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:40.104739"}
Timestamp:    2023-02-28 16:24:40.105168

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":1,"reactive_power":1,"Timestamp":"02-28-2023 16:24:40.306663"}
Timestamp:    2023-02-28 16:24:40.307019

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":5,"Timestamp":"02-28-2023 16:24:40.508098"}
Timestamp:    2023-02-28 16:24:40.508572

```


The nonBatched  flag can be used to opt a selected variable out of the batchPubs operation.
This has to be set up on the server config.

In this example , reactive_power is set to notBatched.


```
                {
                    "id": "active_power",
                    "offset": 3,
                    "variation": "Group30Var5",
                    "name": "Site - Active Power Total Net",
                    "uri": "/components/sel_735"
                },
                {
                    "id": "reactive_power",
                    "offset": 4,
                    "variation": "Group30Var5",
                    "name": "Site - Reactive Power Total Net",
                    "uri": "/components/sel_735",
                     "notBatched":true
                },
```

This time , both active_power and reactive_power are changed at the same time but the active_power setting is throttled by the batchPubs operation.

```
Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":1,"Timestamp":"02-28-2023 16:39:05.102868"}
Timestamp:    2023-02-28 16:39:05.103290

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":1,"Timestamp":"02-28-2023 16:39:05.304254"}
Timestamp:    2023-02-28 16:39:05.304629

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":1,"Timestamp":"02-28-2023 16:39:05.506085"}
Timestamp:    2023-02-28 16:39:05.506483

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":1,"Timestamp":"02-28-2023 16:39:05.707586"}
Timestamp:    2023-02-28 16:39:05.707945

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":2,"Timestamp":"02-28-2023 16:39:05.908704"}
Timestamp:    2023-02-28 16:39:05.909150

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":3,"Timestamp":"02-28-2023 16:39:06.109239"}
Timestamp:    2023-02-28 16:39:06.109473

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":4,"Timestamp":"02-28-2023 16:39:06.310105"}
Timestamp:    2023-02-28 16:39:06.310426

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":5,"Timestamp":"02-28-2023 16:39:06.513151"}
Timestamp:    2023-02-28 16:39:06.513698

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":5,"Timestamp":"02-28-2023 16:39:06.715286"}
Timestamp:    2023-02-28 16:39:06.715688

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":5,"Timestamp":"02-28-2023 16:39:06.915816"}
Timestamp:    2023-02-28 16:39:06.916323

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":5,"Timestamp":"02-28-2023 16:39:07.118264"}
Timestamp:    2023-02-28 16:39:07.118721

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":5,"Timestamp":"02-28-2023 16:39:07.320455"}
Timestamp:    2023-02-28 16:39:07.320826

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":3,"reactive_power":5,"Timestamp":"02-28-2023 16:39:07.519759"}
Timestamp:    2023-02-28 16:39:07.519912

Method:       pub
Uri:          /pub/test
ReplyTo:      (null)
Process Name: DNP3_M_lopeno
Username:     root
Body:         {"active_power":5,"reactive_power":5,"Timestamp":"02-28-2023 16:39:07.720986"}
Timestamp:    2023-02-28 16:39:07.721307

```


The operation times  are exaggerated to allow the test results to be shown. In practice ,batchSet faster times will be used (100-200 mS).








 