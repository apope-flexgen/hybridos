#!/bin/sh
# run this with config/dnp3_scale_sign_test_client.json
# run this with config/dnp3_scale_sign_test_server.json
# use scripts/dnp3_scale_sign_test.sh to set up server

function fimssend {
   VAL=$1
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_Float32  $VAL 
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_int32  $VAL
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_signed_int32  $VAL
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_unsigned_int32  $VAL
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_int16  $VAL
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_signed_int16  $VAL
   /usr/local/bin/fims/fims_send -m set -u /site/operation/test_unsigned_int16  $VAL
   #/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_noscale $VAL
   #/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_noscale $VAL
   #/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_clscale_2.5 $VAL
   #/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_clscale_2.5 $VAL
   #/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_svscale_2.5 $VAL
   #/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_svscale_2.5 $VAL


}
i="0"
j=0
echo " hi there $i"
while true
do
  /usr/local/bin/fims/fims_send -m set -u /metrics/alvin/gen_ds1_status $j
  #/usr/local/bin/fims/fims_send -m set -u /site/operation/remote_enable_flag $j
  /usr/local/bin/fims/fims_send -m set -u /site/operation/remote_enable_flag "LATCH_ON"
  /usr/local/bin/fims/fims_send -m set -u /metrics/alvin/gen_net_mw $i
  /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_total_active_ower $j
  /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_average_soc $i
#  /usr/local/bin/fims/fims_send -m set -r /me -u /metrics/alvin/gen_ds1_status $j
#  /usr/local/bin/fims_send -m set -r /me -u /metrics/alvin/gen_net_mw $i
  i=$[$i+1]
  j=$((1-j))
  echo " hi there i $i j $j"

  case "$i" in 
    "1") echo "                           its one"
         VAL='{"value":2.5}'
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 0.1
         fimssend $VAL
         ;;
    "2") echo "                           its two"
         VAL='{"value":-2.5}'
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 4440000.0
         fimssend $VAL
         ;;
    "3") echo "                           its three"
         VAL='{"value":250000}'
         fimssend $VAL
         ;;
    "4") echo "                           its four"
         VAL='{"value":-250000}'
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 1.0
         fimssend $VAL
         ;;
    "5") echo "                           its five"
         VAL='{"value":65534}'
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 1.0
         fimssend $VAL
         ;;
    "6") echo "                           its six"
         VAL='{"value":2147483647}'
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 1000.0
         fimssend $VAL
         ;;
    "7") echo "                           its seven"
         VAL='{"value":2147483648}'
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 1.0
         fimssend $VAL
         ;;       
    "8") echo "                           its eight"
         VAL='{"value":4294967295}' 
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 1.0
         fimssend $VAL
         ;;    
    "9") echo "                           its nine"
         VAL='{"value":4294967296}' 
         /usr/local/bin/fims/fims_send -m set -u /features/active_power/fr_baseload_kW_cmd 1.0
         fimssend $VAL
         ;;
    *) echo "                           its default";;
  esac

  if [ "$i" -eq "10" ]; then
    i=0
  fi
  sleep 2.0
done
exit 0

#sample results

# Uri:     /site/operation/test_Float32                Body:    {"value":2.5}
# Uri:     /site/operation/test_int32                  Body:    {"value":2}
# Uri:     /site/operation/test_signed_Tint32          Body:    {"value":2}
# Uri:     /site/operation/test_signed_Fint32          Body:    {"value":2}
# Uri:     /site/operation/test_signed_Tint16          Body:    {"value":2}
# Uri:     /site/operation/test_signed_Fint16          Body:    {"value":2}

# Uri:     /site/operation/test_Float32                Body:    {"value":-2.5}
# Uri:     /site/operation/test_int32                  Body:    {"value":0}
# Uri:     /site/operation/test_signed_Tint32          Body:    {"value":0}
# Uri:     /site/operation/test_signed_Fint32          Body:    {"value":0}
# Uri:     /site/operation/test_signed_Tint16          Body:    {"value":0}
# Uri:     /site/operation/test_signed_Fint16          Body:    {"value":0}

# Uri:     /site/operation/test_Float32                Body:    {"value":250000}
# Uri:     /site/operation/test_int32                  Body:    {"value":250000}
# Uri:     /site/operation/test_signed_Tint32          Body:    {"value":250000}
# Uri:     /site/operation/test_signed_Fint32          Body:    {"value":250000}
# Uri:     /site/operation/test_signed_Tint16          Body:    {"value":32767}
# Uri:     /site/operation/test_signed_Fint16          Body:    {"value":65535}

# Uri:     /site/operation/test_Float32                Body:    {"value":-250000}
# Uri:     /site/operation/test_int32                  Body:    {"value":0}
# Uri:     /site/operation/test_signed_Tint32          Body:    {"value":0}
# Uri:     /site/operation/test_signed_Fint32          Body:    {"value":0}
# Uri:     /site/operation/test_signed_Tint16          Body:    {"value":0}
# Uri:     /site/operation/test_signed_Fint16          Body:    {"value":0}

# Uri:     /site/operation/test_Float32                Body:    {"value":2147483647}
# Uri:     /site/operation/test_int32                  Body:    {"value":2147483647}
# Uri:     /site/operation/test_signed_Tint32          Body:    {"value":2147483647}
# Uri:     /site/operation/test_signed_Fint32          Body:    {"value":2147483647}
# Uri:     /site/operation/test_signed_Tint16          Body:    {"value":32767}
# Uri:     /site/operation/test_signed_Fint16          Body:    {"value":65535}

# Uri:     /site/operation/test_Float32                Body:    {"value":2147483648}
# Uri:     /site/operation/test_int32                  Body:    {"value":2147483648}
# Uri:     /site/operation/test_signed_Tint32          Body:    {"value":2147483647}
# Uri:     /site/operation/test_signed_Fint32          Body:    {"value":2147483648}
# Uri:     /site/operation/test_signed_Tint16          Body:    {"value":32767}
# Uri:     /site/operation/test_signed_Fint16          Body:    {"value":65535}
                # {
                #     "id": "fr_baseload_kW_cmd",
                #     "offset": 1,
                #     "name": "Baseload Power Cmd",
                #     "uri": "/features/active_power",
                #     "scale":12
                # },
                # {
                #     "id": "fr_inactive_kW_cmd",
                #     "offset": 2,
                #     "name": "Inactive Baseload Power Cmd",
                #     "uri": "/features/active_power",
                #     "scale":-1.2
                # },
#uri [/assets/ess/summary] num vars 8
# ..                            [available_ess_num]
# ..                            [ess_average_soc]
# ..                            [ess_chargeable_power]
# ..                            [ess_dischargeable_power]
# ..                            [ess_total_active_power]
# ..                            [ess_total_apparent_power]
# ..                            [ess_total_reactive_power]
# ..                            [running_ess_num]

#idx [analog.0] ->name :[ess_total_active_power] offset : [0]  vIdx [0]===>
#idx [analog.1] ->name :[ess_total_reactive_power] offset : [1]  vIdx [1]===>



i="0"
j=0
echo "val $i"
while true
do
  /usr/local/bin/fims/fims_send -m set -r /me -u /metrics/alvin/gen_ds1_status $j
  /usr/local/bin/fims_send -m set -r /me -u /metrics/alvin/gen_net_mw $i
  i=$[$i+1]
  j=$((1-j))
done
