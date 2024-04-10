#!/bin/bash

CERT_DIR=/home/docker/hybridos/gcom_dnp3_interface/security_certificate_stuff/out

# Check if the directory exists
if [ ! -d "$CERT_DIR" ]; then
    # If the directory doesn't exist, create it
    mkdir -p "$CERT_DIR"
fi

#create Root CA
new_root_ca=false
if [ ! -e $CERT_DIR/RootCA.key ] || [ ! -e $CERT_DIR/RootCA.pem ]; then
new_root_ca=true
openssl genrsa -out $CERT_DIR/RootCA.key 4096
openssl req -new -x509 -days 1826 -key $CERT_DIR/RootCA.key -out $CERT_DIR/RootCA.pem -subj "/C=US/O=xzy/OU=abc/CN=ROOT-CN"
fi

#create Intermediate CA
if [[ $new_root_ca == "true" ]] || [ ! -e $CERT_DIR/IntermediateCA.key ] || [ ! -e $CERT_DIR/IntermediateCA.pem ]; then
openssl genrsa -out $CERT_DIR/IntermediateCA.key 4096
openssl req -new -sha256 -key $CERT_DIR/IntermediateCA.key -nodes -out $CERT_DIR/IntermediateCA.csr -subj "/C=US/O=xyz/OU=abc/CN=INTERIM-CN"
openssl x509 -req -days 1000 -extfile MyOpenssl.conf -extensions int_ca -in $CERT_DIR/IntermediateCA.csr -CA $CERT_DIR/RootCA.pem -CAkey $CERT_DIR/RootCA.key -CAcreateserial -out $CERT_DIR/IntermediateCA.pem
fi

#create EndUser certificates
#server
CERT_NAME=server
openssl genrsa -out $CERT_DIR/$CERT_NAME.key 2048
openssl req -new -key $CERT_DIR/$CERT_NAME.key -out $CERT_DIR/$CERT_NAME.csr -subj "/C=US/O=xyz/OU=abc/CN=TLS"
openssl x509 -req -in $CERT_DIR/$CERT_NAME.csr -CA $CERT_DIR/IntermediateCA.pem -CAkey $CERT_DIR/IntermediateCA.key -set_serial 01 -out $CERT_DIR/$CERT_NAME.pem -days 500 -sha1

#client
CERT_NAME=client
openssl genrsa -out $CERT_DIR/$CERT_NAME.key 2048
openssl req -new -key $CERT_DIR/$CERT_NAME.key -out $CERT_DIR/$CERT_NAME.csr -subj "/C=US/O=xyz/OU=abc/CN=TLS"
openssl x509 -req -in $CERT_DIR/$CERT_NAME.csr -CA $CERT_DIR/IntermediateCA.pem -CAkey $CERT_DIR/IntermediateCA.key -set_serial 02 -out $CERT_DIR/$CERT_NAME.pem -days 500 -sha1

cat $CERT_DIR/RootCA.pem $CERT_DIR/IntermediateCA.pem > $CERT_DIR/chain.pem