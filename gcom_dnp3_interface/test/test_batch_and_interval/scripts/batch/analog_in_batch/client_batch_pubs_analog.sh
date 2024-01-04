# run script on server
for i in {0..1000}
do
  fims_send -m pub -u /analog/in/batch/TestBatchPubClient $i
done
echo "done"