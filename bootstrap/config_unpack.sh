#!/bin/bash
#
# p wilshire
# 10_26_2022

# 11_02_2022
# we need to add the ssh port to the system to allow running from windows through a mapped port

# 11_03_2022
# fixed tar error reporting 

# 11_10_2022
# Made so that untarring can go to a server and retrieve from a server along with
# local transferring of the untar

# remote sync
# $1 is a serverip, it rsyncs the files from the server into the local dir
# we'll probably get tar files , so we'll unpack them in place.

if [ ! -e /usr/bin/rsync ] ; then
  echo "unable to proceed ; \"rsync\" not installed"
  exit
fi
if [ ! -e /usr/bin/ssh ] ; then
  echo "unable to proceed ; \"openssh-clients\" not installed"
  exit
fi

# taking in two arguents of $1 where tars are, $2 where untarred files will go
if [ $# -eq 2 ] ; then
  serverdir=$1
  destdir=$2
else 
  echo "please supply the location of where config goes and source of where config tar is"
fi

# separating ip and path if that applies
serverip=`echo ${serverdir} | cut -d ':' -f1 `
serverpath=`echo ${serverdir} | cut -d ':' -f2 `
serverport=22

destip=`echo ${destdir} | cut -d ':' -f1 `
destpath=`echo ${destdir} | cut -d ':' -f2 `

# determining if we are getting from remote or not
if [ "${serverpath}" != "${serverip}" ] ; then
  remote=true
elif [ "${destpath}" != "${destip}" ] ; then
  remote=false  
fi

uptime=`date +%F_%H%M%S | sed -e 's/\//_/g'`

localdir=`pwd`
logdir="/var/log/flexgen"
logfile="cfg_rsync_pull.log"


# ani/integration to confirm this location, this is where each individual rsync_node send it's files via cloud_sync
# this is a dir on site controller that gets all the cloud_sync uploads
# serverdir="/home/hybridos/configs/brp/batcave"


basedir=`pwd`

node="rsync"
#mkdir -p ${stagedir}/${node}
#mkdir -p ${datadir}/${node}
mkdir -p ${logdir}

# dont forget firewall-cmd --add-port 2022/tcp

# if you want to get tar files from another server
if [ "${remote}" == true ] ; then
  rsync -avLi  --rsh="ssh -p${serverport}" --out-format='%n' ${serverip}:${serverpath}/* ${destpath} \
                              > /tmp/${node}_${uptime}.files 2>/tmp/${node}_${uptime}.err
  # unpack any tar archives in date/time order 
  # ignore it all if there are no files
  files=`ls  ${destpath}/*.tar.gz 2>/tmp/${node}_${uptime}.tar`
  for f in ${files} ;
  do
    cd $destpath
    echo " extract file $f "
    tar xzf $f
    rm $f
  done
elif [ "${remote}" == false ] ; then # if you want to send the unpacked tars to another server
  cd $serverpath
  files=`ls  ${serverpath}/*.tar.gz 2>/tmp/${node}_${uptime}.tar`
  for f in ${files} ;
  do
    folder=`echo ${f} | rev | cut -d '/' -f1  | rev | cut -d '_' -f1`
    echo " extract file $f "
    tar xzf $f
    rsync -r $folder "${destip}:${destpath}" 
    rm -rf $folder
  done
else # if it is all local transfers
  cd $serverpath
  files=`ls  ${serverpath}/*.tar.gz 2>/tmp/${node}_${uptime}.tar`
  for f in ${files} ;
  do
    echo " extract file $f "
    tar xzf $f --directory $destpath
  done
fi

# git update code not yet used
#ssh $serverip "cd /home/config/rsync && git add $node && git commit -m \" $node update $uptime\" && git push"

# leave the tar files in place so we dont have to pull them again.

#find ${localdir} -name "*.tar.gz" | xargs tar xvzf {}
if [ -s "/tmp/${node}_${uptime}.err" ] ; then
   echo " ${node}_${uptime} rsync err">> ${logdir}/${logfile}
   cat /tmp/${node}_${uptime}.err     >> ${logdir}/${logfile}
   echo " ============  "             >> ${logdir}/${logfile}
fi

if [ -s "/tmp/${node}_${uptime}.files" ] ; then
   echo " ${node}_${uptime} files"  >> ${logdir}/${logfile}
   echo " pulled to ${localdir}"    >> ${logdir}/${logfile}
   cat /tmp/${node}_${uptime}.files >> ${logdir}/${logfile}
   echo " ============  "           >> ${logdir}/${logfile}
fi

if [ -s "/tmp/${node}_${uptime}.tar" ] ; then
   echo " ${node}_${uptime} tar errors" >> ${logdir}/${logfile}
   cat /tmp/${node}_${uptime}.tar       >> ${logdir}/${logfile}
   echo " ============  "               >> ${logdir}/${logfile}
fi

rm -f /tmp/${node}_${uptime}.files
rm -f /tmp/${node}_${uptime}.err
rm -f /tmp/${node}_${uptime}.tar

