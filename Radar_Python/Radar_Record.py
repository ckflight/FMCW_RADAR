# pip3 install pyserial

import serial.tools.list_ports
import numpy as np
import time
import os
import binascii

'''

    This code records radar data stream to a text file.
    Radar data format is: directly data 512(16bit) = 1024 bytes each time.

'''

# Best setting: 1.03e-3, 1200e-6, 5.85e9, 100e6, 500000, 500, 1, 0, 1
RECORD_TIME         = 10            # in sec
SWEEP_TIME          = 1.05e-3       # in sec
SWEEP_GAP           = 1000.0e-6        # in sec max 1500
SWEEP_START         = 5.8e9
SWEEP_BW            = 200e6
SWEEP_TYPE          = 0             # 0 for Sawtooth, 1 for Triangular
SAMPLING_FREQUENCY  = 250000
NUMBER_OF_SAMPLES   = 250           # NUMBER_OF_SAMPLES(16bit) = SAMPLING_FREQUENCY * SWEEP_TIME(int), max 2000
TX_MODE             = 1            # 0 for continuous tx, 1 for on off with tx
TX_POWER_DBM        = 0
GAIN                = 1             # Last gain stage (1 to 47) gain = -(4.7K / (100 + MCP4022)) MCP4022 max value is 5.1K for 63 decimal

i_freq = 1e3
hz_per_m = 0
if SWEEP_BW:
    hz_per_m = int(1000 * ((3e8 * SWEEP_TIME * i_freq) / (2 * SWEEP_BW)))

SWEEP_FREQ          = 1 / (SWEEP_TIME + SWEEP_GAP)
RECORD_COUNTER      = int(RECORD_TIME * SWEEP_FREQ)

def Serial_Init():

    isPortConnected = False
    deviceId = ''

    # Find STM32 USB Device Id
    while not isPortConnected:
        ports = serial.tools.list_ports.comports()

        for device_ in ports:

            stm_string = device_.device

            for i in range(len(stm_string) - 8):

                str_to_look = "usbmodem"

                if stm_string[i: i + 8] == str_to_look:

                    device_id_number = stm_string[i + 8: len(stm_string)]
                    deviceId = "/dev/cu." + str_to_look + device_id_number
                    print(device_)
                    isPortConnected = True

                    break

    ser = serial.Serial(
        port=deviceId,
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_TWO,
        bytesize=serial.EIGHTBITS,
        #timeout=0.01 # can be None
        timeout=None
    )

    # Clear all input data
    ser.flushInput()

    return ser


