#!/bin/bash

getInstance(){
    printf "Type stop to stop testing. Would you like to test ess_1 or ess_2?\n"
    read instance
    if [[ $instance != "stop" && $instance != "ess_1" && $instance != "ess_2" ]]
    then
        printf "Make sure you type the command correctly.\n"
        getInstance
    fi
    printf "Starting testing with $instance.\n"
}

hey(){
    printf "Hey,\n"
}

dial(){
    printf "Working on $1.\n"
    counter=0
    startPoint=$( jq .$1 fakedComponents/components_$2.json )

    printf "Zeroing\n"
    jq --arg foo "$1" '.[$foo] = 0' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
    fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json

    printf "Dialing up $1.\n"
    while [ $counter -lt 5 ]
    do
        jq --arg foo "$1" '.[$foo] += 10' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        let "counter=counter+1"
        sleep .5
    done

    printf "Dialing down $1.\n"
    while [ $counter -gt 0 ]
    do
        jq --arg foo "$1" '.[$foo] -= 10' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        let "counter=counter-1"
        sleep .5
    done

    printf "Double setting to startpoint just in case.\n"
}

dialMega(){
    printf "Working on $1.\n"
    counter=0
    startPoint=$( jq .$1 fakedComponents/components_$2.json )

    printf "Zeroing\n"
    jq --arg foo "$1" '.[$foo] = 0' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
    fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json

    printf "Dialing up $1.\n"
    while [ $counter -lt 5 ]
    do
        jq --arg foo "$1" '.[$foo] += 1000' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        let "counter=counter+1"
        sleep .5
    done

    printf "Dialing down $1.\n"
    while [ $counter -gt 0 ]
    do
        jq --arg foo "$1" '.[$foo] -= 1000' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        let "counter=counter-1"
        sleep .5
    done

    printf "Double setting to startpoint just in case.\n"
}

dialBool(){
    printf "Working on $1.\n"
    counter=0
    startPoint=$( jq .$1 fakedComponents/components_$2.json )

    printf "Zeroing\n"
    jq --arg foo "$1" '.[$foo] = 0' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
    fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json

    printf "Flipping $1.\n"
    while [ $counter -lt 5 ]
    do
        jq --arg foo "$1" '.[$foo] += 1' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        sleep .5

        jq --arg foo "$1" '.[$foo] -= 1' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        sleep .5

        let "counter=counter+1"
    done

    printf "Double setting to startpoint just in case.\n"
}

dialStatus(){
    printf "Working on $1.\n"
    counter=0
    startPointValue=$( jq .$1.value fakedComponents/components_$2.json )
    startPointstring=$( jq .$1.string fakedComponents/components_$2.json )

    printf "Stopping\n"
    jq --arg foo "$1" '.[$foo].value = 0' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
    jq --arg foo "$1" '.[$foo].string = "Stopped"' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
    fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json

    printf "Flipping $1.\n"
    while [ $counter -lt 5 ]
    do
        jq --arg foo "$1" '.[$foo].value = 0' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        jq --arg foo "$1" '.[$foo].string = "Stopped"' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        sleep .5

        jq --arg foo "$1" '.[$foo].value = 1' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        jq --arg foo "$1" '.[$foo].string = "Running"' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
        fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
        sleep .5

        let "counter=counter+1"
    done

    printf "Double setting to startpoint just in case.\n"
}
set100(){
    printf "Setting to 100.\n"
    jq --arg foo "$1" '.[$foo] = 100' fakedComponents/components_$2.json > temp.json && mv temp.json fakedComponents/components_$2.json
    fims_send -m pub -u /components/$2 -f fakedComponents/components_$2.json
}

hey
getInstance

