# test2.py   open , subscribe and  listen
import sys
import time
sys.path.append(".")
from pyfims_rewrite import pyfims

debug = True
##Connect to fims
p = pyfims()
x = p.Connect("pyfims_test2")
if debug:
    print(" Connect attempt")
    print(x)
if x[1] == 0:
    

    #x = p.Subscribe(["/components/bms_9/cap","/foo"])
    #x = p.Subscribe("/components/bms_9/cap")
    #x = p.Receive()
    #if debug:
    #    print("after subscribe")
    #    print(x)
    x = p.Receive(10)
    if debug : print(x)
    # p.Send("set", "/components/bms_9/cap", "99")
    # assert p.Send("get", "/components/bms_9/cap") == "99"

    # p.Send("set", "/components/bms_9/cap", "69")
    # assert p.Send("get", "/components/bms_9/cap") == "69"
    x = p.Send("set", "/components/bms_1/cap99", "99")
    if debug: 
        print ("after set \"99\" ")
        print(x)
    x = p.Send("set", "/components/bms_1/cap1", 1)
    if debug: 
        print ("after set 1")
        print(x)
    x = p.Send("set", "/components/bms_1/capj", '{"this":"message"}')
    if debug: 
        print ("after set 1")
        print(x)
    x = p.Send("get", "/components/bms_1/cap")
    if debug:
        print ("after get")
        print(x)

p.Close()

#(b'\x00\x00\x00\x0b\x00\x00\x00\x00\x07\x00\x00\x00fims_serverSUCCESS', [], 0, '/tmp/FlexGen_FIMS_Server.socket')
#(b'\x03\x15\x00\t\x00\x00\x00\x00\x04\x00\x00\x00set/components/bms_9/capfims_send1234', [], 0, '/tmp/FlexGen_FIMS_Server.socket')