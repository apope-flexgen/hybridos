import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# Constant Power Factor Test Steps
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "constant_power_factor",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/export_target_kW_cmd": 10000,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 4,
            "/features/reactive_power/constant_power_factor_lagging_limit": -0.95,
            "/features/reactive_power/constant_power_factor_leading_limit": 0.95,
            "/features/reactive_power/constant_power_factor_absolute_mode": True,
            "/features/reactive_power/constant_power_factor_lagging_direction": True,
            "/features/site_operation/power_priority_flag": True,
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/components/bess_aux/reactive_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/export_target_kW_cmd", 10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 4),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_lagging_limit", -0.95),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_leading_limit", 0.95),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_absolute_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_lagging_direction", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 50)  # 50 from transformer losses
        ]
    ),
    # Positive Active, Lagging absolute mode
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 1.01  # Above valid pf range, limited to -1.0
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 0)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 0.97,  # Within range
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -2500)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": -0.97,  # Within range, bidirectional mode
            "/features/reactive_power/constant_power_factor_absolute_mode": False,
            "/features/reactive_power/constant_power_factor_lagging_direction": False,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -2500)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": -0.89,  # Below valid pf range, limited to -0.95
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -3287, wait_secs=5)
    ),
    # pf limit removed
    Steps(
        {
            "/features/reactive_power/constant_power_factor_lagging_limit": 0.0,
            "/features/reactive_power/constant_power_factor_setpoint": -0.89,  # Now within pf range,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -5000)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": -0.67,  # Max site output
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -11000, wait_secs=10)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_absolute_mode": True,   # Use absolute mode as a negative zero
            "/features/reactive_power/constant_power_factor_lagging_direction": True,  # is converted to positive over fims
            "/features/reactive_power/constant_power_factor_setpoint": 0.0,  # Divide by zero case
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -11000)
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,  # Limited by POI limits
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -10000
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -10000, wait_secs=5)
    ),
    Steps(
        {
            "/features/site_operation/power_priority_flag": False,  # Further limited by active power priority
        },
        # Due to soc balancing only one battery will have reactive power capability
        # So available q is sqrt(5500^2 - 4500^2) not sqrt(11000^2-10000^2)
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -3162, wait_secs=10)
    ),
    Steps(
        {

            "/features/site_operation/power_priority_flag": True,
            "/features/reactive_power/constant_power_factor_lagging_limit": 0.97,
        },
        [
            # Demand is accurate but POI value is inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -2500),
            Flex_Assertion(Assertion_Type.approx_neq, "/features/reactive_power/feeder_actual_kVAR", -2500)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
        },
        # POI value is accurate with CLC
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 2500, wait_secs=10)
    ),
    # Same tests on leading (positive) side
    Steps(
        {
            "/features/site_operation/power_priority_flag": True,
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/reactive_power/constant_power_factor_lagging_direction": False,
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/reactive_power/constant_power_factor_setpoint": 1.01  # Above valid pf range, limited to -1.0
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 0)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 0.97,  # Within range
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 2500)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 0.97,  # Within range, bidirectional mode
            "/features/reactive_power/constant_power_factor_absolute_mode": False,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 2500)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 0.89,  # Below valid pf range, limited to -0.95
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 3287, wait_secs=5)
    ),
    # pf limit removed
    Steps(
        {
            "/features/reactive_power/constant_power_factor_leading_limit": 0.0,
            "/features/reactive_power/constant_power_factor_setpoint": 0.89,  # Now within pf range,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 5000)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 0.67,  # Max site output
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 11000, wait_secs=10)
    ),
    Steps(
        {
            "/features/reactive_power/constant_power_factor_setpoint": 0.0,  # Divide by zero case
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 11000)
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_poi_limits_enable": True,  # Limited by POI limits
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 10000
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 10000, wait_secs=5)
    ),
    Steps(
        {
            "/features/site_operation/power_priority_flag": False,  # Further limited by active power priority
        },
        # Due to soc balancing only one battery will have reactive power capability
        # So available q is sqrt(5500^2 - 4500^2) not sqrt(11000^2-10000^2)
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 3162, wait_secs=10)
    ),
    Steps(
        {
            "/features/site_operation/power_priority_flag": True,
            "/features/reactive_power/constant_power_factor_leading_limit": 0.97,
        },
        [
            # Demand is accurate but POI value is inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 2500),
            Flex_Assertion(Assertion_Type.approx_neq, "/features/reactive_power/feeder_actual_kVAR", 2500)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
        },
        # POI value is accurate with CLC
        Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", -2500, wait_secs=10)
    ),
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/export_target_kW_cmd": 0,
            "/features/reactive_power/constant_power_factor_lagging_limit": -0.95,
            "/features/reactive_power/constant_power_factor_leading_limit": 0.95,
            "/features/reactive_power/constant_power_factor_absolute_mode": True,
            "/features/reactive_power/constant_power_factor_lagging_direction": True,
            "/features/site_operation/power_priority_flag": False,
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/export_target_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_lagging_limit", -0.95),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_leading_limit", 0.95),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_absolute_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/constant_power_factor_lagging_direction", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/power_priority_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", False),
        ]
    )
])
def test_constant_power_factor(test):
    return test
