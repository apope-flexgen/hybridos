import pytest
import subprocess
import time
import math
import statistics

from .controls import start_site, toggle_solar_and_gen_maintenance_mode, run_twins_command
from .fims import fims_set, fims_get, fims_del, poll_until_uri_is_at_value, parse_time_from_fims_trigger_output
from .pytest_framework import Site_Controller_Instance

import logging

LOGGER = logging.getLogger(__name__)


# Test precision constants
site_state_update_precision = 20
"""
Upper-bound millisecond inaccuracy caused by 10 millisecond site state update rate (allow for an update at the beginning and an update at the end of each test).
"""

throttle_timeout_fast_precision = 10
"""
Upper-bound ms inaccuracy caused by configured ess throttle_timeout_fast_ms.
"""


def calculate_abs_offset_power_from_rounded_setpoint(target_value: float) -> float:
    """
        Calculates the magnitude of offset to active/reactive power at test end caused by the ESS active/reactive power setpoint round check.
        The round check prevents sets except at rounded integer changes.
        i.e. If increasing steadily we'd have sets at approximately 0.5 kW, 1.5 kW, 2.5 kW, etc.
        Note: It seems as though this offset sometimes (though rarely) does not factor into the timing.
        i.e. A test that should take 3000 ms + 500 more ms due to the offset instead only takes 3000 ms.
        I think the reason why this happens is that sometimes the target value is passed at the same time that
        (target - 0.5) is passed (assuming a positive integer target), so we don't need to wait to reach (target + 0.5) for the final set to be sent.
        But that theory doesn't explain why (target - 0.5) would be passed so late.
        Still, I think just making the target of each test have a fractional part of 0.5 might make the tests slightly more consistent, since
        the offset is 0 if the target has a fractional part of 0.5.
    """
    # the setpoint rounding uses the cmath round function which rounds halfway points away from 0
    # https://cplusplus.com/reference/cmath/round/
    abs_target = abs(target_value)
    floor_abs_target = math.floor(abs_target)
    ceil_abs_target = math.ceil(abs_target)
    if abs_target <= floor_abs_target + 0.5:
        return floor_abs_target + 0.5 - abs_target
    else:
        return ceil_abs_target + 0.5 - abs_target


# Test cases specifying target value to slew to from initial value of 0 at the given slew rate.
# Expected times are approximate, and also do not take into account timing offsets which may be applied in individual tests.
# The min and max target values are -5500 and 5500 due to the asset configuration.
slew_rate_test_cases = [
    { # expected time: < 10 ms AKA within site_controller state machine cycle
        'target_value': 5000.5,
        'slew_rate': 1_000_000_000, # will cause expected slew time to hit floor of 10ms
    },
    { # expected time: < 10 ms AKA within site_controller state machine cycle
        'target_value': -5000.5,
        'slew_rate': 1_000_000_000, # will cause expected slew time to hit floor of 10ms
    },
    { # expected time: 10 ms (a state machine cycle)
        'target_value': 3000.5,
        'slew_rate': 300_000,
    },
    { # expected time: 10 ms (a state machine cycle)
        'target_value': -3000.5,
        'slew_rate': 300_000,
    },
    { # expected time: 50 ms
        'target_value': 3000.5,
        'slew_rate': 60_000,
    },
    { # expected time: 100 ms
        'target_value': 3000.5,
        'slew_rate': 30_000,
    },
    { # expected time: 100 ms
        'target_value': -3000.5,
        'slew_rate': 30_000,
    },
    { # expected time: 200 ms
        'target_value': 3000.5,
        'slew_rate': 15_000,
    },
    { # expected time: 1 s
        'target_value': 3000.5,
        'slew_rate': 3000,
    },
    { # expected time: 1 s
        'target_value': -3000.5,
        'slew_rate': 3000,
    },
    { # expected time: 3 s
        'target_value': 3000.5,
        'slew_rate': 1000,
    },
    { # expected time: 5.5 s
        'target_value': 5.5,
        'slew_rate': 1,
    },
    { # expected time: 6.5 s
        'target_value': 6.5,
        'slew_rate': 1,
    },
    { # expected time: 6 s
        'target_value': 3000.5,
        'slew_rate': 500,
    },
    { # expected time: 6 s
        'target_value': 600.5,
        'slew_rate': 100,
    },
    { # expected time: 50 s
        'target_value': 5000.5,
        'slew_rate': 100,
    },
    { # expected time: 60.5 s
        'target_value': 60.5,
        'slew_rate': 1,
    },
    # Test cases that take longer than a minute are below
    # { # expected time: 5 min
    #     'target_value': 300.5,
    #     'slew_rate': 1,
    # },
    # { # expected time: 5 min
    #     'target_value': 3000.5,
    #     'slew_rate': 10,
    # },
    # { # expected time: 10 min
    #     'target_value': 600.5,
    #     'slew_rate': 1,
    # },
    # { # expected time: 15 min
    #     'target_value': 900.5,
    #     'slew_rate': 1,
    # },
]


