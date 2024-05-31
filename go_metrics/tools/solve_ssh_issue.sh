#!/bin/bash

## for some reason I run into this problem every time I get a new container
## so I'm just making a script for it

eval `ssh-agent -s`
ssh-add -l
mkdir -p /root
cp -a  /tmp/.ssh /root
chmod 0600 /root/.ssh/id_ed25519 ## make sure this matches your ssh key
ssh-add /root/.ssh/id_ed25519


## also copy fims because that doesn't work for me either
\cp -ra /home/docker/hybridos/fims/go_fims/ /usr/lib/golang/src/fims/