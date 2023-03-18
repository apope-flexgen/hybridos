from copy import copy
import logging
import time
import pytest
import sys

sys.path.append("/home/pyexecnetcache/pytests")  # noqa
from pytest_cases import parametrize, fixture
from assertion_framework import Flex_Assertion, Assertion_Type, Tolerance_Type
from test_framework import launch_services, create_pyfims_instance, start_site, get_asset_agg, cleanup, update_variables

site_delay = 5  # How long it takes for site to update/fully slew in the worst case
demand_assertion = None
ess_assertion = None
feed_assertion = None
gen_assertion = None
solar_assertion = None
load_responsibility = 0
# TODO: worth extracting these from configuration?
ess_rated_power = 11000
solar_rated_power = 5000


# Class encapsulating data for a active power or standalone feature
class Feature():
    # enable: uri, value pair used to enable the feature over fims
    # inputs: uri, value pair inputs to set up the feature
    # feature_func: the function encapsulating the feature's behavior
    # expected_demand: the expected site demand modification of the feature
    #                  For active power features this is the test assertion object
    #                  For standalone features this is simply a demand modification value added to/overwriting the test assertion value
    # expected_ess_cmd: same function as expected_demand but for ess
    # expected_solar_cmd: same function as expected_demand but for solar
    def __init__(self, enable, inputs, feature_func, expected_demand=None, expected_ess_cmd=None, expected_solar_cmd=None):
        self.fims_enable = enable
        self.fims_inputs = inputs
        self.func = feature_func
        self.expected_demand = expected_demand
        self.expected_ess_cmd = expected_ess_cmd
        self.expected_solar_cmd = expected_solar_cmd

    def enable(self):
        for uri, value in self.fims_enable.items():
            pfims.SendSet(uri, value)

    def setup(self):
        self.enable()
        for uri, value in self.fims_inputs.items():
            pfims.SendSet(uri, value)

    def run_feature(self):
        self.func(self)


# Child class encapsulating additional standalone feature behavior
class Standalone_Feature(Feature):

    def disable(self):
        uri = next(iter(self.fims_enable))
        pfims.SendSet(uri, False)


# Modify config values once before testing ess pipeline
@ fixture(scope="session")
def ess_configuration():
    asset_changes = {
        "/assets/generators": "delete", "/assets/solar": "delete"
    }
    sequence_changes = {
        "/sequences/Startup/paths/steps,return_id=RunMode1/entry_actions,step_name=Start Solar": "delete",
        "/sequences/Startup/paths/steps,return_id=RunMode1/entry_actions,step_name=Start Gen": "delete"
    }
    variables_changes = {
        "/variables/features/active_power/available_features_runmode1_kW_mode/value": "0xFFF",
        "/variables/internal/available_features_standalone_power/value": "0xFFF",
        "/variables/features/active_power/site_kW_load_interval_ms": 10000
    }
    twins_changes = {
        "/loads/pramp,id=bess_aux_load": 10000,
        "/loads/pcmd,id=bess_aux_load": 0,
        "/ess/soc,id=twins_ess_1": 45,
        "/ess/soc,id=twins_ess_2": 55,
        "/ess/cap,id=twins_ess_1": 999999,  # Give large capacity so the long duration of the full test bench
        "/ess/cap,id=twins_ess_2": 999999   # does not cause the SoC to climb to 100%
    }

    update_variables("/usr/local/etc/config/site_controller/assets.json", asset_changes)
    update_variables("/usr/local/etc/config/site_controller/sequences.json", sequence_changes)
    update_variables("/usr/local/etc/config/site_controller/variables.json", variables_changes)
    update_variables("/usr/local/etc/config/twins/twins/twins.json", twins_changes)

    launch_services()
    global pfims
    pfims = create_pyfims_instance()
    start_site(pfims)

    yield pfims
    cleanup()


# Modify config values once before testing ess pipeline
@ fixture(scope="session")
def ess_solar_configuration():
    asset_changes = {
        "/assets/generators": "delete"
    }
    sequence_changes = {
        "/sequences/Startup/paths/steps,return_id=RunMode1/entry_actions,step_name=Start Gen": "delete"
    }
    variables_changes = {
        "/variables/features/active_power/available_features_runmode1_kW_mode/value": "0xFFF",
        "/variables/internal/available_features_standalone_power/value": "0xFFF",
        "/variables/features/active_power/site_kW_load_interval_ms": 10000
    }
    twins_changes = {
        "/loads/pramp,id=bess_aux_load": 10000,
        "/loads/pcmd,id=bess_aux_load": 0,
        "/ess/soc,id=twins_ess_1": 45,
        "/ess/soc,id=twins_ess_2": 55,
        "/ess/cap,id=twins_ess_1": 999999,  # Give large capacity so the long duration of the full test bench
        "/ess/cap,id=twins_ess_2": 999999   # does not cause the SoC to climb to 100%
    }

    update_variables("/usr/local/etc/config/site_controller/assets.json", asset_changes)
    update_variables("/usr/local/etc/config/site_controller/sequences.json", sequence_changes)
    update_variables("/usr/local/etc/config/site_controller/variables.json", variables_changes)
    update_variables("/usr/local/etc/config/twins/twins/twins.json", twins_changes)

    launch_services()
    global pfims
    pfims = create_pyfims_instance()
    start_site(pfims)

    yield pfims
    cleanup()


