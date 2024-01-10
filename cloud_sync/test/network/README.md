# CloudSync Test Network
This test directory defines a docker compose setup for testing cloud_sync over a docker network.

In order to run the docker compose, you first need to do some setup by running the test/network/build.sh script. You can then run docker compose up to spin up the containers.

The test/network/ subdirectories are as follows:
* client_node/ contains the files specific to the client container
* server_node/ contains the files specific to the server container
* test_scripts/ contains scripts used to modify the running network for testing purposes

You can run pytests on the test network by running the command `pytest -vv` in the test network directory while the docker compose is running.