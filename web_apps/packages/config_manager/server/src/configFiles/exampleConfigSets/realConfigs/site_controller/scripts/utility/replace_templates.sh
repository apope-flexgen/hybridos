#!/bin/bash

TEMPLATES_DIR=/home/templates/$TEMPLATES_FOLDER
CONFIG_DIR=/home/config

FILES=$(find $TEMPLATES_DIR -type f)

echo "Environment variables:"
env
echo

for f in $FILES
do
    targetFile=${f/$TEMPLATES_DIR/$CONFIG_DIR}
    dir=$(dirname ${targetFile})
    mkdir -p $dir
    cp $f $targetFile
    grep "##SITE_NAME##" -q $targetFile  && echo "Replacing ##SITE_NAME## in $targetFile with $SITE_NAME" && sed -i "s/##SITE_NAME##/$SITE_NAME/g" $targetFile
done
