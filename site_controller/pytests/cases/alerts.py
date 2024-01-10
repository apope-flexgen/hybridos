from pytest_cases import parametrize, fixture

from pytests.fims import no_fims_msgs
from ..pytest_framework import Site_Controller_Instance
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps, Teardown


# Generate configs changes for alerts
def generate_alerts_config():
    keys = ["faults", "alarms"]
    sequence_names = ["Init", "Ready", "RunMode1", "RunMode2", "Shutdown", "Standby", "Startup"]
    seq_uris = [f"/dbi/site_controller/sequences/sequences/{name}/paths/0" for name in sequence_names]
    seq_migs = [{"uri": f"{uri}/active_{key}", "up": [{"name": f"/assets/get_any_ess_{key}"}], "down": [{"name": "/bypass"}]}
                for uri in seq_uris for key in keys]
    alert_edits: list[dict] = [
        # Enable site level alarms and faults for all sequence types
        *seq_migs,
        # Modify register_ids to not compete with modbus_clients
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables/faults",
            "up": {"name": "Faults", "register_id": "test_faults", "type": "Int", "ui_type": "fault"},
            "down": {"name": "Faults", "register_id": "faults", "type": "Int", "ui_type": "fault"}
        },
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables/alarms",
            "up": {"name": "Alarms", "register_id": "test_alarms", "type": "Int", "ui_type": "alarm"},
            "down": {"name": "Alarms", "register_id": "alarms", "type": "Int", "ui_type": "alarm"}
        },
        {
            "uri": "/dbi/site_controller/sequences/sequences/RunMode1/paths/0/timeout",
            "up": {"value": 15},
            "down": {"value": 45}
        },
    ]
    return alert_edits


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
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_alerts_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
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
        ],
        post_lambda=[
            lambda: no_fims_msgs("/events", "pub")
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
        ],
        post_lambda=[
            lambda: no_fims_msgs("/events", "pub")
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
        ],
        post_lambda=[
            lambda: no_fims_msgs("/events", "pub")
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
        ],
        post_lambda=[
            lambda: no_fims_msgs("/events", "pub")
        ]
    ),

    # Solar
    Steps(
        [{}, {
            "/components/pv_1": {"faults": [{"value": 1, "string": "Something faulty is happening"}, {"value": 2, "string": "It is my fault"}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_faulted", 1),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/events", "pub")
        ]
    ),
    Steps(
        [{}, {
            "/components/pv_2": {"faults": [{"value": 1, "string": "Something faulty is happening"}, {"value": 2, "string": "It is my fault"}, {"value": 3, "string": "Whoa this is a lot of faults"}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/solar_num_faulted", 2),
        ],
        post_lambda=[
            lambda: no_fims_msgs("/events", "pub")
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
            lambda: Site_Controller_Instance.get_instance().mig.download(generate_alerts_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
        ]
    )
])
def test_alerts(test):
    return test
