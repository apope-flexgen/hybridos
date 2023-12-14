

# migrations to change certain standalone feature enable flags to true or false
from pytest_cases import parametrize, fixture
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.fims import fims_set
from pytests.pytest_framework import Site_Controller_Instance

from pytests.pytest_steps import Setup, Steps, Teardown

standalone_enable_keys = [
    "active_power_poi_limits_enable",
    "pfr_enable_flag", # To be replaced with fr_mode_enable_flag
    "watt_watt_adjustment_enable_flag",
    "ldss_enable_flag",
    "load_shed_enable",
    "solar_shed_enable",
    "ess_discharge_prevention_enable",
    "agg_asset_limit_enable",
    "active_power_closed_loop_zero_bypass_enable",
    "reactive_power_closed_loop_enable",
    "reactive_power_poi_limits_enable"
]

def up_config():
    '''
    standalone enabling and availability:
    [index] [enabled]   [available]
    0       true        true
    1       false       true
    2       false       false
    ...     true        true
    
    - true/false combo is not tested because it errors out on SC startup with message:
      "Enable flag \"{}\" was set as true but the feature is not available"
      TODO add a means to test error states such as this ^^
    '''

    config_edits: list[dict] = []
    for index, key in enumerate(standalone_enable_keys):
        value = (index not in [1,2])
        config_edits.append({
            "uri": f"/dbi/site_controller/variables/variables/features/standalone_power/{key}/value",
            "up": value
        })
    
    config_edits.append({
        "uri": "/dbi/site_controller/variables/variables/internal/available_features_standalone_power/value",
        "up":  "7fd" # 11111111101
    })

    # enable watchdog
    config_edits.append({
        "uri": "/dbi/site_controller/variables/variables/features/site_operation/watchdog_enable/value",
        "up":  "true"
    })

    return config_edits

def down_config():
    '''
    Undo dbi setpoints so they will be reloaded on SC restart. Ideally these would be reverted to exactly what they were at
    function execution, but in this case these won't have impact on other tests and should closely match what the state
    was entering the function.
    '''
    for key in standalone_enable_keys:
        fims_set(f"/dbi/site_controller/variables/variables/features/standalone_power/{key}/value", False)
    fims_set("/dbi/site_controller/variables/variables/internal/available_features_standalone_power/value", "0xFFF")
    fims_set("/dbi/site_controller/variables/variables/features/site_operation/watchdog_enable/value", False)

# Test if standalone features and watchdog_enable are correctly enabled on startup based on enable_flag.value and available
@ fixture
@ parametrize("test", [
    Setup(
        "enable_flags",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(up_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    # check watchdog_enable.value
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/site_operation/watchdog_enable", True),
        ]
    ),
    # check enabled standalones [0, 3, 4, 5, 6, 7, 8, 9, 10]
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/ldss_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/load_shed_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/solar_shed_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/ess_discharge_prevention_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/agg_asset_limit_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_closed_loop_zero_bypass_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_closed_loop_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", True),
        ]
    ),
    # check disabled standalones [1, 2]
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/pfr_enable_flag", False), # To be replaced with fr_mode_enable_flag
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/watt_watt_adjustment_enable_flag", False),
        ]
    ),
    Teardown(
        {},
        [],
        pre_lambda=[
            lambda: down_config(),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
])
def test_enable_flags(test):
    return test
