# VSBCMI

Sound Blaster emulation for DOS via CMI/HDA/AC97/SBLive/Ensoniq.

Changes from [VSBHDA](https://github.com/Baron-von-Riedesel/VSBHDA) and [SBEMU](https://github.com/crazii/SBEMU):
 * Support for CMI8X38 using driver originally from SBEMU, MPXPlay
 * Legacy FM is left in initialized state for CMI8X38 (port 388) when started with /OPL0,   
   and joystick port is always left in initialized state for CMI8X38
 * MPU401 UART "passthrough" for CMI8X38 (following SBEMU implementation) when started with /PXXX
 * Resampling code giving less distorted sound when upsampling to 44100   
   (and a compile time option to avoid interpolation when resampling)

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

To create the binaries, Open Watcom v2.0 is recommended. DJGPP v2.05
may also be used, but cannot create the 16-bit variant of VSBHDA.

In all cases the JWasm assembler (v2.17 or better) is also needed.
For Open Watcom, a few things from the HX development package (HXDEV)
are required - see Makefile for details.
