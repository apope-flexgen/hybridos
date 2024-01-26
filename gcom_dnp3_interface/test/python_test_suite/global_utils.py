from git_info import GitInfo
from timestamp import TimeStamp

# Modify these as you need to.
INTERFACE = "gcom_dnp3".lower()
CLIENT_CONTAINER = "client_container"
SERVER_CONTAINER = "server_container"
DOCKER_PYTHON_SCRIPT_DIR = "/home/docker/hybridos/gcom_dnp3_interface/test/python_test_suite" # this should be vlinked to the LOCAL_PYTHON_SCRIPT_DIR
LOCAL_PYTHON_SCRIPT_DIR = "C:/flexgen/hybridos/gcom_dnp3_interface/test/python_test_suite"
CLIENT_CONTAINER_TITLE = "Client" # Easiest thing to do is to add the following to ~/.bashrc:  PROMPT_COMMAND='echo -en "\033]0;Server\a"'
SERVER_CONTAINER_TITLE = "Server" # Easiest thing to do is to add the following to ~/.bashrc:  PROMPT_COMMAND='echo -en "\033]0;Client\a"'
CLIENT_IP_AND_PORT = "localhost:4041"
SERVER_IP_AND_PORT = "localhost:4040"

# All you really need is the configs directory.
# The other directories will be made for you.
CONFIGS_DIR = "configs"
TEST_CONFIG_DIR = "test_configs"
TEST_OUTPUT_DIR = "test_output"
TEST_LOG_DIR = "logs"
TEST_CONSOLE_OUTPUT_DIR="console"
dirs = [CONFIGS_DIR, TEST_CONFIG_DIR, TEST_OUTPUT_DIR, TEST_LOG_DIR, TEST_CONSOLE_OUTPUT_DIR]

# Types of registers
analog_output_registers = ['AnOPInt32', 'AnOPInt16', 'AnOPF32', 'analogOS', 'holding_registers']
binary_output_registers = ['CROB', 'binaryOS', 'coils']
analog_input_registers = ['analog', 'input_registers']
binary_input_registers = ['binary', 'discrete_inputs']

# Global variables, populated in gen_test_script.py
all_config_file_pairs = []  # list of [client_config_filename, server_config_filename] pairs
all_config_pairs = []       # list of [ConfigFile(client_config_filename), ConfigFile(server_config_filename)] pairs
all_config_register_sets = [] # list of register lists (i.e. [MergedRegister(client_register,server_register), MergedRegister(client_register,server_register),...]) for each config file pair
test_register_sets = []     # list of test registers [MergedRegister(pub_test_register),MergedRegister(set_test_register)] for each config pair
all_fims_commands = []      # list of [client_commands, server_commands] for each register set
all_expected_messages = []  # list of [{client_test_num:test_num_expected_message_list},{server_test_num:test_num_expected_message_list}]
commands_by_test_id = {}    # dictionary of all commands by test_id
output_file_content = []
output_filenames = []
received_messages_client = []
received_messages_server = []
new_test_cases = True
timestamp = TimeStamp()
git_info = GitInfo()

# Constants. Do not change.
MIN_INT_16 = -32768
MAX_INT_16 = 32767
MAX_INT_32 = 2147483647
MIN_INT_32 = -2147483648
MAX_INT_64 = 9223372036854775807
MIN_INT_64 = -9223372036854775808
MIN_UINT_16 = 0
MAX_UINT_16 = 65535
MAX_UINT_32 = 4294967295
MIN_UINT_32 = 0
MAX_UINT_64 = 18446744073709551615
MAX_FLOAT_32 = 3.402823466 * 10**38
MIN_FLOAT_32 = 1.175494351 * 10**-38
MAX_FLOAT_64 = 1.7976931348623158 * 10**308
MIN_FLOAT_64 = 2.2250738585072014 * 10**-308