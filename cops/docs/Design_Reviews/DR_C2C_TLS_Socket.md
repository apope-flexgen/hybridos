# Design Review

## Goals

Reimplement the C2C connection as a TLS socket so that we don't have to maintain our own encryption code.

## Affected Repos

`cops` \
`config`

## Approach

Follow the design of `scheduler`'s fleet-site websocket, but instead of using a secure websocket, use a more lightweight TLS socket. In this case, we apply TLS on top of a TCP connection instead of an HTTP connection.

Currently the instance of cops with the lower IP address is considered the server when forming the connection. The same approach will be used to determine the server in the TLS socket connection.

We use port 8000 for C2C connections, the port number will continue to be hardcoded as 8000.

Like scheduler, the socket connection will be secured with TLS. Both instances will need an SSL certificate containing the server's public key. Only the server will need the associated private key.

Ask infrastructure how we should manage keys to ensure that we don't share keys between customers or applications.

## Interface

C2C messages will be encrypted by the encryption algorithm decided on by TLS. Connections will otherwise be unchanged.

## Testing

Test that both the server and client cops can send and receive C2C messages as before.

## Backward Compatibility

Not backwards compatible.

## Configuration

In config, under the cops directory, there will be new `server.crt` files `server.key` files. The `server.crt` should match between the two machines. The `server.key` should be the same as well. Although the client will not use the `server.key`, the `server.key` file will be placed on both machines to prevent users from forgetting to put the `server.key` on the server machine.

Server: \
server.crt \
server.key

Client: \
server.crt \
server.key