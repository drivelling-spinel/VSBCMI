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
* [VSBCMI 1.8.4](https://github.com/drivelling-spinel/VSBCMI/releases/tag/v1.8.4.103-beta) or later.   
  See __CM8738-howto__ for details on configuring VSBCMI. 

## Configuration and startup

* ``USBUHCIL``
* _optionally in case a USB hub is connected_ ``USBHUB``
* ``USBJSTIK``
* Finally, start ``VSBCMI`` as usual, only appending ``/JXX`` command line argument,   
  XX being a number from 1 to 16 that corresponds to "speed rating" of the machine.
  The faster the machine the bigger the recommended number is.
  For example, for a Core 2 Quad Q9550 CPU a value of 10 works.
  For odler CPUs a smaller value is recommended, and for faster machines, a greater one.
  
## Altering joystick mappings

  Use the following command to update joystick mappings. No reboot is requied in this case.   
  ```USBJSTIK < CONFIG.CFG```   
  As an example, ```CONFIG.CFG``` contents follow:   

          ;P:20
          Map A01 from Joy0,Btn,0
          Map A02 from Joy0,Btn,3
          Map B01 from Joy0,Btn,1
          Map B02 from Joy0,Btn,4
          Map BX from Joy0,Btn,2,6
          Map BY from Joy0,Btn,5,7

2026,  
[CC BY-NC]( "https://creativecommons.org/licenses/by-nc/4.0/),  
Ludicrous_peridot

