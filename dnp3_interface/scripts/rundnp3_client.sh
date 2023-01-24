DATE=0112
echo "logfile=/mnt/sdc1/dnp3/dnp3_log_$DATE.txt "
#exit 0

nohup /home/flexgen/dnp3_interface/build/release/dnp3_client /home/flexgen/dnp3_interface/config/alvin_dnp3_client.json > /mnt/sdc1/dnp3/dnp3_log_$DATE.txt &

