from scipy import signal
import struct
import numpy as np
from matplotlib import pyplot as plt
import sys

def lag_xcorr(data=[], samprate=1000):

    dataarray=np.array(data)
    data0=dataarray[:,0]
    data1=dataarray[:,1]

    # Subtract mean value, this is important!
    data0_mean=data0-np.mean(data0)
    data1_mean=data1-np.mean(data1)
        
    corr=np.correlate(data0_mean,data1_mean,"full")
    samp_lag=np.argmax(corr)-len(data0_mean)+1 # I think this is the right way to do it
    time_lag=float(samp_lag/samprate)
    
    return(samp_lag)