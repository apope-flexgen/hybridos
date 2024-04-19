# Automated Action tests
from os import times
from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown
from ..fims import fims_set
from subprocess import run
from time import sleep

#####################################################################HELPER FUNCS########################################################################

def pause_psm():
    run("docker pause psm", shell=True)

def resume_psm():
    run("docker unpause psm", shell=True)

def clear_faults():
    sleep(1.5)
    fims_set("/assets/ess/ess_2/clear_faults", True)
    sleep(1.5)

#####################################################################TESTS########################################################################

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Test maint mode interactions",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True, wait_secs=0),
        ],
        pre_lambda=[
            lambda: Steps.place_assets_in_maint_dynamic(solar=True, gen=True, ess=True),
        ]
    ),
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/is_faulted", True, wait_secs=7), # watchdog timeout is at 5 seconds
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/watchdog_status", False, wait_secs=0), # watchdog timeout is at 5 seconds
        ],
        pre_lambda=[
            lambda: pause_psm(),
        ]    
    ),
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/is_faulted", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/watchdog_status", True, wait_secs=0), # watchdog timeout is at 5 seconds
        ],
        pre_lambda=[
            lambda: resume_psm(),
            lambda: clear_faults(), # had trouble with doing this normally
        ]   
    ),
    Teardown(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ],
        pre_lambda=[
            lambda: Steps.remove_all_assets_from_maint_dynamic(),
        ]
    )
])
def test_watchdog_when_in_maintenance(test):
    return test
