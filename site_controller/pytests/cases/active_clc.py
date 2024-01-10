import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from ..pytest_steps import Setup, Steps, Teardown


# Frequency Response with CLC
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "fr_clc",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/standalone_power/fr_enable_mask": 24,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/standalone_power/poi_limits_enable": False,
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/features/standalone_power/active_power_closed_loop_step_size_kW": 10,  # Large step size to speed up tests
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_enable_mask", 24),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_step_size_kW", 10),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 1000, wait_secs=10),
        ]
    ),
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -5000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 6000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Put all assets in maint mode. There should not be any CLC offset in this state (once correction slews to 0)
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 1050)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 1050, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 4000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -6000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Put all assets in maint mode. There should not be any CLC offset in this state (once correction slews to 0)
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -6000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Same tests, positive command
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -4000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 6000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -5000)
        ]
    ),
    # Put all assets in maint mode. There should not be any CLC offset in this state (once correction slews to 0)
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 1050)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 1050, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 6000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -5000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -6000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 4000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -5000)
        ]
    ),
    # Put all assets in maint mode. There should not be any CLC offset in this state (once correction slews to 0)
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True
        },
        [
            # Offset goes to 0 as there are no assets available
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 4000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -5000)
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 0,
            "/features/standalone_power/fr_mode_enable_flag": False
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
        ]
    )
])
def test_fr_clc(test):
    return test


# ESS Only Target SoC with CLC
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "ess_tsoc_clc",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 1,
            "/features/active_power/asset_priority_runmode1": 1,
            "/features/active_power/target_soc_load_enable_flag": False,
            "/features/active_power/ess_charge_control_kW_limit": 10000,
            "/components/bess_aux/active_power_setpoint": -1000,
            "/features/standalone_power/active_power_closed_loop_step_size_kW": 10,  # Large step size to speed up tests

        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/target_soc_load_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_charge_control_kW_limit", 10000, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 1000, wait_secs=10)
        ]
    ),
    Steps(
        {
            "/features/active_power/ess_charge_control_target_soc": 100
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -10000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 11000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -9000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 10000)
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
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 1050)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 1050, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -9000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 10000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -10000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 9000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -11000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 10000)
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
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -11000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 10000)
        ]
    ),
    # Same tests, positive command
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/features/active_power/ess_charge_control_target_soc": 0,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 10000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -9000)
        ]
    ),
    # ESS output is maxed by the CLC correction
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 11000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -10000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 10000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -11000)
        ]
    ),
    # No bug in this case as ESS can reduce its output
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 9000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -10000)
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
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950)
        ]
    ),
    # Now exit maint mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False
        },
        [
            # First make sure there is no windup. CLC should restart at offset 0. Cannot use demand as it slews instantly
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0, wait_secs=0),
            # POI inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -950, wait_secs=0),
            # Then make sure CLC closes in on the value as it did previously
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 9000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -10000)
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/active_power/ess_charge_control_target_soc": 90,
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_charge_control_target_soc", 90),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", 0, )
        ]
    )
])
def test_ess_tsoc_clc(test):
    return test


# ESS + Solar Target SoC with CLC
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "solar_tsoc_clc",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 1,
            "/features/active_power/asset_priority_runmode1": 1,
            "/features/active_power/target_soc_load_enable_flag": False,
            "/features/active_power/ess_charge_control_kW_limit": 10000,
            "/components/bess_aux/active_power_setpoint": -1000,
            "/features/standalone_power/active_power_closed_loop_step_size_kW": 10,  # Large step size to speed up tests
            "/assets/generators/gen_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/target_soc_load_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_charge_control_kW_limit", 10000, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 1000, wait_secs=10)
        ]
    ),
    Steps(
        {
            "/features/active_power/ess_charge_control_target_soc": 100
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 6000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            # Demand is modified
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 4000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -6000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Same tests, positive command
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/features/active_power/ess_charge_control_target_soc": 0,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 15000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -14000)
        ]
    ),
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 16000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -15000)
        ]
    ),
    # Same tests, negative inaccuracy
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 15000, wait_secs=10),
            # Load makes POI value inaccurate
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -16000)
        ]
    ),
    # No bug in this case as solar is available to be reduced
    Steps(
        {
            "/features/standalone_power/active_power_closed_loop_enable": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 14000, wait_secs=30),
            # CLC closes in on the commanded value
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -15000)
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/active_power/ess_charge_control_target_soc": 90,
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 0,
            "/assets/generators/gen_1/maint_mode": False
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_charge_control_target_soc", 90),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=10)
        ]
    )
])
def test_solar_tsoc_clc(test):
    return test


