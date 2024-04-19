# Automated Action tests
from pytest_cases import parametrize, fixture
from pytests.assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from pytests.pytest_steps import Setup, Steps, Teardown
from pytests.fims import fims_set, fims_get
from pytest_utils.fims_listen_parser import listen_reply, listen_reply_validator
from subprocess import Popen, PIPE
from typing import List, Union, Any
from json import loads
import threading
import time


#####################################################################HELPER FUNCS########################################################################

def setup_soc():
    """Idea here is to make ess_1 take the brunt of the demand requested.
    When this happens the prior algorithm would have broken due to power priority"""
    fims_set("/components/ess_psm_1/bms_soc", 90)
    fims_set("/components/ess_psm_2/bms_soc", 30)
    fims_set("/components/ess_psm_3/bms_soc", 30)
    fims_set("/components/ess_psm_4/bms_soc", 30)

#####################################################################TESTS########################################################################

###### Run with config_dev_four_ess ######

@ fixture
@ parametrize("test", [
    Setup(
        "This will attempt to create the soc balancing bug",
        # put non ess into maintenance mode
        # Set reactive power feat to reactive power setpoint and set a cmd of 50 kW
        # Set reactive power priority
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 2,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 1000, # scaler is 1000 aka 1MW
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 25000, # AKA 25MW more than the ess can provide we want to go full blast scaler is 1 kinda cringe they are different lmao
            "/features/site_operation/power_priority_flag": True, 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 1000), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 25000), 
            # HERE LAY A PAINFUL WAIT, but the slews need to be sufficently low to cause the bug. I salute those waiting. 07
            # 21980 = all the ess pushing max. It is 21980 instead of 22000 because rated power gets limited by the balancing algorithm 
            # when there is reactive power present. The limited rated in this case becomes 5.49.
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 21980, wait_secs=120, tolerance=0.01), # should match feat_request as much as it can (aka sum rated ess - reactive)
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 21980, tolerance_type=Tolerance_Type.abs, tolerance=100), # should match feat_request as much as it can (aka sum rated ess - reactive)
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 21980, tolerance_type=Tolerance_Type.abs, tolerance=100), # should match feat_request as much as it can (aka sum rated ess - reactive)
        ],
        pre_lambda=[
            lambda: Steps.place_assets_in_maint_dynamic(solar=True, gen=True), 
            lambda: setup_soc(),
        ]
    ),
    Teardown(
        {
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0, 
            "/features/active_power/active_power_setpoint_kW_cmd": 0, 
            "/features/site_operation/power_priority_flag": False, 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", False),
        ],
        pre_lambda=[
            lambda: Steps.remove_all_assets_from_maint_dynamic(), 
        ]
    )
])
def test_battery_balancing_algorithm(test):
    return test
