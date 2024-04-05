from typing import Union
from pytest_cases import parametrize, fixture
from pytests.fims import fims_set, fims_del
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown
from pytests.controls import run_psm_command

# Test standalone FR interactions with other features. Adapted from original standalone PFR tests
def add_underfrequency_configs(trigger_freq_hz: Union[float, None] = None, droop_freq_hz: Union[float, None] = None, 
                               active_cmd_kw: Union[float, None] = None, instant_recovery_freq_hz: Union[float, None] = None, 
                               trigger_duration_sec: Union[float, None] = None, recovery_freq_hz: Union[float, None] = None, 
                               recovery_duration_sec: Union[float, None] = None, cooldown_duration_sec: Union[float, None] = None,
                               slew_rate_kw: Union[float, None] = None, cooldown_slew_rate_kw: Union[float, None] = None):
    """Use this function to setup configuration for each test"""
    # Add underfrequency variables to config so that asymmetric underfrequency variable configuarion parsing can be tested
    uf_prefix = "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0"
    new_variables = {}
    config_edits: list[dict] = []
    if trigger_freq_hz is not None:
        new_variables["trigger_freq_hz"] = {
                "name": "Underfrequency trigger",
                "ui_type": "status",
                "unit": "Hz",
                "value": trigger_freq_hz
                }
        config_edits.append({
            "uri": f"{uf_prefix}/trigger_freq_hz",
            "up": new_variables["trigger_freq_hz"]
        })
    if droop_freq_hz is not None:
        new_variables["droop_freq_hz"] = {
                "name": "Full Response Frequency",
                "ui_type": "status",
                "unit": "Hz",
                "value": droop_freq_hz
                }
        fims_set("/features/standalone_power/uf_pfr_droop_freq_hz", new_variables["droop_freq_hz"])
        config_edits.append({
            "uri": f"{uf_prefix}/droop_freq_hz",
            "up": new_variables["droop_freq_hz"]
        })
    if active_cmd_kw is not None:
        new_variables["active_cmd_kw"] = {
                "name": "Active Response Command",
                "ui_type": "status",
                "unit": "kW",
                "value": active_cmd_kw,
                "scaler": 1
                }
        fims_set("/features/standalone_power/uf_pfr_active_cmd_kw", new_variables["active_cmd_kw"])
        config_edits.append({
            "uri": f"{uf_prefix}/active_cmd_kw",
            "up": new_variables["active_cmd_kw"]
        })
    if instant_recovery_freq_hz is not None:
        new_variables["instant_recovery_freq_hz"] = {
                "name": "Instant Recovery Frequency",
                "ui_type": "status",
                "unit": "Hz",
                "value": instant_recovery_freq_hz
                }
        config_edits.append({
            "uri": f"{uf_prefix}/instant_recovery_freq_hz",
            "up": new_variables["instant_recovery_freq_hz"]
        })
    if trigger_duration_sec is not None:
        new_variables["trigger_duration_sec"] = {
                "name": "Response Duration",
                "ui_type": "status",
                "unit": "s",
                "value": trigger_duration_sec,
                "var_type": "Int"
                }
        fims_set("/features/standalone_power/trigger_duration_sec", new_variables["trigger_duration_sec"])
        config_edits.append({
            "uri": f"{uf_prefix}/trigger_duration_sec",
            "up": new_variables["trigger_duration_sec"]
        })
    if recovery_freq_hz is not None:
        new_variables["recovery_freq_hz"] = {
                "value": recovery_freq_hz,
                "name": "UF PFR Begin Recovery Frequency",
                "unit": "Hz",
                "scaler": 1,
                "enabled": False,
                "type": "number",
                "ui_type": "status",
                "options": []
                }
        config_edits.append({
            "uri": f"{uf_prefix}/recovery_freq_hz",
            "up": new_variables["recovery_freq_hz"]
        })
    if recovery_duration_sec is not None:
        new_variables["recovery_duration_sec"] = {
                "value": recovery_duration_sec,
                "name": "UF PFR Recovery Duration",
                "unit": "s",
                "scaler": 1,
                "enabled": False,
                "type": "number",
                "ui_type": "none",
                "options": []
                }
        config_edits.append({
            "uri": f"{uf_prefix}/recovery_duration_sec",
            "up": new_variables["recovery_duration_sec"]
        })
    if cooldown_duration_sec is not None:
        new_variables["cooldown_duration_sec"] = {
                "value": cooldown_duration_sec,
                "name": "UF PFR Cooldown Duration",
                "unit": "s",
                "scaler": 1,
                "enabled": False,
                "type": "number",
                "ui_type": "none",
                "options": []
                }
        config_edits.append({
            "uri": f"{uf_prefix}/cooldown_duration_sec",
            "up": new_variables["cooldown_duration_sec"]
        })
    if slew_rate_kw is not None:
        new_variables["slew_rate_kw"] = {
                "name": "Slew Rate",
                "scaler": 1,
                "ui_type": "control",
                "value": slew_rate_kw,
                "var_type": "Int"
                }
        config_edits.append({
            "uri": f"{uf_prefix}/slew_rate_kw",
            "up": new_variables["slew_rate_kw"]
        })
    if cooldown_slew_rate_kw is not None:
        new_variables["cooldown_slew_rate_kw"] = {
                "name": "Cooldown Slew Rate",
                "scaler": 1,
                "ui_type": "control",
                "value": cooldown_slew_rate_kw,
                "var_type": "Int"
                }
        config_edits.append({
            "uri": f"{uf_prefix}/cooldown_slew_rate_kw",
            "up": new_variables["cooldown_slew_rate_kw"]
        })
    return config_edits

