Gcom_Modbus_Client

 Are the Pubs Working  Checklist

p. wilshire 
10_25_2023
v 1.2


# 1
Read Config and extrack pub groups
Each pubgroup will consist of one or more register blocks.
Make sure we list the pub groups on strtup.

And will have a list of map items 
The pubgroup will have a frequency and an offset in Milliseconds

the config will allow Coils and Holding registers to be polled for pubs if required
pub_coil:false
pub_holding:false
pub_input:true
pub_direct:true

done 11_04_2023


# 2
register the pubgroups with the timer.
done 

# 3
Check that the pub timer callback occurs as specified in the config
done 

# 4
The pub timer callback should collect together the register groups associated with  the pub group
Assemble the vector of io_points (struct map) and prepare io_work items for submission to the poll channel
TODO modify groups

# 5
The groups will have to be restructured to allow for disabled points 
Allow any point to be disabled  make sure we do not attempt to read disabled  points , reconstruct io_work items if needed.
done


# 6
Send all the io_work groups to the Poll channel.
record the time taken to send and recieve data from the server
record any timeouts or reconnect attempts required.
keep track of any recoverable errors (server busy etc.)
DOne


# 7 
The io_threads will poll the data from the server and route the completed io_work items to the response thread.
make sure that all the reciveved io_work items are sent to the response thread. 
Done

# 8 
make sure that the response thread correctly reassambles the pub group from the expected number of io_work items.
make sure that the response thread places the used io_work object back into the poolChannel for reuse.
done


# 9 
The response thread will reassemble the pub group discarding any stale data from prevous communication attempts
Make sure the response thread will discard any io_work items that do not match the timestamp of the current object.
done

# 10 
make sure that any assembly of any incomplete pub groups is halted , logged and discarded when a new pub group 
(same name different time) starts to arrive.
TODO

# 11
Once completed. Ensure that the io_work items in the pub group are correctly decoded into fims output messages.
done

# 12 
check that all the bit_field / bit map / enum objects are decoded correctly.
DONE


# 13 
make sure that the raw value , returned in the io_work object is saved with the map item.
Done 11_10_2023

# 14 
Ensure that the pub operation is logged and timed.
TODO

# 15 
ensure that the io_work object use count ( and timings ) is monitored.
TODO

# 16 
ensure that the timer object is correctly shut down when the system restarts or runs a reconfig.
TODO

# 17 
make sure that scale / shift / signed options all work also make a note of use bool to force true/false or 1/0 options for bool outputs
TODO

# 18
When a pub has completed make sure that a sync signal is sent to the timer to allow it to delay then next pub request if needed.
TODO sync seems to be broken

# 19
make sure that the system / component / io points can handle clothed or naked formats 
TODO

# 20 
Ensue that byte swap , including  floats , works correctly.
DONE 11_10_2023 allow all options for 4 reister byte swap 1234  2143  3412 4321 1324 2413 3142 4231 

# 21
Extra credit 
Allow fims commands to stop start  reshedule timers ( inc offsets)




