from pytest_cases import parametrize, fixture

from ..pytest_framework import Site_Controller_Instance
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# Fault testing
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "test_alerts",
        {},
        [
            # Site level
            Flex_Assertion(Assertion_Type.obj_eq, "/site/operation", [], wait_secs=.1, pattern="active_faults.options"),
            Flex_Assertion(Assertion_Type.obj_eq, "/site/operation", [], wait_secs=.1, pattern="active_alarms.options"),

            # Deprecated asset level summary totals
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_total_alarms", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_total_faults", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_total_alarms", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_total_faults", 0, wait_secs=.1),

            # Asset summary total counts
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_alarmed", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_faulted", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_alarmed", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_faulted", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/gen_num_alarmed", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/gen_num_faulted", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/feeders/summary/feeder_num_alarmed", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/feeders/summary/feeder_num_faulted", 0, wait_secs=.1),

            # is_faulted & is_alarmed
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/is_alarmed", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/is_alarmed", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/feeders/feed_1/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/feeders/feed_1/is_alarmed", False, wait_secs=.1),
        ],
        post_lambda=[
            Site_Controller_Instance.get_instance().mig.before_alerts,
            Site_Controller_Instance.get_instance().restart_site_controller
        ]

    ),
    # Ensure site level ess fault & 1 num_faulted
    Steps(
        [{}, {
            "/components/ess_twins": {"test_alarms": [{"string": "Something alarming is happening", "value": 1}]},
            "/components/ess_twins": {"test_alarms": [{"string": "It is alarming", "value": 2}]}
        }],
        [
            Flex_Assertion(Assertion_Type.obj_eq, "/site/operation",
                           [{'name': 'Asset Alarm Detected', 'return_value': 1}], pattern="active_alarms.options"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_alarmed", 1, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_total_alarms", 1, wait_secs=.1),
        ]
    ),

    # There are 2 "alarming" machines which == ess_num_alarmed
    Steps(
        [{}, {
            "/components/ess_real_ls": {"test_alarms": [{"value": 1, "string": "Something alarming is happening"}, {"value": 2, "string": "It is alarming"}, {"value": 3, "string": "Whoa this is a lot of alarms"}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_alarmed", 1, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_total_alarms", 1, wait_secs=.1),
        ]
    ),

    # Ensure site level ess fault & 1 num_faulted
    Steps(
        [{}, {
            "/components/ess_twins": {"test_faults": [{"value": 1, "string": "Something faulty is happening"}, {"value": 2, "string": "It is my fault"}]},
        }],
        [
            Flex_Assertion(Assertion_Type.obj_eq, "/site/operation",
                           [{"name": "Site Sequence Fault Detected", "return_value": 1}], pattern="active_faults.options"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_faulted", 1, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_total_faults", 1, wait_secs=.1),
        ]
    ),

    # There are 2 "faulting" machines which == ess_num_faulted
    Steps(
        [{}, {
            "/components/ess_real_ls": {"test_faults": [{"value": 1, "string": "Something faulty is happening"}, {"value": 2, "string": "It is my fault"}, {"value": 3, "string": "Whoa this is a lot of faults"}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_faulted", 1, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_total_faults", 1, wait_secs=.1),
        ]
    ),

    # Solar
    Steps(
        [{}, {
            "/components/pv_1": {"faults": [{"value": 1, "string": "Something faulty is happening"}, {"value": 2, "string": "It is my fault"}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_faulted", 1),
        ]
    ),
    Steps(
        [{}, {
            "/components/pv_2": {"faults": [{"value": 1, "string": "Something faulty is happening"}, {"value": 2, "string": "It is my fault"}, {"value": 3, "string": "Whoa this is a lot of faults"}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_faulted", 2),
        ]
    ),

    # Clear out faults
    Steps(
        {
            "/site/operation/clear_faults_flag": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/ess_num_faulted", 0, wait_secs=8),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_faulted", 0, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/is_faulted", False, wait_secs=.1),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0, wait_secs=.1)
        ]
    ),

    # SC is currently in state Ready - hit "start site"
    Teardown(
        {
            "/site/operation/enable_flag": True
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", True, wait_secs=8),
        post_lambda=[
            Site_Controller_Instance.get_instance().mig.after_alerts,
            Site_Controller_Instance.get_instance().restart_site_controller
        ]
    )
])
def test_alerts(test):
    return test
