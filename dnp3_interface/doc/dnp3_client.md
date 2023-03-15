dnp3_client - config.json Documentation

Author: Phil Wishire

Date Created: 06/23/2020

Date Modified: 06/23/2020

Date Modified: 07/15/2020
      added deadand

# example config.json file:

<pre><code>
{

    if(ret) ret = getCJint(cj,"version",         sys->version,        false );
                  legacy value not used

    if(ret) ret = getCJint(cj,"frequency",       sys->frequency,      false);
		  set the frequency for a class 0 (static) scan

    if(ret) ret = getCJint(cj,"freq1",           sys->freq1,          false);
		  set the frequency for a class 1 (exception) scan

    if(ret) ret = getCJint(cj,"freq2",           sys->freq2,          false);
		  set the frequency for a class 2 (exception) scan

    if(ret) ret = getCJint(cj,"freq3",           sys->freq3,          false);
		  set the frequency for a class 3 (exception) scan

    if(ret) ret = getCJint(cj,"rangefreq",       sys->rangeFreq,       false);
		  set the frequency for ranged  (exception) scan

    if(ret) ret = getCJint(cj,"rangeAStart",     sys->rangeAStart,     false);
		  set the analog range start index for a  ranged  (exception) scan

    if(ret) ret = getCJint(cj,"rangeAStop",      sys->rangeAStop,      false);
		  set the analog range end index for a  ranged (exception) scan

    if(ret) ret = getCJint(cj,"rangeBStart",     sys->rangeBStart,     false);
		  set the binary range start index for a  ranged (exception) scan

    if(ret) ret = getCJint(cj,"rangeBStop",      sys->rangeBStop,      false);
		  set the binary range end index for a  ranged (exception) scan

    if(ret) ret = getCJint(cj,"port",            sys->port,            false);
		  set the port number (default 20000)

    if(ret) ret = getCJint(cj,"master_address",  sys->master_address,  false);
		  set the client address 

    if(ret) ret = getCJint(cj,"station_address", sys->station_address, false);
		  set the server address 

    if(ret) ret = getCJstr(cj,"id",              sys->id,              true);
		  set the system id ( can be different for client and server)

    if(ret) ret = getCJstr(cj,"protocol",        sys->protocol,        false);
                  legacy value not used

    if(ret) ret = getCJstr(cj,"ip_address",      sys->ip_address,      true);
                  ip address for client to connect to server , server uses this as the interface to listen on can be set to 0.0.0.0 

    if(ret) ret = getCJstr(cj,"name",            sys->name,            false);
                  legacy value not used

    if(ret) ret = getCJint(cj,"debug",           sys->debug,           false);
                  turns on / off debug allowed values 0,1 and 2

    if(ret) ret = getCJstr(cj,"base_uri",         tmp_uri,             false);
                  overrides the default /components and /interfaces base uris

    if(ret) ret = getCJstr(cj,"local_uri",        sys->local_uri,      false);
                  provides a way to set/get local access to the data points

    if(ret) ret = getCJbool(cj,"unsol",           sys->munsol,         false);
                  allows usolicited  events to be sent and recieved in hte system, st on both client AND server

    if(ret) ret = getCJbool(cj,"useOffset",       sys->useOffset,      false);
                  depreceted idx over rules offset 

    if(ret) ret = getCJbool(cj,"useVindex",       sys->useVindex,      false);
                   use with care , creates a densly populated point array. must be set on client AND server 

    if(ret) ret = getCJdouble(cj,"deadband",      sys->deadband,       false);
                   defaults to 0.1 , specifies change in value required before update to the client


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
                "clazz":1,
		        "evariation":"Group32Var7",
                "uri":"/metrics/misc",
                "signed": true,
                "deadband":0.5


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
