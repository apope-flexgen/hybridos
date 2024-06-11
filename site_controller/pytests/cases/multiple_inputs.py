import pytest

from pytest_cases import parametrize, fixture

from cases.active_power import APS_kW_slew_uri
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_framework import Site_Controller_Instance
from ..pytest_steps import Setup, Steps, Teardown
from ..fims import fims_get, fims_set

apskWcmd = {}
apskWSlew = {}
APS_kW_cmd_uri = "/dbi/site_controller/variables/variables/features/active_power/active_power_setpoint_kW_cmd"

def modify_variables():
    global apskWcmd
    global apskWSlew

    apskWcmd = fims_get(APS_kW_cmd_uri)
    apskWSlew = fims_get(APS_kW_slew_uri)
    apskWcmdcopy = apskWcmd
    apskWSlewcopy = apskWSlew
    apskWcmdcopy['multiple_inputs'] = True
    apskWSlewcopy['multiple_inputs'] = True

    config_edits: list[dict] = [
        {
            "uri": APS_kW_cmd_uri,
            "up": apskWcmdcopy
        },
        {
            "uri": APS_kW_slew_uri,
            "up": apskWSlewcopy
        }
    ]
    return config_edits

def restore_original():
    fims_set(APS_kW_cmd_uri, apskWcmd)
    fims_set(APS_kW_slew_uri, apskWSlew)

@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "test_multiple_inputs",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(modify_variables()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True),
        ]
    ),
    Steps(
        {
            "/site/input_sources/local": True,
            "/site/input_sources/remote": True,
            "/site/input_sources/dnp3": True,
            "/site/input_sources/ui": True,
        },
        [
            # make sure the state makes sense
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/ui", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/dnp3", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/local", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/remote", True),

            # make sure the ui is "highlighted"
            Flex_Assertion(Assertion_Type.obj_eq, "/features/active_power", True, pattern="active_power_setpoint_kW_cmd_ui.enabled"),
        ]
    ),
    Steps(
        {
            "/site/input_sources/ui": False,
            "/site/input_sources/dnp3": False,
            "/site/input_sources/remote": False,
            "/site/input_sources/local": False,
        },
        [
            # make sure the state makes sense
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/ui", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/dnp3", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/remote", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/input_sources/local", True),

            # make sure the ui is NOT "highlighted"
            Flex_Assertion(Assertion_Type.obj_eq, "/features/active_power", False, pattern="active_power_setpoint_kW_cmd_ui.enabled"),
        ]
    ),
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd_local": 100,
            "/features/active_power/active_power_setpoint_kW_cmd_remote": 110,
            "/features/active_power/active_power_setpoint_kW_cmd_dnp3": 120,
            "/features/active_power/active_power_setpoint_kW_cmd_ui": 130,
        },
        [
            # make sure the value is 100 aka none of the other enpoints are valid
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 100),
        ]
    ),
    Steps(
        {
            "/site/input_sources/remote": True,
        },
        [
            # make sure the value is 110 aka when I enable the remote endpoint takes over
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 110),
        ]
    ),
    Steps(
        {
            "/site/input_sources/dnp3": True,
        },
        [
            # make sure the value is 120 aka when I enable the dnp3 endpoint takes over
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 120),
        ]
    ),
    Steps(
        {
            "/site/input_sources/ui": True,
        },
        [
            # make sure the value is 130 aka when I enable the dnp3 endpoint takes over
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 130),

            # make sure the ui is "highlighted"
            Flex_Assertion(Assertion_Type.obj_eq, "/features/active_power", True, pattern="active_power_setpoint_kW_cmd_ui.enabled"),
        ]
    ),
    # TRANSITION 
    # now I'm testing slew behavior for the aps feature slew using multiple_inputs
    Steps(
        {
            # "/site/input_sources/ui": True, # still in ui mode
            "/features/active_power/active_power_setpoint_kW_slew_rate_ui": 10000, # 10 MW slew for ui
            "/features/active_power/active_power_setpoint_kW_cmd_ui": 0, # get to zero to make test simple
            "/features/active_power/active_power_setpoint_kW_slew_rate_dnp3": 100, # .1 MW slew for dnp3
            "/features/active_power/active_power_setpoint_kW_cmd_dnp3": 0, # dnp3 slew is going to be going back to zero
        },
        [
            # slew to 0
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
        ]
    ),
    # We are in ui mode we should be able to slew fast @ 10MW/second
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd_ui": 10000, # slew to 10MW
        },
        [
            # slew to 10 MW, 5 seconds is plenty of time to get there (trying to allow for slower asset lvl slew rate)
            # the point is the dnp3 version at 0.1MW won't get there in 5 seconds
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feature_kW_demand", 10000, wait_secs=5), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 10000, wait_secs=0),
        ]
    ),
    # enable dnp3 should slew towards 0 at a slow rate
    Steps(
        {
            "/site/input_sources/ui": False,
        },
        [
            # slew towards 0 MW for 10 seconds, 
            # should get to 9 MW
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feature_kW_demand", 9000, wait_secs=10), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feature_kW_demand", 8500, wait_secs=5), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0, wait_secs=0),
        ]
    ),
    # enable ui should slew back up very fast
    Steps(
        {
            "/site/input_sources/ui": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feature_kW_demand", 10000, wait_secs=2), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 10000, wait_secs=0),
        ]
    ),
    Teardown(
        {},
        [],
        post_lambda=[
            lambda: restore_original(),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    )
])
def test_multiple_inputs(test):
    return test
