# run script on client
for i in {0..500}
do
  fims_send -m set -u /binary/out/batch/TestBatchSetClient true
  fims_send -m set -u /binary/out/batch/TestBatchSetClient false
done
echo "done"