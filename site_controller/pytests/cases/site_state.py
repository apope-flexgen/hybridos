

# Site State Test Steps
from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "site_state",
        {},
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state", "RunMode1")
    ),
    Steps(
        {},
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state_enum", 3)  # RunMode1 == 3
    ),
    Teardown({}, [])
])
def test_site_state(test):
    return test
