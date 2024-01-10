
### Modbus Client Data Flow: The Pub Path

**Author:** P. Wilshire  
**Date:** September 26, 2023

#### 1. Introduction

The "Pub Path" serves as a crucial segment of the `modbus_client` data flow. This document will detail its functioning, from setup to synchronization and the handling of different edge cases.

#### 2. Configuration Specification

Clients are configured with components that have a pub request specification. A simplified specification includes:

```json
{
  "components": [
    {
      "id": "clou_ess_01_hs",
      ...
      "device_id": 98,
      "frequency": 50,
      "offset_time": 0,
      "registers": [
        {
          "type": "Holding Registers",
          "starting_offset": 1000,
          ...
        }
      ]
    }
  ]
}
```

Key elements in the configuration:

- **component id, devie_id, frequency, offset time:** Define a pub request.
- **type, starting_offset, number_of_registers:** Define the data blocks associated with the pub request.
- **maps :** Define the items within the data blocks associated with the pub request.

#### 3. System Initialization

Upon system startup:

- A series of pub request objects are established.
- These objects reference the register array and produce a vector of `io_work` objects to be placed on the `io_pub` queue.
- When it's time to pub, placing  the items on the io_pub queue triggers the io_threads to send these requests to the servers.
- THe io_threads then place the results of each io_work object onto an io_responce queue to be serviced by te io_response thread.
- Each `io_work` object has a timestamp reference (`tNow`) and is tagged as part of a sequence related to an individual pub request event.

#### 4. Timer Requests

For each component, two timer objects are generated:

1. One is based on the component's frequency, offset, etc.: `<device_id>_<name>_<id>`
2. The other is for request timeouts: `<device_id>_<name>_<id>_timeout`

The first timer object triggers a single pub request and awaits a pub sync notification before any subsequent pub request is released. 
The second timer object serves as a notifier for detecting system delays during pub request servicing.

#### 5. Pub Request Flow

When the pub timer callback is activated:

- The `pub_timeout` object is initiated.
- The `pub_request` queue gets populated with copies of the `io_work` objects.
- The system then awaits either a pub sync callback from the response system or the occurrence of `pub_timeout`.
- If `pub_timeout` is received, the pub request will be re-issued up to 5 times (or we'll perform a more comprehensive error handling process). 
   During this time , events are logged, and the system may potentially shut down.

#### 6. Pub Request timeout


Upon servicing of the pub timer callback:

- The response system receives completion notifications for each `io_work` object. 
   The io_threads will produce a response to the io_work input object even if communications with the serveer fails.

- the io_work request may fail for the following reasons.
    Lost connection
    Connection TImeout 
       We'll need to reopen the connection. no point in attempting to communication with the server if this is not done.
    Invalid Data
       This can usually be fixed using a modbus_flush. So the reaction should be to retry the request after a flush. bit dont sit there in a tight loop.
    Invalid IO Point 
        This is may be a config error. The trouble is, if multiple points have been requested, any point in the group could be the bad item.
        An option here would be to retry the points one register at a time.
        But consider the effect of the point request taking a finite time per point. The effect of running a single point at a time may result in a cascade of errors.
        Possibly consider an option to run a binary search for the bad point.

        The current system of rejectng the whole block because one point is invalid is "not ideal"
        Also consider if the server has previously responded to that point which has now become bad. It look like we need a means to validate points and then detect for validated points suddenly going offline.
        we should also consider the option to complete the pub request , with error notification, with the data from the other points that responded with no errors.
     Server Busy
         possibly delay for a short period and then retry a finite number of times.
           

- For incomplete tasks, decisions must be made: If 9 out of 10 `io_work` objects complete successfully, the decision might be to publish the successful ones and report the failure.


- Once the required number of io_work objects have been received, Post all responses, pub completion starts.
    - Returned data is decoded into local data objects.
    - A composite pub message is created for transmission to FIMS. Potential decode failures must be addressed.

- After receiving all data for a pub, the response system:
    - Cancels the pub timeout timer.
    - Issues a pub sync.

- The remaining time in the pub interval is examined, and the next pub request might be postponed if the time left is less than a predetermined percentage of the pub interval.

One design consideration is how to handle late incoming results for requests from a previous pub.
If a subsequent pub request has been completed, consider throwing the late result away.


One reason for a completing a late Pub response would be the need to service Set requests while the pub is in progress.

Questions: Do we want to consider a pub request as uninterruptibe. Do we want to completethe whole pub request before we consider any sets.
         : if we are in a multi threaded environment the io_work requests may get out of order. The current design allows that.
        : do we allow set requests on other threads while still processing pub requests.

If the system sends several sets during the pub request , these will be serviced preferentailly over the pub io_work items.

An input filter may be applied to batch/throttle  the set requests but, with large numbers of set requests a given pub request may be delayed beyond the "normal" response time.

Clearly an individual, one off, delay in responding to a pub request may be "normal" but if the server or network starts to fail  then the system may have to be smart enough to handle the situation.


So, to accomodate some of these considerations.
The config pub_request object 

     Its time for a pub request.
     do we have any outstanding requests of the same type ??
     if so are they past their completion time ( default 2*frequency)
     if so we'll have to cancel them , we are no longer interested in the data.
     take the time tNow
     create a timeout timer request
    prepare a list of IO_work points point them back to the pub_request.
    do we have any disabled points ?
    ( just keep a vector of disabled point map items)
    split up the io_works if so.

    send them to the pub queue.


    A pub_request strucure needs to be defined 
      do we need an instance of these ?
    The io_work object will have a reference to the pub_request object.
    The io_work object will also have a reference to the config register map. (It needs this to get to the encode/decode data anyway)
    The system will keep statistics on the performance of the pub request operation.
    Individual data points may also have stats. 
    We may consider point enable / disable. 
      Customer has a new config and a given point may not be included. We may need to keep running but disable that point until we get our config to match the customer's setup.
    If a point is disabled the io_work object will have to be split into two objects. So give the io_work object a vector for sub_work objects.
       



#### 7. Conclusion

The Pub Path workflow, as part of the `modbus_client`, showcases the importance of synchronization, timing, and error handling in ensuring efficient and accurate data flow. Properly understanding each step will be essential for debugging, extending, or interfacing with the system.