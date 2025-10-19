# VSBCMI and games sound in DOS HOWTO 

## The fine print

_The guide is licensed by it's author under the terms of [CC BY-NC license](https://creativecommons.org/licenses/by-nc/4.0/). The author is not affiliated with Microsoft, IBM, C-Media Electronics or any other bodies that hold right over any of the commercial works (software, intellectual or otherwise) referenced in the guide. No claims are being made over the rights (full or partial) for any commercial works (software, intellectual or otherwise) referenced in this guide and they remain with the owning bodies in accorance with the appropriate law (or laws). The guide is presented in hopes of being useful, but with no warranty, nor even implied warranty of merchantability of fitness for a particular purpose. The author will not be held accountable for any damage caused to the persons, their data, hardware or any other property, following any actions taken by those persons in connection (direct or indirect) with activities described in the guide._

## Part 1 - Computer configuration

Game compatibility will differ depending on which hardare and software conponents are used in the computer. In oder to avoid any misunderstanding, concrete hardware configuration that was used for running the games is first given:

|  Component     |                                                                                          |
|----------------|------------------------------------------------------------------------------------------|
| Motherboard    | Gigabyte GA-G41M-Combo G41                                                               |
| CPU            | Core 2 Quad Q9550                                                                        |
| Memory         | 2x4Gb DDR3 downclocked to ...                                                            |
| Chipset        | Intel ICH7                                                                               |
| Video          | Intel integrated video (see above) with VGA/D-SUB output                                 |
| HDD            | WDC 80Gb IDE (3 primary and 3 logical partitions, 9 to 29 Gb each)                       |
| DVD-ROM        | A-Open 16x IDE                                                                           |
|                | VIDE-CDD.SYS version 2.14, SHSUCD version 3-6, SHSU-CDH version 2.1                      |
| Floppy         | 3.5" FDD (with integrated USB hub and card reader)                                       |
| OS             | FreeDOS 1.3-RC3 release 2020-05-31                                                       |
|                | JemmEx v5.85                                                                             |
| Input          | A4Tech ML160 (BIOS USB compatibility on) + PS/2 wired keyboard                           | 
|                | Logitech mouse driver v7.02 (__without__ CLOAKING)                                       |
| Joysticks      | InterAct PC Flight Force SV-242 + SV-240 connected to game port                          |
| Soundcard      | Leadtek WinFast 4X (PCI, CMI-8738) with TOSLINK bracket                                  |
|                | PCIAUDIO.COM v1.98, C3DMIX.COM v0.4                                                      |
| MIDI module    | X3MB (Buran edition) connected to game port                                              |


## Part 2 - Incompatible games

These games currently are known to crash, hang, play no sound or misbehave is some other way:

* Inherit the Earth (CD-ROM talkie version)
* Strike Commander 
* Blackthorne _(needs upstream bugfixes from v1.8)_
* Electro Man

_The below is copied from `vsbhda.txt`_
* Comanche
* Privateer
* Rayman
* SuperFrog
* Zone 66


## Part 3 - Games that need VSBCMI16

Games running in 16-bit protected mode require the 16-bit variant of the driver, `VSBCMI16`.  
This in turn should be run with `HDPMI16I.EXE` extender loaded.

* Tyrian 2000

## Part 4 - Game compatibility tips

_NOTE: work is in progress with this section, including the sections that already have some content in them_

### VSBHDA notes
_The below is copied from `vsbhda.txt`_

Here are some programs/games listed that require special actions:

- Aladdin: requires EMS, max. XMS memory is 31MB (XMSRes 31).
- Blood: setting SB IRQ to 2 or 5 may be required. Generally, on some
      machines the DOS/4GW DOS extender has problems with IRQ 7.
- Creative's SB16 diagnose.exe: needs cmdline option /CF1.
- Jungle Book: needs SETPVI.EXE to be run before launched.
- Lemmings 2: Uses direct disk access ( Int 25h ) - hence the binary
      has to be located on a FAT12/FAT16 disk.
- Screamer: set max. XMS memory to 31MB (XMSRes 31).
- Stargunner: to run SETUP.EXE requires Jemm's NOVCPI option; the game
      doesn't restore the SB interrupt vector - it may be necessary to reload
      vsbhda after the program has been run.
- "Sword and Fairy 1" (Chinese Paladin): allocates a sound buffer in
      extended memory - see notes in 4.3.2).
- System Shock: set max. XMS memory to 31MB (XMSRes 31) before running
      HDPMI32i.
- Terminal Velocity: set max. XMS memory to 31MB (XMSRes 31).
- "The Flight of the Amazon Queen": requires SETPVI.
- X128 (Sinclair Spectrum Emulator): requires Jemm's NOVCPI option.

### Legend of Kyrandia CD-ROM

