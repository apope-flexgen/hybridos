#!/usr/bin/env python

import json
import subprocess
import argparse

FIMS_SEND="/usr/local/bin/fims_send"

def fimsGet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/me", "-u", uri], stderr=subprocess.STDOUT).strip()


def checkTelemetry(filepath):
    with open(filepath) as uri_list_file:
        data = uri_list_file.read()

        uri_list = data.split("\n")

        uri_list_file.close()

        new_list = [uri.strip() for uri in uri_list]

        for item in new_list:
            print("        {:<80}: {}".format( item, fimsGet(item)))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("filepath", help="the filepath to the Modbus client config file (e.g. /path/to/modbus_client.json")
    args = parser.parse_args()
    checkTelemetry(args.filepath)

if __name__ == "__main__":
    main()