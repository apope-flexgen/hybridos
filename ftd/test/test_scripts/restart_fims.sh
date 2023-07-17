# Restart fims_server and any process that will not automatically restart from a fims_server crash
date
pkill fims_server
sleep 0.5 # give time for old fims_server to clean up
/usr/local/bin/fims_server & 
sleep 0.5 # give time for new fims_server to start up
pkill dbi
pkill metrics
sudo pkill cops
sudo pkill web_server
