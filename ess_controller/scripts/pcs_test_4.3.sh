# test 4.3  Check read voltage
#run after test  4.2.3
COMP=/components/component_one
    PVAL=1001
    QVAL=501
#    while [ $HBVAL -le 60 ]
#    do
     #   HBVAL=$(( HBVAL+1 ))	 # increments $n
     #   /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"p_reference\":{\"value\":$PVAL}}"
    #	/usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"ems_cmd\":{\"value\":3}}"
    #    sleep 0.2
    #    /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"q_reference\":{\"value\":$QVAL}}"
    #	/usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"ems_cmd\":{\"value\":3}}"
    #    sleep 2
        echo -n "vdc_bus "&& /usr/local/bin/fims/fims_send -m get -u $COMP/vdc_bus -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        echo -n "active_power "&& /usr/local/bin/fims/fims_send -m get -u $COMP/active_power -r/$$ 
        echo -n "reactive_power "&& /usr/local/bin/fims/fims_send -m get -u $COMP/reactive_power -r/$$ 
        echo -n "dc_input_power_bess1 "&& /usr/local/bin/fims/fims_send -m get -u $COMP/dc_input_power_bess1 -r/$$ 
        echo -n "dc_input_current_bess1 "&& /usr/local/bin/fims/fims_send -m get -u $COMP/dc_input_current_bess1 -r/$$ 
        sleep 1
        echo
        echo
        echo " after 1 sec"
        echo -n "vdc_bus "&& /usr/local/bin/fims/fims_send -m get -u $COMP/vdc_bus -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        echo -n "active_power "&& /usr/local/bin/fims/fims_send -m get -u $COMP/active_power -r/$$ 
        echo -n "reactive_power "&& /usr/local/bin/fims/fims_send -m get -u $COMP/reactive_power -r/$$ 
        echo -n "dc_input_power_bess1 "&& /usr/local/bin/fims/fims_send -m get -u $COMP/dc_input_power_bess1 -r/$$ 
        echo -n "dc_input_current_bess1 "&& /usr/local/bin/fims/fims_send -m get -u $COMP/dc_input_current_bess1 -r/$$ 
    
 
echo "end of test---"


