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
