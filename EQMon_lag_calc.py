import Queue
import threading
import time
from xcorr_func import lag_xcorr

# Had problems importing EQMon_test_trigger.py, so here it is

import serial
import time
import struct
import numpy as np
import sys

from matplotlib import pyplot as plt
from matplotlib import animation

def adsTwoChannelStream(data_q, duration=30,dataprint=0):    
    serialport='/dev/ttyACM0'   # I think this is the front usb port on my MacBook
    ser = serial.Serial(serialport, baudrate=115200,timeout=0)
    ### On windows machines, the connection takes time to be established and a time.sleep(2) command will be necessary

                # This part syncs this code with the arduino so byte ordered-ness is preserved
    run=False   # Doing this replaces delimiters which would take up serial com time
    ser.reset_output_buffer() # get that junk outta here
    while run == False:
        print("Waiting for the Arduino")
        time.sleep(0.2)     # checks every 0.1 seconds
        meow = ser.read(1)  # reads the serial port
        if meow == "g":     # waiting to get the ready signal from the arduino
            ser.write("s")     # send the start signal to the arduino
            while run == False:     # now we just wait for the trigger receipt confirmation
                if ser.read(1) == "k":
                    run = True      # exit the loop and let's go!
        # I hope no one hates me for making this whole thing one function, I've had problems making global serial vars
    samplecount = 0
    starttime=time.time()
    while time.time()<starttime+duration and not data_q.full():   # runs for 'duration' seconds or until the queue overflows
    
        if ser.in_waiting > 4:      # waits until the full 4 bytes (2x 2-byte packets) is ready
            if ser.in_waiting > 60: # checking to see if the serial input buffer is over flowing
                ser.close()
                sys.exit("Arduino Buffer Overflow. Reading too slow, losing data.")
    
            data_q.put([struct.unpack('>h',(ser.read(2)))[0] * 0.1875,struct.unpack('>h',(ser.read(2)))[0] * 0.1875]) # puts data points in list and appends them to the queue
            samplecount += 1

    ser.write("x")
    ser.close()    
    return samplecount

def get_lag(data_q,plotgen):
    print threading.currentThread().getName(), 'Starting'
    lagstart=time.time()
    lastlagtime=time.time()
    outputlist=[]
    while not data_q.full():
        # print data_q.qsize(),"start"
        if not data_q.empty():
            # print data_q.qsize()
            outputlist.append(data_q.get())
            if time.time() > lagstart+3: # After 3 seconds of data collection, this deletes the first entry everytime a new one is added. 
                # print data_q.qsize(),"pre-pop"
                outputlist.pop(0)         # This is great because it works independent of sample rate, but any delays at the beginning will reduce output size
    #                print(newdata)
                if time.time()-lastlagtime > 1:
                    samplerate=float(len(outputlist)/3)
                    print(lag_xcorr(outputlist,samprate=samplerate))
                    lastlagtime = time.time()
#                if plotgen==1:
#                    meow=plt.figure(1)
#                    meow.plt(x1,y1,x2,y2)
#                    meow.xlabel('Time (ms)')
#                    meow.show()
    print threading.currentThread().getName(), 'Exiting'


data_queue = Queue.Queue(maxsize=100)

# thread_dataget=threading.Thread(name="Data Collection",target=adsTwoChannelStream,args=(data_queue,9999))

plotgen=1
dataprint=1

thread_adsTwoChannelStream=threading.Thread(target=adsTwoChannelStream,args=(data_queue,9999,dataprint,))
thread_lagcomp=threading.Thread(name="Lag Computation",target=get_lag,args=(data_queue,plotgen,))

thread_adsTwoChannelStream.start()
thread_lagcomp.start()

stoptime=time.time() + 30
while (time.time() < stoptime):
    # print data_queue.qsize()
    time.sleep(10)