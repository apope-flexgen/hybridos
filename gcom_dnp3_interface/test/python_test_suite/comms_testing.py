'''
The comms_testing module contains all of the utility functions for
setting up docker testing with modbus and dnp3 client/server
'''
import os
import sys
import subprocess
import threading
import time
import json
import re
import requests
from custom_thread import CustomThread
from gen_test_script import get_config_file_pairs, get_config_pairs, \
get_test_register_sets, gen_commands, build_output_files, load_output_files
from run_test_case import run_test_case
from timestamp import timestamp
try:
    from user_global_utils import INTERFACE, LOCAL_PYTHON_SCRIPT_DIR, \
    TEST_CONSOLE_OUTPUT_DIR, SERVER_CONTAINER, CLIENT_CONTAINER, \
    DOCKER_PYTHON_SCRIPT_DIR, CLIENT_PORT, SERVER_PORT, LOCAL_HYBRIDOS_DIR, \
    CONFIGS_DIR, DIRS, PATH_TO_SSH_TOKENS, USE_LATEST_RELEASE
except ImportError:
    from global_utils import INTERFACE, LOCAL_PYTHON_SCRIPT_DIR, \
    TEST_CONSOLE_OUTPUT_DIR, SERVER_CONTAINER, CLIENT_CONTAINER, \
    DOCKER_PYTHON_SCRIPT_DIR, CLIENT_PORT, SERVER_PORT, LOCAL_HYBRIDOS_DIR, \
    CONFIGS_DIR, DIRS, PATH_TO_SSH_TOKENS, USE_LATEST_RELEASE

def subprocess_run_quiet(args):
    '''
    Call subprocess.run without printing anything to stdout.
    '''
    return subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)

def run_server(filename: str, stop: threading.Event) -> None:
    '''
    Run the modbus/dnp3 server and write the output to file.
    '''
    if "gcom_modbus" in INTERFACE:
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{filename}_{timestamp.file_fmt}.txt",
                  'w', newline='\n', encoding="utf-8") as file:
            with subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}",
                                   f"{INTERFACE}_server",
                                   f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}.json", "bypass"], \
                                   stdout=file, stderr=subprocess.STDOUT, text=True, bufsize=1) \
                                   as console_output_stream:
                while not stop.is_set():
                    try:
                        console_output_stream.communicate(timeout=1)
                    except subprocess.TimeoutExpired:
                        pass
    else:
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{filename}_{timestamp.file_fmt}.txt",
                  'w', newline='\n', encoding="utf-8") as file:
            with subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}",
                                   f"{INTERFACE}_server", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}.json"],
                                   stdout=file, stderr=subprocess.STDOUT, text=True, bufsize=1) \
                                   as console_output_stream:
                while not stop.is_set():
                    try:
                        console_output_stream.communicate(timeout=1)
                    except subprocess.TimeoutExpired:
                        pass

def run_client(filename: str, stop: threading.Event) -> None:
    '''
    Run the modbus/dnp3 client and write the output to file.
    '''
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{filename}_{timestamp.file_fmt}.txt",
              'w', newline='\n', encoding="utf-8") as file:
        with subprocess.Popen(["docker", "exec", "-it", f"{CLIENT_CONTAINER}",
                               f"{INTERFACE}_client", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}.json"],
                               stdout=file, stderr=subprocess.STDOUT, text=True, bufsize=1) \
                               as console_output_stream:
            while not stop.is_set():
                try:
                    console_output_stream.communicate(timeout=1)
                except subprocess.TimeoutExpired:
                    pass

def start_containers(client_file: str, server_file: str) -> (CustomThread, CustomThread):
    '''
    Start the client and server containers to run in the background. Also set up the
    Flask application on both client and server containers to accept commands from
    the main program.
    '''
    print(f"Starting {SERVER_CONTAINER}...")
    subprocess_run_quiet(["docker", "start", SERVER_CONTAINER])
    print(f"Starting {CLIENT_CONTAINER}...")
    subprocess_run_quiet(["docker", "start", CLIENT_CONTAINER])
    time.sleep(3)
    print("Starting testing...")
    server_container = CustomThread(target=run_server, args=(server_file))
    server_container.start()
    subprocess.run(["docker", "exec", "-dit", SERVER_CONTAINER,
                    "python3", f"{DOCKER_PYTHON_SCRIPT_DIR}/flask_app.py"], check=False)
    time.sleep(3)
    client_container = CustomThread(target=run_client, args=(client_file))
    client_container.start()
    subprocess.run(["docker", "exec", "-dit", CLIENT_CONTAINER,
                    "python3", f"{DOCKER_PYTHON_SCRIPT_DIR}/flask_app.py"], check=False)
    time.sleep(3)
    return client_container, server_container

