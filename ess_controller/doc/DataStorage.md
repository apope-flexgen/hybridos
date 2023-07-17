###  Data Storage

The System Data Storage uses a number of applications to save run time data both on the target system and remotely.

```
storage - the original, javascript, fims to influx interface.
            - /usr/local/bin/storage /usr/local/etc/config/storage
FTD     - captures fims data and creates archive files
            - /usr/local/bin/ftd -c /usr/local/etc/config/ftd/config.json -s /usr/local/etc/config/storage/storage.json

```

## Storage

This it the older, original, fims data storage utility.
It populates a selected database (perhaps one or more) in the influx database. 
It is written in javascript, with some performance issues that impact the high volume data expected in future systems.

It is driven by a config file that serves to :

* setup the influx database
* direct the capture of incoming fims data from modbus or dnp3 periphals.

A typical config file

```
{
    "dbName": "hos_hist",
    "measurements": [
        {
            "baseUri": "/assets/ess",
            "measurement": "ess"
        },
        {
            "baseUri": "/assets/feeders",
            "measurement": "feeders"
        },
        {
            "source": ["ess_1","ess_2"],
            "baseUri": "/components",
            "measurement": "essComponents"
        },
        {
            "source": ["bms_1","bms_2","bms_3","bms_4"],
            "baseUri": "/components",
            "measurement": "bmsComponents"
        },
        {
            "source": "sel_351s",
            "baseUri": "/components",
            "measurement": "relay"
        },
        {
            "baseUri": "/components",
            "measurement": "components"
        },
        {
            "baseUri": "/site",
            "measurement": "site"
        },
        {
            "baseUri": "/features",
            "measurement": "features"
        },
        {
            "baseUri": "/metrics",
            "measurement": "metrics"
        },
        {
            "baseUri": "/storage",
            "measurement": null
        }
    ]
}
```

The influx database system must be running before the storage system is started.

```
influxd  (uses /etc/influxdb/influxdb.conf)
```

The storage system needs to have the influx database set up.
The /usr/local/bin/setup uses the same config file to set up the database and query / retention policies.
Note that these options are missing from the simple sample config file.
Once the basic database is setup , simply changing "dbName" in the config file causes a new database to be used.



```
/usr/local/bin/setup /usr/local/etc/config/storage/

```


Once the storage system is running the following fims messages are translated into database entries.

```
fims_send -m pub -u /components/bms_1 '{"test":1,"voltage":1345, "current":285}'

```

A simple test to see the effect of this query 

```
influx -database "hos_hist" -execute 'select * from bmsComponents'
name: bmsComponents
time               current source test voltage
----               ------- ------ ---- -------
164568423929100000 285     bms_1  1    1345

```

other options are available for the query.
```
influx -database "hos_hist" -execute 'select * from bmsComponents' -format "json" -pretty -precision rfc3339

influx -database "hos_hist" -execute 'select * from bmsComponents' -precision rfc3339
name: bmsComponents
time                     current source test voltage
----                     ------- ------ ---- -------
2022-02-24T06:30:39.291Z 285     bms_1  1    1345
```

This format will also work

```
fims_send -m pub -u /components/bms_1 '{"test":{"value":1},"voltage":1345, "current":285}'

```

The config options will dictate where the monitored data items will be placed in the database.

Influx uses "measurements" and "sources" to organize its data. 

A "measurement" is like a table, used for related data items.
A "source" is a component inside a "measurement" that provided data. 

The storage system has a number of query options.

```
fims_send -m get -r /$$  -u /storage/components/bms_1 
{ 
    "bms_1":{
        "test":{"value":1,"timestamp":"2022-02-24T06:30:39"},
        "voltage":{"value":1345,"timestamp":"2022-02-24T06:30:39"},
        "current":{"value":285,"timestamp":"2022-02-24T06:30:39"}
    }
}
```
fims_send -m get -r /$$  -u /storage/components/bms_1/voltage 
{ 
    "voltage":{"value":1345,"timestamp":"2022-02-24T06:30:39"}
}


Note that a bad query may cause storage to flag an error and restart.


## FTD Capture Fims Data


Raw fims data is captured by the ftd module.

This uses the following config file  saved in 

```
/usr/local/etc/config/ftd/config.json
```

```
{
    "period" : 50,
    "archive": "/home/hybridos/data",
    "uris": [
        {
            "uri": "/assets/ess",
            "type": "clothed",
            "group": "",
            "destination": "influx"
        },
        {
            "uri": "/components",
            "type": "naked",
            "group": "",
            "measurement": ""
        },
        {
            "uri": "/events",
            "type": "naked",
            "group": "",
            "destination": "mongo",
            "measurement": "events"
        }
    ]
}

```

A simple test of the events capture can be triggered like this

