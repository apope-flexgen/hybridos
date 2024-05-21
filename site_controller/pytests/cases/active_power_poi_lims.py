import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from ..pytest_steps import Setup, Steps, Teardown
from pytest_utils.misc import set_all_ess_soc, ESS_LABELS

# Frequency Response with Poi limits
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "fr_poi_lims",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/standalone_power/fr_enable_mask": 24,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_min_kW": -11000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 11000,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 5,
            "/components/bess_aux/active_power_setpoint": 0  # No load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_enable_mask", 24),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -11000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 11000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", 0, wait_secs=10),
        ]
    ),
    # within limit
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -5000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # beyond limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_min_kW": -4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 4000)
        ]
    ),
    # Load tests (Feature does not track load)
    # within limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_min_kW": -5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 6000, wait_secs=5)
        ]
    ),
    # beyond limit, limit violated by load
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_min_kW": -4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 5000)
        ]
    ),
    # Same tests on positive side
    # within limit
    Steps(
        {
            "/components/bess_aux/active_power_setpoint": 0,
            "/features/active_power/active_power_setpoint_kW_cmd": 5000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -5000, wait_secs=5)
        ]
    ),
    # beyond limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_max_kW": 4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -4000)
        ]
    ),
    # Load tests (Feature does not track load)
    # within limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_max_kW": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -4000, wait_secs=5)
        ]
    ),
    # Would still be within limit due to load but no tracking
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_max_kW": 4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -3000)
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/standalone_power/fr_mode_enable_flag": False
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False)
        ]
    )
])
def test_fr_poi_lims(test):
    return test


# ESS Only Target SoC with POI limits
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "ess_tsoc_poi_lims",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 1,
            "/features/active_power/asset_priority_runmode1": 1,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/active_power/target_soc_load_enable_flag": False,
            "/features/active_power/ess_charge_control_kW_limit": 10000,
            "/components/bess_aux/active_power_setpoint": 0  # No load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/target_soc_load_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_charge_control_kW_limit", 10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 0, wait_secs=5),
        ]
    ),
    # within limit
    Steps(
        {
            "/features/active_power/ess_charge_control_target_soc": 100,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -10000, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 10000),
        ]
    ),
    # beyond limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_min_kW": -9000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -9000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 9000),
        ]
    ),
    # Test untracked load
    # within limit (ignoring load)
    Steps(
        {
            "/components/bess_aux/active_power_setpoint": -1000,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -10000, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 11000),
        ]
    ),
    # beyond limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_min_kW": -9000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -9000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 10000),
        ]
    ),
    # Test poi load tracking
    # Demand reduced by load to stay in limit
    # (reduced 1k by feature discharging 1k to cover load, and another 1k because the POI is no longer available)
    Steps(
        {
            "/features/active_power/target_soc_load_enable_flag": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -7000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", 9000),
        ]
    ),
    # Same tests, discharge side
    # within limit
    Steps(
        {
            "/features/active_power/ess_charge_control_target_soc": 0,
            "/features/active_power/target_soc_load_enable_flag": False,
            "/components/bess_aux/active_power_setpoint": 0  # No load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 10000, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -10000),
        ]
    ),
    # beyond limit
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_max_kW": 9000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 9000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -9000),
        ]
    ),
    # Test untracked load
    # within limit
    Steps(
        {
            "/components/bess_aux/active_power_setpoint": -1000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 10000, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -9000),
        ]
    ),
    # Would still be within limit but load not tracked
    Steps(
        {
            "/features/standalone_power/active_power_poi_limits_max_kW": 9000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 9000),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -8000),
        ]
    ),
    # Test poi load tracking
    # Demand increased up to limit based on load
    Steps(
        {
            "/features/active_power/target_soc_load_enable_flag": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 10000, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/shared_poi/active_power", -9000),
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/active_power/ess_charge_control_target_soc": 90,
            "/features/active_power/target_soc_load_enable_flag": False,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_charge_control_target_soc", 90),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/target_soc_load_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000)
        ]
    )
])
def test_ess_tsoc_poi_lims(test):
    return test

