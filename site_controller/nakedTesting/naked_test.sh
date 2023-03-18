#!/bin/bash
touch output.txt

fims_set_getStatus(){
    fims_send -m set -u /assets/$1/$2 $3
    sleep 2
    fims_send -m get -u /assets/$1/status -r /reply
}

fims_set_getBreakerStatus(){
    fims_send -m set -u /assets/$1/$2 $3
    sleep 2
    fims_send -m get -u /assets/$1/status -r /reply
}

fims_set_getValue(){
    fims_send -m set -u /assets/$1/$2 $3
    sleep 2 
    fims_send -m get -u /assets/$1/$2 -r /reply
}
printf "What asset would you like to test?\n"
printf "Assets:\n"
printf "    -ess\n"
printf "    -feeder\n"
printf "    -features\n"
printf "Type stop to end testing.\n"
read asset

while [ $asset != "stop" ]
do

if [ $asset == "ess" ]
then

<< ess
"clear_faults"
"start"
"stop"
"enter_standby"
"exit_standby"
"limits_override"
"autobalancing_enable" is not tested currently. This feature is depreciated. (despite being visual on the ui)
"autobalancing_disable" is not tested currently. This feature is depreciated. (despite being visual on the ui)
"maint_active_power_setpoint"
"maint_reactive_power_setpoint"
"open_dc_contactors"
"close_dc_contactors"
ess

#put ess_1 into maint
    printf "Working with ess_1\n"
    fims_send -m set -u /assets/ess/ess_1/maint_mode true
    sleep 2

    for endpoint in "start" "stop" "start" "maint_active_power_setpoint" "maint_reactive_power_setpoint" "stop" "limits_override";
    do
        if [[ "$endpoint" == "start" ]]
        then
            fims_set_getStatus ess/ess_1 $endpoint true > currentTest.txt
            printf "    Start\n             "
            ./valueParse currentTest.txt Running
        fi
        if [[ "$endpoint" == "stop" ]]
            then
            fims_set_getStatus ess/ess_1 $endpoint true > currentTest.txt
            printf "    Stop\n             "
            ./valueParse currentTest.txt Stopped
        fi

        if [[ "$endpoint" == "maint_active_power_setpoint" || "$endpoint" == "maint_reactive_power_setpoint" ]]
        then
            fims_set_getValue ess/ess_1 $endpoint 4250.25 > currentTest.txt
            printf "    maint_active_power_setpoint/maint_reactive_power_setpoint to 4250.25\n             "
            ./valueParse currentTest.txt 4250.25
            fims_set_getValue ess/ess_1 $endpoint 1000 > currentTest.txt
            printf "    maint_active_power_setpoint/maint_reactive_power_setpoint to 1000\n             "
            ./valueParse currentTest.txt 1000
        fi

        if [[  "$endpoint" == "limits_override" ]]
        then
            fims_set_getValue ess/ess_1 $endpoint true > currentTest.txt
            printf "    limits_override to true\n             "
            ./valueParse currentTest.txt true
            fims_set_getValue ess/ess_1 $endpoint false > currentTest.txt
            printf "    limits_override to false\n             "
            ./valueParse currentTest.txt false
        fi

    done

    fims_send -m set -u /assets/ess/ess_1/maint_mode false
fi

if [ $asset == "feeder" ]
then

<< feeder
"breaker_close"
"breaker_close_permissive"
"breaker_close_permissive_remove"
"breaker_open"
"breaker_reset"
feeder

    printf "Working with feeder\n"
    fims_send -m set -u /assets/feeders/feed_1/maint_mode true

    for endpoint in  "breaker_open" "breaker_close";
    #need to add test for "breaker_reset"
    do
        if [ $endpoint == "breaker_open" ]
        then
            fims_send -m set -u /assets/feeders/feed_1/breaker_open true
            sleep 2
            fims_send -m get -u /assets/feeders/summary/feed_1_breaker_status -r /J > currentTest.txt
            printf "    Open breaker\n          "
            ./valueParse currentTest.txt Open
        else
            fims_send -m set -u /assets/feeders/feed_1/breaker_close true
            sleep 2
            fims_send -m get -u /assets/feeders/summary/feed_1_breaker_status -r /J > currentTest.txt
            printf "    Closed breaker\n          "
            ./valueParse currentTest.txt Closed

        fi

    done

    fims_send -m set -u /assets/feeders/feed_1/maint_mode false
