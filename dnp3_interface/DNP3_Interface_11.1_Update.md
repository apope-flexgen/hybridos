### DNP3 Server  11.1 update summary

## new issues 02_14_2023
## updated    02_28_2023
## updated    03_01_2023

* fix crash in parseMessages caused by invalid fims_data

* track state in sys object.

* cops interface

* DNS entries may be alredy there.

* add Heartbeat as per modbus.
   Specify Client heartbeat send register and update period. Value increments.
   Specify Client heartbeat read register , client expects this register to change value or the system is presumed to be down.

* Shutdown option on client - server comms lost 
   After a config ( with default) period of waiting for client server comms to be established  the client will shutdown and restart.
   Now possible since we have a timed service inside the client main for batched sets.


* Single shot error report for component comms lost.
    It was called comms lost because that is the error reported by DNP3 . A better error code would be "Variable Update Timeout" or something like that.


   currently , we get this on every update timeout  , the message is confusing the state is "OFFLINE" or [0]
```
    Time [3915.004] Comms lost  db [analog_1] state [0] ONLINE [1]
    Time [3915.004] Comms lost  db [analog_2] state [0] ONLINE [1]
    Time [3915.004] Comms lost  db [binary_1] state [0] ONLINE [1]
``` 

This needs to ba a one shot or latched message with a report of off line on onine when the state is restored.



* Add bitfield encode / decode
  Copy operation from modbus


* fix large floating point number
   produce an error (latched) and send 0 to server ( or client)

    Currently largest floating point number you can send ( to 13 decimal places) is  34.0282356779733e+37

```
on client

fims_send -m set -u /testcli/testop/TestOPF32toSite 34.0282356779733e+37
```


```
produces on server

Method:       set
Uri:          /testsrv/testop/TestOPF32toSite
ReplyTo:      (null)
Process Name: DNP3_O_hoth
Username:     root
Body:         3.4028234663852886e+38
Timestamp:    2023-02-14 14:19:22.821244
```

## work completed or in progress on current hotfix -- /bugfix/server_batchSet

* Fix bug preventing batchSets working in the server.
       note: Bug only seen if batchSets configured for server.
       There was no process for turning batched pubs into builder updates which are used to update the invisible opendnp3 database.  

       If you turn off batchSets on the server  (set to 0.0)  or disable a variable  (notBatched=true) and there is no bug.

* add missing variable Groups,Versions and Types
    Add full list of groiups and vars , add missing OutputStatus and Counter types
    Test all the above ... needs test framework.

* add requirement for server to terminate  if it is missing  initial input data values.
   The server issues gets on all the requested data items but currenlty does not  terminate if responses are not seen.


## work planned for the rest of the 11.1 update.

* add sparse pub option
   Only pub changes


* Split input and output data areas. 
    Allow the same URI / VAR names in both input and output areas. 
    Provide fims access to each area using a prefix.
    The use of the same names in the client and server data spaces allows a transparent interaction where  the Client "sets" an output value and the server responds on a different feedback input value.
    On the client the feedback value will have the same URI as the output values.
    So we Set active_power to 1234 on the client
    the server repeats the Set to active_power.
    the server gets a pub indicating the "actual_active_power" from the PCS system. say 1100
    this is then sent back to the Client via a pub into the server.
    The client receives this pub and then pubs the result on the same URI as the original Set.
    Under "normal" operation  no prefix is needed. 
    It's just there to allow data points with the same uri to be examined when needed.

    currently working on the big fix branch.
    needs to be ported across to 11.1

* DBI based config
    currently working on the big fix branch.
    needs to be ported across to 11.1

* Hot Config Reload
    currently working on the big fix branch
    needs to be ported across to 11.1

