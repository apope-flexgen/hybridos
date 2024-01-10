Gcom_Modbus_Client


Code Review 
p. wilshire 
11_11_2023


todo

gcom_config_any
gcom_iothreads
gcom_modbus_pub
gcom_modbus_decode
gcom_modbus_encode
gcom_fims
gcom_stats
gcom_config
gcom_utils

pending

gcom_perf
gcom_timer
gcom_logger


done 

version
gcom_heartbeat
gcom_watchdog







# 1
Read Config and extract all objects 
done



# 2
Set up a fims_listener and process threads to receive incoming messages.
need to throttle message 

# 3
Set up Timer Object 
Pub Timers

WatchDog timers
Heartbeat timers
all done


# 4

Set up 
IO_Threads and response thread
all done


# 5

Command line options
  just file name run with named config
  -c file name run with named config
  -f (-u) dbi name load from dbi
  -t test_name  run named tests.
add "detect" on the end to allow the system to check all items on the config and run with missing items disable.
called overrides and its done 11_10_2023



# 6 
Pub st

    if(!getItemFromMap(gcom_map, "connection.connection_timeout",   myCfg.connection.connection_timeout,  2,               true,true,false)) ok = false;
 
    if(!getItemFromMap(gcom_map, "connection.max_num_connections",  myCfg.connection.max_num_connections, 1,               true,true,false)) ok = false;

    if(!getItemFromMap(gcom_map, "connection.allow_multi_sets",     myCfg.allow_multi_sets,                bval,           true,true,false)) ok = false;
    if(!getItemFromMap(gcom_map, "connection.force_multi_sets",     myCfg.force_multi_sets,                bval,           true,true,false)) ok = false;
    if(!getItemFromMap(gcom_map, "connection.max_reg_size",         myCfg.max_reg_size,                    125,           true,true,false)) ok = false;
    if(!getItemFromMap(gcom_map, "connection.max_bit_size",         myCfg.max_bit_size,                    125,           true,true,false)) ok = false;

// TODO
// TODO
    if(!getItemFromMap(gcom_map, "connection.transfer_timeout",     myCfg.connection.transfer_timeout,  2,                 true,true,false)) ok = false;
    if(!getItemFromMap(gcom_map, "connection.data_buffer_size",     myCfg.connection.data_buffer_size,  100000,            true,true,false)) ok = false;
