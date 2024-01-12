import os
import json
import random
import sys

class Register:
    def __init__(self, uri, register_name):
        self.uri = uri
        self.register_name = register_name
        self.value = 0

class Input:
    def __init__(self, uri, registers):
        self.uri = uri
        self.input_registers = {}
        self.output_registers = {}
        self.register_map = {}
        self.register_map_backwards = {}
        for (newName, oldName) in registers.items():
            self.input_registers[oldName] = Register(self.uri, oldName)
            self.output_registers[newName] = Register(self.uri, newName)
            self.register_map[newName] = oldName
            if oldName in self.register_map_backwards:
                self.register_map_backwards[oldName].append(newName)
            else:
                self.register_map_backwards[oldName] = [newName]
        
    def gen_random_values(self):
        for (oldName, newNames) in self.register_map_backwards.items():
            newValue = random.randint(0,100)
            self.input_registers[oldName].value = newValue
            for newName in newNames:
                self.output_registers[newName].value = newValue

    def gen_naked_input_message(self):
        msgBody = {}
        for (registerName,register) in self.input_registers.items():
            msgBody[registerName] = register.value
        return f"fims_send -m pub -u {self.uri} '{json.dumps(msgBody,indent=None,separators=(',',':'))}'"
    
    def gen_clothed_input_message(self):
        msgBody = {}
        for (registerName,register) in self.input_registers.items():
            msgBody[registerName] = {"value":register.value}
        return f"fims_send -m pub -u {self.uri} '{json.dumps(msgBody,indent=None,separators=(',',':'))}'"
    
    def gen_naked_input_message_single_pubs(self):
        string_output = ""
        for (registerName,register) in self.input_registers.items():
            string_output += f"fims_send -m pub -u {self.uri}/{registerName} {register.value}\n"
        return string_output

    def gen_clothed_input_message_single_pubs(self):
        string_output = ""
        for (registerName,register) in self.input_registers.items():
            string_output += "fims_send -m pub -u "+self.uri+"/"+registerName+" '{\"value\":"+str(register.value)+"}\n"
        return string_output
    



class Echo:
    def __init__(self,uri, publishRate, null_value_default=0, inputs=None, echo=None):
        self.uri = uri
        self.publishRate = publishRate
        self.null_value_default = null_value_default
        self.inputs = []
        self.output_registers = {}
        self.input_registers = {}
        self.register_map = {}
        self.register_map_backwards = {}
        self.echo_registers = {}
        if inputs != None:
            for input in inputs:
                self.inputs.append(Input(input['uri'],input['registers']))
                self.output_registers.update(self.inputs[-1].output_registers)
                self.input_registers.update(self.inputs[-1].input_registers)
                self.register_map.update(self.inputs[-1].register_map)
                self.register_map_backwards.update(self.inputs[-1].register_map_backwards)
        if echo != None:
            for echo_register in echo:
                self.output_registers[echo_register] = Register(self.uri, echo_register)
                self.echo_registers[echo_register] = self.output_registers[echo_register]

    def gen_random_input_values(self):
        for input in self.inputs:
            input.gen_random_values()

    def gen_random_echo_values(self):
        for echo_register_name, echo_register in self.echo_registers:
            newValue = random.randint(-100,100)
            echo_register.value = newValue

    def gen_expected_output_message(self):
        msgBody = {}
        for output_register_name, output_register in self.output_registers.items():
            msgBody[output_register_name] = output_register.value
        return json.dumps(msgBody,indent=None,separators=(',',':'))

if os.path.isfile(sys.argv[1]):
    with open(sys.argv[1], 'r') as file:
        json_struct = json.loads(file.read())
    echo_objects = []
    for i, echo_object in enumerate(json_struct['echo']):
        print(f"ECHO OBJECT {i}")
        echo_objects.append(Echo(**echo_object))
        print(echo_objects[-1].gen_expected_output_message())
        for echo_input in echo_objects[-1].inputs:
            echo_input.gen_random_values()
            print(echo_input.gen_naked_input_message())
        print(echo_objects[-1].gen_expected_output_message())
        for echo_input in echo_objects[-1].inputs:
            echo_input.gen_random_values()
            print(echo_input.gen_clothed_input_message())
        print(echo_objects[-1].gen_expected_output_message())
        print()
        print()
