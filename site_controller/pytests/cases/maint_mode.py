# Maint mode tests
# requires the pcs_bms_model sandbox configs
from pytest_cases import parametrize, fixture
from pytests.fims import fims_get, fims_set
from time import sleep
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type
from pytests.pytest_steps import Setup, Steps, Teardown
from pytests.fims import fims_get, fims_set
from subprocess import run

def test_chargeable_power():
    sleep(.1)
    charge = fims_get("/assets/ess/ess_01/system_chargeable_power")
    charge = charge * -1
    Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion()

def test_dischargeable_power():
    sleep(.1)
    charge = fims_get("/assets/ess/ess_01/system_dischargeable_power")
    Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion()

def set_bms_soc(value: int):
    for i in range(0, 9):
        uri = "/components/bms_1/sbmu_" + str(i) + "_soc_value"
        fims_set(uri, value, destination="psm")

def disable_bms_faults():
    uri = "/components/bms_1/disablefault"
    fims_set(uri, True, destination="psm")

def enable_bms_faults():
    uri = "/components/bms_1/disablefault"
    fims_set(uri, False, destination="psm")

def test_component_chargeable_power_respected():
    comp_charge = fims_get("/components/flexgen_ess_01/ess_max_charge_power")
    comp_charge = abs(comp_charge) * -1
    Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

def test_component_dischargeable_power_respected():
    comp_charge = fims_get("/components/flexgen_ess_01/ess_max_discharge_power")
    Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

