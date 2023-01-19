# pip3 install pyserial

import serial.tools.list_ports
import numpy as np
import matplotlib.pyplot as mp
import peakutils
import time

from PyQt5 import QtWidgets, QtCore
from vispy.scene import SceneCanvas, visuals
from vispy.app import use_app
from vispy import scene

record_file = open("Radar_Records/_rcmp76KHZ_5.5G_400MHZ_1.txt", "r")

data = str(record_file.readline())
RECORD_COUNTER = int(data[0:len(data) - 1])
print("Record Counter: ", str(RECORD_COUNTER))

data = str(record_file.readline())
RECORD_TIME = int(data[0:len(data) - 1])
print("Record Time: ", str(RECORD_TIME), " sec.")

data = str(record_file.readline())
SWEEP_TIME = int(data[0:len(data) - 1]) / 1000000
print("Sweep Time : ", str(SWEEP_TIME), " microsec.")

data = str(record_file.readline())
SWEEP_GAP = int(data[0:len(data) - 1]) / 1000000
print("Sweep Gap : ", str(SWEEP_GAP), " microsec.")

data = str(record_file.readline())
SAMPLING_FREQUENCY = int(data[0:len(data) - 1])
print("Sampling Freqeuncy : ", str(SAMPLING_FREQUENCY), " Hz.")

data = str(record_file.readline())
NUMBER_OF_SAMPLES = int(data[0:len(data) - 1])
print("Samples per sweep : ", str(NUMBER_OF_SAMPLES))

data = str(record_file.readline())
TX_MODE = int(data[0:len(data) - 1])
print("Tx Mode : ", str(TX_MODE))

data = str(record_file.readline())
TX_POWER_DBM = int(data[0:len(data) - 1])
print("Tx Power : ", str(TX_POWER_DBM), " dBm.")

data = str(record_file.readline())
hz_per_m = int(data[0:len(data) - 1])
print("Hz per m : ", str(hz_per_m))

data = str(record_file.readline())
DATA_LOG = int(data[0:len(data) - 1])
print("Data Log : ", str(DATA_LOG))

data = str(record_file.readline())
ADC_SELECT = int(data[0:len(data) - 1])
print("ADC Select : ", str(ADC_SELECT))

line_number_textFile = 0

is_restart_clicked = False

CANVAS_SIZE = (1200, 700)  # (width, height)

# freq plot variables
MAX_FREQ_RANGE = SAMPLING_FREQUENCY / 2 # for 933KHz sampling 933K / 2 is max range
FREQ_RANGE = MAX_FREQ_RANGE / 1
AVERAGING_NUM = 120

FREQ_NUM_LINE_POINTS = int((FREQ_RANGE / 2)) + 2 - AVERAGING_NUM # this is the array size according to averaging.

FREQ_X_LIMIT_MIN = 0.0
FREQ_X_LIMIT_MAX = float(FREQ_RANGE)

FREQ_Y_LIMIT_MIN = 0
FREQ_Y_LIMIT_MAX = 100

# time plot variables
TIME_MS = 20
TIME_NUM_LINE_POINTS = NUMBER_OF_SAMPLES * TIME_MS # TIME_MS millisecond range plot

TIME_X_LIMIT_MIN = 0.0
TIME_X_LIMIT_MAX = float(TIME_MS / 1000)

TIME_Y_LIMIT_MIN = 1.0
TIME_Y_LIMIT_MAX = 2.5

# calculate this so that if record is 10 sec plot should take 10 seconds.
#FREQ_TIME_BETWEEN_PLOT = (FREQ_RANGE / SAMPLING_FREQUENCY) * 2 # Full freq range takes 1 second, half range 0.5 sec
#FREQ_TIME_BETWEEN_PLOT = TIME_BETWEEN_PLOT * 1  # i add this line to change plot speed. multp for slower div for faster

