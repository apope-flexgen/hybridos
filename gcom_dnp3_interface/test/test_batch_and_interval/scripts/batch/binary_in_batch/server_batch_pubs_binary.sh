# run script on server
for i in {0..500}
do
  fims_send -m pub -u /binary/in/batch/TestBatchPubServer true
  fims_send -m pub -u /binary/in/batch/TestBatchPubServer false
done
echo "done"