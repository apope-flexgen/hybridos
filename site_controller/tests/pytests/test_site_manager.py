import pytest

# Site Manager Pytests
from pytest_cases import parametrize

from .pytest_framework import Site_Controller_Instance
from .pytest_steps import Steps
from .cases.active_clc import test_fr_clc, test_ess_tsoc_clc, test_solar_tsoc_clc
from .cases.reactive_clc import test_reactive_setpoint_clc
from .cases.reactive_power_poi_lims import test_reactive_poi_lims
from .cases.constant_power_factor import test_constant_power_factor
from .cases.active_power_poi_lims import test_fr_poi_lims, test_ess_tsoc_poi_lims
from .cases.reactive_setpoint import test_reactive_power_setpoint
from .cases.sequences import test_num_ess_transitions
from .cases.alerts import test_alerts


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
    test_reactive_setpoint_clc,
    test_num_ess_transitions,
    test_alerts,
])
def test_site_manager(request: pytest.FixtureRequest, current_test: Steps):
    # Extract the pytest id
    Site_Controller_Instance.get_instance() # Lazy initialization
    current_id = request.node.name[request.node.name.find("[")+1:request.node.name.find("]")]
    current_test.run_steps(current_id)