def kill_containers(client_container, server_container) -> None:
    '''
    Kill the client and server containers so that we can close everything out.
    '''
    pkill_client_name = f"{INTERFACE}_client"
    pkill_server_name = f"{INTERFACE}_server"
    if len(pkill_client_name) > 15: # if the process name is > 15 characters, pkill is weird and doesn't work
        pkill_client_name = pkill_client_name[:15]
        pkill_server_name = pkill_server_name[:15]
    client_kill = f"pkill {pkill_client_name}\n"
    server_kill = f"pkill {pkill_client_name}\n"
    print(f"Killing {INTERFACE}_client...")
    client_container.join(1, requests.get, f"http://localhost:{CLIENT_PORT}" + \
                          f"/docker/run_command?command={client_kill}")
    print(f"Killing {INTERFACE}_server...")
    server_container.join(1, requests.get, f"http://localhost:{SERVER_PORT}" + \
                          f"/docker/run_command?command={server_kill}")
    print(f"Stopping {CLIENT_CONTAINER}...")
    subprocess_run_quiet(["docker","stop",CLIENT_CONTAINER])
    print(f"Stopping {SERVER_CONTAINER}...")
    subprocess_run_quiet(["docker","stop",SERVER_CONTAINER])


def gen_test_cases(new_test_cases=True) -> None:
    '''
    Use functions from gen_test_cases.py to generate new test cases, if
    relevant. Also reads in the configs and loads expected test output.
    '''
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    if new_test_cases:
        print("Generating new test cases...")
        gen_commands()
        build_output_files()
        print("Done generating test cases.")
    else:
        load_output_files()

def gen_directories() -> None:
    '''
    Make sure all directories needed for testing already exist.
    '''
    for folder in DIRS:
        if not os.path.exists(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}"):
            os.mkdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}")

def run_client_test_case(test_id: str, test_case: dict) -> (bool, str):
    '''
    Run a single test case on the client. Commands get sent to server and client
    output is read from fims_listen.
    '''
    return run_test_case("client", test_id, test_case, f"localhost:{SERVER_PORT}")

def run_server_test_case(test_id: str, test_case: dict) -> (bool, str):
    '''
    Run a single test case on the server. Commands get sent to client and server
    output is read from fims_listen.
    '''
    return run_test_case("server", test_id, test_case, f"localhost:{CLIENT_PORT}")

def build_containers():
    '''
    Build the client and server containers and make sure they have Flask.
    (I couldn't install flask as part of the Dockerfile for some reason...)
    '''
    print("Building containers...")
    subprocess_run_quiet(["docker", "stop", CLIENT_CONTAINER])
    subprocess_run_quiet(["docker", "stop", SERVER_CONTAINER])
    subprocess_run_quiet(["docker", "rm", CLIENT_CONTAINER])
    subprocess_run_quiet(["docker", "rm", SERVER_CONTAINER])
    subprocess_run_quiet(["docker-compose", "up", "-d", "--build"])
    print("Containers built.")
    print("Installing Flask...")
    subprocess_run_quiet(["docker", "exec", CLIENT_CONTAINER, "pip3", "install", "flask"])
    subprocess_run_quiet(["docker", "exec", SERVER_CONTAINER, "pip3", "install", "flask"])
    subprocess_run_quiet(["docker", "stop", CLIENT_CONTAINER])
    subprocess_run_quiet(["docker", "stop", SERVER_CONTAINER])
    print("Done installing Flask.")

def containers_exist() -> bool:
    '''
    Check if the containers needed for testing already exist. Assume they have the correct
    dependencies if they DO exist.
    '''
    result1 = subprocess_run_quiet(["docker", "ps", "-a", "--filter", f"name={CLIENT_CONTAINER}"])
    output1 = CLIENT_CONTAINER in result1.stdout
    result2 = subprocess_run_quiet(["docker", "ps", "-a", "--filter", f"name={SERVER_CONTAINER}"])
    output2 = SERVER_CONTAINER in result2.stdout
    return output1 and output2

def update_string_in_file(file_path, old_string, new_string):
    '''
    Look for old_string in the specified file. Replace all instances of old_string
    with new_string and overwrite the current file with the changes.
    '''
    try:
        # Read the content of the file
        with open(file_path, 'r', encoding="utf-8") as file:
            content = file.read()

        # Use regular expression for replacement
        updated_content = content.replace(old_string, new_string)

        # Write the updated content back to the file
        with open(file_path, 'w', encoding="utf-8") as file:
            file.write(updated_content)

    except FileNotFoundError:
        print(f'File not found: {file_path}')
    except Exception as some_exception:
        print(f'Error: {some_exception}')


