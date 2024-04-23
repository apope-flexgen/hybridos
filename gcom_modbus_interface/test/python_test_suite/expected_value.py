'''
The expected_value module defines the ExpectedValue class. An ExpectedValue
is specific to the test suite's test cases, in which an expected message body
can contain multiple expected values with various options for allowing flexibility.

For example, expected messages may take the form of:
{
    "uri": "/some/uri/point",
    "body": 5
}
{
    "uri": "/some/uri",
    "body": {"point": 5}
}
{
    "uri": "/some/uri",
    "body": {
        "point": {"value": 5, "tolerance": 0.001}
        }
}
or many other similar forms. The ExpectedValue parses the input for a single key
and allows comparison to an actual message value.

Note: this is currently unused, but it provides a framework for potential
future directions with fims_listen stuff.
'''

class ExpectedValue:
    '''
    An ExpectedValue is specific to the test suite's test cases, in which an expected
    message body can contain multiple expected values with various options for
    allowing flexibility.

    For example, expected messages may take the form of:
    {
        "uri": "/some/uri/point",
        "body": 5
    }
    {
        "uri": "/some/uri",
        "body": {"point": 5}
    }
    {
        "uri": "/some/uri",
        "body": {
            "point": {"value": 5, "tolerance": 0.001}
            }
    }
    or many other similar forms. The ExpectedValue parses the input for a single key
    and allows comparison to an actual message value.
    '''
    def __init__(self, value):
        self.value = value
        self.tolerance = 0
        self.reject_values = []
        self.max_value = None
        self.min_value = None
        self.clothed = None

    @classmethod
    def from_kwargs(cls, **kwargs):
        '''
        Load an ExpectedValue from a dictionary.
        '''
        if "value" not in kwargs:
            if "max_value" in kwargs:
                expected_value = cls(kwargs["max_value"])
            elif "min_value" in kwargs:
                expected_value = cls(kwargs["min_value"])
            else:
                return None
        else:
            expected_value = cls(kwargs["value"])
        has_only_value = True
        for key, value in kwargs.items():
            if hasattr(expected_value, key):
                setattr(expected_value, key, value)
                if key != "value":
                    has_only_value = False
        if has_only_value:
            expected_value.value = kwargs
            expected_value.clothed = True
        return expected_value

    def compare_raw_value(self, value) -> (bool, str, str):
        '''
        compare_raw_value compares ONLY the "value" field of an ExpectedValue to
        the "value" field of a message. Mostly for internal use.
        Returns (is_equal, expected, actual).
        '''
        # Compare reject_values
        for reject_value in self.reject_values:
            if isinstance(value, type(reject_value)): # make sure the types are the same
                if value == reject_value and isinstance(value, str):
                    return False, f'Message does not contain "{value}"', f'"{value}"'
                if value == reject_value:
                    return False, f'Message does not contain {value}', f'{value}'

        # Extract the raw "value" field if we have a dictionary
        expected_value = self.value
        if isinstance(self.value, dict):
            if "value" in self.value:
                expected_value = self.value['value']

        ## Compare the exact value
        if value == expected_value:
            return True, "", ""
        if isinstance(value, str):
            if isinstance(expected_value, str):
                return False, f'"{expected_value}"', f'"{value}"'
            return False, f'{expected_value}', f'"{value}"'
        if self.tolerance > 0:
            if abs(value - expected_value) < self.tolerance:
                return False, f'{expected_value} ± {self.tolerance}', f'{value}'
        if self.max_value is not None:
            if value > self.max_value:
                return False, f'Value < {self.max_value}', f'{value}'
        if self.min_value is not None:
            if value < self.min_value:
                return False, f'Value >= {self.min_value}', f'{value}'
        return False, f"{expected_value}", f"{value}"

    def compare_value(self, value) -> (bool, str, str):
        '''
        Compare a field taken straight from a fims message to an expected value.
        '''
        if isinstance(value, dict) and self.clothed is not None:
            if self.clothed:
                if isinstance(self.value, dict):
                    for key, field_value in self.value:
                        if key == 'value' and 'value' in value:
                            matches, expected, actual = self.compare_raw_value(value['value'])
                            if not matches:
                                return matches, expected, actual
                        else:
                            if key in value and value[key] != field_value:
                                return False, f'{self.value}', f'{value}'
                            if key not in value:
                                return False, f'{self.value}', f'Missing attribute "{key}"'
                else:
                    if 'value' in value:
                        return self.compare_raw_value(value['value'])
            else:
                return False, "naked value", "clothed value"
        return self.compare_raw_value(value)
