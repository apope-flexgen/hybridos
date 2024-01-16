#!/bin/sh

# first use this to get just the Uri and Body fields

grep -e Uri  -e Body ~/1b8/0430/fims.out  \
    | sed -e 's$Uri:$/usr/local/bin/fims/fims_send -m pub -u$g'   -e 's/Body://g' -e "s/}}/}}/g" > /tmp/fims_work.out

echo "created work file /tmp/fims_work.out"

# input
#Method:  pub
#Uri:     /components/catl_mbmu_stat_r
#ReplyTo: (null)
#Body:    {"mbmu_status":1,"Timestamp":"04-30-2021 09:18:22.476955"}
#Timestamp:   2021-04-30 09:18:22.477452
#Method:  pub
#Uri:     /components/catl_mbmu_summary_r
#ReplyTo: (null)
#Body:    {"mbmu_voltage":1317.2,"mbmu_current":21402,"mbmu_soc":59.3,"mbmu_soh":100,"mbmu_max_cell_voltage":3.189,"mbmu_min_cell_voltage":3.148,"mbmu_avg_cell_voltage":3.172,"mbmu_max_cell_t
#emperature":72,"mbmu_min_cell_temperature":70,"mbmu_avg_cell_temperature":70,"mbmu_max_charge_current":17474,"mbmu_max_discharge_current":22546,"Timestamp":"04-30-2021 09:18:22.486902"}
#Timestamp:   2021-04-30 09:18:22.487506

#output
#/usr/local/bin/fims/fims_send -m pub -u     /components/catl_mbmu_stat_r
#    {"mbmu_status":1,"Timestamp":"04-30-2021 09:18:22.476955"}
#/usr/local/bin/fims/fims_send -m pub -u     /components/catl_mbmu_summary_r
#    {"mbmu_voltage":1317.2,"mbmu_current":21402,"mbmu_soc":59.3,"mbmu_soh":100,"mbmu_max_cell_voltage":3.189,"mbmu_min_cell_voltage":3.148,"mbmu_avg_cell_voltage":3.172,"mbmu_max_cell_temper
#ature":72,"mbmu_min_cell_temperature":70,"mbmu_avg_cell_temperature":70,"mbmu_max_charge_current":17474,"mbmu_max_discharge_current":22546,"Timestamp":"04-30-2021 09:18:22.486902"}

# next we need to put the two lines together

fout=""
rm -f /tmp/fims_work2.out
cat /tmp/fims_work.out | \
    while read ff ; do  
        if [ "$ff" != "" ]; then 
            #echo $ff
            fout="${fout}${ff} '";
            read gg;
            fout="${fout}${gg} '";
            if grep -e components  <<<"$ff";
            then  
                echo "fout = $fout" >> /tmp/fims_work2.out; 
                echo " sending $fout" ;
                $fout;
                sleep 0.1
            fi
            
            if grep -e assets <<<"$ff";
            then
                if grep -e pub <<<"$ff";
                then
                    echo " skipping asset pub"
                else
                    echo "fout = $fout" >> /tmp/fims_work2.out; 
                    echo " sending $fout" ;
                    $fout;
                    sleep 0.1
                fi
            fi

            fout="";
        fi;
    done

# next we need to "chunk" it

#grep '/usr' /tmp/fims_work.out | sed -e 's$/usr$\n/usr$' > /tmp/fims_chunk.out



