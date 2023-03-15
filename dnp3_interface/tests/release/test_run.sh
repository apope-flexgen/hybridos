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