# Active Power Setpoint ESS + Solar with POI limits
# The intent with this test is to ensure that POI limits are observed insofar as is possible when solar is uncurtailed,
# particularly when under a discharge setpoint.
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "active_power_setpoint_ess_solar_poi_lims",
        {
            "/assets/generators/gen_1/maint_mode": True,
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_maximize_solar_flag": True,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/components/bess_aux/active_power_setpoint": 0,  # No load
            "/components/bess_aux/reactive_power_setpoint": 0  # No load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 0, wait_secs=5), # need time to slew toward the setpoint
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/reactive_power", 0, wait_secs=0), # need time to slew toward the setpoint
        ]
    ),
    # Some ESS available
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -10000,
            "/assets/ess/ess_1/maint_mode": True,
        },
        [
            # Demand: -active_power_setpoint_kW_cmd (10000) + solar (5000)
            # Reduced chargeable potential to 5500
            # Actual calculation looks like -5500 + 5000
            # site_kW_demand = -500
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -500, wait_secs=5),
            # Only 5500 ESS available (SoC balancing gives 5500 instead of 5000)
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -5500, wait_secs=0),
            # Solar gets its full 5000 as well
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 550, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=0), # the 50 here is the biomass load
        ]
    ),
    # No ESS available
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -10000,
            "/assets/ess/ess_2/maint_mode": True,
        },
        [
            # Reduced chargeable potential to 0
            # Actual calculation looks like -0 + 5000
            # site_kW_demand = 5000
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000, wait_secs=5),
            # No ESS avail
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0, wait_secs=0),
            # Solar still gets its full 5000
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000, wait_secs=0),
        ]
    ),
    # Make sure POI limit is accurate
    Steps(
        {
            "/features/active_power/active_power_setpoint_maximize_solar_flag": False,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/active_power/active_power_setpoint_kW_cmd": 10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 3500,
        },
        [
            # Because of available assets we should get an aggregate of 5000 which should hit the max limit
            # Make sure the correct limit is applied and that solar gets as much power as it can
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 3500, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 3500, wait_secs=0),
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/generators/gen_1/maint_mode": False,
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_maximize_solar_flag": False,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/components/bess_aux/active_power_setpoint": -500,  # load
            "/components/bess_aux/reactive_power_setpoint": -50  # load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000, wait_secs=0)
        ]
    )
])
def test_active_power_setpoint_ess_solar_poi_lims(test):
    return test

