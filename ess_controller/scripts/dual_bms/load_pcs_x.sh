fims_send -m set  -u /ess/cfg/cfile/ess/pcs_1_manager.json -f configs/dbi/ess_controller5/pcs_x_sc2750_manager.json 
fims_send -m get  -r /281 -u /ess/amap  | jq
