#!/usr/bin/python3
import csv
import time
import os

import numpy as np
import matplotlib.pyplot as plt
tdelay = 0.5
# with open('esp.csv', newline='') as csvfile:
#     spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
#     for row in spamreader:
#         #print(', '.join(row))
#         print (row[1])
#         cmd="/usr/local/bin/fims/fims_send -m set -u /components/catl_sbmu_9 " + row[1]
#         print (cmd)
#         os.system(cmd)
#         time.sleep(tdelay)


# get wa 
# 2000 1 
# 2000 1 
# 000 0.1 
# 000 0.1 
# 2000 1 
# 2000 1 
# 2000 1 
# 2000 1
# 
# 10000 /5.2  = 1923
#  

def getavg(va):
    i = 0
    asum = 0.0
    if len(va) > 0:
        while  i  < len(va):
            asum += va[i]
            i += 1
        return asum/len(va)
    return 0

def setVal(comp, name, value):
    cmd = F"/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess{comp} '{{\"{name}\":{{\"value\":{value}}}}}'"
    #print(F"running {cmd}\n")
    d = os.popen(cmd).read()
    #print(F"running {cmd} : returned [{d}]\n")

def getVal(comp, name):
    cmd = F"/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess{comp}/{name}"
    d = os.popen(cmd).read()
    #print(F"running {cmd} : returned [{d}]\n")
    #return d
    return d.rstrip("\n")

def testFilt1():
    dt = 0.1
    b = np.arange(0, 30, dt)
    c = []
    d = []
    filt = 0.75 * dt
    xlast = 0
    for x in b:
        xc = 100

        if x< 10 :
            xc = 0
        elif x> 20 :
            xc = 0
        xd = xc * filt - (xlast *(filt-1)) # * dt
        c.append(xc)
        d.append(xd)
        xlast = xd
    print(c)
    print(d)
    return b, c, d 

def testData():
    # Fixing random state for reproducibility
    np.random.seed(19680801)

    dt = 0.01
    t = np.arange(0, 30, dt)
    nse1 = np.random.randn(len(t))                 # white noise 1
    nse2 = np.random.randn(len(t))                 # white noise 2

    # Two signals with a coherent part at 10Hz and a random part
    s1 = np.sin(2 * np.pi * 10 * t) + nse1
    s2 = np.sin(2 * np.pi * 10 * t) + nse2
    return dt, t, s1, s2

def testPlot(t, s1, s2):
    fig, axs = plt.subplots(1, 1)
    axs.plot(t, s1, t, s2)
    axs.set_xlim(0, 30)
    axs.set_xlabel('time')
    axs.set_ylabel('s1 and s2')
    axs.grid(True)

    #cxy, f = axs[1].cohere(s1, s2, 256, 1. / dt)
    #axs[1].set_ylabel('coherence')

    fig.tight_layout()
    plt.show()

def testPlot2():
    # Fixing random state for reproducibility
    np.random.seed(19680801)

    dt = 0.01
    
    nse1 = np.random.randn(len(t))                 # white noise 1
    nse2 = np.random.randn(len(t))                 # white noise 2

    # Two signals with a coherent part at 10Hz and a random part
    s1 = np.sin(2 * np.pi * 10 * t) + nse1
    s2 = np.sin(2 * np.pi * 10 * t) + nse2

    fig, axs = plt.subplots(2, 1)
    axs[0].plot(t, s1, t, s2)
    axs[0].set_xlim(2)
    axs[0].set_xlabel('time')
    axs[0].set_ylabel('s1 and s2')
    axs[0].grid(True)

    cxy, f = axs[1].cohere(s1, s2, 256, 1. / dt)
    axs[1].set_ylabel('coherence')

    fig.tight_layout()
    plt.show()

