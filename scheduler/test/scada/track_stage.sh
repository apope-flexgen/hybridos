while :
do
    clear
    /usr/local/bin/fims_send -m get -u /scheduler/scada -r /me | jq '.stage'
    sleep 2
done
