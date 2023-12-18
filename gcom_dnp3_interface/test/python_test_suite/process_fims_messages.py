import re
import json
from datetime import datetime

def process_message(message):
    message_blocks = re.split(r'\n\s*\n', message)

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
            return message_dict