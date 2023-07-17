# Sequences tests
from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# TODO: dynamic configuration so other endpoints (num_running/avail) can be tested as well
# TODO: can't figure out how to get parametrize to generate (setup, step 1, teardown, setup, step 2, teardown automatically)
#       instead it generates (setup, step 1, step 2, teardown)
@ fixture
@ parametrize("test", [
    # Test Startup with reduced ESS
    Setup(
        "num_ess_transitions",
        {"/site/operation/enable_flag": True},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
        ]
    ),
    # Confirm shutdown
    Steps(
        {"/site/operation/disable_flag": True},
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", False)
    ),
    # Confirm startup error
    Steps(
        {
            "/site/operation/disable_flag": False,
            "/assets/ess/ess_1/maint_mode": True,
            "/site/operation/enable_flag": True
        },
        [
            # TODO: dbi edit to reduce step timeout to speed up this test
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", False, wait_secs=15),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 4),
        ]
    ),
    Teardown(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/site/operation/clear_faults_flag": True,
            "/site/operation/enable_flag": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", True, wait_secs=15),
        ]
    )
])
def test_num_ess_transitions(test):
    return test
