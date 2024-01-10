# run script on server
for i in {0..3}
do
  fims_send -m pub -u /binary/in/TestIntervalPubClient true
  sleep 10
  fims_send -m pub -u /binary/in/TestIntervalPubClient false
  sleep 10
done
echo "done"