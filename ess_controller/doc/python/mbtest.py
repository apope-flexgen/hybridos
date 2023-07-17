

from pymodbus.client.sync import ModbusTcpClient

client = ModbusTcpClient('192.168.1.19')
print (client)

#client.write_coil(1, True)
for x in range (1,4000):
	resulth = client.read_holding_registers(x,1,unit=1)
	resulti = client.read_input_registers(x,1,unit=1)
	#print(result.bits[0])
	try:
		print("holding register at :",x,resulth.registers)
	except:
		pass
		#print(" no holding register at:", x)
	try:
		print("input register at:", x,resulti.registers)
	except:
		pass
		#print(" no input register at:", x)
client.close()

