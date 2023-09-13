#!/bin/sh
fims_send -m set  -u /ess/cfg/cfile/setup_bms '{
	"pname":"ess",
	"amname":"bms",
	"setup":[ 
		  {
       			"func":"setupBms",
			"name":"bms_2",
			"pname":"bms",
			"type":"CATL_3456",
			"every":0.1,
       			"targ":"/test/mySched:Targ2",
       			"debug":1,
       			"fcn":"testRunSchedOpts",
			"schedid":"bms_2_id"
		  }
               ]
	}'