# Energy Arb test for all pipelines
def energy_arb(energy_arb_feature):
    # Set inputs
    energy_arb_feature.setup()
    # TODO: some kind of dynamic slew based on distance from reference? Could save some time
    time.sleep(site_delay + (site_delay * 2 * -energy_arb_feature.fims_inputs["/components/bess_aux/active_power_setpoint"]))
    solar_running = pfims.SendGet("/assets/solar/summary/num_solar_running", "/pytest")[1]
    solar_potential = 0
    if solar_running > 0:
        solar_potential = pfims.SendGet("/features/active_power/max_potential_solar_kW", "/pytest")[1]["value"]

    global ess_assertion
    global solar_assertion
    global demand_assertion

    # Set expected outputs
    actual_avg_soc = pfims.SendGet("/features/active_power/soc_avg_running", "/pytest")[1]["value"]
    logging.error("Arbitrage price %s", energy_arb_feature.fims_inputs["/features/active_power/price"])
    logging.error("Soc avg running %s", actual_avg_soc)
    logging.error("asset priority: %s", energy_arb_feature.fims_inputs["/features/active_power/asset_priority_runmode1"])
    ess_assertion = copy(energy_arb_feature.expected_ess_cmd)
    demand_assertion = copy(energy_arb_feature.expected_demand)
    # Support any asset configuration by allowing solar to be set intuitively but correcting it based on actual config
    solar_assertion = copy(energy_arb_feature.expected_solar_cmd)
    if solar_potential == 0:
        demand_assertion -= solar_assertion
        solar_assertion.overwrite_value(solar_potential)
    logging.error("Expected demand: %s", demand_assertion.value)
    logging.error("Expected ess cmd: %s", ess_assertion.value)
    logging.error("Expected solar cmd: %s", solar_assertion.value)
    load_inclusion = pfims.SendGet("/features/active_power/site_kW_load_inclusion", "/pytest")[1]["value"]
    site_load = pfims.SendGet("/features/active_power/site_kW_load", "/pytest")[1]["value"]
    logging.error("load %s included %s", site_load, load_inclusion)
    feature_demand = pfims.SendGet("/features/active_power/feature_kW_demand", "/pytest")[1]["value"]
    logging.error("got feature demand %s", feature_demand)


# Parameters for Energy Arb tests
@ fixture
@ parametrize("feature_obj", [
    # Charge2 no solar
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": -10,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/components/bess_aux/active_power_setpoint": 0,  # Load
         # Even if load is set 0 there will still be a tiny amount that must
         # be satisfied due to ess prioritized before feeder and no load flag
         # for this feature. Set priority to resolve this
         "/features/active_power/asset_priority_runmode1": 1},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -2000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -2000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge1 no solar
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": 0,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/components/bess_aux/active_power_setpoint": 0,
         "/features/active_power/asset_priority_runmode1": 1},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -1000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -1000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge1 with solar
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": 1,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/components/bess_aux/active_power_setpoint": -1000,
         "/features/active_power/asset_priority_runmode1": 1},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power - 1000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -1000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    ),
    # no ESS, solar
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": 15,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/features/active_power/soc_min_limit": 0,
         "/features/active_power/soc_max_limit": 100,
         "/components/bess_aux/active_power_setpoint": 0,
         "/features/active_power/asset_priority_runmode1": 1},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    ),
    # Discharge1, solar
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": 30,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/features/active_power/soc_min_limit": 0,
         "/features/active_power/soc_max_limit": 100,
         "/components/bess_aux/active_power_setpoint": 0,
         "/features/active_power/asset_priority_runmode1": 1},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power + 1000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 1000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    ),
    # Discharge2, solar
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": 100,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/features/active_power/soc_min_limit": 0,
         "/features/active_power/soc_max_limit": 100,
         "/components/bess_aux/active_power_setpoint": 0,
         "/features/active_power/asset_priority_runmode1": 1},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power + 2000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 2000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    ),
    # Charge2 but discharge due to load and priority
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": -10,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/features/active_power/soc_min_limit": 0,
         "/features/active_power/soc_max_limit": 100,
         "/components/bess_aux/active_power_setpoint": -1000,
         "/features/active_power/asset_priority_runmode1": 0},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power + 1000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 1000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    ),
    # Discharge2 with additional load compensation
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 0},
        {"/features/active_power/price": 100,
         "/features/active_power/threshold_charge_2": -10,
         "/features/active_power/threshold_charge_1": 10,
         "/features/active_power/threshold_dischg_1": 30,
         "/features/active_power/threshold_dischg_2": 100,
         "/features/active_power/max_charge_2": -2000,
         "/features/active_power/max_charge_1": -1000,
         "/features/active_power/max_dischg_1": 1000,
         "/features/active_power/max_dischg_2": 2000,
         "/features/active_power/soc_min_limit": 0,
         "/features/active_power/soc_max_limit": 100,
         "/components/bess_aux/active_power_setpoint": -1000,
         "/features/active_power/asset_priority_runmode1": 0},
        energy_arb,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 3000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 3000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # TODO: soc limits are not exposed to fims, only configuration
    # Customer requirement or can these be exposed to allow testing?
    # Charge2 soc >= max but no limit due to price
    # Feature(
    #     {"/features/active_power/runmode1_kW_mode_cmd": 0},
    #     {"/features/active_power/price": -10,
    #      "/features/active_power/threshold_charge_2": -10,
    #      "/features/active_power/threshold_charge_1": 10,
    #      "/features/active_power/threshold_dischg_1": 30,
    #      "/features/active_power/threshold_dischg_2": 100,
    #      "/features/active_power/max_charge_2": -2000,
    #      "/features/active_power/max_charge_1": -1000,
    #      "/features/active_power/max_dischg_1": 1000,
    #      "/features/active_power/max_dischg_2": 2000,
    #      "/features/active_power/soc_min_limit": 0,
    #      "/features/active_power/soc_max_limit": 49,
    #      "/components/bess_aux/active_power_setpoint": -1000,
    #      "/features/active_power/asset_priority_runmode1": 1},
    #     energy_arb,
    #     expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -2000),
    #     expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -2000),
    #     expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    # ),
    # # Charge1 soc >= max, limited to 0
    # Feature(
    #     {"/features/active_power/runmode1_kW_mode_cmd": 0},
    #     {"/features/active_power/price": 0,
    #      "/features/active_power/threshold_charge_2": -10,
    #      "/features/active_power/threshold_charge_1": 10,
    #      "/features/active_power/threshold_dischg_1": 30,
    #      "/features/active_power/threshold_dischg_2": 100,
    #      "/features/active_power/max_charge_2": -2000,
    #      "/features/active_power/max_charge_1": -1000,
    #      "/features/active_power/max_dischg_1": 1000,
    #      "/features/active_power/max_dischg_2": 2000,
    #      "/features/active_power/soc_min_limit": 0,
    #      # set to 49 instead of 50 due to soc scaling
    #      # (soc_avg_running tends to be about 49.46 when set to 50)
    #      "/features/active_power/soc_max_limit": 49,
    #      "/components/bess_aux/active_power_setpoint": -1000,
    #      "/features/active_power/asset_priority_runmode1": 1},
    #     energy_arb,
    #     expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 0),
    #     expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
    #     expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    # ),
    # # Discharge1 soc <= min, limited to 0
    # Feature(
    #     {"/features/active_power/runmode1_kW_mode_cmd": 0},
    #     {"/features/active_power/price": 30,
    #      "/features/active_power/threshold_charge_2": -10,
    #      "/features/active_power/threshold_charge_1": 10,
    #      "/features/active_power/threshold_dischg_1": 30,
    #      "/features/active_power/threshold_dischg_2": 100,
    #      "/features/active_power/max_charge_2": -2000,
    #      "/features/active_power/max_charge_1": -1000,
    #      "/features/active_power/max_dischg_1": 1000,
    #      "/features/active_power/max_dischg_2": 2000,
    #      "/features/active_power/soc_min_limit": 50,
    #      "/features/active_power/soc_max_limit": 100,
    #      "/components/bess_aux/active_power_setpoint": -1000,
    #      "/features/active_power/asset_priority_runmode1": 1},
    #     energy_arb,
    #     expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power),
    #     expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
    #     expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    # ),
    # # Discharge2 soc <= min but no limit due to price
    # Feature(
    #     {"/features/active_power/runmode1_kW_mode_cmd": 0},
    #     {"/features/active_power/price": 100,
    #      "/features/active_power/threshold_charge_2": -10,
    #      "/features/active_power/threshold_charge_1": 10,
    #      "/features/active_power/threshold_dischg_1": 30,
    #      "/features/active_power/threshold_dischg_2": 100,
    #      "/features/active_power/max_charge_2": -2000,
    #      "/features/active_power/max_charge_1": -1000,
    #      "/features/active_power/max_dischg_1": 1000,
    #      "/features/active_power/max_dischg_2": 2000,
    #      "/features/active_power/soc_min_limit": 50,
    #      "/features/active_power/soc_max_limit": 100,
    #      "/components/bess_aux/active_power_setpoint": -1000,
    #      "/features/active_power/asset_priority_runmode1": 1},
    #     energy_arb,
    #     expected_demand=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power + 2000),
    #     expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 2000),
    #     expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    # )
])
def energy_arb_params(feature_obj):
    return feature_obj


