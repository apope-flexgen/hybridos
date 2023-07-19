We currently have 3 ways to, flexibly, manipulate data.
metrics - typescript, interesting config , tricky to set up ( by comparison to other projects)
fims_echo - go, not yet complete ,  interesting config, architecture may not scale (already showing problems)  
ess_controller - C++ , very complete , well-developed config, quite fast but is a bit of a "bolt on " design.


The project is to combine the best parts of each (in go if we can), make it easy to configure into a new entity,
Let's call it "go-metrics".

We have to stream in a lot of data - 5000 bytes 10 times per second from 100 data sources.
We have to pick out about 100 data items from each source and perform metrics ( calculations) on them.
We have to extract about 300 data items from multiple sources and publish new data streams with that data.
 
We are going to have "large" systems with lots of memory to do this with.
In fact, I bought such a system two years ago called "Big Phil" it had 32 X 1.5G cores and 96G of memory.

This project could be considered as an ess_controller mk 2 

FlexGen want to develop its Software Platform into something that can be used by 3rd Party companies.
This project intends to "clean up" what we have and provide the basis for that transition.

Having said that , I can see, that we have most of the pieces on place for this to work. Last summer produced the Update Tool but also was a design exercise for this project.

So we have a multi threaded Fims message receiver. Parses into interface objects. Merges selected data into data objects. Use read write locks to protect data access.
Incoming data is pushed into the system, Outgoing data is pulled from the same system.
Incoming data can result in an "immediate" data output.
Each selected component can have a list of Metrics ( ess_controller actions) applied to them in entry (Set) or exit (Pub) from the system.
This means that we can combine data from multple sources ( different input objects) into internal data objects and then use them to present data lists to the rest of the system.
But it has to be real time. Lets give us a 10 milliSec response target, it could be under 1 milliSec ( We have yet to determine that).
(By comparison the "ess_controller" was designed with a 250 microSecond response capability.)

Why not use the ess_controller ? Good question. It has a lot of fine design concepts, but those concepts could be put together in a better framework. 
Why not develop fims_echo ? Another good question. That product does not look like it will scale well and it would get more complex as the design continues. 
Why not Metrics ?  Same thing. It is complex to configure , not really best for 3rd party handover.
  
Another thought , Why invent a new wheel , do we loose all the work we've already done?
   Not at all , 80 - 90 % of software design is in the concept , not the lines of code.
   We have a lot of "concepts" already captured. Go allows us to quickly turn those into code.
  We also, now, have a clear idea of the problem and how to solve it.

Configs have to be written for the end user, not the code developer.

Ashton and Preston have been doing some research already.
The same problem was given to fims_echo, metrics and ess_controller

  fims_echo took 14% cpu
  metrics                8%
  ess                        < 1%

110 lines of metrics config can be compressed into 11 lines of ess config.
fims_echo could not be configured for that operation.

So, I have broken the design up into a number of small incremental steps.
I/We will have a complete product design spec completed in Jan 2023.
We will produce an MVP(Min Viable Product)  by the end of March 2023.

A number of research tasks are in progress to facilitate this.

For example :

Profile Data
  determine and record real time performance data.

Thread fims input  
   do not unmerge the input object, pass that onto a thread.
   allow a uri_prefix to specify operations /ess_1/metrics/system/stuff ( where /ess_1/metrics  is used to trigger              operations and /system/stuff is the object). 
  
Complete the json merge
      make it perform fast,
      allow it to expand the object or not,
      allow it to optionally trigger the metrics ( Calculations)
 
Define metrics operations
  Simple - echo one uri to another 
  Medium - add, subtract max min operators
  Complex - rule based processing (go - grule for example)