# Remove underfrequency variables from config to revert back to default 
def remove_underfrequency_configs():
    uf_prefix = "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0"
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
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs(trigger_freq_hz= 59,droop_freq_hz=57, active_cmd_kw=-1000, slew_rate_kw=1000, cooldown_slew_rate_kw=1000)),
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

# Force start PFR
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_force_start",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs(trigger_freq_hz=59, droop_freq_hz=57, active_cmd_kw=3000, instant_recovery_freq_hz=60, trigger_duration_sec=10, recovery_freq_hz=59.5, recovery_duration_sec=5, cooldown_duration_sec=10)),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
            "/features/standalone_power/uf_pfr_slew_rate_kw": 1000,
            "/components/ess_psm/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 50,
            # place solar and gen  assets in maint to simplify this test
            **Steps.config_dev_place_solar_in_maint(),
            **Steps.config_dev_place_gen_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
        ]
    ),
    Steps(
        {
            # 1MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 1000,
            "/components/bess_aux/active_power_setpoint": -1000, # active_power_setpoint will cover
            "/features/standalone_power/uf_pfr_droop_bypass_flag": True, # bypass droop to make test simple
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5) 
        ]
    ),
    Steps(
        {
            # underfrequency event but not enough to hit trigger @59Hz 
            # going to use force_start to start the response
            # frequency -> 59.5 in pre_lambda 
            "/features/standalone_power/uf_pfr_force_start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 59.5 -r /$$'),
        ]
    ),
    Steps(
        {
            # recovers in 5 seconds, BUT when the bool is still flipped true it should restart over and over
            # make sure it's still running
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=16), # ten second cooldown_duration_sec
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ]
    ),
    Steps(
        {
            # stop it
            "/features/standalone_power/uf_pfr_force_start": False,
            # let it finish
            # make sure we aren't pushing extra power anymore
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_active_response_status", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_active_response_status", False, wait_secs=6),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_in_cooldown", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5, wait_secs=5) 
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/standalone_power/uf_pfr_droop_bypass_flag": False, # reinstate droop
            **Steps.config_dev_remove_solar_from_maint(),
            **Steps.config_dev_remove_gen_from_maint(),
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
def test_pfr_force_start(test):
    return test

# Force start PFR
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "pfr_force_start_pulse",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs(
                trigger_freq_hz=1, # we will trigger it move out of the way
                droop_freq_hz=0, # must be less than trigger
                active_cmd_kw=1000, # simple small
                instant_recovery_freq_hz=60, # instant_recover at 60, but won't be able to
                trigger_duration_sec=15, # FR will last for 15 seconds
                cooldown_duration_sec=5, # 5 second cooldown
                recovery_freq_hz=59, # seems reasonable, but won't be able to recover
                recovery_duration_sec=5, # 5 second recovery to give time for tests
                slew_rate_kw=1000, 
                cooldown_slew_rate_kw=1000
                )),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/features/standalone_power/uf_pfr_active_cmd_kw": 1000,
            "/components/ess_psm/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 50,
            # place solar and gen  assets in maint to simplify this test
            **Steps.config_dev_place_solar_in_maint(),
            **Steps.config_dev_place_gen_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
        ]
    ),
    Steps(
        {
            # 1MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 1000,
            "/components/bess_aux/active_power_setpoint": -1000, # active_power_setpoint will cover
            "/features/standalone_power/uf_pfr_droop_bypass_flag": True, # bypass droop to make test simple
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=7), # load being annoying swinging use 7 seconds
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5) 
        ]
    ),
    Steps(
        {
            # underfrequency event but not enough to hit trigger @59Hz 
            # going to use force_start to start the response
            # frequency -> 59.5 in pre_lambda 
            "/features/standalone_power/uf_pfr_force_start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=1),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 59.5 -r /$$'),
        ]
    ),
    Steps(
        {
            "/features/standalone_power/uf_pfr_force_start": False, # pulse the flag. When set false then the component will behave as if it was a regular FR event
            # NOTE: you have bled 1 second of recovery time already from prior step since we are above the recovery_freq_hz
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_in_recovery", True, wait_secs=0), # will already be ticking the recovery timer 
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_in_cooldown", True, wait_secs=3), # timer finishes at 5 seconds so at 6 seconds we should be in_cooldown
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5, wait_secs=10) # make sure we aren't pushing extra power anymore
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/standalone_power/uf_pfr_droop_bypass_flag": False, # reinstate droop
            **Steps.config_dev_remove_solar_from_maint(),
            **Steps.config_dev_remove_gen_from_maint(),
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
def test_pfr_force_start_pulse(test):
    return test


# Force start PFR
@ fixture
@ parametrize("test", [
    # Preconditions 
    # demonstrate that the trigger_duration_sec will be obeyed
    # event will start again as soon as cooldown is over
    # follow this link and look at first graph mentioning force_start https://flexgen.atlassian.net/wiki/spaces/API/pages/10223652/SRM1+-+Frequency+Response#Over-Frequency-Response-Graph
    Setup(
        "pfr_force_start_graph_1",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs(
                trigger_freq_hz=1, # we will trigger it move out of the way
                droop_freq_hz=0, # must be less than trigger
                active_cmd_kw=1000, # simple small
                instant_recovery_freq_hz=60, # instant_recover at 60, but won't be able to
                trigger_duration_sec=15, # FR will last for 15 seconds
                cooldown_duration_sec=5, # 5 second cooldown
                recovery_freq_hz=59, # seems reasonable, but won't be able to recover
                slew_rate_kw=1000, 
                cooldown_slew_rate_kw=1000
                )),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/components/ess_psm/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 50,
            # place solar and gen  assets in maint to simplify this test
            **Steps.config_dev_place_solar_in_maint(),
            **Steps.config_dev_place_gen_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
        ]
    ),
    Steps(
        {
            # 1MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 1000,
            "/components/bess_aux/active_power_setpoint": -1000, # active_power_setpoint will cover
            "/features/standalone_power/uf_pfr_droop_bypass_flag": True, # bypass droop to make test simple
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5) 
        ]
    ),
    Steps(
        {
            # going to use force_start to start the response
            # frequency -> 59.5 in pre_lambda 
            # feat should want to recover, but won't be allowed because force_start true
            "/features/standalone_power/uf_pfr_force_start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 59.5 -r /$$'),
        ]
    ),
    Steps(
        {
            # wait for 5 seconds and check it's still running
            # at second 8 of 15
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ]
    ),
    Steps(
        {
            # wait for 5 seconds and check it's still running
            # at second 13 of 15
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ]
    ),
    Steps(
        {
            # wait for 3 seconds and check it's in cooldown
            # at second 16 of 15 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_in_cooldown", True, tolerance=0.1, wait_secs=3),
        ]
    ),
    Steps(
        {
            # wait for 5 seconds and check it's starting again
            # cooldown last 5 seconds
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_active_response_status", True, tolerance=0.1, wait_secs=5),
        ]
    ),
    Steps(
        {
            # stop it
            "/features/standalone_power/uf_pfr_force_start": False,
            # should instant recover
            # make sure we aren't pushing extra power anymore
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5, wait_secs=15) 
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/standalone_power/uf_pfr_droop_bypass_flag": False, # reinstate droop
            **Steps.config_dev_remove_solar_from_maint(),
            **Steps.config_dev_remove_gen_from_maint(),
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
def test_pfr_force_start_graph_1(test):
    return test

# Force start PFR
@ fixture
@ parametrize("test", [
    # Preconditions 
    # demonstrate that the trigger_duration_sec will be obeyed
    # event will start again as soon as cooldown is over
    # follow this link and look at second graph mentioning force_start https://flexgen.atlassian.net/wiki/spaces/API/pages/10223652/SRM1+-+Frequency+Response#Over-Frequency-Response-Graph
    Setup(
        "pfr_force_start_graph_2",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs(
                trigger_freq_hz=1, # we will trigger it move out of the way
                droop_freq_hz=0, # must be less than trigger
                active_cmd_kw=1000, # simple small
                instant_recovery_freq_hz=60, # instant_recover at 60, but won't be able to
                trigger_duration_sec=15, # FR will last for 15 seconds
                cooldown_duration_sec=5, # 5 second cooldown
                recovery_freq_hz=59, # seems reasonable, but won't be able to recover
                recovery_duration_sec=1, # quick recovery to prove we bypass it
                slew_rate_kw=1000, 
                cooldown_slew_rate_kw=1000
                )),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/components/ess_psm/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 50,
            # place solar and gen  assets in maint to simplify this test
            **Steps.config_dev_place_solar_in_maint(),
            **Steps.config_dev_place_gen_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 1000,
            "/components/bess_aux/active_power_setpoint": -1000, # active_power_setpoint will cover
            "/features/standalone_power/uf_pfr_droop_bypass_flag": True, # bypass droop to make test simple
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5) 
        ]
    ),
    Steps(
        {
            # going to use force_start to start the response
            # frequency -> 59.5 in pre_lambda 
            # feat should want to recover, but won't be allowed because force_start true
            "/features/standalone_power/uf_pfr_force_start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 59.5 -r /$$'),
        ]
    ),
    Steps(
        {
            # wait for 5 seconds and check it's still running
            # at second 8 of 15
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ]
    ),
    Steps(
        {
            # stop it
            "/features/standalone_power/uf_pfr_force_start": False,
            # should instantly recover. NOT BECAUSE WE ARE AT instant_recovery_freq_hz, but because it has been longer than recovery_duration_sec
            # above recovery_freq_hz, BUT force_start "ors together" and overrides it.
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_in_cooldown", True, tolerance=0.1, wait_secs=3),
        ]
    ),
    Steps(
        {
            # make sure we aren't pushing extra power anymore
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5, wait_secs=5)  # 6 + 5 + ramp down
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/standalone_power/uf_pfr_droop_bypass_flag": False, # reinstate droop
            **Steps.config_dev_remove_solar_from_maint(),
            **Steps.config_dev_remove_gen_from_maint(),
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
def test_pfr_force_start_graph_2(test):
    return test

# Force start PFR
@ fixture
@ parametrize("test", [
    # Preconditions 
    # demonstrate that the trigger_duration_sec will be obeyed
    # event will start again as soon as cooldown is over
    Setup(
        "pfr_asymmetric_slew",
        {},
        [],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(add_underfrequency_configs(
                trigger_freq_hz=1, # we will trigger it move out of the way
                droop_freq_hz=0, # must be less than trigger
                active_cmd_kw=1000, # simple small
                instant_recovery_freq_hz=60, # instant_recover at 60, but won't be able to
                trigger_duration_sec=15, # FR will last for 15 seconds
                cooldown_duration_sec=60, # LONG cooldown to give time to play with slow ramp down
                recovery_freq_hz=59, # seems reasonable, but won't be able to recover
                recovery_duration_sec=1, # quick recovery to prove we bypass it
                slew_rate_kw=1000, 
                cooldown_slew_rate_kw=50 # VERY SLOW cooldown ramp 
                )),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/fr_mode_enable_flag": True,
            "/components/ess_psm/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 50,
            # place solar and gen  assets in maint to simplify this test
            **Steps.config_dev_place_solar_in_maint(),
            **Steps.config_dev_place_gen_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/fr_mode_enable_flag", True),
        ]
    ),
    Steps(
        {
            # 5MW active power command, 1MW load 
            "/features/active_power/active_power_setpoint_kW_cmd": 1000,
            "/components/bess_aux/active_power_setpoint": -1000, # active_power_setpoint will cover
            "/features/standalone_power/uf_pfr_droop_bypass_flag": True, # bypass droop to make test simple
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_load", 1050, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 50, tolerance=0.5) 
        ]
    ),
    Steps(
        {
            # going to use force_start to start the response
            # frequency -> 59.5 in pre_lambda 
            # feat should want to recover, but won't be allowed because force_start true
            "/features/standalone_power/uf_pfr_force_start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_frequency", 59.5, wait_secs=3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 2000, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -1000, tolerance=0.1, wait_secs=0),
        ],
        pre_lambda=[
            lambda: run_psm_command('fims_send -m set -u /components/grid/fcmd 59.5 -r /$$'),
        ]
    ),
    Steps(
        {
            # stop it and watch it ramp down slow
            "/features/standalone_power/uf_pfr_force_start": False,
            # should instantly recover. NOT BECAUSE WE ARE AT instant_recovery_freq_hz, but because it has been longer than recovery_duration_sec
            # above recovery_freq_hz, BUT force_start "ors together" and overrides it.
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/uf_pfr_in_cooldown", True, tolerance=0.1, wait_secs=3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 1850, wait_secs=0), # cooldown_slew_rate_kw * 3
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -850, tolerance=0.1, wait_secs=0), # cooldown_slew_rate_kw * 3
        ]
    ),
    Steps(
        {
            # wait another 5 seconds recheck
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/site_kW_demand", 1600, wait_secs=5), # 1850 - cooldown_slew_rate_kw * 5
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -600, tolerance=0.5, wait_secs=0),  # 850 - cooldown_slew_rate_kw * 5 
        ]
    ),
    Teardown(
        {
            # frequency -> 60 in pre_lambda 
            "/features/standalone_power/fr_mode_enable_flag": False,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/features/standalone_power/active_power_poi_limits_enable": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -10000,
            "/features/standalone_power/active_power_poi_limits_max_kW": 10000,
            "/features/standalone_power/uf_pfr_droop_bypass_flag": False, # reinstate droop
            **Steps.config_dev_remove_solar_from_maint(),
            **Steps.config_dev_remove_gen_from_maint(),
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
def test_pfr_asymmetric_slew(test):
    return test