# Target SoC test for all pipelines
def target_soc(target_soc_feature):
    # Set inputs
    target_soc_feature.setup()

    global ess_assertion
    global solar_assertion
    global demand_assertion
    global load_responsibility
    load_responsibility = target_soc_feature.fims_inputs["/features/active_power/target_soc_load_enable_flag"]\
        * -target_soc_feature.fims_inputs["/components/bess_aux/active_power_setpoint"]
    # Load tests need significant delay to support load buffer smoothing due to delayed publishes
    time.sleep(site_delay + (site_delay * 2 * load_responsibility))
    solar_running = pfims.SendGet("/assets/solar/summary/num_solar_running", "/pytest")[1]
    solar_potential = 0
    if solar_running > 0:
        solar_potential = pfims.SendGet("/features/active_power/max_potential_solar_kW", "/pytest")[1]["value"]

    # Set expected outputs
    actual_avg_soc = pfims.SendGet("/features/active_power/soc_avg_running", "/pytest")[1]["value"]
    logging.error("Soc avg running %s", actual_avg_soc)
    ess_assertion = copy(target_soc_feature.expected_ess_cmd)
    logging.error("Target soc %s", target_soc_feature.fims_inputs["/features/active_power/ess_charge_control_target_soc"])
    logging.error("Charge disable %s discharge disable %s",
                  target_soc_feature.fims_inputs["/features/active_power/ess_charge_control_charge_disable"], target_soc_feature.fims_inputs["/features/active_power/ess_charge_control_discharge_disable"])
    solar_assertion = copy(target_soc_feature.expected_solar_cmd)
    solar_assertion.overwrite_value(solar_potential)
    demand_assertion = copy(target_soc_feature.expected_demand)
    demand_assertion.overwrite_value(ess_assertion.value)
    demand_assertion += solar_assertion.value
    logging.error("Expected demand: %s", demand_assertion.value)
    logging.error("Expected ess cmd: %s", ess_assertion.value)
    logging.error("Expected solar cmd: %s", solar_assertion.value)


# Parameters for Target SoC tests
@ fixture
@ parametrize("feature_obj", [
    # Charge far, no limit
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 100,
         "/features/active_power/ess_charge_control_kW_limit": 20000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -ess_rated_power),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -ess_rated_power),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge far, no limit
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 0,
         "/features/active_power/ess_charge_control_kW_limit": 20000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, ess_rated_power),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, ess_rated_power),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge far, smaller limit
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 100,
         "/features/active_power/ess_charge_control_kW_limit": 5000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -5000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -5000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge far, smaller limit
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 0,
         "/features/active_power/ess_charge_control_kW_limit": 5000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 5000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 5000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge near
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 50.1,
         "/features/active_power/ess_charge_control_kW_limit": 10000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        # Because soc is scaled and not exactly the value given, assert small charge
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -500, Tolerance_Type.abs, 500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -500, Tolerance_Type.abs, 500),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge near
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 48.9,
         "/features/active_power/ess_charge_control_kW_limit": 10000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        # Because soc is scaled and not exactly the value given assert, small discharge
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 500, Tolerance_Type.abs, 500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 500, Tolerance_Type.abs, 500),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge, charge disabled
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 100,
         "/features/active_power/ess_charge_control_kW_limit": 5000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": True,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge, discharge disabled
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 0,
         "/features/active_power/ess_charge_control_kW_limit": 5000,
         "/features/active_power/target_soc_load_enable_flag": False,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": True},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charging, load enabled
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 100,
         "/features/active_power/ess_charge_control_kW_limit": 5000,
         "/features/active_power/target_soc_load_enable_flag": True,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 500),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharging, load enabled
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 1},
        {"/features/active_power/ess_charge_control_target_soc": 0,
         "/features/active_power/ess_charge_control_kW_limit": 5000,
         "/features/active_power/target_soc_load_enable_flag": True,
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/ess_charge_control_charge_disable": False,
         "/features/active_power/ess_charge_control_discharge_disable": False},
        target_soc,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 5500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 5500),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    )
])
def target_soc_params(feature_obj):
    return feature_obj


