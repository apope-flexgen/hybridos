import pytest
import random

# Site Manager Pytests
from pytest_cases import parametrize, fixture

from pytests.pytest_framework import Site_Controller_Instance
from pytests.pytest_steps import Setup, Steps, Teardown

from .assertion_framework import Assertion_Type, Flex_Assertion, Tolerance_Type

@ fixture
@ parametrize("test", [
        Setup(
            "test_fims_site_sets",
            {},
            [],
        )
    ] + [
        Steps(
            {},
            [
                Flex_Assertion(Assertion_Type.approx_eq, uri, val, fims_method='set', wait_secs=0) for val in values
            ]
        ) for (uri, values) in [
            ("/site/cops/cops_heartbeat", [0, random.randint(1, 1000)]),
            ("/site/configuration/reserved_float_1", [0, 1 + 1000 * random.random()]),
            ("/site/configuration/reserved_float_5", [0, 1 + 1000 * random.random()]),
            ("/site/configuration/reserved_float_8", [0, 1 + 1000 * random.random()]),
            ("/site/debug/state", [0, 1]),
            ("/site/configuration/reserved_bool_1", [True, False]),
            ("/site/operation/primary_controller", [False, True]), # Make sure we end with primary true, otherwise site_controller stops pubbing
            ("/site/operation/enable_flag", [True, False]),
            ("/site/input_sources/ui", [True, False]),
            ("/site/debug/fault", [True, False]),
        ]
    ] + [
        Teardown(
            {},
            [],
            # Undo reckless sets
            post_lambda=[
                lambda: Site_Controller_Instance.get_instance().restart_site_controller()
            ]
        )
    ]
)
def test_fims_site_sets(test):
    return test

@ fixture
@ parametrize("test", [
        Setup(
            "test_fims_features_sets",
            {},
            [],
        )
    ] + [
        Steps(
            {},
            [
                Flex_Assertion(Assertion_Type.approx_eq, uri, val, fims_method='set', wait_secs=0) for val in values
            ]
        ) for (uri, values) in [
            ("/features/active_power/runmode1_kW_mode_cmd", [0, 6]),
            ("/features/reactive_power/reactive_setpoint_kVAR_cmd", [0, 1 + 1000 * random.random()]),
            ("/features/standalone_power/active_power_poi_limits_enable", [True, False]),
        ]
    ] + [
        Teardown(
            {},
            [],
            # Undo reckless sets
            post_lambda=[
                lambda: Site_Controller_Instance.get_instance().restart_site_controller()
            ]
        )
    ]
)
def test_fims_features_sets(test):
    return test

@ fixture
@ parametrize("test", [
        Setup(
            "test_fims_assets_sets",
            {},
            [],
        )
    ] + [
        Steps(
            {},
            [
                Flex_Assertion(Assertion_Type.approx_eq, uri, val, fims_method='set', wait_secs=0) for val in values
            ]
        ) for (uri, values) in [
            ("/assets/ess/ess_1/maint_active_power_setpoint", [0, 1 + 1000 * random.random()]),
            ("/assets/ess/ess_1/maint_mode", [True, False]),
            ("/assets/ess/ess_2/clear_faults", [True]),
            ("/assets/solar/solar_1/maint_active_power_setpoint", [0, 1 + 1000 * random.random()]),
            ("/assets/solar/solar_1/maint_mode", [True, False]),
            ("/assets/solar/solar_2/clear_faults", [True]),
            ("/assets/generators/gen_1/maint_active_power_setpoint", [0, 1 + 1000 * random.random()]),
            ("/assets/generators/gen_1/maint_mode", [True, False]),
            ("/assets/generators/gen_1/clear_faults", [True]),
            ("/assets/feeders/feed_1/breaker_reset", [True]),
        ]
    ] + [
        Teardown(
            {},
            [],
            # Undo reckless sets
            post_lambda=[
                lambda: Site_Controller_Instance.get_instance().restart_site_controller()
            ]
        )
    ]
)
def test_fims_assets_sets(test):
    return test

# Test runner AKA main() for each individual test
@ parametrize("current_test", [
    test_fims_site_sets,
    test_fims_features_sets,
    test_fims_assets_sets,
])
def test_fims_interface(request: pytest.FixtureRequest, current_test: Steps):
    # Extract the pytest id
    Site_Controller_Instance.get_instance()  # Lazy initialization
    current_id = request.node.name[request.node.name.find("[")+1:request.node.name.find("]")]
    current_test.run_steps(current_id)
