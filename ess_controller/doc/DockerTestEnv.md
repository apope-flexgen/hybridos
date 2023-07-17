### Docker Config Test Environment

## introduction 

This document describes the docker test environment used to run the config tests on the ess_controller
The config tests are planned to exercise every  modbus data item defined in the modbus maps and routed through the  path :

```
    modbus_server->modbus_client->ess_controller
```

A different set of tests will verify the path 

```
   modbus_server->modbus_client->ess_controller->site_controller
```

This is a static set of tests where preset values are set in the modbus_server using ssh  
and the effect of setting those values traced through all the  variables affected by that modbus data entry.

This test operation provides a vital check on the configuration and data paths through the system.


## Basic Setup 

This describes the basic set up for two test containers.
Its a bit manual at the moment. We will hjijack one of the docker_testng systems and use that as soon as we can set one up.


The "target" container is at 172.17.0.3 , this runs the following start up operation.
Note that the gcc930 containers have a volume mount for the system src directory mounts=ed on /home/build.
The git source is managed and edited under Windows but the system is compiled and run inside the linux based docker containers. 
The docker continers do not , currently, maintain time sync required for correct git operation.  

```
cd /home/build/git_build/ess_controller/
  . scripts/benv.sh▬
fims_server > /dev/null 2>&1 &
 sh configs/modbus_server/bms_1_echo.sh
 modbus_server configs/modbus_server/bms_1_modbus_server.json  &
```


The "host" container is at 172.17.0.2

```
 cd /home/build/git_build/ess_controller/
 . scripts/benv.sh
fims_server > /dev/null 2>&1 &
modbus_client configs/modbus_client/bms_1_modbus_client.json > /dev/null 2>&1 &
```


You have to have set up no password login between the host and the target so that 
```
  # on 172.17.0.2
  ssh root@172.17.0.3 ls
```

works without a password

## no password login

first you have to install openssh-server

```
yum install openssh-server
```

this involves setting up keys on each using 

```
ssh-keygen
```

and   then

```
cp /root/.ssh/id_rsa /etc/ssh/ssh_host_rsa_key

/sbin/sshd
```

ignore these

```
Could not load host key: /etc/ssh/ssh_host_ecdsa_key
Could not load host key: /etc/ssh/ssh_host_ed25519_key
```

Set up the root password on the target 

(use passwd) 

use ssh-copy-id on the host to transfer the host keys into the target's authorized_users 

```
ssh-copy-id root@172.17.0.3
```


run the ess_controller on the host

```
ess_controller -x
```

then run the loader script on the host ( or use the dbi system to load the configs)

```
sh scripts/dual_bms/load_bms_1.sh
```

The tests are all in the scripts/release/  directory

```
   sh scripts/release/800.4_test_dual_bms_rack_modbus.sh
```


You should see this output after a few minutes.

```
 test413 [passed]  >>  >>>>>>>>>>>  fault /status/rack_01/RackMinCellTemp value  5.3 expect 5.3
 test414 [passed]  >>         >> get  /status/rack_01/RackMinCellTemp   ->     5.3
 test415 [passed]  >>  >>>>>>>>>>>  fault /status/rack_01/RackAvgCellVoltage value  4.3 expect 4.3
 test416 [passed]  >>         >> get  /status/rack_01/RackAvgCellVoltage   ->     4.3
 test417 [passed]  >>  >>>>>>>>>>>  fault /status/rack_01/RackAvgCellTemp value  53 expect 53
 test418 [passed]  >>         >> get  /status/rack_01/RackAvgCellTemp   ->     53
#########################
######## 800.4_test_dual_bms_rack_modbus: passed 418 out of 418
#########################
######## 800.4_test_dual_bms_rack_modbus: all tests passed
####################################################

```
## The test scripts

This are a little strange at the moment 

Here is a sample 

```
scripts/release/800.4_test_dual_bms_rack_modbus.sh


#!/bin/sh

#
# file 800.4_test_dual_bms_rack_modbus
#

rtest="800.4_test_dual_bms_rack_modbus"

tfile=./scripts/release/test_run.sh
. $tfile

```

This pulls in a text file defining the test.

Lets check out parts of that file


```

# defs for 800.4_test_dual_bms_modbus.sh
# p. wilshire
# 03/20/2022
#
# the test script dots in this to run the config set up by 800.4_test_dual_bms.sh

echo  "the ess controller must be running    ess_controller -n ess -c ess_init1 -x"
echo "the dual bms system must be loaded  sh scripts/dual_bms/load_bms_1.sh"


FimsDir=/usr/local/bin/
FimsName=/ess
TargetIp='172.17.0.3"

# global arrays usd for the tests
descs=()
acts=()
resps=()
act_res=()

# define test functions to set / get fims data

# here is an example
#            $1 - var           $2 - value    $3 - delay  $4 - description
# setAssetVal  /status/ess/build   newversion    0.1 " set up new version"
function setAssetVal()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set -r /$$ -u $1 '{
      "value": $2
}'
    | jq | grep $2
EOF
)
  resps[idx]=$(cat<<EOF
    "value": $2
EOF
)
  sleeps[idx]="$3"
  idx=$(($idx+1))
}

```