# Export Target test for all pipelines
def export_target(export_target_feature):
    # Set inputs
    export_target_feature.setup()

    global ess_assertion
    global solar_assertion
    global demand_assertion
    global load_responsibility
    load_responsibility = export_target_feature.fims_inputs["/features/active_power/export_target_load_enable_flag"]\
        * -export_target_feature.fims_inputs["/components/bess_aux/active_power_setpoint"]
    # Load tests need significant delay to support load buffer smoothing due to delayed publishes
    time.sleep(site_delay + (site_delay * 2 * load_responsibility))
    solar_running = pfims.SendGet("/assets/solar/summary/num_solar_running", "/pytest")[1]
    solar_potential = 0
    if solar_running > 0:
        solar_potential = pfims.SendGet("/features/active_power/max_potential_solar_kW", "/pytest")[1]["value"]

    # Set expected outputs
    ess_assertion = copy(export_target_feature.expected_ess_cmd)
    solar_assertion = copy(export_target_feature.expected_solar_cmd)
    demand_assertion = copy(export_target_feature.expected_demand)
    logging.error("overwritten expected demand %s", demand_assertion.value)
    logging.error("overwritten expected ess cmd %s", ess_assertion.value)
    if solar_potential == 0:
        # If there is a load solar cannnot handle and ess is charging it must stop charging to handle it instead (priority 0)
        # TODO: cleaner way to design the reuse of params to support this?
        if load_responsibility\
                and export_target_feature.fims_inputs["/components/bess_aux/active_power_setpoint"] < 0\
                and ess_assertion < 0:
            demand_assertion = max(demand_assertion - ess_assertion, load_responsibility)
            ess_assertion.overwrite_value(0)
        ess_assertion += solar_assertion
        if ess_assertion > ess_rated_power:
            ess_assertion.overwrite_value(ess_rated_power)
            demand_assertion.overwrite_value(ess_rated_power)
        solar_assertion.overwrite_value(solar_potential)
    logging.error("Export Target Cmd %s", export_target_feature.fims_inputs["/features/active_power/export_target_kW_cmd"])
    actual_avg_soc = pfims.SendGet("/features/active_power/soc_avg_running", "/pytest")[1]["value"]
    logging.error("Soc avg running %s", actual_avg_soc)
    site_load = pfims.SendGet("/features/active_power/site_kW_load", "/pytest")[1]["value"]
    slew_rate = pfims.SendGet("/features/active_power/export_target_kW_slew_rate", "/pytest")[1]["value"]
    logging.error("slew rate %s", slew_rate)
    logging.error("Load enabled %s and load %s",
                  bool(load_responsibility), site_load)
    logging.error("Expected demand: %s", demand_assertion.value)
    logging.error("Expected ess cmd: %s", ess_assertion.value)
    logging.error("Expected solar cmd: %s", solar_assertion.value)


# Parameters for Export Target tests
@ fixture
@ parametrize("feature_obj", [
    # Charge far
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": -20000,
         "/features/active_power/export_target_load_enable_flag": False,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 0},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -ess_rated_power),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -ess_rated_power),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge near
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": -3000,
         "/features/active_power/export_target_load_enable_flag": False,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 0},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -3000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -3000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge near
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": 3000,
         "/features/active_power/export_target_load_enable_flag": False,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 0},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 3000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
        # Solar priority, shifted to ESS if appropriate
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 3000)
    ),
    # Discharge near, ess prioritized
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": 3000,
         "/features/active_power/export_target_load_enable_flag": False,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 4},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 3000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 3000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge Far
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": 20000,
         "/features/active_power/export_target_load_enable_flag": False,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 0},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, ess_rated_power + solar_rated_power),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, ess_rated_power),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, solar_rated_power)
    ),
    # Charge with load
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": -3000,
         "/features/active_power/export_target_load_enable_flag": True,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 0},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -2500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -3000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 500)
    ),
    # Discharge with load
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 2},
        {"/features/active_power/export_target_kW_cmd": 3000,
         "/features/active_power/export_target_load_enable_flag": True,
         "/features/active_power/export_target_kW_slew_rate": 1000000,  # default slew
         "/components/bess_aux/active_power_setpoint": -500,  # Load
         "/features/active_power/asset_priority_runmode1": 0},
        export_target,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 3500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
        # Solar priority, shifted to ESS if appropriate
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 3500)
    )
])
def export_target_params(feature_obj):
    return feature_obj


# Absolute ESS test for all pipelines
def absolute_ess(abs_ess_feature):
    # Set inputs
    abs_ess_feature.setup()
    time.sleep(site_delay)
    solar_running = pfims.SendGet("/assets/solar/summary/num_solar_running", "/pytest")[1]
    solar_potential = 0
    if solar_running > 0:
        solar_potential = pfims.SendGet("/features/active_power/max_potential_solar_kW", "/pytest")[1]["value"]

    global ess_assertion
    global solar_assertion
    global demand_assertion

    # TODO: feature should have expected demand and expected for each asset? Just for active or for standalone too?
    ess_assertion = copy(abs_ess_feature.expected_ess_cmd)
    solar_assertion = copy(abs_ess_feature.expected_solar_cmd)
    demand_assertion = copy(abs_ess_feature.expected_demand)
    logging.error("Abs ESS cmd %s", ess_assertion.value)
    solar_assertion.overwrite_value(solar_potential)
    demand_assertion += solar_potential
    logging.error("Feature expected demand %s", demand_assertion.value)


# Parameters for Absolute ESS tests
@ fixture
@ parametrize("feature_obj", [
    # zero test
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": False,
         "/features/active_power/absolute_ess_kW_cmd": 0.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # not enough discharge potential, limited by dispatch
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": False,
         "/features/active_power/absolute_ess_kW_cmd": 20000.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 20000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 20000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # not enough charge potential, limited by dispatch
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": True,
         "/features/active_power/absolute_ess_kW_cmd": -20000.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -20000),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -20000),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge positive flag and positive cmd
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": False,
         "/features/active_power/absolute_ess_kW_cmd": 500.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 500),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge negative flag and positive cmd
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": True,
         "/features/active_power/absolute_ess_kW_cmd": 500.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -500),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -500),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Discharge positive flag and negative cmd
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": False,
         "/features/active_power/absolute_ess_kW_cmd": -400.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, 400),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, 400),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    ),
    # Charge negative flag and negative cmd
    Feature(
        {"/features/active_power/runmode1_kW_mode_cmd": 6},
        {"/features/active_power/absolute_ess_direction_flag": True,
         "/features/active_power/absolute_ess_kW_cmd": -400.0},
        absolute_ess,
        expected_demand=Flex_Assertion(Assertion_Type.approx_eq, -400),
        expected_ess_cmd=Flex_Assertion(Assertion_Type.approx_eq, -400),
        expected_solar_cmd=Flex_Assertion(Assertion_Type.approx_eq, 0)
    )


])
def abs_ess_params(feature_obj):
    return feature_obj