Producing an ISO of the game CD with patched sound drivers is necessary. 
1. Using [WestPak2](https://sourceforge.net/projects/westpak2/files/) extract ALFX.DRV from SOUND.PAK in Dune II.
2. Using the same tool extract SB*.ADL fromm DRIVERS.PAK in Lands of Lore. 
3. Overwrite all files with these names found on Kyrandia CD-ROM with the exctracted ones (there are multiple game data directories on the disc).
4. Create (and optionally) burn the ISO with [ImgBurn](https://www.imgburn.com).

### The Hand of Fate CD-ROM

Producing an ISO of the game CD with patched sound drivers is necessary. 
1. Using [WestPak2](https://sourceforge.net/projects/westpak2/files/) extract SB*.ADV from DRIVERS.PAK on the CD-ROM.
2. Using the same tool patch INTRODRV.PAK replacing all SB*.ADV files with the extracted ones.
3. Overwrite INTRODRV.PAK with the patched version.
4. Create (and optionally) burn the ISO with [ImgBurn](https://www.imgburn.com).
5. Consider configuring your sound card to use IRQ 5 - using IRQ 7 may not always work with this game.

Additionally, as per `vsbhda.txt`, allocates a sound buffer in extended memory, so will benefit from running `XMSRES /L 15` command (see notes in section 4.3.2 in `vsbhda.txt` on "Extended Memory Address").

### Lands of Lore 2

Lands of Lore II works great with VSBCMI and actually sounds much better when configured for 16-bit sound. For MIDI music via the PCI card to work overwrite the following driver files with those found in [Descent Shareware v1.0](https://www.classicdosgames.com/game/Descent.html): 
- HMIDET.386
- HMIDRV.386 
- HMIMDRV.386

### Slipstream 5000

In fact a similar driver update as described above for Lands of Lore 2 also fixes MIDI playback with Slipsteram 5000 shareware version. It does not seem to be reqired for the full version of the game.

### Archimedean Dynasty / Schleichfahrt

Archimedean Dynasty / Schleichfahrt have two sets of `.EXE` files each:

- Default `.EXE` files bound to CauseWay DOS extender
  - `SF.EXE`
  - `AD.EXE`
- Files ending with `4G`, bound to DOS/4GW
  - `SG4G.EXE`
  - `AD4G.EXE`

While both sets work with `VSBCMI`, the ones ending with `4G` are recommended.   
To run the CauseWay-bound versions, issue `JEMMEX.EXE NOVCMI` command first.

### Dark Forces

If one wants music via external MIDI device in Dark Forces there's the [Dark Forces DeHacker](https://ctpax-cheater.losthost.org/htmldocs/trouble.htm#df) power tool that one can patch IMUSE.EXE with. 

### Tie Fighter 

But what about Tie Fighter? Look no further than [TIEIMUSE.COM](https://ludicrous-site.vercel.app/other#tie-fighter-cd-rom-midi-fix), which patches Tie Fighter CD-ROM version of IMUSE.EXE, thus preventing the game from entering an infinite loop. Alternatively, [an iMuse patcher](https://ctpax-cheater.losthost.org/htmldocs/trouble.htm#imusefix) is available from the same group that created Dark Forcer DeHacker.

### X-Wing

For Collector's Edition CD-ROM no special steps are required.   
Floppy version, however, needs VSBCMI started with `/CF1` option.

### Monkey Island 2

As with X-Wing above, with floppy version `/CF1` option is requried with VSBCMI. This applies also if game has \@NewRisingSun sound drivers patch applied.   
CD-ROM version from 1996 does not require the special option.   
The "Ultimate Talkie Edition" also does not require the special option, but may need system slowdown utility in order to work correctly with digitized speech. [CpuSpd](https://www.vogons.org/viewtopic.php?t=74359) is a recommended tool for that.

### Kasparov's Gambit

Official 1.1 patch is requried to fix issues with video playback that the game has. Get it from, e.g., from The Patches Scrolls.

### Black Zone

Black Zone does not work with currently released version of `HDPMI32I.EXE` and needs a special build, posted by the maintainer in [this issue discussion](https://github.com/Baron-von-Riedesel/VSBHDA/issues/54). With this version of DOS extender, VSBCMI also needs to be started with options `/OPL0 /CF1 /DF10` (i.e. with FM port forwarding) _or_ `/OPL1 /CF1` (i.e. with software FM emulation).   
While this allows playing the game with sound and music, __a lot__ can be done to improve how the game sounds. See [this fan site](https://black-zone-shrine.vercel.app) for details. 

### Electro Man

It is recommended that the game is used with C-Media proprietary driver instead of VSBCMI.
If crashes or loss of keyboard control are observed after some time spent playing (also even with C-Medie driver), 
switching from FreeDOS to MS-DOS may help.
The same could apply for other X-Land games, such as "The Adventures of Robbo" and "Heartlight".

### Whizz

Game is not compatible with FreeDOS and requires MS-DOS to run.
It also fails to detect SoundBlaster when `/CF1` option is given to VSBCMI.


### Quake

_-sspeed option and sound pack mods_



2025,  
[CC BY-NC]( "https://creativecommons.org/licenses/by-nc/4.0/),  
Ludicrous_peridot

