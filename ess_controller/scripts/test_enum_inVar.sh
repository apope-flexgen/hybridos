#!/bin/sh
# simple test for the enum basics
# 1<<12 = 4096
# 2<<12 = 8192
# 3<<12 = 12288
# 4<<12 = 16384
# 5<<12 = 20480

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum_out '
{
     "test32":0,
     "test24":0,
     "test4096":0
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "test_enum":{"value":0, 
     "amap":"ess",
     "ifChanged": false,
     "debug":1,
     "actions":{"onSet":[{"enum":[
          {"inValue":23, "mask":255, "uri" :"/test/enum_out:test23","amap":"ess"},
          {"inValue":24, "mask":255, "uri":"/test/enum_out:test24","amap":"ess"},
          {"inValue":4096, "mask":4096, "uri":"/test/enum_out:test4096","amap":"ess"}
          ]
            }]}}
}'

echo  we should just get all 0
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo  send 23
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":23}'

echo  we should just get 23 set as 23
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo  send 24
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":24}'

echo  we should now get 24 set as 24
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo  send 4096
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":4096}'

echo  we should now get 4096 set as 4096
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo now use outValue

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "test_enum":{"value":0, 
     "amap":"ess",
     "ifChanged": false,
     "debug":1,
     "actions":{"onSet":[{"enum":[
          {"inValue":23, "mask":255, "outValue":-23,"uri" :"/test/enum_out:test23","amap":"ess"},
          {"inValue":24, "mask":255, "outValue":-24,"uri":"/test/enum_out:test24","amap":"ess"},
          {"inValue":4096, "mask":4096, "outValue":-4096,"uri":"/test/enum_out:test4096","amap":"ess"}
          ]
            }]}}
}'
echo  send 23
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":23}'
echo  send 24
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":24}'
echo  send 4096
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":4096}'

echo  we should now get then all as -ve values
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo now check enabled
echo disable enable34 and try to change the value
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "enable24":{"value":true}
}' 

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "test_enum":{"value":0, 
     "amap":"ess",
     "ifChanged": false,
     "debug":1,
     "actions":{"onSet":[{"enum":[
          {"enabled":false, "inValue":23, "mask":255, "outValue":23,"uri" :"/test/enum_out:test23","amap":"ess"},
          {"enable":"/test/enum:enable24","inValue":24, "mask":255, "outValue":240,"uri":"/test/enum_out:test24","amap":"ess"},
          {"inValue":4096, "mask":4096, "outValue":4096,"uri":"/test/enum_out:test4096","amap":"ess"}
          ]
            }]}}
}'
echo  send 23
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":23}'
echo  send 24
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":24}'
echo  send 4096
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":4096}'
echo  we should now get then all as +ve values except 23 and test24 = 240
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo disable enable34 set test24 to 0 and try to change the value
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum_out '
{
 "test24":{"value":0}
}' 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "enable24":{"value":false}
}' 

echo  we should now get then all as +ve values except 23 and test24 = 0
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo  send 24
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":24}'

echo  we should now get then all as +ve values except 23 and test24 still = 0
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo now set enable24 true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "enable24":{"value":true}
}' 

echo  send 24
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":24}'

echo  we should now get then all as +ve values except 23 and test24 now = 240 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo test shift and mask
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '
{
 "test_enum":{"value":0, 
     "amap":"ess",
     "ifChanged": false,
     "debug":1,
     "actions":{"onSet":[{"enum":[
     {"enabled":false, "inValue":23, "mask":255, "outValue":23,"uri" :"/test/enum_out:test23","amap":"ess"},
     {"enable":"/test/enum:enable24","inValue":24, "mask":255, "outValue":240,"uri":"/test/enum_out:test24","amap":"ess"},
     {"inValue":4096, "mask":4096, "outValue":4096,"uri":"/test/enum_out:test4096","amap":"ess"},
     {"shift":2, "mask":1, "inValue":1, "outValue":"shift2mask1in1","uri":"/test/enum_out:testshift1","amap":"ess"},
     {"shift":3, "mask":3, "inValue":1, "outValue":"shift3mask3in1","uri":"/test/enum_out:testshift2","amap":"ess"},
     {"shift":3, "mask":3, "inValue":2, "outValue":"shift3mask3in2","uri":"/test/enum_out:testshift2","amap":"ess"}
     ]
  }]}}
}'

echo  send 4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":4}'

echo  we should now get testshift1 = shift2mask1in1
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo  send 8
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":8}'
echo clear testshift1
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum_out '{"testshift1":"ignore"}'


echo  we should now get testshift2 = shift3mask3in1
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

echo  send 16
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/enum '{"test_enum":16}'

echo  we should now get testshift2 = shift3mask3in2
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/enum_out | jq

exit



