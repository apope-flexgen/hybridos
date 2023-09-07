import pytest

# Site Manager Pytests
from pytest_cases import parametrize


from pytests.pytest_framework import Site_Controller_Instance
from pytests.pytest_steps import Steps
from pytests.cases.active_clc import test_fr_clc, test_ess_tsoc_clc, test_solar_tsoc_clc
from pytests.cases.reactive_clc import test_reactive_setpoint_clc
from pytests.cases.reactive_power_poi_lims import test_reactive_poi_lims
from pytests.cases.constant_power_factor import test_constant_power_factor
from pytests.cases.active_power_poi_lims import test_fr_poi_lims, test_ess_tsoc_poi_lims
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


# Test runner AKA main() for each individual test
@ parametrize("current_test", [
    test_reactive_poi_lims,
    test_fr_poi_lims,
    test_ess_tsoc_poi_lims,
    test_constant_power_factor,
    test_reactive_power_setpoint,
    test_fr_clc,
    test_ess_tsoc_clc,
    test_solar_tsoc_clc,
    test_agt_runmode1,
    test_reactive_setpoint_clc,
    test_ess_chargeable_derate,
    test_ess_dischargeable_derate,
    test_maint_active_power_rounding,
    test_maint_reactive_power_rounding,
    test_alerts,
    test_ldss,
    test_init,
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
    test_persistent_autobalancing
])
def test_site_manager(request: pytest.FixtureRequest, current_test: Steps):
    # Extract the pytest id
    Site_Controller_Instance.get_instance()  # Lazy initialization
    current_id = request.node.name[request.node.name.find("[")+1:request.node.name.find("]")]
    current_test.run_steps(current_id)