```
 fims_send -m pub -u /events '{"bms_rack_01_event":{"name":"BMS Rack 1","status":"Init","soc":58.6}}'

```

This produces a file

```
ls /home/hybridos/data/
kpp_test_events-events-1645602614.tar.gz

```

The files contain the following data 

```
events-1645602614/metadata.dat
/events
--OptDataStart
destination:mongo
measurement:events
database:kpp_test
--OptDataEnd
bms_rack_01_event       map[string]interface {}
time    uint64

```
```
hexdump -C events-1645602614/1645602614
00000000  00 01 42 ef fb d5 b7 dd  ff e0 00 05 d8 aa b6 f8  |..B.............|
00000010  2c 4d                                             |,M|
00000012
```



This is a trigger for the dts system to transfer the data to mongodb


The file in /home/hybridos/data dissappears  and mongodb has some new data.

```
mongo
> show databases
admin                    0.000GB
dbi                      0.000GB
hybridos_authentication  0.000GB
kpp_test                 0.000GB
local                    0.000GB

> use kpp_test
switched to db kpp_test

> show collections
events
> db.events.find()
{ "_id" : ObjectId("6215ea0b157d77b0d88af4c7"), "time" : NumberLong("1645602609310797") }
>
ctrl-d to quit.

A little test to find the data 

 python
Python 2.7.5 (default, Nov 16 2020, 22:23:17)
[GCC 4.8.5 20150623 (Red Hat 4.8.5-44)] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> hex(1645602609310797)
'0x5d8aab6f82c4d'
>>>

ctrl-d to quit

This matches the event data.
```

```
 Another event ... produces no data...
 fims_send -m post -u /events '{"source":"ess", "message":"ess starting at 0.00","severity":1}'

```



