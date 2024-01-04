# Load Model

## Fields

| Struct Field | JSON Field   | Type          | Description                                                                                                |
| ------------ | ------------ | ------------- | ---------------------------------------------------------------------------------------------------------- |
| ID           | id           | string        | ID to publish and receive commands as. Use this ID in power system tree                                    |
| Aliases      | aliases      | []string      | alternate IDs to mirror as (in case real asset is spread across multiple assets)                           |
| V            | v            | float64       | AC voltage                                                                                                 |
| I            | i            | float64       | 1ph AC current rms                                                                                         |
| Di           | di           | float64       | 1ph active current rms                                                                                     |
| Qi           | qi           | float64       | 1ph reactive current rms                                                                                   |
| P            | p            | float64       | 3ph active power in kW                                                                                     |
| Q            | q            | float64       | 3ph reactive power in kVAR                                                                                 |
| S            | s            | float64       | 3ph apparent power in kVA                                                                                  |
| Pf           | pf           | float64       | Power factor, IEEE sign convention. P/Q/PF -> +/+/+, +/-/-, -/+/-, -/-/+                                   |
| F            | f            | float64       | bus frequency                                                                                              |
| Ph           | ph           | float64       | bus phase angle (future use)                                                                               |
| On           | on           | bool          | On status                                                                                                  |
| Oncmd        | oncmd        | bool          | On command, momentary                                                                                      |
| Offcmd       | offcmd       | bool          | Off command, momentary                                                                                     |
| Pcmd         | pcmd         | float64       | Load command, kW negative is a load to keep with convention of other components                            |
| Qcmd         | qcmd         | float64       | Command sent to component, kVAR                                                                            |
| Pramp        | pramp        | float64       | active power ramp rate, kW/min                                                                             |
| Qramp        | qramp        | float64       | reactive power ramp rate, kVAR/min                                                                         |
| Noise        | noise        | float64       | One standard deviation of noise on kW/kVAR command following, less aggressive than RandomWalk              |
| RandomWalk   | randomwalk   | bool          | Enables RandomWalk mode. Uniform noise, ramp rate limited, is used for Pcmd and Qcmd rather than the input |
| Pmin         | pmin         | float64       | P min limit for RandomWalk                                                                                 |
| Pmax         | pmax         | float64       | P max limit for RandomWalk                                                                                 |
| Qmin         | qmin         | float64       | Q min limit for RandomWalk                                                                                 |
| Qmax         | qmax         | float64       | Q max limit for RandomWalk                                                                                 |
| Dactive      | dactive      | droop         | Active power (kW/Hz) droop settings (see [Common Configuration](config.md))                                |
| Dreactive    | dreactive    | droop         | Reactive power (kVAR/V) droop settings (see [Common Configuration](config.md))                             |
| CtrlWord1    | ctrlword1    | int           | Control word int typically configured for turning on and off                                               |
| CtrlWord1Cfg | ctrlword1cfg | []ctrlwordcfg | See explanation in [Common Configuration](config.md))                                                      |