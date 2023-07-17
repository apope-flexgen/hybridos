import logging
import time
from .fims import fims_set, fims_threaded_pub
from .assertion_framework import Flex_Assertion
from .pytest_report import close_report, setup_report, report_id, report_steps, report_expected

class Steps():
    '''Parent class for executing standardized test steps.'''
    def __init__(self, inputs, expected_values):
        '''
        Parameters:
        fims_steps ({} OR []): Fims commands to issue
            - {} => Obj of set commands
            - [{}, {}] => Array of [set commands, pub commands]
        expected_values: List of Flex_Assertions, containing an expected value, URI to compare against, and type of comparison
        '''
        # Allow suppling just obj {} of set commands
        if not isinstance(inputs, list):
            inputs = [inputs]
        self.set_steps = inputs[0]
        self.pub_steps = inputs[1] if len(inputs) > 1 else {}
        self.expected_values = expected_values

    # Run fims test steps
    def run_steps(self, id):
        # Report the id
        report_id(id)
        # Run and report steps
        kill_commands = []
        for uri, value in self.set_steps.items():
            fims_set(uri, value)
        for uri, value in self.pub_steps.items():
            kill_commands.append(fims_threaded_pub(uri, value))
        report_steps(self.set_steps)
        report_steps(self.pub_steps)
        [k() for k in kill_commands] # Kill pub threads
        # Check and report results
        self.get_result()

    # Check test values
    def get_result(self):
        # Single expected value provided
        if not isinstance(self.expected_values, list):
            if not isinstance(self.expected_values, Flex_Assertion):
                logging.error("Assertion is invalid")
            # Reported the expected and actual values for this test
            report_expected(self.expected_values)
            # Assert the expected, actual pair
            self.expected_values.make_assertion()
        # Multiple expected values
        else:
            # Report all expected, actual pairs for this test
            report_expected(self.expected_values)
            for i, assertion in enumerate(self.expected_values):
                if not isinstance(assertion, Flex_Assertion):
                    logging.error(f"Assertion {i} is invalid")
                # Assert single expected, actual pair
                assertion.make_assertion()
    
    # Static methods to supply frequently used steps.
    @staticmethod
    def disable_solar_and_gen():
        return {
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
            "/assets/generators/gen_1/maint_mode": True
        }
    @staticmethod
    def enable_solar_and_gen():
        return {
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/assets/generators/gen_1/maint_mode": False
        }

# Child class for setup behavior
class Setup(Steps):
    # report_name: File name of the test report
    # fims_steps: Fims commands to issue
    # expected_values: List of Flex_Assertions, containing an expected value, URI to compare against, and type of comparison
    def __init__(self, name, inputs, expected_values):
        self.report_name = name
        super().__init__(inputs, expected_values)

    def run_steps(self, id=None):
        setup_report(self.report_name)
        super().run_steps("setup")
        time.sleep(4) # Provide a bit of extra time for Setup steps to be the change they want to see in the world.


# Child class for teardown behavior
class Teardown(Steps):
    def run_steps(self, id=None):
        super().run_steps("teardown")
        close_report()
