'''
The comms_configs module contains class structures for ConfigFiles,
Components, Registers, and MergedRegisters in dnp3 and modbus config
files. Essentially, a MergedRegister contains all of the information
for a register found in both the client config and the server config,
based on offset and register type. Does not account for devices with
multiple device_ids.
'''
import os
import json
try:
    from user_global_utils import LOCAL_PYTHON_SCRIPT_DIR, CONFIGS_DIR
except ImportError:
    from global_utils import LOCAL_PYTHON_SCRIPT_DIR, CONFIGS_DIR

# Types of registers
analog_output_registers = ['AnOPInt32', 'AnOPInt16', 'AnOPF32', 'analogOS', 'holding_registers']
binary_output_registers = ['CROB', 'binaryOS', 'coils']
analog_input_registers = ['analog', 'input_registers']
binary_input_registers = ['binary', 'discrete_inputs']

class MergedRegister:
    '''
    A MergedRegister contains all of the information
    for a register found in both the client config and the server config,
    based on offset and register type. Does not account for devices with
    multiple device_ids.
    '''
    def __init__(self, client_register, server_register):
        self.data_type = None
        self.client_variation = None
        self.server_variation = None
        self.client_id = ""
        self.server_id = ""
        self.client_uri = ""
        self.server_uri = ""
        self.client_size = None
        self.client_float = None
        self.client_signed = None
        if client_register.frequency != 0:
            self.frequency = client_register.frequency
        else:
            self.frequency = 0
        if client_register.register_type == server_register.register_type:
            self.register_type = client_register.register_type
        else:
            return
        if client_register.offset == server_register.offset:
            self.offset = client_register.offset
        else:
            return
        if (self.register_type in analog_output_registers or
            self.register_type in binary_output_registers):
            self.method = "set"
        elif (self.register_type in analog_input_registers or
              self.register_type in binary_input_registers or
              self.register_type == "counter"):
            self.method = "pub"
        else:
            return

        all_client_fields = vars(client_register)
        all_server_fields = vars(server_register)
        for field in all_client_fields:
            setattr(self, f"client_{field}" + field, all_client_fields.get(field, None))
            setattr(self, f"server_{field}", all_server_fields.get(field, None))
        for field in all_server_fields:
            setattr(self, f"client_{field}", all_client_fields.get(field, None))
            setattr(self, f"server_{field}", all_server_fields.get(field, None))
        if self.register_type in ['Analog', 'analog']:
            if self.client_variation is None and self.server_variation is None:
                self.data_type = "float32"
            elif (self.client_variation in ["Group30Var1","Group30Var3"] or
                  self.server_variation in ["Group30Var1","Group30Var3"]):
                self.data_type = "int32"
            elif (self.client_variation in ["Group30Var2","Group30Var4"] or
                  self.server_variation in ["Group30Var2","Group30Var4"]):
                self.data_type = "int16"
            elif (self.client_variation in ["Group30Var5"] or
                  self.server_variation in ["Group30Var5"]):
                self.data_type = "float32"
            elif (self.client_variation in ["Group30Var6"] or
                  self.server_variation in ["Group30Var6"]):
                self.data_type = "float64"
        elif self.register_type in 'counter':
            if self.client_variation is None and self.server_variation is None:
                self.data_type = "uint32"
            elif (self.client_variation in ["Group20Var1","Group20Var3","Group20Var5","Group20Var7"] or
                  self.server_variation in ["Group20Var1","Group20Var3","Group20Var5","Group20Var7"]):
                self.data_type = "uint32"
            elif (self.client_variation in ["Group20Var2","Group20Var4","Group20Var6","Group20Var8"] or
                  self.server_variation in ["Group20Var2","Group20Var4","Group20Var6","Group20Var8"]):
                self.data_type = "uint16"
        elif (self.register_type in binary_input_registers or
              self.register_type in binary_output_registers):
            self.data_type = "bool"
        elif self.register_type in ['AnOPInt32', 'AnOPInt16', 'AnOPF32', 'analogOS']:
            if self.client_variation is None and self.server_variation is None:
                if self.register_type == 'AnOPInt32':
                    self.data_type = "int32"
                if self.register_type == 'AnOPInt16':
                    self.data_type = "int16"
                if self.register_type == 'AnOPF32':
                    self.data_type = "float32"
            elif self.client_variation == "Group40Var1" or self.server_variation =="Group40Var1":
                self.data_type = "int32"
            elif self.client_variation == "Group40Var2" or self.server_variation =="Group40Var2":
                self.data_type = "int16"
            elif self.client_variation == "Group40Var3" or self.server_variation =="Group40Var3":
                self.data_type = "float32"
            elif self.client_variation == "Group40Var4" or self.server_variation =="Group40Var4":
                self.data_type = "float64"
        elif self.register_type in ["holding_registers", "input_registers"]:
            if not self.client_float:
                if self.client_size == 1:
                    self.data_type = "int16"
                elif self.client_size == 2:
                    self.data_type = "int32"
                elif self.client_size == 4:
                    self.data_type = "int64"
                else:
                    self.data_type = "int32"
                if not self.client_signed:
                    self.data_type = "u" + self.data_type
            if self.client_float:
                if self.client_size == 2:
                    self.data_type = "float32"
                elif self.client_size == 4:
                    self.data_type = "float64"
        else:
            self.data_type = "bool"

    def __str__(self):
        return f'"type": {self.register_type}, "offset":{self.offset}, ' + \
               f'"client_id":{self.client_id}, "server_id":{self.server_id}, ' + \
               f'"client_uri":{self.client_uri}, "server_uri":{self.server_uri}'

    def gen_fims_message(self, value):
        '''
        generate a fims message that could be sent to the config file's
        counterpart (client or server) to yield some sort of output.
        '''
        if self.method == "pub":
            uri = self.server_uri
            reg_id = self.server_id
            sleep_time = self.frequency * 3 / 1000
        elif self.method == "set":
            uri = self.client_uri
            reg_id = self.client_id
            sleep_time = 0
        if isinstance(value, bool):
            if value:
                value = "true"
            else:
                value = "false"
        if 'test' in self.server_id or sleep_time == 0:
            return f'fims_send -m {self.method} -u {uri}' + \
                   f' \'{{"{reg_id}":{value}}}\'\nsleep {sleep_time}\n'

        return f'fims_send -m {self.method} -u {uri}' + \
               f' \'{{"{reg_id}":{value}}}\'\nsleep {sleep_time}\n'

    def gen_expected_result(self, value):
        '''
        given a value that was sent to the config file's counterpart
        (client or server), return a message that can be expected to
        be output by the config file's program (server or client)
        '''
        if value > 1000:
            tolerance = value*0.001
        else:
            tolerance = 0.001
        if self.method == "pub":
            uri = self.client_uri
            reg_id = self.client_id
            if "float" not in self.data_type:
                return {'method': self.method, 'uri': uri, 'body':{reg_id:value}}
            return {'method': self.method,
                    'uri': uri,
                    'body':{reg_id:{"value":value, "tolerance":tolerance}}}
        if self.method == "set":
            uri = self.server_uri
            reg_id = self.server_id
            if "float" not in self.data_type:
                return {'method': self.method, 'uri': uri, 'body':{reg_id:value}}
            return {'method': self.method,
                    'uri': uri,
                    'body':{reg_id:{"value":value, "tolerance":tolerance}}}
        return {'method': self.method,
                'uri': self.client_uri,
                'body':{self.client_id:{"value":value, "tolerance":tolerance}}}


