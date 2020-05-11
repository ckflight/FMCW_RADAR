# FMCW-Radar
Frequency Modulated Continuous Wave Radar with 27 dBm output power covering around 150 meters with a proper patch antenna.
Auto gain configuration with digital potantiometer in MCU board. Range compansation filter is implemented as well. IF signal is available at different stage of amplifier and filter circuit for testing with other configurations.

Board has designed as 2 parts. In this way processing and controlling unit can be redesigned with FPGA or any other option:

1: Radar RF Board

2: Radar MCU Board(I will add FPGA Board later with VHDL files as well)

STM32F4 has triangular waveform generation option with its DAC and also ADF4158 is used for PLL topology. Either option can be used for waveform generation. Check schematic to populate necessary resistor values to use either mode. ADF4158 can generate pulses with some gap between each sweep. In this way N number of samples are packed and buffered over USB during this gap. 
Main loop runs with a FSM to check conditions of sweep. ADF4158 Muxout is generating pulse according to the sweep condition. External interrupt tracks this pulse to move between FSM states. 

MCU Board talks to Python script to configure its hardware so PC is in Host mode and controls everything. In this way, any parameters can be changed on hardware without reflashing the firmware on it during field test.

N number of samples is determined by sweep bandwith rise time and sampling frequency. 
For example:

  If sampling freq = 500KSamples/sec and sweep rise time is 1ms for 200MHz bandwith then in 1 sec system will pack 500 samples   of 16 bit data and transfer 1000 8 bit samples to Python script.
  
  
Python Script: I uploaded a example python script where stationary object are creating peak at some frequencies. I will later add real time plot by simply doing record and plotting with 2 different treads.  
  


![rf](https://user-images.githubusercontent.com/61315249/81537200-82045480-9375-11ea-87f2-c4e1bb67b51a.png)

![mcu](https://user-images.githubusercontent.com/61315249/81537194-7f096400-9375-11ea-9049-51c95cfd9812.png)

![IMG_20200511_111503](https://user-images.githubusercontent.com/61315249/81540098-d14c8400-9379-11ea-9681-371c4e8f2141.jpg)

![IMG_20200511_112710](https://user-images.githubusercontent.com/61315249/81540570-854e0f00-937a-11ea-8197-5f7794a0b7da.jpg)