fi

if [ $asset == "features" ]
then
    printf "Working on numeric sets\n"
    printf "Feature sets-\n"
    printf "    active_power\n"

    printf "        manual_solar_kW_cmd\n           "
    fims_send -m set -u /features/active_power/manual_solar_kW_cmd -- -12
    fims_send -m get -u /features/active_power/manual_solar_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1
    printf "        manual_ess_kW_cmd\n             "
    fims_send -m set -u /features/active_power/manual_ess_kW_cmd 12
    fims_send -m get -u /features/active_power/manual_ess_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        grid_target_kw_cmd\n             "
    fims_send -m set -u /features/active_power/grid_target_kW_cmd -- -12
    fims_send -m get -u /features/active_power/grid_target_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        export_target_kw_cmd\n             "
    fims_send -m set -u /features/active_power/export_target_kW_cmd 12
    fims_send -m get -u /features/active_power/export_target_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        export_target_kW_slew_rate\n             "
    fims_send -m set -u /features/active_power/export_target_kW_slew_rate -- 20
    fims_send -m get -u /features/active_power/export_target_kW_slew_rate -r /reply > currentTest.txt
    ./valueParse currentTest.txt 20
    sleep 0.1

    printf "        fr_site_nominal_hz\n             "
    fims_send -m set -u /features/active_power/fr_site_nominal_hz 12
    fims_send -m get -u /features/active_power/fr_site_nominal_hz -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_offset_hz\n             "
    fims_send -m set -u /features/active_power/fr_offset_hz -- -12
    fims_send -m get -u /features/active_power/fr_offset_hz -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        fr_baseload_kw_cmd\n             "
    fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 12
    fims_send -m get -u /features/active_power/fr_baseload_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_inactive_kw_cmd\n             "
    fims_send -m set -u /features/active_power/fr_inactive_kW_cmd -- -12
    fims_send -m get -u /features/active_power/fr_inactive_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        fr_total_kw_cmd\n             "
    fims_send -m set -u /features/active_power/fr_total_kW_cmd 12
    fims_send -m get -u /features/active_power/fr_total_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_UF_active_kW_cmd\n             "
    fims_send -m set -u /features/active_power/fr_UF_active_kW_cmd -- -12
    fims_send -m get -u /features/active_power/fr_UF_active_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        fr_UF_droop_percent\n             "
    fims_send -m set -u /features/active_power/fr_UF_droop_percent 12
    fims_send -m get -u /features/active_power/fr_UF_droop_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_UF_hz_trigger_percent\n             "
    fims_send -m set -u /features/active_power/fr_UF_hz_trigger_percent -- -12
    fims_send -m get -u /features/active_power/fr_UF_hz_trigger_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        fr_UF_hz_recover_percent\n             "
    fims_send -m set -u /features/active_power/fr_UF_hz_recover_percent 12
    fims_send -m get -u /features/active_power/fr_UF_hz_recover_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_UF_hz_instant_recover_percent\n             "
    fims_send -m set -u /features/active_power/fr_UF_hz_instant_recover_percent -- -12
    fims_send -m get -u /features/active_power/fr_UF_hz_instant_recover_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        fr_UF_slew_rate\n             "
    fims_send -m set -u /features/active_power/fr_UF_slew_rate 12
    fims_send -m get -u /features/active_power/fr_UF_slew_rate -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        fr_UF_trigger_time\n             "
    fims_send -m set -u /features/active_power/fr_UF_trigger_time 12
    fims_send -m get -u /features/active_power/fr_UF_trigger_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        fr_UF_cooldown_time\n             "
    fims_send -m set -u /features/active_power/fr_UF_cooldown_time 12
    fims_send -m get -u /features/active_power/fr_UF_cooldown_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        fr_UF_recover_time\n             "
    fims_send -m set -u /features/active_power/fr_UF_recover_time 12
    fims_send -m get -u /features/active_power/fr_UF_recover_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1


    printf "        fr_OF_active_kW_cmd\n             "
    fims_send -m set -u /features/active_power/fr_OF_active_kW_cmd 12
    fims_send -m get -u /features/active_power/fr_OF_active_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_OF_droop_percent\n             "
    fims_send -m set -u /features/active_power/fr_OF_droop_percent 12
    fims_send -m get -u /features/active_power/fr_OF_droop_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_OF_hz_trigger_percent\n             "
    fims_send -m set -u /features/active_power/fr_OF_hz_trigger_percent 12
    fims_send -m get -u /features/active_power/fr_OF_hz_trigger_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_OF_hz_recover_percent\n             "
    fims_send -m set -u /features/active_power/fr_OF_hz_recover_percent 12
    fims_send -m get -u /features/active_power/fr_OF_hz_recover_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_OF_hz_instant_recover_percent\n             "
    fims_send -m set -u /features/active_power/fr_OF_hz_instant_recover_percent 12
    fims_send -m get -u /features/active_power/fr_OF_hz_instant_recover_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        fr_OF_slew_rate\n             "
    fims_send -m set -u /features/active_power/fr_OF_slew_rate 12
    fims_send -m get -u /features/active_power/fr_OF_slew_rate -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        fr_OF_trigger_time\n             "
    fims_send -m set -u /features/active_power/fr_OF_trigger_time 12
    fims_send -m get -u /features/active_power/fr_OF_trigger_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        fr_OF_cooldown_time\n             "
    fims_send -m set -u /features/active_power/fr_OF_cooldown_time 12
    fims_send -m get -u /features/active_power/fr_OF_cooldown_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        fr_OF_recover_time\n             "
    fims_send -m set -u /features/active_power/fr_OF_recover_time 12
    fims_send -m get -u /features/active_power/fr_OF_recover_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        threshold_charge_1\n             "
    fims_send -m set -u /features/active_power/threshold_charge_1 12
    fims_send -m get -u /features/active_power/threshold_charge_1 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        threshold_charge_2\n             "
    fims_send -m set -u /features/active_power/threshold_charge_2 12
    fims_send -m get -u /features/active_power/threshold_charge_2 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        threshold_dischg_1\n             "
    fims_send -m set -u /features/active_power/threshold_dischg_1 12
    fims_send -m get -u /features/active_power/threshold_dischg_1 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        threshold_dischg_2\n             "
    fims_send -m set -u /features/active_power/threshold_dischg_2 12
    fims_send -m get -u /features/active_power/threshold_dischg_2 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        max_charge_1\n             "
    fims_send -m set -u /features/active_power/max_charge_1 12
    fims_send -m get -u /features/active_power/max_charge_1 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        max_charge_2\n             "
    fims_send -m set -u /features/active_power/max_charge_2 12
    fims_send -m get -u /features/active_power/max_charge_2 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        max_dischg_1\n             "
    fims_send -m set -u /features/active_power/max_dischg_1 12
    fims_send -m get -u /features/active_power/max_dischg_1 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        max_dischg_2\n             "
    fims_send -m set -u /features/active_power/max_dischg_2 12
    fims_send -m get -u /features/active_power/max_dischg_2 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        price\n             "
    fims_send -m set -u /features/active_power/price 12
    fims_send -m get -u /features/active_power/price -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

