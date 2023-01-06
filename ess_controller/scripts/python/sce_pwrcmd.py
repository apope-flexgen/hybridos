#!/usr/bin/python3
#
# Graham Briggs 08/2022
#
# Run from ESS controller command line to send simulated power commands
#
# Usage:
# $ sce_pwrcmd shape pmax period every OPTIONS
# 
# shape: (required) string containing 'charge' 'discharge' 'sinusoid' 'pulse' Note 'pulse' creates a 50% duty cycle pulse train alternating between charge and discharge. 
# Pmax: (optional) maximum power command in Watts. if not specified default is 900kW. Must be positive for charge and discharge shapes. 
# Period: (optional) waveform period in seconds. if not specified default is 60s. Not used for 'charge' and 'discharge' shapes. 
# every: (optional) waveform refresh rate in seconds (must be at least 2*period, ideally at least 10*period). if not specified default is 1s. Not used for 'charge' and 'discharge' shapes
# OPTIONS: (optional) 'key'='value' pairs
#   'alternate' = 'True', 'False': turns on or off alternating between charge and discharge for sine and pulse shapes. for sine will take abs(). send negative power to charge in this case
#   'pwrUri' = 'string' where string contains the uri for the target power command variable.
#   'duty_cycle' = val (0, 1). Used to set pulse train duty cycle. Default  50% if not specified.  
#
# $ sce_pwrcmd 'file' filepath every
# $ sce_pwrcmd 'file' filepath every 0 OPTIONS
#
# filepath: (required) string containing path and filename to a .csv containing power commands in kW with no other data. 
# every: (optional) time in seconds between each entry in filename. If not provided a default of 1 will be selected
# if using OPTIONS as above must send dummy value in argv[4]. Sorry. Not all options are available for running from file. 

import sys
import math
from numpy import sin, pi, linspace, append
import time
#import matplotlib.pyplot as plt
import os

def usage():
    print("sce_pwrcmd.py")
    print("Usage:")
    print("$./sce_pwrcmd shape pmax period every OPTIONS")
    print("$./sce_pwrcmd 'file' filepath every")
    print("$./sce_pwrcmd 'file' filepath every 0 OPTIONS")

def sendPowerCmd(pCmd, options):
    cmdStr = "'{\""+options["var"]+"\":"+str(pCmd)+"}'"
    sysStr = "/usr/local/bin/fims_send -m set -r /me -u " + options["pwrUri"] + " " + cmdStr
    #print(sysStr)
    os.system("echo " + sysStr)
    os.system(sysStr)

def processSinusoid(pMax, tPeriod, tStep, options):
    pvec = []
    tvec, tStepNew = linspace(0, tPeriod, math.floor(tPeriod/tStep)+1, True, True)
    pvec = pMax*sin(tvec*2*(pi/tPeriod))
    if options["alternate"]=="False": #options["alternate"]="" if not set, so "False" is not default.
        pvec = abs(pvec)
        if pMax < 0:
            pvec = - pvec
    return tStepNew, pvec

def processPulse(pMax, tPeriod, tStep, options):
    if options["duty_cycle"] >=1:
        pvec = processConstantPower("positive", pMax)
        return tStep, pvec
    elif options["duty_cycle"] <=0:
        return tStep, [0]
    num =  math.floor(tPeriod/tStep)
    tvec, tStepNew = linspace(1, tPeriod, num, True, True)
    tvec = tvec/tvec #Normalize.... but there are almost certainly better ways to do this
    tvec[-math.floor(num*(1-options["duty_cycle"])):] = 0
    if options["alternate"]=="True":
        tvec = append(tvec, -1 * tvec) #numpy append
    for i in range(len(tvec)):
        if tvec[i] == -0.0:
            tvec[i] = 0.0
    print(pMax*tvec)
    return tStepNew, pMax*tvec

def processConstantPower(sign, pMax):
    if sign == "negative":
        pvec = -pMax
    else:
        pvec = pMax
    return [pvec]

