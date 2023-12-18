# run script on client
for i in {0..1000}
do
  fims_send -m set -u /analog/out/batch/TestBatchSetClient $i
done
echo "done"