from unittest.result import failfast
from pytest_cases import parametrize, fixture
from pytests.fims import fims_set, fims_del
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown

# Reactive Power Run Mode 1 - Automatic Voltage Regulation Tests


# Add undervoltage variables to config so that asymmetric undervoltage variable configuarion can be tested
def add_undervoltage_configs():
    new_variables = {
        "avr_under_deadband_volts": {
            "name": "Undervoltage Deadband",
            "ui_type": "control",
            "unit": "V",
            "value": 100
        },
        "avr_under_droop_volts": {
            "name": "Undervoltage Droop",
            "ui_type": "control",
            "unit": "V",
            "value": 50
        },
        "avr_under_rated_kVAR": {
            "name": "Undervoltage Rated Power",
            "ui_type": "control",
            "unit": "VAR",
            "scaler": 1000,
            "value": 500
        }
    }
    fims_set("/dbi/site_controller/variables/variables/features/reactive_power/avr_under_deadband_volts", new_variables["avr_under_deadband_volts"])
    fims_set("/dbi/site_controller/variables/variables/features/reactive_power/avr_under_droop_volts", new_variables["avr_under_droop_volts"])
    fims_set("/dbi/site_controller/variables/variables/features/reactive_power/avr_under_rated_kVAR", new_variables["avr_under_rated_kVAR"])
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/variables/variables/features/reactive_power/avr_under_deadband_volts",
            "up": new_variables["avr_under_deadband_volts"]
        },
        {
            "uri": "/dbi/site_controller/variables/variables/features/reactive_power/avr_under_droop_volts",
            "up": new_variables["avr_under_droop_volts"]
        },
        {
            "uri": "/dbi/site_controller/variables/variables/features/reactive_power/avr_under_rated_kVAR",
            "up": new_variables["avr_under_rated_kVAR"]
        }
    ]
    return config_edits


# Remove undervoltage variables from config to revert back to default 
def remove_undervoltage_configs():
    fims_del("/dbi/site_controller/variables/variables/features/reactive_power/avr_under_deadband_volts")
    fims_del("/dbi/site_controller/variables/variables/features/reactive_power/avr_under_droop_volts")
    fims_del("/dbi/site_controller/variables/variables/features/reactive_power/avr_under_rated_kVAR")


# Test overvoltage with symmetric variables 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "avr_overvoltage_symmetric",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_cmd_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_deadband_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_droop_volts", 20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_rated_kVAR", 1000)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 0)
        ]
    ),
    Steps(
        {
            # Directly set shared POI voltage to act as actual_volts (3x desired actual_volts because it is split evenly by POIs)
            "/components/shared_poi/voltage_l1_l2": 1380
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 460),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_request_kVAR", -500),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -500)
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/voltage_l1_l2": 1200
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", False)
        ]
    )
])
def test_avr_overvoltage_symmetric(test):
    return test


# Test undervoltage with symmetric variables 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "avr_undervoltage_symmetric",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_cmd_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_deadband_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_droop_volts", 20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_rated_kVAR", 1000)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 0)
        ]
    ),
    Steps(
        {
            # Directly set shared POI voltage to act as actual_volts (3x desired actual_volts because it is split evenly by POIs)
            "/components/shared_poi/voltage_l1_l2": 1020
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 340),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_request_kVAR", 500),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 500)
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/voltage_l1_l2": 1200
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", False)
        ]
    )
])
def test_avr_undervoltage_symmetric(test):
    return test


# Test overvoltge with asymmetric variables 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "avr_overvoltage_asymmetric",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_cmd_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_deadband_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_droop_volts", 20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_rated_kVAR", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_deadband_volts", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_droop_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_rated_kVAR", 500)
        ],
        # Restart with asymmetric configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_undervoltage_configs()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 0)
        ]
    ),
    Steps(
        {
            # Directly set shared POI voltage to act as actual_volts (3x desired actual_volts because it is split evenly by POIs)
            "/components/shared_poi/voltage_l1_l2": 1380
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 460),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_request_kVAR", -500),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -500)
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/voltage_l1_l2": 1200
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", False)
        ],
        # Revert to default configs
        post_lambda=[
            lambda: remove_undervoltage_configs(),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    )
])
def test_avr_overvoltage_asymmetric(test):
    return test


# Test undervoltge with asymmetric variables
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "avr_undervoltage_asymmetric",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_cmd_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_deadband_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_droop_volts", 20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_rated_kVAR", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_deadband_volts", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_droop_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_rated_kVAR", 500)
        ],
        # Restart with asymmetric configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_undervoltage_configs()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 0)
        ]
    ),
    Steps(
        {
            # Directly set shared POI voltage to act as actual_volts (3x desired actual_volts because it is split evenly by POIs)
            "/components/shared_poi/voltage_l1_l2": 750
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 250),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_request_kVAR", 500),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 500)
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/voltage_l1_l2": 1200
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", False)
        ],
        # Revert to default configs
        post_lambda=[
            lambda: remove_undervoltage_configs(),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    )
])
def test_avr_undervoltage_asymmetric(test):
    return test


# Test when POI limits are enabled (positive limit)
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "avr_positive_poi_limits",
        {},
        [ 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_cmd_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_deadband_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_droop_volts", 20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_rated_kVAR", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_deadband_volts", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_droop_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_under_rated_kVAR", 500)
        ],
        # Restart with asymmetric configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_undervoltage_configs()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 0,
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -400,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 400
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_min_kVAR", -400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_max_kVAR", 400)
        ]
    ),
    Steps(
        {
            # Directly set shared POI voltage to act as actual_volts (3x desired actual_volts because it is split evenly by POIs)
            "/components/shared_poi/voltage_l1_l2": 750
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 250),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_request_kVAR", 500),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", 400)
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/voltage_l1_l2": 1200,
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -10000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 10000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_min_kVAR", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_max_kVAR", 10000)
        ],
        # Revert to default configs
        post_lambda=[
            lambda: remove_undervoltage_configs(),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    )
])
def test_avr_positive_poi_limits(test):
    return test


# Test when POI limits are enabled (negative limit) 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "avr_negative_poi_limits",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_cmd_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_deadband_volts", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_droop_volts", 20),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_over_rated_kVAR", 1000)
        ]
    ),
    Steps(
        {
            "/features/reactive_power/runmode1_kVAR_mode_cmd": 0,
            "/features/standalone_power/reactive_power_poi_limits_enable": True,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -400,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 400
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/runmode1_kVAR_mode_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_min_kVAR", -400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_max_kVAR", 400)
        ]
    ),
    Steps(
        {
            # Directly set shared POI voltage to act as actual_volts (3x desired actual_volts because it is split evenly by POIs)
            "/components/shared_poi/voltage_l1_l2": 1380
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 460),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_request_kVAR", -500),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/site_kVAR_demand", -400)
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/voltage_l1_l2": 1200,
            "/features/standalone_power/reactive_power_poi_limits_enable": False,
            "/features/standalone_power/reactive_power_poi_limits_min_kVAR": -10000,
            "/features/standalone_power/reactive_power_poi_limits_max_kVAR": 10000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_actual_volts", 400),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/reactive_power/avr_status_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_min_kVAR", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/reactive_power_poi_limits_max_kVAR", 10000)
        ]
    )
])
def test_avr_negative_poi_limits(test):
    return test