# ESS Discharge Prevention test for all pipelines
def edp(edp_feature):
    logging.error("EDP soc: %s", edp_feature.fims_inputs["/features/standalone_power/ess_discharge_prevention_soc"])
    # Set inputs
    edp_feature.setup()
    time.sleep(site_delay)

    # Update values
    global demand_assertion
    global ess_assertion
    avg_soc = pfims.SendGet("/features/active_power/soc_avg_running", "/pytest")[1]["value"]
    if avg_soc <= edp_feature.fims_inputs["/features/standalone_power/ess_discharge_prevention_soc"]:
        # logging.error("ess assertion %s", ess_assertion)
        # new_expected_ess_cmd = min(ess_assertion, 0)
        # if isinstance(new_expected_ess_cmd, Flex_Assertion):
        #     logging.error("edp reduction: %s", new_expected_ess_cmd.value)
        # else:
        #     logging.error("edp reduction: %s", new_expected_ess_cmd)
        # demand_assertion += new_expected_ess_cmd - ess_assertion
        demand_assertion.add_tolerance(Tolerance_Type.abs, 25)
        # ess_assertion.overwrite_value(new_expected_ess_cmd)
        ess_assertion.add_tolerance(Tolerance_Type.abs, 25)
        ess_assertion.max_limit = 0


# Parameters for ESS Discharge Prevention
@ fixture
@ parametrize("feature_obj", [
    Standalone_Feature(
        {"/features/standalone_power/ess_discharge_prevention_enable": True},
        {"/features/standalone_power/ess_discharge_prevention_soc": 0},
        edp
    ),
    Standalone_Feature(
        {"/features/standalone_power/ess_discharge_prevention_enable": True},
        {"/features/standalone_power/ess_discharge_prevention_soc": 100},
        edp
    ),
])
def edp_params(feature_obj):
    return feature_obj


# PFR Test for ESS, TODO: other assets
def standalone_pfr(pfr_feature):
    logging.error("PFR freq offset: %s", pfr_feature.fims_inputs["/features/standalone_power/pfr_offset_hz"])
    # Set inputs
    pfr_feature.setup()
    if pfr_feature.expected_demand == 0:
        return

    time.sleep(site_delay)

    global demand_assertion
    global ess_assertion

    # Ensure PFR response does not cross 0, but can deviate in either direction if zero
    new_demand = pfr_feature.expected_demand
    if demand_assertion > 0:
        new_demand = min(pfr_feature.fims_inputs["/features/standalone_power/pfr_limits_max_kW"],
                         max(demand_assertion + pfr_feature.expected_demand, 0))
    elif demand_assertion < 0:
        new_demand = max(pfr_feature.fims_inputs["/features/standalone_power/pfr_limits_min_kW"],
                         min(demand_assertion + pfr_feature.expected_demand, 0))
    demand_modification = new_demand - demand_assertion
    # TODO: apply diff to appropriate asset based on dispatch priority for other asset pipelines
    logging.error("PFR modifies ess %s by %s", demand_assertion.value, demand_modification)
    ess_assertion += demand_modification
    logging.error("resulting ess %s", ess_assertion)
    demand_assertion += demand_modification


# Parameters for standalone PFR
# Expected demand used as the expected modification to the current site demand
@ fixture
@ parametrize("feature_obj", [
    Standalone_Feature(
        {"/features/standalone_power/pfr_enable_flag": True},
        {"/features/standalone_power/pfr_offset_hz": 3.5,
         "/features/standalone_power/pfr_deadband": 0.017,
         "/features/standalone_power/pfr_droop_percent": 5.0,
         "/features/standalone_power/pfr_limits_min_kW": -5000.0,
         "/features/standalone_power/pfr_limits_max_kW": 5000.0},
        standalone_pfr,
        -5000.0
    ),
    Standalone_Feature(
        {"/features/standalone_power/pfr_enable_flag": True},
        {"/features/standalone_power/pfr_offset_hz": 1.817,
         "/features/standalone_power/pfr_deadband": 0.017,
         "/features/standalone_power/pfr_droop_percent": 5.0,
         "/features/standalone_power/pfr_limits_min_kW": -5000.0,
         "/features/standalone_power/pfr_limits_max_kW": 5000.0},
        standalone_pfr,
        -3000.0
    ),
    Standalone_Feature(
        {"/features/standalone_power/pfr_enable_flag": True},
        {"/features/standalone_power/pfr_offset_hz": 0.005,
         "/features/standalone_power/pfr_deadband": 0.017,
         "/features/standalone_power/pfr_droop_percent": 5.0,
         "/features/standalone_power/pfr_limits_min_kW": -5000.0,
         "/features/standalone_power/pfr_limits_max_kW": 5000.0},
        standalone_pfr,
        0.0
    ),
    Standalone_Feature(
        {"/features/standalone_power/pfr_enable_flag": True},
        {"/features/standalone_power/pfr_offset_hz": -0.005,
         "/features/standalone_power/pfr_deadband": 0.017,
         "/features/standalone_power/pfr_droop_percent": 5.0,
         "/features/standalone_power/pfr_limits_min_kW": -5000.0,
         "/features/standalone_power/pfr_limits_max_kW": 5000.0},
        standalone_pfr,
        0.0
    ),
    Standalone_Feature(
        {"/features/standalone_power/pfr_enable_flag": True},
        {"/features/standalone_power/pfr_offset_hz": -1.817,
         "/features/standalone_power/pfr_deadband": 0.017,
         "/features/standalone_power/pfr_droop_percent": 5.0,
         "/features/standalone_power/pfr_limits_min_kW": -5000.0,
         "/features/standalone_power/pfr_limits_max_kW": 5000.0},
        standalone_pfr,
        3000.0
    ),
    Standalone_Feature(
        {"/features/standalone_power/pfr_enable_flag": True},
        {"/features/standalone_power/pfr_offset_hz": -3.5,
         "/features/standalone_power/pfr_deadband": 0.017,
         "/features/standalone_power/pfr_droop_percent": 5.0,
         "/features/standalone_power/pfr_limits_min_kW": -5000.0,
         "/features/standalone_power/pfr_limits_max_kW": 5000.0},
        standalone_pfr,
        5000.0
    ),
])
def standalone_pfr_params(feature_obj):
    return feature_obj


