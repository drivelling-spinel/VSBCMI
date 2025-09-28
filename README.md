# VSBCMI v1.7.3

Sound Blaster emulation for DOS via CMI/HDA/AC97/SBLive/Ensoniq.

__Note:__ for current version testing is only performed with CM8738 cards. 
          as much as the author would like to retain proper funtion with other chips original VSBHDA supports 
          no guarantee is made VSBCMI will work with them, even if original VSBHDA does.

Changes from [VSBHDA](https://github.com/Baron-von-Riedesel/VSBHDA) and [SBEMU](https://github.com/crazii/SBEMU):
 * Support for CMI8X38 using driver originally from SBEMU, MPXPlay, with some changes specific to these chips:
    * _Detected_ revision of the chip is displayed when TSR is loaded
    * MPU401 UART "passthrough" (following SBEMU implementation) when started with /PXXX
    * Joystick port is always left in initialized state 
    * Digital output (S/PDIF Out) is enabled if started with /O3
    * When digital output is enabled for chips with revision of 37 and before,   
      legacy FM is also routed to digital output and S/PDIF In (CD-In) is enabled
    * There is also an option to enable OPL ports "passthrough" (see /DF below)  
 * Resampling code giving less distorted sound when upsampling to 44100 
   and option to upsample to 48000 added (/F48000) 
   There is also a compile time option to switch off interpolation when resampling.
 * SoundFont support is switched off by default (compile time) in favor of UART port forwarding
 * /DF option added to provide bit flags in hexadecimal format that control behavior of the TSR. 
   Several flags can be combined. The following flags are supported:
   
   | flag   | meaning
   |--------|------------------------------------------------------------------------------------------|
   | 01     | Bypass certain sanity checks when installing TSR and print additional debug information  |
   | 02     | Enable forwarding of wave and FM sound to S/PDIF Out even for newer chips (rev. after 37)|
   | 04     | Do not enable recording of CD-Audio dital in                                             |
   | 08     | Disable chip built in OPL emulation on port 388                                          |
   | 10     | Virtualize port 220 (and 388 if possible) and forward to chip internal PCI OPL ports     |
   | 20     | Enable chip internal MPU-401 UART emulation on port 330 (unless /PXXX is given)          |
   | 40     | Use longer delays for UART passthrough with older chips (rev. up to 37)                  |

A [HOWTO document](/HOWTO/CM8738-howto.md) for CMI8738-based cards is also included.

Forked from v1.7 of ...

# VSBHDA
Sound blaster emulation for HDA (and AC97/SBLive); a fork of crazii's SBEMU: https://github.com/crazii/SBEMU

Works with unmodified HDPMI binaries, making it compatible with HX.

Supported Sound cards:
 * HDA ( Intel High Definition Audio )
 * Intel ICH, Nvidia nForce, SiS 7012
 * VIA VT82C686, VT8233/35/37
 * SB Live, SB Audigy
 * SB based on ES1371/1373 (Ensoniq)

Emulated modes/cards:
8-bit, 16-bit, mono, stereo, high-speed;
Sound blaster 1.0, 2.0, Pro, Pro2, 16.

Requirements:
 * HDPMI32i - DPMI host with port trapping; 32-bit protected-mode
 * HDPMI16i - DPMI host with port trapping; 16-bit protected-mode
 * JEMMEX 5.84 - V86 monitor with port trapping; v86-mode
 
VSBHDA uses some source codes from:
 * MPXPlay: https://mpxplay.sourceforge.net/ - sound card access
 * DOSBox: https://www.dosbox.com/ - OPL3 FM emulation
 * TinySoundFont: https://github.com/schellingb/TinySoundFont - MIDI synthesizer emulation

To create the binaries, Open Watcom v2.0 is recommended, more specifically
[this build](https://github.com/open-watcom/open-watcom-v2/releases/tag/2024-02-02-Build).
DJGPP v2.05 may also be used, but cannot create the 16-bit variant of VSBHDA.

In all cases the JWasm assembler (v2.17 or better) is also needed.
For Open Watcom, a few things from the HX development package (HXDEV)
are required - see Makefile for details. Moreover, for 32-bit variant, 
a certain version of startup code may be required, as hilighted 
[here](https://github.com/Baron-von-Riedesel/VSBHDA/issues/40#issuecomment-3062018190).

