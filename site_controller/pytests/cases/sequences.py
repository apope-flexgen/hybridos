# Sequences tests
from pytest_cases import parametrize, fixture
from pytests.fims import fims_get, fims_set
from time import sleep
from pytests.pytest_framework import Site_Controller_Instance
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown


# Generate config changes for init tests
def generate_init_config():
    current_aps_slew = fims_get("/dbi/site_controller/variables/variables/features/active_power/active_power_setpoint_kW_slew_rate/value")
    config_edits: list[dict] = [
        # Modify register_ids to not compete with modbus_clients
        {
            "uri": "/dbi/site_controller/variables/variables/site/operation/disable_flag/value",
            "up": True,
            "down": False
        },
        {
            "uri": "/dbi/site_controller/variables/variables/features/active_power/active_power_setpoint_kW_slew_rate",
            "up": 10,
            "down": current_aps_slew
        },
    ]
    return config_edits


# Make sure the init state is always run, handling previous error cases such as disable flag being set
@ fixture
@ parametrize("test", [
    Setup(
        "init",
        # First ensure the disable state is reached to make sure we reach the old error state
        {
            "/components/bess_aux/active_power_setpoint": 0
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/disable_flag", True, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state_enum", 6),  # Disabled state
        ],
        # Restart with the overwritten configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_init_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    ),
    # Confirm shutdown
    Steps(
        # Send a power command and ensure it slews at the new, slower rate
        {
            "/site/operation/enable_flag": True,
            "/features/active_power/runmode1_kW_mode_cmd": 2,
            "/features/active_power/active_power_setpoint_kW_cmd": 10000
        },
        [
            # Should have slewed -30kWs in 3 seconds +50kWs of losses at zero
            # Really we just care about the order of magnitude though
            Flex_Assertion(Assertion_Type.greater_than_eq, "/features/active_power/feeder_actual_kW", 0),
        ]
    ),
    # Restart site
    Teardown(
        {"/features/active_power/active_power_setpoint_kW_cmd": 0},
        [Flex_Assertion(Assertion_Type.approx_eq, "/features/active_power/active_power_setpoint_kW_cmd", 0)],
        # Restart with the original configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.download(generate_init_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
        ]
    )
])
def test_init(test):
    return test


# Generate config changes for init tests
def generate_watchdog_config():
    current_ready_state_faults = fims_get("/dbi/site_controller/sequences/sequences/Ready/paths/0/active_faults")
    config_edits: list[dict] = [
        # Add watchdog fault to Ready state
        {
            "uri": "/dbi/site_controller/sequences/sequences/Ready/paths/0/active_faults",
            "up": [{"name": "/assets/ess/ess_2/watchdog_fault"}],
            "down": current_ready_state_faults
        }
    ]
    return config_edits


@ fixture
@ parametrize("test", [
    Setup(
        "watchdog_fault",
        {},
        [
            # Enters ready state without any faults
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0),
        ],
        # Restart with the overwritten configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.upload(generate_watchdog_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller(False)
        ]
    ),
    Steps(
        # Trigger watchdog timeout by disconnecting modbus component
        {"/components/ess_real_ls/component_connected": 0},
        # Confirm we get the fault instead of segfaulting
        # Value 2 comes from the mask (bit position 1)
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 2, wait_secs=5)
    ),
    # Confirm fault is cleared
    Steps(
        {
            "/components/ess_real_ls/component_connected": 1,
            "/site/operation/clear_faults_flag": True
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0)
    ),
    # Restart site
    Teardown(
        {"/site/operation/enable_flag": True},
        [Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", True, wait_secs=5)],
        # Restart with the original configs
        pre_lambda=[
            lambda: Site_Controller_Instance.get_instance().mig.download(generate_watchdog_config()),
            lambda: Site_Controller_Instance.get_instance().restart_site_controller()
        ]
    )
])
def test_watchdog_fault(test):
    return test


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
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", False, wait_secs=45),
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 4),
        ]
    ),
    # Confirm fault is cleared
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/site/operation/clear_faults_flag": True,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0)
    ),
    # Restart site
    Teardown(
        {
            "/site/operation/enable_flag": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/running_status_flag", True, wait_secs=5),
        ]
    )
])
def test_num_ess_transitions(test):
    return test


