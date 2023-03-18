# Feeder Model

## Fields

| Struct Field | JSON Field   | Type          | Description                                                                        |
| ------------ | ------------ | ------------- | ---------------------------------------------------------------------------------- |
| ID           | id           | string        | ID to publish and receive commands as. Use this ID in power system tree            |
| Aliases      | aliases      | []string      | alternate IDs to mirror as (in case real asset is spread across multiple assets)   |
| I            | i            | float64       | 1ph AC current rms                                                                 |
| Di           | di           | float64       | 1ph active current rms                                                             |
| Qi           | qi           | float64       | 1ph reactive current rms                                                           |
| P            | p            | float64       | 3ph active power in kW                                                             |
| Q            | q            | float64       | 3ph reactive power in kVAR                                                         |
| S            | s            | float64       | 3ph apparent power in kVA                                                          |
| Pf           | pf           | float64       | Power factor, IEEE sign convention. P/Q/PF -> +/+/+, +/-/-, -/+/-, -/-/+           |
| F1           | f1           | float64       | Top of feeder AC frequency                                                         |
| F2           | f2           | float64       | Bottom of feeder AC frequency                                                      |
| Ph           | ph           | float64       | bus phase angle (future use)                                                       |
| Pmax         | pmax         | float64       | W trip threshold                                                                   |
| Qmax         | qmax         | float64       | VAR trip threshold                                                                 |
| Smax         | smax         | float64       | VA trip threshold                                                                  |
| V1           | v1           | float64       | Top of feeder voltage                                                              |
| V2           | v2           | float64       | Bottom of feeder voltage                                                           |
| V            | v            | float64       | Voltage sent down the tree                                                         |
| Polrev       | polrev       | bool          | Polarity convention, when false, power flowing up the tree to the grid is positive |
| Closed       | closed       | bool          | Closed status                                                                      |
| Closecmd     | closecmd     | bool          | Close command (must not be tripped)                                                |
| Opencmd      | opencmd      | bool          | Open command                                                                       |
| Resetcmd     | resetcmd     | bool          | Reset command (for trip)                                                           |
| Tripcmd      | tripcmd      | bool          | Trip command (from internal or external)                                           |
| Tripped      | tripped      | bool          | Tripped status                                                                     |
| Status       | status       | []bitfield    | bitfield status                                                                    |
| StatusCfg    | statuscfg    | []bitfieldcfg | list of configurations for status, see [Common Configuration](config.md))          |
| CtrlWord1Cfg | ctrlword1    | int           | Control word int typically configured for turning on and off                       |
| CtrlWord1    | ctrlword1cfg | []ctrlwordcfg | Control word configuration (see [Common Configuration](config.md))                 |
| Dactive      | N/A          | droop         | Active power (kW/Hz) droop settings, internal use only                             |
| Dreactive    | N/A          | droop         | Reactive power (kVAR/V) droop settings, internal use only                          |