# POI Limits test for all pipelines?
def poi_limits(poi_limits_feature):
    # Set inputs
    poi_limits_feature.setup()
    min_limit = poi_limits_feature.fims_inputs["/features/standalone_power/poi_limits_min_kW"]
    max_limit = poi_limits_feature.fims_inputs["/features/standalone_power/poi_limits_max_kW"]
    logging.error("poi lims %s %s", min_limit, max_limit)
    time.sleep(site_delay)
    load_inclusion = pfims.SendGet("/features/active_power/site_kW_load_inclusion", "/pytest")[1]["value"]
    site_load = pfims.SendGet("/features/active_power/site_kW_load", "/pytest")[1]["value"]

    global demand_assertion
    global ess_assertion

    new_demand = min(max_limit + load_inclusion * site_load, max(demand_assertion, min_limit + load_inclusion * site_load))
    demand_modification = new_demand - demand_assertion
    logging.error("new poi demand %s modifies ess %s by %s", new_demand, demand_assertion, demand_modification)
    # TODO: apply diff to appropriate asset based on dispatch priority for other asset pipelines
    ess_assertion += demand_modification
    logging.error("resulting ess %s", ess_assertion)
    demand_assertion += demand_modification


# Parameters for POI limits tests
# TODO: for all pipelines?
# TODO: How to modify soc in twins.json and restart the service on the fly from this container?
#       tested soc-based limits for some features (not all) for the time being
@ fixture
@ parametrize("feature_obj", [
    Standalone_Feature(
        {"/features/standalone_power/poi_limits_enable": True},
        {"/features/standalone_power/poi_limits_load_enable": False,
         "/features/standalone_power/poi_limits_min_kW": -20000.0,
         "/features/standalone_power/poi_limits_max_kW": 20000.0},
        poi_limits
    ),
    Standalone_Feature(
        {"/features/standalone_power/poi_limits_enable": True},
        {"/features/standalone_power/poi_limits_load_enable": False,
         "/features/standalone_power/poi_limits_min_kW": -10000.0,
         "/features/standalone_power/poi_limits_max_kW": 10000.0},
        poi_limits
    ),
    Standalone_Feature(
        {"/features/standalone_power/poi_limits_enable": True},
        {"/features/standalone_power/poi_limits_load_enable": False,
         "/features/standalone_power/poi_limits_min_kW": -4000.0,
         "/features/standalone_power/poi_limits_max_kW": 4000.0},
        poi_limits
    ),
    Standalone_Feature(
        {"/features/standalone_power/poi_limits_enable": True},
        {"/features/standalone_power/poi_limits_load_enable": True,
         "/features/standalone_power/poi_limits_min_kW": -20000.0,
         "/features/standalone_power/poi_limits_max_kW": 20000.0},
        poi_limits
    ),
    Standalone_Feature(
        {"/features/standalone_power/poi_limits_enable": True},
        {"/features/standalone_power/poi_limits_load_enable": True,
         "/features/standalone_power/poi_limits_min_kW": -10000.0,
         "/features/standalone_power/poi_limits_max_kW": 10000.0},
        poi_limits
    ),
    Standalone_Feature(
        {"/features/standalone_power/poi_limits_enable": True},
        {"/features/standalone_power/poi_limits_load_enable": True,
         "/features/standalone_power/poi_limits_min_kW": -4000.0,
         "/features/standalone_power/poi_limits_max_kW": 4000.0},
        poi_limits
    )
])
def poi_limits_params(feature_obj):
    return feature_obj


# Watt-Watt test for all pipelines
def watt_watt(watt_watt_feature):
    # Set inputs (only enable flag)
    watt_watt_feature.setup()
    # Small adjustment so shorter site delay
    time.sleep(site_delay)

    global demand_assertion
    global ess_assertion

    # Setup expected watt-watt curve and calculate it's correction
    watt_watt_correction = get_curve_cmd_py(demand_assertion, {-10000: -9892.16, 0: 33.52, 10000: 10105.68})
    logging.error("watt-watt correction: %s from demand %s", watt_watt_correction, demand_assertion)
    # # TODO: apply diff to appropriate asset based on dispatch priority for other asset pipelines
    ess_assertion.overwrite_value(watt_watt_correction)
    logging.error("resulting assertion %s", ess_assertion)
    demand_assertion.overwrite_value(watt_watt_correction)


# Python implementation of Site_Controller_Utils::get_curve_cmd()
# We could call the c++ function from python but it would require changing the input parameters as python
# cannot easily send a std::vector<std::pair>>
def get_curve_cmd_py(value, curve_points):
    cmd = 0
    keys = list(curve_points.keys())
    values = list(curve_points.values())

    if value < keys[0]:
        cmd = ((values[1] - values[0]) / (keys[1] - keys[0])) * (value - keys[0]) + values[0]
    elif value >= keys[len(keys)-1]:
        cmd = ((values[len(keys)-1] - values[len(keys)-2])
               / (keys[len(keys)-1] - keys[len(keys)-2])) * (value - keys[len(keys)-1]) + values[len(keys)-1]
    else:
        for i in range(len(keys) - 1):
            if value >= keys[i] and value < keys[i+1]:
                cmd = ((values[i+1] - values[i]) / (keys[i+1] - keys[i])) * (value - keys[i+1]) + values[i+1]
                break
    return cmd


# Parameters for Watt-Watt tests
@ fixture
@ parametrize("feature_obj", [
    Standalone_Feature(
        {"/features/standalone_power/watt_watt_adjustment_enable_flag": True},
        {},  # Watt-watt does not require any inputs
        watt_watt
    ),
])
def watt_watt_params(feature_obj):
    return feature_obj


