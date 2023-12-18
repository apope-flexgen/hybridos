import json
import random

uris = {}
register_types = {}
with open("sce_bop_rtac_dnp3_server.json", 'r') as file:
    try:
        json_text = json.load(file)
    except Exception as e:
        print('Could not load json file')
        quit()
    
    for register_group in json_text['registers']:
        for register in register_group['map']:
            if not (register['uri'] in uris):
                uris[register['uri']] = []
            uris[register['uri']].append((register['id'], register_group['type']))

messages = []
for i in range(10):
    for uri in uris:
        message_body = {}
        for (register_id, reg_group_type) in uris[uri]:
            if reg_group_type == 'analog':
                message_body[register_id] = random.randint(0,1<<15)
            elif reg_group_type == 'binary':
                message_body[register_id] = random.choice([True, False])
        message = f"fims_send -m pub -u {uri} '{json.dumps(message_body)}'"
        messages.append(message)
    messages.append("sleep 0.2")

with open('run_script.sh', 'w', newline='\n') as file:
    for message in messages:
        file.write(message)
        file.write("\n")