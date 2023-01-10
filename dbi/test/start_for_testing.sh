pkill metrics
sudo pkill mongod
pkill fims

sleep 3s
sudo mongod &
sleep 3s
# fims server is not needed for dbi_test.js testing, but is needed for manual testing of dbi.js
/usr/local/bin/fims/fims_server &
# sleep 3s
# node ../src/dbi.js

# // create or update a record (SET):
# /usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/assets '{"nominal_voltage": 550.0,"nominal_frequency": 60.0,"start_value": 207,"stop_value": 207}'

# // get a record (GET):
# /usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/assets ''
# /usr/local/bin/fims/fims_send -m get -u /dbi/site_controller ''
# /usr/local/bin/fims/fims_send -m get -u /dbi/_show_collections -r /me

# // delete a record (DEL):
# /usr/local/bin/fims/fims_send -m del -u /dbi/site_controller/assets '45'
