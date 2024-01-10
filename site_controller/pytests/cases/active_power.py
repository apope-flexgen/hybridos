import logging
import subprocess
from pytests.fims import no_fims_msgs
from unittest.result import failfast
from pytest_cases import parametrize, fixture
from pytests.assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from pytests.pytest_steps import Setup, Steps, Teardown


# Generic feature (active power setpoint) testing ess chargeable power derating
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "ess_chargeable_derate",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/components/bess_aux/active_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", 0),
        ]
    ),
    Steps(
        {
            # Average SoC, no derating
            "/features/active_power/active_power_setpoint_kW_cmd": -11000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", -5500),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_hs/active_power_setpoint", -5500)
        ]
    ),
    Steps(
        {
            # High soc, start derating
            "/components/ess_twins/bms_soc": 94,
            "/components/ess_real_ls/bms_soc": 95
        },
        [
            # Make sure the demand and setpoints are both derated and still negative (have not flipped sign due to previous bug)
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", -3500, tolerance=0.5, duration_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_hs/active_power_setpoint", -2500, tolerance=0.5, duration_secs=5),
        ]
    ),
    Steps(
        {
            # 100% soc within site controller but not 100% raw soc
            # Derated to minimum site controller value, as some component chargeable power is still available
            "/components/ess_twins/bms_soc": 97,
            "/components/ess_real_ls/bms_soc": 98
        },
        [
            # Make sure the demand and setpoints are both derated and still negative (have not flipped sign due to previous bug)
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", -100, tolerance=0.5, duration_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_hs/active_power_setpoint", -100, tolerance=0.5, duration_secs=5)
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/components/ess_twins/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 60
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_ls/bms_soc", 60),
        ]
    )
])
def test_ess_chargeable_derate(test):
    return test


# Generic feature (active power setpoint) testing ess dischargeable power derating
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "ess_dischargeable_derate",
        {
            **Steps.disable_solar_and_gen(),
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/features/active_power/active_power_setpoint_load_method": 0,
            "/components/bess_aux/active_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_load_method", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power_setpoint", 0),
        ]
    ),
    Steps(
        {
            # Average SoC, no derating
            "/features/active_power/active_power_setpoint_kW_cmd": 11000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", 5500),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_hs/active_power_setpoint", 5500)
        ]
    ),
    Steps(
        {
            # Low soc, start derating
            # soc is a little bit higher than the equivalent for the chargeable tests due to asymmetric battery capacity (4-97%)
            "/components/ess_twins/bms_soc": 7,
            "/components/ess_real_ls/bms_soc": 6
        },
        [
            # Make sure the demand and setpoints are both derated and still negative (have not flipped sign due to previous bug)
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", 3500, tolerance=0.5, duration_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_hs/active_power_setpoint", 2500, tolerance=0.5, duration_secs=5),
        ]
    ),
    Steps(
        {
            # 0% soc within site controller but not 0% raw soc
            # Derated to minimum site controller value, as some component dischargeable power is still available
            "/components/ess_twins/bms_soc": 3,
            "/components/ess_real_ls/bms_soc": 2
        },
        [
            # Make sure the demand and setpoints are both derated and still positive (have not flipped sign due to previous bug)
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", 100, tolerance=0.5, duration_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_hs/active_power_setpoint", 100, tolerance=0.5, duration_secs=5)
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
            "/features/active_power/active_power_setpoint_kW_cmd": 0,
            "/components/ess_twins/bms_soc": 50,
            "/components/ess_real_ls/bms_soc": 60
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_real_ls/bms_soc", 60),
        ]
    )
])
def test_ess_dischargeable_derate(test):
    return test


# Maint mode testing active power setpoint rounding
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "maint_active_power_rounding",
        {
            "/assets/solar/solar_2/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 1),
        ]
    ),
    Steps(
        {
            # Set a fractional command and ensure only sets are not spammed and the component has a rounded value as well
            "/assets/solar/solar_2/maint_active_power_setpoint": 4.57
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/pv_2/active_power_setpoint", 5),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/components/pv_2/active_power_setpoint")
        ]
    ),
    Steps(
        {
            "/assets/generators/gen_1/maint_mode": True,
            # Repeat for Gen, round down this time
            "/assets/generators/gen_1/maint_active_power_setpoint": 4.3
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_available", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/easygen_3500xt/active_power_setpoint", 4),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/components/easygen_3500xt/active_power_setpoint")
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            # Repeat for ess, negative number this time
            "/assets/ess/ess_1/maint_active_power_setpoint": -2.5
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/active_power_setpoint", -3),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/components/ess_twins/active_power_setpoint")
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/assets/generators/gen_1/maint_mode": False
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_available", 1),
        ]
    )
])
def test_maint_active_power_rounding(test):
    return test


# Maint mode testing reactive power setpoint rounding
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "maint_reactive_power_rounding",
        {
            "/assets/solar/solar_2/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 1),
        ]
    ),
    Steps(
        {
            # Set a fractional command and ensure only sets are not spammed and the component has a rounded value as well
            "/assets/solar/solar_2/maint_reactive_power_setpoint": 4.57
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/pv_2/reactive_power_setpoint", 5),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/components/pv_2/reactive_power_setpoint")
        ]
    ),
    Steps(
        {
            "/assets/generators/gen_1/maint_mode": True,
            # Repeat for Gen, round down this time
            "/assets/generators/gen_1/maint_reactive_power_setpoint": 4.3
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_available", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/easygen_3500xt/reactive_power_setpoint", 4),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/components/easygen_3500xt/reactive_power_setpoint")
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            # Repeat for ess, negative number this time
            "/assets/ess/ess_1/maint_reactive_power_setpoint": -2.5
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/reactive_power_setpoint", -3),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/components/ess_twins/reactive_power_setpoint")
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/assets/generators/gen_1/maint_mode": False
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_available", 1),
        ]
    )
])
def test_maint_reactive_power_rounding(test):
    return test
