
#!/usr/bin/env python
from pymodbus.client.sync import ModbusTcpClient

# TODO #1 ask for ip_address from command line 
# TODO #2 read config foile as specified in command line 

client = ModbusTcpClient('192.168.1.19')

print (client)

coils = []
irs = []
hrs = []
sum_hrs = []

max_reg=2000

count_hrs = 0
base_hrs = 0
#client.write_coil(1, True)
for x in range (1,max_reg):
    #global hrs
    #global sum_hrs
    #global irs
    #global coils
    resulth = client.read_holding_registers(x,1,unit=1)
    resulti = client.read_input_registers(x,1,unit=1)
    resultc = client.read_coils(x,1,unit=1)
	#print(result.bits[0])
    try:
        print("holding register at :",x,resulth.registers)
        hrs.append(x)
        if count_hrs == 0:
            base_hrs = x
        else:
            if x != base_hrs+count_hrs:
                print(" new base , count ", base_hrs, count_hrs)
                sum_hrs.append({base_hrs, count_hrs})
                base_hrs = x
                count_hrs = 1        
        count_hrs += 1

    except:
        pass
	#print(" no holding register at:", x)
    try:
        print("input register at:", x,resulti.registers)
        irs.append(x)
    except:
        pass
	try:
		print("cois at:", x,resultc.bits[0])
		coils.append(x)
	except:
		pass
		#print(" no input register at:", x)

print(" hrs new base , count ", base_hrs, count_hrs)
sum_hrs.append({base_hrs, count_hrs})
 
client.close()

print(len(hrs))
print (sum_hrs)

# produce summary maps
sum_coils = []
sum_irs = []

basex = 0
count = 0
#print (hrs)

for x1 in hrs:
	global sum_hrs
	if count == 0:
		count = 1
		basex = x1
	else:
		if x1 == basex + count:
			count = count + 1
		else:
            		sum_hrs.append({base,count})
            		print(" base , count",base,count)
			base = x1
			count = 1

print(len(sum_hrs))

for x1,x2 in sum_hrs:
	print(" base , count", x1, x2)
