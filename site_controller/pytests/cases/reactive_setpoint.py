import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from ..pytest_steps import Setup, Steps, Teardown


# Reactive Setpoint Test Steps
# Currently just for testing Closed Loop Control with Slews
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "reactive_setpoint",
        {
            **Steps.disable_solar_and_gen(),
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 2,
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/site_operation/power_priority_flag": True,
            "/components/bess_aux/reactive_power_setpoint": -500,
            "/features/standalone_power/reactive_power_closed_loop_step_size_kVAR": 5,  # Keep step size under asset slew rates
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_step_size_kVAR", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 550),
        ]
    ),
    # Positive command, inaccurate at the POI
    Steps(
        {
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", -450)
        ]
    ),
    # Closes in with CLC
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 1550, wait_secs=20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", -1000)
        ]
    ),
    # Positive command, opposite inaccuracy
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/components/bess_aux/reactive_power_setpoint": 500
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", -1450, wait_secs=5)
        ]
    ),
    # Closes in with CLC
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 550, wait_secs=20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", -1000)
        ]
    ),
    # Negative command, inaccurate at the POI
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -1000,
            "/components/bess_aux/reactive_power_setpoint": -500
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 1550, wait_secs=5)
        ]
    ),
    # Closes in with CLC
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -450, wait_secs=20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 1000)
        ]
    ),
    # Negative command, opposite inaccuracy
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/components/bess_aux/reactive_power_setpoint": 500
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 550, wait_secs=5)
        ]
    ),
    # Closes in with CLC
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -1450, wait_secs=20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 1000)
        ]
    ),
    # Offset resets when feature is disabled
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0)
    ),
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/site_operation/power_priority_flag": False,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 5,
            "/features/standalone_power/reactive_power_closed_loop_step_size_kVAR": 1,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_step_size_kVAR", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 5),
        ]
    )
])
def test_reactive_power_setpoint(test):
    return test