<<potentialbug
    printf "        pfr_limits_min_kW\n             "
    fims_send -m set -u /features/active_power/pfr_limits_min_kW 12
    fims_send -m get -u /features/active_power/pfr_limits_min_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
 
    printf "        pfr_limits_max_kW\n             "
    fims_send -m set -u /features/active_power/pfr_limits_max_kW 12
    fims_send -m get -u /features/active_power/pfr_limits_max_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
potentialbug

    printf "        absolute_ess_kW_cmd\n             "
    fims_send -m set -u /features/active_power/absolute_ess_kW_cmd 12
    fims_send -m get -u /features/active_power/absolute_ess_kW_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        ess_charge_control_target_soc\n             "
    fims_send -m set -u /features/active_power/ess_charge_control_target_soc 12
    fims_send -m get -u /features/active_power/ess_charge_control_target_soc -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        ess_charge_control_kW_limit\n             "
    fims_send -m set -u /features/active_power/ess_charge_control_kW_limit 12
    fims_send -m get -u /features/active_power/ess_charge_control_kW_limit -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "    reactive power\n"
 
    printf "        runmode1_kVAR_mode_cmd\n             "
    fims_send -m set -u /features/reactive_power/runmode1_kVAR_mode_cmd 2
    fims_send -m get -u /features/reactive_power/runmode1_kVAR_mode_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 2
    sleep 0.1
 
    printf "        reactive_setpoint_kVAR_cmd\n             "
    fims_send -m set -u /features/reactive_power/reactive_setpoint_kVAR_cmd 12
    fims_send -m get -u /features/reactive_power/reactive_setpoint_kVAR_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        active_voltage_cmd\n             "
    fims_send -m set -u /features/reactive_power/active_voltage_cmd 12
    fims_send -m get -u /features/reactive_power/active_voltage_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        active_voltage_deadband\n             "
    fims_send -m set -u /features/reactive_power/active_voltage_deadband 12
    fims_send -m get -u /features/reactive_power/active_voltage_deadband -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
 
    printf "        active_voltage_droop_percent\n             "
    fims_send -m set -u /features/reactive_power/active_voltage_droop_percent 12
    fims_send -m get -u /features/reactive_power/active_voltage_droop_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        power_factor_cmd\n             "
    fims_send -m set -u /features/reactive_power/power_factor_cmd 0.55
    fims_send -m get -u /features/reactive_power/power_factor_cmd -r /reply > currentTest.txt
    ./valueParse currentTest.txt 0.55
    sleep 0.1

    printf "    standalone_power\n"

    printf "        poi_limits_max_kW\n             "
    fims_send -m set -u /features/standalone_power/poi_limits_max_kW 12
    fims_send -m get -u /features/standalone_power/poi_limits_max_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        poi_limits_min_kW\n             "
    fims_send -m set -u /features/standalone_power/poi_limits_min_kW 12
    fims_send -m get -u /features/standalone_power/poi_limits_min_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        soc_poi_target_soc\n             "
    fims_send -m set -u /features/standalone_power/soc_poi_target_soc 12
    fims_send -m get -u /features/standalone_power/soc_poi_target_soc -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        soc_poi_limits_low_min_kW\n             "
    fims_send -m set -u /features/standalone_power/soc_poi_limits_low_min_kW 12
    fims_send -m get -u /features/standalone_power/soc_poi_limits_low_min_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        soc_poi_limits_low_max_kW\n             "
    fims_send -m set -u /features/standalone_power/soc_poi_limits_low_max_kW 12
    fims_send -m get -u /features/standalone_power/soc_poi_limits_low_max_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        soc_poi_limits_high_min_kW\n             "
    fims_send -m set -u /features/standalone_power/soc_poi_limits_high_min_kW 12
    fims_send -m get -u /features/standalone_power/soc_poi_limits_high_min_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt -12
    sleep 0.1

    printf "        soc_poi_limits_high_max_kW\n             "
    fims_send -m set -u /features/standalone_power/soc_poi_limits_high_max_kW 12
    fims_send -m get -u /features/standalone_power/soc_poi_limits_high_max_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        pfr_droop_percent\n             "
    fims_send -m set -u /features/standalone_power/pfr_droop_percent 12
    fims_send -m get -u /features/standalone_power/pfr_droop_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        pfr_deadband\n             "
    fims_send -m set -u /features/standalone_power/pfr_deadband 12
    fims_send -m get -u /features/standalone_power/pfr_deadband -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_priority_setting\n             "
    fims_send -m set -u /features/standalone_power/ldss_priority_setting 12
    fims_send -m get -u /features/standalone_power/ldss_priority_setting -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_warmup_time\n             "
    fims_send -m set -u /features/standalone_power/ldss_warmup_time 12
    fims_send -m get -u /features/standalone_power/ldss_warmup_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_cooldown_time\n             "
    fims_send -m set -u /features/standalone_power/ldss_cooldown_time 12
    fims_send -m get -u /features/standalone_power/ldss_cooldown_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_start_gen_time\n             "
    fims_send -m set -u /features/standalone_power/ldss_start_gen_time 12
    fims_send -m get -u /features/standalone_power/ldss_start_gen_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_stop_gen_time\n             "
    fims_send -m set -u /features/standalone_power/ldss_stop_gen_time 12
    fims_send -m get -u /features/standalone_power/ldss_stop_gen_time -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_max_load_threshold_percent\n             "
    fims_send -m set -u /features/standalone_power/ldss_max_load_threshold_percent 12
    fims_send -m get -u /features/standalone_power/ldss_max_load_threshold_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        ldss_min_load_threshold_percent\n             "
    fims_send -m set -u /features/standalone_power/ldss_min_load_threshold_percent 12
    fims_send -m get -u /features/standalone_power/ldss_min_load_threshold_percent -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

