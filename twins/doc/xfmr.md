# Transformer Model

Provides step up/step down transformers. This is a through component, so can have children in configuration.
Turns ratio, per unit impedance and efficiency can be specified. Voltage changes due to P and Q flowing through
the transformer are applied to the voltage passed down to children. No load and load losses are passed up to the
parent asset of the transformer. Line impedances can be modeled with the transformer with turns ratio set to 1.

Additional detail on the calculations of transformer impedance and losses can be found in [TwinsTransformerCalculations.pdf](TwinsTransformerCalculations.pdf)

## Fields
| Struct Field | JSON Field | Type          | Description                                                                           |
| ------------ | ---------- | ------------- | ------------------------------------------------------------------------------------- |
| ID           | id         | string        | Device id                                                                             |
| Aliases      | aliases    | []string      | Other ids the component should publish/listen as                                      |
| I            | i          | float64       | 1ph AC current rms                                                                    |
| Di           | di         | float64       | 1ph active current rms                                                                |
| Qi           | qi         | float64       | 1ph reactive current rms                                                              |
| P            | p          | float64       | 3ph active power in kW                                                                |
| Q            | q          | float64       | 3ph reactive power in kVAR                                                            |
| S            | s          | float64       | 3ph apparent power in kVA                                                             |
| F            | F          | float64       | AC bus frequency (same on both sides of transformer)                                  |
| Ph           | Ph         | float64       | AC bus phase (future use)                                                             |
| Sn           | Sn         | float64       | Rated kVA, used for impedance and loss calculations. If omitted, transformer is ideal |
| Vn           | Vn         | float64       | Rated V, top (parent) side                                                            |
| V1           | V1         | float64       | Top of feeder voltage                                                                 |
| V2           | V2         | float64       | Bottom of feeder voltage                                                              |
| V            | V          | float64       | Voltage sent down the tree                                                            |
| N            | N          | float64       | Turns ratio, top/bottom (parent side/children side). >1 is step down                  |
| Zpu          | Zpu        | float64       | Per unit impedance, in %, default 7%                                                  |
| XoR          | XoR        | float64       | Ratio of per unit reactance to per unit resistance, default 6                         |
| Eff          | Eff        | float64       | Efficiency at 50% load, default 99%                                                   |
| pnoload      | N/A        | float64       | Internal variable dealing with loss and impedance calculations                        |
| xpu          | N/A        | float64       | Internal variable dealing with loss and impedance calculations                        |
| rpu          | N/A        | float64       | Internal variable dealing with loss and impedance calculations                        |
| calcdrop     | N/A        | bool          | Internal variable dealing with loss and impedance calculations                        |
| calcloss     | N/A        | bool          | Internal variable dealing with loss and impedance calculations                        |
| Status       | Status     | []bitfield    | bitfield status                                                                       |
| StatusCfg    | StatusCfg  | []bitfieldcfg | List of configurations for status, see table in [Common Configuration](config.md))    |
| Dactive      | N/A        | droop         | Active power (kW/Hz) droop settings, internal use only                                |
| Dreactive    | N/A        | droop         | Reactive power (kVAR/V) droop settings, internal use only                             |
