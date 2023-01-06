#!/usr/bin/python

import pandas as pd
from datetime import datetime
import csv
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
#headers = ['Time','Sensor Value']
#df = pd.read_csv('./DataLog2.csv',names=headers)
#print (df)

#df['Date'] = df['Date'].map(lambda x: datetime.strptime(str(x), '%Y/%m/%d %H:%M:%S.%f'))
#xx = df['Time']
#yy = df['Sensor Value']

target = 4.2
curr2 = 2.5;
curr = 2.5;
yr = []
yt = []
xx=[]
baset = 2.5
for i in range(0, 20):
   #print (yy[i])

   curr2 += (target-curr) * (target-curr) / 5.0
   yt.append(baset+((curr2 - curr) * 3.5))
   yr.append(curr2)
   xx.append(i)
   curr = curr2

print

#print (xx)
#print (yy)


# plot
plt.plot(xx,yr, label="volts")
plt.plot(xx,yt, label="temp")
# beautify the x-labels
plt.gcf().autofmt_xdate()

plt.show()

