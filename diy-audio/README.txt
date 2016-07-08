PCB Production
==============
Choose the service you would ever like. I have uploaded the design to OSHPark, too.
VERY IMPORTANT: The substrate thickness has to be as thin as possible.
The PCB is design to sit between lifted pins of the IT6613 (the HDMI transmitter) and OSSC-PCB. Just look at the installation pictures to get an idea ;)
I have produced the PCBs with OSHPark and have chosen 0.8mm thickness. The PCB must not be thicker.
Link to PCB at OSHPark:
- https://oshpark.com/shared_projects/rmFBAKyo (for v1.3)
- https://oshpark.com/shared_projects/vKUEgBSy (for v1.5)

Part list of the DIY-Audio PCB
==============================
running number ------ footprint(s) --------- device/part --------- package ----- mouser ordering key
     01         |  U1                 |  PCM1808               |  TSSOP14     |  595-PCM1808PWR
     02         |  U2                 |  74LVC1G34             |  SOT23-5     |  595-SN74LVC1G34DCKR
     03         |  U3                 |  MK2705SLF             |  SOIC8       |  972-MK2705SLF
     04         |  FB1, FB2           |  Ferrit Beat           |  SMD0603     |  81-BLM18PG471SN1D
     05         |  C1                 |  tantal-cap. 47uF/16V  |  SMD case C  |  74-293D476X9016C2TE3
     06         |  R11, R12           |  res. >2.2 kOhm        |  SMD0603     |  71-CRCW06034K99FKEAH (e.g. 4.99k)
     07         |  C11, C13, C15      |  tantal-cap. 10uF/6.3V |  SMD case A  |  74-TR3A106M6R3C1500
     08         |  C12, C14, C16, C21 | cap. 0.1uF/50V         |  SMD0603     |  77-VJ0603V104MXACBC
     09         |  C17, C18, C31      | tantal-cap. 1uF/16V    |  SMD case A  |  74-293D105X9016A2TE3
     10         |  R19, R20           | res. ***               |  SMD0603     |  see description below
     11         |  C19, C20           | cap. ***               |  SMD0603     |  see description below
     12         |  R31                | res. 33 Ohm            |  SMD0603     |  71-CRCW060333R0FKEAH
     13         |  C32, C33           | cap. 0.01uF/50V        |  SMD0603     |  77-VJ0603Y103KXACBC

RC-filtering of the Audio
=========================
The RC filters (R19,C19) and (R20,C20) mainly bandlimits the noise. The filter also avoids aliasing effects if the bandwith of the audio signal increases.
The PCM1808 samples the audio signal with 96kHz. This means that the cut-off frequency of the filters have to be 96kHz or below. The firmware will also
allows you to further downsample the datastream to 48kHz. If you want to use that I recommend you to set the filter to 48kHz or below to avoid aliasing.
The cut-off frequency f_c of a simple RC low pass filter is determined by f_c = 1/(2*pi*R*C).
here is my choice which gives a cut-off frequency of 47.938kHz.
     10         |  R19, R20  |  res. 3.32 kOhm   |  SMD0603  |  71-CRCW0603-3.32K-E3  
     11         |  C19, C20  |  cap. 1000pF/50V  |  SMD0603  |  77-VJ0603Y102KXACBC



PCB-Installation
=================
(see pictures; assumes that at least the SCART connector has been desoldered)
01 - v1.3 - bend up pins 9, 10 and 11 of the IT6613 (v1.3 only!!!)
   - v1.5 - cut three wires between each pair of solder joints marked with I2S, WS and SCK (v1.5 only)
02 - make a placement check (might look different between v1.3 and v1.5)
03 - cut off the plastic around pin 2, 4 and 6 of the SCART connector if needed
04 - solder the IT6613 into place (if not done yet) and approximately fix the audio pcb into place (v1.5 does not sit under some pins)
05 - solder the SCART plug into place and solder the audio PCB onto the SCART connector (remind to check the pin assignment at the IT6613 during soldering the first leg)
06 - v1.3 - solder pin 9, 10 and 11 of the IT6613 onto the audio PCB (v1.3 only)
   - v1.5 - solder the three top solder joints on the OSSC PCB (I2S, WS and SCK) to the DIY audio PCB (v1.5 only)
07 - connect the wires
08 - 5V and 3.3V can be get from U5
09 - 27MHz from the crystal oscillator (pin between U7 and U9)