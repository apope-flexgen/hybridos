# fims_relay
`fims_relay` is a service that relays messages to/from `fims` from published Docker container TCP ports.
It accomplishes this by:
- Running a publisher socket that publishes fims messages to a TCP port.
- Running a pull socket that pulls from incoming messages on a TCP port.
- Running a REST API to handle get, set, post and del messages.

The underlying messaging library is [zeromq](https://github.com/zeromq/zeromq.js).

This service is intended to run alongside other HybridOS containers that expose `/tmp` (the location of the FIMS socket) and `/usr/local/lib` (location of napi binary) via Docker volumes.

# Example Compose Setup with TWINS
```
version: "3.3"

services:
  twins:
    image: flexgen/twins_pm:${TWINS_VERSION}
    container_name: twins
    ports:
      - 10000:10000
      - 10001:10001
      - 10002:10002
      - 10003:10003
      - 10004:10004
      - 10005:10005
    networks:
      localnet:
        ipv4_address: 172.3.27.2
    tty: true
    volumes:
      - ./config/twins/config:/home/config
      - /tmp:/tmp
      - fims-node-dir:/usr/local/lib

  fims_relay:
    image: flexgen/fims_relay
    container_name: fims_relay
    depends_on:
      - twins
    tty: true
    ports:
      - 4000:4000
      - 4001:4001
      - 8080:8089
    volumes:
     - fims-node-dir:/fims-node-dir
     - /tmp:/tmp

networks:
  localnet:
    ipam:
      driver: default
      config:
        - subnet: 172.3.27.0/16

volumes:
  fims-node-dir:
```

# Getting Started
## Build
- `docker build . -t fims_relay`
## Run tests
- `docker build . --target test`
## Run container
- `docker run fims_relay`

# Setting up your client for socket interactions ([ØMQ](https://zeromq.org/get-started/))
## Subscribing and receiving data from fims_relay
```
var zmq = require("zeromq"),
  sub = zmq.socket("sub");

sub.connect(PUB_IP_ADDRESS);
sub.subscribe('/components/grid');

sub.on("message", function(uri, data) {
    let uri = uri.toString();
    let json = JSON.parse(data);
  }
});
```

## Pushing messages to fims_relay
```
var zmq = require("zeromq"),
  push = zmq.socket("push");

push.connect(PULL_IP_ADDRESS);

push.send(JSON.stringify({
  method: 'get',
  uri: '/me',
  replyto: null,
  body: JSON.stringify({}), // JSON.stringify key-value body here.
  username: null
}));
```

## View REST API Documentation 
After starting `fims_relay`, visit `localhost:8089/api` to view REST API documentation.