TIME_COLOR_CHOICES = ["black", "red", "blue"]
TIME_SCALE_CHOICES = ["scale/1","scale/2", "scale/4", "scale/8", "scale/16", "scale/32"]

FREQ_COLOR_CHOICES = ["black", "red", "blue"]
FREQ_SCALE_CHOICES = ["scale/1","scale/2", "scale/4", "scale/8", "scale/16", "scale/32"]

# Decode data_record
freq_sample_array = []
time_sample_array = []

USB_PLOT = 1
MICROCARD_PLOT = 0

def Moving_Average_Filter(a, n = 1):

    ret = np.cumsum(a, dtype=float)
    ret[n:] = ret[n:] - ret[:-n]
    return ret[n - 1:] / n


def FFT_Calculate(sample_data, sample_period, window = None, average = 1):

    if window:
        w = window(len(sample_data))
        sample_data = [sample_data[i] * w[i] for i in range(len(w))]

    fs = 0.5/float(sample_period)
    fft_ = np.fft.rfft(sample_data)

    #fft2_ = pyfftw.FFTW(sample_data);

    averages = average
    fft_ = Moving_Average_Filter(np.abs(fft_), averages)
    bins = len(fft_)
    f_step = fs/bins
    fx = [f_step*i for i in range(0, bins)]

    #peakIndexes = peakutils.indexes(finalFFT, 0.025, min_dist=50, thres_abs=True)

    return fx, fft_

