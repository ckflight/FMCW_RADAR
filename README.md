# FMCW_RADAR

Example screenshots are below

Frequency Modulated Continuous Wave Radar with 26 dBm output power covering around 250m range with a 10dBi gain patch antenna.

Auto gain configuration with digital potantiometer in MCU board. -40dB/decade Range compansation filter is implemented as well. IF signal is available at different stage of amplifier and filter circuit for testing with other configurations.

Board has designed as 2 parts. In this way processing and controlling unit can be redesigned with FPGA or any other option:

1: Radar RF Board

2: Radar MCU Board(I will add FPGA Board later with VHDL files as well)

Both pcbs have 4 layers. 50 Ohm impedance microstrip transmission line is making 0.342mm trace width with below stackup. SMA connectors need cutout for preventing reflections from 1mm wide sma pads. CST studio simulation is used for S11 performance of the connector.
Stackup layer information can be found here: https://docs.oshpark.com/services/four-layer/

STM32F4 has triangular waveform generation option with its DAC and also ADF4158 is used for PLL topology. Either option can be used for waveform generation. Check schematic to populate necessary resistor values to use either mode. ADF4158 can generate pulses with some gap between each sweep. In this way N number of samples are packed and buffered over USB during this gap. 
Main loop runs with a FSM to check conditions of sweep. ADF4158 Muxout is generating pulse according to the sweep condition. External interrupt tracks this pulse to move between FSM states. 

MCU Board talks to Python script to configure its hardware so PC is in Host mode and controls everything. In this way, any parameters can be changed on hardware without reflashing the firmware on it during field test.

N number of samples is determined by sweep bandwith rise time and sampling frequency. 
For example:

  If sampling freq = 500KSamples/sec and sweep rise time is 1ms for 200MHz bandwith then in 1 millisecond, system will pack 500 samples   of 16 bit data and transfer 1000 8 bit samples to Python script. So 1MByte data per second is needed for 500KHz sampling frequency.
  
  
Python Script: I have uploaded an example record file along with python scripts where peaks of the stationary objects can be seen. I will later add real time plot by simply doing record and plotting with 2 different treads simultaneously.  
  
![RADAR_RF](https://user-images.githubusercontent.com/61315249/82238636-4acb1000-9940-11ea-8323-65df61ffcf1b.png)

![RADAR_MCU](https://user-images.githubusercontent.com/61315249/82238631-47d01f80-9940-11ea-91dd-7cf658414dd1.png)

![IMG_20200511_111503](https://user-images.githubusercontent.com/61315249/81540098-d14c8400-9379-11ea-9681-371c4e8f2141.jpg)

![radar2](https://user-images.githubusercontent.com/61315249/82351877-d8bcfe80-9a05-11ea-9fe4-1619731c1cea.png)

![radar3](https://user-images.githubusercontent.com/61315249/82351882-d9559500-9a05-11ea-9772-c6e71b53d2af.png)