# This test makes sure that the sequence endpoint /<asset_type>/allow_auto_restart
# successfully prevents assets from automatically restarting. aka if an asset stops for
# any reason it will not startup again.

# REQUIREMENTS TO PASS THIS TEST:
#     1. add /ess/allow_auto_restart && /solar/allow_auto_restart && /gen/allow_auto_restart to runmode_1 sequences.json
#     2. THESE REQUIREMENTS EXIST IN CONFIG BRANCH (PYTEST_COMPATABLE_auto_restart_prevention)
@ fixture
@ parametrize("test", [
    # Test Startup with reduced ESS
    Setup(
        "auto_restart_prevention",
        {"/site/operation/enable_flag": True},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
        ],
        # Sleep between sets to give enough time to process and change states
        pre_lambda=[
            lambda: fims_set("/site/operation/disable_flag", True), lambda: sleep(0.1),
            lambda: fims_set("/site/operation/disable_flag", False), lambda: sleep(1),
        ]
    ),
    # place all assets in maint_mode
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/generators/gen_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
        ]
    ),
    # turn assets off THEY SHOULD NEVER AUTO RESTART
    Steps(
        {
            "/assets/ess/ess_1/stop": True,
            "/assets/solar/solar_1/stop": True,
            "/assets/generators/gen_1/stop": True,
            "/assets/ess/ess_2/stop": True,
            "/assets/solar/solar_2/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/status", "Stopped"),
        ]
    ),
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/generators/gen_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=2),  # give a little time for a sanity check
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/status", "Stopped"),
        ]
    ),
    Steps(
        {
            "/features/active_power/export_target_kW_cmd": 1000,  # run an active_power command and make sure all assets still stopped
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=2),  # give a little time for a sanity check
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/status", "Stopped"),
        ]
    ),
    Teardown(
        {"/site/operation/enable_flag": True},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 2, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 2),
        ]
    )
])
def test_auto_restart_prevention(test):
    return test


# This test makes sure that the sequence endpoint /gen/allow_auto_restart
# successfully prevents assets from automatically restarting while in runmode_1.
# runmode_2 will allow for generator autostart.

# REQUIREMENTS TO PASS THIS TEST:
#     1. sequences.json calls to /gen/allow_auto_restart in run1 (value: true)  and run2 (value: false)

# Test description:
#     - Startup in runmode_1
#         - manually stop generator and make sure it does not come back online
#     - Swap to runmode_2
#         - manually stop generator and make sure it does come back online
@ fixture
@ parametrize("test", [
    # start site and make sure we have needed assets
    Setup(
        "auto_restart_prevention_agt",
        {"/site/operation/enable_flag": True},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 1, wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 1),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_running", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_available", 1),
        ]
    ),
    Steps(
        {
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 0),
        ]
    ),
    # turn off solar should start gen to cover, but be prevented by the feature
    Steps(
        {
            "/assets/solar/solar_1/stop": True,
            "/assets/solar/solar_2/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/status", "Stopped"),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/status", "Stopped"),
        ],
        post_lambda=[
            lambda: fims_set("/assets/solar/solar_1/maint_mode", False),
            lambda: fims_set("/assets/solar/solar_2/maint_mode", False),
        ]
    ),
    Steps(
        {
            "/components/shared_poi/utility_status": False,  # Use this endpoint to swap to runmode_2 gen should start in this runmode when needed
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/site_state", "RunMode2", wait_secs=15),
        ]
    ),
    # place gen into maint_mode
    Steps(
        {
            "/assets/generators/gen_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
        ]
    ),
    # turn gen off SHOULD RESTART with valid soc
    Steps(
        {
            "/components/ess_psm/bms_soc": 10,
            "/assets/generators/gen_1/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/status", "Stopped"),
        ]
    ),
    Steps(
        {
            "/assets/generators/gen_1/maint_mode": False,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/status", "Running", wait_secs=10),
        ]
    ),
    Teardown(
        {
            "/components/ess_psm/bms_soc": 50,
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/site/operation/clear_faults_flag": True,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0),
    )
])
def test_auto_restart_prevention_agt(test):
    return test


# This test verifies the validity of the agt sequences file.
# There are a couple new sequence endpoints in this test as well.
# agt HAS A SPECIAL REQUIREMENT THAT THE SITE ALWAYS IMMEDIATELY
# TRIES TO ENTER STARTUP. As such the only way to disable the site
# is to manually enter the site disabled state.