<<no
    printf "        start_first_gen_soc\n             "
    fims_send -m set -u /features/standalone_power/start_first_gen_soc 12
    fims_send -m get -u /features/standalone_power/start_first_gen_soc -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        edp_soc\n             "
    fims_send -m set -u /features/standalone_power/edp_soc 12
    fims_send -m get -u /features/standalone_power/edp_soc -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
no

    printf "        _closed_loop_control_manual_offset\n             "
    fims_send -m set -u /features/standalone_power/_closed_loop_control_manual_offset 12
    fims_send -m get -u /features/standalone_power/_closed_loop_control_manual_offset -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "    site_operation\n"

<<watchdog
    printf "        watchdog_pet\n             "
    fims_send -m set -u /features/site_operation/watchdog_pet 12
    fims_send -m get -u /features/site_operation/watchdog_pet -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        watchdog_duration_ms\n             "
    fims_send -m set -u /features/site_operation/watchdog_duration_ms 5000
    fims_send -m get -u /features/site_operation/watchdog_duration_ms -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1
watchdog

    printf "Site sets-\n"

    printf "    cops\n"
    printf "        Only set is heartbeat and I'm not testing it\n"

    printf "    configuration\n"

    printf "        reserved_float_1\n             "
    fims_send -m set -u /site/configuration/reserved_float_1 12
    fims_send -m get -u /site/configuration/reserved_float_1 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_2\n             "
    fims_send -m set -u /site/configuration/reserved_float_2 12
    fims_send -m get -u /site/configuration/reserved_float_2 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_3\n             "
    fims_send -m set -u /site/configuration/reserved_float_3 12
    fims_send -m get -u /site/configuration/reserved_float_3 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_4\n             "
    fims_send -m set -u /site/configuration/reserved_float_4 12
    fims_send -m get -u /site/configuration/reserved_float_4 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_5\n             "
    fims_send -m set -u /site/configuration/reserved_float_5 12
    fims_send -m get -u /site/configuration/reserved_float_5 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_6\n             "
    fims_send -m set -u /site/configuration/reserved_float_6 12
    fims_send -m get -u /site/configuration/reserved_float_6 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_7\n             "
    fims_send -m set -u /site/configuration/reserved_float_7 12
    fims_send -m get -u /site/configuration/reserved_float_7 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        reserved_float_8\n             "
    fims_send -m set -u /site/configuration/reserved_float_8 12
    fims_send -m get -u /site/configuration/reserved_float_8 -r /reply > currentTest.txt
    ./valueParse currentTest.txt 12
    sleep 0.1

    printf "        debug/state needs research\n"

    printf "\n\nMoving onto boolean sets-\n"
    printf "Features sets-\n"

    printf "    active_power\n"

    printf "        fr_UF_cooldown_status\n             "
    fims_send -m set -u /features/active_power/fr_UF_cooldown_status true
    fims_send -m get -u /features/active_power/fr_UF_cooldown_status -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_UF_droop_limit_flag\n             "
    fims_send -m set -u /features/active_power/fr_UF_droop_limit_flag true
    fims_send -m get -u /features/active_power/fr_UF_droop_limit_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_UF_slew_override_flag\n             "
    fims_send -m set -u /features/active_power/fr_UF_slew_override_flag true
    fims_send -m get -u /features/active_power/fr_UF_slew_override_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_UF_enable_flag\n             "
    fims_send -m set -u /features/active_power/fr_UF_enable_flag true
    fims_send -m get -u /features/active_power/fr_UF_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_UF_recover_latch\n             "
    fims_send -m set -u /features/active_power/fr_UF_recover_latch true
    fims_send -m get -u /features/active_power/fr_UF_recover_latch -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_UF_status_flag\n             "
    fims_send -m set -u /features/active_power/fr_UF_status_flag true
    fims_send -m get -u /features/active_power/fr_UF_status_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_UF_droop_bypass_flag\n             "
    fims_send -m set -u /features/active_power/fr_UF_droop_bypass_flag true
    fims_send -m get -u /features/active_power/fr_UF_droop_bypass_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_cooldown_status\n             "
    fims_send -m set -u /features/active_power/fr_OF_cooldown_status true
    fims_send -m get -u /features/active_power/fr_OF_cooldown_status -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_droop_limit_flag\n             "
    fims_send -m set -u /features/active_power/fr_OF_droop_limit_flag true
    fims_send -m get -u /features/active_power/fr_OF_droop_limit_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_slew_override_flag\n             "
    fims_send -m set -u /features/active_power/fr_OF_slew_override_flag true
    fims_send -m get -u /features/active_power/fr_OF_slew_override_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_enable_flag\n             "
    fims_send -m set -u /features/active_power/fr_OF_enable_flag true
    fims_send -m get -u /features/active_power/fr_OF_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_recover_latch\n             "
    fims_send -m set -u /features/active_power/fr_OF_recover_latch true
    fims_send -m get -u /features/active_power/fr_OF_recover_latch -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_status_flag\n             "
    fims_send -m set -u /features/active_power/fr_OF_status_flag true
    fims_send -m get -u /features/active_power/fr_OF_status_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        fr_OF_droop_bypass_flag\n             "
    fims_send -m set -u /features/active_power/fr_OF_droop_bypass_flag true
    fims_send -m get -u /features/active_power/fr_OF_droop_bypass_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        absolute_ess_direction_flag\n             "
    fims_send -m set -u /features/active_power/absolute_ess_direction_flag true
    fims_send -m get -u /features/active_power/absolute_ess_direction_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        ess_charge_control_charge_disable\n             "
    fims_send -m set -u /features/active_power/ess_charge_control_charge_disable true
    fims_send -m get -u /features/active_power/ess_charge_control_charge_disable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        ess_charge_control_discharge_disable\n             "
    fims_send -m set -u /features/active_power/ess_charge_control_discharge_disable true
    fims_send -m get -u /features/active_power/ess_charge_control_discharge_disable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        charge_dispatch_solar_enable_flag\n             "
    fims_send -m set -u /features/active_power/charge_dispatch_solar_enable_flag true
    fims_send -m get -u /features/active_power/charge_dispatch_solar_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        charge_dispatch_gen_enable_flag\n             "
    fims_send -m set -u /features/active_power/charge_dispatch_gen_enable_flag true
    fims_send -m get -u /features/active_power/charge_dispatch_gen_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        charge_dispatch_feeder_enable_flag\n             "
    fims_send -m set -u /features/active_power/charge_dispatch_feeder_enable_flag true
    fims_send -m get -u /features/active_power/charge_dispatch_feeder_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        charge_dispatch_solar_dischg_priority_flag\n             "
    fims_send -m set -u /features/active_power/charge_dispatch_solar_dischg_priority_flag true
    fims_send -m get -u /features/active_power/charge_dispatch_solar_dischg_priority_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        target_soc_load_enable_flag\n             "
    fims_send -m set -u /features/active_power/target_soc_load_enable_flag true
    fims_send -m get -u /features/active_power/target_soc_load_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        export_target_load_enable_flag\n             "
    fims_send -m set -u /features/active_power/export_target_load_enable_flag true
    fims_send -m get -u /features/active_power/export_target_load_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "    standalone_power\n"

    printf "        poi_limits_enable\n             "
    fims_send -m set -u /features/standalone_power/poi_limits_enable true
    fims_send -m get -u /features/standalone_power/poi_limits_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        soc_poi_limits_enable\n             "
    fims_send -m set -u /features/standalone_power/soc_poi_limits_enable true
    fims_send -m get -u /features/standalone_power/soc_poi_limits_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        pfr_enable_flag\n             "
    fims_send -m set -u /features/standalone_power/pfr_enable_flag true
    fims_send -m get -u /features/standalone_power/pfr_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        watt_watt_adjustment_enable_flag\n             "
    fims_send -m set -u /features/standalone_power/watt_watt_adjustment_enable_flag true
    fims_send -m get -u /features/standalone_power/watt_watt_adjustment_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        ldss_enable_flag\n             "
    fims_send -m set -u /features/standalone_power/ldss_enable_flag true
    fims_send -m get -u /features/standalone_power/ldss_enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        load_shed_enable\n             "
    fims_send -m set -u /features/standalone_power/load_shed_enable true
    fims_send -m get -u /features/standalone_power/load_shed_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        solar_shed_enable\n             "
    fims_send -m set -u /features/standalone_power/solar_shed_enable true
    fims_send -m get -u /features/standalone_power/solar_shed_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        closed_loop_control_enable\n             "
    fims_send -m set -u /features/standalone_power/closed_loop_control_enable true
    fims_send -m get -u /features/standalone_power/closed_loop_control_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        edp_enable\n             "
    fims_send -m set -u /features/standalone_power/edp_enable true
    fims_send -m get -u /features/standalone_power/edp_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "    site_operation\n"