d = os.popen('date').read()
print("running on:", d)
#a,b,c,d =testData()
#print(c)
#The formula I have is y[n] = x*filtFac - (y[n-1]*(filtFac - 1)) 
# or y[n] = x*filtFac - y[n-1]*(filtFac)  +y[n-1]  

#b,c,d = testFilt1()
#testPlot(b,c,d)

#time.sleep(tdelay)

setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
#foo = getVal("/status/test/sbmu_9_cmd", "sbmu_raw_current")
#print ("value returned :[",foo,"]")
#os._exit(0)

#time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9_cmd"
cmd +=" '{\"sbmu_raw_current\":{\"value\":2000,\"debug\":true, \"amap\":\"ess\",\"depth\":16,"
cmd += "\"ifChanged\":false,\"vecAv\":\"/status/test/sbmu_9:raw_current_vec\","
cmd += "\"outFilt\":\"/status/test/sbmu_9:sbmu_filt_current\","
cmd += "\"outAv\":\"/status/test/sbmu_9:sbmu_current_outAv\","
cmd += "\"outMax\":\"/status/test/sbmu_9:sbmu_current_outMax\","
cmd += "\"outMin\":\"/status/test/sbmu_9:sbmu_current_outMin\","
cmd += "\"outSp\":\"/status/test/sbmu_9:sbmu_current_outSp\","
cmd += "\"outSum\":\"/status/test/sbmu_9:sbmu_current_outSum\","
cmd += "\"window\":0,\"filtFac\":0.75,\"actions\":"
cmd += "{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
print(cmd)
#os._exit(0)

os.system(cmd)
print("done")

#os._exit(0)

#"outFilt\":\"/status/test/sbmu_9:sbmu_filt_current
setVal("/status/test/sbmu_9", "sbmu_filt_current", 2000)

print("set depth to 16")
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9_cmd"
cmd +=" '{\"sbmu_raw_current\":{\"value\":2000,\"depth\":16}}'"
os.system(cmd)

print ( "send out 16 values at 2000")

setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outSum")
print ("sum returned :",sum1)

print("set depth to 8")
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9_cmd"
cmd +=" '{\"sbmu_raw_current\":{\"value\":2000,\"depth\":8}}'"
os.system(cmd)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outSum")
print ("sum returned :",sum1)

print("set depth back to 16")
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9_cmd"
cmd +=" '{\"sbmu_raw_current\":{\"value\":2000,\"depth\":16}}'"
os.system(cmd)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 0000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)

sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outSum")
print ("sum returned :",sum1)
print ("set 50% to 1000") 
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outSum")
print ("sum returned :",sum1)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outMax")
print ("max returned :",sum1)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outMin")
print ("min returned :",sum1)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outAv")
print ("avg returned :",sum1)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outSp")
print ("Spread returned :",sum1)

print ("set 100% to 1000") 
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 0000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 1000)
#time.sleep(tdelay)

# cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
# resp = os.popen(cmd).read()
# print("resp:", resp)
# print(resp)
foo = 1234
print ("value preset :",foo)

foo = getVal("/status/test/sbmu_9", "sbmu_filt_current")
print ("value returned :",foo)

#os._exit(0)
sum1 = getVal("/status/test/sbmu_9", "sbmu_current_outSum")


setVal("/status/test/sbmu_9", "sbmu_filt_current", 1001)

print ("set raw_current to 2000" )

setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
foo = getVal("/status/test/sbmu_9", "sbmu_filt_current")
print ("value returned :",foo)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
foo = getVal("/status/test/sbmu_9", "sbmu_filt_current")
print ("value returned :",foo)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
foo = getVal("/status/test/sbmu_9", "sbmu_filt_current")
print ("value returned :",foo)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
foo = getVal("/status/test/sbmu_9", "sbmu_filt_current")
print ("value returned :",foo)
setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", 2000)
foo = getVal("/status/test/sbmu_9", "sbmu_filt_current")
sum2 = getVal("/status/test/sbmu_9", "sbmu_current_outSum")
print ("value returned :",foo, "sum1 :", sum1, "sum2 :", sum2)

