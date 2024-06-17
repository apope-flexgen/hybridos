# Fims API pytests
from pytest_cases import parametrize, fixture
from ..fims import fims_get, fims_set, no_fims_msgs
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
    edits: list[dict] = [{
        "uri": "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/active_cmd_kw/multiple_inputs",
        "up": True, },
        {"uri": "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/inactive_cmd_kw/multiple_inputs",
         "up": True, },]
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
            Flex_Assertion(
                Assertion_Type.obj_eq,
                "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/active_cmd_kw/multiple_inputs",
                current_active_inputs_selection),
            Flex_Assertion(
                Assertion_Type.obj_eq,
                "/dbi/site_controller/variables/variables/features/standalone_power/frequency_response/components/0/inactive_cmd_kw/multiple_inputs",
                current_inactive_inputs_selection)
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


# Prevent grid mode spam by not allowing it to send sets until it calls appropriate sequences functions
@ fixture
@ parametrize("test", [
    Setup(
        "grid_mode_doesnt_spam",
        {
            "/components/ess_psm/on_off_grid_mode": 100,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/components/ess_psm/on_off_grid_mode", 100),
        ],
        pre_lambda=[
            # we want to go sit in the ready state.
            # aka not call the sequences function that will then allow the set to go out from grid_mode_setpoint.
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(auto_restart=False),],
        post_lambda=[
            lambda: no_fims_msgs("/components/ess_psm/on_off_grid_mode"),
        ]
    ),
    Steps(
        {
            "/site/operation/enable_flag": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/site/operation/running_status_flag", True, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq,
                           "/components/ess_psm/on_off_grid_mode", 0),  # 0 is following
        ]
    ),
    # nothing to really do for teardown
    Teardown(
        {},
        []
    )
])
def test_grid_mode_doesnt_spam(test):
    return test


mask_before_testing = {}

# setup_available_features_mask


def setup_multiple_inputs(value: str):
    global mask_before_testing
    if len(mask_before_testing.keys()) == 0:
        mask_before_testing = fims_get("/dbi/site_controller/variables/variables/features/active_power/available_features_runmode1_kW_mode")

    update_mask = fims_get("/dbi/site_controller/variables/variables/features/active_power/available_features_runmode1_kW_mode")
    update_mask['value'] = value

    # Get the currently configured multiple_inputs selections if present, default to false if not

    edits: list[dict] = [
        {
            "uri": "/dbi/site_controller/variables/variables/features/active_power/available_features_runmode1_kW_mode",
            "up": update_mask,
        }
    ]
    return edits

# Validate feature dropdown by checking the cmd object


@ fixture
@ parametrize("test", [
    Setup(
        "validate_feat_dropdown",
        {},
        [],
        # Restart with 0x3F mask (all feats on)
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(setup_multiple_inputs("0x3F")),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.obj_eq, "/features/active_power",
                           [
                               {
                                   "name": "Disabled",
                                   "return_value": 5
                               },
                               {
                                   "name": "ESS Calibration",
                                   "return_value": 4
                               },
                               {
                                   "name": "Manual",
                                   "return_value": 3
                               },
                               {
                                   "name": "Active Power Setpoint",
                                   "return_value": 2
                               },
                               {
                                   "name": "Target SOC",
                                   "return_value": 1
                               },
                               {
                                   "name": "Energy Arbitrage",
                                   "return_value": 0
                               }
                           ],
                           wait_secs=1, pattern="runmode1_kW_mode_cmd.options"),
        ],
        # Restart with 0xF mask (remove disabled/ess calibration)
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(setup_multiple_inputs("0xF")),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.obj_eq, "/features/active_power",
                           [
                               {
                                   "name": "Manual",
                                   "return_value": 3
                               },
                               {
                                   "name": "Active Power Setpoint",
                                   "return_value": 2
                               },
                               {
                                   "name": "Target SOC",
                                   "return_value": 1
                               },
                               {
                                   "name": "Energy Arbitrage",
                                   "return_value": 0
                               }
                           ],
                           wait_secs=1, pattern="runmode1_kW_mode_cmd.options"),
        ],
        # Restart with 0x2F mask (remove only ess calibration) (PROVES YOU CAN HAVE A "JUMP")
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(setup_multiple_inputs("0x2F")),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    ),
    Steps(
        {},
        [
            Flex_Assertion(Assertion_Type.obj_eq, "/features/active_power",
                           [
                               {
                                   "name": "Disabled",
                                   "return_value": 5
                               },
                               {
                                   "name": "Manual",
                                   "return_value": 3
                               },
                               {
                                   "name": "Active Power Setpoint",
                                   "return_value": 2
                               },
                               {
                                   "name": "Target SOC",
                                   "return_value": 1
                               },
                               {
                                   "name": "Energy Arbitrage",
                                   "return_value": 0
                               }
                           ],
                           wait_secs=1, pattern="runmode1_kW_mode_cmd.options"),
        ]
    ),
    # nothing to really do for teardown restore prior state and finish
    Teardown(
        {},
        [],
        post_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(setup_multiple_inputs(mask_before_testing['value'])),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(True)
        ]
    )
])
def test_validate_feat_dropdown(test):

    # Multiple ESSs running at different SOCs


@ fixture
@ parametrize("test", [
    Setup(
        "ess_soc_monitoring",
        {},
        [
            # ESS 1 & 2 running outside of maintenance mode
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_running", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_running", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_running", 62.5, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_controllable", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_controllable", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_controllable", 62.5, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_all", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_all", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_all", 62.5, wait_secs=0),
        ],
        pre_lambda=[
            # Different state of charge for each ess
            lambda: fims_set("/components/psm_ess_1/soc", 45, destination="psm"),
            lambda: fims_set("/components/psm_ess_2/soc", 80, destination="psm"),
        ],
    ),
    Steps(
        {
            # ESS 1 running in maintenance mode, ESS 2 running out of maintenance mode
            "/assets/ess/ess_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_running", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_running", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_running", 62.5, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_controllable", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_controllable", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_controllable", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_all", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_all", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_all", 62.5, wait_secs=0),
        ]
    ),
    Steps(
        {
            # ESS 1 & 2 running in maintenance mode
            "/assets/ess/ess_2/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_running", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_running", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_running", 62.5, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_all", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_all", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_all", 62.5, wait_secs=0),
        ]
    ),
    Steps(
        {
            # ESS 1 stopped & ESS 2 running, (both) in maintenance mode
            "/assets/ess/ess_1/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_neq, "/assets/ess/ess_1/start", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_running", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_running", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_running", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_all", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_all", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_all", 62.5, wait_secs=0),
        ]
    ),
    Steps(
        {
            # ESS 1 & 2 stopped in maintenance mode
            "/assets/ess/ess_2/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_neq, "/assets/ess/ess_1/start", True, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_neq, "/assets/ess/ess_2/start", True, wait_secs=1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_running", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_running", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_running", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_controllable", 0, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_min_all", 45, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_max_all", 80, wait_secs=0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/soc_avg_all", 62.5, wait_secs=0),
        ]
    ),
    Teardown(
        {
            "/assets/ess/ess_1/start": True,
            "/assets/ess/ess_2/start": True,
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
        },
        []
    )
])
def test_ess_soc_monitoring(test):
    return test