<<watchdog
    printf "        watchdog_enable\n             "
    fims_send -m set -u /features/site_operation/watchdog_enable false
    fims_send -m get -u /features/site_operation/watchdog_enable -r /reply > currentTest.txt
    ./valueParse currentTest.txt false
    sleep 0.1
watchdog

    printf "Site sets-\n"
    printf "    operation\n"


<<no
    printf "        primary_controller\n"
    fims_send -m set -u /site/operation/primary_controller true
    fims_send -m get -u /site/operation/primary_controller -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        dbi_update\n"
    fims_send -m set -u /site/operation/dbi_update true
    fims_send -m get -u /site/operation/dbi_update -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1
no

    printf "        disable_flag\n          "
    fims_send -m set -u /site/operation/disable_flag true
    fims_send -m get -u /site/operation/disable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        enable_flag\n           "
    fims_send -m set -u /site/operation/enable_flag true
    fims_send -m get -u /site/operation/enable_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        standby_flag\n          "
    fims_send -m set -u /site/operation/standby_flag true
    fims_send -m get -u /site/operation/standby_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        standby_flag\n          "
    fims_send -m set -u /site/operation/standby_flag false
    fims_send -m get -u /site/operation/standby_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt false
    sleep 0.1

    printf "        clear_faults_flag\n         "
    fims_send -m set -u /site/operation/clear_faults_flag true
    fims_send -m get -u /site/operation/clear_faults_flag -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "    input_sources\n"

