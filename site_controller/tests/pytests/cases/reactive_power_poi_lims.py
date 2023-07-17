import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# Reactive Power POI Limits test steps
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "reactive_power_poi_lims",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/export_target_kW_cmd": 0,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 2,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0,
            "/features/standalone_power/active_power_soc_poi_limits_enable": False,
            "/features/site_operation/power_priority_flag": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/export_target_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_soc_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", False),
        ]
    ),
    # Basic positive reactive test, no limits, no active power feature/limits
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 10000
        },
        # Long wait while slewing from zero to large positive command
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 10000, wait_secs=10)
    ),
    # Within positive limit, no active power feature/limits
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -15000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 15000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 10000
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 10000, wait_secs=1)
    ),
    # Outside positive limit, no active power feature/limits
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -5000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 5000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 10000
        },
        [
            # First assertion ensures the limit has been applied but the slew has not been violated
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/reactive_power/site_kVAR_demand", 9500, wait_secs=1),
            # Final assertion ensures the limit has been followed after slewing
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 5000, wait_secs=10),
        ]
    ),
    # Outside positive limit, unlimited active power feature reduces available power
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -5000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 5000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 10000,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/active_power/export_target_kW_cmd": 10000
        },
        Flex_Assertion(Assertion_Type.less_than_eq, "/features/reactive_power/site_kVAR_demand", 4000),
    ),
    # Outside positive limit, limited active power feature makes more reactive power available again
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -5000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 5000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 10000,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_min_kW": -2500,
            "/features/standalone_power/active_power_poi_limits_max_kW": 2500,
            "/features/active_power/export_target_kW_cmd": 10000
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 5000),
    ),
    # Basic negative reactive feature test, no limits, no active power feature/limits
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -10000
        },
        # Long wait while slewing from large positive to large negative command
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -10000, wait_secs=20)
    ),
    # Within negative limit, no active power feature/limits
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -15000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 15000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -10000
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -10000, wait_secs=1)
    ),
    # Outside negative limit, no active power feature/limits
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -5000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 5000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -10000
        },
        [
            # First assertion ensures the limit has been applied but the slew has not been violated
            Flex_Assertion(Assertion_Type.greater_than_eq, "/features/reactive_power/site_kVAR_demand", -9500),
            # Final assertion ensures the limit has been followed after slewing
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -5000, wait_secs=10),
        ]
    ),
    # Outside negative limit, unlimited active power feature reduces available power
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -5000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 5000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -10000,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/active_power/export_target_kW_cmd": 10000
        },
        Flex_Assertion(Assertion_Type.greater_than_eq, "/features/reactive_power/site_kVAR_demand", -4000),
    ),
    # Outside positive limit, limited active power feature makes more reactive power available again
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -5000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 5000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -10000,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_min_kW": -2500,
            "/features/standalone_power/active_power_poi_limits_max_kW": 2500,
            "/features/active_power/export_target_kW_cmd": 10000
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -5000),
    ),
    # Teardown steps
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -10000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 10000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/active_power/export_target_kW_cmd": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/export_target_kW_cmd", 0),
        ]
    )
])
def test_reactive_poi_lims(test):
    return test
