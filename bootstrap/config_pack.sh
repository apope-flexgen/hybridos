#!/bin/bash
#
# p wilshire
# 10_26_2022

# 11_02_2022 
#    push the whole config
#    but just the .json files 
# $1 is the node or system name , will use $HOSTNAME if not defined 

# remote sync
# $2 is a serverip, it rsyncs the files to the server
# local sync
# creates a list of chnaged files and then tars them up ready for cloud_sync to deploy the tarball
# the 11_02_2022  change tars up the whole config set each time
# $2 is the server  if we dont give a server the operation is local 

# TODO get from node ev remove  $1 $HOSTNAME ess-1nf-block1
# HOSTNAME is not a good idea for docker so we run with 2 args in that case.

if [ $# -ge 2 ] ; then
 node=$2
else 
 node=${HOSTNAME}
fi
echo "$0 running with node as $node"
#TODO this becomes $1 default to local for no args
if [ $# -ge 1 ] ; then
  serverip=$1
else 
  serverip="local"
  #serverip="root@172.30.0.23"
fi

uptime=$(date +%F_%H%M%S | sed -e 's/\//_/g')


cs_config="/usr/local/etc/config/cloud_sync/cloud_sync.json"
# cloudsync uses this read from cloud_sync
datadir="/home/data"

if [ -f  "${cs_config}" ] ; then
  ddir=$(jq '.clients.main.directory' ${cs_config})
  if [ "${ddir}" != "null" ] ; then 
    datadir=$(echo ${ddir} | sed "s/^\([\"']\)\(.*\)\1\$/\2/g")
  else
    echo " Unable to find cloud sync config dir ; Using default ${datadir}"
  fi
else
    echo " Unable to find cloud sync config file ${cs_config}"
fi

# local copy to detect diffs
# remove part or all to run a complete resync
stagedir="/home/rsync"

# whre the config files are locally (can be a link)
srcdir="/usr/local/etc/config"

# log details
logdir="/var/log/flexgen/rsync"
logfile="cfg_rsync_local.log"

# rsync destination (if we are not using local and rsync'ing directly) on server 
serverdir="/home/config/rsync"


mkdir -p "${datadir}"
mkdir -p "${stagedir}"
mkdir -p "${logdir}"


if [ "${serverip}" == "local" ]; then

   rsync -aLi  --out-format='%n' ${srcdir} ${stagedir}/${node} | grep -e ".json" \
                             > /tmp/${node}_${uptime}.files 2>/tmp/${node}_${uptime}_err.out

   #echo " files detected "
   #cat /tmp/${node}_${uptime}.files                           
   #if [  -s "/tmp/${node}_${uptime}.files" ] ; then
   #   echo " adding files "
   #fi
   # if nothing changed we get no files.
   if [  -s "/tmp/${node}_${uptime}.files" ] ; then
      #mkdir -p ${datadir}/${node}
      #tar --transform 's,^,/prefix/,' -x -f arch.tar
      # send just changes
      # redo the file list
      cd /usr/local/etc/config && find -L . -name "*.json" > /tmp/${node}_${uptime}.files2
      #cat /tmp/${node}_${uptime}.files2
      #echo "=================="                           
      #cd /usr/local/etc && /usr/bin/tar -czf ${datadir}/${node}_${uptime}_files.tar.gz \
      #                       --transform "s,^,${node}/," -T /tmp/${node}_${uptime}.files 2>/tmp/${node}_${uptime}_err.out
      # send the whole list no need for time date
      cd /usr/local/etc/config && /usr/bin/tar -czf ${datadir}/${node}_files.tar.gz \
                             --transform "s,^./,${node}/," -T /tmp/${node}_${uptime}.files2 2>/tmp/${node}_${uptime}_err.out

   fi

else

   rsync -aLi  --out-format='%n' ${srcdir} ${serverip}:${serverdir}/${node} | grep -e ".json" \
                            > /tmp/${node}_${uptime}.files 2>/tmp/${node}_${uptime}_err.out
   #ssh $serverip "cd /home/config/rsync && git add $node && git commit -m \" $node update $uptime\" && git push"

fi

if [ -s "/tmp/${node}_${uptime}_err.out" ] ; then
   echo " ${node}_${uptime}_err"      >> ${logdir}/${logfile}
   cat /tmp/${node}_${uptime}_err.out >> ${logdir}/${logfile}
fi

# keep a log of changed files
if [ -s "/tmp/${node}_${uptime}.files" ] ; then
   echo " ${node}_${uptime}_files"  >> ${logdir}/${logfile}
   cat /tmp/${node}_${uptime}.files >> ${logdir}/${logfile}
fi


rm -f /tmp/${node}_${uptime}.files
rm -f /tmp/${node}_${uptime}.files2
rm -f /tmp/${node}_${uptime}_err.out


