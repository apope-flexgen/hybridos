# set up the functions for the assets scheduler 
   

/usr/local/bin/fims_send -m set -r /$$ -u /ess/full/system/commands/run '{"value":15,"uri":"/control/dataMaps:data_maps","every":2,"offset":0,"debug":0}'

# runs dataMap testing functions without using the scheduler
# fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":15,"cmd":"run_rack","pname":"rack","current":1.5, "timeMs":51.0}}'