# REQUIREMENTS TO PASS THIS TEST:
#     1. config branch specific to agt testing

# Test description (always verifying correct assets running):
#     - Disable site to prevent it running straight to runmode1
#     - Enable site make sure we enter runmode1
#     - Send LOU
#         - Make sure we enter Runmode2
#     - Restore utility_status
#         - Make sure we enter Runmode1
#     - Send LOU + a BESS fault
#         - Make sure we enter Runmode2-b where only the generator is running
#     - Restore utility_status
#         - Make sure we enter Runmode1
@ fixture
@ parametrize("test", [
    # start site and make sure we have needed assets
    Setup(
        "agt_sequences",
        {
            "/site/operation/disable_flag": True,
            "/components/shared_poi/utility_status": True,
            "/site/configuration/reserved_bool_15": True
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_running", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 0),
        ]
    ),
    Steps(
        {
            "/site/operation/disable_flag": False,  # should startup and go to runmode1
        },
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "RunMode1", wait_secs=15),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_2/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/generators/gen_1/status", "Stopped"),
        ]
    ),
    Steps(
        {
            "/components/shared_poi/utility_status": False,  # LOU should enter runmode2
        },
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "RunMode2", wait_secs=15),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_2/status", "Running"),
        ]
    ),
    Steps(
        {
            "/components/shared_poi/utility_status": True,  # LOU return runmode1
        },
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "RunMode1", wait_secs=15),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Running"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/generators/gen_1/status", "Stopped"),
        ]
    ),
    Steps(
        [
            # setup
            {
                "/components/shared_poi/utility_status": False,  # LOU should enter runmode2
            },
            # pubs
            {
                # pubbing a fault should enter final runmode with just gen
                "/components/ess_psm": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            },
        ],
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "RunMode2", wait_secs=20),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/generators/gen_1/status", "Running"),
        ]
    ),
    Steps(
        [
            # setup
            {
                "/components/shared_poi/utility_status": True,  # stay in gen solo on return
            }
        ],
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "RunMode2", wait_secs=15),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/generators/gen_1/status", "Running"),
        ]
    ),
    Steps(
        [
            # setup
            {
                "/site/operation/disable_flag": True,
                "/components/shared_poi/utility_status": True,
            }
        ],
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "Shutdown", wait_secs=15),
        ]
    ),
    # Test this visually
    Steps(
        [
            # setup
            {
                "/site/operation/disable_flag": False,
            },
            # pubs
            {
                # pubbing a fault should enter Shutdown loop
                "/components/ess_psm": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            },
        ],
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Stopped", wait_secs=5),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Stopped"),
            Flex_Assertion(Assertion_Type.str_cmp, "/assets/generators/gen_1/status", "Stopped"),
        ]
    ),

    Steps(
        [
            # setup
            {
                "/site/operation/disable_flag": True,
                "/components/shared_poi/utility_status": False,  # goto runmode2 on startup
            }
        ],
        [
            Flex_Assertion(Assertion_Type.str_cmp, "/site/operation/site_state", "Shutdown", wait_secs=10),
        ]
    ),
    # Test this visually
    #    Steps(
    #        [
    #            # setup
    #            {
    #                "/site/operation/disable_flag": False,
    #            },
    #            # pubs
    #            {
    #                "/components/ess_psm": {"faults": [{"string": "Something alarming is happening", "value": 1}]}, # pubbing a fault should enter Shutdown loop
    #                "/components/easygen_3500xt": {"faults": [{"string": "Something alarming is happening", "value": 1}]}, # pubbing a fault should enter Shutdown loop
    #            },
    #         ],
    #        [
    #            Flex_Assertion(Assertion_Type.str_cmp, "/assets/ess/ess_1/status", "Stopped"),
    #            Flex_Assertion(Assertion_Type.str_cmp, "/assets/solar/solar_1/status", "Stopped"),
    #            Flex_Assertion(Assertion_Type.str_cmp, "/assets/generators/gen_1/status", "Stopped"),
    #        ]
    #    ),
    Teardown(
        {
            "/site/operation/disable_flag": True,
            "/site/operation/clear_faults_flag": True,
            "/components/shared_poi/utility_status": True,
        },
        Flex_Assertion(Assertion_Type.approx_eq, "/site/operation/active_faults", 0),
    )
])
def test_agt_sequences(test):
    return test
