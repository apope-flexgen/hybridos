#!/usr/bin/env python

import json
import subprocess
import argparse
import re

FIMS_SEND="/usr/local/bin/fims_send"

def fimsGet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/$$", "-u", uri], stderr=subprocess.STDOUT).strip()

def fimsSet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "set", "-u", uri, "0"], stderr=subprocess.STDOUT).strip()

def fimsSetThree(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "set", "-r", "/$$", "-u", uri, "3"], stderr=subprocess.STDOUT).strip()
    
def fimsSetBool(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "set", "-u", uri, "false"], stderr=subprocess.STDOUT).strip()

def printURIValue(base_uri, id):
    uri = "{}/{}".format(base_uri, id)
    fimsSet(uri)
    value = fimsGet(uri)

    # Extract the value stored in the value field if the field exists
    if "value" in value:
        value = re.search('(?<="value":).*?(?=,)', value).group() 

    # print("        {:<80}: {}".format(uri, value))

def printCROBURIValue(base_uri, id):
    uri = "{}/{}".format(base_uri, id)
    fimsSetBool(uri)
    value = fimsGet(uri)
    # Extract the value stored in the value field if the field exists
    if "value" in value:
        value = re.search('(?<="value":).*?(?=,)', value).group() 

    # print("        {:<80}: {}".format(uri, value))

def checkTelemetry(filepath):
    with open(filepath) as config_file:
        config = json.load(config_file)

    # print("Data Points:")

    for register in config["registers"]:
        # print("    " + register["type"])
        if register["type"] == "AnOPInt32":
            for data_point in register["map"]:
                printURIValue(data_point["uri"], data_point["id"])
        if register["type"] == "CROB":
            for data_point in register["map"]:
                printCROBURIValue(data_point["uri"], data_point["id"])

    print(fimsSetThree("/fleet/features/ercotAs/pueblo1/frrs_up_gen_responsibility_actual"))
    print(fimsSetThree("/fleet/features/ercotAs/pueblo1/frrs_down_gen_responsibility_actual"))
    print(fimsSetThree("/fleet/features/ercotAs/pueblo1/ffr_gen_responsibility"))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("filepath", help="the filepath to the DNP3 server config file (e.g. /path/to/dnp3_server.json")
    args = parser.parse_args()
    checkTelemetry(args.filepath)

if __name__ == "__main__":
    main()