#print ("value returned :",foo)



print(" test drop out")
t = 0
while t < 32:
    val = 2000 
    #if t > 2  and t < 10:
    #   val = 0
    setVal("/status/test/sbmu_9_cmd", "sbmu_raw_current", val)
    #avg  = getVal("/status/test/sbmu_9", "sbmu_current_outAv")
    #print (" t: ", t , "val :", val,  "avg  :",avg,)
    
    t += 1




t = 0
while t < 32:
    val = 2000 
    if t > 2  and t < 10:
       val = 0
    if val == 0:
        cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9_cmd"
        cmd +=" '{\"sbmu_raw_current\":{\"value\":0,\"depth\":1}}'"
        os.system(cmd)
    else:   
        cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9_cmd"
        cmd +=" '{\"sbmu_raw_current\":{\"value\":2000,\"depth\":16}}'"
        os.system(cmd)
    avg  = getVal("/status/test/sbmu_9", "sbmu_current_outAv")
    weight = 1.0
    wabs = abs(float(avg) - float(val))
    if wabs  > float(avg)* 0.25:
        weight = (float(avg) -wabs)/float(avg)
    print (" t: ", t , "val :", val,  "abs:" , wabs, "avg  :",avg,"weight :", weight)
    
    t += 1

print("weighted avg mode")
av = []
aw = []

t = 0
while t < 32:
    val = 2000 
    av.append(val)
    aw.append(100.0)
    t += 1

os._exit(0)


time.sleep(tdelay)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 2000"
os.system(cmd)
time.sleep(tdelay)

##does not work when inital value is 0
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 3000"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 3000"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 3000"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(tdelay)

# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
# {
#  "test_movavg":{"value":0, 
#      "amap":"ess",
#      "depth":16,
#      "ifChanged": false,
#      "vecAv":"/test/sbmu_1:current_testVec",
#      "outAv":"/test/sbmu_1:avgCurrent",
#      "outSum":"/test/sbmu_1:sumCurrent",
#      "outFilt":"/test/sbmu_1:filtCurrent",
#      "filtFac":0.25,
#      "actions":{"onSet":[{                     
#          "func":[{"func":"MathMovAvg","amap":"ess"}]
#             }]}}}
# '

## 1 depth 1


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":1,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(tdelay)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

## 2 depth 1
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":1,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(tdelay)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

## 1 depth 2
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":2,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

print("should be 5.15")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

## 2 depth 2  not correct
## this may be due to the first value being a dummy one
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":2,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

print("should be 20.6")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

##4 depth 5
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":5,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":14.3}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":12.3}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(tdelay)

print("should be 9.44")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

print("should be 47.2")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

## 5 depth 5 not correct
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":8.3}}'"
os.system(cmd)
time.sleep(tdelay)

print("should be 11.1")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

print("should be 55.5")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/filtCurrent | jq"
os.system(cmd)
time.sleep(tdelay)

# # # test for multiple actions
# cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/test/pcs '{\"start_grad_p\":{\"value\": 11.0,\"actions\":{\"onSet\":[{\"limits\": [ {\"low\": 0.1,\"high\": 3010.0}]},{\"remap\":[{\"bit\": 0,\"uri\": \"/components/pcsm_control\",\"var\": \"start_grad_p\"}]}, {\"limits\":[{\"low\":0.2,\"high\":2400}]}]}}}'"
# os.system(cmd)
# time.sleep(tdelay)

# # #response   {"start_grad_p":{"value":11,"actions":{"onSet":[{"limits":[{"low":0.1,"high":3010}]},{"remap":[{"bit":0,"uri":"/components/pcsm_control","var":"start_grad_p"}]},{"limits":[{"low":0.2,"high":2400}]}]}}}
# cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/pcs/start_grad_p | jq"
# os.system(cmd)
# time.sleep(tdelay)