class Register:
    '''
    A Register represents a client or server io_point and all of its attributes.
    '''
    def __init__(self, component, register_type, json_reg):
        self.parent = component
        self.idx = None
        self.id = None
        self.register_type = register_type
        self.frequency = component.frequency
        self.offset = None
        self.uri = ""
        self.scale = None
        fields = ['name', 'id', 'offset', 'idx', 'scale', 'signed',
                  'float', 'size', 'shift', 'byte_swap', 'uri', 'class',
                  'deadband', 'timeout', 'invert_mask', 'enum', 'word_order',
                  'bit_strings', 'individual_bits', 'bit_field', 'packed_register',
                  'debounce', 'use_bool', 'format', 'variation', 'evariation']
        for field in fields:
            setattr(self, field, json_reg.get(field, None))
        if self.offset is None and self.idx is not None:
            self.offset = self.idx
        if self.uri is None:
            self.uri = self.parent.uri
        if self.scale is None:
            self.scale = 1

    def __str__(self):
        return f'"type": {self.register_type}, "id":{self.id}, ' + \
               f'"offset":{self.offset}, "uri":{self.uri}'


class Component:
    '''
    A Component represents a DNP3 component (single config file) or a Modbus
    component (multiple can be in the same file). Registers are loaded as part
    of loading a Component.
    '''
    def __init__(self, config_file, base_uri, component_id, frequency, register_map):
        self.parent = config_file
        if len(base_uri) == 0 and len(component_id) == 0:
            self.uri = ""
        elif len(base_uri) > 0:
            if base_uri[0] == "/":
                self.uri = base_uri + "/" + component_id
            else:
                self.uri = "/" + base_uri + "/" + component_id
        self.component_id = component_id
        self.frequency = frequency
        self.registers = []
        self.register_dict = {}
        if isinstance(register_map, list):
            for reg_group in register_map:
                if 'type' in reg_group and 'map' in reg_group:
                    register_type = reg_group['type']
                    if register_type in ["Input Registers", "Input"]:
                        register_type = "input_registers"
                    elif register_type == "Holding":
                        register_type = "holding_registers"
                    elif register_type in ["Coils", "Coil"]:
                        register_type = "coils"
                    elif register_type == "Discrete Inputs":
                        register_type = "discrete_inputs"
                    reg_map = reg_group['map']
                    for reg in reg_map:
                        self.registers.append(Register(self,register_type, reg))
        elif isinstance(register_map, dict):
            for register_type in register_map:
                for reg in register_map[register_type]:
                    self.registers.append(Register(self, register_type, reg))
        for reg in self.registers:
            if reg.register_type not in self.register_dict:
                self.register_dict[reg.register_type] = {}
            self.register_dict[reg.register_type][reg.offset] = reg

    def __str__(self):
        return f'"component_id": {self.component_id}'

