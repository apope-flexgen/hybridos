"""
Tests active and reactive power commands for Site Controller

Precondition(s):
  - Site is started and in Running - Grid state
  - All BESS units are running
  - POI limits are disabled
"""

import unittest
import subprocess
import re
import time

FIMS_SEND = "/usr/local/bin/fims_send"

def fimsGet(uri):
    value = subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/me", "-u", uri], stderr=subprocess.STDOUT).strip()
    if "value" in value:
        value = re.search('(?<="value":).*?(?=,)', value).group() 
    return value

def fimsSet(uri, value):
    subprocess.call([FIMS_SEND, "-m", "set", "-u", uri, '{"value":' + str(value) + '}'])

class TestPowerCommands(unittest.TestCase):
    def test_active_power_command_800kW_discharge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", 800)
        time.sleep(2)
        expected = 800
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 12)

    def test_active_power_command_4MW_discharge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", 4000)
        time.sleep(2)
        expected = 4000
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 30)

    def test_active_power_command_10MW_discharge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", 10000)
        time.sleep(2)
        expected = 10000
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 110)

    def test_active_power_command_15MW_discharge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", 15000)
        time.sleep(2)
        expected = 10000
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 110)

    def test_active_power_command_800kW_charge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", -800)
        time.sleep(2)
        expected = -800
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 12)

    def test_active_power_command_4MW_charge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", -4000)
        time.sleep(2)
        expected = -4000
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 30)

    def test_active_power_command_10MW_charge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", -10000)
        time.sleep(2)
        expected = -10000
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 110)

    def test_active_power_command_15MW_charge(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", -15000)
        time.sleep(2)
        expected = -10000
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 110)

    def test_active_power_command_0MW(self):
        fimsSet("/features/active_power/fr_baseload_kW_cmd", 0)
        time.sleep(2)
        expected = 0
        actual = float(fimsGet("/features/active_power/ess_actual_kW"))
        self.assertAlmostEqual(expected, actual, None, None, 10)
        
if __name__ == "__main__":
    unittest.main()
