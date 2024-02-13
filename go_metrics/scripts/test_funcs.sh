#!/bin/bash
#"name":"rss.json",
#"author":"Phil Wilshire",
#"desc":"test the rss function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

if [ "$logname" == "" ] ;
then
  logname=$testname
fi
#logname=sqrt
#cfgname=examples/math/sqrt
#timeout=20
#timeused=0

dir=echo
# dir=$(basename $(dirname "$0"))
echo "dir is $dir"


# if [ $# -gt 0 ] ; then 
#   dir=$1
# fi

cfgname=examples/$dir/$logname
timeout=20
timeused=0
PASS=0
FAIL=0

gitdir=$(cd `dirname $0` && cd ../.. && pwd)
#gitdir='/home/docker/hybridos/go_metrics'
echo "gitdir #2 is $gitdir"
#. $gitdir/scripts/test_finctions.sh
declare -a cmds
addcmd()
{
    cmd="$@"
    cmds[${#cmds[@]}]=$cmd
}

showcmds()
{
    echo "show cmds "
    for cmd in "${cmds[@]}"
    do
      echo $cmd
    done
}

runcmds()
{
    #echo "run cmds "
    for cmd in "${cmds[@]}"
    do
      res=`$cmd`
      echo $cmd
      #echo $res
    done
}

start ()
{
    #start config_doc_1 config_doc_1 5
    echo " logname $logname cfg $cfgname timeout $timeout"
    mkdir -p ${gitdir}/logs/fims
    mkdir -p ${gitdir}/logs/run
    timeout $timeout fims_listen > ${gitdir}/logs/fims/$logname 2>&1&
    echo running ${gitdir}/logs/fims/$logname
    timeout $timeout go_metrics ${gitdir}/${cfgname}.json > ${gitdir}/logs/run/$logname 2>&1&
    echo "timeout $timeout go_metrics ${gitdir}/${cfgname}.json log  ${gitdir}/logs/run/$logname"
    test
    echo "Passed tests: $PASS"
    echo "Failed tests: $FAIL"
}

usetime()
{

    #echo -n sleep for $1
    to=$1
    sleep $to
    timeused=$(($timeused + $to))    
    timeleft=$(($timeout - $timeused))
    #echo " timeleft now  $timeleft"
}

runget ()
{
    if [[ "$1" != "fims_send" ]] ; then
    shift
    fi
    echo -n "response =>" 
    #echo $@
    #echo $expect
    res=`$@`
    #len=${#res}
    echo $res
    if [[ "$res" == "$expect" ]] ; then
      #echo $res
      echo "                    Pass"
      ((PASS++))
    else
      echo ' expect '
      echo $expect
      echo "==================> Fail"
      ((FAIL++))
    fi
}


expect()
{
     cat ${gitdir}/logs/fims_expect/$logname
}


read_fims_log_nots()
{
    cat ${gitdir}/logs/fims/$logname | grep -v "Timestamp"
}

read_fims_log()
{
    echo fims_log  ${gitdir}/logs/fims/${logname}
    cat ${gitdir}/logs/fims/$logname
}

read_run_log()
{
    cat ${gitdir}/logs/run/$logname
}

read_config()
{
    echo config  : ${gitdir}/${cfgname}.json
    cat ${gitdir}/${cfgname}.json
}

checkres ()
{
    res=$1
    exp=$2
    tol=$3
    qres=`python -c "if abs( $res - $exp)<$tol: print(\"pass\")"`
    if [ "$qres" == "" ] ;
    then
    qres="fail"
    fi
    #echo "got $res expect $exp tolerance $tol => $qres"
    echo $qres

}


help ()
{
    echo " help : show this help"
    echo " start : start the test"
    echo " fims : read the fims capture"
    echo " log : read the go_metrics log file"
    echo " result : read the test result"
    echo " config : show config"
    echo " expect : show expected result (Not Yet Ready)"
}

menu()
{
if [ "$1"  ==  "" ] ; then
    help $@
fi

if [ "$1"  ==  "help" ] ; then
    help $@
fi

if [ "$1"  ==  "start" ] ; then
    start $@
fi

if [ "$1"  ==  "fims" ] ; then
    read_fims_log $@
fi

if [ "$1"  ==  "read_fims_nots" ] ; then
    read_fims_log_nots $@
fi

if [ "$1"  ==  "log" ] ; then
    read_run_log $@
fi

if [ "$1"  ==  "result" ] ; then
    read_fims_log $@
fi


if [ "$1"  ==  "config" ] ; then
    read_config $@
fi

if [ "$1"  ==  "expect" ] ; then
    expect $@
fi
}
