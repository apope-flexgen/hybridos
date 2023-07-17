
#!/bin/sh
#test batchPubs
# p wilshire  02_27_2023

val=1234

runPub () {
    val=$1
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":$val, 
        \"reactive_power\":$val, 
        \"apparent_power\":$val,
        \"frequency\":$val, 
        \"kwh_delivered\":$val
     }"
}

runSinglePub () {
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":1000, 
        \"reactive_power\":1000
     }"
    sleep 1.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":2000, 
        \"reactive_power\":2000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":3000, 
        \"reactive_power\":3000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":4000, 
        \"reactive_power\":4000
     }"
    sleep 0.2
    fims_send -m pub -u /components/sel_735 " {
        \"active_power\":5000, 
        \"reactive_power\":5000
     }"
} 


fims_send -m pub -u /local_server/_system '{  "batchPubDebug": 1}'

fims_send -m pub -u /local_server/_system '{  "batchPubs": 0}'

#runPub 10000
#sleep 1
#runPub 22000
#sleep 1
#runPub 32300
#sleep 1
#runPub 423400
#sleep 1
#runPub 0


#fims_send -m pub -u /local_server/_system '{  "batchPubs": 5000}'

#runPub 10000
#sleep 1
#runPub 22000
#sleep 1
#runPub 32300
#sleep 1
#runPub 423400

#fims_send -m pub -u /local_server/_system '{  "batchPubs": 0}'
#fims_send -m pub -u /local_server/_system '{  "batchPubDebug": 0}'
