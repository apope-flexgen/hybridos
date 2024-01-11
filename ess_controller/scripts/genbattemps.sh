#!/bin/sh
# script to generate the battery temp arrays
total_arrays=2
total_racks=8
total_bats=12
offset=1728
i=0
j=$total_racks
k=$total_bats
echo "{"
for (( i=0; i<=$(( $total_arrays -1 )); i++ ))
do 
echo -n "\"id\": \"catl_cell_temp_${i}_${j}_${k}\","
echo -n "\"frequency\":2000,"
echo -n "\"offset_time\":$((${i}+1))00,"
echo -n "\"registers\":[ {"
echo -n "\"type\":\"Input Registers\","
echo -n "\"starting_offset\":${offset},"
echo -n "\"number_of_registers\":$(( $total_racks * $total_bats)),"
echo -n "\"map\":[ "
for (( j=0; j<=$(( $total_racks -1 )); j++ ))
do 
for (( k=0; k<=$(( $total_bats -1 )); k++ ))
do 
    echo -n "  {"
    echo -n "   \"id\": \"array_${i}_rack_${j}_cell_${k}_temp\",  "
    echo  -n "    \"offset\": $offset  "
    echo -n "  }"
    if [ $j -eq $(( $total_racks -1)) ] ; then
      if [ $k -eq $(( $total_bats -1)) ] ; then
        echo  -n ""
      else
        echo -n ","
      fi
    else
      echo -n ","
    fi
    #echo
    offset=`expr $offset + 1`
done
done
echo -n "]}]"
if [ $i -lt $(( $total_arrays -1)) ] ; then
echo -n ","
fi
echo
done
echo "}"

