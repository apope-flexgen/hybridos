#!/bin/sh
# test the clone system from a script        
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/bms '
      {"getStatus":{"value":"dummy","enabled":false,"actions": {"onSet":[{"func":[
          {"func":"GetSbmuStatus","amap":"sbmu_1"}
           ]}]}}}} 
        '


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
      {"clone":{"value":"dummy","enabled":true,
                "actions": {"onSet":[{"clone":[
                    {"replace":"##cell##",   "with":"%02d", "from":1, "to": 4},
                    {"replace":"##module##", "with":"%d",   "from":1, "to": 8},
                    {"replace":"##sbmu##",   "with":"%d",   "from":1, "to": 4}
                ]}]},
                "src":      "file:battery_template.json",
                "dest":     "file:/tmp/battery_clone2.json",
                "options":  [{"@@BAT_ID@@":"@#"},{"@@BATT_VALUE@@":"@value"}]}}}
                ' 