The tests are set up as a series of layered functions with increasing complexity

Once you have all the functions defined , the txt file can then start the test execution.

```

idx=1
# test      modbus_uri                       modbus_item               local name                      ui name                              values
#
#test        $1                              $2            $3    $4
setAssetVal  /ess/config/bms/enable          true          0.1   "  >> set enable   ->        /config/bms:enable"
setAssetVal  /ess/config/bms/enum_opt        true          0.1   "  >> set enum_opt   ->     /config/bms:enum_opt"

return 0
```

## Using ssh across containers


The test function by setting a value on the modbus_server and watching that value get processed through the modbus_clinet and ess_controller.

This is needed so that the system operates in a full "modbus data environment". I represents the function with the presence of all the associated data.

Here is a snapshot of the system running during a test.

```
top - 09:58:49 up  5:56,  0 users,  load average: 1.71, 1.91, 1.89
Tasks:   7 total,   1 running,   6 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.7 us,  0.2 sy,  0.0 ni, 98.8 id,  0.0 wa,  0.0 hi,  0.3 si,  0.0 st
KiB Mem : 16242728 total, 13482732 free,  1056040 used,  1703956 buff/cache
KiB Swap:  4194304 total,  4194304 free,        0 used. 14492192 avail Mem

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
  101 root      20   0 3297248  34252   4068 S  73.8  0.2  44:27.22 modbus_client
  188 root      20   0  205740  62392   8004 S  13.3  0.4   6:26.52 ess_controller
   30 root      20   0 1942216 125276   4308 S   1.7  0.8   0:50.86 fims_server
    1 root      20   0   11840   2820   2548 S   0.0  0.0   0:00.08 bash
   17 root      20   0   11836   3032   2628 S   0.0  0.0   0:00.06 sh
   23 root      20   0   11840   2956   2524 S   0.0  0.0   0:00.23 sh
 6228 root      20   0   56196   3924   3332 R   0.0  0.0   0:00.02 top


```

A test value is transferred to the "Modbus Server" using this typical function.


```

testFault ()
{
    setSSHVal    $1                      $2     $5           1.2      " >>>>>>>>>>>  fault $4 value  $5 expect $6"
    getAsset  ${FimsName}$4              "$6"          0.1     "        >> get  $4   ->     $6"
}


testFault /components/bms_1_rack_01 rack_run_state      0 /status/rack_01/SystemState  0 Normal

```

And the low level script functions are as follows.

```
#           $1 - variable name            $2 - expected value    $3 - delay   $4 - description 
# getAsset  /assets/ess/summary/version   newversion             0.1         "get the new version"
function getAsset()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m get -r /$$ -u $1
    | jq
EOF
)
  resps[idx]=$(cat<<EOF
"$2"
EOF
)
  sleeps[idx]="$3"
  idx=$(($idx+1))
}


#           $1 - variable base name   $2 - var name      $3 - set value    $4 - delay   $5 - description 
# setSSHVal  /components/bms_1       RackSoc             23.5              1           "set the modbus_value"
setSSHVal()
{
  descs[idx]=$5

  acts[idx]=$(cat<<EOF
ssh root@${TargetIp} "fims_send -m pub  -u $1 '{\"$2\":$3}'"
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
  sleeps[idx]="$4"
  idx=$(($idx+1))
}

```



## Test Results

The test results are saved in two files.

```

 /var/log/ess_controller/tests/output/800.4_test_dual_bms_rack_modbus_log.txt

 /var/log/ess_controller/tests/output/800.4_test_dual_bms_rack_modbus_results.txt

```


The whole test is controlled by a script "test_run.sh". This file also defines the location of the test results.

Note the test is executed by the "run_tests" function using the arrays set up by the test description.  

```
#!/bin/sh
# full test run

#$1 desc
#$2 act
#$3 resp
#$4 outfile

#run_test $test1_desc $test1_act $test1_resp $outfile
num_passed=0
num_failed=0
fails=()

tfile=./scripts/release/test_definitions.sh
. $tfile

# load in the test setup defs...
deffile=./scripts/release/${rtest}.txt
. $deffile

mkdir -p /var/log/ess_controller/tests/output
outfile=/var/log/ess_controller/tests/output/${rtest}_results.txt
logfile=/var/log/ess_controller/tests/output/${rtest}_log.txt

rm -f $outfile
test_header $outfile

rm -f $logfile
test_header $logfile

fidx=1

run_tests $fidx $idx

ix=$(($idx-1))
test_tail $outfile

echo
echo

cat $outfile

```

