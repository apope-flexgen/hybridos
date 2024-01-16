### Ess Controller v 10.1  Site COntroller Interface

This document accompanies the v10.1_04 training video

## Introduction

The site controller interface is a special case. Not wanting to duplicate effort the ess_controller sets 
itself up directly from the modbus_server.json.

This means that the ess_controller can keep in sync with the site controller data map.


A special parser is used to decode the site controller data map into the /site/ess_ls , /site/ess_ls tables.

This video demonstrates the test mode of loading the site interface.
The next video (05) goes into more details of the loader/template system. 

## Site Interface Loader definition

To accomdate this parser a special config line is used in the loader definition.

```
{ "site":"sierra_ess_server_tmpl",      "uri":"/config/ctmpl:sierra_ess_1_server_tmpl", "aname":"site", "pname":"ess"  }
```

When used in the load system the es_controller has details of the hierarchy of the data set.
In this case it is under an area (aname) called "site" which is a child of (pname) "ess".


## Site Interface Config Loader

Here is the config option in the loader context

```
"/config/load": {       
         "ess_controller": {
          "value":false,
          "type":"master",
          "file":"ess_controller",
          "aname":"ess",
          "final":"ess_final",
          "new_options":[
             { "load":"sierra_bms_manager",             "aname":"bms",  "pname":"ess", "summary":"bms/summary", "svec":"bms/bms_" },
             { "load":"sierra_bms_manager_modbus",      "aname":"bms",  "pname":"ess", "summary":"bms/summary", "svec":"bms/bms_" },
             { "file":"sierra_bms_controls",            "aname":"bms",  "pname":"ess", "summary":"bms/summary" },

             { "file":"sierra_pcs_manager",             "aname":"pcs",  "pname":"ess", "summary":"pcs/summary", "svec":"pcs/pcs_" },
             { "file":"siera_pcs_modbus_data",         "aname":"pcs",  "pname":"ess", "summary":"pcs/summary" },
             { "file":"siera_pcs_controls",            "aname":"pcs",  "pname":"ess", "summary":"pcs/summary" },

             { "site":"sierra_ess_server_tmpl",      "uri":"/config/ctmpl:sierra_ess_server_tmpl", "aname":"site", "pname":"ess"  }
          ]
        }
    }
```

## What is a template ??

A template is a file that is going to be processed by the system before bebg loaded into the system config.
Normally template files are processed with layered expaansion rules , using variable substitutions to create multiple instances of a config layout.
This process is disussed in the next video.

The whole file is read into a parameter called "body".
Then the whole file is processed by a special handler and inserted into the system datamap.

The file, in this case, is an exact copy of the same file used to configure the  modbus_server interface to the site controller. 
Change data in one and the other follows,


## Example Site Interface File

```
{
    "fileInfo": {
    },
    "system":
    {   "name":"FlexGen_ESS_Controller",
        "protocol": "Modbus TCP",
        "id": "/site",
        "ip_address": "0.0.0.0",
        "port": 502 
    },
    "registers": {
        "holding_registers": [
            {"id": "life", "offset": 1000, "size": 2, "name": "Life", "signed": true, "uri": "/site/ess_ls" },
            {"id": "start_stop", "offset": 1002, "name": "Control_Command", "uri": "/site/ess_ls" },
            {"id": "run_mode", "offset": 1003, "name": "Run_mode", "uri": "/site/ess_ls" },
            {"id": "on_off_grid_mode", "offset": 1004, "name": "On/Off grid mode setting", "uri": "/site/ess_ls" },
            {"id": "active_power_setpoint", "offset": 1005, "scale": 10, "name": "Power_Set_AC", "unit": "kW", "signed": true, "uri": "/site/ess_ls" },
            {"id": "reactive_power_setpoint", "offset": 1006, "scale": 10, "name": "Active_Set_AC", "unit": "kVar", "signed": true, "uri": "/site/ess_ls" },
            {"id": "active_power_rising_slope", "offset": 1007, "scale": 10, "name": "Active_power_rising_slope", "unit": "%/s", "uri": "/site/ess_ls" },
            {"id": "active_power_droop_slope", "offset": 1008, "scale": 10, "name": "Active_power_droop_slope", "unit": "%/s", "uri": "/site/ess_ls" },
            {"id": "reactive_power_rising_slope", "offset": 1009, "scale": 10, "name": "Reactive_power_rising_slope", "unit": "%/s", "uri": "/site/ess_ls" },
            {"id": "reactive_power_droop_slope", "offset": 1010, "scale": 10, "name": "Reactive_power_droop_slope", "unit": "%/s", "uri": "/site/ess_ls" },
            {"id": "bms_dc_contactors", "offset": 1011, "name": "BMS DC Contactors", "uri": "/site/ess_ls"},
            {"id": "clear_faults", "offset": 1012, "name": "Clear_Faults_Command", "uri": "/site/ess_ls", "individual_bits": true, "bit_strings": ["clear_faults"]}
        ]
    }
}
 ```

