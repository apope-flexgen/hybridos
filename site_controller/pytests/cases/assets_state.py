

# Asset State Test Steps
from time import sleep
from pytest_cases import parametrize, fixture
from pytests.fims import fims_get, fims_set
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown


# Default configuration values from assets.json
default_asset_status_type = None
default_asset_status_mask = None
default_local_status_mask = None
default_status_register_id = None
custom_asset_status_register = "test_status"
custom_other_status_register = "marital_status"


# Override the asset wide status type to be bitfield
def generate_asset_bit_field_config():
    # DBI won't let us set naked string values, so grab the whole config and override the specific values
    global default_asset_status_type
    global default_local_status_mask
    default_asset_status_type = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0/status_type")
    default_local_status_mask = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0/local_mode_status_mask")
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0")
    updated_config["status_type"] = "bit_field"
    updated_config["local_mode_status_mask"] = "0x6"
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0",
            "up": updated_config,
        },
    ]
    return config_edits


# Revert the updated asset wide override
def revert_asset_bit_field_config():
    # DBI won't let us set naked string values, so grab the whole config and override the specific values
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0")
    updated_config["status_type"] = default_asset_status_type
    updated_config["local_mode_status_mask"] = default_local_status_mask
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0",
            "down": updated_config,
        },
    ]
    return config_edits


# Override the local status type to be bitfield
def generate_local_bit_field_config():
    # DBI won't let us set naked string values, so grab the whole config and override the specific values
    global default_local_status_mask
    default_local_status_mask = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0/local_mode_status_mask")
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0")
    updated_config["local_mode_status_type"] = "bit_field"
    updated_config["local_mode_status_mask"] = "0x6"
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0",
            "up": updated_config,
        },
    ]
    return config_edits


# Revert the updated asset wide override
def revert_local_bit_field_config():
    # DBI won't let us set naked string values, so grab the whole config and override the specific values
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0")
    # Local mode status type shared the asset-wide status type by default. Remove the manual override
    del updated_config["local_mode_status_type"]
    updated_config["local_mode_status_mask"] = default_local_status_mask
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0",
            "down": updated_config,
        },
    ]
    return config_edits


# Override status register status_type
# Takes string argument for the new type to use. Either "random_enum" or "bit_field"
# Sets the asset instance level status type default to the opposite of the value received
def generate_overridden_status_type(new_type, new_mask):
    global default_status_register_id
    global default_asset_status_mask
    global default_asset_status_type
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0")
    default_asset_status_mask = updated_config["running_status_mask"]
    default_asset_status_type = updated_config["status_type"]
    # Parse json structure for a single component (ess_psm)
    default_status_register_id = updated_config["components"][0]["variables"]["status"]["register_id"]
    updated_config["components"][0]["variables"]["status"]["register_id"] = custom_asset_status_register
    updated_config["components"][0]["variables"]["status"]["status_type"] = new_type
    updated_config["running_status_mask"] = new_mask
    updated_config["status_type"] = "random_enum" if new_type is "bit_field" else "bit_field"
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0",
            "up": updated_config,
        },
    ]
    return config_edits


# Revert the overridden Status register's status_type
def revert_overridden_status_type():
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0")
    updated_config["running_status_mask"] = default_asset_status_mask
    updated_config["status_type"] = default_asset_status_type
    # Parse json structure for a single component (ess_psm)
    del updated_config["components"][0]["variables"]["status"]["status_type"]
    updated_config["components"][0]["variables"]["status"]["register_id"] = default_status_register_id
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0",
            "down": updated_config,
        },
    ]
    return config_edits


# Setup additional register with type Status to test both types of processing simultaneously
def generate_other_status_register():
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables")
    # Create a new object to insert into the asset
    updated_config[custom_other_status_register] = {"name": "marital status", "register_id": custom_other_status_register, "type": "Status"}
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables",
            "up": updated_config,
        },
    ]
    return config_edits


# Revert the additional register with type Status
def revert_other_status_register():
    updated_config = fims_get("/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables")
    # Delete the new object that was inserted
    del updated_config[custom_other_status_register]
    config_edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/assets/assets/ess/asset_instances/0/components/0/variables",
            "down": updated_config,
        },
    ]
    return config_edits


# Test defaulted random_enum status processing
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "default_local_mode",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False),
        ]
    ),
    Steps(
        {"/components/ess_psm/local_mode_signal": 1},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_alarm", 1),
        ]
    ),
    Teardown(
        {"/components/ess_psm/local_mode_signal": 0},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False),
        ]
    )
])
def test_default_local_mode(test):
    return test


# Test overriding the asset wide status type for processing
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "asset_bit_field_local_mode",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False),
            # The system status processessing will not longer work due to the change to bit_field without a corresponding
            # change in component publish. Use available instead for these tests as it's not dependent on a "running" status
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False),
        ],
        # Restart with the overwritten configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_asset_bit_field_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    ),
    # Multiple values, 1 and 2 should be matched by the bit field mask 0x6 (0b110)
    Steps(
        {"/components/ess_psm/local_mode_signal": 1},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_alarm", 1),
        ],
        # Reset to remote mode
        post_lambda=[
            lambda: fims_set("/components/ess_psm/local_mode_signal", 0), lambda: sleep(1),
            lambda: Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False).make_assertion()
        ]
    ),
    # Multiple values, 1 and 2 should be matched by the bit field mask 0x6 (0b110)
    Steps(
        {"/components/ess_psm/local_mode_signal": 2},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_alarm", 1),
        ]
    ),
    Teardown(
        {"/components/ess_psm/local_mode_signal": 0},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False),
        ],
        # Restart with the overwritten configs
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.download(revert_asset_bit_field_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    )
])
def test_asset_bit_field_local_mode(test):
    return test


