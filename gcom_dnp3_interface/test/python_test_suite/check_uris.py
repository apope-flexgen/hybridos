import os
import json
from datetime import datetime
from global_utils import *
from git_info import *

def get_uri_vars(filename):
    uris = {}
    if os.path.isfile(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}"):
        with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/{filename}", 'r') as file:
            try:
                json_text = json.load(file)
            except Exception as e:
                print(f'Could not load json file {filename}: {e}')
                return
        try:
            if 'registers' in json_text:
                for register_dict in json_text['registers']:
                    if 'type' in register_dict:
                        register_type = register_dict['type']
                        if not ('client' in filename and (register_type in analog_input_registers or register_type in binary_input_registers)):
                            continue
                    else:
                        continue
                        
                    for register in register_dict['map']:
                        if 'id' in register:
                            id = register['id']
                        else:
                            print('Could not parse \'id\' for register. Skipping.')
                            continue
                        if 'uri' in register:
                            uri = '{}'.format(register['uri'])
                        else:
                            if 'base_uri' in json_text['system'] and 'id' in json_text['system']:
                                uri = '{}/{}'.format(json_text['system']['base_uri'],json_text['system']['id'])
                                register['uri'] = uri
                            else:
                                print('Could not parse \'uri\' for register. Skipping.')
                                continue
                        if not (uri in uris):
                            uris[uri] = {"individual":[], "group":[]}
                        if 'batch_pub_rate' in register or 'interval_pub_rate' in register:
                            uris[uri]["individual"].append(id)
                        else:
                            uris[uri]["group"].append(id)
        except:
            pass
    return uris

def check_message_ids(all_messages):
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
        alone_var = ""
        if 'process' in message and 'client' in message['process'] and 'uri' in message and 'body' in message and 'timestamp' in message and message['uri'] in uris:
            uri = message['uri']
            body = message['body']
            if uri in uris:
                if isinstance(body, dict) and len(body) > 2: # timestamp will always be one
                    for id in uris[uri]['group']:
                        if not id in body:
                            found_all = False
                            missing_vars.append(id)
                        else:
                            found_vars.append(id)
                elif isinstance(body, dict):
                    for key in body: # there should only be one key that's not the timestamp
                        if key == "Timestamp":
                            continue
                        if not key in uris[uri]['individual']:
                            var_is_alone = True
                            alone_var = key
                else: # raw value
                    pass
                try:
                    timestamp = datetime.fromtimestamp(message['timestamp']).strftime('%Y-%m-%d %H:%M:%S.%f')
                except:
                    timestamp = message['timestamp']
                if not found_all:
                    # print(f"Uri: {uri}\nTimestamp: {timestamp}\nMissing Ids: {missing_vars}\nFound Ids: {found_vars}\n")
                    bad_messages += 1
                elif var_is_alone:
                    # print(f"Uri: {uri}\nTimestamp: {timestamp}\nVar is alone: {alone_var}\n")
                    bad_messages += 1
                else:
                    good_messages += 1
    print(f"\nComplete messages: {good_messages}")
    print(f"Incomplete messages: {bad_messages}")
    return_str = f"\nComplete messages: {good_messages}\nIncomplete messages: {bad_messages}\n"
    return return_str