## Ugly code extract 


Here is a segment of the core cocde used to process this file.
It is useful to help understand how the system works...


```
if(cjsite)
            {
                // get the uri, the av and run 
                cJSON* cja = cJSON_GetObjectItem(cji, "uri");
                if(cja && cja->valuestring)
                {
                    assetVar* aV = vm->getVar(vmap, cja->valuestring, nullptr);
                    if(aV) loadSiteMapAv(vmap, vm, aV);
                }

            }

```

So we simply take the "id" and the "uri" fields and load them into the  system data map.

More code details... 

```
    //For each item in the register map
    cjid = cJSON_GetObjectItem(cji, "id");
    cjuri = cJSON_GetObjectItem(cji, "uri");
    if(cjid && cjuri && cjid->valuestring&&cjuri->valuestring)
    {
        if(0) FPS_PRINT_INFO("found input register id [{}] uri [{}]", cjid->valuestring, cjuri->valuestring);
        int ival = 0;
        vm->setVal(vmap, cjuri->valuestring, cjid->valuestring, ival);
    }
```

The result, for the above example, would look like this:

```
{
    "/site/ess_ls": {
        "life":0,
        "start_stop":0, 
        "run_mode":0,
        "on_off_grid_mode":0,
        "active_power_setpoint":0,
        "reactive_power_setpoint":0,
        "active_power_rising_slope":0,
        "active_power_droop_slope":0,
        "reactive_power_rising_slope":0,
        "reactive_power_droop_slope":0,
        "bms_dc_contactors":0,
        "clear_faults":0            
    }
}
```


## Testing the Server Load operation

For this example we'll not use the full loader ( until the next video)

```
   command line.
echo setup LoadServer command
fims_send -m set -r /$$ -u /ess/system/commands '
         {"loadServer":{"value":"test",
                "help": "load a Modbus Server interface",
                "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadServer"}]}]}}}'

```
##  Set up the system/command 

```
    "/system/commands":{
        "loadServer":{
            "value":"test",
            "help": "load a Modbus Server interface",
            "ifChanged":false,
            "enabled":true, 
            "actions":{
                "onSet":[{"func":[{"func":"LoadServer"}]}]}}
    }
```

## Example : load a server 

```
fims_send -m set -r /$$ -u /ess/full/system/commands/loadServer '
             {"value":"test","server":"demo_modbus_server.json"}'
```


## Test result !!

```
fims_send -m get -r /$$ -u /ess/naked/site/ess_ls | jq
{
  "/site/ess_ls": {
    "LsVal1": 123,
    "LsVal2": 4456,
    "active_power_droop_slope": 0,
    "active_power_rising_slope": 0,
    "active_power_setpoint": 0,
    "bms_dc_contactors": 0,
    "chargeable_power": 0,
    "clear_faults": 0,
    "dischargeable_power": 0,
    "life": 0,
    "on_off_grid_mode": 0,
    "reactive_power_droop_slope": 0,
    "reactive_power_rising_slope": 0,
    "reactive_power_setpoint": 0,
    "run_mode": 0,
    "start_stop": 0
  }
}
```

### Conclusion

This part of the loader system demonstrates an easy way to get the site controller interface set up with the ess_controller.
Of course all these variables will need to be populated using the  actions and functions we have already discussed.


Next ... the loader and templates.

