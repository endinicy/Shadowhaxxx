"""
Future Work:
    
- maybe check out the ugly lag_comp script 
- do we want to have two sensors at each location for noise?

"""


import numpy as np
from matplotlib import pyplot as plt

#from xcorr_func2 import lag_xcorr
from xcorr_func import lag_xcorr as lag_xcorr_old

import Queue
import threading
import time

# Had problems importing EQMon_test_trigger.py, so here it is

import serial
import struct
import sys


# GIMME SOME SPACE FOR THESE ARGUMENTS
import argparse

parser = argparse.ArgumentParser(description='Optional app description')
parser.add_argument('--acq_time', type=float,
                    help='Duration of data acquisition, default 30.')
parser.add_argument('--lagplot', type=str,
                    help='Include line on graph for \'corrected\' second channel (second channel with the lag subtracted), default False')
parser.add_argument('--frame_time', type=float,
                    help='Seconds between lag computations and plot refreshes')
parser.add_argument('--output_name', type=str,
                    help='File name prefix for output files, files will be named \'<output_name>_raw.txt\' and \'<output_name>_lag.txt\', default is no output file creation')
parser.add_argument('--data_buffer_length', type=float,
                    help='Seconds of data over which the lag is computed and time domain of output graph, default 3')
parser.add_argument('--plot',type=str,
                    help='Display plot? default True')

args = parser.parse_args()

if args.acq_time is not None:
    acq_time=args.acq_time
else:
    acq_time = 30
    
if args.frame_time is not None:
    frame_time=args.frame_time
else:
    frame_time = 0.01
    
if args.output_name is not None:
    output_name=args.output_name
    outputfile = True
else:
    outputfile = False
    
if args.data_buffer_length is not None:
    data_buffer_length=args.data_buffer_length
else:
    data_buffer_length = 3

if args.plot == "False":
    plot=False
else:
    plot=True

if args.lagplot == "True":
    lagplot=True
else:
    lagplot = False

if outputfile == True:
    raw_data_output_file = open(output_name+"_raw.txt", 'w+') # opening the output files
    lag_data_output_file = open(output_name+"_lag.txt", 'w+') # doing it first so any problems arise before threading makes errors a million times more complicated

