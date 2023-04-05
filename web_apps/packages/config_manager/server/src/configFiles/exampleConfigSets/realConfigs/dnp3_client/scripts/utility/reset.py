#!/usr/bin/env python

import json
import subprocess
import argparse

FIMS_SEND="/usr/local/bin/fims_send"

def fimsSet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "set", "-r","/me", "-u", uri, "0"], stderr=subprocess.STDOUT).strip()
    
def fimsSetTrue(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "set", "-r", "/me", "-u", uri, "false"], stderr=subprocess.STDOUT).strip()

def printURIValue(base_uri, id):
    uri = "{}/{}".format(base_uri, id)
    value = fimsSet(uri).split(id)[1][2:-1]  # e.g. value = {"watchdog_pet":0} => ":0} => 0
    print("        {:<80}: {}".format(id, value))

def printCROBURIValue(base_uri, id):
    uri = "{}/{}".format(base_uri, id)
    value = fimsSetTrue(uri).split(id)[1][2:-1]  # e.g. value = {"watchdog_pet":0} => ":0} => 0
    print("        {:<80}: {}".format(id, value))

def checkTelemetry(filepath):
    with open(filepath) as config_file:
        config = json.load(config_file)

    base_uri = "{}/{}".format(config["system"]["base_uri"], config["system"]["id"])
    print("Base URI: "+ base_uri)
    print("Data Points:")

    for register in config["registers"]:
        print("    " + register["type"])
        if register["type"] == "AnOPInt32":
            for data_point in register["map"]:
                printURIValue(base_uri, data_point["id"])
            print
        if register["type"] == "CROB":
            for data_point in register["map"]:
                printCROBURIValue(base_uri, data_point["id"])
            print

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("filepath", help="the filepath to the DNP3 client config file (e.g. /path/to/dnp3_client.json")
    args = parser.parse_args()
    checkTelemetry(args.filepath)

if __name__ == "__main__":
    main()