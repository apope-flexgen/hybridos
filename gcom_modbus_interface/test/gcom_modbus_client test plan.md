gcom_modbus_client test plan

p wilshire 
04_16_2024



So this is how I see the test plan going forward. Just my thoughts.

This plan assumes that the interface unit testing where input and output values are matched to config options has been completed.
That testing will still use the modified python test framework.

This is the next level of testing realated to operational conditions.
This testing assumes that the config max_num_connections is set to 1. The gcom_modbus_server only handles one connection thread.



1/ Validate the client config reads. (use good and bad configs).


2 connection
2.1/ validate the server connect / disconnect( ip and port)
2.2 make sure events are correct, indicating connection made and / or lost
2.3 add bad data points (invalid offset) to the client, make sure the systems still connect and disables the bad points 



3 client pub output
3.1 / with client and  server connected, make sure the client pubs data
3.2 / add bad data points to the client with client and  server connected, make sure the client pubs data for th valid points.

3.3/ with the server disconnected make sure that the pubs stop and restart again when  the server is reconnected.
3.4/ pub some sample data on the server, make sure that the client shows the changed values on its pub output
3.5/ send some invalid uris to the server , make sure the response is correct.


4 server set output
4.1/ with client and server connected make sure that fims sets on the client result in fims sets from the server, 
4.2/ send the clinet sets with bad uris, check response
4.3/ send the clinet sets with bad data names, check responses


5 Network stress testing 
5.1/ add delay value below transaction timeout (250)  to server response 
     /interfaces/site/_bug '{"delay":200}'
     system should still operate and pub data.

5.2/ add delay value above  transaction timeout (250)  to server response 
     /interfaces/site/_bug '{"delay":300}'
     system should not be able to maintain a connection. No pub data. logs should show connection attempts.

5.3/ reduce delay value below  transaction timeout (200)  to server response 
     /interfaces/site/_bug '{"delay":200}'
     system should recover  and pub data.

5.4/ add a temporary delay  delay value above  transaction timeout (300)  to server response 
     /interfaces/site/_bug '{"sdelay":300}'
     system should recover from the delay , reconnecting if required and continue operation.

6 System stress testing
6.1/ introduce more fims pubs to unknown uris on the server. The system should not produce any excess error messages.

6.2/ introduce fims sets the same uris used for the pubs on the server. The system should not produce any excess error messages.

6.3/ introduce more fims pubs to unknown uris on the client. The system should not produce any excess error messages.

6.4/ introduce more fims pubs to known uris on the client. The system should not produce any excess error messages.


6.5/ connect more than one client to the same server.
        The system should produce log messages but not fail. 

6.6/ use a large config file with several groups publishing data at different frequencies.

6.7/ Add a debounce to coil and holding registers.  Write set values on the clinet at an interval above the deadband.
         the server should not produce outputs at a greater frequency than the deadband interval, values may be skipped inside the deadband.

7 Point management
7.1/ Disable an input value from the server. The point should not be present in the list of values pubbed by the client    
7.2/ Enable an input value from the server. The point should  be present in the list of values pubbed by the client    
7.3/ force  input value from the server. The point should  show the forced value  being pubbed from the client  
7.3/ force  output value to the  server. The point should  show the forced value  being set from the server  

8 Limited extra features
8.1/ Full data. use the "_full" request  to see more details on an individual or set of multiple uris.
8.1/ stats. use the "_stats" request  to see details of the system communications stats.