def update_ip_address_in_json(file_path, new_ip_address):
    '''
    Look for an "ip_address" field in the specified json file. Replace the value
    of the json field with the new_ip_address (as a string).
    '''
    try:
        # Read the content of the file
        with open(file_path, 'r', encoding="utf-8") as file:
            content = file.read()

        # Use regular expression to find and update the "ip_address" field
        pattern = re.compile(r'"ip_address":\s*"[^"]+"')
        content = pattern.sub(f'"ip_address": "{new_ip_address}"', content)

        # Convert the updated content back to a Python dictionary
        updated_json = json.loads(content)

        # Write the updated JSON back to the file
        with open(file_path, 'w', encoding="utf-8") as file:
            json.dump(updated_json, file, indent=2)

    except FileNotFoundError:
        print(f'File not found: {file_path}')
    except Exception as some_exception:
        print(f'Error: {some_exception}')

def extract_nth_instance(file_path, target_string, instance_num):
    '''
    Find the nth instance of a target string and return the full line with
    that string. Used for extracting ip_address from the docker-compose.yml
    file.
    '''
    try:
        # Open the file
        with open(file_path, 'r', encoding="utf-8") as file:
            current_count = 0

            # Iterate through each line
            for line in file:
                # Count occurrences of the target string in the line
                occurrences_in_line = line.count(target_string)

                # Update the overall count
                current_count += occurrences_in_line

                # If the nth occurrence is found, return the line
                if current_count >= instance_num:
                    return line
        return None

    except FileNotFoundError:
        print(f'Error: File not found: {file_path}')
        return None
    except Exception as some_exception:
        print(f'Error: {some_exception}')
        return None

def add_line_after(file_path, search_content, new_line):
    '''
    Add a line with specific content after the line that contains the
    search content.
    '''
    try:
        with open(file_path, 'r', encoding="utf-8") as file:
            lines = file.readlines()

        with open(file_path, 'w', encoding="utf-8") as file:
            for line in lines:
                file.write(line)
                if search_content in line:
                    file.write(new_line + '\n')

    except FileNotFoundError:
        print(f'File not found: {file_path}')
    except Exception as some_exception:
        print(f'Error: {some_exception}')

def update_file_contents():
    '''
    Update docker-compose.yml, Dockerfile, and all client config files with the
    user-specific information from either user_global_utils.py or global_utils.py
    '''
    print("Updating files...")
    tmp_dir = LOCAL_HYBRIDOS_DIR.replace("C:", "//c")
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                          "//c/flexgen/hybridos", tmp_dir)
    tmp_dir = PATH_TO_SSH_TOKENS.replace("C:", "//c")
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                          "//c/Users/StephanieReynolds/.ssh", tmp_dir)
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/Dockerfile",
                          f"RUN yum install -y {INTERFACE}_interface\n", "")
    if USE_LATEST_RELEASE:
        add_line_after(f"{LOCAL_PYTHON_SCRIPT_DIR}/Dockerfile",
                       "RUN yum install -y fims", f"RUN yum install -y {INTERFACE}_interface")
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                          "4040", f"{SERVER_PORT}")
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                          "4041", f"{CLIENT_PORT}")
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                          "client_container", CLIENT_CONTAINER)
    update_string_in_file(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                          "server_container", SERVER_CONTAINER)
    server_ip = extract_nth_instance(f"{LOCAL_PYTHON_SCRIPT_DIR}/docker-compose.yml",
                                     "ipv4_address:", 1).replace("ipv4_address: ","").strip()
    for config in os.listdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}"):
        if "client" in config:
            file_path = f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{config}"
            update_ip_address_in_json(file_path, f'{server_ip}')
    print("Done updating files.")

def check_for_binaries():
    '''
    Make sure that we have the necessary binaries needed to volume mount them into the
    containers that we will create. (To make sure we have the latest build.)
    '''
    client_bin = f"{LOCAL_HYBRIDOS_DIR}/{INTERFACE}_interface/build/release/{INTERFACE}_client"
    server_bin = f"{LOCAL_HYBRIDOS_DIR}/{INTERFACE}_interface/build/release/{INTERFACE}_server"
    if not os.path.exists(client_bin):
        print(f"Cannot find {INTERFACE}_client binary in build/release " + \
              "directory! Please run 'make' first!")
        sys.exit(1)

    if not os.path.exists(server_bin):
        print(f"Cannot find {INTERFACE}_server binary in build/release " + \
              "directory! Please run 'make' first!")
        sys.exit(1)
