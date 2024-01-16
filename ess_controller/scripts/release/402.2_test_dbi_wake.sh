#!/bin/sh
#
#   dbi_test
#
# this will update dbi every time the value is written with a change
# dbiStatus == "init" to also force a dbiWrite
# once that has been processes that value will change to "OK"
# EnableDbiUpdate must also be set to true
# if dbiStatus == "once"
#   EnableDbiUpdate will be set to false after a single update.
if [ -f /home/config/sim/setup.txt ] ; then
    . /home/config/sim/setup.txt
fi

if [ -f  ~/config_ess/install/configs/sim/setup.txt ] ; then 
    . ~/config_ess/install/configs/sim/setup.txt
    echo "using ~/config_ess/install/configs/sim/setup.txt"
fi


# load in the test setup defs...

#mkdir -p /home/config/tests/output

num_passed=0
num_failed=0
fails=()

tfile=./scripts/release/test_definitions.sh
. $tfile

rtest="402.2_test_dbi_wake"

# load in the test setup defs...
deffile=./scripts/release/${rtest}.txt
. $deffile

mkdir -p /var/log/tests/output
outfile=/var/log/tests/output/${rtest}_results.txt
logfile=/var/log/tests/output/${rtest}_log.txt

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

exit
