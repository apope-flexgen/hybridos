'''
The compare_messages module contains what is needed to compare an expected
message to an actual message in terms of uri, method, and values contained
within the body of the message.
'''
from numeric_limits import MAX_INT_16

# RETURN CODES
OTHER_ISSUE = -1
INCORRECT_FORMAT = 0
NO_METHOD = 1
NO_URI = 2
NO_BODY = 3
MISSING_URI = 4
MISSING_KEY = 5
INCORRECT_VALUE = 6

SUCCESS = 10

def check_message_format(test_id, expected_message, actual_message):
    '''
    Check_message_format checks that the arguments are in the proper format
    of fims messages (dictionaries with a uri, method, and body, at minimum)

    If invalid, return INCORRECT_FORMAT (0) and an error message indicating the
    issue with the message.
    '''
    if not isinstance(expected_message, dict):
        return_string = f"Test Case {test_id}:\n" + \
                        "Expected message is in incorrect format for parsing!\n"
        return INCORRECT_FORMAT, return_string
    if not isinstance(actual_message, dict):
        return_string = f"Test Case {test_id}: Actual message is in incorrect format for parsing!\n"
        return INCORRECT_FORMAT, return_string
    if not 'method' in expected_message:
        return_string = f"Test Case {test_id}:\nExpected message is missing fims method!\n"
        return NO_METHOD, return_string
    if not 'method' in actual_message:
        return_string = f"Test Case {test_id}: Actual message is missing fims method!\n"
        return NO_METHOD, return_string
    if not 'uri' in expected_message:
        return_string = f"Test Case {test_id}:\nExpected message is missing uri!\n"
        return NO_URI, return_string
    if not 'uri' in actual_message:
        return_string = f"Test Case {test_id}: Actual message is missing uri!\n"
        return NO_URI, return_string
    if not 'body' in expected_message:
        return_string = f"Test Case {test_id}:\nExpected message is missing body!\n"
        return NO_BODY, return_string
    if not 'body' in actual_message:
        return_string = f"Test Case {test_id}: Actual message is missing body!\n"
        return NO_BODY, return_string
    return SUCCESS, ""

def compare_messages(test_id, expected_message, actual_message):
    '''
    The compare_messages function compares an expected message to an
    actual message in terms of uri, method, and values contained
    within the body of the message.
    '''
    expected_value = 0
    tolerance = 0
    reject_values=[]
    [return_code, return_message] = check_message_format(test_id, expected_message, actual_message)
    if return_code != SUCCESS:
        return return_code, return_message
    if expected_message['uri'] == actual_message['uri']:
        return_value = SUCCESS
        return_string = ""
        for key in expected_message['body']:
            if isinstance(expected_message['body'][key], dict):
                if 'value' in expected_message['body'][key]:
                    expected_value = expected_message['body'][key]['value']
                if 'tolerance' in expected_message['body'][key]:
                    tolerance = expected_message['body'][key]['tolerance']
                else:
                    tolerance = 0
                if 'reject_values' in expected_message['body'][key]:
                    reject_values = expected_message['body'][key]['reject_values']
                else:
                    reject_values=[]
            else:
                expected_value = expected_message['body'][key]
                tolerance = 0
                reject_values=[]
            if isinstance(actual_message['body'],dict) and key in actual_message['body']:
                value = actual_message['body'][key]
                if (not isinstance(value, dict)) and (not isinstance(value, list)) and abs(value - expected_value) <= tolerance:
                    continue
                if value in reject_values:
                    return_value = INCORRECT_VALUE
                    return_string = f'Test Case {test_id}:\n' + \
                                    f'Expected: "{key}":{expected_value}\n' + \
                                    f'Got     : "{key}":{value}\n'
                elif isinstance(value, list):
                    if value == expected_value:
                        continue
                    return_value = INCORRECT_VALUE
                    return_string = f'Test Case {test_id}:\n' + \
                                    f'Expected: "{key}":{expected_value}\n' + \
                                    f'Got     : "{key}":{value}\n'
                elif isinstance(value, dict):
                    if 'value' in value:
                        value = value['value']
                        if abs(value - expected_value) <= tolerance:
                            continue
                        return_value = INCORRECT_VALUE
                        return_string = f'Test Case {test_id}:\n' + \
                                        f'Expected: "{key}":{expected_value}\n' + \
                                        f'Got     : "{key}":{value}\n'
                    else:
                        if abs(value - expected_value) <= tolerance:
                            continue
                        return_value = INCORRECT_VALUE
                        return_string = f'Test Case {test_id}:\n' + \
                                        f'Expected: "{key}":{expected_value}\n' + \
                                        f'Got     : "{key}":{value}\n'
                else:
                    return_string = f'Test Case {test_id}:\n' + \
                                    f'Expected: "{key}":{expected_value}\n' + \
                                    f'Got     : "{key}":{value}\n'
                    return_value = INCORRECT_VALUE
            else:
                return_string = f'Test Case {test_id}:\n' + \
                                f'Expected: "{key}": {expected_value}\n' + \
                                f'Got     : "{key}": Missing from message body!\n'
                return_value = MISSING_KEY
        return return_value, return_string
    parent_uri = actual_message['uri'][0:actual_message['uri'].rindex("/")]
    key = actual_message['uri'][actual_message['uri'].rindex("/")+1:]
    return_string = ""
    return_value = SUCCESS
    if expected_message['uri'] == parent_uri:
        if key in expected_message['body']:
            if isinstance(expected_message['body'][key], dict):
                if 'value' in expected_message['body'][key]:
                    expected_value = expected_message['body'][key]['value']
                if 'tolerance' in expected_message['body'][key]:
                    tolerance = expected_message['body'][key]['tolerance']
                else:
                    tolerance = 0
                if 'reject_values' in expected_message['body'][key]:
                    reject_values = expected_message['body'][key]['reject_values']
                else:
                    reject_values=[]
            else:
                expected_value = expected_message['body'][key]
                tolerance = 0
                reject_values=[]
            value = actual_message['body']
            if (not isinstance(value, dict)) and (not isinstance(value, list)) and abs(value - expected_value) <= tolerance:
                pass
            elif value in reject_values:
                return_value = INCORRECT_VALUE
                return_string = f'Test Case {test_id}:\n' + \
                                f'Expected: "{key}":{expected_value}\n' + \
                                f'Got     : "{key}":{value}\n'
            elif isinstance(value, list):
                if value == expected_value:
                    pass
                else:
                    return_value = INCORRECT_VALUE
                    return_string = f'Test Case {test_id}:\n' + \
                                    f'Expected: "{key}":{expected_value}\n' + \
                                    f'Got     : "{key}":{value}\n'
            elif isinstance(value, dict):
                if 'value' in value:
                    value = value['value']
                    if abs(value - expected_value) <= tolerance:
                        pass
                    else:
                        return_value = INCORRECT_VALUE
                        return_string = f'Test Case {test_id}:\n' + \
                                        f'Expected: "{key}": {expected_value}\n' + \
                                        f'Got     : "{key}": {value}\n'
                else:
                    if abs(value - expected_value) <= tolerance:
                        pass
                    else:
                        return_value = INCORRECT_VALUE
                        return_string = f'Test Case {test_id}:\n' + \
                                        f'Expected: "{key}": {expected_value}\n' + \
                                        f'Got     : "{key}": {value}\n'
            else:
                return_string = f'Test Case {test_id}:\n' + \
                                f'Expected: "{key}": {expected_value}\n' + \
                                f'Got     : "{key}": {value}\n'
                return_value = INCORRECT_VALUE
        else:
            return_string = f'Test Case {test_id}:\n' + \
                            f'Expected: "{key}": Missing!\n' + \
                            f'Got     : "{key}": {actual_message["body"]}\n'
            return_value = MISSING_KEY
    else:
        return_string = f'Test Case {test_id}:\n' + \
                        f'Expected: "uri": {expected_message["uri"]}\n' + \
                        f'Got     : "uri": {actual_message["uri"]}\n'
        return_value = MISSING_URI
    return return_value, return_string


