#!/usr/bin/python3
import csv
with open('esp.csv', newline='') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    for row in spamreader:
        #print(', '.join(row))
        print (row[1])
        cmd="/usr/local/bin/fims/fims_send -m set -u /components/catl_sbmu_9 " + row[1]
        print (cmd)

