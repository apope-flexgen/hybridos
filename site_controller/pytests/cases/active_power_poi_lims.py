import pytest

from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# Frequency Response with Poi limits
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "fr_poi_lims",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 4,
            "/features/active_power/asset_priority_runmode1": 0,
            "/features/active_power/fr_enable_mask": 24,
            "/features/active_power/fr_baseload_cmd_kw": 0,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_min_kW": -11000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 11000,
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 5,
            "/components/bess_aux/active_power_setpoint": 0  # No load
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 4),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/fr_enable_mask", 24),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/fr_baseload_cmd_kw", 0),
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
            "/features/active_power/fr_baseload_cmd_kw": -5000
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
            "/features/active_power/fr_baseload_cmd_kw": 5000
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
            "/features/active_power/fr_baseload_cmd_kw": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/fr_baseload_cmd_kw", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000)
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

# The commented test case below (active_power_setpoint_ess_solar_poi_lims) currently violates POI limits.
# Here is a description of the details on why exactly the limits get violated.
#
# Test case context: Active Power Setpoint is the active power feature. We have a setpoint of -10 MW 
# with a slew rate of 1 GW/s, and we have enabled the maximize_solar_flag. The site has 2 ESS and 2 solar 
# assets. The solar assets are able to give a total of 5 MW. POI limits is enabled with a backfeed limit 
# of 3.5 MW. We start the site and then put both ESS into maintenance mode. What we see is that the POI 
# limit is violated because the solar assets discharge at the full 5 MW.
#
# Logic leading to behavior:
# Let the current asset output (in this case just solar cmd) be X. Since the ess aren't available, we know X 
# is between 0 and 5 MW. (0 <= X <= 5)
# In active power setpoint mode's execution, we try to slew down to -10 MW from whatever our current asset 
# output is. Because site_controller updates about every 10 ms, we can slew by at most 1 GW/s * 10 ms = 10 MW. So, 
# the slew leads to a site demand of about X-10 MW (value will fluctuate depending on exact time between updates). We 
# also set the solar request to its max of 5 MW since maximize_solar_flag is enabled.
# asset_cmd.calculate_feature_kW_demand is called, which adds the solar request to the site demand. The site demand 
# is now X-5 MW.
# The site demand is less than 3.5 MW because X-5 <= 0. Since the site demand is less than 3.5 MW, the POI limits 
# execution sees that it is within the acceptable range and does not modify it.
# The site discharge production limits are calculated at the beginning of 
# Site_Manager::dispatch_active_power(int asset_priority). Because there is no standalone feature modification 
# to the site demand, the discharge production limit is calculated to be the full solar request.
# Power is dispatched accordingly.
# So, in this case the site demand isn't accurately representing the power at the POI.
#
# # Active Power Setpoint ESS + Solar with POI limits
# @ fixture
# @ parametrize("test", [
#     # Preconditions
#     Setup(
#         "active_power_setpoint_ess_solar_poi_lims",
#         {
#             "/assets/generators/gen_1/maint_mode": True,
#             "/features/active_power/runmode1_kW_mode_cmd": 2,
#             "/features/active_power/active_power_setpoint_kW_cmd": 0,
#             "/features/active_power/active_power_setpoint_maximize_solar_flag": True,
#             "/features/active_power/asset_priority_runmode1": 0,
#             "/features/standalone_power/active_power_poi_limits_enable": False,
#             "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
#             "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
#             "/components/bess_aux/active_power_setpoint": 0  # No load
#         },
#         [
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000),
#             Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 0),
#         ]
#     ),
#     # Some ESS available
#     Steps(
#         {
#             "/features/active_power/active_power_setpoint_kW_cmd": -10000,
#             "/assets/ess/ess_1/maint_mode": True,
#         },
#         [
#             # Demand unlimited: -10000 ESS + 5000 uncurtailed solar
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000),
#             # Only 5500 ESS available (SoC balancing gives 5500 instead of 5000)
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -5500),
#             # Solar gets its full 5000 as well
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 550, tolerance=0.1),
#         ]
#     ),
#     # No ESS available
#     Steps(
#         {
#             "/features/active_power/active_power_setpoint_kW_cmd": -10000,
#             "/assets/ess/ess_2/maint_mode": True,
#         },
#         [
#             # Demand unlimited: -10000 ESS + 5000 uncurtailed solar
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", -5000),
#             # No ESS avail
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0),
#             # Solar still gets its full 5000
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000),
#         ]
#     ),
#     # Make sure POI limit is accurate
#     Steps(
#         {
#             "/features/standalone_power/active_power_poi_limits_enable": True,
#             "/features/standalone_power/active_power_poi_limits_max_kW": 3500,
#         },
#         [
#             # Demand aggregate would be -5000 if we don't consider available assets meaning it would hit the min limit
#             # But because of available assets we should get an aggregate of 5000 which should hit the max limit
#             # Make sure the correct limit is applied and that solar gets as much power as it can
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 3500),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -3500),
#         ]
#     ),
#     # Cleanup
#     Teardown(
#         {
#             "/assets/generators/gen_1/maint_mode": False,
#             "/features/active_power/runmode1_kW_mode_cmd": 2,
#             "/features/active_power/active_power_setpoint_kW_cmd": 0,
#             "/features/active_power/active_power_setpoint_maximize_solar_flag": False,
#             "/features/active_power/asset_priority_runmode1": 0,
#             "/features/standalone_power/active_power_poi_limits_enable": False,
#             "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
#             "/features/standalone_power/active_power_poi_limits_max_kW": 10000
#         },
#         [
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/asset_priority_runmode1", 0),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
#             Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000)
#         ]
#     )
# ])
# def test_active_power_setpoint_ess_solar_poi_lims(test):
#     return test
