from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# AGT Target Soc (ESS + Solar with POI lims)
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "agt_runmode1",
        {
            "/features/standalone_power/active_power_poi_limits_enable": True,
            "/features/active_power/runmode1_kW_mode_cmd": 1,
        },
        [
            # Solar handles load, ESS charges off of remaining solar up to POI limit (it's max available charge)
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 197, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 103),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -250),
        ]
    ),
    Steps(
        {
            # ESS charge reduces as load increases to stay within POI limit
            "/components/bess_aux/active_power_setpoint": -140  # max load value
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 197, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 103),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", -163),
        ]
    ),
    Steps(
        {
            # ESS stops charging at it's desired target
            "/components/ess_psm/bms_soc": 88,  # 88 = is 90% after SoC scaling
            "/components/bess_aux/active_power_setpoint": -50  # reset load to default so ess could charge more if it wanted
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -53, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 103),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0),
        ]
    ),
    Steps(
        {
            # ESS also does nothing in maint mode
            "/components/ess_psm/bms_soc": 50,  # reset SoC back to a charging value
            "/assets/ess/ess_1/maint_mode": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", -53),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 103),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 0),
        ]
    ),
    Steps(
        {
            # ESS picks up the remaining load when no solar available and POI limit is reduced
            # I believe the POI limit will never be this small but cover this case anyways
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
            "/features/standalone_power/active_power_poi_limits_min_kW": -100,
            "/components/bess_aux/active_power_setpoint": -140
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/feeder_actual_kW", 100, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/solar_kW_cmd", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/ess_kW_cmd", 40),
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/features/standalone_power/active_power_poi_limits_min_kW": -197,
            "/components/bess_aux/active_power_setpoint": -50
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/active_power_poi_limits_min_kW", -197),
            Flex_Assertion(Assertion_Type.approx_eq, "/components/bess_aux/active_power", 50),
        ]
    )
])
def test_agt_runmode1(test):
    return test
