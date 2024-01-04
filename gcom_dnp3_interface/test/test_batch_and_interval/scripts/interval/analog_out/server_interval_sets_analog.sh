# run script on client
for i in {0..5}
do
  fims_send -m set -u /analog/out/TestIntervalSetServer $i
  sleep 10
done
echo "done"