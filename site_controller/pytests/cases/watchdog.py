# Automated Action tests
from subprocess import Popen, PIPE, run
from time import sleep
import threading
from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_framework import Site_Controller_Instance
from ..pytest_steps import Setup, Steps, Teardown
from ..fims import fims_set, fims_get
from ..pytest_utils.fims_listen_parser import grok_reply

#####################################################################HELPER FUNCS########################################################################

def pause_psm():
    run("docker pause psm", shell=True)

def resume_psm():
    run("docker unpause psm", shell=True)

def clear_faults():
    sleep(1.5)
    fims_set("/assets/ess/ess_2/clear_faults", True)
    sleep(1.5)

prior_config = {}
def config_edit(grab_prior: bool):
    global prior_config
    if grab_prior:
        prior_config = fims_get("/dbi/site_controller/variables/variables/features/site_operation")
    sensible_heart_config = {
            "heartbeat_counter": {
                "name": "Heartbeat Counter",
                "ui_type": "status",
                "value": 1,
                "var_type": "Int"
                },
            "heartbeat_duration_ms": {
                "name": "Heartbeat Duration",
                "ui_type": "none",
                "unit": "ms",
                "value": 1000,
                "var_type": "Int"
                },
            "watchdog_duration_ms": {
                "name": "Watchdog Timer Duration",
                "ui_type": "none",
                "unit": "ms",
                "value": 5000,
                "var_type": "Int"
                },
            "watchdog_enable": {
                "name": "Enable Watchdog",
                "type": "enum_slider",
                "ui_type": "control",
                "value": False,
                "var_type": "Bool"
                },
            "max_heartbeat": {
                "name": "Watchdog Max Heartbeat",
                "ui_type": "control",
                "value": 255,
                "var_type": "Int"
                },
            "min_heartbeat": {
                "name": "Watchdog Min Heartbeat",
                "ui_type": "control",
                "value": 0,
                "var_type": "Int"
                },
            "watchdog_pet": {
                "name": "Watchdog Pet",
                "ui_type": "none",
                "value": 1,
                "var_type": "Int"
                }
            }

    edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/variables/variables/features/site_operation",
            "up": sensible_heart_config,
            "down": prior_config,
        }
    ]
    return edits

stdout = []
def listen_within(min: int, max: int):
    """listen for all heartbeats and make sure it's within bounds"""
    def thread_timer(timeout: int):
        """ Spawn a thread that will be an indicator to kill the coming subprocess.
        Why a thread??? Answer: So that we can digest the pubs in real time. """
        sleep(timeout)

    global stdout
    stdout = []
    uri = "/features/site_operation"

    # run proc for timeout seconds then kill it and collect the output
    cmd = ["fims_listen"]
    if uri is not None:
        cmd.append("-u")
        cmd.append(uri)
    reply = None
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE, universal_newlines=True)
    listen_range = max - min
    thread = threading.Thread(target=thread_timer, args=[listen_range + 5])
    thread.start()

    # while the proc is alive
    while proc.poll() is None:
        if not thread.is_alive():
            proc.kill()
            break

        # read a single line if you can
        if proc.stdout is not None:
            stdout.append(proc.stdout.readline())
        reply = grok_reply(stdout=stdout)
        if reply is not None:
            stdout = [] # you have digested a full fims_listen discard.
            heartbeat_value = reply.body["heartbeat_counter"]["value"]
            assert(not (heartbeat_value > max or heartbeat_value < min))

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

# test heartbeat fims endpoints
@ fixture
@ parametrize("test", [
    Setup(
        "beat_goes_on_and_on_and_on_and",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(config_edit(True)),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    # not going to bother testing the original 0-255 as long as they changed it's good enough for me
    Steps(
        {
            "/features/site_operation/max_heartbeat": 5,
            "/features/site_operation/min_heartbeat": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/max_heartbeat", 5, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/min_heartbeat", 0, wait_secs=0),
        ],
        post_lambda=[
            lambda: sleep(1.5),
            lambda: listen_within(min=0, max=5),
        ]   
    ),
    Teardown(
        {},
        [],
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance(
            ).mig.download(config_edit(False)),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
        ]
    )
])
def test_watchdog_fims_endpoints(test):
    return test
