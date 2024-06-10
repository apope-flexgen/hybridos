Docker test setup for DTS. To be used for testing DTS running in concert with an Influx container and Mongo container. Containers can be stopped to test DTS' ability to run when one or more databases go down.

You can run pytests on the test network by running the command `pytest -vv` in the test docker directory while the docker compose is running.