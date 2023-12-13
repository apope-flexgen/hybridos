import os
import json
import re
import datetime
import sys

def get_uri_vars(filename):
    uris = {}
    if os.path.isfile(filename):
        with open(filename, 'r') as file:
            try:
                json_text = json.load(file)
            except Exception as e:
                print(f'Could not load json file {filename}: {e}')
                return
        if 'outputs' in json_text:
            for output in json_text['outputs']:
                if 'name' in json_text['outputs'][output]:
                    key = json_text['outputs'][output]['name']
                else:
                    key = output
                uri = json_text['outputs'][output]['uri']
                if not (uri in uris):
                    uris[uri] = []
                uris[uri].append(key)
    return uris

def check_message_ids(all_messages, expected_uris):
    uri_counts = {}
    
    good_messages = 0
    bad_messages = 0
    for message in all_messages:
        found_all = True
        missing_vars = []
        found_vars = []
        if 'process' in message and 'go_metrics' in message['process'] and 'uri' in message and 'body' in message and 'timestamp' in message and message['uri'] in expected_uris:
            uri = message['uri']
            body = message['body']
            if uri in expected_uris:
                if not (uri in uri_counts):
                    uri_counts[uri] = 0
                uri_counts[uri] += 1
                if isinstance(body, dict):
                    for id in expected_uris[uri]:
                        if not id in body:
                            found_all = False
                            missing_vars.append(id)
                        else:
                            found_vars.append(id)
                else: # raw value
                    pass

                if not found_all:
                    # print(f"Uri: {uri}\nTimestamp: {timestamp}\nMissing Ids: {missing_vars}\nFound Ids: {found_vars}\n")
                    bad_messages += 1
                else:
                    good_messages += 1
    for uri in expected_uris:
        if uri in uri_counts:
            print(f"{uri}: {uri_counts[uri]}")
        else:
            print(f"{uri}: 0")
    # print(f"\nComplete messages: {good_messages}")
    # print(f"Incomplete messages: {bad_messages}")
    # return_str = f"\nComplete messages: {good_messages}\nIncomplete messages: {bad_messages}\n"

def parse_log_file(filename):
    # Initialize a list to store all messages
    messages = []

    # Open the file and read its contents
    if os.path.isfile(filename):
        with open(filename, 'r') as file:
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
    return messages
    
if __name__ == '__main__':
    messages = parse_log_file(sys.argv[1])
    expected_uris = get_uri_vars(sys.argv[2])
    check_message_ids(messages, expected_uris)