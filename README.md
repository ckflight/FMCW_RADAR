# Frequency Modulated Continuous Wave Radar Hardware Design and the Firmware

#### Later Update: 
  New version of the hardware is available in the **Radar 2** folder. For **Radar 2** design, two separated boards in **Radar MCU** and **Radar RF** folders are brought together with several necessary improvements. I recommend using the second design since it is more reliable and tested. The reason behind two separate board was to change digital part of the board without changing the RF part of it. However, one united design performs better and have a better temperature cooling control which affect the overall perfomance significantly.
  
  ##### Important Power Supply Note: I used switching regulator for 3.3 and 5V power sources to test their effect on the RF performance and oscillation of ripple voltage on OpAmp analog amplifier side. Therefore do not use switching regulator in your design. Change them to LDO type of clean supplies. Or add second LDO stage to the switching regulator unless you want an AM modulated ( 1.4 MHz switching freq. is very f*cking visible ) signal at the OpAmp side. After observing these affects i had to fine tune components which is not necessary if you use a cleaner source !!!

#### Radar Info:
 * Frequency Modulated Continuous Wave Radar with 26 dBm output power covering around 250 meters range with 10 dBi gain patch antenna (400 meters in theory by using a better ADC with higher sampling rate to match the noise floor of the RF receiver chain).
 
 $$ Range(radar) = {\sqrt[4]{Pt * G^2 * \lambda^2 * \sigma \over Pmin * 4\pi^3}} $$
 
  * Receiver chain has LNA, Gain Block and Mixer. The gain block drops the Noise Figure from 7 dB to 2 dB, which doubles the detection range. To determine the radar detection range, minimum detectable signal is needed. RF receiver chain has thermal noise and it determines the noise floor for Pmin. Signal to Noise (SNR) ratio of 20 dB is assumed for a good detectable signal then:
 
 $$ Pthermal(watt) = {k * T * B * F} $$
 
 $$ Pthermal(dBm) = {10\log_{10} {kT \over 1mW} + NF + 10\log_{10} BW} $$
  
 $$ Pmin(dBm) = {Pthermal(dBm) + 20dB} $$
 
 
 * Radar, has an auto gain configuration with a digital potentiometer to control intermediate frequency (IF) signal gain at the last stage of the analog amplifier before sampling with ADC.
 
 * -40 dB / Decade Range Compensation Filter is implemented. Closer objects reflect more powerful RF signal which suppresses the signal of the objects at further distance. By amplifying the weaker signals and filtering the closer ones, each frequency band is adjusted to the same amplitude.
 
 * The IF signal is available at different stages of the amplifier and filter circuit for testing with other configurations.

 * The VCO range of the radar is from 5.5 to 6.1 GHz
 
 * Directional coupler has low insertion loss of 0.25 dB and 16 dB coupling.
 
 * ADF4158 Frequency Synthesizer is available due to the nonlinear characteristics of the VCO profile for accurate distance detection. ADF4158 can generate pulses with some gap between each sweep. During this gap samples are packed and buffered over USB.
 
  * STM32F4's DAC has a triangular waveform generation option and it can be used for VCO signal generation as well. Both option are available on board. Check schematic to populate necessary resistor values to use either mode.
 
 * SMA connector has 1 mm pad size, but the 50 Ohm transmission line is 0.342 mm wide. To prevent reflection, cutout is implemented beneath the SMA pad with CST Studio simulation which is also available at my repository https://github.com/ckflight/CST_ADS_MODELS_SIMULATIONS

 * PCB has 4 layers. 50 Ohm impedance microstrip transmission line is making 0.342mm trace width by using oshpark's stack-up below. Stack-up layer information can be found here: https://docs.oshpark.com/services/four-layer/

 * Main loop runs with a Finite State Machine (FSM) to control radar, which checks the condition of the start of the sweep and the gap. ADF4158 Muxout is generating a pulse according to the sweep condition. External interrupt tracks this pulse to move between FSM states. 

 * Hardware is configured using Python script so the PC is in Host mode and controls everything. In this way, any hardware parameters can be changed without reflashing the firmware.

 * N number of samples are determined by sweep bandwidth rise time and the sampling frequency. 
   For example:

   Fs = 500.000 samples/sec and the sweep rise time is 1 millisecond (ms) for 200 MHz bandwidth. The system will pack 500 x  16 bit ADC samples and transfer 1000 x 8 bit ADC samples to Python script in 1 ms. Therefore, 1 MegaByte data is buffered to Host PC for 500 KHz sampling frequency in 1 second.
  
 * Python Script: I have uploaded an example record file along with python scripts where the peaks of the stationary objects can be seen. 

### Object Type Detection:

 * Object type detection is done with the similar code that i wrote for my project at the repository https://github.com/ckflight/RADAR_24GHZ_NEURAL_NETWORK
 
## RADAR 2 Screenshot:
![Radar 2](https://user-images.githubusercontent.com/61315249/101276899-46bded00-37c1-11eb-818b-c42eb3b57e8a.png)

## RADAR 1 RF Board Screenshot:
![RADAR_RF](https://user-images.githubusercontent.com/61315249/82238636-4acb1000-9940-11ea-8323-65df61ffcf1b.png)

## RADAR 1 MCU Board Screenshot:
![RADAR_MCU](https://user-images.githubusercontent.com/61315249/82238631-47d01f80-9940-11ea-91dd-7cf658414dd1.png)

![IMG_20200511_111503](https://user-images.githubusercontent.com/61315249/81540098-d14c8400-9379-11ea-9681-371c4e8f2141.jpg)

## PYTHON Script Plots:
![radar2](https://user-images.githubusercontent.com/61315249/82351877-d8bcfe80-9a05-11ea-9fe4-1619731c1cea.png)

![radar3](https://user-images.githubusercontent.com/61315249/82351882-d9559500-9a05-11ea-9772-c6e71b53d2af.png)



