import pytest
import subprocess
import time

from .controls import start_site, toggle_solar_and_gen_maintenance_mode
from .fims import fims_set, fims_get, poll_until_uri_is_at_value, parse_time_from_fims_trigger_output

import logging

LOGGER = logging.getLogger(__name__)

@pytest.fixture(scope="function")
def cleanup_test_reactive_setpoint_slew_rate():
    """
        Sets up and cleans up the Reactive Setpoint Slew Rate test.
    """
    # Collect values of settings before test edits them so that cleanup can reset them
    runmode1_active_power_feature = fims_get('/features/active_power/runmode1_kW_mode_cmd')['value']
    export_target_kw_cmd = fims_get('/features/active_power/export_target_kW_cmd')['value']
    runmode1_reactive_power_feature = fims_get('/features/reactive_power/runmode1_kVAR_mode_cmd')['value']
    reactive_setpoint_kvar_cmd = fims_get('/features/reactive_power/reactive_setpoint_kVAR_cmd')['value']
    reactive_setpoint_slew_rate = fims_get('/features/reactive_power/reactive_setpoint_kVAR_slew_rate')['value']
    ess_kvar_slew_rate = fims_get('/dbi/site_controller/variables/variables/site/configuration/ess_kVAR_slew_rate/value')
    ess_1_throttle_timeout_fast_ms = fims_get('/dbi/site_controller/assets/assets/ess/asset_instances/0/throttle_timeout_fast_ms')
    ess_2_throttle_timeout_fast_ms = fims_get('/dbi/site_controller/assets/assets/ess/asset_instances/1/throttle_timeout_fast_ms')

    yield

    # Cleanup
    fims_set('/dbi/site_controller/variables/variables/site/configuration/ess_kVAR_slew_rate/value', ess_kvar_slew_rate)
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/0/throttle_timeout_fast_ms', ess_1_throttle_timeout_fast_ms)
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/1/throttle_timeout_fast_ms', ess_2_throttle_timeout_fast_ms)
    fims_set('/features/active_power/runmode1_kW_mode_cmd', runmode1_active_power_feature)
    fims_set('/features/active_power/export_target_kW_cmd', export_target_kw_cmd)
    fims_set('/features/reactive_power/runmode1_kVAR_mode_cmd', runmode1_reactive_power_feature)
    fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', reactive_setpoint_kvar_cmd)
    fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', reactive_setpoint_slew_rate)
    toggle_solar_and_gen_maintenance_mode(False)
    # Kill site_controller so that MCP restarts it and it reloads its configuration from DBI
    subprocess.run('pkill site_controller', shell=True)


