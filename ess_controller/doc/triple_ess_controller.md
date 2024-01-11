From the ess_controller standpoint I think we have a good path forward and know how to do this.

We are just making sure that the UI (full_stack) team can do what we propose. 
I am confident that we can come up with a solution.

In short, we split up the current 17 (?) rack bms/pcs combo into an ess part and a bms part.
Template the bms part to allow 2 systems  bms_1 and bms_2 from a single template. 

run 3 instances of the ess_controllers one for each bms and one for the overall ess.
They can all chat to each other using fims.
We used something like this for the gpio controller in the tx100.

Modbus_client splits its data between ess / bms_1 / bms_2 .
This way we get two heavy lifters in the bms controllers managing the high data rate to the individual bms units.
The overall management is handled by the top level ess_controller.
We can split control as required between the main ess and the surrogate bms units.

The UI has to be able to accept data from 3 different sources. 

On a point of interest, the round trip of a fims command / response to the gpio controller on the tx100 was less than 150uSeconds.
(microSeconds) so this is not a source of concern.

The bms_controllers ( as they are now called) can chew up 50-60% cpu  if needed , they will be each running on their own cpu cores.   
We can live with the old modbus_client .
It's down to 50% cpu ( from 125%) thanks to reducing the number of component threads. 
The new modbus_client will be entering beta test at the end of next week ( estimated <5%cpu) .
 So, it may be mature enough for dual sierra deployment.

The bms and ess controllers all use directed fims messages to direct the modbus_data.
The ui may be a bit less flexible but we simply block and discard the unwanted messages from that source.

The ess_controller will provide a unified interface to the site_controller as usual.

 The PCS control can be split between the BMS units and the ESS as seems fit. The bms and ess controllers can all schedule start / stop tasks on each other as needed.

So, it looks good. Key components have been tested and we are working through testing to identify any config adjustments.

We still have the , mildly complex, task of breaking up the current config and templating the bms units.
3 -4 days with jimmy and myself helping.

 
The dbi config manager and ess split config loader makes this whole thing a lot easi