@pytest.fixture(scope="function")
def setup_cleanup_test_slew_rate():
    """
        Sets up and cleans up a Slew Rate test.
    """
    # Collect values of settings before test edits them so that cleanup can reset them
    ess_1_kW_slew_rate = fims_get('/dbi/site_controller/assets/assets/ess/asset_instances/0/slew_rate')
    ess_kvar_slew_rate = fims_get('/dbi/site_controller/variables/variables/site/configuration/ess_kVAR_slew_rate/value')
    ess_1_throttle_timeout_fast_ms = fims_get('/dbi/site_controller/assets/assets/ess/asset_instances/0/throttle_timeout_fast_ms')

    # Set the ESS kvar and kW slew rate values, which can only be set at start-up, to a number larger than
    # any of the slew values used in tests so that the ESS slew rates do
    # not get in the way of the slew tests by limiting more aggressively than the tested value
    fims_set('/dbi/site_controller/variables/variables/site/configuration/ess_kVAR_slew_rate/value', 2500_000_000)
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/0/slew_rate', 2500_000_000)
    # Set the ESS throttle_timeout_fast_ms to the desired number to obtain the given precision on ESS setpoint set times
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/0/throttle_timeout_fast_ms', throttle_timeout_fast_precision)

    # restart site_controller
    Site_Controller_Instance.get_instance().restart_site_controller()
    # put solar and gen in maintenance mode
    toggle_solar_and_gen_maintenance_mode(True)
    # Put all but first ESS in maintenance mode
    fims_set('/assets/ess/ess_2/maint_mode', True)
    assert fims_get('/assets/ess/ess_2/maint_mode')['value'] == True


    yield


    # Cleanup
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/0/slew_rate', ess_1_kW_slew_rate)
    fims_set('/dbi/site_controller/variables/variables/site/configuration/ess_kVAR_slew_rate/value', ess_kvar_slew_rate)
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/0/throttle_timeout_fast_ms', ess_1_throttle_timeout_fast_ms)
    # restart site_controller
    Site_Controller_Instance.get_instance().restart_site_controller()


