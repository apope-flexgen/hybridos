TLS (SSL) Certificates
======================

This tutorial was shamelessly stolen from [here](https://gist.github.com/cpburnz/6451de544af7efddd317).

For the TLDR, just run `sh make_certificate.sh`.
   
   

Certificate Authority
---------------------

Before you can do anything with TLS certificates, you need to choose a
Certificate Authority (CA) to use. For public facing websites with HTTPS, you
need to use a Root Certificate Authority which will not be covered by this
document. For internal services using self-signed certificates, a self-signed
Certificate Authority is sufficient because you control every end.



Creating Certificates
---------------------

Prerequisites
=============

The following command is required::

	openssl
	
This can be installed on Debian based distributions (e.g., Ubuntu) with
*apt-get*::

	sudo apt-get install openssl



Subjects
========

Every certificate should have a *subject* which describes the owner and use of
the certificate::

	/C=<country>/ST=<state>/L=<locality>/O=<organization>/OU=<organization-unit>/CN=<common-name>/emailAddress=<email>

Where:

- *\<country\>* is the country (e.g., "US").

- *\<state\>* is the state or province (e.g., "Michigan").

- *\<locality\>* is the locality or city (e.g., "Grand Rapids").

- *\<organization\>* is the name of the organization or individual (e.g.,
  "Acme Inc.").

- *\<organization-unit\>* is the organization unit (e.g., "IT").

- *\<common-name\>* is the common name for the certificate (e.g., "Internal
  Server Certificate").

- *\<email\>* optionally is a contact email address (e.g., "it@example.com").

Example::

	/C=US/ST=Michigan/L=Grand Rapids/O=Acme Inc./OU=IT/CN=Internal Server Certificate/emailAddress=it@example.com

See `Certificate subject X.509 <http://stackoverflow.com/q/6464129/369450>`_
for more information.



Self-signed Root Certificates
=============================

A self-signed root certificate can be created with::

	openssl req -new -newkey rsa:4096 -nodes -x509 -days <days> -keyout <key> -out <cert> -subj <subject>

Where:

- *\<days\>* is the number of days the root certificate is valid. It is
  impossible to create a certificate which is valid indefinitely, but you can
  any duration.

- *\<key\>* is the file to store the private key of the certificate (e.g.,
  ``ca_key.pem``).

- *\<cert\>* is the file to store the public certificate (e.g.,
  ``ca_cert.pem``).

- *\<subject\>* is as described in the `Subjects <#subjects>`_ section above.

Example::

	openssl req -new -newkey rsa:4096 -nodes -x509 -days 365 -keyout ca_key.pem -out ca_cert.pem -subj '/C=US/ST=Michigan/L=Grand Rapids/O=Acme Inc./OU=IT/CN=Internal Root Certificate/emailAddress=it@example.com'

This creates a root private key ``ca_key.pem`` and public certificate
``ca_cert.pem``.



CA-signed Certificates
======================

Now we can create certificates which are signed by the CA certificate. Regular
certificates are created in 2 steps.

1. A private key and Certificate Signing Request (CSR) for that key must be
   created.
2. The CSR must be signed by the CA to generate the actual signed certificate
   for the private key.

First, create a private key::

	openssl genpkey -algorithm rsa -pkeyopt rsa_keygen_bits:2048 -out <key>

Then, create the CSR for the private key::

	openssl req -new -key <key> -out <csr> -subj <subject>

Or, create a private key and CSR in one step::

	openssl req -new -newkey rsa:2048 -nodes -keyout <key> -out <csr> -subj <subject>

Where:

- *\<key\>* is the file to store the private key of the future certificate
  (e.g., "server_key.pem").

- *\<csr\>* is the file to store the CSR which needs to be signed (e.g.,
  "server_csr.pem")
  
- *\<subject\>* is as described in the `Subjects <#subjects>`_ section above.

Example::

	openssl req -new -newkey rsa:2048 -nodes -keyout server_key.pem -out server_csr.pem -subj '/C=US/ST=Michigan/L=Grand Rapids/O=Acme Inc./OU=IT/CN=Internal Server Certificate/emailAddress=it@example.com'

This creates a new private key ``server_key.pem`` and a CSR ``server_csr.pem``
which still needs to be signed.

Now, sign the CSR with the CA::

	openssl x509 -req -days <days> -extfile /etc/ssl/openssl.cnf -extensions usr_cert -CAcreateserial -CA <ca-cert> -CAkey <ca-key> -in <csr> -out <cert>

Where:

- *\<days\>* is the number of days the signed certificate should be valid for
  (e.g., 365).
  
- *\<ca-cert\>* is the CA (or signing) public certificate (e.g.,
  ``ca_cert.pem``).

- *\<ca-key\>* is the CA (or signing) private key (e.g., ``ca_key.pem``).

- *\<csr\>* is the CSR being signed (e.g., ``server_csr.pem``).

- *\<cert\>* is the new certificate generated from the CSR which was signed by
  the CA (e.g., ``server_cert.pem``).

.. NOTE:: This will create a serial file with the same base name as
   *\<ca-cert\>* but with a ".srl" extension (e.g., ``ca_cert.srl``). The
   serial file is created the first time a certificate is signed by the CA,
   and is used to give a unique serial number to every certificate signed by
   the CA. KEEP THIS FILE WHERE YOU SIGN CERTIFICATES WITH THE CA.

Example::

	openssl x509 -req -days 365 -extfile /etc/ssl/openssl.cnf -extensions usr_cert -CAcreateserial -CA ca_cert.pem -CAkey ca_key.pem -in server_csr.pem -out server_cert.pem

This creates the new public certificate ``server_cert.pem`` from the CSR
``server_csr.pem`` which was signed by the CA using ``ca_key.pem``,
``ca_cert.pem``, and ``ca_cert.srl``.



Chained Certificates
====================

Certificates can be created and signed using a chain of CAs. Any signed
certificate can act as a CA. A certificate ``client_cert.pem`` could be
created by creating a new private key ``client_key.pem`` and CSR
``client_csr.pem``, and by signing the CSR with the server certificate acting
as the CA using ``server_key.pem``, ``server_cert.pem``, and
``server_cert.srl`` (automatically generated).

Example::

	openssl req -new -nodes -keyout client_key.pem -out client_csr.pem -subj '/C=US/ST=Michigan/L=Grand Rapids/O=Acme Inc./OU=IT/CN=Internal Client Certificate/emailAddress=it@example.com'
	openssl x509 -req -days 365 -extfile /etc/ssl/openssl.cnf -extensions usr_cert -CAcreateserial -CA server_cert.pem -CAkey server_key.pem -in client_csr.pem -out client_cert.pem



Verifying Certificates
======================

A certificate can be verified, but the whole certificate chain (excluding the
one being verified) must be concatenated into a single file::

	openssl verify -CAfile <ca-chain> <cert>

Where:

- *\<ca-chain\>* is a file containing each public certificate in the chain
  which signed the certificate being verified.

- *\<cert\>* is the file containing the certificate to verify.

Example::

	cat ca_cert.pem server_cert.pem >> ca_chain.pem
	openssl verify -CAfile ca_chain.pem client_cert.pem
	
	
.. TODO:: How to verify a public and private key pair.
