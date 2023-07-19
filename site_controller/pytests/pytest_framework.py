# directory reach
import logging
import signal
import os
import sys
import time
from types import FunctionType
from subprocess import PIPE, run
from threading import Thread

from .pytest_steps import Setup, Teardown
from .assertion_framework import Assertion_Type, Flex_Assertion
from .fims import fims_get, fims_set, fims_del
from .config_migrations import Config_Migrator

# Achieves a single site controller launch and teardown through a singleton class.
sc_singleton = None


class Site_Controller_Instance:
    @staticmethod
    def get_instance():
        global sc_singleton
        if (sc_singleton == None):
            sc_singleton = Site_Controller_Instance()
        return sc_singleton

    # Start site and enable its communication for tests
    def __init__(self):
        self.mig = Config_Migrator()
        running_status = Setup(
            "start_site",
            {"/site/operation/enable_flag": True},
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", True, 10)
        )
        running_status.run_steps()

    # Start site_controller with custom configuration
    # auto_restart: whether to automatically restart the site after the process has restarted
    def restart_site_controller(self, auto_restart=True):
        """
            Restart site_controller with custom configuration and no persistent setpoints
        """
        # delete persistent setpoints so site_controller restarts in a fresh state
        fims_del('/dbi/site_controller/setpoints')
        # Find pid of current site_controller process
        pid = get_sc_pid()
        logging.info(f"Killing SC pid {pid}")
        run(f"kill {pid}", shell=True, check=True)
        logging.info("Sleeping 2s until site controller is rebooted by MCP")
        time.sleep(2)
        # Don't automatically restart the site if specified
        if not auto_restart:
            return

        running_status = Setup(
            "start_site",
            {"/site/operation/enable_flag": True},
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", True, 5)
        )
        running_status.run_steps()


def get_sc_pid():
    pid = run(
        "ps aux|grep '/bin/site_controller'|grep -v 'grep'|awk '{print $2}'",
        shell=True, check=True, stdout=PIPE
    ).stdout.strip()
    if (len(pid) <= 0):
        # If SC isn't running, other deps probably aren't either. Therefore, exit.
        logging.error("No site_controller process found. Exiting.")
        sys.exit(1)
    return pid.decode()


# Get aggregate of the asset provided
def get_asset_agg(asset: str) -> float:
    # TODO: POI feeder not first
    if asset == "feed":
        return fims_get("/assets/feeders/feed_1/active_power")["active_power"]
    elif asset == "gen":
        full_asset_name = "generators"
    else:
        full_asset_name = asset

    # Get resulting ess active powers
    full_assets = fims_get(f"/assets/{full_asset_name}")
    num_assets = len(full_assets) - 1

    total_power = 0
    for i in range(num_assets):
        base_uri = f"/assets/{full_asset_name}/{asset}_{i+1}/"
        in_maint = fims_get(base_uri + "maint_mode")["value"]
        if in_maint:
            continue
        response = fims_get(base_uri + "active_power")
        if isinstance(response, dict):
            response = response["active_power"]
        total_power += response
    return total_power


# if unable to continue running tests for some reason, exit
def discontinue_tests(exit_code: int):
    os._exit(exit_code)


# Make sure we clean up properly on sigint
def signal_handler(sig: int, frame: FunctionType):
    discontinue_tests(0)


# Handle sigint to ensure proper teardown occurs
signal.signal(signal.SIGINT, signal_handler)
