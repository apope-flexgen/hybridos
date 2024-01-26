from global_utils import *
import signal
import subprocess
import threading
from typing import Callable
from check_test_case import check_test_case
from custom_thread import CustomThread
from process_fims_messages import *

import time
import requests

def capture_fims_listen(message_list: str, container: str, filename: str, stop: threading.Event) -> None:
    global received_messages_client, received_messages_server
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{filename}_{timestamp.file_fmt}.log", 'w', newline='\n') as file:
        with subprocess.Popen(["docker", "exec", "-it", f"{container}", f"fims_listen"] ,stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, bufsize=1) as fims_listen_stream:
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
                                received_messages_server.append(process_message(message))
                            else:
                                received_messages_client.append(process_message(message))
                            message = ""
                finally:
                    my_timer.cancel()
            subprocess.run(["docker", "exec", container, "/bin/bash", "-c", "pkill -o fims_listen"])
            fims_listen_stream.stdout.close()

def run_commands(commands: list, request_ip_and_port: str) -> None:
    '''
    Run a list of commands sequentially, using a command execution function.
    '''
    for command in commands:
        command = command.replace("+","%2B")
        response = requests.get(f"http://{request_ip_and_port}/docker/run_command?command={command}")

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
    global received_messages_server, received_messages_client
    try:
        commands = test_case['commands']
        expected_messages = test_case['expected']
    except KeyError:
        pass
    if message_list == "server":
        message_list_len = len(received_messages_server)
    else:
        message_list_len = len(received_messages_client)
    run_commands(commands, request_ip_and_port)
    time.sleep(0.1)
    if message_list == "server":
        actual_messages = received_messages_server[message_list_len:]
    else:
        actual_messages = received_messages_client[message_list_len:]
    test_case['actual'] = actual_messages
    result, error_message = check_test_case(test_id, expected_messages, actual_messages)
    if result:
        test_case['result'] = "PASS"
    else:
        test_case['result'] = f"FAIL:\n{error_message}"
    
    test_case['git_branch'] = git_info.branch
    test_case['git_commit_hash'] = git_info.commit_hash
    test_case['git_commit_author'] = git_info.author

    return result, error_message
    



    
