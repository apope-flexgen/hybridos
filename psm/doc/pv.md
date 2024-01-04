# Photovoltaic solar System Model

## Fields

| Struct Field | JSON Field   | Type          | Description                                                                                                              |
| ------------ | ------------ | ------------- | ------------------------------------------------------------------------------------------------------------------------ |
| ID           | id           | string        | ID to publish and receive commands as. Use this ID in power system tree                                                  |
| Aliases      | aliases      | []string      | alternate IDs to mirror as (in case real asset is spread across multiple assets)                                         |
| V            | v            | float64       | AC voltage                                                                                                               |
| I            | i            | float64       | 1ph AC current rms                                                                                                       |
| Di           | di           | float64       | 1ph active current rms                                                                                                   |
| Qi           | qi           | float64       | 1ph reactive current rms                                                                                                 |
| P            | p            | float64       | 3ph active power in kW                                                                                                   |
| Q            | q            | float64       | 3ph reactive power in kVAR                                                                                               |
| S            | s            | float64       | 3ph apparent power in kVA                                                                                                |
| Pf           | pf           | float64       | Power factor, IEEE sign convention. P/Q/PF -> +/+/+, +/-/-, -/+/-, -/-/+                                                 |
| F            | f            | float64       | bus frequency                                                                                                            |
| Ph           | ph           | float64       | bus phase angle (future use)                                                                                             |
| On           | on           | bool          | On status                                                                                                                |
| Oncmd        | oncmd        | bool          | On command, momentary                                                                                                    |
| Offcmd       | offcmd       | bool          | Off command, momentary                                                                                                   |
| Pcmd         | pcmd         | float64       | Use for DC solar production in kW                                                                                        |
| Plim         | plim         | float64       | Power limit setpoint, will limit output to range min(Phigh,Plim,Pcmd)). May be given as % of Phigh if PercentCmd is true |
| Phigh        | phigh        | float64       | Max output capability of inverter in kW, must configure                                                                  |
| Qcmd         | qcmd         | float64       | Command sent to component, May be given as % of Phigh if PercentCmd is true, ignored if PfMode is true                   |
| Pramp        | pramp        | float64       | active power ramp rate, kW/min                                                                                           |
| Qramp        | qramp        | float64       | reactive power ramp rate, kW/min                                                                                         |
| PercentCmd   | percentcmd   | bool          | If true, accepts Plim and Qcmd as % of Phigh                                                                             |
| PfMode       | pfmode       | bool          | set true to follow pfcmd to set reactive power, ignores qcmd                                                             |
| Noise        | noise        | float64       | One standard deviation of noise on kW/kVAR command following                                                             |
| PfCmd        | pfcmd        | float64       | Power factor command, limited to 0.1-1 positive or negative, applies after all transforms on plim                        |
| Status       | status       | []bitfield    | bitfield status                                                                                                          |
| StatusCfg    | statuscfg    | []bitfieldcfg | list of configurations for status, see [Common Configuration](config.md))                                                |
| CtrlWord1    | ctrlword1    | int           | Control word int typically configured for turning on and off                                                             |
| CtrlWord1Cfg | ctrlword1cfg | []ctrlwordcfg | See explanation in [Common Configuration](config.md))                                                                    |
| CtrlWord2    | ctrlword2    | int           | Control word int typically configured for PF mode                                                                        |
| CtrlWord2Cfg | ctrlword2cfg | []ctrlwordcfg | See explanation in [Common Configuration](config.md))                                                                    |
| Dactive      | dactive      | droop         | Active power (kW/Hz) droop settings (see [Common Configuration](config.md))                                              |
| Dreactive    | dreactive    | droop         | Reactive power (kVAR/V) droop settings (see [Common Configuration](config.md))                                           |
