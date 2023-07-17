import sys
import subprocess
import signal
import time
import datetime
import json
import csv
import matplotlib.pyplot as plt

def parse_fims_dump(file_src, uri, comp):
    try:
        f_r = open(file_src)
    except OSError:
        print("Error: Unable to read/open file: ", file_src)
        sys.exit()
        
    uri_found = False
    comp_found = False
    valList = []
    timeList = []
    with f_r:
        lines = f_r.readlines()
        for line in lines:
            if "Uri" in line:
                lineUri = line.split("Uri:", 1)[1].strip()
                if lineUri == uri:
                    uri_found = True
                else:
                    uri_found = False
            elif uri_found and "Body" in line:
                if comp in line:
                    body = line.split("Body:", 1)[1].strip()
                    if "(null)" in body:
                        body = ""
                    b = json.loads(body)
                    valList.append(b[comp]["value"])
                    comp_found = True
                else:
                    comp_found = False
                uri_found = False
            elif comp_found and "Timestamp" in line:
                times = line.split("Timestamp:", 1)[1].strip()
                timeList.append(str(times))
                comp_found = False
    f_r.close()
    return timeList, valList

def main():
    t, ESSMaxChargePower = parse_fims_dump("fims.out", "/assets/ess/summary", "max_charge_power")
    t, BMSMaxChargePower = parse_fims_dump("fims.out", "/assets/bms/summary", "max_charge_power")
    t, PCSMaxChargePower = parse_fims_dump("fims.out", "/assets/pcs/summary", "max_derated_charge_power")
    t, PCSOutputPower = parse_fims_dump("fims.out", "/assets/pcs/summary", "active_power")
    i = 0
    s = 0
    with open("0715_charge.csv", 'w', newline= '') as csvfile:
        wr = csv.writer(csvfile)
        wr.writerow(("time", "seconds", "ESSMaxChargePower", "BMSMaxChargePower", "PCSMaxChargePower", "PCSOutputPower"))
        for time in t:
            if PCSOutputPower[i] < -10:
                wr.writerow((time, s, ESSMaxChargePower[i], BMSMaxChargePower[i], PCSMaxChargePower[i], PCSOutputPower[i]))
                s = s+1
            i = i+1
            
    t, ESSMaxDischargePower = parse_fims_dump("fims.out", "/assets/ess/summary", "max_discharge_power")
    t, BMSMaxDischargePower = parse_fims_dump("fims.out", "/assets/bms/summary", "max_discharge_power")
    t, PCSMaxDischargePower = parse_fims_dump("fims.out", "/assets/pcs/summary", "max_derated_discharge_power")
    t, PCSOutputPower = parse_fims_dump("fims.out", "/assets/pcs/summary", "active_power")
    i = 0
    s = 0
    with open("0715_discharge.csv", 'w', newline= '') as csvfile:
        wr = csv.writer(csvfile)
        wr.writerow(("time", "seconds", "ESSMaxDischargePower", "BMSMaxDischargePower", "PCSMaxDischargePower", "PCSOutputPower"))
        for time in t:
            if PCSOutputPower[i] > 10:
                wr.writerow((time, s, ESSMaxDischargePower[i], BMSMaxDischargePower[i], PCSMaxDischargePower[i], PCSOutputPower[i]))
                s = s+1
            i = i+1
            
    t, MaxDischargePowerEst = parse_fims_dump("fims.out", "/assets/bms/summary", "max_discharge_power_est")
    t, MaxDischargePowerEstFilt = parse_fims_dump("fims.out", "/assets/bms/summary", "max_discharge_power_est_filt")
    i = 0
    s = 0
    depth = 16
    with open("0715_discharge_filt.csv", 'w', newline= '') as csvfile:
        wr = csv.writer(csvfile)
        wr.writerow(("time", "seconds", "MaxDischargePowerEst", "MaxDischargePowerEstFilt", "Average"))
        for time in t:
            if PCSOutputPower[i] < -10:
                if s >= depth:
                    avg = sum(MaxDischargePowerEst[i-depth:i])/depth
                else:
                    avg = MaxDischargePowerEst[i]
                wr.writerow((time, s, MaxDischargePowerEst[i], MaxDischargePowerEstFilt[i], avg))
                s = s+1
            i = i+1
            
    t, MaxDischargePowerEst = parse_fims_dump("fims.out", "/assets/bms/summary", "max_discharge_power_est")
    t, MaxDischargePowerEstFilt = parse_fims_dump("fims.out", "/assets/bms/summary", "max_discharge_power_est_filt")
    i = 0
    s = 0
    with open("0715_charge_filt.csv", 'w', newline= '') as csvfile:
        wr = csv.writer(csvfile)
        wr.writerow(("time", "seconds", "MaxDischargePowerEst", "MaxDischargePowerEstFilt"))
        for time in t:
            if PCSOutputPower[i] < -10:
                wr.writerow((time, s, MaxDischargePowerEst[i], MaxDischargePowerEstFilt[i]))
                s = s+1
            i = i+1
    
    #plt.plot(MaxChargePower)
    
if __name__ == "__main__":
    main()