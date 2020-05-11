# pip3 install pyserial

import serial.tools.list_ports
import numpy as np
import matplotlib.pyplot as mp
import peakutils
import time

'''
    This code records radar data stream to a text file.
    Radar data format is: directly data 512(16bit) = 1024 bytes each time.

    READ HERE:
    If i sample 10k in one second then i can plot one fft upto 5KHz once in a second.
    But i need higher update rate for good plot so i can sample 10K in 0.1 sec for example and plot fft upto 5k every 0.1sec
    To achieve that my system should be able to sample 10K * 10 = 100K samples in one second.
    44100 samples can give fft of 20k bandwidth for one sec or
    512*20 samples can give fft of 10K bandwitdh and since Fs = 200K
    i can take 512*20 samples of fft every 512*20 / Fs seconds. 

'''

record_file = open("data_record.txt", "r")

data = str(record_file.readline())
RECORD_COUNTER = int(data[0:len(data)-1])
print("Record Counter: ", str(RECORD_COUNTER))

data = str(record_file.readline())
RECORD_TIME = int(data[0:len(data)-1])
print("Record Time: ", str(RECORD_TIME), " sec.")

data = str(record_file.readline())
SWEEP_TIME = int(data[0:len(data)-1]) / 1000000
print("Sweep Time : ", str(SWEEP_TIME), " microsec.")

data = str(record_file.readline())
SWEEP_GAP = int(data[0:len(data)-1]) / 1000000
print("Sweep Gap : ", str(SWEEP_GAP), " microsec.")

data = str(record_file.readline())
SAMPLING_FREQUENCY = int(data[0:len(data)-1])
print("Sampling Freqeuncy : ", str(SAMPLING_FREQUENCY), " Hz.")

data = str(record_file.readline())
NUMBER_OF_SAMPLES = int(data[0:len(data)-1])
print("Samples per sweep : ", str(NUMBER_OF_SAMPLES))

data = str(record_file.readline())
TX_MODE = int(data[0:len(data)-1])
print("Tx Mode : ", str(TX_MODE))

data = str(record_file.readline())
TX_POWER_DBM = int(data[0:len(data)-1])
print("Tx Power : ", str(TX_POWER_DBM), " dBm.")

data = str(record_file.readline())
hz_per_m = int(data[0:len(data)-1])
print("Hz per m : ", str(hz_per_m))

FREQ_RANGE      = 10000

def Moving_Average_Filter(a, n = 10):

    ret = np.cumsum(a, dtype=float)
    ret[n:] = ret[n:] - ret[:-n]
    return ret[n - 1:] / n


def FFT_Calculate(sample_data, sample_period, window = None, average = 1):

    if window:
        w = window(len(sample_data))
        sample_data = [sample_data[i] * w[i] for i in range(len(w))]

    fs = 0.5/float(sample_period)
    fft_ = np.fft.rfft(sample_data)

    averages = average
    fft_ = Moving_Average_Filter(np.abs(fft_), averages)
    bins = len(fft_)
    f_step = fs/bins
    fx = [f_step*i for i in range(0, bins)]

    #peakIndexes = peakutils.indexes(finalFFT, 0.025, min_dist=50, thres_abs=True)

    return fx, fft_

def Time_Plot(samples):

    time_range = 0.1  # sec
    time_step = np.linspace(0.0, time_range, len(samples))

    ax1.clear()

    # mp.ylim(2, 3)  # limit y axis between var1 and var2
    # mp.xlim(0, len(time_step))  # limit y axis between var1 and var2

    mp.xlabel('Time')
    mp.ylabel('Value')

    mp.grid()

    ax1.plot(time_step, samples, 'c')

    # Calculate
    mp.pause(0.01)

def Freq_Plot(samples, freq_bin, sampling_freq, freq_range, ylim, average):

    if freq_range >= freq_bin:
        freq_range = freq_bin

    fx, fy = FFT_Calculate(samples, 1 / sampling_freq, np.hamming, average)

    ax1.clear()

    mp.ylim(0, ylim)  # limit y axis between var1 and var2
    mp.xlim(0, len(fx)/(freq_bin / (freq_range * 2)))  # limit x axis between var1 and var2
    #mp.xlim(0, len(fx))  # limit x axis between var1 and var2

    mp.xlabel('Freq')
    mp.ylabel('Amplitude')

    mp.grid()

    ax1.plot(fx, fy, 'c')

    mp.pause(freq_bin/sampling_freq)
    #time.sleep(freq_bin/sampling_freq)

def Freq_Plot2(samples, sampling_freq, ylim, average):

    fx, fy = FFT_Calculate(samples, 1 / sampling_freq, np.hamming, average)

    ax1.clear()

    mp.ylim(0, ylim)  # limit y axis between var1 and var2
    mp.xlim(0, len(fx))  # limit x axis between var1 and var2

    mp.xlabel('Freq')
    mp.ylabel('Amplitude')

    mp.grid()

    ax1.plot(fx, fy, 'c')

    mp.pause(0.01)


fig = mp.figure(figsize=(12, 6))
ax1 = fig.add_subplot(1, 1, 1)

# Decode data_record
sample_array = []

# Number of lines in the recorded text file. I set it to a large number for now.
line_number_textFile = 0

while line_number_textFile < RECORD_COUNTER:

    sample_line = record_file.readline()
    samples_hex = bytes.fromhex(sample_line) # get hex data from string
    length_line = len(samples_hex)

    if length_line > 0:

        index = 0
        while index < length_line:

            current_sample_16bit = (samples_hex[index] << 8) | (samples_hex[index+1])

            current_sample_float = (current_sample_16bit / 2**10) * 5.0

            index += 2
            sample_array.append(round(current_sample_float, 3))

            if len(sample_array) == FREQ_RANGE:

                #Time_Plot(sample_array)
                #Freq_Plot(sample_array, FREQ_RANGE, SAMPLING_FREQUENCY, freq_range=250000, ylim=10000, average=10)
                Freq_Plot2(sample_array, SAMPLING_FREQUENCY, ylim=1000, average=1)

                sample_array.clear()

        # move to the next line
        line_number_textFile += 1

    else:
        # all done exit loop
        line_number_textFile += 1

exit(1)