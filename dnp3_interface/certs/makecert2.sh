#create Root CA
openssl genrsa -out out/RootCA.key 4096
openssl req -new -x509 -days 1826 -key out/RootCA.key -out out/RootCA.pem -subj "/C=US/O=xzy/OU=abc/CN=ROOT-CN"

#create Intermediate CA
openssl genrsa -out out/IntermediateCA.key 4096
openssl req -new -sha256 -key out/IntermediateCA.key -nodes -out out/IntermediateCA.csr -subj "/C=US/O=xyz/OU=abc/CN=INTERIM-CN"
openssl x509 -req -days 1000 -extfile MyOpenssl.conf -extensions int_ca -in out/IntermediateCA.csr -CA out/RootCA.pem -CAkey out/RootCA.key -CAcreateserial -out out/IntermediateCA.pem

#create EndUser certificates
openssl genrsa -out out/my_server.key 2048
openssl req -new -key out/my_server.key -out out/my_server.csr -subj "/C=US/O=xyz/OU=abc/CN=USER-CN"
openssl x509 -req -in out/my_server.csr -CA out/IntermediateCA.pem -CAkey out/IntermediateCA.key -set_serial 01 -out out/my_server.pem -days 500 -sha1



