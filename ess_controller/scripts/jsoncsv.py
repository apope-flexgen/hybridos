
#!/usr/bin/env python
# json_data being the literal file data, in this example

import json
import csv
import sys

data = {}

#with open(sys.argv[1], 'r') as json_file:
#    data = json.load(json_file)
#print (data)
#print(" Data keys ...\n")
#print (data.keys())
#with open('/tmp/test.csv', 'w') as outf:
#    dw = csv.DictWriter(outf, data.keys())
#    dw.writeheader()
#    for row in data.keys():
#        dw.writerow(row)
from collections import OrderedDict #To maintain key value pair order      
_json=json.loads(open(sys.argv[1], 'r').read()) #, object_pairs_hook=OrderedDict) 
#print(" Json keys ...")
#print (_json.keys())
nam = ""
first = 0
for key in _json.keys():
    #print ("key..", end="")
    if(key == "name"):
        #print(_json[key])
        nam =_json[key]
        #print(nam)
        first = 1
        continue
    #print("value keys..")
    #print (_json[key].keys())
    if (first == 1):
        first = 2
        print("fname", end=",")
        sep = " "
        for ki in _json[key]:
            print (sep, ki,end="")
            sep = ","
        print ("")
    if(first > 1):
        print(nam, end=",")
        print(key, end=",")

        sep = ""
        for ki in _json[key]:
            print (sep, _json[key][ki], end="")
            sep = ","
        print("")

#out=open('/tmp/test.csv', 'w')
#writer = csv.writer(out)               #create a csv.write
#writer.writerow(_json.keys())      # header row
#for row in _json:
#    writer.writerow(row.values())