/usr/local/etc/config/storage/storage.json  | jq
```
```
{
  "dbName": "kpp_test",
  "measurements": [
    {
      "source": [
        "pcs_running_info",
        "pcs_parameter_setting"
      ],
      "baseUri": "/components",
      "measurement": "pcs_modbus_data"
    },
    {
      "source": [
        "summary"
      ],
      "baseUri": "/assets/pcs",
      "measurement": "pcs_assets_data"
    },
    {
      "source": [
        "bms_info",
        "bms_controls",
        "ems_running_info"
      ],
      "baseUri": "/components",
      "measurement": "bms_modbus_data"
    },
    {
      "source": [
        "hvac_info",
        "hvac_params"
      ],
      "baseUri": "/components",
      "measurement": "bms_hvac_modbus_data"
    },
    {
      "source": [
        "bms_rack_01_info",
        "bms_rack_02_info",
        "bms_rack_03_info",
        "bms_rack_04_info",
        "bms_rack_05_info",
        "bms_rack_06_info",
        "bms_rack_07_info",
        "bms_rack_08_info",
        "bms_rack_09_info",
        "bms_rack_10_info",
        "bms_rack_11_info",
        "bms_rack_12_info",
        "bms_rack_13_info",
        "bms_rack_14_info",
        "bms_rack_15_info",
        "bms_rack_16_info",
        "bms_rack_17_info",
        "bms_rack_18_info",
        "bms_rack_01_controls",
        "bms_rack_02_controls",
        "bms_rack_03_controls",
        "bms_rack_04_controls",
        "bms_rack_05_controls",
        "bms_rack_06_controls",
        "bms_rack_07_controls",
        "bms_rack_08_controls",
        "bms_rack_09_controls",
        "bms_rack_10_controls",
        "bms_rack_11_controls",
        "bms_rack_12_controls",
        "bms_rack_13_controls",
        "bms_rack_14_controls",
        "bms_rack_15_controls",
        "bms_rack_16_controls",
        "bms_rack_17_controls",
        "bms_rack_18_controls"
      ],
      "baseUri": "/components",
      "measurement": "bms_rack_modbus_data"
    },
    {
      "source": [
        "summary"
      ],
      "baseUri": "/assets/bms",
      "measurement": "bms_assets_data"
    },
    {
      "source": [
        "rack_01",
        "rack_02",
        "rack_03",
        "rack_04",
        "rack_05",
        "rack_06",
        "rack_07",
        "rack_08",
        "rack_09",
        "rack_10",
        "rack_11",
        "rack_12",
        "rack_13",
        "rack_14",
        "rack_15",
        "rack_16",
        "rack_17",
        "rack_18"
      ],
      "baseUri": "/assets/bms",
      "measurement": "bms_rack_assets_data"
    },
    {
      "baseUri": "/components",
      "measurement": "components"
    },
    {
      "baseUri": "/site",
      "measurement": "site"
    },
    {
      "baseUri": "/features",
      "measurement": "features"
    },
    {
      "baseUri": "/metrics",
      "measurement": "metrics"
    },
    {
      "baseUri": "/storage",
      "measurement": null
    }
  ],
  "retentionPolicies": [
    {
      "name": "autogen",
      "duration": "0s",
      "_duration": "INF",
      "shardGroupDuration": "168h0m0s",
      "replicaN": 1,
      "default": false
    },
    {
      "name": "infinite_rp",
      "duration": "0s",
      "_duration": "INF",
      "shardGroupDuration": "168h0m0s",
      "replicaN": 1,
      "default": false
    },
    {
      "name": "30_day_rp",
      "duration": "720h0m0s",
      "_duration": "30d",
      "shardGroupDuration": "24h0m0s",
      "replicaN": 1,
      "default": true
    },
    {
      "name": "120_day_rp",
      "duration": "2880h0m0s",
      "_duration": "120d",
      "shardGroupDuration": "24h0m0s",
      "replicaN": 1,
      "default": false
    }
  ],
  "continuousQueries": [
    {
      "name": "components_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "components",
      "from": "components",
      "groupBy": "time(1m), *"
    },
    {
      "name": "ess_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "ess",
      "from": "ess",
      "groupBy": "time(1m), *"
    },
    {
      "name": "metrics_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "metrics",
      "from": "metrics",
      "groupBy": "time(1m), *"
    },
    {
      "name": "site_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "site",
      "from": "site",
      "groupBy": "time(1m), *"
    },
    {
      "name": "features_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "features",
      "from": "features",
      "groupBy": "time(1m), *"
    },
    {
      "name": "pcs_modbus_data_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "pcs_modbus_data",
      "from": "pcs_modbus_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "bms_modbus_data_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "bms_modbus_data",
      "from": "bms_modbus_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "bms_hvac_modbus_data_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "bms_hvac_modbus_data",
      "from": "bms_hvac_modbus_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "bms_rack_modbus_data_1min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "120_day_rp",
      "measurement": "bms_rack_modbus_data",
      "from": "bms_rack_modbus_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "pcs_assets_data_1min_cq",
      "db": "brp_northfork",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "brp_northfork",
      "rp": "120_day_rp",
      "measurement": "pcs_assets_data",
      "from": "pcs_assets_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "bms_assets_data_1min_cq",
      "db": "brp_northfork",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "brp_northfork",
      "rp": "120_day_rp",
      "measurement": "bms_assets_data",
      "from": "bms_assets_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "bms_rack_assets_data_1min_cq",
      "db": "brp_northfork",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "brp_northfork",
      "rp": "120_day_rp",
      "measurement": "bms_rack_assets_data",
      "from": "bms_rack_assets_data",
      "groupBy": "time(1m), *"
    },
    {
      "name": "components_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "components",
      "from": "components",
      "groupBy": "time(10m), *"
    },
    {
      "name": "ess_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "ess",
      "from": "ess",
      "groupBy": "time(10m), *"
    },
    {
      "name": "metrics_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "metrics",
      "from": "metrics",
      "groupBy": "time(10m), *"
    },
    {
      "name": "site_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "site",
      "from": "site",
      "groupBy": "time(10m), *"
    },
    {
      "name": "features_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "features",
      "from": "features",
      "groupBy": "time(10m), *"
    },
    {
      "name": "pcs_modbus_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "pcs_modbus_data",
      "from": "pcs_modbus_data",
      "groupBy": "time(10m), *"
    },
    {
      "name": "bms_modbus_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "bms_modbus_data",
      "from": "bms_modbus_data",
      "groupBy": "time(10m), *"
    },
    {
      "name": "bms_hvac_modbus_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "bms_hvac_modbus_data",
      "from": "bms_hvac_modbus_data",
      "groupBy": "time(10m), *"
    },
    {
      "name": "bms_rack_modbus_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "bms_rack_modbus_data",
      "from": "bms_rack_modbus_data",
      "groupBy": "time(10m), *"
    },
    {
      "name": "pcs_assets_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "pcs_assets_data",
      "from": "pcs_assets_data",
      "groupBy": "time(10m), *"
    },
    {
      "name": "bms_assets_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "bms_assets_data",
      "from": "bms_assets_data",
      "groupBy": "time(10m), *"
    },
    {
      "name": "bms_rack_assets_data_10min_cq",
      "db": "sierra_lab_test",
      "select": "MIN(*), MAX(*), MEAN(*), LAST(*)",
      "into": "sierra_lab_test",
      "rp": "infinite_rp",
      "measurement": "bms_rack_assets_data",
      "from": "bms_rack_assets_data",
      "groupBy": "time(10m), *"
    }
  ]
}
```

In the absence of any other packages (DTS, DBI)running this 