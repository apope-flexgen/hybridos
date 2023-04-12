#!/bin/bash

# check if an argument was provided
if [ -z "$1" ]; then
    echo "usage: ./ssh-key-registration.sh hybridos@#.#.#.#"
    exit 1
fi

# generate RSA key pair
ssh-keygen -t rsa -f /home/hybridos/.ssh/id_rsa -q -P ""

# copy public key to remote server
ssh-copy-id -i ~/.ssh/id_rsa.pub $1
