# UI FIMS Send on the FIMS page in Inspector

## Goals

* Allow user to send FIMS messages from the UI in the same manner as it is done on the Command Line.


## Modules/Repos Affected

* web_ui
* web_server


## Assumptions

* Sending FIMS messages from the UI is valuable.


## Interface

* Send a FIMS message using the same syntax as on the Command Line, e.g., `/assets/ess/ess_1 '{ "maint_active_power_setpoint": 0}'`. The message sent will be displayed in the browser's console.
* Multiple URI/value pairs can be sent at once by separating them with a comma, e.g., `/assets/ess/ess_1 '{ "maint_active_power_setpoint": 0}', /assets/ess/ess_1 '{ "maint_reactive_power_setpoint": 0}'`.
* The URI and key/value object for anything displayed in the UI can be determined by simply clicking in the display area for that URI/key/value *when the Inspector pages are visible*. For example, clicking on "Features/Active Power Management/Site Load Active Power" (click somewhere between the text and the numerical display) shows `/features/active_power '{"site_kW_load": 1398.7576904296875}'` in the browser's console. You can use this info to formulate a FIMS Send or a FIMS Listen.


-Desmond Mullen, 01/24/20