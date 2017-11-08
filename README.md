Open Source Scan Converter
==============

Open Source Scan Converter is a low-latency video digitizer and scan conversion board designed mainly for connecting retro video game consoles and home computers into modern displays. Please check the [wikipage](http://junkerhq.net/xrgb/index.php?title=OSSC) for more detailed description and latest features.

Requirements for building and debugging firmware
---------------------------------------------------
* Hardware
  * OSSC board
  * USB Blaster compatible JTAG debugger, e.g. Terasic Blaster (for FW installation and debugging)
  * micro SD/SDHC card (for FW update via SD card)

* Software
  * [Altera Quartus II + Cyclone IV support](http://dl.altera.com/?edition=lite) (v 16.1 or higher - free Lite Edition suffices)
  * GCC (or another C compiler) for host architecture (for building a SD card image)
  * Make
  * [iconv](https://en.wikipedia.org/wiki/Iconv) (for building with JP lang menu)


Architecture
------------------------------
* [Reference board schematics](https://www.niksula.hut.fi/~mhiienka/ossc/diy-v1.5/ossc_v1.5-diy_schematic.pdf)
* [Reference PCB project](https://github.com/marqs85/ossc_pcb)


Building software image
--------------------------
1. Enter BSP directory:
~~~~
cd software/sys_controller_bsp
~~~~
2. (Optionally) edit BSP settings:
~~~~
nios2-bsp-editor
~~~~
3. Generate BSP:
~~~~
nios2-bsp-generate-files --bsp-dir . --settings settings.bsp
~~~~
NOTE: the previous step must be done every time after RTL/bitstream is built

4. Enter software root directory:
~~~~
cd software/sys_controller
~~~~
5. Build SW for target configuration:
~~~~
make [OPTIONS] [TARGET]
~~~~
OPTIONS may include following definitions:
* OSDLANG=JP (Japanese language menu)
* ENABLE_AUDIO=y (Includes audio setup code for v1.6 PCB / DIY audio add-on board)

TARGET is typically one of the following:
* all (Default target. Compiles an ELF for direct downloading to Nios2 during testing)
* generate_hex (Generates a memory initialization file required for bitstream)
* clean (cleans ELF and intermediate files. Should be invoked every time OPTIONS are changed between compilations, expect with generate_hex where it is done automatically)

6. Optionally test updated SW by downloading ELF to Nios2 CPU via JTAG (RTL-SW interface in active FW must be compatible new SW BSP configuration)
~~~~
nios2-download -g --accept-bad-sysid sys_controller.elf
~~~~


Building RTL / bitstream
--------------------------
1. Load the project (ossc.qpf) in Quartus
2. Generate the FPGA bitstream (Processing -> Start Compilation). NOTE: make sure software image (software/sys_controller/mem_init/sys_onchip_memory2_0.hex) is up to date before generating bitstream.
3. Ensure that there are no severe timing violations by looking into Timing Analyzer report

If only software image is updated, bitstream can be quickly rebuilt by running "Processing->Update Memory Initialization File" and "Processing->Start->Start Assembler" in Quartus.

Installing firmware via JTAG
--------------------------
The bitstream can be either directly programmed into FPGA (volatile method, suitable for quick testing), or into serial flash chip where it is automatically loaded every time FPGA is subsequently powered on (nonvolatile method, suitable for long-term use).

To program FPGA, open Programmer in Quartus, select your USB Blaster device, add configuration file (output_files/ossc.sof) and press Start

To program flash, FPGA configuration file must be first converted into JTAG indirect Configuration file (.jic). Open conversion tool ("File->Convert Programming Files") in Quartus, click "Open Conversion Setup Data", select "ossc.cof" and press Generate. Then open Programmer, add generated file (output_files/ossc.jic) and press Start after which flash is programmed. Installed/updated firmware is activated after power-cycling the board.


Generating SD card image
--------------------------
Bitstream file (Altera propiertary format) must be wrapped with custom header structure (including checksums) so that it can be processed reliably on the CPU. This can be done with included helper application which generates a disk image which can written to a SD card and subsequently loaded on OSSC:

1. Compile tools/create_fw_img.c
~~~~
cd tools && gcc create_fw_img.c -o create_fw_img
~~~~
2. Generate the firmware image:
~~~~
./create_fw_img <rbf> <version> [version_suffix]
~~~~
where
* \<rbf\> is RBF format bitstream file (typically ../output_files/ossc.rbf)
* \<version\> is version string (e.g. 0.78)
* \[version_suffix\] is optional max. 8 character suffix name (e.g. "mytest")


Debugging
--------------------------
1. Rebuild the software in debug mode:
~~~~
make clean && make APP_CFLAGS_DEBUG_LEVEL="-DDEBUG"
~~~~
NOTE: Fw update functionality via SD card is disabled in debug builds due to code space limitations. If audio support is enabled on debug build, other functionality needs to be disabled as well.

2. Program Nios2 CPU via JTAG and open terminal for UART
~~~~
nios2-download -g --accept-bad-sysid sys_controller.elf && nios2-terminal
~~~~
Remember to close nios2-terminal after debug session, otherwise any JTAG transactions will hang/fail.
