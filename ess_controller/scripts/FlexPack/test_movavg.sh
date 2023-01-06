/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '
{
 "test_movavg":{"value":0, 
     "amap":"flex",
     "depth":16,
     "vecAv":"/test/flex:testVec",
     "outAv":"/test/flex:outVec",
     "actions":{"onSet":[{                     
         "func":[{"func":"MathMovAvg","amap":"flex"}]
            }]}}
'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '
{
 "test_movavg":{"value":12.3}
}'
echo "should be 12.3"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/test/flex/outVec | jq
