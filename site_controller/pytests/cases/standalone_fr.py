from unittest.result import failfast
from pytest_cases import parametrize, fixture
from pytests.fims import fims_set, fims_del
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown
from pytests.controls import run_psm_command

# Test standalone FR interactions with other features. Adapted from original standalone PFR tests


# Add underfrequency variables to config so that asymmetric underfrequency variable configuarion parsing can be tested
uf_prefix = "/dbi/site_controller/variables/variables/features/standalone_power/components/0"
def add_underfrequency_configs():
    new_variables = {
        "trigger_freq_hz": {
            "name": "Underfrequency Deadband",
            "ui_type": "control",
            "unit": "Hz",
            "value": 59.0
        },
        "droop_freq_hz": {
            "name": "Full Response Frequency",
            "ui_type": "none",
            "unit": "Hz",
            "value": 57.0,
        },
        "active_cmd_kw": {
            "name": "Active Cmd kW",
            "ui_type": "none",
            "unit": "kW",
            "value": -1000,
            "scaler": 1
        }
    }
    fims_set("/features/standalone_power/uf_pfr_trigger_freq_hz", new_variables["trigger_freq_hz"])
    fims_set("/features/standalone_power/uf_pfr_droop_freq_hz", new_variables["droop_freq_hz"])
    fims_set("/features/standalone_power/uf_pfr_active_cmd_kw", new_variables["active_cmd_kw"])
    config_edits: list[dict] = [
        {
            "uri": f"{uf_prefix}/trigger_freq_hz",
            "up": new_variables["trigger_freq_hz"]
        },
        {
            "uri": f"{uf_prefix}/droop_freq_hz",
            "up": new_variables["droop_freq_hz"]
        },
        {
            "uri": f"{uf_prefix}/active_cmd_kw",
            "up": new_variables["active_cmd_kw"]
        }
    ]
    return config_edits


# Remove underfrequency variables from config to revert back to default 
def remove_underfrequency_configs():
    fims_del(f"{uf_prefix}/trigger_freq_hz")
    fims_del(f"{uf_prefix}/droop_freq_hz")
    fims_del(f"{uf_prefix}/active_cmd_kw")


# PFR + Active Power Setpoint mode (untracked load)
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "pfr_untracked_load",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_active_cmd_kw", 1000),
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -4000) 
        ]
    ),
    Steps(
        {
            # frequency -> 56.5 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 56.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 6000, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_output_kw", 1000),
            
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 0,
            # frequency -> 60 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 60),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_active_cmd_kw", 0),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ]
    )
])
def test_pfr_untracked_load(test):
    return test


# PFR + Active Power Setpoint mode (offset load)
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_offset_load",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 1,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000) 
        ]
    ),
    Steps(
        {
            # underfrequency event 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 7000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -6000),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0)
        ],
                pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ]
    )
])
def test_pfr_offset_load(test):
    return test


# PFR + Active Power Setpoint mode (minimum load)
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_minimum_load",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 2,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True)
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),     
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000) 
        ]
    ),
    Steps(
        {
            # underfrequency event 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 7000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -6000),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0
            # frequency -> 60 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 60),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ]
    )
])
def test_pfr_minimum_load(test):
    return test


# PFR + Active Power Setpoint mode (untracked load) + POI limit 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_untracked_load_poi_lim",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_max_kW": 4000,
            "/features/standalone_power/active_power_poi_limits_min_kW": -4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -4000)
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -3000) 
        ]
    ),
    Steps(
        {
            # underfrequency event 
            # frequency -> 56.5 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 56.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -3000),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 60),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000)
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ]
    )
])
def test_pfr_untracked_load_poi_lim(test):
    return test


# PFR + Active Power Setpoint mode (offset load) + POI limit 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_offset_load_poi_lim",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 1,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_max_kW": 4000,
            "/features/standalone_power/active_power_poi_limits_min_kW": -4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -4000)
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -4000) 
        ]
    ),
    Steps(
        {
            # underfrequency event 
            # frequency -> 56.5 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 56.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -4000),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 60),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000)
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ]
    )
])
def test_pfr_offset_load_poi_lim(test):
    return test


# PFR + Active Power Setpoint mode (minimum load) + POI limit 
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_minimum_load_poi_lim",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 2,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/standalone_power/active_power_poi_limits_max_kW": 4000,
            "/features/standalone_power/active_power_poi_limits_min_kW": -4000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -4000)
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -4000) 
        ]
    ),
    Steps(
        {
            # underfrequency event 
            # frequency -> 56.5 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 56.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -4000),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 60),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -10000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_max_kW", 10000)
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ]
    )
])
def test_pfr_minimum_load_poi_lim(test):
    return test


# Test that asymmetric endpoints parse and are available correctly (Active Power Setpoint mode + untracked load)
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_asymmetric_configs",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_droop_freq_hz", 57.0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_trigger_freq_hz", 59.0),
        ],
        # Restart with asymmetric configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True)
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 5000,
            "/components/bess_aux/active_power_setpoint": -1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -4000) 
        ]
    ),
    Steps(
        {
            # underfrequency event 
            # frequency -> 56.5 in pre_lambda 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 56.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 6000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -5000),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 56.5 -r /$$'),
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 60),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0)
        ],
        pre_lambda=[
            lambda: remove_underfrequency_configs(),
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$'),
        ],
    )
])

def test_pfr_asymmetric_configs(test):
    return test
