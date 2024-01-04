Gcom_Modbus_Client

 Are the Sets Working  Checklist

p. wilshire 
10_25_2023
10_26_2023

# 1
Read Config and extract all objects 
  done 10_25_2023


# 2
Set up a fims_listener to receive incoming set ( and get etc ) messages.
 sets done 10_25_2023
 gets done 11-10-2023

# 2.1
Record stats on fims message size etc
done 10_26_2023

# 2.2
Allow message buffer size to be in the system config.
done 10_26_2023


# 3
Respond to a fims set message by extracting the data points and value using a std::any object.
done 10_25_2023

# 3.1
record process name and user name in the struct map object ( who did this )
done 10_26_2023


# 4
Allow single or multi, naked or clothed input objects.
done 10_26_2023


# 5
Assemble the vector of io_points (struct map) and prepare io_work items for submission to the set channel
The groups will have to be restructured to allow for disabled points 
Done 11-10-2023

# 5.1 
Allow any point to be disabled  make sure we do not attempt to read disabled  points , reconstruct io_work items if needed.
done 10_25_2023


# 6
Send all the io_work groups to the Set channel.
# 6.1
Done use the set channel
                setWork(io);

# 6.2 
record the time taken to send and recieve data from the server

# 6.3
record any timeouts or reconnect attempts required.

# 6.4
keep track of any recoverable errors (server busy etc.)

# 6.5 
add config options to control error handling

# 6.7
dont use max_run to trigger end of attepts
Done

# 7 
The io_threads will route the completed io_work items to the response thread.
The response thread should record timings and send any fims response associated with the set request.
done 10_26_2023

# 8 
The response thread should record the raw value sent out to the server
make sure that the raw value , returned in the io_work object is saved with the map item.

DOne 11_10_2023

# 9 
Ensure that the set operation is logged and timed.

# 10
Ensure that the set operation takes precidence over any pending pubs
setWork(io)  done 10_26_2023

# 11
Ensure that the multi register option is handled correctly.
Config options added to enable / disable multi sets
Done 11-10-2023


# 11.1 
  If a multi fims set is used prepare and manage a set group like a pub group to send a fims response after all the objects have been processed
TODO check 
# 11.2 
  notify an attempted set on a disabled register.
   maybe not

# 12
Ensure that the byte / word order of size 2 and 4 registers works correctly.
Done word_order 11-10_2023


# 13
Ensure that the float opertion is rejected for size 1 registrs
TODO

# 14
Ensure that the float opertion works correctly for size 2 and 4 registers with and without byte swap 
TODO

# 15 
ensure that the io_work object use count ( and timings ) is monitored.
TODO

# 16
make sure that  the extended set operations work correctly to enable / disable / debounce / force and unforce registers
both  normal coil and holding outputs but also (TODO) Inputs and Discrete Input registers.

_disable / _enable /_force /_unforce in place 10_25_2023.
_local inplace 11_10_2023

# 16.1

Set up Local / remote get operations 
done 

# 17 
check off_by_one
done

# 18 
check force_multi_sets
 in place 10_25_2023  needs testing 
done 11_10_2023


# 19 
trace io_work to response thread and check for good / bad results
done 10_26_2023

# 19.1 
   bad register response
TODO

# 19.2 
    good set response
done 10_26_2023






