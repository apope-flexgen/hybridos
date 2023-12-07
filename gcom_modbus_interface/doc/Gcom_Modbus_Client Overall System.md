Gcom_Modbus_Client
p. wilshire 
  10_25_2023
  10_28_2023
  11_04_2023
  11_10_2023


### Post 11_03 Demo Actions

## Limit io_fims objects  TODO
  we'll create a limited pool of these objects , say 5 or 10 , see how many we use in a real system.
  the make_io_fims will be replaced with get_io_fims that will read object from the pool.
  Also we'll use the new LIFO_Channel ( name) that uses the deque object and pulls the first objet fromthe head of the list.

## Make sure auto_delete works 
DONE
we probably need a test for this

## Sort out the relationship between local get/sets and pubs etc
Done 11_10_2023

# Disabled points

If a point is disabled it means that it cannot be reached on the server

- pub request
  If a disabled point recieves a forced value then that point  responds with the forced value during a poll.
  This means that, when we create the io_work requests for a poll with a disabled point  but that items has received a forced value  we will include the disabled point but flag the io_work to use the local value.
  Done 11_10_2023

  The io_thread will not attempt to connect to the modbus server for a disabled point but will pass the io_work to the response_thread to include its value in the output if the value has the forced value flag true. 

- remove disable state
  if you send an _enable fims request that will remove all the disabled flags for the io_points flag.
  on the next poll or set cycle , if the auto_disable option is set and the point cannot be reached the point will be automatically disabled.

- remote get 
  If a disabled point recieves a remote get  then that request should return the disabled response.

- local get 
  If a disabled point recieves a local get  then that request should return the local value.

- local set  
  If a disabled point recieves a local set  then that request should return the local value.
  Te local_set flag should be set to allow the value 

- remote set 
  If a disabled point recieves a remote set  then that request should attempt to remote set but also return the disabled response if it is in error
   If auto_disable is set and the point does manage to connect with the server that point will be reenabled

- remote get 
  If a disabled point recieves a remote get  then that request should attempt to remote get but also return the disabled response if it is in error
   If auto_disable is set and the point does manage to connect with the server that point will be reenabled

# cannot connect 
TODO
Tidy up and limit the connect attempts 

# events 
TODO
Add events for connect / disconnect / restart / reload/ point errors

# dbi load 
TODO
need to add this

# dbi /file reload 
TODO
need to add this

# file name 
Done 11_10_2023
allow missing .json 

# Heartbeat
Done 11_09_2023

# Watchdog
Done 11_10_2023

# group multi remote / local sets 
Possibly todo


### Overall System  Overview

## System Basics

The system has two main functions:

- Query the Modbus Server on one or more periodic intervals to collect the Publish Data sent to the remote Modbus Server and replay that as Fims publish messages on the local Modbus Client.
- Allow incoming Fims Set messages to be translated into control outputs on the remote Modbus Server.

Some minor functions include:
- Network Connection maintenance 
- Fault Recovery
- Data Flow management 
- Heartbeat generation and forwarding 
- Watchdog  monitoring
- System Alarm and Event generation.


The complexity of the system is increased to allow for network and system performance of the Modbus Interface components.

The Gcom project has also added enterprise quality diagnostics and performance monitoring to the basic system.

A number of integration and commisioning enhancements have been added to the system to greatly improve the time to bring up and deploy these systems in the field.

## Expected results 

- Integration problems  including setting up a modbus client config to communicate with a modbus server ( Vendor or FlexGen ) will be reduced by a factor of 10 ( guess)
  - The system can validate a conection to the remote system and then check the correctness of the configuration and identify missing or incorrect points.
  - The system can automatically disable incorrectly defined or missing  points  allowing integration to continue with the remaining working points while the correct configuration options are reviewed.
  - The system can be tuned to optimize the connection to different hardware devices and can attempt to self correct  data paths where possible.
  - diagnostics and performance data is published and available in optional detailed queries ( Using Fims)
  - An Api will be available to support FlexGen Ui systems.

