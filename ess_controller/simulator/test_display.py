import os
print ("Display is: %s" % (os.environ['DISPLAY'],))
import matplotlib
print ("Default backend is: %s" % (matplotlib.get_backend(),))
matplotlib.use('TkAgg')
print ("Backend is now: %s" % (matplotlib.get_backend(),))
from matplotlib import pyplot
pyplot.ioff()
pyplot.plot([1,2,3])
pyplot.show()