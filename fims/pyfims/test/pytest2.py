import pyfims_rewrite as py

pyfims_instance = py.pyfims()
print("Connect")

x = pyfims_instance.Connect("/pytest_connection")
print(x)

#def test_absolute_ess(site_cntlr, pyfims_instance, input_values):
    # subscribe to uri(s)
#x = pyfims_instance.Subscribe("/pytest_connection")
#    assert x[1] == 0


print ("send")
# set absolute_ess_direction_flag
x = pyfims_instance.Send("set", "/features/active_power/absolute_ess_direction_flag", None, True)
#   assert x[1] == 0
print(x)

print("get")
# get absolute_ess_direction_flag
x = pyfims_instance.Send("get", "/features/active_power/absolute_ess_direction_flag", "/pytest_connection")
# for debugging:
#assert x == None
print (x)

print("receive")

x = pyfims_instance.Receive(10)
#    # for debugging:
#    assert x == None
print(x)

pyfims_instance.Close()