- Commissioning teams will get continuous performace data from the system allowing status and diagnostic data to be used to quickly determine system performance.
  - Lets guess that the new tool will reduce the time taken to troubleshoot Modbus Client Communicaton problems by 50%
  - Single point and multi point  queries will be available to examine local / remote data points as well as an indication of which process last set a data value and when that value was updates.
  - Commissioning and test teams will be able to force values and / or disable data points as required for system testing.
  - The system can provide a rapid point by point scan of the remote Server.
  - Site Teams will be able to look are encoded / decoded values and raw data as needed.
  - The use of a simple UI tool will enable io point manipulation on a par with the current SEL or Modbus Poll.


## Software Overview

The Gcom Modbus Client uses a combination of standard C++17 systems and series of custom objects to control and direct the data flow between fims and Modbus Servers.
These custom objects were originally designed for use with the ess_controller and enhanced for the go_metrics projects.
In particular the Flexgen channels and FlexGen Timer systems were deveoped over three years ago and have been used in the ess_controler since its first inception.



The data is managed by a number of fundamental objects.
A series of threads handle data operations.
Data flow uses FlexGen custom Data Channels to provide data management and thread syncronisation.


The system Configuration is managed by a interpretation of the config json file that has been transferred into a c++ abstract (std::any) object.
The Client Configuration is then "extracted" from the "gcom_any" object as a series of object views.
The basic io_points are ordered into two views. 
   One defines the publish groups, however, these groups include Coil and Holding output registers as well as the Discrete Inputs and Input registers.
   The other organises the variables buy component names and register id



Fims input data is managed by an io_fims data object.

A high speed Fims Input thread reads the fims data into an input buffer and then passes the completed buffer to one or more  Process threads for data extraction.

This double buffering of fims data  allows the fims_server to maximise its data flow and eliminates the risk of missing data objects from the server.

The fims data is then proken down into work tickets, these work tickets are collected together onto work group objects with one or more tickets.

These work group objects and tickets are dispatched to on or more IO Threads to allow them to communicate with the Modbus Servers.
The IO Threads will complete the IO Operation , if possible , and then transfer the completed work tickets to a single response thread.
The response thread is the only component in teh system that has write access to the collection of io points defined by the system.
In fact the raw data component of the io point is only accessed by the response thread. This removes any thread data contention problems.
The use of the single response thread also removes the need to lock access to the io_points.
Other componetns in the system can send a work ticket to the response thread to get data into or out of any io_point.

Individual work tickets are processed by one or more io_threads as the Modbus Server spec allows.
These work tickets are reassembled by the response thread into completed work group components.
For example a pub request may call for several 10s or even 100s of individual work ticket items.
Only when the server has resoonded to all of the indiidual tickes can a pub response (fims output) be formed.
Config options will enable partial pubs skipping failed work tickets.

Local Work tickets
These bypass the server sommunications and are routed straight to the 
response thread.

Here they are or are not grouped.
All temporary objects (io_fims , io_work, io_group) are returned to their respective buffer pools for recycling as needed .
When the system wants an instance of one of these objects it will attempt to get a previously used object from a buffer pool before making a new instance of the selected object.
object recovered from a buffer pool will be sanitized or reset before reuse in the system.



In addition to the main process threads the system has additional components :
- Timer 
    This controls the initiation of a pub or watchdog requests.
    The timer has two important features
    - All timeer requests are synced together with a selectable offset.
      So if you want to have a process running every second with a 0 offset and another one running every second with a .25 second offset the processes will be triggered to run 0.25 seconds apart regardless of when the timer was initally started.
      This also has the benifit of providing a priority scheme.
      A timer scheduled at 0.24 seconds past the 1 second interval will always run before a timer scheduled at 0.25 seconds past the same interval
    - A pub timer can be delayed by the late completion of a previous pub request.
      This means that, when running with a delayed network, or a slowly responding ModBus Server if a pub request ( scheduled at every 100mS) takes over 50 mS ( config option) to complete the next pub request will be delayed but the time of the overrun.
      There is no point it hitting a poorly responding system with repeatd pub requests , thus making the overall situation worse.
- Heartbeat
  Attached to a timer the sytem can provide an original or a reflective herbeat to the Modbus Server

