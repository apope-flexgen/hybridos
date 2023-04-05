#!/usr/bin/env python

import json
import subprocess
import argparse
import re

FIMS_SEND="/usr/local/bin/fims_send"

def fimsGet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/me", "-u", uri], stderr=subprocess.STDOUT).strip()
    
def printURIValue(base_uri, id):
    uri = "{}/{}".format(base_uri, id)
    value = fimsGet(uri)

    # Extract the value stored in the value field if the field exists
    if "value" in value:
        value = re.search('(?<="value":).*?(?=,)', value).group() 

    print("        {:<60}: {}".format(uri, value))

def checkTelemetry(filepath):
    with open(filepath) as config_file:
        config = json.load(config_file)

    print("Data Points:")

    for register in config["registers"]:
        print("    " + register["type"])
        for data_point in register["map"]:
            printURIValue(data_point["uri"], data_point["id"])
        print

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("filepath", help="the filepath to the DNP3 server config file (e.g. /path/to/dnp3_server.json")
    args = parser.parse_args()
    checkTelemetry(args.filepath)

if __name__ == "__main__":
    main()