# Active Power Setpoint with CLC and zero bypass
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "active_clc_zero_bypass",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/standalone_power/active_power_closed_loop_step_size_kW": 10,  # Large step size to speed up tests
            "/features/standalone_power/active_power_closed_loop_enable": True,
            "/features/standalone_power/active_power_closed_loop_zero_bypass_enable": True,
            "/features/standalone_power/active_power_closed_loop_zero_bypass_deadband_kW": 300,
            "/components/bess_aux/active_power_setpoint": -500
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_step_size_kW", 10),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_zero_bypass_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_zero_bypass_deadband_kW", 300),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", -500, wait_secs=5)
        ]
    ),
    Steps(
        {
            # Active power command set to value outside zero bypass deadband, CLC applies as normal
            "/features/active_power/active_power_setpoint_kW_cmd": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 1550, wait_secs=10), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 1550)
        ]
    ),
    Steps(
        {
            # Active power command set to 0, CLC bypassed (expected behavior comes from behavior when the same command is sent when CLC is turned off)
            "/features/active_power/active_power_setpoint_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 550),        
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 0)
        ]
    ),
    Steps(
        {
            # Active power command set to nonzero value inside deadband, CLC bypassed (expected behavior comes from behavior when the same command is sent when CLC is turned off)
            "/features/active_power/active_power_setpoint_kW_cmd": 100
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 100),         
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 450),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 100)
        ]
    ),
    Steps(
        {
            # Reset for next test 
            "/features/active_power/active_power_setpoint_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 550),        
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 0)
        ]
    ),
    Steps(
        {
            # Test with a slow Active Power Setpoint slew rate 
            "/features/active_power/active_power_setpoint_kW_slew_rate": 10,
            "/features/active_power/active_power_setpoint_kW_cmd": 100
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 100, wait_secs=10),         
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 450),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 100)
        ]
    ),
    Steps(
        {
            # Reset for next test 
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_kW_slew_rate": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_total_correction", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 0), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 550),        
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 0)
        ]
    ),
    Steps(
        {
            # Negative active power command set to value outside zero bypass deadband, CLC applies as normal
            "/features/active_power/active_power_setpoint_kW_cmd": -1000,
            "/components/bess_aux/active_power_setpoint": 500
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", -1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", -1450, wait_secs=10), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -1450)
        ]
    ),
    Steps(
        {
            # Zero bypass and cmd of 0
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", 0, wait_secs=10), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 0, tolerance=50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 0)
        ]
    ),
    Steps(
        {
            # No zero bypass and cmd of 0
            "/features/standalone_power/active_power_closed_loop_zero_bypass_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_actual_kW", -450, tolerance=50, wait_secs=10), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 0, tolerance=50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -450, tolerance=50)
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/features/standalone_power/active_power_closed_loop_zero_bypass_enable": False,
            "/features/standalone_power/active_power_closed_loop_step_size_kW": 1,
            "/features/active_power/active_power_setpoint_kW_slew_rate": 1000000,
            "/features/active_power/active_power_setpoint_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_zero_bypass_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_step_size_kW", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_slew_rate", 1000000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0)
        ]
    )
])
def test_active_clc_zero_bypass(test):
    return test