class ConfigFile:
    '''
    The ConfigFile class represents either a modbus or dnp3 client or server
    config file. Initializing an object of the ConfigFile class uses the
    filename to load a config file from the config directory, including all
    of its components and registers.
    '''
    def __init__(self, filename):
        self.filename = filename
        self.client_or_server = "client"
        if "client" in filename:
            self.client_or_server = "client"
        elif "server" in filename:
            self.client_or_server = "server"
        self.components = []
        if os.path.isfile(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}"):
            with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}", 'r',
                      encoding="utf-8") as file:
                try:
                    json_text = json.load(file)
                except json.JSONDecodeError as load_file_error:
                    print(f'Could not load json file {filename}: {load_file_error}')
                    return
                if 'components' in json_text:
                    for component in json_text['components']:
                        has_base_uri = False
                        has_component_id = False
                        has_frequency = False
                        has_registers = False
                        if 'component_id' in component:
                            base_uri = component['component_id']
                            has_base_uri = True
                        else:
                            base_uri = "components"
                            has_base_uri = True
                        if 'id' in component:
                            component_id = component['id']
                            has_component_id = True
                        if 'frequency' in component:
                            frequency = component['frequency']
                            has_frequency = True
                        if 'registers' in component:
                            has_registers = True
                        if has_base_uri and has_component_id and has_frequency and has_registers:
                            self.components.append(Component(self, base_uri, component_id,
                                                             frequency, component['registers']))

                elif 'registers' in json_text:
                    if 'system' in json_text:
                        json_sys = json_text['system']
                        if 'base_uri' in json_sys:
                            base_uri = json_sys['base_uri']
                        else:
                            base_uri = ""
                        if 'id' in json_sys:
                            component_id = json_sys['id']
                        else:
                            component_id = ""
                        if 'frequency' in json_sys:
                            frequency = json_sys['frequency']
                        else:
                            frequency = 0
                    else:
                        base_uri = ""
                        component_id = ""
                        frequency = 0
                    self.components.append(Component(self, base_uri, component_id,
                                                     frequency, json_text['registers']))

    def __str__(self):
        return self.filename
