'''
The user-specific variables used to generate the test environment and locate
test documents. Modify as necessary and then save the file as user_global_utils.py
so that changes are not overridden by git pull.
'''
# Modify these as you need to.
INTERFACE = "gcom_dnp3".lower()
CLIENT_CONTAINER = "client_container"
SERVER_CONTAINER = "server_container"
CONTAINER_HYBRIDOS_DIR = "/home/docker/hybridos" # this will be vlinked to the LOCAL_HYBRIDOS_DIR
LOCAL_HYBRIDOS_DIR = "C:/flexgen/hybridos"
DOCKER_PYTHON_SCRIPT_DIR = f"{CONTAINER_HYBRIDOS_DIR}/gcom_dnp3_interface/test/python_test_suite"
LOCAL_PYTHON_SCRIPT_DIR = f"{LOCAL_HYBRIDOS_DIR}/gcom_dnp3_interface/test/python_test_suite"
PATH_TO_SSH_TOKENS = "C:/Users/StephanieReynolds/.ssh"
CLIENT_PORT = 4041
SERVER_PORT = 4040

# Do you want to generate new tests or use what is in LOCAL_PYTHON_SCRIPT_DIR/test_configs already?
NEW_TEST_CASES = True

# If you have a local build in the LOCAL_HYBRIDOS_DIR/INTERFACE_DIR/build/release directory,
# change this to false
USE_LATEST_RELEASE = True

# All you really need is the configs directory.
# The other directories will be made for you.
CONFIGS_DIR = "configs"
TEST_CONFIG_DIR = "test_configs"
TEST_OUTPUT_DIR = "test_output"
TEST_LOG_DIR = "logs"
TEST_CONSOLE_OUTPUT_DIR="console"
DIRS = [CONFIGS_DIR, TEST_CONFIG_DIR, TEST_OUTPUT_DIR, TEST_LOG_DIR, TEST_CONSOLE_OUTPUT_DIR]
