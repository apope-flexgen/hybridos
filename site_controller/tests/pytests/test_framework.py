# directory reach
import logging
import signal
import os
import time
from pathlib import Path
import sys

from pytest import ExitCode
import pytest
# TODO: this is dumb. Started working on installing these as packages using pip
# There's surely a better approach than these relative imports
# Import pyfims from fims
currentDir = Path(__file__).absolute()
fimsDir = os.path.join(currentDir.parent.parent.parent.parent, "fims/pyfims")
sys.path.append("/home/pyexecnetcache/pyfims")
sys.path.append("/home/vagrant/git/fims/pyfims")
from pyfims_rewrite import pyfims  # noqa Disable formatting for this line. It will break the import

# Import hybridos run from scripts
scriptsDir = os.path.join(currentDir.parent.parent.parent.parent, "scripts")
sys.path.append("/home/pyexecnetcache/scripts")
sys.path.append("/home/vagrant/git/scripts")
from vsaap import launch_sites  # noqa
from hybridos_run import service_struct, stop_hybridos, enable_specific_services, run_hybridos  # noqa
from modify_config import update_variables, revert_file  # noqa

pfims = None


# Launch hybridos services
def launch_services():
    enable_specific_services({"fims": service_struct("fims"), "site_controller": service_struct("site_controller", False),
                              "modbus_client": service_struct("modbus_client")})
    run_hybridos(True)


# Startup fims connection
def create_pyfims_instance():
    # connect to fims server
    global pfims
    pfims = pyfims()
    x = pfims.Connect("pytest")

    # if unable to connect to fims server, cannot continue with tests
    if x == None or x[1] != 0:
        logging.error("Error connecting to fims %s", x[1])
        discontinue_tests(ExitCode.INTERRUPTED)

    return pfims


# Start site and enable its communication
def start_site(pfims):
    # Waiting for twins container to start up with modified config
    time.sleep(10)
    pfims.SendSet("/site/operation/primary_controller", True)
    pfims.SendSet("/site/operation/enable_flag", True)
    # Waiting for site to start with response from all twins assets
    time.sleep(5)
    running_status = pfims.SendGet("/site/operation/running_status_flag", "/pytest")[1]["value"]
    if running_status != True:
        logging.error("Site failed to start")
        discontinue_tests(ExitCode.INTERNAL_ERROR)


# Get aggregate of the asset priovided
def get_asset_agg(asset):
    if asset == "feed":
        return pfims.SendGet("/assets/feeders/feed_1/active_power", "/pytest")[1]["active_power"]["value"]
    elif asset == "gen":
        full_asset_name = "generators"
    else:
        full_asset_name = asset

    # Get resulting ess active powers
    num_assets = pfims.SendGet(f"/assets/{full_asset_name}/summary/num_{asset}_running", "/pytest")[1]

    total_power = 0
    for i in range(num_assets):
        uri = f"/assets/{full_asset_name}/{asset}_{i+1}/active_power"
        logging.error("uri %s", uri)
        response = pfims.SendGet(f"/assets/{full_asset_name}/{asset}_{i+1}/active_power", "/pytest")[1]
        logging.error("got response %s", response)
        if isinstance(response, dict):
            response = response["active_power"]["value"]
        total_power += response
    return total_power


# Cleanup fixture resources
def cleanup():
    if pfims != None:
        pfims.Close()

    # No need to revert as it will modify file inodes and break synching via volume mounting
    # Instead files are copied so we are free to edit them as needed
    # Preserving as a reference
    # revert_file("/usr/local/etc/config/site_controller/assets.json")
    # revert_file("/usr/local/etc/config/site_controller/sequences.json")
    # revert_file("/usr/local/etc/config/site_controller/variables.json")
    # revert_file("/usr/local/etc/config/twins/twins/twins.json")

    stop_hybridos()


# if unable to continue running tests for some reason, exit
def discontinue_tests(exit_code):
    cleanup()
    os._exit(exit_code)


# Make sure we clean up properly on sigint
def signal_handler(sig, frame):
    discontinue_tests(0)


# Handle sigint to ensure proper teardown occurs
signal.signal(signal.SIGINT, signal_handler)
