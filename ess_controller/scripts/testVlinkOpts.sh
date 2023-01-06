
#!/bin/sh

# p wilshire 03-2-2022

# Test the vlink opts function

/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/vlinks/ess '{
     "link1":{
        "setamap":true,
        "pname":"ess",
        "amname":"pcs",
        "value":"/test/vlink:link1", "vlink":"/comp/input:vlink1",
         "new_options": [
             {"value":"/test/vlink:link1_a"},
             {"value":"/test/vlink:link1_b"},
             {"value":"/test/vlink:link1_c"}

         ]
     }, 
     "link2":{
        "value":"/test/vlink:link2", "vlink":"/comp/input:vlink2",
        "setamap":false,
         "new_options": [
             {"value":"/test/vlink:link2_a"},
             {"value":"/test/vlink:link2_b"},
             {"value":"/test/vlink:link2_c", "vlink":"/test/vlink:link2_cv"}
         ]
     }
}' | jq

/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/control/ess '{
    "linkrun":{
        "value":"dummy",
        "debug":true, 
        "actions":{
            "onSet":[{"func":[{"func":"RunAllVLinks"}]}]
        }
     }
 }'    

/usr/local/bin/fims_send -m set   -u /ess/test/vlink/link1 0
/usr/local/bin/fims_send -m set   -u /ess/test/vlink/link2_b 0
/usr/local/bin/fims_send -m set   -u /ess/test/vlink/link2_cv 0

echo system at the start
/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/test/vlink | jq

echo set link1 to 245 link_2_b to 456  link2_cv to 345 
/usr/local/bin/fims_send -m set   -u /ess/test/vlink/link1 245 
/usr/local/bin/fims_send -m set   -u /ess/test/vlink/link2_b 456 
/usr/local/bin/fims_send -m set   -u /ess/test/vlink/link2_cv 345 

echo 

/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/test/vlink | jq
/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/comp/input | jq

/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/full/amap | jq
/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/full/amap/pcs | jq

#return 0


# # run twice to clear
# /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123
# /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123

# echo " initial inspect "
# /usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/testlimits  | jq

# echo " change temp to 57"
# /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/temp  57  | jq

# echo " test once"
# /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123

# echo "inspect"
# /usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/testlimits  | jq

# echo " test again"
# /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123

# echo "inspect again"
# /usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/testlimits  | jq
