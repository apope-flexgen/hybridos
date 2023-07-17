default = {
    'name': 'Default',
    'color_code': 'gray',
    'icon': 'Build',
    'constants': [
        {
            'id': 'soc_target',
            'name': 'Target SoC Setpoint',
            'type': 'Float',
            'unit': '%',
            'uri': '/features/active_power/ess_charge_control_target_soc',
            'value': 0
        }
    ],
    'variables': []
}

target_soc = {
    'name': 'Target SOC',
    'color_code': 'lightGreen',
    'icon': 'BatteryCharging',
    'constants': [
        {
            'id': 'runmode1_kW_mode_cmd',
            'name': 'Active Power Feature',
            'type': 'Int',
            'unit': '',
            'uri': '/features/active_power/runmode1_kW_mode_cmd',
            'value': 1
        }
    ],
    'variables': [
        {
            'id': 'soc_target',
            'name': 'Target SoC Setpoint',
            'type': 'Float',
            'unit': '%',
            'uri': '/features/active_power/ess_charge_control_target_soc',
            'value': 100
        }
    ]
}

absolute_ess = {
    'name': 'Absolute ESS',
    'color_code': 'lightBlue',
    'icon': 'BatteryVert',
    'constants': [
        {
            'id': 'active_power_feature_selection',
            'name': 'Active Power Feature Selection',
            'type': 'Int',
            'unit': '',
            'uri': '/features/active_power/runmode1_kW_mode_cmd',
            'value': 6
        }
    ],
    'variables': [
        {
            'id': 'charge_flag',
            'name': 'Charge Flag',
            'type': 'Bool',
            'unit': '',
            'uri': '/features/active_power/absolute_ess_direction_flag',
            'value': False
        },
        {
            'id': 'absolute_ess_setpoint',
            'name': 'Absolute ESS Setpoint',
            'type': 'Float',
            'unit': 'kW',
            'uri': '/features/active_power/absolute_ess_kW_cmd',
            'value': 1000
        }
    ]
}

modes_cfg = {
    'absolute_ess': absolute_ess,
    'target_soc': target_soc,
    'default': default
}
