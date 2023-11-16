# Sequences tests
from pytest_cases import parametrize, fixture
from pytests.fims import fims_get, fims_set
from time import sleep
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from pytests.pytest_steps import Setup, Steps, Teardown
from pytests.fims import fims_get, fims_set
from subprocess import run

def set_bms_soc(value: int):
    fims_set("/components/twins_ess_1/soc", value, destination="twins")

def test_chargeable_power():
    charge = fims_get("/assets/ess/ess_1/system_chargeable_power")
    charge = charge * -1
    Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

def test_dischargeable_power():
    charge = fims_get("/assets/ess/ess_1/system_dischargeable_power")
    Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

def test_component_chargeable_power_respected():
    comp_charge = fims_get("/components/ess_twins/chargeable_power")
    Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_1/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

def test_component_dischargeable_power_respected():
    comp_charge = fims_get("/components/ess_twins/dischargeable_power")
    Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_1/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

# This test will put an ESS in maint_mode and then 
# test the min charge functionality
@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Min_Charge_Discharge",
        {
            **Steps.place_assets_in_maint(),
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_1/maint_soc_limits_enable": False,
            "/assets/ess/ess_1/maint_voltage_limits_enable": False,
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
        ]
    ),
    # set SoC to very high
    # remove SoC protection buffers
    Steps(
        {
            "/assets/ess/ess_1/limits_override": True,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 97, tolerance_type=Tolerance_Type.abs, tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/limits_override", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", True),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
        ]
    ),
    # set min charge to 5 MW
    # set charge to 5.5 MW
    # Should be derated below min charge make sure we are charging at min charge
    Steps(
        {
            "/assets/ess/ess_1/maint_chargeable_min_limit": 5000,
            "/assets/ess/ess_1/maint_active_power_setpoint": -5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_chargeable_min_limit", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/system_chargeable_power", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", -5500),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", -5000),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # set charge to 0
    # set soc to 100%
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 100),
        ],
        pre_lambda=[
            lambda: set_bms_soc(100),
        ]
    ),
    # ensure we don't charge
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": -5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 0, tolerance_type=Tolerance_Type.abs,  tolerance=100),
        ]
    ),
    # set charge to 0
    # reset soc
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 98),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", False),
        ],
        pre_lambda=[
            lambda: set_bms_soc(98),
        ]
    ),
    # ensure we charge at derated with lambda
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": -5500,
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_1/system_chargeable_power", 5000),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # prep for next test
    # set SoC to very low
    # Remove protection buffers from battery
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 3),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_1/system_dischargeable_power", 5000),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
        ]
    ),
    # set min discharge to 5 MW
    # set discharge to 5.5 MW
    # Should be derated below min discharge make sure we are discharging at min discharge
    Steps(
        {
            "/assets/ess/ess_1/maint_dischargeable_min_limit": 5000,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": True,
            "/assets/ess/ess_1/maint_active_power_setpoint": 5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_dischargeable_min_limit", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/system_dischargeable_power", 5000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 5500),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 5000),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # set soc to 0%
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 0, tolerance_type=Tolerance_Type.abs, tolerance=1),
        ],
        pre_lambda=[
            lambda: set_bms_soc(0),
        ]
    ),
    # ensure we don't discharge
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove min limits
    # set soc to 2%
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_1/system_dischargeable_power", 5000),
        ],
        pre_lambda=[
            lambda: set_bms_soc(2),
        ]
    ),
    # check the discharge is derated
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 5500,
        },
        [
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_1/active_power", 5000),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # set SoC to very high
    # remove SoC protection buffers
    # set the min limit to above rated power. Make sure we charge at rated power.
    Steps(
        {
            "/assets/ess/ess_1/limits_override": True,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": True,
            "/assets/ess/ess_1/maint_chargeable_min_limit": 10000,
            "/assets/ess/ess_1/maint_active_power_setpoint": -5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 97, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/limits_override", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", -5500),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # set SoC to very low
    # set the min limit to above rated power. Make sure we discharge at rated power.
    Steps(
        {
            "/assets/ess/ess_1/maint_dischargeable_min_limit": 10000,
            "/assets/ess/ess_1/maint_active_power_setpoint": 5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 3, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 5500),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # set SoC to mid
    # set the min limits to above rated power. Make sure we discharge at rated power.
    Steps(
        {
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 50, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 5500),
        ],
        pre_lambda=[
            lambda: set_bms_soc(50),
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # set SoC to mid
    # set the min limits to above rated power. Make sure we charge at rated power.
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": -5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 50, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", -5500),
        ],
        pre_lambda=[
            lambda: set_bms_soc(50),
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # Turn off maint_mode and zero all setpoints
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_1/maint_soc_limits_enable": False,
            "/assets/ess/ess_1/maint_voltage_limits_enable": False,
            "/assets/ess/ess_1/maint_max_soc_limit": 100,
            "/assets/ess/ess_1/maint_min_soc_limit": 0,
            "/assets/ess/ess_1/maint_max_voltage_limit": 3.5,
            "/assets/ess/ess_1/maint_min_voltage_limit": 2.5,
            "/assets/ess/ess_1/maint_chargeable_min_limit": 0,
            "/assets/ess/ess_1/maint_dischargeable_min_limit": 0,
            **Steps.remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_max_soc_limit", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_soc_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_max_voltage_limit", 3.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_voltage_limit", 2.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_chargeable_min_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_dischargeable_min_limit", 0),
        ]
    )
])
def test_min_charge_discharge(test):
    return test


