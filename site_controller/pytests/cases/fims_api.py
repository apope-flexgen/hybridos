# Fims API pytests
from pytest_cases import parametrize, fixture
from ..fims import fims_get
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_framework import Site_Controller_Instance
from ..pytest_steps import Setup, Steps, Teardown


# Multiple inputs endpoint with special check before processing
current_active_inputs_selection = False
current_inactive_inputs_selection = False    # Generic multiple inputs endpoint


# Enable multiple inputs for FR variables
def setup_multiple_inputs():
    global current_active_inputs_selection
    global current_inactive_inputs_selection
    # Get the currently configured multiple_inputs selections if present, default to false if not
    active_response = fims_get(
        "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/active_cmd_kw")
    if "multiple_inputs" in active_response:
        current_active_inputs_selection = active_response["multiple_inputs"]
    else:
        current_active_inputs_selection = False
    inactive_response = fims_get(
        "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/inactive_cmd_kw")
    if "multiple_inputs" in inactive_response:
        current_active_inputs_selection = inactive_response["multiple_inputs"]
    else:
        current_active_inputs_selection = False
    edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/active_cmd_kw/multiple_inputs",
            "up": True,
        },
        {
            "uri": "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/inactive_cmd_kw/multiple_inputs",
            "up": True,
        },
    ]
    return edits


# Revert multiple inputs for FR variables
def teardown_multiple_inputs():
    edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/active_cmd_kw/multiple_inputs",
            # Even if multiple inputs wasn't previously present in configuration, leave it in but set to the default value false
            "down": current_active_inputs_selection,
        },
        {
            "uri": "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/inactive_cmd_kw/multiple_inputs",
            # Even if multiple inputs wasn't previously present in configuration, leave it in but set to the default value false
            "down": current_inactive_inputs_selection,
        },
    ]
    return edits


# Frequency Response Multiple Inputs
@ fixture
@ parametrize("test", [
    # Preconditions
    Setup(
        "fr_multiple_inputs",
        {
            **Steps.disable_solar_and_gen()
        },
        [
            # The multiple_inputs status isn't exposed on fims so simply verify that the new endpoints are gettable
            Flex_Assertion(Assertion_Type.obj_eq, "/features/standalone_power",
                           "UF PFR Active Response Command: UI", pattern="uf_pfr_active_cmd_kw_ui.name"),
            Flex_Assertion(Assertion_Type.obj_eq, "/features/standalone_power",
                           "UF PFR Inactive Command: UI", pattern="uf_pfr_inactive_cmd_kw_ui.name"),
            Flex_Assertion(Assertion_Type.obj_eq, "/features/standalone_power",
                           "UF PFR Active Response Command: BRP DNP3", pattern="uf_pfr_active_cmd_kw_dnp3.name"),
            Flex_Assertion(Assertion_Type.obj_eq, "/features/standalone_power",
                           "UF PFR Inactive Command: BRP DNP3", pattern="uf_pfr_inactive_cmd_kw_dnp3.name")
        ],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(setup_multiple_inputs()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
        ]
    ),
    Steps(
        {
            "/features/standalone_power/uf_pfr_active_cmd_kw_ui": 500,
            "/features/standalone_power/uf_pfr_active_cmd_kw_dnp3": 1000,
            "/features/standalone_power/uf_pfr_inactive_cmd_kw_ui": 500,
            "/features/standalone_power/uf_pfr_inactive_cmd_kw_dnp3": 1000
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/features/standalone_power/uf_pfr_active_cmd_kw_ui", 500),
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/features/standalone_power/uf_pfr_active_cmd_kw_dnp3", 1000),
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/features/standalone_power/uf_pfr_inactive_cmd_kw_ui", 500),
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/features/standalone_power/uf_pfr_inactive_cmd_kw_dnp3", 1000),
        ]
    ),
    # Cleanup
    Teardown(
        {
            **Steps.enable_solar_and_gen(),
        },
        [
            # Check DBI as the framework cannot easily check that the multiple inputs endpoints do not exist
            Flex_Assertion(Assertion_Type.obj_eq, "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/active_cmd_kw/multiple_inputs", current_active_inputs_selection),
            Flex_Assertion(Assertion_Type.obj_eq, "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/inactive_cmd_kw/multiple_inputs", current_inactive_inputs_selection)
        ],
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance(
            ).mig.download(teardown_multiple_inputs()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
        ]
    )
])
def test_fr_multiple_inputs(test):
    return test
