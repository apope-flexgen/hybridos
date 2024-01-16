#! /bin/python3

from json import dumps
from typing import Dict

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

