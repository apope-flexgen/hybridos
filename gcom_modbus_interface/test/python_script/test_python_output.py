import re
import sys
import os
import json
from compare_messages import *
from datetime import datetime
from test_utils import *
from git_info import *
from check_uris import *
from gen_test_script import get_config_file_pairs, get_config_pairs, get_test_register_sets

def parse_test_cases(filename):
    if os.path.isfile(f"{LOCAL_PYTHON_SCRIPT_DIR}/{EXPECTED_TEST_OUTPUT_DIR}/{filename}"):
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{EXPECTED_TEST_OUTPUT_DIR}/{filename}", 'r') as file:
            try:
                json_text = json.load(file)
            except Exception as e:
                print(f'Could not load json file {filename}: {e}')
                return
        try:
            if 'test_cases' in json_text:
                test_cases = {}
                for key in json_text['test_cases']:
                    test_cases[int(key)] = json_text['test_cases'][key]
                return test_cases
        except Exception as e:
            print(f'Error when parsing json file {filename}: {e}')
            return

def parse_log_file(filename, test_register):
    # Initialize a list to store all messages
    messages = []

    # Open the file and read its contents
    if os.path.isfile(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{filename}"):
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/{filename}", 'r') as file:
            file_contents = file.read()
    else:
        return (None,None)

    # Split the file contents into individual messages based on blank lines
    message_blocks = re.split(r'\n\s*\n', file_contents)

    method_pattern = r"Method:\s*(\w+)"
    uri_pattern = r"Uri:\s*([\w/]+)"
    value_pattern = r"Body:\s*([^\n]+)"
    replyto_pattern = r"ReplyTo:\s*([^\n]+)"
    process_name_pattern = r"Process Name:\s*([^\n]+)"
    username_pattern = r"Username:\s*([^\n]+)"
    timestamp_pattern = r"Timestamp:\s*([^\n]+)"

    # Iterate through each message block
    for message_block in message_blocks:
        # Use regular expressions to find matches in each message block
        method_match = re.search(method_pattern, message_block)
        uri_match = re.search(uri_pattern, message_block)
        value_match = re.search(value_pattern, message_block)
        replyto_match = re.search(replyto_pattern, message_block)
        process_name_match = re.search(process_name_pattern, message_block)
        username_match = re.search(username_pattern, message_block)
        timestamp_match = re.search(timestamp_pattern, message_block)

        # Extract values if all matches are found
        if all((method_match, uri_match, value_match, replyto_match, process_name_match, username_match, timestamp_match)):
            method = method_match.group(1)
            uri = uri_match.group(1)
            value = value_match.group(1)
            replyto = replyto_match.group(1)
            process_name = process_name_match.group(1)
            username = username_match.group(1)
            timestamp_str = timestamp_match.group(1)

            # Assuming "value" could be in JSON format, try parsing it
            try:
                value = json.loads(value)
            except json.JSONDecodeError:
                pass  # Value is not JSON, keep it as a string

            try:
                timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S.%f').timestamp()
            except:
                timestamp = timestamp_str

            # Create a dictionary for the current message and add it to the list
            if (method == "pub" and "client" in filename) or (method == "set" and "server" in filename):
                message_dict = {
                    "method": method,
                    "uri": uri,
                    "body": value,
                    "replyto": replyto,
                    "process": process_name,
                    "username": username,
                    "timestamp": timestamp
                }
                messages.append(message_dict)
    message_dict = {}
    current_test_id = -1
    test_case_messages = None
    for idx, message in enumerate(messages):
        if (not isinstance(test_register, int)) and (test_register.server_id in message['uri'] or test_register.client_id in message['uri']):
            if isinstance(message['body'], int) and current_test_id != message['body']:
                if test_case_messages != None:
                    message_dict[current_test_id] = test_case_messages
                current_test_id = message['body']
                test_case_messages = []
            elif isinstance(message['body'], dict) and 'value' in message['body'] and current_test_id != message['body']['value']:
                if test_case_messages != None:
                    message_dict[current_test_id] = test_case_messages
                current_test_id = message['body']['value']
                test_case_messages = []
        elif isinstance(message['body'], dict):
            if ((not isinstance(test_register, int)) and test_register.client_id in message['body'] and current_test_id != message['body'][test_register.client_id]) or ((not isinstance(test_register, int)) and test_register.server_id in message['body'] and current_test_id != message['body'][test_register.server_id]):
                if test_case_messages != None:
                    message_dict[current_test_id] = test_case_messages
                test_case_messages = []
                if(test_register.client_id in message['body']):
                    current_test_id = message['body'][test_register.client_id]
                elif(test_register.server_id in message['body']):
                    current_test_id = message['body'][test_register.server_id]
            if test_case_messages != None:
                test_case_messages.append(message)
        else:
            print(type(message['body']))
            if test_case_messages != None:
                test_case_messages.append(message)
    if test_case_messages != None:
        message_dict[current_test_id] = test_case_messages

    output_filename = f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}/{filename.replace('.log', '_test_output.json')}"
    with open(output_filename, 'w', newline='\n') as file:
        file_contents = {"test_results": message_dict}
        file.write(json.dumps(file_contents, indent=4))
    return message_dict, messages