# Closed Loop Control test for all pipelines
def clc(clc_feature):
    # Set inputs
    clc_feature.setup()

    global ess_assertion
    global demand_assertion
    global feed_assertion

    clc_tolerance = clc_feature.fims_inputs["/features/standalone_power/active_power_closed_loop_regulation_deadband_kW"]
    invert_poi = -1 if not pfims.SendGet("/site/configuration/invert_poi_kW", "/pytest")[1]["value"] else 1
    # Find expected demand at POI
    site_min_power = pfims.SendGet("/features/active_power/total_site_kW_rated_charge", "/pytest")[1]["value"]
    site_max_power = pfims.SendGet("/features/active_power/total_site_kW_rated_discharge", "/pytest")[1]["value"]
    # If EDP is enabled remove ESS from rated discharge consideration
    ess_max_power = pfims.SendGet("/features/active_power/rated_ess_kW", "/pytest")[1]["value"]
    if ess_assertion.max_limit != None:
        site_max_power -= ess_max_power - ess_assertion.max_limit

    # Determine the average feed actual abd load to get the expected demand correction
    feed_actual = get_asset_agg("feed") * invert_poi
    site_load = pfims.SendGet("/features/active_power/site_kW_load", "/pytest")[1]["value"]
    # Handle case where CLC is limited by the available site production and cannot reach its desired value
    expected_poi = demand_assertion.value
    expected_poi = site_min_power - site_load if demand_assertion < site_min_power - site_load else expected_poi
    expected_poi = site_max_power - site_load if demand_assertion > site_max_power - site_load else expected_poi
    clc_correction = demand_assertion - feed_actual
    clc_min_limit = (clc_feature.fims_inputs["/features/standalone_power/active_power_closed_loop_min_offset"]
                     * clc_feature.fims_inputs["/features/standalone_power/active_power_closed_loop_step_size_kW"])
    clc_max_limit = (clc_feature.fims_inputs["/features/standalone_power/active_power_closed_loop_max_offset"]
                     * clc_feature.fims_inputs["/features/standalone_power/active_power_closed_loop_step_size_kW"])
    # Handle case where deviation is outside of CLC's configured limits
    clc_correction = clc_min_limit if clc_correction < clc_min_limit else clc_correction
    clc_correction = clc_max_limit if clc_correction > clc_max_limit else clc_correction
    # TODO: expected ess or expected demand? both?
    ess_assertion += clc_correction
    logging.error("site min %s and max %s", site_min_power, site_max_power)
    logging.error("site load %s so max should be %s", site_load, (site_max_power - site_load))
    logging.error("expected demand %s", expected_poi)
    logging.error("Reference feed %s", feed_actual)
    logging.error("clc correction %s", clc_correction)
    # Overwrite tolerances based on CLC requirements
    feed_assertion = Flex_Assertion(Assertion_Type.approx_eq, expected_poi, Tolerance_Type.abs, clc_tolerance)
    # TODO: it's assumed the feature will provide a more restrictive tolerance unless absolute necessary
    #       e.g. unresolved target soc disparity causing large deviation in expected kW cmd
    #       So for the time being only use the clc_tolerance if its less restrictive
    if clc_tolerance > ess_assertion.get_kW_tolerance():
        ess_assertion.tolerance_type = Tolerance_Type.abs
        ess_assertion.tolerance = clc_tolerance
    demand_assertion += clc_correction
    if clc_tolerance > ess_assertion.get_kW_tolerance():
        demand_assertion.tolerance_type = Tolerance_Type.abs
        demand_assertion.tolerance = clc_tolerance
    else:
        feed_assertion.tolerance = demand_assertion.get_kW_tolerance()

    time.sleep(5)  # Must sleep long enough to apply correction with possible changing load

    # As the feed_actual value can drift outside of the deadband momentarily, check against an average feed value
    feed_avg = 0
    for i in range(5):
        feed_avg += get_asset_agg("feed")
        time.sleep(0.5)
    feed_avg /= 5

    logging.error("Feed avg %s vs expected POI %s", invert_poi * feed_avg, expected_poi)
    feed_assertion.make_assertion(invert_poi * feed_avg)


# Parameters for CLC tests
@ fixture
@ parametrize("feature_obj", [
    Standalone_Feature(
        {"/features/standalone_power/active_power_closed_loop_enable": True},
        {"/features/standalone_power/active_power_closed_loop_max_offset": 1000,
         "/features/standalone_power/active_power_closed_loop_min_offset": -1000,
         "/features/standalone_power/active_power_closed_loop_step_size_kW": 5,
         "/features/standalone_power/active_power_closed_loop_steady_state_deadband_kW": 1,
         "/features/standalone_power/active_power_closed_loop_regulation_deadband_kW": 50,
         "/components/bess_aux/active_power_setpoint": 0,           # Load setpoint
         "/features/active_power/site_kW_load_interval_ms": 100},   # CLC is incompatiable with load buffer unless deadband is larger than possible load deviations
        clc
    ),
    Standalone_Feature(
        {"/features/standalone_power/active_power_closed_loop_enable": True},
        {"/features/standalone_power/active_power_closed_loop_max_offset": 1000,
         "/features/standalone_power/active_power_closed_loop_min_offset": -1000,
         "/features/standalone_power/active_power_closed_loop_step_size_kW": 5,
         "/features/standalone_power/active_power_closed_loop_steady_state_deadband_kW": 1,
         "/features/standalone_power/active_power_closed_loop_regulation_deadband_kW": 250,
         "/components/bess_aux/active_power_setpoint": 0,           # Load setpoint
         "/features/active_power/site_kW_load_interval_ms": 500},   # CLC is incompatiable with load buffer unless deadband is larger than possible load deviations
        clc
    ),
    Standalone_Feature(
        {"/features/standalone_power/active_power_closed_loop_enable": True},
        {"/features/standalone_power/active_power_closed_loop_max_offset": 1000,
         "/features/standalone_power/active_power_closed_loop_min_offset": -1000,
         "/features/standalone_power/active_power_closed_loop_step_size_kW": 5,
         "/features/standalone_power/active_power_closed_loop_steady_state_deadband_kW": 1,
         "/features/standalone_power/active_power_closed_loop_regulation_deadband_kW": 50,
         "/components/bess_aux/active_power_setpoint": -2000,       # Expected that CLC is insufficient due to it's max offset to compensate for this load
         "/features/active_power/site_kW_load_interval_ms": 100},   # CLC is incompatiable with load buffer unless deadband is larger than possible load deviations
        clc
    )
    # TODO: maybe some kind of duration test that tests the response time
    #       Not really a customer requirement and doesn't fit the current design so untested for now
])
def clc_params(feature_obj):
    return feature_obj


