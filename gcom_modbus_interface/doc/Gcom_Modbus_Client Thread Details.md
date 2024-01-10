Gcom_Modbus_Client

Thread details

p. wilshire 
10_26_2023


### Overview

The system has the following threads.

listener_thread
  this is a simple fims_listener thread.
  it runs a fims_receive function using an io_fims object to collect the fims data.
  The io_fims objects are stored in  the io_fimsChan Channel.
  The make_fims process  recovers recycled io_fims objects or creates new ones as needed.


  Once fims data is received the io_fims object is sent to one ot more process_threads using the io_fimsProcessChan to queue the io_fims object and the io_fimsProcessWakeChan to wake up a process_thread to service the data.


process_thread;
  This is a fims process  thread it decodes the io_fims object and 
  originates  the set / get  fims operations
  More than one process thread can be used but , if  strict serialization of the incoming data is required at the exit of the process thread then the collect thread will have to be used. ( See below)  


  If the  opertions defined in the io_fims message require communications with the modbus server one or more  io_work objects are created.

  Each io_work object will contain refernces to one or more io_points and a data buffer (in the case of a set). The data buffer contains the encoded data in the fims set message.

  Where a number of registers are to be handled a vector of io_point requests is created, this vector is then sorted by device_id, reg type and grouped into blocks of contiguous of contiguous objects or io_points . These can be read from the modbus_server in single read write cycles to one or more registers.
  Note that the current flexgen modbus_server cannot handle multiple register writes.

 The io_work objects are sent to a Poll or Set chennels to be processed by the io_threads.

 Currently only one process thread is used.  The serialization operation may need to be completd.




collect_thread;
Future concept, used to sync multiple process_tread and provide a time ordered  output.
As each fims listener thread receives a new fims message it sends a copy of the io_fims structure to the collect_thread.
The collect thread stores thee references in a vector organised in time order.
As the process thread completes working with its io_fims message and it ready to submit it to the io_thread set or poll channels t will send another signal to the collect_thread indicating that the io_fims object processing it complete.
If the io_fims object is at the top of the vector the collect thread will send a "continue" message back to the process thread. The process thread can then continue and send the io_work objects to the io_threads.
The process_thread then send a "done" message back to the collect_thread which sends the io_fims buffer back to the  io_fimsChannel for reuse.
The collect_thread then removed the reference to the io_fims messag from its vector and then proceeds to examine the next (if any) io_fims message on the vetor.
If that message has been completed by the process_thread then the collect_tread can send the "continue"t thread.


This process is a bit interactive but ensures that the set and poll channels are loaded with the complete poll or set group before getting another set of io_work items.





ioThread
These receive either Local/Remote Poll/Set io_work objects. 
They are responsible for  maintaining the modbus_context and attepmting to either communicate with the modbus_server or access the local database of io_points.

Initially the threads may be unable to connect with the server or the threads will loose the server connection after being set up.
This is handled as follows.

Each threads starts up as not connected ( io_thread->connected = false).
If the system has been given an ip address the thread will attempt to connect to that address.

SetupModbusForThread

This will leave the thread as either connected or not depending on the availability of the remote server.
Notice that the attepmts to connect with a specified server are controlled by a delay process.

OkToConnect
Determines if sufficient time has passed between the last connect attempt by a thread and the current time.

DelayConnect is called after each , allowed , connect attempt to set the delay time for the next connection attempt. 
myCfg.reconnect_delay , ( time in seconds) is used to provide a short delay between successive connect attempts.
This is preset to 0.100 seconds but can be overridden in the config.



The io_threads will start running and immediately attepmt to connect to the server.
If they fail the threads will be running in local mode only.
However , if all threads are disconnected from the server one thread will take over the dispatch of any remote requests that may have been issued prior to the threads disconnecting.
This dispatch will simply set the io_work->error to (-2) to cause the response thread to ignore any values and just permit the collection of the response groups.