def compare_times(timestamp1, timestamp2, min_delta=-1, max_delta=-1,
                  expected_delta=-1, max_variation=-1):
    '''
    The compare times function compares the time that two messages were received
    and ensures that they are spaced apart by a certain amount of time (min or
    max delta) or are within an expected delta +/- max_variation.
    Returns True if within the expected time constraints. Returns False otherwise.
    '''
    if min_delta != -1 and abs(timestamp1 - timestamp2) < min_delta:
        return False
    if max_delta != -1 and abs(timestamp1 - timestamp2) > max_delta:
        return False
    if abs(timestamp1 - timestamp2) > (expected_delta + max_variation):
        return False
    if abs(timestamp1 - timestamp2) < (expected_delta - max_variation):
        return False
    return True


def calc_time_between_messages(message_list):
    '''
    For a list of fims messages, calculate the min, max, and average time
    between messages with the same uri.
    '''
    client_message_dict = {}
    server_message_dict = {}
    for message in message_list:
        if (('process' in message) and ('uri' in message) and
            ('timestamp' in message) and ('method' in message)):
            if 'client' in message['process'] and 'pub' in message['method']:
                if message['uri'] in client_message_dict:
                    client_message_dict[message['uri']].append(message)
                else:
                    client_message_dict[message['uri']] = [message]
            if 'server' in message['process'] and 'set' in message['method']:
                if message['uri'] in server_message_dict:
                    server_message_dict[message['uri']].append(message)
                else:
                    server_message_dict[message['uri']] = [message]
    client_results = {}
    for uri, client_message_list in client_message_dict.items():
        if len(client_message_dict) < 2:
            continue
        total_time = 0
        num_messages_minus_1 = 0
        min_time = MAX_INT_16
        max_time = 0
        prev_time = client_message_list[0]['timestamp']
        for message in client_message_list[1:]:
            time_delta = message['timestamp'] - prev_time
            total_time += time_delta
            num_messages_minus_1 += 1
            if time_delta < min_time:
                min_time = time_delta
            if time_delta > max_time:
                max_time = time_delta
            prev_time = message['timestamp']
        client_results[uri] = {"max_time_delta": round(max_time*1000, 3),
                               "min_time_delta": round(min_time*1000, 3),
                               "average_time_delta": round(total_time/num_messages_minus_1*1000, 3)}
    server_results = {}
    for uri, server_message_list in server_message_dict.items():
        if len(server_message_dict) < 2:
            continue
        total_time = 0
        num_messages_minus_1 = 0
        min_time = MAX_INT_16
        max_time = 0
        prev_time = server_message_list[0]['timestamp']
        for message in server_message_list[1:]:
            time_delta = message['timestamp'] - prev_time
            total_time += time_delta
            num_messages_minus_1 += 1
            if time_delta < min_time:
                min_time = time_delta
            if time_delta > max_time:
                max_time = time_delta
            prev_time = message['timestamp']
        server_results[uri] = {"max_time_delta": round(max_time*1000, 3),
                               "min_time_delta": round(min_time*1000, 3),
                               "average_time_delta": round(total_time/num_messages_minus_1*1000, 3)}
    return client_results, server_results
