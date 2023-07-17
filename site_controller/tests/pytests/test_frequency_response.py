import pytest
import subprocess

from .controls import start_site, toggle_solar_and_gen_maintenance_mode, run_twins_command
from .fims import fims_set, fims_get, poll_until_uri_is_at_value, parse_body_from_fims_listen_output, parse_time_from_fims_trigger_output


@pytest.fixture(scope="function")
def setup_cleanup_test_fast_frequency_response():
    """
        Sets up and cleans up the Fast Frequency Response test.
    """
    # Collect values of settings before test edits them so that cleanup can reset them
    grid_frequency = run_twins_command('fims_send -m get -u /components/grid/fcmd -r /$$')
    runmode1_active_power_feature = fims_get('/features/active_power/runmode1_kW_mode_cmd')['value']
    frequency_response_component_enable_mask = fims_get('/features/active_power/fr_enable_mask')['value']
    uf_ffr_active_cmd_kw = fims_get('/features/active_power/uf_ffr_active_cmd_kw')['value']

    # Setup: start the site in Frequency Response mode with only the FFR response component active
    # with a command equal to the sum of all ESS rated active powers. Grid @ 60 Hz and all non-ESS
    # assets in maintenance mode. Let ESSs settle to 0kW in case they were previously non-zero
    assert run_twins_command('fims_send -m set -u /components/grid/fcmd 60 -r /$$') == 60
    fims_set('/features/active_power/runmode1_kW_mode_cmd', 5)
    fims_set('/features/active_power/fr_enable_mask', 4)
    fims_set('/features/active_power/uf_ffr_active_cmd_kw', 11000)
    start_site()
    toggle_solar_and_gen_maintenance_mode(True)
    poll_until_uri_is_at_value('/assets/ess/ess_1/active_power_setpoint', expected=0, tolerance=1)
    poll_until_uri_is_at_value('/assets/ess/ess_2/active_power_setpoint', expected=0, tolerance=1)

    yield

    # Cleanup
    run_twins_command(f'fims_send -m set -u /components/grid/fcmd {grid_frequency} -r /$$')
    toggle_solar_and_gen_maintenance_mode(False)
    fims_set('/features/active_power/runmode1_kW_mode_cmd', runmode1_active_power_feature)
    fims_set('/features/active_power/fr_enable_mask', frequency_response_component_enable_mask)
    fims_set('/features/active_power/uf_ffr_active_cmd_kw', uf_ffr_active_cmd_kw)


def test_fast_frequency_response(setup_cleanup_test_fast_frequency_response):
    """
        Tests the Fast Frequency Response (FFR) component of the Frequency Response active power feature.
        When frequency detected falls below FFR threshold frequency, site_controller should immediately
        set ESSs to output full FFR active command. The very first SET to be sent to each ESS should be
        for the full command value since FFR should bypass asset slew rates. The SETs should be sent in
        a single site_controller state machine cycle.
    """
    # Launch fims_trigger commands to time how long it takes for each active power setpoint to be sent to the ESSs
    fims_trigger_1 = subprocess.Popen(['fims_trigger', '-p', '/components/shared_poi/frequency', '-e', '58', '-s', '/components/ess_twins/active_power_setpoint', '-e', '5500'],
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    fims_trigger_2 = subprocess.Popen(['fims_trigger', '-p', '/components/shared_poi/frequency', '-e', '58', '-s', '/components/ess_real_hs/active_power_setpoint', '-e', '5500'],
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

    # Launch fims_listen commands to listen for the first active power setpoint sent to each of the two ESSs
    fims_listen_1 = subprocess.Popen(['fims_listen', '-m', 'set', '-u', '/components/ess_twins/active_power_setpoint', '-n', '1'],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    fims_listen_2 = subprocess.Popen(['fims_listen', '-m', 'set', '-u', '/components/ess_real_hs/active_power_setpoint', '-n', '1'],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

    # Begin the frequency deviation event
    assert run_twins_command('fims_send -m set -u /components/grid/fcmd 58 -r /$$') == 58

    # Wait for fims_listen and fims_trigger commands to terminate
    fims_trigger_1.wait()
    fims_trigger_2.wait()
    fims_listen_1.wait()
    fims_listen_2.wait()

    # setpoints should be sent within 20 milliseconds
    milliseconds_between_freq_detection_and_response_1 = parse_time_from_fims_trigger_output(fims_trigger_1.communicate()[0])
    assert milliseconds_between_freq_detection_and_response_1 < 20
    milliseconds_between_freq_detection_and_response_2 = parse_time_from_fims_trigger_output(fims_trigger_2.communicate()[0])
    assert milliseconds_between_freq_detection_and_response_2 < 20

    # Value sent to each ESS should be half of the full FFR active command since there are two ESSs
    kw_value_sent_to_ess_1 = parse_body_from_fims_listen_output(fims_listen_1.communicate()[0])['value']
    assert kw_value_sent_to_ess_1 == 5500
    kw_value_sent_to_ess_2 = parse_body_from_fims_listen_output(fims_listen_2.communicate()[0])['value']
    assert kw_value_sent_to_ess_2 == 5500
