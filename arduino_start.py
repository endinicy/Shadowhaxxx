import serial
import time

def arduino_start(ser,):
    ### Setting up the serial connection.
    ### On windows machines, the connection takes time to be established and a time.sleep(2) command will be necessary
    finalwait = True
    ser.write("x")
    ser.reset_input_buffer()
    while True:
        if ser.read(1) == "g": # waiting to get the ready signal from the arduino
            ser.write("s")     # send the start signal to the arduino
            print("Arduino Ready, here we go!\n")
            ser.flushInput()
            while finalwait:
                if ser.read(1) == "k":
                    finalwait = False
            break
            
        else:
            print("Waiting on the Arduino")
            time.sleep(0.1)
        time.sleep(0.1)        # checks every 0.1 seconds
        
