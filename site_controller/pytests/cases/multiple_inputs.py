import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_framework import Site_Controller_Instance
from ..pytest_steps import Setup, Steps, Teardown
from ..fims import fims_get, fims_set

apskWcmd = {}
APS_kW_cmd_uri = "/dbi/site_controller/variables/variables/features/active_power/active_power_setpoint_kW_cmd"

def modify_variable():
    global apskWcmd

    apskWcmd = fims_get(APS_kW_cmd_uri)
    apskWcmdcopy = apskWcmd
    apskWcmdcopy['multiple_inputs'] = True

    config_edits: list[dict] = [
        {
            "uri": APS_kW_cmd_uri,
            "up": apskWcmdcopy
        }
    ]
    return config_edits

def restore_original():
    fims_set(APS_kW_cmd_uri, apskWcmd)

@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "test_multiple_inputs",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(modify_variable()),
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