while [ $instance != "stop" ]
do

    if [ $instance == "ess_1" ]
    then
        for status in "pcs_active_power" "active_power_setpoint" "pcs_apparent_power" "Auto Balancing Status" "component_connected" "pcs_current_l1" "pcs_current_l2" "pcs_current_l3" "dc_contactor_closed" "pcs_frequency" "frequency_setpoint" "on_off_grid_mode" "bms_num_racks_online" "bms_max_cell_temp" "bms_min_cell_temp" "pcs_reactive_power" "reactive_power_setpoint" "bms_soc" "bms_soh" "pcs_working_state" "chargeable_energy" "chargeable_power" "dischargeable_energy" "dischargeable_power" "pcs_voltage_l1_l2" "pcs_voltage_l2_l3" "pcs_voltage_l3_l1" "voltage_setpoint"
        do
            if [[ $status == "pcs_active_power" || $status == "active_power_setpoint" || $status == "pcs_apparent_power" || $status == "pcs_reactive_power" || $status == "reactive_power_setpoint" || $status == "chargeable_power" || $status == "dischargeable_power" || $status == "pcs_voltage_l1_l2" || $status == "pcs_voltage_l2_l3" || $status == "pcs_voltage_l3_l1" || $status == "voltage_setpoint" || $status == "chargeable_energy" || $status == "dischargeable_energy" || $status == "pcs_current_l1" || $status == "pcs_current_l2" || $status == "pcs_current_l3" ]]
            then
                dialMega $status ess_twins
            fi

            if [[ $status == "pcs_frequency" || $status == "frequency_setpoint" || $status == "bms_num_racks_online" || $status == "bms_max_cell_temp" || $status == "bms_min_cell_temp" || $status == "bms_soc" || $status == "pcs_voltage_dc" || $status == "bms_soh" ]]
            then
                dial $status ess_twins
                if [[ $status == "bms_soh" ]]
                then
                    set100 $status ess_twins
                fi
            fi

            if [[ $status == "component_connected" || $status == "dc_contactor_closed" || $status == "on_off_grid_mode" ]]
            then
                dialBool $status ess_twins
            fi

        done

    fi

    if [ $instance == "ess_2" ]
    then
        for status in "active_power" "active_power_setpoint" "pcs_apparent_power" "Auto Balancing Status" "component_connected" "pcs_current_l1" "pcs_current_l2" "pcs_current_l3" "dc_contactor_closed" "pcs_frequency" "frequency_setpoint" "on_off_grid_mode" "bms_num_racks_online" "bms_max_cell_temp" "bms_min_cell_temp" "pcs_reactive_power" "reactive_power_setpoint" "bms_soc" "bms_soh" "pcs_working_state" "chargeable_energy" "chargeable_power" "dischargeable_energy" "dischargeable_power" "pcs_voltage_l1_l2" "pcs_voltage_l2_l3" "pcs_voltage_l3_l1" "voltage_setpoint"
        do
            filter="has(\"$status\") fakedComponents/components_ess_real_hs.json"
            inHighSpeed=$( jq $filter ) 
 
            if [[ $status == "active_power" || $status == "active_power_setpoint" || $status == "pcs_apparent_power" || $status == "pcs_reactive_power" || $status == "reactive_power_setpoint" || $status == "chargeable_power" || $status == "dischargeable_power" || $status == "pcs_voltage_l1_l2" || $status == "pcs_voltage_l2_l3" || $status == "pcs_voltage_l3_l1" || $status == "voltage_setpoint" || $status == "chargeable_energy" || $status == "dischargeable_energy" || $status == "pcs_current_l1" || $status == "pcs_current_l2" || $status == "pcs_current_l3" ]]
            then
               if [[ $inHighSpeed == "true" ]]
                then
                    dialMega $status ess_real_hs
                else
                    dialMega $status ess_real_ls
                fi
            fi

            if [[ $status == "pcs_frequency" || $status == "frequency_setpoint" || $status == "bms_num_racks_online" || $status == "bms_max_cell_temp" || $status == "bms_min_cell_temp" || $status == "bms_soc" || $status == "pcs_voltage_dc" || $status == "bms_soh" ]]
            then
                if [[ $inHighSpeed == "true" ]]
                then
                    dial $status ess_real_hs
                    if [[ $status == "bms_soh" ]]
                    then
                        set100 $status ess_real_hs
                    fi
                else
                    dial $status ess_real_ls
                    if [[ $status == "bms_soh" ]]
                    then
                        set100 $status ess_real_ls
                    fi
                fi
            fi

            if [[ $status == "component_connected" || $status == "dc_contactor_closed" || $status == "on_off_grid_mode" ]]
            then
                if [[ $inHighSpeed == "true" ]]
                then
                    dialBool $status ess_real_hs
                else
                    dialBool $status ess_real_ls
                fi
            fi

        done
    fi

    getInstance
done