class Controls(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        layout = QtWidgets.QVBoxLayout()

        self.time_color_label = QtWidgets.QLabel("Time Color:")
        layout.addWidget(self.time_color_label)
        self.time_color_chooser = QtWidgets.QComboBox()
        self.time_color_chooser.addItems(TIME_COLOR_CHOICES)
        layout.addWidget(self.time_color_chooser)

        self.freq_color_label = QtWidgets.QLabel("Freq Color:")
        layout.addWidget(self.freq_color_label)
        self.freq_color_chooser = QtWidgets.QComboBox()
        self.freq_color_chooser.addItems(FREQ_COLOR_CHOICES)
        layout.addWidget(self.freq_color_chooser)

        self.time_scale_label = QtWidgets.QLabel("Time Scale:")
        layout.addWidget(self.time_scale_label)
        self.time_scale_chooser = QtWidgets.QComboBox()
        self.time_scale_chooser.addItems(TIME_SCALE_CHOICES)
        layout.addWidget(self.time_scale_chooser)

        self.freq_scale_label = QtWidgets.QLabel("Freq Scale:")
        layout.addWidget(self.freq_scale_label)
        self.freq_scale_chooser = QtWidgets.QComboBox()
        self.freq_scale_chooser.addItems(FREQ_SCALE_CHOICES)
        layout.addWidget(self.freq_scale_chooser)

        self.restart_label = QtWidgets.QLabel("Restart Plot")
        layout.addWidget(self.restart_label)
        self.restart_button = QtWidgets.QPushButton()
        self.restart_button.setText("Restart")
        layout.addWidget(self.restart_button)


        layout.addStretch(1)
        self.setLayout(layout)


class CanvasWrapper:
    def __init__(self):
        self.canvas = SceneCanvas(size=CANVAS_SIZE)
        self.grid = self.canvas.central_widget.add_grid()

        global line_number_textFile

        # Time Plot
        # color array
        print("TIME_NUM_LINE_POINTS",TIME_NUM_LINE_POINTS)
        color = np.ones((TIME_NUM_LINE_POINTS, 4), dtype=np.float32)
        color[:, 0] = np.linspace(0, 1, TIME_NUM_LINE_POINTS)
        color[:, 1] = color[::-1, 0]

        time_plot_data = np.zeros((TIME_NUM_LINE_POINTS, 2), dtype=np.float32) # 2x6000 array

        # background color
        self.view_time_plot = self.grid.add_view(0, 0, bgcolor='#063970')
        self.time_line = visuals.Line(time_plot_data, parent=self.view_time_plot.scene, color=color)
        self.view_time_plot.camera = "panzoom"

        print("TIME_X_LIMIT_MIN", TIME_X_LIMIT_MIN)
        print("TIME_X_LIMIT_MAX", TIME_X_LIMIT_MAX)
        print("TIME_Y_LIMIT_MIN", TIME_Y_LIMIT_MIN)
        print("TIME_Y_LIMIT_MAX", TIME_Y_LIMIT_MAX)
        self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN, TIME_X_LIMIT_MAX),
                                             y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))

        # add x axis
        xaxis1 = scene.AxisWidget(orientation='top', axis_label='X Axis', axis_font_size=10, axis_label_margin=6, tick_label_margin=3)
        xaxis1.height_max = CANVAS_SIZE[1]

        self.grid.add_widget(xaxis1, row=0, col=0)
        xaxis1.link_view(self.view_time_plot)

        # add y axis
        yaxis1 = scene.AxisWidget(orientation='left', axis_label='Y Axis', axis_font_size=10, axis_label_margin=6, tick_label_margin=3)
        yaxis1.width_max = 0 # distance from right side of the window

        self.grid.add_widget(yaxis1, row=0, col=1)
        yaxis1.link_view(self.view_time_plot)




        # Freq Plot ***************************
        # color array
        print("FREQ_NUM_LINE_POINTS", FREQ_NUM_LINE_POINTS)
        color = np.ones((FREQ_NUM_LINE_POINTS, 4), dtype=np.float32)
        color[:, 0] = np.linspace(0, 1, FREQ_NUM_LINE_POINTS)
        color[:, 1] = color[::-1, 0]

        freq_plot_data = np.zeros((FREQ_NUM_LINE_POINTS, 2), dtype=np.float32)  # 2x6000 array

        # background color
        self.view_freq_plot = self.grid.add_view(1, 0, bgcolor='#063970')
        self.freq_line = visuals.Line(freq_plot_data, parent=self.view_freq_plot.scene, color=color)
        self.view_freq_plot.camera = "panzoom"

        print("FREQ_X_LIMIT_MIN", FREQ_X_LIMIT_MIN)
        print("FREQ_X_LIMIT_MAX", FREQ_X_LIMIT_MAX)
        print("FREQ_Y_LIMIT_MIN", FREQ_Y_LIMIT_MIN)
        print("FREQ_Y_LIMIT_MAX", FREQ_Y_LIMIT_MAX)
        self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN, FREQ_X_LIMIT_MAX),
                                             y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))

        # add x axis
        xaxis2 = scene.AxisWidget(orientation='top', axis_label='X Axis', axis_font_size=10, axis_label_margin=6,
                                 tick_label_margin=3)
        xaxis2.height_max = CANVAS_SIZE[1] / 2

        self.grid.add_widget(xaxis2, row=1, col=0)
        xaxis2.link_view(self.view_freq_plot)

        # add y axis
        yaxis2 = scene.AxisWidget(orientation='left', axis_label='Y Axis', axis_font_size=10, axis_label_margin=6,
                                 tick_label_margin=3)
        yaxis2.width_max = 0  # distance from right side of the window

        self.grid.add_widget(yaxis2, row=1, col=1)
        yaxis2.link_view(self.view_freq_plot)


    def set_time_color(self, color):
        print(f"Changing line color to {color}")
        self.time_line.set_data(color=color)

    def set_freq_color(self, color):
        print(f"Changing line color to {color}")
        self.freq_line.set_data(color=color)

    def set_time_scale(self, scale):
        if scale == "scale/1":
            self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN / 1, TIME_X_LIMIT_MAX / 1),
                                           y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))
        if scale == "scale/2":
            self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN / 2, TIME_X_LIMIT_MAX / 2),
                                           y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))
        elif scale == "scale/4":
            self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN / 4, TIME_X_LIMIT_MAX / 4),
                                           y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))
        elif scale == "scale/8":
            self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN / 8, TIME_X_LIMIT_MAX / 8),
                                           y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))
        elif scale == "scale/16":
            self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN / 16, TIME_X_LIMIT_MAX / 16),
                                           y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))
        elif scale == "scale/32":
            self.view_time_plot.camera.set_range(x=(TIME_X_LIMIT_MIN / 32 , TIME_X_LIMIT_MAX / 32),
                                           y=(TIME_Y_LIMIT_MIN, TIME_Y_LIMIT_MAX))


    def set_freq_scale(self, scale):
        if scale == "scale/1":
            self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN / 1, FREQ_X_LIMIT_MAX / 1),
                                           y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))
        if scale == "scale/2":
            self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN / 2, FREQ_X_LIMIT_MAX / 2),
                                           y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))
        elif scale == "scale/4":
            self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN / 4, FREQ_X_LIMIT_MAX / 4),
                                           y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))
        elif scale == "scale/8":
            self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN / 8, FREQ_X_LIMIT_MAX / 8),
                                           y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))
        elif scale == "scale/16":
            self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN / 16, FREQ_X_LIMIT_MAX / 16),
                                           y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))
        elif scale == "scale/32":
            self.view_freq_plot.camera.set_range(x=(FREQ_X_LIMIT_MIN / 32, FREQ_X_LIMIT_MAX / 32),
                                           y=(FREQ_Y_LIMIT_MIN, FREQ_Y_LIMIT_MAX))

    def restart_plot(self):
        print("Restart is clicked")
        global is_restart_clicked
        is_restart_clicked = True

    def update_data(self, new_data_dict):
        #print("Updating data...")
        self.time_line.set_data(new_data_dict["time"])
        self.freq_line.set_data(new_data_dict["freq"])

