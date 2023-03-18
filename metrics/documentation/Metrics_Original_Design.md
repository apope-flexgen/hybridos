# Metrics Original Design

This is a record of the original design notes. The implementation has strayed from this, but this should give some insight to how it arose

## Thoughts

There will be a data object that stores the metrics. This is the master data structure. It serves as the data
source for pubs and get replies. Other data structures will map into this one to accept gets, or receive sets
and pubs.

```javascript
{
    "pubId1":{ // who does this publish as
        "metricId1":{
            "value":val, // normal publish stuff
            "name":name,
            "scale":1,
            "unit":unit,
            "ui_type":type, // next three are for UI(?), copy implementation of other modules
            "type":type,
            "options":list,
            "state":object, // may be needed for some aggregations
            "inputs":list, // inputs for the metric, so they may be buffered for cross-source
            "operation":op // operation run at every update
        },
        ...
    }
    ...
}
```

Listening for pub/set

```javascript
{
    "sourceUri1":{
        "sourceId1":[
            ref1, // reference into master data structure [metricId].inputs[x]
            ref2
        ],
        "sourceId2":[]
    },
    "sourceUri2":{}
}
```

## Examples

### Min/Max

We have a battery system reporting on `/components/ess_1` that has four readings from individual battery
groups for each of min/max cell temperature and voltage. We would like to aggregate these each into
system level min/max readings.

#### Config file

```javascript
{
    "publishRate": 1000, // 1s
    "metrics":[
        {
            "id":"system_max_cell_temp",
            "name":"System Maximum Cell Temperature",
            "scale":1,
            "unit":"°C",
            "ui_type":"status", // next three are for UI(?), copy implementation of other modules
            "type":"number",
            "options":[],
            "inputs" : [
                {"uri":"/components/ess_1","id":"bms1_max_cell_temp"},
                {"uri":"/components/ess_1","id":"bms2_max_cell_temp"},
                {"uri":"/components/ess_1","id":"bms3_max_cell_temp"},
                {"uri":"/components/ess_1","id":"bms4_max_cell_temp"}
            ],
            "operation":"max"
        }
    ]
}
```

#### Master data object

```javascript
let mdo = {
    "/metrics/kpi": {
        "system_max_cell_temp":{
            "value":27.4,
            "name":"System Maximum Cell Temperature",
            "scale":1,
            "unit":"°C",
            "ui_type":"status",
            "type":"number",
            "options":[],
            "inputs" : {
                "/components/ess_1/bms1_max_cell_temp":27.4,
                "/components/ess_1/bms2_max_cell_temp":26.4,
                "/components/ess_1/bms3_max_cell_temp":26.9,
                "/components/ess_1/bms4_max_cell_temp":27.4
            },
            "operation":"max"
        },
        "system_max_cell_temp" :{}
    },
    "/components/11_f5_sel351s": { // spoof as another module
        "fault_word":{}
    },
    ...
}
```

#### pub/set map

```javascript
let psmap = {
    "/components/ess_1":{
        "bms1_max_cell_temp":[ { uri:"/metrics/kpi", id:"system_max_cell_temp"} ],
        "bms2_max_cell_temp":[ { uri:"/metrics/kpi", id:"system_max_cell_temp"} ],
        "bms3_max_cell_temp":[ { uri:"/metrics/kpi", id:"system_max_cell_temp"} ],
        "bms4_max_cell_temp":[ { uri:"/metrics/kpi", id:"system_max_cell_temp"} ]
    }
}
```