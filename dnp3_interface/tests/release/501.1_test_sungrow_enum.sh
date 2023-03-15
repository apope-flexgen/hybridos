#!/bin/sh
# p. wilshire
# 12/24/2021
#
#   test charge limiting
#
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

rtest="501.1_test_sungrow_enum"

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
