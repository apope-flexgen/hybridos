#!/bin/bash
# tcp_dump.sh
# run a continuous tcp_dump against a selected interface.

if [[ $# < 3 ]]; then
    echo "at least three arguments are required for this script."
    echo "usage: sh /path/to/tcpdump.sh FOLDERNAME FILENAME DEV PORT(optional)"
    exit 1
fi

FOLDERNAME=$1
FILENAME=$2
DEV=$3
PORT=""
if [[ $# == 4 ]]; then
    PORT=$4
fi

mkdir -p ${FOLDERNAME}/${DEV}

while [[ ! -f /tmp/dump_stop ]] ;
do
    if [[ ! -z $PORT ]]; then # particular port:
        echo "port provided, using port: ${PORT}"
        timeout 310 tcpdump -i ${DEV} "port ${PORT}" -G 300 -w "${FOLDERNAME}/${DEV}/${FILENAME}_%F_%T.pcap"&
    else # all ports:
        echo "port not provided, scanning all ports"
        timeout 310 tcpdump -i ${DEV} -G 300 -w "${FOLDERNAME}/${DEV}/${FILENAME}_%F_%T.pcap&"
    fi
    sleep 300
done

echo "finishing main loop due to /tmp/dump_stop existing. Removing the temp file."

rm /tmp/dump_stop

exit 0
