# run script on server
for i in {0..500}
do
  fims_send -m pub -u /binary/in/batch/TestBatchPubClient true
  fims_send -m pub -u /binary/in/batch/TestBatchPubClient false
done
echo "done"