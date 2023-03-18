# Metrics Overview and Quickstart

This document is the overview and quick start guide for writing a metrics config.

## Operators

* Extents - max, min
* Aggregates - integrate
* Math - sum, product, average, rss
* Bit Manipulation- and, or, enum, bitfield, count, bitfieldpositioncount
* Logic - srff, compare, compareand, compareor
* Utility - echo, select
* Special - runtime, unicompare, pulse, currentTimeMilliseconds, compareMillisecondsToCurrentTime, millisecondsToRFC3339, combineInputsToArray

Metrics are stored until overwritten (extents, math, bitwise, single input) or updated
(aggregates). Metrics can take its data from multiple sources on FIMS and can also
use other metrics as inputs to pipeline complex calculations

## FIMS interface

* listen - This module listens for pubs from data sources it cares about
* pub - metrics are published at the assigned rate
* get - other modules can 'get' against the same URIs that are pubbed
* set - other modules can 'set' in data sources
* del - some metrics with state can be reset

## Configuration

Besides setting the operation described above, one must set `param`s for some
of the operations.

Additionally, there is a `naked` flag for when you want to only pub the
normal contents of `value` as the value behind the metric id.

The `inputs` array defines the FIMS URIs that metrics listens to and pulls in 
as in inputs for the metric operations. Some metrics have defined input order,
others not. Refer to [the operator APIs](doc/operators.md) for more.

Normally, all metrics are recomputed at the `publishRate`. If the `outputs`
array is present on any metric, that metric will recompute as soon as an
input is received from FIMS and the new `value` will be `set` out to
the FIMS end points defined in the array if it is different from the
last value `set`. If you want to `set` out to the FIMS end points
*regardless* of whether the new value equals the last value you can set
`param.setOutputOnUpdate` to true. If `param.setOutputOnUpdate` is false or
is not present, the `set` will only happen if the new value is different.
Note that `param.setOutputOnUpdate` will set to the outputs at *every*
publish interval, regardless of whether its inputs are active.

`initialInput` is used to initialize metrics like `echo` to ensure data type 
on launch. The value supplied for `initialInput` will be applied to all of 
the input uris supplied in the configuration.

There is a small number of metrics for which it makes sense to initialize 
their value, in which case an `initialValue` property can be set. For most
operations, this will be immediately overwritten on the next publish cycle,
but some operations like `srff` have a *keep* state that can make use of this.

```javascript
{
    "publishRate": 1000, // in ms, how often to publish all metrics
    "listenRate": 2, // in ms, how often to pull FIMS message
    "publishUris":[ // list of objects defining each of the uris
                    // that metrics publishes to and the associated metrics
        {   
            "uri": "/metrics/kpi", // baseURI of publish
            "metrics":[
                {
                    "id": "system_sum_diff_cell_temp", // appended to baseURI
                    "name": "System Sum/Diff Cell Temperature",
                    "scale": 1, // name, scale, unit, ui_type, type and 
                    "unit": "°C", // options are for the UI, if needed
                    "ui_type": "status", // if left off, they don't publish
                    "type": "number",
                    "options": [],
                    "inputs" : [ // list of uri/id pairs that make up 
                                 // the inputs to the operation. Some
                                 // operations are sensitive to order
                        {"uri":"/components/ess_1","id":"bms1_max_cell_temp"},
                        {"uri":"/components/ess_1","id":"bms2_max_cell_temp"},
                        {"uri":"/components/ess_1","id":"bms3_max_cell_temp"},
                        {"uri":"/components/ess_1","id":"bms4_max_cell_temp"}
                    ],
                    "operation":"sum", // refer to operations list
                    "param":{"operations":"+-+-"}
                    // the operations API list will define what parameters,
                    // if any, may be used for a particular operation
                }
            ]
        },
        {
            "uri": "/components/dio_echo",
            "naked": true, // if you want to publish naked, do so here
            "metrics": [
                {
                    "id": "di1",
                    "name": "Digital Input 1",
                    // unit and other UI should be left off when naked
                    "inputs": [ 
                        {"uri":"/components/ed588","id":"di1"} 
                    ],
                    // some operations allow their stored value to be
                    // `set` to another URI, all uri/id pairs in this list
                    // will get set with the same value
                    "outputs": [
                        {"uri":"/site/operation","id":"spareinput"}
                    ]
                }
            ]
        }
    ]
}
```