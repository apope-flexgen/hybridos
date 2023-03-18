# Generator Model

## Fields

| Struct Field     | JSON Field       | Type          | Description                                                                      |
| ---------------- | ---------------- | ------------- | -------------------------------------------------------------------------------- |
| ID               | id               | string        | ID to publish and receive commands as. Use this ID in power system tree          |
| Aliases          | aliases          | []string      | alternate IDs to mirror as (in case real asset is spread across multiple assets) |
| V                | v                | float64       | AC voltage                                                                       |
| I                | i                | float64       | 1ph AC current rms                                                               |
| Di               | di               | float64       | 1ph active current rms                                                           |
| Qi               | qi               | float64       | 1ph reactive current rms                                                         |
| P                | p                | float64       | 3ph active power in kW                                                           |
| Q                | q                | float64       | 3ph reactive power in kVAR                                                       |
| S                | s                | float64       | 3ph apparent power in kVA                                                        |
| Pf               | pf               | float64       | Power factor, IEEE sign convention. P/Q/PF -> +/+/+, +/-/-, -/+/-, -/-/+         |
| F                | f                | float64       | bus frequency                                                                    |
| Ph               | ph               | float64       | bus phase angle (future use)                                                     |
| On               | on               | bool          | status if unit is on                                                             |
| Oncmd            | oncmd            | bool          | Command to turn unit on, overridden if control word configs write here           |
| Offcmd           | offcmd           | bool          | As above, but off                                                                |
| Pcmd             | pcmd             | float64       | Command sent to component, kW                                                    |
| Qcmd             | qcmd             | float64       | Command sent to component, kVAR                                                  |
| Pramp            | pramp            | float64       | active power ramp rate, kW/min                                                   |
| Qramp            | qramp            | float64       | reactive power ramp rate, kW/min                                                 |
| PfMode           | pfmode           | bool          | set true to follow pfcmd to set reactive power, ignores qcmd                     |
| PfCmd            | pfcmd            | float64       | Power factor command, limited to 0.1-1 positive or negative                      |
| Noise            | noise            | float64       | One standard deviation of noise on kW/kVAR command following                     |
| Vcmd             | vcmd             | float64       | AC voltage command for grid forming                                              |
| Fcmd             | fcmd             | float64       | AC frequency command for grid forming                                            |
| GridForming      | gridforming      | bool          | Status if the Generator is grid forming                                          |
| GridFormingCmd   | gridformingcmd   | bool          | Command to enter grid forming mode. Can be set with Generator on or off          |
| GridFollowingCmd | gridfollowingcmd | bool          | Command to enter grid following mode. Can be set with Generator on or off        |
| Status           | status           | []bitfield    | bitfield status                                                                  |
| StatusCfg        | statuscfg        | []bitfieldcfg | list of configurations for status, see [Common Configuration](config.md))        |
| CtrlWord1Cfg     | ctrlword1        | int           | Control word int typically configured for turning on and off                     |
| CtrlWord1        | ctrlword1cfg     | []ctrlwordcfg | See explanation in [Common Configuration](config.md))                            |
| CtrlWord2Cfg     | ctrlword2        | int           | Control word int typically configured for PF mode                                |
| CtrlWord2        | ctrlword2cfg     | []ctrlwordcfg | See explanation in [Common Configuration](config.md))                            |
| Dactive          | dactive          | droop         | Active power (kW/Hz) droop settings (see [Common Configuration](config.md))      |
| Dreactive        | dreactive        | droop         | Reactive power (kVAR/V) droop settings (see [Common Configuration](config.md))   |
