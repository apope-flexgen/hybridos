Setting up a test environemnt for go_metrics
P. Wilshire
04_19_2023

### Introduction

This document describes the integration test environment for go_metrics.

The tests are run as a series of scripts. The outputs generated can be used in a "nasimi" type system with pub capture enhancements.
The scripts run in a container ( flexgen:centos-dev ) with the git repos volume mounted under /home/docker.
Each test will run the go_metrics program and a fims_listen for a brief time with a selected config.
The configs match the example configs in the go_metrics repo and the scripts/xxx/test_yyy also match these directory structures.

### Test environment

Put your git checkout on a windows dir
checkout initial_setup or a later branch.

Volume mount that into your docker container. We'll put a docker compose thing in the repo in a couple of days.

go_metrics will require a few changes to go_fims ( these will be integrated into fims / dev real soon now )

```
cd /home/docker/git/go_metrics
cp -a fims /usr/lib/golang/src

```

Next build the go_metrics package


```
cd /home/docker/git/go_metrics/src
go mod tidy ( one time)
go build -o /usr/local/bin/go_metrics
```

run it

```
go_metrics

2023/04/19 19:39:08 Error unmarshaling json file:
[] metrics > {} 0 > expression: metrics expression produces possible result type int but gets cast to float (warning only)
Subscribed to:
        /components/feeder_52m1
        /components/feeder_52u1
        /components/bms_74b
        /components/feeder
        /some/level2
        /some/output
        /some/status/output
        /some/output1
        /some/output10
        /some/output16
        /some/output13
        /some/output4
        /some/output7
```


In another terminal window run "fims_listen"

```
Method:       pub
Uri:          /some/output7
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"output7_bobcat":{"scale":100,"ui_type":"none","units":"deg C","value":null},"output7_cheetah":{"scale":100,"ui_type":"none","units":"deg C","value":null},"output7_lion":{"scale":100,"ui_type":"none","units":"deg C","value":null}}
Timestamp:    2023-04-19 19:41:48.514882

Method:       pub
Uri:          /some/output1
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"output1_bobcat":{"scale":100,"ui_type":"none","units":"deg C","value":null},"output1_cheetah":{"scale":100,"ui_type":"none","units":"deg C","value":null},"output1_lion":{"scale":100,"ui_type":"none","units":"deg C","value":null}}
Timestamp:    2023-04-19 19:41:48.515033

Method:       pub
Uri:          /some/output4
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"output4_bobcat":{"scale":100,"ui_type":"none","units":"deg C","value":null},"output4_cheetah":{"scale":100,"ui_type":"none","units":"deg C","value":null},"output4_lion":{"scale":100,"ui_type":"none","units":"deg C","value":null}}
```
Simple test for fims get

```
fims_send -m get -r /$$ -u /some/output4/output4_bobcat
{"output4_bobcat":null}
```

### An example  test run 

```
sh scripts/math/test_subtract.sh  start

 logname subtract cfg examples/math/subtract timeout 10
running /home/docker/git/go_metrics/logs/fims/subtract
timeout 10 go_metrics /home/docker/git/go_metrics/examples/math/subtract.json log  /home/docker/git/go_metrics/logs/run/subtract

sleep for 2 timeleft now  8
Test fims get
{"float_output1":0,"float_output2":0}
sleep for 2 timeleft now  6

Test fims get
{"float_output1":120.44,"float_output2":-1014.1199999999999}

cmd -> fims_send -m get -r /32459 -u /some/output
 expect
{"float_output1":120.44,"float_output2":-1014.1199999999999}
{"float_output1":120.44,"float_output2":-1014.1199999999999}
Pass
sleep for 2 timeleft now  4

Test fims get
{"float_output1":-19.1,"float_output2":-53.1}

cmd -> fims_send -m get -r /32459 -u /some/output
 expect
{"float_output1":-19.1,"float_output2":-53.1}
{"float_output1":-19.1,"float_output2":-53.1}
Pass
sleep for 4 timeleft now  0
test complete
```

### Test script options


( Under a bit of churn at the moment but start always works)

sh scripts/math/test_sqrt.sh <option>

 help : show this help
 start : start the test
 read_fims : read the fims capture
 log : read the go_metrics log file
 result : read the test result
 config : show config  
 expect : show expected result (TODO)


### Running a test 

sh scripts/math/test_subtract.sh  start

sh scripts/math/test_subtract.sh  log   show the go_metrics log output

sh scripts/math/test_subtract.sh  fims   show the go_metrics fims_listen output

sh scripts/math/test_subtract.sh  config   show the go_metrics test config





