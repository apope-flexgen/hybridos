#!/bin/sh
# script to set up build env for modbus_client and dnp3
# note this adds cJSON which may be set up via a third party rpm
# not this does not get libmodbus

yum install wget -y
yum update
wget https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/e/epel-release-7-14.noarch.rpm
rpm -Uvh epel-release-7-14.noarch.rpm
yum install cmake3 -y
yum install nano -y
yum install git -y

BASEDIR=`pwd`

cd $BASEDIR
mkdir rigtorp
cd rigtorp
wget https://github.com/rigtorp/SPSCQueue/archive/refs/tags/v1.1.tar.gz rigtorpv1.1.tar.gz
mv v1.1.tar.gz rigtorpv1.1.tar.gz
tar xvzf rigtorpv1.1.tar.gz
cd SPSCQueue-1.1/
mkdir build && cd build && cmake3 ../ && make && make install

cd $BASEDIR
mkdir tscns
cd  tscns
git clone https://github.com/MengRao/tscns.git
cd tscns/
cp tscns.h /usr/local/include/

cd $BASEDIR
mkdir cjson
cd cjson
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON/
make && make install

cd $BASEDIR
mkdir pmap
cd pmap
wget https://github.com/greg7mdp/parallel-hashmap/archive/refs/tags/1.33.tar.gz
mv 1.33.tar.gz pmap_1.33.tar.gz
tar xvzf pmap_1.33.tar.gz
cd parallel-hashmap-1.33/
mkdir build && cd build && cmake3 ../ && make && make install

cd $BASEDIR
mkdir spdlog
cd spdlog
wget https://github.com/gabime/spdlog/archive/refs/tags/v1.9.2.tar.gz
mv v1.9.2.tar.gz spdlog_v1.9.2.tar.gz
tar xvzf spdlog_v1.9.2.tar.gz
cd spdlog-1.9.2/
mkdir build && cd build && cmake3 ../ && make && make install


cd $BASEDIR
mkdir simdjson 
cd simdjson 
wget https://github.com/simdjson/simdjson/archive/refs/tags/v1.0.2.tar.gz
mv  v1.0.2.tar.gz simdjson-v1.0.2.tar.gz
tar xvzf simdjson-v1.0.2.tar.gz
cd simdjson-1.0.2/
mkdir build && cd build && cmake3 ../ && make && make install

cd $BASEDIR
mkdir openssl
cd  openssl
wget https://www.openssl.org/source/openssl-1.1.1.tar.gz
tar xvzf openssl-1.1.1.tar.gz
cd openssl-1.1.1
./config
make && make install
LD_LIBRARY_PATH=/usr/local/lib64  openssl version

cd $BASEDIR
mkdir opendnp3
cd opendnp3/
wget https://github.com/dnp3/opendnp3/archive/refs/tags/3.1.1.tar.gz
mv 3.1.1.tar.gz opendnp3_3.1.1.tar.gz
tar xvzf opendnp3_3.1.1.tar.gz
cd opendnp3-3.1.1/
mkdir build
cd build/
cmake3 -DDNP3_TLS=ON ../
make && make install
