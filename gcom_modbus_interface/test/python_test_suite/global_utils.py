'''
The user-specific variables used to generate the test environment and locate
test documents. Modify as necessary and then save the file as user_global_utils.py
so that changes are not overridden by git pull.
'''
# Modify these as you need to.
USER = "PhilWilshire"
#  HOST                                  COMTAINER
#  C:/Users/{USER}/flexgen/dockercont => /home/docker

LOCAL_DIR                   = f"C:/Users/{USER}/flexgen/dockercont"
DOCKER_DIR                 = "/home/docker"

HOST_TEST_DIR              = f"{LOCAL_DIR}/python_test_modbus/python_test_suite"
HOST_BUILD_DIR             = f"{LOCAL_DIR}/git/hybridos"
CONTAINER_TEST_DIR         = f"{DOCKER_DIR}/python_test_modbus/python_test_suite"
CONTAINER_BUILD_DIR        = f"{DOCKER_DIR}/git/hybridos"

INTERFACE                  = "gcom_modbus".lower()
CLIENT_CONTAINER           = "modbus_client"
SERVER_CONTAINER           = "modbus_server"

CONTAINER_HYBRIDOS_DIR     = CONTAINER_TEST_DIR   #"/home/docker/python_test_modbus" # this will be vlinked to the LOCAL_HYBRIDOS_DIR
LOCAL_HYBRIDOS_DIR         = HOST_BUILD_DIR       #f"C:/Users/{USER}/flexgen/dockercont/git/hybridos"
LOCAL_PYTHON_SCRIPT_DIR    = HOST_TEST_DIR        #C:/Users/PhilWilshire/flexgen/dockercont/python_test_modbus/python_test_suite"
DOCKER_PYTHON_SCRIPT_DIR   = CONTAINER_TEST_DIR   #f"{LOCAL_HYBRIDOS_DIR}/python_test_suite"

PATH_TO_SSH_TOKENS = f"C:/Users/{USER}/.ssh"
CLIENT_PORT = 4041
SERVER_PORT = 4040

# Do you want to generate new tests or use what is in LOCAL_PYTHON_SCRIPT_DIR/test_configs already?
NEW_TEST_CASES = False

# If you have a local build in the LOCAL_HYBRIDOS_DIR/INTERFACE_DIR/build/release directory,
# change this to false
USE_LATEST_RELEASE = False

# All you really need is the configs directory.
# The other directories will be made for you.
CONFIGS_DIR = "configs/basic_tests"
TEST_CONFIG_DIR = "test_configs/basic_tests"
TEST_OUTPUT_DIR = "test_output/basic_tests"
TEST_LOG_DIR = "logs/basic_tests"
TEST_CONSOLE_OUTPUT_DIR="console/basic_tests"
DIRS = [CONFIGS_DIR, TEST_CONFIG_DIR, TEST_OUTPUT_DIR, TEST_LOG_DIR, TEST_CONSOLE_OUTPUT_DIR]
