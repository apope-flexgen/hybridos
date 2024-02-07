import subprocess
from global_utils import *
import time
import os
from process_fims_messages import *
import signal
from gen_test_script import *
from check_test_case import *
from check_uris import *
from check_test_case import *
import argparse
from test_python_output import *

# Define the command line arguments
parser = argparse.ArgumentParser(description='Run dnp3 test')
parser.add_argument('-n', '--new', action='store_true') 

args = parser.parse_args()

new_test_cases = args.new

for folder in dirs:
    if not os.path.exists(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}"):
        os.mkdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}")

dnp3_server = None
dnp3_client = None
server_script = None
client_script = None
fims_listen_server = None
fims_listen_client = None

def sig_handler(signum=None, frame=None):
    global dnp3_server, dnp3_client, server_script, client_script, fims_listen_client, fims_listen_server

    if dnp3_server is not None:
        if isinstance(dnp3_server, subprocess.Popen):
            dnp3_server.kill()
        else:
            print("dnp3_server is not a subprocess and is not None.")

    if dnp3_client is not None:
        if isinstance(dnp3_client, subprocess.Popen):
            dnp3_client.kill()
        else:
            print("dnp3_client is not a subprocess and is not None.")

    if server_script is not None:
        if isinstance(server_script, subprocess.Popen):
            server_script.kill()
        else:
            print("server_script is not a subprocess and is not None.")

    if client_script is not None:
        if isinstance(client_script, subprocess.Popen):
            client_script.kill()
        else:
            print("client_script is not a subprocess and is not None.")

    if fims_listen_server is not None:
        if isinstance(fims_listen_server, subprocess.Popen):
            fims_listen_server.kill()
        else:
            print("fims_listen_server is not a subprocess and is not None.")

    if fims_listen_client is not None:
        if isinstance(fims_listen_client, subprocess.Popen):
            fims_listen_client.kill()
        else:
            print("fims_listen_client is not a subprocess and is not None.")
    print("writing to server log")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{server_file}.log", 'w', newline='\n') as file:
        file.write(fims_listen_server_output)
    print("writing to client log")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{client_file}.log", 'w', newline='\n') as file:
        file.write(fims_listen_client_output)
    print("writing to server console output")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{server_file}.txt", 'w', newline='\n') as file:
        file_contents = dnp3_server.stdout.readlines()
        file.write("".join(file_contents))
    print("writing to client console output")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{client_file}.txt", 'w', newline='\n') as file:
        file_contents = dnp3_client.stdout.readlines()
        file.write("".join(file_contents))

signal.signal(signal.SIGINT, sig_handler)

server_files = []
client_files = []

test_timestamp=timestamp.file_fmt
expected_test_output = {}
if args.new:
    print("Generating new test cases")
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    gen_commands()
    build_output_files()
    write_command_files()
    write_expected_message_files()
    for (test_set_num, [client_filename, server_filename]) in enumerate(ALL_CONFIG_FILE_PAIRS):
        expected_test_output.update(ALL_EXPECTED_MESSAGES[test_set_num][0])
        expected_test_output.update(ALL_EXPECTED_MESSAGES[test_set_num][1])
    print("Done")
else:
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    for [client_filename, server_filename] in ALL_CONFIG_FILE_PAIRS:
        client_filename = "expected_fims_output_" + client_filename
        server_filename = "expected_fims_output_" + server_filename
        client_test_cases = parse_test_cases(client_filename)
        server_test_cases = parse_test_cases(server_filename)
        if client_test_cases != None and server_test_cases != None:
            ALL_EXPECTED_MESSAGES.append([client_test_cases,server_test_cases])
            expected_test_output.update(client_test_cases)
            expected_test_output.update(server_test_cases)

subprocess.run(f'docker start {SERVER_CONTAINER}')
time.sleep(3)
subprocess.run(f'docker start {CLIENT_CONTAINER}')
time.sleep(3)

messages = []
test_cases = {}
current_test_id = -1
passed_cases = 0
failed_cases = 0
failed_cases_list = []