def test_output():
    global all_config_file_pairs, all_expected_messages, test_register_sets
    all_messages = []
    server_messages = []
    client_messages = []
    get_git_info()
    print(f"Test date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')}")
    for (test_set_num, [client_filename, server_filename]) in enumerate(all_config_file_pairs):
        client_filename = client_filename.replace(".json","") + ".log"
        server_filename = server_filename.replace(".json","") + ".log"
        pub_test_register = test_register_sets[test_set_num][0]
        set_test_register = test_register_sets[test_set_num][1]
        [message_dict, message_list] = parse_log_file(client_filename, pub_test_register)
        if message_dict != None:
            client_messages.append(message_dict)
            all_messages.append(message_list)
        [message_dict, message_list] = parse_log_file(server_filename, set_test_register)
        if message_dict != None:
            server_messages.append(message_dict)
            all_messages.append(message_list)
    for (test_set_num, [client_test_cases, server_test_cases]) in enumerate(all_expected_messages):
        passed_cases = 0
        failed_cases = 0
        failed_cases_list = []
        print("Client Test Cases\n")
        for test_case in client_test_cases:
            if client_messages != None and len(client_messages) > test_set_num:
                if (not test_case in client_messages[test_set_num]):
                    print(f"Test case {test_case}: Missing!")
                    failed_cases += 1
                    failed_cases_list.append(test_case)
                else:
                    for expected_message in client_test_cases[test_case]:
                        passed_subconditions = []
                        highest_return_value = -1
                        final_return_message = ""
                        one_match = False
                        if len(client_messages[test_set_num][test_case]) == 0:
                            passed_subconditions.append(False)
                            print(f"Test Case {test_case}:\nExpected: {expected_message['body']}\nGot: No messages for this test case!\n")
                        else:
                            for message in client_messages[test_set_num][test_case]:
                                [return_code, return_message] = compare_messages(test_case, expected_message, message)
                                if return_code == SUCCESS:
                                    one_match = True
                                else:
                                    if return_code >= highest_return_value:
                                        highest_return_value = return_code
                                        final_return_message = return_message
                        if one_match:
                            passed_subconditions.append(True)
                        else:
                            passed_subconditions.append(False)
                            if final_return_message!= "":
                                print(final_return_message)

                    if all(passed_subconditions):
                        passed_cases += 1
                    else:
                        failed_cases += 1
                        failed_cases_list.append(test_case)

    
        print(f"Passed: {passed_cases}")
        print(f"Failed: {failed_cases}")
        if len(failed_cases_list) > 0:
            print(f"Failed: {failed_cases_list}", sep=', ')

        passed_cases = 0
        failed_cases = 0
        failed_cases_list = []
        print("\nServer Test Cases")
        for test_case in server_test_cases:
            if server_messages != None and len(server_messages) > test_set_num:
                if (not test_case in server_messages[test_set_num]):
                    print(f"Test case {test_case}: Missing!")
                    failed_cases += 1
                    failed_cases_list.append(test_case)
                else:
                    for expected_message in server_test_cases[test_case]:
                        passed_subconditions = []
                        highest_return_value = -1
                        final_return_message = ""
                        one_match = False
                        if len(server_messages[test_set_num][test_case]) == 0:
                            passed_subconditions.append(False)
                            print(f"Test Case {test_case}:\nExpected: {expected_message['body']}\nGot: No messages for this test case!\n")
                        elif len(server_test_cases[test_case]) == 0:
                            if test_case in server_messages[test_set_num] and len(server_messages[test_set_num][test_case]) != 0:
                                print(f"Test case {test_case}:\nExpected: No messages for this test case!\nGot: {len(server_messages[test_case])} messages")
                                failed_cases += 1
                                failed_cases_list.append(test_case)
                            else:
                                passed_cases += 1
                        else:
                            for message in server_messages[test_set_num][test_case]:
                                [return_code, return_message] = compare_messages(test_case, expected_message, message)
                                if return_code == SUCCESS:
                                    one_match = True
                                else:
                                    if return_code >= highest_return_value:
                                        highest_return_value = return_code
                                        final_return_message = return_message
                        if one_match:
                            passed_subconditions.append(True)
                        else:
                            passed_subconditions.append(False)
                            if final_return_message != "":
                                print(final_return_message)
                    if all(passed_subconditions):
                        passed_cases += 1
                    else:
                        failed_cases += 1
                        failed_cases_list.append(test_case)
        
        print(f"Passed: {passed_cases}")
        print(f"Failed: {failed_cases}")
        if len(failed_cases_list) > 0:
            print(f"Failed: {failed_cases_list}", sep=', ')
        
        check_message_ids(all_messages[test_set_num])
    
        # uncomment if you care about average time between messages
        [client_times, server_times] = calc_time_between_messages(all_messages[test_set_num])
        print("Client times")
        for key in client_times:
            print(f"{key}:\n\tAverage: {client_times[key]['average_time_delta']}\n\tMin: {client_times[key]['min_time_delta']}\n\tMax: {client_times[key]['max_time_delta']}")
        # print("Server times")
        # for key in server_times:
        #     print(f"{key}:\n\tAverage: {server_times[key]['average_time_delta']}\n\tMin: {server_times[key]['min_time_delta']}\n\tMax: {server_times[key]['max_time_delta']}")



    
if __name__ == '__main__':
    global all_config_file_pairs, all_expected_messages
    for folder in dirs:
        if not os.path.exists(folder):
            os.mkdir(folder)
    get_config_file_pairs()
    get_config_pairs()
    get_test_register_sets()
    for [client_filename, server_filename] in all_config_file_pairs:
        client_filename = "expected_fims_output_" + client_filename
        server_filename = "expected_fims_output_" + server_filename
        client_test_cases = parse_test_cases(client_filename)
        server_test_cases = parse_test_cases(server_filename)
        if client_test_cases != None and server_test_cases != None:
            all_expected_messages.append([client_test_cases,server_test_cases])
    test_output()