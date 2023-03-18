# Energy Storage System Model

## Fields

| Struct Field        | JSON Field       | Type          | Description                                                                                                  |
| ------------------- | ---------------- | ------------- | ------------------------------------------------------------------------------------------------------------ |
| ID                  | id               | string        | ID to publish and receive commands as. Use this ID in power system tree                                      |
| Aliases             | aliases          | []string      | alternate IDs to mirror as (in case real asset is spread across multiple assets)                             |
| Cap                 | cap              | float64       | Capacity in kWh                                                                                              |
| V                   | v                | float64       | AC voltage                                                                                                   |
| I                   | i                | float64       | 1ph AC current rms                                                                                           |
| Di                  | di               | float64       | 1ph active current rms                                                                                       |
| Qi                  | qi               | float64       | 1ph reactive current rms                                                                                     |
| P                   | p                | float64       | 3ph active power in kW                                                                                       |
| Q                   | q                | float64       | 3ph reactive power in kVAR                                                                                   |
| S                   | s                | float64       | 3ph apparent power in kVA                                                                                    |
| Pf                  | pf               | float64       | Power factor, IEEE sign convention. P/Q/PF -> +/+/+, +/-/-, -/+/-, -/-/+                                     |
| F                   | f                | float64       | bus frequency                                                                                                |
| Ph                  | ph               | float64       | bus phase angle (future use)                                                                                 |
| On                  | on               | bool          | status if unit is on                                                                                         |
| Oncmd               | oncmd            | bool          | Command to turn unit on, overridden if control word configs write here                                       |
| Offcmd              | offcmd           | bool          | As above, but off                                                                                            |
| Standby             | standby          | bool          | Boolean status if ESS is in standby                                                                          |
| StandbyCmd          | standbycmd       | bool          | Command to enter standby, only enters from On state                                                          |
| ContactorControl    | kseparate        | bool          | Flag to require use of kac and kdc when turning unit on, otherwise ignore contactors altogether in operation |
| AcContactor         | kac              | bool          | status if AC contactor is closed                                                                             |
| AcContactorOpenCmd  | kacclosecmd      | bool          | Command to close AC contactor, overridden if control word configs write here                                 |
| AcContactorCloseCmd | kacopencmd       | bool          | As above, but open                                                                                           |
| DcContactor         | kdc              | bool          | status if DC contactor is closed                                                                             |
| DcContactorOpenCmd  | kdcclosecmd      | bool          | Command to close DC contactor, overridden if control word configs write here                                 |
| DcContactorCloseCmd | kdcopencmd       | bool          | As above, but open                                                                                           |
| Racks               | racks            | float64       | Number of racks the ESS has                                                                                  |
| RacksInService      | racksinservice   | float64       | Equals Racks when running                                                                                    |
| Pcmd                | pcmd             | float64       | Command sent to component                                                                                    |
| Phigh               | phigh            | float64       | Max discharge limit, must configure                                                                          |
| Pcharge             | pcharge          | float64       | Chargeable power feedback                                                                                    |
| Plow                | plow             | float64       | Max charge limit (given positive), must configure                                                            |
| Pdischarge          | pdischarge       | float64       | Dischargeable power feedback                                                                                 |
| Qcmd                | qcmd             | float64       | Command sent to component                                                                                    |
| Qhigh               | qhigh            | float64       | Max capacitive kVAR limit                                                                                    |
| Qlow                | qlow             | float64       | Max inductive kVAR limit                                                                                     |
| Soc                 | soc              | float64       | State of charge in 0-100%                                                                                    |
| Rte                 | rte              | float64       | Round trip efficiency of BESS at phigh power in 0-100%                                                       |
| pesr                | N/A              | float64       | Internal variable. Power normalized ESR factor                                                               |
| Idleloss            | idleloss         | float64       | No load loss of BESS in kW                                                                                   |
| IvCurve             | ivcurve          | float64       | Will be a list of (v,soc) points defining SOC curve                                                          |
| Idc                 | idc              | float64       | DC current                                                                                                   |
| Vdc                 | vdc              | float64       | DC voltage                                                                                                   |
| Vcmd                | vcmd             | float64       | AC voltage command for grid forming                                                                          |
| Fcmd                | fcmd             | float64       | AC frequency command for grid forming                                                                        |
| PfMode              | pfmode           | bool          | set true to follow pfcmd to set reactive power, ignores qcmd                                                 |
| PfCmd               | pfcmd            | float64       | Power factor command, limited to 0.1-1 positive or negative                                                  |
| Noise               | noise            | float64       | One standard deviation of noise on kW/kVAR command following                                                 |
| GridForming         | gridforming      | bool          | Status if the ESS is grid forming                                                                            |
| GridFormingCmd      | gridformingcmd   | bool          | Command to enter grid forming mode. Can be set with ESS on or off                                            |
| GridFollowingCmd    | gridfollowingcmd | bool          | Command to enter grid following mode. Can be set with ESS on or off                                          |
| Status              | status           | []bitfield    | bitfield status                                                                                              |
| StatusCfg           | statuscfg        | []bitfieldcfg | list of configurations for status, see [Common Configuration](config.md))                                    |
| CtrlWord1           | ctrlword1        | int           | Control word int typically configured for turning on and off                                                 |
| CtrlWord1Cfg        | ctrlword1cfg     | []ctrlwordcfg | Control word configuration (see [Common Configuration](config.md))                                           |
| CtrlWord2           | ctrlword2        | int           | Control word int typically configured for contactor control                                                  |
| CtrlWord2Cfg        | ctrlword2cfg     | []ctrlwordcfg | Control word configuration (see [Common Configuration](config.md))                                           |
| CtrlWord3           | ctrlword3        | int           | Control word int typically configured for pf mode                                                            |
| CtrlWord3Cfg        | ctrlword3cfg     | []ctrlwordcfg | Control word configuration (see [Common Configuration](config.md))                                           |
| CtrlWord4           | ctrlword4        | int           | Control word int typically configured for standby                                                            |
| CtrlWord4Cfg        | ctrlword4cfg     | []ctrlwordcfg | Control word configuration (see [Common Configuration](config.md))                                           |
| Dactive             | dactive          | droop         | Active power (kW/Hz) droop settings (see [Common Configuration](config.md))                                  |
| Dreactive           | dreactive        | droop         | Reactive power (kVAR/V) droop settings (see [Common Configuration](config.md))                               |
