#!/bin/bash

# outlet
if [[ $# -ne 2 ]]; then
    printf "usage: ./send_pdu_restart.sh <outlet> <PDU IP>\n"
elif ! [[ $1 =~ ^[0-9]+$ ]]; then
    printf "Invalid PDU outlet provided\n"
    exit -1
else
    # ssh to the PDU IP provided, reboot the outlet provided
    # No support for RSA key pair, use sshpass to send the password
    # `UserKnownHostsFile=/dev/null`, `StrictHostKeyChecking=no`: bypass the first time connection warning
    #   TODO: The connection is susceptible to MITM attacks. The host is already known from the initial SSH setup,
    #   but Golang's exec doesn't appear to support this in its bash environment
    sshpass -p <password> ssh -tt -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no apc@$2 << EOF
    olReboot $1
    exit
EOF
    exit 0
fi