def Configuration_Process():

    isDone = False
    state = 0

    while not isDone:

        # Send start condition and hardware configuration paramaters
        if state == 0:

            ser.write("==".encode('ascii'))

            sw_t = np.uint16(SWEEP_TIME * 1e6)
            sw_t_msb = np.uint8((sw_t >> 8) & 0xFF)
            sw_t_lsb = np.uint8(sw_t & 0xFF)

            ser.write(binascii.hexlify(sw_t_msb))
            ser.write(binascii.hexlify(sw_t_lsb))

            sw_g = np.uint16(SWEEP_GAP * 1e6)
            sw_g_msb = np.uint8((sw_g >> 8) & 0xFF)
            sw_g_lsb = np.uint8(sw_g & 0xFF)

            ser.write(binascii.hexlify(sw_g_msb))
            ser.write(binascii.hexlify(sw_g_lsb))

            rec_t = np.uint8(RECORD_TIME)
            ser.write(binascii.hexlify(rec_t))

            fs = np.uint16(SAMPLING_FREQUENCY / 1e3)
            fs_msb = np.uint8((fs >> 8) & 0xFF)
            fs_lsb = np.uint8(fs & 0xFF)

            ser.write(binascii.hexlify(fs_msb))
            ser.write(binascii.hexlify(fs_lsb))

            num_sample = np.uint16(NUMBER_OF_SAMPLES)
            num_sample_msb = np.uint8((num_sample >> 8) & 0xFF)
            num_sample_lsb = np.uint8(num_sample & 0xFF)

            ser.write(binascii.hexlify(num_sample_msb))
            ser.write(binascii.hexlify(num_sample_lsb))

            sweep_start = np.uint16(SWEEP_START / 1e7)
            sweep_start_msb = np.uint8((sweep_start >> 8) & 0xFF)
            sweep_start_lsb = np.uint8(sweep_start & 0xFF)

            ser.write(binascii.hexlify(sweep_start_msb))
            ser.write(binascii.hexlify(sweep_start_lsb))

            sweep_bw = np.uint16(SWEEP_BW / 1e6)
            sweep_bw_msb = np.uint8((sweep_bw >> 8) & 0xFF)
            sweep_bw_lsb = np.uint8(sweep_bw & 0xFF)

            ser.write(binascii.hexlify(sweep_bw_msb))
            ser.write(binascii.hexlify(sweep_bw_lsb))

            tx_mode = np.uint8(TX_MODE)
            ser.write(binascii.hexlify(tx_mode))

            n = (((4700 / GAIN) - 100) * 64) / 5100
            gain_resistance_decimal = np.uint8(n)
            ser.write(binascii.hexlify(gain_resistance_decimal))

            sw_type = np.uint8(SWEEP_TYPE)
            ser.write(binascii.hexlify(sw_type))

            state = 1
            pass

        # Receive response and tx power from radar
        elif state == 1:

            # Wait until response is received or timeout
            while ser.inWaiting() == 0 and state == 1:
                pass

            if ser.inWaiting() > 0:
                config_read = ser.read(ser.inWaiting())

                if config_read[0:2] == str("==").encode('ascii'):

                    dbm = config_read[2]
                    isDone = True
            pass

    return dbm

if os.path.exists("data_record.txt"):
    os.remove("data_record.txt")
data_record_file = open("data_record.txt", "w")

ser = Serial_Init()
TX_POWER_DBM = Configuration_Process()

rx_counter = 0
isRecordDone = 0

data_record_file.write(str(RECORD_COUNTER))
data_record_file.write("\r\n")
data_record_file.write(str(RECORD_TIME))
data_record_file.write("\r\n")
data_record_file.write(str(int(SWEEP_TIME * 1000000)))
data_record_file.write("\r\n")
data_record_file.write(str(int(SWEEP_GAP * 1000000)))
data_record_file.write("\r\n")
data_record_file.write(str(SAMPLING_FREQUENCY))
data_record_file.write("\r\n")
data_record_file.write(str(NUMBER_OF_SAMPLES))
data_record_file.write("\r\n")
data_record_file.write(str(TX_MODE))
data_record_file.write("\r\n")
data_record_file.write(str(TX_POWER_DBM))
data_record_file.write("\r\n")
data_record_file.write(str(hz_per_m))
data_record_file.write("\r\n")

start_time = time.time()

while not isRecordDone:

    read_size = ser.inWaiting()

    # Read if available
    #if read_size > 0:

    calculation_start = time.time()

    # if fix byte is read first and last transfers will be ignored
    rx_data = ser.read(NUMBER_OF_SAMPLES * 2)  # 16 bit NUMBER_OF_SAMPLES = 8 bit 2*NUMBER_OF_SAMPLES
    rx_data_str = str(binascii.b2a_hex(rx_data))  # takes hex values and stores them as string working perfectly

    rx_data_length = len(rx_data_str)

    # b'.....' so remove these 3 bytes
    data_record_file.write(rx_data_str[2:len(rx_data_str) - 1])
    data_record_file.write("\r\n")

    rx_counter += 1;
    if rx_counter == RECORD_COUNTER:
        isRecordDone = 1
        done_time = time.time() - start_time
        print("Completed in ", str(round(done_time, 1)), "second.")

    calculation_done = time.time() - calculation_start

    print("Calculation time: ", str(round(calculation_done, 4)), "   rx counter: ", str(rx_counter))
    #print(rx_counter)



print("TX Power: ", str(TX_POWER_DBM))
print("Meters per KHz: ",str(hz_per_m))
exit(1)









