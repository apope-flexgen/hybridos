'''
The run_test_case module contains the code necessary to run a single test case
on either dnp3 or modbus server/client.
'''
import time
import subprocess
import threading
import requests
try:
    from user_global_utils import LOCAL_PYTHON_SCRIPT_DIR, TEST_LOG_DIR
except ImportError:
    from global_utils import LOCAL_PYTHON_SCRIPT_DIR, TEST_LOG_DIR
from check_test_case import check_test_case
from process_fims_messages import process_message
from timestamp import timestamp

RECEIVED_MESSAGES_CLIENT = []
RECEIVED_MESSAGES_SERVER = []

def capture_fims_listen(message_list: str, container: str, filename: str,
                        stop: threading.Event) -> None:
    '''
    Capture a fims listen output on a given container.
    '''
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{filename}_{timestamp.file_fmt}.log",
              'w', newline='\n', encoding="utf-8") as file:
        with subprocess.Popen(["docker", "exec", "-it", container, "fims_listen"],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              universal_newlines=True, bufsize=1) as fims_listen_stream:
            message = ""
            kill = lambda process : process.kill()
            while not stop.is_set():
                my_timer = threading.Timer(5, kill, [fims_listen_stream])
                try:
                    my_timer.start()
                    line = fims_listen_stream.stdout.readline()
                    if line is not None:
                        file.write(line)
                        message += line
                        if "Timestamp:" in line:
                            if message_list == "server":
                                RECEIVED_MESSAGES_SERVER.append(process_message(message))
                            else:
                                RECEIVED_MESSAGES_CLIENT.append(process_message(message))
                            message = ""
                finally:
                    my_timer.cancel()
            subprocess.run(["docker", "exec", container, "/bin/bash",
                            "-c", "pkill -o fims_listen"], check=True)
            fims_listen_stream.stdout.close()

def run_commands(commands: list, request_ip_and_port: str) -> None:
    '''
    Run a list of commands sequentially, using a command execution function.
    '''
    for command in commands:
        command = command.replace("+","%2B")
        requests.get(f"http://{request_ip_and_port}/docker/run_command?command={command}")

def run_test_case(message_list: str, test_id: str, test_case: dict,
                  request_ip_and_port: str) -> (bool, str):
    '''
    Test case must have:
    - Commands
    - Expected (list of messages)
        - Method (optional)
        - URI (required)
        - Body (semi-optional; not all fields required)
            - Fields can have:
                - value
                - tolerance
                - reject_values
        - Offset_ms (time since start of test)

    Test case dictionary is modified to include:
    - Commands
    - Expected
    - Actual
    - Result
    - Git info
    - Test timestamp
    '''
    try:
        commands = test_case['commands']
        expected_messages = test_case['expected']
    except KeyError:
        pass
    if message_list == "server":
        message_list_len = len(RECEIVED_MESSAGES_SERVER)
    else:
        message_list_len = len(RECEIVED_MESSAGES_CLIENT)
    run_commands(commands, request_ip_and_port)
    time.sleep(0.1)
    if message_list == "server":
        actual_messages = RECEIVED_MESSAGES_SERVER[message_list_len:]
    else:
        actual_messages = RECEIVED_MESSAGES_CLIENT[message_list_len:]
    test_case['actual'] = actual_messages
    result, error_message = check_test_case(test_id, expected_messages, actual_messages)
    if result:
        test_case['result'] = "PASS"
    else:
        test_case['result'] = f"FAIL:\n{error_message}"

    return result, error_message
