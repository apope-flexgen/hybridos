#!/bin/sh 
# assemble all the components into the targ space and send it to the target

target="192.168.110.13"
tdir=targ/targ/ 
mkdir -p ${tdir}/usr/local/bin
cp /usr/local/bin/dnp3_client     ${tdir}/usr/local/bin
cp /usr/local/bin/dnp3_server     ${tdir}/usr/local/bin

mkdir -p ${tdir}/usr/local/lib
cp /usr/local/lib/libdmap.so     ${tdir}/usr/local/lib
cp /usr/local/lib/libopendnp3.so ${tdir}/usr/local/lib

mkdir -p ${tdir}/usr/local/lib64
cp /usr/local/lib64/libcrypto.so ${tdir}/usr/local/lib64
cp /usr/local/lib64/libssl.so    ${tdir}/usr/local/lib64

sudo mkdir -p ${tdir}/usr/local/etc/config/dnp3_interface
cp  config/test_dnp3*.json       ${tdir}/usr/local/etc/config/dnp3_interface
cp  config/test_tls*.json        ${tdir}/usr/local/etc/config/dnp3_interface
cp  config/test_rtu*.json        ${tdir}/usr/local/etc/config/dnp3_interface

mkdir -p ${tdir}/usr/local/etc/config/dnp3_interface/certs
cp  certs/*  ${tdir}/usr/local/etc/config/dnp3_interface/certs


mkdir -p ${tdir}/home/hybridos/dnp3_interface/scripts
cp  config/test_dnp3*.sh        ${tdir}/home/hybridos/dnp3_interface/scripts
cp  scripts/run_nontls*.sh      ${tdir}/home/hybridos/dnp3_interface/scripts
cp  scripts/run_tls*.sh         ${tdir}/home/hybridos/dnp3_interface/scripts
cp  scripts/run_rtu*.sh         ${tdir}/home/hybridos/dnp3_interface/scripts
cp  scripts/run_dnp3*.sh        ${tdir}/home/hybridos/dnp3_interface/scripts
cp  scripts/install_dnp3.sh    ${tdir}/home/hybridos/dnp3_interface/scripts

#
#install_dnp3.sh
# tdir=/tmp/targ/targ 
# sudo cp ${tdir}/usr/local/bin/*    /usr/local/bin
# sudo cp ${tdir}/usr/local/lib/*    /usr/local/lib
# sudo cp ${tdir}/usr/local/lib64/*  /usr/local/lib64

# sudo mkdir -p /usr/local/etc/config/dnp3_interface
# sudo cp  -a ${tdir}/usr/local/etc/config/dnp3_interface/* /usr/local/etc/config/dnp3_interface





