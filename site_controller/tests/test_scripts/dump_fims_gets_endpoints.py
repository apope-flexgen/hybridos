# Prints out gets at all endpoints
# Assumes that non-endpoint gets work
import sys
import subprocess
import json

uris = [
    "/features/summary",
    "/features/active_power",
    "/features/reactive_power",
    "/features/standalone_power",
    "/features/site_operation",

    "/site/summary",
    "/site/operation",
    "/site/configuration",
    "/site/cops",
    "/site/input_sources",

    "/assets/ess/summary",
    "/assets/ess/ess_1",
    "/assets/ess/ess_2",
    "/assets/feeders/summary",
    "/assets/feeders/feed_1",
    "/assets/feeders/feed_2",
    "/assets/generators/summary",
    "/assets/generators/gen_1",
    "/assets/generators/gen_2",
    "/assets/solar/summary",
    "/assets/solar/solar_1",
    "/assets/solar/solar_2",
]
# Loop for all desired parent uris
for uri in uris:
    # Get the parent JSON object
    msg = subprocess.run(["fims_send", "-m", "get", "-u", uri, "-r", "/me"], stdout=subprocess.PIPE).stdout
    # Parse the JSON for endpoints
    try:
        msg_json = json.loads(msg)
    except ValueError:
        print("get at " + uri + ":")
        print("Invalid JSON response\n")
        continue
    for endpoint in msg_json:
        # Get the endpoint and print reply
        print("get at " + uri + "/" + endpoint + ":")
        print(subprocess.run(["fims_send", "-m", "get", "-u", uri + "/" + endpoint, "-r", "/me"], stdout=subprocess.PIPE).stdout.decode())