# This test will put an ESS in maint_mode and then 
# test the SoC limits functionality 
@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "maint_soc_limits",
        {
            **Steps.place_assets_in_maint(),
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_1/maint_soc_limits_enable": False,
            "/assets/ess/ess_1/maint_voltage_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
        ]
    ),
    # set SoC to very high
    # Remove protection buffers from battery
    # Enable SoC limits
    # set max SoC limit to 95% 
    Steps(
        {
            "/assets/ess/ess_1/limits_override": True,
            "/assets/ess/ess_1/maint_soc_limits_enable": True,
            "/assets/ess/ess_1/maint_max_soc_limit": 95,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 97,tolerance_type=Tolerance_Type.abs, tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/limits_override", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_soc_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_max_soc_limit", 95),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
        ]
    ),
    # set charge to 5.5 MW
    # make sure we are not charging
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": -5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", -5500),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 0, wait_secs=1),
        ]
    ),
    # remove the max SoC limit
    Steps(
        {
            "/assets/ess/ess_1/maint_soc_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_1/system_chargeable_power", 0),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # prep for next test
    # set SoC to very low
    # remove buffer protections
    # enable SoC limits
    # set min SoC limit to 5% 
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/limits_override": True,
            "/assets/ess/ess_1/maint_soc_limits_enable": True,
            "/assets/ess/ess_1/maint_min_soc_limit": 5,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/ess_twins/bms_soc", 3,tolerance_type=Tolerance_Type.abs, tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/limits_override", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_soc_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_soc_limit", 5),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
        ]
    ),
    # set discharge to 5.5 MW
    # make sure we are not discharging
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 5500,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 5500),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/active_power", 0, wait_secs=1),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # remove the max SoC limit
    # ensure we charge
    Steps(
        {
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_1/active_power", 0, wait_secs=1),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_1/maint_soc_limits_enable": False,
            "/assets/ess/ess_1/maint_voltage_limits_enable": False,
            "/assets/ess/ess_1/maint_max_soc_limit": 100,
            "/assets/ess/ess_1/maint_min_soc_limit": 0,
            "/assets/ess/ess_1/maint_max_voltage_limit": 3.5,
            "/assets/ess/ess_1/maint_min_voltage_limit": 2.5,
            "/assets/ess/ess_1/maint_chargeable_min_limit": 0,
            "/assets/ess/ess_1/maint_dischargeable_min_limit": 0,
            **Steps.remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_max_soc_limit", 100),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_soc_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_max_voltage_limit", 3.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_min_voltage_limit", 2.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_chargeable_min_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_dischargeable_min_limit", 0),
        ]
    )
])
def test_maint_soc_limits(test):
    return test
