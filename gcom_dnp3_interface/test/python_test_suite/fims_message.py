'''
This module defines the FimsMessage class, which can be used to
parse fims messages into a structure and make comparisons between
messages.

Note: this is currently unused, but it provides a framework for potential
future directions with fims_listen stuff.
'''
from datetime import datetime
from typing import Type
from process_fims_messages import process_message
from expected_value import ExpectedValue

class FimsMessage:
    '''
    The FimsMessage class can be used to parse fims messages into a
    structure and make comparisons between messages.
    '''
    def __init__(self, **kwargs):
        self.method: str = "(null)"
        self.uri: str = "(null)"
        self.body: dict = {}
        self.replyto: str = "(null)"
        self.process_name: str = "(null)"
        self.username: str = "(null)"
        self.timestamp: str = "(null)"
        for key, value in kwargs.items():
            if hasattr(self, key):
                setattr(self, key, value)

        try:
            self.timestamp_float: float = datetime.strptime(self.timestamp,
                                                            '%Y-%m-%d %H:%M:%S.%f').timestamp()
        except (AttributeError, ValueError, OverflowError, OSError):
            self.timestamp_float = 0

        if not isinstance(self.body, dict):
            uri_frags = self.uri.split("/")
            if len(uri_frags) > 0:
                self.body = {uri_frags[-1]: self.body}
                self.uri = "/".join(self.uri[:-1])
        else:
            if "value" in self.body:
                uri_frags = self.uri.split("/")
                if len(uri_frags) > 0:
                    self.body = {uri_frags[-1]: self.body}
                    self.uri = "/".join(self.uri[:-1])

    def __str__(self):
        output = f"Method:       {self.method}\n"
        output += f"Uri:          {self.uri}\n"
        output += f"ReplyTo:      {self.body}\n"
        output += f"Process Name: {self.replyto}\n"
        output += f"Username:     {self.process_name}\n"
        output += f"Body:         {self.username}\n"
        output += f"Timestamp:    {self.timestamp}\n\n"
        return output

    @classmethod
    def fromstring(cls, string):
        '''
        Parse a FimsMessage object directly from a string.
        '''
        message = process_message(string)
        if message is None:
            return None
        return cls(**message)

    def compare_timestamp(self, expected_message: Type['FimsMessage']) -> (bool, str):
        '''
        Compare this message's timestamp to what is expected.

        Currently unimplemented.
        '''
        return True, ""

    def compare_body_to_expected(self, expected_body: dict) -> (bool, str):
        '''
        Compare this message's body to what is expected, key by key.
        '''
        return_message_keys = {}
        matches = True
        for key, expected_value in expected_body:
            if key in self.body:
                expected_value_object = ExpectedValue(expected_value)
                compare_result = expected_value_object.compare_value(self.body[key])
                if not compare_result[0]:
                    matches = False
                    return_message_keys[key] = [compare_result[1], compare_result[2]]
            else:
                matches = False
                return_message_keys[key] = [expected_value, "Missing!"]
        if matches:
            return True, ""

        return_message = ""
        if len(return_message_keys) > 1:
            for (key,[expected,actual]) in return_message_keys.items():
                return_message += f'"{key}":\n\tExpected: {expected}\n\tGot: {actual}'
            return False, return_message

        for (key,[expected,actual]) in return_message_keys.items():
            return_message += f'Expected: "{key}":{expected}\n\tGot: "{key}":{actual}'
        return False, return_message

    def compare_to_expected(self, expected_message: Type['FimsMessage']) -> (bool, str):
        '''
        Compare the entire message to what is expected, field by field.

        If a field is left unspecified (or null) by the expected message, it is
        not considered as part of the comparison.
        '''
        checks = [
                (expected_message.method, self.method, "Method"),
                (expected_message.uri, self.uri, "Uri"),
                (expected_message.replyto, self.replyto, "ReplyTo"),
                (expected_message.process_name, self.process_name, "Process Name"),
                (expected_message.username, self.username, "Username"),
                ]
        for expected, actual, attribute in checks:
            if expected not in ['(null)', actual]:
                return False, f"Expected: {attribute}: {expected}\nGot: {attribute}: {actual}"
        if expected_message.timestamp != "(null)" or expected_message.timestamp_float > 0:
            matches, return_message = self.compare_timestamp(expected_message)
            if not matches:
                return False, return_message
        return self.compare_body_to_expected(expected_message.body)
    