test_result_output = ""
test_result_output += str(git_info)
date = f"Test date: {timestamp.print_fmt}"
print(date)
test_result_output += date + "\n"

for (test_set_num, [client_file,server_file]) in enumerate(ALL_CONFIG_FILE_PAIRS):
    client_file = client_file.replace(".json","")
    server_file = server_file.replace(".json","")
    pub_test_id = test_register_sets[test_set_num][0].client_id
    set_test_id = test_register_sets[test_set_num][1].server_id
    if INTERFACE == "gcom_modbus":
        dnp3_server = subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}", f"{INTERFACE}_server", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{server_file}.json", "bypass"],stdout=subprocess.PIPE, text=True)
        time.sleep(3)
        dnp3_client = subprocess.Popen(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", f"{INTERFACE}_client", "-c", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{client_file}.json"],stdout=subprocess.PIPE, text=True)
        time.sleep(3)
    else:
        dnp3_server = subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}", f"{INTERFACE}_server", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{server_file}.json"],stdout=subprocess.PIPE, text=True)
        time.sleep(3)
        dnp3_client = subprocess.Popen(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", f"{INTERFACE}_client", f"{DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{client_file}.json"],stdout=subprocess.PIPE, text=True)
        time.sleep(3)
    fims_listen_client_output = ""
    with subprocess.Popen(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", f"fims_listen"] ,stdout=subprocess.PIPE, text=True, bufsize=1) as fims_listen_client:
        server_script = subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}", f"sh", f"{DOCKER_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/{server_file}.sh"],stdout=subprocess.PIPE)
        message = ""
        while True:
            line = fims_listen_client.stdout.readline()
            if not line:
                break
            message += line
            if "Timestamp:" in line:
                messages.append(process_message(message))
                if isinstance(messages[-1],dict) and isinstance(messages[-1]['body'], dict) and pub_test_id in messages[-1]['body'] and current_test_id != messages[-1]['body'][pub_test_id]:
                    if current_test_id in test_cases:
                        result, output = check_test_case(str(current_test_id), expected_test_output[current_test_id], test_cases[current_test_id])
                        if result:
                            print(f"Test Case {current_test_id}: Pass")
                            test_result_output += f"Test Case {current_test_id}: Pass\n"
                            passed_cases += 1
                        else:
                            failed_cases += 1
                            failed_cases_list.append(current_test_id)
                            test_result_output += output + "\n"
                    current_test_id = messages[-1]['body'][pub_test_id]
                    if isinstance(current_test_id, int) or isinstance(current_test_id, float):
                        current_test_id = int(current_test_id)
                        test_cases[current_test_id] = [messages[-1]]
                    else:
                        print("error getting test id:", current_test_id)
                elif isinstance(messages[-1],dict) and current_test_id in test_cases:
                    test_cases[current_test_id].append(messages[-1])
                message = ""
            if server_script.poll() != None or not line:
                fims_listen_client.kill()
                line = fims_listen_client.stdout.readlines()
                break 
            fims_listen_client_output += line
        fims_listen_client.kill()
    subprocess.run(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", "pkill", f"fims_listen"])
    
    fims_listen_server_output = ""
    with subprocess.Popen(["docker", "exec", "-it", f"{SERVER_CONTAINER}", f"fims_listen"] ,stdout=subprocess.PIPE, text=True, bufsize=1) as fims_listen_server:
        client_script = subprocess.Popen(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", f"sh", f"{DOCKER_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/{client_file}.sh"],stdout=subprocess.PIPE)
        message = ""
        while True:
            line = fims_listen_server.stdout.readline()
            message += line
            if "Timestamp:" in line:
                messages.append(process_message(message))
                if isinstance(messages,list) and isinstance(messages[-1],dict) and set_test_id in messages[-1]['uri'] and ((isinstance(messages[-1]['body'],int) and current_test_id != messages[-1]['body']) or ('value' in messages[-1]['body'] and current_test_id != messages[-1]['body']['value'])):
                    if current_test_id in test_cases:
                        result, output = check_test_case(str(current_test_id), expected_test_output[current_test_id], test_cases[current_test_id])
                        if result:
                            print(f"Test Case {current_test_id}: Pass")
                            test_result_output += f"Test Case {current_test_id}: Pass\n"
                            passed_cases += 1
                        else:
                            failed_cases += 1
                            failed_cases_list.append(current_test_id)
                            test_result_output += output + "\n"
                    
                    current_test_id = messages[-1]['body']
                    if isinstance(current_test_id, int) or isinstance(current_test_id, float):
                        current_test_id = int(current_test_id)
                        test_cases[current_test_id] = [messages[-1]]
                    elif isinstance(current_test_id, dict) and 'value' in current_test_id:
                        current_test_id = current_test_id['value']
                        if isinstance(current_test_id, int) or isinstance(current_test_id, float):
                            current_test_id = int(current_test_id)
                            test_cases[current_test_id] = [messages[-1]]
                        else:
                            print("error getting test id:", current_test_id)
                    else:
                        print("error getting test id:", current_test_id) 
                        current_test_id = -1
                elif isinstance(messages,list) and isinstance(messages[-1],dict) and current_test_id in test_cases:
                    test_cases[current_test_id].append(messages[-1])
                if current_test_id == -1:
                        fims_listen_server.kill()
                        break
                message = ""
            if client_script.poll() != None or not line:
                fims_listen_server.kill()
                line = fims_listen_server.stdout.readlines()
                break  
            fims_listen_server_output += line
        fims_listen_server.kill()
    subprocess.run(["docker", "exec", "-it", f"{SERVER_CONTAINER}", "pkill", f"fims_listen"])
    dnp3_server.kill()
    dnp3_client.kill()
    subprocess.run(["docker", "exec", "-it", f"{SERVER_CONTAINER}", "pkill", f"{INTERFACE}_server"])
    subprocess.run(["docker", "exec", "-it", f"{CLIENT_CONTAINER}", "pkill", f"{INTERFACE}_client"])
    print("writing to server log")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{server_file}_{test_timestamp}.log", 'w', newline='\n') as file:
        file.write(fims_listen_server_output)
    print("writing to client log")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{client_file}_{test_timestamp}.log", 'w', newline='\n') as file:
        file.write(fims_listen_client_output)
    print("writing to server console output")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{server_file}_{test_timestamp}.txt", 'w', newline='\n') as file:
        server_output = dnp3_server.stdout.read()
        file.write(server_output)
    print("writing to client console output")
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/{client_file}_{test_timestamp}.txt", 'w', newline='\n') as file:
        client_output = dnp3_client.stdout.read()
        file.write(client_output)

    
    
    print(f"\n\nPassed: {passed_cases}")
    test_result_output += f"\n\nPassed: {passed_cases}\n"
    print(f"Failed: {failed_cases}")
    test_result_output += f"Failed: {failed_cases}\n"
    failed_cases_list_str = "["
    for case in failed_cases_list:
        failed_cases_list_str += str(case)
        failed_cases_list_str += ", "
    failed_cases_list_str = "]"
    test_result_output += f"Failed: {failed_cases_list_str}\n"
    if len(failed_cases_list) > 0:
        print(f"Failed: {failed_cases_list}", sep=', ')
    
    test_result_output += check_message_ids(messages)


    [client_times, server_times] = calc_time_between_messages(messages)
    test_result_output += "\nTime between pubs\n"
    print("Time between pubs")
    for key in client_times:
        print(f"{key}:\n\tAverage: {client_times[key]['average_time_delta']}\n\tMin: {client_times[key]['min_time_delta']}\n\tMax: {client_times[key]['max_time_delta']}")
        test_result_output += f"{key}:\n\tAverage: {client_times[key]['average_time_delta']}\n\tMin: {client_times[key]['min_time_delta']}\n\tMax: {client_times[key]['max_time_delta']}\n"
    
    with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_CONSOLE_OUTPUT_DIR}/test_results.txt", 'w', newline='\n') as file:
        file.write(test_result_output)
