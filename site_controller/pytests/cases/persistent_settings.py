from pytest_cases import parametrize, fixture
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown


# Confirm a valid setting like maint_active_power_setpoint does persist
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "persistent_setpoint",
        {
            "/assets/ess/ess_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 1000,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/dbi/site_controller/setpoints/assets/ess/ess_1/maint_active_power_setpoint", 1000),
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/maint_mode": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_active_power_setpoint", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
        ]
    )
])
def test_persistent_setpoint(test):
    return test


# Confirm contactor sets do not persist in dbi
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "persistent_contactors",
        {
            "/assets/ess/ess_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/close_dc_contactors": True,
        },
        [
            # We don't have functional dc contactors in test bench. Just confirm that the set is not in dbi for now
            Flex_Assertion(Assertion_Type.approx_eq, "/dbi/site_controller/setpoints/assets/ess/ess_1/close_dc_contactors",
                           "GET on /dbi/site_controller/setpoints/assets/ess/ess_1/close_dc_contactors encountered an error: field close_dc_contactors does not exist"),
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/open_dc_contactors": False,
        },
        [
            # We don't have functional dc contactors in test bench. Just confirm that the set is not in dbi for now
            Flex_Assertion(Assertion_Type.approx_eq, "/dbi/site_controller/setpoints/assets/ess/ess_1/open_dc_contactors",
                           "GET on /dbi/site_controller/setpoints/assets/ess/ess_1/open_dc_contactors encountered an error: field open_dc_contactors does not exist"),
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/ess/ess_1/maint_mode": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
        ]
    )
])
def test_persistent_contactors(test):
    return test


# Confirm autobalancing sets do not persist in dbi
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "persistent_autobalancing",
        {
            "/assets/ess/ess_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/autobalancing_enable": True,
        },
        [
            # We don't have functional autobalancing in test bench. Just confirm that the set is not in dbi for now
            Flex_Assertion(Assertion_Type.approx_eq, "/dbi/site_controller/setpoints/assets/ess/ess_1/autobalancing_enable",
                           "GET on /dbi/site_controller/setpoints/assets/ess/ess_1/autobalancing_enable encountered an error: field autobalancing_enable does not exist"),
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/autobalancing_disable": True,
        },
        [
            # We don't have functional autobalancing in test bench. Just confirm that the set is not in dbi for now
            Flex_Assertion(Assertion_Type.approx_eq, "/dbi/site_controller/setpoints/assets/ess/ess_1/autobalancing_disable",
                           "GET on /dbi/site_controller/setpoints/assets/ess/ess_1/autobalancing_disable encountered an error: field autobalancing_disable does not exist"),
        ]
    ),
    # Cleanup
    Teardown(
        {
            "/assets/ess/ess_1/maint_mode": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
        ]
    )
])
def test_persistent_autobalancing(test):
    return test
