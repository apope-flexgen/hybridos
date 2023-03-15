#create Root CA
openssl genrsa -out RootCA.key 4096
openssl req -new -x509 -days 1826 -key RootCA.key -out RootCA.pem -subj "/C=US/O=xzy/OU=abc/CN=ROOT-CN"

#create Intermediate CA
openssl genrsa -out IntermediateCA.key 4096
openssl req -new -sha256 -key IntermediateCA.key -nodes -out IntermediateCA.csr -subj "/C=US/O=xyz/OU=abc/CN=INTERIM-CN"
openssl x509 -req -days 1000 -extfile MyOpenssl.conf -extensions int_ca -in IntermediateCA.csr -CA RootCA.pem -CAkey RootCA.key -CAcreateserial -out IntermediateCA.pem

#create EndUser certificates
openssl genrsa -out my_server.key 2048
openssl req -new -key my_server.key -out my_server.csr -subj "/C=US/O=xyz/OU=abc/CN=USER-CN"
openssl x509 -req -in my_server.csr -CA IntermediateCA.pem -CAkey IntermediateCA.key -set_serial 01 -out my_server.pem -days 500 -sha1


