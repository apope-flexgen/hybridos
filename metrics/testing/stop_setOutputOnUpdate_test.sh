sudo pkill mongod
sleep 3s
pkill metrics
sleep 3s
pkill fims

# the following double-checks to see if any metrics processes have been left running.
# if so, use the process ID displayed to run a `kill -9 [process ID]`
ps aux | grep 'metrics'