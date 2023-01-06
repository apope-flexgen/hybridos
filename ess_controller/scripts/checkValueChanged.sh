
#!/bin/sh

# p wilshire 03-15-2022

# Test the CheckValueChanged function

 /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/voltage  123  | jq
 /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/current  45  | jq
 /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/temp  56  | jq

 /usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged '{
     "value":false, 
     "actions":{
         "onSet":[{"func":[{"func":"CheckValueChanged"}]}]}
         } '  | jq

# run twice to clear
/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123
/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123

echo " initial inspect "
/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/testlimits  | jq

echo " change temp to 57"
/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/temp  57  | jq

echo " test once"
/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123

echo "inspect"
/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/testlimits  | jq

echo " test again"
/usr/local/bin/fims_send -m set  -r/x$$ -u /ess/testlimits/ess/CheckChanged 123

echo "inspect again"
/usr/local/bin/fims_send -m get  -r/x$$ -u /ess/naked/testlimits  | jq
