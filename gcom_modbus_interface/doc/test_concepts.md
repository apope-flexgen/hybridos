dnp3 / modebus test concepts
p. wilshire
10_09_2023

I have been looking at the options for dnp3/modbus testing. I have a miserable set of test scripts but I think we can do better.
Now this is just a proposal for discussion
The first thing , I think,  we need to do is to set up some configs that contain all the variations etc.
Then we need to run server and client  on  different nodes. These can be ssh nodes or docker containers.
more on that in a minute.
then for client ,
we need to start a fims listener on the server (using a timeout  and collect the output in a fims_listener log
then we need to send out a load of sets  to the different test points
 fims_send -m set -r /me -u /components/dnp3_master '{"testid":101}' 
 fims_send -m set -r /me -u /components/dnp3_master '{"testCROB_4":"LATCH_OFF"}'
 fims_send -m set -r /me -u /components/dnp3_master '{"testid":102}' 
 fims_send -m set -r /me -u /components/dnp3_master '{"testCROB_5":"LATCH_ON"}'
 fims_send -m set -r /me -u /components/dnp3_master '{"testid":103}'  
 fims_send -m set -r /me -u /components/dnp3_master '{"testCROB_6":true}'
 fims_send -m set -r /me -u /components/dnp3_master '{"testid":104}'  
 fims_send -m set -r /me -u /components/dnp3_master '{"testCROB_7":false}'
The fims listen will contain the testid numbers >
we can scan those to make sure the server set outputs match the expected values
Ie create a python scanner to match outputs with test ids and expected results.
If we are smart , and we are smart, we can send dummy fims messages to the serve to say this is test_id and We expect this result
This would all be contained in the fims log and we can identify  test ids the expected results and the actual results.
I'll see if I can put together an example of this in a few minutes.
Now all of this needs some way to start/stop the clients and severs on different nodes with different configs
We need to start and stop fims-listeners
and we need to collect the fims_listener output and then extract the test results.
I propose a simple node or python based command server that listens to post messages and provides responses
I have the bare bones of it already in place.
this is a script that listens for commands on a websocket.
The commands can be
1/ run this prog with these args on a specified  ip
2/ send this file to this dir on a specified  ip
3/ get this file from this dir from a specified ip.
now this neat little tool will simply run on either the docker containers or the ssh nodes when we run on real hardware.
It runs everywhere.
When it runs it is given an argument of its own ip address.
If a request comes in for that ip address it simply runs the command locally.
If a request comes in for a different ip address the tools then acts as a relay and forwards the command to the designated remote ip address.
why do we do this.
When running in a docker container we can specify a mapped port 5000:5000
which means that requests sent to local host (windows) port 5000 will be sent to the docker container with that port mapping.
That docker container will have access to all the other docker containers on the same network.
So , from windows , we send a command to port 5000 to run a fims_listen on 172.17.0.5
the forwarder ( running on 172.17.0.6) will detect that the destination ip is different form its own ip and forward  the command to the correct node.
the same relay object running on the 172.17.0.5 node will receive the command and run the fims_listen and send a response back to 172.17.0.6 which will return the same response  to the original requester running on windows.
When running on real hardware we only have to set up ssh access to one node in the test network and the relay will give us access to all the other nodes in the network.
This means that the windows system can send commands/files  anywhere on the test system just by asking the relay on port 5000 to distribute the request  and collect the response on  the correct node.
"Its not for me so lets ask the dest ip to do the job."
I'll set up a demo for you this morning.
 All this means  we can , from a windows or a linux system , talk to any node on our test network and get them to run things and collect data.
the test results will be all contained in a fims_listen output and we process that to get the pass /fail results.
Here is a typical test setup for the client
1/ send server config to the server
2/ send the client config to the client
3/ start the dnp3_server on the server with the server config
4/ start a fims_listen on the server
5/ start the dnp3_client on the client with the client config
6/ in a loop
    send the the testid (fims_sets) to the server
    send the test sets to the client
7/ stop the fims_listen on the server
8/ pull the fims_listen log file back to windows.
9/ run the test result extractor script (python) under windows to collect the results. 