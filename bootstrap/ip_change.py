import os
import re
import socket
import subprocess

def write_changes(filename, ip_addr, port):
    file = open(filename, "r")
    data = file.read()
    file.close()
    data = re.sub(r"\"ip_address\": \"[0-9]*.[0-9]*.[0-9]*.[0-9]*\"","\"ip_address\": \""+ip_addr+"\"", data)
    data = re.sub(r"\"port\": [0-9]*","\"port\": "+str(port), data)
    file = open(filename, "w")
    file.write(data)
    file.close()

mc_path = "/home/config/modbus_client"
ms_path = "/home/config/modbus_server"
dc_path = "/home/config/dnp3_client"
ds_path = "/home/config/dnp3_server"

# get ip address and name of container   
hostname = socket.gethostname()   
ip_addr = socket.gethostbyname(hostname)   
print("Your Computer Name is:"+hostname)   
print("Your Computer IP Address is:"+ip_addr)
# find number of container from name
numeric = re.findall(r'\d+', hostname)
print("Your Machine Number is:"+numeric[0])

mypath=""
if os.path.exists(mc_path):
    mypath = mc_path
elif os.path.exists(ms_path):
    mypath = ms_path
clients = []
for filename in os.listdir(mypath):
    if os.path.isfile(os.path.join(mypath, filename)) and filename.endswith('.json'):
        clients.append(os.path.join(mypath, filename))

regular = True
for filename in clients:
    if "_ess" in filename and "flexgen_ess" not in filename:
        regular = False
        break

print(regular)

if regular:
    # twins: make sure servers increment port number
    if "twins" in hostname:
        numfiles = 0
        filenames = []
        for path in os.listdir(ms_path):
            if os.path.isfile(os.path.join(ms_path, path)):
                numfiles += 1
                filenames.append(os.path.join(ms_path, path))
        for f in filenames:
            fname = f[f.rindex('/')+1:]
            filenum = re.findall(r'\d+', fname)
            port = 502
            if "ess" in fname:
                port += 2 + int(filenum[0])
            elif "bms" in fname: 
                port += int(filenum[0])
            write_changes(f, "0.0.0.0", port)
    # ess-controller: calculate port numbers to connect to based on machine number and number of configs
    elif "ess-controller" in hostname:
        # get number of modbus client files and calculate their port numbers
        numfiles = 0
        filenames = []
        for path in os.listdir(mc_path):
            if os.path.isfile(os.path.join(mc_path, path)):
                numfiles += 1
                filenames.append(os.path.join(mc_path, path))
        # get twins ip address and calculate port offset
        ip_segs = ip_addr.split('.')
        site_num = (int(ip_segs[2]) - 10) // 2
        twins_ip = "192.168."+str(11+2*(site_num))+".5"
        print("site number: "+str(site_num))
        port_offset = 502
        # loop through modbus client files
        for f in filenames:
            dest_port = port_offset
            if "bms" in f:
                fname = f[f.rindex('/')+1:]
                bms_num = re.findall(r'\d+', fname)
                dest_port += int(bms_num[0])
                print (bms_num[0])
            write_changes(f, twins_ip, dest_port)
    elif "site-controller" in hostname:
        # loop through modbus client files
        numfiles = 0
        filenames = []
        for path in os.listdir(mc_path):
            if os.path.isfile(os.path.join(mc_path, path)):
                numfiles += 1
                filenames.append(os.path.join(mc_path, path))
        # find site number
        ip_segs = ip_addr.split('.')
        site_num = 0
        if ip_segs[0] == "10":
            site_num = int(ip_segs[3]) - 9
        else:
            site_num = (int(ip_segs[2]) - 10) // 2
        print("site number: "+str(site_num))
        for f in filenames:
            fname = f[f.rindex('/')+1:]
            ess_num = re.findall(r'\d+', fname)
            print (ess_num)
            if ess_num[0] == '01':
                print("in ess 01, ip should be "+str(int(ess_num[0])+9))
                ess_ip = "192.168."+str(10+2*(site_num))+"."+str(int(ess_num[0])+9)
                write_changes(f, ess_ip, 502)
            else:
                twins_ip = "192.168."+str(10+2*(site_num))+".250"
                write_changes(f, twins_ip, str(503+int(ess_num[0])))
    elif "fleet-manager" in hostname:
        print ("doing nothing for now")
else:
    if "twins" in hostname:
        numfiles = 0
        filenames = []
        for path in os.listdir(ms_path):
            if os.path.isfile(os.path.join(ms_path, path)):
                numfiles += 1
                filenames.append(os.path.join(ms_path, path))
        filenames.sort()
        index = 0
        for f in filenames:
            fname = f[f.rindex('/')+1:]
            port = 10000 + index
            index += 1
            write_changes(f, "0.0.0.0", port)
    elif "site-controller" in hostname:
        ip_segs = ip_addr.split('.')
        site_num = 0
        if ip_segs[0] == "10":
            site_num = int(ip_segs[3]) - 9
        else:
            site_num = (int(ip_segs[2]) - 10) // 2
        twins_ip = "192.168."+str(10+2*(site_num))+".250"
        numfiles = 0
        filenames = []
        for path in os.listdir(mc_path):
            if os.path.isfile(os.path.join(mc_path, path)):
                numfiles += 1
                filenames.append(os.path.join(mc_path, path))
        filenames.sort()
        index = 0
        for f in filenames:
            fname = f[f.rindex('/')+1:]
            port = 10000 + index
            index += 1
            write_changes(f, twins_ip, port)
