# Common Configuration

Each of the components that can be modeled are described in their individual
documentation. In each doc you will find a table with the following columns

* *Struct Field* - the name of the field as found in the source code
* *JSON Field* - the lower case name (if available) to use for the field in configuration
* *Type* - the datatype, bool, int, float64, string, list of strings, or the custom types below
* *Description* - a hopefully thorough description of how to use the field

*Struct Field*s that begin with an upper case letter are available to be
configured via JSON, and will publish at the pub interval for each component,
with the exception of `ID`, `Aliases`, `StatusCfg`, `CtrlWord1Cfg`,
`CtrlWord2Cfg`, `CtrlWord3Cfg`, `CtrlWord4Cfg`, `Dactive` and `Dreactive`,
(see `config.go`) as their structure doesn't conform to FIMS standards. 
*Struct Fields*s that begin with a lower case letter are not available for
configuration via JSON, calculated as an internal variable from others
provided.

If one of the *JSON Fields* appears as a `"twins_id"` in `assets.json`, it
is replaced in FIMS interactions with the field name from `assets.json`.
Every field that is published on FIMS can be `set` directly at runtime,
which may be useful to re-initialize a test or skip to getting a battery
fully charged. *JSON Fields* that are included in Control Word Configurations,
details below, still accept direct FIMS sets, but will be overridden by the
control word.

## Component Specific Documentation

* [ESS](ess.md)
* [Generators](gen.md)
* [Grids](grid.md)
* [Feeders](feed.md) 
* [Loads](load.md)
* [Solar](pv.md)
* [Transformers](xfmr.md)

## Control Word Configuration

Configuring the control word comes in the the form below. The first list of
objects defines the integer `value`s of the control word accepted. The
`controls` list under that defines which `field` (based on the table above)
should be set when the control word is received, and the `value` to set it to.
Control words can currently only act on `bool`s. `ctrlword1` and `ctrlword2`
can act on the same fields; `ctrlword2` will take precedence. If control word
configs are not set, or they don't touch certain controls, like if neither
touch `accontactoropencmd` or `accontactorclosecmd`, those controls can
still be `set` directly with no interference from the control word.

### Example control word configuration

```json
"ctrlword1cfg":[
    {
        "value": 1,
        "controls": [
            { "field": "closecmd", "value": true },
            { "field": "opencmd", "value": false }
        ]
    },
    {
        "value": 0,
        "controls": [
            { "field": "opencmd", "value": true },
            { "field": "closecmd", "value": false }
        ]
    }
]
```

## Status Configuration

The `StatusCfg` field takes a list of status configuration objects which map 
boolean fields of the assets to bitfield status. Multiple status bitfields may be 
present simultaneously.
 
| Field  | Description                                            |
| ------ | ------------------------------------------------------ |
| value  | Number for bit position                                |
| string | String emitted with the status                         |
| field  | Name of the twins field enable this status (lowercase) |
| invert | Optional to emit the status when the field is false    |

### Example status configuration

```json
"statuscfg": [
    {
        "value":4,
        "string":"Running",
        "field":"on"
    },
    {
        "value":7,
        "string":"Stopped",
        "field":"on",
        "invert":true
    },
    {
        "value":6,
        "string":"Forming the grid",
        "field":"gridforming"
    }
]
```

## Droop Configuration

Both active (kW/Hz) and reactive (kVAR/V) droop are configured with typical
droop percentage settings. Since the droop functions are generic, keep in
mind that the Y axis (ynom below) is the P or Q rating of the device, and
the X axis is the Hz or V rating of the device.

```json
"dactive":{
    "ynom": 1100,
    "comment": "ynom is rated active power",
    "xnom": 60,
    "comment": "xnom is rated frequency",
    "percent": 0.05
},
```