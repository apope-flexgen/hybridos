'''
Read in a set of dnp3 or modbus client config docs, parse them into
registers, and create a set of test cases to run against the client or
server. Also output a test config document that describes the test cases
and the expected results
'''
import json
import os
try:
    from user_global_utils import LOCAL_PYTHON_SCRIPT_DIR, TEST_CONFIG_DIR, \
        CONFIGS_DIR, DIRS
except ImportError:
    from global_utils import LOCAL_PYTHON_SCRIPT_DIR, TEST_CONFIG_DIR, \
        CONFIGS_DIR, DIRS
from gen_test_cases import test_basics, COMMANDS_BY_TEST_ID
from comms_configs import ConfigFile, MergedRegister, \
    analog_input_registers, analog_output_registers
from timestamp import timestamp

ALL_CONFIG_FILE_PAIRS = []  # list of [client_config_filename, server_config_filename] pairs
ALL_CONFIG_PAIRS = []       # list of [ConfigFile(client_config_filename),ConfigFile(server_config_filename)] pairs
ALL_CONFIG_REGISTER_SETS = [] # list of register lists (i.e. [MergedRegister(client_register,server_register), MergedRegister(client_register,server_register),...]) for each config file pair
TEST_REGISTER_SETS = []     # list of test registers [MergedRegister(pub_test_register),MergedRegister(set_test_register)] for each config pair
ALL_FIMS_COMMANDS = []      # list of [client_commands, server_commands] for each register set
ALL_EXPECTED_MESSAGES = []  # list of [{client_test_num:test_num_expected_message_list},{server_test_num:test_num_expected_message_list}]
OUTPUT_FILE_CONTENT = []
OUTPUT_FILENAMES = []

def load_output_files():
    '''
    Load all of the test config output files (that may have had changes since
    they were auto-generated). Use these instead of generating new ones.
    '''
    for (client_filename, server_filename) in ALL_CONFIG_FILE_PAIRS:
        client_file = None
        server_file = None
        try:
            with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONFIG_DIR}/{client_filename}",
                      "r", encoding="utf-8") as file:
                client_file = json.loads(file)
        except:
            pass
        try:
            with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONFIG_DIR}/{server_filename}",
                      "r", encoding="utf-8") as file:
                server_file = json.loads(file)
        except:
            pass
        OUTPUT_FILE_CONTENT.append((client_file, server_file))


def write_output_files(output_dir, include_timestamp=True):
    '''
    Write the output test config files to the specified directory (either test_output or
    test_configs).
    '''
    for ((client_filename, server_filename),(output_file_client, output_file_server)) \
        in zip(ALL_CONFIG_FILE_PAIRS, OUTPUT_FILE_CONTENT):
        if include_timestamp:
            client_filename = client_filename.replace(".json", f"_{timestamp.file_fmt}.json")
            server_filename = server_filename.replace(".json", f"_{timestamp.file_fmt}.json")
        with open(f"{output_dir}/{client_filename}", "w", newline="\n", encoding="utf-8") as file:
            file.write(json.dumps(output_file_client, indent=4))
        with open(f"{output_dir}/{server_filename}", "w", newline="\n", encoding="utf-8") as file:
            file.write(json.dumps(output_file_server, indent=4))


def build_output_files():
    '''
    Generate the JSON object structure for the output test config files. Prepopulate
    with generated expected messages and fims commands.
    '''
    output_file_client = {}
    output_file_server = {}
    for expected_message_set in ALL_EXPECTED_MESSAGES:
        client_expected_messages = expected_message_set[0]
        server_expected_messages = expected_message_set[1]
        for test_id,expected_messages in client_expected_messages.items():
            output_file_client[test_id] = {}
            output_file_client[test_id]['commands'] = COMMANDS_BY_TEST_ID[test_id]
            output_file_client[test_id]['expected'] = expected_messages
            output_file_client[test_id]['actual'] = None
            output_file_client[test_id]['result'] = None
        output_file_client['git_commit_hash'] = None
        output_file_client['git_branch'] = None
        output_file_client['git_commit_author'] = None

        for test_id,expected_messages in server_expected_messages.items():
            output_file_server[test_id] = {}
            output_file_server[test_id]['commands'] = COMMANDS_BY_TEST_ID[test_id]
            output_file_server[test_id]['expected'] = expected_messages
            output_file_server[test_id]['actual'] = None
            output_file_server[test_id]['result'] = None
            output_file_server[test_id]['git_commit'] = None
            output_file_server[test_id]['git_branch'] = None
            output_file_server[test_id]['git_commit_hash'] = None
        output_file_server['git_commit_hash'] = None
        output_file_server['git_branch'] = None
        output_file_server['git_commit_author'] = None
    OUTPUT_FILE_CONTENT.append([output_file_client, output_file_server])

    write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONFIG_DIR}", False)


