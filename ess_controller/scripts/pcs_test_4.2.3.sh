COMP=/components/component_one
# continue until $n equals 5
#while [ 1 ]
#do
    PVAL=1001
    QVAL=501
#    while [ $HBVAL -le 60 ]
#    do
        HBVAL=$(( HBVAL+1 ))	 # increments $n
        /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"p_reference\":{\"value\":$PVAL}}"
    #	/usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"ems_cmd\":{\"value\":3}}"
        sleep 0.2
        /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"q_reference\":{\"value\":$QVAL}}"
    #	/usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"ems_cmd\":{\"value\":3}}"
        #sleep 2
        echo -n "q_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        sleep 0.2
        echo -n "q_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        sleep 0.2
        echo -n "q_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        sleep 0.2
        echo -n "q_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        sleep 0.2
        echo -n "q_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        sleep 0.2
        echo -n "q_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
        echo -n "p_reference "&& /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        sleep 1
    PVAL=1000
    QVAL=500
#    while [ $HBVAL -le 60 ]
#    do
        HBVAL=$(( HBVAL+1 ))	 # increments $n
        /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"p_reference\":{\"value\":$PVAL}}"
        /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"q_reference\":{\"value\":$QVAL}}"
    #	/usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"ems_cmd\":{\"value\":3}}"
        sleep 2
        echo -n "p_reference "&&        /usr/local/bin/fims/fims_send -m get -u $COMP/p_reference -r/$$ 
        echo -n "q_reference "&&/usr/local/bin/fims/fims_send -m get -u $COMP/q_reference -r/$$ 
#    done
        /usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"enable_main_selector\":{\"value\":1}}"
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        echo -n "inverter_status "&& /usr/local/bin/fims/fims_send -m get -u $COMP/inverter_status -r/$$ 
        sleep 0.2
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        echo -n "inverter_status "&& /usr/local/bin/fims/fims_send -m get -u $COMP/inverter_status -r/$$ 
        sleep 0.2
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        echo -n "inverter_status "&& /usr/local/bin/fims/fims_send -m get -u $COMP/inverter_status -r/$$ 
        sleep 0.2
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        echo -n "inverter_status "&& /usr/local/bin/fims/fims_send -m get -u $COMP/inverter_status -r/$$ 
        sleep 2
        echo " last check"
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        echo -n "inverter_status "&& /usr/local/bin/fims/fims_send -m get -u $COMP/inverter_status -r/$$ 
        sleep 0.2
        # 2 means off  #4 means   #19 means  #7 Means #6 means on
        ##/usr/local/bin/fims/fims_send -m set -u $COMP -r/$$ "{\"enable_main_selector\":{\"value\":0}}"
        sleep 0.2
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        sleep 2
        echo -n "enable_main_selector "&& /usr/local/bin/fims/fims_send -m get -u $COMP/enable_main_selector -r/$$ 
        echo -n "inverter_status "&& /usr/local/bin/fims/fims_send -m get -u $COMP/inverter_status -r/$$ 
        echo " test 4.2.3 completed invertor status should be 6"

 
#done

