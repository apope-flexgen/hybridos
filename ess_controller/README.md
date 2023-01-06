# ess_controller

release v10.1
 

The ESS Controller Coordinates the batteries and their safety mechanisms with the environmental systems, including the HVAC and fire fighting system (FFS), within the battery container or building and  the DC/AC power conversion  ystem (PCS). The ESS controller responds autonomously to battery or PCS faults that could lead  to equipment damage or fire

To further limit access of controllers supplied by offshore vendors to  the higher level control networks, FlexGen proposes to supply its own ESS Controller in that position. This will further ensure that there can be no sabotage of the high value equipment (batteries and PCS) by mis-firing a sequence or ignoring protections. The FlexGen ESS Controller will also isolate the control networks between the System BMS, the environmental systems and the DC/AC PCS to limit unauthorized interaction.


## Jobs  include controlling

• BMS - Battery Management Systems (rack and system)

• PCS - Power Control System , and coordinating/sequencing that with the BMS

• EXCON - External connections (dry contact I/O and modbus)

• EMS - Environmental management system (ambient temp, HVAC)

• FSS - Fire suppression system (turn off the HVAC when there's a fire)

***

## Table of Contents
* [Getting Started](doc/md/guide.md)   
* [User Interface](doc/md/ess_controller_ui.md)  
* [System Alarms and Faults](doc/md/alarms.md)  
* [System Monitoring](doc/md/monitoring.md)
* [ESS Controller Calculator](doc/md/ess_calculator.md)
* [ESS Controller Functions](doc/md/functions.md)  
* [Configuration Options and Features](doc/md/config_options.md)
* [System Date and Time](doc/md/date_time.md)
* [Database Interface (DBI)](doc/md/ess_dbi.md)