def gen_commands():
    '''
    Use the gen_test_cases module to generate all test cases and add them to the fims_commands and
    expected_messages for their respective config files.
    '''
    test_id = 0
    for register_set, test_register_set in list(zip(ALL_CONFIG_REGISTER_SETS,TEST_REGISTER_SETS)):
        pub_test_register = test_register_set[0]
        set_test_register = test_register_set[1]
        if isinstance(pub_test_register, int) or isinstance(set_test_register,int):
            continue
        fims_commands = [[],[]]
        expected_messages = [{},{}]
        for register in register_set:
            if register in [pub_test_register, set_test_register]:
                continue
            if register.method == "pub":
                [test_id, temp_fims_commands, temp_expected_messages] = \
                    test_basics(test_id, pub_test_register, register)
                fims_commands[1].extend(temp_fims_commands)
                expected_messages[0].update(temp_expected_messages)
            if register.method == "set":
                [test_id, temp_fims_commands, temp_expected_messages] = \
                    test_basics(test_id, set_test_register, register)
                fims_commands[0].extend(temp_fims_commands)
                expected_messages[1].update(temp_expected_messages)
        ALL_FIMS_COMMANDS.append(fims_commands)
        ALL_EXPECTED_MESSAGES.append(expected_messages)

def get_test_register_sets():
    '''
    Find the registers that have "test" in their "id" field. Designate these as the test registers
    for the designated config files.
    '''
    for registers in ALL_CONFIG_REGISTER_SETS:
        test_registers = [-1,-1]
        for register in registers:
            if 'test' in register.client_id and register.register_type in analog_input_registers:
                test_registers[0] = register
            if 'test' in register.client_id and register.register_type in analog_output_registers:
                test_registers[1] = register
        TEST_REGISTER_SETS.append(test_registers)

def get_config_pairs():
    '''
    Read in the config files using the filenames retrieved in get_config_file_pairs.
    Parse the files into ConfigFile structures, including extracting components,
    register groups, and registers. Match server and client registers to form MergeRegisters
    wherever possible.
    '''
    for [client_file, server_file] in ALL_CONFIG_FILE_PAIRS:
        client_config = ConfigFile(client_file)
        server_config = ConfigFile(server_file)
        for comp_idx, component in enumerate(client_config.components):
            if len(server_config.components) <= comp_idx:
                break
            registers = []
            for register_type in component.register_dict:
                serv_comp = server_config.components[comp_idx]
                if register_type in serv_comp.register_dict:
                    for offset in component.register_dict[register_type]:
                        if offset in serv_comp.register_dict[register_type]:
                            client_register = component.register_dict[register_type][offset]
                            server_register = serv_comp.register_dict[register_type][offset]
                            registers.append(MergedRegister(client_register, server_register))
            ALL_CONFIG_REGISTER_SETS.append(registers)

def get_config_file_pairs():
    '''
    Read all config files in the config directory and put them in pairs based on
    their names (matching server and clients should be named similarly)
    '''
    all_config_files = []
    for file in os.listdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}"):
        if file.lower().endswith('.json'):
            all_config_files.append(file)
    changes_to_all_config_files = True
    while changes_to_all_config_files:
        if len(all_config_files) > 0:
            for file in all_config_files.copy():
                if 'client' in file:
                    matching_server_file = file.replace('client','server')
                    if matching_server_file in all_config_files:
                        ALL_CONFIG_FILE_PAIRS.append([file,matching_server_file])
                        all_config_files.remove(file)
                        all_config_files.remove(matching_server_file)
                        changes_to_all_config_files = True
                        break
                if 'server' in file:
                    matching_client_file = file.replace('server','client')
                    if matching_client_file in all_config_files:
                        ALL_CONFIG_FILE_PAIRS.append([matching_client_file,file])
                        all_config_files.remove(file)
                        all_config_files.remove(matching_client_file)
                        changes_to_all_config_files = True
                        break
                changes_to_all_config_files = False
        else:
            changes_to_all_config_files = False


if __name__ == '__main__':
    #messages = {}
    for folder in DIRS:
        if not os.path.exists(folder):
            os.mkdir(folder)
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    gen_commands()
    build_output_files()
