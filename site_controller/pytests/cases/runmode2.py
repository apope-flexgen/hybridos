from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# Runmode2 Tests
# Test old "load support" (disabled) behavior against generic test bench
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "load_support",
        {
            "/components/shared_poi/utility_status": False,
            "/features/active_power/runmode2_kW_mode_cmd": 0,
            "/features/standalone_power/solar_shed_enable": True,
            "/assets/generators/gen_1/maint_mode": True,
            "/components/bess_aux/active_power_setpoint": -50
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode2_kW_mode_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/solar_shed_enable", True),
            # TODO: Judsen's string comp needed?
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state", "Runmode2"),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 50),
        ]
    ),
    # Solar starts at 0, meaning ESS must handle load
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 50),
        ]
    ),
    # Solar shedding reduces so that solar handles load and ESS charge
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 200, wait_secs=30),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -150),
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/utility_status": True,
            "/assets/generators/gen_1/maint_mode": False,
            "/features/standalone_power/solar_shed_enable": False,
            "/features/active_power/runmode2_kW_mode_cmd": 2,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state", "Runmode1"),
    )
])
def test_load_support(test):
    return test


# Test new Generator Charge feature
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "generator_charge",
        {
            "/components/shared_poi/utility_status": False,
            "/features/active_power/runmode2_kW_mode_cmd": 1,
            "/features/standalone_power/solar_shed_enable": True,
            "/components/bess_aux/active_power_setpoint": -50,
            "/components/ess_twins/bms_soc": 50,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/runmode2_kW_mode_cmd", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/solar_shed_enable", True),
            # TODO: Judsen's string comp needed?
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state", "Runmode2"),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 50),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/soc", 50),
        ]
    ),
    # Solar starts at 0, gen picks up load and ESS
    Steps(
        {
            # No solar available
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -200),
            # Gen picks up ESS + Load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", -250, wait_secs=30),
        ]
    ),
    # Solar shedding reduces so that solar handles some load
    Steps(
        {
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
        },
        [
            # Reduced Shedding by 1
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 25),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -200),
            # Gen picks up ESS + Load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", -225),
        ]
    ),
    Steps(
        {},
        [
            # No Shedding
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 100, wait_secs=30),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -200),
            # Gen picks up ESS + Load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", -150),
        ]
    ),
    # SoC above LDSS threshold but below max solar shedding
    Steps(
        {
            "/components/ess_twins/bms_soc", 90
        },
        [
            # Solar still uncurtailed
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 100),
            # ESS only gets 50kW, other 50kW goes to load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -50),
            # No Gen
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0),
        ]
    ),
    # SoC above LDSS threshold, gen does not start if load increases
    Steps(
        {
            "/components/bess_aux/active_power_setpoint", -140
        },
        [
            # Solar still uncurtailed
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 100),
            # ESS only gets 50kW, other 50kW goes to load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 40),
            # No Gen
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0),
        ]
    ),
    # SoC above max shedding
    Steps(
        {
            "/components/bess_aux/active_power_setpoint", -50,
            "/components/ess_twins/bms_soc", 95
        },
        [
            # Solar still uncurtailed
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0),
            # ESS covers load
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 50),
            # No Gen
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 0),
        ]
    ),
    # SoC drops below LDSS min threshold, generator comes back online
    Steps(
        {
            "/components/ess_twins/bms_soc", 5
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -200, wait_secs=30),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 250),
        ]
    ),
    # Generator does not stop if load reduces
    Steps(
        {
            "/components/bess_aux/active_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -200),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/gen_kW_cmd", 200),
        ]
    ),
    Teardown(
        {
            "/components/shared_poi/utility_status": True,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state", "Runmode1"),
    )
])
def test_load_support(test):
    return test
