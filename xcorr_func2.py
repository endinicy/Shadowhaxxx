from scipy import signal
import struct
import numpy as np
from matplotlib import pyplot as plt
import sys

def lag_xcorr(data, samprate=1000., avg=True,):
#
    channels = np.arange(len(data))

    if avg==True:
        for i in channels:
            data[i]=data[i]-np.mean(data[i])
        
    output = ["you don't want to see this"]*(len(data)-1)
    for i in channels-1:
        corr=np.correlate(data[0],data[i],"full")
        samp_lag=np.argmax(corr)-len(data[i])+1 # I think this is the right way to do it
        time_lag=float(samp_lag/samprate)
        output[i] = [samp_lag , time_lag]
    
    return(output)

#### SOME TEST STUFF
    
#times=np.arange(20,step=0.1)
#meow1=np.sin(times)
#meow2=np.sin(times+2)
#
#print meow1
#
#plt.plot(times,meow1,times,meow2)
#plt.show()
#
#print lag_xcorr([np.sin(np.arange(1000)),np.sin(np.arange(1000)+100)],samprate=10)