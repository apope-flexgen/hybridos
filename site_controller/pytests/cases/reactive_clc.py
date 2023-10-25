import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from ..pytest_steps import Setup, Steps, Teardown


# Reactive Power CLC
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "reactive_setpoint_clc",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 2,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0,
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/standalone_power/reactive_power_closed_loop_step_size_kW": 5,  # Keep step size under asset slew rates
            "/components/bess_aux/reactive_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_step_size_kW", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/reactive_power", 1000)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": -2000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -2000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 3000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -950, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 2000)
        ]
    ),
    # Put all assets in maint mode. In this feature demand will go to 0 which deviates outside steady state and resets offset
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 1050)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0 i.e. the uncorrected value
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 1050, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -950, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 2000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/components/bess_aux/reactive_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -2000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 1000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -3000, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 2000)
        ]
    ),
    # Put all assets in maint mode. In this feature demand will go to 0 which deviates outside steady state and resets offset
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -950)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0 i.e. the uncorrected value
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -950, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -3000, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 2000)
        ]
    ),
    # Same tests, positive command
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 2000,
            "/components/bess_aux/reactive_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 2000, wait_secs=40),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -950)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 3000, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -2000)
        ]
    ),
    # Put all assets in maint mode. In this feature demand will go to 0 which deviates outside steady state and resets offset
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 1050)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", 1050, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 3000, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -2000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/components/bess_aux/reactive_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 2000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -3000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/reactive_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 1050, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -2000)
        ]
    ),
    # Put all assets in maint mode. In this feature demand will go to 0 which deviates outside steady state and resets offset
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -950)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -950, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 1050, wait_secs=40),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/reactive_power", -2000)
        ]
    ),
    # Teardown steps
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0,
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/components/bess_aux/reactive_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/reactive_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=10)
        ]
    )
])
def test_reactive_setpoint_clc(test):
    return test


# Reactive Power CLC with zero bypass
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "reactive_clc_zero_bypass",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 2,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0,
            "/features/standalone_power/reactive_power_closed_loop_enable": True,
            "/features/standalone_power/reactive_power_closed_loop_zero_bypass_enable": True,
            "/features/standalone_power/reactive_power_closed_loop_zero_bypass_deadband_kVAR": 300,
            "/features/standalone_power/reactive_power_closed_loop_step_size_kW": 10,  # Large step size to speed up tests 
            "/components/bess_aux/reactive_power_setpoint": -500
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_zero_bypass_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_zero_bypass_deadband_kVAR", 300),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_step_size_kW", 10),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/reactive_power", 500)
        ]
    ),
    Steps(
        {
            # Reactive power command set to value outside zero bypass deadband, CLC applies as normal
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/ess_actual_kVAR", 1540, wait_secs=10), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", -1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 1540)
        ]
    ),
    Steps(
        {
            # Reactive power command set to 0, CLC bypassed (expected behavior comes from behavior when the same command is sent when CLC is turned off)
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/ess_actual_kVAR", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 540),        
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 0)
        ]
    ),
    Steps(
        {
            # Reactive power command set to nonzero value inside deadband, CLC bypassed (expected behavior comes from behavior when the same command is sent when CLC is turned off)
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 100
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/ess_actual_kVAR", 100),         
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 440),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 100)
        ]
    ),
    Steps(
        {
            # Reset for next test 
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/ess_actual_kVAR", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 540),        
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 0)
        ]
    ),
    Steps(
        {
            # Test with a slow Reactive Power Setpoint slew rate 
            "/features/reactive_power/reactive_setpoint_kVAR_slew_rate": 10,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 100
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/ess_actual_kVAR", 100, wait_secs=10),         
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/feeder_actual_kVAR", 440),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 100)
        ]
    ),
    # Teardown steps
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/standalone_power/reactive_power_closed_loop_enable": False,
            "/features/standalone_power/reactive_power_closed_loop_zero_bypass_enable": False,
            "/features/standalone_power/reactive_power_closed_loop_step_size_kW": 1,
            "/features/reactive_power/reactive_setpoint_kVAR_slew_rate": 1000,
            "/features/reactive_power/reactive_setpoint_kVAR_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_zero_bypass_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_step_size_kW", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_slew_rate", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/reactive_setpoint_kVAR_cmd", 0)
        ]
    )
])
def test_reactive_clc_zero_bypass(test):
    return test