@ parametrize("feature", [energy_arb_params, target_soc_params, export_target_params, abs_ess_params])
@ parametrize("edp", [edp_params])  # TODO: add back None
@ parametrize("standalone_pfr", [standalone_pfr_params])  # TODO: add back None
@ parametrize("poi_limits", [poi_limits_params])  # TODO: add back None
# Separated watt-watt and CLC to save time as they accomplish the same thing (only one or the other will run)
@ parametrize("correction_feature", [watt_watt_params, clc_params])  # TODO: add back None
def test_ess_pipeline(ess_configuration, feature, edp, standalone_pfr, poi_limits, correction_feature):
    # Reset values
    global demand_assertion
    global ess_assertion
    global solar_assertion
    ess_assertion = None
    solar_assertion = None
    demand_assertion = None

    # Run feature
    feature.run_feature()
    # Collect and run standalone features
    standalone_features = [edp, standalone_pfr, poi_limits, correction_feature]
    for standalone_feature in standalone_features:
        if standalone_feature:
            standalone_feature.run_feature()

    # Get result
    invert_poi = -1 if pfims.SendGet("/site/configuration/invert_poi_kW", "/pytest")[1]["value"] else 1
    site_min_power = pfims.SendGet("/features/active_power/total_site_kW_rated_charge", "/pytest")[1]["value"]
    site_max_power = pfims.SendGet("/features/active_power/total_site_kW_rated_discharge", "/pytest")[1]["value"]
    # If EDP is enabled remove ESS from rated discharge consideration
    ess_max_power = pfims.SendGet("/features/active_power/rated_ess_kW", "/pytest")[1]["value"]

    feed_agg = get_asset_agg("feed")
    ess_agg = get_asset_agg("ess")
    gen_agg = get_asset_agg("gen")
    solar_agg = get_asset_agg("solar")

    # Correct if potentials were limited (EDP/Agg asset limit) or request was beyond potentials
    if ess_assertion.max_limit != None:
        site_max_power -= ess_max_power - ess_assertion.max_limit
        logging.error("final limiting ess assertion %s", ess_assertion)
        if ess_assertion > 0:
            demand_assertion -= ess_assertion - ess_assertion.max_limit
            ess_assertion.overwrite_value(ess_assertion.max_limit)
            logging.error("got %s from max value %s", ess_assertion.value, ess_assertion.max_limit)
    if demand_assertion < site_min_power:
        demand_assertion.overwrite_value(site_min_power)
        ess_assertion.overwrite_value(max(ess_assertion, site_min_power))
    elif demand_assertion > site_max_power:
        demand_assertion.overwrite_value(site_max_power)
        ess_assertion.overwrite_value(min(ess_assertion, site_max_power))

    # Disable Standalone Features
    # Active Power features are disabled automatically as only one can run at a time
    for standalone_feature in standalone_features:
        if standalone_feature:
            standalone_feature.disable()

    logging.error("Final expected ess: %s", ess_assertion.value)
    logging.error("Final expected solar: %s", solar_assertion.value)
    logging.error("Final expected demand: %s", demand_assertion.value)

    # Verify result using custom assertions
    ess_assertion.make_assertion(ess_agg)
    solar_assertion.make_assertion(solar_agg)
    demand_assertion.make_assertion(ess_agg + gen_agg + solar_agg)


# TODO:
# @ parametrize("feature", [export_target_params])
# @ parametrize("edp", [edp_params])  # TODO: add back None
# @ parametrize("standalone_pfr", [standalone_pfr_params])  # TODO: add back None
# @ parametrize("poi_limits", [poi_limits_params])  # TODO: add back None
# # Separated watt-watt and CLC to save time as they accomplish the same thing (only one or the other will run)
# @ parametrize("correction_feature", [watt_watt_params])  # TODO: add back None
# def test_ess_solar_pipeline(ess_solar_configuration, feature, edp, standalone_pfr, poi_limits, correction_feature):
#     # Reset values
#     global demand_assertion
#     global ess_assertion
#     global solar_assertion
#     ess_assertion = None
#     solar_assertion = None
#     demand_assertion = None

#     # Run feature
#     feature.run_feature()
#     # Collect and run standalone features
#     standalone_features = [edp, standalone_pfr, poi_limits, correction_feature]
#     for standalone_feature in standalone_features:
#         if standalone_feature:
#             standalone_feature.run_feature()

#     # Get result
#     invert_poi = -1 if pfims.SendGet("/site/configuration/invert_poi_kW", "/pytest")[1]["value"] else 1
#     site_min_power = pfims.SendGet("/features/active_power/total_site_kW_rated_charge", "/pytest")[1]["value"]
#     site_max_power = pfims.SendGet("/features/active_power/total_site_kW_rated_discharge", "/pytest")[1]["value"]
#     # If EDP is enabled remove ESS from rated discharge consideration
#     ess_max_power = pfims.SendGet("/features/active_power/rated_ess_kW", "/pytest")[1]["value"]

#     feed_agg = get_asset_agg("feed")
#     ess_agg = get_asset_agg("ess")
#     gen_agg = get_asset_agg("gen")
#     solar_agg = get_asset_agg("solar")

#     # Correct if potentials were limited (EDP/Agg asset limit) or request was beyond potentials
#     if ess_assertion.max_limit != None:
#         site_max_power -= ess_max_power - ess_assertion.max_limit
#         logging.error("final limiting ess assertion %s", ess_assertion)
#         if ess_assertion > 0:
#             demand_assertion -= ess_assertion - ess_assertion.max_limit
#             ess_assertion.overwrite_value(ess_assertion.max_limit)
#             logging.error("got %s from max value %s", ess_assertion.value, ess_assertion.max_limit)
#     if demand_assertion < site_min_power:
#         demand_assertion.overwrite_value(site_min_power)
#         ess_assertion.overwrite_value(max(ess_assertion, site_min_power))
#     elif demand_assertion > site_max_power:
#         demand_assertion.overwrite_value(site_max_power)
#         ess_assertion.overwrite_value(min(ess_assertion, site_max_power))

#     # Disable Standalone Features
#     # Active Power features are disabled automatically as only one can run at a time
#     for standalone_feature in standalone_features:
#         if standalone_feature:
#             standalone_feature.disable()

#     logging.error("Final expected ess: %s", ess_assertion.value)
#     logging.error("Final expected solar: %s", solar_assertion.value)
#     logging.error("Final expected demand: %s", demand_assertion.value)

#     # Verify result using custom assertions
#     ess_assertion.make_assertion(ess_agg)
#     solar_assertion.make_assertion(solar_agg)
#     demand_assertion.make_assertion(ess_agg + gen_agg + solar_agg)
