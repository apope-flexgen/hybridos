# run script on client
for i in {0..500}
do
  fims_send -m set -u /binary/out/batch/TestBatchSetServer true
  fims_send -m set -u /binary/out/batch/TestBatchSetServer false
done
echo "done"