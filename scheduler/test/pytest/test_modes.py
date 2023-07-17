from fims import send_set, send_get, send_post, send_del
# need to import set_up since it is used as a test fixture. Pylance seemingly unable to detect that it is being used as a fixture
from fixtures import set_up
from sample_modes import default, modes_cfg, absolute_ess, target_soc

# SETs


def test_set_modes():
    assert {'default': default} == send_del('/scheduler/modes')
    assert modes_cfg == send_set('/scheduler/modes', modes_cfg)
    assert modes_cfg == send_get('/scheduler/modes')


def test_set_individual_mode():
    edited_target_soc = {
        'name': 'Target State of Charge',
        'color_code': 'lightBlue',
        'icon': 'BatteryVert',
        'constants': [{'id': 'apf', 'name': 'Feature', 'type': 'Bool', 'unit': 'asdf', 'uri': '/feature/uri', 'value': True}],
        'variables': [{'id': 'tsoc', 'name': 'Setpoint', 'type': 'Int', 'unit': 'percent', 'uri': '/setpoint/uri', 'value': 50}]
    }
    assert edited_target_soc == send_set('/scheduler/modes/target_soc', edited_target_soc)
    assert edited_target_soc == send_get('/scheduler/modes/target_soc')


def test_set_mode_name():
    name = 'Abs ESS'
    assert name == send_set('/scheduler/modes/absolute_ess/name', name)
    assert name == send_get('/scheduler/modes/absolute_ess/name')


def test_set_mode_color_code():
    color_code = 'teal'
    assert color_code == send_set('/scheduler/modes/absolute_ess/color_code', color_code)
    assert color_code == send_get('/scheduler/modes/absolute_ess/color_code')


def test_set_mode_icon():
    icon = 'Moon'
    assert icon == send_set('/scheduler/modes/absolute_ess/icon', icon)
    assert icon == send_get('/scheduler/modes/absolute_ess/icon')


def test_set_mode_constants():
    constants = [
        {'id': 'feat', 'name': 'Feature', 'type': 'Float', 'unit': '', 'uri': '/features/selection', 'value': 200.5},
        {'id': 'oop', 'name': 'Object-Oriented Programming', 'type': 'Bool', 'unit': 'objects', 'uri': '/favorites/colors/red', 'value': True}
    ]
    assert constants == send_set('/scheduler/modes/target_soc/constants', constants)
    assert constants == send_get('/scheduler/modes/target_soc/constants')


def test_set_mode_individual_constant():
    # this also tests that a SET to an individual constant will not allow the ID to be overwritten
    constant = {'id': 'feat', 'name': 'Feature', 'type': 'Float', 'unit': '', 'uri': '/features/selection', 'value': 200.5}
    result = send_set('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd', constant)
    constant['id'] = 'runmode1_kW_mode_cmd'
    assert constant == result
    assert constant == send_get('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd')


def test_set_mode_constant_name():
    name = 'Feature Selection'
    assert name == send_set('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/name', name)
    assert name == send_get('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/name')


def test_set_mode_constant_unit():
    unit = 'Timothies'
    assert unit == send_set('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/unit', unit)
    assert unit == send_get('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/unit')


def test_set_mode_constant_uri():
    uri = '/features/active_power/runmode1_kW_mode_cmd_scheduler'
    assert uri == send_set('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/uri', uri)
    assert uri == send_get('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/uri')


def test_set_mode_constant_value():
    value = 2
    assert value == send_set('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/value', value)
    assert value == send_get('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/value')


def test_set_mode_variables():
    variables = [
        {'id': 'soc', 'name': 'SoC', 'type': 'Int', 'unit': 'percent', 'uri': '/features/soc', 'value': 50},
        {'id': 'oop', 'name': 'Object-Oriented Programming', 'type': 'Bool', 'unit': 'objects', 'uri': '/favorites/colors/red', 'value': True}
    ]
    assert variables == send_set('/scheduler/modes/target_soc/variables', variables)
    assert variables == send_get('/scheduler/modes/target_soc/variables')


def test_set_mode_individual_variable():
    # this also tests that a SET to an individual variable will not allow the ID to be overwritten
    variable = {'id': 'cmd', 'name': 'Command', 'type': 'Int', 'unit': 'percent', 'uri': '/features/soc_cmd', 'value': 25}
    result = send_set('/scheduler/modes/target_soc/variables/soc_target', variable)
    variable['id'] = 'soc_target'
    assert variable == result
    assert variable == send_get('/scheduler/modes/target_soc/variables/soc_target')


