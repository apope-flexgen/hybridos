dnp3_master - config.json Documentation

Author: Phil Wishire

Date Created: 06/23/2020

Date Modified: 06/23/2020


# example config.json file:

<pre><code>
{

    char* tmp_uri = NULL;
    if(ret) ret = getCJint(cj,"version",         sys->version,        false );
    if(ret) ret = getCJint(cj,"frequency",       sys->frequency,      false);
    if(ret) ret = getCJint(cj,"freq1",           sys->freq1,          false);
    if(ret) ret = getCJint(cj,"freq2",           sys->freq2,          false);
    if(ret) ret = getCJint(cj,"freq3",           sys->freq3,          false);
    if(ret) ret = getCJint(cj,"port",            sys->port,           true);
    if(ret) ret = getCJint(cj,"local_address",   sys->local_address,  false);
    if(ret) ret = getCJint(cj,"remote_address",  sys->remote_address, false);
    if(ret) ret = getCJstr(cj,"id",              sys->id,             true);
    if(ret) ret = getCJstr(cj,"protocol",        sys->protocol,       false);
    if(ret) ret = getCJstr(cj,"ip_address",      sys->ip_address,     true);
    //if(ret) ret = getCJstr(cj,"pub",             sys->pub,            false);
    if(ret) ret = getCJstr(cj,"name",            sys->name,           false);
    if(ret) ret = getCJint(cj,"debug",           sys->debug,          false);
    //TODO use this
    if(ret) ret = getCJstr(cj,"base_uri",         tmp_uri,            false);
    if(ret) ret = getCJstr(cj,"local_uri",        sys->local_uri,     false);
    if(ret) ret = getCJbool(cj,"unsol",           sys->munsol,        false);

  "system": {
        "id": "fm_dnp3_master",
        "name": "fm_dnp3_master",
        "protocol": "DNP3",
        "version": "0001",
        "ip_address": "10.0.2.20",
        "port": 20001,
        "frequency":1000,
        "freq1":1000,
        "freq2":1000,
        "freq3":1000,
        "local_address":1,
        "remote_address":10,
        "local_uri": "/fm_master_local",
        "base_uri": "/components",
        "debug": 0,
        "unsol": 0
  },
  "objects": {
        "analog": [
            {
                "id": "sel_735_active_power",
                "offset": 0,
                "name": "Sel 735 Active Power",
                "unit": "MW",
		        "variation":"Group30Var5",
                "xxclazz":1,
		        "evariation":"Group32Var7",
                "uri":"/metrics/misc",
                "xxsigned": true


            },
            {
                "id": "sel_735_reactive_power",
                "offset": 1,
                "name": "Sel 735 Reactive Power",
                "unit": "MW",
		        "variation":"Group30Var5",
                "xxclazz":1,
		        "evariation":"Group32Var7",
                "uri":"/metrics/misc",
                "xxsigned": true

            },
            
                    ],

        "binary": [
            {
                "id": "voltage_regulator",
                "offset": 0,
                "name": "Voltage Regulator"
            },
            {
                "id": "power_system_stabilizer",
                "offset": 1,
                "name": "Power System Stabilizer"
            },
            {
                "id": "gen_resource_switches",
                "offset": 2,
                "name": "Generation Resource Switches"
            },
            {
                "id": "gen_resource_breaker_status",
                "offset": 3,
                "name": "Generation Resource Breaker Status"
            },
            {
                "id": "gen_virtual_breaker_status",
                "offset": 4,
                "name": "Gen Virtual Breaker Status"
            }
        ],
        
        "AnOPInt32": [
            {
                "id": "TestOPtoSite",
                "offset": 0,
                "name": "Test OUTPut To Site",
                "unit": "MW",
                "signed": true,
                "uri": "/testing/testop"

            }
        ],
        "CROB": [
            {
                "id": "testCROB_4",
                "offset": 4,
                "name": "testCROB_4",
                "unit": "None",
                "signed": true
            }
       ]

    }
}
	
</code></pre>


# descriptions:

* "objects" - name of the highest-level JSON object and cannot be modified
	* "analog" - One of the input types from the outststion. format determined by the outstation config
	* "binary" - One of the input types from the outststion. values true or false
  * "
		* "name" - string name for the variable.  may be displayed on the UI
		* "unit" - string name for the variables units of measure.  may be displayed on the UI
		* "ui_type" Determines what type of variable format to use when displaying on the UI.  options are:
			* "status" - default.  variable will display on the UI as a status variable.
			* "control" - variable will will display on the UI as a control variable.
			* "fault" - reserved for "active_faults".
			* "alarm" - reserved for "active_alarms".
			* "none" - variable will not display on the UI.
		* "type" - TBD
		* "var_type" - determines what type of variable format to use in Site Manager.  options are:
			"Float" - default. 32-bit float
			"Int"  - 32-bit integer
			"Bool" - boolean
			"String" - string (dynamically allocated char*).
		* "value" - sets the default initial value for the variable, corresponding to the "var_type"
		* "scaler" - declares the scale of the value WRT the "unit" 
		* "enabled" - for controls, sets whether the control is enabled or disabled on the UI.
		* "num_options" - indicates how many entries are used in the "options" array.  use 0 for empty "options" array.
		* "options" - used when additional information is needed about the variable.  e.g. individual fault names for "active_faults" and button state names for boolean controls.
			* "name" - string name associated with the corresponding "value".
			* "value" - value associated with the corresponding "name".  
	

# appendix: 

* "defaults" - this variable contains all default values for each JSON object parameter for reference.  if a variable has the default value, it is not necessary to list that parameter for that variable.
