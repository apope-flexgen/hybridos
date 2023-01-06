sudo echo "##### ESS STOP #####"

sudo pkill MCP

pkill modbus_client
pkill ess_controller
pkill twins

pkill storage
pkill metrics
pkill events

sudo pkill influxd
sudo pkill mongod

sudo pkill modbus_server
sudo pkill web_server

pkill fims
pkill dbi
