"""
Tests POI limits from Site Controller

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
    def test_active_power_command_15MW_discharge(self):

        # Send 15 MW baseload command and see if active power is limited to 11 MW
        fimsSet("/features/active_power/fr_baseload_kW_cmd", 15000)
        time.sleep(2)
        self.assertEqual(15, float(fimsGet("/features/active_power/site_kW_demand")) / 1000)
        self.assertEqual(11, float(fimsGet("/features/active_power/ess_kW_cmd")) / 1000)
        self.assertEqual(11, float(fimsGet("/features/active_power/max_potential_ess_kW")) / 1000)
        self.assertAlmostEqual(11, float(fimsGet("/features/active_power/ess_actual_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/max_potential_feeder_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(-11, float(fimsGet("/features/active_power/feeder_actual_kW")) / 1000, None, None, 0.5)

        # Enable POI limits and see if active power is now limited to 10 MW
        fimsSet("/features/standalone_power/poi_limits_enable", "true")
        time.sleep(2)
        self.assertEqual(10, float(fimsGet("/features/active_power/site_kW_demand")) / 1000)
        self.assertEqual(10, float(fimsGet("/features/active_power/ess_kW_cmd")) / 1000)
        self.assertAlmostEqual(11, float(fimsGet("/features/active_power/max_potential_ess_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/ess_actual_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/max_potential_feeder_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(-10, float(fimsGet("/features/active_power/feeder_actual_kW")) / 1000, None, None, 0.5)

        # Disable POI limits and see if active power is now limited to 11 MW
        fimsSet("/features/standalone_power/poi_limits_enable", "false")
        time.sleep(2)
        self.assertEqual(15, float(fimsGet("/features/active_power/site_kW_demand")) / 1000)
        self.assertEqual(11, float(fimsGet("/features/active_power/ess_kW_cmd")) / 1000)
        self.assertEqual(11, float(fimsGet("/features/active_power/max_potential_ess_kW")) / 1000)
        self.assertAlmostEqual(11, float(fimsGet("/features/active_power/ess_actual_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/max_potential_feeder_kW")) / 1000, None, None, 0.5)
        self.assertAlmostEqual(-11, float(fimsGet("/features/active_power/feeder_actual_kW")) / 1000, None, None, 0.5)

    def test_active_power_command_15MW_charge(self):
        
        # Send -15 MW baseload command and see if active power is limited to -11 MW
        fimsSet("/features/active_power/fr_baseload_kW_cmd", -15000)
        time.sleep(2)
        self.assertEqual(-15, float(fimsGet("/features/active_power/site_kW_demand")) / 1000)
        self.assertAlmostEqual(-9.9, float(fimsGet("/features/active_power/ess_kW_cmd")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(-10, float(fimsGet("/features/active_power/min_potential_ess_kW")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(-10, float(fimsGet("/features/active_power/ess_actual_kW")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/feeder_actual_kW")) / 1000, None, None, 0.9)

        # Enable POI limits and see if active power is now limited to -9.9 MW
        fimsSet("/features/standalone_power/poi_limits_enable", "true")
        time.sleep(2)
        self.assertEqual(-9.9, float(fimsGet("/features/active_power/site_kW_demand")) / 1000)
        self.assertEqual(-9.9, float(fimsGet("/features/active_power/ess_kW_cmd")) / 1000)
        self.assertAlmostEqual(-11, float(fimsGet("/features/active_power/min_potential_ess_kW")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(-9.9, float(fimsGet("/features/active_power/ess_actual_kW")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/feeder_actual_kW")) / 1000, None, None, 0.9)

        # Disable POI limits and see if active power is now limited to -11 MW
        fimsSet("/features/standalone_power/poi_limits_enable", "false")
        time.sleep(2)
        self.assertEqual(-15, float(fimsGet("/features/active_power/site_kW_demand")) / 1000)
        self.assertAlmostEqual(-9.9, float(fimsGet("/features/active_power/ess_kW_cmd")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(-10, float(fimsGet("/features/active_power/min_potential_ess_kW")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(-10, float(fimsGet("/features/active_power/ess_actual_kW")) / 1000, None, None, 0.9)
        self.assertAlmostEqual(10, float(fimsGet("/features/active_power/feeder_actual_kW")) / 1000, None, None, 0.9)

if __name__ == "__main__":
    unittest.main()