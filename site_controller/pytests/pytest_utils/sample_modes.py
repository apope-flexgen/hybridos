from typing import List, Dict, Union

class Constants_Variables_Json(Dict):
    mode_id: str = ""
    name: str = ""
    unit: str = ""
    uri: str = ""
    value_type: str = ""
    value: Union[str, float, int] = ""
    is_template: bool = False
    batch_prefix: str = ""
    batch_range: Union[List,None] = None
    batch_value: Union[List,None] = None

    def __eq__(self, other):
        if isinstance(other, Constants_Variables_Json):
            return self.mode_id == other.mode_id and self.name == other.name and self.unit == other.unit and self.uri == other.uri and self.value_type == other.value_type and self.value == other.value and self.is_template == other.is_template and self.batch_prefix == other.batch_prefix and self.batch_range == other.batch_range and self.batch_value == other.batch_value
        else:
            return False

    def __init__(self, input_dict: Dict):
        # Define default values for attributes
        default_values = {
                'id': "",
                'name': "",
                'unit': "",
                'uri': "",
                'type': "",
                'value': "",
                'is_template': False,
                'batch_prefix': '',
                'batch_range': None,
                'batch_value': None
                }

        if input_dict is None:
            input_dict = {}  # If input_dict is None, initialize it as an empty dictionary

        input_dict.update({key: value for key, value in default_values.items() if key not in input_dict})

        super().__init__(**{**default_values, **input_dict})
        self.mode_id = input_dict.get('id', "")
        self.name = input_dict.get('name', "")
        self.unit = input_dict.get('unit', "")
        self.uri = input_dict.get('uri', "")
        self.value_type = input_dict.get('type', "")
        self.value = input_dict.get('value', "")
        self.is_template = input_dict.get('is_template', False)
        self.batch_prefix = input_dict.get('batch_prefix', "")
        self.batch_range = input_dict.get('batch_range', None)
        self.batch_value = input_dict.get('batch_value', None)

class Mode(Dict):
    name: str = ""
    color_code: str = ""
    icon: str = ""
    constants: Union[List[Constants_Variables_Json], None] = None
    variables: Union[List[Constants_Variables_Json], None] = None

    def __init__(self, input_dict: Dict) -> None:
        # Define default values for attributes
        default_values = {
                'name': "",
                'color_code': "",
                'icon': "",
                'constants': None,
                'variables': None
                }

        if input_dict is None:
            input_dict = {}  # If input_dict is None, initialize it as an empty dictionary

        input_dict.update({key: value for key, value in default_values.items() if key not in input_dict})

        super().__init__(**{**default_values, **input_dict})
        self.name = input_dict.get('name', "")
        self.color_code = input_dict.get('color_code', "")
        self.icon = input_dict.get('icon', "")
        self.constants = input_dict.get('constants', None)
        self.variables = input_dict.get('variables', None)

    def __str__(self):
        return f"name: {self.name}\ncolor_code: {self.color_code}\nicon: {self.icon}\nconstants: {self.constants}\nvariables: {self.variables}\n"

    def __eq__(self, other):
        if isinstance(other, Mode):
            return self.name == other.name and self.color_code == other.color_code and self.icon == other.icon and self.constants == other.constants and self.variables == other.variables
        return False

    def to_dict(self):
        return {'name': f'{self.name}',
                'color_code': f'{self.color_code}',
                'icon': f'{self.icon}',
                'constants': self.constants,
                'variables': self.variables
                }

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
            'value': 0,
            "is_template": False,
            "batch_prefix": "",
            "batch_range": None,
            "batch_value": None
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
            'value': 1,
            "is_template": False,
            "batch_prefix": "",
            "batch_range": None,
            "batch_value": None
        }
    ],
    'variables': [
        {
            'id': 'soc_target',
            'name': 'Target SoC Setpoint',
            'type': 'Float',
            'unit': '%',
            'uri': '/features/active_power/ess_charge_control_target_soc',
            'value': 100,
            "is_template": False,
            "batch_prefix": "",
            "batch_range": None,
            "batch_value": None
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
            'value': 6,
            "is_template": False,
            "batch_prefix": "",
            "batch_range": None,
            "batch_value": None
        }
    ],
    'variables': [
        {
            'id': 'charge_flag',
            'name': 'Charge Flag',
            'type': 'Bool',
            'unit': '',
            'uri': '/features/active_power/absolute_ess_direction_flag',
            'value': False,
            "is_template": False,
            "batch_prefix": "",
            "batch_range": None,
            "batch_value": None
        },
        {
            'id': 'absolute_ess_setpoint',
            'name': 'Absolute ESS Setpoint',
            'type': 'Float',
            'unit': 'kW',
            'uri': '/features/active_power/absolute_ess_kW_cmd',
            'value': 1000,
            "is_template": False,
            "batch_prefix": "",
            "batch_range": None,
            "batch_value": None
        }
    ]
}

bat_bal = {
	'name': 'Battery Balancing',
	'color_code': 'Orange',
	'icon': 'BatteryCharging',
	'constants': [],
	'variables': [
    {
		'id': 'maint_mode',
		'name': 'Maintenance Mode',
		'type': 'Bool',
		'unit': '',
		'uri': '/ess_#/maint_mode',
		'value': True,
		"is_template": True,
		"batch_prefix": "/assets/ess",
		"batch_range": ["1..2"],
		"batch_value": [1,2]
	},
	{
		'id': 'start',
		'name': 'Start',
		'type': 'Bool',
		'unit': '',
		'uri': '/ess_#/actions/Calibration1/start',
		'value': True,
		"is_template": True,
		"batch_prefix": "/assets/ess",
		"batch_range": ["1..2"],
		"batch_value": [1,2]
	}	
    ]
}

modes_cfg = {
	'absolute_ess': absolute_ess,
	'target_soc': target_soc,
    'battery_balancing': bat_bal,
	'default': default
}