def adsTwoChannelStream(data_q, plot_q, duration=30,):    
    try_ports = ['/dev/tty.usbmodemFA131','/dev/ttyACM0','/dev/ttyACM1']
    working_ports = []
    for port in try_ports:
        try:
            ser = serial.Serial(port, baudrate=115200,timeout=1)
            working_ports.append(port)
        except serial.SerialException, err_msg:
            if err_msg.errno == 2:  # Error 2 is 'does not exist', we're just going to mave on
                True
            if err_msg.errno == 13:
                print "Insufficient permission to open port ", port
                
    if len(working_ports) == 0:
       print "Acceptible device not found at any of these ports ", try_ports
    if len(working_ports) > 1:
       print "Multiple ports worked, ", working_ports, ". Specify which port using 'port' argument."
    
    takedata = True     # variable to keep track of whether or not things are working well enough for us to take data
    if len(working_ports) != 1:
        takedata = False
        
    ### On windows machines, the connection takes time to be established and a time.sleep(2) command will be necessary

                # This part syncs this code with the arduino so byte ordered-ness is preserved
    run=False   # Doing this replaces delimiters which would take up serial com time
    
    retry = 0   # counter to keep track of how many times we have tried to reset the Arduino
    
    while run == False and takedata==True:
        print "Syncing with the Arduino... ", retry
        ser.reset_input_buffer() # get that junk outta here
        time.sleep(0.2)     # checks every 0.2 seconds
        arduino_listen = ser.read(1)  # reads the serial port
        if arduino_listen == "g":     # waiting to get the ready signal from the arduino
            ser.write("s")     # send the start signal to the arduino
            ser.reset_input_buffer()
            print "Arduino Sync'd (1 of 2)"
            ser.reset_input_buffer()
            going_back_to_gs = 0
            while run == False and not going_back_to_gs:     # now we just wait for the trigger receipt confirmation
                arduino_listen = ser.read(1)
                if arduino_listen == "k":
                    run = True      # exit the loop and let's go!
                    print "ONWARDSSSSss!! (2 of 2) \n"
                else:
                    print "Sync Phase 2 Failed, starting over\n"
                    going_back_to_gs = 1
        else:
            ser.write("s")
            retry += 1
            time.sleep(0.1)
            ser.reset_input_buffer()
        if retry > 10:
            print "Arduino failed to sync 10 times, exiting. Is the Arduino code right? Is this code right?"
        # I hope no one hates me for making this whole thing one function, I've had problems making global serial vars
    samplecount = 0
    
    starttime=time.time()
    while takedata:# time.time()<starttime+duration and not data_q.full() and not plot_q.full:   # runs for 'duration' seconds or until the queue overflows
    
        if ser.in_waiting > 4:      # waits until the full 4 bytes (2x 2-byte packets) is ready
            if ser.in_waiting > 60: # checking to see if the serial input buffer is over flowing
                print ("Arduino Buffer Overflow. Reading too slow, losing data.")
                takedata=False
            # print([struct.unpack('>h',(ser.read(2)))[0] * 0.1875,struct.unpack('>h',(ser.read(2)))[0] * 0.1875])

            plot_q.put([struct.unpack('>h',(ser.read(2)))[0] * 0.1875,struct.unpack('>h',(ser.read(2)))[0] * 0.1875]) # puts data points in list and appends them to the queue
            if plot_q.qsize() > 99:
                print("Plot Buffer Overflow (Normal Output for Script Timeout)")
                
                takedata=False
            samplecount += 1

    if len(working_ports) == 1:
        ser.write("x")
        ser.close()    
    print("Data Acquisition Exiting \n")


data_queue = Queue.Queue(maxsize=100)
plot_queue = Queue.Queue(maxsize=100)

thread_adsTwoChannelStream=threading.Thread(target=adsTwoChannelStream,args=(data_queue, plot_queue ,acq_time))
#thread_plot=threading.Thread(target=plot_gen,args=(plot_queue,2))

thread_adsTwoChannelStream.start()
#thread_plot.start()


channel0y = np.array([])
channel1y = np.array([])

signalextrema=[0,1]


if outputfile == True:
    raw_data_output_file = open(output_name+"_raw.txt", 'r+')

while plot_queue.empty(): # wait till the data starts coming
    if not thread_adsTwoChannelStream.isAlive():
        sys.exit("Error getting data from Arduino (1)")


data_start_time = time.time()
data_buffer_stop_time = data_start_time + data_buffer_length
alert_time = time.time()

while time.time() < data_buffer_stop_time:
    if not thread_adsTwoChannelStream.isAlive():
        sys.exit("Error getting data from Arduino (2)")
    new_data = plot_queue.get(timeout=1)
    channel0y = np.append(channel0y,new_data[0])
    channel1y = np.append(channel1y,new_data[1])
    if outputfile == True:
        new_data.insert(0,time.time()-data_start_time)
        raw_data_output_file.write(str(new_data)+"\n")
    if alert_time < time.time():
        print "Gathering Data to Perform First Lag Calculation", int(data_buffer_stop_time - time.time())
        alert_time = time.time() + 1

if plot:    
    channelx = np.linspace(0,data_buffer_length,len(channel0y))
    #channel0y = channel0y - np.mean(channel0y)# subtracting averages
    #channel1y = channel1y - np.mean(channel1y)# subtracting averages

    signalextrema = [min(np.append(channel0y,channel1y))-1,max(np.append(channel0y,channel1y))+1]
    
    plt.ion()
    
    fig = plt.figure()
    ax = fig.add_subplot(111)
    line1, = ax.plot(channelx,channel0y, 'r-') # Returns a tuple of line objects, thus the comma
    line2, = ax.plot(channelx,channel1y, 'b-')
    
    
    
    if lagplot == True:
        line3, = ax.plot(channelx,channel1y, 'k-')
    axes = plt.gca()
    
    lag_s=""
    font = {'family': 'serif',
            'color':  'darkred',
            'weight': 'normal',
            'size': 16,
            }
    plt.xlabel('Time (s)', fontdict=font)
    plt.ylabel('Voltage (mV)', fontdict=font)
    #plottext = plt.text(1.5, 1500, 'Lag: %s'%(lag), fontdict=font)
    plottitle = plt.title('Lag: %s'%(lag_s), fontdict=font)

