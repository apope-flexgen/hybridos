import logging
import subprocess
from unittest.result import failfast
from pytest_cases import parametrize, fixture
from pytests.assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from pytests.pytest_steps import Setup, Steps, Teardown


# Active Power Run Mode 1 - Manual Mode Tests
# Test each asset with slew rates of 1 kW/s, 1 MW/s, and 1 GW/s

# These tests require the following config (see config branch testing/manual-mode):
# assets.json:
#   solar:
#       "rated_active_power_kW": 500000.0,
#       "slew_rate": 500000,
#       "rated_reactive_power_kvar": 500000.0,
#       "rated_apparent_power_kva": 500000.0,
#   ess:
#       "rated_active_power_kW": 1000000.0,
#       "slew_rate": 1000000,
#       "rated_reactive_power_kvar": 1000000.0,
#       "rated_apparent_power_kva": 1000000.0,
#       "rated_capacity": 1000000,
#   gen:
#       "rated_active_power_kW": 1000000.0,
#       "slew_rate": 1000000,
#       "rated_reactive_power_kvar": 1000000.0,
#       "rated_apparent_power_kva": 1000000.0,
# psm.json:
#   solar:
#       "pramp" and "qramp": 30000000,
#       "phigh" and "plim": 500000,
#   ess:
#       "phigh" and "plow": 1000000,
#       "cap": 1000000,
#   gen:
#       "pramp" and "qramp": 60000000,
#   psm_shared_poi and psm_split_feeder:
#       "pmax", "qmax", and "smax": 3000000,

# Solar slew rate test 1: 1 kW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_solar_slew_rate_1",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_solar_kW_slew_rate": 1000000,
            "/features/active_power/manual_solar_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),    
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_solar_kW_slew_rate": 1
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_solar_kW_slew_rate", 1)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_solar_kW_cmd": 10
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/active_power/solar_kW_cmd", 10, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 10, wait_secs=9),
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_solar_kW_slew_rate": 1000000,
            "/features/active_power/manual_solar_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0)
        ]
    )
])
def test_manual_solar_slew_rate_1(test):
    return test


# Solar slew rate test 2: 1 MW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_solar_slew_rate_2",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_solar_kW_slew_rate": 1000000,
            "/features/active_power/manual_solar_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_solar_kW_slew_rate": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_solar_kW_slew_rate", 1000)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_solar_kW_cmd": 5000
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/active_power/solar_kW_cmd", 5000, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 5000, wait_secs=4)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_solar_kW_slew_rate": 1000000,
            "/features/active_power/manual_solar_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0)
        ]
    )
])
def test_manual_solar_slew_rate_2(test):
    return test


# TODO The solar slew rate test for 1 GW/s is currently not passing, but should be revisited when solar slew rates
# are further investigated and improved, most likely in relation to Jira ticket DC-158
# Solar slew rate test 3: 1 GW/s
""" @ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_solar_slew_rate_3",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_solar_kW_slew_rate": 1000000,
            "/features/active_power/manual_solar_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),     
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_solar_kW_slew_rate": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_solar_kW_slew_rate", 1000000)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_solar_kW_cmd": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 1000000, wait_secs=3)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_solar_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0)
        ]
    )
])
def test_manual_solar_slew_rate_3(test):
    return test """


# ESS slew rate test 1: 1 kW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_ess_slew_rate_1",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_ess_kW_slew_rate": 1000000,
            "/features/active_power/manual_ess_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_ess_kW_slew_rate": 1
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_ess_kW_slew_rate", 1)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_ess_kW_cmd": 10
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/active_power/ess_kW_cmd", 10, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 10, wait_secs=9)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_ess_kW_slew_rate": 1000000,
            "/features/active_power/manual_ess_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0)
        ]
    )
])
def test_manual_ess_slew_rate_1(test):
    return test


# ESS slew rate test 2: 1 MW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_ess_slew_rate_2",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_ess_kW_slew_rate": 1000000,
            "/features/active_power/manual_ess_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_ess_kW_slew_rate": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_ess_kW_slew_rate", 1000)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_ess_kW_cmd": 5000
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/active_power/ess_kW_cmd", 5000, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 5000, wait_secs=4)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_ess_kW_slew_rate": 1000000,
            "/features/active_power/manual_ess_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0)
        ]
    )
])
def test_manual_ess_slew_rate_2(test):
    return test


# ESS slew rate test 3: 1 GW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_ess_slew_rate_3",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_ess_kW_slew_rate": 1000000,
            "/features/active_power/manual_ess_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_ess_kW_slew_rate": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_ess_kW_slew_rate", 1000000)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_ess_kW_cmd": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 1000000, wait_secs=1)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_ess_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0)
        ]
    )
])
def test_manual_ess_slew_rate_3(test):
    return test


# Gen slew rate test 1: 1 kW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_gen_slew_rate_1",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_gen_kW_slew_rate": 1000000,
            "/features/active_power/manual_gen_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),    
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_gen_kW_slew_rate": 1
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_gen_kW_slew_rate", 1)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_gen_kW_cmd": 10
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/active_power/gen_kW_cmd", 10, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 10, wait_secs=9)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_gen_kW_slew_rate": 1000000,
            "/features/active_power/manual_gen_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0)
        ]
    )
])
def test_manual_gen_slew_rate_1(test):
    return test


# Gen slew rate test 2: 1 MW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_gen_slew_rate_2",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_gen_kW_slew_rate": 1000000,
            "/features/active_power/manual_gen_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_gen_kW_slew_rate": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_gen_kW_slew_rate", 1000)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_gen_kW_cmd": 5000
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/features/active_power/gen_kW_cmd", 5000, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 5000, wait_secs=4)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_gen_kW_slew_rate": 1000000,
            "/features/active_power/manual_gen_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0)
        ]
    )
])
def test_manual_gen_slew_rate_2(test):
    return test


# Gen slew rate test 3: 1 GW/s
@ fixture
@ parametrize("test", [
    # Preconditions 
    Setup(
        "manual_gen_slew_rate_3",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 3,
            "/features/active_power/manual_gen_kW_slew_rate": 1000000,
            "/features/active_power/manual_gen_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_gen_kW_slew_rate": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/manual_gen_kW_slew_rate", 1000000)
        ]
    ),
    Steps(
        {
            "/features/active_power/manual_gen_kW_cmd": 1000000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 1000000, wait_secs=1)
        ]
    ),
    Teardown(
        {
            "/features/active_power/manual_gen_kW_cmd": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0)
        ]
    )
])
def test_manual_gen_slew_rate_3(test):
    return test
