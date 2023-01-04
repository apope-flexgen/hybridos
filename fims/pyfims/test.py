import sys
import time
sys.path.append(".")
from pyfims_rewrite import pyfims

##Connect to fims
p = pyfims()
p.Connect("pyfims_test")

p.Send("set", "/components/bms_9/cap", "99")
assert p.Send("get", "/components/bms_9/cap") == "99"

p.Send("set", "/components/bms_9/cap", "69")
assert p.Send("get", "/components/bms_9/cap") == "69"

p.Close()