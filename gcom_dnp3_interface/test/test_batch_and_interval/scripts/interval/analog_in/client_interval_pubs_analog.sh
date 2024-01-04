# run script on server
for i in {0..5}
do
  fims_send -m pub -u /analog/in/TestIntervalPubClient $i
  sleep 10
done
echo "done"