# run script on client
for i in {0..5}
do
  fims_send -m set -u /analog/out/TestIntervalSetClient $i
  sleep 10
done
echo "done"