- Watchdog  
  The system can identify one or more io points to act as monitored watchdog items. These points must change values on a periodic interval or a fault or warning will be generated.

- Perf 
  A performance objct that makes it easy to monitior and measure system performance metrics

- Logging
  The system provides a level of smart logging . This avaoide repeated identical log mesages but also reveals the filtered data if a logging system event occurs.




### Overall System  Checklist

This is a list of things to validate as thE progect neArs completion.




# 1
Read Config and extract all objects 

done PSW 10_25_2023
   ongoing additons to extract serial data and watchdog components 


# 2
Set up a fims_listener and process threads to receive incoming messages.
done PSW 10_26_2023
    Currently running wide open , may need to provide LI/FO opeation on the data store channel.


# 3
Set up Timer Object 
Pub Timers
done PSW 10_26_2023
TODO sort out the sync problem


WatchDog timers

Heartbeat timers

The presence of a comp heartbeat config opton will trigger a timeer to send an io_work object to the response thread to check the heartbeat at the heartbeat interval.
This will bypass the io_threads ( if sent there it will be rerouted to the response thread )
the heatbeat opeation will check for a value changed and emit events to indicate heartbeat missing, active or recovered.
This heartbeat output will increment by 1 unless a heartbeat read uri is detected, in which case the output will be the input  value plus 1.


The basic client (MVP) will have a simple watchdog running at the same frequency.

The enhanced watchdog ( if we have time) will have three trigger levels.
The Alarm level will specify a time after which, with no change in the watchdog signal will issue an alarm alert and set the system state to alarm.
The Fault level will specify a time after which, with no change in the watchdog signal will issue an fault alert and set the system state to fault.
Once either an alarm alert or a fault alert has been issued the recover time counter will be set up.
If  the watchdog signal is restored the system state will be set to recovering. 
if the system state is in recovering the alarm and fault timeouts are increased to the config levels.
If the watchdog signal continues to be good the recovery timeout will be decreased. Once this timeout has expired , with the watchdog signal still ok , the system will enter the restored state and eventually to the normal state.

Should the watchdog signal recover after an alarm alert the watchdog will start a recovery time countdown. When the recovery time has completd the watchdog will return to the norml state. 

We can have a special heartbeat and watchdog objects to handle any state storage required
look at createpubs 

We have named maps of watchdogs (wdogs) and hearbeats (hbs)

The Response Thread

The response_thread is the only place we have access to the io_point raw  data.
The io_threads  will use the io_work work space to read and store data.
The response thread will translate the io_work data into the io_point raw value.
The response thread will also prepare the decoded output from io_work data or the io_point raw data if a local object request is received.

If we want to check a heartbeat of a watchdog we have to send an io_work message to the response thread.
When a response thread gets a poll response for an item that includes a watchdog io_point the watchdog process is called to manag the watchdog state.
a watchdog point is also added as a timer object to manage the state when the poll is not running. The timercallback also sends an io_work item to the response thread if it needs to monitor or adjust io_point data.

TODO the currrent system uses a static allocator to generate the pub_group items. I think we'll modify this to use the standard shared / stored pointer system to manage these items. We'll try to set up a common io_work_group object.




# 4

Set up 
IO_Threads and response thread
done PSW 10_26_2023
   still to add new io_group object to hold replyto etc.



# 5

Command line options
  just file name run with named config
  -c file name run with named config  done 11_03
  -f (-u) dbi name load from dbi
  -t test_name  run named tests.
  added override system 

added full override system   done 11_03 


# 6 
Pub status report 
inc pid git data and SOH 
done 11_03

# 7
Watchdog 
done 11_10_2023

# 8
Heartbeat 
done 11_09_2023

# 9
Fims Pubs 
done 11_03 but need to handle disabled points see note above  done

# 10
Fims Sets 
done 11_03 
local /remote done 11_10

# 11
Fims Remote Gets 
done 11_03 but see note

# 12
Fims Local Gets 
done 11_03 but see note

# 13 
Pass down format
TODO

# 14 
Format override.
TODO

# 15 
Stats reset
TODO

# 16 
Events
TODO

# 17
Clean up connect attempts
TODO

# add tNow to io_point
TODO
