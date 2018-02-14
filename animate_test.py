import numpy as np
from matplotlib import pyplot as plt
from matplotlib import animation

from xcorr_func2 import lag_xcorr
from xcorr_func import lag_xcorr as lag_xcorr_old

import Queue
import threading
import time

# Had problems importing EQMon_test_trigger.py, so here it is

import serial
import struct
import sys

acq_time = 98098
lagplot=False
frame_time = 0.01
outputfile = False
data_buffer_length = 3 
raw_output = 'raw_data.txt'

def adsTwoChannelStream(data_q, plot_q, duration=30,):    
#    serialport='/dev/tty.usbmodemFA131'   # I think this is the front usb port on my MacBook
    serialport='/dev/ttyACM0'
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
                    print "ONWARDS!"
        # I hope no one hates me for making this whole thing one function, I've had problems making global serial vars
    samplecount = 0
    starttime=time.time()
    while True:# time.time()<starttime+duration and not data_q.full() and not plot_q.full:   # runs for 'duration' seconds or until the queue overflows
    
        if ser.in_waiting > 4:      # waits until the full 4 bytes (2x 2-byte packets) is ready
            if ser.in_waiting > 60: # checking to see if the serial input buffer is over flowing
                ser.close()
                sys.exit("Arduino Buffer Overflow. Reading too slow, losing data.")
            # print([struct.unpack('>h',(ser.read(2)))[0] * 0.1875,struct.unpack('>h',(ser.read(2)))[0] * 0.1875])
            plot_q.put([struct.unpack('>h',(ser.read(2)))[0] * 0.1875,struct.unpack('>h',(ser.read(2)))[0] * 0.1875]) # puts data points in list and appends them to the queue
            samplecount += 1

    ser.write("x")
    ser.close()    
    print("Donezo!")
    return samplecount


data_queue = Queue.Queue(maxsize=100)
plot_queue = Queue.Queue(maxsize=100)

thread_adsTwoChannelStream=threading.Thread(target=adsTwoChannelStream,args=(data_queue, plot_queue ,acq_time))
#thread_plot=threading.Thread(target=plot_gen,args=(plot_queue,2))

thread_adsTwoChannelStream.start()
#thread_plot.start()


channel0y = np.array([])
channel1y = np.array([])

signalextrema=[0,0]



raw_data_output_file = open(raw_output, 'r+')

while plot_queue.empty(): # wait till the data starts coming
    True


data_start_time = time.time()
data_buffer_stop_time = data_start_time + data_buffer_length
alert_time = time.time()

while time.time() < data_buffer_stop_time:
    new_data = plot_queue.get()
    raw_data_output_file.write(new_data)
    channel0y = np.append(channel0y,new_data[0])
    channel1y = np.append(channel1y,new_data[1])
    if alert_time < time.time():
        print "Gathering Data to Populate Graph", int(data_buffer_stop_time - time.time())
        alert_time = time.time() + 1
    

channelx = np.linspace(0,data_buffer_length,len(channel0y))

#channel0y = channel0y - np.mean(channel0y)# subtracting averages
#channel1y = channel1y - np.mean(channel1y)# subtracting averages


signalextrema = [min(np.append(channel0y,channel1y)),max(np.append(channel0y,channel1y))]

plt.ion()

fig = plt.figure()
ax = fig.add_subplot(111)
line1, = ax.plot(channelx,channel0y, 'r-') # Returns a tuple of line objects, thus the comma
line2, = ax.plot(channelx,channel1y, 'b-')



if lagplot == True:
    line3, = ax.plot(channelx,channel1y, 'k-')
axes = plt.gca()

lag=""
font = {'family': 'serif',
        'color':  'darkred',
        'weight': 'normal',
        'size': 16,
        }
plt.xlabel('Time (s)', fontdict=font)
plt.ylabel('Voltage (mV)', fontdict=font)
#plottext = plt.text(1.5, 1500, 'Lag: %s'%(lag), fontdict=font)
plottitle = plt.title('Lag: %s'%(lag), fontdict=font)
lastplot = time.time()

lag_data = []


while not plot_queue.full() and time.time() < data_start_time + acq_time:
    if not plot_queue.empty():
        new_data = plot_queue.get()
        raw_data_output_file.write(new_data)
        channel0y = np.append(channel0y[1:],new_data[0])
        channel1y = np.append(channel1y[1:],new_data[1])
    if time.time()-lastplot > frame_time:
#        channel0y = channel0y - np.mean(channel0y)# subtracting averages
#        channel1y = channel1y - np.mean(channel1y)# subtracting averages
#        signalextrema = [min(np.append(channel0y,channel1y)),max(np.append(channel0y,channel1y))]
        signalextrema = [min(np.append(channel0y,channel1y)),max(np.append(channel0y,channel1y))]
        axes.set_ylim(signalextrema)

        samplerate = len(channel1y)/data_buffer_length
        old_data = np.column_stack((channel0y,channel1y))
        lag = lag_xcorr_old(old_data,samplerate)
#        lag = lag_xcorr(data=[channel0y,channel1y], samprate=samplerate)[0][0]
#        plottext.set_text('Lag: %s'%(lag))
        plottitle.set_text('Lag: %s'%(lag))
        
        lag_data.append([time.time()-data_start_time,lag])
        
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
elif outputfile:
    with open(outputfile, 'a') as the_file:
        the_file.write(str(lag_data))

else:
    print lag_data
    
raw_data_output_file.close()
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