class MyMainWindow(QtWidgets.QMainWindow):
    closing = QtCore.pyqtSignal()

    def __init__(self, canvas_wrapper: CanvasWrapper, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.setFixedWidth(CANVAS_SIZE[0])
        self.setFixedHeight(CANVAS_SIZE[1])

        central_widget = QtWidgets.QWidget()
        main_layout = QtWidgets.QHBoxLayout()

        self._controls = Controls()
        main_layout.addWidget(self._controls)
        self._canvas_wrapper = canvas_wrapper
        main_layout.addWidget(self._canvas_wrapper.canvas.native)

        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

        self._connect_controls()

    def _connect_controls(self):
        self._controls.time_color_chooser.currentTextChanged.connect(self._canvas_wrapper.set_time_color)
        self._controls.freq_color_chooser.currentTextChanged.connect(self._canvas_wrapper.set_freq_color)
        self._controls.time_scale_chooser.currentTextChanged.connect(self._canvas_wrapper.set_time_scale)
        self._controls.freq_scale_chooser.currentTextChanged.connect(self._canvas_wrapper.set_freq_scale)
        self._controls.restart_button.clicked.connect(self._canvas_wrapper.restart_plot)

    def closeEvent(self, event):
        print("Closing main window!")
        self.closing.emit()
        return super().closeEvent(event)


class DataSource(QtCore.QObject):

    new_data = QtCore.pyqtSignal(dict)
    finished = QtCore.pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self._should_end = False

        # create arrays
        self._time_domain_data = np.zeros((TIME_NUM_LINE_POINTS, 2), dtype = np.float32) # 2 x NUM_LINE_POINTS array
        self._freq_domain_data = np.zeros((FREQ_NUM_LINE_POINTS, 2), dtype = np.float32) # 2 x NUM_LINE_POINTS array

    def run_data_creation(self):

        print("Run data creation is starting")
        global line_number_textFile
        global record_file
        global is_restart_clicked

        while line_number_textFile < RECORD_COUNTER:

            if is_restart_clicked == True:

                print("Reset variables")

                is_restart_clicked = False
                print("is_restart_clicked: ", is_restart_clicked)

                record_file.seek(0)  # move file cursor to line 12, first 11 lines are radar info data

                line_number_textFile = 0
                print("line_number_textFile: ",line_number_textFile)

                # Read info lines
                for i in range(0, 11):
                    _line = record_file.readline()
                    print(_line)

            if self._should_end:
                print("Data source is told to stop")
                break

            # set this time according to the update rate
            #print("TIME_BETWEEN_PLOT",TIME_BETWEEN_PLOT)
            #time.sleep(TIME_BETWEEN_PLOT/1000)
            time.sleep(0.00001)

            sample_line = record_file.readline()
            samples_hex = bytes.fromhex(sample_line)  # get hex data from string
            length_line = len(samples_hex)

            index = 0

            # If there is data available
            if length_line > 0:

                while index < length_line:

                    if ADC_SELECT == 1:
                        # For faster operation i transfer 4 bit and 6 bit of the samples in 2 bytes
                        current_sample_16bit = ((samples_hex[index] & 0xF) << 6) | (samples_hex[index + 1] & 0x3F)

                        current_sample_float = (current_sample_16bit / 2 ** 10) * 5.0

                        #current_sample_float -= 2.25 # bias voltage

                    else:
                        current_sample_16bit = ((samples_hex[index] & 0xFF) << 8) | (samples_hex[index + 1] & 0xFF)

                        current_sample_float = (current_sample_16bit / 2 ** 12) * 3.3

                        #current_sample_float -= 2.5 # bias voltage

                    index += 2
                    freq_sample_array.append(current_sample_float)
                    time_sample_array.append(current_sample_float)

            # Add another plot and try plotting with pointwise update from left to right

            # Plot Freq Line
            if len(freq_sample_array) == FREQ_RANGE:
                fx, fy = FFT_Calculate(freq_sample_array, 1 / SAMPLING_FREQUENCY, np.hamming, AVERAGING_NUM)
                self._freq_domain_data[:, 0] = fx
                self._freq_domain_data[:, 1] = fy

                freq_sample_array.clear()

                #calculate time until this one is called then sleep for according to the left time

            # Plot Time Line
            if len(time_sample_array) == TIME_NUM_LINE_POINTS:
                time_step = np.linspace(0.0, float(TIME_MS / 1000), len(time_sample_array))
                self._time_domain_data[:, 0] = time_step
                self._time_domain_data[:, 1] = time_sample_array

                time_sample_array.clear()

                # calculate time until this one is called then sleep for according to the left time

            line_number_textFile += 1

            data_dict = {
                "time": self._time_domain_data,
                "freq": self._freq_domain_data,
            }

            self.new_data.emit(data_dict)
            #print("Data creation")

            if line_number_textFile == RECORD_COUNTER:
                line_number_textFile = 0

                freq_sample_array.clear()
                time_sample_array.clear()

        print("Data source finishing")
        self.finished.emit()

    def stop_data(self):
        print("Data source is quitting...")
        self._should_end = True

if __name__ == '__main__':

    app = use_app("pyqt5")
    app.create()

    canvas_wrapper = CanvasWrapper()
    win = MyMainWindow(canvas_wrapper)
    data_thread = QtCore.QThread(parent=win)
    data_source = DataSource()
    data_source.moveToThread(data_thread)

    # update the visualization when there is new data
    data_source.new_data.connect(canvas_wrapper.update_data)
    # start data generation when the thread is started
    data_thread.started.connect(data_source.run_data_creation)
    # if the data source finishes before the window is closed, kill the thread
    data_source.finished.connect(data_thread.quit, QtCore.Qt.DirectConnection)
    # if the window is closed, tell the data source to stop
    win.closing.connect(data_source.stop_data, QtCore.Qt.DirectConnection)
    # when the thread has ended, delete the data source from memory
    data_thread.finished.connect(data_source.deleteLater)

    win.show()
    data_thread.start()
    app.run()

    print("Waiting for data source to close gracefully...")
    data_thread.wait(5000)