### Dc Coupled Design Overview

## Introduction

The Dc Coupled design combines a PV genertor , multiple DC coupled Bms system and a PCS system into one unit.


The system can operate in one of many modes.

1/ PV provides grid power.
2/ Batteries provide grid power.
3/ Batteries charged from PV
4/ Batteries charged from Grid
5/ PV provides power to grid and charge batteries
6/ Batteries charging in balanced mode from grid or solar power
7/ Batteries discharging in balanced mode from grid or solar power


In general the PV will provide power for the grid with a possible battery charging option.
Grid can be powered from PV or batteries  or both.


The three possible use cases are:
    PV provides power for grid , any excess power charges batteries, or is curtailed.
        Batteries power Grid
    Grid provides power to charge batteris.



The generic term "batreries" is used to describe one or more battery modules. DCS (DC Coupled Storage)
These are DC coupled so we have to think in terms of Power rather than Voltage plus Current.

The DCS units each have one or more associated battery Racks.
Thise are connected by Bus Bars and , for these, we have to think in terms of Voltage and Current.
All batteries will have a common Voltage.


### Its a Power System.

Considering the connection between the PV array and the PCS we need to think in terms of Power. 

The PCS will accept the PV voltage and draw current as required.

I suggest that the concept of a power manager be introduced into the system. More later.

The individual DCS systems will manage a group of bateries and will provide , by means of a dc-dc power stage an interface to  the connection between the PV and the PCS.
These units can operate independently and they are directed to provide or consume power based on the needs of the system.
I think that the system can either be in a net charge or discharge state but the power distribution to or from the individual DCS units will depend on the overall state of charge (SOC) of the attached battery units.

There may be a mode where some DCS systems are charging while others are discharging. 
If this is included as an iniital design consideration it will add minimal complexity at this stage but will be difficult to retrofit if not considered now.


### Power Management Control

There are, in this design concept, two , identifiable power management stages. Put thos in place and the complxity of the "rest" of the design is reduced.

The simpest is the main PCS power  manager.

                               +---------+
|PV power|           =====>    |         |
                               | Power   |  <====> | PCS | <===>  Grid   
|DCS Power|          <=====>   | Manager |
                               |         |
                               +---------+



The DCS system also has a power Manager whose job is to distribute the charge/discharge requirements to the individual DCS units.


| Batt 1 |<====>|                              +===============+
                |         +-------+            |               |
| Batt 2 |<====>|<=======>| DCS_1 | <======>   |   DCS Power   |
                |         +-------+            |   Manager     |
| Batt 3 |<====>|                              |               |
                                               |               |<================> {}
| Batt 1 |<====>|                              |               |
                |         +-------+            |               |
| Batt 2 |<====>|<=======>| DCS_2 | <======>   |               |
                |         +-------+            |               |
| Batt 3 |<====>|                              +===============+



The DCS Power Manager instructs the individual DCS units to  manage its total power requirements  

Lets say 
     DCS_1 has a SOC of 80
     DCS_2 has a SOC of 60
     DCS_3 has a SOC of 40
     
The Total DCS system has a power discharge requirement  of 100 units

   Ask DCS_1 for 44.445 units
   Ask DCS_2 for 33.333 units
   Ask DCS_3 for 22.222 units


The Total DCS system has a power charge requirement in which case the demands are reversed
   Give  DCS_1  22.222 units
   Give  DCS_2  33.333 units
   Give  DCS_3  44.445 units


The dynamic power balance equation 
    DCS_n = Preq * Dcs_n.SOC/Dcs_SOC_Total 
     

We can set a Demand SOC into this equation to make it maintain the overall soc balance
This means thet the DCS's above the SOC limit have to provide power to suit the demad AND provide charge for the DCS units below the min SOC. Note that this feature may not be needed or required.

