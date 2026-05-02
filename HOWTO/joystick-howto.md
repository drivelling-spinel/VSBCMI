# VSBCMI and joystick HOWTO 

## The fine print

_The guide is licensed by it's author under the terms of [CC BY-NC license](https://creativecommons.org/licenses/by-nc/4.0/). The author is not affiliated with Microsoft, IBM, C-Media Electronics or any other bodies that hold right over any of the commercial works (software, intellectual or otherwise) referenced in the guide. No claims are being made over the rights (full or partial) for any commercial works (software, intellectual or otherwise) referenced in this guide and they remain with the owning bodies in accorance with the appropriate law (or laws). The guide is presented in hopes of being useful, but with no warranty, nor even implied warranty of merchantability of fitness for a particular purpose. The author will not be held accountable for any damage caused to the persons, their data, hardware or any other property, following any actions taken by those persons in connection (direct or indirect) with activities described in the guide._

## Prereqiuisites

* USB joystick or "controller" (gamepad) that is compatible with __USB 1__ standard
* PC that has USB 1 (UHCI) USB controller to connect the joystick to
* [Bret Johnson's USB drivers for DOS](https://bretjohnson.us)   
  _Note that while similar drivers package may be available on the target machine, or as part of FreeDOS distribution
  VSBCMI has only been tested with the specific version of the driver availble on Bret Johnson's homepage
  at the moment of writing so may not work equally well with other package versions._
* [VSBCMI 1.8.4](https://github.com/drivelling-spinel/VSBCMI/releases/tag/v1.8.4.103) or later.   
  See __CM8738-howto__ for details on configuring VSBCMI. Additionally, consult __games-howto__ on individual game compatibility status.
* _Optionally_, e.g. for Saitek ST30 (aka Impact X7-33U) Plug & Play USB Joystick, apply the patch found in VSBCMI distribution:   
  * Copy `JOYPATCH.COM` to the same directory where `USBJSTIK.COM` is found
  * Back up the original `USBJSTIK.COM`
  * Run `JOYPATCH.COM`


## Configuration and startup

* ``USBUHCIL``
* _optionally in case joysticks are connected to a USB hub_ ``USBHUB``
* ``USBJSTIK``
* Start ``VSBCMI`` normally, also appending ``/JXX`` command line argument,   
  XX being a number from 1 to 16 that corresponds to "speed rating" of the machine.
  The faster the machine the bigger the recommended number is.
  For example, for a Core 2 Quad Q9550 CPU a value of 9 works.
  For older CPUs a smaller value is recommended, and for faster machines, a bigger one.
  
If loading the above via `AUTOEXEC.BAT` on startup causes crashes, consider placing USB- and VSBHDA-related commands _after_ CD-ROM driver.
  
## Altering joystick mappings

  Use the following command to update joystick mappings. No reboot is requied in this case.   
  ```USBJSTIK < CONFIG.CFG```   
  As an example, ```CONFIG.CFG``` contents follow, where joystick buttons are remapped:   

          ;P:20
          Map A01 from Joy0,Btn,0
          Map A02 from Joy0,Btn,3
          Map A03 from Joy0,Btn,1
          Map A04 from Joy0,Btn,4

  `USBINTRO.DOC` included with USBDOS package has detailed information on setting up the mappings.  
  Below several ready to use recipes are included:
  
### Joystick mapping recipes

#### Two joysticks

For head to head battles on the same PC. Tested with __Wacky Wheels__, __Speedball II__ and 
__Super Street Fighter II The New Challengers__.

         Map AX from Joy0,Axis,0
         Map AY from Joy0,Axis,1
         Map A01 from Joy0,Btn,0
         Map A02 from Joy0,Btn,1
         Map B01 from Joy1,Btn,0
         Map B02 from Joy1,Btn,1
         Map BX from Joy1,Axis,0
         Map BY from Joy1,Axis,1

#### Thrustmaster joystick

Configures POV hat and slider. Tested with __Wing Commander III__ (Thrustmaster joystick patch applied),   
and also in __Allegro__ setup utility, where this type of joystick is referred to as Logitech Wingman Extreme.   

         T:Y
         Map AX Joy 0 Axis 0
         Map AY Joy 0 Axis 1
         Map AZ Joy 0 Axis 2
         Map BX Joy 0 Dpad 0 X
         Map BY Joy 0 Dpad 0 Y

#### Capcom gamepad

For 6-button joysticks / gamepads - tested with __Super Street Fighter II The New Challengers__.   

         Map A01 from Joy0,Btn,0
         Map A02 from Joy0,Btn,3
         Map B01 from Joy0,Btn,1
         Map B02 from Joy0,Btn,4
         Map BX from Joy0,Btn,2,6
         Map BY from Joy0,Btn,5,7


## Game compatibility

### Super Steet Fighter II The New Challengers

The game is compatbile with VSBCMI and USBJSTIK, but has a bug preventing two players from using joysticks for head to head battles. 
A patch fixing the bug has been provided on VOGONS message board.

### Whizz

Whizz, in unmodified form, does not work with USBJSTIK (even if the steps from __games-howto__ have been followed). 
An unoficial patch has been produced that fixes the issue, that is yet to be published.

### Incompatible games

In addition to the games listed as incompatible with VSBCMI in __games-howto__, the following games do not work if option `/J` is given:

* Sam & Max Hit the Road
* Day of the Tentacle

2026,  
[CC BY-NC]( "https://creativecommons.org/licenses/by-nc/4.0/),  
Ludicrous_peridot

