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
            "/features/active_power/runmode1_kW_mode_cmd": 4,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/active_power/fr_enable_mask": 24,
            "/features/active_power/fr_baseload_cmd_kw": 0,
            "/features/standalone_power/poi_limits_enable": False,
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/features/standalone_power/active_power_closed_loop_step_size_kW": 10,  # Large step size to speed up tests
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 4),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/fr_enable_mask", 24),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/fr_baseload_cmd_kw", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_step_size_kW", 10),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 1000, wait_secs=10),
        ]
    ),
    Steps(
        {
            "/features/active_power/fr_baseload_cmd_kw": -5000
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
            "/features/active_power/fr_baseload_cmd_kw": 5000,
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
            "/features/active_power/fr_baseload_cmd_kw": 0,
            "/features/standalone_power/active_power_closed_loop_enable": False,
            "/components/bess_aux/active_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/fr_baseload_cmd_kw", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", 0),
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
