import serial
import time
import struct
import numpy as np
import sys





def adsTwoChannelStream(data_q, duration=30,):    
    serialport='/dev/tty.usbmodemFA131'   # I think this is the front usb port on my MacBook
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
