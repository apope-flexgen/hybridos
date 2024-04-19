import pytest

# Site Manager Pytests
from pytest_cases import parametrize

from pytests.pytest_framework import Site_Controller_Instance
from pytests.pytest_steps import Steps
from pytests.cases.active_clc import test_fr_clc, test_ess_tsoc_clc, test_solar_tsoc_clc, test_active_clc_zero_bypass, test_slow_clc_update_rate, test_active_clc_poi_lims
from pytests.cases.reactive_clc import test_reactive_setpoint_clc, test_reactive_clc_zero_bypass
from pytests.cases.reactive_power_poi_lims import test_reactive_poi_lims
from pytests.cases.constant_power_factor import test_constant_power_factor
from pytests.cases.active_power_poi_lims import test_fr_poi_lims, test_ess_tsoc_poi_lims, test_active_power_setpoint_ess_solar_poi_lims
from pytests.cases.reactive_setpoint import test_reactive_power_setpoint
from pytests.cases.site_state import test_site_state
from pytests.cases.sequences import test_init, test_watchdog_fault, test_num_ess_transitions, test_auto_restart_prevention, test_auto_restart_prevention_agt, test_agt_sequences
from pytests.cases.alerts import test_alerts
from pytests.cases.ldss import test_ldss
from pytests.cases.active_power import test_ess_chargeable_derate, test_ess_dischargeable_derate, test_maint_active_power_rounding, test_maint_reactive_power_rounding
from pytests.cases.agt_runmode1 import test_agt_runmode1
from pytests.cases.manual_mode import test_manual_solar_slew_rate_1, test_manual_solar_slew_rate_2, test_manual_ess_slew_rate_1, test_manual_ess_slew_rate_2, test_manual_ess_slew_rate_3, test_manual_gen_slew_rate_1, test_manual_gen_slew_rate_2, test_manual_gen_slew_rate_3
from pytests.cases.templating_revamp import test_ranged, test_traditional
from pytests.cases.persistent_settings import test_persistent_contactors, test_persistent_autobalancing, test_persistent_setpoint
from pytests.cases.assets_state import test_default_local_mode, test_asset_bit_field_local_mode, test_local_bit_field_local_mode, test_bit_field_status_override, test_random_enum_status_override
from pytests.cases.avr import test_avr_overvoltage_symmetric, test_avr_undervoltage_symmetric, test_avr_overvoltage_asymmetric, test_avr_undervoltage_asymmetric, test_avr_positive_poi_limits, test_avr_negative_poi_limits, test_avr_voltage_setpoint_limits, test_avr_overvoltage_slew, test_avr_undervoltage_slew
from pytests.cases.standalone_fr import test_pfr_untracked_load, test_pfr_offset_load, test_pfr_minimum_load, test_pfr_untracked_load_poi_lim, test_pfr_offset_load_poi_lim, test_pfr_minimum_load_poi_lim, test_pfr_asymmetric_configs, test_pfr_force_start, test_pfr_force_start_pulse, test_pfr_force_start_graph_1, test_pfr_force_start_graph_2, test_pfr_asymmetric_slew
from pytests.cases.maint_mode import test_min_charge_discharge, test_maint_soc_limits, test_maint_cell_volt_limits, test_maint_rack_volt_limits
from pytests.cases.ess_calibration import test_ess_cali
from pytests.cases.enable_flags import test_enable_flags
from pytests.cases.automated_actions import test_completed_automated_action, test_aborted_automated_action, test_silent_automated_action, test_path_switch_automated_action, test_failed_automated_action, test_alarm_automated_action, test_maint_mode_early_exit, test_scheduler_balancing
from pytests.cases.soc_balancing_algorithm import test_battery_balancing_algorithm
from pytests.cases.fims_api import test_fr_multiple_inputs, test_grid_mode_doesnt_spam

# Test runner AKA main() for each individual test
@ parametrize("current_test", [

    #
    # config_dev
    #
    test_active_clc_zero_bypass,
    test_agt_runmode1,
    test_reactive_setpoint_clc,
    test_reactive_clc_zero_bypass,
    test_ess_chargeable_derate,
    test_ess_dischargeable_derate,
    test_maint_active_power_rounding,
    test_maint_reactive_power_rounding,
    test_alerts,
    test_default_local_mode,
    test_asset_bit_field_local_mode,
    test_local_bit_field_local_mode,
    test_bit_field_status_override,
    test_random_enum_status_override,
    test_ldss,
    test_init,
    test_watchdog_fault,
    test_num_ess_transitions,
    test_auto_restart_prevention,
    test_auto_restart_prevention_agt,
    test_agt_sequences,
    test_site_state,
    test_manual_solar_slew_rate_1,
    test_manual_solar_slew_rate_2,
    test_manual_ess_slew_rate_1,
    test_manual_ess_slew_rate_2,
    test_manual_ess_slew_rate_3,
    test_manual_gen_slew_rate_1,
    test_manual_gen_slew_rate_2,
    test_manual_gen_slew_rate_3,
    test_ranged,
    test_traditional,
    test_persistent_setpoint,
    test_persistent_contactors,
    test_persistent_autobalancing,
    test_avr_overvoltage_symmetric,
    test_avr_undervoltage_symmetric,
    test_avr_overvoltage_asymmetric,
    test_avr_undervoltage_asymmetric,
    test_avr_overvoltage_slew,
    test_avr_undervoltage_slew,
    test_avr_positive_poi_limits,
    test_avr_negative_poi_limits,
    test_avr_voltage_setpoint_limits, 
    test_pfr_untracked_load,
    test_pfr_offset_load,
    test_pfr_minimum_load,
    test_pfr_untracked_load_poi_lim,
    test_pfr_offset_load_poi_lim,
    test_pfr_minimum_load_poi_lim,
    test_pfr_asymmetric_configs,
    test_pfr_force_start,
    test_pfr_force_start_pulse,
    test_pfr_force_start_graph_1,
    test_pfr_force_start_graph_2,
    test_pfr_asymmetric_slew,
    test_active_power_setpoint_ess_solar_poi_lims,
    test_min_charge_discharge,
    test_maint_soc_limits,
    test_maint_cell_volt_limits,
    test_maint_rack_volt_limits,
    test_ess_cali,
    test_enable_flags,
    test_fr_poi_lims,
    test_fr_clc,
    test_slow_clc_update_rate,
    test_active_clc_poi_lims,
    test_completed_automated_action,
    test_aborted_automated_action,
    test_silent_automated_action,
    test_path_switch_automated_action,
    test_failed_automated_action,
    test_alarm_automated_action,
    test_maint_mode_early_exit,
    test_fr_multiple_inputs,
    test_scheduler_balancing,
    test_battery_balancing_algorithm,
    test_grid_mode_doesnt_spam
    #
    # config_dev_slow_slews
    # TODO find a way to make these tests run automatically under different configs rather than just commenting out :(
    #
    # test_active_clc_zero_bypass
    # test_fr_clc
])
def test_site_manager(request: pytest.FixtureRequest, current_test: Steps):
    # Extract the pytest id
    Site_Controller_Instance.get_instance()  # Lazy initialization
    current_id = request.node.name[request.node.name.find("[")+1:request.node.name.find("]")]
    current_test.run_steps(current_id)
