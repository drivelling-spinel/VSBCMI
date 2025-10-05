# CMI8738-based PCI soundcard HOWTO for DOS

## The fine print

_The guide is licensed by it's author under the terms of [CC BY-NC license](https://creativecommons.org/licenses/by-nc/4.0/). The author is not affiliated with Microsoft, IBM, C-Media Electronics or any other bodies that hold right over any of the commercial works (software, intellectual or otherwise) referenced in the guide. No claims are being made over the rights (full or partial) for any commercial works (software, intellectual or otherwise) referenced in this guide and they remain with the owning bodies in accorance with the appropriate law (or laws). The guide is presented in hopes of being useful, but with no warranty, nor even implied warranty of merchantability of fitness for a particular purpose. The author will not be held accountable for any damage caused to the persons, their data, hardware or any other property, following any actions taken by those persons in connection (direct or indirect) with activities described in the guide._

## Part 1 - Installing the Card

### Prerequisites
1. A PC with DOS installed and a free PCI slot. [FreeDOS](https://www.freedos.org) is a proven open source alternative to commercial DOS variants and all below steps have been carried out in FreeDOS. Many guides are available on installing FreeDOS from various media. 
2. DOS driver package from C-Media. This is either available on a CD that came with your card or can be obtained from online sources, e.g. Vogons Driver Library. The package should provide at least these files: 

       SETAUDIO.COM
       C3DMIX.COM
 
 3. Optionally a game to test that the card is operational. [Electro Man](https://www.classicdosgames.com/game/Electro_Man.html) is a small, free, easy to set up game and does not rely on DMA to play sound. (_this game works with C-Media proprietary driver, but may hang when using VSBCMI_)

### Steps

1. Install the card and boot your PC. In the BIOS settings check that the PCI slot used is not assigned with IRQ 5 or IRQ 7 since these will be used for Sound Blaster emulation later on. If you don't know which slot BIOS is referring to, proceed to the next steps but take note of the IRQ card driver reports later on.

2. Unpack C-MEDIA drivers package or use the installer shipped with the drivers. In the latter case, your `AUTOEXEC.BAT` will likely be updated to load driver TSR on every boot. This in not conventient, since another driver will be used in the described setup, moreover `C3DMIX.COM` will be updating the entries after each successful run to store the selected settings. Hence the following change is advised after the installation:

       REM Assuming driver installation has left below entries in AUTOEXEC.BAT
       REM add GOTO statement before the entries 
       GOTO SKIPC3D
       C:\PATH\TO\PCIAUD\SETAUDIO ...
       SET BLASTER=...
       C:\PATH\TO\PCIAUD\C3DMIX ...
       REM add label after the entries
       REM the C3DMIX part above will be updated with the mixer settings by the .COM file
       :SKIPC3D

3. Check that the card operates. From the directory the drivers have been installed run

       SETAUDIO

   Check the driver output for the result and take note of the _card's own_ IRQ.  
   
   If the output did not show any errors, it's time to familiarize oneself  with C-Media mixer program. While doing so it is advised to check that volume is at reasonable level and output is not unexpectedly muted.

       C3DMIX

   Now is the time to check sound output in a game. If using Electro Man as suggested, first set BLASTER environment variable, since the driver TSR will not set it by itself:
   
       REM Use the values output by SETAUDIO, the below is just an example
       SET BLASTER=A220 I5 D1 T4
       CONFIG
       EM

   If the sound is playing, all is set to proceed to the next step.   
   
   __Note__: if the card's own IRQ matches the desired IRQ of the virtual Sound Blaster to be emualted, adjustment in BIOS is necessary before proceding further.

## Part 2 - Installing VSBCMI

### Prerequisites
1. JemmEx (properly Jemm Extended Memory Manager) with JLOAD program. If the machine has FreeDOS installed, JemmEx is probably already there and is configured (at least for boot options that have EMS enabled). It is still advised that an up to date distribution of Jemm is used. For the purpose of the guide [v5.86](https://github.com/Baron-von-Riedesel/Jemm/releases/tag/v5.86pre1) was used.
2. [VSBHDA v1.7](https://github.com/Baron-von-Riedesel/VSBHDA/releases/tag/v1.7) for all files VSBCMI depends for it's operation.
3. [VSBCMI](https://github.com/drivelling-spinel/VSBCMI/releases/tag/v1.7.3) executable.

### Steps
1. Ensure JemmEx is configured to load in `CONFIG.SYS`/`FDCONFIG.SYS` . For example:

       DEVICE=C:\PATH\TO\JEMMB.586\JEMMEX.EXE VERBOSE I=B000-B7FF I=TEST X=TEST MAXEXT=32764 X2MAX=32764

   Reboot the PC.
   
   While JemmEx allows being loaded from the command line as well, using this kind of setup or running VSBHDA/VSBCMI with other EMM programs is out of scope of this guide. 

2. Load additional JemmEx modules using `.DLL`-s that are shipped with VSBHDA:
    
       C:\PATH\TO\JEMMB.586\JLOAD C:\PATH\TO\VSBHDA.17\QPIEMU.DLL
       C:\PATH\TO\JEMMB.586\JLOAD C:\PATH\TO\VSBHDA.17\JHDPMI.DLL

   These steps can be added to `AUTOEXEC.BAT` if they worked fine.
   
3. Start VSBCMI

       C:\PATH\TO\VSBHDA.17\HDPMI32I
       SET BLASTER=A220 I5 D1 H5 P330 T6
       C:\PATH\TO\VSBCMI\VSBCMI /OPL0

   Take note of the revision of the chip deteced by the driver. Behavior is slightly different with older cards (revisions 37 or less) and more modern ones (revisions after 37).
   
   Now is the time to test the driver's and card's operation with games installed on the PC. If everything sounds and works correctly in "analog mode", digital sound output can be configured.

   __Note:__ Not all  games will work after loading the driver. Some popular games, for example [Tyrian/Tyrian 2000](https://www.classicdosgames.com/game/Tyrian_2000.html), use 16-bit protected mode, and require a special version of emulator. Below is a (non exhaustive) list of games that are known to __have not__ worked well, and thus are not best testing candidates:   
   
   * Electro Man (_this game works, however, with C-Media proprietary driver as advised above_)
   * X-Wing classic (floppy edition)
   * Inherit the Earth CD-ROM
   * Strike Commander 
   * Blackthorne
   * Whizz
   * Black Zone


## Part 3 - Configuring digital output

### Word of warning

With more recent revisions of the chip (revision number after 37) - for example CMI-8738/PCI-6ch-mx - sound distortion may occur if chip's own FM playback and S/PDIF-in (e.g. from a CD/DVD-ROM drive that has digital CD-audio output) are routed via S/PDIF Out port. Because of this, for these newer chips the driver splits out only digital audio by default and lets all other "outputs" be heard from analog Speaker Out. 

On the other hand, with earlier revisions of the chip (37 and before), sending both Wave Out and FM to card's S/PDIF Out seems to work correctly, so the driver will enable this feature when digital output is requested. However, at least with one chip with revision 37 listening to S/PDIF-in produced distorted sound for CD-audio still.

It is advised to keep that in mind when assessing the fitness of digital output with CMI-8X38 for each individual's purposes and for planning cabling/connections "downstream" from the sound card.

### Steps

1. Reboot the PC and use a modified command to load VSBCMI (note `/F44100`). 

       REM Assuming .DLL-s are already loaded with JLOAD
       C:\PATH\TO\VSBHDA.17\HDPMI32I
       SET BLASTER=A220 I5 D1 H5 P330 T6
       C:\PATH\TO\VSBCMI\VSBCMI /OPL0 /F44100 /O3
       
2. Now check sound outpot with actual game, and if the sound is still a bit too high-pitched when compared with what is heard via analog output, try different frequency:

       REM Assuming .DLL-s are already loaded with JLOAD
       C:\PATH\TO\VSBHDA.17\HDPMI32I
       SET BLASTER=A220 I5 D1 H5 P330 T6
       C:\PATH\TO\VSBCMI\VSBCMI /OPL0 /F48000 /O3
       
   These steps can be added to `AUTOEXEC.BAT` if they worked fine.

3. _Optional:_ As explained in the warning above, for chip revisions after 37, FM will not be sent to S/PDIF Out by default. As en experiment, one can try running VSBCMI with additional `/DF2` command line argument and see if FM Output then sounds clean or distorted.

4. All done, congratulations!


## Part 4 - Using C-Media C3DMIX.COM mixer with VSBCMI

1. Use C-Media mixer program to configure sound output levels 

       C:\PATH\TO\PCIAUD\C3DMIX 

   Because VSBCMI handles configuration of digital output, there is no need to change S/PDIF settings in the mixer program.

2. Quit the program and look for the lines similar to the below in `AUTOEXEC.BAT` where the program has stored the settings.
   
       C:\PATH\TO\PCIAUD\C3DMIX /MFF000 /FFF400 /WDD400 /LDD4DD /E00500 /A00500 /C00500 /P10000 /400000 /R0f0ff /D740ff /Q0  

    If these are in a _"label-protected area"_ of the file (see Step 2 of Part 1 above), they can be also copied to the part of the file immediately following VSBCMI invocation. In this way, every time mixer settings are adjusted via `C3DMIX.COM` they will also be loaded into the sound card upon next reboot, only, __remove__ the arguments starting with `/D` and `/Q` to prevent mixer program from interfering  with digital output settings.
    
       REM Assuming the below two lines have been added to AUTOEXEC.BAT
       SET BLASTER=A220 I5 D1 H5 P330 T6
       C:\PATH\TO\VSBCMI\VSBCMI /OPL0 /F44100
       REM add mixer call after them
       C:\PATH\TO\PCIAUD\C3DMIX /MFF000 /FFF400 /WDD400 /LDD4DD /E00500 /A00500 /C00500 /P10000 /400000 /R0f0ff  
    
3. _Optional_: To exercise more control over mixer one may want to keep the working settings permanent, and only have `C3DMIX.COM` change _current_ mixer levels until next reboot. To achieve this, retain the position of the `C3DMIX` line in `AUTOEXEC.BAT` in the _"label-protected area"_, save the stable working mixer settings in a separate BAT-file, let's say `C3DMIX_D.BAT`:

        C:\PATH\TO\PCIAUD\C3DMIX /MFF000 /FFF400 /WDD400 /LDD4DD /E00500 /A00500 /C00500 /P10000 /400000 /R0f0ff  

   And then add the call to the new file from `AUTOEXEC.BAT`:
   
       REM Assuming the below two lines have been added to AUTOEXEC.BAT
       SET BLASTER=A220 I5 D1 H5 P330 T6
       C:\PATH\TO\VSBCMI\VSBCMI /OPL0 /F44100
       REM add mixer call after them
       CALL C3DMIX_D



2025,  
[CC BY-NC]( "https://creativecommons.org/licenses/by-nc/4.0/),  
Ludicrous_peridot

 