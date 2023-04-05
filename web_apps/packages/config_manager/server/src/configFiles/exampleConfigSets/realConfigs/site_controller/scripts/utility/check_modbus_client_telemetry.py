#!/usr/bin/env python

import json
import subprocess
import argparse

FIMS_SEND="/usr/local/bin/fims_send"

def fimsGet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/me", "-u", uri], stderr=subprocess.STDOUT).strip()

def printURIValue(compID, id):
    uri = "/components/{}/{}".format(compID, id)
    value = fimsGet(uri)
    print("        {:<40}: {}".format(id, value))

def checkTelemetry(filepath):
    with open(filepath) as config_file:
        config = json.load(config_file)
    
    for component in config["components"]:
        compID = component["id"]
        print("Component ID: " + compID)
        print("Data Point:")
        for register in component["registers"]:
            print("    " + register["type"])
            for data_point in register["map"]:
                if "individual_bits" in data_point and data_point["individual_bits"]:
                    for bit_string in data_point["bit_strings"]:
                        if bit_string:
                            printURIValue(compID, bit_string)
                else:
                    printURIValue(compID, data_point["id"])
            print
        print

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("filepath", help="the filepath to the Modbus client config file (e.g. /path/to/modbus_client.json")
    args = parser.parse_args()
    checkTelemetry(args.filepath)

if __name__ == "__main__":
    main()