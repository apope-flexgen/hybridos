# run script on client
for i in {0..5}
do
  fims_send -m set -u /binary/out/TestIntervalSetServer true
  sleep 10
  fims_send -m set -u /binary/out/TestIntervalSetServer false
  sleep 10
done
echo "done"