import unittest
import subprocess
import time

FIMS_SEND = "/usr/local/bin/fims_send"
NUM_ESS = int(subprocess.check_output("ls /home/config/modbus_client | grep ess | wc -l", shell=True))

def fimsGet(uri):
    return subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/me", "-u", uri], stderr=subprocess.STDOUT).strip()

def fimsSet(uri, value):
    subprocess.call([FIMS_SEND, "-m", "set", "-u", uri, '{"value":' + str(value) + '}'])

class TestPowerCommands(unittest.TestCase):
    def test_active_power_command_100kW_discharge(self):
    
        # Send 100 kW command to each ESS
        for n in range(1, NUM_ESS + 1):
            id = str(n)
            if n >= 1 and n < 10:
                id = "0" + str(n)
            fimsSet("/assets/ess/ess_{}/maint_active_power_setpoint".format(id), 100)
        
        time.sleep(2)

        # Check if each ESS is reporting 100 kW
        expected = 100
        for n in range(1, NUM_ESS + 1):
            id = str(n)
            if n >= 1 and n < 10:
                id = "0" + str(n)
            actual = float(fimsGet("/components/sungrow_ess_{}_hs/active_power".format(id)))
            self.assertAlmostEqual(expected, actual, None, None, 5)

    def test_active_power_command_200kW_discharge(self):

        # Send 200 kW command to each ESS
        for n in range(1, NUM_ESS + 1):
            id = str(n)
            if n >= 1 and n < 10:
                id = "0" + str(n)
            fimsSet("/assets/ess/ess_{}/maint_active_power_setpoint".format(id), 200)

        time.sleep(2)

        # Check if each ESS is reporting 100 kW
        expected = 200
        for n in range(1, NUM_ESS + 1):
            id = n
            if n >= 1 and n < 10:
                id = "0" + str(n)
            actual = float(fimsGet("/components/sungrow_ess_{}_hs/active_power".format(id)))
            self.assertAlmostEqual(expected, actual, None, None, 5)

    def test_active_power_command_100kW_charge(self):

        # Send -100 kW command to each ESS
        for n in range(1, NUM_ESS + 1):
            id = str(n)
            if n >= 1 and n < 10:
                id = "0" + str(n)
            fimsSet("/assets/ess/ess_{}/maint_active_power_setpoint".format(id), -100)

        time.sleep(2)

        # Check if each ESS is reporting -100 kW
        expected = -100
        for n in range(1, NUM_ESS + 1):
            id = n
            if n >= 1 and n < 10:
                id = "0" + str(n)
            actual = float(fimsGet("/components/sungrow_ess_{}_hs/active_power".format(id)))
            self.assertAlmostEqual(expected, actual, None, None, 5)

    def test_active_power_command_200kW_charge(self):

        # Send -200 kW command to each ESS
        for n in range(1, NUM_ESS + 1):
            id = str(n)
            if n >= 1 and n < 10:
                id = "0" + str(n)
            fimsSet("/assets/ess/ess_{}/maint_active_power_setpoint".format(id), -200)

        time.sleep(2)

        # Check if each ESS is reporting -200 kW
        expected = -200
        for n in range(1, NUM_ESS + 1):
            id = n
            if n >= 1 and n < 10:
                id = "0" + str(n)
            actual = float(fimsGet("/components/sungrow_ess_{}_hs/active_power".format(id)))
            self.assertAlmostEqual(expected, actual, None, None, 5)
    
    def test_active_power_command_0kW(self):

        # Send 0 kW command to each ESS
        for n in range(1, NUM_ESS + 1):
            id = str(n)
            if n >= 1 and n < 10:
                id = "0" + str(n)
            fimsSet("/assets/ess/ess_{}/maint_active_power_setpoint".format(id), 0)

        time.sleep(2)

        # Check if each ESS is reporting 0 kW
        expected = 0
        for n in range(1, NUM_ESS + 1):
            id = n
            if n >= 1 and n < 10:
                id = "0" + str(n)
            actual = float(fimsGet("/components/sungrow_ess_{}_hs/active_power".format(id)))
            self.assertAlmostEqual(expected, actual, None, None, 5)


if __name__ == "__main__":
    unittest.main()
