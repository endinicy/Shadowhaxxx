import matplotlib.pyplot as plt
import numpy as np
import random
import time

channel0x = range(0,5)
channel0y = range(0,5)
channel1x = range(0,5)
channel1y = range(0,5)

plt.ion()

fig = plt.figure()
ax = fig.add_subplot(111)
line1, = ax.plot(channel0y, 'r-') # Returns a tuple of line objects, thus the comma
line2, = ax.plot(channel1y, 'b-')

lastplot = time.time()

while True:
    if True:
        new_data = [random.randint(0,400)/100.,random.randint(0,400)/100.]
        channel0y.append(new_data[0])
        channel0y.pop(0)
        channel1y.append(new_data[1])
        channel1y.pop(0)        
    if time.time()-lastplot > 0.01:
        line1.set_ydata(channel0y)
        line2.set_ydata(channel1y)
        fig.canvas.draw()
        fig.canvas.flush_events()
        lastplot = time.time()
        print "meow"