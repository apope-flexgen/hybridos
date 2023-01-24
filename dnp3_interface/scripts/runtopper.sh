
#!/bin/sh
# script to run topper.sh
nohup   /home/flexgen/topper.sh > /mnt/sdc1/dnp3/run_topper.out&



#[flexgen@flexgen-01 dnp3_interface]$ cat ../topper.sh
##!/bin/sh
#FILE=/mnt/sdc1/dnp3/top_mem.txt
#while [ 1 ] ; do
#date >> $FILE
#top -b -n 1 | grep -e dnp3 -e PID >> $FILE
#sleep 120
#done