def processFile(file):
    pvec = []
    i = 0
    for line in file:
        i = i+1
        print(line)
        try:
            pvec.append(line)
        except OSError:
            print("CSV must contain numbers only. Note UTF-8 csv filetype not supported and may be the issue here. Exiting")
            print("Error encountered in csv file line ",i)
            sys.exit()
    return pvec

def processInput(shape, pMax, tPeriod, tStep, filename, options):
    pvec = None
    if shape == 'file':
        if not filename.endswith(".csv"):
            print("Only .csv file types are supported. Exiting")
            sys.exit()
        try:
            file = open(filename)
        except OSError:
            if filename == "":
                print("Please specify path to file")
            else:
                print("Could not open file: ",filename)
            sys.exit()
        pvec = processFile(file)
    
    elif shape == 'charge':
        pvec = processConstantPower("negative", pMax)
    
    elif shape == 'discharge':
        pvec = processConstantPower("positive", pMax)

    elif shape == 'sinusoid':
        tStep, pvec = processSinusoid(pMax, tPeriod, tStep, options)
    
    elif shape == 'pulse':
        tStep, pvec = processPulse(pMax, tPeriod, tStep, options)
    else:
        print("Shape not recognized. Exiting")
        sys.exit()
    return tStep, pvec


def processInputArgs(options, kwargs):
    shape = None
    filename = None
    tStep = None
    pMax = None
    tPeriod = None
    duty = 0.5

    for option in options.keys():
        if option in kwargs.keys():
            options[option] = kwargs[option]
    if options["pwrUri"] == "":
        options["pwrUri"] = "/ess_0/site/ess_hs"
    if options["var"] =="":
        options["var"]="active_power_setpoint"
    print("var is ",options["var"])
    #need duty cycle as a float instead of string
    if options["duty_cycle"] !="":
        try:
            duty = float(options["duty_cycle"])
        except OSError:
            print("Error in entered duty cycle. Defaulting to 50%")
        except IndexError:
            print("Error in entered duty cycle. Defaulting to 50%")
    options["duty_cycle"] = duty

    try:
        shape = sys.argv[1]
    except OSError:
        print("Must supply shape argument. Exiting")
        sys.exit()
    
    if shape == 'file':
        try:
            filename = sys.argv[2]
        except OSError:
            print("Must specify .csv file path. Exiting")
            sys.exit()
        
        try:
            tStep = float(sys.argv[3])
        except OSError:
            print("Using default value for 'every' of 1s")
            tStep = 1.0
        except IndexError:
            print("Using default value for 'every' of 1s")
            tStep = 1.0

    else:
        try:
            pMax = float(sys.argv[2])
        except IndexError:
            print("using default value for maximum power of 900kW ")
            pMax = 900.0

        try:
            tPeriod = float(sys.argv[3])
        except IndexError:
            if shape in ['sinusoid', 'pulse']:
                print("using default value for periodicity 60s ")
            tPeriod = 60.0

        try:
            tStep = float(sys.argv[4])
        except IndexError:
            if shape in ['sinusoid', 'pulse']:
                print("using default value for update rate of 1s ")
            tStep = 1.0
    return shape, pMax, tPeriod, tStep, filename

def main(**kwargs):
    options = {"alternate":"", "plot":"", "pwrUri":"", "duty_cycle":"", "var":""} #True/false is stringified for these
    if len(sys.argv) == 1:
        usage()
        sys.exit()

    shape, pMax, tPeriod, tStep, filename = processInputArgs(options, kwargs)
    tStep, pvec = processInput(shape, pMax, tPeriod, tStep, filename, options)

    #plt.plot(pvec)
    #Command loop. Sends power commands contained in pvec to PCS every tStep seconds
    i = 0
    while(True):
        if i >= len(pvec)-1:
            i=0
        sendPowerCmd(pvec[i], options)
        #if (options["plot"]=="True"): #option values are actually strings here
        #    updatePlot()
        time.sleep(tStep)
        i = i+1

if __name__ == "__main__":
    if len(sys.argv) > 5:
        kwargs = dict(arg.split("=") for arg in sys.argv[5:])
    else:
        kwargs = {}
    main(**kwargs) #Any arg after 6th is a key=value pair so create **kwargs and pass to main