# This test will put an ESS in maint_mode and then 
# test the min charge functionality
@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    # place soc limits at very high and low
    # remove SoC protection buffers
    # start up ess_01
    # ensure the system_chargeable_power is less than 4 MW
    Setup(
        "Min_Charge_Discharge",
        {
            **Steps.place_assets_in_maint(),
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_max_soc_limit": 99,
            "/assets/ess/ess_01/maint_soc_limits_enable": True,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": True,
            "/assets/ess/ess_01/start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/status", "Running"),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/system_chargeable_power", 4199, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ]
    ),
    # set SoC to very high
    Steps(
        {
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 97),
        ],
        pre_lambda=[
            lambda: disable_bms_faults(),
            lambda: set_bms_soc(97)
        ]
    ),
    # set min charge limit to 4 MW
    # set charge to 4 MW
    # Should not be derated and have full rated power
    Steps(
        {
            "/assets/ess/ess_01/maint_chargeable_min_limit": 4000,
            "/assets/ess/ess_01/maint_active_power_setpoint": -4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_chargeable_min_limit", 3000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_chargeable_power", 4200, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", -4200),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", -4200, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # set charge to 0
    # set soc to 99%
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 99),
        ],
        pre_lambda=[
            lambda: set_bms_soc(99)
        ]
    ),
    # set charge to non 0 and make sure we don't charge (because of the SoC limit)
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": -4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # set charge to 0
    # reset soc
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 98),
        ],
        pre_lambda=[
            lambda: set_bms_soc(98),
        ]
    ),
    # remove min limits and make sure we still are not derated
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": -4200,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", -4200),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_chargeable_power", 4200, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", -4200),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # add back protection buffers and confirm the charge is derated
    Steps(
        {
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/system_chargeable_power", 4199, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", -4199, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # prep for next test
    # set SoC to very low
    # Remove protection buffers from battery
    # ensure the system_dischargeable_power is less than 4 MW
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": True,
        },
        [
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", True),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/system_dischargeable_power", 4199, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3)
        ]
    ),
    # set min discharge to 4 MW
    Steps(
        {
            "/assets/ess/ess_01/maint_dischargeable_min_limit": 4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_dischargeable_min_limit", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 4000),
        ]
    ),
    # set discharge to 5 MW
    # Should not be derated below min discharge. Make sure we are discharging at requested setpoint
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 4200,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 4200, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 4200),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 4200, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # reset the SoC low
    # place SoC limit and make sure we don't discharge
    Steps(
        {
            "/assets/ess/ess_01/maint_min_soc_limit": 4,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3)
        ]
    ),
    # remove min SoC limit
    # check the discharge still is not derated
    Steps(
        {
            "/assets/ess/ess_01/maint_min_soc_limit": 2,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_active_power_setpoint": 4200,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 4200, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 4200, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    # add back protection buffers and confirm the discharge is derated
    Steps(
        {
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/system_dischargeable_power", 4199, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", 4199, tolerance_type=Tolerance_Type.abs, tolerance=0),
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
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": True,
            "/assets/ess/ess_01/maint_chargeable_min_limit": 10000,
            "/assets/ess/ess_01/maint_active_power_setpoint": -4200,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 97, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", -4200),
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
            "/assets/ess/ess_01/maint_dischargeable_min_limit": 10000,
            "/assets/ess/ess_01/maint_active_power_setpoint": 4200,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 3, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 4200),
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
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 50, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 4200),
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
            "/assets/ess/ess_01/maint_active_power_setpoint": -4200,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 50, tolerance_type=Tolerance_Type.abs,  tolerance=1, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", -4200),
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
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_max_soc_limit": 99,
            "/assets/ess/ess_01/maint_min_soc_limit": 0,
            "/assets/ess/ess_01/maint_max_cell_voltage_limit": 3.5,
            "/assets/ess/ess_01/maint_min_cell_voltage_limit": 2.5,
            "/assets/ess/ess_01/maint_chargeable_min_limit": 0,
            "/assets/ess/ess_01/maint_dischargeable_min_limit": 0,
            **Steps.remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_soc_limit", 99),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_soc_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_cell_voltage_limit", 3.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_cell_voltage_limit", 2.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_chargeable_min_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_dischargeable_min_limit", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(50),
            lambda: enable_bms_faults()
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
    # set SoC to very high
    # remove soc limits(protection buffers) and enable SoC charge/discharge limits
    # start up ess_01
    Setup(
        "maint_soc_limits",
        {
            **Steps.place_assets_in_maint(),
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_soc_limits_enable": True,
            "/assets/ess/ess_01/start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 97),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/status", "Running"),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97)
        ]
    ),
    # set max SoC limit to 95% 
    # You won't be able to charge
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": -4000,
            "/assets/ess/ess_01/maint_max_soc_limit": 95,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_soc_limit", 95),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", -4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove the max SoC limit
    # You should charge
    Steps(
        {
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/system_chargeable_power", 1, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", -1, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # prep for next test
    # set SoC to very low
    # set min SoC limit to 5% 
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_soc_limits_enable": True,
            "/assets/ess/ess_01/maint_min_soc_limit": 5,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_soc_limit", 5),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3)
        ]
    ),
    # set discharge to 4 MW
    # make sure we are not discharging
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove the max SoC limit
    # ensure we charge
    Steps(
        {
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/active_power", 1, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    Teardown(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_max_soc_limit": 99,
            "/assets/ess/ess_01/maint_min_soc_limit": 0,
            "/assets/ess/ess_01/maint_max_cell_voltage_limit": 3.5,
            "/assets/ess/ess_01/maint_min_cell_voltage_limit": 2.5,
            "/assets/ess/ess_01/maint_chargeable_min_limit": 0,
            "/assets/ess/ess_01/maint_dischargeable_min_limit": 0,
            **Steps.remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_soc_limit", 99),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_soc_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_cell_voltage_limit", 3.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_cell_voltage_limit", 2.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_chargeable_min_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_dischargeable_min_limit", 0),
        ]
    )
])
def test_maint_soc_limits(test):
    return test

