#!/bin/sh
# script to install dnp3 systems 
# run as root
# copy libraries
# /usr/local/lib
# libdmap.so  libopendnp3.so  libopendnp3.so.2  libopendnp3.so.2.3.3
#
if [ $(id -u) -ne 0 ]; then
   echo "This script must be run as root"
   exit;
fi
tdir=/tmp/targ

rm -f /usr/local/lib/libdmap.so  
rm -f /usr/local/lib/libopendnp3*  
cp ${tdir}/usr/local/lib/libopendnp3.so  /usr/local/lib
cp ${tdir}/usr/local/lib/libdmap.so  /usr/local/lib
ln -sf /usr/local/lib/libopendnp3.so /usr/local/lib/libopendnp3.so.2
ln -sf /usr/local/lib/libopendnp3.so.2 /usr/local/lib/libopendnp3.so.2.3.3

# libcrypto.so  libcrypto.so.1.1  libssl.so  libssl.so.1.1
rm -f /usr/local/lib64/libcrypto.so*
rm -f /usr/local/lib64/libssl.so*
cp ${tdir}/usr/local/lib64/libcrypto.so  /usr/local/lib64
ln -sf  ${tdir}/usr/local/lib64/libcrypto.so  /usr/local/lib64/libcrypto.so.1.1

cp ${tdir}/usr/local/lib64/libssl.so  /usr/local/lib64
ln -sf  ${tdir}/usr/local/lib64/libssl.so  /usr/local/lib64/libssl.so.1.1

rm -f /usr/local/bin/dnp3_client
rm -f /usr/local/bin/dnp3_server
cp ${tdir}/usr/local/bin/* /usr/local/bin
chmod +x /usr/local/bin/dnp3_client
chmod +x /usr/local/bin/dnp3_server

cp ${tdir}/usr/lib/systemd/system/* /usr/lib/systemd/system
systemctl daemon-reload 

touch /var/log/dnp3_client.log
chmod a+rw  /var/log/dnp3_client.log

#sudo LD_LIBRARY_PATH=/usr/local/lib64: /usr/local/bin/dnp3_client ${tdir}/home/hybrios/config/test_dnp3_client.json> /var/log/dnp3_client.log 2>&1&
