# Framework for more flexible test assertions
from enum import Enum
import time
from typing import Union
from pytest import approx
import logging
from .pytest_report import report_actual
from .fims import fims_get


class Tolerance_Type(Enum):
    abs = 0
    rel = 1
    both = 2

class Assertion_Type(Enum):
    approx_eq = 0
    less_than_eq = 1
    greater_than_eq = 2
    approx_neq = 3
    obj_eq = 4


# Framework for flexible assertions
class Flex_Assertion():
    # type: type of assertion
    # uri: fims URI that holds the value to compare against
    # value: one or more values to compare against (ascending order)
    #               if type is a range check, the first value is the min, and the second value is the max
    # tolerance_type: the type of tolerance used by the assertion (see Assertion_Type)
    # tolerances: one or more tolerances used based on the assertion type
    #               if type is both, abs tolerance is given first and relative tolerance is given second
    # max_limit: Max limit on the value, used by features like edp (0) and agg asset limit to prevent further discharge
    # wait_secs: How long to wait before querying the uri for the value (in seconds)
    # pattern: eg."active_faults.options", only used in Assertion_type.obj_eq
    def __init__(self, type: Assertion_Type, uri: str, value: Union[int, float, bool], wait_secs=3.0, tolerance_type=Tolerance_Type.rel, tolerance=0.05, max_limit=None, pattern=None):
        self.type = type
        self.uri = uri
        self.value = value
        self.tolerance = tolerance
        self.tolerance_type = tolerance_type
        self.max_limit = max_limit
        self.wait = wait_secs
        self.pattern = pattern 

    # Less than comparison of this class against other value
    def __lt__(self, other):
        if isinstance(other, Flex_Assertion):
            return self.value < other.value
        else:
            return self.value < other

    # Greater than comparison of this class against other value
    def __gt__(self, other):
        if isinstance(other, Flex_Assertion):
            return self.value > other.value
        else:
            return self.value > other

    # Actual value is equal to expected within tolerance
    def is_approx_equal(self, this_value, other):
        if type(this_value) == bool:
            # no tolerance used for bools
            return other == this_value
        if self.tolerance_type == Tolerance_Type.abs:
            return other == approx(this_value, abs=self.tolerance)
        elif self.tolerance_type == Tolerance_Type.rel:
            return other == approx(this_value, rel=self.tolerance)
        else:
            return other == approx(this_value, abs=self.tolerance[0], rel=self.tolerance[1])

    # Actual value is less than or equal to value within tolerance
    def is_approx_less_than(self, this_value, other):
        return this_value < other or self.is_approx_equal(this_value, other)

    # Actual value is greater than or equal to value within tolerance
    def is_approx_greater_than(self, this_value,  other):
        return this_value > other or self.is_approx_equal(this_value, other)

    # Equal comparison of this class against other value
    def __eq__(self, other):
        return self.is_approx_equal(other, self.value)

    # Not equal comparison of this class against other value
    def __ne__(self, other):
        return not self.is_approx_equal(other, self.value)

    # Less than or equal to comparison of this class against other value
    def __le__(self, other):
        return self.is_approx_less_than(self.value, other)

    # Greater than or equal to comparison of this class against other value
    def __ge__(self, other):
        return self.is_approx_greater_than(self.value, other)

    # Substraction operation using this class and another value
    def __sub__(self, other):
        if isinstance(other, Flex_Assertion):
            return self.value - other.value
        else:
            return self.value - other

    # Right hand side substraction operation using this class and another value
    def __rsub__(self, other):
        return other - self.value

    # Add to this class's expected value (overrides -=)
    def __isub__(self, other):
        if isinstance(other, Flex_Assertion):
            self.value -= other.value
        else:
            self.value -= other
        return self

    # Addition operation using this class and another value
    def __add__(self, other):
        if isinstance(other, Flex_Assertion):
            return self.value + other.value
        else:
            return self.value + other

    # Right hand side addition operation using this class and another value
    def __radd__(self, other):
        return other + self.value

    # Add to this class's expected value (overrides +=)
    def __iadd__(self, other):
        if isinstance(other, Flex_Assertion):
            self.value = self.value + other.value
        else:
            self.value = self.value + other
        return self

    # Overwrite this class's expected value(s)
    # (equivalent to the assignment operator, =, as we cannot override it)
    def overwrite_value(self, other):
        if isinstance(other, Flex_Assertion):
            self.value = other.value
        else:
            self.value = other

    # Get tolerance. If both are used return the larger
    def get_kW_tolerance(self):
        if self.tolerance_type == Tolerance_Type.both:
            return max(self.tolerance[0], self.tolerance[1] * abs(self.value))
        elif self.tolerance_type == Tolerance_Type.rel:
            return self.tolerance * abs(self.value)
        else:
            return self.tolerance

    # Add an additional tolerance type and value or overwrite the existing value
    def add_tolerance(self, other_type, other_tolerance):
        if other_type == Tolerance_Type.both:
            self.tolerance_type = other_type
            self.tolerance = other_tolerance
        elif self.tolerance_type == Tolerance_Type.both:
            # If both tolerances used, index 0 is abs and index 1 is rel
            self.tolerance[other_type == Tolerance_Type.rel] = other_tolerance
        elif self.tolerance_type != other_type:
            temp = self.tolerance
            self.tolerance = [None, None]
            # If this assertion has one tolerance and receives the other, use both
            self.tolerance[self.tolerance_type == Tolerance_Type.rel] = temp
            self.tolerance[other_type == Tolerance_Type.rel] = other_tolerance
            self.tolerance_type = Tolerance_Type.both
        else:
            self.tolerance = other_tolerance

    # Get the actual value
    # TODO: Possibly move the get_asset_agg logic here as well to aggregate multiple uris as needed
    def get_actual(self):
        response = fims_get(self.uri)
        # Extract clothed value
        if isinstance(response, dict) and len(response.keys()) == 1:
            response = response[list(response.keys())[0]] # Flatten if == 1 key
        if isinstance(response, dict) and "value" in response.keys():
            response = response["value"]
        return response

    # Assert based on type
    def make_assertion(self):
        time.sleep(self.wait)
        actual_value = self.get_actual()
        report_actual(self.uri, actual_value)
        if self.type == Assertion_Type.approx_eq:
            assert actual_value == self, f"{self.uri}:"
        elif self.type == Assertion_Type.less_than_eq:
            assert actual_value <= self, f"{self.uri}:"
        elif self.type == Assertion_Type.greater_than_eq:
            assert actual_value >= self, f"{self.uri}:"
        elif self.type == Assertion_Type.approx_neq:
            assert actual_value != self, f"{self.uri}:"
        elif self.type == Assertion_Type.obj_eq:
            self.tolerance = Tolerance_Type.abs
            if self.pattern:
                for key in self.pattern.split("."):
                    if (not isinstance(actual_value, dict) or key not in actual_value.keys()):
                        logging.error(f"Key [{key}] not found for response [{actual_value}]")
                        assert False
                    actual_value = actual_value[key]
            assert actual_value == self, f"{self.uri}:"

    # Construct string representation for reporting that includes both the type and tolerance
    # Called manually for reporting
    def report(self) -> str:
        type_str = ""
        if self.type in [Assertion_Type.approx_eq, Assertion_Type.obj_eq]:
            type_str = "=="
        elif self.type == Assertion_Type.less_than_eq:
            type_str = "<="
        elif self.type == Assertion_Type.greater_than_eq:
            type_str = ">="
        return type_str + " " + self.__str__()

    # Str representation used in displaying pytest results
    # Called automatically on failure
    def __str__(self) -> str:
        # Bools + objs don't use tolerance
        if type(self.value) == bool or self.type == Assertion_Type.obj_eq:
            return str(self.value)
        ret = f"{self.value} "
        if self.tolerance_type == Tolerance_Type.both:
            ret += f"\u00B1{self.tolerance[0]} (abs) or \u00B1{self.tolerance[1] * abs(self.value)} (rel)"
        elif self.tolerance_type == Tolerance_Type.abs:
            ret += f"\u00B1{self.tolerance} (abs)"
        elif self.tolerance_type == Tolerance_Type.rel:
            ret += f"\u00B1{self.tolerance * abs(self.value)} (rel)"
        return ret

    # Only used for logging so just use str representation
    def __repr__(self) -> str:
        return self.__str__()
