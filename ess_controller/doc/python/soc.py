volts = 0.0

for soc in range (1,100):
  volts = 0.0
  current = 2.8
  if soc< 15:
     volts = 3.1 + (3.4 - 3.1) * (soc/15.0)
  elif soc < 80:
     volts = 3.4
  else:
    current = 2.8 - 2.8 * (soc-80)/20.0
    volts = 3.4 + (3.6 - 3.4) * ((soc-80.0) /20.0)
  print volts, current , volts*current