def test_reactive_setpoint_slew_rate(cleanup_test_reactive_setpoint_slew_rate):
    """
        Tests Reactive Power Setpoint feature's slew rate variable, reactive_setpoint_kVAR_slew_rate.
        Ensures it can be edited via FIMS, the new slew takes effect immediately, and the slew is
        followed.

        NOTE: Since this test is measuring command changes across time, it will take significantly
        longer than other tests to run. To get an estimate of total runtime, add up all "expected time"s
        found in each test case.
    """
    # Test precision constants
    # upper bound ms inaccuracy caused by 10 ms site state update rate (allow for an update at the beginning and an update at the end of each test)
    site_state_update_precision = 20
    # upper bound ms inaccuracy caused by configured ess throttle_timeout_fast_ms
    throttle_timeout_fast_precision = 0
    # upper bound total ms inaccuracy (not counting variable inaccuracy specific to a test case)
    base_precision = site_state_update_precision + throttle_timeout_fast_precision

    # kvar offset to reactive power at test end caused by ESS reactive setpoint round check which prevents sets except at rounded integer changes
    # i.e. if increasing steadily we'd have sets at approximately 0.5 kvar, 1.5 kvar, 2.5 kvar, etc.
    offset_power_from_rounded_setpoint = 1

    # there is still some degree of error unaccounted for, which is only significant at long timescales (> 1 minute, see the trials below).
    # This coefficient attempts to account for that error by adding imprecision relative to test time.
    # (Speculation: the source of this error might be rounding in slew calculations and update timing)
    # The following trials had throttle_timeout_fast_precision set to 0
    # Trials: With timer end at 300 and slew rate 1 (~5 minute test), saw errors of up to 50 ms
    # Trials: With timer end at 600 and slew rate 1 (~10 minute test), saw errors of -812 ms (I think this datapoint might have been due to ESS setpoint round check?), 119 ms, 120 ms, 118 ms
    # Trials: With timer end at 900 and slew rate 1 (~15 minute test), saw errors of 314 ms, 299 ms
    variable_precision_time_coefficient = 0.0005 # = 0.05% chosen based on trials

    # TODO: I still have sometimes (though very rarely) seen the slew time be outside of tolerance.
    # i.e. with timer end at 600 and slew rate 1, I once saw an error of -812 ms. With the error so close to 1 second, I feel it's likely that the setpoint round check may be related.
    # It may be worth looking into this error further, but I cannot reliably recreate it.

    # Start by setting the ESS kVAr slew rate value, which can only be set at start-up, to a number larger than
    # any of the slew values used for the reactive setpoint slew in tests so that the ESS kVAr slew rate does
    # not get in the way of the reactive setpoint slew tests by limiting more aggressively than the tested value
    fims_set('/dbi/site_controller/variables/variables/site/configuration/ess_kVAR_slew_rate/value', 2500_000_000)
    # Also set the ESS throttle_timeout_fast_ms to the desired number to obtain the given precision on ESS setpoint set times
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/0/throttle_timeout_fast_ms', throttle_timeout_fast_precision)
    fims_set('/dbi/site_controller/assets/assets/ess/asset_instances/1/throttle_timeout_fast_ms', throttle_timeout_fast_precision)
    # Kill site_controller so that MCP restarts it and it reloads its configuration from DBI
    subprocess.run('pkill site_controller', shell=True)
    # Give site_controller a couple seconds to start back up
    time.sleep(2)
    # Start the site then put all non-ESS in maintenance mode and wait for power setpoints to settle down
    start_site()
    toggle_solar_and_gen_maintenance_mode(True)

    for test_case in [
            {
                'reactive_power_at_timer_end': 10_000,
                'reactive_setpoint_slew_rate': 1000_000_000, # will cause expected slew time to hit floor of 10ms
                # expected time: < 10 ms AKA within site_controller state machine cycle
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 300_000,
                # expected time: 10.003 ms (slightly more than a state machine cycle)
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 60_000,
                # expected time: 50.016 ms
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 30_000,
                # expected time: 100.03 ms
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 15_000,
                # expected time: 200.06 ms
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 3000,
                # expected time: 1 s, 0.3 ms
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 1000,
                # expected time: 3 s, 1 ms
            },
            {
                'reactive_power_at_timer_end': 5,
                'reactive_setpoint_slew_rate': 1,
                # expected time: 5 s
            },
            {
                'reactive_power_at_timer_end': 6,
                'reactive_setpoint_slew_rate': 1,
                # expected time: 7 s
            },
            {
                'reactive_power_at_timer_end': 3000,
                'reactive_setpoint_slew_rate': 500,
                # expected time: 6 s, 2 ms
            },
            {
                'reactive_power_at_timer_end': 6000,
                'reactive_setpoint_slew_rate': 1000,
                # expected time: 6 s, 1 ms
            },
            {
                'reactive_power_at_timer_end': 60,
                'reactive_setpoint_slew_rate': 1,
                # expected time: 61 s
            },
            {
                'reactive_power_at_timer_end': 6000,
                'reactive_setpoint_slew_rate': 100,
                # expected time: 60 s, 10 ms
            },
            # # Test cases that take longer than a minute are below
            # {
            #     'reactive_power_at_timer_end': 300,
            #     'reactive_setpoint_slew_rate': 1,
            #     # expected time: 5 min, 1 s
            # },
            # {
            #     'reactive_power_at_timer_end': 3000,
            #     'reactive_setpoint_slew_rate': 10,
            #     # expected time: 5 min, 100 ms
            # },
            # {
            #     'reactive_power_at_timer_end': 600,
            #     'reactive_setpoint_slew_rate': 1,
            #     # expected time: 10 min, 1 s
            # },
            # {
            #     'reactive_power_at_timer_end': 900,
            #     'reactive_setpoint_slew_rate': 1,
            #     # expected time: 15 min, 1 s
            # },
    ]:
        reactive_power_at_timer_end = test_case['reactive_power_at_timer_end']
        # Make sure actual command sent is greater than the target so we can ensure the target is reached and passed
        reactive_setpoint_cmd = test_case['reactive_power_at_timer_end'] + 10 # add 10 just to be safe, a smaller amount would probably be fine

        reactive_setpoint_slew_rate = test_case['reactive_setpoint_slew_rate']
        expected_slew_milliseconds = reactive_power_at_timer_end / reactive_setpoint_slew_rate * 1000
        # account for setpoint rounding check in reactive power at end of test if the target reactive power split between each asset
        # is not halfway between two integers (when total target reactive power is even).
        # Note that this calculation assumes that reactive_power_at_timer_end is an integer
        if reactive_power_at_timer_end % 2 == 0:
            expected_slew_milliseconds += offset_power_from_rounded_setpoint / reactive_setpoint_slew_rate * 1000
        
        margin_of_error_milliseconds = base_precision + variable_precision_time_coefficient * expected_slew_milliseconds

        # Start in Site Export Target with command of 0 so active power does not interfere with reactive power testing
        fims_set('/features/active_power/runmode1_kW_mode_cmd', 2)
        fims_set('/features/active_power/export_target_kW_cmd', 0)
        # Start from a zero reactive power state in Reactive Power Setpoint mode
        fims_set('/features/reactive_power/runmode1_kVAR_mode_cmd', 2)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', 1000_000_000)
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', 0)
        # wait for power setpoints to settle down
        for uri in ['/assets/ess/ess_1/active_power_setpoint',   '/assets/ess/ess_2/active_power_setpoint',
                    '/assets/ess/ess_1/reactive_power_setpoint', '/assets/ess/ess_2/reactive_power_setpoint']:
            assert poll_until_uri_is_at_value(uri=uri, expected=0, tolerance=1, timeout_seconds=30) == True
        
        # reset reactive power slew rate to 0 and wait for slew to update
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', 0)
        time.sleep(0.1)

        # Launch fims_trigger commands to time how long it takes for the reactive power setpoint to slew to its target value.
        # Ending value is reactive power trigger divided by 2 since it is divided across two ESSs
        fims_trigger_1 = subprocess.Popen(['fims_trigger', '-s', '/features/reactive_power/reactive_setpoint_kVAR_cmd', '-e', str(reactive_setpoint_cmd), '-s', '/components/ess_twins/reactive_power_setpoint', '-g', str(reactive_power_at_timer_end/2)],
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)        
        fims_trigger_2 = subprocess.Popen(['fims_trigger', '-s', '/features/reactive_power/reactive_setpoint_kVAR_cmd', '-e', str(reactive_setpoint_cmd), '-s', '/components/ess_real_hs/reactive_power_setpoint', '-g', str(reactive_power_at_timer_end/2)],
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        # Set the slew rate
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_slew_rate', reactive_setpoint_slew_rate)

        # Send a command to kick-off the slew
        fims_set('/features/reactive_power/reactive_setpoint_kVAR_cmd', reactive_setpoint_cmd)

        # Wait for fims_trigger commands to terminate.
        # Set its timeout to be double the expected slew time (divide milliseconds by 500 since timeout is given in seconds) or 1 second for very quick test cases
        trigger_timeout = max(expected_slew_milliseconds/500, 1)
        fims_trigger_1.wait(timeout=trigger_timeout)
        fims_trigger_2.wait(timeout=trigger_timeout)
        
        # Parse the fims_trigger outputs for the test result
        milliseconds_between_command_and_response_1 = parse_time_from_fims_trigger_output(fims_trigger_1.communicate()[0])
        milliseconds_between_command_and_response_2 = parse_time_from_fims_trigger_output(fims_trigger_2.communicate()[0])

        # Verify the difference between the actual slew time and the expected slew time is within the margin of error.
        # If the expected time was less than one state machine cycle, use maximum check instead of margin of error
        def check_result(expected: float, actual: float, tolerance: float) -> None:
            LOGGER.info(f"Reactive slew test: slew rate:{test_case['reactive_setpoint_slew_rate']}, actual:{actual}, expected:{expected}, error:{actual-expected}, tolerance:{tolerance}")
            assert abs(actual - expected) < tolerance
        check_result(expected_slew_milliseconds, milliseconds_between_command_and_response_1, margin_of_error_milliseconds)
        check_result(expected_slew_milliseconds, milliseconds_between_command_and_response_2, margin_of_error_milliseconds)