def test_reactive_setpoint_slew_rate(setup_cleanup_test_slew_rate):
    """
        Tests Reactive Power Setpoint feature's slew rate variable, reactive_setpoint_kVAR_slew_rate.
        Ensures it can be edited via FIMS, the new slew takes effect immediately, and the slew is
        followed.

        NOTE: Since this test is measuring command changes across time, it will take significantly
        longer than other tests to run. To get an estimate of total runtime, add up all "expected time"s
        found in each test case.
    """
    # upper bound total ms inaccuracy (not counting variable inaccuracy specific to a test case)
    base_precision = site_state_update_precision + throttle_timeout_fast_precision

    # there is still some degree of error unaccounted for, which is only significant at long timescales (> 1 minute, see the trials below).
    # This coefficient attempts to account for that error by adding imprecision relative to test time.
    # (Speculation: the source of this error might be rounding in slew calculations and update timing)
    # The following trials were with reactive setpoint slew and had throttle_timeout_fast_precision set to 0
    # Trials: With timer end at 300 and slew rate 1 (~5 minute test), saw errors of up to 50 ms
    # Trials: With timer end at 600 and slew rate 1 (~10 minute test), saw errors of 119 ms, 120 ms, 118 ms, 121 ms
    # Trials: With timer end at 900 and slew rate 1 (~15 minute test), saw errors of 314 ms, 299 ms, 305 ms
    variable_precision_time_coefficient = 0.0005 # = 0.05% chosen based on trials

    for test_case in slew_rate_test_cases:
        target_value = test_case['target_value']
        # Make sure actual command sent is greater than the target so we can ensure the target is reached and passed
        reactive_setpoint_cmd = test_case['target_value']
        if reactive_setpoint_cmd > 0:
            reactive_setpoint_cmd += 100
        elif reactive_setpoint_cmd < 0:
            reactive_setpoint_cmd -= 100

        slew_rate = test_case['slew_rate']
        expected_slew_milliseconds = abs(target_value / slew_rate * 1000)
        # account for setpoint rounding check in reactive power at end of test
        expected_slew_milliseconds += calculate_abs_offset_power_from_rounded_setpoint(target_value) / slew_rate * 1000
        
        margin_of_error_milliseconds = base_precision + variable_precision_time_coefficient * expected_slew_milliseconds

        # Start in Site Export Target and Reactive Power Setpoint mode with 0 active and reactive power
        fims_set('/features/active_power/runmode1_kW_mode_cmd', 2)
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', 1000_000_000)
        fims_set('/features/active_power/active_power_setpoint_kW_cmd', 0)
        fims_set('/features/reactive_power/runmode1_kVAR_mode_cmd', 2)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', 1000_000_000)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', 0)
        # wait for power setpoints to settle down
        for uri in ['/assets/ess/ess_1/active_power_setpoint',   '/assets/ess/ess_2/active_power_setpoint',
                    '/assets/ess/ess_1/reactive_power_setpoint', '/assets/ess/ess_2/reactive_power_setpoint']:
            assert poll_until_uri_is_at_value(uri=uri, expected=0, tolerance=0.00001, timeout_seconds=1) == True
        
        # reset reactive power slew rate to 0 and wait for slew to update
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', 0)
        time.sleep(0.1)

        # Launch fims_trigger commands to time how long it takes for the reactive power setpoint to slew to its target value.
        if target_value > 0:
            target_comparison_trigger = '-g'
        if target_value < 0:
            target_comparison_trigger = '-l'
        fims_trigger = subprocess.Popen(['fims_trigger', '-s', '/features/reactive_power/reactive_setpoint_kVAR_cmd', '-e', str(reactive_setpoint_cmd), '-s', '/components/ess_twins/reactive_power_setpoint', target_comparison_trigger, str(target_value)],
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        # Set the slew rate
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', slew_rate)

        # Send a command to kick-off the slew
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', reactive_setpoint_cmd)

        # Wait for fims_trigger commands to terminate.
        # Set its timeout to be 5 seconds more than the longest expected slew time or 5 seconds for very quick test cases
        trigger_timeout = max(5 + (expected_slew_milliseconds + margin_of_error_milliseconds)/1000, 5)
        fims_trigger.wait(timeout=trigger_timeout)
        
        # Parse the fims_trigger outputs for the test result
        milliseconds_between_command_and_response = parse_time_from_fims_trigger_output(fims_trigger.communicate()[0])

        # Verify the difference between the actual slew time and the expected slew time is within the margin of error.
        def check_result(expected: float, actual: float, tolerance: float) -> None:
            LOGGER.info("\n".join([
                f"Reactive setpoint slew test: slew rate: {test_case['slew_rate']},"
                f"actual: {actual},"
                f"expected: {expected},"
                f"error: {actual-expected},"
                f"tolerance: {tolerance}"
            ]))
            assert abs(actual - expected) < tolerance
        check_result(expected_slew_milliseconds, milliseconds_between_command_and_response, margin_of_error_milliseconds)


def test_active_power_setpoint_slew_rate(setup_cleanup_test_slew_rate):
    """
        Tests Active Power Setpoint feature's slew rate variable, active_power_setpoint_kW_slew_rate.
        Ensures it can be edited via FIMS, the new slew takes effect immediately, and the slew is
        followed.

        NOTE: Since this test is measuring command changes across time, it will take significantly
        longer than other tests to run. To get an estimate of total runtime, add up all "expected time"s
        found in each test case.
    """
    # upper bound total ms inaccuracy (not counting variable inaccuracy specific to a test case)
    base_precision = site_state_update_precision + throttle_timeout_fast_precision

    # there is still some degree of error unaccounted for, which is only significant at long timescales (> 1 minute, see the trials below).
    # This coefficient attempts to account for that error by adding imprecision relative to test time.
    # (Speculation: the source of this error might be rounding in slew calculations and update timing)
    # The following trials were with export target slew and had throttle_timeout_fast_precision set to 0
    # Trials: With timer end at 300 and slew rate 1 (~5 minute test), saw errors of less than 40 ms
    # Trials: With timer end at 600 and slew rate 1 (~10 minute test), saw errors of 27 ms, 31 ms, 26 ms, 28 ms
    # Trials: With timer end at 900 and slew rate 1 (~15 minute test), saw errors of 40 ms, 54 ms, 46 ms
    variable_precision_time_coefficient = 0.0001 # = 0.01% chosen based on trials

    for test_case in slew_rate_test_cases:
        target_value = test_case['target_value']
        # Make sure actual command sent is greater than the target so we can ensure the target is reached and passed
        export_target_cmd = test_case['target_value']
        if export_target_cmd > 0:
            export_target_cmd += 100
        elif export_target_cmd < 0:
            export_target_cmd -= 100

        slew_rate = test_case['slew_rate']
        expected_slew_milliseconds = abs(target_value / slew_rate * 1000)
        # account for setpoint rounding check in active power at end of test
        expected_slew_milliseconds += calculate_abs_offset_power_from_rounded_setpoint(target_value) / slew_rate * 1000
        
        margin_of_error_milliseconds = base_precision + variable_precision_time_coefficient * expected_slew_milliseconds

        # Start in Site Export Target and Reactive Power Setpoint mode with 0 active and reactive power
        fims_set('/features/active_power/runmode1_kW_mode_cmd', 2)
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', 1000_000_000)
        fims_set('/features/active_power/active_power_setpoint_kW_cmd', 0)
        fims_set('/features/reactive_power/runmode1_kVAR_mode_cmd', 2)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', 1000_000_000)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', 0)
        # wait for power setpoints to settle down
        for uri in ['/assets/ess/ess_1/active_power_setpoint',   '/assets/ess/ess_2/active_power_setpoint',
                    '/assets/ess/ess_1/reactive_power_setpoint', '/assets/ess/ess_2/reactive_power_setpoint']:
            assert poll_until_uri_is_at_value(uri=uri, expected=0, tolerance=0.00001, timeout_seconds=1) == True
        
        # reset export target slew rate to 0 and wait for slew to update
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', 0)
        time.sleep(0.1)

        # Launch fims_trigger commands to time how long it takes for the active power setpoint to slew to its target value.
        # Ending value is active power trigger divided by 2 since it is divided across two ESSs
        if target_value > 0:
            target_comparison_trigger = '-g'
        if target_value < 0:
            target_comparison_trigger = '-l'
        fims_trigger = subprocess.Popen(['fims_trigger', '-s', '/features/active_power/active_power_setpoint_kW_cmd', '-e', str(export_target_cmd), '-s', '/components/ess_twins/active_power_setpoint', target_comparison_trigger, str(target_value)],
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        # Set the slew rate
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', slew_rate)

        # Send a command to kick-off the slew
        fims_set('/features/active_power/active_power_setpoint_kW_cmd', export_target_cmd)

        # Wait for fims_trigger commands to terminate.
        # Set its timeout to be 5 seconds more than the longest expected slew time or 5 seconds for very quick test cases
        trigger_timeout = max(5 + (expected_slew_milliseconds + margin_of_error_milliseconds)/1000, 5)
        fims_trigger.wait(timeout=trigger_timeout)
        
        # Parse the fims_trigger outputs for the test result
        milliseconds_between_command_and_response = parse_time_from_fims_trigger_output(fims_trigger.communicate()[0])

        # Verify the difference between the actual slew time and the expected slew time is within the margin of error.
        def check_result(expected: float, actual: float, tolerance: float) -> None:
            LOGGER.info("\n".join([
                f"Active power setpoint slew test: slew rate: {test_case['slew_rate']},"
                f"actual: {actual},"
                f"expected: {expected},"
                f"error: {actual-expected},"
                f"tolerance: {tolerance}"
            ]))
            assert abs(actual - expected) < tolerance
        check_result(expected_slew_milliseconds, milliseconds_between_command_and_response, margin_of_error_milliseconds)


def test_competing_asset_feature_slews(setup_cleanup_test_slew_rate):
    """
        Tests the case of asset slews competing with the top level setpoint slews, such that the two slew rates work in opposition and
        settle at a point different than the setpoint under certain conditions.
    """
    test_competing_asset_cases = [
        {
            "setpoint":     4_000,  # charge 4MW
            "slew_rate":    300_000  # slew small enough to expose bug
        },
        {
            "setpoint":     -4_000, # discharge 4MW
            "slew_rate":    300_000  # slew small enough to expose bug
        }
    ]
    for test_case in test_competing_asset_cases:

        # Start in Active Power Setpoint with 0 active and reactive power
        # Also turn on ess, gen, and solar
        fims_set('/features/active_power/runmode1_kW_mode_cmd', 2)
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', 1000_000_000)
        fims_set('/features/active_power/active_power_setpoint_kW_cmd', 0)
        fims_set('/features/reactive_power/runmode1_kVAR_mode_cmd', 2)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', 1000_000_000)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', 0)
        toggle_solar_and_gen_maintenance_mode(False)
        fims_set("/assets/ess/ess_1/maint_mode", False)

        # wait for power setpoints to settle down
        for uri in ['/assets/ess/ess_1/active_power_setpoint',   '/assets/ess/ess_2/active_power_setpoint',
                    '/assets/ess/ess_1/reactive_power_setpoint', '/assets/ess/ess_2/reactive_power_setpoint']:
            assert poll_until_uri_is_at_value(uri=uri, expected=0, tolerance=0.00001, timeout_seconds=1) == True
                
        # reset export target slew rate to 0 and wait for slew to update
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', 0)
        time.sleep(0.1)

        # Set the slew rate
        fims_set('/features/active_power/active_power_setpoint_kW_slew_rate', test_case["slew_rate"])

        # Send a command to kick-off the slew
        fims_set('/features/active_power/active_power_setpoint_kW_cmd', test_case["setpoint"])

        time.sleep(0.1) # This is more than enough time for it to reach the setpoint.

        # Find average over 10 iterations
        responses = []
        for i in range(0, 10):
            response = fims_get("/features/active_power/site_kW_demand")["value"]
            responses.append(response)
            time.sleep(.2)
        
        avg = statistics.mean(responses)
        stdev = statistics.stdev(responses)

        def check_result(expected: float, actual: float, tolerance: float) -> None:
            LOGGER.info("\n".join([
                f"Active power setpoint slew test: slew rate: {test_case['slew_rate']},"
                f"actual: {actual},"
                f"expected: {expected},"
                f"error: {actual-expected},"
                f"tolerance: {tolerance}"
            ]))
            assert abs(actual - expected) < tolerance
        check_result(test_case["setpoint"], avg, 1)
        check_result(0, stdev, .1)