lastplot = time.time()
    
lag_data = []

print "Doing the data thing for the next", int(data_start_time+acq_time-time.time()), "seconds..."

while not plot_queue.full() and time.time() < data_start_time + acq_time:
    if not thread_adsTwoChannelStream.isAlive():
        sys.exit("Error getting data from Arduino (3)")
    if not plot_queue.empty():
        new_data = plot_queue.get()
        
        channel0y = np.append(channel0y[1:],new_data[0])
        channel1y = np.append(channel1y[1:],new_data[1])
        if outputfile == True:
            new_data.insert(0,time.time()-data_start_time)
            raw_data_output_file.write(str(new_data)+"\n")
    if time.time()-lastplot > frame_time:
        samplerate = float(len(channel1y)/data_buffer_length)
        old_data = np.column_stack((channel0y,channel1y))
        lag = lag_xcorr_old(old_data,samplerate)
        lag_data.append([time.time()-data_start_time,lag])
        lag_s = lag/samplerate
        
        if plot:
            signalextrema = [min(np.append(channel0y,channel1y))-1,max(np.append(channel0y,channel1y))+1]
            axes.set_ylim(signalextrema)
    
    
    #        lag = lag_xcorr(data=[channel0y,channel1y], samprate=samplerate)[0][0]
    #        plottext.set_text('Lag: %s'%(lag))
            plottitle.set_text('Lag: %s'%(round(lag_s,4)))
            
            if lagplot == True:
                line3.set_xdata(np.linspace(0,data_buffer_length,len(channel0y))+(lag / samplerate))
                line3.set_ydata(channel1y)
                
            
            line1.set_ydata(channel0y)
            line2.set_ydata(channel1y)
            fig.canvas.draw()
            fig.canvas.flush_events()
        
        # print '{0:.16f}'.format( float(lag/samplerate)), samplerate
        
        lastplot = time.time()

if plot_queue.full():
    print "Queue full, try increasing the minimum time between frames (frame_time)"

if outputfile:
    lag_data_output_file.write(str(lag_data[:]))
        
    
if outputfile == True:
    raw_data_output_file.close()
    
while thread_adsTwoChannelStream.isAlive():
    print "Waiting for Data Acquisition Script to Exit (Buffer Overflow) ", plot_queue.qsize()
    time.sleep(1)
#stoptime=time.time() + 30
#while (time.time() < stoptime):
#    if data_queue.qsize() > 0:
#        time.sleep(1)



"""
fig = plt.figure()
ax = plt.axes(xlim=(0, 2), ylim=(-2, 2))
line, = ax.plot([], lw=2)
def init():
    line.set_data([], [])
    return line,

# animation function.  This is called sequentially
def animate(i):
    x = np.linspace(0, 2, 1000)
    y = np.sin(1 * np.pi *(x + 0.01 * i))
    line.set_data(x, y)
    return line,

# call the animator.  blit=True means only re-draw the parts that have changed.
anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=200, interval=20, blit=True)

# save the animation as an mp4.  This requires ffmpeg or mencoder to be
# installed.  The extra_args ensure that the x264 codec is used, so that
# the video can be embedded in html5.  You may need to adjust this for
# your system: for more information, see
# http://matplotlib.sourceforge.net/api/animation_api.html
# anim.save('basic_animation.mp4', fps=30, extra_args=['-vcodec', 'libx264'])

plt.show()
*/
"""