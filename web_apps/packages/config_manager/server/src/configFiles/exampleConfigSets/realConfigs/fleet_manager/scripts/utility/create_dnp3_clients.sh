#!/bin/bash

CONFIG_DIR=/home/config
TEMPLATE_FILE=$CONFIG_DIR/dnp3_client/templates/site_client.json

SITE_NAMES=("magnolia" "northfork")
IP_ADDRESS=172.3.27
OCTET=101

for name in ${SITE_NAMES[@]}
do
    targetFile=$CONFIG_DIR/dnp3_client/site_"$name"_client.json
    cp $TEMPLATE_FILE $targetFile
    echo "In loop with ip address octet $OCTET and site name $name"
    echo "Replacing ##SITE_NAME## in $targetFile with $name"                     && sed -i "s/##SITE_NAME##/$name/g" $targetFile
    echo "Replacing ##IP_ADDRESS_OCTET## in $targetFile with $IP_ADDRESS_$OCTET" && sed -i "s/##OCTET##/$((OCTET++))/g" $targetFile
done
