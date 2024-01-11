#!/bin/bash

fims_send="/usr/local/bin/fims_send"
#cd ../
dir=$(find / -maxdepth 4 -type d -name "psm")
echo $dir
#Start dbi, mongo, and fims_server if they aren't running
pgrep -x fims_server > /dev/null
if [ $? -ne 0 ]; then
    /usr/local/bin/fims_server &
fi
pgrep -x mongod > /dev/null
if [ $? -ne 0 ]; then
    mongod --config /etc/mongod.conf
fi
pgrep -x dbi > /dev/null
if [ $? -ne 0 ]; then
    /usr/local/bin/dbi &
fi
#default config
FILES=$(find $dir -maxdepth 3 -type f -name 'psm_dflt.json') 
#echo -e $FILES
# echo -n "Found these files:"
# echo -e $FILES

for f in $FILES; do
    fName=$(basename ${f})
    #echo ${fName%.*}
    dirname=$(dirname ${f})
    echo "Loading $fName from $dirname to database..."
    $fims_send -m set -u /dbi/psm/${fName%.*} -f $f
done

#update config
FILES=$(find $dir -maxdepth 3 -type f -name 'psm_*_updt.json') 
# echo -e $FILES

for f in $FILES; do
    fName=$(basename ${f})
    #echo ${fName%.*}
    dirname=$(dirname ${f})
    echo "Loading $fName from $dirname to database..."
    $fims_send -m set -u /dbi/psm/${fName%.*} -f $f
done

FILES=$(find $dir -maxdepth 3 -type f -name 'psm_tree.json') 
# echo -e $FILES

for f in $FILES; do
    fName=$(basename ${f})
    #echo ${fName%.*}
    dirname=$(dirname ${f})
    echo "Loading $fName from $dirname to database..."
    $fims_send -m set -u /dbi/psm/${fName%.*} -f $f
done
