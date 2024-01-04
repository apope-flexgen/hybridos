# Grid Model

There should always be a `grid` component at the top of a the configured power
system tree. It does not take commands, but absorbs the balance of P and Q
at the end of a simulation tick.

## Fields

| Struct Field      | JSON Field | Type     | Description                                                                                                     |
| ----------------- | ---------- | -------- | --------------------------------------------------------------------------------------------------------------- |
| ID                | id         | string   | ID to publish and receive commands as. Use this ID in power system tree                                         |
| Aliases           | aliases    | []string | alternate IDs to mirror as (in case real asset is spread across multiple assets)                                |
| I                 | i          | float64  | 1ph AC current rms                                                                                              |
| Di                | di         | float64  | 1ph active current rms                                                                                          |
| Qi                | qi         | float64  | 1ph reactive current rms                                                                                        |
| P                 | p          | float64  | 3ph active power in kW                                                                                          |
| Q                 | q          | float64  | 3ph reactive power in kVAR                                                                                      |
| S                 | s          | float64  | 3ph apparent power in kVA                                                                                       |
| Pf                | pf         | float64  | Power factor, IEEE sign convention. P/Q/PF -> +/+/+, +/-/-, -/+/-, -/-/+                                        |
| Vcmd              | vcmd       | float64  | AC voltage from grid                                                                                            |
| Fcmd              | fcmd       | float64  | AC frequency from grid                                                                                          |
| V                 | v          | float64  | AC voltage stored after droop                                                                                   |
| F                 | f          | float64  | bus frequency stored after droop                                                                                |
| Ph                | ph         | float64  | bus phase angle (future use)                                                                                    |
| Dactive           | dactive    | droop    | Active power (kW/Hz) droop settings (see [Common Configuration](config.md)), should be stiff, may be omitted    |
| Dreactive         | dreactive  | droop    | Reactive power (kVAR/V) droop settings (see [Common Configuration](config.md)), should be stiff, may be omitted |
| DactiveExternal   | N/A        | droop    | Used for internal calculations                                                                                  |
| DreactiveExternal | N/A        | droop    | Used for internal calculations                                                                                  |