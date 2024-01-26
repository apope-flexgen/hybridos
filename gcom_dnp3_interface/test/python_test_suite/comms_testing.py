import os
import subprocess
import threading
import time
from custom_thread import CustomThread
from gen_test_script import *
from process_fims_messages import *
from run_test_case import *
from global_utils import *


            

def run_server(filename: str, stop: threading.Event) -> None:
    if "gcom_modbus" in INTERFACE:
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{filename}_{timestamp.file_fmt}.txt", 'w', newline='\n') as file:
            with subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}", f"{INTERFACE}_server", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}.json", "bypass"] ,stdout=file, stderr=subprocess.STDOUT, text=True, bufsize=1) as console_output_stream:
                while not stop.is_set():
                    try:
                        console_output_stream.communicate(timeout=1)
                    except subprocess.TimeoutExpired:
                        pass
    else:
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{filename}_{timestamp.file_fmt}.txt", 'w', newline='\n') as file:
            with subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}", f"{INTERFACE}_server", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}.json"] ,stdout=file, stderr=subprocess.STDOUT, text=True, bufsize=1) as console_output_stream:
                while not stop.is_set():
                    try:
                        console_output_stream.communicate(timeout=1)
                    except subprocess.TimeoutExpired:
                        pass

def run_client(filename: str, stop: threading.Event) -> None:
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{filename}_{timestamp.file_fmt}.txt", 'w', newline='\n') as file:
        with subprocess.Popen(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", f"{INTERFACE}_client", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}.json"] ,stdout=file, stderr=subprocess.STDOUT, text=True, bufsize=1) as console_output_stream:
            while not stop.is_set():
                try:
                    console_output_stream.communicate(timeout=1)
                except subprocess.TimeoutExpired:
                    pass

def start_containers(client_file: str, server_file: str) -> (CustomThread, CustomThread):
    subprocess.run(["docker", "start", SERVER_CONTAINER])
    subprocess.run(["docker", "start", CLIENT_CONTAINER])
    time.sleep(3)
    server_container = CustomThread(target=run_server, args=(server_file))
    server_container.start()
    subprocess.run(["docker", "exec", "-dit", SERVER_CONTAINER, "python3", f"{DOCKER_PYTHON_SCRIPT_DIR}/flask_app.py"])
    time.sleep(3)
    client_container = CustomThread(target=run_client, args=(client_file))
    client_container.start()
    subprocess.run(["docker", "exec", "-dit", CLIENT_CONTAINER, "python3", f"{DOCKER_PYTHON_SCRIPT_DIR}/flask_app.py"])
    time.sleep(3)
    return client_container, server_container

def kill_containers(client_container, server_container) -> None:
    client_kill = f"pkill {INTERFACE}_client\n"
    server_kill = f"pkill {INTERFACE}_server\n"
    client_container.join(requests.get, f"http://{CLIENT_IP_AND_PORT}/docker/run_command?command={client_kill}")
    server_container.join(requests.get, f"http://{SERVER_IP_AND_PORT}/docker/run_command?command={server_kill}")
    subprocess.run(["docker","stop",SERVER_CONTAINER])
    subprocess.run(["docker","stop",CLIENT_CONTAINER])
    

def gen_test_cases(new_test_cases=True) -> None:
    global output_file_content
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    if new_test_cases:
        print("Generating new test cases")
        gen_commands()
        build_output_files()
        print("Done")
    else:
        load_output_files()

def gen_directories() -> None:
    for folder in dirs:
        if not os.path.exists(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}"):
            os.mkdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}")

def run_client_test_case(test_id: str, test_case: dict) -> (bool, str):
    global received_messages_client
    return run_test_case("client", test_id, test_case, SERVER_IP_AND_PORT)

def run_server_test_case(test_id: str, test_case: dict) -> (bool, str):
    global received_messages_server
    return run_test_case("server", test_id, test_case, CLIENT_IP_AND_PORT)