# Test overriding the local mode specific status type for processing
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "local_bit_field_local_mode",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False),
            # Unlike the previous test, this test does not change the system status processing, so controllable is still functional
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False),
        ],
        # Restart with the overwritten configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_local_bit_field_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    ),
    # Multiple values, 1 and 2 should be matched by the bit field mask 0x6 (0b110)
    Steps(
        {"/components/ess_psm/local_mode_signal": 1},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_alarm", 1),
        ],
        # Reset to remote mode
        post_lambda=[
            lambda: fims_set("/components/ess_psm/local_mode_signal", 0), lambda: sleep(1),
            lambda: Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False).make_assertion()
        ]
    ),
    # Multiple values, 1 and 2 should be matched by the bit field mask 0x6 (0b110)
    Steps(
        {"/components/ess_psm/local_mode_signal": 2},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", True, wait_secs=15),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_alarm", 1),
        ]
    ),
    Teardown(
        {"/components/ess_psm/local_mode_signal": 0},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/local_mode_status", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/is_alarmed", False),
        ],
        # Restart with the overwritten configs
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.download(revert_local_bit_field_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()  # Auto start the site after this test
        ]
    )
])
def test_local_bit_field_local_mode(test):
    return test


# Test overriding status as bit_field
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "test_bit_field_status_override",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", ""),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, ""),
        ],
        # Override default "random_enum" status type with "bit_field"
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_overridden_status_type("bit_field", "0x6")),
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_other_status_register()),
            # Don't restart site controller as the required number of assets will not be running due to the status processing override
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    ),
    # Process zero status values
    Steps(
        [{},
        {
            # Running status value 0 does not match bit 0 (mask is 0b0110)
            "/components/ess_psm": {custom_asset_status_register: [{"string": "stopped", "value": 0}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 1),
        ]
    ),
    Steps(
        [{},
        {
            "/components/ess_psm": {custom_other_status_register: [{"string": "divorced", "value": 0}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, "divorced"),
        ]
    ),
    # Process nonzero status values
    Steps(
        [{},
        {
            # Running status value 1 matches bit 1 (mask is 0b0110)
            "/components/ess_psm": {custom_asset_status_register: [{"string": "running", "value": 1}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "running"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2),
        ]
    ),
    Steps(
        [{},
        {
            "/components/ess_psm": {custom_other_status_register: [{"string": "single", "value": 1}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, "single"),
        ]
    ),
    # Process different nonzero status values
    Steps(
        [{},
        {
            # Running status value 2 matches bit 2 (mask is 0b0110)
            "/components/ess_psm": {custom_asset_status_register: [{"string": "running (standby)", "value": 2}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "running (standby)"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2),
        ]
    ),
    Steps(
        [{},
        {
            "/components/ess_psm": {custom_other_status_register: [{"string": "married", "value": 2}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, "married"),
        ]
    ),
    Teardown(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ],
        # Restart with the overwritten configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.download(revert_overridden_status_type()),
            lambda: Site_Controller_Instance.get_instance().mig.download(revert_other_status_register()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()  # Auto start the site after this test
        ]
    )
])
def test_bit_field_status_override(test):
    return test


# Test overriding status as random_enum
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "test_random_enum_status_override",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", ""),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, ""),
        ],
        # Override default "bit_field" status type with "random_enum"
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_overridden_status_type("random_enum", "0x1")),
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_other_status_register()),
            # Don't restart site controller as the required number of assets will not be running due to the status processing override
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    ),
    # Process zero status values
    Steps(
        [{},
        {
            # Running status value 0 does not match bit 0 (mask is 0b0010)
            "/components/ess_psm": {custom_asset_status_register: [{"string": "stopped", "value": 0}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 1),
        ]
    ),
    Steps(
        [{},
        {
            "/components/ess_psm": {custom_other_status_register: [{"string": "divorced", "value": 0}]}
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, "divorced"),
        ]
    ),
    # Process nonzero status values
    Steps(
        [{},
        {
            # Running status value 1 matches bit 1 (mask is 0b0010)
            "/components/ess_psm": {custom_asset_status_register: [{"string": "running", "value": 1}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "running"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2),
        ]
    ),
    Steps(
        [{},
        {
            "/components/ess_psm": {custom_other_status_register: [{"string": "single", "value": 1}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, "single"),
        ]
    ),
    # Process different nonzero status values
    Steps(
        [{},
        {
            # Running status value 1 does not match bit 2 (mask is 0b0010)
            "/components/ess_psm": {custom_asset_status_register: [{"string": "standby", "value": 2}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "standby"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 1),
        ]
    ),
    Steps(
        [{},
        {
            "/components/ess_psm": {custom_other_status_register: [{"string": "married", "value": 2}]},
        }],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/" + custom_other_status_register, "married"),
        ]
    ),
    Teardown(
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ],
        # Restart with the overwritten configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.download(revert_overridden_status_type()),
            lambda: Site_Controller_Instance.get_instance().mig.download(revert_other_status_register()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()  # Auto start the site after this test
        ]
    )
])
def test_random_enum_status_override(test):
    return test
