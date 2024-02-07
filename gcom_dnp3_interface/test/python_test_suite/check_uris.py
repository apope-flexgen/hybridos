'''
The check_uris module contains two functions: get_uri_vars and
check_message_ids. Get_uri_vars looks at a modbus_client or
dnp3_client config file and extracts all publish uris and the
ids of all io_points included in that uri. Check_message_ids will
look at a list of messages and check that each message contains all
expected ids for that uri.
'''
import os
import json
try:
    from user_global_utils import LOCAL_PYTHON_SCRIPT_DIR, CONFIGS_DIR
except ImportError:
    from global_utils import LOCAL_PYTHON_SCRIPT_DIR, CONFIGS_DIR
from comms_configs import analog_input_registers, binary_input_registers

def get_uri_vars(filename):
    '''
    Get_uri_vars looks at a modbus_client or dnp3_client config file and
    extracts all publish uris and the ids of all io_points included in that uri.
    '''
    uris = {}
    if os.path.isfile(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}"):
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}",
                  'r', encoding="utf-8") as file:
            try:
                json_text = json.load(file)
            except json.JSONDecodeError as load_file_exception:
                print(f'Could not load json file {filename}: {load_file_exception}')
                return uris
        try:
            if 'registers' in json_text:
                for register_dict in json_text['registers']:
                    if 'type' in register_dict:
                        register_type = register_dict['type']
                        if not ('client' in filename and (register_type in analog_input_registers or
                                                          register_type in binary_input_registers)):
                            continue
                    else:
                        continue

                    for register in register_dict['map']:
                        if 'id' in register:
                            reg_id = register['id']
                        else:
                            print('Could not parse \'id\' for register. Skipping.')
                            continue
                        if 'uri' in register:
                            uri = f"{register['uri']}"
                        else:
                            if 'base_uri' in json_text['system'] and 'id' in json_text['system']:
                                uri = f"{json_text['system']['base_uri']}/"
                                uri += f"{json_text['system']['id']}"
                                register['uri'] = uri
                            else:
                                print('Could not parse \'uri\' for register. Skipping.')
                                continue
                        if uri not in uris:
                            uris[uri] = {"individual":[], "group":[]}
                        if 'batch_pub_rate' in register or 'interval_pub_rate' in register:
                            uris[uri]["individual"].append(reg_id)
                        else:
                            uris[uri]["group"].append(reg_id)
        except:
            pass
    return uris

def check_message_ids(all_messages):
    '''
    Check_message_ids will look at a list of messages and check that each
    message contains all expected ids for that uri.
    '''
    uris = {}

    for file in os.listdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}"):
        if file.lower().endswith('.json'):
            uris.update(get_uri_vars(file))
    good_messages = 0
    bad_messages = 0
    for message in all_messages:
        found_all = True
        missing_vars = []
        found_vars = []
        var_is_alone = False
        if ('process' in message and 'client' in message['process'] and
            'uri' in message and 'body' in message and 'timestamp' in message and
            message['uri'] in uris):
            uri = message['uri']
            body = message['body']
            if uri in uris:
                if isinstance(body, dict) and len(body) > 2: # timestamp will always be one
                    for reg_id in uris[uri]['group']:
                        if not reg_id in body:
                            found_all = False
                            missing_vars.append(reg_id)
                        else:
                            found_vars.append(reg_id)
                elif isinstance(body, dict):
                    for key in body: # there should only be one key that's not the timestamp
                        if key == "Timestamp":
                            continue
                        if not key in uris[uri]['individual']:
                            var_is_alone = True
                else: # raw value
                    pass
                if not found_all:
                    bad_messages += 1
                elif var_is_alone:
                    bad_messages += 1
                else:
                    good_messages += 1
    print(f"\nComplete messages: {good_messages}")
    print(f"Incomplete messages: {bad_messages}")
    return_str = f"\nComplete messages: {good_messages}\nIncomplete messages: {bad_messages}\n"
    return return_str
