#!/usr/bin/python

import pandas as pd
from datetime import datetime
import csv
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
headers = ['Time','Sensor Value']
df = pd.read_csv('./DataLog2.csv',names=headers)
print (df)

#df['Date'] = df['Date'].map(lambda x: datetime.strptime(str(x), '%Y/%m/%d %H:%M:%S.%f'))
xx = df['Time']
yy = df['Sensor Value']

ysum = 0;
yr = []
for i in range(0, len(xx)):
   print (yy[i])
   ysum += (i * yy[i])
   yr.append(ysum)
print

#print (xx)
#print (yy)


# plot
plt.plot(xx,yr)
# beautify the x-labels
plt.gcf().autofmt_xdate()

plt.show()

