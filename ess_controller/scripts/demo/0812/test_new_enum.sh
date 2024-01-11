#!/bin/sh
# simple test for the enum basics
# 1<<12 = 4096
# 2<<12 = 8192
# 3<<12 = 12288
# 4<<12 = 16384
# 5<<12 = 20480
#for i in {1..50}; do echo $i; done
#exit
SYS=/flex

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/site/ess_ls/sbmi_1_alarms 0 
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/site/ess_ls/sbmi_1_alarms 


/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/enum '
{
"sbmu_warning_1": {
            "value": 0,
            "note":"Sbmu Warning table with degrees reg 1 Appendix 3",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 3,"inValue": 0,"uri": "/alarms/sbmi_1:single_cell_over_volt", "outValue": "Normal"},
                        { "shift": 0,"mask": 3,"inValue": 1,"uri": "/alarms/sbmi_1:single_cell_over_volt", "outValue": "Warn 1"},
                        { "shift": 0,"mask": 3,"inValue": 2,"uri": "/alarms/sbmi_1:single_cell_over_volt", "outValue": "Warn 2"},
                        { "shift": 0,"mask": 3,"inValue": 3,"uri": "/alarms/sbmi_1:single_cell_over_volt", "outValue": "Warn 3"},

                        { "shift": 2,"mask": 3,"inValue": 0,"uri": "/alarms/sbmi_1:single_cell_under_volt", "outValue": "Normal"},
                        { "shift": 2,"mask": 3,"inValue": 1,"uri": "/alarms/sbmi_1:single_cell_under_volt", "outValue": "Warn 1"},
                        { "shift": 2,"mask": 3,"inValue": 2,"uri": "/alarms/sbmi_1:single_cell_under_volt", "outValue": "Warn 2"},
                        { "shift": 2,"mask": 3,"inValue": 3,"uri": "/alarms/sbmi_1:single_cell_under_volt", "outValue": "Warn 3"},

                        { "shift": 4,"mask": 3,"inValue": 0,"uri": "/alarms/sbmi_1:single_cell_over_temp", "outValue": "Normal"},
                        { "shift": 4,"mask": 3,"inValue": 1,"uri": "/alarms/sbmi_1:single_cell_over_temp", "outValue": "Warn 1"},
                        { "shift": 4,"mask": 3,"inValue": 2,"uri": "/alarms/sbmi_1:single_cell_over_temp", "outValue": "Warn 2"},
                        { "shift": 4,"mask": 3,"inValue": 3,"uri": "/alarms/sbmi_1:single_cell_over_temp", "outValue": "Warn 3"},

                        { "shift": 6,"mask": 3,"inValue": 0,"uri": "/alarms/sbmi_1:single_cell_under_temp", "outValue": "Normal"},
                        { "shift": 6,"mask": 3,"inValue": 1,"uri": "/alarms/sbmi_1:single_cell_under_temp", "outValue": "Warn 1"},
                        { "shift": 6,"mask": 3,"inValue": 2,"uri": "/alarms/sbmi_1:single_cell_under_temp", "outValue": "Warn 2"},
                        { "shift": 6,"mask": 3,"inValue": 3,"uri": "/alarms/sbmi_1:single_cell_under_temp", "outValue": "Warn 3"},

                        { "shift": 8,"mask": 3,"inValue": 0,"uri": "/alarms/sbmi_1:SOC_low", "outValue": "Normal"},
                        { "shift": 8,"mask": 3,"inValue": 1,"uri": "/alarms/sbmi_1:SOC_low", "outValue": "Warn 1"},
                        { "shift": 8,"mask": 3,"inValue": 2,"uri": "/alarms/sbmi_1:SOC_low", "outValue": "Warn 2"},
                        { "shift": 8,"mask": 3,"inValue": 3,"uri": "/alarms/sbmi_1:SOC_low", "outValue": "Warn 3"},

                        { "shift": 10,"mask": 3,"inValue": 0,"uri": "/alarms/sbmi_1:TMS_fault", "outValue": "Normal"},
                        { "shift": 10,"mask": 3,"inValue": 1,"uri": "/alarms/sbmi_1:TMS_fault", "outValue": "Warn 1"},
                        { "shift": 10,"mask": 3,"inValue": 2,"uri": "/alarms/sbmi_1:TMS_fault", "outValue": "Warn 2"},
                        { "shift": 10,"mask": 3,"inValue": 3,"uri": "/alarms/sbmi_1:TMS_fault", "outValue": "Warn 3"},


                        { "shift": 0,"mask": 3,"inValue": 0,"uri": "/alarms/bms:sbmi_1_single_cell_over_volt", "outValue": "Normal"},
                        { "shift": 0,"mask": 3,"inValue": 1,"uri": "/alarms/bms:sbmi_1_single_cell_over_volt", "outValue": "Warn 1"},
                        { "shift": 0,"mask": 3,"inValue": 2,"uri": "/alarms/bms:sbmi_1_single_cell_over_volt", "outValue": "Warn 2"},
                        { "shift": 0,"mask": 3,"inValue": 3,"uri": "/alarms/bms:sbmi_1_single_cell_over_volt", "outValue": "Warn 3"},

                        { "shift": 2,"mask": 3,"inValue": 0,"uri": "/alarms/bms:sbmi_1_single_cell_under_volt", "outValue": "Normal"},
                        { "shift": 2,"mask": 3,"inValue": 1,"uri": "/alarms/bms:sbmi_1_single_cell_under_volt", "outValue": "Warn 1"},
                        { "shift": 2,"mask": 3,"inValue": 2,"uri": "/alarms/bms:sbmi_1_single_cell_under_volt", "outValue": "Warn 2"},
                        { "shift": 2,"mask": 3,"inValue": 3,"uri": "/alarms/bms:sbmi_1_single_cell_under_volt", "outValue": "Warn 3"},

                        { "shift": 4,"mask": 3,"inValue": 0,"uri": "/alarms/bms:sbmi_1_single_cell_over_temp", "outValue": "Normal"},
                        { "shift": 4,"mask": 3,"inValue": 1,"uri": "/alarms/bms:sbmi_1_single_cell_over_temp", "outValue": "Warn 1"},
                        { "shift": 4,"mask": 3,"inValue": 2,"uri": "/alarms/bms:sbmi_1_single_cell_over_temp", "outValue": "Warn 2"},
                        { "shift": 4,"mask": 3,"inValue": 3,"uri": "/alarms/bms:sbmi_1_single_cell_over_temp", "outValue": "Warn 3"},

                        { "shift": 6,"mask": 3,"inValue": 0,"uri": "/alarms/bms:sbmi_1_single_cell_under_temp", "outValue": "Normal"},
                        { "shift": 6,"mask": 3,"inValue": 1,"uri": "/alarms/bms:sbmi_1_single_cell_under_temp", "outValue": "Warn 1"},
                        { "shift": 6,"mask": 3,"inValue": 2,"uri": "/alarms/bms:sbmi_1_single_cell_under_temp", "outValue": "Warn 2"},
                        { "shift": 6,"mask": 3,"inValue": 3,"uri": "/alarms/bms:sbmi_1_single_cell_under_temp", "outValue": "Warn 3"},

                        { "shift": 8,"mask": 3,"inValue": 0,"uri": "/alarms/bms:sbmi_1_SOC_low", "outValue": "Normal"},
                        { "shift": 8,"mask": 3,"inValue": 1,"uri": "/alarms/bms:sbmi_1_SOC_low", "outValue": "Warn 1"},
                        { "shift": 8,"mask": 3,"inValue": 2,"uri": "/alarms/bms:sbmi_1_SOC_low", "outValue": "Warn 2"},
                        { "shift": 8,"mask": 3,"inValue": 3,"uri": "/alarms/bms:sbmi_1_SOC_low", "outValue": "Warn 3"},

                        { "shift": 10,"mask": 3,"inValue": 0,"uri": "/alarms/bms:sbmi_1_TMS_fault", "outValue": "Normal"},
                        { "shift": 10,"mask": 3,"inValue": 1,"uri": "/alarms/bms:sbmi_1_TMS_fault", "outValue": "Warn 1"},
                        { "shift": 10,"mask": 3,"inValue": 2,"uri": "/alarms/bms:sbmi_1_TMS_fault", "outValue": "Warn 2"},
                        { "shift": 10,"mask": 3,"inValue": 3,"uri": "/alarms/bms:sbmi_1_TMS_fault", "outValue": "Warn 3"},


                        { "shift": 0,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:sbmi_1_alarms[0]", "outValue": true, "note": "Bit 0 - Single Cell Overvoltage Warning 1"},
                        { "shift": 0,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:sbmi_1_alarms[1]", "outValue": true, "note": "Bit 1 - Single Cell Overvoltage Warning 2"},
                        { "shift": 0,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:sbmi_1_alarms[2]", "outValue": true, "note": "Bit 2 - Single Cell Overvoltage Warning 3"},

                        { "shift": 2,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:sbmi_1_alarms[3]", "outValue": true, "note": "Bit 3 - Single Cell Undervoltage Warning 1"},
                        { "shift": 2,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:sbmi_1_alarms[4]", "outValue": true, "note": "Bit 4 - Single Cell Undervoltage Warning 2"},
                        { "shift": 2,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:sbmi_1_alarms[5]", "outValue": true, "note": "Bit 5 - Single Cell Undervoltage Warning 3"},

                        { "shift": 4,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:sbmi_1_alarms[6]", "outValue": true, "note": "Bit 6 - Single Cell Overtemperature Warning 1"},
                        { "shift": 4,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:sbmi_1_alarms[7]", "outValue": true, "note": "Bit 7 - Single Cell Overtemperature Warning 2"},
                        { "shift": 4,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:sbmi_1_alarms[8]", "outValue": true, "note": "Bit 8 - Single Cell Overtemperature Warning 3"},

                        { "shift": 6,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:sbmi_1_alarms[9]", "outValue": true, "note": "Bit 9 - Single Cell Undertemperature Warning 1"},
                        { "shift": 6,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:sbmi_1_alarms[10]", "outValue": true, "note": "Bit 10 - Single Cell Undertemperature Warning 2"},
                        { "shift": 6,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:sbmi_1_alarms[11]", "outValue": true, "note": "Bit 11 - Single Cell Undertemperature Warning 3"},

                        { "shift": 8,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:sbmi_1_alarms[12]", "outValue": true, "note": "Bit 12 - SOC Low Warning 1"},
                        { "shift": 8,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:sbmi_1_alarms[13]", "outValue": true, "note": "Bit 13 - SOC Low Warning 2"},
                        { "shift": 8,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:sbmi_1_alarms[14]", "outValue": true, "note": "Bit 14 - SOC Low Warning 3"},

                        { "shift": 10,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:sbmi_1_alarms[15]", "outValue": true, "note": "Bit 15 - TMS Fault Level 1"},
                        { "shift": 10,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:sbmi_1_alarms[16]", "outValue": true, "note": "Bit 16 - TMS Fault Level 2"},
                        { "shift": 10,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:sbmi_1_alarms[17]", "outValue": true, "note": "Bit 17 - TMS Fault Level 3"}
                    ]
           }]}}
}' | jq

#exit
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/site/ess_ls/sbmi_1_alarms 0 
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/site/ess_ls/sbmi_1_alarms 

for i in {1..50}; do 
 echo $i; 

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/enum "{ \"sbmu_warning_1\":$i }"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/naked/alarms/bms | jq 

done
echo
echo done with cyclic test 
echo -n "get site/ess_ls/sbmi_1_alarms  :>>"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/naked/site/ess_ls/sbmi_1_alarms 

echo
echo  "get alarms/bms  :>>"

/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/alarms/bms | jq 

exit

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



