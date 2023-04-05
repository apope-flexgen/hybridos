"""
Matthew Myers test thing

Precondition(s):
  - 
"""

import unittest
import subprocess
import re
import time

FIMS_SEND = "/usr/local/bin/fims_send"

def fimsGet(uri):
    point = uri.split('/')
    value = subprocess.check_output([FIMS_SEND, "-m", "get", "-r", "/me", "-u", uri], stderr=subprocess.STDOUT).strip()
    if "value" in value:
        value = re.search('(?<="value":).*?(?=,)', value).group() 

    value = float(value.split(point[3])[1][2:-1])

    return value

def fimsSet(uri, value):
    subprocess.call([FIMS_SEND, "-m", "set", "-u", uri, '{"value":' + str(value) + '}'])

class MetricsTestPlan(unittest.TestCase):
    def test_matthews_test_plan(self):

        fimsSet("/ercot/outstation/load_updated_basepoint", 1)
        time.sleep(2)
        self.assertAlmostEqual(0, fimsGet("/ercot/outstation/gen_net_mw"), None, None, 0)
    
        fimsSet("/ercot/outstation/load_updated_basepoint", 2)
        time.sleep(2)
        self.assertAlmostEqual(0, fimsGet("/ercot/outstation/gen_net_mw"), None, None, 0)

if __name__ == "__main__":
    unittest.main()