# Here lay the most scrutiny I was willing/able to write on 
# the day of May 15th of the year 2024
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "aps_poi_limits_gauntlet",
        {
            "/assets/generators/gen_1/maint_mode": True, # don't nobody care about a generator
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_maximize_solar_flag": True,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/active_power/asset_priority_runmode1": 1, # going to use priority 1 to ensure solar is always covering load
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000, # -10 MW
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000, # 10 MW
            "/components/bess_aux/active_power_setpoint": 0,  # No load
            "/components/bess_aux/reactive_power_setpoint": 0  # No load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 1, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 0, wait_secs=3, tolerance_type=Tolerance_Type.abs, tolerance=50),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/reactive_power", 0, wait_secs=0, tolerance_type=Tolerance_Type.abs, tolerance=5),
        ],
        post_lambda= [
            lambda: set_all_ess_soc(50, ess_config=ESS_LABELS.CONFIG_DEV),
        ]
    ),
    # APS = 0
    # POI Limits = -15/15
    # ESS effective charge = 0
    # all solar should be exported to the grid
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/standalone_power/active_power_poi_limits_min_kW": -15000, # -15 MW
            "/features/standalone_power/active_power_poi_limits_max_kW": 15000, # 15 MW
        },
        [
            # Demand: active_power_setpoint_kW_cmd (0) + solar (5000)
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            # there is the biomass load idk how to turn off
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000, tolerance_type=Tolerance_Type.abs, tolerance=100), 
        ]
    ),
    # APS = 0
    # POI Limits = -15/15
    # ESS effective charge = 0 (should push a bit of power to cover load)
    # all solar should be exported to the grid
    Steps(
        {
            # provide some more load 0.55 MW
            "/components/bess_aux/active_power_setpoint": -500,  # load
            "/components/bess_aux/reactive_power_setpoint": -50,  # load
            # turn on offset behavior ess should cover load
            "/features/active_power/active_power_setpoint_load_method": 1
        },
        [
            # Demand: active_power_setpoint_kW_cmd (0) + solar (5000) + bess aux load + biomass load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5550, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=10),
            # this can swing (due to slews) a bit giving some tolerance
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 550, tolerance_type=Tolerance_Type.abs, tolerance=50), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            # ESS will cover the load of site should see very near to 5000
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000, tolerance_type=Tolerance_Type.abs, tolerance=25), 
        ]
    ),
    # # APS = -15 
    # # POI Limits = -15/15
    # # ESS effective charge = -11kW
    # # all solar should be exported to the grid
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -15000,
            # provide some more load 0.55 MW
            "/components/bess_aux/active_power_setpoint": 0,  # No load
            "/components/bess_aux/reactive_power_setpoint": 0,  # No load
            # turn on offset behavior ess should cover load
            "/features/active_power/active_power_setpoint_load_method": 0 # remove the offset behavior
        },
        [
            # Demand: You will want to charge as much as possible limited by rated ESS (-11000) + (5000) solar = 6000
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -6000, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=10),
            # this can swing (due to slews) a bit giving some tolerance
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -11000, tolerance_type=Tolerance_Type.abs, tolerance=50), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 6000, tolerance_type=Tolerance_Type.abs, tolerance=150), # biomass load 
        ]
    ),
    # APS = -15 
    # POI Limits = -15/15
    # ESS effective charge = -11kW
    # all solar should be exported to the grid
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -15000,
            # provide some more load 0.55 MW
            "/components/bess_aux/active_power_setpoint": -500,  # load
            "/components/bess_aux/reactive_power_setpoint": -50,  # load
            # turn on offset behavior ess should cover load
            "/features/active_power/active_power_setpoint_load_method": 1 # offset behavior
        },
        [
            # Demand: You will want to charge as much as possible limited by rated ESS (-11000) + (5000) solar = 6000
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -6000, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=10),
            # this can swing (due to slews) a bit giving some tolerance
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -11000, tolerance_type=Tolerance_Type.abs, tolerance=50), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            # biomass load + extra load because poi limits far enough away
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 6550, tolerance_type=Tolerance_Type.abs, tolerance=50), 
        ]
    ),
    # APS = -15 
    # POI Limits = -5/5
    # ESS effective charge = -10kW
    # all solar should be exported to the grid
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -15000,
            # provide some more load 0.55 MW
            "/components/bess_aux/active_power_setpoint": 0,  # No load
            "/components/bess_aux/reactive_power_setpoint": 0,  # No load
            # turn on offset behavior ess should cover load
            "/features/active_power/active_power_setpoint_load_method": 1, # offset behavior
            "/features/standalone_power/active_power_poi_limits_min_kW": 5000, 
            "/features/standalone_power/active_power_poi_limits_max_kW": 5000, 
            "/features/standalone_power/active_power_poi_limits_enable": True, 
        },
        [
            # Demand: You will want to charge as much as possible limited by rated ESS (-11000) + (5000) solar = 6000
            # further limited to importing 5000 (6000 reduced to 5000)
            # offset behavior of bimass
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4950, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=15),
            # this can swing (due to slews) a bit giving some tolerance
            # poi limits your possible intake to 5000
            # Solar (5000) + intake (5000) - biomass ~= 10000
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -9950, tolerance_type=Tolerance_Type.abs, tolerance=50), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            # biomass load + extra load because poi limits far enough away
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 5000, tolerance_type=Tolerance_Type.abs, tolerance=50), 
        ]
    ),
    # APS = -15 
    # POI Limits = -5/5
    # ESS effective charge = -10kW
    # all solar should be exported to the grid
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": -15000,
            # provide some more load 0.55 MW
            "/components/bess_aux/active_power_setpoint": -550,  # load
            "/components/bess_aux/reactive_power_setpoint": -50,  # load
            # turn on offset behavior ess should cover load
            "/features/active_power/active_power_setpoint_load_method": 1, # offset behavior
            "/features/standalone_power/active_power_poi_limits_min_kW": 5000, 
            "/features/standalone_power/active_power_poi_limits_max_kW": 5000, 
            "/features/standalone_power/active_power_poi_limits_enable": True, 
        },
        [
            # Demand: You will want to charge as much as possible limited by rated ESS (-11000) + (5000) solar = 6000
            # poi limits your possible intake to 5000
            # You end up with around .6 MW load so you offset by that
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -4400, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=10),
            # this can swing (due to slews) a bit giving some tolerance
            # Solar (5000) + intake (5000) - biomass ~= 10000
            # ESS is offset to cover .6MW laod 9400
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -9400, tolerance_type=Tolerance_Type.abs, tolerance=50), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            # biomass load + extra load because poi limits far enough away
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 5000, tolerance_type=Tolerance_Type.abs, tolerance=50), 
        ]
    ),
    # APS = 15 
    # POI Limits = -5/5
    # ESS effective discharge = 10kW
    # all solar should be exported to the grid
    Steps(
        {
            "/features/active_power/active_power_setpoint_kW_cmd": 15000,
            # provide some more load 0.55 MW
            "/components/bess_aux/active_power_setpoint": -550,  # load
            "/components/bess_aux/reactive_power_setpoint": -50,  # load
            # turn on offset behavior ess should cover load
            "/features/active_power/active_power_setpoint_load_method": 1, # offset behavior
            "/features/standalone_power/active_power_poi_limits_min_kW": 5000, 
            "/features/standalone_power/active_power_poi_limits_max_kW": 5000, 
            "/features/standalone_power/active_power_poi_limits_enable": True, 
        },
        [
            # Demand: You will want to charge as much as possible limited by rated ESS (-11000) + (5000) solar = 6000
            # poi limits your possible output to 5000
            # You end up with around .6 MW load so you offset by that
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5600, tolerance_type=Tolerance_Type.abs, tolerance=50, wait_secs=10),
            # ESS is offset to cover .6MW laod 600
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 600, tolerance_type=Tolerance_Type.abs, tolerance=50), 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
            # limited export backfeed whatever you wanna call it
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000, tolerance_type=Tolerance_Type.abs, tolerance=50), 
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/generators/gen_1/maint_mode": False,
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_maximize_solar_flag": False,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000, wait_secs=0)
        ]
    )
])
def test_aps_poi_limits_gauntlet(test):
    return test
