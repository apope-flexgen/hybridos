#! /bin/python3

from json import dumps, loads, JSONDecodeError
from typing import Dict, Union, List

"""
Method:       set
Uri:          /components/pv_2/reactive_power_mode
ReplyTo:      (null)
Process Name: site_controller
Username:     hybridos
Body:         {"value":0}
Timestamp:    2023-12-06 14:50:02.277432
"""

class listen_reply:
    """ This class digests a fims_listen and parses it into a class. """
    def __init__(self, method: str, uri: str, replyto: str, process: str, username: str, body: Dict, timestamp: str) -> None:
        self.method = method
        self.uri = uri
        self.replyto = replyto
        self.process = process
        self.username = username
        self.body = body
        self.timestamp = timestamp

    def __str__(self) -> str:
        string = ["Method:", self.method, "\nUri:", self.uri, "\nReplyTo:", self.replyto, "\nProcess Name:", self.process, "\nUser:", self.username, "\nBody:", dumps(self.body), "\nTimestamp:", self.timestamp]
        return ' '.join(string)

    def contains_action_pub(self) -> bool:
        if isinstance(self.body, dict):
            if 'actions' in self.body:
                return True
            else:
                return False
        else:
            return False

class listen_reply_validator:
    """ This class enumerates the data we want to check for. """
    def __init__(self, path_name: str, step_name: str, status: str) -> None:
        self.path_name= path_name
        self.step_name= step_name
        self.status= status

    def __str__(self) -> str:
        return ' '.join([self.path_name, self.step_name, self.status])

def grok_reply(stdout: List) -> Union[listen_reply, None]:
    """ 
    Reads from a list and tries to parse into a listen_reply. 
    If this function returns a non None object it is on the user to 
    modify their stdout if they need to. 
    
    e.g. I just parsed a listen yay. I need to zero my stdout now.
    """
    # parse the output into these
    method = ""
    uri = ""
    reply_to = ""
    process_name = ""
    username = ""
    body = {}
    timestamp = ""

    if stdout != None:
        for line in stdout:
            if line.find("Method:") != -1:
                method = line[line.find("Method:") + len("Method:"):].strip()
            if line.find("Uri:") != -1:
                uri = line[line.find("Uri:") + len("Uri:"):].strip()
            if line.find("ReplyTo:") != -1:
                reply_to = line[line.find("ReplyTo:") + len("ReplyTo:"):].strip()
            if line.find("Process Name:") != -1:
                process_name = line[line.find("Process Name:") + len("Process Name:"):].strip()
            if line.find("Username:") != -1:
                username = line[line.find("Username:") + len("Username:"):].strip()
            if line.find("Body:") != -1:
                body_str = line[line.find("Body:") + len("Body:"):].strip()
                try:
                    body = loads(body_str)
                except JSONDecodeError:
                    print("failed to parse body string")
                    print(body_str)
            if line.find("Timestamp:") != -1:
                timestamp = line[line.find("Timestamp:") + len("Timestamp:"):].strip()
                return listen_reply(method=method, uri=uri, replyto=reply_to, process=process_name, 
                                     username=username, body=body, timestamp=timestamp)
        return None
    else:
        return None
