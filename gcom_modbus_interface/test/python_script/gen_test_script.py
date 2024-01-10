import json
import os
import sys
import random
import math
from test_utils import *
from gen_test_cases import *
from configs import *

def write_command_files():
    global all_config_file_pairs, all_config_pairs, all_config_register_sets, all_fims_commands, all_expected_messages
    for file_set, command_set, test_register_set in list(zip(all_config_file_pairs, all_fims_commands, test_register_sets)):
        client_file = file_set[0]
        server_file = file_set[1]
        client_commands = command_set[0]
        server_commands = command_set[1]
        pub_test_register = test_register_set[0]
        set_test_register = test_register_set[1]

        client_output_filename = f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/{client_file.replace('.json', '.sh')}"
        server_output_filename = f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/{server_file.replace('.json', '.sh')}"
        with open(client_output_filename, 'w', newline='\n') as file:
            for fims_command in client_commands:
                file.write(f"{fims_command}\n")
            uri = set_test_register.client_uri
            file.write(f'fims_send -m set -u {uri} "{{\\"{set_test_register.client_id}\\":-1}}"\n')
            file.write(f"echo \"done\"\n")
        with open(server_output_filename, 'w', newline='\n') as file:
            for fims_command in server_commands:
                file.write(f"{fims_command}\n")
            file.write(f"echo \"done\"\n")

def write_expected_message_files():
    global all_config_file_pairs, all_config_pairs, all_config_register_sets, all_fims_commands, all_expected_messages
    for file_set, expected_message_set in list(zip(all_config_file_pairs, all_expected_messages)):
        client_file = file_set[0]
        server_file = file_set[1]
        client_expected_messages = expected_message_set[0]
        server_expected_messages = expected_message_set[1]


        client_output_filename =f"{LOCAL_PYTHON_SCRIPT_DIR}/{EXPECTED_TEST_OUTPUT_DIR}/expected_fims_output_{client_file}"
        server_output_filename =f"{LOCAL_PYTHON_SCRIPT_DIR}/{EXPECTED_TEST_OUTPUT_DIR}/expected_fims_output_{server_file}"
        with open(client_output_filename, 'w', newline='\n') as file:
            file_contents = {"test_cases": client_expected_messages}
            file.write(json.dumps(file_contents, indent=4))
        with open(server_output_filename, 'w', newline='\n') as file:
            file_contents = {"test_cases": server_expected_messages}
            file.write(json.dumps(file_contents, indent=4))

test_id = 0
def gen_commands():
    global test_id, all_config_file_pairs, all_config_pairs, all_config_register_sets, all_fims_commands, all_expected_messages
    for register_set, test_register_set in list(zip(all_config_register_sets,test_register_sets)):
        pub_test_register = test_register_set[0]
        set_test_register = test_register_set[1]
        if isinstance(pub_test_register, int) or isinstance(set_test_register,int):
            continue
        fims_commands = [[],[]]
        expected_messages = [{},{}]
        for register in register_set:
            if register == pub_test_register or register == set_test_register:
                continue
            if register.method == "pub":
                [test_id, temp_fims_commands, temp_expected_messages] = test_basics(test_id, pub_test_register, register)
                fims_commands[1].extend(temp_fims_commands)
                expected_messages[0].update(temp_expected_messages)
            if register.method == "set":
                [test_id, temp_fims_commands, temp_expected_messages] = test_basics(test_id, set_test_register, register)
                fims_commands[0].extend(temp_fims_commands)
                expected_messages[1].update(temp_expected_messages)
        all_fims_commands.append(fims_commands)
        all_expected_messages.append(expected_messages)

def get_test_register_sets():
    global all_config_register_sets, test_register_sets
    for registers in all_config_register_sets:
        test_registers = [-1,-1]
        for register in registers:
            if 'test' in register.client_id and register.register_type in analog_input_registers:
                test_registers[0] = register
            if 'test' in register.client_id and register.register_type in analog_output_registers:
                test_registers[1] = register
        test_register_sets.append(test_registers)
    
def get_config_pairs():
    global all_config_file_pairs, all_config_pairs, all_config_register_sets
    for [client_file, server_file] in all_config_file_pairs:
        client_config = ConfigFile(client_file)
        server_config = ConfigFile(server_file)
        for comp_idx, component in enumerate(client_config.components):
            if len(server_config.components) <= comp_idx:
                break
            registers = []
            for register_type in component.register_dict:
                if register_type in server_config.components[comp_idx].register_dict:
                    for offset in component.register_dict[register_type]:
                        if offset in server_config.components[comp_idx].register_dict[register_type]:
                            client_register = component.register_dict[register_type][offset]
                            server_register = server_config.components[comp_idx].register_dict[register_type][offset]
                            registers.append(MergedRegister(client_register, server_register))
            all_config_register_sets.append(registers)

def get_config_file_pairs():
    global all_config_file_pairs
    all_config_files = []
    for file in os.listdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}"):
        if file.lower().endswith('.json'):
            all_config_files.append(file)
    changes_to_all_config_files = True
    while changes_to_all_config_files:
        if len(all_config_files) > 0:
            for file in all_config_files:
                if 'client' in file:
                    matching_server_file = file.replace('client','server')
                    if matching_server_file in all_config_files:
                        all_config_file_pairs.append([file,matching_server_file])
                        all_config_files.remove(file)
                        all_config_files.remove(matching_server_file)
                        changes_to_all_config_files = True
                        break
                if 'server' in file:
                    matching_client_file = file.replace('server','client')
                    if matching_client_file in all_config_files:
                        all_config_file_pairs.append([matching_client_file,file])
                        all_config_files.remove(file)
                        all_config_files.remove(matching_client_file)
                        changes_to_all_config_files = True
                        break
                changes_to_all_config_files = False
        else:
            changes_to_all_config_files = False


if __name__ == '__main__':
    #messages = {}
    for folder in dirs:
        if not os.path.exists(folder):
            os.mkdir(folder)
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    gen_commands()
    write_command_files()
    write_expected_message_files()