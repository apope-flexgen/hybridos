# ESS Calibration tests
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
    Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=250).make_assertion(),
    charge = fims_get("/assets/ess/ess_02/system_chargeable_power")
    charge = charge * -1
    Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_02/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=250).make_assertion(),

def test_dischargeable_power():
    sleep(.1)
    charge = fims_get("/assets/ess/ess_01/system_dischargeable_power")
    Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=250).make_assertion(),
    charge = fims_get("/assets/ess/ess_02/system_dischargeable_power")
    Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_02/active_power", charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=250).make_assertion(),

def test_component_chargeable_power_respected():
    comp_charge = fims_get("/components/flexgen_ess_01/ess_max_charge_power")
    comp_charge = comp_charge * -1
    Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_01/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

    comp_charge = fims_get("/components/flexgen_ess_02/ess_max_charge_power")
    comp_charge = comp_charge * -1
    Flex_Assertion(Assertion_Type.greater_than_eq, "/assets/ess/ess_02/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),


def test_component_dischargeable_power_respected():
    comp_charge = fims_get("/components/flexgen_ess_01/ess_max_discharge_power")
    Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_01/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

    comp_charge = fims_get("/components/flexgen_ess_02/ess_max_discharge_power")
    Flex_Assertion(Assertion_Type.less_than_eq, "/assets/ess/ess_02/active_power", comp_charge, 
                   tolerance_type=Tolerance_Type.abs, tolerance=300).make_assertion(),

def set_bms_soc(value: int):
    for i in range(0, 9):
        uri = "/components/bms_1/sbmu_" + str(i) + "_soc_value"
        fims_set(uri, value, destination="psm")

    for i in range(0, 9):
        uri = "/components/bms_2/sbmu_" + str(i) + "_soc_value"
        fims_set(uri, value, destination="psm")

def disable_bms_faults():
    uri = "/components/bms_1/disablefault"
    fims_set(uri, True, destination="psm")

    uri = "/components/bms_2/disablefault"
    fims_set(uri, True, destination="psm")


def enable_bms_faults():
    uri = "/components/bms_1/disablefault"
    fims_set(uri, False, destination="psm")

    uri = "/components/bms_2/disablefault"
    fims_set(uri, False, destination="psm")


@ fixture
@ parametrize("test", [
    # put the site into ess calibration
    Setup(
        "ESS Calibration",
        {
            "/features/active_power/runmode1_kW_mode_cmd": 5,
            "/site/operation/enable_flag": True,
            "/features/active_power/ess_calibration_soc_limits_enable": False,
            "/features/active_power/ess_calibration_rack_voltage_limits_enable": False,
            "/features/active_power/ess_calibration_cell_voltage_limits_enable": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode1_kW_mode_cmd", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_calibration_soc_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_calibration_rack_voltage_limits_enable", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_calibration_cell_voltage_limits_enable", False),
        ]
    ),
    # set SoC to 50 and charge  
    # ensure some default behavior
    Steps(
        {
            "/features/active_power/ess_calibration_kW_cmd": -4000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/soc", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/soc", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/active_power", -4000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/active_power", -4000),
        ],
        pre_lambda=[
            lambda: disable_bms_faults(),
            lambda: set_bms_soc(50)        ]
    ),
    # set SoC to 97 and make sure we charge at configured min charge
    Steps(
        {
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/soc", 97),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/soc", 97),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_chargeable_power", 3000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_chargeable_power", 3000),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected()
        ]
    ),
    # set SoC max limit and make sure we stop charging 
    Steps(
        {
            "/features/active_power/ess_calibration_soc_limits_enable": True,
            "/features/active_power/ess_calibration_max_soc_limit": 96
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_chargeable_power", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_chargeable_power", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected()
        ]
    ),
    # set max rack voltage limit and make sure we stop charging
    # just set it absurdly low
    Steps(
        {
            "/features/active_power/ess_calibration_soc_limits_enable": False,
            "/features/active_power/ess_calibration_rack_voltage_limits_enable": True,
            "/features/active_power/ess_calibration_max_rack_voltage_limit": 1 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_chargeable_power", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_chargeable_power", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected()
        ]
    ),
    # set max cell voltage limit and make sure we stop charging
    # just set it absurdly low
    Steps(
        {
            "/features/active_power/ess_calibration_rack_voltage_limits_enable": False,
            "/features/active_power/ess_calibration_cell_voltage_limits_enable": True,
            "/features/active_power/ess_calibration_max_cell_voltage_limit": 1
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_chargeable_power", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_chargeable_power", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(97),
            lambda: test_chargeable_power(),
            lambda: test_component_chargeable_power_respected()
        ]
    ),
    # set SoC to 3 and make sure we discharge at configured min discharge
    Steps(
        {
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/soc", 3),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 3000),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_dischargeable_power", 3000),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected()

        ]
    ),
    # set SoC min limit and make sure we stop discharging 
    Steps(
        {
            "/features/active_power/ess_calibration_soc_limits_enable": True,
            "/features/active_power/ess_calibration_min_soc_limit": 5 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_dischargeable_power", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected()

        ]
    ),
    # set min rack voltage limit and make sure we stop discharging
    # just set it absurdly low
    Steps(
        {
            "/features/active_power/ess_calibration_soc_limits_enable": False,
            "/features/active_power/ess_calibration_rack_voltage_limits_enable": True,
            "/features/active_power/ess_calibration_min_rack_voltage_limit": 10000 
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_dischargeable_power", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected()

        ]
    ),
    # set min cell voltage limit and make sure we stop discharging
    # just set it absurdly low
    Steps(
        {
            "/features/active_power/ess_calibration_rack_voltage_limits_enable": False,
            "/features/active_power/ess_calibration_cell_voltage_limits_enable": True,
            "/features/active_power/ess_calibration_min_cell_voltage_limit": 10
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/system_dischargeable_power", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/system_dischargeable_power", 0),
        ],
        pre_lambda=[
            lambda: set_bms_soc(3),
            lambda: test_dischargeable_power(),
            lambda: test_component_dischargeable_power_respected()

        ]
    ),
    Teardown(
        {
            "/features/active_power/ess_calibration_cell_voltage_limits_enable": False,
            "/features/active_power/ess_calibration_kW_cmd": 0,
        },
        [
        ],
        pre_lambda=[
            lambda: set_bms_soc(50),
            lambda: enable_bms_faults()
        ]
    )
])
def test_ess_cali(test):
    return test