<<no
    printf "        primary_controller\n"
    fims_send -m set -u /site/input_sources true
    fims_send -m get -u /site/input_sources -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1
no

    printf "    configuration\n"

    printf "        invert_poi_kW\n         "
    fims_send -m set -u /site/configuration/invert_poi_kW true
    fims_send -m get -u /site/configuration/invert_poi_kW -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_1\n           "
    fims_send -m set -u /site/configuration/reserved_bool_1 true
    fims_send -m get -u /site/configuration/reserved_bool_1 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_2\n           "
    fims_send -m set -u /site/configuration/reserved_bool_2 true
    fims_send -m get -u /site/configuration/reserved_bool_2 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_3\n           "
    fims_send -m set -u /site/configuration/reserved_bool_3 true
    fims_send -m get -u /site/configuration/reserved_bool_3 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_4\n           "
    fims_send -m set -u /site/configuration/reserved_bool_4 true
    fims_send -m get -u /site/configuration/reserved_bool_4 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_5\n           "
    fims_send -m set -u /site/configuration/reserved_bool_5 true
    fims_send -m get -u /site/configuration/reserved_bool_5 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_6\n           "
    fims_send -m set -u /site/configuration/reserved_bool_6 true
    fims_send -m get -u /site/configuration/reserved_bool_6 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_7\n           "
    fims_send -m set -u /site/configuration/reserved_bool_7 true
    fims_send -m get -u /site/configuration/reserved_bool_7 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_8\n           "
    fims_send -m set -u /site/configuration/reserved_bool_8 true
    fims_send -m get -u /site/configuration/reserved_bool_8 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_9\n           "
    fims_send -m set -u /site/configuration/reserved_bool_9 true
    fims_send -m get -u /site/configuration/reserved_bool_9 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_10\n          "
    fims_send -m set -u /site/configuration/reserved_bool_10 true
    fims_send -m get -u /site/configuration/reserved_bool_10 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_11\n          "
    fims_send -m set -u /site/configuration/reserved_bool_11 true
    fims_send -m get -u /site/configuration/reserved_bool_11 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_12\n          "
    fims_send -m set -u /site/configuration/reserved_bool_12 true
    fims_send -m get -u /site/configuration/reserved_bool_12 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_13\n          "
    fims_send -m set -u /site/configuration/reserved_bool_13 true
    fims_send -m get -u /site/configuration/reserved_bool_13 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_14\n          "
    fims_send -m set -u /site/configuration/reserved_bool_14 true
    fims_send -m get -u /site/configuration/reserved_bool_14 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_15\n          "
    fims_send -m set -u /site/configuration/reserved_bool_15 true
    fims_send -m get -u /site/configuration/reserved_bool_15 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

    printf "        reserved_bool_16\n          "
    fims_send -m set -u /site/configuration/reserved_bool_16 true
    fims_send -m get -u /site/configuration/reserved_bool_16 -r /reply > currentTest.txt
    ./valueParse currentTest.txt true
    sleep 0.1

fi

printf "\nWhat asset would you like to test?\n"
printf "Assets:\n"
printf "    -ess\n"
printf "    -feeder\n"
printf "    -features\n"
printf "Type stop to end testing.\n"
read asset

done