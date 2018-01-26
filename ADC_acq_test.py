import serial
import time
import struct
import numpy as np
import sys
import threading
import Queue
from arduino_start import arduino_start
from xcorr_func import lag_xcorr
from matplotlib import pyplot as plt 

#### BEGIN: Serial initiation
dat_q=Queue.Queue(maxsize=100)
packetbytes=4 # 4 byte packet size from serial

serialport='/dev/tty.usbmodemFA131'   # I think this is the front usb port on my MacBook
ser = serial.Serial(serialport, baudrate=9600,timeout=0)
print(ser.port+" open")
#### END: Serial initiation


def lagcalc(data_q,):
    print("Starting Lag Calculation \n")
    lastlagtime=time.time()
    starttime=time.time()
    outputlist=[]
    outputlist1=[]
    while time.time() < starttime+10: # 10 seconds of data-ing
        if not data_q.empty():
            raw_data=data_q.get()
            #print(raw_data)
            new_data=[int(raw_data[0]),int(raw_data[1])]
            # print(new_data)
            outputlist.append(new_data)
            outputlist1.append(new_data)
            if time.time() > starttime+3: # After 3 seconds of data collection, this deletes the first entry everytime a new one is added. 
                outputlist.pop(0)         # This is great because it works independent of sample rate, but any delays at the beginning will reduce output size
            if time.time()-lastlagtime > 1:
                samplerate=float(len(outputlist)/3)
                # print(samplerate)
                # print("in_waiting:")
                # print(outputlist)
                print(lag_xcorr(outputlist,samprate=samplerate))
                lastlagtime = time.time()
    print("Exiting Lag Calculation\n")
    outputlistarray=np.array(outputlist1)
    
    plt.figure(2)
    plt.plot(outputlistarray[:,0])
    plt.plot(outputlistarray[:,1])

    plt.show()

def stop_prog(ser):
    print("Closing Arduino Connection\n")
    ser.write("x") # tell the arduino to stop sending data
    ser.close() # close the serial connection


arduino_start(ser)
data_q=Queue.Queue(maxsize=100)
run_start=time.time()

thread_lagcalc=threading.Thread(target=lagcalc,args=(data_q,))
thread_lagcalc.start()

point=0
rawdata=["",""]

while not data_q.full():
    new1=ser.read()
    if new1 == ",":
        point=1
    elif new1 == ";":
        data_q.put(rawdata)
        point=0
        rawdata=["",""]
        # print(data_q.get())
    else:
        rawdata[point] += new1
    
stop_prog(ser)





#  Me trying to use arduino serial.write rather than print
#run_start=time.time()
#ser.reset_input_buffer
#ser.reset_output_buffer
#while True: #time.time() < run_start + 10:
#    if ser.in_waiting > 1:
#        print(struct.unpack("H",ser.read(2))[0])
        
    
# def twos_comp(val, bits):
#     """compute the 2's complement of int value val"""
#     if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
#         val = val - (1 << bits)        # compute negative value
#     return val  