#!/bin/bash
pkill fims_server
pkill twins
pkill fims_echo
/usr/local/bin/fims/fims_server &
sleep 2s;/usr/local/bin/twins twins_tx100.json &
sleep 2s;/usr/local/bin/fims/fims_echo -u /components/bms_e -b '{"warning_21":0,"warning_22":0,"sys_status":0,"bms_heartbeat":0,"bms_power":0,"bms_status":0}' &
/usr/local/bin/fims/fims_echo -u /components/pcs_e -b '{"vac_st":100,"vac_tr":100,"iac_st":7,"iac_tr":9}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu1 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":1,"idc":1,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu2 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":2,"idc":2,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu3 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":3,"idc":3,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu4 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":4,"idc":4,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu5 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":5,"idc":5,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu6 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":6,"idc":6,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu7 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":7,"idc":7,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu8 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":8,"idc":8,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
/usr/local/bin/fims/fims_echo -u /components/sbmu9 -b '{"warning_1":0,"warning_21":0,"warning_22":0,"warning_23":0,"vdc":9,"idc":9,"soc":0,"soh":0,"maxcellvolt":0,"mincellvolt":0,"avgcellvolt":0,"maxcelltemp":0,"mincelltemp":0,"avgcelltemp":0,"icharge":0,"idischarge":0}' &
#sudo /usr/local/bin/modbus_interface/modbus_server tx100_slim_bms_modbus_server.json
#sudo /usr/local/bin/modbus_interface/modbus_server tx100_slim_pcs_modbus_server.json