# This test will put an ESS in maint_mode and then 
# test the Cell Volt limits functionality 
@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    # set SoC to very high
    # remove soc limits(protection buffers) and enable cell volt limits
    # start up ess_01
    Setup(
        "maint_soc_limits",
        {
            **Steps.place_assets_in_maint(),
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": True,
            "/assets/ess/ess_01/start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 97),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/status", "Running"),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97)
        ]
    ),
    # set max volt limit to 3 
    # You won't be able to charge
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": -4000,
            "/assets/ess/ess_01/maint_max_cell_voltage_limit": 3,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_cell_voltage_limit", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", -4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove the max SoC limit
    # You should charge
    Steps(
        {
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/system_chargeable_power", 1, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", -1, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # prep for next test
    # set SoC to very low
    # set min cell volt to 4 
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": True,
            "/assets/ess/ess_01/maint_min_cell_voltage_limit": 4,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_cell_voltage_limit", 4),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3)
        ]
    ),
    # set discharge to 4 MW
    # make sure we are not discharging
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove the cell volt limit
    # ensure we charge
    Steps(
        {
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/active_power", 1, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    Teardown(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_max_soc_limit": 99,
            "/assets/ess/ess_01/maint_min_soc_limit": 0,
            "/assets/ess/ess_01/maint_max_cell_voltage_limit": 3.5,
            "/assets/ess/ess_01/maint_min_cell_voltage_limit": 2.5,
            "/assets/ess/ess_01/maint_chargeable_min_limit": 0,
            "/assets/ess/ess_01/maint_dischargeable_min_limit": 0,
            **Steps.remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_cell_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_soc_limit", 99),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_soc_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_cell_voltage_limit", 3.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_cell_voltage_limit", 2.5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_chargeable_min_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_dischargeable_min_limit", 0),
        ]
    )
])
def test_maint_cell_volt_limits(test):
    return test

# This test will put an ESS in maint_mode and then 
# test the Rack Volt limits functionality 
# This test is kinda sad, but I'm limited by psm functionality as far as I'm aware
@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    # set SoC to very high
    # remove soc limits(protection buffers) and enable cell volt limits
    # start up ess_01
    Setup(
        "maint_soc_limits",
        {
            **Steps.place_assets_in_maint(),
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_rack_voltage_limits_enable": True,
            "/assets/ess/ess_01/start": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 97),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_rack_voltage_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/status", "Running"),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97)
        ]
    ),
    # set max rack volt limit to 1281 which is just below what psm spits out
    # You won't be able to charge
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": -4000,
            "/assets/ess/ess_01/maint_max_rack_voltage_limit": 1281,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_rack_voltage_limit", 1281),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", -4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove the max SoC limit
    # You should charge
    Steps(
        {
            "/assets/ess/ess_01/maint_rack_voltage_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_rack_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/system_chargeable_power", 1, tolerance_type=Tolerance_Type.abs, tolerance=0),
            Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", -1, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected(),
        ]
    ),
    # prep for next test
    # set SoC to very low
    # set min rack volt to 1283
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_soc_protection_buffers_disable": True,
            "/assets/ess/ess_01/maint_rack_voltage_limits_enable": True,
            "/assets/ess/ess_01/maint_min_rack_voltage_limit": 1283,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/flexgen_ess_01/bms_soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_protection_buffers_disable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_rack_voltage_limits_enable", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_rack_voltage_limit", 1283),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3)
        ]
    ),
    # set discharge to 4 MW
    # make sure we are not discharging
    Steps(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", 0, tolerance_type=Tolerance_Type.abs, tolerance=100),
        ]
    ),
    # remove the rack volt limit
    # ensure we charge
    Steps(
        {
            "/assets/ess/ess_01/maint_rack_voltage_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_rack_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/active_power", 1, tolerance_type=Tolerance_Type.abs, tolerance=0),
        ],
        pre_lambda=[
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected(),
        ]
    ),
    Teardown(
        {
            "/assets/ess/ess_01/maint_active_power_setpoint": 0,
            "/assets/ess/ess_01/maint_min_charge_discharge_enable": False,
            "/assets/ess/ess_01/maint_soc_limits_enable": False,
            "/assets/ess/ess_01/maint_rack_voltage_limits_enable": False,
            "/assets/ess/ess_01/maint_max_soc_limit": 99,
            "/assets/ess/ess_01/maint_min_soc_limit": 0,
            "/assets/ess/ess_01/maint_max_rack_voltage_limit": 2000,
            "/assets/ess/ess_01/maint_min_rack_voltage_limit": 1000,
            "/assets/ess/ess_01/maint_chargeable_min_limit": 0,
            "/assets/ess/ess_01/maint_dischargeable_min_limit": 0,
            **Steps.remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_charge_discharge_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_rack_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_soc_limit", 99),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_soc_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_max_rack_voltage_limit", 2000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_min_rack_voltage_limit", 1000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_chargeable_min_limit", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/maint_dischargeable_min_limit", 0),
        ]
    )
])
def test_maint_rack_volt_limits(test):
    return test
