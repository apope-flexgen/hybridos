import logging
import time
import os
from .fims import fims_set, fims_threaded_pub, fims_get
from .assertion_framework import Flex_Assertion
from .pytest_report import close_report, setup_report, report_id, report_steps, report_expected
from typing import Any


class Steps():
    '''Parent class for executing standardized test steps.'''

    def __init__(self, inputs, expected_values, pre_lambda=[], post_lambda=[], *args, **kwargs):
        '''
        Parameters:
        fims_steps ({} OR []): Fims commands to issue
            - {} => Obj of set commands
            - [{}, {}] => Array of [set commands, pub commands]
        expected_values: List of Flex_Assertions, containing an expected value, URI to compare against, and type of comparison
        pre_lambda:  arr of arbitrary functions to be executed before GET commands
        post_lambda: arr of arbitrary functions to be executed after  GET commands
        '''
        # Allow suppling just obj {} of set commands
        if not isinstance(inputs, list):
            inputs = [inputs]
        self.set_steps = inputs[0]
        self.pub_steps = inputs[1] if len(inputs) > 1 else {}
        self.expected_values = expected_values
        self.pre_lambda = pre_lambda
        self.post_lambda = post_lambda
        self.args = args
        self.kwargs = kwargs

    # Run fims test steps
    def run_steps(self, id):
        # Report the id
        report_id(id)
        # Run and report steps
        kill_commands = []
        for uri, value in self.set_steps.items():
            set_reply = fims_set(uri, value, reply_to='/pytest')
            assert set_reply == value
        for uri, value in self.pub_steps.items():
            kill_commands.append(fims_threaded_pub(uri, value))
        report_steps(self.set_steps)
        report_steps(self.pub_steps)
        [k() for k in kill_commands]  # Kill pub threads
        # TODO should pre lambda be before the steps
        [l(*self.args, **self.kwargs) for l in self.pre_lambda]
        # Check and report results
        self.get_result()
        [l(*self.args, **self.kwargs) for l in self.post_lambda]

    # Check test values
    def get_result(self):
        # Single expected value provided
        if not isinstance(self.expected_values, list):
            if not isinstance(self.expected_values, Flex_Assertion):
                raise ValueError("Assertion is invalid")
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
                    raise ValueError(f"Assertion {i} is invalid")
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

    @staticmethod
    def place_assets_in_maint():
        return {
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
            "/assets/generators/gen_1/maint_mode": True,
            "/assets/ess/ess_01/maint_mode": True,
            "/assets/ess/ess_02/maint_mode": True,
        }

    @staticmethod
    def place_assets_in_maint_dynamic(solar: bool = False, gen: bool = False, ess: bool = False):
        """Do a get to get all assets then put them in maint"""
        if solar:
            solar_get: dict[str, Any] = fims_get("/assets/solar")
            del solar_get['summary']
            for key in solar_get.keys():
                fims_set("/assets/solar/" + key + "/maint_mode", True)

        if gen:
            gen_get: dict[str, Any] = fims_get("/assets/generators")
            del gen_get['summary']
            for key in gen_get.keys():
                fims_set("/assets/generators/" + key + "/maint_mode", True)

        if ess:
            ess_get: dict[str, Any] = fims_get("/assets/ess")
            del ess_get['summary']
            for key in ess_get.keys():
                fims_set("/assets/ess/" + key + "/maint_mode", True)

    @staticmethod
    def remove_all_assets_from_maint_dynamic():
        """Do a get to get all assets then remove them from maint"""
        solar_get: dict[str, Any] = fims_get("/assets/solar")
        del solar_get['summary']
        for key in solar_get.keys():
            fims_set("/assets/solar/" + key + "/maint_mode", False)

        gen_get: dict[str, Any] = fims_get("/assets/generators")
        del gen_get['summary']
        for key in gen_get.keys():
            fims_set("/assets/generators/" + key + "/maint_mode", False)

        ess_get: dict[str, Any] = fims_get("/assets/ess")
        del ess_get['summary']
        for key in ess_get.keys():
            fims_set("/assets/ess/" + key + "/maint_mode", False)


    @staticmethod
    def remove_assets_from_maint():
        return {
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/assets/generators/gen_1/maint_mode": False,
            "/assets/ess/ess_01/maint_mode": False,
            "/assets/ess/ess_02/maint_mode": False,
        }

    @staticmethod
    def config_dev_place_assets_in_maint():
        return {
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
            "/assets/generators/gen_1/maint_mode": True,
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True,
        }

    @staticmethod
    def config_dev_place_ESS_in_maint():
        return {
            "/assets/ess/ess_1/maint_mode": True,
            "/assets/ess/ess_2/maint_mode": True,
        }

    @staticmethod
    def config_dev_place_solar_in_maint():
        return {
            "/assets/solar/solar_1/maint_mode": True,
            "/assets/solar/solar_2/maint_mode": True,
        }

    @staticmethod
    def config_dev_remove_solar_from_maint():
        return {
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
        }

    @staticmethod
    def config_dev_place_gen_in_maint():
        return {
            "/assets/generators/gen_1/maint_mode": True,
        }

    @staticmethod
    def config_dev_remove_gen_from_maint():
        return {
            "/assets/generators/gen_1/maint_mode": False,
        }

    @staticmethod
    def ess_1_disable_all_maint_feats():
        return {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/maint_soc_protection_buffers_disable": False,
            "/assets/ess/ess_1/maint_soc_limits_enable": False,
            "/assets/ess/ess_1/maint_cell_voltage_limits_enable": False,
            "/assets/ess/ess_1/maint_rack_voltage_limits_enable": False,
            "/assets/ess/ess_1/maint_min_charge_discharge_enable": False,
            }

    @staticmethod
    def config_dev_remove_assets_from_maint():
        return {
            "/assets/solar/solar_1/maint_mode": False,
            "/assets/solar/solar_2/maint_mode": False,
            "/assets/generators/gen_1/maint_mode": False,
            "/assets/ess/ess_1/maint_mode": False,
            "/assets/ess/ess_2/maint_mode": False,
        }



# Child class for setup behavior
class Setup(Steps):
    # report_name: File name of the test report
    # fims_steps: Fims commands to issue
    # expected_values: List of Flex_Assertions, containing an expected value, URI to compare against, and type of comparison
    def __init__(self, name, inputs, expected_values, pre_lambda=[], post_lambda=[]):
        self.report_name = name
        super().__init__(inputs, expected_values, pre_lambda, post_lambda)

    def run_steps(self, id=None):
        setup_report(self.report_name)
        super().run_steps("setup")
        time.sleep(.2)


# Child class for teardown behavior
class Teardown(Steps):
    def run_steps(self, id=None):
        super().run_steps("teardown")
        close_report()