This means thet, if needed the publish groups timeout  will continue to be serviced should the threads suffer a disconnect after starting.

The publish groups and heartbeat processes and fims_remote requests should check to see if there are any connected threads  
using

GetNumThreads

and not issue rempote io_work objects if there are no threads to process the requests.



The error recovery will attempt to reconnect , clear invalid data , and handle server error states.
Note that the transaction timeout is used to force the server communications attempt to be bounded.

once a responce ( success or error) has been received the io_work object is passed to the responseThread.


error 32 == broken pipe .. do not disable points


responseThread

This examines the completed io_work object.
If the io_work object is part of a pub group the response thread will attempt to collect all members of the pub group.
Once collected the pub_group is then decoded and published as a fims data object.
The response thread also handles out of sequence io_work responses and will reset the pub group (with an error report )  if a new pub group is started before an older one has completed.
The response thread will also handle the completion of set io_work objects ( TODO we'll add the concept of a set group )

io_work objects are collected into "work_groups" for pub , multi set or multi get operations.
The response collator will detect the group name in the io_work object, 
( if the name is missing the io_work object will be ttreated as a single get ot set operation)
the response collector knows the time of the original io_work request and also the number of io_work items in the request.
Once all the io_work object have been collected the work_group can be processed.


This allows me to collect a multi set operation into a work_group and provide a  fims replyto on the success / failute of the whole group.
multi_set is a new feature that allows you to define  a mutli variable set  from one fims input message.
I think we had it with dnp3
The modbus server can either get this multi set split into single register operations or as a set of io_work object setting multiple values at once.
This single multi split is done when the group is generated in the fims process thread.


When a fims multi set input is received,  a transient work_group ( like a pub group but not fixed by config ) is set up with a collection of io_work items.

The response thread will attempt to collect all of these together and provide a fims response ( to the replyto) indicating  success or failure of the whole group.
( We may extend  this to indicate which registers failed) 
this operation  relies on the io_thread having a time bounded response to an individual request.
So the io_thread will decide , within a fixed time window , if the interaction with the server can be completed.
When multiple io_threads are available , the ordering of the io_work operations may well be disrupted.
the response thread will take care of collecting the completed io_work operations and then running the response oiperation on the completed set of io_work requests.



The pub_group response will be the same as currently designed ,
The set_group will have the simple pass fail response sent to  the fims replyto if specified.
The get_groups are like pub_groups but will use the replyto for the fims response.

All of these options will have a "local" flag in the io_work object. The pub requests will always have the local flag set to false.
If the "local" flag is true the io_thread will not "get" the data from the modbus_server but instead read the data from the local io_point.
Or if the operation is a "set" the io_thread will update the local io_point without going through the modbus_server connection.
The uri extensions "_local" will trigger the local operation.

The io_point will have a buffer reg16[4] reg8[1] to store the local copy of the data. ( this concept was not in the original modbus_client)

So when we get a completed pub group in the response thread the handler will also copy the  io_work data into the io_point local store
 When we get a completed set group or a completed single set the ip_point local store will be updated.
When we complete a remote get_group  or a single get the handler will also update the local store.



TimerThread

This thread is responsible for collecting timr requests.
Each timer request is given a callback function to process timer completion.
Timer objects are each given a name and can be controlled using its name.

Timer object are mainly used to schedule pub operations.
They can also be used to schedule watchdog and heartbeat monitoring operations.

Pub timers have a special sync operation.
Once a timer opertion has been signalled, using a timer callback, a sync message may be sent to the timer. This is used to detect the need for "deferred" pub requests.
If a pub has taken more than 50% ( configurable) of its cycle time t=before it receives a sync then the next pub request will be delayed.
The pubcompletion logic will typically send a pub sync to the pub timer object.
(TODO) The first time the timer object gets a sync command the timer object will be placed in "sync" mode.


WatchDog Thread
TBD

Process Tread 
TBD