def test_set_mode_variable_name():
    name = 'Setpoint'
    assert name == send_set('/scheduler/modes/target_soc/variables/soc_target/name', name)
    assert name == send_get('/scheduler/modes/target_soc/variables/soc_target/name')


def test_set_mode_variable_unit():
    unit = 'Timothys'
    assert unit == send_set('/scheduler/modes/target_soc/variables/soc_target/unit', unit)
    assert unit == send_get('/scheduler/modes/target_soc/variables/soc_target/unit')


def test_set_mode_variable_uri():
    uri = '/features/active_power/ess_charge_control_target_soc_scheduler'
    assert uri == send_set('/scheduler/modes/target_soc/variables/soc_target/uri', uri)
    assert uri == send_get('/scheduler/modes/target_soc/variables/soc_target/uri')


def test_set_mode_variable_value():
    value = 55.5
    assert value == send_set('/scheduler/modes/target_soc/variables/soc_target/value', value)
    assert value == send_get('/scheduler/modes/target_soc/variables/soc_target/value')

# GETs


def test_get_mode_constant_type():
    # does not have associated SET endpoint because changing the type without changing value is an invalid operation
    assert 'Int' == send_get('/scheduler/modes/target_soc/constants/runmode1_kW_mode_cmd/type')


def test_get_mode_variable_type():
    # does not have associated SET endpoint because changing the type without changing value is an invalid operation
    assert 'Float' == send_get('/scheduler/modes/target_soc/variables/soc_target/type')


def test_get_nonexistent_mode():
    assert { 'error': 'Resource Not Found' } == send_get('/scheduler/modes/ludicrous')

# POSTs


def test_post_mode():
    one_less_mode = {'default': default, 'target_soc': target_soc}
    assert one_less_mode == send_set('/scheduler/modes', one_less_mode)
    assert modes_cfg == send_post('/scheduler/modes', absolute_ess)
    assert modes_cfg == send_get('/scheduler/modes')


def test_post_mode_variable():
    existing_variable = target_soc['variables'][0]
    new_variable = {
        'name': 'Number of Cashews',
        'type': 'Int',
        'unit': 'nuts',
        'uri': '/nuts/types/cashews/quantity',
        'value': 100000
    }
    result = send_post('/scheduler/modes/target_soc/variables', new_variable)
    new_variable['id'] = 'number_of_cashews'
    assert [existing_variable, new_variable] == result
    assert new_variable == send_get('/scheduler/modes/target_soc/variables/number_of_cashews')


def test_post_mode_constant():
    existing_constant = target_soc['constants'][0]
    new_constant = {
        'name': 'Number of Nut Types',
        'type': 'String',
        'unit': 'types',
        'uri': '/nutes/types/quantity',
        'value': 'There are more than 20 different types of edible nuts around the world.'
    }
    result = send_post('/scheduler/modes/target_soc/constants', new_constant)
    new_constant['id'] = 'number_of_nut_types'
    assert [existing_constant, new_constant] == result
    assert new_constant == send_get('/scheduler/modes/target_soc/constants/number_of_nut_types')

# DELs


def test_del_modes():
    assert {'default': default} == send_del('/scheduler/modes')
    assert {'default': default} == send_get('/scheduler/modes')


def test_del_individual_mode():
    one_less_mode = {'default': default, 'target_soc': target_soc}
    assert one_less_mode == send_del('/scheduler/modes/absolute_ess')
    assert one_less_mode == send_get('/scheduler/modes')


def test_del_mode_variables():
    assert [] == send_del('/scheduler/modes/target_soc/variables')
    assert [] == send_get('/scheduler/modes/target_soc/variables')


def test_del_mode_constants():
    assert [] == send_del('/scheduler/modes/target_soc/constants')
    assert [] == send_get('/scheduler/modes/target_soc/constants')


def test_del_mode_individual_variable():
    charge_flag_var = absolute_ess['variables'][0]
    assert [charge_flag_var] == send_del('/scheduler/modes/absolute_ess/variables/absolute_ess_setpoint')
    assert [charge_flag_var] == send_get('/scheduler/modes/absolute_ess/variables')


def test_del_mode_individual_constant():
    assert [] == send_del('/scheduler/modes/absolute_ess/constants/active_power_feature_selection')
    assert [] == send_del('/scheduler/modes/absolute_ess/constants')
